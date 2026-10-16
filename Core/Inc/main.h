/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LCD_RS_Pin GPIO_PIN_0
#define LCD_RS_GPIO_Port GPIOA
#define LCD_RW_Pin GPIO_PIN_1
#define LCD_RW_GPIO_Port GPIOA
#define LCD_E_Pin GPIO_PIN_2
#define LCD_E_GPIO_Port GPIOA
#define LCD_D4_Pin GPIO_PIN_3
#define LCD_D4_GPIO_Port GPIOA
#define LCD_D5_Pin GPIO_PIN_4
#define LCD_D5_GPIO_Port GPIOA
#define LCD_D6_Pin GPIO_PIN_5
#define LCD_D6_GPIO_Port GPIOA
#define LCD_D7_Pin GPIO_PIN_6
#define LCD_D7_GPIO_Port GPIOA
#define NRF_IRQ_Pin GPIO_PIN_2
#define NRF_IRQ_GPIO_Port GPIOB
#define NRF_IRQ_EXTI_IRQn EXTI2_IRQn
#define NRF_CE_Pin GPIO_PIN_11
#define NRF_CE_GPIO_Port GPIOB
#define NRF_CSN_Pin GPIO_PIN_12
#define NRF_CSN_GPIO_Port GPIOB
#define HUMIDITY_Pin GPIO_PIN_6
#define HUMIDITY_GPIO_Port GPIOC
#define TIMER_1_Pin GPIO_PIN_7
#define TIMER_1_GPIO_Port GPIOC
#define COOLER_Pin GPIO_PIN_8
#define COOLER_GPIO_Port GPIOC
#define TIMER_2_Pin GPIO_PIN_9
#define TIMER_2_GPIO_Port GPIOC
#define HEATER_Pin GPIO_PIN_8
#define HEATER_GPIO_Port GPIOA
#define LAMP_DAY_Pin GPIO_PIN_9
#define LAMP_DAY_GPIO_Port GPIOA
#define LAMP_NIGHT_Pin GPIO_PIN_10
#define LAMP_NIGHT_GPIO_Port GPIOA
#define LAMP_SUN_Pin GPIO_PIN_11
#define LAMP_SUN_GPIO_Port GPIOA
#define VOLTAGE_DETECT_Pin GPIO_PIN_12
#define VOLTAGE_DETECT_GPIO_Port GPIOA
#define VOLTAGE_DETECT_EXTI_IRQn EXTI15_10_IRQn
#define BUTTON_DOWN_Pin GPIO_PIN_2
#define BUTTON_DOWN_GPIO_Port GPIOD
#define BUTTON_MAIN_SET_Pin GPIO_PIN_3
#define BUTTON_MAIN_SET_GPIO_Port GPIOB
#define BUTTON_UP_Pin GPIO_PIN_4
#define BUTTON_UP_GPIO_Port GPIOB
#define BUTTON_CLOCK_SET_Pin GPIO_PIN_5
#define BUTTON_CLOCK_SET_GPIO_Port GPIOB
#define GAS_HEATER_Pin GPIO_PIN_8
#define GAS_HEATER_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
