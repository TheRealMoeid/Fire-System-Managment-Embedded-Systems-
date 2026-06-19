#include "scenario_sim.h"
#include "interfaces.h"
#include "main.h"          // for debug_print()
#include <stdio.h>

// Extern declarations for debug functions in main.c
extern void debug_print(const char *format, ...);
extern void LogEvent(const char *event);

/**
 * @brief  Simulates a complete fire scenario over time.
 * @note   This task injects fake ADC and temperature values directly into
 *         the queues (xQueueGas, xQueueTemp) and triggers the flame semaphore.
 *         It is used for integration testing when real sensors are not available.
 */
void vTaskScenarioSim(void *pvParameters) {
    uint16_t fakeGas = 0;
    float fakeTemp = 25.0f;

    debug_print("[Scenario] Simulator started. Waiting 5 seconds...\r\n");
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Phase 1: Smoke buildup (gas level increasing)
    debug_print("[Scenario] Phase 1: Smoke increasing...\r\n");
    LogEvent("Scenario: Smoke buildup phase");
    for (int i = 0; i < 30; i++) {
        fakeGas += 100;
        if (fakeGas > 4095) fakeGas = 4095;
        xQueueSend(xQueueGas, &fakeGas, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Phase 2: Temperature rising
    debug_print("[Scenario] Phase 2: Temperature rising...\r\n");
    LogEvent("Scenario: Temperature rising phase");
    for (int i = 0; i < 20; i++) {
        fakeTemp += 2.0f;
        if (fakeTemp > 80.0f) fakeTemp = 80.0f;
        xQueueSend(xQueueTemp, &fakeTemp, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Phase 3: Flame detection
    debug_print("[Scenario] Phase 3: Flame detected!\r\n");
    LogEvent("Scenario: Flame triggered");
    xSemaphoreGive(xSemaphoreFlame);
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Phase 4: Fire extinguished (values return to normal)
    debug_print("[Scenario] Phase 4: Fire extinguished...\r\n");
    LogEvent("Scenario: Extinguishing phase");
    for (int i = 0; i < 30; i++) {
        if (fakeGas > 0) fakeGas -= 100;
        xQueueSend(xQueueGas, &fakeGas, 0);
        if (fakeTemp > 25.0f) fakeTemp -= 1.5f;
        xQueueSend(xQueueTemp, &fakeTemp, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    debug_print("[Scenario] Scenario finished.\r\n");
    LogEvent("Scenario: Completed");
    vTaskDelete(NULL);
}
