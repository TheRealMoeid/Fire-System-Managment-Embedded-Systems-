// interfaces.h
#ifndef INTERFACES_H
#define INTERFACES_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"

// ---------- Data Queues ----------
extern QueueHandle_t xQueueGas;     // Sends gas ADC value (uint16_t)
extern QueueHandle_t xQueueTemp;    // Sends temperature value (float)
extern QueueHandle_t xQueueCommand; // Sends command to actuators (uint8_t)

// ---------- Binary Semaphore ----------
extern SemaphoreHandle_t xSemaphoreFlame; // Flame interrupt activation

// ---------- System Commands ----------
#define CMD_NORMAL   0
#define CMD_WARNING  1
#define CMD_FIRE     2

// ---------- Risk Levels (Synced with member 2) ----------
typedef enum {
    RISK_NONE,
    RISK_LOW,
    RISK_MEDIUM,
    RISK_HIGH
} RiskLevel_t;

#endif
