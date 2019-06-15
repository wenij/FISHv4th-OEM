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
QueueHandle_t CliMeasurement_Queue = NULL;


static uint8_t * UartRxBuffer;
#define UART_RX_BUF_SIZE 128

static bool HexOutput = true;  // output mode

static bool InfoPending=false;

volatile bool DataPortTxComplete;


static void CliParseCommand(void);

static void CliSendCmd(CliCommandId CmdId, uint32_t data, uint32_t data2, QueueHandle_t Destination);

void cli_task(void * parm)
{
    Message_t PortMsg;
    pstatDynamicMeasurement_t DataMsg;


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
		    if (PortMsg.Type == PSTAT_INFO_IND)
		    {
                PstatMsgContainer_t *payload = (PstatMsgContainer_t*)PortMsg.data;

                if (payload != NULL)
                {
                    CliSendDataPortMeasurementDone( payload->Req.RunStats.Good_Count, payload->Req.RunStats.Fail_Count);

                    vPortFree(payload);
                }

		    }
		    else if (PortMsg.Type == CLI_TEXT_MESSAGE)
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
                    break;
                }

                // Free memory
                if (PortMsg.data != NULL)
                {
                    vPortFree(PortMsg.data);
                }

            }
            else if (PortMsg.Type == CLI_MEASUREMENT_RESP)
            {
                int bufSize = 300;
                int tempSize = 24;
                CliMsgContainer* msg = (CliMsgContainer*)PortMsg.data;

                char * outp = (char*)pvPortMalloc(bufSize);
                char * temp = (char*)pvPortMalloc(tempSize);

                *outp = 0;  // Null terminating the buffer

                strcat(outp, " TS: ");
                NumToString( msg->ADC_CONV_RESP_data.TimeStamp, 8, temp, tempSize, HexOutput);
                strcat(outp, temp);

                if (msg->ADC_CONV_RESP_data.ADC_WE != ADC_NOT_PRESENT)
                {
                    strcat(outp, ", ADC_WE: ");
                    NumToString( msg->ADC_CONV_RESP_data.ADC_WE, 8, temp, tempSize, HexOutput);
                    strcat(outp, temp);
                }

                if (msg->ADC_CONV_RESP_data.ADC_RE != ADC_NOT_PRESENT)
                {
                    strcat(outp, ", ADC_RE: ");
                    NumToString( msg->ADC_CONV_RESP_data.ADC_RE, 8, temp, tempSize, HexOutput);
                    strcat(outp, temp);
                }

                if (msg->ADC_CONV_RESP_data.ADC_DAC_RE != ADC_NOT_PRESENT)
                {
                    strcat(outp, ", ADC_DAC_RE: ");
                    NumToString( msg->ADC_CONV_RESP_data.ADC_DAC_RE, 8, temp, tempSize, HexOutput);
                    strcat(outp, temp);
                }

                if (msg->ADC_CONV_RESP_data.VREF_1_3rd != ADC_NOT_PRESENT)
                {
                    strcat(outp, ", REF_1_3rd: ");
                    NumToString( msg->ADC_CONV_RESP_data.VREF_1_3rd, 8, temp, tempSize, HexOutput);
                    strcat(outp, temp);
                }

                if (msg->ADC_CONV_RESP_data.VREF_2_3rd != ADC_NOT_PRESENT)
                {
                    strcat(outp, ", REF_2_3rd: ");
                    NumToString( msg->ADC_CONV_RESP_data.VREF_2_3rd, 8, temp, tempSize, HexOutput);
                    strcat(outp, temp);
                }

                if (msg->ADC_CONV_RESP_data.TestMeasurement != ADC_NOT_PRESENT)
                {
                    strcat(outp, ", TM: ");
                    NumToString( msg->ADC_CONV_RESP_data.TestMeasurement, 8, temp, tempSize, HexOutput);
                    strcat(outp, temp);
                }

                strcat(outp, ", Scale: ");
                NumToString( msg->ADC_CONV_RESP_data.WE_Scale, 8, temp, tempSize, HexOutput);
                strcat(outp, temp);

                strcat(outp, ", DAC: ");
                NumToString( msg->ADC_CONV_RESP_data.DAC_Setting, 8, temp, tempSize, HexOutput);
                strcat(outp, temp);

                strcat(outp, ", SW: ");
                NumToString( msg->ADC_CONV_RESP_data.SwitchState, 8, temp, tempSize, HexOutput);
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

void CliSendCmd(CliCommandId CmdId, uint32_t data, uint32_t data2,  QueueHandle_t Destination)
{
    Message_t Msg;
    CliMsgContainer *payload = (CliMsgContainer*)pvPortMalloc(sizeof(CliMsgContainer));

    Msg.data = (uint8_t*)payload;
    Msg.Type = CLI_COMMAND_MESSAGE;
    payload->CLI_COMMAND_data.Id = CmdId;
    payload->CLI_COMMAND_data.Param1 = data;
    payload->CLI_COMMAND_data.Param2 = data2;

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
	cli_uart->pRxBuffPtr = NULL;
}

static uint8_t msg_buf[20];
void CliSendDataPortMeasurement( pstatDynamicMeasurement_t * data)
{
    // This is a binary message that goes straight to the UART
    uint8_t * msg = msg_buf;


    msg[0] = 0x55; // Sync 1
    msg[1] = 0xAA; // Sync 2
    msg[2] = 17;    // Length
    msg[3] = 0x01; // ID

    // Timestamp 4 bytes
    msg[4] = (uint8_t)(data->TimeStamp >> 24);
    msg[5] = (uint8_t)(data->TimeStamp >> 16);
    msg[6] = (uint8_t)(data->TimeStamp >> 8);
    msg[7] = (uint8_t)(data->TimeStamp);

    // WE 4 bytes
    msg[8] = (uint8_t)(data->ADC_WE >> 24);
    msg[9] = (uint8_t)(data->ADC_WE >> 16);
    msg[10] = (uint8_t)(data->ADC_WE >> 8);
    msg[11] = (uint8_t)(data->ADC_WE);

    // CE voltage 4 bytes
    msg[12] = (uint8_t)(data->ADC_DAC_RE >> 24);
    msg[13] = (uint8_t)(data->ADC_DAC_RE >> 16);
    msg[14] = (uint8_t)(data->ADC_DAC_RE >> 8);
    msg[15] = (uint8_t)data->ADC_DAC_RE;

    // DAC 2 bytes
    msg[16] = (uint8_t)(data->DAC_Setting >> 8);
    msg[17] = (uint8_t)(data->DAC_Setting);

    // Gain Setting 1 byte
    msg[18] = (uint8_t)data->WE_Scale;

    // Switch setting 1 byte
    msg[19] = (uint8_t)data->SwitchState;

    HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_SET);    // spi1.ChipSelect();
    DataPortTxComplete = false;
    HAL_UART_Transmit_DMA(data_uart, msg, 20);   // Non-blocking Call
}

