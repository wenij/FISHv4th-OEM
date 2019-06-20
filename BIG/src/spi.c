/**
 ******************************************************************************
 * File Name          : SPI.c
 * Description        : This file provides code for the configuration
 *                      of the SPI instances.
 ******************************************************************************
 ** This notice applies to any and all portions of this file
 * that are not between comment pairs USER CODE BEGIN and
 * USER CODE END. Other portions of this file, whether
 * inserted by the user or by software development tools
 * are owned by their respective copyright owners.
 *
 * COPYRIGHT(c) 2018 STMicroelectronics
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"
#include "spi.h"
#include "tim.h"
#include <string.h>

#include "smartio_if.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

SPI_HandleTypeDef* ActiveSPI;

SPI_HandleTypeDef hspi1;



/* SPI1 init function */
void MX_SPI1_Init(void)
{
    ActiveSPI = &hspi1;

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_CR1_CPHA;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK)
    {
        _Error_Handler(__FILE__, __LINE__);
    }

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* spiHandle)
{

    GPIO_InitTypeDef GPIO_InitStruct;
    if(spiHandle->Instance==SPI1)
    {
        /* USER CODE BEGIN SPI1_MspInit 0 */

        /* USER CODE END SPI1_MspInit 0 */
        /* SPI1 clock enable */
        __HAL_RCC_SPI1_CLK_ENABLE();

        /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI 
         */
        GPIO_InitStruct.Pin = SPI1_SCK_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

#ifdef CARSTENS_BOARD
        // MISO - rerouted to PA # 6
        GPIO_InitStruct.Pin = GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
#else
        GPIO_InitStruct.Pin = SPI1_MISO_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
#endif
        GPIO_InitStruct.Pin = SPI1_MOSI_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* USER CODE BEGIN SPI1_MspInit 1 */

        /* USER CODE END SPI1_MspInit 1 */
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* spiHandle)
{

    if(spiHandle->Instance==SPI1)
    {
        /* USER CODE BEGIN SPI1_MspDeInit 0 */

        /* USER CODE END SPI1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_SPI1_CLK_DISABLE();

        /**SPI1 GPIO Configuration
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI 
         */
        HAL_GPIO_DeInit(GPIOB, SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MOSI_Pin);

        /* USER CODE BEGIN SPI1_MspDeInit 1 */

        /* USER CODE END SPI1_MspDeInit 1 */
    }
} 


/* USER CODE BEGIN 1 */

QueueHandle_t SpiSendQueue = NULL;
QueueHandle_t SpiSmartIoQueue = NULL;


static bool DirectSPI = false;

static void SetSpiIntMode(bool direct)
{
    DirectSPI = direct;
}

bool GetSpiIntMode(void)
{
    return(DirectSPI);
}


void SPI_driver_task( void * params )
{
    Message_t PortMsg;
    bool TxError = false;
    bool RxError = false;
    MessageType Command;

    while(1)
    {
        xQueueReceive( SpiSendQueue, (void*)&PortMsg, portMAX_DELAY );

        Command = PortMsg.Type;

        // Process the send command
        switch (Command)
        {
        case SPI_SEND_MESSAGE:
        {
            SpiMsgContainer* Msg = (SpiMsgContainer*)PortMsg.data;

            HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_RESET);	// spi1.ChipSelect();

            if (Msg->length > 0)
            {
                HAL_SPI_Transmit(ActiveSPI, (uint8_t*)&Msg->length, 2, HAL_MAX_DELAY);    // spi1.Write(sendlen); spi1.Write(sendlen >> 8);
                HAL_SPI_Transmit(ActiveSPI, Msg->data, Msg->length, HAL_MAX_DELAY);    //spi1.WriteByteArray(0, sendbuf, sendlen);
            }

            if (Msg->Response)
            {
                // Read
                //if (Msg->rx_length == 0)
                {
                    uint8_t buf[2] = {0xFF, 0xFF};
                    // Read first two bytes to get the length
                    HAL_SPI_Receive( ActiveSPI, buf, 2, 100);	//int low = spi1.Read();
                    Msg->rx_length = (uint16_t)(buf[1] << 8) + (uint16_t)buf[0];
                }

                if (Msg->rx_length > 0)
                {
                    if ((Msg->data != NULL) && (Msg->buf_size < Msg->rx_length))
                    {
                        vPortFree(Msg->data);   // Data buffer too small
                        Msg->data = NULL;
                    }

                    if (Msg->data == NULL)
                    {
                        Msg->data = pvPortMalloc(Msg->rx_length);
                        Msg->buf_size = Msg->rx_length;
                    }

                    HAL_SPI_Receive( ActiveSPI, Msg->data, Msg->rx_length, 100);	//int low = spi1.Read();

                }
                else
                {
                    RxError = true;
                }
            }
            else
            {
                Msg->rx_length = 0;
            }
            HAL_GPIO_WritePin(BT_CSn_GPIO_Port, BT_CSn_Pin, GPIO_PIN_SET);	// spi1.ChipSelect();

            // Send the response
            Msg->TxError = TxError;
            Msg->RxError = RxError;
            PortMsg.Type = SPI_READ_MESSAGE;

            xQueueSend( SpiSmartIoQueue, &PortMsg, 0U );

        }
        break;
        case SPI_DAC_IO_MESSAGE:
        {
            SpiMsgContainer* Msg = (SpiMsgContainer*)PortMsg.data;

            if (Msg->length > 0)
            {
                TxError = !SPI_MsgDAC(Msg->length, Msg->data);
            }

            Msg->TxError = TxError;
            Msg->RxError = false;

            xQueueSend( DAC_Queue, &PortMsg, 0U );

            break;
        }
        case SPI_ADC_IO_MESSAGE:
        {
            SpiMsgContainer* Msg = (SpiMsgContainer*)PortMsg.data;
            uint16_t rxlen = 0;
            uint8_t * rxp = NULL;

            if (Msg->Response)
            {
                // Ensure we have memory for RX
                if (Msg->rx_length > 0)
                {
                    rxp = pvPortMalloc(Msg->rx_length);
                    rxlen = Msg->rx_length;
                }
            }

            TxError = RxError = (SPI_MsgADC( Msg->length, Msg->data, rxp, rxlen, Msg->Response) < 0);

            if (Msg->Response)
            {
                if (Msg->data != NULL)
                {
                    vPortFree(Msg->data);
                }
                Msg->data = rxp;
                Msg->rx_length = rxlen;
            }


            // Send the response
            Msg->TxError = TxError;
            Msg->RxError = RxError;
            PortMsg.Type = SPI_READ_MESSAGE;

            xQueueSend( ADC_Queue, &PortMsg, 0U );

            break;
        }

        case SPI_INT_MODE_SET:
            {
                bool mode = (bool)PortMsg.data;

                SetSpiIntMode(mode);

                xQueueSendToFront( pstat_Queue, &PortMsg, 0U );
            }
            break;

        default:
            {
                SpiMsgContainer* Msg = (SpiMsgContainer*)PortMsg.data;

                vPortFree(Msg->data);
                vPortFree(Msg);
                break;
            }
        }

    }


}


