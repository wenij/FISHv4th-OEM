// FISH_GPIO.h

// FISH_GPIO_WC.h
/* Update SET, CLR and ANDBIT(S):
BFC R4, #8, #12 ; Clear bit 8 to bit 19 (12 bits) of R4 to 0
BFI R9, R2, #8, #12 ; Replace bit 8 to bit 19 (12 bits) of R9 with
; bit 0 to bit 11 from R2
*/
/* File include explains how to make a new WORDCAT:
How to make it the current TOP of the Dictionary:
WORDCAT hOtO.TXT
// THe wordcat is at the end of the file.
// The words are defined ahead of the wordcat.
*/
//=============================== GPIO WORDs =================================//

// IMPLEMENT THE PORT B AND PORTC CONSTANTS

//	GPIOB_ODR: ( -- addr )
 SECTION .text : CONST (2)
GPIOB_ODR_NFA:
	DC8	0x80 +09d
	DC8	'GPIOB-OD'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_FISH_PubRel_NFA
RM_GPIOC_ODR:
  DC32  DOCON, GPIOB_ODR

//	GPIOB_IDR: ( -- addr )
 SECTION .text : CONST (2)
GPIOB_IDR_NFA:
	DC8	0x80 +09d
	DC8	'GPIOB-ID'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	GPIOB_ODR_NFA
RM_GPIOC_IDR:
  DC32  DOCON, GPIOB_IDR

//	GPIOB_MODER: ( -- addr )
 SECTION .text : CONST (2)
GPIOB_MODER_NFA:
	DC8	0x80 +011d
	DC8	'GPIOB-MODE'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	GPIOB_IDR_NFA
RM_GPIOC_MODER:
  DC32  DOCON, GPIOB_MODER

    
//	GPIOC_ODR: \ -- addr \  
 SECTION .text : CONST (2)
GPIOC_ODR_NFA:
	DC8	0x80 +09d
	DC8	'GPIOC-OD'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	WC_FISH_PubRel_NFA
//	DC32	WC_FISH_GPIO_NFA
  DC32 GPIOB_MODER_NFA
RM_SPIC_ODR:
  DC32  DOCON, GPIOC_ODR

//	GPIOC_IDR: \ -- addr \  
 SECTION .text : CONST (2)
GPIOC_IDR_NFA:
	DC8	0x80 +09d
	DC8	'GPIOC-ID'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	GPIOC_ODR_NFA
RM_SPIC_IDR:
  DC32  DOCON, GPIOC_IDR

//	GPIOC_MODER: ( -- addr )
 SECTION .text : CONST (2)
GPIOC_MODER_NFA:
	DC8	0x80 +011d
	DC8	'GPIOC-MODE'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	GPIOC_IDR_NFA
RM_SPIC_MODER:
  DC32  DOCON, GPIOC_MODER


//  SEEBIT  ( 0-based-bit# adrr -- )
/*
: SEEBIT@ ( 1-based-bit# addr -- )
	SWAP \ MAKES STACK ARGS SAME AS SETBIT/CLRBIT
	1 SWAP LSL >R @ R AND R> AND IF ." ON" ELSE ." OFF" THEN ;

*/
 SECTION .text : CONST (2)
SEEBIT_NFA:
	DC8	0x80+6
	DC8	'SEEBI'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32	GPIOC_MODER_NFA
SEEBIT:
  DC32  DOCOL
  DC32  TOR, ONE, SWAP, LSL
  DC32  RFROM, AT, ANDD
  DC32  ZBRAN
      DC32  BITOFF-.
BITON:
  DC32	PDOTQ
	DC8	2
	DC8	'O', 'N'
 ALIGNROM 2,0xFFFFFFFF
  DC32    SEMIS

BITOFF:
  DC32	PDOTQ
	DC8	3
	DC8	'O', 'F', 'F'
 ALIGNROM 2,0xFFFFFFFF
  DC32  SEMIS


/*  SETBIT ( 0-based-bit# adrr -- )
: SB
	>R              \ STORE @ADDR VALUE
	1h SWAP LSL
	R  @ OR
	R>
\	CR .SH
	!
;
*/
//  SETBIT  ( 0-based-bit# adrr -- )
 SECTION .text : CONST (2)
SETBIT_NFA:
	DC8	0x80+6
	DC8	'SETBI'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32	SEEBIT_NFA
SETBIT:
	DC32	.+5
 SECTION .text : CODE (2)
	POPp2w      // val
	POP2n_r1 		// addr
	LDR   t_r0, [w_r2]      // read [val]
  MOV   x_r3, #1
  LSLS   x_r3, n_r1
	ORRS	t_r0, t_r0, x_r3  // modify val
	STR  t_r0, [w_r2]       // Write val
	NEXT

