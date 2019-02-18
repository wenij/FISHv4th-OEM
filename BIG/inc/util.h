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
  *  NumToString
  *    Converts a numeric value to a string
  *
  *  Parameters
  *    input value
  *    numbytes Number of bytes
  *    Pointer to buffer in which the output should be placed
  *    Size of buffer
  *    Hex - true if output should be hexadecimal. false will result in a decimal conversion
  *
  *  Returns
  *    Nothing
  */

 extern void NumToString( uint32_t value, int numbytes, char * output, uint32_t buffer_size, bool Hex);



#ifdef __cplusplus
}
#endif
#endif /* UTIL_H_ */
