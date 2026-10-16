/**
 ******************************************************************************
 * @file    app_globals.h
 * @brief   Declarations of every shared runtime variable.
 *
 * @note    All variables are defined exactly once in app_globals.c.
 *          This header only contains extern declarations, satisfying
 *          MISRA-C:2012 Rule 8.4.
 ******************************************************************************
 */

#ifndef APP_GLOBALS_H
#define APP_GLOBALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "app_types.h"
#include "rtc.h"
#include "ConvertCalendar.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/*==============================================================================
 *                          RTOS HANDLES
 *============================================================================*/
extern TaskHandle_t     nrfTaskHandle;      /**< Receiver task handle           */
extern TaskHandle_t     settingTaskHandle;  /**< Menu / display task handle     */
extern QueueHandle_t    nrfDataR_Queue;     /**< Queue carrying sensor payloads */

/*==============================================================================
 *                          SENSOR DATA
 *============================================================================*/
extern SensorData_t     rxData;                 /**< Latest received payload   */
extern volatile uint8_t new_data_available;     /**< Fresh-data flag for LCD   */

/*==============================================================================
 *                          RTC / CALENDAR
 *============================================================================*/
extern RTC_TimeTypeDef  CurrentTime;
extern RTC_DateTypeDef  CurrentDate;
extern date             gregorianDate;
extern date             PersianDate;

/*==============================================================================
 *                          MENU STATE
 *============================================================================*/
extern MenuState_t      menu_state;

/*==============================================================================
 *                          MAIN SETTINGS (runtime)
 *============================================================================*/
extern float    LowTemp;
extern float    HighTemp;
extern float    EmergencyTemp;
extern uint8_t  LowHumi;
extern uint8_t  HighHumi;
extern uint8_t  nrf_mode;

/*==============================================================================
 *                          MAIN SETTINGS (edit buffers)
 *============================================================================*/
extern float    LowTempEdit;
extern float    HighTempEdit;
extern float    EmergencyTempEdit;
extern uint8_t  LowHumiEdit;
extern uint8_t  HighHumiEdit;
extern uint8_t  nrf_modeEdit;

/*==============================================================================
 *                          CLOCK SETTINGS (runtime)
 *============================================================================*/
extern int8_t   SunriseHour;
extern int8_t   SunriseMinute;
extern int8_t   SunsetHour;
extern int8_t   SunsetMinute;

extern int8_t   Timer1_Status;
extern int8_t   Timer1_NightStatus;
extern int8_t   Timer1_onHour;
extern int8_t   Timer1_onMin;
extern int8_t   Timer1_offHour;
extern int8_t   Timer1_offMin;

extern int8_t   Timer2_Status;
extern int8_t   Timer2_NightStatus;
extern int8_t   Timer2_onHour;
extern int8_t   Timer2_onMin;
extern int8_t   Timer2_offHour;
extern int8_t   Timer2_offMin;

/*==============================================================================
 *                          CLOCK SETTINGS (edit buffers)
 *============================================================================*/
extern int8_t   SunriseHourEdit;
extern int8_t   SunriseMinuteEdit;
extern int8_t   SunsetHourEdit;
extern int8_t   SunsetMinuteEdit;

extern int8_t   Timer1Edit;
extern int8_t   Timer1NightEdit;
extern int8_t   Timer1_onHourEdit;
extern int8_t   Timer1_onMinEdit;
extern int8_t   Timer1_offHourEdit;
extern int8_t   Timer1_offMinEdit;

extern int8_t   Timer2_Edit;
extern int8_t   Timer2_NightEdit;
extern int8_t   Timer2_onHourEdit;
extern int8_t   Timer2_onMinEdit;
extern int8_t   Timer2_offHourEdit;
extern int8_t   Timer2_offMinEdit;

/*==============================================================================
 *                          TIMER RECALCULATION FLAGS
 *============================================================================*/
extern uint8_t  StartCalculateT1;
extern uint8_t  CalculateTimer1ON;
extern uint8_t  CalculateTimer1OFF;

extern uint8_t  StartCalculateT2;
extern uint8_t  CalculateTimer2ON;
extern uint8_t  CalculateTimer2OFF;

/*==============================================================================
 *                          SUN / DAY-NIGHT STATE
 *============================================================================*/
extern volatile uint32_t timer2ARRControl;      /**< Active ARR for sun PWM     */
extern volatile uint16_t PowerOutage;           /**< Power-loss counter         */
extern uint8_t           NightStatus;           /**< 0 = day, 1 = night         */
extern uint8_t           CheckTimeSun;          /**< Sun FSM state              */

/*==============================================================================
 *                          NRF SYSTEM ID
 *============================================================================*/
extern uint8_t  system_id;

/*==============================================================================
 *                          SENSOR DERIVED VALUES
 *============================================================================*/
extern float    LDR_Vout;
extern float    R_LDR;
extern float    lux;

extern float    MQ135_Vout;
extern float    R_MQ135;
extern float    R0_MQ135;
extern float    ratio;
extern float    ppm;
extern float    Vout_sensor;

/*==============================================================================
 *                          DEBUG
 *============================================================================*/
extern UBaseType_t current_priority;

#ifdef __cplusplus
}
#endif

#endif /* APP_GLOBALS_H */
