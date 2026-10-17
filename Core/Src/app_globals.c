/**
 ******************************************************************************
 * @file    app_globals.c
 * @brief   Declaration and definition of shared runtime variables.
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#include "app_globals.h"

/*==============================================================================
 *                          RTOS HANDLES
 *============================================================================*/
TaskHandle_t  nrfTaskHandle     = NULL;
TaskHandle_t  settingTaskHandle = NULL;
QueueHandle_t nrfDataR_Queue    = NULL;

/*==============================================================================
 *                          SENSOR DATA
 *============================================================================*/
SensorData_t     rxData             = {0};
volatile uint8_t new_data_available = 0U;

/*==============================================================================
 *                          RTC / CALENDAR
 *============================================================================*/
RTC_TimeTypeDef CurrentTime;
RTC_DateTypeDef CurrentDate;
date            gregorianDate;
date            PersianDate;

/*==============================================================================
 *                          MENU STATE
 *============================================================================*/
MenuState_t menu_state = MENU_IDLE;

/*==============================================================================
 *                          TIMER RECALCULATION FLAGS
 *============================================================================*/
uint8_t StartCalculateT1   = 0U;
uint8_t CalculateTimer1ON  = 0U;
uint8_t CalculateTimer1OFF = 0U;

uint8_t StartCalculateT2   = 0U;
uint8_t CalculateTimer2ON  = 0U;
uint8_t CalculateTimer2OFF = 0U;

/*==============================================================================
 *                          SUN / DAY-NIGHT STATE
 *============================================================================*/
volatile uint32_t timer2ARRControl = 315U;
volatile uint16_t PowerOutage      = 0U;
uint8_t           NightStatus      = 2U;    /* 2 = invalid until computed */
uint8_t           CheckTimeSun     = 3U;

/*==============================================================================
 *                          NRF SYSTEM ID
 *============================================================================*/
uint8_t system_id = 0U;

/*==============================================================================
 *                          MAIN SETTINGS (runtime)
 *============================================================================*/
float   LowTemp       = 30.0f;
float   HighTemp      = 32.0f;
float   EmergencyTemp = 34.0f;
uint8_t LowHumi       = 40U;
uint8_t HighHumi      = 60U;
uint8_t nrf_mode      = 1U;

/*==============================================================================
 *                          MAIN SETTINGS (edit buffers)
 *============================================================================*/
float   LowTempEdit        = 0.0f;
float   HighTempEdit       = 0.0f;
float   EmergencyTempEdit  = 0.0f;
uint8_t LowHumiEdit        = 0U;
uint8_t HighHumiEdit       = 0U;
uint8_t nrf_modeEdit       = 0U;

/*==============================================================================
 *                          CLOCK SETTINGS (runtime)
 *============================================================================*/
int8_t SunriseHour   = 6;
int8_t SunriseMinute = 0;
int8_t SunsetHour    = 20;
int8_t SunsetMinute  = 0;

int8_t Timer1_Status      = 2;
int8_t Timer1_NightStatus = 0;
int8_t Timer1_onHour      = 23;
int8_t Timer1_onMin       = 59;
int8_t Timer1_offHour     = 0;
int8_t Timer1_offMin      = 2;

int8_t Timer2_Status      = 2;
int8_t Timer2_NightStatus = 0;
int8_t Timer2_onHour      = 23;
int8_t Timer2_onMin       = 59;
int8_t Timer2_offHour     = 0;
int8_t Timer2_offMin      = 3;

/*==============================================================================
 *                          CLOCK SETTINGS (edit buffers)
 *============================================================================*/
int8_t SunriseHourEdit   = 0;
int8_t SunriseMinuteEdit = 0;
int8_t SunsetHourEdit    = 0;
int8_t SunsetMinuteEdit  = 0;

int8_t Timer1Edit         = 0;
int8_t Timer1NightEdit    = 0;
int8_t Timer1_onHourEdit  = 0;
int8_t Timer1_onMinEdit   = 0;
int8_t Timer1_offHourEdit = 0;
int8_t Timer1_offMinEdit  = 0;

int8_t Timer2_Edit        = 0;
int8_t Timer2_NightEdit   = 0;
int8_t Timer2_onHourEdit  = 0;
int8_t Timer2_onMinEdit   = 0;
int8_t Timer2_offHourEdit = 0;
int8_t Timer2_offMinEdit  = 0;

/*==============================================================================
 *                          SENSOR DERIVED VALUES
 *============================================================================*/
float LDR_Vout    = 0.0f;
float R_LDR       = 0.0f;
float lux         = 0.0f;

float MQ135_Vout  = 0.0f;
float R_MQ135     = 0.0f;
float R0_MQ135    = 0.0f;
float ratio       = 0.0f;
float ppm         = 0.0f;
float Vout_sensor = 0.0f;

/*==============================================================================
 *                          DEBUG
 *============================================================================*/
UBaseType_t current_priority = 0U;
