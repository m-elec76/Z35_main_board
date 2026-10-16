/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "rtc.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "app_config.h"
#include "app_globals.h"
#include "app_utils.h"
#include "app_nrf.h"
#include "app_lighting.h"
#include "app_output.h"
#include "app_setting.h"
#include "nrf24.h"
#include "HD44780.h"
#include "ConvertCalendar.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
static void App_CreateTasks(void);
static void App_CreateQueues(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/*==============================================================================
 *                          ONE-SHOT INIT TASK
 *============================================================================*/
/**
 * @brief  Initialise the nRF24L01+ and the LCD, then delete this task.
 * @param  pvParameters  Unused.
 */
void Init_parameters(void *pvParameters)
{
    (void)pvParameters;

    /*------------------- nRF24L01+ initialisation ----------------------*/
    vTaskDelay(pdMS_TO_TICKS(10U));

    (void)nRF24_Init();
    nRF24_SetPowerMode(nRF24_PWR_UP);
    vTaskDelay(pdMS_TO_TICKS(5U));
    nRF24_SetDataRate(nRF24_DR_250kbps);
    nRF24_SetCRCScheme(nRF24_CRC_2byte);
    nRF24_SetAddrWidth(5U);
    nRF24_SetRXPipe(1U, nRF24_AA_ON, (uint8_t)sizeof(SensorData_t));
    nRF24_FlushTX();
    nRF24_FlushRX();
    nRF24_ClearIRQFlags();
    nRF24_SetOperationalMode(nRF24_MODE_RX);
    apply_mode(nrf_mode);
    nRF24_CE_H();

    vTaskDelay(pdMS_TO_TICKS(150U));

    /*------------------- LCD initialisation ----------------------------*/
    (void)HD44780_Init();
    HD44780_Clear();

    /*------------------- Done: remove this task ------------------------*/
    vTaskDelete(NULL);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI2_Init();
  MX_TIM2_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
    App_CreateQueues();
    App_CreateTasks();

    vTaskStartScheduler();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/*==============================================================================
 *                          QUEUE CREATION
 *============================================================================*/
/**
 * @brief  Create all inter-task queues.
 */
static void App_CreateQueues(void)
{
    nrfDataR_Queue = xQueueCreate(10U, sizeof(SensorData_t));
    if (nrfDataR_Queue == NULL)
    {
        Error_Handler();
    }
}

/*==============================================================================
 *                          TASK CREATION
 *============================================================================*/
/**
 * @brief  Create every FreeRTOS task with the original priorities.
 */
static void App_CreateTasks(void)
{
    (void)xTaskCreate(Init_parameters,   "Init",             512U, NULL, 2U, NULL);
    (void)xTaskCreate(Lighting_Manager,  "Lighting_Manager", 512U, NULL, 1U, NULL);
    (void)xTaskCreate(Task_nRF_Receiver, "nrfRx",            512U, NULL, 3U, &nrfTaskHandle);
    (void)xTaskCreate(Setting,           "setting",          512U, NULL, 1U, &settingTaskHandle);
    (void)xTaskCreate(Output_Manager,    "Output_Manager",   512U, NULL, 1U, NULL);
}

/*==============================================================================
 *                          EXTERNAL INTERRUPT CALLBACK
 *============================================================================*/
/**
 * @brief  Central GPIO EXTI dispatcher.
 * @param  GPIO_Pin  Pin that triggered the interrupt.
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    /*------------------- nRF24L01+ RX_DR ----------------------------*/
    if (GPIO_Pin == EXTI_NRF_PIN)
    {
        BaseType_t   xHigherPriorityTaskWoken = pdFALSE;
        uint8_t      length                   = 0U;
        SensorData_t rxData_temporary         = {0};

        nRF24_ReadPayload((uint8_t *)&rxData_temporary, &length);

        if (rxData_temporary.system_id == system_id)
        {
            (void)xQueueSendFromISR(nrfDataR_Queue, &rxData_temporary,
                                    &xHigherPriorityTaskWoken);
        }
        else
        {
            nRF24_FlushRX();
        }

        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

    /*------------------- Power-loss detection -----------------------*/
    if (GPIO_Pin == EXTI_VOLTAGE_PIN)
    {
        PowerOutage++;
        (void)HAL_GPIO_WritePin(SUN_LAMP, GPIO_PIN_RESET);
        (void)HAL_TIM_Base_Start_IT(&htim2);
    }
}

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	 /*------------------- Sun PWM pulse --------------------------------*/
	    if (htim->Instance == TIM2)
	    {
	        (void)HAL_GPIO_WritePin(SUN_LAMP, GPIO_PIN_SET);
	        (void)HAL_TIM_Base_Stop_IT(&htim2);
	        __HAL_TIM_SET_AUTORELOAD(&htim2, timer2ARRControl);
	    }


  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
