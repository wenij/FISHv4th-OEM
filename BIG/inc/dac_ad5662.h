/*
 * dac_ad5662.h
 *
 *  Created on: Jan 16, 2019
 *      Author: carstenh
 */

#include <stdint.h>
#include <stdbool.h>

#ifndef DAC_AD5662_H_
#define DAC_AD5662_H_
#ifdef __cplusplus
 extern "C" {
#endif

#define DAC_MAX 0xFFFF
#define DAC_MIN 0

 extern bool AD5662_Set(uint16_t DacValue);

 extern void SetSpiMode(bool direct);


#ifdef __cplusplus
}
#endif
#endif /* DAC_AD5662_H_ */
