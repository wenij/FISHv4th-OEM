/*
 * smartio_if.h
 *
 *  Created on: Nov 13, 2018
 *      Author: carstenh
 */

#ifndef SMARTIO_IF_H_
#define SMARTIO_IF_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "spi.h"
#include "cli.h"



// SIF message container
typedef struct
{

} SifMessageContainer;


extern void SifTask( void *params);


#endif /* SMARTIO_IF_H_ */
