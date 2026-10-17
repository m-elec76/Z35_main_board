/**
 ******************************************************************************
 * @file    app_setting.c
 * @brief   User interface menu state machine and LCD rendering.
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#include "app_setting.h"
#include "app_config.h"
#include "app_globals.h"
#include "app_nrf.h"
#include "HD44780.h"
#include "ConvertCalendar.h"
#include <stdio.h>
#include <string.h>

/*==============================================================================
 *                          TASK
 *============================================================================*/
void Setting(void *pvParameters)
{
    (void)pvParameters;

    char LCD[17];

    /* ---- Persistent state across loop iterations ------------------- */
    static TickType_t last_wake_time  = 0U;
    static uint16_t   idle_counter    = 0U;
    static uint16_t   LCD_refresh     = 0U;

    static uint8_t    blink_dot_clock = 0U;   /* Blink ':' on idle screen    */
    static uint8_t    Screen          = 0U;   /* 0 = clock, 1 = data         */
    static int8_t     ModifiedCheck   = 0;    /* 0 = none, 1 = clock, 2 = main*/

    static uint16_t   YearCheck       = 0U;
    static uint8_t    MonthCheck      = 0U;
    static uint8_t    dayCheck        = 0U;
    static uint8_t    HourCheck       = 0U;
    static uint8_t    MinCheck        = 0U;

    static int8_t     changed_hour    = 0;
    static int8_t     changed_min     = 0;

    static int8_t     Last_button_up     = 0;
    static int8_t     Last_button_down   = 0;
    static int8_t     Last_button_Cset   = 0;
    static int8_t     Last_button_Mset   = 0;

    for (;;)
    {
        /* ---- Fetch RTC time and date each iteration ------------------ */
        (void)HAL_RTC_GetTime(&hrtc, &CurrentTime, RTC_FORMAT_BIN);
        (void)HAL_RTC_GetDate(&hrtc, &CurrentDate, RTC_FORMAT_BIN);

        /* ---- Fixed 50 ms timing (no drift) --------------------------- */
        last_wake_time = xTaskGetTickCount();
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(50U));

        LCD_refresh++;
        if (LCD_refresh >= LCD_REFRESH_INTERVAL)
        {
            (void)HD44780_Init();
            HD44780_Clear();
            LCD_refresh = 0U;
        }

        /*==============================================================
         *                     MENU STATE MACHINE
         *============================================================*/
        switch (menu_state)
        {
        /*--------------------------------------------------------------
         *  CASE MENU_IDLE - Normal display mode
         *------------------------------------------------------------*/
        case MENU_IDLE:
            /* ---- Persist clock-menu changes when leaving it --------- */
            if (ModifiedCheck == 1)
            {
                if ((YearCheck  != PersianDate.year) ||
                    (MonthCheck != PersianDate.month)||
                    (dayCheck   != PersianDate.day))
                {
                    gregorianDate = jalali2gregorian(PersianDate);
                    CurrentDate.Date  = (uint8_t)gregorianDate.day;
                    CurrentDate.Month = (uint8_t)gregorianDate.month;
                    CurrentDate.Year  = (uint8_t)(gregorianDate.year - 2000);
                    (void)HAL_RTC_SetDate(&hrtc, &CurrentDate, RTC_FORMAT_BIN);
                }

                if ((HourCheck != changed_hour) || (MinCheck != changed_min))
                {
                    CurrentTime.Hours   = (uint8_t)changed_hour;
                    CurrentTime.Minutes = (uint8_t)changed_min;
                    CurrentTime.Seconds = 0U;
                    (void)HAL_RTC_SetTime(&hrtc, &CurrentTime, RTC_FORMAT_BIN);
                }

                /* Sunrise / sunset */
                SunriseHour   = SunriseHourEdit;
                SunriseMinute = SunriseMinuteEdit;
                SunsetHour    = SunsetHourEdit;
                SunsetMinute  = SunsetMinuteEdit;

                /* Timer1 */
                if ((Timer1_Status      != Timer1Edit)         ||
                    (Timer1_NightStatus != Timer1NightEdit)    ||
                    (Timer1_onHour      != Timer1_onHourEdit)  ||
                    (Timer1_onMin       != Timer1_onMinEdit)   ||
                    (Timer1_offHour     != Timer1_offHourEdit) ||
                    (Timer1_offMin      != Timer1_offMinEdit))
                {
                    Timer1_Status      = Timer1Edit;
                    Timer1_NightStatus = Timer1NightEdit;
                    Timer1_onHour      = Timer1_onHourEdit;
                    Timer1_onMin       = Timer1_onMinEdit;
                    Timer1_offHour     = Timer1_offHourEdit;
                    Timer1_offMin      = Timer1_offMinEdit;
                    StartCalculateT1   = 0U;
                }

                /* Timer2 */
                if ((Timer2_Status      != Timer2_Edit)        ||
                    (Timer2_NightStatus != Timer2_NightEdit)   ||
                    (Timer2_onHour      != Timer2_onHourEdit)  ||
                    (Timer2_onMin       != Timer2_onMinEdit)   ||
                    (Timer2_offHour     != Timer2_offHourEdit) ||
                    (Timer2_offMin      != Timer2_offMinEdit))
                {
                    Timer2_Status      = Timer2_Edit;
                    Timer2_NightStatus = Timer2_NightEdit;
                    Timer2_onHour      = Timer2_onHourEdit;
                    Timer2_onMin       = Timer2_onMinEdit;
                    Timer2_offHour     = Timer2_offHourEdit;
                    Timer2_offMin      = Timer2_offMinEdit;
                    StartCalculateT2   = 0U;
                }

                ModifiedCheck = 0;
            }

            /* ---- Persist main-menu changes when leaving it --------- */
            if (ModifiedCheck == 2)
            {
                LowTemp       = LowTempEdit;
                HighTemp      = HighTempEdit;
                EmergencyTemp = EmergencyTempEdit;
                LowHumi       = LowHumiEdit;
                HighHumi      = HighHumiEdit;
                nrf_mode      = nrf_modeEdit;
                apply_mode(nrf_mode);
                ModifiedCheck = 0;
            }

            /* ---- Refresh Persian calendar -------------------------- */
            gregorianDate.year  = CurrentDate.Year + 2000U;
            gregorianDate.month = CurrentDate.Month;
            gregorianDate.day   = CurrentDate.Date;
            PersianDate         = gregorian2jalali(gregorianDate);

            /* ---- Cache current clock for edit comparison ----------- */
            changed_hour = (int8_t)CurrentTime.Hours;
            changed_min  = (int8_t)CurrentTime.Minutes;

            /* ---- Render the idle screen ---------------------------- */
            if (Screen == 0U)
            {
                if (blink_dot_clock > 28U) { blink_dot_clock = 0U; }

                if (blink_dot_clock <= 14U)
                {
                    (void)snprintf(LCD, sizeof(LCD), "%02d:%02d         ",
                                   CurrentTime.Hours, CurrentTime.Minutes);
                    HD44780_SetCursor(0U, 0U);
                    HD44780_PrintStr(LCD);
                }
                else
                {
                    (void)snprintf(LCD, sizeof(LCD), "%02d %02d         ",
                                   CurrentTime.Hours, CurrentTime.Minutes);
                    HD44780_SetCursor(0U, 0U);
                    HD44780_PrintStr(LCD);
                }
                blink_dot_clock++;

                if (new_data_available == 1U)
                {
                    (void)snprintf(LCD, sizeof(LCD), "S%d", rxData.sensor_num);
                    HD44780_SetCursor(0U, 14U);
                    HD44780_PrintStr(LCD);

                    (void)snprintf(LCD, sizeof(LCD), "T:%0.1f\xdf""C  H:%d%% ",
                                   (float)rxData.Temperature / 10.0f,
                                   (int)(rxData.humidity / 10));
                    HD44780_SetCursor(1U, 0U);
                    HD44780_PrintStr(LCD);
                }
                else
                {
                    (void)snprintf(LCD, sizeof(LCD), "S%d", rxData.sensor_num);
                    HD44780_SetCursor(0U, 14U);
                    HD44780_PrintStr(LCD);
                    HD44780_SetCursor(1U, 0U);
                    HD44780_PrintStr("NO Sensor        ");
                }
            }

            if (Screen == 1U)
            {
                (void)snprintf(LCD, sizeof(LCD), "%02d/%02d/%02d    ",
                               PersianDate.year, PersianDate.month, PersianDate.day);
                HD44780_SetCursor(0U, 0U);
                HD44780_PrintStr(LCD);

                if (new_data_available == 1U)
                {
                    (void)snprintf(LCD, sizeof(LCD), "S%d", rxData.sensor_num);
                    HD44780_SetCursor(0U, 14U);
                    HD44780_PrintStr(LCD);

                    (void)snprintf(LCD, sizeof(LCD), "CO2:%03d  L:%d     ",
                                   (int)ppm, (int)lux);
                    HD44780_SetCursor(1U, 0U);
                    HD44780_PrintStr(LCD);
                }
                else
                {
                    (void)snprintf(LCD, sizeof(LCD), "S%d", rxData.sensor_num);
                    HD44780_SetCursor(0U, 14U);
                    HD44780_PrintStr(LCD);
                    HD44780_SetCursor(1U, 0U);
                    HD44780_PrintStr("NO Sensor        ");
                }
            }

            /* ---- Button: switch screen ----------------------------- */
            if ((HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET) &&
                (Last_button_up == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Screen = 1U;
                    HD44780_Clear();
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET) &&
                     (Last_button_down == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Screen = 0U;
                    HD44780_Clear();
                    idle_counter = 0U;
                }
            }

            /* ---- Button: enter clock menu -------------------------- */
            if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(pdMS_TO_TICKS(50U));
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    YearCheck  = PersianDate.year;
                    MonthCheck = PersianDate.month;
                    dayCheck   = PersianDate.day;
                    HourCheck  = (uint8_t)changed_hour;
                    MinCheck   = (uint8_t)changed_min;

                    SunriseHourEdit   = SunriseHour;
                    SunriseMinuteEdit = SunriseMinute;
                    SunsetHourEdit    = SunsetHour;
                    SunsetMinuteEdit  = SunsetMinute;

                    Timer1Edit         = Timer1_Status;
                    Timer1NightEdit    = Timer1_NightStatus;
                    Timer1_onHourEdit  = Timer1_onHour;
                    Timer1_onMinEdit   = Timer1_onMin;
                    Timer1_offHourEdit = Timer1_offHour;
                    Timer1_offMinEdit  = Timer1_offMin;

                    Timer2_Edit        = Timer2_Status;
                    Timer2_NightEdit   = Timer2_NightStatus;
                    Timer2_onHourEdit  = Timer2_onHour;
                    Timer2_onMinEdit   = Timer2_onMin;
                    Timer2_offHourEdit = Timer2_offHour;
                    Timer2_offMinEdit  = Timer2_offMin;

                    ModifiedCheck = 1;
                    menu_state    = MENU_SET_DATE_YEAR;
                    idle_counter  = 0U;
                }
            }
            /* ---- Button: enter main menu --------------------------- */
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(pdMS_TO_TICKS(50U));
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    LowTempEdit       = LowTemp;
                    HighTempEdit      = HighTemp;
                    EmergencyTempEdit = EmergencyTemp;
                    LowHumiEdit       = LowHumi;
                    HighHumiEdit      = HighHumi;
                    nrf_modeEdit      = nrf_mode;

                    ModifiedCheck = 2;
                    menu_state    = MENU_SET_LOW_TEMP;
                    idle_counter  = 0U;
                }
            }
            break;

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> MAIN MENU <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case MENU_SET_LOW_TEMP:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Low Temp:       ");
            (void)snprintf(LCD, sizeof(LCD), "%0.1f\xdf""C           ", LowTempEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    LowTempEdit += 0.1f;
					if (LowTempEdit > HighTempEdit - 00.3f) { LowTempEdit = HighTempEdit - 00.3f; }
                    idle_counter = 0U;
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    LowTempEdit -= 0.1f;
                    if (LowTempEdit <= 10.0f) { LowTempEdit = 10.0f; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_HIGH_TEMP;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_NRF;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_HIGH_TEMP:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("High Temp:         ");
            (void)snprintf(LCD, sizeof(LCD), "%0.1f\xdf""C           ", HighTempEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    HighTempEdit += 0.1f;
                	if (HighTempEdit >= EmergencyTempEdit - 00.3f) { HighTempEdit = EmergencyTempEdit - 00.3f; }
                    idle_counter = 0U;
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    HighTempEdit -= 0.1f;
                	if (HighTempEdit <= LowTempEdit + 00.3f) { HighTempEdit = LowTempEdit + 00.3f; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_EMERGENCY_TEMP;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_LOW_TEMP;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_EMERGENCY_TEMP:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Emergency Temp:         ");
            (void)snprintf(LCD, sizeof(LCD), "%0.1f\xdf""C           ", EmergencyTempEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    EmergencyTempEdit += 0.1f;
                    if (EmergencyTempEdit > 50.0f) { EmergencyTempEdit = 50.0f; }
                    idle_counter = 0U;
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    EmergencyTempEdit -= 0.1f;
                    if (EmergencyTempEdit <= HighTempEdit + 00.3f) { EmergencyTempEdit = HighTempEdit + 00.3f; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_LOW_HUMI;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_HIGH_TEMP;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_LOW_HUMI:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Low humi:         ");
            (void)snprintf(LCD, sizeof(LCD), "%d%%           ", LowHumiEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    LowHumiEdit++;
                 	if (LowHumiEdit >= HighHumiEdit - 3) { LowHumiEdit = HighHumiEdit - 3; }
                    idle_counter = 0U;
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    LowHumiEdit--;
                    if (LowHumiEdit < 5U) { LowHumiEdit = 5U; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_HIGH_HUMI;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_EMERGENCY_TEMP;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_HIGH_HUMI:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("High humi:        ");
            (void)snprintf(LCD, sizeof(LCD), "%d%%          ", HighHumiEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    HighHumiEdit++;
                    if (HighHumiEdit > 95U) { HighHumiEdit = 95U; }
                    idle_counter = 0U;
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    HighHumiEdit--;
                    if (HighHumiEdit <= LowHumiEdit + 3) { HighHumiEdit = LowHumiEdit + 3; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_NRF;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_LOW_HUMI;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_NRF:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Sensor Mode:      ");
            (void)snprintf(LCD, sizeof(LCD), "Mode %d           ", nrf_modeEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    nrf_modeEdit++;
                    if (nrf_modeEdit > 10U) { nrf_modeEdit = 1U; }
                    idle_counter = 0U;
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    nrf_modeEdit--;
                    if (nrf_modeEdit < 1U) { nrf_modeEdit = 10U; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_LOW_TEMP;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_HIGH_HUMI;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

/*>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> CLOCK MENU <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/

        case MENU_SET_DATE_YEAR:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Date Set:           ");
            (void)snprintf(LCD, sizeof(LCD), "%04d/%02d/%02d             ",
                           PersianDate.year, PersianDate.month, PersianDate.day);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "    /%02d/%02d             ",
                           PersianDate.month, PersianDate.day);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    PersianDate.year++;
                    if (PersianDate.year > 1500U) { PersianDate.year = 1400U; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    PersianDate.year--;
                    if (PersianDate.year < 1400U) { PersianDate.year = 1500U; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_DATE_MONTH;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_OFF_MIN;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_DATE_MONTH:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Date Set:           ");
            (void)snprintf(LCD, sizeof(LCD), "%04d/%02d/%02d             ",
                           PersianDate.year, PersianDate.month, PersianDate.day);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%04d/  /%02d             ",
                           PersianDate.year, PersianDate.day);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    PersianDate.month++;
                    if (PersianDate.month > 12U) { PersianDate.month = 1U; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    PersianDate.month--;
                    if (PersianDate.month < 1U) { PersianDate.month = 12U; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_DATE_DAY;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_DATE_YEAR;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_DATE_DAY:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Date Set:           ");
            (void)snprintf(LCD, sizeof(LCD), "%04d/%02d/%02d             ",
                           PersianDate.year, PersianDate.month, PersianDate.day);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%04d/%02d/               ",
                           PersianDate.year, PersianDate.month);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    PersianDate.day++;
                    if (PersianDate.day > 31U) { PersianDate.day = 1U; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    PersianDate.day--;
                    if (PersianDate.day < 1U) { PersianDate.day = 31U; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_CLOCK_HOUR;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_DATE_MONTH;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_CLOCK_HOUR:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Clock Set:         ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           changed_hour, changed_min);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "  :%02d             ", changed_min);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    changed_hour++;
                    if (changed_hour > 23) { changed_hour = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    changed_hour--;
                    if (changed_hour < 0) { changed_hour = 23; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_CLOCK_MIN;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_DATE_DAY;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_CLOCK_MIN:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Clock Set:        ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d            ",
                           changed_hour, changed_min);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%02d:              ", changed_hour);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    changed_min++;
                    if (changed_min > 59) { changed_min = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    changed_min--;
                    if (changed_min < 0) { changed_min = 59; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_SUNRISE_HOUR;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_CLOCK_HOUR;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_SUNRISE_HOUR:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Sunrise Set:        ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           SunriseHourEdit, SunriseMinuteEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "  :%02d             ", SunriseMinuteEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    SunriseHourEdit++;
                    if (SunriseHourEdit > 23) { SunriseHourEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    SunriseHourEdit--;
                    if (SunriseHourEdit < 0) { SunriseHourEdit = 23; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_SUNRISE_MIN;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_CLOCK_MIN;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_SUNRISE_MIN:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Sunrise Set:       ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d            ",
                           SunriseHourEdit, SunriseMinuteEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%02d:              ", SunriseHourEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    SunriseMinuteEdit++;
                    if (SunriseMinuteEdit > 59) { SunriseMinuteEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    SunriseMinuteEdit--;
                    if (SunriseMinuteEdit < 0) { SunriseMinuteEdit = 59; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_SUNSET_HOUR;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_SUNRISE_HOUR;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_SUNSET_HOUR:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Sunset Set:        ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           SunsetHourEdit, SunsetMinuteEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "  :%02d             ", SunsetMinuteEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    SunsetHourEdit++;
                    if (SunsetHourEdit > 23) { SunsetHourEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    SunsetHourEdit--;
                    if (SunsetHourEdit < 0) { SunsetHourEdit = 23; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_SUNSET_MIN;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_SUNRISE_MIN;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_SUNSET_MIN:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Sunset Set:        ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           SunsetHourEdit, SunsetMinuteEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%02d:               ", SunsetHourEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    SunsetMinuteEdit++;
                    if (SunsetMinuteEdit > 59) { SunsetMinuteEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    SunsetMinuteEdit--;
                    if (SunsetMinuteEdit < 0) { SunsetMinuteEdit = 59; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_STATUS;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_SUNSET_HOUR;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER1_STATUS:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Timer 1:              ");
            if (Timer1Edit == 0)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("Off                  ");
            }
            else if (Timer1Edit == 1)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("Two-stage timer  ");
            }
            else if (Timer1Edit == 2)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("Clock timer    ");
            }
            else
            {
                /* Nothing to display. */
            }

            if ((HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET) &&
                (Last_button_up == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer1Edit++;
                    if (Timer1Edit > 2) { Timer1Edit = 0; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET) &&
                     (Last_button_down == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer1Edit--;
                    if (Timer1Edit < 0) { Timer1Edit = 2; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    if (Timer1Edit == 0) { menu_state = MENU_SET_TIMER2_STATUS; }
                    if (Timer1Edit == 1) { menu_state = MENU_SET_TIMER1_NIGHT_STATUS; }
                    if (Timer1Edit == 2) { menu_state = MENU_SET_TIMER1_ON_HOUR; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_SUNSET_MIN;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER1_NIGHT_STATUS:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Night timer 1: ");
            if (Timer1NightEdit == 0)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("Off                  ");
            }
            else if (Timer1NightEdit == 1)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("On                  ");
            }
            else
            {
                /* Nothing to display. */
            }

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer1NightEdit++;
                    if (Timer1NightEdit > 1) { Timer1NightEdit = 1; }
                    idle_counter = 0U;
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer1NightEdit--;
                    if (Timer1NightEdit < 0) { Timer1NightEdit = 0; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_ON_HOUR;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_STATUS;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER1_ON_HOUR:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("timer 1 on:         ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           Timer1_onHourEdit, Timer1_onMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "  :%02d             ", Timer1_onMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer1_onHourEdit++;
                    if (Timer1_onHourEdit > 23) { Timer1_onHourEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer1_onHourEdit--;
                    if (Timer1_onHourEdit < 0) { Timer1_onHourEdit = 23; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_ON_MIN;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    if (Timer1Edit == 1) { menu_state = MENU_SET_TIMER1_NIGHT_STATUS; }
                    if (Timer1Edit == 2) { menu_state = MENU_SET_TIMER1_STATUS; }
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER1_ON_MIN:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("timer 1 on:           ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           Timer1_onHourEdit, Timer1_onMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%02d:               ", Timer1_onHourEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer1_onMinEdit++;
                    if (Timer1_onMinEdit > 59) { Timer1_onMinEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer1_onMinEdit--;
                    if (Timer1_onMinEdit < 0) { Timer1_onMinEdit = 59; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_OFF_HOUR;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_ON_HOUR;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER1_OFF_HOUR:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("timer 1 off:          ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           Timer1_offHourEdit, Timer1_offMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "  :%02d             ", Timer1_offMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer1_offHourEdit++;
                    if (Timer1_offHourEdit > 23) { Timer1_offHourEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer1_offHourEdit--;
                    if (Timer1_offHourEdit < 0) { Timer1_offHourEdit = 23; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_OFF_MIN;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_ON_MIN;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER1_OFF_MIN:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("timer 1 off:            ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           Timer1_offHourEdit, Timer1_offMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%02d:               ", Timer1_offHourEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer1_offMinEdit++;
                    if (Timer1_offMinEdit > 59) { Timer1_offMinEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer1_offMinEdit--;
                    if (Timer1_offMinEdit < 0) { Timer1_offMinEdit = 59; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_STATUS;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER1_OFF_HOUR;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER2_STATUS:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Timer 2:              ");
            if (Timer2_Edit == 0)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("Off                  ");
            }
            else if (Timer2_Edit == 1)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("Two-stage timer  ");
            }
            else if (Timer2_Edit == 2)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("Clock timer    ");
            }
            else
            {
                /* Nothing to display. */
            }

            if ((HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET) &&
                (Last_button_up == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer2_Edit++;
                    if (Timer2_Edit > 2) { Timer2_Edit = 0; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET) &&
                     (Last_button_down == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer2_Edit--;
                    if (Timer2_Edit < 0) { Timer2_Edit = 2; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    if (Timer2_Edit == 0) { menu_state = MENU_SET_DATE_YEAR; }
                    if (Timer2_Edit == 1) { menu_state = MENU_SET_TIMER2_NIGHT_STATUS; }
                    if (Timer2_Edit == 2) { menu_state = MENU_SET_TIMER2_ON_HOUR; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    if (Timer1Edit == 0) { menu_state = MENU_SET_TIMER1_STATUS; }
                    if (Timer1Edit == 1) { menu_state = MENU_SET_TIMER1_OFF_MIN; }
                    if (Timer1Edit == 2) { menu_state = MENU_SET_TIMER1_OFF_MIN; }
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER2_NIGHT_STATUS:
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("Night timer 2: ");
            if (Timer2_NightEdit == 0)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("Off                  ");
            }
            else if (Timer2_NightEdit == 1)
            {
                HD44780_SetCursor(1U, 0U);
                HD44780_PrintStr("On                  ");
            }
            else
            {
                /* Nothing to display. */
            }

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer2_NightEdit++;
                    if (Timer2_NightEdit > 1) { Timer2_NightEdit = 1; }
                    idle_counter = 0U;
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer2_NightEdit--;
                    if (Timer2_NightEdit < 0) { Timer2_NightEdit = 0; }
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_ON_HOUR;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_DEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_STATUS;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER2_ON_HOUR:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("timer 2 on:         ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           Timer2_onHourEdit, Timer2_onMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "  :%02d             ", Timer2_onMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer2_onHourEdit++;
                    if (Timer2_onHourEdit > 23) { Timer2_onHourEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer2_onHourEdit--;
                    if (Timer2_onHourEdit < 0) { Timer2_onHourEdit = 23; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_ON_MIN;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    if (Timer2_Edit == 1) { menu_state = MENU_SET_TIMER2_NIGHT_STATUS; }
                    if (Timer2_Edit == 2) { menu_state = MENU_SET_TIMER2_STATUS; }
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER2_ON_MIN:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("timer 2 on:           ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           Timer2_onHourEdit, Timer2_onMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%02d:               ", Timer2_onHourEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer2_onMinEdit++;
                    if (Timer2_onMinEdit > 59) { Timer2_onMinEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer2_onMinEdit--;
                    if (Timer2_onMinEdit < 0) { Timer2_onMinEdit = 59; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_OFF_HOUR;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_ON_HOUR;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER2_OFF_HOUR:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("timer 2 off:          ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           Timer2_offHourEdit, Timer2_offMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "  :%02d             ", Timer2_offMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer2_offHourEdit++;
                    if (Timer2_offHourEdit > 23) { Timer2_offHourEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer2_offHourEdit--;
                    if (Timer2_offHourEdit < 0) { Timer2_offHourEdit = 23; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_OFF_MIN;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_ON_MIN;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        case MENU_SET_TIMER2_OFF_MIN:
            vTaskDelay(pdMS_TO_TICKS(200U));
            HD44780_SetCursor(0U, 0U);
            HD44780_PrintStr("timer 2 off:            ");
            (void)snprintf(LCD, sizeof(LCD), "%02d:%02d             ",
                           Timer2_offHourEdit, Timer2_offMinEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            vTaskDelay(pdMS_TO_TICKS(200U));
            (void)snprintf(LCD, sizeof(LCD), "%02d:               ", Timer2_offHourEdit);
            HD44780_SetCursor(1U, 0U);
            HD44780_PrintStr(LCD);

            if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_UP) == GPIO_PIN_SET)
                {
                    Timer2_offMinEdit++;
                    if (Timer2_offMinEdit > 59) { Timer2_offMinEdit = 0; }
                }
            }
            else if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
            {
                vTaskDelay(KEY_CHANGE_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_DOWN) == GPIO_PIN_SET)
                {
                    Timer2_offMinEdit--;
                    if (Timer2_offMinEdit < 0) { Timer2_offMinEdit = 59; }
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) &&
                     (Last_button_Cset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_DATE_YEAR;
                    idle_counter = 0U;
                }
            }
            else if ((HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET) &&
                     (Last_button_Mset == GPIO_PIN_RESET))
            {
                vTaskDelay(KEY_TURN_SHORTDEBOUNCE);
                if (HAL_GPIO_ReadPin(BUTTON_MAIN) == GPIO_PIN_SET)
                {
                    menu_state   = MENU_SET_TIMER2_OFF_HOUR;
                    idle_counter = 0U;
                }
            }
            else if (ANY_BUTTON_PRESSED() == 0U)
            {
                idle_counter++;
                if (idle_counter >= EXIT_MENU_SHORT)
                {
                    HD44780_Clear();
                    menu_state   = MENU_IDLE;
                    idle_counter = 0U;
                }
            }
            else
            {
                idle_counter = 0U;
            }
            break;

        default:
            menu_state = MENU_IDLE;
            break;
        }

        /* ---- Last-state button detection for edge-triggered action -- */
        Last_button_up   = HAL_GPIO_ReadPin(BUTTON_UP);
        Last_button_down = HAL_GPIO_ReadPin(BUTTON_DOWN);
        Last_button_Cset = HAL_GPIO_ReadPin(BUTTON_CLOCK);
        Last_button_Mset = HAL_GPIO_ReadPin(BUTTON_MAIN);
    }
}
