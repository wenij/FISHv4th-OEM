// FISH_GPIO.h

// FISH_GPIO_WC.h

// All spi1 chip selects are active low, INIT'D HIGH in SOC_INIT.
//

/* File include explains how to make a new WORDCAT:
How to make it the current TOP of the Dictionary:
$Wordcats_HowTo.c
// THe wordcat is at the end of the file.
// The words are defined ahead of the wordcat.

// BTW the flashloader demo is here:
// https://www.st.com/en/development-tools/flasher-stm32.html
*/
//=============================== GPIO WORDs =================================//

/* : SETBIT ( 0-based-bit# adrr -- )
	>R              \ STORE @ADDR VALUE
	1h SWAP LSL
	R  @ XOR        \ WHAT IS IT? IT'S NOT AND!
	R>
\	CR .SH
	!
;

: CLRBIT ( 0-based-bit# addr -- )
	>R              \ STORE @ADDR VALUE
	1h SWAP LSL
	R  @ XOR        \ WHAT IS IT? IT'S NOT AND!
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
  DC32	WC_FISH_PubRel_NFA
SETBIT:
  DC32  DOCOL
  DC32  TOR, ONE, SWAP, LSL
  DC32  R, AT, XORR, RFROM, STORE
  DC32  SEMIS

//  CLRBIT  ( 0-based-bit# adrr -- )
 SECTION .text : CONST (2)
CLRBIT_NFA:
	DC8	0x80+6
	DC8	'CLRBI'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32	SETBIT_NFA
CLRBIT:
  DC32  DOCOL
  DC32  TOR, ONE, SWAP, LSL
  DC32  R, AT, XORR, RFROM, STORE
  DC32  SEMIS

//  PC0	GPIO_Output	GS0
//	GS0:	( -1 | 0 -- ) Use ON and OFF
 SECTION .text : CONST (2)
GS0_NFA:
	DC8	0x80+3
	DC8	'GS'
	DC8	'0'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    CLRBIT_NFA
GS0: // BIT 01h IN GPIOC_ODR - PC0 GS0
//
//
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    GS0_OFF-.
GS0_ON:
    DC32    LIT, GPIOC_ODR, LIT, 01h, SETBITS
    DC32    SEMIS
GS0_OFF:
    DC32    LIT, GPIOC_ODR, LIT, 01h, CLRBITS
    DC32    SEMIS

//  PC1	GPIO_Output	GS1
//	GS1:	( -1 | 0 -- ) Use ON and OFF
 SECTION .text : CONST (2)
GS1_NFA:
	DC8	0x80+3
	DC8	'GS'
	DC8	'1'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	GS0_NFA
GS1: // BIT 01h IN GPIOC_ODR - PC0 GS0
//
//
		DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    GS1_OFF-.
GS1_ON:
    DC32    LIT, GPIOC_ODR, LIT, 010b, SETBITS
    DC32    SEMIS
GS1_OFF:
    DC32    LIT, GPIOC_ODR, LIT, 010b, CLRBITS
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
//
//
  DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    GS2_OFF-.
GS2_ON:
    DC32    LIT, GPIOC_ODR, LIT, 0100b, SETBITS
    DC32    SEMIS
GS2_OFF:
    DC32    LIT, GPIOC_ODR, LIT, 0100b, CLRBITS
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
//
//
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    SW4_OFF-.
SW4_ON:
    DC32    LIT, GPIOC_ODR, LIT, 0200h, SETBITS
    DC32    SEMIS
SW4_OFF:
    DC32    LIT, GPIOC_ODR, LIT, 0200h, CLRBITS
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
//
//
    DC32    DOCOL
// ZBRAN USES TOS
    DC32  ZBRAN
    DC32    SW3_OFF-.
SW3_ON:
    DC32    LIT, GPIOC_ODR, LIT, 0100h, SETBITS
    DC32    SEMIS
SW3_OFF:
    DC32    LIT, GPIOC_ODR, LIT, 0100h, CLRBITS
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
    DC32    LIT, GPIOC_ODR, LIT, 080h, SETBITS
    DC32    SEMIS
SW2_OFF:
    DC32    LIT, GPIOC_ODR, LIT, 080h, CLRBITS
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

//  PB12	GPIO_Output	ADC-CSn
//	CS_ADC:	( -1 | 0 -- ) PB12 Chip select is -1 argument, TRUE
 SECTION .text : CONST (2)
CS_ADC_NFA:
	DC8	0x80+6
	DC8	'CS-AD'
	DC8	'C'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SW1_NFA
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
    DC32    LIT, GPIOB_ODR, LIT, 04000h, SETBITS // SET BT-CSn TO 1
    DC32    LIT, GPIOB_ODR, LIT, 02000h, SETBITS // SET DAC-CSn TO 1
    DC32    LIT, GPIOB_ODR, LIT, 01000h, CLRBITS // SET ADC-CSn TO 0
    DC32    SEMIS
CS_ADC_OFF:
    DC32    LIT, GPIOB_ODR, LIT, 01000h, SETBITS // SET ADC-CSn TO 1
    DC32    SEMIS

//      PB13	GPIO_Output	BT_DAC
//	CS_DAC:	( -1 | 0 -- ) Chip select is -1 argument, TRUE
 SECTION .text : CONST (2)
CS_DAC_NFA:
	DC8	0x80+6
	DC8	'CS-DA'
	DC8	'C'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CS_ADC_NFA
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
    DC32    LIT, GPIOB_ODR, LIT, 04000h, SETBITS // AND BT-CSn TO 1
    DC32    LIT, GPIOB_ODR, LIT, 02000h, CLRBITS // SET DAC-CSn TO 0
    DC32    LIT, GPIOB_ODR, LIT, 01000h, SETBITS // SET ADC-CSn TO 1
    DC32    SEMIS

CS_DAC_OFF:
    DC32    LIT, GPIOB_ODR, LIT, 02000h, SETBITS // SET DAC-CSn TO 1
    DC32    SEMIS

//  PB14	GPIO_Output	BT_CSn
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
    DC32    LIT, GPIOB_ODR, LIT, 04000h, CLRBITS // AND BT-SC TO 0
    DC32    LIT, GPIOB_ODR, LIT, 02000h, SETBITS // SET DAC-SCn TO 1
    DC32    LIT, GPIOB_ODR, LIT, 01000h, SETBITS // SET ADC-CSn TO 1
    DC32    SEMIS
CS_BT_OFF:
    DC32    LIT, GPIOB_ODR, LIT, 04000h, SETBITS // SET BT-CSn TO 1
    DC32    SEMIS

/*
//      SPI1_LED_TEST_BANNER:	( -- )
 SECTION .text : CONST (2)
SPI1_LED_TEST_BANNER_NFA:
    DC8	0x80 + 20
    DC8	'SPI1-LED-TEST-BANNE'
    DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32        CS_BT_NFA
SPI1_LED_TEST_BANNER:
    DC32    DOCOL
    DC32    LIT, SPI1_LED_TEST_BANNER_$, NULLSTRLEN, TYPE
    DC32    SEMIS
*/
/*
//	?_SPI_LEDS_OFF:	( -- )
 SECTION .text : CONST (2)
?_SPI1_LEDS_OFF_NFA:
    DC8	0x80 + 14
    DC8	'?_SPI_LEDS_OF'
    DC8	'F'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32	SPI1_LED_TEST_BANNER_NFA
?_SPI1_LEDS_OFF:
    DC32    DOCOL
    DC32    LIT, ?_SPI1_LEDS_OFF_$, NULLSTRLEN, TYPE
    DC32    SEMIS
//  ?_SPI_DAC_LED_ON: ( -- )
 SECTION .text : CONST (2)
?_SPI1_DAC_LED_ON_NFA:
    DC8	0x80 + 16
    DC8	'?_SPI_DAC_LED_O'
    DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32	?_SPI1_LEDS_OFF_NFA
Q_SPI_DAC_LED_ON:
    DC32    DOCOL
    DC32    LIT, ?_SPI1_CS_DAC_LED_ON_$, NULLSTRLEN, TYPE
    DC32    SEMIS

//  ?_SPI_BT_LED_ON: ( -- )
 SECTION .text : CONST (2)
?_SPI1_BT_LED_ON_NFA:
    DC8	0x80 + 15
    DC8	'?_SPI_BT_LED_O'
    DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32	SPI1_LED_TEST_BANNER_NFA
Q_SPI_BT_LED_ON:
    DC32    DOCOL
    DC32    LIT, ?_SPI1_CS_BT_LED_ON_$, NULLSTRLEN, TYPE
    DC32    SEMIS

//  ?_SPI_ADC_LED_ON: ( -- )
 SECTION .text : CONST (2)
?_SPI1_ADC_LED_ON_NFA:
    DC8	0x80 + 16
    DC8	'?_SPI_ADC_LED_O'
    DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32	?_SPI1_BT_LED_ON_NFA
Q_SPI_ADC_LED_ON:
    DC32    DOCOL
    DC32    LIT, ?_SPI1_CS_ADC_LED_ON_$, NULLSTRLEN, TYPE
    DC32    SEMIS

//  ?_SPI_DAC_GND_INPUT_$: ( -- )
 SECTION .text : CONST (2)
?_SPI1_DAC_GND_INPUT_NFA:
    DC8	0x80 + 19
    DC8	'?_SPI_DAC_GND_INPU'
    DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32	?_SPI1_ADC_LED_ON_NFA
Q_SPI_DAC_GND_INPUT:
    DC32    DOCOL
    DC32    LIT, ?_SPI1_DAC_GND_INPUT_$, NULLSTRLEN, TYPE
    DC32    SEMIS

// ?YN ~ CONTROL THE key AND FLAG STACK RETURN
// ?YN   ( key -- FLAG key FLAG: 0 FOR N AND -1 FOR Y key)
 SECTION .text : CONST (2)
?YN_NFA:
    DC8	0x80 + 3
    DC8	'?Y'
    DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32	SPI1_LED_TEST_BANNER_NFA
?YN:
    DC32    DOCOL
//    DC32    DUP                         // ( key -- )
    DC32    DUP, TOR                    // ( key -- )
    DC32    LIT, 0059h, EQUAL           // ( key FLAG -- )
    DC32    RFROM, LIT, 004Eh, EQUAL    // ( key FLAG FLAG -- )
    DC32    OR                          // ( -- key YN-FLAG )
    DC32    SEMIS

// QA-Y   ( -- ) GET THE Y KEY ONLY
 SECTION .text : CONST (2)
QA_Y_NFA:
    DC8	0x80 + 4
    DC8	'QA-'
    DC8	'Y'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32    ?YN_NFA
QA_Y:
    DC32    DOCOL
    DC32    ZERO
//  DC32 DOTSHEX
QA_Y_BEGIN:    // DROP KEY EVERY UNTIL LOOP OR EMIT WHEN DONE
    DC32    DROP
// DC32 NOOP
	DC32	KEY	// DO/CO 03 IN RXRDY - GOOD IS DIFF !?!?!?!
//     DC32    SEMIS
    DC32    DUP         // ( key key -- )
  DC32 DOTSHEX

    DC32    LIT, 59h, SUBB

  DC32 DOTSHEX
    DC32    NOT
  DC32 DOTSHEX
    DC32    ZBRAN
    DC32      QA_Y_BEGIN-.
    DC32    EMIT
    DC32    SEMIS

// QA-YN   ( -- ) GET THE Y OR NO KEY ONLY, USING ?YN
 SECTION .text : CONST (2)
QA_YN_NFA:
    DC8	0x80 + 5
    DC8	'QA-Y'
    DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32    QA_Y_NFA
QA_YN:
    DC32    DOCOL
    DC32    ZERO
  DC32 DOTSHEX
QA_YN_BEGIN:    // DROP KEY EVERY UNTIL LOOP OR EMIT WHEN DONE
    DC32    DROP
// DC32 NOOP
	DC32	KEY	// DO/CO 03 IN RXRDY - GOOD IS DIFF !?!?!?!
//      DC32    SEMIS
    DC32    DUP         // ( key key -- )
  DC32 DOTSHEX
///*
    DC32    ?YN         // <?YN ( key -- FLAG )> ( key FLAG -- )
  DC32 DOTSHEX
    DC32    NOT
  DC32 DOTSHEX
    DC32    ZBRAN
    DC32      QA_YN_BEGIN-.
    DC32    EMIT
    DC32    SEMIS
*/
// SPI1-DAC-BB   ( -- )
 SECTION .text : CONST (2)
