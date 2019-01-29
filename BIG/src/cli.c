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
#include "util.h"

QueueHandle_t CliDataQueue;
static uint8_t * UartRxBuffer;
#define UART_RX_BUF_SIZE 128

static bool InfoPending=false;

static void CliParseCommand(void);

static void CliSendCmd(CliCommandId CmdId, uint32_t data, QueueHandle_t Destination);


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
                HAL_UART_Transmit(cli_uart, (uint8_t*)msg->CLI_MESSAGE_data, strlen(msg->CLI_MESSAGE_data), 100); // This is a blocking call.

                // Free memory
                vPortFree(msg->CLI_MESSAGE_data);
                vPortFree(msg);
            }
            else if (PortMsg.Type == CLI_COMMAND_RESP)
            {
                CliMsgContainer* msg = (CliMsgContainer*)PortMsg.data;

                switch (msg->CLI_COMMAND_data.Id)
                {

                default:
                    vPortFree(PortMsg.data);
                    break;
                }
                // Free memory

            }
            else if (PortMsg.Type == CLI_MEASUREMENT_RESP)
            {
                int bufSize = 300;
                CliMsgContainer* msg = (CliMsgContainer*)PortMsg.data;

                char * outp = (char*)pvPortMalloc(bufSize);
                char * temp = (char*)pvPortMalloc(24);

                *outp = 0;

                strcat(outp, "  ADC_WE: ");
                NumToHexString( msg->ADC_CONV_RESP_data.ADC_WE, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", ADC_RE: ");
                NumToHexString( msg->ADC_CONV_RESP_data.ADC_RE, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", ADC_DAC_RE: ");
                NumToHexString( msg->ADC_CONV_RESP_data.ADC_DAC_RE, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", REF_1_3rd: ");
                NumToHexString( msg->ADC_CONV_RESP_data.VREF_1_3rd, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", REF_2_3rd: ");
                NumToHexString( msg->ADC_CONV_RESP_data.VREF_2_3rd, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", Scale: ");
                NumToHexString( msg->ADC_CONV_RESP_data.WE_Scale, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", DAC: ");
                NumToHexString( msg->ADC_CONV_RESP_data.DAC_Setting, 8, temp, 24);
                strcat(outp, temp);

                vPortFree(PortMsg.data);
                vPortFree(temp);

                // Send measurements out
                HAL_UART_Transmit(cli_uart, (uint8_t*)outp, strlen(outp), 100); // This is a blocking call.

                vPortFree(outp);

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

	payload->CLI_MESSAGE_data = (char *)pvPortMalloc( strlen(text) + 2);
	strcpy( payload->CLI_MESSAGE_data, text);

	xQueueSend( queue, &Msg, 0 );
}

void CliSendTextMsgNoCopy(char * text, QueueHandle_t * queue)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
    Msg.Type = CLI_TEXT_MESSAGE;

    payload->CLI_MESSAGE_data = text;

    xQueueSend( queue, &Msg, 0 );
}

void CliSendCmdResp(CliCommandId CmdId, uint32_t data)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
	Msg.Type = CLI_COMMAND_RESP;
	payload->CLI_COMMAND_data.Id = CmdId;
    payload->CLI_COMMAND_data.Param = data;

	xQueueSend( CliDataQueue, &Msg, 0 );
}

void CliSendCmd(CliCommandId CmdId, uint32_t data, QueueHandle_t Destination)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
    Msg.Type = CLI_COMMAND_MESSAGE;
    payload->CLI_COMMAND_data.Id = CmdId;
    payload->CLI_COMMAND_data.Param = data;

    xQueueSend( Destination, &Msg, 0 );
}

void CliSendMeasurementResp(pstatMeasurement_t * data)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
    Msg.Type = CLI_MEASUREMENT_RESP;

    memcpy( &payload->ADC_CONV_RESP_data, data, sizeof(pstatMeasurement_t));

    xQueueSend( CliDataQueue, &Msg, 0 );
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
        // Parse commands.
        if (strcmp("meas", (const char*)&UartRxBuffer[1]) == 0)
        {
            // ADC Read
            CliSendCmd(PSTAT_MEASUREMENT_REQ, 0, pstat_Queue);
        }

        vPortFree(UartRxBuffer);
    }
    else
    {
        /// All text just send to the smartIO interface
        CliSendTextMsgNoCopy( (char*)UartRxBuffer, SifQueue);

        // Send OK
        HAL_UART_Transmit(cli_uart, (uint8_t*)OkMsg, length, 100);
    }

}
