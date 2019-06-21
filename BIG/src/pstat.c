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
static bool MakeMeasurementFromISR(pstatDynamicMeasurement_t * measurement);

static bool SetCurrentScale( WE_Scale_t scale);

static void SetPstatSwitches(uint16_t SW);

static bool CalibratePstat(void);

static WE_Scale_t CurrentScale=WE_SCALE_100_uA;

static uint16_t LastDAC = 0;

static uint16_t LastSW = 0;

static uint32_t PstatGoodCount;
static uint32_t PstatFailCount;


static void pstat_measure_baseline(void);
static void pstat_measure_I_WE(void);
static void pstat_measure_U_DAC(void);
static void pstat_measure_Finish( bool Measuring);



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
            else if (PortMsg.Type == PSTAT_COMMAND_MESSAGE)
            {
                // Commands for pstat online functions like running a sweep
                PstatMsgContainer_t *cmd = (PstatMsgContainer_t*)PortMsg.data;
                Message_t AckMsg;

                switch (cmd->PstatId)
                {
                case PSTAT_RUN_REQ:
                    // Initiate a measurement sweep. We disable SPI comms at this time until it finishes.
                    SPI_SetIrqMode(true);
                    xQueueReceive( pstat_Queue, (void*)&AckMsg, 10 );

                    {
                        PstatRunReq_t *req = &cmd->Req.Run;
                        pstat_meas_start_run( req );
                    }
                    break;

                case PSTAT_CANCEL_REQ:
                    SPI_SetIrqMode(false);
                    xQueueReceive( pstat_Queue, (void*)&AckMsg, 10 );
                    break;

                default:
                    break;
                }

                vPortFree(cmd);

            }
            else if (PortMsg.Type == PSTAT_INFO_IND)
            {
                PstatMsgContainer_t *payload = (PstatMsgContainer_t*)pvPortMalloc(sizeof(PstatMsgContainer_t));
                pstatDynamicMeasurement_t QMsg;

                // Change SPI mode to unblocking
                SPI_SetIrqMode(false);

                // Unload the contents of the data queue
                while (xQueueReceiveFromISR( CliMeasurement_Queue, (void*)&QMsg, NULL))
                {
                    while (1)
                    {
                        if (DataPortTxComplete) break;
                    }

                    CliSendDataPortMeasurement( &QMsg );

                }

                configASSERT(payload != NULL);

                if (payload != NULL)
                {
                    // Tell CLI we're done
                    PortMsg.data = (uint8_t*)payload;

                    payload->PstatId = PSTAT_RUN_COMPLETE_IND;
                    payload->Req.RunStats.Good_Count = PstatGoodCount;
                    payload->Req.RunStats.Fail_Count = PstatFailCount;

                    xQueueSend(CliDataQueue, &PortMsg, 10);
                }

            }

        }

    }
}

/*************************************************************************************
 *  Functions associated with measuring in the timer interrupt
 */
static PstatRunReq_t Config;
static uint16_t CurrentDAC;
static int16_t MeasureCount;
static int16_t ChangeDACCount;
static uint16_t TargetDAC;

static bool CountUp;

typedef enum
{
    PSTAT_MEAS_INIT_TO_START,
    PSTAT_MEAS_START_TO_END,
    PSTAT_MEAS_END_TO_START,
    PSTAT_MEAS_END_TO_FINAL,
    PSTAT_MEAS_DONE
} PstatMeasState_t;

static PstatMeasState_t MeasState;



void pstat_meas_start_run(PstatRunReq_t * cfg)
{
    if (!GetSpiIntMode())
    {
        return;
    }

    // Save the setup configuration
    Config = *cfg;

    PstatGoodCount = PstatFailCount = 0;

    // Initial values
    CurrentDAC = Config.InitialDAC;
    TargetDAC = Config.StartDAC;
    if (Config.InitialDAC <= Config.StartDAC)
    {
        CountUp = true;
    }
    else
    {
        CountUp = false;
    }
    MeasureCount = Config.MeasureTime;
    ChangeDACCount = Config.DACStep;

    MeasState = PSTAT_MEAS_INIT_TO_START;

    SetPstatSwitches(Config.Switch);

    AD5662_Set(CurrentDAC);

    ads1256_PowerUpInit(true);

    DataPortTxComplete = true;

    // Enable the timer
    TimEnableMeasureTimer(Config.TimeSliceUs);

}

static pstatDynamicMeasurement_t QMsg;
static pstatDynamicMeasurement_t Measurement;
bool StateChange = false;
int ADC_State = 0;


void pstat_measure_data_ready(void)
{
    ads1256_Busy = false;

    HW_DEBUG_BT_RST(GPIO_PIN_SET);

    // Disable the interrupt - one will occur every 33 us otherwise.
    DisableADC_DRDY_int();

    // ADC Data Ready interrupt
    switch (ADC_State)
    {
    case 0: // State 0.
        pstat_measure_baseline();
        ADC_State++;
        EnableADC_DRDY_int();
        break;

    case 1:
        pstat_measure_U_DAC();
        ADC_State++;
        EnableADC_DRDY_int();
        break;

    case 2:
        pstat_measure_I_WE();
        pstat_measure_Finish(true);
        ADC_State++;
        break;

    default:
        break;
    }

    HW_DEBUG_BT_RST(GPIO_PIN_RESET);

}

