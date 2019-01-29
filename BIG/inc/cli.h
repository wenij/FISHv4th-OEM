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
     ADC_CAL_REQ,
     DAC_SET_REQ,
     DAC_CAL_REQ
 } CliCommandId;

 typedef struct
 {
     CliCommandId Id;
     uint32_t Param;
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

extern void CliSendTextMsgNoCopy(char * text, QueueHandle_t * queue); // Same as Cli Send Text Message but the text isn't copied before sending

extern void CliSendCmdResp(CliCommandId CmdId, uint32_t data);

extern void CliSendMeasurementResp(pstatMeasurement_t * data);

extern void CliInfoPending(void);



#ifdef __cplusplus
}
#endif
#endif /* CLI_H_ */
