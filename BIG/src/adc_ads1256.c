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
#include <string.h>

#include "spi.h"
#include "tim.h"
/*
 * Local Functions and type definitions
 */

static uint16_t SendReceiveSPI( uint8_t * TxData, uint16_t TxLength, uint8_t *RxData, uint16_t RxLength);
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
  *    Input buffer enabled
  *
  * Return Value:
  *    None
  */
 void ads1256_PowerUpInit( bool buffer_enable )
 {
     uint8_t buf[3];
     uint8_t rxbuf[4];

     // Reset the ADC by pulsing the Reset Line.
     HAL_GPIO_WritePin(ADC_RSTn_GPIO_Port, ADC_RSTn_Pin, GPIO_PIN_RESET);    // ADC Reset Pin

     // Short delay 4 us
     TimDelayMicroSeconds( 4 );

     HAL_GPIO_WritePin(ADC_RSTn_GPIO_Port, ADC_RSTn_Pin, GPIO_PIN_SET);    // ADC Reset Pin

     // Enable input buffer
     buf[0] = ADS1256_BUILD_WRITE_REG_CMD(ADS1256_STATUS_REGISTER);
     buf[1] = 0;  // Number of registers - 1
     buf[2] = buffer_enable ? ADS1256_STATUS_BUFEN : 0;

     SendReceiveSPI(buf, 3, rxbuf, 0);   // Send status register expecting no response.

     // Set PGA to 2
     ads1256_SetPGA(ADS1256_PGA_2);


     // Set Sample rate to 1000 SPS. Reduces noise vs 30 kSPS which is the default. It's painfully slow...

     buf[0] = ADS1256_BUILD_WRITE_REG_CMD(ADS1256_DRATE_REGISTER);
     buf[1] = 0;  // Number of registers - 1
     buf[2] = set_ADS1256_DRATE(ADS1256_DRATE_1000);   // 1000 SPS

     // Leaving it at 30 kSPS
     //SendReceiveSPI(buf, 3, rxbuf, 0);   // Send data rate register expecting no response.


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
     uint8_t buf[3];
     uint8_t rxbuf[4];

     buf[0] = ADS1256_BUILD_WRITE_REG_CMD(ADS1256_ADCON_REGISTER);
     buf[1] = 0;  // Number of registers - 1
     buf[2] = set_ADS1256_PGA(1);   // PGA=2

     SendReceiveSPI(buf, 3, rxbuf, 0);   // Send ad control register expecting no response.
 }

 /*
  * ReadChannel
  *
  * Read an A/D channel. The function will set the channel, wait for a conversion, and read the result back.
  * A number of averages can be requested. That many values will be read back and averaged.
  * Parameters:
  *     Positive Channel
  *     Negative Channel
  *     Averages
  *
  * Return Value:
  *    Measured Value
  */
int32_t ads1256_ReadChannel( ads1256_channel_t Pchannel, ads1256_channel_t Nchannel, uint16_t averages)
{
    bool Continue = false;
    int32_t ret = 0;
    uint8_t buf[3];
    uint8_t rxbuf[4];

    // Initialize
    // Make sure DRDY is high
    Continue = WaitForDataReady(1);

    if (!Continue)
    {
        // Reset and try again if drdy wasn't high
    }

    if (!Continue)
    {
        return(0);
    }

    // Configure the appropriate channel
    buf[0] = ADS1256_BUILD_WRITE_REG_CMD(ADS1256_MUX_REGISTER);
    buf[1] = 0;  // Number of registers - 1
    buf[2] = set_ADS1256_MUX_PSEL(Pchannel) | set_ADS1256_MUX_NSEL(Nchannel);
    //buf[2] = set_ADS1256_MUX_PSEL(ADS1256_CHANNEL_AINCOM) | set_ADS1256_MUX_NSEL(Pchannel);

    SendReceiveSPI(buf, 3, rxbuf, 0);   // Send mux select expecting no response.

    // Start conversion.
    StartConversion();

    // Wait for DRDY to go low.
    Continue = WaitForDataReady(0);

    // Read data.
    if (Continue)
    {
        uint32_t temp;
        int rxlen;

        buf[0] = ADS1256_READ_DATA;

        rxlen = SendReceiveSPI(buf, 1, rxbuf, 3);   // Send Read Data expecting a 3 byte response.

        if (rxlen == 3)
        {
            // The extra byte shift gets the sign bit in the right position
            temp = ( ((uint32_t)rxbuf[0]) << 24) | ( ((uint32_t)rxbuf[1]) << 16) | ((uint32_t)(rxbuf[0]) << 8);

            ret = (int32_t)temp / 256;   // takes care of sign extension

        }
    }

    return(ret);
}


