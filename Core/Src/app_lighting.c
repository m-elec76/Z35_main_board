 /**
 ******************************************************************************
 * @file    app_lighting.c
 * @brief   Artificial sunrise / sunset gradient controller.
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 *
 *
 * @details The task sweeps the ARR register of TIM2 between SUN_ARR_MIN and
 *          SUN_ARR_MAX to emulate smooth natural-light ramps.  The FSM state
 *          is stored in CheckTimeSun:
 *
 *            CheckTimeSun = 1  -> sunrise ramp in progress
 *            CheckTimeSun = 2  -> dim-out ramp before sunset
 *            CheckTimeSun = 3  -> default / idle
 *            CheckTimeSun = 4  -> fast ramp-in just before sunset
 *            CheckTimeSun = 5  -> fast ramp finished; arm sunset
 *            CheckTimeSun = 6  -> sunset ramp in progress
 ******************************************************************************
 */

#include "app_lighting.h"
#include "app_config.h"
#include "app_globals.h"
#include "app_utils.h"
#include "tim.h"
#include "cmsis_os.h"

/*==============================================================================
 *                          LOCAL CONSTANTS
 *============================================================================*/
#define RAMP_STEP_DELAY_MS      (5900U)
#define FAST_RAMP_STEP_DELAY_MS (200U)
#define SETTLE_DELAY_MS         (61000U)
#define LIGHTING_IDLE_DELAY_MS  (1U)

/*==============================================================================
 *                          LOCAL HELPER: STOP SUN OUTPUT
 *============================================================================*/
/**
 * @brief  Turn off the sun PWM output and disarm the voltage-loss IRQ.
 */
static void Lighting_StopSunOutput(void)
{
    Disable_DetectVoltage_Interrupt();
    (void)HAL_TIM_Base_Stop_IT(&htim2);
    (void)HAL_GPIO_WritePin(SUN_LAMP, GPIO_PIN_RESET);
    (void)vTaskPrioritySet(NULL, 1U);
}

/*==============================================================================
 *                          TASK
 *============================================================================*/
