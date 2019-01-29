/*
 * adc_ads1256.h
 *
 * Driver functions for ADC chip ADS1256
 *
 *  Created on: Jan 16, 2019
 *      Author: carstenh
 */

#ifndef ADC_ADS1256_H_
#define ADC_ADS1256_H_
#ifdef __cplusplus
 extern "C" {
#endif


#include "main.h"


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
 extern void ads1256_PowerUpInit( void );


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
 extern void ads1256_Standby( void );

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
 typedef enum
 {
     ADS1256_PGA_1 = 0,
     ADS1256_PGA_2 = 1,
     ADS1256_PGA_4 = 2,
     ADS1256_PGA_8 = 3,
     ADS1256_PGA_16 = 4,
     ADS1256_PGA_32 = 5,
     ADS1256_PGA_64 = 6,
     ADS1256_PGA_64_DUP = 7   // Duplicate???
 } ads1256_pga_t;

extern void ads1256_SetPGA( ads1256_pga_t gain );

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
  *    Data read from the channel
  */
 typedef enum
 {
     ADS1256_CHANNEL_0 = 0,
     ADS1256_CHANNEL_1 = 1,
     ADS1256_CHANNEL_2 = 2,
     ADS1256_CHANNEL_3 = 3,
     ADS1256_CHANNEL_4 = 4,
     ADS1256_CHANNEL_5 = 5,
     ADS1256_CHANNEL_6 = 6,
     ADS1256_CHANNEL_7 = 7,
     ADS1256_CHANNEL_AINCOM = 8
 } ads1256_channel_t;

extern int32_t ads1256_ReadChannel( ads1256_channel_t Pchannel, ads1256_channel_t Nchannel, uint16_t averages);


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
 extern bool ads1256_SelfTest( void );

/*
 * Device Commands
 */
#define ADS1256_WAKEUP  0
#define ADS1256_READ_DATA 0x01
#define ADS1256_READ_DATA_CONT 0x03
#define ADS1256_STOP_CONT_READ 0x0f
#define ADS1256_READ_REGS 0x10  // 0X1R 0x0N  2 byte command returns N bytes starting at address R
#define ADS1256_WRITE_REGS 0x50  //0x5R 0x0N  2 byte command plus data writes N byes starting at address R
#define ADS1256_SELFCAL   0xf0
#define ADS1256_SELFCAL_GAINS  0xf1     // self cal gains
#define ADS1256_SELFCAL_OFFSETS  0xf2
#define ADS1256_SELFCAL_SYSTEMGAIN 0xf3
#define ADS1256_SELFCAL_SYSTEMOFFSET 0xf4
#define ADS1256_SYNC_AD_CONVERSION 0xfc
#define ADS1256_STANDBY_MODE 0xfd
#define ADS1256_RESET  0xfe
#define ADS1256_WAKEUP_2 0xff

#define ADS1256_BUILD_WRITE_REG_CMD(_reg)  ADS1256_WRITE_REGS | (_reg)
 /*
  * Address Map for the device
  */

// Register content definitions

 // Register 0 STATUS Register
#define ADS1256_STATUS_REGISTER  0
#define ADS1256_STATUS_ID3_0 0xF0
#define ADS1256_STATUS_ORDER 0x08
#define ADS1256_STATUS_ACAL  0x04
#define ADS1256_STATUS_BUFEN 0x02
#define ADS1256_STATUS_DRDYn 0x01

 // Register 1 MUX Register
#define ADS1256_MUX_REGISTER 1

#define ADS1256_MUX_PSEL  0xF0
#define ADS1256_MUX_NSEL  0x0F

 // MUX selections
#define ADS1256_MUX_SEL_AIN0 0
#define ADS1256_MUX_SEL_AIN1 2
#define ADS1256_MUX_SEL_AIN2 3
#define ADS1256_MUX_SEL_AIN3 3
#define ADS1256_MUX_SEL_AIN4 4
#define ADS1256_MUX_SEL_AIN5 5
#define ADS1256_MUX_SEL_AIN6 6
#define ADS1256_MUX_SEL_AIN7 7
#define ADS1256_MUX_SEL_AINCOM 8

#define set_ADS1256_MUX_PSEL(_val) ( ((_val)<<4) & ADS1256_MUX_PSEL)
#define get_ADS1256_MUX_PSEL(_val) ( ((_val)& ADS1256_MUX_PSEL) >> 4)

#define set_ADS1256_MUX_NSEL(_val) ( (_val) & ADS1256_MUX_PSEL)
#define get_ADS1256_MUX_NSEL(_val) ( (_val) & ADS1256_MUX_PSEL)

 // Register 2 AD Control Register
#define ADS1256_ADCON_REGISTER 2
 // CLKOUT Selection
#define ADS1256_ADCON_CLKOUT 0x60
#define ADS1256_CLKOUT_SEL_OFF  0
#define ADS1256_CLKOUT_SEL_ONE  1
#define ADS1256_CLKOUT_SEL_DIV2 2
#define ADS1256_CLKOUT_SEL_DIV4 3

#define set_ADS1256_CLKOUT (_val)  ( ((_val)<<5) & ADS1256_ADCON_CLKOUT)
#define get_ADS1256_CLKOUT (_val)  ( ((_val)&ADS1256_ADCON_CLKOUT) >> 5)

 // Sensor Detect Current Sources SDCS
#define ADS1256_ADCON_SDCS 0x18
 // CLKOUT Selections
#define ADS1256_SDCS_SEL_OFF  0
#define ADS1256_SDCS_SEL_0_5_UA  1
#define ADS1256_SDCS_SEL_2_UA 2
#define ADS1256_SDCS_SEL_10_UA 3

#define set_ADS1256_SDCS (_val)  ( ((_val)<<3) & ADS1256_ADCON_SDCS)
#define get_ADS1256_SDCS (_val)  ( ((_val)&ADS1256_ADCON_SDCS) >> 3)

 // PGA setting
#define ADS1256_ADCON_PGA 0x07
 // PGA selection


#define set_ADS1256_PGA (_val) ( (_val) & ADS1256_ADCON_PGA)
#define get_ADS1256_PGA (_val) ( (_val) & ADS1256_ADCON_PGA)

 // Register 3 A/D Data Rate
#define ADS1256_DRATE_REGISTER 3

#define ADS1256_DRATE_30000  0xF0
#define ADS1256_DRATE_15000  0xE0
#define ADS1256_DRATE_7500   0xD0
#define ADS1256_DRATE_3750   0xC0
#define ADS1256_DRATE_2000   0xB0
#define ADS1256_DRATE_1000   0xA1
#define ADS1256_DRATE_500    0x92
#define ADS1256_DRATE_100    0x82
#define ADS1256_DRATE_60     0x72
#define ADS1256_DRATE_50     0x63
#define ADS1256_DRATE_30     0x53
#define ADS1256_DRATE_25     0x43
#define ADS1256_DRATE_15     0x33
#define ADS1256_DRATE_10     0x23
#define ADS1256_DRATE_5      0x13
#define ADS1256_DRATE_2p5    0x03

#define set_ADS1256_DRATE (_val) (_val)
#define get_ADS1256_DRATE (_val) (_val)

 // Register 4 GPIO Control Register
#define ADS1256_GPIO_REGISTER 4

 // Register 5-7 Offset Calibration Registers
#define ADS1256_OFC_BYTE0_REGISTER 5  // LSB
#define ADS1256_OFC_BYTE1_REGISTER 6
#define ADS1256_OFC_BYTE2_REGISTER 7  // MSB

// Register 8-10 Full Calibration bytes
#define ADS1256_FSC_BYTE0_REGISTER 8  // LSB
#define ADS1256_FSC_BYTE1_REGISTER 9
#define ADS1256_FSC_BYTE2_REGISTER 10  // MSB


#ifdef __cplusplus
}
#endif
#endif /* ADC_ADS1256_H_ */
