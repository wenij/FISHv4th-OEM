/*
 * dac_ad5662.c
 *
 *  Created on: Jan 16, 2019
 *      Author: carstenh
 */


#include "dac_ad5662.h"
#include "main.h"
#include "spi.h"

typedef enum
{
    AD5662_NORMAL_OPERATION = 0,
    AD5662_lPM_1_KO = 1,
    AD5662_LPM_100_KO = 2,
    AD5662_LPM_TRI_STATE = 3
} AD5662_MODE;

static bool SendSPI(AD5662_MODE mode, uint16_t dac);

bool AD5662_Set(uint16_t DacValue)
{
    bool ret = true;

    ret = SendSPI(AD5662_NORMAL_OPERATION, DacValue);

    return(ret);
}


bool SendSPI(AD5662_MODE mode, uint16_t dac)
 {
     bool ret = false;
     Message_t Msg;

     uint8_t TxData[3];

     TxData[0] = (uint8_t)mode;
     TxData[1] = (uint8_t)(dac >> 8);
     TxData[2] = (uint8_t)dac;

     // Message format is 24 bits. 6 don't care, 2 mode bits, 16 dac bits.

     SPI_SendDataNoResponse( SPI_DAC_IO_MESSAGE, 3, TxData);

     if (xQueueReceive( DAC_Queue, (void*)&Msg, 100 ))
     {
         ret= true;
     }

     return(ret);
 }
