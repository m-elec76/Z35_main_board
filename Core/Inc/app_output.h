/**
 ******************************************************************************
 * @file    app_output.h
 * @brief   Actuator control task: heaters, humidity, fan, timers.
 ******************************************************************************
 */

#ifndef APP_OUTPUT_H
#define APP_OUTPUT_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  FreeRTOS task entry point.
 * @param  pvParameters  Unused.
 */
void Output_Manager(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* APP_OUTPUT_H */
