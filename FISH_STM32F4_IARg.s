// FISH_STM32F4_IAR.s - FULL WORD
// FISH FOR THE STM IS CURRENTLY SUPPORTING F2 TO F4
// Code is common, seperate linker files used for memory map differences.
$FISH_STM32F4_MAIN_INCLUDES.h
//------------------------------------------------------------------------------

// 2DO: Add VIO_UARTX, VIO_KEY, VIO_?KEY and VIO_EMIT
// (EMIT), (KEY?), and (KEY).

// v1.7:

// 2DO: Signon updated. (FOR TEST - FIX B4 SHIP)
// 2DO: Updated .hex .out and sym.bat

// 2DO:
// #define EOL_DLE         // DLE in QUIT after last line interpreted
// #define EOL_NAK         // NAK in error messages for STDLED editor highlight

// Split out files to FISH_RM_COMMON and FISH_RM_COMMON_CODE_CORTEX_M

// ADDED EHON and EHOFF to FISH RM
// Changes iar.s COLD, ERROR and ABORT
// Changes FISH_STM32F4_SLIB.s SV_INIT_VALUES: & FWARM:
// Adds 1 word to FISH_STM32F4_MEMMAP.s - ERROR_HALT: SV 

// ADDED P(PROMPT), PON(PROMPTON) and POFF(PROMPTOFF) to FISH RM
// Changes FISH_STM32F4_SLIB.s SV_INIT_VALUES: & FWARM: & QUIT:
// Adds 1 word to FISH_STM32F4_MEMMAP.s - PROMPT: SV 

// Added #define TRUE_EQU_NEG_ONE
// Changed all user visible true flags to -1

// Added SXTB Rd, Rm  and SXTH Rd, Rm

// Number Stuff Wordcat reorganized (search NOEXEC or WC_NUMBERS_NFA)
// EXPECT does counted null strings:
// NUMBER to look for null terminatiion instead of BLANK.
// WORD changed to expect null string and elimated BLANKS fill of HERE.
//      SPEEDS UP TOKENIZATION OF TIB INPUT!
// ALL IN AND IN_SV INIT SET TO 1 in QUERY SV_INIT_VALUES CLRTIB
// NUMBERSUFFIX changed to replace suffix with null
// PAREN ( changed to use TIB+1
// DPL restored to dictionairy.

//------------------------------------------------------------------------------
 SECTION .text : CONST (2)
msg_FISH:
// DC8 "?" IS A NULL TERMINATED STRING
// DC8 '?' IS NOT
//#if FISH_PubRel_WORDSET | FISH_DebugSrc_WORDSET
	DC8     'FISH ARM '
//#endif
#if FISH_PubRel_WORDSET & FISH_Debug_WORDSET
        DC8     ' VTOR FIXED - EHON/EHOFF '
//        DC8     '-1 True Flag '
#endif
#if VTOR_PATCH & STM32F205RC
        DC8     'VTOR_PATCH FOR MS '
#endif
#ifdef FISH_Debug_WORDSET
	DC8     'DebugSrc '
#endif
#ifdef  STM32F4_XRC08_168MHZ
        DC8     'STM32F407VG DISCO '
#endif
#ifdef STM32F205RC_XRC10_118MHZ
        DC8     'STM32F205RC STP RPM Board '
#endif
#ifdef FISH_PRO_WORDCAT
	DC8     'Pro '
#endif
        DC8     'RM V1.7 (C)2014-2015 A-TEAM FORTH : '
        DC8     __DATE__        // Null string
msg_FISH_TIMESTAMP:
        DC8     ' at '
        DC8     __TIME__        // Null string
msg_MY_OK:
        DC8     " ok, go fish in BASE "
//-----------------START OF DICTIONARY = Last word in search--------------------
//	NOOP NOOP:	( -- )
 SECTION .text : CONST (2)
NOOP_NFA:
	DC8	0x84
	DC8	'NOO'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	0	// 0 START OF DICTIONARY = Last word in search
NOOP:
	DC32	.+5
 SECTION .text : CODE (2)
	NEXT


//	EXECUTE EXEC:	( cfa -- ) RENAMED: EXECUTE to EXEC
//	Execute a single word whose cfa is on the stack

 SECTION .text : CONST (2)
EXEC_NFA:
	DC8	0x87
	DC8	'EXECUT'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	NOOP_NFA
EXEC:
	DC32	.+5
 SECTION .text : CODE (2)
#ifndef IO2TP
#ifdef XON_XOFF
        BL      TXRDY_SUBR
        BL      XOFF_SUBR
#endif
#endif
EXEC_ACTION:
	POPp2w // as in xeq token at ToS setup for exec	LDM	p!, {w}
	NEXT1

#ifndef IO2TP
// TXRDY_SUBR:
 SECTION .text : CODE (2)
TXRDY_SUBR:
        MOV     w, lr           // Allow for interrupts to use LR
        LDR     y, = USART3_SR  // Line Status Register
txRDY?:
        LDR     n, [y]          // Get Line Status
// THIS IS TXE TEST AND FAILS IN TEXT DOWNLOAD
//        LSRS    n, n, #7      // 80h Bit 7 TXE: Transmit data register empty
// THIS IS ___ AND WORKS IN TEXT DOWNLOAD
        LSRS    n, n, #8        // 100h Bit 8 ORIG
        BCC     txRDY?          // Ready
        BX      w               // lr  - SUBR RETURN

// XOFF_SUBR:
#ifdef XON_XOFF
 SECTION .text : CODE (2)
XOFF_SUBR:
        MOV     w, lr           // Allow for interrupts to use LR
	LDR	n,= USART3_DR
	LDR	y,  = XOFF_CHAR
	STRB	y, [n]
        BX      w               // lr  - SUBR RETURN
#endif // XON_XOFF

// XON_SUBR:
#ifdef XON_XOFF
 SECTION .text : CODE (2)
XON_SUBR:
        MOV     w, lr           // Allow for interrupts to use LR
	LDR	n,= USART3_DR
	LDR	y, = XON_CHAR   // preserve TOS 11 24 01 49
	STRB	y, [n]
        BX      w       //lr              // SUBR RETURN
#endif // XON_XOFF
#endif // IO2TP
 LTORG

//	BL BLANK:	( -- n )
//	Push ascii space character as n to TOS.

 SECTION .text : CONST (2)
BLANK_NFA:
	DC8	0x82
	DC8	'B'
	DC8	'L'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	EXEC_NFA
BLANK:
	DC32	DOCON, 0x20


//========================= START SYSTEM VARIABLES ===========================//
// _SV IS cfa label of WORD's that return their address.
// MODIFIED ALL to be label based vs. offset based.


//	UP UP_SV:	( -- addr of UP ) Value stored here is ALIGNED
//	A system variable, the RAM VAR pointer, which contains
//      the address of the next free memory above in the USERRAMSPACE.
//      The value may be read by .UV and altered by VAR.

 SECTION .text : CONST (2)
UP_NFA:
	DC8	0x82
	DC8	'U'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BLANK_NFA
UP_SV:
        DC32    DOCON, UP


//	FENCE FENCE_SV:	( -- addr of FENCE )
//	A system variable containing an address below which FORGET ting is
//	trapped. To forget below this point the system must alter the contents
//	of FENCE.

 SECTION .text : CONST (2)
FENCE_NFA:
	DC8	0x85
	DC8	'FENC'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	UP_NFA
FENCE_SV:
        DC32    DOCON, FENCE


//	DP DP_SV:	( -- addr of DP ) Value stored here is ALIGNED
//	A system variable, the dictionary pointer, which contains the address
//	of the next free memory above the dictionary. The value may be read
//	by HERE and altered by COMMA (ALIGNED) , ALLOT and CALLOT (UNALIGNED) .

 SECTION .text : CONST (2)
DP_NFA:
	DC8	0x82
	DC8	'D'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FENCE_NFA
DP_SV:
        DC32    DOCON, DP


//	BASE BASE_SV:	( -- addr of NBASE )
//      A system variable containing the current number base used for input
//      and output conversion.

 SECTION .text : CONST (2)
BASE_NFA:
	DC8	0x84
	DC8	'BAS'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DP_NFA
BASE_SV:
        DC32    DOCON, NBASE


//	CURRENT CURRENT_SV:	( -- addr of CURRENT )
//	CURRENT searched everytime. CONTEXT is not used
//	Used by Create, Definitions, etc and by Latest
//      to leave the nfa of the topmost word in the CURRENT
//      vocabulary.  See LATEST NOTE:(FIG) CURR @ @ this CURR @

 SECTION .text : CONST (2)
CURRENT_NFA:
	DC8	0x87
	DC8	'CURREN'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BASE_NFA
CURRENT_SV:
        DC32    DOCON, CURRENT	// LATEST = CURRENT_SV -> MOST RECENT DEF NFA


//	STATE STATE_SV:	( -- addr of CSTATE )
//	A system variable containing the compilation state. A non-zero value
//	indicates compilation. The value itself may be implementation
//	dependent. 0xC0 must be used for INTERPRET to execute IMMEDIATE words.

 SECTION .text : CONST (2)
STATE_NFA:
	DC8	0x85
	DC8	'STAT'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CURRENT_NFA
STATE_SV:
        DC32    DOCON, CSTATE       //Compile STATE


//	OUT OUT_SV:	( -- addr of OUT )
//      A system variable that contains a value incremented by EMIT. The system
//      may alter and examine OUT to control display formating.

 SECTION .text : CONST (2)
OUT_NFA:
	DC8	0x83
	DC8	'OU'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	STATE_NFA
OUT_SV:
        DC32    DOCON, OUT


//	IN IN_SV:	( -- addr of IN )
//	A system variable containing the byte offset within (TIB),
//      from which the tokens are parsed by WORD and moved to HERE,
//      as a counted null terminated string (created by EXPECT).
//      Initialized to 1 to skip count byte produced by EXPECT.

 SECTION .text : CONST (2)
IN_NFA:
	DC8	0x82
	DC8	'I'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	OUT_NFA
IN_SV:
         DC32   DOCON, IN


//	PAD PAD_SV:	( -- addr of PAD )
//      Leave the address of the text output buffer PAD on the stack.
//	Typically at least 82 bytes long.

 SECTION .text : CONST (2)
PAD_NFA:
	DC8	0x83
	DC8	'PA'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	IN_NFA
PAD_SV:
        DC32    DOCON, PAD


//	TIB TIB_SV:	( -- addr of INITTIB )
//      Leave the address of the text input buffer TIB on the stack.
//	Typically at least 82 bytes long.

 SECTION .text : CONST (2)
TIB_NFA:
	DC8	0x83
	DC8	'TI'
	DC8	'B'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	PAD_NFA
TIB_SV:
        DC32    DOCON, TIB


//======================== END SYSTEM VARIABLES ==============================//


//	HERE HERE:	( -- addr ) Value stored here is ALIGNED
//      Leave the address of the next available dictionary location.

 SECTION .text : CONST (2)
HERE_NFA:
	DC8	0x84
	DC8	'HER'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	TIB_NFA
HERE:
	DC32	DOCOL
	DC32	DP_SV
	DC32	AT
	DC32	SEMIS


//	ALLOT ALLOT:	( sn -- ) Warning: UNALIGNED:
//	IF DICTIONAIRY FULL PRINT error MESSAGE AND abort.
//	Add the signed number to the dictionary pointer DP. May be used to
//	reserve dictionary space or re-origin memory.
//	CAUTION!
//	ALLOT and C, (CCOMMA:) ARE THE ONLY WORDS THAT CAN ALLOT UNEVEN AMOUNTS
//	NOTE: USE ALIGN32_DP_FF_PAD after ALLOT or C, (CCOMMA) .

 SECTION .text : CONST (2)
ALLOT_NFA:
	DC8	0x85
	DC8	'ALLO'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	HERE_NFA
ALLOT:
	DC32	DOCOL
	DC32	DICTSPACE	// \ -- n
        DC32    OVER, SUBB
	DC32	ZLESS
	DC32	ZBRAN
	DC32	ALLOTOK-.

	DC32	DICTFULL_ABORT

ALLOTOK:
	DC32	DP_SV
	DC32	PSTORE
	DC32	SEMIS


//	ALIGNED ALIGNED:	( addr -- a-addr ) DPANS94

 SECTION .text : CONST (2)
ALIGNED_NFA:
	DC8	0x87
	DC8	'ALIGNE'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ALLOT_NFA
ALIGNED:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	ADDS    t, t, #3
	LDR	n, =-4
	ANDS	t, t, n
	TPUSH
 LTORG


//	ALIGN32_DP_FF_PAD ALIGN:	( -- ) DPANS94
//	Align DP_SV primitive. Pads current DP with 0xFF until aligned.
//	USED IN , (COMMA:) .  Reflected by HERE .
//	CAUTION! USE AFTER ALLOT or C, (CCOMMA:) .
//	See , (COMMA:) which is ALIGNED.

 SECTION .text : CONST (2)
ALIGN_NFA:
	DC8	0x85
	DC8	'ALIG'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ALIGNED_NFA
ALIGN:
ALIGN32_DP_FF_PAD:
	DC32	DOCOL
	DC32	HERE
	// COULD ADD CODE TO SEE IF ALIGNED ALREADY
	DC32	THREE, PLUS		// 3PLUS
	DC32	DUP, TOR
	DC32	HERE
	DC32	XDO
ALIGN_PAD_DO:
	DC32	LIT, 0xFF, I, CSTORE
	DC32	XLOOP
	DC32	ALIGN_PAD_DO-.

	DC32	RFROM
	DC32	LIT, 0xFFFFFFFC	// -4
	DC32	ANDD
	DC32	DP_SV, STORE
	DC32	SEMIS


//	, COMMA:	( n -- ) ALIGNED
//	IF DICTIONAIRY FULL PRINT Error MESSAGE AND Abort.
//	Write n into next ALIGNED dictionary memory cell, DP reflected by HERE

 SECTION .text : CONST (2)
COMMA_NFA:
	DC8	0x81
	DC8	','+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ALIGN_NFA
COMMA:
	DC32	DOCOL
	DC32	DICTSPACE	// \ -- n
	DC32	ZLESS
	DC32	ZBRAN
	DC32	COMMA_OK-.

	DC32	DICTFULL_ABORT

COMMA_OK:
	DC32	COMMA_PRIM
	DC32	SEMIS


//	C, CCOMMA:	( n -- ) Warning: UNALIGNED:
//	IF DICTIONAIRY FULL PRINT Error MESSAGE AND Abort.
//	Store LS 8 bits of n into the next available dictionary byte, advancing
//	the dictionary pointer.
//	CAUTION!
//	ALLOT and C, (CCOMMA:) ARE THE ONLY WORDS THAT CAN ALLOT UNEVEN AMOUNTS
//	NOTE: USE ALIGN32_DP_FF_PAD after ALLOT or C, (CCOMMA) .
//	See , (COMMA:) which is ALIGNED.

 SECTION .text : CONST (2)
CCOMMA_NFA:
	DC8	0x82
	DC8	'C'
	DC8	','+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	COMMA_NFA
CCOMMA:
	DC32	DOCOL
	DC32	DICTSPACE	// \ -- n
	DC32	ZLESS
	DC32	ZBRAN
	DC32	CCOMMA_OK-.

	DC32	DICTFULL_ABORT

CCOMMA_OK:
	DC32	HERE
	DC32	CSTORE
	DC32	ONE
	DC32	ALLOT_PRIM
	DC32	SEMIS


//	VARALLOT VARALLOT:	( n -- addr ) Value stored here is ALIGNED
//	IF USERVAR SPACE FULL PRINT Error MESSAGE AND Abort.
//	Add n * 32 bits to the RAMVARSPACE pointer UP.
//	Used to allocate space in Ram for VAR's and other system data structures

 SECTION .text : CONST (2)
VARALLOT_NFA:
	DC8	0x88
	DC8	'VARALLO'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CCOMMA_NFA
VARALLOT:
	DC32	DOCOL
	DC32	VARSPACE		// \ -- n
	DC32	FOUR, SLASH, OVER, SUBB
	DC32	ZLESS
	DC32	ZBRAN
	DC32	VALLOT_OK-.

	DC32	LIT, msg_uvfull
	DC32	NULLSTRLEN, TYPE
#ifdef IO2TP
//VARALLOT_BP1:
// DC32 NOOP
        DC32    CLRPAD  // Resets OUT
#endif
	DC32	ABORT

VALLOT_OK:
	DC32    UP_SV, AT       // Address of this allotment
	DC32    SWAP            // n
	DC32    FOUR, STAR      // n = 4 bytes
	DC32    UP_SV
	DC32    PSTORE          // Address of next var available
	DC32    SEMIS


//	LATEST LATEST: ( -- nfa )
//	Initially ADDR OF LAST WORD IN THE DICTIONARY
//      Leave the nfa of the topmost word in the dictionary.

 SECTION .text : CONST (2)
LATEST_NFA:
	DC8	0x86
	DC8	'LATES'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	VARALLOT_NFA
LATEST:
	DC32	DOCOL
	DC32	CURRENT_SV	// LATEST = CURRENT_SV
	DC32	AT
	DC32	SEMIS


//	LFA LFA:	( pfa -- lfa )
//      Convert the parameter field address of a dictionary definition to
//      its link field address.

 SECTION .text : CONST (2)
LFA_NFA:
	DC8	0x83
	DC8	'LF'
	DC8	'A'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LATEST_NFA
LFA:
	DC32    .+5
 SECTION .text : CODE (2)
	POP2t
	SUBS     t, t, #8
	TPUSH


//	CFA CFA:        ( pfa -- cfa )
//      Convert the parameter field address of a dictionary definition to
//      its code field address. <if thumb2 execution addr smudged>

 SECTION .text : CONST (2)
CFA_NFA:
	DC8	0x83
	DC8	'CF'
	DC8	'A'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LFA_NFA
CFA:
	DC32    .+5
 SECTION .text : CODE (2)
	POP2t
	SUBS     t, t, #4
	TPUSH


//	NFA NFA:        ( pfa -- nfa )
//      Convert the parameter field address of a definition to its name
//      field. Modified for nfa alighnment padding

 SECTION .text : CONST (2)
NFA_NFA:
	DC8	0x83
	DC8	'NF'
	DC8	'A'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CFA_NFA
NFA:
	DC32    DOCOL
	DC32    EIGHT, SUBB     // Addr OF lfa, DUPED TO TRAVERSE Padding

NFA_ALIGN_BEGIN:
	DC32    ONE, SUBB       // ADDR 1 BEFORE lfa, eliminate padding
	DC32    DUP, CAT        // ALIGNROM FF padded?
	DC32    LIT, 0xFF, SUBB
	DC32    ZBRAN           // UNTIL
	DC32    NFA_ALIGN_BEGIN-.
	    
	DC32    LIT,-1          // DIRECTION FOR TRAVERSE = Toward nfa
	DC32    TRAVERSE		// \ -- addr2
	DC32    SEMIS


//	PFA PFA:	( nfa -- pfa )
//      Convert the name field address of a compiled definition to its
//      parameter field address.

 SECTION .text : CONST (2)
PFA_NFA:
	DC8	0x83
	DC8	'PF'
	DC8	'A'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	NFA_NFA
PFA:
	DC32	DOCOL		// \ NFA --
	DC32	ONE		// DIRECTION FOR TRAVERSE = Toward CFA
	DC32	TRAVERSE        // \ NFA 1 -- addr2

PFA_ALIGN_BEGIN:
	DC32    ONEP            // ADDR 1 BEFORE lfa, eliminate padding
	DC32    DUP, CAT        // ALIGNROM FF padded?
	DC32    LIT, 0xFF, SUBB
	DC32    ZBRAN           // UNTIL
	DC32    PFA_ALIGN_BEGIN-.

	DC32	EIGHT
	DC32	PLUS
	DC32	SEMIS


//	?ALIGNED QALIGNED: ( value -- )
//	If value not aligned by 4 issue message and ABORT.

 SECTION .text : CONST (2)
QALIGNED_NFA:
	DC8	0x88
	DC8	'?ALIGNE'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    PFA_NFA
QALIGNED:
	DC32	DOCOL
	DC32	LIT, 3, ANDD
	DC32	ZBRAN		//IF no err leave
	DC32	 QALIGN_OK-.

	DC32	PDOTQ
	DC8	46
	DC8	'VALUE MUST BE EVEN NOT ODD AND MULTIPLE OF 4! '
 ALIGNROM 2,0xFFFFFFFF
	DC32	ABORT

