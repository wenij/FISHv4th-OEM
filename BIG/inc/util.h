/*
 * util.h
 *
 *  Created on: Jan 29, 2019
 *      Author: carstenh
 */

#include <stdint.h>

#ifndef UTIL_H_
#define UTIL_H_

#ifdef __cplusplus
 extern "C" {
#endif

/*
 *  NumToHexString
 *    Converts a numeric value to a hexadecimal string
 *
 *  Parameters
 *    input value
 *    numbytes Number of bytes
 *    Pointer to buffer in which the output should be placed
 *    Size of buffer
 *
 *  Returns
 *    Nothing
 */

extern void NumToHexString( uint32_t value, int numbytes, char * output, uint32_t buffer_size);


/*
 *  NumToDecimalString
 *    Converts a numeric value to a decimal string
 *
 *  Parameters
 *    input value
 *    numbytes size in bytes
 *    Pointer to buffer in which the output should be placed
 *    Size of buffer
 *
 *  Returns
 *    Nothing
 */
extern void NumToDecimalString( uint32_t value, int numbytes, char * output, uint32_t buffer_size);



#ifdef __cplusplus
}
#endif
#endif /* UTIL_H_ */
