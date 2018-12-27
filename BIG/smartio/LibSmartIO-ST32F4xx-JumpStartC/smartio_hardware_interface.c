/* Copyright 2017 ImageCraft Creations Inc., All rights reserved.
 * Smart.IO Host Interface Layer
 * https://imagecraft.com/smartio/
 */
//#include <stdio.h>
#include <stdarg.h>
#ifdef STM32F205xx
#include "stm32f2xx_hal_def.h"
#include "stm32f2xx_hal_gpio.h"
#include "stm32f2xx_hal_spi.h"
#include "stm32f2xx_hal.h"
#else
#include "stm32f4xx_hal_def.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal.h"
#endif
#include <spi.h>
#include "main.h"
#include "smartio_interface.h"
#include "smartio_hardware_interface.h"
#include "isr.h"

#include <string.h>

/* ST-Nucleo F411 Smart.IO pin assignments
 * SPI
 *  PORT  PIN AF
 *  porta, 5, 5,       // SCK
 *  porta, 7, 5,       // MOSI
 *  porta, 6, 5,       // MISO
 *  portb, 6,          // CS
 *
 * HOST interrupt
 *  PA9
 *
 * Smart.IO RESET
 *  PC7
 */


/* Hardware Smart.IO
 */



void SmartIO_HardReset(void)
{
	HAL_GPIO_WritePin(BT_RST_GPIO_Port, BT_RST_Pin, GPIO_PIN_RESET);  //portc.Clear(7);
	HAL_Delay(100);	//DelayMilliSecs(10);
	HAL_GPIO_WritePin(BT_RST_GPIO_Port, BT_RST_Pin, GPIO_PIN_SET);	    //portc.Set(7);
}

/* send bytes out using the SPI port
 *  sendbuf: buffer containing content to send
 *  sendlen: number of bytes to send
 */
void SmartIO_SPI_SendBytes(unsigned char *sendbuf, int sendlen)
{
	HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_RESET);	// spi1.ChipSelect();
	HAL_SPI_Transmit(ActiveSPI, (uint8_t*)&sendlen, 2, HAL_MAX_DELAY);    // spi1.Write(sendlen); spi1.Write(sendlen >> 8);
	HAL_SPI_Transmit(ActiveSPI, sendbuf, sendlen, HAL_MAX_DELAY);    //spi1.WriteByteArray(0, sendbuf, sendlen);
    HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_SET); //spi1.ChipDeselect();
}

/* read bytes from the SPI port
 *  replybuf: buffer to fill the content with
 *  buflen: length of "replybuf"
 */
int SmartIO_SPI_ReadBytes(unsigned char *replybuf, int buflen)
{
	HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_RESET);	// spi1.ChipSelect();
	int replylen;

#if 0

	memset(replybuf, 0xff, 6);
	HAL_SPI_Receive( ActiveSPI, replybuf, 6, HAL_MAX_DELAY);	//int low = spi1.Read();

	replylen = (int)replybuf[0] | (int)(replybuf[1] << 8);	// int replylen = (spi1.Read() << 8) | low;
	{
		int i;

		for (i=2; i<6; i++)
		{
			replybuf[i-2] = replybuf[i];
		}
	}
#endif

	memset(replybuf, 0xff, 2);

	HAL_SPI_Receive( ActiveSPI, replybuf, 2, 100);	//int low = spi1.Read();

	replylen = (int)replybuf[0] | (int)(replybuf[1] << 8);	// int replylen = (spi1.Read() << 8) | low;

    // printf("reading %d bytes from Smart.IO\n", replylen);

    if (replylen > buflen)
        replylen = -1;
    else
    {
    	memset(replybuf, 0xff, replylen);

    	HAL_SPI_Receive( ActiveSPI, replybuf, replylen, 100);	//spi1.ReadBytes(replybuf, replylen);

    }

    HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_SET); //spi1.ChipDeselect();

    return replylen;
}


/* Initialize the hardware pins connected to Smart.IO
 */
void SmartIO_HardwareInit(void (*IRQ_ISR)(void))
{
	/* All the init code is done in the STM framework. All we have left to do is to reset.
    // Initialize the SPI
    spi1.SetPins(
                &porta, 5, 5,       // SCK
                &porta, 7, 5,       // MOSI
                &porta, 6, 5,       // MISO
                &portb, 6,          // CS
                true                // active low
                );

    // CPOL - 0, CPHA - 1, 1Mhz clock (fastest for Smart.IO SPI)
    spi1.MakeSPI(8, 0b01, SPI_MHZ);
    spi1.Enable();

    // Smart.IO "SPI IRQ" input, ACTIVE HIGH
    porta.MakeInput(HOST_INTR_PIN);

    // Smart.IO will pull the line HIGH when it transmits.
    EXTI_PORT.MakeEXTI(&porta, PUPDR_NONE, EXTI_RISING_EDGE, 0, IRQ_ISR);

    // RESET pin
    portc.MakeOutput(7, OSPEED_LOW);
    */
	BT_ISR_ptr = IRQ_ISR;

    SmartIO_HardReset();

}


/* Wait until the Host INTR pin to go low as part of the read protocol
 */
void SmartIO_SPI_FinishRead(void)
{
	while( HAL_GPIO_ReadPin(BT_IRQ_GPIO_Port, BT_IRQ_Pin) != 0 )
		;
   // while ((porta.Read() & BIT(HOST_INTR_PIN)) != 0)
   //     ;
}

/* Handle error conditions
 */
void SmartIO_Error(int n, ...)
    {
    va_list args;
    unsigned h;

    va_start(args, n);

    switch (n)
        {
    case SMARTIO_ERROR_BADRETURN:
        printf("SMARTIO_ERROR_BADRETURN\n");
        break;
    case SMARTIO_ERROR_BAD_RETURN_HANDLE:
        printf("SMARTIO_ERROR_BAD_RETURN_HANDLE\n");
        break;
    case SMARTIO_ERROR_BAD_ASYNC_HANDLE:
        printf("SMARTIO_ERROR_BAD_ASYNC_HANDLE\n");
        break;
    case SMARTIO_ERROR_CALLBACK_TABLE_TOO_SMALL:
        printf("SMARTIO_ERROR_CALLBACK_TABLE_TOO_SMALL\n");
        break;
    case SMARTIO_ERROR_CALLBACK_HANDLE_NOT_FOUND:
        h = va_arg(args, unsigned);
        printf("SMARTIO_ERROR_CALLBACK_HANDLE_NOT_FOUND handle %d\n", h);
        break;
    case SMARTIO_ERROR_BUFFER_TOOSMALL:
        printf("SMARTIO_ERROR_BUFFER_TOOSMALL\n");
        break;
    case SMARTIO_ERROR_INVALID_HANDLE2:
        printf("SMARTIO_ERROR_INVALID_HANDLE2\n");
        break;
    case SMARTIO_ERROR_COMMAND_BUFFER_OVERFLOW:
        printf("SMARTIO_ERROR_COMMAND_BUFFER_OVERFLOW\n");
        }
    }
