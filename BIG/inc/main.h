/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H__
#define __MAIN_H__

/* Includes ------------------------------------------------------------------*/
 /* Scheduler includes. */
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* USER CODE BEGIN Includes */

#define BT_IS_HW_DEBUG  // Use BT SPI IO lines for hardware debug signals - see gpio.h and c
#define CARSTENS_BOARD      // Hacked up board with MISO re-routed
#define VERSION "BIG pstat 2.7" // Version rules: Any text you want NOT containing periods, followed by version number formatted as <major>.<minor> at the end

// OS Messaging Queues. They are defined in the respective task source files but references are collated here for simplicity
extern QueueHandle_t SpiSmartIoQueue;	// Queue for SmartIO task SPI interface (separate from others)
extern QueueHandle_t SpiSendQueue;  // Queue for SPI Driver using container
extern QueueHandle_t CliDataQueue;  // Queue for CLI task
extern QueueHandle_t SifQueue;  	// Second Queue for the SmartIO task for general messaging.

extern QueueHandle_t ADC_Queue;		// Reply Queue for ADC driver
extern QueueHandle_t DAC_Queue;		// Reply Queue for DAC driver

extern QueueHandle_t pstat_Queue;   // General pstat task queue
extern QueueHandle_t CliMeasurement_Queue;    // Dedicated measurement result queue

// Global message type definitions, Unique ids for all messages makes identification and tracing of messages easier.
typedef enum
{
	SPI_SEND_MESSAGE,
	SPI_ADC_IO_MESSAGE,
	SPI_DAC_IO_MESSAGE,
	SPI_READ_MESSAGE,
	SPI_INT_MODE_SET,
    CLI_TEXT_MESSAGE,
    CLI_COMMAND_MESSAGE,
    CLI_COMMAND_RESP,
    CLI_MEASUREMENT_RESP,
    CLI_MEASUREMENT_DATA,
    PSTAT_COMMAND_MESSAGE,
    PSTAT_INFO_IND,
	SIF_MESSAGE,
	MEASUREMENT_MESSAGE
} MessageType;

// Global message type used for all communications. All messages posted to queues are done so via copy
// the data pointer can be either dynamic or static.
typedef struct
{
    MessageType Type;
    uint8_t *data;
} Message_t;

/* USER CODE END Includes */

/* Private define ------------------------------------------------------------*/

#define PC14_OSC32_IN_Pin GPIO_PIN_14
#define PC14_OSC32_IN_GPIO_Port GPIOC
#define PC15_OSC32_OUT_Pin GPIO_PIN_15
#define PC15_OSC32_OUT_GPIO_Port GPIOC
#define PH0_OSC_IN_Pin GPIO_PIN_0
#define PH0_OSC_IN_GPIO_Port GPIOH
#define PH1_OSC_OUT_Pin GPIO_PIN_1
#define PH1_OSC_OUT_GPIO_Port GPIOH
#define OTG_FS_PowerSwitchOn_Pin GPIO_PIN_0
#define OTG_FS_PowerSwitchOn_GPIO_Port GPIOC
#define PDM_OUT_Pin GPIO_PIN_3
#define PDM_OUT_GPIO_Port GPIOC
#define B1_Pin GPIO_PIN_0
#define B1_GPIO_Port GPIOA
#define BOOT1_Pin GPIO_PIN_2
#define BOOT1_GPIO_Port GPIOB
#define ADC_DRDYn_Pin GPIO_PIN_5
#define ADC_DRDYn_GPIO_Port GPIOC
#define ADC_DRDYn_EXTI_IRQn EXTI9_5_IRQn
#define ADC_RSTn_Pin GPIO_PIN_0
#define ADC_RSTn_GPIO_Port GPIOB
#define ADC_PDN_Pin GPIO_PIN_1
#define ADC_PDN_GPIO_Port GPIOB
#define ADC_CSn_Pin GPIO_PIN_12
#define ADC_CSn_GPIO_Port GPIOB
#define DAC_CSn_Pin GPIO_PIN_13
#define DAC_CSn_GPIO_Port GPIOB
#define BT_CSn_Pin GPIO_PIN_14
#define BT_CSn_GPIO_Port GPIOB
#define BT_RST_Pin GPIO_PIN_15
#define BT_RST_GPIO_Port GPIOB
#define LD4_Pin GPIO_PIN_12
#define LD4_GPIO_Port GPIOD
#define LD3_Pin GPIO_PIN_13
#define LD3_GPIO_Port GPIOD
#define LD5_Pin GPIO_PIN_14
#define LD5_GPIO_Port GPIOD
#define LD6_Pin GPIO_PIN_15
#define LD6_GPIO_Port GPIOD
#define BT_IRQ_Pin GPIO_PIN_9
#define BT_IRQ_GPIO_Port GPIOA
#define SWDIO_Pin GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA
#define SWCLK_Pin GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA
#define SPI1_SCK_Pin GPIO_PIN_3
#define SPI1_SCK_GPIO_Port GPIOB
#define SPI1_MISO_Pin GPIO_PIN_4
#define SPI1_MISO_GPIO_Port GPIOB
#define SPI1_MOSI_Pin GPIO_PIN_5
#define SPI1_MOSI_GPIO_Port GPIOB
#define SW4_Pin GPIO_PIN_6
#define SW4_GPIO_Port GPIOC
#define SW3_Pin GPIO_PIN_7
#define SW3_GPIO_Port GPIOC
#define SW2_Pin GPIO_PIN_8
#define SW2_GPIO_Port GPIOC
#define SW1_Pin GPIO_PIN_9
#define SW1_GPIO_Port GPIOC
#define G_S0_Pin GPIO_PIN_0
#define G_S0_GPIO_Port GPIOC
#define G_S1_Pin GPIO_PIN_1
#define G_S1_GPIO_Port GPIOC
#define G_S2_Pin GPIO_PIN_2
#define G_S2_GPIO_Port GPIOC


/* ########################## Assert Selection ############################## */
/**
  * @brief Uncomment the line below to expanse the "assert_param" macro in the 
  *        HAL drivers code
  */
/* #define USE_FULL_ASSERT    1U */

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
 extern "C" {
#endif
void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)
#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
