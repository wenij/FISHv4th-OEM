/**
  ******************************************************************************
  * File Name          : SPI.h
  * Description        : This file provides code for the configuration
  *                      of the SPI instances.
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
#ifndef __spi_H
#define __spi_H
#ifdef __cplusplus
 extern "C" {
#endif

//#define USE_SPI1

/* Includes ------------------------------------------------------------------*/
#ifdef STM32F205xx
#include "stm32f2xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif
#include "main.h"




/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SPI_HandleTypeDef* ActiveSPI;


/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

extern void _Error_Handler(char *, int);

void MX_SPI1_Init(void);

void MX_SPI3_Init(void);


/* USER CODE BEGIN Prototypes */
// SPI Driver
extern void SPI_driver_task( void * params );


typedef struct
{
	// Sends data. Responds with SpiRespData
	uint16_t length;	// Use a 0 length to do receive without transmitting first
	bool Response;
	uint16_t rx_length;	// Expected length of receive data. Use 0 if not known. After a read the actual length is here.
	uint16_t buf_size;
	bool TxError;	// Error occurred during transmit
	bool RxError;   // Error occurred during receive
    uint8_t *data;  // Pointer to payload data
} SpiMsgContainer;


extern bool GetSpiIntMode(void);
extern void SPI_SetIrqMode( bool mode);
extern void SPI_SetIrqModeFromISR( bool mode);

extern void SPI_SendData( MessageType Id, uint16_t length, uint16_t reply_length, uint8_t * data, bool Response);
extern void SPI_SendDataNoResponse( MessageType Id, uint16_t length, uint8_t * data);
extern void SPI_ReadData(MessageType Id, uint8_t reply_length,  uint8_t * data);

// returns number of bytes received
// These are direct to SPI messaging functions. Will block the SPI and the caller must have sole ownership of the SPI
extern int SPI_MsgADC( uint16_t TxLength, uint8_t * TxData, uint8_t * RxData, uint16_t RxLength, bool Response);
extern bool SPI_MsgDAC( uint16_t TxLength, uint8_t * TxData);


/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ spi_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
