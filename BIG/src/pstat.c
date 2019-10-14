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
static int CurrentScaleIdx = 2;	// index corresponding to 100 uA

static uint16_t LastDAC = 0;

static uint16_t LastSW = 0;


static uint32_t PstatGoodCount;
static uint32_t PstatFailCount;


static void pstat_measure_baseline(void);
static void pstat_finish_baseline(void);
static void pstat_measure_I_WE(void);
static void pstat_measure_U_DAC(void);
static void pstat_measure_U_RE(void);
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
                case PSTAT_RUN_VA_REQ:
                    // Initiate a measurement sweep. We disable SPI comms at this time until it finishes.
                    CalibratePstat();

                    SPI_SetIrqMode(true);
                    xQueueReceive( pstat_Queue, (void*)&AckMsg, 10 );
                    {
                        PstatRunReqVA_t *req = &cmd->Req.RunVA;
                        pstat_meas_start_VA( req );
                    }
                    break;

                case PSTAT_RUN_CVA_REQ:
                    // Initiate a measurement sweep. We disable SPI comms at this time until it finishes.
                    CalibratePstat();

                    SPI_SetIrqMode(true);
                    xQueueReceive( pstat_Queue, (void*)&AckMsg, 10 );
                    {
                        PstatRunReqCVA_t *req = &cmd->Req.RunCVA;
                        pstat_meas_start_CVA( req );
                    }
                    break;

                case PSTAT_CANCEL_REQ:
                    pstat_meas_cancel();

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
static PstatRunReqVA_t Config;


static uint16_t CurrentDAC;
static int16_t MeasureCount;
static int32_t ChangeDACCount;
static uint16_t TargetDAC;
static uint16_t MeasureCountBase;
static bool GA_ContinueMeasurements = false;

static bool CountUp;

static PstatCmdId Mode;


static void  Set_DAC_Target(uint16_t dac)
{
    TargetDAC = dac;
}

static bool pstat_measure_RestartADC(void)
{
	bool ret = false;

	if (Mode == PSTAT_RUN_GA_REQ)
	{
		ret = GA_ContinueMeasurements;
	}

	return(ret);
}

#define ADC_MAX_DAC_RANGE 0x7FFF00      // Maximum DAC value can't go above here.
static int ADC_LimitCount = 0;

static bool Check_ADC_Value_Limiting(int32_t adc_reading)
{
    bool ret = false;

    if (--ADC_LimitCount <= 0)
    {

        if ( (adc_reading > ADC_MAX_DAC_RANGE) || (adc_reading < -ADC_MAX_DAC_RANGE) )
        {
            ret = true;
        }
    }

    return(ret);
}

static void Set_ADC_Value_Limiting_Hysteresis(void)
{
    ADC_LimitCount = 10;
}

typedef enum
{
    PSTAT_MEAS_INIT_TO_START,
    PSTAT_MEAS_START_TO_END,
    PSTAT_MEAS_END_TO_START,
    PSTAT_MEAS_END_TO_FINAL,
    PSTAT_MEAS_DONE
} PstatMeasState_t;

typedef enum
{
	PSTAT_GA_INIT_PT_1,
	PSTAT_GA_INIT_PT_2,
	PSTAT_GA_SET_INITIAL_VALUE,
	PSTAT_GA_RUN_CYCLE
} PstatGA_States_t;

typedef enum
{
	PSTAT_GA_COARSE_MEASURE,
	PSTAT_GA_FINE_MEASURE,
	PSTAT_GA_FINE_MEASURE_DONE
} PstatGA_Measure_States_t;

static PstatMeasState_t MeasState;
static bool CancelPending = false;
static bool CancelInProgress = false;

void pstat_meas_cancel(void)
{
    CancelPending = true;
    CancelInProgress = false;
}