QALIGN_OK:
	DC32	SEMIS


//	?ERROR QERROR: ( f nullstr -- )
//	If f NON ZERO type error message (nullstr) and ABORT.

 SECTION .text : CONST (2)
QERROR_NFA:
	DC8	0x86
	DC8	'?ERRO'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	QALIGNED_NFA
QERROR:
	DC32	DOCOL
	DC32	SWAP
	DC32	ZBRAN	//IF no err leave
	DC32	 QERROR_FINISH-.

	DC32	ERROR // \ nullstr-addr ---  ABORT's
	DC32	BRAN	//ELSE
	DC32	 QERROR_DONE-.

QERROR_FINISH:
	DC32	DROP	//endif

QERROR_DONE:
	DC32	SEMIS


//	DIGIT DIGIT:	( c base -- digit 1 ) (ok)
//			( c base -- 0 (bad)
//	Converts the ascii character c using base to its binary
//	equivalent digit, accompanied by a true flag. If the conversion is
//	invalid, leaves only a false flag.

 SECTION .text : CONST (2)
DIGIT_NFA:
 	DC8	0x85
	DC8	'DIGI'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	QERROR_NFA
DIGIT:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2w         		// Number base
	POP2t			// ASCII DIGIT
	SUBS   t, t, #'0'
	BMI   DIGI2             // Number error

	CMP   t, #9
	BLE   DIGI1             // Number = 0 THRU 9

	// Combine?
	SUBS   t, t, #7
	CMP   t, #10            // Number 'A' THRU 'Z'?
	BLT   DIGI2             // NO

DIGI1:
	CMP     t, w            // COMPARE Number TO base
	BGE     DIGI2

	MOV	w, t            // NEW BINARY Number
	MOVS	t, #1           // TRUE FLAG
	DPUSH

	// Number error
DIGI2:
	MOVS   t, #0		// FALSE FLAG
	TPUSH


//	ENCLOSE ENCL:	( addr1 c -- addr1 n1 n2 n3 )
//	A text scanning iterative primitive used only in WORD.
//	IN is used before and after callS for the iteration
//	Typically TIB + IN = addr1
//      noop EX: \ TIB   32 -- TIB   0 4 5
//      NULL EX: \ TIB   32 -- ADDR-B4-NULL 0 1 1 So DFIND CAN FIND IT!
//      NULL EX: \ WORD/ENCL -> TIB 1, 0, 20'S
//      From the text address addr1 and an ascii delimiting character c,
//      is determined the byte offset to the first non-delimiter character n1,
//      the offset to the first delimiter after the text n2,
//      and the offset to the first character not included.
//      This procedure will not process past an ASCII 'null', treating it
//      as an unconditional delimiter. NULL termination by EXPECT in QUERY.

 SECTION .text : CONST (2)
ENCL_NFA:
	DC8	0x87
	DC8	'ENCLOS'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DIGIT_NFA
ENCL:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t			// TERMINATOR CHAR
	LDR     n, [p]		// get text address but leave on stack
	MOVS    w, #0
	SUBS	w, w, #1        // CHAR  COUNTER
	SUBS    n, n, #1        // ADDR -1

//  SCAN TO FIRST NON-TERMINATOR CHARACTER
//	and PUSH count to stack leaving last 2 params to compute
ENCL1:
	ADDS    n, n, #1	// ADDR+1
	ADDS    w, w, #1        // COUNT+1
	LDRB    x, [n]
	CMP     t, x            //CMP	AL,[BX]
	BEQ     ENCL1           //JZ	ENCL1	// WAIT FOR NON-TERMINATOR
	PUSHw			//offset to the first non-delimiter character
				//EXiT is now only DPUSH
	CMP     x, #0           //CMP	AH,[BX]	// null CHAR?
	BNE     ENCL2           //JNZ	ENCL2	// NO

// FOUND null BEFORE 1ST NON-TERM CHAR
// RETURN ok args = NULL EX//
// \ TIB   32 -- ADDR-B4-NULL 0 1 1 So DFIND CAN FIND IT!
//
	MOV	t, x
	ADDS    w, w, #1	// w = offset to the delimiter after the text
	DPUSH

//   FOUND FIRST TEXT CHAR - COUNT THE CHARS
ENCL2:
	ADDS    n, n, #1        // ADDR+1
	ADDS    w, w, #1        //COUNT+1
	LDRB    x, [n]
	CMP    	t, x            //TERMINATOR CHAR?
	BEQ     ENCL4           //YES

	CMP     x, #0           //null CHAR?
	BNE     ENCL2           //NO, LOOP AGAIN

ENCL3:	//   FOUND null AT END OF TEXT
ENCL4:	//   FOUND TERMINATOR CHARACTER
	MOV     t, w            // COUNT+1 =
	ADDS    t, t, #1        // offset to the first character not included
	DPUSH


//	0 NULL:	( -- ) IMMEDIATE
//	Don't return to Interpret, return to Quit at end of a line of input.
//      Executed at end of TIB when QUERY lenght or CR entered.

 SECTION .text : CONST (2)
NULL_NFA:
	DC8	0x0C1			// A BINARY ZER
	DC8	0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ENCL_NFA
NULL:
//	DC32	SEMIS			// Actual Semis code below
	DC32	.+5
 SECTION .text : CODE (2)
	POPr2i	// pop docol saved IP from Rstack
	NEXT


//	WORD WORD:	( c -- )
//	Parse the text in TIB , until a delimiter c is found, tokenizing input.
//	Move the token to HERE, with a count byte and 2 nulls at the end.
//	Leading occurrences of c are ignored. IN is incremented.
//	See IN. Usually used inside a definition.

 SECTION .text : CONST (2)
WORD_NFA:
	DC8	0x84
	DC8	'WOR'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	NULL_NFA
WORD:
	DC32	DOCOL
WORD1:
//      This is required for number suffix's at least ???
// NOT WITH NEW COUNTED NULL STRING EXPECT
//	DC32	HERE, LIT, MAXWORDLEN+2, BLANKS
	DC32	TIB_SV
WORD2:
	DC32	IN_SV
	DC32	AT
	DC32	PLUS
        DC32	SWAP	// ( TIB+IN CH-DELIM -- )
	DC32	ENCL    // ( TIB+IN 32 -- TIB+IN   0 4 5 )
WENCL:
	DC32	IN_SV	// encl TOS = end of word offset to be added to IN
	DC32	PSTORE  // TIB+IN   0 4
	DC32	OVER    // 
	DC32	SUBB    // Char cnt going to R and HERE
	DC32	TOR
	DC32	R
	DC32	HERE
	DC32	CSTORE   // CHAR CNT OF word
	DC32	PLUS
	DC32	HERE
	DC32	ONEP
//	DC32	RFROM
// MOVE word TO DICT FOR finding ENDING WITH NULLS
        DC32    R
	DC32	CMOVE   // ( from to count -- )
// add null to end of token - NUMBER adjusted        
        DC32    ZERO, HERE, ONEP, RFROM, PLUS, CSTORE
#ifdef  IOBUFS_DEBUG
        DC32    HERE, ONE, DUMP
#endif        
	DC32	SEMIS


//	NUMBER NUMBER:	( addr -- sd )
//	If error print message and ABORT.
//      Convert a character string left at addr with a preceeding count,
//      and a terminating null, to a signed number, in the current numeric base.
//      If a decimal point is encountered in the text, its position will be
//      given in DPL, but no other effect occurs. If numeric conversion is
//      not possible, an error message will be given.

 SECTION .text : CONST (2)
NUMBER_NFA:
	DC8	0x86
	DC8	'NUMBE'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WORD_NFA
NUMBER:
	DC32	DOCOL
        DC32    BASE_TO_R12     // Save current BASE
        DC32    NUMBERSUFFIX
	DC32	ZERO, ZERO
	DC32	ROT		// ( 0 0 addr -- )

	DC32	DUP, ONEP	// ( 0 0 addr addr+1 -- )
	DC32	CAT		// ( 0 0 addr 1rstchar -- )
	DC32	LIT, '-'	// Minus sign = 0x2D
	                	// ( 0 0 addr 1rstchar 0x2D --- )
	DC32	EQUAL		// ( 0 0 addr f -- )
#ifdef TRUE_EQU_NEG_ONE         // FIX ASSUMPTION THIS FLAG WILL BE ONE!
        DC32    ONE, ANDD       // ( 0 0 addr f -- ) FLAG IS ONE OR ZERO
#endif
	DC32	DUP, TOR	// ( 0 0 addr f -- ) copy of f >R
	DC32	PLUS		// ASSUMPTION FLAG WILL BE 1 or 0
	DC32   LIT, -1

NUMB1:      			// Begin
	DC32	DPL_SV, STORE
	DC32	PNUMBER   	// ( 0 0 addr -- ud=(LSW MSW) ADDR=PAST#)
// END OF NUMBER CHARACTER CHECK HERE
	DC32	DUP
	DC32	CAT
// Convert from space to NULL terminated string fix here
//	DC32	BLANK    	//20h
        DC32    ZERO
	DC32	SUBB
		        	// while
	DC32	ZBRAN   	// IF NULL (WAS BLANK)
	DC32    NUMB2-. 	// DONE UNLESS IT NEEDS TO BE NEGATED

	DC32	DUP
	DC32	CAT
	DC32	LIT, '.'        // 2EH period
	DC32	SUBB

NUMERRTEST:
// IF zero (NOT GOING TO ERROR) LEAVE BASE ALONE FOR LOOP!
// consumed flag here so provide true flag if you don't
//        DC32    DUP
        DC32    ZBRAN
        DC32      NOBRESTOR-.

// GOING TO ERROR SO RESTORE BASE!
        DC32    BASE_FROM_R12   // Restore BASE
//NOBRESTOR:
        DC32    ONE     // provide non zero flag for error
	DC32	LIT, msg_number_error
	DC32	QERROR // ( f nullstr-addr -- ) IF f TRUE EXECUTE ERROR!

NOBRESTOR:      // Reset DPL to zero
	DC32	ZERO
	DC32	BRAN
	DC32    NUMB1-.

NUMB2:
	DC32	DROP
	DC32	RFROM           // FLAG OF "-" TEST
	    			// IF
	DC32	ZBRAN
	DC32    NUMB3-.

	DC32	DNEGATE

NUMB3:        		        // endif
        DC32    BASE_FROM_R12   // Restore BASE
        DC32    SEMIS


//	ERROR ERROR:	( nullstr --- )
//	Execute error notification and restart of system.
//      IN_SV is saved to assist in determining the location of the error.

 SECTION .text : CONST (2)
ERROR_NFA:
	DC8	0x85
	DC8	'ERRO'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	DFIND_NFA
	DC32	NUMBER_NFA
ERROR:
	DC32	DOCOL
	DC32	HERE, COUNT, TYPE       // Offending item
	DC32	NULLSTRLEN, TYPE        // Passed in null string
#ifdef IO2TP
ERROR_BP1:
 DC32 NOOP   // View error message
        DC32    CLRPAD  // Resets OUT
#endif
#ifdef XON_XOFF
        DC32    XOFF    // TEMP TEST THRE
#endif
        
//      DO THIS WHERE STATE IS SET BACK TO ZERO
        DC32    STATE_SV, AT
        DC32    ZBRAN
        DC32     CREATED_OK-.

//      NON-ZERO CSDP = RESTORE DP TO forget DAMAGED WORDS
        DC32    CSDP_SV, AT   // Create saves dp here for if word exists err
        DC32    ZBRAN
        DC32     CREATED_OK-.

// assume ITS A CREATE'd WORD!!!!
ERROR_FIXUP:     // CSDP_SV contains NFA of definition that has error.
        DC32    CSDP_SV, AT     // SHOULD BE AT NFA!!
        DC32    PFA     // \ nfa -- pfa
        DC32    LFA, AT // \ pfa --- lfa
        DC32    CURRENT_SV, STORE       // Has to be NFA of last good word
// RESET DP!
        DC32    CSDP_SV, AT
        DC32    DP_SV, STORE

CREATED_OK:
        DC32    SEMIC_CREATE            // RESET CSDP FOR AUTO FORGET
#ifdef ABORT_STOP_TILL_CO
        DC32    ABORT
#else
	DC32	QUIT	// FROM Error
#endif
//	DC32	QUIT	// FROM Error


//	' TICK:	( -- pfa ) IMMEDIATE
//	Used in the form: .
//	' CCcc
//	Leaves the parameter field address of dictionary word CCcc.
//	If the word is not found after a search of CURRENT,
//	an appropriate error message is given.
//	As a compiler directive, executed in a colon-definition to compile
//      the address as a literal.
//	Pronounced "TICK".

 SECTION .text : CONST (2)