void pstat_measure_tick_int(void)
{
    StateChange = false;
    ADC_State = 0;

    // Initiate a measurement and a data transmission
    HW_DEBUG_BT_CS(GPIO_PIN_SET);

    if (GetSpiIntMode())
    {

        if (DataPortTxComplete)
        {
            if (xQueueReceiveFromISR( CliMeasurement_Queue, (void*)&QMsg, NULL))
            {
                CliSendDataPortMeasurement( &QMsg );
            }
        }

        if (--MeasureCount <= 0)
        {

            // Time to measure.
            MakeMeasurementFromISR(&Measurement);

            MeasureCount = Config.MeasureTime;

        }
        else
        {
            pstat_measure_Finish(false); // No measurement so we just run Finish up. This is where the DAC is programmed
        }

    }

    HW_DEBUG_BT_CS(GPIO_PIN_RESET);
}

/**************************************************************
 * Measurement utility functions
 */

static const WE_Scale_t scales[8] = {WE_SCALE_UNITY, WE_SCALE_316_uA, WE_SCALE_100_uA, WE_SCALE_31p6_uA, WE_SCALE_10_uA, WE_SCALE_3p16_uA, WE_SCALE_1_uA, WE_SCALE_316_nA};

// Multipliers with reference to scale 316 uA (x100)
static const int multipliers[8] = {1, 100, 316, 1000, 3160, 10000, 31600, 100000};

#define DAC_SCALE_THRESHOLD (0x7FFFFF * 100 / 316)

//#define USE_CYCLE_MEASUREMENT

void pstat_measure_baseline(void)
{
    int i = 1;
    int32_t value;
    uint32_t test, base;

    // First conversion contains the baseline measurement
    value = ads1256_ReadDataWhenReady( 0 );

    base = (uint32_t)(value < 0 ? -value : value);

    for (i=1; i<7; i++)
    {
        test = base * multipliers[i] / 100;
        if (test >= DAC_SCALE_THRESHOLD)
        {
            break;
        }
    }

    SetCurrentScale((WE_Scale_t)scales[i]);

    ads1256_InitiateReadChannel(ADS1256_CHANNEL_2, ADS1256_CHANNEL_AINCOM);

}

void pstat_measure_I_WE(void)
{
    Measurement.ADC_WE = ads1256_ReadDataWhenReady(0);

}
void pstat_measure_U_DAC(void)
{
    Measurement.ADC_DAC_RE = ads1256_ReadDataWhenReady(0);

    ads1256_InitiateReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);


}

void pstat_measure_Finish( bool Measuring)
{

    if (Measuring)
    {
        Measurement.TimeStamp = xTaskGetTickCountFromISR();   // Get Time Stamp ISR

        Measurement.SwitchState = LastSW;

        Measurement.DAC_Setting = CurrentDAC;

        if (xQueueSendToBackFromISR( CliMeasurement_Queue, &Measurement, NULL))
        {
            PstatGoodCount++;
        }
        else
        {
            PstatFailCount++;
        }

    }


    if ( (CountUp && (CurrentDAC >= TargetDAC)) || ((!CountUp) && (CurrentDAC <= TargetDAC)) )
    {
        StateChange = true;
    }

    switch (MeasState)
    {
    case PSTAT_MEAS_INIT_TO_START:
        if (StateChange)
        {
            MeasState = PSTAT_MEAS_START_TO_END;

            TargetDAC = Config.EndDAC;

            CountUp = (TargetDAC > CurrentDAC);

        }
        break;
    case PSTAT_MEAS_START_TO_END:
        if (StateChange)
        {
            if (Config.Count <= 1)
            {
                if (Config.FinalDAC != CurrentDAC)
                {
                    MeasState = PSTAT_MEAS_END_TO_FINAL;
                    TargetDAC = Config.FinalDAC;
                }
                else
                {
                    MeasState = PSTAT_MEAS_DONE;
                }
            }
            else
            {
                MeasState = PSTAT_MEAS_END_TO_START;
                TargetDAC = Config.StartDAC;
            }

            CountUp = (TargetDAC > CurrentDAC);

        }
        break;
    case PSTAT_MEAS_END_TO_START:
        if (StateChange)
        {
            Config.Count--;

            MeasState = PSTAT_MEAS_START_TO_END;

            TargetDAC = Config.EndDAC;

            CountUp = (TargetDAC > CurrentDAC);

        }
        break;

    case PSTAT_MEAS_END_TO_FINAL:
        if (StateChange)
        {
            MeasState = PSTAT_MEAS_DONE;
        }
        break;
    case PSTAT_MEAS_DONE:
        break;
    default:
        break;

    }

    if (MeasState != PSTAT_MEAS_DONE)
    {
        // Handle DAC stepping
        if (--ChangeDACCount <= 0)
        {
            ChangeDACCount = Config.DACStep;

            if (CountUp)
            {
                if (CurrentDAC <= DAC_MAX - Config.DACStep)
                {
                    CurrentDAC += Config.DACStep;
                }
                else
                {
                    CurrentDAC = DAC_MAX;
                }
            }
            else
            {
                if (CurrentDAC >= Config.DACStep)
                {
                    CurrentDAC -= Config.DACStep;
                }
                else
                {
                    CurrentDAC = 0;
                }
            }

            AD5662_Set(CurrentDAC);
        }

    }
    else
    {
        Message_t msg;

        // Done - we need to stop the interrupt and send a confirmation to the host
        TimDisableMeasureTimer();

        msg.Type = PSTAT_INFO_IND;
        msg.data = NULL;

        xQueueSendToFrontFromISR( pstat_Queue, &msg, NULL);
        //xQueueSendToFront( pstat_Queue, &msg, 10);

    }
}