void Lighting_Manager(void *pvParameters)
{
    (void)pvParameters;

    /* ---- Local state ------------------------------------------------- */
    static uint8_t    EnableInterruptDV = 0U;   /**< Arm IRQ only once            */
    static uint8_t    circle_minute     = 0U;   /**< 1-min-before-sunset minute   */
    static uint8_t    circle_hour       = 0U;   /**< 1-min-before-sunset hour     */
    static TickType_t OneMinDelay       = 0U;

    /* ---- Outputs at startup ----------------------------------------- */
    (void)HAL_GPIO_WritePin(SUN_LAMP, GPIO_PIN_RESET);
    Disable_DetectVoltage_Interrupt();

    for (;;)
    {
        /* -------------------------------------------------------------
         * Compute the "1 minute before sunset" reference time.
         * -----------------------------------------------------------*/
        if (SunsetMinute == 0)
        {
            circle_minute = 59U;
            circle_hour   = (uint8_t)(SunsetHour - 1);
        }
        else
        {
            circle_minute = (uint8_t)(SunsetMinute - 1);
            circle_hour   = (uint8_t)SunsetHour;
        }

        /* -------------------------------------------------------------
         * Refresh day / night state (0 = day, 1 = night).
         * -----------------------------------------------------------*/
        NightStatus = (uint8_t)calculate_night_day();

        /*==============================================================
         *                          DAYTIME
         *============================================================*/
        if (NightStatus == 0U)
        {
            /* --- Arm the sunrise ramp at the configured sunrise time --- */
            if ((CurrentTime.Hours   == SunriseHour) &&
                (CurrentTime.Minutes == SunriseMinute) &&
                (EnableInterruptDV == 0U))
            {
                if ((Timer1_Status == 1) && (Timer1_NightStatus == 1))
                {
                    StartCalculateT1 = 0U;
                }
                if ((Timer2_Status == 1) && (Timer2_NightStatus == 1))
                {
                    StartCalculateT2 = 0U;
                }

                Enable_DetectVoltage_Interrupt();
                (void)vTaskPrioritySet(NULL, 3U);
                EnableInterruptDV++;
                CheckTimeSun = 1U;
            }

            /* --- Sunrise phase: decrease ARR from 315 down to 80 --------*/
            if (CheckTimeSun == 1U)
            {
                PowerOutage = 0U;
                (void)HAL_GPIO_WritePin(LAMP_NIGHT, GPIO_PIN_RESET);
                (void)HAL_GPIO_WritePin(LAMP_DAY,   GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(RAMP_STEP_DELAY_MS));

                if (timer2ARRControl > 0U) { timer2ARRControl--; }

                if (PowerOutage != 0U)
                {
                    Lighting_StopSunOutput();
                    OneMinDelay = xTaskGetTickCount();
                    vTaskDelayUntil(&OneMinDelay, pdMS_TO_TICKS(SETTLE_DELAY_MS));
                    timer2ARRControl = 315U;
                    CheckTimeSun     = 3U;
                    EnableInterruptDV= 0U;
                    PowerOutage      = 0U;
                }
                else if (timer2ARRControl <= SUN_ARR_MIN)
                {
                    timer2ARRControl = SUN_ARR_MIN;
                    CheckTimeSun     = 2U;          /* Sunrise complete */
                }
                else
                {
                    /* Keep ramping. */
                }
            }

            /* --- Dim-out phase: increase ARR from 80 up to 315 ----------*/
            if (CheckTimeSun == 2U)
            {
                PowerOutage = 0U;
                (void)HAL_GPIO_WritePin(LAMP_NIGHT, GPIO_PIN_SET);
                (void)HAL_GPIO_WritePin(LAMP_DAY,   GPIO_PIN_RESET);
                vTaskDelay(pdMS_TO_TICKS(FAST_RAMP_STEP_DELAY_MS));

                timer2ARRControl++;

                if (PowerOutage != 0U)
                {
                    Lighting_StopSunOutput();
                    OneMinDelay = xTaskGetTickCount();
                    vTaskDelayUntil(&OneMinDelay, pdMS_TO_TICKS(SETTLE_DELAY_MS));
                    timer2ARRControl = 315U;
                    CheckTimeSun     = 3U;
                    EnableInterruptDV= 0U;
                    PowerOutage      = 0U;
                }
                else if (timer2ARRControl >= SUN_ARR_MAX)
                {
                    timer2ARRControl = SUN_ARR_MAX;
                    Lighting_StopSunOutput();
                    CheckTimeSun     = 3U;
                    EnableInterruptDV= 0U;
                }
                else
                {
                    /* Keep ramping. */
                }
            }

            /* --- Idle state --------------------------------------------*/
            if (CheckTimeSun == 3U)
            {
                (void)HAL_GPIO_WritePin(SUN_LAMP,   GPIO_PIN_RESET);
                (void)HAL_GPIO_WritePin(LAMP_NIGHT, GPIO_PIN_SET);
                (void)HAL_GPIO_WritePin(LAMP_DAY,   GPIO_PIN_RESET);
            }

            /* --- Fast ramp-in just before sunset -----------------------*/
            if ((CurrentTime.Hours == circle_hour) &&
                (CurrentTime.Minutes == circle_minute))
            {
                if (EnableInterruptDV == 0U)
                {
                    Enable_DetectVoltage_Interrupt();
                    (void)vTaskPrioritySet(NULL, 3U);
                    CheckTimeSun    = 4U;
                    EnableInterruptDV++;
                }

                if (CheckTimeSun == 4U)
                {
                    PowerOutage = 0U;
                    (void)HAL_GPIO_WritePin(LAMP_DAY,   GPIO_PIN_RESET);
                    (void)HAL_GPIO_WritePin(LAMP_NIGHT, GPIO_PIN_SET);
                    vTaskDelay(pdMS_TO_TICKS(FAST_RAMP_STEP_DELAY_MS));

                    if (timer2ARRControl > 0U) { timer2ARRControl--; }

                    if (PowerOutage != 0U)
                    {
                        Lighting_StopSunOutput();
                        OneMinDelay = xTaskGetTickCount();
                        vTaskDelayUntil(&OneMinDelay, pdMS_TO_TICKS(SETTLE_DELAY_MS));
                        timer2ARRControl = 315U;
                        CheckTimeSun     = 3U;
                        EnableInterruptDV= 0U;
                        PowerOutage      = 0U;
                    }
                    else if (timer2ARRControl <= SUN_ARR_MIN)
                    {
                        timer2ARRControl = SUN_ARR_MIN;
                        CheckTimeSun     = 5U;      /* Fast ramp finished */
                    }
                    else
                    {
                        /* Keep ramping. */
                    }
                }
            }
        }

        /*==============================================================
         *                          NIGHTTIME
         *============================================================*/
        if (NightStatus == 1U)
        {
            /* --- Sunset ramp starts at the configured time ------------ */
            if ((CurrentTime.Hours   == SunsetHour) &&
                (CurrentTime.Minutes == SunsetMinute) &&
                (CheckTimeSun == 5U))
            {
                CheckTimeSun    = 6U;
                EnableInterruptDV = 0U;
            }

            /* --- Sunset phase: increase ARR from 80 up to 315 ----------*/
            if (CheckTimeSun == 6U)
            {
                PowerOutage = 0U;
                (void)HAL_GPIO_WritePin(LAMP_NIGHT, GPIO_PIN_RESET);
                (void)HAL_GPIO_WritePin(LAMP_DAY,   GPIO_PIN_SET);
                vTaskDelay(pdMS_TO_TICKS(RAMP_STEP_DELAY_MS));

                timer2ARRControl++;

                if (PowerOutage != 0U)
                {
                    Lighting_StopSunOutput();
                    OneMinDelay = xTaskGetTickCount();
                    vTaskDelayUntil(&OneMinDelay, pdMS_TO_TICKS(SETTLE_DELAY_MS));
                    timer2ARRControl = 315U;
                    CheckTimeSun     = 3U;
                    EnableInterruptDV= 0U;
                    PowerOutage      = 0U;
                }
                else if (timer2ARRControl >= SUN_ARR_MAX)
                {
                    timer2ARRControl = SUN_ARR_MAX;
                    Lighting_StopSunOutput();
                    CheckTimeSun     = 3U;
                }
                else
                {
                    /* Keep ramping. */
                }
            }

            /* --- Idle state --------------------------------------------*/
            if (CheckTimeSun == 3U)
            {
                (void)HAL_GPIO_WritePin(SUN_LAMP,   GPIO_PIN_RESET);
                (void)HAL_GPIO_WritePin(LAMP_NIGHT, GPIO_PIN_RESET);
                (void)HAL_GPIO_WritePin(LAMP_DAY,   GPIO_PIN_SET);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(LIGHTING_IDLE_DELAY_MS));
    }
}
