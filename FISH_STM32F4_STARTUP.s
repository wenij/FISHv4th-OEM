// FISH_STM32F4_STARTUP.s
//Valid thru v1.8
//----------------------------------------------------------------------
#ifdef USE_CMAIN
 PUBLIC RET2c
#endif

 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
 PUBLIC STM32Fx_COLD_FISH
 PUBLIC  __iar_program_start
__iar_program_start
// MAIN() entry point defined by #Defines in FISH_STM32F4_CONFIG_DEFINES.h
// means you can have c main() call FISH or not~!
// #define USE_CMAIN     // Affects cstartup_M.c STM32Fx_COLD_FISH and RET2c
// :NONAME FM0_COLD ( -- ) Reset Vector entry point. Setup FISH Virtual Machine.
STM32Fx_COLD_FISH:
// Initialize DICT RAM segment

	ldr	n, = 0x11111111	        // fill pattern
	ldr	t, = RAM_START          // START OF RAM WHERE DICT IS ALLOCATED
                                        // IN MEMMAP segment
	ldr	y, = RAM_INIT_END       // BEFORE UNINT SECTION IN MEMMAP
_fillRAM:
	str	n, [t]
	adds	t, t, #4
	cmp	t, y
	blo	_fillRAM
        
#ifdef USE_CMAIN
//	PUSH lr to sp for BYE
//	SUB	sp, sp, #4
//	MOV	t, lr
//	STR	t, [sp]
#endif

	LDR  	p, =PINIT	// my parameters
	LDR  	r, =RINIT	// p and r = FISH operating enviorment,
				// r = return stack.
// ADR works here because it loads a PC releative offset!
// Not a anywhere 32 bit addr - use LDR for that!

	ADR	i, FM4_WARM	// i is the interpreter pointer.
	PUSHi2r			// FM4_WARM is 5-6 lines down from here Go FISH!
	NEXT

 LTORG

//---------------------------------FM4_WARM-------------------------------------
// :NONAME FM4_WARM ( -- ) Start of FISH interpretation PRE-EXECUTION. then QUIT
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
FM4_WARM:
#ifdef TESTRAM
        DC32    flogRAM
#endif
        DC32	FWARM		        // FISH Init for WTEST code
        DC32	SoCinit		        // SYSCLK, systick, MS
        DC32    UART3_INIT

// TEST CODE GOES HERE - Pre FISH Execution Environment
WTEST:

// COPIES CODE TO RAM - CHECK FISH CODE FOR THIS KIND OF RELOCATION!
// RELEATIVELY LINKED PRIOR DEFNITIONS!
//        DC32    FLASH_TEST_CODE_IN_RAM
// FLASH OPS CAN BE PERFORMED FROM FLASH WITH POTENTIAL WAIT STATES
//        DC32    FLASH_TEST_CODE_IN_FLASH      // EXECUTES IN FLASH

// TEST CODE END
FWARM_STARTING_UP:
//        DC32    DOTS            // SHOW ANY STACK ARTIFACTS HERE
        DC32	CR
	DC32	LIT, 0xFB, EMIT // ANSI ASCII CheckMark
	DC32	LIT, 0xF7, EMIT // ANSI ASCII 2 wavy's
	DC32	CR
	DC32	COLD	        // WARM ABORT THEN QUIT

#ifdef USE_CMAIN
	DC32	RET2c	// shouldnt get here, return to c main and restart
#endif
//------------------------ for meta-single-stepping ----------------------------
//:NONAME ssNEXT ( -- ) System Internal hi level breakpoint.
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
ssNEXT1:
	LDM	w!, {x} // contents of cfa, (pfa), -> x, bump w to cfa+4
	MOV     pc, x	// w preserves cfa+4 (pfa) for DOCOL's benefit

//---------------------------- SYSTICK ISR -------------------------------------
// STI_ON: 7 E000E010h !  STI_OFF: 5 E000E010h !
// Restore lr to pc, save all working registers!

 PUBLIC FMx_SYSTICK_ISR
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
FMx_SYSTICK_ISR:
// Start of the working asm isr-------------------------------------------------
// save what you use
        PUSH    { t, n, lr}
        LDR     n, = STICKER
        LDR     t, [n]
        ADDS    t, t, #1
        STR     t, [n]
// restore what was being used
        POP     { t, n, pc } // exec lr -> pc
// End of the working asm isr---------------------------------------------------
 LTORG
 
// LABEL FOR HI-LEVEL WORD ISR CREATE WORD!
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
ISR_SEMIS:
	DC32	.+5 // Test of re-useable 
 SECTION .text : CODE (2)
	POPr2i	// SEMIS to balance DOCOL!!!
        POP     { r0-r5, r10-r12, pc }

#if 0
// Start of the Hi-Level WORD ISR Test------------------------------------------
// The Hi-Level WORD ISR Pre amble.
DOCOL_ISR:
        PUSH    { r0-r5, r10-r12, lr }
	LDR	w,  [PC, #0XC] //= MY_LTORG The High Level Target LTORG Label.

	NEXT1   // -> SEMIS_ISR RETURN required instead of SEMIS!

// The Hi-Level WORD ISR Post amble.
MY_LTORG_ISR_SEMIS:
// .+8 = correct target DOCOL entry to Hi Level WORD Label because of LTORG
	DC32	.+8 // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//>>>>>>>>>>>>>>THIS IS WHERE HI-LEVEL WORD CFA AS LTORG GOES!!!!!!!!!!!!!!!!!!!
        DC32    NONAME_STCTR_INCR
//>>>>>>>>>>>>>>THIS IS WHERE HI-LEVEL WORD CFA AS LTORG GOES!!!!!!!!!!!!!!!!!!!
 SECTION .text : CODE (2)
	POPr2i	// SEMIS to balance DOCOL!!!
        POP     { r0-r5, r10-r12, pc }

// The test Hi-Level target WORD.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
NONAME_STCTR_INCR:
        DC32    DOCOL
        DC32    ONE, STCTR, PSTORE      // Incr STICKER
//        DC32    STCTR, QUES   // Works showing longer execution can be done.
        DC32    MY_LTORG_ISR_SEMIS
// End of the Hi-Level WORD ISR Test--------------------------------------------
// NOTE: LTORG manual in this section!
#endif

/*
//-------------------------- ISR's and inline ASM ------------------------------
//:NONAME ISR_FISH Hi level call of FISH word from ISA
//      Save all regs and index a table of CFA's = execute if non-zero

//:NONAME ILA Inline Assembly code execution
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
ILA:
	DC32	DOCOL
        DC32    NOOP
//	DC32	ASM_START	// HW ISSUE
//	NOP
//      DC32    ASM_END
	DC32	SEMIS
*/
