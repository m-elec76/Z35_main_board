/**
 ******************************************************************************
 * @file    app_utils.c
 * @brief   Helper functions (day/night calculation, IRQ handling).
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#include "app_utils.h"
#include "app_config.h"
#include "app_globals.h"
#include "main.h"

/*==============================================================================
 *                          DAY / NIGHT CALCULATION
 *============================================================================*/
int8_t calculate_night_day(void)
{
    /* Convert every time-of-day to minutes since midnight. */
    const uint16_t current = (uint16_t)((CurrentTime.Hours * 60U) + CurrentTime.Minutes);
    const uint16_t sunrise = (uint16_t)((SunriseHour     * 60) + SunriseMinute);
    const uint16_t sunset  = (uint16_t)((SunsetHour      * 60) + SunsetMinute);

    if (sunrise < sunset)
    {
        /* Normal case: sunrise occurs before sunset. */
        return ((current >= sunrise) && (current < sunset)) ? 0 : 1;
    }
    else
    {
        /* Inverted case: sunrise occurs after sunset (crosses midnight). */
        return ((current >= sunrise) || (current < sunset)) ? 0 : 1;
    }
}

/*==============================================================================
 *                          DISABLE POWER-LOSS INTERRUPT
 *============================================================================*/
void Disable_DetectVoltage_Interrupt(void)
{
    /* Disable the NVIC line for PA12. */
    EXTI->IMR1 &= ~(1UL << 12U);

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = EXTI_VOLTAGE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    (void)HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/*==============================================================================
 *                          ENABLE POWER-LOSS INTERRUPT
 *============================================================================*/
void Enable_DetectVoltage_Interrupt(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = EXTI_VOLTAGE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    (void)HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Enable the NVIC line for PA12. */
    EXTI->IMR1 |= (1UL << 12U);
}
