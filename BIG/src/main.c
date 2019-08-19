
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2018 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include <string.h>

#include "main.h"

#ifdef STM32F205xx
#include "stm32f2xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif

#include "cmsis_os.h"

#include "spi.h"
#include "usart.h"
#include "gpio.h"
#include "isr.h"
#include "tim.h"

/* USER CODE BEGIN Includes */
// define filesystem in use - Defined here outside of main this is GLOBAL for all to see
#define littlefs
//#define fatfs

// Includes for other tasks
#include "smartio_if.h"
#include "cli.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#ifdef littlefs
/* int lfs_PSTAT_init(lfs_t *lfs, const struct lfs_config *cfg)
 * to be called conditionally after test for lfs exists
 */
#include "lfs.h"
#include "flash_io.h"


#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  *
  * @retval None
  */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

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

#ifdef USE_SPI1
  MX_SPI1_Init();
#else
  MX_SPI3_Init();
#endif


#ifdef STM32F205xx
  MX_USART3_UART_Init();
#endif
  MX_USART2_UART_Init();

#ifdef STM32F205xx
  USART3_ISR_ENABLE();
#else
  USART2_ISR_ENABLE();
#endif

  MX_TIM2_Init();
  MX_TIM3_Init();

  //MX_RTC_Init();

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

#ifdef littlefs
  // Initialize file system
  int lfs_PSTAT_status = lfs_PSTAT_init();

  // Test Code
  lfs_file_t file;

  // Create a file
  if (lfs_PSTAT_status == LFS_ERR_OK)
  {
      unsigned char read_buffer[16];

      // Create the file "foo"
      lfs_PSTAT_status = lfsopen( &file, "foo", LFS_O_CREAT|LFS_O_RDWR );

      // Write something to foo
      lfs_PSTAT_status = lfswrite( &file, "This is a test", 14 );

      // Flush and Close foo
      lfs_PSTAT_status = lfsflush( &file );
      lfs_PSTAT_status = lfsclose( &file );

      // Open foo
      lfs_PSTAT_status = lfsopen( &file, "foo", LFS_O_RDWR );

      // Read foo data
      int read_size = lfsread( &file, read_buffer, 16 );

      // Close foo again
      lfs_PSTAT_status = lfsclose( &file );

  }

#endif

  /* Create a queue used by a task.  Messages are received via this queue. */

  SpiSendQueue = xQueueCreate( 4 /*Queue size */, sizeof( Message_t ) );
  SpiSmartIoQueue = xQueueCreate( 1 /*Queue size */, sizeof( Message_t ) );
  CliDataQueue = xQueueCreate( 4 /*Queue size */, sizeof( Message_t ) );
  SifQueue = xQueueCreate( 4 /*Queue size */, sizeof( Message_t ) );
  ADC_Queue = xQueueCreate( 1 /*Queue size */, sizeof( Message_t ) );

  DAC_Queue = xQueueCreate( 1 /*Queue size */, sizeof( Message_t ) );

  pstat_Queue = xQueueCreate( 4 /*Queue size */, sizeof( Message_t ) );
  pstatMeasurement_Queue = xQueueCreate( 128, sizeof(pstatMeasurement_t));  // Large queue pumping results from interrupt to interface.

  xTaskCreate( SPI_driver_task, "SpiDriver", configMINIMAL_STACK_SIZE+100, NULL, 31, NULL );	// Highest Priority

  xTaskCreate( pstat_task, "pstat", configMINIMAL_STACK_SIZE+100, NULL, 30, NULL ); // second highest Priority

  xTaskCreate( cli_task, "CLI", configMINIMAL_STACK_SIZE+100, NULL, 15, NULL );	// middle Priority

  xTaskCreate( SifTask, "SmartIO", configMINIMAL_STACK_SIZE+100, NULL, 1, NULL ); // Lowest Priority

  /* Start scheduler */
  osKernelStart();

  return 0;

  /* USER CODE END 3 */

}

/**
  * @brief System Clock Configuration
  * @retval None
  */
#ifdef STM32F205xx
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

      /**Initializes the CPU, AHB and APB busses clocks
      */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

      /**Initializes the CPU, AHB and APB busses clocks
      */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                                |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      _Error_Handler(__FILE__, __LINE__);
    }

      /**Configure the Systick interrupt time
      */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

      /**Configure the Systick
      */
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
#else
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}
#endif

/* USER CODE BEGIN 4 */
// lfs_config struct unit test
// this is bombing the debugger
/*
 *
 void ut_lfs_cfg(){
	  int readback = lfs_cfg.block_count; // THOUGHT THIS NEEDED CASTING TO ( lfs_size_t)
}
*/
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

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  else if (htim->Instance == TIM3)
  {
      pstat_measure_tick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  file: The file name as string.
  * @param  line: The line in file as a number.
  * @retval None
  */
void _Error_Handler(char *file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
