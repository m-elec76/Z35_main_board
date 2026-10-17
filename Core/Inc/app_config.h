/**
 ******************************************************************************
 * @file    app_config.h
 * @brief   Central configuration (pins, constants, timings).
 *
 * @author  Mohammadali Abbasi <m.elec76a@gmail.com>
 * @date    2026
 * @license Apache-2.0
 ******************************************************************************
 */

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

/*==============================================================================
 *                              BUTTON PIN MAP
 *============================================================================*/
#define BUTTON_UP       GPIOB, GPIO_PIN_4
#define BUTTON_DOWN     GPIOD, GPIO_PIN_2
#define BUTTON_CLOCK    GPIOB, GPIO_PIN_5
#define BUTTON_MAIN     GPIOB, GPIO_PIN_3

/**
 * @brief  Logical OR of the four user buttons.
 * @return 1 when at least one button is currently pressed, else 0.
 */
#define ANY_BUTTON_PRESSED()  ( (HAL_GPIO_ReadPin(BUTTON_UP)    == GPIO_PIN_SET) || \
                                (HAL_GPIO_ReadPin(BUTTON_DOWN)  == GPIO_PIN_SET) || \
                                (HAL_GPIO_ReadPin(BUTTON_MAIN)  == GPIO_PIN_SET) || \
                                (HAL_GPIO_ReadPin(BUTTON_CLOCK) == GPIO_PIN_SET) )

/*==============================================================================
 *                              BUTTON DEBOUNCE
 *============================================================================*/
#define KEY_TURN_DEBOUNCE            pdMS_TO_TICKS(300U)
#define KEY_CHANGE_DEBOUNCE          pdMS_TO_TICKS(50U)
#define KEY_TURN_SHORTDEBOUNCE       pdMS_TO_TICKS(50U)
#define KEY_CHANGE_SHORTDEBOUNCE     pdMS_TO_TICKS(50U)

/** Idle-loop ticks before exiting a menu with a short grace period. */
#define EXIT_MENU_SHORT              (20U)
/** Idle-loop ticks before exiting a menu normally. */
#define EXIT_MENU                    (90U)

/*==============================================================================
 *                              LCD REFRESH RATE
 *============================================================================*/
/** Number of UI-loop ticks between forced LCD re-inits: 2400 * 50 ms = 2 min. */
#define LCD_REFRESH_INTERVAL         (2400U)

/*==============================================================================
 *                              OUTPUT PIN MAP
 *============================================================================*/
#define SUN_LAMP        GPIOA, GPIO_PIN_11
#define LAMP_DAY        GPIOA, GPIO_PIN_9
#define LAMP_NIGHT      GPIOA, GPIO_PIN_10
#define HEATER          GPIOA, GPIO_PIN_8
#define GAS_HEATER      GPIOB, GPIO_PIN_8
#define EMERGENCY_FAN   GPIOC, GPIO_PIN_8
#define HUMIDIFIER      GPIOC, GPIO_PIN_6
#define TIMER1_OUT      GPIOC, GPIO_PIN_7
#define TIMER2_OUT      GPIOC, GPIO_PIN_9

/*==============================================================================
 *                              ADC SCALING
 *============================================================================*/
#define ADC_MAX         (4095.0f)
#define V_REF           (3.3f)
#define RL              (10000.0f)      /* Load resistor: 10 kOhm */

/*==============================================================================
 *                              MQ-135 CALIBRATION (NH3 curve)
 *============================================================================*/
#define MQ135_A_NH3     (110.0f)
#define MQ135_B_NH3     (-2.70f)

/*==============================================================================
 *                              EXTERNAL INTERRUPT PINS
 *============================================================================*/
#define EXTI_NRF_PIN        GPIO_PIN_2
#define EXTI_VOLTAGE_PIN    GPIO_PIN_12

/*==============================================================================
 *                              SUN (ARTIFICIAL LIGHT) LIMITS
 *============================================================================*/
/** Minimum ARR value -> brightest output. */
#define SUN_ARR_MIN             (80U)
/** Maximum ARR value -> darkest output. */
#define SUN_ARR_MAX             (315U)

/*==============================================================================
 *                              UI TASK TIMING
 *============================================================================*/
/** Main UI loop period in milliseconds. */
#define UI_LOOP_PERIOD_MS       (50U)

/*==============================================================================
 *                              SENSOR TIMEOUT
 *============================================================================*/
/** Consecutive empty ticks before declaring the remote sensor lost. */
#define NRF_NO_SENSOR_LIMIT     (200U)

#ifdef __cplusplus
}
#endif

#endif /* APP_CONFIG_H */
