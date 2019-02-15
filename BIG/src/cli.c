/*
 * cli.c
 *
 *  Created on: Jan 12, 2019
 *      Author: carstenh
 */
#include <string.h>
#include <stdlib.h>
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

                *outp = 0;  // Null terminating the buffer

                strcat(outp, "  ADC_WE: ");
                NumToDecimalString( msg->ADC_CONV_RESP_data.ADC_WE, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", ADC_RE: ");
                NumToDecimalString( msg->ADC_CONV_RESP_data.ADC_RE, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", ADC_DAC_RE: ");
                NumToDecimalString( msg->ADC_CONV_RESP_data.ADC_DAC_RE, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", REF_1_3rd: ");
                NumToDecimalString( msg->ADC_CONV_RESP_data.VREF_1_3rd, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", REF_2_3rd: ");
                NumToDecimalString( msg->ADC_CONV_RESP_data.VREF_2_3rd, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", Scale: ");
                NumToDecimalString( msg->ADC_CONV_RESP_data.WE_Scale, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", DAC: ");
                NumToDecimalString( msg->ADC_CONV_RESP_data.DAC_Setting, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, ", SW: ");
                NumToDecimalString( msg->ADC_CONV_RESP_data.SwitchState, 8, temp, 24);
                strcat(outp, temp);

                strcat(outp, "\n\r");

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

			// Parse and distribute incoming messages. It is assumed that the UartRxBuffer is disposed of in this call.
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
    payload->CLI_COMMAND_data.Param1 = data;

	xQueueSend( CliDataQueue, &Msg, 0 );
}

void CliSendCmd(CliCommandId CmdId, uint32_t data, QueueHandle_t Destination)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
    Msg.Type = CLI_COMMAND_MESSAGE;
    payload->CLI_COMMAND_data.Id = CmdId;
    payload->CLI_COMMAND_data.Param1 = data;

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

#define MAX_NUM_PARAMS 8
static uint32_t parameter_list[MAX_NUM_PARAMS];


static int CliParseParameterString(int offset);

void CliParseCommand(void)
{
    int length = cli_uart->RxXferCount;
    int num_args = 0;

    if (length <= 0)  return;

    // Parse commands.
    if (strncmp("meas", (const char*)UartRxBuffer, 4) == 0)
    {
        uint32_t parm = DAC_INVALID_VALUE;

        num_args = CliParseParameterString(4);

        if (num_args >= 1)
        {
            parm = parameter_list[0];
        }
        // Perform a measurement
        CliSendCmd(PSTAT_MEASUREMENT_REQ, parm, pstat_Queue);

        vPortFree(UartRxBuffer);
    }
    else if (strncmp("pson", (const char*)UartRxBuffer, 4) == 0)
    {
        uint32_t parm = SW_INVALID;

        num_args = CliParseParameterString(4);

        if (num_args >= 1)
        {
            parm = parameter_list[0];
        }

        // Set PSTAT switches according to parm
        CliSendCmd(PSTAT_ON_REQ, parm, pstat_Queue);

        vPortFree(UartRxBuffer);
    }
    else if (strncmp("cal", (const char*)UartRxBuffer, 4) == 0)
    {
        // Run self calibration
        CliSendCmd(PSTAT_CAL_REQ, 0, pstat_Queue);

        vPortFree(UartRxBuffer);
    }
    else if (strncmp("sio", (const char*)UartRxBuffer, 3) == 0)
    {
        char * p;
        int i = 3;  // Length of "sio"

        p = (char*)UartRxBuffer + i;

        while ( p[i] == ' ')
        {
            i++;
        }

        if (p[i] != 0)
        {
            /// Send All text just received to the smartIO interface
            CliSendTextMsgNoCopy( &p[i], SifQueue);

            // Send OK
            HAL_UART_Transmit(cli_uart, (uint8_t*)OkMsg, length, 100);
        }
    }

}

int CliParseParameterString(int offset)
{
    int count = 0;
    char *p;
    char temp[16];
    int idx;
    char *tp;
    bool HexMode = false;

    p = (char*)UartRxBuffer + offset;
    tp = temp;
    *tp = 0;
    idx = 0;
    while (1)
    {
        if (*p <= ' ')
        {
            *tp = 0;
            // See what is in the temp buffer
            if (idx > 0)
            {
                if (HexMode)
                {
                    parameter_list[count] = strtol(temp, NULL, 16);
                }
                else
                {
                    parameter_list[count] = strtol(temp, NULL, 10);
                }

                count++;

            }

            if ( (*p != 0) && (count < MAX_NUM_PARAMS))
            {
                // Start again if there is more to do
                HexMode = false;
                idx = 0;
                tp = temp;
            }
            else
            {
                // Done
                break;
            }
        }
        else if (idx < sizeof(temp)-1)
        {
            if ( (*p >= '0') && (*p <= '9'))
            {
                *tp++ = *p;
                idx++;
            }
            else if ( ((*p >= 'a') && (*p <= 'f')) || ((*p>='A') && (*p <='F')))
            {
                *tp++ = *p;
                idx++;
                HexMode = true;
            }
            else if ( (*p == 'x') || (*p == 'X') )
            {
                HexMode = true;
                // All leading values discarded
                tp = temp;
                idx = 0;
            }
        }
        p++;
    }
    return(count);
}
