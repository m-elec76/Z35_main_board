/**
 ******************************************************************************
 * @file    app_types.h
 * @brief   Global data structures and enums.
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#ifndef APP_TYPES_H
#define APP_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/*==============================================================================
 *                          WIRELESS SENSOR PAYLOAD
 *============================================================================*/
/**
 * @brief  Sensor payload received over the nRF24L01+ link.
 * @note   Struct is packed so that the byte layout matches the transmitter.
 */
typedef struct __attribute__((packed))
{
    uint8_t  system_id;         /**< Network identifier of the sender       */
    uint8_t  sensor_num;        /**< Logical sensor node number             */
    uint16_t Temperature;       /**< Temperature value multiplied by 10     */
    uint16_t humidity;          /**< Relative humidity multiplied by 10     */
    uint16_t gas;               /**< Raw ADC reading from the MQ-135        */
    uint8_t  burglar_alarm;     /**< PIR alarm flag (0 = idle, 1 = alarm)   */
    uint16_t ldr;               /**< Raw ADC reading from the LDR           */
} SensorData_t;

/*==============================================================================
 *                          MENU STATE MACHINE
 *============================================================================*/
typedef enum
{
    MENU_IDLE = 0,
    /* ---- Main configuration menu ---- */
    MENU_SET_LOW_TEMP,
    MENU_SET_HIGH_TEMP,
    MENU_SET_EMERGENCY_TEMP,
    MENU_SET_LOW_HUMI,
    MENU_SET_HIGH_HUMI,
    MENU_SET_NRF,
    /* ---- Clock configuration menu ---- */
    MENU_SET_DATE_YEAR,
    MENU_SET_DATE_MONTH,
    MENU_SET_DATE_DAY,
    MENU_SET_CLOCK_HOUR,
    MENU_SET_CLOCK_MIN,
    MENU_SET_SUNRISE_HOUR,
    MENU_SET_SUNRISE_MIN,
    MENU_SET_SUNSET_HOUR,
    MENU_SET_SUNSET_MIN,
    MENU_SET_TIMER1_STATUS,
    MENU_SET_TIMER1_NIGHT_STATUS,
    MENU_SET_TIMER1_ON_HOUR,
    MENU_SET_TIMER1_ON_MIN,
    MENU_SET_TIMER1_OFF_HOUR,
    MENU_SET_TIMER1_OFF_MIN,
    MENU_SET_TIMER2_STATUS,
    MENU_SET_TIMER2_NIGHT_STATUS,
    MENU_SET_TIMER2_ON_HOUR,
    MENU_SET_TIMER2_ON_MIN,
    MENU_SET_TIMER2_OFF_HOUR,
    MENU_SET_TIMER2_OFF_MIN
} MenuState_t;

#ifdef __cplusplus
}
#endif

#endif /* APP_TYPES_H */
