/**
 ******************************************************************************
 * @file    app_utils.h
 * @brief   Small stateless helper functions shared between tasks.
 ******************************************************************************
 */

#ifndef APP_UTILS_H
#define APP_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/**
 * @brief  Compute whether the RTC time is inside the day window.
 * @return 0 when inside [sunrise, sunset) (day),
 *         1 otherwise (night).
 */
int8_t calculate_night_day(void);

/**
 * @brief  Detach the PC817 voltage-loss interrupt from the NVIC.
 */
void Disable_DetectVoltage_Interrupt(void);

/**
 * @brief  Reattach the PC817 voltage-loss interrupt (rising edge).
 */
void Enable_DetectVoltage_Interrupt(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_UTILS_H */
