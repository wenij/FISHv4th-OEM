/*
Should an error processed by EHON, ater its has gotten CO do a EHOFF?

Fixes in versions comments:

Cloned OEM and doing test pstat in that:
SOC_INIT SPI3 init test =
AHB1ENR     = 0100087h  GPIO port A,B,C & H clk enable
APB1ENR     = 010048000h  PWREN, UART3 AND SPI3
GPIOB_MODER = 055000A85h
GPIOB_ODR   = 07002h
GPIOB_AFRL  = 0666000h AF6 for spi3 on pb3-5
GPIOC_MODER = 055415h Sets GAIN-S0-S2 and SW1-SW4 high
GPIOC_ODR   = 03C0h SWITCH1-4 ON
GPIOC_AFRH  = 07700h PC10 as TX / PC11 as RX
SPI3_CR1    = 033Dh + SPIEN

DISCO Check
PSTAT 

SOC_INIT SPI1: 


The old issues are PSTAT branch hangs in SOC_INIT
and 1.8 and OEM fail +LOOP tests
I have redone a clone of the FISHv4th-OEM branch v1.8x,
which has none of the PSTAT work.
I didn't understand that COMMIT affect my local repo's .git folder,
not the repository. My local commits must be PUSH'd~!
So I saved the old repo and will repopulate from there with care~!

FIRST~!:
Verifying all files match (old LOCAL repo and new clone)
IN THE big-pstat branch I cloned.
FISHv4th-OEM_branch_BIG-PSTAT in my Desktop.
It is not where my copied directory of the last good is...
Using Dev ant's Code Compare.


Code Compare tool rocks~!

Suggestions:
FUART_BRR_BAUD  //where FUART denotes FISH word vs user UART words.

Deleted SOC_INIT test code that shows some of the bugs in 1.8 and OEM:
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


// SPI1-DAC-BB   ( n IS IN THE DACWORD ALLOCATION IN (MEMMAP) -- )
// MSB OF THE 24BITS FOR THE DAC IS IN THE MSB OF DACWORD
// SHIFT OUT LEFT 24X
// THIS PRIMITIVE WORD NEEDS "ON DAC-CS" PREAMBLE, AND 24 (OR22) CALLS...
 SECTION .text : CONST (2)
SPI1_DAC_BB_NFA:
  DC8	0x80 + 11d
  DC8	'SPI1-DAC-B'
	DC8	'B'+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32  PSTATDACDATA_NFA
SPI1_DAC_BB:
	DC32	.+5
 SECTION .text : CODE (2)
// READ DACWORD WHICH HAS MSB IN LSB ORDER
// SHIFT AND PRESERVE BETWEEN CALLS
// BY WRITING VALUE BACK TO DACWORD (MEMMAP)
	LDR n_r1, = PSTAT_DAC_DATA	// read DACWORD [val]
  LDR t_r0, [n]
// SHIFT OUT A BIT AT A TIME, MSB FIRST
  LSLS  t_r0, t_r0, #1  // modify val, AFFECTS ASPR-CARRY FLAG
STORE_BB_BIT:
  STR t_r0, [n_r1]     	// modify val, AFFECT ALU FLAG ~ 

  BCC OFF_BB_BIT        // FALL THRU IF CARRY SET
// PRODUCE an 0d OR FFh on tos
ON_BB_BIT:
  MOVS	t_r0, #-1  // TRUE -1 ON
	TPUSH_r0

OFF_BB_BIT:
	EORS	t_r0, t_r0	// OFF
	TPUSH_r0


*/
