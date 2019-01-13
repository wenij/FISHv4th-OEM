/*
 * smartio_if.h
 *
 *  Created on: Nov 13, 2018
 *      Author: carstenh
 */

#ifndef SMARTIO_IF_H_
#define SMARTIO_IF_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "spi.h"
#include "cli.h"

 typedef enum
 {
	 SIF_CLI_MESSAGE,
	 SIF_CLI_COMMAND
 } SifMessageType;

// SIF message container
 typedef struct
 {
	 SifMessageType SifMsgType;
	 union
	 {
		 char * 	Msg;	// Associated with SIF_CLI_MESSAGE
		 CliCmd_t   CliCmd; // Associated with SIF_CLI_COMMAND
	 };
 } SifMessageContainer;


 typedef struct
 {
	 MessageType Type;
	 union
	 {
		 SifMessageContainer SifMsg; // Associated with SIF_MESSAGE
	 };
 } SifPortMessage;


extern void SifTask( void *params);

extern void SifSendCliMessage( SifPortMessage * Msg, char * content);


#endif /* SMARTIO_IF_H_ */
