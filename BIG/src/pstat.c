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
#include "stm32f2xx_hal_gpio.h"
#else
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#endif

#include <string.h>

#include "gpio.h"
#include "main.h"
#include "cli.h"
#include "spi.h"
#include "pstat.h"
#include "adc_ads1256.h"
#include "dac_ad5662.h"
#include "tim.h"

QueueHandle_t ADC_Queue = NULL;
QueueHandle_t DAC_Queue = NULL;

QueueHandle_t pstat_Queue = NULL;

static bool MakeMeasurement( uint16_t DACvalue, pstatMeasurement_t * measurement);
static bool MakeSingleTestMeasurement( uint16_t DACvalue, uint8_t Pchannel, uint8_t Nchannel, uint8_t buffer_state, pstatMeasurement_t * measurement);

static bool SetCurrentScale( WE_Scale_t scale);

static void SetPstatSwitches(uint16_t SW);

static bool CalibratePstat(void);

static WE_Scale_t CurrentScale=WE_SCALE_100_uA;

static uint16_t LastDAC = 0;

static uint16_t LastSW = 0;

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
                        bool MadeTheMeasurement = true;

                        switch( GET_PSTAT_COMMAND(cmd->CLI_COMMAND_data.Param1) )
                        {
                        case PSTAT_MEASURE_ALL:
                            LastDAC = (uint16_t)GET_PSTAT_MEAS_DAC(cmd->CLI_COMMAND_data.Param1);
                            MakeMeasurement(LastDAC, &measurement);
                            break;
                        case PSTAT_MEASURE_REPEAT:
                            MakeMeasurement(LastDAC, &measurement);
                            break;
                        case PSTAT_MEASURE_CHANNELS:
                            LastDAC = (uint16_t)GET_PSTAT_MEAS_DAC(cmd->CLI_COMMAND_data.Param1);
                            MakeSingleTestMeasurement( LastDAC,
                                    GET_PSTAT_MEAS_PCHAN(cmd->CLI_COMMAND_data.Param2),
                                    GET_PSTAT_MEAS_NCHAN(cmd->CLI_COMMAND_data.Param2),
                                    GET_PSTAT_MEAS_BUFFER(cmd->CLI_COMMAND_data.Param2),
                                    &measurement);

                            break;
                        default:
                            MadeTheMeasurement = false;
                            break;
                        }

                        if (MadeTheMeasurement)
                        {
                            CliSendMeasurementResp(&measurement);
                        }

                    }
                    break;

                case PSTAT_CAL_REQ:
                    CalibratePstat();
                    break;

                case PSTAT_ON_REQ:
                    switch (GET_PSTAT_COMMAND(cmd->CLI_COMMAND_data.Param1))
                    {
                    case PSTAT_ON_SWITCH:
                        SetPstatSwitches( GET_PSTAT_PSON_SWITCH(cmd->CLI_COMMAND_data.Param1));
                        break;

                    case PSTAT_ON_SWITCH_GAIN:
                        SetPstatSwitches( GET_PSTAT_PSON_SWITCH(cmd->CLI_COMMAND_data.Param1));
                        SetCurrentScale( GET_PSTAT_PSON_GAIN(cmd->CLI_COMMAND_data.Param1));
                        break;
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

    ads1256_PowerUpInit(true);

    measurement->WE_Scale = CurrentScale;

    measurement->SwitchState = LastSW;

    SetCurrentScale(measurement->WE_Scale);

    AD5662_Set(DACvalue);

    //TimDelayMicroSeconds(200);  // Allow dac to settle. Not necessary.

    measurement->TimeStamp = xTaskGetTickCount();   // Get Time Stamp

    measurement->ADC_WE = ads1256_ReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM, 1);

    measurement->ADC_DAC_RE = ads1256_ReadChannel(ADS1256_CHANNEL_2, ADS1256_CHANNEL_AINCOM, 1);

    measurement->ADC_RE = ads1256_ReadChannel(ADS1256_CHANNEL_4, ADS1256_CHANNEL_AINCOM, 1);

    measurement->VREF_2_3rd = ads1256_ReadChannel(ADS1256_CHANNEL_5, ADS1256_CHANNEL_AINCOM, 1);

    measurement->VREF_1_3rd = ads1256_ReadChannel(ADS1256_CHANNEL_6, ADS1256_CHANNEL_AINCOM, 1);

    measurement->TestMeasurement = ADC_NOT_PRESENT;


    return(ret);
}

// This function makes a single measurement on a specific set of channels and fills in the Measurement *
bool MakeSingleTestMeasurement( uint16_t DACvalue, uint8_t Pchannel, uint8_t Nchannel, uint8_t buffer_state, pstatMeasurement_t * measurement)
{
    bool ret = true;

    measurement->DAC_Setting = DACvalue;

    ads1256_PowerUpInit(buffer_state);

    measurement->WE_Scale = CurrentScale;

    measurement->SwitchState = LastSW;

    SetCurrentScale(measurement->WE_Scale);

    AD5662_Set(DACvalue);

    measurement->TimeStamp = xTaskGetTickCount();   // Get Time Stamp

    //TimDelayMicroSeconds(200);  // Allow dac to settle. Not necessary.

    measurement->TestMeasurement = ads1256_ReadChannel(Pchannel, Nchannel, 1);

    measurement->ADC_WE = ADC_NOT_PRESENT;

    measurement->ADC_DAC_RE = ADC_NOT_PRESENT;

    measurement->ADC_RE = ADC_NOT_PRESENT;

    measurement->VREF_2_3rd = ADC_NOT_PRESENT;

    measurement->VREF_1_3rd = ADC_NOT_PRESENT;

    return(ret);
}

bool CalibratePstat(void)
{
    bool ret = true;

    ads1256_PowerUpInit(true);

    // Set switches
    SetPstatSwitches(SW_2_ENABLE | SW_3_ENABLE);

    ret &= AD5662_Set(0x8000); // 0 reading

    TimDelayMicroSeconds (2000);  // Allow dac to settle

    // Set switches and DAC
    ret &= ads1256_Cal(CAL_OFFSET);

    ret &= AD5662_Set(0xFFFF); // Max

    TimDelayMicroSeconds (2000);  // Allow dac to settle

    ret &= ads1256_Cal(CAL_GAIN);

    return(ret);
}


bool SetCurrentScale( WE_Scale_t scale)
{
    bool ret = true;

    CurrentScale = scale;

    HAL_GPIO_WritePin(G_S0_GPIO_Port, G_S0_Pin, (scale & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(G_S1_GPIO_Port, G_S1_Pin, (scale & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    HAL_GPIO_WritePin(G_S2_GPIO_Port, G_S2_Pin, (scale & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    return(ret);
}

void SetPstatSwitches(uint16_t SW)
{

    LastSW = SW;

    HAL_GPIO_WritePin(SW1_GPIO_Port, SW1_Pin, (LastSW & SW_1_ENABLE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SW2_GPIO_Port, SW2_Pin, (LastSW & SW_2_ENABLE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SW3_GPIO_Port, SW3_Pin, (LastSW & SW_3_ENABLE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(SW4_GPIO_Port, SW4_Pin, (LastSW & SW_4_ENABLE) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