void pstat_meas_start_VA(PstatRunReqVA_t * cfg)
{
    if (!GetSpiIntMode())
    {
        TimDisableMeasureTimer();
        return;
    }

    CancelPending = false;
    CancelInProgress = false;

    Mode = PSTAT_RUN_VA_REQ;

    // Save the setup configuration
    Config = *cfg;

    PstatGoodCount = PstatFailCount = 0;

    // Initial values
    CurrentDAC = Config.InitialDAC;
    Set_DAC_Target(Config.StartDAC);

    if (Config.InitialDAC <= Config.StartDAC)
    {
        CountUp = true;
    }
    else
    {
        CountUp = false;
    }
    MeasureCount = Config.MeasureTime;
    ChangeDACCount = Config.DACTime;
    MeasureCountBase = Config.MeasureTime;

    ADC_LimitCount = 0;

    MeasState = PSTAT_MEAS_INIT_TO_START;

    SetPstatSwitches(Config.Switch);

    AD5662_Set(CurrentDAC);

    ads1256_PowerUpInit(true);

    DataPortTxComplete = true;

    // Enable the timer
    TimEnableMeasureTimer(Config.TimeSliceUs);

}

static PstatRunReqGA_t GA_Config;
static PstatGA_States_t GA_State;

void pstat_meas_start_GA(PstatRunReqGA_t * cfg)
{
    if (!GetSpiIntMode())
    {
        TimDisableMeasureTimer();
        return;
    }

    CancelPending = false;
    CancelInProgress = false;

    Mode = PSTAT_RUN_VA_REQ;

    GA_State = PSTAT_GA_INIT_PT_1;

    // Save the setup configuration
    GA_Config = *cfg;

    PstatGoodCount = PstatFailCount = 0;

    // Initial values
    CurrentDAC = GA_Config.InitialDAC_Value;

    // DAC Targets are a bit meaningless since we are going by current. Instead we set the DAC to MAX range.
    if (GA_Config.InitialCurrentTarget > GA_Config.StartCurrentTarget)
    {
    	// DAC and measured data has inverted signs
        CountUp = true;
        Set_DAC_Target(DAC_MAX);
    }
    else
    {
        CountUp = false;
        Set_DAC_Target(DAC_MIN);
    }
    MeasureCount = GA_Config.MeasureTime;
    ChangeDACCount = GA_Config.DACTime;
    MeasureCountBase = GA_Config.MeasureTime;

    ADC_LimitCount = 0;

    MeasState = PSTAT_MEAS_INIT_TO_START;

    SetPstatSwitches(Config.Switch);

    AD5662_Set(CurrentDAC);

    ads1256_PowerUpInit(true);

    DataPortTxComplete = true;

    // Enable the timer
    TimEnableMeasureTimer(Config.TimeSliceUs);

}

static PstatRunReqCVA_t CVA_Config;
static uint32_t TimeAtFirstDAC;
static uint32_t TimeAtEndDAC;