int SPI_MsgADC( uint16_t TxLength, uint8_t * TxData, uint8_t * RxData, uint16_t RxLength, bool Response)
{
    int ret = -1;

    HAL_GPIO_WritePin(ADC_CSn_GPIO_Port, ADC_CSn_Pin, GPIO_PIN_RESET);    // ADC Chip select

    if (TxLength > 0)
    {
        HAL_SPI_Transmit(ActiveSPI, TxData,TxLength, HAL_MAX_DELAY);
    }

    //The command and data read should be spaced t6 (50 ADC clocks, min = 6.6 us).
    TimDelayMicroSeconds(8);

    if (Response)
    {
        // Read

        if ( (RxLength > 0) && (RxData != NULL))
        {

            if (HAL_OK == HAL_SPI_Receive( ActiveSPI, RxData, RxLength, 100))
            {
                ret = RxLength;
            }
        }
    }
    else
    {
        ret = 0;
    }

    TimDelayMicroSeconds(3);    // t10: 1.1 us, 8 clk cycles

    HAL_GPIO_WritePin(ADC_CSn_GPIO_Port, ADC_CSn_Pin, GPIO_PIN_SET);    // ADC Chip select

    return(ret);
}

bool SPI_MsgDAC( uint16_t TxLength, uint8_t * TxData)
{
    bool ret;

    HAL_GPIO_WritePin(DAC_CSn_GPIO_Port, DAC_CSn_Pin, GPIO_PIN_RESET);    // DAC Chip select

    ret = (HAL_OK == HAL_SPI_Transmit(ActiveSPI, TxData, TxLength, HAL_MAX_DELAY));

    HAL_GPIO_WritePin(DAC_CSn_GPIO_Port, DAC_CSn_Pin, GPIO_PIN_SET);  // DAC Chip select

    return(ret);
}


void SPI_SendData(MessageType Id, uint16_t length,  uint16_t reply_length, uint8_t * data, bool Response)
{
    Message_t msg;
    SpiMsgContainer *msg_p;

    if ( Response || (length > 0))   // Length should only be 0 if getting a response aka a Read
    {
        msg.data = pvPortMalloc( sizeof(SpiMsgContainer));
        msg.Type = Id;
        msg_p = (SpiMsgContainer*)msg.data;
        msg_p->length = length;
        if (reply_length > length)
        {
            msg_p->buf_size = reply_length;
        }
        else
        {
            msg_p->buf_size = length;
        }

        if (length > 0)
        {
            msg_p->data = pvPortMalloc(msg_p->buf_size);

            configASSERT(msg_p->data != NULL);

            memcpy(msg_p->data, data, length);
        }
        else
        {
            msg_p->data = NULL;
        }

        msg_p->rx_length = reply_length;

        msg_p->Response = Response;

        xQueueSend( SpiSendQueue, &msg, 0 );
    }
}

void SPI_SendDataNoResponse(MessageType Id, uint16_t length, uint8_t * data)
{
    SPI_SendData(Id, length, 0, data, false);
}


void SPI_ReadData(MessageType Id, uint8_t reply_length,  uint8_t * data)
{
    SPI_SendData(Id, 0, reply_length, data, true);
}

void SPI_SetIrqMode( bool mode)
{
    Message_t msg;

    msg.Type = SPI_INT_MODE_SET;
    msg.data = (uint8_t*)mode;

    xQueueSendToBack( SpiSendQueue, &msg, 0 );
}

void SPI_SetIrqModeFromISR( bool mode)
{
    Message_t msg;

    msg.Type = SPI_INT_MODE_SET;
    msg.data = (uint8_t*)mode;

    xQueueSendToBackFromISR( SpiSendQueue, &msg, 0 );
}
/* USER CODE END 1 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
