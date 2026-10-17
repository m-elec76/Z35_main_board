/**
 ******************************************************************************
 * @file    app_lighting.h
 * @brief   Artificial sunrise / sunset gradient controller.
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */
#ifndef APP_LIGHTING_H
#define APP_LIGHTING_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  FreeRTOS task entry point.
 * @param  pvParameters  Unused.
 */
void Lighting_Manager(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* APP_LIGHTING_H */