static uint8_t TxMsg[13];

void CliSendDataPortMeasurementDone( uint32_t GoodMeasurementCount, uint32_t BadMeasurementCount)
{
    // This is a binary message that goes straight to the UART
    uint8_t * msg = TxMsg;

    do {
        if (DataPortTxComplete) break;
    } while (1);

    msg[0] = 0x55; // Sync 1
    msg[1] = 0xAA; // Sync 2
    msg[2] = 9;
    msg[3] = 0x02; // ID
    msg[4] =(uint8_t)(GoodMeasurementCount >> 24);
    msg[5] =(uint8_t)(GoodMeasurementCount >> 16);
    msg[6] =(uint8_t)(GoodMeasurementCount >> 8);
    msg[7] =(uint8_t)(GoodMeasurementCount);
    msg[8] =(uint8_t)(BadMeasurementCount >> 24);
    msg[9] =(uint8_t)(BadMeasurementCount >> 16);
    msg[10] =(uint8_t)(BadMeasurementCount >> 8);
    msg[11] =(uint8_t)(BadMeasurementCount);
    msg[12] = 0x1A; // EOF marker

    HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_SET);    // spi1.ChipSelect();
    DataPortTxComplete = false;
    HAL_UART_Transmit_DMA(data_uart, msg, 13); // This is a non-blocking call.
}

#define MAX_NUM_PARAMS 16
static uint32_t parameter_list[MAX_NUM_PARAMS];


static int CliParseParameterString(int offset);

static void SendOK(void)
{
    static const char * OkMsg = "OK\n\r";

    HAL_UART_Transmit(cli_uart, (uint8_t*)OkMsg, strlen(OkMsg), 100);
}

static void SendErr(void)
{
    static const char * ErrMsg = "ERR\n\r";

    HAL_UART_Transmit(cli_uart, (uint8_t*)ErrMsg, strlen(ErrMsg), 100);
}

