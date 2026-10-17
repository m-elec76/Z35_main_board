/**
 ******************************************************************************
 * @file    app_output.c
 * @brief   Actuator control task (heaters, timers, etc).
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#include "app_output.h"
#include "app_config.h"
#include "app_globals.h"

/*==============================================================================
 *                          TASK
 *============================================================================*/
void Output_Manager(void *pvParameters)
{
    (void)pvParameters;

    /* ---- Local interval state (kept across loop iterations) ---------- */
    static int8_t T1HourOn  = 0, T1MinOn  = 0;
    static int8_t T1HourOff = 0, T1MinOff = 0;
    static int8_t T2HourOn  = 0, T2MinOn  = 0;
    static int8_t T2HourOff = 0, T2MinOff = 0;

    for (;;)
    {
        /*==============================================================
         *                  HEATER & GAS HEATER
         *============================================================*/
        if ((rxData.Temperature / 10.0f) <= LowTemp)
        {
            (void)HAL_GPIO_WritePin(HEATER,     GPIO_PIN_RESET);
            (void)HAL_GPIO_WritePin(GAS_HEATER, GPIO_PIN_RESET);
        }
        else if ((rxData.Temperature / 10.0f) >= HighTemp)
        {
            (void)HAL_GPIO_WritePin(HEATER,     GPIO_PIN_SET);
            (void)HAL_GPIO_WritePin(GAS_HEATER, GPIO_PIN_SET);
        }
        else
        {
            /* Hysteresis band: keep current state. */
        }

        /*==============================================================
         *                      HUMIDIFIER
         *============================================================*/
        if ((int)(rxData.humidity / 10) <= (int)LowHumi)
        {
            (void)HAL_GPIO_WritePin(HUMIDIFIER, GPIO_PIN_RESET);
        }
        else if ((int)(rxData.humidity / 10) >= (int)HighHumi)
        {
            (void)HAL_GPIO_WritePin(HUMIDIFIER, GPIO_PIN_SET);
        }
        else
        {
            /* Hysteresis band: keep current state. */
        }

        /*==============================================================
         *                      EMERGENCY FAN
         *============================================================*/
        if ((rxData.Temperature / 10.0f) >= EmergencyTemp)
        {
            (void)HAL_GPIO_WritePin(EMERGENCY_FAN, GPIO_PIN_RESET);
        }
        else if ((rxData.Temperature / 10.0f) <= HighTemp)
        {
            (void)HAL_GPIO_WritePin(EMERGENCY_FAN, GPIO_PIN_SET);
        }
        else
        {
            /* Hysteresis band: keep current state. */
        }

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> TIMER 1 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        /* ---------------- Timer1: OFF --------------------------------- */
        if (Timer1_Status == 0)
        {
            (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_SET);
        }

        /* ---------------- Timer1: two-stage, night-mode OFF ----------- */
        if ((Timer1_Status == 1) && (Timer1_NightStatus == 0))
        {
            if (StartCalculateT1 == 0U)
            {
                T1HourOn = CurrentTime.Hours   + Timer1_onHour;
                T1MinOn  = CurrentTime.Minutes + Timer1_onMin;

                if (T1MinOn > 59)  { T1MinOn  -= 60; T1HourOn++; }
                if (T1HourOn > 23) { T1HourOn -= 24; }

                (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_RESET);
                CalculateTimer1ON  = 0U;
                CalculateTimer1OFF++;
                StartCalculateT1++;
            }
            else if ((CurrentTime.Hours   == T1HourOn) &&
                     (CurrentTime.Minutes == T1MinOn)  &&
                     (CalculateTimer1ON == 0U))
            {
                T1HourOff = CurrentTime.Hours   + Timer1_offHour;
                T1MinOff  = CurrentTime.Minutes + Timer1_offMin;

                if (T1MinOff > 59)  { T1MinOff  -= 60; T1HourOff++; }
                if (T1HourOff > 23) { T1HourOff -= 24; }

                (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_SET);
                CalculateTimer1OFF = 0U;
                CalculateTimer1ON++;
            }
            else if ((CurrentTime.Hours   == T1HourOff) &&
                     (CurrentTime.Minutes == T1MinOff)  &&
                     (CalculateTimer1OFF == 0U))
            {
                T1HourOn = CurrentTime.Hours   + Timer1_onHour;
                T1MinOn  = CurrentTime.Minutes + Timer1_onMin;

                if (T1MinOn > 59)  { T1MinOn  -= 60; T1HourOn++; }
                if (T1HourOn > 23) { T1HourOn -= 24; }

                (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_RESET);
                CalculateTimer1ON  = 0U;
                CalculateTimer1OFF++;
            }
            else
            {
                /* Nothing to do. */
            }
        }

        /* ---------------- Timer1: two-stage, night-mode ON ------------ */
        if ((Timer1_Status == 1) && (Timer1_NightStatus == 1) && (NightStatus == 0U))
        {
            if (StartCalculateT1 == 0U)
            {
                T1HourOn = CurrentTime.Hours   + Timer1_onHour;
                T1MinOn  = CurrentTime.Minutes + Timer1_onMin;

                if (T1MinOn > 59)  { T1MinOn  -= 60; T1HourOn++; }
                if (T1HourOn > 23) { T1HourOn -= 24; }

                (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_RESET);
                CalculateTimer1ON  = 0U;
                CalculateTimer1OFF++;
                StartCalculateT1++;
            }
            else if ((CurrentTime.Hours   == T1HourOn) &&
                     (CurrentTime.Minutes == T1MinOn)  &&
                     (CalculateTimer1ON == 0U))
            {
                T1HourOff = CurrentTime.Hours   + Timer1_offHour;
                T1MinOff  = CurrentTime.Minutes + Timer1_offMin;

                if (T1MinOff > 59)  { T1MinOff  -= 60; T1HourOff++; }
                if (T1HourOff > 23) { T1HourOff -= 24; }

                (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_SET);
                CalculateTimer1OFF = 0U;
                CalculateTimer1ON++;
            }
            else if ((CurrentTime.Hours   == T1HourOff) &&
                     (CurrentTime.Minutes == T1MinOff)  &&
                     (CalculateTimer1OFF == 0U))
            {
                T1HourOn = CurrentTime.Hours   + Timer1_onHour;
                T1MinOn  = CurrentTime.Minutes + Timer1_onMin;

                if (T1MinOn > 59)  { T1MinOn  -= 60; T1HourOn++; }
                if (T1HourOn > 23) { T1HourOn -= 24; }

                (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_RESET);
                CalculateTimer1ON  = 0U;
                CalculateTimer1OFF++;
            }
            else
            {
                /* Nothing to do. */
            }
        }

        /* ---------------- Timer1: force off during night -------------- */
        if ((Timer1_Status == 1) && (Timer1_NightStatus == 1) && (NightStatus == 1U))
        {
            (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_SET);
        }

        /* ---------------- Timer1: clock mode -------------------------- */
        if (Timer1_Status == 2)
        {
            if ((CurrentTime.Hours == Timer1_onHour) && (CurrentTime.Minutes == Timer1_onMin))
            {
                (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_RESET);
            }
            else if ((CurrentTime.Hours == Timer1_offHour) && (CurrentTime.Minutes == Timer1_offMin))
            {
                (void)HAL_GPIO_WritePin(TIMER1_OUT, GPIO_PIN_SET);
            }
            else
            {
                /* Nothing to do. */
            }
        }

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> TIMER 2 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        /* ---------------- Timer2: OFF --------------------------------- */
        if (Timer2_Status == 0)
        {
            (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_SET);
        }

        /* ---------------- Timer2: two-stage, night-mode OFF ----------- */
        if ((Timer2_Status == 1) && (Timer2_NightStatus == 0))
        {
            if (StartCalculateT2 == 0U)
            {
                T2HourOn = CurrentTime.Hours   + Timer2_onHour;
                T2MinOn  = CurrentTime.Minutes + Timer2_onMin;

                if (T2MinOn > 59)  { T2MinOn  -= 60; T2HourOn++; }
                if (T2HourOn > 23) { T2HourOn -= 24; }

                (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_RESET);
                CalculateTimer2ON  = 0U;
                CalculateTimer2OFF++;
                StartCalculateT2++;
            }
            else if ((CurrentTime.Hours   == T2HourOn) &&
                     (CurrentTime.Minutes == T2MinOn)  &&
                     (CalculateTimer2ON == 0U))
            {
                T2HourOff = CurrentTime.Hours   + Timer2_offHour;
                T2MinOff  = CurrentTime.Minutes + Timer2_offMin;

                if (T2MinOff > 59)  { T2MinOff  -= 60; T2HourOff++; }
                if (T2HourOff > 23) { T2HourOff -= 24; }

                (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_SET);
                CalculateTimer2OFF = 0U;
                CalculateTimer2ON++;
            }
            else if ((CurrentTime.Hours   == T2HourOff) &&
                     (CurrentTime.Minutes == T2MinOff)  &&
                     (CalculateTimer2OFF == 0U))
            {
                T2HourOn = CurrentTime.Hours   + Timer2_onHour;
                T2MinOn  = CurrentTime.Minutes + Timer2_onMin;

                if (T2MinOn > 59)  { T2MinOn  -= 60; T2HourOn++; }
                if (T2HourOn > 23) { T2HourOn -= 24; }

                (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_RESET);
                CalculateTimer2ON  = 0U;
                CalculateTimer2OFF++;
            }
            else
            {
                /* Nothing to do. */
            }
        }

        /* ---------------- Timer2: two-stage, night-mode ON ------------ */
        if ((Timer2_Status == 1) && (Timer2_NightStatus == 1) && (NightStatus == 0U))
        {
            if (StartCalculateT2 == 0U)
            {
                T2HourOn = CurrentTime.Hours   + Timer2_onHour;
                T2MinOn  = CurrentTime.Minutes + Timer2_onMin;

                if (T2MinOn > 59)  { T2MinOn  -= 60; T2HourOn++; }
                if (T2HourOn > 23) { T2HourOn -= 24; }

                (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_RESET);
                CalculateTimer2ON  = 0U;
                CalculateTimer2OFF++;
                StartCalculateT2++;
            }
            else if ((CurrentTime.Hours   == T2HourOn) &&
                     (CurrentTime.Minutes == T2MinOn)  &&
                     (CalculateTimer2ON == 0U))
            {
                T2HourOff = CurrentTime.Hours   + Timer2_offHour;
                T2MinOff  = CurrentTime.Minutes + Timer2_offMin;

                if (T2MinOff > 59)  { T2MinOff  -= 60; T2HourOff++; }
                if (T2HourOff > 23) { T2HourOff -= 24; }

                (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_SET);
                CalculateTimer2OFF = 0U;
                CalculateTimer2ON++;
            }
            else if ((CurrentTime.Hours   == T2HourOff) &&
                     (CurrentTime.Minutes == T2MinOff)  &&
                     (CalculateTimer2OFF == 0U))
            {
                T2HourOn = CurrentTime.Hours   + Timer2_onHour;
                T2MinOn  = CurrentTime.Minutes + Timer2_onMin;

                if (T2MinOn > 59)  { T2MinOn  -= 60; T2HourOn++; }
                if (T2HourOn > 23) { T2HourOn -= 24; }

                (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_RESET);
                CalculateTimer2ON  = 0U;
                CalculateTimer2OFF++;
            }
            else
            {
                /* Nothing to do. */
            }
        }

        /* ---------------- Timer2: force off during night -------------- */
        if ((Timer2_Status == 1) && (Timer2_NightStatus == 1) && (NightStatus == 1U))
        {
            (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_SET);
        }

        /* ---------------- Timer2: clock mode -------------------------- */
        if (Timer2_Status == 2)
        {
            if ((CurrentTime.Hours == Timer2_onHour) && (CurrentTime.Minutes == Timer2_onMin))
            {
                (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_RESET);
            }
            else if ((CurrentTime.Hours == Timer2_offHour) && (CurrentTime.Minutes == Timer2_offMin))
            {
                (void)HAL_GPIO_WritePin(TIMER2_OUT, GPIO_PIN_SET);
            }
            else
            {
                /* Nothing to do. */
            }
        }
    }
}