/*
 CLRBIT ( 0-based-bit# addr -- )
: CB
	>R              \ STORE @ADDR VALUE
	1h SWAP LSL
	R  @ NOT AND
	R>
\	CR .SH
	!
;
*/

//  CLRBIT  ( 0-based-bit# adrr -- )
 SECTION .text : CONST (2)
CLRBIT_NFA:
	DC8	0x80+6
	DC8	'CLRBI'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32	SETBIT_NFA
CLRBIT:
	DC32	.+5
 SECTION .text : CODE (2)
	POPp2w                  // addr
	POP2n_r1 		            // val	
	LDR   t_r0, [w_r2]      // read [val]
  MOV   x_r3, #1
  LSLS   x_r3, n_r1
	BIC	t_r0, t_r0, x_r3    // modify val
	STR   t_r0, [w_r2]      // Write val
	NEXT

//	ANDBITS ANDBITS:	( addr val -- )

 SECTION .text : CONST (2)
ANDBITS_NFA:
	DC8	0x87
	DC8	'ANDBIT'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CLRBIT_NFA
ANDBITS:
	DC32	.+5
 SECTION .text : CODE (2)
	POPp2w      // val
	POP2n_r1 		// addr
	LDR   t_r0, [n]	      // read [val]
	ANDS	t_r0, t_r0, w	  // modify val
	STR	  t_r0, [n]	      // Write val
	NEXT

//	SETBITS SETBITS:	( addr val -- )
//	OR val bits into addr. See also CLRBITS.
//	Preserves bits at addr - useful for ARM SoC initialization.

 SECTION .text : CONST (2)
SETBITS_NFA:
	DC8	0x87
	DC8	'SETBIT'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ANDBITS_NFA
SETBITS:
	DC32	.+5
 SECTION .text : CODE (2)
	POPp2w              // val w_r2
	POP2n_r1        		// addr n_r1
	LDR   t_r0, [n]     // read[val]t_r0
	ORRS	t_r0, t_r0, w	// modify val
	STR	  t_r0, [n]	    // Write val
	NEXT


//	CLRBITS CLRBITS:	( addr val -- )
//	XOR val bits into addr. See also SETBITS.
//	Preserves bits at addr - useful for ARM SoC initialization.

 SECTION .text : CONST (2)
CLRBITS_NFA:
	DC8	0x87
	DC8	'CLRBIT'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SETBITS_NFA
CLRBITS:
	DC32	.+5
 SECTION .text : CODE (2)
	POPp2w                    // val
	POP2n_r1 		              // addr
	LDR	  t_r0, [n_r1]        // read [val]
  BICS  t_r0, t_r0, w_r2    // modify val  - AND-NOT
	STR	  t_r0, [n_r1]		    // write val
	NEXT


//  PC0	GPIO_Output	GS0
//	GS0:	( -1 | 0 -- ) Use ON and OFF
 SECTION .text : CONST (2)
GS0_NFA:
	DC8	0x80+3
	DC8	'GS'
	DC8	'0'+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32  CLRBITS_NFA
GS0: // BIT 01h IN GPIOC_ODR - PC0 GS0
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    GS0_OFF-.
GS0_ON:
    DC32    LIT, 0d, LIT, GPIOC_ODR, SETBIT
    DC32    SEMIS
GS0_OFF:
    DC32    LIT, 0d, LIT, GPIOC_ODR, CLRBIT
    DC32    SEMIS

//  PC1	GPIO_Output	GS1 POTC BOT 0
//	GS1:	( -1 | 0 -- ) Use ON and OFF
 SECTION .text : CONST (2)
GS1_NFA:
	DC8	0x80+3
	DC8	'GS'
	DC8	'1'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	GS0_NFA
GS1: // BIT 0h IN GPIOC_ODR - PC0 GS0
		DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    GS1_OFF-.
GS1_ON:
    DC32    LIT, 1d, LIT, GPIOC_ODR, SETBIT
    DC32    SEMIS
GS1_OFF:
    DC32    LIT, 1d, LIT, GPIOC_ODR, CLRBIT
    DC32    SEMIS

//  PC2	GPIO_Output	GS2
//	GS2:	( -1 | 0 -- ) Use ON and OFF
 SECTION .text : CONST (2)
GS2_NFA:
	DC8	0x80+3
	DC8	'GS'
	DC8	'2'+0x80
 ALIGNROM 2,0xFFFFFFFF
	//DC32	WC_FISH_PubRel_NFA
	DC32	GS1_NFA
GS2: // BIT 01h IN GPIOC_ODR - PC0 GS0
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    GS2_OFF-.
GS2_ON:
    DC32    LIT, 2d, LIT, GPIOC_ODR, SETBIT
    DC32    SEMIS
