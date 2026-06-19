#ifndef SCENARIO_SIM_H
#define SCENARIO_SIM_H

#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief  Simulates a fire scenario by injecting fake sensor data into queues.
 * @param  pvParameters: Not used.
 * @retval None
 */
void vTaskScenarioSim(void *pvParameters);

#endif /* SCENARIO_SIM_H */
