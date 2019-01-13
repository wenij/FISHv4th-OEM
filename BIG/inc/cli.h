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


#define TXT_MESSAGE_BUF_SIZE 128

 typedef enum
 {
	 CLI_MESSAGE,
	 CLI_COMMAND
 } CliMessageType;

 typedef struct
 {
	 uint8_t Id;
	 uint8_t Parameters[8];
 } CliCmd_t;

// CLI message container
 typedef struct
 {
	CliMessageType CliMessageType;
	union
	{
		char *msg;	// 0-terminated string associated with CLI_MESSAGE
		CliCmd_t Cmd; // Command structure associated with CLI_COMMAND
	};
 } CliMsgContainer;

 typedef struct
 {
	 MessageType Type;
	 union
	 {
		 CliMsgContainer CliMsg;
	 };

 } CliPortMessage;


extern void cli_task(void * parm);

extern void CliSendMsg(CliPortMessage* Msg, char * content);

extern void CliSendCmd(CliPortMessage* Msg, CliCmd_t * msg);

extern void CliInfoPending(void);



#ifdef __cplusplus
}
#endif
#endif /* CLI_H_ */
