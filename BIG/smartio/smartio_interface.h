/* Copyright 2017 ImageCraft Creations Inc., All rights reserved.
 * Smart.IO Host Interface Layer
 * https://imagecraft.com/smartio/
 */
#pragma once
#include <stdint.h>

enum { SMARTIO_STATUS_OK, SMARTIO_ERROR_BADRETURN, SMARTIO_ERROR_BAD_RETURN_HANDLE, SMARTIO_ERROR_BAD_ASYNC_HANDLE,
       SMARTIO_ERROR_CALLBACK_TABLE_TOO_SMALL,
       SMARTIO_ERROR_CALLBACK_HANDLE_NOT_FOUND, SMARTIO_ERROR_BUFFER_TOOSMALL, SMARTIO_ERROR_INVALID_HANDLE2,
       SMARTIO_ERROR_BAD_ARGTYPE, SMARTIO_ERROR_COMMAND_BUFFER_OVERFLOW,
};

enum { SMARTIO_ARG_NONE, SMARTIO_ARG_ONE, SMARTIO_ARG_TWO, SMARTIO_ARG_THREE, SMARTIO_ARG_FOUR, SMARTIO_ARG_ONE32, SMARTIO_ARG_STRING };

enum SPI_State { SPI_NOTCONNECTED, SPI_IDLE, SPI_TRANSMITTING, SPI_SMARTIO_REPLY, SPI_SMARTIO_ASYNC_REQUEST };
extern volatile enum SPI_State SPI_State;

uint32_t SmartIO__SendBytes(unsigned char *sendbuf, int sendlen);
void SmartIO__RegisterCallback(uint16_t handle, void (*func)(), int argtype);
void SmartIO_ISR(void);
void SmartIO_ProcessUserInput(void);
