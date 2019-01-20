/*
 * adc_ads1256.c
 *
 *  Created on: Jan 16, 2019
 *      Author: carstenh
 */
#ifdef STM32F205xx
#include "stm32f2xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif
#include "adc_ads1256.h"

#include "spi.h"
/*
 * Local Functions and type definitions
 */

static uint16_t SendReceiveSPI( int Address, uint8_t * TxData, uint16_t TxLength, uint8_t **RxData, uint16_t RxLength);
static bool WaitForDataReady(GPIO_PinState value);
static void StartConversion(void);

 /*
  * Driver Functions for A/D Converter ADS 1256
  */

 /*
  * PowerUpInit
  *
  * Reset and power up the ADC
  * Parameters:
  *    None
  *
  * Return Value:
  *    None
  */
 void ads1256_PowerUpInit( void )
 {
     // Reset the ADC by pulsing the Reset Line.
     HAL_GPIO_WritePin(ADC_RSTn_GPIO_Port, ADC_RSTn_Pin, GPIO_PIN_RESET);    // ADC Reset Pin

     // Short delay 2 ms
     vTaskDelay(  pdMS_TO_TICKS(2) );

     HAL_GPIO_WritePin(ADC_RSTn_GPIO_Port, ADC_RSTn_Pin, GPIO_PIN_SET);    // ADC Reset Pin

 }


 /*
  * Standby
  *
  * Place ADC in standby mode
  * Parameters:
  *    None
  *
  * Return Value:
  *    None
  */
 void ads1256_Standby( void )
 {
     // TBD

 }

 /*
  * SetPGA
  *
  * Set path gain
  * Parameters:
  *    Gain
  *
  * Return Value:
  *    None
  */

 void ads1256_SetPGA( ads1256_pga_t gain )
 {
     // TBD
 }

 /*
  * ReadChannel
  *
  * Read an A/D channel. The function will set the channel, wait for a conversion, and read the result back.
  * A number of averages can be requested. That many values will be read back and averaged.
  * Parameters:
  *     Channel
  *     Pos (true is the positive input select, false is negative input select)
  *     Averages
  *
  * Return Value:
  *    None
  */
int32_t ads1256_ReadChannel( ads1256_channel_t channel, bool pos, uint16_t averages)
{
    bool Continue = false;
    uint8_t buf;

    // Make sure DRDY is high
    Continue = WaitForDataReady(1);

    if (!Continue)
    {
        // Reset and try again if drdy wasn't high
    }

    if (!Continue)
    {
        Return(0);
    }

    // Configure the appropriate channel and polarity. The opposing channel remains unchanged.
    buf = 0;
    if (pos)
    {
        buf = set_ADS1256_MUX_PSEL(channel);
    }
    else
    {
        buf = set_ADS1256_MUX_NSEL(channel);
    }


    // Start conversion.

    // Wait for DRDY to go low.

    // Read data.

    return(0);
}


 /*
  * ReadChannelPair
  *
  * Read an A/D channel Pair. Typical for differential measurements.
  * The function will set the channels, wait for a conversion, and read the result back.
  * A number of averages can be requested. That many values will be read back and averaged.
  * Parameters:
  *     Channel 1 - First Channel
  *     Pos 1 (true is the positive input select, false is negative input select) for first channel
  *     Channel 2 - Second Channel
  *     Pos 2 (true is the positive input select, false is negative input select) for second channel
  *     Averages
  *     Samples - Return buffer array with length 2 containing both measurements (channel1, channel2)
  *
  * Return Value:
  *     Result.
  */
int32_t ads1256_ReadChannelPair( ads1256_channel_t channel1, bool pos1, ads1256_channel_t channel2, bool pos2, uint16_t averages)
{
    return(0);
}

 /*
  * SelfTest
  *
  * Perform Self Test (TBD what exactly that is)
  * Parameters:
  *    None
  *
  * Return Value:
  *    true if test passes
  *    false if test failed
  */
 bool ads1256_SelfTest( void )
 {
     return(false);
 }

 /****************************************
  *   Local Functions
  ****************************************/

 /*
  * SendReceiveSPI
  *
  * Send and receive interaction with SPI driver
  * Parameters:
  *    Address - address of target register
  *    TxData - Pointer to buffer with transmit data content (will be copied by function)
  *    TxLength - number of bytes to transmit
  *    RxData - Pointer to received data (dynamically allocated buffer). Null if no data received.
  *    RxLength - length of data expected
  *
  *
  * Return Value:
  *    Number of bytes received
  */
 uint16_t SendReceiveSPI( int Address, uint8_t * TxData, uint16_t TxLength, uint8_t **RxData, uint16_t RxLength)
 {
     uint16_t ret = 0;
     Message_t Msg;
     SpiMsgContainer * SpiMsg;

     SPI_SendData( SPI_ADC_IO_MESSAGE, TxLength, RxLength, TxData, (RxLength > 0));

     if (xQueueReceive( ADC_Queue, (void*)&Msg, 100 ))
     {
         SpiMsg = (SpiMsgContainer*)Msg.data;

         if (SpiMsg->rx_length > 0)
         {
             *RxData = SpiMsg->data;
             ret = RxLength;
         }
         else
         {
             *RxData = NULL;
         }

         vPortFree(SpiMsg);
     }

     return(ret);
 }

 /*
  * WaitForDataReady
  *
  * Waits for data to become ready
  * Parameters:
  *    value Expected target value for drdy
  *
  * Return Value:
  *    true if success
  *    false if function timed out (10 ms timout)
  */
 bool WaitForDataReady(GPIO_PinState value)
 {
     bool ret = false;
     GPIO_PinState drdy;

     drdy = HAL_GPIO_ReadPin(ADC_DRDYn_GPIO_Port, ADC_DRDYn_Pin);    // ADC DRDY
     if (drdy != value)
     {
         int i;
         int tmo = 0;
         while (tmo < 10)
         {
             for (i=0; i<256; i++)
             {
                 drdy = HAL_GPIO_ReadPin(ADC_DRDYn_GPIO_Port, ADC_DRDYn_Pin);    // ADC DRDY
                 if (drdy == value)
                 {
                     return(true);
                 }
             }

             vTaskDelay(  pdMS_TO_TICKS(2) );   // Allow others to run
             tmo += 2;
         }

     }

     return(false);
 }

 /*
  * StartConversion
  *
  * Start Conversion by asserting SYNC signal
  * Parameters:
  *    None
  *
  * Return Value:
  *    None
  */
 static void StartConversion(void)
 {
     volatile GPIO_PinState drdy;
     int i;

     HAL_GPIO_WritePin(ADC_PDN_GPIO_Port, ADC_PDN_Pin, GPIO_PIN_RESET);    // ADC SYNC/PDN

     // Should be over 500 ns in length. Timing diagram indicates the
     for (i=0; i<512; i++)
     {
         drdy = HAL_GPIO_ReadPin(ADC_DRDYn_GPIO_Port, ADC_DRDYn_Pin);    // ADC SYNC/PDN
     }

     HAL_GPIO_WritePin(ADC_PDN_GPIO_Port, ADC_PDN_Pin, GPIO_PIN_SET);    // ADC SYNC/PDN
 }

