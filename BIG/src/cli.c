/*
 * cli.c
 *
 *  Created on: Jan 12, 2019
 *      Author: carstenh
 */
#include "cli.h"
#include "usart.h"


QueueHandle_t CliDataQueue;

void cli_task(void * parm)
{

  {
	  char * msg = "B.I.G. - Bedbug Intelligence Group - PSTAT\n\r";

	  HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
  }

	for (;;)
	{
		// Handle CLI
		vTaskDelay(  pdMS_TO_TICKS(20) );	// Give others a chance to run

	}
}
