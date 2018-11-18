// FISH #defines for system wide control
// Clock defines are in the asm preproccesor

// Configuration:
//#define FISH_Debug_WORDSET    // Affecs Prompt only

// this define must keep RESET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//#define USE_CMAIN     // Affects cstartup_M.c FM0_COLD and RET2c

// System:
//#define PSTAT_SPI1_TEST
#define PSTAT_BB_NOINIT_TEST    // This is for the Bit Bang where SCLK 1 sets MOSI
//#define PSTAT_BB_TEST
#define TRUE_EQU_NEG_ONE
#define XON_XOFF        // Use XON_XOFF Flow Control
#define ABORT_STOP_TILL_CO
#define EOL_DLE         // DLE 0x10/16d in QUIT after last line interpreted
#define EOL_NAK         // NAK in error messages for STDLED editor highlight
//#define DEFAULT_BAUD_TEST
//#define CTS_RTS       // Use Hardware Flow Control
//#define IO2TP         // for ports before i/o done - enable PADCLR and TIBCLR
//#define TOSCT         // TOS Cache Testing QUERY BACKSPACING OVER INPUT!!!
//#define SLOW_POWERUP  // For slow power supply
//#define CLKOUT        // PIO0_1 is CLKOUT
//#define TESTRAM       // run flogram at start-up
//#define DEBUG_FLASH   // 

//#define IOBUFS_DEBUG
//IOBUFS_DEBUG #define:
//NOOP & .SH IN EXPECT AFTER LEAVE 
//TIB DUMP OF EXPECT IN QUERY
//HERE DUMP IN INTERPRET'S DFIND
//WORD PARSES USING SPACE - DOES SPACE AFTER? 

// VTOR_PATCH defined in 205 ASM PREPROC CONFIG - UNTIL 205 VTOR issue RESOLVED.

// COPY OF ALL CONFIGURATION Assembly Preprocessor sections:

/*
STM32F205RC-STM118mhg Configuration Assembly Preprocessor Section:
Assembly Preprocessor Defined Symbols:
FISH_Debug_WORDSET      // Affects prompt only
VTOR_PATCH              // 205 specific
STM32F205RC
STM32F4_FLASH_1_MB
STM32F205RC_SRAM128
STM32F205RC_XRC10_118MHZ
*/
/*
STM32F4_PubRel Configuration Assembly Preprocessor Section:
Assembly Preprocessor Defined Symbols:
STM32F4_XRC08_168MHZ
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Defined Symbols:
FISH_Debug_WORDSET
STM32F4_XRC08_168MHZ
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
/*
STM32F4_DEBUG Configuration Assembly Preprocessor Section:
Assembly Preprocessor Additional Include Directories:

Assembly Preprocessor Defined Symbols:
*/
