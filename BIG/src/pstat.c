/*
 * pstat.c
 *
 * Potentiostat task
 *
 *  Created on: Jan 19, 2019
 *      Author: carstenh
 */

#ifdef STM32F205xx
#include "stm32f2xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif

#include <string.h>

#include "gpio.h"
#include "main.h"
#include "cli.h"
#include "spi.h"
#include "pstat.h"
#include "adc_ads1256.h"
#include "dac_ad5662.h"


QueueHandle_t ADC_Queue = NULL;
QueueHandle_t DAC_Queue = NULL;

QueueHandle_t pstat_Queue = NULL;

static bool MakeMeasurement( uint16_t DACvalue, pstatMeasurement_t * measurement);
static bool SetCurrentScale( WE_Scale_t scale);
static void EnablePstat(void);
static void DisablePstat(void);
static bool CalibratePstat(void);

static WE_Scale_t CurrentScale=WE_SCALE_100_uA;


void pstat_task(void * parm)
{
    Message_t PortMsg;


    for (;;)
    {
        // Handle pstat Queue
        if (xQueueReceive( pstat_Queue, (void*)&PortMsg, 10 ))
        {
            if (PortMsg.Type == CLI_COMMAND_MESSAGE)
            {

                // Commands for basic tasks like measure, enable and disable circuit, and calibration.
                CliMsgContainer *cmd = (CliMsgContainer*)PortMsg.data;

                switch (cmd->CLI_COMMAND_data.Id)
                {
                case PSTAT_MEASUREMENT_REQ:
                    {
                        pstatMeasurement_t measurement;

                        PortMsg.Type = CLI_MEASUREMENT_RESP;

                        MakeMeasurement(cmd->CLI_COMMAND_data.Param1, &measurement);

                        CliSendMeasurementResp(&measurement);

                    }
                    break;

                case PSTAT_CAL_REQ:
                    break;

                case PSTAT_ON_REQ:
                    if (cmd->CLI_COMMAND_data.Param1)
                    {
                        EnablePstat();
                    }
                    else
                    {
                        DisablePstat();
                    }
                    break;

                default:
                    break;
                }

                vPortFree(cmd);
            }

        }

    }
}

// This function makes one measurement of all ADC channels and populates the measurement * with the results
bool MakeMeasurement( uint16_t DACvalue, pstatMeasurement_t * measurement)
{
    bool ret = true;

    measurement->DAC_Setting = DACvalue;

    ads1256_PowerUpInit();

    measurement->WE_Scale = CurrentScale;

    SetCurrentScale(measurement->WE_Scale);

    AD5662_Set(DACvalue);

    measurement->ADC_WE = ads1256_ReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_1, 1);

    measurement->ADC_DAC_RE = ads1256_ReadChannel(ADS1256_CHANNEL_2, ADS1256_CHANNEL_3, 1);

    measurement->ADC_RE = ads1256_ReadChannel(ADS1256_CHANNEL_4, ADS1256_CHANNEL_3, 1);  // Neg is same as above.

    measurement->VREF_2_3rd = ads1256_ReadChannel(ADS1256_CHANNEL_5, ADS1256_CHANNEL_7, 1);

    measurement->VREF_1_3rd = ads1256_ReadChannel(ADS1256_CHANNEL_6, ADS1256_CHANNEL_7, 1);


    return(ret);
}

bool CalibratePstat(void)
{

}


bool SetCurrentScale( WE_Scale_t scale)
{
    bool ret = true;

    HAL_GPIO_WritePin(G_S0_GPIO_Port, G_S0_Pin, (scale & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(G_S1_GPIO_Port, G_S1_Pin, (scale & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(G_S2_GPIO_Port, G_S2_Pin, (scale & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);


    return(ret);
}

void EnablePstat(void)
{
    HAL_GPIO_WritePin(SW1_GPIO_Port, SW1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SW2_GPIO_Port, SW2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SW3_GPIO_Port, SW3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(SW4_GPIO_Port, SW4_Pin, GPIO_PIN_SET);
}

void DisablePstat(void)
{
    HAL_GPIO_WritePin(SW1_GPIO_Port, SW1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SW2_GPIO_Port, SW2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SW3_GPIO_Port, SW3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SW4_GPIO_Port, SW4_Pin, GPIO_PIN_RESET);
}
