#ifndef TEST_HARNESS_H
#define TEST_HARNESS_H

#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief  Monitors system state and verifies actuator behavior.
 * @param  pvParameters: Not used.
 * @retval None
 */
void vTaskTestHarness(void *pvParameters);

#endif /* TEST_HARNESS_H */