GS2_OFF:
    DC32    LIT, 2d, LIT, GPIOC_ODR, CLRBIT
    DC32    SEMIS


//  PC9	GPIO_Output	SW4
//	SW4:	( -1 | 0 -- ) Use ON and OFF
 SECTION .text : CONST (2)
SW4_NFA:
	DC8	0x80+3
	DC8	'SW'
	DC8	'4'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	GS2_NFA
SW4: // BIT 0200h IN GPIOC_ODR - PC9 SW4
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    SW4_OFF-.
SW4_ON:
    DC32    LIT, 9d, LIT, GPIOC_ODR, SETBIT
    DC32    SEMIS
SW4_OFF:
    DC32    LIT, 9d, LIT, GPIOC_ODR, CLRBIT
    DC32    SEMIS

//  PC8	GPIO_Output	SW3
//	SW3:	( -1 | 0 -- ) Use ON and OFF
 SECTION .text : CONST (2)
SW3_NFA:
	DC8	0x80+3
	DC8	'SW'
	DC8	'3'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SW4_NFA
SW3: // BIT 0100h IN GPIOC_ODR - PC8 SW3
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    SW3_OFF-.
SW3_ON:
    DC32    LIT, 8d, LIT, GPIOC_ODR, SETBIT
    DC32    SEMIS
SW3_OFF:
    DC32    LIT, 8d, LIT, GPIOC_ODR, CLRBIT
    DC32    SEMIS

//  PC7	GPIO_Output	SW2
//	SW2:	( -1 | 0 -- ) Use ON and OFF
 SECTION .text : CONST (2)
SW2_NFA:
	DC8	0x80+3
	DC8	'SW'
	DC8	'2'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SW3_NFA
SW2: // BIT 080h IN GPIOC_ODR - PC7 SW2
	DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    SW2_OFF-.
SW2_ON:
    DC32    LIT, 7d, LIT, GPIOC_ODR, SETBIT
    DC32    SEMIS
SW2_OFF:
    DC32    LIT, 7d, LIT, GPIOC_ODR, CLRBIT
    DC32    SEMIS

//  PC6	GPIO_Output	SW1
//  SW1:	( -1 | 0 -- ) Use ON and OFF
/*
: SW1 ( ON | OFF -- )	\ BIT PC6
	IF 6 GPIOC-ODR SETBIT ELSE 6 GPIOC-ODR CLRBIT THEN ;

*/
 SECTION .text : CONST (2)
SW1_NFA:
	DC8	0x80+3
	DC8	'SW'
	DC8	'1'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SW2_NFA
SW1: // BIT 040h IN GPIOC_ODR - PC6 SW1
    DC32    DOCOL
// ZBRAN USES TOS ~ ON|OFF
    DC32  ZBRAN
    DC32    SW1_OFF-.
SW1_ON:
    DC32    LIT, 6d, LIT, GPIOC_ODR, SETBIT
    DC32    SEMIS
SW1_OFF:
    DC32    LIT, 6d, LIT, GPIOC_ODR, CLRBIT
    DC32    SEMIS

// .BT-CS	PB14	IS LEDGREEN
// .DAC-CS	PB13	IS LEDRED
// .ADC-CS	PB12	IS LEDBLUE


//  PB13	GPIO_Output	BT_DAC
// .DAC-CS	PB13	IS LEDRED
//	CS_DAC:	( -1 | 0 -- ) Chip select is -1 argument, TRUE
 SECTION .text : CONST (2)
CS_DAC_NFA:
	DC8	0x80+6
	DC8	'CS-DA'
	DC8	'C'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SW1_NFA
CS_DAC: // BIT 2000h IN GPIOB_ODR PB13 DAC-CSn
// TURN OFF OTHER CHIP SELECTS WHEN YOU TURN THIS ONE ON~!
//  CREATE BETTER CODE BY LEARNING HOW TO USE VIC'S RMW___ WORDS.
//	PB14	BT_CSn  4000h
//	PB13	DAC-CSn 2000h
//	PB12	ADC-CSn 1000h
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    CS_DAC_OFF-.
CS_DAC_ON:
    DC32    LIT, GPIOB_ODR, LIT, 04000h, SETBITS // SET CS-BT TO 1
    DC32    LIT, GPIOB_ODR, LIT, 02000h, CLRBITS // SET CS-DAC TO 0
    DC32    LIT, GPIOB_ODR, LIT, 01000h, SETBITS // SET CS-ADC TO 1
    DC32    SEMIS

CS_DAC_OFF:
    DC32    LIT, GPIOB_ODR, LIT, 02000h, SETBITS // SET CS-DAC TO 1
    DC32    SEMIS