void pstat_meas_start_CVA(PstatRunReqCVA_t * cfg)
{
    if (!GetSpiIntMode())
    {
        TimDisableMeasureTimer();
        return;
    }

    CancelPending = false;
    CancelInProgress = false;

    Mode = PSTAT_RUN_CVA_REQ;

    // Save the setup configuration
    CVA_Config = *cfg;

    PstatGoodCount = PstatFailCount = 0;

    // Initial values
    CurrentDAC = CVA_Config.StartDAC;
    Set_DAC_Target(CVA_Config.EndDAC);

    MeasureCount = 1;

    TimeAtFirstDAC = CVA_Config.DACTimeAtStart;
    TimeAtEndDAC = CVA_Config.DACTimeAtEnd;
    MeasureCountBase = 1;

    MeasState = PSTAT_MEAS_INIT_TO_START;

    SetPstatSwitches(CVA_Config.Switch);

    AD5662_Set(CurrentDAC);

    ads1256_PowerUpInit(true);

    DataPortTxComplete = true;

    // Enable the timer
    TimEnableMeasureTimer(CVA_Config.TimeSliceUs);

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
        pstat_finish_baseline();
        ADC_State++;
        EnableADC_DRDY_int();
        break;

    case 2:
        pstat_measure_I_WE();
        ADC_State++;
        EnableADC_DRDY_int();
        break;

    case 3:
        pstat_measure_U_DAC();
        ADC_State++;
        EnableADC_DRDY_int();
        break;

    case 4:
        pstat_measure_U_RE();
        pstat_measure_Finish(true);
         if (pstat_measure_RestartADC())
        {
			ADC_State = 0;
            EnableADC_DRDY_int();
        }
        else
        {
        	ADC_State++;
        }
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

            MeasureCount = MeasureCountBase;

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

// Multipliers to convert ADC reading to current; Working unit is nA.
//   4096 ADC clicks per mV, or 4096000 DAC clicks per volt.
//   316000 nA per volt on scale 316 uA, which is 316 nA per mv or 316/4096 nA per ADC click. Our unit will be scaled to 1 ADC click on highest sensitivity.
static const int currentscale[8] = {1, 316000, 100000, 31600, 10000, 3160, 1000, 316};

#define ADC_GAIN_DOWN_THRESHOLD 0x7E0000

#define ADC_GAIN_UP_THRESHOLD (ADC_GAIN_DOWN_THRESHOLD * 100 / 316)


//#define USE_CYCLE_MEASUREMENT

// Scale a current measurement to one lsb at highest sensitivity.
static int64_t ScaledCurrentMeasurement(void)
{
	int64_t meas = (int64_t)Measurement.ADC_WE;

	meas *= (int64_t)currentscale[CurrentScaleIdx] * 4096;

	meas /= 1000;

	return( meas);
}

// Scale a current value in uA to the internal ADC unit.
static int64_t ScaleCurrentValue_uA(int32_t CurrentUA)
{
	int64_t temp = (int64_t)CurrentUA;

	temp *= 1000 * 4096 / 316;

	return(temp);
}

static int64_t ScaleCurrentValue_nA(int32_t CurrentnA)
{
	int64_t temp = (int64_t)CurrentnA;

	temp *=  4096 / 316;

	return(temp);
}

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
        if (test >= ADC_GAIN_UP_THRESHOLD)
        {
            break;
        }
    }

    SetCurrentScale((WE_Scale_t)scales[i]);


    ads1256_InitiateReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);

}

static bool RetakeWE;

void pstat_finish_baseline(void)
{
    int32_t value;
    uint32_t base;

    // First measurement after baselining. Check that the ADC is not close to maxing out.

    value = ads1256_ReadDataWhenReady( 0 );

    base = (uint32_t)(value < 0 ? -value : value);

    RetakeWE = false;

    if (base >= ADC_GAIN_DOWN_THRESHOLD)
    {
        if ( (int)CurrentScale < WE_SCALE_UNITY)
        {
            SetCurrentScale((WE_Scale_t)( (int)CurrentScale + 1));
            RetakeWE = true;
        }

    }

    if (!RetakeWE)
    {
        Measurement.ADC_WE = value;
    }

    ads1256_InitiateReadChannel(ADS1256_CHANNEL_0, ADS1256_CHANNEL_AINCOM);
}

void pstat_measure_I_WE(void)
{
    int32_t measurement;

    measurement = ads1256_ReadDataWhenReady(0);

    if (RetakeWE)
    {
        // We are doing this because when we have a good measurement already there is no need to take another one. In fact it oftentimes ends up exceeding the ADC range.
        Measurement.ADC_WE = measurement;
    }

    ads1256_InitiateReadChannel(ADS1256_CHANNEL_2, ADS1256_CHANNEL_AINCOM);  // DAC_RE

}

void pstat_measure_U_DAC(void)
{
    Measurement.ADC_DAC_RE = ads1256_ReadDataWhenReady(0);
    ads1256_InitiateReadChannel(ADS1256_CHANNEL_4, ADS1256_CHANNEL_AINCOM);  // RE
}