TICK_NFA:
	DC8	0x0C1
	DC8	''''+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ERROR_NFA
TICK:
	DC32	DOCOL
	DC32	DFIND   // noop there for TIB entry
	DC32	ZEQU
	DC32	LIT, msg_word_error
	DC32	QERROR    // \ f nullstr-addr --
	DC32	DROP
	DC32	LITERAL
	DC32	SEMIS


//	EXPECT EXPECT:  ( addr count -- )
//	Transfer characters from the terminal to the buffer starting at addr+1,
//	until a "return" or the count of characters have been received.
//	One or two nulls are added at the end creating a null-terminated string.
//      A count byte is placed at addr, the count being derived by $LEN
//	Use COUNT with TYPE for strings saved this way.
//      Strings longer that 255 will have an invalid count byte. Use $LEN:
//      $LEN and TYPE can be used when the argument to $LEN is addr+1.

//      Comments below use TIB input (QUERY in INTERPRET) as an example.

 SECTION .text : CONST (2)
EXPECT_NFA:
	DC8	0x86
	DC8	'EXPEC'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    TICK_NFA
EXPECT:			// ( NOS TOS -- NOS TOS )
	DC32	DOCOL	// ( TIB LEN -- )
        DC32    ONEP    // ( TIB LEN+1 -- ) (Index and Limit)+1 for count byte
	DC32	OVER	// ( TIB LEN+1 TIB -- )
	DC32	PLUS	// ( TIB TIB+LEN+1 -- )
	DC32	OVER	// ( TIB TIB+LEN+1 TIB -- )
        DC32    ONEP    // ( TIB TIB+LEN+1 TIB+1 -- ) SKIP COUNT BYTE
#ifdef XON_XOFF
	DC32	XON
#endif
// ( ORGIGINAL-TIB TIB+LEN+1 TIB+1 -- ORGIGINAL-TIB ) 
//                (Index & Limit)+1 for count byte
	DC32	XDO	// DO	Index = addr, Limit = addr+cnt
EXPE1:
	DC32	KEY	// CAN BREAKPOINT OR CHANGE TO BE 0x0D ONLY
#ifdef XON_XOFF
        DC32    IF_EOL_SEND_XOFF	// IF CR QUICK XOFF!
#endif
// goto main branch (EL1:)if not tab
	DC32	DUP, LIT, 9, EQUAL	// ? TAB
        DC32    ZBRAN                   // Not tab
        DC32     EL1-.                  // Goto MAIN LOOP
        
// REPLACE TAB WITH SPACE               // -- ch f
        DC32    DROP, BLANK             // Convert tab to space
EL1:
// MAIN LOOP ENTERED WITH CHAR ON TOS
	DC32	DUP		// ( TIB key key -- )
// FALL THRU IF BACKSPACE OR DELETE KEY
	DC32	BACKSPACE_CHAR
        DC32    EQUAL
        DC32    OVER
        DC32    LIT, 07Fh       // DEL KEY
        DC32    EQUAL
        DC32    OR
// BRANCH IF NOT BACKSPACE OR DELETE KEY
	DC32	ZBRAN           // If not backspace
	DC32	 EXPE2-.        // go here

// CASE OF BACKSPACE/DELETE KEY
	DC32	DROP	        // ( TOS> 8 TIB -- TIB ) Drop Backspace on stack
	DC32	DUP	        // ( TOS> TIB -- TOS> TIB TIB )  Current TIB addr
	DC32	I               // ( TOS> TIB TIB -- I=TIB+? TIB TIB )
// COMPENSATE FOR COUNT BYTE - BUMP ADDR+1 (NEXT CHAR LOCATION) BACK TO CURRENT
        DC32    ONEM
	DC32	EQUAL           // ( TOS> I=TIB+? TIB TIB -- f TIB )
	DC32	DUP             // ( TOS> f TIB -- f f TIB )
// I AND RFROM STARTS AT TIB+1 
// REMOVE INDEX FROM LOOP TO REPLACE IT WITH BACKSPACE CORRECTED ADDRESS
	DC32	RFROM           // ( TOS> f f TIB -- TIB+1 f f TIB )
// ADDING 2 BECAUSE TIB+1 IS FROM I AND FLAG MAKES IT = TO 1
// WHEN AT THE BEGIINING (ADDR+1) OF THE ADDR BUFFER
	DC32	TWO
	DC32	SUBB
// ADD FLAG 
	DC32	PLUS
	DC32	TOR
	DC32	ZBRAN           // If not at beginning of line
	DC32	 EXPE6-.        // Goto bsout

	DC32	BELL	        // At beginning of TIB issue bell
	DC32	BRAN	        // Goto end of loop
	DC32	 EXPE33-.

EXPE6:
	DC32	BSOUT	        // endif
EXPE7:	// LABEL NOT USED BECAUSE BELL AND BSOUT Emit THEMSELVES
	DC32	BRAN	        // Goto end of loop
	DC32	 EXPE33-.

EXPE2:
	DC32	DUP
	DC32	LIT, 0x0D	// cr
	DC32	EQUAL
	DC32	ZBRAN           // If not cr
	DC32	 EXPE4-.        // Goto save this char and loop again
        
	DC32	LEAVE           // ( TOS> ODh TIB --  )
#ifdef  IOBUFS_DEBUG        
        DC32    DOTSHEX
#endif
        // CR DROPPED HERE I USED AT END FOR CASE OF EXPECT COUNT REACHED
	DC32	DROP            // ( TOS> TIB -- )
	DC32	BLANK           // ( TOS> 20h TIB -- ) space IS FOR EMIT!
	DC32	ZERO            // ( TOS> 0 20h TIB -- ) null
	DC32	BRAN	        // Goto store these at end of loop
	DC32	 EXPE5-.

EXPE4:
	DC32	DUP	        // Regular char 

EXPE5:
	DC32	I               // I=TIB Store char
	DC32	CSTORE          // or 1rst null in CASE OF CR
	DC32	ZERO            // 1rst or second null if cr
        // CASE OF CR ( TOS> 0 20h TIB -- )
	DC32	I
	DC32	ONEP
	DC32	CSTORE
EXPE3:
        // CASE OF CR ( TOS> 20H TIB -- )
	DC32	EMIT
EXPE33:
	DC32	XLOOP
	DC32	 EXPE1-.        // Loop is LEAVE'd

#ifdef XON_XOFF
	DC32	XOFF	        // FOR CASE OF count reached befor cr
#endif
// Creat count byte        
        // ( ORIGINAL ADDR -- )
        // ORIGINAL ADDR (TIB) ON STACK HERE SO CORRECT FOR $LEN
        DC32    ONEP            // PAST COUNT BYTE FOR CORRECT COUNT
        // NULLSTRLEN:          // ( addr -- addr len )
        DC32    NULLSTRLEN      // ( TOS> LEN TIB+1 -- )
        DC32    SWAP, ONEM      // BACK UP TO COUNT BYTE
        DC32    CSTORE
	DC32	SEMIS


//	COUNT COUNT:	( addr1 --- addr2 n )
//	Leave the byte address addr2 and byte count n of a count byte string,
//      beginning at address addr1. It is presumed that the first byte at
//      addr1 contains the text byte count and the actual text starts with
//      the second byte. NFA's may report SMUDGE'd byte counts, handled only
//      by ID. User Strings may be up to 255 characters.
//      Typically COUNT is followed by TYPE.
//      See TYPE $LEN and EXPECT

 SECTION .text : CONST (2)
COUNT_NFA:
	DC8	0x85
	DC8	'COUN'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	EXPECT_NFA
COUNT:
	DC32	DOCOL
	DC32	DUP
	DC32	ONEP
	DC32	SWAP
//        DC32    CATLT7F
	DC32	CAT     // n could be SMUDGE'd count from NFA's
	DC32	SEMIS   // Only internal usage is ID.


//      $LEN NULLSTRLEN:	( addr - addr len )
//      Count length of null terminated string.
//      TYPE can be used after this word.

 SECTION .text : CONST (2)
NULLSTRLEN_NFA:
	DC8	0x84
	DC8	'$LE'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	COUNT_NFA
NULLSTRLEN:
	DC32	.+5
 SECTION .text : CODE (2)
	NDPOP2w				// Get but leave addr on stack
	EORS		t, t, t		// zero count

NSLEN_LOOP:
	LDRB	        n, [w,t]
	ORRS		n, n, n
	BEQ		NSLEN_DONE
	ADDS		t, t, #1
	B		NSLEN_LOOP

NSLEN_DONE:
	TPUSH


//	FORGET FORGET:	( -- )
//	PRINTS NEW HERE IF SUCCESSFUL. FIXED FIG VERSION MEMORY LEAK.
//	Executed in the form:
//	FORGET CCcc
//	Deletes definition named CCcc from the dictionary with all entries
//	physically following it. In fig-FORTH, an error message will occur
//	if the CURRENT and CONTEXT vocabularies are not currently the same.
//	MODIFIED: To update Current along with context and reset DP,
//	and execute .DS and .VS

 SECTION .text : CONST (2)
FORGET_NFA:
	DC8	0x86
	DC8	'FORGE'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	NULLSTRLEN_NFA
FORGET:
	DC32	DOCOL
	DC32	TICK		        // \ -- pfa ELSE error message
	DC32	DUP
        DC32    LIT, FLASH_SPAGE        // DONT FORGET FISH WORDS SAVED IN FLASH
	DC32	LESSTHAN
	DC32	LIT, msg_forget_fish
	DC32	QERROR                  // \ f nullstr-addr --
        DC32	DUP
        DC32    LIT, FLASH_PPAGE        // DONT FORGET USER WORDS IN FLASH
	DC32	LESSTHAN
	DC32	LIT, msg_forget_saved
	DC32	QERROR                  // \ f nullstr-addr --
//	MOVE NFA update of the DP to after Currrent and Context
	DC32	DUP		        // PFA PFA --
	DC32	LFA                     // PFA LFA --

// VAR space reclamation starts here:
// PFA LFA --
        DC32    TOR                     // LFA_BOT

VS_RECVR_LOOP:
        DC32    LATEST, PFA, LFA        // LFA_TOP
        DC32    DUP, AT                 // LFA NEXT_LFA
        DC32    CURRENT_SV, STORE       // SET NEXT WORD TO CHECK
        DC32    DUP, FOURP, AT          // GET CFA
        DC32    LIT, DOVAR, EQUAL       // IS IT A DOVAR
        DC32    ZBRAN
        DC32     VS_RECVR_NEXT-.
        
        DC32    LIT, -4, UP_SV, PSTORE  // DECREMENT UP

VS_RECVR_NEXT:
        DC32    R, EQUAL
        DC32    ZBRAN
        DC32     VS_RECVR_LOOP-.

        DC32    RFROM
// VAR SPACE RECLAMATION DONE

// PFA LFA --
	DC32	AT              // LFA
	DC32	CURRENT_SV
	DC32	STORE
	DC32	NFA             // FROM PFA
	DC32	DP_SV
	DC32	STORE
	DC32	DOTVARSPACE, DOTDICTSPACE
	DC32	SEMIS


//	BYE BYE: ( -- )	COLD RESTART WITH SERIAL INITIALIZATION

 SECTION .text : CONST (2)
BYE_NFA:
	DC8	0x83
	DC8	'BY'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FORGET_NFA
BYE:
	DC32 	.+5
 SECTION .text : CODE (2)
        B       FM3_COLD      // __iar_program_start
 LTORG


//	ABORT ABORT:	( -- )
//	Clear the stacks, zero out and execute RUN or QUIT
//	DOES NOT RESET DICTIONAIRY. SEE COLD.

 SECTION .text : CONST (2)
ABORT_NFA:
	DC8	0x85
	DC8	'ABOR'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BYE_NFA
ABORT:
	DC32	DOCOL
	DC32	SPSTO
	DC32    RPSTO
#ifdef IO2TP
//ABORT_BP1:
//	DC32    NOOP    // TEMP TEST IO2TP
	DC32	CLRTIB  // Resets IN
	DC32	CLRPAD  // Resets OUT
#endif
        DC32    zero_OUT
//	DC32	QSTACK	// IF STACK error abort using QERROR->ERROR

// LOOK UP RUN AND EXEC IT
        DC32    LIT, msg_RUN
        DC32    LATEST
//ABORT_BP1_B4_PFIND_RUN:
// DC32 NOOP
        DC32    PFIND   // 0 OR pfa len 1
        DC32    ZBRAN
        DC32      ABORT_QUIT-.

        DC32    DROP    // LEN
        DC32    CFA, EXEC
ABORT_QUIT:
//      DC32    SEMIC_CREATE // Use ERROR if need to recover from bad definition
#ifdef ABORT_STOP_TILL_CO
        DC32    LIT, ERROR_HALT, AT
        DC32    ZBRAN
        DC32      CO_END-.
// halt on all errors until "CO" entered
// BEGIN
        DC32    LIT, msg_CO, NULLSTRLEN, TYPE
NOT_CO:
        DC32    QKEY
        DC32    ZBRAN
        DC32      NOT_CO-.
// IF KEY = "C'
        DC32    KEY, LIT, 43h, EQUAL
        DC32    ZBRAN
        DC32      NOT_CO-.
// NEXT KEY "0"
        DC32    KEY, LIT, 4Fh, EQUAL
        DC32    ZBRAN
        DC32      NOT_CO-.
// NEXT KEY "Enter"
        DC32    KEY, LIT, 0Dh, EQUAL
        DC32    ZBRAN
        DC32      NOT_CO-.
// UNTIL
        DC32    CR
CO_END:
#endif
	DC32	QUIT	// OI
#ifdef ABORT_STOP_TILL_CO
$FISH_CO_MSGS.h
#endif

//	COLD COLD:	( -- )	MODIFIED:
//	FIGISH SYSTEM COLD = WARM (INIT RAMVARSPACE) -> type signon MSG ->ABORT
//	CLEARS DICTIONAIRY. SEE ABORT.

 SECTION .text : CONST (2)
COLD_NFA:
	DC8	0x84
	DC8	'COL'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ABORT_NFA
COLD:
	DC32	DOCOL
	DC32	WARM		// FISH System VAR init.
	DC32	SIGNON
#ifdef ABORT_STOP_TILL_CO
        DC32    QUIT
#else
	DC32	ABORT
#endif

//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_SYS: = FISH System: CATEGORY

 SECTION .text : CONST (2)
WC_FISH_SYS_NFA:
	DC8	0x80+4+12
        DC8     0x0D, 0x0A
	DC8	'FISH System:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    COLD_NFA


//----------------------------- DOTSTACK SECTION -------------------------------

//	.SH DOTSHEX:	( -- )
//	Nondestructively print items on stack in HEX, preserving base.
//	Prints TOS 1rst, i.e. TOS, NOS, etc.

 SECTION .text : CONST (2)
DOTSHEX_NFA:
	DC8	0x83
	DC8	'.S'
	DC8	'H'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_FISH_SYS_NFA
DOTSHEX:
	DC32	DOCOL
        DC32    LIT, 16, DOTSBASE
	DC32	SEMIS


//	.SD DOTSDEC:	( -- )
//	Nondestructively print items on stack in DECIMAL, preserving base.
//	Prints TOS 1rst, i.e. TOS, NOS, etc.

 SECTION .text : CONST (2)
DOTSDEC_NFA:
	DC8	0x83
	DC8	'.S'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTSHEX_NFA
DOTSDEC:
	DC32	DOCOL
        DC32    LIT, 10, DOTSBASE
	DC32	SEMIS


//	.SB DOTSBIN:	( -- )
//	Nondestructively print items on stack in BINARY, preserving base.
//	Prints TOS 1rst, i.e. TOS, NOS, etc.

 SECTION .text : CONST (2)
DOTSBIN_NFA:
	DC8	0x83
	DC8	'.S'
	DC8	'B'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTSDEC_NFA
DOTSBIN:
	DC32	DOCOL
	DC32	TWO, DOTSBASE
	DC32	SEMIS


//	.S DOTS:	( -- )
//	Nondestructively print items on stack in current base.
//	Prints TOS 1rst, i.e. TOS, NOS, etc.

 SECTION .text : CONST (2)
DOTS_NFA:
	DC8	0x82
	DC8	'.'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTSBIN_NFA
DOTS:
	DC32	DOCOL
	DC32	INITSO_SV, AT, SPAT, FOURP
	DC32	EQUAL, ZEQU, ZBRAN
	DC32	DOTSEND-.	                // IF NOTHING ON STACK

	DC32	PDOTQ
	DC8	5
	DC8	'TOS> '
 ALIGNROM 2,0xFFFFFFFF
      
	DC32	INITSO_SV, AT, SPAT, FOURP
	DC32	XDO

DOTSLOOP:
	DC32	I, AT
	DC32	BASE_SV, AT
        DC32    LIT, 10, EQUAL                  // 
        DC32    ZBRAN                           // NOT = DECIMAL
        DC32    DBSUSEDUDOT-.
        DC32    DOT_BASE_SUFFIX
        DC32    BRAN
        DC32    DBSUSEDOT-.

DBSUSEDUDOT:
        DC32    UDOT_BASE_SUFFIX

DBSUSEDOT:
        DC32    SPACE
	DC32	FOUR, XPLOOP
	DC32	DOTSLOOP-.

DOTSEND:
	DC32	SEMIS


//	2DUP TDUP:	( n2 n1 -- n2 n1 n2 n1 )
//	Duplicate top two stack items. The prefix 2 convention means
//      an operation on the top two stack items.

 SECTION .text : CONST (2)
TDUP_NFA:
	DC8	0x84
	DC8	'2DU'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTS_NFA
TDUP:
	DC32 	.+5
 SECTION .text : CODE (2)
// TDUP: OPT by picking pops
	LDR     t, [p]          //
	LDR     w, [p, #4]      //
	DPUSH			// 


//	-DUP ZNDUP:	( n1 -- n1 (if zero)
//                      ( n1 -- n1 n1 (non-zero)
//	Reproduce n1 only if it is non-zero. Used in type andSPACES.
//      This is usually used to copy a value just before IF, to eliminate
//      the need for an ELSE part to drop it. 

 SECTION .text : CONST (2)
DDUP_NFA:
	DC8	0x84
	DC8	'-DU'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	TDUP_NFA
ZNDUP:
	DC32	DOCOL
	DC32	DUP
	DC32	ZBRAN	// IF
	DC32	DDUP1-.

	DC32	DUP	//endif
DDUP1:
	DC32	SEMIS


//	ROT ROT: ( nl n2 n3 --- n2 n3 nl )
//	Rotate the top three values on the stack, bringing the third to the top.

 SECTION .text : CONST (2)
ROT_NFA:
	DC8	0x83
	DC8	'RO'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DDUP_NFA
ROT:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2w
	POP2n
#ifdef TOSCT
        LDR     t, [p]    // get new TOS
#endif
	POP2t
	PUSHn
	DPUSH			//  --  LSW MSW )


//	I I:	( -- n )
//      Used within a DO-LOOP to copy the loop index to the stack. Other use
//      is implementation dependent.
//      See R.

 SECTION .text : CONST (2)
I_NFA:
	DC8	0x81
	DC8	'I'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ROT_NFA
I:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR     t, [r]	// GET INDEX VALUE
	TPUSH


//	SP@ SPAT:	( -- addr )
//	\ -- addr = Current TOP of the parameter stack (p) )
//      Place the address of the TOP of the parameter stack (p)
//	on the parameter stack (p) as it was before RP@ was executed.
//      e.g.  1  2  SP@  @  .   .   .     would type 2  2  1

 SECTION .text : CONST (2)
SPAT_NFA:
	DC8	0x83
	DC8	'SP'
	DC8	'@'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	I_NFA
SPAT:
	DC32	.+5
 SECTION .text : CODE (2)
	MOV	t, p
	TPUSH


//	RP@ RPAT:	( -- addr )
//	\ -- addr = Current TOP of the Return Stack (r) )
//      Place the address of the TOP of the return stack (r)
//	on the parameter stack (p) as it was before RP@ was executed.
//      e.g.  1 >R  2 >R  RP@  @  .   .   . would type 2  2  1

 SECTION .text : CONST (2)
RPAT_NFA:
	DC8	0x83
	DC8	'RP'
	DC8	'@'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPAT_NFA
RPAT:
	DC32	.+5
 SECTION .text : CODE (2)
	MOV	t, r
	TPUSH


//	LEAVE LEAVE:	( -- )
//      Force termination of a DO-LOOP at the next opportunity by setting
//      the loop limit equal to the current value of the index. The index
//      itself remains unchanged, and execution proceeds until LOOP
//      or +LOOP is encountered.

 SECTION .text : CONST (2)
LEAVE_NFA:
	DC8	0x85
	DC8	'LEAV'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	RPSTO_NFA
	DC32	RPAT_NFA
LEAVE:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR     w, [r]  	// GET Index
	STR     w, [r, #4]  // Store it at Limit
	NEXT


//	R> RFROM:	( -- n )
//      Remove the top value from the return stack and leave it on the
//      computation stack. See >R and R.

 SECTION .text : CONST (2)
RFROM_NFA:
	DC8	0x82
	DC8	'R'
	DC8	'>'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LEAVE_NFA
RFROM:
	DC32	.+5
 SECTION .text : CODE (2)
	POPr2t
	TPUSH


//	R R:	( -- n )
//	Copy the top of the return stack to the computation stack.

 SECTION .text : CONST (2)
R_NFA:
	DC8	0x81
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	RFROM_NFA
R:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR     t, [r]	// Get Index
	TPUSH

//	>R TOR:	( n -- )
//      Remove a number from the computation stack and place as the most
//      accessable on the return stack. Use should be balanced with R> in
//      the same definition.

 SECTION .text : CONST (2)
TOR_NFA:
	DC8	0x82
	DC8	'>'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	R_NFA
TOR:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2n   // preserve TOS
	PUSHn2r
#ifdef TOSCT
        LDR     t, [p]
#endif
	NEXT


//	OVER OVER:	( nl n2 -- nl n2 n1 )
//	Copy the second stack value, placing it as the new top.

 SECTION .text : CONST (2)
OVER_NFA:
	DC8	0x84
	DC8	'OVE'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	TOR_NFA
OVER:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2w		// n2
#ifdef TOSCT
// Get new t - This could become REFRESHt
	LDR	t, [p]		// t invalid so get it
        POP2t                   // do the increment
#else
	POP2t		// n1
#endif
	PUSHt		// -- n1 )
	DPUSH		//  --  LSW MSW )


//	DROP DROP:	( n1 -- )
//	Drop n1 from the stack.

 SECTION .text : CONST (2)
DROP_NFA:
	DC8	0x84
	DC8	'DRO'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	OVER_NFA
DROP:
	DC32	.+5
 SECTION .text : CODE (2)
#ifdef TOSCT
//        POP2t           // this one just adds 4 to p
        ADDS	p, p, #4
        LDR     t, [p]  // REFRESH t
#else // DROP:
//        POP2t		// Opt to just do p
        ADDS	p, p, #4

#endif
        NEXT


//	SWAP SWAP:	( nl n2 -- n2 n1 )
//	Exchange the top two values On the stack.

 SECTION .text : CONST (2)
SWAP_NFA:
	DC8	0x84
	DC8	'SWA'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DROP_NFA
SWAP:
	DC32	.+5
 SECTION .text : CODE (2)
#ifdef TOSCT
        MOV     w, t
        LDR     t, [p, #4]
        ADDS    p, p, #8
#else // SWAP:
	POP2w		// n2
	POP2t		// n1
#endif
	DPUSH		//  --  LSW MSW )


//	DUP DUP:	( n1 -- n1 n1 )
//	Duplicate the value n1 on the stack.

 SECTION .text : CONST (2)
DUP_NFA:
	DC8	0x83
	DC8	'DU'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SWAP_NFA
DUP:
	DC32	.+5
 SECTION .text : CODE (2)
//#ifdef TOSCT
// NO NEED TO REFRESH t ?
//#else DUP:
// OPT by picking pops
	LDR     t, [p]
//#endif
	TPUSH

//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_STACK_NFA = Stack Stuff: CATEGORY

 SECTION .text : CONST (2)
WC_STACK_NFA:
	DC8	0x80+4+12
        DC8     0x0D, 0x0A
	DC8	'Stack Stuff:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    DUP_NFA


//	ANDBITS ANDBITS:	( addr val -- )

 SECTION .text : CONST (2)
ANDBITS_NFA:
	DC8	0x87
	DC8	'ANDBIT'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_STACK_NFA
ANDBITS:
	DC32	.+5
 SECTION .text : CODE (2)
	POPp2w 		// val
	POP2n 		// addr
	LDR     t, [n]	// read [val]
	ANDS	t, t, w	// modify val
	STR	t, [n]	// Write val
	NEXT


//      SETBITS SETBITS:	( addr val -- ) OR val bits into addr.
//	See also CLRBITS.
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
	POPp2w 		// val
	POP2n 		// addr
	LDR     t, [n]	// read[val]
	ORRS	t, t, w	// modify val
	STR	t, [n]	// Write val
	NEXT


//      CLRBITS CLRBITS:	( addr val -- )
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
	POPp2w          	// val
	POP2n 		        // addr
	LDR	t, [n]	        // read [val]
        BICS    t, t, w         // modify val  - AND-NOT
	STR	t, [n]		// write val
	NEXT


//	CMOVE CMOVE:	( from to count -- )
//      Move the specified quantity of bytes beginning at address from to
//      address to. The contents of address from is moved first proceeding
//      toward high memory. Further specification is necessary on word
//      addressing computers.

 SECTION .text : CONST (2)
CMOVE_NFA:
	DC8	0x85
	DC8	'CMOV'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CLRBITS_NFA
CMOVE:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2n //    ldr	n, [p],#4      //COUNT
	POP2w //    ldr	w, [p],#4      //DEST
	POP2x //    ldr	x, [p],#4      //SOURCE
	CMP     n, #0
	BEQ     CM2
CM1:
//not for THUMB
//strcpy  LDRB R2, [R1], #1
//        STRB R2, [R0], #1
//        TST R2, R2      // repeat if R2 is nonzero
//        BNE strcpy
	LDRB    t, [x]
	STRB    t, [w]
	SUBS    n, n, #1
	CMP     n, #0
	BEQ     CM2

	ADDS	w, w, #1
	ADDS	x, x, #1
	BNE     CM1
CM2:
#ifdef TOSCT
        LDR     t, [p]  // REFRESH t
#endif
	NEXT


//	FILL FILL:	( addr quan b -- )
//      Fill memory at the address with the specified quantity of bytes b.

 SECTION .text : CONST (2)
FILL_NFA:
	DC8	0x84
	DC8	'FIL'
	DC8	'L'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CMOVE_NFA
FILL:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t			// Fill CHAR
	POP2n			// Fill COUNT
	POP2w			// Beginning ADDR
	CMP     n, #0
	BEQ     FEND            // Count is zero
	ADDS	x, n, w
FLOOP:
	STRB	t, [w]
	ADDS	w, w, #1
	CMP	w, x
	BNE	FLOOP
FEND:
#ifdef TOSCT
        LDR     t, [p]  // REFRESH t
#endif
	NEXT


//	ERASE ERASE:	( addr n -- )
//      Clear a region of memory to zero from addr over n addresses.

 SECTION .text : CONST (2)
ERASE_NFA:
	DC8	0x85
	DC8	'ERAS'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FILL_NFA
ERASE:
	DC32	DOCOL
	DC32	ZERO
	DC32	FILL
	DC32	SEMIS


//	BLANKS BLANKS:	( addr count -- )
//      Fill an area of memory beginning at addr with blanks (0x20).

 SECTION .text : CONST (2)
BLANKS_NFA:
	DC8	0x86
	DC8	'BLANK'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ERASE_NFA
BLANKS:
	DC32	DOCOL
	DC32	BLANK
	DC32	FILL
	DC32	SEMIS


//	+! PSTORE:	( n addr -- )
//	Add n to the value at the address.
//	Pronounced Plus Store

 SECTION .text : CONST (2)
PSTORE_NFA:
	DC8	0x82
	DC8	'+'
	DC8	'!'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BLANKS_NFA
PSTORE:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2n			// ADDRESS
#ifdef TOSCT
        LDR     t, [p]          // REFRESH t
#endif
	POP2t			// INCREMENT
	LDR     w, [n]
	ADDS    t, t, w
	STR     t, [n]
	NEXT


//	@ AT:	( addr -- n )
//	Read 32 bit contents of address to TOS.
//	Pronounced Fetch, as in Fetch word at addr.

 SECTION .text : CONST (2)
AT_NFA:
	DC8	0x81
	DC8	'@'+0x80 //C0H
 ALIGNROM 2,0xFFFFFFFF
//	DC32	TOGGLE_NFA
	DC32	PSTORE_NFA
AT:
	DC32	.+5
 SECTION .text : CODE (2)
//#ifdef TOSCT
//        LDR     t, [t]
//        STR     t, [p]
//        NEXT
//#else AT:
	POP2n
	LDR     t, [n]
	TPUSH
//#endif
	
//	C@ CAT:	( addr -- b )
//	Leave the 8 bit contents of addr on the stack.
//	Pronounced "Char Fetch", as in Fetch byte at addr.

 SECTION .text : CONST (2)
CAT_NFA:
	DC8	0x82
	DC8	'C'
	DC8	'@'+0x80 //C0H
 ALIGNROM 2,0xFFFFFFFF
	DC32	AT_NFA
CAT:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2n
	LDRB    t, [n]
	TPUSH


//	! STORE:	( n addr -- )
//      Store n at address. Pronounced "Store".

 SECTION .text : CONST (2)
STORE_NFA:
	DC8	0x81
	DC8	'!'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CAT_NFA
STORE:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t		// ADDR
	POP2n		// DATA
	STR	n, [t]
	NEXT


//	C! CSTORE:	( n addr -- )
//      Store LS 8 bits of n at address. Pronounced "Char Store".

 SECTION .text : CONST (2)
CSTORE_NFA:
	DC8	0x82
	DC8	'C'
	DC8	'!'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	STORE_NFA
CSTORE:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	POP2n
	STRB    n, [t]
	NEXT


//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_RWMEMORY_NFA = Read and Write Memory With: CATEGORY

 SECTION .text : CONST (2)
WC_RWMEMORY_NFA:
	DC8	0x80+31
        DC8     0x0D, 0x0A
	DC8	'Read and Write Memory With:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    CSTORE_NFA


//	] RBRAC:	( -- )
//	Resume compilation, by STATE back to 0xC0 (zero is interpreting)
//      0xC0 STATE  used in INTERPRET to execute Immediate words while compiling
//      See [.

 SECTION .text : CONST (2)
RBRAC_NFA:
	DC8	0x81
	DC8	']'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	SMUDGE_NFA
	DC32	WC_RWMEMORY_NFA
RBRAC:
	DC32	DOCOL
        DC32    strva, 0xC0, CSTATE
	DC32	SEMIS


//	[ LBRAC:	( -- ) IMMEDIATE
//	Used in a colon-definition in form
//              xxx    [  words   ]    more
//	Suspend compilation by setting STATE to zero (0xC0 is compiling).
//	The words after [ are executed, not compiled. This allows calculation
//	or compilation exceptions before resuming compilation with ] .
//	See LITERAL and ]

 SECTION .text : CONST (2)
LBRAC_NFA:
	DC8	0x0C1
	DC8	'['+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	RBRAC_NFA
LBRAC:
	DC32	DOCOL
	DC32	strva, 0, CSTATE
	DC32	SEMIS


//	[COMPILE] BCOMP:	( -- ) IMMEDIATE
//	Used in a colon-definition in form:
//              :  xxx    [COMPILE]   immediate_word   ;
//	[COMPILE] will force the compilation of an IMMEDIATE WORD
//	that would otherwise execute during compilation. The above example
//	will select the FORTH vocabulary then xxx executes, rather than at
//	compile time.

 SECTION .text : CONST (2)
BCOMP_NFA:
	DC8	0x0C9
	DC8	'[COMPILE'
	DC8	']'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LBRAC_NFA
BCOMP:
	DC32	DOCOL
	DC32	DFIND   // noop there for TIB entry
//	-FIND \ --- pfa len tf (found) \ --- ff (not found)
	DC32	ZEQU
	DC32	LIT, msg_qstack
	DC32	QERROR    // \ f nullstr-addr --
	DC32	DROP
	DC32	CFA
	DC32	COMMA	// alighned, INCREMENTS DP REFLECTED by HERE
	DC32	SEMIS


//	DLITERAL DLITERAL:	( d -- d ) IMMEDIATE
//				( d -- )   (compiling)
//	If compiling, compile a stack double number into a literal. Later
//	execution of the definition containing the literal will push it to
//	the stack. If executing, the number will remain on the stack.
//	: xxx    [ calculate ]  DLITERAL  ; //

 SECTION .text : CONST (2)
DLITERAL_NFA:
	DC8	0x0C8
	DC8	'DLITERA'
	DC8	'L'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BCOMP_NFA
DLITERAL:
	DC32	DOCOL
	DC32	STATE_SV
	DC32	AT
	DC32	ZBRAN	// IF
	DC32	 DLIT1-.

	DC32	SWAP
	DC32	LITERAL
	DC32	LITERAL	// endif
DLIT1:
	DC32	SEMIS


//	LITERAL LITERAL:	( n -- ) IMMEDIATE
//	If compiling, then compile the stack value n as a 32 bit literal.
//	This definition is immediate so that it will execute during a colon
//	definition. The intended use is:
//	: xxx    [ calculate ]  LITERAL  ; //
//	Compilation is suspended for the compile time calculation of value.
//	Compilation is resumed and LITERAL compiles this value.

 SECTION .text : CONST (2)
LITERAL_NFA:
	DC8	0x0C7
	DC8	'LITERA'
	DC8	'L'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DLITERAL_NFA
LITERAL:
	DC32	DOCOL
	DC32	STATE_SV
	DC32	AT
	DC32	ZBRAN   	// If not compiling
	DC32	 LITE1-.        // go here

	DC32	COMP            // Compile
	DC32	LIT             // Literal
	DC32	COMMA	        // Then COMPILE THE LITERAL VALUE
LITE1:
	DC32	SEMIS


//	#> EDIGS: 	( d=(LSW MSW) -- addr count )
//	Terminates numeric output conversion by dropping dnum,
//	leaving the text address and character count suitable for type.

 SECTION .text : CONST (2)
EDIGS_NFA:
	DC8	0x82
	DC8	'#'
	DC8	'>'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LITERAL_NFA
EDIGS:
	DC32	DOCOL
	DC32	DROP
	DC32	DROP
	DC32	HLD_SV
	DC32	AT
	DC32	PAD_SV
	DC32	OVER
	DC32	SUBB
	DC32	SEMIS


//	HOLD HOLD:	( c -- )
//	Used between <# and #> to insert an ascii character into a pictured
//      numeric output string.
//      e.g. 0x2E HOLD will place a decimal point.

 SECTION .text : CONST (2)
HOLD_NFA:
	DC8	0x84
	DC8	'HOL'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	EDIGS_NFA
HOLD:
	DC32	DOCOL
	DC32	LIT,-1
	DC32	HLD_SV
	DC32	PSTORE
	DC32	HLD_SV
	DC32	AT
	DC32	CSTORE
	DC32	SEMIS


//	SIGN SIGN:	( n d=<LSW MSW> -- d=<LSW MSW> )
//      Place an ascii "-" sign just before a converted numeric output
//      string in the text output buffer when n is negative. n is discarded
//      but double number dnum is maintained. Must be used between <# and #>.

 SECTION .text : CONST (2)
SIGN_NFA:
	DC8	0x84
	DC8	'SIG'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	HOLD_NFA
SIGN:
	DC32	DOCOL
	DC32	ROT
	DC32	ZLESS
	DC32	ZBRAN		// If
	DC32	 SIGN1-.

	DC32	LIT, '-'	// 0x2D
	DC32	HOLD		// Endif
SIGN1:
	DC32	SEMIS


//	#S DIGS:	( d1=<LSW MSW> -- d2=<LSW MSW> )
//      Generates ascii text in the text output buffer, by the use of #,
//      until a zero double number d2 results.
//      Used between <# and #>.
//      EACH NUMBER = BASE*number+(character-'0')

 SECTION .text : CONST (2)
DIGS_NFA:
	DC8	0x82
	DC8	'#'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SIGN_NFA
DIGS:
	DC32	DOCOL
DIGS1:				// Begin generating ascii chars by DIG
	DC32	DIG		//\ d1=<LSW MSW> -- d2=QUO=<LSW MSW>
	DC32	OVER
	DC32	OVER
	DC32	OR
	DC32	ZEQU
	DC32	ZBRAN	        //UNTIL
	DC32	 DIGS1-.
	DC32	SEMIS


//	# DIG:	( d1=<LSW MSW> -- d2=QUO=<LSW MSW> ) BASE MUST BE SET!
//	Generate from a double number d1, the next ascii character which is
//      placed in an output string. Result d2 is the quotient after division
//      by base, and is maintained for further processing.
//      Used between <# and #>.
//      See #S.
//      EACH NUMBER = BASE*number+(character-'0')

 SECTION .text : CONST (2)
DIG_NFA:
	DC8	0x81
	DC8	'#'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DIGS_NFA
DIG:
	DC32	DOCOL
	DC32	BASE_SV         // EX 0 1 0
	DC32	AT		// -- 0 1 0 A
	DC32	MSMOD           // -- 0 1 0 0
	DC32	ROT             // -- 0 0 0 1
	DC32	LIT,9
	DC32	OVER
	DC32	LESSTHAN
	DC32	ZBRAN	        // If
	DC32	 DIG1-.

	DC32	LIT,7
	DC32	PLUS	        // Endif
DIG1:
	DC32	LIT, 0x30
	DC32	PLUS
	DC32	HOLD
	DC32	SEMIS


//	<# BDIGS: 	( -- )
//	Setup for pictured numeric output formatting using the words:
//              <# # #S SIGN HOLD #>
//      The conversion is done on a double number producing text at Pad

 SECTION .text : CONST (2)
BDIGS_NFA:
	DC8	0x82
	DC8	'<'
	DC8	'#'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DIG_NFA
BDIGS:
	DC32	DOCOL
        DC32    strva, PAD, NHLD
	DC32	SEMIS


//	DOES> DOES:	( pfa -- ) MODIFIED:
//	A word which defines the run-time action within a high-level
//      defining word. DOES> alters the code field and first parameter of
//      the new word to execute the sequence of compiled word addresses
//      following DOES>. Used in combination with <BUILDS. When the DOES>
//      part executes it begins with the address of the first parameter of
//      the new word on the stack. This allows interpretation using this
//      area or its contents. Typical uses include the Forth assembler,
//      multidimensional arrays, and compiler generation.

 SECTION .text : CONST (2)
DOES_NFA:
	DC8	0x85
	DC8	'DOES'
	DC8	'>'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BDIGS_NFA
DOES:
	DC32	DOCOL
	DC32	RFROM	// Get the addr of the first word after DOES>
	DC32	LATEST	// ( -- addr ) nfa OF the current <creator> word
	DC32	PFA	// Use the pfa to store a run-time cfa
        DC32    DUP, CFA
        DC32    LIT, CREATE_DOES_GOTO
        DC32    SWAP, STORE     // STORE GOTO
	DC32	STORE	// Store the run-time cfa as 1rst pfa Parameter.
	DC32	SEMIS


//	<BUILDS BUILDS:	( -- )	MODIFIED: See CREATE and DOES>
//	Used within a colon-definition
//              CCcc  <BUILDS  ...  DOES>     ...
//	Each time CCcc is executed, <BUILDS defines a new word with a
//      high-level execution proceedure. Executing CCcc in the form
//              CCcc nnnn
//      uses <BUILDS to create a dictionary entry for nnnn with a call to
//      the DOES> part for nnnn. When nnnn is later executed, it has the
//      address of its parameter area on the stack and executes the words
//      after DOES> in CCcc. <BUILDS and DOES> allow runtime proceedures to
//      written in high-level code.

 SECTION .text : CONST (2)
BUILDS_NFA:
	DC8	0x87
	DC8	'<BUILD'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOES_NFA
BUILDS:
	DC32	DOCOL   // CREATE DOES DICTSPACE CHECK
	DC32	CREATE                  // CREATE's A CON POINTING TO IT'S PFA
	DC32	SEMIS


//      +LOOP PLOOP:	( n1 -- ) IMMEDIATE
//			( addr n2 -- ) COMPILE
//	At run time, DO begins a sequence with repetitive execution
//	controlled by a loop limit n1 and an index with initial value n2. DO
//	removes these from the stack. Upon reaching LOOP the index is
//	incremented by one.
//	Until the new index equals or exceeds the limit, execution loops
//	back to just after DO// otherwise the loop parameters are discarded
//	and execution continues ahead. Both n1 and n2 are determined at
//	run-time and may be the result of other operations.
//	Within a loop 'I' will copy the current value of the index to the
//	stack. See I, LOOP, +LOOP, LEAVE.
//
//	When compiling within the colon definition, DO compiles (DO), leaves
//	the following address addr and n for later error checking.

 SECTION .text : CONST (2)
PLOOP_NFA:
	DC8	0x0C5
	DC8	'+LOO'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BUILDS_NFA
PLOOP:
	DC32	DOCOL
	DC32	THREE
	DC32	QPAIR
	DC32	COMP
	DC32	XPLOOP
	DC32	BACK
	DC32	SEMIS


//	LOOP LOOP:	( addr n -- ) IMMEDIATE
//	Occurs in a colon-definition in form:
//		DO ... LOOP
//	At run-time, LOOP selectively controls branching back to the
//	corresponding DO based on the loop index and limit. The loop index
//	is incremented by one and compared to the limit. The branch back to
//	DO occurs until the index equals or exceeds the limit, then
//	the parameters are discarded and execution continues ahead.
//
//	At compile-time. LOOP compiles (LOOP) and uses addr to calculate an
//	offset to DO. n is used for error testing.

 SECTION .text : CONST (2)
LOOP_NFA:
	DC8	0x0C4
	DC8	'LOO'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	PLOOP_NFA
LOOP:
	DC32	DOCOL
	DC32	THREE
	DC32	QPAIR
	DC32	COMP
	DC32	XLOOP
	DC32	BACK
	DC32	SEMIS


//	DO DO:	( n1 n2 -- ) IMMEDIATE
//		(addr n -- ) COMPILE
//	Occurs in a colon-definition in form:
//	DO ... LOOP

 SECTION .text : CONST (2)
DO_NFA:
	DC8	0x0C2
	DC8	'D'
	DC8	'O'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LOOP_NFA
DO:
	DC32	DOCOL
	DC32	COMP
	DC32	XDO
	DC32	HERE
	DC32	THREE
	DC32	SEMIS


//	ENDIF ENDIF:	( addr n -- ) IMMEDIATE
//	Occurs in a colon-definition in form:
//		IF ... ENDIF
//		IF ... ELSE ... ENDIF
//	At run-time, ENDIF serves only as the destination of a forward
//	branch from IF or ELSE. It marks the conclusion of the conditional
//	structure. THEN is another name for ENDIF. Both names are supported
//	in fig-FORTH. See also IF and ELSE.
//
//	At compile-time, ENDIF computes the forward branch offset from addr
//	to HERE and stores it at addr. n is used for error tests.

 SECTION .text : CONST (2)
ENDIF_NFA:
	DC8	0x0C5
	DC8	'ENDI'
	DC8	'F'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DO_NFA
ENDIF:
	DC32	DOCOL
	DC32	QCOMP
	DC32	TWO     // 2 is the Error checking number for if.
	DC32	QPAIR
	DC32	HERE
	DC32	OVER
	DC32	SUBB
	DC32	SWAP
	DC32	STORE
	DC32	SEMIS


//	THEN THEN:	( -- ) IMMEDIATE
//	An alias for ENDIF.

 SECTION .text : CONST (2)
THEN_NFA:
	DC8	0x0C4
	DC8	'THE'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ENDIF_NFA
THEN:
	DC32	DOCOL
	DC32	ENDIF
	DC32	SEMIS


//	ELSE ELSE:	( addr1 n1 -- )	IMMEDIATE
//			( addr2 n2 -- )	COMPILING
//	Occurs within a colon-definition in the form:
//		IF ... ELSE ... ENDIF
//	At run-time, ELSE executes after the true part following IF. ELSE
//	forces execution to skip over the following false part and resumes
//	execution after the ENDIF. It has no stack effect.
//
//	At compile-time ELSE emplaces BRANCH reserving a branch offset,
//	leaves the address addr2 and n2 for error testing. ELSE also
//	resolves the pending forward branch from IF by calculating the
//	offset from addr1 to HERE and storing at addr1.

 SECTION .text : CONST (2)
ELSE_NFA:
	DC8	0x0C4
	DC8	'ELS'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	THEN_NFA
ELSE:
	DC32	DOCOL
	DC32	TWO
	DC32	QPAIR
	DC32	COMP
	DC32	BRAN
	DC32	HERE
	DC32	ZERO
	DC32	COMMA	// aligned, INCREMENTS DP REFLECTED by HERE
	DC32	SWAP
	DC32	TWO
	DC32	ENDIF
	DC32	TWO
	DC32	SEMIS


//	IF IF:	( f -- ) IMMEDIATE
//		( -- addr n ) COMPILE
//	Occurs is a colon-definition in form:
//		IF (tp) ...  ENDIF      .
//		IF (tp) ... ELSE (fp) ... ENDIF
//	At run-time, IF selects execution based on a boolean flag. If f is
//	true (non-zero), execution continues ahead thru the true part. If f
//	is false (zero), execution skips till just after ELSE to execute
//	the false part. After either part, execution resumes after ENDIF.
//	ELSE and its false part are optional.; if missing, false execution
//	skips to just after ENDIF..
//
//	At compile-time IF compiles 0BRANCH and reserves space for an offset
//	at addr. addr and n are used later for resolution of the offset and
//	error testing.

 SECTION .text : CONST (2)
IF_NFA:
	DC8	0x0C2
	DC8	'I'
	DC8	'F'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ELSE_NFA
IF:
	DC32	DOCOL
	DC32	COMP
	DC32	ZBRAN
	DC32	HERE
	DC32	ZERO
	DC32	COMMA
	DC32	TWO     // 2 is the Error checking number for if.
	DC32	SEMIS


//	UNTIL UNTIL:	( f -- ) IMMEDIATE
//			( addr n -- ) COMPILE:
//	Occurs within a colon-definition in the form:
//		BEGIN ... UNTIL
//	At run-time, UNTIL controls the conditional branch back to the
//	corresponding BEGIN. If f is false, execution returns to just after.
//	BEGIN: 	if true, execution continues ahead.
//	At compile-time, UNTIL compiles (0BRANCH) and an offset from HERE to
//	addr. n is used for error tests.

 SECTION .text : CONST (2)
UNTIL_NFA:
	DC8	0x0C5
	DC8	'UNTI'
	DC8	'L'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	IF_NFA
UNTIL:
	DC32	DOCOL
	DC32	ONE
	DC32	QPAIR
	DC32	COMP
	DC32	ZBRAN
	DC32	BACK
	DC32	SEMIS


//	AGAIN AGAIN:	( addr n -- ) IMMEDIATE
//	Used in a colon-definition in the form:
//			BEGIN ... AGAIN
//	At run-time, AGAIN forces execution to return to corresponding BEGIN.
//	There is no effect on the stack. Execution cannot leave this loop
//	(unless R> DROP is executed one level below).
//
//	At compile time, AGAIN compiles BRANCH with an offset from HERE to
//	addr. n is used for compile-time error checking.

 SECTION .text : CONST (2)
AGAIN_NFA:
	DC8	0x0C5
	DC8	'AGAI'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	UNTIL_NFA
AGAIN:
	DC32	DOCOL
	DC32	ONE
	DC32	QPAIR
	DC32	COMP
	DC32	BRAN
	DC32	BACK
	DC32	SEMIS


//	REPEAT REPEAT:	( addr n -- ) IMMEDIATE
//	Used within a colon-definition in the form:
//		BEGIN ... WHILE ... REPEAT
//	At run-time, REPEAT forces an unconditional branch back to just
//	after the corresponding BEGIN.
//
//	At compile-time, REPEAT compiles BRANCH and the offset from HERE to
//	addr. n is used for error testing.

 SECTION .text : CONST (2)
REPEAT_NFA:
	DC8	0x0C6
	DC8	'REPEA'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	AGAIN_NFA
REPEAT:
	DC32	DOCOL
	DC32	TOR
	DC32	TOR
	DC32	AGAIN
	DC32	RFROM
	DC32	RFROM
	DC32	TWO
	DC32	SUBB
	DC32	ENDIF
	DC32	SEMIS


//	WHILE WHILE:	( f -- ) IMMEDIATE
//			(ad1 nl -- ad1 n1 ad2 n2 ) COMPILE
//	Occurs in a colon-definition in the form:
//		BEGIN ... WHILE (tp) ... REPEAT
//	At run-time, WHILE selects conditional execution based on boolean
//	flag f. If f is true (non-zero), WHILE continues execution of the
//	true part thru to REPEAT, which then branches back to BEGIN. If f is
//	false (zero), execution skips to just after REPEAT, exiting the
//	structure.
//
//	At compile time, WHILE emplaces (0BRANCH) and leaves ad2 of the
//	reserved offset. The stack values will be resolved by REPEAT.

 SECTION .text : CONST (2)
WHILE_NFA:
	DC8	0x0C5
	DC8	'WHIL'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	REPEAT_NFA
WHILE:
	DC32	DOCOL
	DC32	IF
	DC32	TWOP
	DC32	SEMIS


//	BEGIN BEGIN:	( -- addr n ) IMMEDIATE
//	Occurs in a colon-definition in form:
//	BEGIN ... UNTIL
//	BEGIN ... AGAIN
//	BEGIN ... WHILE ... REPEAT
//	At run-time, BEGIN marks the start of a sequence that may be
//	repetitively executed. It serves as a return point from the
//	corresponding UNTIL, AGAIN or REPEAT. When executing UNTIL, a return
//	to BEGIN will occur if the top of the stack is false//
//	for AGAIN and REPEAT a return to BEGIN always occurs.
//
//	At compile time BEGIN leaves its return address and n for compiler
//	error checking.

 SECTION .text : CONST (2)
BEGIN_NFA:
	DC8	0x0C5
	DC8	'BEGI'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WHILE_NFA
BEGIN:
	DC32	DOCOL
	DC32	QCOMP
	DC32	HERE
	DC32	ONE
	DC32	SEMIS


//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_COMPILE_NFA = Inside Colon WORDS: CATEGORY

 SECTION .text : CONST (2)
WC_COMPILE_NFA:
	DC8	0x80+4+19
        DC8     0x0D, 0x0A
	DC8	'Inside Colon WORDS:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    BEGIN_NFA


//	IMMEDIATE IMMED:	( -- )
//	Mark the most recently made definition so that when encountered at
//      compile time, it will be executed rather than being compiled. i.e.
//	the precedence bit in its header is set.
//	This method allows definitions to handle unusual compiling
//	situations, rather. than build them into the fundamental compiler.
//	The system may force compilation of an immediate definition by
//	preceeding it with [COMPILE].

 SECTION .text : CONST (2)
IMMED_NFA:
	DC8	0x89
	DC8	'IMMEDIAT'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_COMPILE_NFA
IMMED:
	DC32	DOCOL
	DC32	LATEST
	DC32	LIT, 0x40
	DC32	TOGGLE
	DC32	SEMIS


//	CREATE CREATE:	( -- )
//      Create a word in The Dictionary.
//      Make searchable and forgetable NFA/LFA, DOCON, ADDR->PFA
//	USE <BUILDS with DOES> instead of CREATE.
//	IF DICTIONAIRY FULL PRINT Error MESSAGE AND Abort.
//
//      Saves HERE in CSDP in case there's an error creating a word.
//      CSDP is reset when a word is completed, by ; <BUILDS (?) CON and VAR.
//
//	A defining word used in the form:
//		CREATE CCcc
//	by such words as VAR, CON AND <BUILDS to create a dictionary header for
//	a new word. The code field contains the address of the words
//	parameter field.

 SECTION .text : CONST (2)
CREATE_NFA:
	DC8	0x86
	DC8	'CREAT'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	IMMED_NFA
CREATE:
	DC32	DOCOL
	DC32	DICTSPACE	//	\ -- n
        DC32    LIT, 15, SUBB   // One less than smallest definition
	DC32	ZLESS
	DC32	ZBRAN
	DC32	CREATEOK-.

	DC32	DICTFULL_ABORT

CREATEOK:
	DC32	DFIND                   // Parse nfa
//	-FIND DFIND \ --- pfa len tf (found) \ --- ff (not found)
	DC32	ZBRAN
	DC32	 CREA1-.

#ifdef IO2TP
//CREATE_BP1:
// DC32 NOOP
        DC32    CLRPAD  // Resets OUT
#endif
	DC32	DROP
	DC32	NFA
	DC32	IDDOT
	DC32	LIT, msg_wordexists
// Nothing created yet so ABORT is correct.
//        DC32    ERROR
        DC32    NULLSTRLEN, TYPE
        DC32    ABORT

CREA1:
// Save HERE in CSDP so ERROR can reset DP if this definition fails.
// CSDP is reset by SEMIC_CREATE when this definition is completed.
// SEMIC_CREATE used in ; CON and VAR to reset CSDP.

// This new definitions cstring name has been moved to HERE 
        DC32    HERE, DUP       // This will become the new CURRENT (this NFA).
        DC32    CSDP_SV, STORE  // HERE to CSDP to restore in case of error.
// Dup HERE-NFA to allot NFAx
        DC32    DUP
// Limit NFA allot to maxwordlen. DOES NOT CHANGE actual count!
// If count is larger than MAXWORDLEN behaviour of this def is unpredictable!
	DC32	CAT
	DC32	LIT, MAXWORDLEN // ELIMINATED WIDTH AND WIDTH_SV 
	DC32	MIN
	DC32	ALLOT_PRIM      // HERE (DP) at end of NFA with padding.
// Dup HERE-NFA to set count byte, and to be consumed later
// for setting CURRENT after previous CURRENT compiled into LFA.
        DC32    DUP
// Make regular (not immediate) NFA count byte
	DC32	LIT, 0x80
	DC32	TOGGLE          // Count Byte = 80+count
// HERE is new now. Mark end of NFA for PFIND search
	DC32	HERE            // DP after allot of NFA
	DC32	LIT,0x80        // Set last character in NFA
	DC32	TOGGLE          // smudge bit
// Bump DP to LFA
	DC32	ONE, DP_SV      // MOVE HERE (DP)
	DC32	PSTORE          // TO LFA
// Set LFA
	DC32	LATEST		// LATEST = CURRENT_SV -> MOST RECENT DEF NFA
	DC32	COMMA		// Compile last words NFA into this link field.
				// DUP'd HERE should be on stack
// Have to set CURRENT with last dup's HERE-NFA after LATEST is compiled
	DC32	CURRENT_SV 	// LATEST = CURRENT_SV
	DC32	STORE		// so storing NEWWORD_NFA into CURRENT.
        DC32    COMP, DOCON     // Compile DOCON as CFA
	DC32	HERE, FOURP     // Compute and
        DC32    COMMA           // COMPILE Addr of PFA
//	DC32	SMUDGE          // ELIMINATED
//      DC32    SEMIC_CREATE    // USED IN CALLERS OF CREATE, NOT HERE!
	DC32	SEMIS


//	CON CON:	( n -- )  MODIFIED: and CONSTANT renamed CON
//	IF DICTIONAIRY FULL PRINT Error MESSAGE AND Abort.
//	A defining word used in the form:
//              n CONSTANT CCcc
//	to create word CCcc, with its parameter field containing n. When
//	CCcc is later executed, it will push the value of n to the stack.

 SECTION .text : CONST (2)
CONSTANT_NFA:
	DC8	0x88
	DC8	'CONSTAN'
        DC8     'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CREATE_NFA
CONSTANT:
	DC32	DOCOL, CON, SEMIS   // CREATE DOES DICTSPACE CHECK


 SECTION .text : CONST (2)
CON_NFA:
	DC8	0x83
	DC8	'CO'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	CREATE_NFA
	DC32	CONSTANT_NFA
CON:
	DC32	DOCOL   // CREATE DOES DICTSPACE CHECK
	DC32	CREATE	// Make Header NFA/LFA, DOCON, ADDR->PFA
        DC32    LIT, -4, DP_SV, PSTORE  // back up to ADDR->PFA
	DC32	COMMA		        // Replace with Con value
        DC32    SEMIC_CREATE            // RESET AUTO FORGET
//        DC32    DOTDICTSPACE
	DC32	SEMIS


//	VAR VAR:	( -- ) RENAMED: VARIABLE to VAR
//	IF RAMVARSPACE FULL PRINT Error MESSAGE AND Abort.
//	A defining word used in the form:
//              VARIABLE CCcc
//	When VARIABLE is executed, it creates the definition CCcc with its
//	with its cell space uninitialized. When CCcc is executed,
//	its cell space address is put on the stack. Use with @ or !
//	NOTE: VAR's cell space is allocated in ram. See UP and VARALLOT.

 SECTION .text : CONST (2)
VARIABLE_NFA:
	DC8	0x88
	DC8	'VARIABL'
        DC8     'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CON_NFA
VARIABLE:
	DC32	DOCOL, VAR_, SEMIS   // CREATE DOES DICTSPACE CHECK

 SECTION .text : CONST (2)
VAR_NFA:
	DC8	0x83
	DC8	'VA'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	CON_NFA
	DC32	VARIABLE_NFA
VAR_:
	DC32	DOCOL   // CREATE DOES DICTSPACE CHECK
	DC32	CREATE	// Make Header NFA and LFA, update CURRENT
        DC32    LIT, -8, DP_SV, PSTORE
        DC32    COMP, DOVAR     // CFA of VAR's
//        DC32    COMP, DOCON     // CFA of VAR's
// aligned, INCREMENTS DP REFLECTED by HERE
// compile the value \ v --
	DC32	ONE
	DC32	VARALLOT	// Do .VS check and return VAR addr.
	DC32	COMMA		// Compile address of the ram allotment.
        DC32    SEMIC_CREATE            // RESET AUTO FORGET
//        DC32    DOTDICTSPACE
//        DC32    DOTVARSPACE
	DC32	SEMIS


//	; SEMI:	( -- ) IMMEDIATE
//	Terminate a colon-definition and stop further compilation.
//	Compiles the run-time ;S.

 SECTION .text : CONST (2)
SEMI_NFA:
	DC8	0x0C1
	DC8	';'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    VAR_NFA
SEMI:
	DC32	DOCOL
	DC32	QCSP
	DC32	COMP
	DC32	SEMIS
	DC32	LBRAC
        DC32    SEMIC_CREATE            // RESET AUTO FORGET
//        DC32    DOTDICTSPACE
	DC32	SEMIS


//	: COLON:	( -- ) IMMEDIATE
//	IF DICTIONAIRY FULL PRINT Error MESSAGE AND Abort.
//	Used in the form called a colon-definition:
//      	: CCcc     ...     //
//	Creates a dictionary entry defining CCcc as equivalent to the
//	following sequence of Forth word definitions '...' until the next
//	';' or ';CODE'.
//	The compiling process is done by the text interpreter as long as
//	STATE is non-zero. Other details are that the CONTEXT vocabulary is
//	set to the CURRENT vocabulary and that words with the precedence bit
//	set (P) are executed rather than being compiled.

 SECTION .text : CONST (2)
COLON_NFA:
	DC8	0x0C1
	DC8	':'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SEMI_NFA
COLON:
	DC32	DOCOL
//	DICTSPACE TEST DONE IN CREATE
	DC32	QEXEC
	DC32	SCSP	// Save the stack position in CSP. Compiler security.
	DC32    CREATE	// Make Header NFA and LFA, update CURRENT
	DC32	RBRAC	// Resume compiliting by setting STATE_SV to 0xC0
	DC32	LIT, -8, DP_SV, PSTORE
        DC32    COMP, DOCOL     // aligned, INCREMENTS DP REFLECTED by HERE
	DC32 	SEMIS


//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_CREATE_NFA = CREATE WORDS With: CATEGORY

 SECTION .text : CONST (2)
WC_CREATE_NFA:
	DC8	0x80+22
        DC8     0x0D, 0x0A
	DC8	'CREATE WORDS With:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    COLON_NFA

//	DABS DABS: ( sd=(LSW MSW) -- ud=(LSW MSW) )
//      Leave the absolute value ud of a signed double number.

 SECTION .text : CONST (2)
DABS_NFA:
	DC8	0x84
	DC8	'DAB'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_CREATE_NFA
DABSF:
DABS:
	DC32	DOCOL
	DC32	DUP
	DC32	DPM
	DC32	SEMIS


//	DNEGATE DNEGATE: ( d1 -- d<2>=(LSW MSW) ) RENAMED: DMINUS to DNEGATE
//      Convert d1 to its double number two's complement d2.

 SECTION .text : CONST (2)
DNEGATE_NFA:
	DC8	0x87
	DC8	'DNEGAT'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DABS_NFA
DNEGATE:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t			// MSW   //POP	BX
	POP2w			// LSW   //POP	CX
	MVNS    t, t            // negate MSW
	MVNS    w, w            // negate LSW
	ADDS	w, w, #1        // add 1 to LSW
	DPUSH           	//  --  LSW MSW )


//	DPL_SV:	( -- addr of NDPL ) Contains # of digits after . in double number
//	A system variable containing the number of digits to the right of the
//	decimal on double integer input. It may also be used to hold output
//	column location of a decimal point, in system generated formating. The
//	default value on single number input is -1.
 SECTION .text : CONST (2)
DPL_NFA:
	DC8	0x83
	DC8	'DP'
	DC8	'L'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DNEGATE_NFA
DPL_SV:
        DC32    DOCON, NDPL


//	D.R DDOTR:	( sd=<LSW MSW> n -- ) SIGNED:
//	Print a signed double number sd right aligned
//	in a field n characters wide.

 SECTION .text : CONST (2)
DDOTR_NFA:
	DC8	0x83
	DC8	'D.'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DPL_NFA
DDOTR:
	DC32	DOCOL
	DC32	TOR	// N CHAR FIELD LEN to r
	DC32	SWAP	// ( LSW MSW -- MSW LSW )
	DC32	OVER	// ( MSW LSW -- MSW LSW MSW )
	DC32	DABS	// ( MSW LSW MSW -- MSW uLSW uMSW )
	DC32	BDIGS	// <# SET HLD to Pad
	DC32	DIGS	// ( d=<LSW MSW> -- d=<LSW MSW> )
	DC32	SIGN	// ( d=<LSW MSW> -- d=<LSW MSW> )
	DC32	EDIGS	//	#>  ( d=<LSW MSW>  ---  addr  count )
	DC32	RFROM	// GET THE N CHAR FIELD LEN
	DC32	OVER	// ( addr count n -- addr count n count )
	DC32	SUBB	// ( n addr count addr -- n addr count-n )
	DC32	SPACES	// SPACES  ( n -- )
	DC32	TYPE	// ( n addr -- )
	DC32	SEMIS


//	D. DDOT:	( S32-2'S COMPLEMET = (d=<LSW MSW> -- )
//	Print a signed double number from a 32 bit two's complement value.
//      The high-order 32 bits are most accessable on the stack. Conversion
//      is performed according to the current base. A blank follows.

 SECTION .text : CONST (2)
DDOT_NFA:
	DC8	0x82
	DC8	'D'
	DC8	'.'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DDOTR_NFA
DDOT:
	DC32	DOCOL
	DC32	ZERO
	DC32	DDOTR
	DC32	SPACE
	DC32	SEMIS


//	D+ DPLUS:	( LSW1 MSW1 LSW2 MSW2 --- LSW MSW )  SIGNED:
//	Leave the signed double number sum of two signed double numbers.

 SECTION .text : CONST (2)
DPLUS_NFA:
	DC8	0x82
	DC8	'D'
	DC8	'+'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DDOT_NFA
DPLUS:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t	//    ldr     t, [p],#4       // MS
	POP2n	//    ldr     n, [p],#4       // LS
	POP2x	//    ldr     x, [p],#4       // MS
	POP2w	//    ldr     w, [p],#4       // LS
	ADDS	w, w, n         // LS sum, set status flags
	ADCS    t, t, x         // MS sum + carry
	DPUSH			//  --  LSW MSW )


//	S->D STOD:	( n -- d=<LSW MSW> ) SIGNED:
//      Sign extend a single number to form a double number.
//	: S->D DUP 0< NEGATE // hi level equivalent

 SECTION .text : CONST (2)
STOD_NFA:
	DC8	0x84
	DC8	'S->'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DPLUS_NFA
STOD:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2w		        // POP LSW
	EORS	t, t		// Zero MSW
	ORRS    w, w, w         // OR LSW
	BPL     STOD1           // LSW is POS

	SUBS     t, t, #1       // LSW is NEG
STOD1:
	DPUSH			//  --  LSW MSW )


//	2* TWOSTAR:	( n -- n*2 ) LSL 1

 SECTION .text : CONST (2)
TWOSTAR_NFA:
	DC8	0x82
	DC8	'2'
	DC8	'*'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	STOD_NFA
TWOSTAR:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	LSLS	t, t, #1	//
	TPUSH


//	2/ TWOSLASH:	( n -- n/1 ) ASR 1 (FLOORED)

 SECTION .text : CONST (2)
TWOSLASH_NFA:
	DC8	0x82
	DC8	'2'
	DC8	'/'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	TWOSTAR_NFA
TWOSLASH:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	ASRS	t, t, #1	//
	TPUSH
        

//	1- ONEM:	( n -- n-1 )

 SECTION .text : CONST (2)
ONEM_NFA:
	DC8	0x82
	DC8	'1'
	DC8	'-'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	TWOSLASH_NFA
ONEM:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	SUBS	t, t, #1	//
	TPUSH


//	1+ ONEP:	( n -- n+1 )

 SECTION .text : CONST (2)
ONEP_NFA:
	DC8	0x82
	DC8	'1'
	DC8	'+'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ONEM_NFA
ONEP:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	ADDS	t, t, #1	//
	TPUSH


//	2+ TWOP:	( n -- n+2 )

 SECTION .text : CONST (2)
TWOP_NFA:
	DC8	0x82
	DC8	'2'
	DC8	'+'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ONEP_NFA
TWOP:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	ADDS	t, t, #2
	TPUSH


//	4+ FOURP:	( n -- n+4 )

 SECTION .text : CONST (2)
FOURP_NFA:
	DC8	0x82
	DC8	'4'
	DC8	'+'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	TWOP_NFA
FOURP:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	ADDS	t, t, #4
	TPUSH


//	4- FOURM:	( n -- n-4 )

 SECTION .text : CONST (2)
FOURM_NFA:
	DC8	0x82
	DC8	'4'
	DC8	'-'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FOURP_NFA
FOURM:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	SUBS	t, t, #4
	TPUSH


//	0 ZERO: ( -- 0 )

 SECTION .text : CONST (2)
ZERO_NFA:
	DC8	0x81
	DC8	'0'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FOURM_NFA
ZERO:
	DC32	DOCON, 0


//	1 ONE: ( -- 1 )

 SECTION .text : CONST (2)
ONE_NFA:
	DC8	0x81
	DC8	'1'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ZERO_NFA
ONE:
	DC32	DOCON, 1


//	2 TWO: ( -- 2 )

 SECTION .text : CONST (2)
TWO_NFA:
	DC8	0x81
	DC8	'2'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ONE_NFA
TWO:
	DC32	DOCON, 2


//	3 THREE: ( -- 3 )

 SECTION .text : CONST (2)
THREE_NFA:
	DC8	0x81
	DC8	'3'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	TWO_NFA
THREE:
	DC32	DOCON, 3


//	4 FOUR: ( -- 4 )

 SECTION .text : CONST (2)
FOUR_NFA:
	DC8	0x81
	DC8	'4'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	THREE_NFA
FOUR:
	DC32	DOCON, 4


//	M* MSTAR:	( n1 n2 -- d=<S32LSW S32MSW> )	SIGNED:
//      A mixed magnitude math operation which leaves the double number
//      signed product of two signed number.

 SECTION .text : CONST (2)
MSTAR_NFA:
	DC8	0x82
	DC8	'M'
	DC8	'*'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FOUR_NFA
MSTAR:
	DC32	DOCOL,TDUP
	DC32	XORR
	DC32	TOR
	DC32	ABS
	DC32	SWAP
	DC32	ABS
	DC32	USTAR
	DC32	RFROM
	DC32	DPM
	DC32	SEMIS


//	M/ MSLASH:	( sd  n1  ---  d=<s32REM  S32QUO> )
//      A mixed magnitude math operator which leaves the signed remainder
//      and signed quotient from sd, a double number dividend and divisor n1.
//      The remainder takes its sign from the dividend.

 SECTION .text : CONST (2)
MSLASH_NFA:
	DC8	0x82
	DC8	'M'
	DC8	'/'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	MSTAR_NFA
MSLASH:
	DC32	DOCOL
	DC32	OVER
	DC32	TOR
	DC32	TOR
	DC32	DABS	// ( LSW MSW -- uLSW uMSW )
	DC32	R
	DC32	ABS
	DC32	USLASH
  	DC32	RFROM
	DC32	R
	DC32	XORR
	DC32	PM
	DC32	SWAP
	DC32	RFROM
	DC32	PM
	DC32	SWAP
	DC32	SEMIS


//	* STAR:	( n1 n2 -- prod=n3 )
//	Leave the signed product n3 of two (32bit) signed numbers n1 and n2.

 SECTION .text : CONST (2)
STAR_NFA:
	DC8	0x81
	DC8	'*'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	MSLASH_NFA
STAR:
	DC32	DOCOL
	DC32	MSTAR
	DC32	DROP
	DC32	SEMIS


//	/MOD SLMOD:	( n1 n2 -- rem quot )	SIGNED:
//	Leave the remainder and signed quotient of n1/n2. The remainder has
//	the sign of the dividend.

 SECTION .text : CONST (2)
SLMOD_NFA:
	DC8	0x84
	DC8	'/MO'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	STAR_NFA
SLMOD:
	DC32	DOCOL
	DC32	TOR
	DC32	STOD
	DC32	RFROM
	DC32	MSLASH
	DC32	SEMIS


//	/ SLASH:        ( sn1 sn2 -- sn ) SIGNED:
//	Leave the signed quotient sn of sn1/sn2.

 SECTION .text : CONST (2)
SLASH_NFA:
	DC8	0x81
	DC8	'/'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SLMOD_NFA
SLASH:
	DC32	DOCOL
	DC32	SLMOD
	DC32	SWAP
	DC32	DROP
	DC32	SEMIS


//	MOD MOD:	( sn1 n2 -- modulo )
//	Leave the remainder of sn1/n2, with the same sign as sn1.

 SECTION .text : CONST (2)
MODD_NFA:
	DC8	0x83
	DC8	'MO'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SLASH_NFA
MOD:
	DC32	DOCOL
	DC32	SLMOD
	DC32	DROP
	DC32	SEMIS


//	*/MOD SSMOD:	( n1 n2 n3 -- n4 n5 )
//	Leave the quotient n5 and remainder n4 of the operation n1*n2/n3 A
//	31 bit intermediate product is used as for */.

 SECTION .text : CONST (2)
SSMOD_NFA:
	DC8	0x85
	DC8	'*/MO'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	MODD_NFA
SSMOD:
	DC32	DOCOL
	DC32	TOR
	DC32	MSTAR
	DC32	RFROM
	DC32	MSLASH
	DC32	SEMIS


//	*/ SSLASH:	( n1 n2 n3 -- n4 )
//	Leave the ratio n4 = n1*n2/n3 where all are signed numbers.
//	Retention of an intermediate 31 bit product permits greater accuracy
//	than would. be available with the sequence:
//		n1  n2  *  n3  /

 SECTION .text : CONST (2)
SSLASH_NFA:
	DC8	0x82
	DC8	'*'
	DC8	'/'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SSMOD_NFA
SSLASH:
	DC32	DOCOL
	DC32	SSMOD
	DC32	SWAP
	DC32	DROP
	DC32	SEMIS


//	+ PLUS:	( n1 n2 -- n3 )
//	Add n1 and n2, leaving sum n3

 SECTION .text : CONST (2)
PLUS_NFA:
	DC8	0x81
	DC8	0xAB	// '+' + 0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SSLASH_NFA
PLUS:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	POP2n
	ADDS	t, t, n
	TPUSH


//	- SUBB:	( n1 n2 -- n3 )
//	Leave the difference of n1-n2 as n3.
//	1 2 - . -1
//	2 1 - .  1

 SECTION .text : CONST (2)
SUB_NFA:
	DC8	0x81
	DC8	'-'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	PLUS_NFA
SUBB:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	POP2n
	SUBS	t, n, t
	TPUSH


//	= EQUAL:	(n1 n2 -- f )
//	Leave a non-zero true flag if n1=n2// otherwise leave a false flag.

 SECTION .text : CONST (2)
EQUAL_NFA:
	DC8	0x81
	DC8	'='+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SUB_NFA
EQUAL:
	DC32	.+5
 SECTION .text : CODE (2)
#ifdef TRUE_EQU_NEG_ONE
        EORS    t, t
#endif
        POP2t
        POP2n
        SUBS    t, t, n
        BEQ     EQUAL_TRUE
        EORS    t, t
        TPUSH

EQUAL_TRUE:
#ifdef TRUE_EQU_NEG_ONE
	SUBS    t, #1 // -1
#else
        ADDS    t, #1   // 1
#endif
        TPUSH
/*
	DC32	DOCOL
	DC32	SUBB
	DC32	ZEQU
	DC32	SEMIS
*/


//	< LESSTHAN:	( n1 n2 -- f ) SIGNED:
//	Leave a true flag if n1 is less than n2 otherwise leave a false flag.
//	C0 1 < . 0  -1 C0 < . 1

 SECTION .text : CONST (2)
LESSTHAN_NFA:
	DC8	0x81
	DC8	'<'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	EQUAL_NFA
LESSTHAN:
	DC32	.+5
 SECTION .text : CODE (2)
#ifdef TRUE_EQU_NEG_ONE
	EORS	t, t    // zero t
	SUBS	t, #1   // -1
#else
        MOVS    t, #1
#endif
	POP2n				// n2
	POP2w				// n1
	CMP     n, w        // n1 < n2
	BGT	LESS1

	EORS	t, t    // zero t =< n
LESS1:
	TPUSH


//	U< ULESSTHAN:	( n1 n2 -- f )	UNSIGNED:
//	Leave a true flag if n1 is less than n2 otherwise leave a false flag.
//	-1 -2 U< . 0  -1 2 U< . 0  1 2 U< . 1

 SECTION .text : CONST (2)
ULESSTHAN_NFA:
	DC8	0x82
	DC8	'U'
	DC8	'<'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LESSTHAN_NFA
ULESSTHAN:
	DC32	DOCOL,TDUP
	DC32	XORR, ZLESS
	DC32	ZBRAN
	DC32	ULES1-.	//IF

	DC32	DROP, ZLESS
	DC32	ZEQU
	DC32	BRAN
	DC32	ULES2-.

ULES1:
	DC32	SUBB,ZLESS	//ELSE

ULES2:
	DC32	SEMIS		//endif


//	> GREATERTHAN:	( n1 n2 --  f )	SIGNED:	L0
//	Leave a true flag if n1 is greater than n2 otherwise a false flag.
//	-1 1 > . 0  1 -1 > . 1

 SECTION .text : CONST (2)
GREATERTHAN_NFA:
	DC8	0x81
	DC8	'>'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ULESSTHAN_NFA
GREATERTHAN:
	DC32	DOCOL
	DC32	SWAP
	DC32	LESSTHAN
	DC32	SEMIS


//	0= ZEQU:	( n -- f )
//	Leave a true flag is the number is equal to zero, otherwise leave a
//	false flag. CHANGED Code dependent on true flag being 1 FOR -1 TRUE

 SECTION .text : CONST (2)
ZEQU_NFA:
	DC8	0x82
	DC8	'0'
	DC8	'='+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	GREATERTHAN_NFA
ZEQU:
	DC32	.+5
 SECTION .text : CODE (2)
        EORS    t, t
	POP2n
	CMP	n, #0
	BNE	ZEQU_ZERO

#ifdef TRUE_EQU_NEG_ONE
	SUBS	t, t, #1
#else
        ADDS    t, t, #1
#endif

ZEQU_ZERO:
	TPUSH


//	0< ZLESS:	( n -- f )
//	Leave a true flag if the number is less than zero (negative),
//	otherwise leave a false flag.

 SECTION .text : CONST (2)
ZLESS_NFA:
	DC8	0x82
	DC8	'0'
	DC8	'<'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ZEQU_NFA
ZLESS:
	DC32	.+5
 SECTION .text : CODE (2)
#ifdef TOSCT    // REPLACING t SO THIS IS FASTER THAN POP2n
        POP2t
        MOV     n, t
#else
	POP2n
#endif
#ifdef TRUE_EQU_NEG_ONE
        EORS    t, t
	SUBS	t, #1	// TRUE -1
#else
	MOVS	t, #1	// TRUE
#endif
	ORRS	n, n, n	// SET FLAGS
	BMI	ZLESS1	// JS	ZLESS1

	EORS	t, t	// FALSE
ZLESS1:
	TPUSH


//	BIN BIN:	( -- )
//	Set the numeric conversion base to TWO (bianry).

 SECTION .text : CONST (2)
BIN_NFA:
	DC8	0x83
	DC8	'BI'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ZLESS_NFA
BIN:
	DC32	DOCOL
        DC32    STRVA, 2, NBASE
	DC32	SEMIS


//	HEX HEX:	( -- )
//	Set the numeric conversion base to sixteen (hexadecimal).

 SECTION .text : CONST (2)
HEX_NFA:
	DC8	0x83
	DC8	'HE'
	DC8	'X'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BIN_NFA
HEX:
	DC32	DOCOL
        DC32    STRVA, 16, NBASE
	DC32	SEMIS


//	DECIMAL DECIMAL:	( -- )
//	Set the numeric conversion base to TEN (decimal).
//	DEC IS A NUMBER!

 SECTION .text : CONST (2)
DECIMAL_NFA:
	DC8	0x87
	DC8	'DECIMA'
	DC8	'L'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	HEX_NFA
DECIMAL:
	DC32	DOCOL
        DC32    STRVA, 10, NBASE
	DC32	SEMIS


//	NEGATE NEGATE: ( n1 -- n2 ) RENAMED: MINUS to NEGATE
//	Leave the two's complement of a number.

 SECTION .text : CONST (2)
NEGATE_NFA:
	DC8	0x86
	DC8	'NEGAT'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DECIMAL_NFA
NEGATE:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t			// MVN YES
	MVNS     t, t         	// 1's compliment
	ADDS     t, t, #1       // 2's compliment
	TPUSH


//	ABS ABS:	( n -- ub )
//      Leave the absolute value of n as un.

 SECTION .text : CONST (2)
ABS1_NFA:
	DC8	0x83
	DC8	'AB'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	NEGATE_NFA
ABS:
	DC32	DOCOL
	DC32	DUP
	DC32	PM
	DC32	SEMIS


//	MIN MIN:	( n1 n2 -- min )
//      Leave the smaller of two numbers.

 SECTION .text : CONST (2)
MIN_NFA:
	DC8	0x83
	DC8	'MI'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ABS1_NFA
MIN:
	DC32	DOCOL,TDUP
	DC32	GREATERTHAN
	DC32	ZBRAN	//IF
	DC32	 MIN1-.

	DC32	SWAP	//endif
MIN1:
	DC32	DROP
	DC32	SEMIS


//	MAX MAX:	( n1 n2 -- max )
//      Leave the greater of two numbers.

 SECTION .text : CONST (2)
MAX_NFA:
	DC8	0x83
	DC8	'MA'
	DC8	'X'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	MIN_NFA
MAX:
	DC32	DOCOL,TDUP
	DC32	LESSTHAN
	DC32	ZBRAN	//IF
	DC32	 MAX1-.

	DC32	SWAP	//endif
MAX1:
	DC32	DROP
	DC32	SEMIS


//	AND ANDD:	( n1 n2 -- n2 )
//	Leave the bitwise logical and of n1 and n2 as n3.

 SECTION .text : CONST (2)
ANDD_NFA:
	DC8	0x83
	DC8	'AN'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	MAX_NFA
ANDD:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	POP2n
	ANDS     t, t, n
	TPUSH


//	OR OR:	( n1 n2 -- n3 )
//	Leave the bit-wise logical or of n1 and n2 as n3.

 SECTION .text : CONST (2)
OR_NFA:
	DC8	0x82
	DC8	'O'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ANDD_NFA
OR:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	POP2n
	ORRS     t, t, n
	TPUSH


//	NOT NOT: ( nl -- n2 )
//	Leave the bitwise logical not of n1 as n2

 SECTION .text : CONST (2)
NOT_NFA:
	DC8	0x83
	DC8	'NO'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	OR_NFA
NOT:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	MVNS		t, t	// 1's compliment
	TPUSH


//	XORR XOR:	( nl n2 -- n3 )
//	Leave the bitwise logical exclusive or n1 and n2 as n3

 SECTION .text : CONST (2)
XORR_NFA:
	DC8	0x83
	DC8	'XO'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	NOT_NFA
XORR:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	POP2n
	EORS     t, t, n
	TPUSH

//	SXTH SXH:	( nl -- n3 )
//	Sign extend HALFWORD on the stack

 SECTION .text : CONST (2)
SXTH_NFA:
	DC8	0x84
	DC8	'SXT'
	DC8	'H'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	XORR_NFA
SXH:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	SXTH     t, t
	TPUSH


//	SXTB SXB:	( nl -- n3 )
//	Signe extend byte in the word on the stack

 SECTION .text : CONST (2)
SXTB_NFA:
	DC8	0x84
	DC8	'SXT'
	DC8	'B'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SXTH_NFA
SXB:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	SXTB     t, t
	TPUSH


//	REVW REVW:	( n -- n )
//	Reverse bytes in n.

 SECTION .text : CONST (2)
REVW_NFA:
	DC8	0x84
	DC8	'REV'
	DC8	'W'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SXTB_NFA
REVW:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	REV     t, t
	TPUSH


//	ASR ASR:   ( sn count -- sn' )  
//      Shift sn (sign-extended) right by count.
//	Valid count values are 0 to 31.

 SECTION .text : CONST (2)
ASR_NFA:
	DC8	0x83
        DC8     'AS'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	REVW_NFA
ASR:
        DC32    .+5
 SECTION .text : CODE (2)
        POP2w           ; shift count
        POP2t           ; original data
        ASRS    t, t, w
        TPUSH           ; shifted data


//	LSR LSR:   ( n count -- n' )
//      Logical (zero-extended) shift right by count.
//	Valid count values are 0 to 31.

 SECTION .text : CONST (2)
LSR_NFA:
	DC8	0x83
        DC8     'LS'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	ASR_NFA
LSR:
        DC32    .+5
 SECTION .text : CODE (2)
        POP2w           ; shift count
        POP2t           ; original data
        LSRS    t, t, w
        TPUSH           ; shifted data


//	LSL LSL:   ( n count -- n' )
//      Logical (zero-extended) shift left by count.
//	Valid count values are 0 to 31.

 SECTION .text : CONST (2)
LSL_NFA:
	DC8	0x83
        DC8     'LS'
	DC8	'L'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LSR_NFA
LSL:
        DC32    .+5
 SECTION .text : CODE (2)
        POP2w           ; shift count
        POP2t           ; original data
        LSLS    t, t, w
        TPUSH           ; shifted data---


//	.R DOTR:	( sn1 n2 -- ) SIGNED:
//	Print the signed number sn1 right aligned in a field whose width is n2.
//      No following blank is printed.

 SECTION .text : CONST (2)
DOTR_NFA:
	DC8	0x82
	DC8	'.'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	LSL_NFA
DOTR:
	DC32	DOCOL
	DC32	TOR
	DC32	STOD
	DC32	RFROM
	DC32	DDOTR
	DC32	SEMIS


//	.RU DOTRU:	( n1 n2 -- ) UNSIGNED:
//	Print the unsigned number n1 right aligned in a field whose width is n2.
//	No following blank is printed.

 SECTION .text : CONST (2)
DOTRU_NFA:
	DC8	0x83
	DC8	'.R'
	DC8	'U'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTR_NFA
DOTRU:
	DC32	DOCOL
	DC32	TOR
	DC32	ZERO
	DC32	BDIGS, DIGS, EDIGS
	DC32	DUP, RFROM
	DC32	SWAP, SUBB, SPACES, TYPE
#ifdef IO2TP
          DC32 NOOP
        DC32    CLRPAD  // Resets OUT
#endif
	DC32	SEMIS


//	U. UDOT:	( n -- ) USIGNED DOT

 SECTION .text : CONST (2)
UDOT_NFA:
	DC8	0x82
	DC8	'U'
	DC8	'.'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTRU_NFA
UDOT:
	DC32	DOCOL
	DC32	ZERO
	DC32	DDOT    // SPACE DONE HERE
	DC32	SEMIS


//	? QUES:	( addr -- )
//	Print the value contained at the address in free format according to
//      the current base using DOT.

 SECTION .text : CONST (2)
QUES_NFA:
	DC8	0x81
	DC8	'?'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	UDOT_NFA
QUES:
	DC32	DOCOL
	DC32	AT
	DC32	DOT
	DC32	SEMIS


//	. DOT:	( n -- ) SIGNED 2'S COMPLEMENT:
//	Print a number from a signed 32 bit two's complement value,
//      converted according to the numeric base.
//      A trailing blanks follows.

 SECTION .text : CONST (2)
DOT_NFA:
	DC8	0x81
	DC8	'.'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	QUES_NFA
DOT:
	DC32	DOCOL
	DC32	STOD
	DC32	DDOT    // SPACE DONE HERE
	DC32	SEMIS


//------------------------------ DOTBASE SECTION -------------------------------
//	.H DOTHEX:	( n -- )
//	Prints TOS in Hex using DOT, not affecting Base in the system

#ifndef SRM
//	.B DOTBIN:	( n -- )
//	Prints TOS in BINARY using DOT, not affecting Base in the system

 SECTION .text : CONST (2)
DOTBIN_NFA:
	DC8	0x82
	DC8	'.'
	DC8	'B'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOT_NFA
DOTBIN:
	DC32	DOCOL
        DC32    TWO, DOTBASE
	DC32	SEMIS
#endif  // not SRM


 SECTION .text : CONST (2)
DOTHEX_NFA:
	DC8	0x82
	DC8	'.'
	DC8	'H'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTBIN_NFA
DOTHEX:
	DC32	DOCOL
#ifdef SRM
        DC32    BASE_TO_R12, HEX, UDOT, BASE_FROM_R12
#else
        DC32    LIT, 16, DOTBASE        // this version appends BASE Suffix
#endif
	DC32	SEMIS


#ifndef SRM
//	.D DOTDEC:	( n -- )
//	Prints TOS in DECIMAL using DOT, not affecting Base in the system

 SECTION .text : CONST (2)
DOTDEC_NFA:
	DC8	0x82
	DC8	'.'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTHEX_NFA
DOTDEC:
	DC32	DOCOL
        DC32    LIT, 10, DOTBASE
	DC32	SEMIS
#endif  // not SRM


//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_NUMBERS_NFA = NUMBER Stuff: CATEGORY

 SECTION .text : CONST (2)
WC_NUMBERS_NFA:
	DC8	0x80+4+13
        DC8     0x0D, 0x0A
	DC8	'NUMBER Stuff:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    DOTDEC_NFA


//=============================== UART0_INIT =================================//

// LFA ABOVE NEEDS TO BE WC_NUMBERS_NFA
$FISH_STM32F4_UART3_INIT.s
// LFA BELOW NEEDS TO BE UART3_INIT_NFA

//=============================== UART0_INIT =================================//

//	MYBAUD MYBAUD: ( n -- ) BAUD MUST BE IN DECIMAL or EQUIVALENT!!!
//	MUST BE USED BEFORE USING UART0_INIT!!!
//	Because FISH does a reset if you invoke a HARD FAULT RESET
//	THIS MECHANISM IS ESSENTIAL TO STAYING AT USER SET BAUDRATE THRU A RESET!
//	SET NON-INIT RAM VARIABLES DBAUD TO ZERO AND UBAUD n.
//	See UART0_INIT

 SECTION .text : CONST (2)
MYBAUD_NFA:
	DC8	0x86
	DC8	'MYBAU'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	UART3_INIT_NFA
MYBAUD:
	DC32	DOCOL
        DC32    strva, 0, DBAUD
	DC32	LIT, UBAUD, STORE	// SET UBAUD TO NEW BAUD FOR UART0_INIT
	DC32	SEMIS


//	STM32F4 UART3_LSR UART3_LSR: ( -- value )

 SECTION .text : CONST (2)
UART3_LSR_NFA:
	DC8	0x89
	DC8	'UART3_LS'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	MYBAUD_NFA
UART3_LSR:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR	n, = USART3_SR
	LDRB	t, [n]
	TPUSH
 LTORG


//	STM32F4 UART3_RX UART3_RX: ( -- addr )
//	This is the serial terminal.

 SECTION .text : CONST (2)
UART3_RX_NFA:
	DC8	0x88
	DC8	'UART3_R'
	DC8	'X'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	UART3_LSR_NFA
UART0_RX:
        DC32    DOCON, USART3_DR

//	STM32F4 UART3_TX UART3_TX: ( -- addr )
//	This is the serial terminal.

 SECTION .text : CONST (2)
UART3_TX_NFA:
	DC8	0x88
	DC8	'UART3_T'
	DC8	'X'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	UART3_RX_NFA
UART3_TX:
        DC32    DOCON, USART3_DR


#ifdef XON_XOFF
//	XOFF XOFF: ( -- ) Send XOFF

 SECTION .text : CONST (2)
XOFF_NFA:
	DC8	0x84
	DC8	'XOF'
	DC8	'F'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	UART3_TX_NFA
XOFF:
	DC32	.+5
 SECTION .text : CODE (2)	// Does not incr OUT and wait for line status
#ifdef IO2TP
        NEXT
#else
        BL      TXRDY_SUBR
        BL      XOFF_SUBR
	NEXT
#endif // #else IO2TP
//#endif // XON_XOFF after XON:


//	XON XON: ( -- ) Send XON

 SECTION .text : CONST (2)
XON_NFA:
	DC8	0x83
	DC8	'XO'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	XOFF_NFA
XON:
	DC32	.+5
 SECTION .text : CODE (2)       // Does not incr OUT or wait for line status.
#ifdef IO2TP
        NEXT
#else
// Required, no 'quick send'
        BL      TXRDY_SUBR
        BL      XON_SUBR
	NEXT
#endif // #else IO2TP
#endif // XON_XOFF

//---------------- EMIT KEY ?KEY CR SECTION ------------------------------

//	SPACES SPACES:	( n -- )
//	Transmit n ascii blanks (0x20) to the output device.

 SECTION .text : CONST (2)
SPACES_NFA:
	DC8	0x86
	DC8	'SPACE'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
#ifdef XON_XOFF
        DC32    XON_NFA
#else
        DC32	UART0_TX_NFA
#endif
SPACES:
	DC32	DOCOL
	DC32	ZERO
	DC32	MAX
	DC32	ZNDUP
	DC32	ZBRAN
	DC32	 SPACES_DONE-.

	DC32	ZERO
	DC32	XDO	//DO

SPACES_DO:
	DC32	SPACE
	DC32	XLOOP	//LOOP
	DC32	 SPACES_DO-.

SPACES_DONE:
	DC32	SEMIS


//	SPACE SPACE: ( -- )
//	Emit a space character, 0x20.

 SECTION .text : CONST (2)
SPACE_NFA:
	DC8	0x85
	DC8	'SPAC'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPACES_NFA
SPACE:
	DC32	DOCOL
	DC32	BLANK
	DC32	EMIT
	DC32	SEMIS


//	ID. IDDOT:	( NFA-addr -- )	?RENAME .NFA
//	Print a definition's name from its name field address.

 SECTION .text : CONST (2)
IDDOT_NFA:
	DC8	0x83
	DC8	'ID'
	DC8	'.'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SPACE_NFA
IDDOT:
	DC32	DOCOL
//	NOW COUNT is on NFA, fig code copied it out to pad
	DC32	COUNT   // Count could be SMUDGE'd.
	DC32	LIT, MAXWORDLEN // Strip SMUDGEing from count
	DC32	ANDD            // only time 
	DC32	TYPE    // TYPE needs actual count un SMUDGE'd
	DC32	SPACE
	DC32	SEMIS


//	." DOTQ:	( -- ) IMMEDIATE
//	Type quoted string out or compile inline string to be typed at runtime
//	CHANGED COMPILE TIME ACTION TO WORK WITH 4 byte alighment REQUIREMENT.
//	Used in the form:
//		.' CCcc"
//	Compiles an in-line string CCcc (delimited by the trailing ") with
//	an execution proceedure to transmit the text to the output device.
//	If executed outside a definition, ." will immediately print the text
//	until the final ',. The maximum number of characters may be an
//	installation dependent value. See (.").

 SECTION .text : CONST (2)
DOTQ_NFA:
	DC8	0x0C2
	DC8	'.'
	DC8	'"'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	IDDOT_NFA
DOTQ:
	DC32	DOCOL
	DC32	LIT, '"'	// 0x22	ending qoute delimiter for WORD
	DC32	STATE_SV
	DC32	AT
	DC32	ZBRAN	// IF Not COMPILING
	DC32	 DOTQ1-.

	DC32	COMP    // COMPILING
	DC32	PDOTQ   // compile LIT, cnt-str-addr, count, type
	DC32	WORD   // \ cH-DELIM  --  CHCNT-STR OF TOKEN TO HERE
	DC32	HERE
	DC32	CAT
	DC32	ONEP
        DC32    ALIGNED
	DC32	ALLOT_PRIM
	DC32	BRAN	// ELSE
	DC32	 DOTQ2-.
DOTQ1:
	DC32	WORD   // \ cH-DELIM  --  CHCNT-STR OF TOKEN TO HERE
	DC32	HERE
	DC32	COUNT
	DC32	TYPE    // TYPE needs actual count un SMUDGE'd
                        // endif
#ifdef IO2TP
//DOTQ_BP1:
// DC32 NOOP
        DC32    CLRPAD  // Resets OUT
#endif
DOTQ2:
	DC32	SEMIS


//	TYPE TYPE:	( addr count -- )
// 	ADDR must be ALIGNED!
//	OUT MUST BE INITIALIZED!!
//	EMIT COUNT OF 8-bits in a string, preferably ascii characters,
//	(last nfa char (+80h)ok!) from addr thru count,

 SECTION .text : CONST (2)
TYPE_NFA:
	DC8	0x84
	DC8	'TYP'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    DOTQ_NFA
TYPE:
	DC32	DOCOL
	DC32	ZNDUP
	DC32	ZBRAN	// IF NOTHING TO TYPE
	DC32	 TYPE1-.

#ifdef XON_XOFF
        DC32    XOFF    // TEMP TEST THRE
#endif
	DC32	OVER
	DC32	PLUS
	DC32	SWAP
	DC32	XDO	// DO
TYPE2:
	DC32	I
	DC32	CATLT7F // Limit output to ASCII characters under 80h
	DC32	EMIT
tt1:
	DC32	XLOOP	// LOOP
tt2:
	DC32	 TYPE2-.

	DC32	BRAN	// ELSE
	DC32	 TYPE3-.
TYPE1:
	DC32	DROP	// endif
TYPE3:
	DC32	SEMIS


//	EMIT EMIT:	( c -- )
//	Transmit ascii character c to the selected output device. OUT is
//      incremented for each character output.

 SECTION .text : CONST (2)
EMIT_NFA:
	DC8	0x84
	DC8	'EMI'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    TYPE_NFA
EMIT:
#ifndef IO2TP
// NOT IO2TP SECTION:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t		        // GET CHAR
        BL      TXRDY_SUBR
        LDR     n, = USART3_DR // 
// BSOUT handles negative out issue
        STRB    t, [n]          // EMIT (Send) Char
        LDR     n, = OUT        // Increment Out
        LDR     t, [n]
        ADDS    t, t, #1
        STR     t, [n]
        NEXT
#else // if IO2TP Transmit char to PAD, increment OUT
// SECTION .text : CONST (2)
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t		        // Get CHAR
	LDR   	w, = OUT        // Update OUT
	LDR   	x, [w]		// LDRB would limit pad to >255
	LDR   	n, = PAD
        ADD     n, x
	STRB	t, [n]          // Send CHAR to PAD
//      +!
        ADDS    x, x, #1        // +  Increment by 1
        STR     x, [w]          // ! new OUT value to OUT
	
        NEXT
#endif	// IO2TP
 LTORG


//	KEY KEY:	(  -- ch )

 SECTION .text : CONST (2)
KEY_NFA:
	DC8	0x83
	DC8	'KE'
	DC8	'Y'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	EMIT_NFA
KEY_INTERPRETED_ENTRY:
#ifndef IO2TP
	DC32	DOCOL
        DC32    XON     // EXEC SETS XOFF SO UNDO IT
        DC32    KEY     // TE GET KEY!
        DC32    SEMIS


//:NONAME KEY KEY:      ( -- CH ) 0-7Fh
KEY:
	DC32	.+5
 SECTION .text : CODE (2)
        LDR     w, = USART3_DR // 
        LDR     x, = USART3_SR // 
rxRDY?:
        LDR     n, [x]          // Get Line Status
//      LSRS    n, n, #5        // Bit 5 RXNE: Read data register not empty
// THIS IS ___ AND FAILS TEXT DOWNLOAD
        LSRS    n, n, #6        // Bit 6 ORIG - REQ'D FOR TEXT FILE DOWNLOAD
        BCC     rxRDY?          // sets carry flag

        LDR     t, [w]
        TPUSH
#else
	DC32	DOCOL, LIT, 0X0D, SEMIS		// cr executes NULL
#endif	// IO2TP
 LTORG


//	?KEY QKEY: ( -- f )
//	Return zero unless key in fifo - key is not consumed.
//	: TB BEGIN ?KEY UNTIL ; Will execute until any key is entered.

 SECTION .text : CONST (2)
QKEY_NFA:
	DC8	0x84
	DC8	'?KE'
	DC8	'Y'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	KEY_NFA // -1
QKEY:
	DC32	.+5
 SECTION .text : CODE (2)
#ifdef IO2TP
	EORS	t, t    // zero t
#else
#ifdef XON_XOFF // XON IN QKEY
        BL      TXRDY_SUBR
        BL      XON_SUBR
#endif
	EORS	t, t    // zero t
        LDR     x, = USART3_SR
        LDR     n, [x]          // Get Line Status
        LSRS    n, n, #6        // Char available
        BCC     NO_KEY          // No char, not Ready

// HAVE A KEY - DON'T CONSUME IT
#ifdef TRUE_EQU_NEG_ONE
	SUBS	t, #1   // -1
#else
        ADDS    t, #1
#endif
#endif  // DEFAULT TO NO KEY IF IO2TP
NO_KEY:
        TPUSH
 LTORG


//	CRS CRS:	( n -- )
//      Emit n cr'S (0x0d) and lf (0x0A)

 SECTION .text : CONST (2)
CRS_NFA:
        DC8	0x83
        DC8	'CR'
        DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32	QKEY_NFA
CRS:
        DC32	DOCOL
        DC32    ZERO
        DC32	XDO
CRS_BEGIN:
	DC32	CR
	DC32	XLOOP
        DC32     CRS_BEGIN-.
        DC32    SEMIS


//	CR CR:	( -- )
//      Emit cr (0x0d) and lf (0x0A)

 SECTION .text : CONST (2)
CR_NFA:
        DC8	0x82
        DC8	'C'
        DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32	CRS_NFA
CR:
#ifdef IO2TP
        DC32    DOCOL, zero_OUT
        DC32	LIT, msg_cr, NULLSTRLEN, TYPE, SEMIS
#else
        DC32	DOCOL
        DC32    zero_OUT
        DC32	PDOTQ
	DC8	2
	DC8	0x0D, 0x0A
 ALIGNROM 2,0xFFFFFFFF
        DC32    SEMIS
#endif


//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_UART0_NFA = FISH IO: CATEGORY

 SECTION .text : CONST (2)
WC_UARTx_NFA:
	DC8	0x80+4+8
        DC8     0x0D, 0x0A
	DC8	'FISH IO:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    CR_NFA

//      CMSIS_ENABLE_IRQS IRQS_RESUME:  ( -- )
 SECTION .text : CONST (2)
IRQS_RESUME_NFA:
	DC8	0x8B
	DC8	'IRQS_RESUM'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_UARTx_NFA
CMSIS_ENABLE_IRQS:
	DC32 .+5
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
 IMPORT C_CMSIS_ENABLE_IRQS
        BL      C_CMSIS_ENABLE_IRQS
        NEXT

//      CMSIS_DISABLE_IRQS IRQS_SUSPEND:        ( -- )
 SECTION .text : CONST (2)
IRQS_SUSPEND_NFA:
	DC8	0x8C
	DC8	'IRQS_SUSPEN'
	DC8	'D'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	IRQS_RESUME_NFA
CMSIS_DISABLE_IRQS:
	DC32 .+5
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
 IMPORT C_CMSIS_DISABLE_IRQS
        BL      C_CMSIS_DISABLE_IRQS
        NEXT


//	SYSTICK_IRQ_OFF SYSTICK_IRQ_OFF:       	( -- )
//      Turn SYSTICK interrupt off.
//      STCTR only incremented when SYSTICK interrupt is on.
//      STI_ON: 7 E000E010h !  STI_OFF: 5 E000E010h ! E000E010h @ .H
 SECTION .text : CONST (2)
SYSTICK_IRQ_OFF_NFA:
	DC8	0x8F
	DC8	'SYSTICK_IRQ_OF'
	DC8	'F'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	IRQS_SUSPEND_NFA
SYSTICK_IRQ_OFF:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR	n, = SYST_CSR	// SYSTICK Control and Status Register
	MOVS	t, #5
        STR     t, [n]
	NEXT
// LTORG	 //Always outside of code, else data in words


//	SYSTICK_IRQ_ON SYSTICK_IRQ_ON:  ( -- )
//      Turn SYSTICK interrupt on.
//      STCTR only incremented when SYSTICK interrupt is on.
//      STI_ON: 7 E000E010h !  STI_OFF: 5 E000E010h ! E000E010h @ .H
 SECTION .text : CONST (2)
SYSTICK_IRQ_ON_NFA:
	DC8	0x8E
	DC8	'SYSTICK_IRQ_O'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	SYSTICK_IRQ_OFF_NFA
SYSTICK_IRQ_ON:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR	n, = SYST_CSR	// SYSTICK Control and Status Register
	MOVS	t, #7
        STR     t, [n]
	NEXT
 LTORG	 //Always outside of code, else data in words

//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_IRQ_NFA = INTERRUPTS: CATEGORY

 SECTION .text : CONST (2)
WC_IRQ_NFA:
	DC8	0x80+4+11
        DC8     0x0D, 0x0A
	DC8	'INTERRUPTS:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    SYSTICK_IRQ_ON_NFA


//------------------------------------------------------------------------------
$FISH_STM32F4_FLASH.s       // FLASH_SAVE and FLASH_FORGET Words
// LFA BELOW NEEDS TO BE FLASH_FORGET_NFA
//------------------------------------------------------------------------------

//-------------------------- PROMPT VECTOR SECTION -----------------------------

//      POFF ( -- )   Set NULL$ address in PROMPT to zero so quit does CR only.

 SECTION .text : CONST (2)
POFF_NFA:
	DC8	0x84
	DC8	'POF'
	DC8	'F'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FLASH_FORGET_NFA
POFF:
        DC32    DOCOL
        DC32    STRVA, 0, PROMPT
        DC32    SEMIS

//      PON ( -- )   Set NULL$ address in PROMPT to msg_MY_OK

 SECTION .text : CONST (2)
PON_NFA:
	DC8	0x83
	DC8	'PO'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	POFF_NFA
PON:
        DC32    DOCOL
        DC32    STRVA, msg_MY_OK, PROMPT
        DC32    SEMIS

//      P ( -- addr )   Address of PROMPT, contains 0 or NULL$ address.

 SECTION .text : CONST (2)
P_NFA:
	DC8	0x81
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	PON_NFA
P:
        DC32    DOCON, PROMPT

//-------------------------- ERROR_HALT SECTION -----------------------------

//      EHOFF ( -- )   Set ERROR_HALT to 0

 SECTION .text : CONST (2)
EHOFF_NFA:
	DC8	0x85
        DC8     'EHOF'
	DC8	'F'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	P_NFA
EHOFF:
        DC32    DOCOL, ZERO, LIT, ERROR_HALT, STORE, SEMIS

//      EHON ( -- )   Set ERROR_HALT to 1

 SECTION .text : CONST (2)
EHON_NFA:
	DC8	0x84
        DC8     'EHO'
	DC8	'N'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	EHOFF_NFA
EHON:
        DC32    DOCOL, ONE, LIT, ERROR_HALT, STORE, SEMIS

//-------------------------- DOTDICTSPACE SECTION ------------------------------

//	.DS DOTDICTSPACE:	( n -- )
//	Prints number of bytes availble in dictionary in DECIMAL using DOT,
//	not affecting Base in the system

 SECTION .text : CONST (2)
DOTDICTSPACE_NFA:
	DC8	0x83
	DC8	'.D'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	FLASH_FORGET_NFA
//      DC32    P_NFA
        DC32    EHON_NFA
DOTDICTSPACE:
	DC32	DOCOL
	DC32	DICTSPACE
	DC32	BASE_TO_R12     // Save current BASE
	DC32	DOTDEC
	DC32	BASE_FROM_R12   // Restore BASE
	DC32	LIT, msg_dictspace
	DC32	NULLSTRLEN, TYPE
#ifdef IO2TP
//DOTDICTSPACE_BP1:
// DC32 NOOP
        DC32    CLRPAD  // Resets OUT
#endif
	DC32	SEMIS


//-------------------------- DOTVARSPACE SECTION -------------------------------

//	.VS DOTVARSPACE:	( n -- )
//	Prints number of bytes availble in RAM VAR SPACE in DECIMAL using DOT,
//	not affecting Base in the system.

 SECTION .text : CONST (2)
DOTVARSPACE_NFA:
	DC8	0x83
	DC8	'.V'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTDICTSPACE_NFA
DOTVARSPACE:
	DC32	DOCOL
	DC32	VARSPACE
	DC32	BASE_TO_R12     // Save current BASE
	DC32	FOUR, SLASH
	DC32	DOTDEC
	DC32	BASE_FROM_R12   // Restore BASE
	DC32	LIT, msg_uvspace
	DC32	NULLSTRLEN, TYPE
#ifdef IO2TP
//DOTVARSPACE_BP1:
// DC32 NOOP
        DC32    CLRPAD  // Resets OUT
#endif
	DC32	SEMIS


//	DUMP DUMP:	( addr n -- )
//	Print adrr and n lines of 4 columns of memory values in hexadecimal.
//	Address must be even and a multiple of 4 else error message issued.
//      Any key presssed will stop DUMP.

 SECTION .text : CONST (2)
DUMP_NFA:
	DC8	0x84
	DC8	'DUM'
	DC8	'P'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DOTVARSPACE_NFA
DUMP:
	DC32	DOCOL

	DC32	OVER, QALIGNED  // \ -- addr n
// DON'T SAVE BASE UNTIL AFTER ALIGNED TEST
        DC32    BASE_TO_R12     // Save current BASE
        DC32    HEX
	DC32	ZERO, XDO
DUMP_ADDR_LINE:
	DC32	CR
	DC32	DUP, LIT, 10, DOTRU     // Print addr in Field
	DC32	FOUR, ZERO, XDO         // Four colum loop
DUMP_EACH_LOC:
	DC32	DUP, AT
// Little Endian means addresses will print correctly
// but to see character strings REVW is needed
// but the addresses are scrambled!
//        DC32    REVW                    // Reverse bytes in word
        DC32    LIT, 14, DOTRU          // Diplay in Field
	DC32	FOURP
	DC32	XLOOP
	DC32	 DUMP_EACH_LOC-.

#ifdef IO2TP
DUMP_BP1:
 DC32 NOOP
        DC32    CLRPAD  // Resets OUT
#endif
        DC32    QKEY
        DC32    ZBRAN
        DC32     DUMP_CONT-.
        DC32    LEAVE
DUMP_CONT:
        DC32    XLOOP
	DC32	DUMP_ADDR_LINE-.

        DC32    DROP, CR
        DC32    BASE_FROM_R12   // Restore BASE
	DC32	SEMIS


//      VBASE VBASE:	( -- addr )
//      Return base addr of VAR's .

 SECTION .text : CONST (2)
VBASE_NFA:
	DC8	0x85
	DC8	'VBAS'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DUMP_NFA
VBASE:
        DC32    DOCON,  RAMVARSPACE_START


//      DBASE DBASE:	( -- addr )
//      Return base addr of the dictionary.

 SECTION .text : CONST (2)
DBASE_NFA:
	DC8	0x85
	DC8	'DBAS'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	VBASE_NFA
DBASE:
        DC32    DOCON,  ORIG


//      RBASE RBASE:	( -- addr )
//      Return base addr of RAM THE DICTIONARY IS IN!.

 SECTION .text : CONST (2)
RBASE_NFA:
	DC8	0x85
	DC8	'RBAS'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DBASE_NFA
RBASE:
        DC32    DOCON,  RAM_START    // RAM WHERE DICT ALLOACTED IN MEMMAP


//	CLS CLS:	( -- )
//	Clear serial terminal screen using a formfeed character

 SECTION .text : CONST (2)
CLS_NFA:
	DC8	0x83
	DC8	'CL'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	RBASE_NFA
CLS:
        DC32	DOCOL
        DC32    PDOTQ
        DC8     4
        DC8     0x1B, '['       // ESC[ = ANSI VT100 ESC SETUP
        DC8     '2J'            // Clearscreen
 ALIGNROM 2,0xFFFFFFFF
        DC32    PDOTQ
        DC8     4
        DC8     0x1B, '['       // ESC[ = ANSI VT00 ESC SETUP
        DC8     ';H'            // CURSORHOME \ upper left corner
 ALIGNROM 2,0xFFFFFFFF
        DC32    CR              // Reset OUT
	DC32	SEMIS


//	( PAREN:	( -- ) IMMEDIATE
//	Used in the form:
//		( CCcc )
//	Ignore comments within parenthesis. A right parenthesis ends comment.
//      The comments may be multi-line but a space or tab is required
//      for any line that is otherwise empty for formatting purposes,
//      else a comment error will be printed. May occur during execution or in a
//	colon-definition. A blank after the leading parenthesis is required.

 SECTION .text : CONST (2)
PAREN_NFA:
	DC8	0x0C1
	DC8	'('+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	CLS_NFA
PAREN:
// Slow Single line comment
//      DC32    DOCOL, LIT, ')', WORD, SEMIS
	DC32	DOCOL
PML_LOOP:
	DC32    LIT, ')'
        DC32    TIB_CHAR_SCAN   // ( c -- f )
//
        DC32    ZEQU            // If null
        DC32    ZBRAN           // found fall thru
        DC32      PE_DONE-.     // else were done

// GET ANOTHER LINE
        DC32    CR
        DC32    QUERY
//  AND IF ONLY CR (null) ERR
//        DC32    TIB_SV, CAT
        DC32    LIT, TIB+1, CAT // PAST COUNT BYTE
        DC32    ZBRAN
        DC32      PAREN_ERR-.

//  If closing paren not in this line LOOP
        DC32    BRAN
        DC32      PML_LOOP-.

PE_DONE:
        DC32    SEMIS

PAREN_ERR:
        DC32    LIT, msg_paren_err
	DC32	NULLSTRLEN, TYPE        // Passed in null string
        DC32    SEMIS


//	BACKSLASH BACKSLASH:	( --  )
//      After a trailing space treat the rest of the line as a comment.
//      By setting a null terminator in TIB

 SECTION .text : CONST (2)
BACKSLASH_NFA:
	DC8	0x0C1
	DC8	'\\'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	PAREN_NFA
BACKSLASH:
        DC32    DOCOL
        DC32    ZERO, TIB_SV, IN_SV, AT, PLUS, CSTORE
        DC32    SEMIS


//	SYSCLK SYSCLK:	( -- value )
//      Returns system clock value in Hz, i.e. 48000000 = 48Mhz.

 SECTION .text : CONST (2)
SYSCLK_NFA:
	DC8	0x86
	DC8	'SYSCL'
	DC8	'K'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	BACKSLASH_NFA
SYSCLK:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR	n, = SYSCLOCK	// SystemCoreClock
	LDR	t, [n]
	TPUSH	// Push -- t
 LTORG	 //Always outside of code, else data in words


//      STCTR   ( -- addr ) SYSTICK Interrupt increments this with 32but wrap.
//      STI_ON: 7 E000E010h !  STI_OFF: 5 E000E010h ! E000E010h @ .H
 SECTION .text : CONST (2)
STCTR_NFA:
	DC8	0x85
	DC8	'STCT'
	DC8	'R'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	DC32	STI_ON_NFA
        DC32    SYSCLK_NFA
STCTR:
        DC32    DOCON, STICKER


//      DELAY ( n value -- ) 7 E000E010h !   5 E000E010h !
 SECTION .text : CONST (2)
DELAY_NFA:
	DC8	0x85
	DC8	'DELA'
	DC8	'Y'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	STCTR_NFA
DELAY:
	DC32	.+5
 SECTION .text : CODE (2)
        POP2n   // Reload value in n
        POP2t   // loop count in t
// SET STCTR TO NEGATIVE LOOP COUNT TO END AT ZERO
        LDR     y, = STICKER
	MVNS    t, t            // 1's compliment
	ADDS    t, t, #1        // 2's compliment
        STR     t, [y]
// Load SYST_RVR with countdown value
        LDR     w, = SYST_RVR
        STR     n, [w]
// and reset SYST_CVR to start countdown.
       LDR     w, = SYST_CVR
// Writing it clears the System Tick counter and the COUNTFLAG bit in STCTRL.
        STR     n, [w]
// If n=0 in t user is just setting reload value
        CMP     t, #0           // LOOP OF ZERO
        BEQ     DELAY_DONE
// INTERRUPT VERSION: negate n to STCTR and leave when STCTR = 0
// STI_ON: 7 E000E010h !  STI_OFF: 5 E000E010h ! E000E010h @ .H
// Save and restore user interrupt setting
// y = STICKER
        LDR     w, = SYST_CSR
        LDR     t, [w]          // Save user SYSTICK interrupt setting
        MOVS    n, #7
        STR     n, [w]          // Turn SYSTICK interrupt on in case it's off
DELAY_LOOP:
        LDR     n, [y]
        CMP     n, #0
        BNE     DELAY_LOOP
DELAY_DONE:
        STR     t, [w]  // Restore user SYSTICK interrupt setting
	NEXT
 LTORG


//	MS MS: ( n -- ) n * 1 millisecond execution time
//      SYSTICK_IRQ_ON/OFF  STI_ON: 7 E000E010h !  STI_OFF: 5 E000E010h ! 

 SECTION .text : CONST (2)
MS_NFA:
	DC8	0x82
	DC8	'M'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	DELAY_NFA
MS:
        DC32    DOCOL
#ifdef STM32F4_IRC16_48MHZ
        DC32    LIT, 47999d     // BB7F 1ms @ 48mhz RELOAD COUNTER VALUE
#endif
#ifdef STM32F205RC_XRC10_118MHZ
        DC32    LIT, 117999d    // 1CCEFh 1ms @ 168mhz RELOAD COUNTER VALUE
#endif
#ifdef STM32F4_XRC08_168MHZ
        DC32    LIT, 167999d    // 2903Fh 1ms @ 168mhz RELOAD COUNTER VALUE
#endif
        DC32    DELAY
        DC32    SEMIS


//      WORDCAT WORDCAT: ( -- )
//      Creates a Word Category NFA and LFA that cannot be searched for,
//      but is displayed by WORDS and MYWORDS to label a group of Words.
//      Define a group of Words and then add a category name with WORDCAT.
//      EX: WORDCAT MY APP WORDS:
//      It must be on a line of it's own.
//      The colon at the end is FISH convention.
 SECTION .text : CONST (2)
WORDCAT_NFA:
	DC8	0x87
	DC8	'WORDCA'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    MS_NFA
WORDCAT:
	DC32	DOCOL
        DC32    HERE, TOR
        DC32    ZERO, CCOMMA    // Count byte place holder
        DC32    LIT, 0Dh, CCOMMA
        DC32    HERE
        DC32    LIT, 0Dh, WORD
        DC32    DUP, CAT
        DC32    LIT, 0Ah, ROT, CSTORE
        DC32    DUP, ONEP, ALLOT
        DC32    LIT, 084h        // Count before system text
        DC32    PLUS, R, CSTORE
        DC32    LIT, 0Dh, CCOMMA
        DC32    LIT, 08Ah, CCOMMA
        DC32    ALIGN32_DP_FF_PAD
        DC32    LATEST, COMMA
        DC32    RFROM
        DC32    LIT, CURRENT
        DC32    STORE
	DC32	SEMIS


//	WORDS WORDS:	( -- ) RENAMED: VLIST to WORDS
//      Output list of Words in the dictionary in search order.
//      Uses WC_ nfa's to format Word Categories.
//      Uses FENCE for where to stop.
//      See MYWORDS.

 SECTION .text : CONST (2)
WORDS_NFA:
	DC8	0x85
	DC8	'WORD'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WORDCAT_NFA
WORDS:
	DC32	DOCOL
        DC32    THREE, SPACES
	DC32	LATEST
#ifdef XON_XOFF
        DC32    XOFF    // TEMP TEST THRE
#endif
WORDS1:  // ADD nfa length to current out_uv & verify it doesn't violate csll.

        DC32    ZERO, OVER      // -- nfa zero nfa
        DC32    ONEP, CAT       // If wc_ header skip
        DC32    LIT, 0x0D       // -- nfa zero (c@) 0x0D
        DC32    EQUAL, ZEQU     // -- nfa zerro flag
        DC32    ZBRAN           // -- nfa zero
        DC32     WORDS2-.       // wc_ goto
        
        DC32    DROP            // -- nfa
        DC32    DUP, PFA, LFA   // -- nfa lfa
        DC32    OVER, SUBB      // -- nfa (lfa - nfa)

WORDS2: // -- nfa n

	DC32	OUT_SV, AT
        DC32    PLUS
	DC32	LIT, 74         // was :NONAME CSLL - WORDS line length constant.
	DC32	GREATERTHAN
	DC32	ZBRAN	        // If not at end of line
	DC32	 WORD21-.        // skip cr and out reset

	DC32	CR              // Start another line
        DC32    THREE, SPACES

WORD21:
#ifdef  IO2TP
WORDS_BP1:
 DC32 NOOP
#endif
// For MYWORDS test FENCE and stop if less
        DC32    DUP             // nfa
        DC32    FENCE_SV, AT
        DC32    LESSTHAN
        DC32    ZBRAN
        DC32     WORDSCONT-.

        DC32    BRAN
        DC32     WORDSDONE-.

WORDSCONT:
	DC32	DUP	        // nfa
	DC32	IDDOT
	DC32	TWO, SPACES

        DC32    DUP, ONEP, CAT  // Take nfa and look for WORDCAT signature
        DC32    LIT, 0x0D       // which is cr
        DC32    EQUAL
        DC32    ZBRAN           // If not wordcat
        DC32     NOT_WC-.       // skip

        DC32    zero_OUT

NOT_WC:
	DC32	PFA		// \ nfa -- pfa
	DC32	LFA		// \ pfa -- lfa
	DC32	AT              // Is next lfa
	DC32	DUP
	DC32	ZEQU            // Zero = end of dictionary

// REMOVED SO WORDS AND MYWORDS CAN BE USED IN DOWNLOAD FILES
//	DC32	QKEY           // Zero or break key \ ^C = 0x03
//	DC32	OR

	DC32	ZBRAN	        // Until break key or end of dictionary
	DC32	 WORDS1-.

#ifdef  IO2TP
WORDS_BP2:
 DC32 NOOP
#endif
WORDSDONE:
	DC32	DROP, CR
	DC32	SEMIS


//	MYWORDS MYWORDS: ( -- )
//      Print only system defined words by manipulating FENCE.

 SECTION .text : CONST (2)
MYWORDS_NFA:
	DC8	0x87
	DC8	'MYWORD'
	DC8	'S'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WORDS_NFA
MYWORDS:
	DC32	DOCOL
        DC32    strva , FLASH_SPAGE, FENCE
        DC32    WORDS                   // now print words in ram
        DC32    strva, 0 , FENCE
        DC32    SEMIS


//	FISH_ONLY FISH_ONLY	( -- ) MODIFIED:
//      RESET DP, UP AND CURRENT TO ORIGINAL FLASH IMAGE

 SECTION .text : CONST (2)
FISH_ONLY_NFA:
	DC8	0x80+9
	DC8	'FISH_ONL'
	DC8	'Y'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	MYWORDS_NFA
FISH_ONLY:
	DC32	.+5
 SECTION .text : CODE (2)
//	LDR	n, = TASK_NFA           // preserve TOS
#ifdef FISH_PubRel_WORDSET
        LDR     n, = WC_FISH_PubRel_NFA
#endif
#ifdef FISH_STM32M407vg_PRO_WORDCAT
        LDR     n, = WC_FISH_PRO_NFA
#endif
	LDR	y, = CURRENT 	        // CURRENT SETTING
	STR	n, [y]
        LDR     y, = FPC                // FLASH CURRENT
        STR     n, [y]
	LDR	n, = RAMVARSPACE_START
	LDR	y, = UP			// UP SETTING
	STR	n, [y]
        LDR     y, = FPSV                // FLASH USER VARS
        STR     n, [y]
	LDR	n, = ORIG
	LDR	y, = DP			// DP SETTING
	STR	n, [y]
	NEXT
 LTORG


//	FISH FISH:	( -- )
//	Print Flash Status and FISH Signon Message.


 SECTION .text : CONST (2)
FISH_NFA:
	DC8	0x84
	DC8	'FIS'
	DC8	'H'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FISH_ONLY_NFA
FISH:
	DC32	DOCOL
        DC32    FLASH_SCAN
	DC32	SIGNON
	DC32	SEMIS

//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_PubRel: = FISH Reference Model: CATEGORY
 SECTION .text : CONST (2)
WC_FISH_PubRel_NFA:
	DC8	0x80+4+21
        DC8     0x0D, 0x0A
	DC8	'FISH Reference Model:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    FISH_NFA

;**** FIRST WORD LISTED****

//=============================== WORDCAT ====================================//

#ifdef FISH_STM_M3_PRO_WORDCAT
$FISH_STM_M3_PRO_WORDSET.s
#endif
// FIRST WORDCAT

//------------------------------------------------------------------------------
// FOR MULTI TASKING MUST BE PLACED IN RAM AND OFFSET USED IN SYSTEM VARS!!!!!!
//            T A S K 
//
;.data		// Place TASK at beginning of RAM, or not.
/*
 SECTION .text : CONST (2)
TASK_NFA:
	DC8	0x84
	DC8	'TAS'
	DC8	'K'+0x80
 ALIGNROM 2,0xFFFFFFFF
//	TASK Is top of dictionary until new definitionsa added
//	PATCH CONTEXT or LATEST else LFA in TASK FROM SYM TABLE
//	To debug dictionary searches - see - PFIND & DFIND
//	DC32	ZEQU_NFA	// 4 (no match) bombing at +
//	DC32	NOOP_NFA	// Patch here to shorten test of PFIND
//	DC32	CAT_NFA		// C@ before @
//	DC32	LESS_NFA	// < before =
//       DC32	SoCinit_NFA	//XON_NFA		// FULL DICT SEARCH
        DC32    MS_NFA
TASK:
	DC32	DOCOL
	DC32	SEMIS
*/

//------------------------------------------------------------------------------
//:NONAME SECTION:
// THESE STAY HERE (INSTEAD OF SLLIB) BECAUSE THEY ARE CONDITIONALLY INCLUDED
// HI-LEVEL THEM ASM SECTIONS
#ifdef IO2TP
//:NONAME CLRPAD:     ( -- ) FILL PAD with BLANK'S.
 SECTION .text : CONST (2)
// ALIGNROM 2,0xFFFFFFFF
CLRPAD:
        DC32    DOCOL, PAD_SV, LIT, IOBUFSIZE
        DC32    BLANK,  FILL
        DC32    zero_OUT
        DC32    SEMIS
#endif

#ifdef IO2TP
//:NONAME CLRTIB:     ( -- ) FILL TIB with NULLS to terminate input
 SECTION .text : CONST (2)
// ALIGNROM 2,0xFFFFFFFF
CLRTIB:
	DC32    DOCOL, TIB_SV, LIT, IOBUFSIZE
        DC32    ERASE
        DC32    STRVA, 1, IN
        DC32    SEMIS
#endif

#ifdef USE_CMAIN
//:NONAME RET2c:      ( -- ) POP main.c saved lr saved in FM0_COLD
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
RET2c:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR	t, [sp]
	ADD	sp, sp, #4
	MOV	pc, t
#endif

#ifdef TESTRAM
//:NONAME flogRAM:    ( -- ) Non destructively test RAM
// RAM_START and RAM_END defined in FISH_M0_EQUATES.H
 SECTION .text : CONST (2)
flogRAM:
	DC32	.+5
 SECTION .text : CODE (2)
	mov	r8, t
	mov	r9, n
	mov	r10, i		//  SAVE IP !!!
	ldr	t, = RAM_START	// RAM_START
	ldr	y, = RAM_END	// (RAM_END +1)		//  limit
_flogRAM:
	ldr	x, [t]	//  save original contents

	movs	n, #0	//  all zeros
	str	n, [t]
	ldr	w, [t]
	eors	w, w, n
	bne	_trap

	ldr	n, =-1	//  all ones
	str	n, [t]
	ldr	w, [t]
	eors	w, w, n
	bne	_trap

	movs	n, t	//  it's own address
	str	n, [t]
	ldr	w, [t]
	eors	w, w, n
	bne	_trap

	mvns	n, t	//  complement of it's own address
	str	n, [t]
	ldr	w, [t]
	eors	w, w, n
	bne	_trap


	ldr	n, = 0xFFFFFFFE
	movs	i, #32
_walkzeros:
	str	n, [t]
	ldr	w, [t]
	eors	w, w, n
	bne	_trap

	lsls	n, n, #1
	adds	n, n, #1
	subs	i, i, #1
	bne	_walkzeros


	movs	n, #1
	movs	i, #32
_walkones:
	str	n, [t]
	ldr	w, [t]
	eors	w, w, n
	bne	_trap

	lsls	n, n, #1
	subs	i, i, #1
	bne	_walkones


	ldr	n, = 0xFFFFFFFE
	movs	i, #32
_slidezeros:
	str	n, [t]
	ldr	w, [t]
	eors	w, n
	bne	_trap

	lsls	n, n, #1
	subs	i, i, #1
	bne	_slidezeros


	movs	n, #1
	movs	i, #32
_slideones:
	str	n, [t]
	ldr	w, [t]
	eors	w, n
	bne	_trap

	lsls	n, n, #1
	adds	n, n, #1
	subs	i, i, #1
	bne	_slideones


	str	x, [t]	//  restore original contents
	adds	t, t, #4
	cmp	t, y
	blo	_flogRAM

_notrap:
	mov	t, r8
	mov	n, r9
	mov	i, r10	//  RESTORE IP !!!
	NEXT
_trap:	b	.
 LTORG
#endif  // TESTRAM
//------------------------------------------------------------------------------
// Placing blocks that can change at end of link map
// Place :NONAME for static Library first in FISH_STM_M3_SLIB.s

// $PROJ_DIR$\..\FISH_RM_COMMON
// $PROJ_DIR$\..\FISH_RM_CORTEX_M_COMMON_CODE
// In Assembler preprocessor set additional include directories 
$FISH_RM_MSGS.h
// equals below
//#include ".\..\FISH_COMMON_CODE\FISH_RM_MSGS.h"

// SYSTEMCLOCK #define set in FISH_M0_EQUATES.s for SYSCLOCK value.
// THIS IS A :NONAME WORDSET

$FISH_STM32F4_SOC_INIT.s

 END

