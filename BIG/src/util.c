/*
 * util.c
 *
 *  Created on: Jan 29, 2019
 *      Author: carstenh
 */

#include <stdlib.h>
#include <string.h>
#include "util.h"

void NumToHexString( uint32_t value, int numbytes, char * output, uint32_t buffer_size)
{
    int digits = numbytes * 2;

    // 2, 4 or 8 digits
    if (digits <= 2)
    {
        digits = 2;
        value &= 0xFF;
    }
    else if (digits <= 4)
    {
        digits = 4;
        value &= 0xFFFF;
    }
    else
    {
        digits = 8;
    }

    if (buffer_size >= digits * 2 + 1)
    {
        itoa( value, output, 16);
    }

}

void NumToDecimalString( uint32_t value, int numbytes, char * output, uint32_t buffer_size)
{
    int digits = numbytes * 2;

    if (digits <= 2)
    {
        digits = 4;
        value &= 0xFF;
    }
    else if (digits <= 4)
    {
        digits = 6;
        value &= 0xFFFF;
    }
    else
    {
        digits = 11;
    }

    if (buffer_size >= digits * 2 + 1)
    {
        itoa( value, output, 10);
    }

}