void CliParseCommand(void)
{
    int length = cli_uart->RxXferCount;
    int num_args = 0;
    bool OK = false;

    if (length <= 0)  return;

    // Parse commands.
    if (strncmp("meas", (const char*)UartRxBuffer, 4) == 0)
    {
        uint32_t parm = 0;
        uint32_t parm2 = 0;

        num_args = CliParseParameterString(4);

        OK = true;

        switch (num_args)
        {
        case 0:
            SET_PSTAT_COMMAND(parm, PSTAT_MEASURE_REPEAT);
            break;

        case 1:
            SET_PSTAT_COMMAND(parm, PSTAT_MEASURE_ALL);
            SET_PSTAT_MEASURE_DAC(parm, parameter_list[0]);
            break;

        case 4:
            SET_PSTAT_COMMAND(parm, PSTAT_MEASURE_CHANNELS);
            SET_PSTAT_MEASURE_DAC(parm, parameter_list[0]);
            SET_PSTAT_MEASURE_CHANNELS(parm2, parameter_list[1], parameter_list[2], parameter_list[3]);
            break;
        default:
            OK = false; // Command is not OK
            break;
        }

        if (OK)
        {
            // Perform a measurement
            CliSendCmd(PSTAT_MEASUREMENT_REQ, parm, parm2, pstat_Queue);
        }

    }
    else if (strncmp("pson", (const char*)UartRxBuffer, 4) == 0)
    {
        uint32_t parm = 0;

        num_args = CliParseParameterString(4);

        OK = true;

        switch (num_args)
        {
        case 1:
            SET_PSTAT_COMMAND(parm, PSTAT_ON_SWITCH);
            SET_PSTAT_PSON_SW(parm, parameter_list[0]);
            break;

        case 2:
            SET_PSTAT_COMMAND(parm, PSTAT_ON_SWITCH_GAIN);
            SET_PSTAT_PSON_SW_SG(parm, parameter_list[0], parameter_list[1]);
            break;

        default:
            OK = false;
            break;
        }

        if (OK)
        {
            // Set PSTAT switches according to parm
            CliSendCmd(PSTAT_ON_REQ, parm, 0, pstat_Queue);
        }

    }
    else if (strncmp("cal", (const char*)UartRxBuffer, 3) == 0)
    {
        // Run self calibration
        CliSendCmd(PSTAT_CAL_REQ, 0, 0, pstat_Queue);

        OK = true;
    }
    else if (strncmp("sio ", (const char*)UartRxBuffer, 4) == 0)
    {
        int i = 4;  // Length of "sio"

        while ( UartRxBuffer[i] == ' ')
        {
            i++;
        }

        if (UartRxBuffer[i] != 0)
        {
            /// Send All text just received to the smartIO interface
            CliSendTextMsg( (char*)&UartRxBuffer[i], SifQueue);

            OK = true;
        }
    }
    else if (strncmp("dec", (const char*)UartRxBuffer, 3) == 0)
    {
        // output format is decimal
        HexOutput = false;

        OK = true;
    }
    else if (strncmp("hex", (const char*)UartRxBuffer, 3) == 0)
    {
        // output format is hex
        HexOutput = true;

        OK = true;
    }
    else if (strncmp("run ", (const char*)UartRxBuffer, 4) == 0)
    {
        num_args = CliParseParameterString(4);


        if (num_args == 11)
        {
            // New Command
            PstatRunReq_t cmd;

            cmd.InitialDAC = parameter_list[0];
            cmd.StartDAC = parameter_list[1];
            cmd.EndDAC = parameter_list[2];
            cmd.FinalDAC = parameter_list[3];
            cmd.DACStep = parameter_list[4];
            cmd.DACTime = parameter_list[5];
            cmd.TimeSliceUs = parameter_list[6];
            cmd.MeasureTime = parameter_list[7];
            cmd.Count = parameter_list[8];
            cmd.Switch = parameter_list[9];
            cmd.MeasurementType = (PstatMeasurementType)parameter_list[10];

            PstatSendRunReq( &cmd );

            OK = true;
        }
    }
    else if (strncmp("end", (const char*)UartRxBuffer, 3) == 0)
    {
        OK = true;

        CliSendCmd(PSTAT_CANCEL_REQ, 0, 0, pstat_Queue);
    }
    else if (strncmp("ver", (const char*)UartRxBuffer, 3) == 0)
    {
        // Version number
        char * temp = (char*)pvPortMalloc(32);

        temp[0] = 0;

        strcat(temp, "Version: ");
        strcat(temp, VERSION);
        strcat(temp, "\n\r");
        HAL_UART_Transmit(cli_uart, (uint8_t*)temp, strlen(temp), 100);

        vPortFree(temp);

        OK = true;
    }

    vPortFree(UartRxBuffer);

    if (OK)
    {
        SendOK();
    }
    else
    {
        SendErr();
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