/*
 * ads1256_Cal
 *
 * Perform Calibration
 * Parameters:
 *    Offset or Gain
 *
 * Return Value:
 *    true if calibration succeeds
 *    false if calibration failed
 */
bool ads1256_Cal( ads1256_cal_type_t calType)
{
    bool ret;
    uint8_t buf[3];
    uint8_t rxbuf[4];

    ret = WaitForDataReady(1);

    if (ret)
    {
        // Configure the appropriate channel
        buf[0] = ADS1256_BUILD_WRITE_REG_CMD(ADS1256_MUX_REGISTER);
        buf[1] = 0;  // Number of registers - 1
        buf[2] = set_ADS1256_MUX_PSEL(ADS1256_CHANNEL_AINCOM) | set_ADS1256_MUX_NSEL(ADS1256_CHANNEL_0);

        SendReceiveSPI(buf, 3, rxbuf, 0);   // Send ad control register expecting no response.

        // Issue the calibration command
        if (calType == CAL_OFFSET)
        {
            buf[0] = ADS1256_SELFCAL_SYSTEMOFFSET;
        }
        else
        {
            buf[0] = ADS1256_SELFCAL_SYSTEMGAIN;
        }
        SendReceiveSPI(buf, 1, rxbuf, 0);   // Send Cal command expecting no response.


        TimDelayMicroSeconds(200);

        ret = WaitForDataReady(10);

    }

    return(ret);

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
 uint16_t SendReceiveSPI( uint8_t * TxData, uint16_t TxLength, uint8_t *RxData, uint16_t RxLength)
 {
     uint16_t ret = 0;

     if (!GetSpiIntMode())
     {
         Message_t Msg;
         SpiMsgContainer * SpiMsg;

         SPI_SendData( SPI_ADC_IO_MESSAGE, TxLength, RxLength, TxData, (RxLength > 0));

         if (xQueueReceive( ADC_Queue, (void*)&Msg, 100 ))
         {
             SpiMsg = (SpiMsgContainer*)Msg.data;

             if (SpiMsg->rx_length > 0)
             {
                 ret = RxLength;
                 if (SpiMsg->rx_length < RxLength)
                 {
                     ret = SpiMsg->rx_length;
                 }
                 memcpy( RxData, SpiMsg->data, ret);

             }

             if (SpiMsg->data != NULL)
             {
                 vPortFree(SpiMsg->data);
             }
             vPortFree(SpiMsg);
         }
     }
     else
     {
         ret = SPI_MsgADC( TxLength, TxData, RxData, RxLength, (RxLength > 0));

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
     volatile int tmo = 100000;
     bool ret = true;

     while ( HAL_GPIO_ReadPin(ADC_DRDYn_GPIO_Port, ADC_DRDYn_Pin) != value)
     {
         tmo -= 1;
         if (tmo < 0)
         {
             ret = false;
             break;
         }
     }

     return(ret);
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

     uint8_t buf[3];
     uint8_t rxbuf[4];

     // Send SYNC and WAKEUP
     buf[0] = ADS1256_BUILD_WRITE_REG_CMD(ADS1256_SYNC_AD_CONVERSION);
     SendReceiveSPI(buf, 1, rxbuf, 0);   // SYNC Command

     //TimDelayMicroSeconds(5);  // Gap T11, 24 clocks, before next command

     buf[0] = ADS1256_BUILD_WRITE_REG_CMD(ADS1256_WAKEUP);
     SendReceiveSPI(buf, 1, rxbuf, 0);   // WAKEUP Command

     //TimDelayMicroSeconds(5);  // Gap T11, 24 clocks, before next command

 }

