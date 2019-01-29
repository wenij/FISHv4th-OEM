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

void (*BT_ISR_ptr)(void) = NULL;


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if ( (GPIO_Pin == BT_IRQ_Pin) && (BT_ISR_ptr != NULL))
	{
		BT_ISR_ptr();
	}
}

void USART3_ISR_ENABLE(void)
{
    HAL_NVIC_SetPriority(USART3_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
}

void USART2_ISR_ENABLE(void)
{
	HAL_NVIC_SetPriority(USART2_IRQn, 1, 1);
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


