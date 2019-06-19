/*
 * isr.c
 *
 *  Created on: Nov 19, 2018
 *      Author: carstenh
 */
#include <main.h>
#include <stddef.h>
#include <string.h>
#ifdef STM32F205xx
#include "stm32f2xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif
#include "isr.h"
#include <usart.h>
#include "cli.h"
#include "pstat.h"

void (*BT_ISR_ptr)(void) = NULL;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == ADC_DRDYn_Pin)
	{
	    pstat_measure_data_ready();
	}
	else if ( (GPIO_Pin == BT_IRQ_Pin) && (BT_ISR_ptr != NULL))
	{
		BT_ISR_ptr();
	}
}


void HAL_GPIO_EXTI_Enable(void)
{
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void USART3_ISR_ENABLE(void)
{
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 3);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void USART2_ISR_ENABLE(void)
{
	HAL_NVIC_SetPriority(USART2_IRQn, 6, 2);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->pRxBuffPtr != NULL)
	{
		huart->pRxBuffPtr[huart->RxXferCount] = 0;	// Making sure we are 0 terminated
		CliInfoPending();
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_RESET);    // spi1.ChipSelect();
    DataPortTxComplete = true;
}