SPI1_DAC_BB_NFA:
    DC8	0x80 + 11d
    DC8	'SPI1-DAC-B'
    DC8	'B'+0x80
 ALIGNROM 2,0xFFFFFFFF
    DC32    CS_BT_NFA
SPI1_DAC_BB:
    DC32    DOCOL
    DC32    LIT, 2000, MS
SPI1_DAC_BB_BEGIN:
//	PB14	BT_CSn  4000h
//	PB13	DAC-CSn 2000h
//	PB12	ADC-CSn 1000h
//  The SPI1_DR is 16 bits wide
/*
    DC32    TRUE_NEG_1, CS_DAC
    DC32    strkk, 0000h, SPI1_DR
    DC32    ZERO, CS_DAC
*/
    DC32    TRUE_NEG_1, CS_BT
    DC32    LIT, 3000, MS
    DC32    ZERO, CS_BT

//	PB14	BT_CSn  4000h
//	PB13	DAC-CSn 2000h
//	PB12	ADC-CSn 1000h
// The SPI1_DR is 16 bits wide
/*
    DC32    TRUE_NEG_1, CS_DAC
    DC32    strkk, 00F0h, SPI1_DR // NOT SEEING IT.
    DC32    LIT, 00F0h, LIT, SPI1_DR, STORE
    DC32    ZERO, CS_DAC
*/
    DC32    TRUE_NEG_1, CS_DAC
    DC32    LIT, 3000, MS
    DC32    ZERO, CS_DAC

//	PB14	BT_CSn   4000h
//	PB13	DAC-CSn 2000h
//	PB12	ADC-CSn 1000h
// The SPI1_DR is 16 bits wide
/*
    DC32    TRUE_NEG_1, CS_DAC
    DC32    strkk, 00FFh, SPI1_DR
    DC32    ZERO, CS_DAC
*/
    DC32    TRUE_NEG_1, CS_ADC
    DC32    LIT, 3000, MS
    DC32    ZERO, CS_ADC

    DC32    LIT, 1000, MS
    DC32    CR
    DC32    ZERO
    DC32    ZBRAN
      DC32    SPI1_DAC_BB_BEGIN-.
    DC32    SEMIS

//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_SYS: = FISH System: CATEGORY

 SECTION .text : CONST (2)
WC_FISH_GPIO_NFA:
	DC8	0x80+4+5        // +4 is format chars constant
                                // +n is Name lenght
        DC8     0x0D, 0x0A
	DC8	'GPIO:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
	// DC32	WC_FISH_PubRel_NFA        // Where this wordcat links in.
       DC32    SPI1_DAC_BB_NFA


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