void pstat_measure_U_RE(void)
{
    Measurement.ADC_RE = ads1256_ReadDataWhenReady(0);

}

void pstat_measure_Finish_VA(void)  // Finish Voltammetry
{
    bool ADC_Limiting = Check_ADC_Value_Limiting(Measurement.ADC_DAC_RE);

    if ( (CountUp && (CurrentDAC >= TargetDAC)) || ((!CountUp) && (CurrentDAC <= TargetDAC)) ||  ADC_Limiting)
    {
        StateChange = true;

        if (ADC_Limiting)
        {
            TargetDAC = CurrentDAC;     // So we change direction cleanly.
        }

        Set_ADC_Value_Limiting_Hysteresis();
    }

    if (CancelPending)
    {
    	MeasState = PSTAT_MEAS_END_TO_FINAL;	// Begin ramp-down
        Set_DAC_Target(Config.FinalDAC);
        CountUp = (TargetDAC > CurrentDAC);
        StateChange = false;
        CancelInProgress = true;
        CancelPending = false;
    }

    switch (MeasState)
    {
    case PSTAT_MEAS_INIT_TO_START:
        if (StateChange)
        {
            MeasState = PSTAT_MEAS_START_TO_END;

            Set_DAC_Target(Config.EndDAC);

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
                    Set_DAC_Target(Config.FinalDAC);
                }
                else
                {
                    MeasState = PSTAT_MEAS_DONE;
                }
            }
            else
            {
                MeasState = PSTAT_MEAS_END_TO_START;
                Set_DAC_Target(Config.StartDAC);
            }

            CountUp = (TargetDAC > CurrentDAC);

        }
        break;
    case PSTAT_MEAS_END_TO_START:
        if (StateChange)
        {
            Config.Count--;

            MeasState = PSTAT_MEAS_START_TO_END;

            Set_DAC_Target(Config.EndDAC);

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
            ChangeDACCount = Config.DACTime;

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
}

void pstat_measure_Finish_CVA(void)
{
	bool Finish = false;

	if (CancelInProgress) return;

    // ChronoVoltammetry
    if (TimeAtFirstDAC > 0)
    {
        TimeAtFirstDAC--;

        if (TimeAtFirstDAC == 0)
        {
            // End of the period. Change the DAC.
            CurrentDAC = CVA_Config.EndDAC;

            AD5662_Set(CurrentDAC);
        }

    }
    else if (TimeAtEndDAC > 0)
    {
        TimeAtEndDAC--;
    }
    else
    {
    	Finish = true;
    }

    if ( Finish || CancelPending )
    {
        CurrentDAC = CVA_Config.FinalDAC;

        AD5662_Set(CurrentDAC);

        MeasState = PSTAT_MEAS_DONE;

        if (CancelPending)
        {
        	CancelInProgress = true;
        	CancelPending = false;
        }
    }
}

static int64_t ga_pt_1;
static int64_t ga_slope;
static int32_t ga_measurement;
static uint16_t ga_DACStep;
static uint16_t ga_FineCount;

static PstatGA_Measure_States_t GA_SubState;

#define SLOPE_DAC_SPAN 16	// Number of DAC clicks the slope is calculated over.
#define MAX_FINE_TUNE_STEPS  4 // Max times we loop through fine tuning of measurements.

