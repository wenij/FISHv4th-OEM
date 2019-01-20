/*
 * cli.c
 *
 *  Created on: Jan 12, 2019
 *      Author: carstenh
 */
#include <string.h>

#include "main.h"
#include "cli.h"
#include "usart.h"
#include "smartio_if.h"

QueueHandle_t CliDataQueue;
static uint8_t * UartRxBuffer;

static bool InfoPending=false;

static void CliParseCommand(void);

void cli_task(void * parm)
{
    Message_t PortMsg;

	{
		char * msg = "B.I.G. - Bedbug Intelligence Group - PSTAT\n\r";

		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
	}

	InfoPending = false;
	// Parse and distribute incoming messages

	// Ready for a message
	UartRxBuffer=pvPortMalloc(TXT_MESSAGE_BUF_SIZE);

	HAL_UART_Receive_IT( &huart2, (uint8_t*)UartRxBuffer, sizeof UartRxBuffer);


	for (;;)
	{
		// Handle CLI Queue
		if (xQueueReceive( CliDataQueue, (void*)&PortMsg, 10 ))
		{
			if (PortMsg.Type == CLI_TEXT_MESSAGE)
			{
			    CliMsgContainer* msg = (CliMsgContainer*)PortMsg.data;

                // CLI Message
                HAL_UART_Transmit(&huart2, (uint8_t*)msg->string, strlen(msg->string), 100); // This is a blocking call.

                // Free memory
                vPortFree(msg->string);
                vPortFree(msg);
            }
            else if (PortMsg.Type == CLI_COMMAND_MESSAGE)
            {
                // TBD

            }

		}

		// check for incoming USART messages
		if (InfoPending)
		{
			InfoPending = false;

			// Parse and distribute incoming messages
			CliParseCommand();

			// Ready for another message
			UartRxBuffer=pvPortMalloc(TXT_MESSAGE_BUF_SIZE);
			HAL_UART_Receive_IT( &huart2, (uint8_t*)UartRxBuffer, sizeof UartRxBuffer);

		}

	}
}


void CliSendTextMsg(char * text, QueueHandle_t * queue)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
	Msg.Type = CLI_TEXT_MESSAGE;

	payload->string = (char *)pvPortMalloc( strlen(text) + 2);
	strcpy( payload->string, text);

	xQueueSend( queue, &Msg, 0 );
}

void CliSendTextMsgNoCopy(char * text, QueueHandle_t * queue)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
    Msg.Type = CLI_TEXT_MESSAGE;

    payload->string = text;

    xQueueSend( queue, &Msg, 0 );
}

void CliSendCmd(CliCmd_t * msg)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
	Msg.Type = CLI_COMMAND_MESSAGE;

	memcpy(&payload->Cmd, msg, sizeof(CliCmd_t));

	xQueueSend( CliDataQueue, &Msg, 0 );
}

void CliInfoPending(void)
{
	InfoPending = true;
}

static const char * OkMsg = "OK\n\r";

void CliParseCommand(void)
{
	// For now just send data to the SmartIO task.
    {
        CliSendTextMsgNoCopy( (char*)UartRxBuffer, SifQueue);

        // Send OK
        HAL_UART_Transmit(&huart2, (uint8_t*)OkMsg, strlen(OkMsg), 100);
    }

}
