// fire_logic.h
#ifndef FIRE_LOGIC_H
#define FIRE_LOGIC_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "interfaces.h"

// ---------- Public Functions (for use in freertos.c) ----------
void vTaskFireLogic(void *pvParameters);

// ---------- Risk Assessment Function (currently internal(vagti code bagie omad avaz kon) ----------
RiskLevel_t AssessRisk(uint16_t gasADC, float tempC, uint8_t flameDetected);
/*
 * ============================================================================
 * TODO: INTEGRATION NOTES FOR MEMBER 2 (SENSOR INTERFACE)
 * ============================================================================
 * 
 * IMPORTANT: 
 * Currently, the fire logic system is using a temporary/dummy version of the 
 * 'AssessRisk' function to calculate fire hazards based on mock data. 
 * 
 * Once Member 2 finishes and delivers the official Sensor Interface 
 * (sensors.c and sensors.h), the following steps MUST be taken to integrate 
 * the codes properly:
 * 
 * Step 1: Include the Official Header
 *    Add '#include "sensors.h"' at the top of 'fire_logic.c'.
 * 
 * Step 2: Remove the Dummy Function
 *    Locate the temporary 'AssessRisk' function currently defined in 
 *    'fire_logic.c' and delete it (or comment it out). If you don't do this, 
 *    the compiler will throw a "multiple definition" error.
 * 
 * Step 3: Link to the Official Function
 *    Make sure that the main logic task ('vTaskFireLogic') is now calling 
 *    the official 'AssessRisk' function provided by Member 2.
 * 
 * Step 4: Verify Parameters and Return Types
 *    Double-check that the arguments passed to 'AssessRisk' (like gas and 
 *    temperature values) and its return values (Safe, Warning, Fire) perfectly 
 *    match the definitions in Member 2's header file.
 * 
 * ============================================================================
 */

#endif