void pstat_measure_Finish_GA(void) // Finish Galvanometry
{

	GA_ContinueMeasurements = false;

    switch(GA_State)
    {
    case PSTAT_GA_INIT_PT_1:
    	// First data point in current slope calculation.
    	ga_pt_1 = ScaledCurrentMeasurement();

    	if ( (CountUp && ((DAC_MAX - CurrentDAC) >= SLOPE_DAC_SPAN)) || (CurrentDAC < SLOPE_DAC_SPAN))
    	{
			AD5662_Set(CurrentDAC + SLOPE_DAC_SPAN);
    	}
    	else
    	{
            AD5662_Set(CurrentDAC - SLOPE_DAC_SPAN);
    	}

    	GA_State = PSTAT_GA_INIT_PT_2;  // Set up to measure the second data point in slope calculation

    	GA_ContinueMeasurements = true;

    	break;
    case PSTAT_GA_INIT_PT_2:
    {
    	// Second data point in current slope calculation.
    	int64_t meas = ScaledCurrentMeasurement();

    	ga_slope = (meas - ga_pt_1) / SLOPE_DAC_SPAN;

    	// Ensure slope is not 0
    	if (ga_slope == 0)
    	{
    		ga_slope = 1;
    	}

    	// Ensure slope is a positive number.
    	if (ga_slope < 0)
    	{
    		ga_slope = -ga_slope;
    	}

    	// Calculate current step size DAC equivalent. CurrentStep is in nA. Used for coarse setting.
    	int64_t temp = ScaleCurrentValue_nA(GA_Config.CurrentStep);

    	ga_DACStep = temp / ga_slope;	// Number of DAC Steps per current step

        AD5662_Set(CurrentDAC);	// Return DAC to it's original position.


    	GA_State = PSTAT_GA_SET_INITIAL_VALUE;  // Set the initial current value

    	GA_SubState = PSTAT_GA_COARSE_MEASURE;

    	GA_ContinueMeasurements = true;

    	break;
    }
    case PSTAT_GA_SET_INITIAL_VALUE:
    {
        bool ADC_Limiting = Check_ADC_Value_Limiting(Measurement.ADC_DAC_RE);

        if ( (CountUp && (CurrentDAC >= TargetDAC)) || ((!CountUp) && (CurrentDAC <= TargetDAC)) ||  ADC_Limiting)
        {
            StateChange = true;

            if (ADC_Limiting)
            {
                TargetDAC = CurrentDAC;     // So we change direction cleanly.
            }

            Set_ADC_Value_Limiting_Hysteresis();
        }

		switch (GA_SubState)
		{
		case PSTAT_GA_COARSE_MEASURE:
			if (StateChange)
			{
				GA_SubState = PSTAT_GA_FINE_MEASURE;

				Set_DAC_Target(GA_Config.EndDAC);

				CountUp = (TargetDAC > CurrentDAC);

			}
			// Continue measurements if value is too far off.
	    	GA_ContinueMeasurements = true;

	    	ga_FineCount = 0;
			break;
		case PSTAT_GA_FINE_MEASURE:
			if (StateChange)
			{
				if (GA_Config.Count <= 1)
				{
					if (GA_Config.FinalDAC != CurrentDAC)
					{
						MeasState = PSTAT_MEAS_END_TO_FINAL;
						Set_DAC_Target(Config.FinalDAC);
					}
					else
					{
						MeasState = PSTAT_MEAS_DONE;
					}
				}
				else
				{
					MeasState = PSTAT_MEAS_END_TO_START;
					Set_DAC_Target(GA_Config.StartDAC);
				}

				CountUp = (TargetDAC > CurrentDAC);

			}
			break;
		case PSTAT_MEAS_END_TO_START:
			if (StateChange)
			{
				Config.Count--;

				MeasState = PSTAT_MEAS_START_TO_END;

				Set_DAC_Target(GA_Config.EndDAC);

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
				ChangeDACCount = GA_Config.DACTime;

				if (CountUp)
				{
					if (CurrentDAC <= DAC_MAX - GA_Config.DACStep)
					{
						CurrentDAC += GA_Config.DACStep;
					}
					else
					{
						CurrentDAC = DAC_MAX;
					}
				}
				else
				{
					if (CurrentDAC >= GA_Config.DACStep)
					{
						CurrentDAC -= GA_Config.DACStep;
					}
					else
					{
						CurrentDAC = 0;
					}
				}

				AD5662_Set(CurrentDAC);
			}

		}
    }
    	break;
    case PSTAT_GA_RUN_CYCLE:
    	break;
    }

    if (GA_State >= PSTAT_GA_SET_INITIAL_VALUE)
    {
        bool ADC_Limiting = Check_ADC_Value_Limiting(Measurement.ADC_DAC_RE);

        if ( (CountUp && (CurrentDAC >= TargetDAC)) || ((!CountUp) && (CurrentDAC <= TargetDAC)) ||  ADC_Limiting)
        {
            StateChange = true;

            if (ADC_Limiting)
            {
                TargetDAC = CurrentDAC;     // So we change direction cleanly.
            }

            Set_ADC_Value_Limiting_Hysteresis();
        }

		switch (MeasState)
		{
		case PSTAT_MEAS_INIT_TO_START:
			if (StateChange)
			{
				MeasState = PSTAT_MEAS_START_TO_END;

				Set_DAC_Target(GA_Config.EndDAC);

				CountUp = (TargetDAC > CurrentDAC);

			}
			break;
		case PSTAT_MEAS_START_TO_END:
			if (StateChange)
			{
				if (GA_Config.Count <= 1)
				{
					if (GA_Config.FinalDAC != CurrentDAC)
					{
						MeasState = PSTAT_MEAS_END_TO_FINAL;
						Set_DAC_Target(Config.FinalDAC);
					}
					else
					{
						MeasState = PSTAT_MEAS_DONE;
					}
				}
				else
				{
					MeasState = PSTAT_MEAS_END_TO_START;
					Set_DAC_Target(GA_Config.StartDAC);
				}

				CountUp = (TargetDAC > CurrentDAC);

			}
			break;
		case PSTAT_MEAS_END_TO_START:
			if (StateChange)
			{
				Config.Count--;

				MeasState = PSTAT_MEAS_START_TO_END;

				Set_DAC_Target(GA_Config.EndDAC);

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
				ChangeDACCount = GA_Config.DACTime;

				if (CountUp)
				{
					if (CurrentDAC <= DAC_MAX - GA_Config.DACStep)
					{
						CurrentDAC += GA_Config.DACStep;
					}
					else
					{
						CurrentDAC = DAC_MAX;
					}
				}
				else
				{
					if (CurrentDAC >= GA_Config.DACStep)
					{
						CurrentDAC -= GA_Config.DACStep;
					}
					else
					{
						CurrentDAC = 0;
					}
				}

				AD5662_Set(CurrentDAC);
			}

		}
    }
}

