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

 typedef enum
 {
	 CLI_MESSAGE,
	 CLI_COMMAND
 } CliMessageType;

// CLI message container
 typedef struct
 {
	CliMessageType CliMessageType;
	uint16_t length;	// length of data in msg buffer
	char *msg;
 } CliMsgContainer;

 typedef struct
 {
	 CliMessageType CliMessageType;
	 uint8_t Id;
	 uint8_t Parameters[8];
 } CliCmdContainer;

 typedef struct
 {
	 MessageType Type;
	 union
	 {
		 CliMsgContainer Message;
		 CliCmdContainer Cmd;
	 };

 } CliPortMessage;


extern void cli_task(void * parm);

#ifdef __cplusplus
}
#endif
#endif /* CLI_H_ */
