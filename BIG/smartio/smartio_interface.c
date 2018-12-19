/* Copyright 2017 ImageCraft Creations Inc., All rights reserved.
 * Smart.IO Host Interface Layer
 * https://imagecraft.com/smartio/
 */
//#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "smartio_api.h"
#include "smartio_interface.h"
#include "smartio_hardware_interface.h"

volatile enum SPI_State SPI_State;

typedef struct callback {
    uint8_t argtype;
    uint16_t handle;
    void (*func)();
} CALLBACK_ENTRY;

CALLBACK_ENTRY callback_table[CALLBACK_TABLE_SIZE];
int callback_table_index;

static CALLBACK_ENTRY *FindCallbackEntry(uint16_t handle);

void (*SmartIO_Connect_Callback)(void);
void (*SmartIO_Disconnect_Callback)(void);

void SmartIO_Init(void (*connect_callback)(void), void (*disconnect_callback)(void))
    {
    SmartIO_Connect_Callback = connect_callback;
    SmartIO_Disconnect_Callback = disconnect_callback;
    SmartIO_HardwareInit(SmartIO_ISR);
    SPI_State = SPI_IDLE;
    }

/* Interrupt handler for the HOST_INTR
 */
void SmartIO_ISR(void)
    {
    if (SPI_State == SPI_NOTCONNECTED)
        return;

    if (SPI_State == SPI_TRANSMITTING)
        SPI_State = SPI_SMARTIO_REPLY;
    else if (SPI_State == SPI_IDLE)
        SPI_State = SPI_SMARTIO_ASYNC_REQUEST;
    }

uint32_t SmartIO__SendBytes(unsigned char *sendbuf, int sendlen)
    {
    uint16_t op = FETCH_WORD(sendbuf, 0);
    int is_syscall = (sendbuf[1] == 0xFF);
    SPI_State = SPI_TRANSMITTING;

    SmartIO_SPI_SendBytes(sendbuf, sendlen);

    while (1)
    {
    	if (SPI_State == SPI_SMARTIO_REPLY)
    	{
    		break;
    	}
    }

    unsigned char *replybuf = &host_sram_block[0];
    int replylen = SmartIO_SPI_ReadBytes(replybuf, HOST_SRAM_POOL_SIZE);

    if (replylen < 0)
        {
        SmartIO_Error(SMARTIO_ERROR_BUFFER_TOOSMALL);
        return 0;
        }

    uint32_t handle = FETCH_WORD(replybuf, 0);

    if (is_syscall)
        {
        if (op >= __SMARTIO_SYS_MULTIBYTE_RETURN)
            handle = (uint32_t)&replybuf[0];
        else if (replylen == 4)
            handle |= FETCH_WORD(replybuf, 2) << 16;
        }
    else if (replylen != 2 && replylen != 4)
        SmartIO_Error(SMARTIO_ERROR_BAD_RETURN_HANDLE);
    else if (replylen == 4)
        {
        uint16_t handle2 = FETCH_WORD(replybuf, 2);

        if (handle2 != handle+1)
            SmartIO_Error(SMARTIO_ERROR_INVALID_HANDLE2);
        }

#if 0
    printf("Received: [");
    for (int i = 0; i < replylen; i++)
        printf("%2X ", replybuf[i]);
    printf("]\n");
#endif // 0
    SPI_State = SPI_IDLE;
    SmartIO_SPI_FinishRead();

    return handle;
    }

void SmartIO__RegisterCallback(uint16_t handle, void (*func)(), int argtype)
    {
    CALLBACK_ENTRY *p = FindCallbackEntry(handle);

    if (p == 0)
        {
        if (callback_table_index == CALLBACK_TABLE_SIZE)
            {
            SmartIO_Error(SMARTIO_ERROR_CALLBACK_TABLE_TOO_SMALL);
            return;
            }

        p = &callback_table[callback_table_index++];
        }
    p->handle = handle;
    p->func = func;
    p->argtype = argtype;
    }

static CALLBACK_ENTRY *FindCallbackEntry(uint16_t handle)
    {
    CALLBACK_ENTRY *p = &callback_table[0];

    for (int i = 0; i < callback_table_index; i++)
        {
        if (p->handle == handle)
            return p;
        p++;
        }
    return 0;
    }

void SmartIO_ProcessUserInput(void)
    {
    unsigned char *replybuf = &host_sram_block[0];
    int replylen = SmartIO_SPI_ReadBytes(host_sram_block, HOST_SRAM_POOL_SIZE);
    SmartIO_SPI_FinishRead();
    SPI_State = SPI_IDLE;

    if (replylen > 0)
        {
        uint16_t handle = FETCH_WORD(replybuf, 0);

        if (handle == 0)
            {
            if (replylen >= 4)
                {
                uint16_t opc = FETCH_WORD(replybuf, 2);

                switch (opc)
                    {
                case SMARTIO_HOST_CONNECT:
                    if (SmartIO_Connect_Callback)
                        SmartIO_Connect_Callback();
                    break;
                case SMARTIO_HOST_DISCONNECT:
                    if (SmartIO_Disconnect_Callback)
                        SmartIO_Disconnect_Callback();
                    break;
                    }
                }
            return;
            }
        CALLBACK_ENTRY *p = FindCallbackEntry(handle);
        if (p == 0)
            SmartIO_Error(SMARTIO_ERROR_CALLBACK_HANDLE_NOT_FOUND, handle);
        else
            {
            switch (p->argtype)
                {
            case SMARTIO_ARG_NONE:
                p->func();
                break;
            case SMARTIO_ARG_STRING:
                replybuf[replylen] = 0;
                p->func(&replybuf[2]);
                break;
            case SMARTIO_ARG_ONE:
                p->func(*(uint16_t *)&replybuf[2]);
                break;
            case SMARTIO_ARG_TWO:
                p->func(*(uint16_t *)(&replybuf[2]), *(uint16_t *)(&replybuf[4]));
                break;
            case SMARTIO_ARG_THREE:
                p->func(*(uint16_t *)(&replybuf[2]), *(uint16_t *)(&replybuf[4]), *(uint16_t *)(&replybuf[6]));
                break;
            case SMARTIO_ARG_FOUR:
                p->func(*(uint16_t *)(&replybuf[2]), *(uint16_t *)(&replybuf[4]), *(uint16_t *)(&replybuf[6]), *(uint16_t *)(&replybuf[8]));
                }
            }
        }
    else if (replylen == 0)
        SmartIO_Error(SMARTIO_ERROR_BAD_ASYNC_HANDLE);
    else if (replylen < 0)
        SmartIO_Error(SMARTIO_ERROR_BUFFER_TOOSMALL);
    }