// This function makes a measurement from an ISR
bool MakeMeasurementFromISR(pstatDynamicMeasurement_t * measurement)
{
    SetCurrentScale(WE_SCALE_316_uA);

    ads1256_InitiateReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);

    EnableADC_DRDY_int();

    return(true);

#if 0
    int i = 1;
    int32_t value;
    uint32_t test, base;

    SetCurrentScale(WE_SCALE_316_uA);


#ifdef USE_CYCLE_MEASUREMENT
    ads1256_CycleChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);  // First measurement, ignoring the result.

    value = ads1256_CycleChannel(ADS1256_CHANNEL_2, ADS1256_CHANNEL_AINCOM);
#else

    value = ads1256_ReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);
#endif

    base = (uint32_t)(value < 0 ? -value : value);

    for (i=1; i<7; i++)
    {
        test = base * multipliers[i] / 100;
        if (test >= DAC_SCALE_THRESHOLD)
        {
            break;
        }
    }

    SetCurrentScale((WE_Scale_t)scales[i]);

#ifdef USE_CYCLE_MEASUREMENT

    measurement->ADC_DAC_RE = ads1256_CycleChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);

    measurement->ADC_WE = ads1256_ReadDataWhenReady(0);
#else
    measurement->ADC_DAC_RE = ads1256_ReadChannel(ADS1256_CHANNEL_2, ADS1256_CHANNEL_AINCOM);

    measurement->ADC_WE = ads1256_ReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);
#endif

    measurement->WE_Scale = CurrentScale;

    // Measure DAC+RE

    measurement->TimeStamp = xTaskGetTickCountFromISR();   // Get Time Stamp ISR

    measurement->SwitchState = LastSW;

    measurement->DAC_Setting = CurrentDAC;

    if (xQueueSendToBackFromISR( CliMeasurement_Queue, measurement, NULL))
    {
        PstatGoodCount++;
    }
    else
    {
        PstatFailCount++;
    }

    return(true);
#endif

}

// This function makes one measurement of all ADC channels and populates the measurement * with the results
bool MakeMeasurement( uint16_t DACvalue, pstatMeasurement_t * measurement)
{
    bool ret = true;

    measurement->DAC_Setting = DACvalue;

    CurrentDAC = DACvalue;

    ads1256_PowerUpInit(true);

    measurement->WE_Scale = CurrentScale;

    measurement->SwitchState = LastSW;

    SetCurrentScale(measurement->WE_Scale);

    AD5662_Set(DACvalue);

    //TimDelayMicroSeconds(200);  // Allow dac to settle. Not necessary.

    measurement->TimeStamp = xTaskGetTickCount();   // Get Time Stamp

    measurement->ADC_WE = ads1256_ReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);

    measurement->ADC_DAC_RE = ads1256_ReadChannel(ADS1256_CHANNEL_2, ADS1256_CHANNEL_AINCOM);

    measurement->ADC_RE = ads1256_ReadChannel(ADS1256_CHANNEL_4, ADS1256_CHANNEL_AINCOM);

    measurement->VREF_2_3rd = ads1256_ReadChannel(ADS1256_CHANNEL_5, ADS1256_CHANNEL_AINCOM);

    measurement->VREF_1_3rd = ads1256_ReadChannel(ADS1256_CHANNEL_6, ADS1256_CHANNEL_AINCOM);

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

    measurement->TestMeasurement = ads1256_ReadChannel(Pchannel, Nchannel);

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


void PstatSendRunReq( PstatRunReq_t * Cmd)
{
    Message_t Msg;
    PstatMsgContainer_t *payload = (PstatMsgContainer_t*)pvPortMalloc(sizeof(PstatMsgContainer_t));

    Msg.data = (uint8_t*)payload;
    Msg.Type = PSTAT_COMMAND_MESSAGE;

    payload->PstatId = PSTAT_RUN_REQ;
    payload->Req.Run = *Cmd;


    xQueueSend( pstat_Queue, &Msg, 0 );
}

