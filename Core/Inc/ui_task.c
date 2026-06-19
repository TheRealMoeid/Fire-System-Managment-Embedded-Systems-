// ui_task.c
// -------------------------------------------------------
// UI Task for Smart Fire Management System
// Handles: LCD 16x2 (I2C) + UART serial log
// Board: STM32F411 | RTOS: FreeRTOS
// -------------------------------------------------------

#include "ui_task.h"
#include "usart.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>

// -------------------------------------------------------
// LCD Low-Level Driver (I2C with PCF8574 expander)
// Pin mapping (standard for most I2C LCD modules):
//   P0 = RS, P1 = RW, P2 = EN
//   P3 = Backlight, P4=D4, P5=D5, P6=D6, P7=D7
// -------------------------------------------------------

#define LCD_BACKLIGHT  0x08
#define LCD_EN         0x04
#define LCD_RW         0x02
#define LCD_RS         0x01

static void LCD_SendByte(uint8_t data) {
    HAL_I2C_Master_Transmit(&hi2c1, LCD_I2C_ADDR, &data, 1, 10);
}

static void LCD_Pulse(uint8_t data) {
    LCD_SendByte(data | LCD_EN);
    HAL_Delay(1);
    LCD_SendByte(data & ~LCD_EN);
    HAL_Delay(1);
}

static void LCD_Send4Bits(uint8_t data, uint8_t mode) {
    uint8_t highNibble = (data & 0xF0) | LCD_BACKLIGHT | mode;
    LCD_Pulse(highNibble);
}

static void LCD_SendNibble(uint8_t data, uint8_t mode) {
    uint8_t highNibble = (data & 0xF0) | LCD_BACKLIGHT | mode;
    uint8_t lowNibble  = ((data << 4) & 0xF0) | LCD_BACKLIGHT | mode;
    LCD_Pulse(highNibble);
    LCD_Pulse(lowNibble);
}

static void LCD_Cmd(uint8_t cmd) {
    LCD_SendNibble(cmd, 0x00);  // RS=0 for command
}

static void LCD_Char(uint8_t ch) {
    LCD_SendNibble(ch, LCD_RS);  // RS=1 for data
}

static void LCD_Init(void) {
    HAL_Delay(50);

    // Init sequence for 4-bit mode
    LCD_Send4Bits(0x30, 0x00); HAL_Delay(5);
    LCD_Send4Bits(0x30, 0x00); HAL_Delay(1);
    LCD_Send4Bits(0x30, 0x00); HAL_Delay(1);
    LCD_Send4Bits(0x20, 0x00); HAL_Delay(1); // Switch to 4-bit

    LCD_Cmd(0x28); // 4-bit, 2 lines, 5x8 font
    LCD_Cmd(0x0C); // Display ON, cursor OFF
    LCD_Cmd(0x06); // Increment cursor, no shift
    LCD_Cmd(0x01); // Clear display
    HAL_Delay(2);
}

static void LCD_SetCursor(uint8_t col, uint8_t row) {
    uint8_t rowOffset[] = {0x00, 0x40};
    LCD_Cmd(0x80 | (col + rowOffset[row]));
}

static void LCD_Print(const char *str) {
    while (*str) {
        LCD_Char((uint8_t)(*str++));
    }
}

// Clears one row by writing 16 spaces
static void LCD_ClearRow(uint8_t row) {
    LCD_SetCursor(0, row);
    LCD_Print("                ");  // 16 spaces
}

// -------------------------------------------------------
// UART Helper
// -------------------------------------------------------
static void UART_Print(const char *msg) {
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);
}

// -------------------------------------------------------
// UI Display Logic
// -------------------------------------------------------

static void UI_ShowNormal(uint16_t gas, float temp) {
    char line1[17];
    char line2[17];
    char uartMsg[64];

    // Line 1: Status
    snprintf(line1, sizeof(line1), "Status: NORMAL  ");

    // Line 2: Sensor readings
    snprintf(line2, sizeof(line2), "G:%-4d T:%-4.1fC  ", gas, temp);

    LCD_ClearRow(0);
    LCD_SetCursor(0, 0);
    LCD_Print(line1);

    LCD_ClearRow(1);
    LCD_SetCursor(0, 1);
    LCD_Print(line2);

    // UART log
    snprintf(uartMsg, sizeof(uartMsg), "[UI] NORMAL | Gas=%d Temp=%.1f\r\n", gas, temp);
    UART_Print(uartMsg);
}

static void UI_ShowWarning(uint16_t gas, float temp) {
    char line2[17];
    char uartMsg[64];

    LCD_ClearRow(0);
    LCD_SetCursor(0, 0);
    LCD_Print("!! WARNING !!   ");

    snprintf(line2, sizeof(line2), "G:%-4d T:%-4.1fC  ", gas, temp);
    LCD_ClearRow(1);
    LCD_SetCursor(0, 1);
    LCD_Print(line2);

    snprintf(uartMsg, sizeof(uartMsg), "[UI] WARNING | Gas=%d Temp=%.1f\r\n", gas, temp);
    UART_Print(uartMsg);
}

static void UI_ShowFire(uint16_t gas, float temp) {
    char uartMsg[64];

    LCD_ClearRow(0);
    LCD_SetCursor(0, 0);
    LCD_Print("*** FIRE !!!  **");

    LCD_ClearRow(1);
    LCD_SetCursor(0, 1);
    LCD_Print("EVACUATE NOW!   ");

    snprintf(uartMsg, sizeof(uartMsg), "[UI] *** FIRE *** | Gas=%d Temp=%.1f\r\n", gas, temp);
    UART_Print(uartMsg);
}

// -------------------------------------------------------
// vTaskUI - Main UI Task
// Register this in freertos.c with Normal priority
// -------------------------------------------------------

// These globals are defined in fire_logic.c / main.c
extern volatile uint16_t g_gasADC;
extern volatile float    g_temperature;

void vTaskUI(void *pvParameters) {
    uint8_t cmd = CMD_NORMAL;
    uint8_t lastCmd = 0xFF;  // Force first draw
    uint32_t lastUartTick = 0;

    LCD_Init();

    LCD_SetCursor(0, 0);
    LCD_Print("Fire Monitor    ");
    LCD_SetCursor(0, 1);
    LCD_Print("Initializing... ");
    HAL_Delay(1500);

    UART_Print("[UI] Task started.\r\n");

    for (;;) {
        // Read latest command from queue (wait up to 200ms)
        if (xQueuePeek(xQueueCommand, &cmd, pdMS_TO_TICKS(200)) != pdTRUE) {
            cmd = CMD_NORMAL;
        }

        uint16_t gas  = g_gasADC;
        float    temp = g_temperature;

        // Only redraw LCD if state changed
        // For UART: always log every 2 seconds
        uint8_t stateChanged = (cmd != lastCmd);
        uint8_t timeToLog = ((xTaskGetTickCount() - lastUartTick) > pdMS_TO_TICKS(2000));

        if (stateChanged || timeToLog) {
            switch (cmd) {
                case CMD_FIRE:
                    UI_ShowFire(gas, temp);
                    break;
                case CMD_WARNING:
                    UI_ShowWarning(gas, temp);
                    break;
                case CMD_NORMAL:
                default:
                    UI_ShowNormal(gas, temp);
                    break;
            }

            lastCmd = cmd;
            if (timeToLog) {
                lastUartTick = xTaskGetTickCount();
            }
        }

        vTaskDelay(pdMS_TO_TICKS(250));
    }
}
