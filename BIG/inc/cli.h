/*
 * cli.h
 *
 *  Created on: Jan 12, 2019
 *      Author: carstenh
 *
 * Command Line Interface handler task. Uses USART2.
 */

#ifndef CLI_H_
#define CLI_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include "main.h"

 /* Scheduler includes. */
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pstat.h"

#define TXT_MESSAGE_BUF_SIZE 128

 typedef enum
 {
     PSTAT_MEASUREMENT_REQ,
     PSTAT_CAL_REQ,
     PSTAT_ON_REQ
 } CliCommandId;

 typedef struct
 {
     CliCommandId Id;
     uint32_t Param1;
     uint32_t Param2;
 } CliCmd_t;

// CLI message container
 typedef struct
 {
	union
	{
	    char * CLI_MESSAGE_data;	// String associated with CLI_MESSAGE
	    CliCmd_t CLI_COMMAND_data;  // data structure associated with CLI_COMMAND
		pstatMeasurement_t ADC_CONV_RESP_data;  // ADC_CONV_RESP structure
	};
 } CliMsgContainer;



extern void cli_task(void * parm);

// Send text to CLI. The text message is copied in to the message.
// This way it can be called with a text in quotes "text" as a parameter.
extern void CliSendTextMsg(char * text, QueueHandle_t * queue);

extern void CliSendCmdResp(CliCommandId CmdId, uint32_t data);

extern void CliSendMeasurementResp(pstatMeasurement_t * data);

extern void CliInfoPending(void);

extern void CliSendDataPortMeasurement( pstatMeasurement_t * data); // Specifically sends a measurement on the data port
extern void CliSendDataPortMeasurementDone( uint32_t GoodMeasurementCount, uint32_t BadMeasurementCount);

// Binary CLI data port messages
#define CLI_SYNC1 0x55
#define CLI_SYNC2 0xAA

#define CLI_CMD_BUFFER_LENGTH(_len) ((_len)+3)
#define CLI_MEASURED_DATA_ID 1
#define CLI_MEASURED_DATA_LENGTH 17

#define CLI_MEASUREMENT_DONE_ID 2
#define CLI_MEASUREMENT_DATA_LENGTH 1


#ifdef __cplusplus
}
#endif
#endif /* CLI_H_ */