void pstat_measure_Finish_CGA(void)
{
    // ChronoGalvanomtry
    if (TimeAtFirstDAC > 0)
    {
        TimeAtFirstDAC--;

        if (TimeAtFirstDAC == 0)
        {
            // End of the period. Change the DAC.
            CurrentDAC = CVA_Config.EndDAC;

            AD5662_Set(CurrentDAC);
        }

    }
    else if (TimeAtEndDAC > 0)
    {
        TimeAtEndDAC--;
    }
    else
    {
        CurrentDAC = CVA_Config.FinalDAC;

        AD5662_Set(CurrentDAC);

        MeasState = PSTAT_MEAS_DONE;
    }
}

void pstat_measure_Finish( bool Measuring)
{

    if (Measuring)
    {
        Measurement.TimeStamp = xTaskGetTickCountFromISR();   // Get Time Stamp ISR

        Measurement.SwitchState = LastSW;

        Measurement.DAC_Setting = CurrentDAC;

        Measurement.WE_Scale = CurrentScale;

        if (xQueueSendToBackFromISR( CliMeasurement_Queue, &Measurement, NULL))
        {
            PstatGoodCount++;
        }
        else
        {
            PstatFailCount++;
        }

    }


    switch (Mode)
    {
    case PSTAT_RUN_VA_REQ:
        pstat_measure_Finish_VA();
        break;

    case PSTAT_RUN_CVA_REQ:
        pstat_measure_Finish_CVA();
        break;

    case PSTAT_RUN_GA_REQ:
        pstat_measure_Finish_GA();
        break;

    case PSTAT_RUN_CGA_REQ:
        pstat_measure_Finish_CGA();
        break;

    default:
        MeasState = PSTAT_MEAS_DONE;
        break;
    }


    if (MeasState == PSTAT_MEAS_DONE)
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
        if (test >= ADC_GAIN_UP_THRESHOLD)
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

    // bump PGA to 2 so full scale is +/- 4096 mv.
    //ads1256_SetPGA(ADS1256_PGA_2);

    // Set switches. This will ensure we calibrate exactly to the DAC 0 and max output.
    SetPstatSwitches(SW_2_ENABLE | SW_3_ENABLE);

    ret &= AD5662_Set(0x8000); // 0 reading

    TimDelayMicroSeconds (2000);  // Allow dac to settle

    // Set switches and DAC
    ret &= ads1256_Cal(CAL_OFFSET);

    ret &= AD5662_Set(0); // Max

    TimDelayMicroSeconds (2000);  // Allow dac to settle

    ret &= ads1256_Cal(CAL_GAIN);

    // Back to PGA=1
    //ads1256_SetPGA(ADS1256_PGA_1);

    return(ret);
}


