// FISH STM32F4 DISCO

// for stm32f4xx.h usage
#define STM32F40XX
//Without this:
// Fatal Error[Pe035]: #error directive: 
//      "Please select first the target STM32F4xx device
//      used in your application (in stm32f4xx.h file)" 

// For CMSIS function usage
#include "stm32f4xx.h"
#include "core_cm4.h"     // For CMSIS NVIC_SystemReset function


#pragma call_graph_root = "interrupt"
__weak void HardFault_Handler( void ) {

// Don't use printf or other large lib constructs, use your own or tool debug io  
  NVIC_SystemReset();
}
//__weak void HardFault_Handler( void ) { FM3_COLD(); }

void C_CMSIS_DISABLE_IRQS() {
  __disable_irq();
}

void C_CMSIS_ENABLE_IRQS() {
  __enable_irq();
}

/*
void C_CMSIS_REV(unsigned long word)
{
// prototype is:
// unsigned long __REV(unsigned long);
// This works if word is passed in r0
  __REV(word);
// Results is returned in r0
}
*/


#ifdef USE_CMAIN // defined in FISH_STM32F4_CONFIG_DEFINES.h
// May use this to prototype Smart-IO board attached to the fishy disco~!
int main(void) {

 extern void FM3_COLD();
	volatile static int i = 0 ;
	while(1) {
		FM3_COLD();		// FISH_return2c returns here
		i++ ;
	}
	return 0 ;
}
#endif