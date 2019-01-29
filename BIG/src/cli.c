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
#include "adc_ads1256.h"
#include "dac_ad5662.h"

QueueHandle_t CliDataQueue;
static uint8_t * UartRxBuffer;
#define UART_RX_BUF_SIZE 128

static bool InfoPending=false;

static void CliParseCommand(void);

static void CliSendCmd(CliCmd_t * msg, QueueHandle_t Destination);


void cli_task(void * parm)
{
    Message_t PortMsg;

	{
		char * msg = "B.I.G. - Bedbug Intelligence Group - PSTAT\n\r";

		HAL_UART_Transmit(cli_uart, (uint8_t*)msg, strlen(msg), 100);
	}

	InfoPending = false;
	// Parse and distribute incoming messages

	// Ready for a message
	UartRxBuffer=pvPortMalloc(TXT_MESSAGE_BUF_SIZE);

	HAL_UART_Receive_IT( cli_uart, (uint8_t*)UartRxBuffer, UART_RX_BUF_SIZE);


	for (;;)
	{
		// Handle CLI Queue
		if (xQueueReceive( CliDataQueue, (void*)&PortMsg, 10 ))
		{
			if (PortMsg.Type == CLI_TEXT_MESSAGE)
			{
			    CliMsgContainer* msg = (CliMsgContainer*)PortMsg.data;

                // CLI Message
                HAL_UART_Transmit(cli_uart, (uint8_t*)msg->string, strlen(msg->string), 100); // This is a blocking call.

                // Free memory
                vPortFree(msg->string);
                vPortFree(msg);
            }
            else if (PortMsg.Type == CLI_COMMAND_RESP)
            {
                // TBD
                // Free memory

                vPortFree(PortMsg.data);
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
			HAL_UART_Receive_IT( cli_uart, (uint8_t*)UartRxBuffer, UART_RX_BUF_SIZE);

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

void CliSendResp(CliCmd_t * msg)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
	Msg.Type = CLI_COMMAND_RESP;

	memcpy(&payload->Cmd, msg, sizeof(CliCmd_t));

	xQueueSend( CliDataQueue, &Msg, 0 );
}

void CliSendCmd(CliCmd_t * msg, QueueHandle_t Destination)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
    Msg.Type = CLI_COMMAND_MESSAGE;

    memcpy(&payload->Cmd, msg, sizeof(CliCmd_t));

    xQueueSend( Destination, &Msg, 0 );
}


void CliInfoPending(void)
{
	InfoPending = true;
}

static const char * OkMsg = "OK\n\r";

static uint8_t CliParseParameterString(char * src, uint8_t *params, uint8_t buf_length )
{
    int count;
    uint8_t val;

    while (*src != 0)
    {
        src++;
    }
}

void CliParseCommand(void)
{
    int length = cli_uart->RxXferCount;

    if (length <= 0)  return;

    if (UartRxBuffer[0] == '*')
    {
        CliCmd_t cmd;

        // Parse commands.
        if (strcmp("adc_read", (const char*)&UartRxBuffer[1]) == 0)
        {
            // ADC Read
            cmd.Id = PSTAT_MEASUREMENT_REQ;

            CliSendCmd( &cmd, pstat_Queue);

        }
    }
    else
    {
        /// All text just send to the smartIO interface
        CliSendTextMsgNoCopy( (char*)UartRxBuffer, SifQueue);

        // Send OK
        HAL_UART_Transmit(cli_uart, (uint8_t*)OkMsg, length, 100);
    }

}
