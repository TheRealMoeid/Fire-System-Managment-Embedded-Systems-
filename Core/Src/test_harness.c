#include "test_harness.h"
#include "interfaces.h"
#include "main.h"
#include "stm32f4xx_hal.h"   // for GPIO read

/* TODO: When Team Member 4 (Actuators) provides the pin definitions,
   uncomment the following lines and adjust the pin names accordingly.
   Currently we assume the pins are named WATER_PUMP_Pin and BUZZER_Pin
   and their respective ports.
*/
// #include "actuators.h"   // TODO: uncomment after member 4 delivers

// Temporarily define the pins (these should match CubeMX settings)
#ifndef WATER_PUMP_Pin
#define WATER_PUMP_Pin       GPIO_PIN_1
#define WATER_PUMP_GPIO_Port GPIOB
#endif

#ifndef BUZZER_Pin
#define BUZZER_Pin           GPIO_PIN_0
#define BUZZER_GPIO_Port     GPIOB
#endif

// Extern variables
extern volatile uint8_t g_systemState;
extern void debug_print(const char *format, ...);
extern void LogEvent(const char *event);

void vTaskTestHarness(void *pvParameters) {
    uint8_t lastState = 0xFF;
    uint32_t fireStartTime = 0;
    uint8_t fireActive = 0;

    debug_print("[TestHarness] Started monitoring...\r\n");
    LogEvent("Test Harness started");

    for(;;) {
        vTaskDelay(pdMS_TO_TICKS(500));   // Check every 500 ms

        // Log state changes
        if (g_systemState != lastState) {
            debug_print("[TestHarness] State changed: %d -> %d\r\n", lastState, g_systemState);
            lastState = g_systemState;
        }

        // Test 1: During FIRE state, the water pump must be ON
        if (g_systemState == CMD_FIRE) {
            if (!fireActive) {
                fireStartTime = xTaskGetTickCount();
                fireActive = 1;
                debug_print("[TestHarness] Fire state entered. Pump should be ON.\r\n");
            }

            // After 1 second, verify the pump is active
            if ((xTaskGetTickCount() - fireStartTime) > pdMS_TO_TICKS(1000)) {
                if (HAL_GPIO_ReadPin(WATER_PUMP_GPIO_Port, WATER_PUMP_Pin) == GPIO_PIN_RESET) {
                    debug_print("[TestHarness] FAIL: Pump is OFF during FIRE!\r\n");
                    LogEvent("TEST FAIL: Pump off during fire");
                } else {
                    debug_print("[TestHarness] PASS: Pump is ON during FIRE.\r\n");
                }
                // Prevent repeated messages
                fireStartTime = xTaskGetTickCount();
            }
        } else {
            if (fireActive) {
                uint32_t elapsed = xTaskGetTickCount() - fireStartTime;
                // Expect pump to stay on for at least 5 seconds after fire ends (safety)
                if (elapsed < pdMS_TO_TICKS(5000)) {
                    debug_print("[TestHarness] WARN: Pump turned off before safety window.\r\n");
                    LogEvent("TEST WARN: Pump safety window violated");
                } else {
                    debug_print("[TestHarness] PASS: Pump stayed on for safety period.\r\n");
                }
                fireActive = 0;
            }
        }

        // Test 2: In WARNING state, buzzer should beep intermittently (not tested here, just note)
        // This can be extended when buzzer logic is implemented by member 4.
    }
}
