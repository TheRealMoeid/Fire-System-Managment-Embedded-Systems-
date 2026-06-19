// ui_task.h
#ifndef UI_TASK_H
#define UI_TASK_H

#include "FreeRTOS.h"
#include "queue.h"
#include "interfaces.h"

// -------------------------------------------------------
// LCD I2C Address
// Most common values: 0x27 or 0x3F
// If LCD shows nothing, try changing this to 0x3F
// -------------------------------------------------------
#define LCD_I2C_ADDR  (0x27 << 1)  // HAL expects 8-bit address

// LCD dimensions
#define LCD_COLS  16
#define LCD_ROWS  2

// -------------------------------------------------------
// Public function: Register this in freertos.c
// -------------------------------------------------------
void vTaskUI(void *pvParameters);

#endif // UI_TASK_H
