// FISH STM32F4_EQUATES.h
// Valid thru v1.8
// #defines are placed in the Assembler Preproccesor Defined Symbols section.
// EQUATES are for REAL VALUES used in FISH assembly code
// 
DEFAULT_BAUD    EQU     9600            // 9600 until UART0_INIT USED 2580h
DEFAULT_BASE    EQU     10              // BASE at powerup
XON_CHAR        EQU     17              // ASCII DECIMAL XON_CHAR
XOFF_CHAR       EQU     19              // ASCII DECIMAL XOFF_CHAR
IOBUFSIZE       EQU     96              // For Tib and Pad
MAXWORDLEN	EQU     31              // magic!!! for ID. <<<<<<<<<<<<<<<<<<<<

#ifdef STM32F4_IRC16_16MHZ
SYSTEMCLOCK     EQU     16000000        // 
#endif
#ifdef STM32F4_IRC16_32MHZ
SYSTEMCLOCK     EQU     32000000        // 
#endif
#ifdef STM32F4_IRC16_48MHZ
SYSTEMCLOCK     EQU     48000000        // 
#endif
#ifdef STM32F205RC_XRC10_118MHZ
//SYSTEMCLOCK     EQU     210000000       // TEST NO PLL CHANGES FROM XRC08_168mhz
SYSTEMCLOCK     EQU     118000000       // 7088980
#endif
#ifdef STM32F4_XRC08_168MHZ
SYSTEMCLOCK     EQU     168000000       // A037A00
#endif

//-------------- System memory mapping equates and structures ------------------

// RAM RUNTIME ADDRS ARE DEINED BY LABELS IN MEMMAP SEGMENTS

FLASH_START     EQU     0x08000000
FLASH_SPAGE     EQU     0x08020000      // Reserve 128k for FISH
FLASH_PPAGE     EQU     0x08100000      // FLASH_SAVE USES THIS


// BATTERY BACKED RAM
BBRAM_START	EQU	40024000h
BBRAM_END	EQU	40025000h       // 4K - 4096
BBRAM           EQU     BBRAM_START

//-------------- System STACK AND UNINIT mapping equates -----------------------
// THIS SHOULD BE A STATIC LINK - AT END OF SRAM2...
// MAP: CSTACK$$Base          0x2001e000         --   Gb  - Linker created -
SRAM2_CSTACK     EQU     2001E000h       // 16K FOR CSTACK & HEAP ICF SETS AT END


