// fire_logic.c
#include "fire_logic.h"
#include "main.h"          // For accessing global variables
#include <stdio.h>         // For sprintf
#include <string.h>        // For strlen

// These functions are defined in main.c
extern void debug_print(const char *format, ...);
extern void LogEvent(const char *event);

// Global variables definition (defined in main.c)
extern volatile uint16_t g_gasADC;
extern volatile float g_temperature;
extern volatile uint8_t g_systemState;
extern volatile uint8_t g_flameDetected;

// ---------- Helper functions to peek the latest value from the queue ----------
static uint8_t GetLatestGasValue(uint16_t *value) {
    return (xQueuePeek(xQueueGas, value, 0) == pdTRUE);
}

static uint8_t GetLatestTempValue(float *value) {
    return (xQueuePeek(xQueueTemp, value, 0) == pdTRUE);
}

// ---------- Risk Assessment Function (Temporary logic - to be synced later) ----------
RiskLevel_t AssessRisk(uint16_t gasADC, float tempC, uint8_t flameDetected) {
    // Flame has the highest priority
    if (flameDetected) {
        return RISK_HIGH;
    }

    // Gas thresholds (Sample values for MQ-2)
    // Note: These values must be adjusted with actual sensor calibration
    if (gasADC > 3000) {          // Very thick smoke
        return RISK_HIGH;
    } else if (gasADC > 2000) {   // Medium smoke
        return RISK_MEDIUM;
    } else if (gasADC > 1000) {   // Light smoke
        return RISK_LOW;
    }

    // Temperature thresholds
    if (tempC > 60.0f) {          // Critical temperature
        return RISK_HIGH;
    } else if (tempC > 45.0f) {   // High temperature
        return RISK_MEDIUM;
    } else if (tempC > 35.0f) {   // Slightly high temperature
        return RISK_LOW;
    }

    return RISK_NONE;
}

// ---------- Main State Machine Task (High Priority) ----------
void vTaskFireLogic(void *pvParameters) {
    uint16_t gasVal = 0;
    float tempVal = 25.0f;
    uint8_t flame = 0;
    RiskLevel_t risk;
    uint8_t command;
    char logMsg[64];

    // Reset the command queue at startup
    xQueueReset(xQueueCommand);

    debug_print("[FireLogic] Task started. Waiting for sensor data...\r\n");

    for(;;) {
        // 1. Check the flame semaphore
        if (xSemaphoreTake(xSemaphoreFlame, 0) == pdTRUE) {
            flame = 1;
            g_flameDetected = 1;
            LogEvent("Flame detected by sensor!");
        } else {
            flame = 0;
            g_flameDetected = 0;
        }

        // 2. Get the latest sensor data from queues
        if (GetLatestGasValue(&gasVal)) {
            g_gasADC = gasVal;
        }
        if (GetLatestTempValue(&tempVal)) {
            g_temperature = tempVal;
        }

        // 3. Evaluate risk level using current values
        risk = AssessRisk(g_gasADC, g_temperature, flame);

        // 4. Convert risk level to system command
        switch (risk) {
            case RISK_HIGH:
                command = CMD_FIRE;
                g_systemState = CMD_FIRE;
                sprintf(logMsg, "FIRE! Gas=%d Temp=%.1f", g_gasADC, g_temperature);
                LogEvent(logMsg);
                debug_print("[ALERT] %s\r\n", logMsg);
                break;

            case RISK_MEDIUM:
                command = CMD_WARNING;
                g_systemState = CMD_WARNING;
                sprintf(logMsg, "WARNING: High gas level (%d)", g_gasADC);
                LogEvent(logMsg);
                // Print warning once every 5 seconds
                static uint32_t lastWarnTick = 0;
                if ((xTaskGetTickCount() - lastWarnTick) > pdMS_TO_TICKS(5000)) {
                    debug_print("[WARN] %s\r\n", logMsg);
                    lastWarnTick = xTaskGetTickCount();
                }
                break;

            case RISK_LOW:
                command = CMD_WARNING; // We also issue a warning at low risk
                g_systemState = CMD_WARNING;
                break;

            case RISK_NONE:
            default:
                command = CMD_NORMAL;
                g_systemState = CMD_NORMAL;
                break;
        }

        // 5. Send command to the actuators queue
        xQueueSend(xQueueCommand, &command, 0);

        // 6. 50ms delay (20Hz check frequency)
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
