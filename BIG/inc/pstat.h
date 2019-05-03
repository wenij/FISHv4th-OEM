/*
 * pstat.h
 *
 * Potentiostat task
 *  Created on: Jan 19, 2019
 *      Author: carstenh
 */

#ifndef PSTAT_H_
#define PSTAT_H_
#ifdef __cplusplus
 extern "C" {
#endif

 /* WE scale settings */
 typedef enum
 {
     WE_SCALE_316_nA = 0,
     WE_SCALE_1_uA = 1,
     WE_SCALE_3p16_uA = 2,
     WE_SCALE_10_uA = 3,
     WE_SCALE_31p6_uA = 4,
     WE_SCALE_100_uA = 5,
     WE_SCALE_316_uA = 6,
     WE_SCALE_UNITY = 7
 } WE_Scale_t;

 // Switch Settings
 typedef enum
 {
     SW_1_ENABLE = 1,
     SW_2_ENABLE = 2,
     SW_3_ENABLE = 4,
     SW_4_ENABLE = 8,
     SW_INVALID = 0x10
 } SW_State_t;

#define ADC_NOT_PRESENT 0x08000000

 /* Structure containing one data point */
 typedef struct
 {
     WE_Scale_t WE_Scale;;
     uint16_t DAC_Setting;
     uint16_t SwitchState;
     uint32_t TimeStamp;    // Timestamp 1 ms tick
     int32_t ADC_WE;
     int32_t ADC_DAC_RE;
     int32_t ADC_RE;
     int32_t VREF_1_3rd;
     int32_t VREF_2_3rd;
     int32_t TestMeasurement;
 } pstatMeasurement_t;


 typedef enum
 {
      PSTAT_RUN_REQ,
      PSTAT_CANCEL_REQ,
 } PstatCmdId;

// Command Structure for Run command
typedef struct
{
    uint16_t InitialDAC;
    uint16_t StartDAC;
    uint16_t EndDAC;
    uint16_t FinalDAC;
    uint32_t TimeUs1;
    uint16_t MeasureDACStep1;
    uint32_t TimeUs2;
    uint16_t MeasureDACStep2;
    uint16_t Count;
    uint8_t  Switch;
} PstatRunReq_t;

// Command container for Pstat commands. MessageType: PSTAT_COMMAND_MESSAGE
typedef struct
{
    PstatCmdId PstatId;
    union
    {
        PstatRunReq_t Run;
    } Req;
} PstatMsgContainer_t;

extern void PstatSendRunReq( PstatRunReq_t * );

  /*******************************
   *
   * CLI Test Command definitions encapsulated in the PSTAT_MEASUREMENT_REQ
   */

#define SET_PSTAT_COMMAND(_dest,_val) (_dest) = (((_dest) & 0x0000FFFF) | ((uint32_t)((_val) << 16)))
#define GET_PSTAT_COMMAND(_val) ((_val) >> 16)

 // Test command structure for meas test command PSTAT_MEASUREMENT_REQ
 // 0000xxxx 00000000   // Set DAC and measure all data
 // 00010000 00000000   // Measure all data using existing settings
 // 0002xxxx ppnnbb00   // Set DAC and measure only channels designated by P and N (AINP and AINN) using input buffer state B
#define PSTAT_MEASURE_ALL 0
#define PSTAT_MEASURE_REPEAT 1
#define PSTAT_MEASURE_CHANNELS 2

#define GET_PSTAT_MEAS_DAC(_val) (((_val) & 0xFFFF))
#define GET_PSTAT_MEAS_PCHAN(_val) (((_val) & 0xFF000000) >> 24)
#define GET_PSTAT_MEAS_NCHAN(_val) (((_val) & 0x00FF0000) >> 16)
#define GET_PSTAT_MEAS_BUFFER(_val) (((_val) & 0x0000FF00) >> 8)

#define SET_PSTAT_MEASURE_DAC(_dest,_val) (_dest) = (((_dest) & 0xFFFF0000) | ((uint32_t)((_val) & 0xFFFF)))
#define SET_PSTAT_MEASURE_CHANNELS(_dest,_p,_n,_b) (_dest) = (((_dest) & 0x0000FFFF) | ((uint32_t)((_p) & 0xFF) << 24) | ((uint32_t)((_n) & 0xFF) << 16) | ((uint32_t)((_b) & 0xFF) << 8) )

 // Test command structure for pson test command PSTAT_ON_REQ
 // 000000ss 00000000  // Set Switch State
 // 0001ggss 00000000  // Set gain State
 //
#define PSTAT_ON_SWITCH  0
#define PSTAT_ON_SWITCH_GAIN 1
#define GET_PSTAT_PSON_GAIN(_val) (((_val) & 0x0000FF00) >> 8)
#define GET_PSTAT_PSON_SWITCH(_val) (((_val) & 0x000000FF))

#define SET_PSTAT_PSON_SW(_dest,_sw) (_dest) = (((_dest) & 0xFFFFFF00) | ((uint32_t)((_sw) & 0xFF)))
#define SET_PSTAT_PSON_SW_SG(_dest,_sw,_sg) (_dest) = (((_dest) & 0xFFFF0000) | ((uint32_t)((_sg) & 0xFF) << 8) | ((uint32_t)((_sw) & 0xFF)) )



#define DAC_INVALID_VALUE 0x10000

extern void pstat_task(void * parm);

extern void pstat_meas_start_run(PstatRunReq_t * cfg );

extern void pstat_measure_tick(void);

#ifdef __cplusplus
}
#endif
#endif /* PSTAT_H_ */
