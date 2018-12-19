/* Copyright 2017 ImageCraft Creations Inc., All rights reserved.
 * Smart.IO Host Interface Layer
 * https://imagecraft.com/smartio/
 */
#define STM32F40XX
#include <stm32f4xx.h>

// Change to "if 0" if you do not want to call printf when there is a fault
#if 1
#define PRINTF(fmt, ...)  printf(fmt, __VA_ARGS__)
#else
#define PRINTF(fmt, ...)
#endif

#define HANDLER(handler) void handler ##_Handler(void) { PRINTF("%s\n", #handler); while (1); }

//HANDLER(SVC)
HANDLER(NMI)
HANDLER(MemManage)
HANDLER(BusFault)
HANDLER(UsageFault)
HANDLER(DebugMon)
//HANDLER(PendSV)
// HANDLER(HardFault)


void yHardFault_Handler(unsigned PC, int arg, unsigned PSR)
    {
    PRINTF("Hard Fault PC %x arg %d PSR %x HFSR %x CFSR %x\n", PC, arg, PSR, SCB->HFSR, SCB->CFSR);
    }

