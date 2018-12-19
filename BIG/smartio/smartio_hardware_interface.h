/* Copyright 2017 ImageCraft Creations Inc., All rights reserved.
 * Smart.IO Host Interface Layer
 * https://imagecraft.com/smartio/
 */
#pragma once

// Smart.IO Max 1MHz, do not change
#define SPI_MHZ             1000000

void SmartIO_SPI_SendBytes(unsigned char *sendbuf, int sendlen);
int  SmartIO_SPI_ReadBytes(unsigned char *replybuf, int buflen);
void SmartIO_HardwareInit(void (*IRQ_ISR)(void));
void SmartIO_SPI_FinishRead(void);
void SmartIO_Error(int n, ...);
