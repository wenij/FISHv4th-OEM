// FISH_STM32Fx_MEMMAP.s
// Valid thru v1.8
// STM32F407VG_SRAM128_TCM64
// STM32F205RC_SRAM128
// USE LINKER BETTER!!!
//------------------------------------------------------------------------------
// FISH FORTH Memory Map for the STM32Fx series
// Tested:
// STM32F407vg ARM Cortex M4F series SystemsOnaChip (SoC). STM32F4 Disco Brd.
// STP STM32F206RC RPM Brd.

// Create a new section for Temporary Word Dictionairy Section when ready
// WITH IT'S OWN BASE AND CURRENT POINTERS TO COMPILE INTO,
// A relocated transitory dictionary segment managed by system words.
// Transitory dictionaly segment can be created, say as SoC addr constants,
// used in compiling initialization routines, then un-linked after useage.
//  SECTION .tcm_pref_temp_dict : DATA(2)
//RAM_TEMP_DICT        DS8     48000
//RAM_TEMP_DICT_END:

// Create a new section for ISR's when ready
// WITH IT'S OWN BASE AND CURRENT POINTERS TO COMPILE INTO,
// THAT system words manage the creation/destruction of ISR WORDS.
//  SECTION .tcm_pref_isr_ramcode : DATA(2)
//RAM_ISR_AND_CODE        DS8     48000
//RAM_ISR_AND_CODE_END:

// Currently linked segements:
//  These need to be able to fit into 128K RAM chips!
// .sram_1rst_dict_stdio_bufs IS WHERE THE DICTIONAIRY AND AND I/O BUFFERS ARE
// .sram_2nd_intvecs_unint is where Ram intvecs and unitialized ram are.
// .tcm_pref_stacks_and_vars IS WHERE THE FISH STACKS, SYSTEM and USER VARS ARE.

//----------- Start of RAM = DICTIONAIRY AND Standard I/O Buffers --------------
   SECTION .sram_1rst_dict_stdio_bufs : DATA (2)
RAM_START:      // RAM_END IS BEFORE UNINT AREA IN .sram2_64K segment
SYSCLOCK        DS32    1               // MUST BE SET AT STARTUP = FULL MHZ #
FPA:            DS32    1               // FPADDR 0 = NO FLASH AVAILABLE
FPC:            DS32    1               // FPCURR
FPSV:           DS32    1               // FP System VAR
// The above 4 words are the SIGNATURE for saved code in a flash page.
// The FISH Dictionary starts 0x10 bytes after RBASE (RAMstart)
DICTSPACE_START:
ORIG:           DS8     64000		// Dictionary at beginning of RAM
DICTSPACE_END:                          // For DICTSPACE calculation
// Stdio Buffers
PAD:		DS8	IOBUFSIZE	// LINE LENGTH
TIB:		DS8	IOBUFSIZE       // LINE LENGTH
TIB_DMA         DS8     1024
TIB_DMA_BUF_END:
// FLASH_CODE COMMAND BLOCK IN RAM FOR FLASH PROGRAMMING
FLASH_CODE_SRAM2_START:
FLASH_CODE_RS   DS8     68
FLASH_CODE_SRAM2_END:

//------------------- SRAM2_64K Ram intvecs and unitialized ram STRUCTURES ----------------
   SECTION .sram_2nd_intvecs_unint : DATA (2)

RAM_INTVECS             DS8     128*4
RAM_INTVECS_END:
RAM_INIT_END:                // end of 1's fill

ALL_UNINIT_SRAM2_START: EQU     .
// Define use of this resource.
// DBAUD and UBAUD USED FOR SAVING BAUDRATE BETWEEN RESETS AND HARD FAULT RESETS
DBAUD           DS32    1
UBAUD           DS32    1
// Free unitialized ram
ALL_UNINIT_SRAM2_FREE_START:    EQU     .
// NEEDS TO STOP before block CSTACK, block HEAP WHICH IS DYNAMIC NOW!
ALL_UNINIT_SRAM2_END            EQU     SRAM2_CSTACK

//-------------- RAM Stack, system and use vars STRUCTURES ---------------------
  SECTION .tcm_pref_stacks_and_vars : DATA(2)
STICKER:                DS32    1       // SYSTICK INTERRUPT ACCUMULATOR
// For VARSPACE calculation
RAMVARSPACE_START:
RAMVARSPACE:            DS8     64*4    // VAR's are in RAM
END_RAMVARSPACE_SV_START:
SV_START:
 ALIGNRAM 2
// REARRANGE LATER FOR TASKING AND SIZE BASED OFFSETS!!
// BYTE VALUES
NBASE:          DS32    1       // BASE_SV for number conversion
IN:             DS32    1       // IN_SV Offset into tib
OUT:            DS32    1       // OUT_SV Offset into pad or terminal string
CSTATE:         DS32    1       // STATE_SV Compile STATE 0 = 
// ADDRESS VALUES initialed in FWARM:
INITSO:         DS32    1       // INITS0_SV Parameter Stack Addr
INITRO:         DS32    1       // INITR0_SV Return Stack Addr
INITTIB:        DS32    1       // TIB_SV TIB Addr
UP:             DS32    1       // UP_SV See VARALLOT
DP:         	DS32    1	// DP_SV IS See LATEST, COMMA, ALLOT AND CALLOT
CSDP            DS32    1       // Create saves DP here to restore if errors 
FENCE:      	DS32    1       // FENCE_SV See FORGET
CURRENT:        DS32    1       // CURRENT_SV See LATEST
PROMPT:         DS32    1       // Init in FWARM TO msg_MY_OK, $ OR 0 (QUIT)
ERROR_HALT:	DS32	1       // Init in FWARM & EHOFF to 0 and EHON to 1
SV_END:
// THESE ARE INITIALIZED WHEN USED AT RUN-TIME
CSP:            DS32    1       // CSP_SV SCSP saves stack pos here
NDPL:           DS32    1       // DPL_SV for number conversion
NHLD:           DS32    1       // HLD_SV for number conversion
// FISH Stacks
STACKSIZE       EQU     32*4            // SHOULD BE GOOD
RPAD            DS32    4
BOTTOM_RSTACK   DS32    STACKSIZE	// END OF RETURN STACK ADDR
RINIT	        EQU     .              	// RETURN STACK register r
SPAD            DS32    4              // IF IN CCM UNDERFLOW = RESERVED MMAP
BOTTOM_PSTACK   DS32    STACKSIZE
PINIT	        EQU     .              // PARAMETER STACK register p
PSTART          EQU     .

