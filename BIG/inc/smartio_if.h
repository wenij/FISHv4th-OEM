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

typedef enum
{
	APP_IS_OFFLINE,
	APP_CAME_ONLINE,
	APP_IS_ONLINE

} command_app_state_t;

// Main command handler. Pass it a string from the UART and it will buffer the string and act upon it if terminated with CRLF. In this demo mode it will send things to the app.
extern bool Command( char * buf, int length);

extern void SifInit(void);	// Initializes command handler

extern void SifAppInit(void);  // Initialize App

extern bool AppConnected;
extern command_app_state_t AppCommandHandler(void);

extern void SifSendInfoString(char * info);

extern void SifInfoPending(void);

extern void SifTask( void *params);

#endif /* SMARTIO_IF_H_ */
