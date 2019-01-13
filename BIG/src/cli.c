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
CliPortMessage PortMsg;

static bool InfoPending=false;
static char MsgBuf[TXT_MESSAGE_BUF_SIZE];	// UART incoming message buffer
static char MsgBufIPC[TXT_MESSAGE_BUF_SIZE];	// IPC Message buffer

static void CliParseCommand(void);

void cli_task(void * parm)
{

	{
		char * msg = "B.I.G. - Bedbug Intelligence Group - PSTAT\n\r";

		HAL_UART_Transmit(&huart2, (uint8_t*)msg, strlen(msg), 100);
	}

	InfoPending = false;
	// Parse and distribute incoming messages

	// Ready for another message
	HAL_UART_Receive_IT( &huart2, (uint8_t*)MsgBuf, sizeof MsgBuf);


	for (;;)
	{
		// Handle CLI Queue
		if (xQueueReceive( CliDataQueue, (void*)&PortMsg, 10 ))
		{
			if (PortMsg.Type == CLI_MESSAGE_TYPE)
			{
				if (PortMsg.CliMsg.CliMessageType == CLI_MESSAGE)
				{
					// CLI Message
					HAL_UART_Transmit(&huart2, (uint8_t*)PortMsg.CliMsg.msg, strlen(PortMsg.CliMsg.msg), 100);
				}
				else if (PortMsg.CliMsg.CliMessageType == CLI_COMMAND)
				{
					// TBD

				}

			}
		}

		// check for incoming USART messages
		if (InfoPending)
		{
			InfoPending = false;

			// Parse and distribute incoming messages
			CliParseCommand();

			// Ready for another message
			HAL_UART_Receive_IT( &huart2, (uint8_t*)MsgBuf, sizeof MsgBuf);

		}

	}
}


void CliSendMsg(CliPortMessage* Msg, char * content)
{
	Msg->Type = CLI_MESSAGE_TYPE;
	Msg->CliMsg.CliMessageType = CLI_MESSAGE;
	Msg->CliMsg.msg = content;

	xQueueSend( CliDataQueue, Msg, 0 );
}

void CliSendCmd(CliPortMessage* Msg, CliCmd_t * msg)
{
	Msg->Type = CLI_MESSAGE_TYPE;
	Msg->CliMsg.CliMessageType = CLI_COMMAND;
	memcpy(&Msg->CliMsg.Cmd, msg, sizeof(CliCmd_t));

	xQueueSend( CliDataQueue, Msg, 0 );
}

void CliInfoPending(void)
{
	InfoPending = true;
}

static const char * OkMsg = "OK\n\r";

static SifPortMessage SifMsg;

void CliParseCommand(void)
{
	// For now just send data to the SmartIO task.
	strcpy(MsgBufIPC, MsgBuf);	// Copy to another buffer so we free up the receive buffer immediately.

	SifSendCliMessage(&SifMsg, MsgBufIPC);

	// Send OK
	HAL_UART_Transmit(&huart2, (uint8_t*)OkMsg, strlen(OkMsg), 100);

}
