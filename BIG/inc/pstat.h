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

 /* Structure containing one data point */
 typedef struct
 {
     WE_Scale_t WE_Scale;;
     uint16_t DAC_Setting;
     uint16_t SwitchState;
     int32_t ADC_WE;
     int32_t ADC_DAC_RE;
     int32_t ADC_RE;
     int32_t VREF_1_3rd;
     int32_t VREF_2_3rd;
 } pstatMeasurement_t;

#define DAC_INVALID_VALUE 0x10000

extern void pstat_task(void * parm);

#ifdef __cplusplus
}
#endif
#endif /* PSTAT_H_ */
