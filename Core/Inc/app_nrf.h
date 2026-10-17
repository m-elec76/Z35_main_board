/**
 ******************************************************************************
 * @file    app_nrf.h
 * @brief   nRF24L01+ radio layer and receiver task.
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#ifndef APP_NRF_H
#define APP_NRF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief  Select one of the ten pre-shared network profiles.
 * @param  mode  Valid range is 1..10; out-of-range values fall back to 1.
 */
void apply_mode(uint8_t mode);

/**
 * @brief  FreeRTOS task: consume RX queue and refresh derived sensor values.
 * @param  pvParameters  Unused.
 */
void Task_nRF_Receiver(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* APP_NRF_H */
