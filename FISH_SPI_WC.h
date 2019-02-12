// FISH_SPI_WC.h
/* File include explains how to make a new WORDCAT:
How to make it the current TOP of the Dictionary:
$WORDCAT HowTo.txt
*/
//=============================== SPI WORDs =================================//

// IMPLEMENT THE PORT B AND PORTC CONSTANTS

//	SPIB_ODR: ( -- addr )
 SECTION .text : CONST (2)
SPIC_ODR_NFA:
	DC8	0x80 +09d
	DC8	'SPIC-OD'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	WC_FISH_PubRel_NFA
	DC32	WC_FISH_GPIO_NFA
RM_SPIC_ODR:
//  DC32  DOCON, SPIC_ODR

//	SPIB_IDR: ( -- addr )
 SECTION .text : CONST (2)
SPIC_IDR_NFA:
	DC8	0x80 +09d
	DC8	'SPIC-ID'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPIC_ODR_NFA
RM_SPIC_IDR:
//  DC32  DOCON, SPIC_IDR

//	SPIC_MODER: ( -- addr )
 SECTION .text : CONST (2)
SPIC_MODER_NFA:
	DC8	0x80 +011d
	DC8	'SPIC-MODE'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPIC_IDR_NFA
RM_SPIC_MODER:
//  DC32  DOCON, SPIC_MODER

//	SPIB_ODR: ( -- addr )
 SECTION .text : CONST (2)
SPIB_ODR_NFA:
	DC8	0x80 +09d
	DC8	'SPIB-OD'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPIC_MODER_NFA
RM_SPIB_ODR:
//  DC32  DOCON, SPIB_ODR

//	SPIB_IDR: ( -- addr )
 SECTION .text : CONST (2)
SPIB_IDR_NFA:
	DC8	0x80 +09d
	DC8	'SPIB-ID'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPIB_ODR_NFA
RM_SPIB_IDR:
//  DC32  DOCON, SPIB_IDR

//	SPIB_MODER: ( -- addr )
 SECTION .text : CONST (2)
SPIB_MODER_NFA:
	DC8	0x80 +011d
	DC8	'SPIB-MODE'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPIB_IDR_NFA
RM_SPIB_MODER:
//  DC32  DOCON, SPIB_MODER

//	DACWORD: ( -- addr ) A SYSTEM VAR - USE WITH C@'S THRU THE WORD
 SECTION .text : CONST (2)
PSTATDACDATA_NFA:
	DC8	0x80 +07d
	DC8	'DACWOR'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_FISH_GPIO_NFA
PSTATDACDATA:
  DC32    DOCON, PSTAT_DAC_DATA

/*  MOVE DACWORD AND  DACW ABOVE HERE~!
OVR?	( -- ON|OFF ) return flag 0N = overflow true
 SPI3-SR @ 080h AND ?TRUE
*/
 SECTION .text : CONST (2)
OVR_QUSTION_NFA:
    DC8	0x80 + 4
    DC8	'OVR'
    DC8	'?'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	PSTATDACDATA_NFA
OVR_QUSTION:									//
	DC32 .+5
 SECTION .text : CODE (2)
        LDR     n, =SPI3_SR     // n_r1
        LDR     t, [n]          // t_r0,
// CMP IS NOT ALWAYS GONNA WORK~! SHIFT OUT TO FLAG OR BIT TEST~!
//        CMP     t, #80h         // OVR = 1  = Bit 6
        ANDS    t, t, #80h        // RXNE = 1 RCV BUFFER FULL = Bit 0
        BEQ     OVR_TRUE
// ULTIMATELY WANT COUNTED OR TIMED OUTS
        EORS	t, t, t         // t_r0, XERO T
        TPUSH_r0
        NEXT
OVR_TRUE:
        EORS	t, t, t   // t_r0, XERO T
        SUB   t, t, #1  // make -1 TRUE
        TPUSH_r0
        NEXT

// Commit literal constants to pool to be copied!
// LTORG

/* 
~RXNE	( -- ) Wait for RXNE ON
 BEGIN SPI3-SR @ 1 AND UNTIL
*/
 SECTION .text : CONST (2)
TILDA_RXNE_NFA:
    DC8	0x80 + 5
    DC8	'~RXN'
    DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	OVR_QUSTION_NFA
TILDA_RXNE:									//
	DC32 .+5
 SECTION .text : CODE (2)
BEGIN_TILDA_RXNE:
        LDR     n, =SPI3_SR     // n_r1
        LDR     t, [n]          // t_r0,
// CMP IS NOT ALWAYS GONNA WORK~! SHIFT OUT TO FLAG OR BIT TEST~!
//        CMP     t, #1           // RXNE = 1 RCV BUFFER FULL = Bit 0
        ANDS    t, t, #1        // RXNE = 1 RCV BUFFER FULL = Bit 0
        BNE     BEGIN_TILDA_RXNE
        NEXT

// Commit literal constants to pool to be copied!
// LTORG

/*
~TXE ( -- ) Wait for TXE ON
 BEGIN SPI3-SR @ 2 AND UNTIL
*/
 SECTION .text : CONST (2)
TILDA_TXE_NFA:
    DC8	0x80 + 4
    DC8	'~TX'
    DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	TILDA_RXNE_NFA
TILDA_TXE:									//
	DC32 .+5
 SECTION .text : CODE (2)
BEGIN_TILDA_TXE:
        LDR     n, =SPI3_SR     // n_r1
        LDR     t, [n]          // t_r0,
// CMP IS NOT ALWAYS GONNA WORK~! SHIFT OUT TO FLAG OR BIT TEST~!
//        CMP     t, #2           // TXE = 1 TX BUFFER EMPTY = Bit 1
        ANDS    t, t, #2        // RXNE = 1 RCV BUFFER FULL = Bit 0
        BNE     BEGIN_TILDA_TXE
        NEXT

// Commit literal constants to pool to be copied!
// LTORG

//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_SYS: = FISH System: CATEGORY

 SECTION .text : CONST (2)
WC_FISH_SPI_NFA:
	DC8	0x80+4+11d        // +4 is format chars constant
                                // +n is Name lenght
        DC8     0x0D, 0x0A
	DC8	'PSTAT SPI:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
//  DC32 PSTATDACDATA_NFA
  DC32 TILDA_TXE_NFA


/*

//=============================== SPI -BIG-PSTAT-TEST-SW- WORDCAT ===============================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_SPI: = FISH System: CATEGORY

 SECTION .text : CONST (2)
WC_FISH_BIG_PSTAT_SPI_NFA:
	DC8	0x80+4+23        // +4 is format chars constant
                                // +n is Name lenght
    DC8     0x0D, 0x0A
    DC8	    'SPI-BIG-PSTAT-TEST-SW:'
    DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPI1_LED_TEST_NFA       // Where this wordcat links in.

*/