//  PB14	GPIO_Output	BT_CSn
// .BT-CS	PB14	IS LEDGREEN
//	CS_BT:	( -1 | 0 -- ) Chip select ON is -1 argument.
 SECTION .text : CONST (2)
CS_BT_NFA:
	DC8	0x80+5d
	DC8	'CS-B'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CS_DAC_NFA
CS_BT:  // BIT 4000h IN GPIOB_ODR PB14 BT-CSn
// TURN OFF OTHER CHIP SELECTS WHEN YOU TURN THIS ONE ON~!
//  CREATE BETTER CODE BY LEARNING HOW TO USE VIC'S RMW___ WORDS.
//	PB14	BT_CSn  4000h
//	PB13	DAC-CSn 2000h
//	PB12	ADC-CSn 1000h
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    CS_BT_OFF-.
// TRUE CS-BT WORKS
CS_BT_ON:
    DC32    LIT, GPIOB_ODR, LIT, 04000h, CLRBITS // SET CS-BT TO 0
    DC32    LIT, GPIOB_ODR, LIT, 02000h, SETBITS // SET CS-DAC TO 1
    DC32    LIT, GPIOB_ODR, LIT, 01000h, SETBITS // SET CS-ADC TO 1
    DC32    SEMIS
CS_BT_OFF:
    DC32    LIT, GPIOB_ODR, LIT, 04000h, SETBITS // SET CS-BT TO 1
    DC32    SEMIS
//  PB12	GPIO_Output	ADC-CSn
// .ADC-CS	PB12	IS LEDBLUE
//	CS_ADC:	( -1 | 0 -- ) PB12 Chip select is -1 argument, TRUE
 SECTION .text : CONST (2)
CS_ADC_NFA:
	DC8	0x80+6
	DC8	'CS-AD'
	DC8	'C'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CS_BT_NFA
CS_ADC: // BIT 1000h IN GPIOB_ODR - PB12 ADC-CSn
// TURN OFF OTHER CHIP SELECTS WHEN YOU TURN THIS ONE ON~!
//  CREATE BETTER CODE BY LEARNING HOW TO USE VIC'S RMW___ WORDS.
//	PB14	BT_CSn  4000h
//	PB13	DAC-CSn 2000h
//	PB12	ADC-CSn 1000h
        DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    CS_ADC_OFF-.
CS_ADC_ON:
    DC32    LIT, GPIOB_ODR, LIT, 04000h, SETBITS // SET CS-BT TO 1
    DC32    LIT, GPIOB_ODR, LIT, 02000h, SETBITS // SET CS-DAC TO 1
    DC32    LIT, GPIOB_ODR, LIT, 01000h, CLRBITS // SET CS-ADC TO 0
    DC32    SEMIS
CS_ADC_OFF:
    DC32    LIT, GPIOB_ODR, LIT, 01000h, SETBITS // SET CS-ADC TO 1
    DC32    SEMIS

/* MOVED TO SPI WC - DELETE AFTER TESTED
//	DACWORD: ( -- addr ) A SYSTEM VAR - USE WITH C@'S THRU THE WORD
 SECTION .text : CONST (2)
PSTATDACDATA_NFA:
	DC8	0x80 +07d
	DC8	'DACWOR'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CS_ADC_NFA
PSTATDACDATA:
  DC32    DOCON, PSTAT_DAC_DATA
*/
// ?TRUE ( n -- f ) IF VALUE IS NONZERO RETURN -1 ELSE 0
 SECTION .text : CONST (2)
QTRUE_NFA:
  DC8	0x80 + 5d
	DC8	'?TRU'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CS_ADC_NFA
QTRUE:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t_r0                  // val
	cmp	t, #0                 //
  beq QOFF 
  EORS	t_r0, t_r0, t_r0
  SUBS	t_r0, t_r0,  #1
  PUSHt_r0
	NEXT
QOFF:    
  PUSHt_r0
	NEXT
    
//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_SYS: = FISH System: CATEGORY

 SECTION .text : CONST (2)
WC_FISH_GPIO_NFA:
	DC8	0x80+4+11d        // +4 is format chars constant
                                // +n is Name lenght
        DC8     0x0D, 0x0A
	DC8	'PSTAT GPIO:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32 QTRUE_NFA


/*

//=============================== GPIO -BIG-PSTAT-TEST-SW- WORDCAT ===============================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_GPIO: = FISH System: CATEGORY

 SECTION .text : CONST (2)
WC_FISH_BIG_PSTAT_GPIO_NFA:
	DC8	0x80+4+23        // +4 is format chars constant
                                // +n is Name lenght
    DC8     0x0D, 0x0A
    DC8	    'GPIO-BIG-PSTAT-TEST-SW:'
    DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPI1_LED_TEST_NFA       // Where this wordcat links in.

*/