//      FISH_STM32F4_UART3_INIT.s
// Valid thru v1.8

//	STM32F4 UART3_INIT UART3_INIT: ( -- )
//UART3_INIT: should be renamed. It sets baud rates for the UART's BRR register
//and programs the BRR, determined by system vars DBAUD and UBAUD or a user
//supplied decimal baud rate value.
//Can this handle a baud parameter without breaking UBAUD's concept?

//	User use of this word MUST use MYBAUD first. IT MUST BE USED FIRST.
//	THIS IS TO PRESERVE YOUR BAUD DURING HARD FAULT RESET, which will 
//      get reset to default baud ~ mucking your terminal up. No fun~!

//      Logic in this word:
//	WHEN DBAUD IS NOT ZERO USE DEFAULT_BAUD	        // pwrup unint ram
//	WHEN DBAUD IS ZERO USE UBAUD			// when called or reset

 SECTION .text : CONST (2)
UART3_INIT_NFA:
	DC8	0x8A
	DC8	'UART3_INI'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_NUMBERS_NFA
UART3_INIT:
	DC32	DOCOL
// GET DEFAULT OR USER SET BAUD (MYBAUD)
	DC32	LIT, DBAUD, AT, ZEQU
	DC32	ZBRAN
        DC32    BAUDDEFAULT-.	        // IF DBAUD not ZERO SET DEFAULT_BAUD
	DC32	LIT, UBAUD, AT          // IARBUG , TOR
        DC32    TOR		        // ELSE USE UBAUD
	DC32	BRAN
        DC32    BAUD1-.		        // SET UBAUD TO ZERO TO PRESERVE UBAUD
BAUDDEFAULT:			        // ELSE
	DC32	LIT, DEFAULT_BAUD       // IARBUG , TOR
        DC32    TOR	                // SET
BAUD1:
// BAUD IS ON R STACK
// USE IT TO CALCULATE UART3_BRR VALUE
// HAVE TO USE PCLK1 WHICH IS EQUATED IN SOCINIT
// PCLK1 = 42000000 
	DC32	LIT, PCLK1      // 280DE80h @ 168MHZ SYSCLK
	DC32	RFROM   	// 2580h = 9600 DEFAULT_BAUD
	DC32	SLASH
	DC32	LIT, USART3_BRR
//        DC32 NOOP
        DC32    STORE           // 1117h FOR 9600/42MHZ PCLK1D
        DC32	SEMIS

