// FISH_SPI_WC.h
/* 
How to make a new WORDCAT:
Make it the current TOP of the Dictionary:
$WORDCAT HowTo.txt
*/
//=============================== SPI WORDs =================================//

// IMPLEMENT THE SPI3 CONSTANTS

//	SPI3_DR: ( -- addr )
 SECTION .text : CONST (2)
SPI3_DR_NFA:
	DC8	0x80 +07d
	DC8	'SPI3-D'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_FISH_GPIO_NFA
PSTAT_SPI3_DR:
  DC32  DOCON, SPI3_DR

//	SPI3_SR: \ -- addr \  
 SECTION .text : CONST (2)
SPI3_SR_NFA:
	DC8	0x80 +07d
	DC8	'SPI3-S'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPI3_DR_NFA
PSTAT_SPI3_SR:
  DC32  DOCON, SPI3_SR
    
//	SPI3_CR2: \ -- addr \  
 SECTION .text : CONST (2)
SPI3_CR2_NFA:
	DC8	0x80 +08d
	DC8	'SPI3-CR'
	DC8	'2'+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32 SPI3_SR_NFA
PSTAT_SPI3_CR2:
  DC32  DOCON, SPI3_CR2
    
//	SPI3_CR1: \ -- addr \  
 SECTION .text : CONST (2)
SPI3_CR1_NFA:
	DC8	0x80 +08d
	DC8	'SPI3-CR'
	DC8	'1'+0x80
 ALIGNROM 2,0xFFFFFFFF
  DC32 SPI3_CR2_NFA
PSTAT_SPI3_CR1:
  DC32  DOCON, SPI3_CR1

//	DACWORD: ( -- addr ) A SYSTEM VAR - USE WITH C@'S THRU THE WORD
 SECTION .text : CONST (2)
PSTATDACDATA_NFA:
	DC8	0x80 +07d
	DC8	'DACWOR'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	WC_FISH_GPIO_NFA
  DC32  SPI3_CR1_NFA
PSTATDACDATA:
  DC32    DOCON, PSTAT_DAC_DATA

/*  MOVE DACWORD AND DACW ABOVE HERE~!
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
//        CMP     t, #40h         // OVR = 1  = Bit 6
        ANDS    t, t, #40h        // RXNE = 1 RCV BUFFER FULL = Bit 0
        BNE     OVR_TRUE
//        LSRS    t, t, #7
//        BCS     OVR_TRUE
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
        BEQ     BEGIN_TILDA_RXNE
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
        BEQ     BEGIN_TILDA_TXE
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