bool SetCurrentScale( WE_Scale_t scale)
{
    bool ret = true;

    CurrentScale = scale;

    CurrentScaleIdx = 7-scale;

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

void PstatSendCancelReq( void)
{
    Message_t Msg;
    PstatMsgContainer_t *payload = (PstatMsgContainer_t*)pvPortMalloc(sizeof(PstatMsgContainer_t));

    Msg.data = (uint8_t*)payload;
    Msg.Type = PSTAT_COMMAND_MESSAGE;

    payload->PstatId = PSTAT_CANCEL_REQ;

    xQueueSend( pstat_Queue, &Msg, 0 );
}

void PstatSendRunVA_Req( PstatRunReqVA_t * Cmd)
{
    Message_t Msg;
    PstatMsgContainer_t *payload = (PstatMsgContainer_t*)pvPortMalloc(sizeof(PstatMsgContainer_t));

    Msg.data = (uint8_t*)payload;
    Msg.Type = PSTAT_COMMAND_MESSAGE;

    payload->PstatId = PSTAT_RUN_VA_REQ;
    payload->Req.RunVA = *Cmd;


    xQueueSend( pstat_Queue, &Msg, 0 );
}

void PstatSendRunCVA_Req( PstatRunReqCVA_t * Cmd)
{
    Message_t Msg;
    PstatMsgContainer_t *payload = (PstatMsgContainer_t*)pvPortMalloc(sizeof(PstatMsgContainer_t));

    Msg.data = (uint8_t*)payload;
    Msg.Type = PSTAT_COMMAND_MESSAGE;

    payload->PstatId = PSTAT_RUN_CVA_REQ;
    payload->Req.RunCVA = *Cmd;


    xQueueSend( pstat_Queue, &Msg, 0 );
}

void PstatSendRunGA_Req( PstatRunReqGA_t * Cmd)
{
    Message_t Msg;
    PstatMsgContainer_t *payload = (PstatMsgContainer_t*)pvPortMalloc(sizeof(PstatMsgContainer_t));

    Msg.data = (uint8_t*)payload;
    Msg.Type = PSTAT_COMMAND_MESSAGE;

    payload->PstatId = PSTAT_RUN_CVA_REQ;
    payload->Req.RunGA = *Cmd;


    xQueueSend( pstat_Queue, &Msg, 0 );
}

void PstatSendRunCGA_Req( PstatRunReqCGA_t * Cmd)
{
    Message_t Msg;
    PstatMsgContainer_t *payload = (PstatMsgContainer_t*)pvPortMalloc(sizeof(PstatMsgContainer_t));

    Msg.data = (uint8_t*)payload;
    Msg.Type = PSTAT_COMMAND_MESSAGE;

    payload->PstatId = PSTAT_RUN_CGA_REQ;
    payload->Req.RunCGA = *Cmd;


    xQueueSend( pstat_Queue, &Msg, 0 );
}

