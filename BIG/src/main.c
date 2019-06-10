
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

// Includes for other tasks
#include "smartio_if.h"
#include "cli.h"
// Fatfs stuff
#include "fatfs.h"

// littlefs includes
#include "lfs.h"
/*
#include "FF_Disk_t.h"
#include "FF_ioman.h"
#include "ff_flashDisk_Init.h"
// THis gets included in the init function.
//#include "ff_disk_t.h"
 *
 */
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
extern DRESULT USER_read (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
extern DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
extern void initialise_monitor_handles(void);
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
// Move to file that initializes the structures
/* FatFs stuff
static FATFS fatfs;
static FIL fatfile;
static DIR fatdir;
static FILINFO fatfileinfo;
*/
// littlefs structs
// This is the 1rst args for the init of the file system
static lfs_t lfs_internal_flash;
// This works, static alone does not. Also can declare struct, but scope is unclear to me.
static struct lfs_config lfs_cfg;
// numerous sub structs to deal with
static lfs_cache_t lfs_read_cache;
static lfs_cache_t lfs_write_cache;

/* this has no typedef
static lfs_config lfs_cfg;
*/
// lfs will be feed these for the read and write cache
// I hear tell they can be smaller.
// The name can be changed if the fatfs dependency is resolved,
// Either by duplicating them for fatfs or deleting fatfs references.
static unsigned char USER_read_buffer[512];
static unsigned char USER_write_buffer[512] = { 0x5f, 0xc5 };
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

  /* USER CODE BEGIN 2  This is tested and works */
  /* 1meg Flash sector allocation
   * sector#	address		size	#512k sectors
   * 0			0x8000000	16k
   * 1			0x8004000	16k
   * 2			0x8008000	16k
   * 3			0x800C000	16k
   * 4			0x8010000	64k
   * 5			0x8020000	128k
   * 6			0x8040000	128k
   * 7			0x8060000	128k
   * 8			0x8080000	128k	250 512k sectors
   * 9			0x80A0000	128k	250 512k sectors
   * 10			0x80C0000	128k	250 512k sectors
   * 11			0x80E0000	128k	250 512k sectors
   */
  /* Unit test USER_read */
//DRESULT readstatus = USER_read(	0,	USER_read_buffer,	0,	1); // Called in the main function.
//if (readstatus)
//	  while(1);
  /* Unit test USER_write */
//  DRESULT writestatus = USER_write(	0,	USER_write_buffer,	0,	1);
//  if (writestatus)
//	  while(1);
// this prototype writes the same sector this reads.
//  readstatus = USER_read(	0,	USER_read_buffer,	0,	1); // Called in the main function.
//  if (readstatus)
//	  while(1);
/*
  FF_Disk_t * p0Disk;
#define flashPARTITION_NUMBER            0
  char diskName[] = {'p','F','L','A','S','H'};
  char *pcName = diskName;

  p0Disk = FF_FlashDiskInit( pcName,
		  //uint8_t *pucDataBuffer,
		  (uint8_t *) 0x8080000,	// beginning of 1000 sectors of flash disk
          //uint32_t ulSectorCount,
		  1000ul,
          //size_t xIOManagerCacheSize )
							  0);
*/
// The above is Fatfs - here is littlefs, lfs
  // Configure the lfs_config struct
  lfs_cfg.read_size = 512;
  lfs_cfg.prog_size = 512;
  lfs_cfg.block_size = 128 * 1024;	// 128k this is the  erase block size.
  lfs_cfg.block_count = 4;
  lfs_cfg.block_cycles = 10000; // Number of erase cycles before we should move data to another block.
  lfs_cfg.cache_size = 128; // littlefs needs a read cache, a program cache, and one additional
		    // cache per file. These are smaller than a block~!
  lfs_cfg.lookahead_size = 8 * 64; // lookahead buffer is stored as a compact bitmap,
		  // so each byte of RAM can track 8 blocks. Blocks are 512k.
		  // The code says: must be multiple of 64-bits
		  // Don't know requirements for this yet. Aren't blocks allocated?
		  // So how many blocks should this be tracking? Right now allocating nominal values.
		  void *context;
		  int (*read)(const struct lfs_config *c, lfs_block_t block,
		              lfs_off_t off, void *buffer, lfs_size_t size);
		  int (*prog)(const struct lfs_config *c, lfs_block_t block,
		              lfs_off_t off, const void *buffer, lfs_size_t size);
		  int (*erase)(const struct lfs_config *c, lfs_block_t block);
		  int (*sync)(const struct lfs_config *c);
		  void *read_buffer; // Optional statically allocated read buffer. Must be cache_size.
		  void *prog_buffer; // Optional statically allocated program buffer. Must be cache_size.
		  void *lookahead_buffer; // Optional statically allocated lookahead buffer. Must be lookahead_size
		    // and aligned to a 64-bit boundary.

		  // Below are optional. file_max is useful. Name length handled by other attributes.
		  lfs_cfg.name_max; // This is optional. The size of the info struct
		  	// is controlled by the LFS_NAME_MAX define.
		  	// Defaults to LFS_NAME_MAX when zero. LFS_NAME_MAX is stored in
		    // superblock and must be respected by other littlefs drivers.
		  lfs_cfg.file_max;
		  lfs_cfg.attr_max;
  // unit test the static struct lfs_cfg
  // either this call or the function added below ruins the debug run - end up in HAL timer crap.
  // commented both out, Need to replug in target?
  //ut_lfs_cfg();
  // configure the lookahead buffers

  // Configure the read cache
  lfs_read_cache.block = 1; // fudge
  lfs_read_cache.buffer = USER_read_buffer; // rename
  lfs_read_cache.off = 0; // fudge
  lfs_read_cache.size = 512; // not sure caches are same size as 512k blocks~!
  // Configure the write cache
  lfs_write_cache.block = 1; // fudge
  lfs_write_cache.buffer = USER_write_buffer; // rename
  lfs_write_cache.off = 0; // fudge
  lfs_write_cache.size = 512; // All the 512's need to be a symbol
// Add the read and write cache to lfs_internal_flash struct
  lfs_internal_flash.pcache = lfs_write_cache; // Verify this is write cache
  lfs_internal_flash.rcache = lfs_read_cache;

  // the lfs_init call prototype here.
  /* USER CODE END 2 */


  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


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
