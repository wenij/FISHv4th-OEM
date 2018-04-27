// Created FISH_STM32F4_SLIB.s
// - To place :NONAME's before dynamic content to make their CFA addresses static.

//------------------------------------------------------------------------------
// SECTION HEADER TO ALIGN ALL HI LEVEL CODE - ASM SECTION AT END DIFFERS!
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
//------------------------------------------------------------------------------
//:NONAME BELL:       ( -- ) Emit BELL char.
BELL:
	DC32	DOCOL, LIT, 0x07, EMIT, SEMIS

//:NONAME EIGHT:      ( -- n )
//:NONAME BACKSPACE_CHAR:  ( -- n ) 
EIGHT:
BACKSPACE_CHAR:
        DC32    DOCON, 8

//:NONAME BSOUT:      ( -- ) Emit BACKSPACE char erasing it and maintain OUT.
BSOUT:
	DC32	DOCOL
        DC32    BACKSPACE_CHAR, EMIT
        DC32    SPACE
        DC32    BACKSPACE_CHAR, EMIT
        DC32    LIT, -3, OUT_SV, PSTORE // Keep OUT in synch
        DC32    SEMIS

//:NONAME zero_OUT:       ( -- ) 
zero_OUT:
        DC32	DOCOL, strva, 0, OUT, SEMIS

//:NONAME DOTSBASE:   ( n -- ) Pass n as base to .S
DOTSBASE:
	DC32	DOCOL
	DC32	BASE_TO_R12     // Save current BASE
	DC32	BASE_SV, STORE, DOTS
	DC32	BASE_FROM_R12   // Restore BASE
	DC32	SEMIS

//:NONAME PRINT_SUFFIX ( -- ) Print char representing BASE ( b d or h )
PRINT_SUFFIX:
        DC32    DOCOL
        DC32    BASE_SV, AT, DUP
        DC32    TWO, EQUAL
        DC32    ZBRAN
        DC32     DEC_OR_HEX-.

IS_BIN:        
        DC32    LIT, 'b', EMIT, DROP
        DC32    SEMIS
        
DEC_OR_HEX:
        DC32    LIT, 10, EQUAL
        DC32    ZBRAN
        DC32     IS_HEX-.

IS_DECIMAL:
        DC32    LIT, 'd', EMIT
        DC32    SEMIS

IS_HEX:
        DC32    LIT, 'h', EMIT
        DC32    SEMIS

//:NONAME DOT_BASE_SUFFIX: ( n -- ) DOT with BASE suffix appended.
// USED IN . .D and .SD
DOT_BASE_SUFFIX:
	DC32    DOCOL
	DC32	STOD            // Extend sign
	DC32	ZERO            // and no padding
	DC32	DDOTR           // in call to D.R
        DC32    PRINT_SUFFIX    // Read BASE and print suffix
        DC32    SEMIS

//:NONAME UDOT_BASE_SUFFIX: ( n -- ) UDOT with BASE suffix appended.
// USED IN .B .H .SB AND .SH
UDOT_BASE_SUFFIX:
	DC32    DOCOL
        DC32	ZERO, ZERO      // No sign and no padding
	DC32	DDOTR           // in call to D.R
        DC32    PRINT_SUFFIX    // Read BASE and print suffix
        DC32    SEMIS

//:NONAME DOTBASE:   ( n -- ) Use n as temporary BASE to output with.
//       Save current BASE and use n as BASE for this DOT or U.
//       Used by .B .D and .H
DOTBASE:
	DC32	DOCOL
        DC32    BASE_TO_R12     // Save current BASE
	DC32	DUP, BASE_SV, STORE     // Save and set BASE to use
// IF THIS IS ELIMINATED AND UDOT ONLY USED -1 .D IS 42432762367
        DC32    LIT, 10, EQUAL  // Use saved BASE for DECIMAL case
        DC32    ZBRAN           // Fall thru for DECIMAL Case
        DC32    DBUSEDUDOT-.    // Else handle BIN and HEX as unsigned.

        DC32    DOT_BASE_SUFFIX

        DC32    BRAN
        DC32    DBASEDONE-.     // Finish with space and reestoration of BASE.

DBUSEDUDOT:
        DC32    UDOT_BASE_SUFFIX

DBASEDONE:
        DC32    SPACE           // Provide normal . formatting
	DC32	BASE_FROM_R12   // Restore BASE
	DC32	SEMIS

//:NONAME INITSO_SV:	( -- addr of INITSO )
//	A system variable that contains the initial value for the stack pointer.
//	Pronounced S-zero. See SP!
INITSO_SV:
        DC32    DOCON, INITSO

//:NONAME INITRO_SV:	( -- addr of INITRO )
//	A system variable containing the initial location of the return stack.
//	Pronounced R-zero. See RP!
INITRO_SV:
        DC32    DOCON, INITRO


//:NONAME CSP_SV:	( -- addr of CSP ) Stack address save for compile error checking
//       A system variable temporarily storing the stack pointer position, for
//       compilation error checking.
CSP_SV:
        DC32    DOCON, CSP

//:NONAME HLD_SV:	( -- addr of NHLD ) Holds address of numeric output conversion
//	A system variable that holds the address of the latest character of
//	text during numeric output conversion. Usually pad relative.
HLD_SV:
	DC32    DOCON, NHLD

//:NONAME DICTFULL_ABORT ( -- ) Issue Dictionary Full Message and ABORT
DICTFULL_ABORT:
        DC32    DOCOL
	DC32	LIT, msg_dictfull
	DC32	NULLSTRLEN, TYPE
	DC32	ABORT

//:NONAME ALLOT_PRIM: ( un -- ) ALLOT WITHOUT DICTIONARY CHECK
ALLOT_PRIM:
        DC32    DOCOL
	DC32	DP_SV
	DC32	PSTORE
	DC32	SEMIS

//:NONAME COMMA_PRIM ( CFA -- ) COMMA WITHOUT DICTSPACE TEST
COMMA_PRIM:
        DC32    DOCOL
	DC32	ALIGN32_DP_FF_PAD       // ASM CANDIDATE
	DC32	HERE	                // is NEW DP
	DC32	STORE                   // CFA
	DC32	FOUR
	DC32	ALLOT_PRIM              // ALLOT W/O DICTSPACE CHECK
	DC32	SEMIS

//:NONAME QCOMP: ( -- ) Issue error message if not compiling.
QCOMP:
	DC32	DOCOL
	DC32	STATE_SV
	DC32	AT
	DC32	ZEQU
	DC32	LIT, msg_qcomp
	DC32	QERROR    // ( f nullstr-addr -- )
	DC32	SEMIS

//:NONAME QEXEC:	( -- ) Issue error message if not executing.
QEXEC:
	DC32	DOCOL
	DC32	STATE_SV
	DC32	AT
	DC32	LIT, msg_qexec
	DC32	QERROR    // ( f nullstr-addr -- )
	DC32	SEMIS

//:NONAME QPAIR:	( n1 n2 -- ) Issue an error message if n1 does not equal n2.
//	The message indicates that compiled conditionals do not match.
QPAIR:
	DC32	DOCOL
	DC32	SUBB
	DC32	LIT, msg_qpair
	DC32	QERROR    // ( f nullstr-addr -- )
	DC32	SEMIS

//:NONAME QSTACK:	( -- ) If Stack Error type error message then Abort.
//	Issue an error message if the stack is out of bounds. This
//	definition may be installation dependent.
QSTACK:
	DC32	DOCOL
	DC32	SPAT
	DC32	INITSO_SV
	DC32	AT
	DC32	SWAP
	DC32	ULESSTHAN	// U< ( 2 1 -- 0 ) ( 1 2 -- 1 )
	DC32	LIT, msg_qstack
	DC32	QERROR    // ( f nullstr-addr -- )
	DC32	SEMIS

//:NONAME PDOTQ:	( -- ) DOTQ run-time
//      The run-time proceedure, compiled by ." which transmits the
//      following in-line text to the selected output device. See ."
//      USE SINGLE QUOTES IN SOURCE USAGE!!!!
PDOTQ:
	DC32	DOCOL
	DC32	RFROM				// Addr of string = aofstr
	DC32	DUP, COUNT, TYPE
	DC32	COUNT, PLUS
	DC32	ALIGNED
	DC32	TOR
	DC32	SEMIS

//:NONAME QUERY:	( -- ) Use Expect to fill TIB
//	Input characters into TIB as a NULL terminated string using EXPECT.
//      TIB is filled until CR/EMTER or count argument to EXPECT is reached.
//      IN is set to zero and the string in TIB is NULL terminated. 
QUERY:
	DC32	DOCOL
	DC32	TIB_SV
	DC32	LIT, IOBUFSIZE-2	// 96-2=94
	DC32	EXPECT
        DC32    strva, 1, IN
	DC32	SEMIS

//:NONAME PNUMBER:	( 0 0 addr1 -- d addr2 )
//                      ( 0 0 addr1 -- LSW MSW addr2=addr1+chars )
//	Convert the ascii text beginning at addr1+l
//	with regard to BASE into a 64 bIt unsigned number d.
//	Addr2 is the address of the first unconvertable digit, usually space
//      or NULL.
//	Used by NUMBER.
PNUMBER:
	DC32	DOCOL
PNUM1:				// Begin
	DC32	ONEP	        // ( LSW MSW addr+1 -- )
	DC32	DUP, TOR	// ( LSW MSW  addr+1 -- ) copy of addr+1 >r
	DC32	CAT		// ( LSW MSW c -- )
	DC32	BASE_SV, AT	// ( LSW MSW c base -- )
	DC32    DIGIT   	// ( LSW MSW c base -- 0 0 digit 1 (ok)
                               // | ( 0 0 0 (bad))
// digit gets 0, 0x20 or 0x2E and returns 0 then branches to fromr semis...
	DC32	ZBRAN		//
	DC32     PNUM2-.	// If digit retuned 0 branch, else fall thru

	DC32    SWAP		// ( LSW MSW digit -- )
	DC32	BASE_SV, AT	// ( LSW digit MSW base -- )
//	U* (un un --  ud=<LSW MSW> )
	DC32   USTAR  		// ( LSW digit LSW MSW  -- )
	DC32	DROP
	DC32    ROT
	DC32	BASE_SV, AT	// ( base*accn bn base -- )
	DC32	USTAR           // ( base*accn bn*base --   )
	DC32	DPLUS           // ( base*accn+bn*base --   )
	DC32	DPL_SV, AT, ONEP
	DC32	ZBRAN		// IF true
	DC32    PNUM3-.	        // else do

	DC32	ONE, DPL_SV, PSTORE

PNUM3:				// endif
				// ( 0 0 <c | 0> -- )
	DC32	RFROM		// ( 0 0 <c | 0> addr+1 -- ) copy of addr+1 <r
	DC32	BRAN
	DC32    PNUM1-.	        // REPEAT

PNUM2:
	DC32	RFROM		// copy of addr+1 <r
	DC32	SEMIS

//:NONAME DFIND:	( --- pfa len tf )(found)
//		( --- ff ) (not found)
//	Accepts the next text word (delimited by blanks) in the input stream
//      to HERE, and searches the CONTEXT and then CURRENT vocabularies
//      for a matching entry. If found, the dictionary entry's parameter
//      field address, its length byte, and a boolean true is left.
//      Otherwise, only a boolean false is left.
//      DFIND LEN HAS TO BE smudged header byte for INTERPRET.
DFIND:
	DC32	DOCOL
#ifdef IO2TP
TIB_INPUT_HERE_IN_DFIND:
 DC32 NOOP	// R5 = @30A0h ENTER INTO TIB HERE
#endif
	DC32	BLANK   // ( ch -- )
	DC32	WORD    // ( cH-DELIM  --- )
	DC32	HERE	// ( txt -- )
	DC32	LATEST	// ( txt nfa -- )
	DC32	PFIND   // ( txt nfa -- pfa wordlen 1 ) (ok)
                        // | ( txt nfa -- 0 ) (bad)
	DC32	SEMIS

//:NONAME COMP:	( -- ) Compile CFA of word that follows inside a definition.
// DOTQ: DO: LOOP: PLOOP: LITERAL: DLITERALl COLON: SEMIS:
// DO DICTSPACE CHECK, AS DOES COMMA, CCOMMA and ALLOT
// SO COMMA_PRIM CREATED AND USED HERE FOR COMPILER SPEEDUP
//	Compile CFA of word that follows.
//	When the word containing COMPILE executes, the execution address of
//      the word following COMPILE is copied (compiled) into the dictionary.
//      This allows specific compilation situations to be handled in
//      addition to simply compiling an execution address (which the
//      interpreter already does).
COMP:
	DC32	DOCOL
//	DC32	QEXEC   // QCOMP
	DC32	RFROM   // addr of next word in body of this word
	DC32	DUP
	DC32	FOURP	// addr past next word...
	DC32	TOR     // that where execution resumes after
	DC32	AT      // compiling this word
//	DC32	COMMA	// ALIGNED, INCREMENTS DP REFLECTED by HERE
	DC32    COMMA_PRIM // NO DICTSPACE CHECK HERE
        DC32	SEMIS

//:NONAME MSMOD:	( ud1 n2 -- n3 ud4 )
//      An unsigned mixed magnitude math operation which leaves a double
//      quotient ud4 and remainder u3, from a double dividend ud1 and single
//      divisor u2. Primarily used in pictured numeric output DIG
MSMOD:
	DC32	DOCOL           // EX 1 0 A
	DC32	TOR             // -- 1 0
	DC32	ZERO            // -- 1 0 0
	DC32	R               // -- 1 0 0 A
	DC32	USLASH          // ( uLSW uMSW u32 --- u32REMAINDER u32QUOTIENT )
	DC32	RFROM
	DC32	SWAP            // -- 0 A 0 1 0
	DC32	TOR
	DC32	USLASH
	DC32	RFROM
	DC32	SEMIS

//:NONAME SCSP:	( -- ) Save the parameter stack position in CSP.
//      Part of the compiler security.
//	Means Store (save) Compile time stack position.
SCSP:
	DC32	DOCOL
	DC32	SPAT
	DC32	CSP_SV
	DC32	STORE
	DC32	SEMIS

//:NONAME QCSP: ( -- ) Issue error message if stack and CSP don't match.
//      Indicates unbalanced compilation. Part of compiler security
QCSP:
	DC32	DOCOL
	DC32	SPAT
	DC32	CSP_SV
	DC32	AT
	DC32	SUBB
	DC32	LIT, msg_qstack
	DC32	QERROR    // ( f nullstr-addr -- )
	DC32	SEMIS

//:NONAME INTERPRET:	( -- ) The Inner Interpreter
//	The outer text interpreter which sequentially executes or compiles
//	text from the TIB depending on STATE. If
//	the word name cannot be found after a search of CURRENT
//	it is converted to a number according to the current base.
//	That also failing, an error message will TYPE and ABORT.
//	Text input will be taken according to the convention for WORD. If a
//	decimal point is found as part of a number, a double number value
//	will be left. The decimal point is saved in DPL but otherwise
//	has no other purpose than to force this action.
//	See NUMBER.
INTERPRET:
	DC32	DOCOL
INTE1:			        // Begin
	DC32	DFIND           // IO2TP noop there for TIB entry
//	-FIND ( --- pfa len tf )(found) ( --- ff ) (not found)
	DC32	ZBRAN	        // IF not a word,
	DC32	 INTE2-.        // TRY AS A NUMBER

	DC32	STATE_SV        // CSTATE 0 = INTERPRET, Cxh = COMPILING
	DC32	AT              // DFIND len has to be SMUDGED NFA count
	DC32	LESSTHAN	// for CSTATE to pass IMMEDIATE words
	DC32	ZBRAN	        // To execute immdeiate words while compiling
	DC32	 INTE3-.

	DC32	CFA
	DC32	COMMA_PRIM	// No DICTSPACE check, aligned.
	DC32	BRAN	//ELSE
	DC32	 INTE4-.

INTE3:
	DC32	CFA     //EXECUTE FOUND WORD
	DC32	EXEC	//endif
INTE4:
	DC32	QSTACK   // IF STACK error abort using qerr
	DC32	BRAN	//ELSE
	DC32	 INTE1-.

INTE2:
	DC32	HERE
	DC32	NUMBER //( addr   ---   32LSW 32MSW ) IF error quit
	DC32	DPL_SV
	DC32	AT
	DC32	ONEP
	DC32	ZBRAN	// IF 1+ = 0
	DC32	 INTE6-.	// SINGLE NUMBER

	DC32	DLITERAL   // PUSH DOUBLE NUMBER TO STACK
	DC32	BRAN	// ELSE DOUBLE NUMBER
	DC32	 INTE7-.

INTE6:
	DC32	DROP
	DC32	LITERAL	// PUSH SINGLE NUMBER TO STACK

INTE7:
	DC32	QSTACK	// endif   // IF STACK error abort using qerr

INTE5:
	DC32	BRAN	// AGAIN
	DC32	 INTE1-.

//:NONAME QUIT:	( -- ) The Outer Interpreter.
//	USE QUERY TO GET INPUT. IF INPUT = CR EXECUTE NULL TO RETURN
//	THEN INTERPRET EXECUTES WORDS, CONVERTS NUMBERS OR RESTART THRU ABORT 
QUIT:
	DC32	DOCOL
	DC32	LBRAC   // Set CSTATE to zero - not compiling.
        DC32    SPSTO   // ADDED IN PAREN TEST
QUIT1:                  // Begin
	DC32	RPSTO
	DC32	QUERY   	// ( -- ) Input to TIB
	DC32	INTERPRET   	// ( -- ) RETURN BY CR placing null in tib ->  NULL:
#ifdef IO2TP
//BP1_QUIT:
// DC32 NOOP
 DC32    CLRPAD  // Resets OUT
#endif
// ISSUE ONLY CR IF COMPILING OR IF PROMPT: SET TO 0 WITH POFF(PROMPTOFF)
	DC32	STATE_SV        // 0xC0 is compiling
	DC32	AT
	DC32	ZEQU            // NOT = TO ZERO IF COMPILING SO 0 IF COMPILING
	DC32	ZBRAN		// BRANCH IF 0 - CSTATE IS NON-ZERO = compiling
	DC32	 QUIT2-.        // CR, LOOP AGAIN

// v1.7 new feature to vector the system prompt
// zero means only cr with every line - for download suppression of all but errs
        DC32    LIT, PROMPT, AT
	DC32	ZBRAN		// IF PROMPT$ = 0
	DC32	 QUIT2-.        // CR, LOOP AGAIN

// PROMPT$ is not zero
        DC32    LIT, PROMPT, AT
//        DC32    LIT, msg_MY_OK
	DC32    NULLSTRLEN, TYPE
        DC32    BASE_SV, AT
        DC32    DOTDEC
#ifdef IO2TP
BP2_QUIT:
 DC32 NOOP
	DC32	CLRPAD  // Resets OUT
	DC32	CLRTIB  // Resets IN
#endif
QUIT2:
        DC32    CR		// HERE FOR BOTH COMPILE AND INTERPRET
//BP23_QUIT:
// DC32 NOOP
	DC32	BRAN		// AGAIN
	DC32	 QUIT1-.

//:NONAME BACK:	( addr -- ) Branch back primitive
//	Calculate the backward branch offset from HERE to addr and compile
//      into the next available dictionary memory address.
BACK:
	DC32	DOCOL
	DC32	HERE
	DC32	SUBB
	DC32	COMMA
	DC32	SEMIS

//:NONAME SEMIC_CREATE:      ( -- ) Used after CREATE to reset CSDP
//      Used in ; CON and VAR to reset CSDP which is used to auto forget
//      Words that have errors during compilation.
SEMIC_CREATE:
	DC32	DOCOL
        DC32    strva, 0, CSDP  // RESET AUTO FORGET WHEN CREATE ERROR
        DC32    SEMIS

//:NONAME CSDP_SV ( -- addr of CSDP ) Holds where to reset if definition fails
CSDP_SV:
	DC32	DOCON, CSDP

//:NONAME	PM:  ( n1 n2 -- n3 ) Apply the sign of n2 to n1, which is left as n3.
PM:
	DC32	DOCOL
	DC32	ZLESS
	DC32	ZBRAN	//IF
	DC32	PM1-.

	DC32	NEGATE	//endif
PM1:
	DC32	SEMIS

//:NONAME	DPM: ( d1 n -- d2) Apply the sign of n to the double number d1.
DPM:
	DC32	DOCOL
	DC32	ZLESS
	DC32	ZBRAN	//IF
	DC32	DPM1-.

	DC32	DNEGATE	//endif
DPM1:
	DC32	SEMIS

//:NONAME SIGNON:     ( -- ) type SIGNON message.
SIGNON:
	DC32    DOCOL
#ifdef IO2TP
        DC32    zero_OUT
#endif
        DC32    CR, LIT, msg_FISH
        DC32    NULLSTRLEN, TYPE
        DC32    LIT, msg_FISH_TIMESTAMP
        DC32    NULLSTRLEN, TYPE, CR
#ifdef IO2TP
SIGNON_BP1:
 DC32 NOOP
        DC32    CLRPAD  // Resets OUT
#endif
        DC32    SEMIS

//:NONAME WARM:       ( -- ) Perform FWARM and FLASH_SCAN
//      SYSTEM RAMVARSPACE INIT
//      Patch in FLASH_SAVE'd Code.
WARM:
	DC32	DOCOL
	DC32	FWARM
        DC32    FLASH_SCAN
	DC32	SEMIS

//:NONAME	TRAVERSE TRAVERSE: ( addr1 n -- addr2 ) Used in NFA and PFA
//	USED only in NFA and PFA, padding alignment is done there.
//	Move across a variable length padding aligned name field with smudged 1rst and last byte.
//	addr1 is the address of either the length byte or the last letter.
//      If n=1, the motion is toward nfa - hi memory, if n=-l, the motion is
//      toward CFA - low memory. The addr2 resulting is address
//	of the other end of the name.
TRAVERSE:
	DC32	DOCOL
	DC32	SWAP
TRAVERSE_BEGIN:
	DC32	OVER	// Begin
	DC32	PLUS
	DC32	LIT,0x7F
	DC32	OVER
	DC32	CAT
	DC32	LESSTHAN
	DC32	ZBRAN	//until
	DC32	 TRAVERSE_BEGIN-.

	DC32	SWAP
	DC32	DROP
	DC32	SEMIS

//------------------------------------------------------------------------------
// ASM CODE START - MUST BE ALLIGNED AGAIN
/* AS IN:
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
LABEL:
	DC32	.+5
 SECTION .text : CODE (2)
        ASM     CODE
*/
//------------------------------------------------------------------------------
//:NONAME SECTION:
// Upper case names are FISH model primitives.
// lower case names are v4th model primitives.

//:NONAME strva:    ( -- ) GET VALUE THEN ADDR FROM NEXT 2 CELLS AND WRITE TO ADDR
//      Use proper LABEL for values/addresses!!! EX: FPA vs FPADDR
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
strkk:
strva:
STRVA:
strda:
	DC32	.+5
 SECTION .text : CODE (2)
        LDM     i!, {n} // GET [i] TO n.
        LDM     i!, {w} // next [i] yp w.
        STR     n, [w]  // n w ! w/o stack
        NEXT

//:NONAME atk:	 (  -- ) get value at inline const address
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
atk:	; ( x1 -- x1, x2 )
	DC32	.+5
 SECTION .text : CODE (2)
//	DUP     
	ILK	t
	ldr	t, [t]
	TPUSH

//:NONAME ork:    ( x -- x' ) Get inline konstant to OR with TOS.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
bisk:
ork:
	DC32	.+5
 SECTION .text : CODE (2)
	ILK	w
        POP2t
	orrs	t,w
	TPUSH

//:NONAME andk:   ( x -- x' ) Get inline konstant to AND with TOS.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
andk:
	DC32	.+5
 SECTION .text : CODE (2)
	ILK	w
        POP2t
	ands	t,w
	TPUSH

//:NONAME strk:   ( n -- ) Get inline konstant and store it to TOS addr.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
strk:	; ( x -- )
	DC32	.+5
 SECTION .text : CODE (2)
	ILK	x
        POP2t
	str	t, [x]
//	DROP
	NEXT


//:NONAME RMWAMD:	 ( bitdata_ilk3, mask_ilk2, addr_ilk1 -- )
// ALIASES: rmwkkk AND rmwamd
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
RMWAMD:	
	DC32	.+5
 SECTION .text : CODE (2)
	POP2x   // addr_ilk1
	POP2n
	POP2t
	ldr	w, [x]
	bic	w, w, n
	and	t, t, n
	orr	w, w, t
	str	w, [x]
	NEXT

 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
rmwkkk:
rmwamd:	; ( -- )	addr_ilk1,  mask_ilk2,  bitdata_ilk3
	DC32	.+5
 SECTION .text : CODE (2)
	ILK	x
	ILK	y
	ILK	k
	ldr	w, [x]
	bic	w, w, y
	and	k, k, y
	orr	w, w, k
	str	w, [x]
	NEXT

 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
begin:	; r:( -- addr )
	DC32	.+5
 SECTION .text : CODE (2)
	str	i, [r, #-4]!
	NEXT

 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
until:	; ( x -- )
	DC32	.+5
 SECTION .text : CODE (2)
        POP2t
	cmp	t, #0
//	DROP
	ite	eq
	ldreq	i, [r]		; r:( -- )
	addne	r, r, #4	; r:( addr -- )
	NEXT



//:NONAME SV_INIT_VALUES: Table of FISH SYSTEM VAR Iinitial values.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
SV_INIT_VALUES:
// 4 words of System Variables
        DC32    DEFAULT_BASE            // NBASE
        DC32    1                       // IN
        DC32    0                       // OUT
        DC32    0                       // CSTATE
// 5 addresses of System Data Structures
        DC32    PINIT                   // INITSO
        DC32    RINIT                   // INITRO
        DC32    TIB                     // INITTIB
        DC32    RAMVARSPACE             // UP
        DC32    ORIG                    // DP
// 4 addresses of System Pointers
        DC32    0                       // CSDP
        DC32    0                       // FENCE
#ifdef FISH_PubRel_WORDSET
        DC32    WC_FISH_PubRel_NFA      // CURRENT
#endif
#ifdef FISH_NXP_M0_PRO_WORDCAT
        DC32    WC_FISH_PRO_NFA         // CURRENT
#endif
        DC32    msg_MY_OK               // PROMPT
// 13d WORDS TO COPY


//:NONAME FWARM   ( -- ) FISH SYSTEM VAR Initialization primitive.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
FWARM:
	DC32	.+5
 SECTION .text : CODE (2)
FWASM:
        LDR     t, = SV_INIT_VALUES
        LDR     n, = END_RAMVARSPACE_SV_START
        LDR     w, = 13d
FWBEGIN:
        LDR     x, [t]
        STR     x, [n]
        ADDS    t, t, #4
        ADDS    n, n, #4
        SUBS    w, w, #1
        BNE     FWBEGIN
        
        NEXT
 LTORG  // Always outside of code, else data in words

//:NONAME DOCOL   ( -- )  COMPILED VERSION OF COLON
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
DOCOL:	// no cfa for DOCOL!!!!!	<<<<<<<<<<<<<<<<<<
	PUSHi2r		// save IP to Rstack
	MOV	i, w	// jam new IP = cfa+4
	NEXT

//:NONAME SEMIS:	( -- ) COMPILED VERSION OF SEMICOLON
//      Terminate a colon-definition,
//	: pushed the calling word return addr to the return stack
//	This retrieves it and returns to calling word.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
SEMIS:
	DC32	.+5
 SECTION .text : CODE (2)
	POPr2i	// pop docol saved IP from Rstack
	NEXT

//:NONAME DOVAR   ( -- ) COMPILED VERSION OF VAR
// VAR is a CON but DOVAR is 'duplicated' to allow search for it's CFA
// so FORGET can reclaim VAR space.
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
DOVAR:
	LDM	w!, {t}
	TPUSH

//:NONAME DOCON   ( -- ) COMPILED VERSION OF CON
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
DOCON:
	LDM	w!, {t}
	TPUSH

//:NONAME LIT:        ( -- n ) The primitive that pushes a number in a definition.
//      Within a colon-definition, LIT is automatically compiled before each
//      32 bit literal number encountered in input text. Later execution of
//      LIT causes the contents of the next dictionary address ( the LITERAL #)
//      to be pushed to the stack.
//      Use LITERAL
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
LIT:
	DC32	.+5
 SECTION .text : CODE (2)
	LIT2t 	// LDM		i!, {t}
		// fetch memory i points to into {t}, inc i after
	TPUSH

//:NONAME RLIT:	( -- n ) LIT primitive that can have it's value relocated.
//	SEE LIT. ALLOWS RELOCATION OF THIS type OF LIT
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
RLIT:
	DC32	.+5
 SECTION .text : CODE (2)
	LIT2t 		// LDM		i!, {t}
			// fetch memory i points to into {t}, inc i after
	TPUSH

//:NONAME GOTO:   ( cfa -- ) Redirect execution to cfa in another hi-level word.
//	CAUTION!!!!!!! A SYSTEM WORD EXPOSED FOR THE BOLD AND BRAVE!!!!!!!
//	Stack IN CURRENT WORD MUST BE CLEANED UP!
//	AND STACK MUSTE BE SET UP as expected where you GOTO.
//	YOU CANNOT VIOLATE controls structure (LOOPS, BEGINs IFs, etc).
//	Unless you come from the same point in the same control structure!
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
GOTO:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2i // set i as pointer to cfa to start execution
	NEXT

//:NONAME CREATE_DOES_GOTO:   ( -- )
//      PUSH ADDRESS OF CREATED WORDS PFA THEN
//      Redirect execution to cfa of the CREATE DOES> word.
//      THIS IS LIKE A DOCON IE NO DOCOL!!!!
//	CAUTION!!!!!!! A SYSTEM WORD EXPOSED FOR THE BOLD AND BRAVE!!!!!!!
//	Stack IN CURRENT WORD MUST BE CLEANED UP!
//	AND STACK MUSTE BE SET UP as expected where you GOTO.
//	YOU CANNOT VIOLATE controls structure (LOOPS, BEGINs IFs, etc).
//	Unless you come from the same point in the same control structure!
CREATE_DOES_GOTO:
 SECTION .text : CODE (2)
// WILL NEED A NEW CASE IN FLASH_SAVE!
// NO DOCOL - SO FUDGE ONE
// PUSH ADDR AFTER EXEC IN INTERPRET TO r:
	PUSHi2r		// save IP to Rstack
// AND THIS LOADS GOTO ADDR
        LDR     i, [t, #4]      // GET GOTO ADDRESS
// THIS PUSHES PFA TO t
        ADDS    t, t, #8        // COMPUTE REAL PFA
        PUSHt
	NEXT

//:NONAME DICTSPACE:  ( -- n ) Calculate and push dictionary space available
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
DICTSPACE:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR	t, = DICTSPACE_END
	LDR	n, = DP
	LDR	n, [n]
	SUBS	t, t, n
	TPUSH
// LTORG

//:NONAME VARSPACE:    ( -- n ) Calculate and push VAR space available
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
VARSPACE:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR	t, = END_RAMVARSPACE_SV_START
	LDR	n, = UP	// UP IS ALLOCATION POINTER FOR VARS AND VARALLOT
	LDR	n, [n]
	SUBS	t, t, n
	TPUSH
 LTORG

// Rewrite for TOSCT interop?
//:NONAME PFIND:	( addr1 addr2 -- pfa b tf ) (ok)
//                      ( addr1 addr2 -- ff ) (bad)
//      LEN HAS TO BE smudged header byte for INTERPRET.
//	Header LEN BYTE REQUIRED by Interpret for immediate word execution
//	Searches the dictionary starting at the name field address addr2,
//      matching to the text at addr1. Returns parameter field address,
//      length byte of name field and boolean true for a good match. If no
//      match is found, only a boolean false is left.
//
//	To step thru to a word set bp at :NONAME
//	and watch r3 for match of word addr from symbol file
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
PFIND:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2x   // R3 = Dict Name NFA
	POP2w	// R2 = String Addr usually at here
	MOV     rb, w   	// Save TXT String ADDR

//	SEARCH LOOP
//	nfa length test
//      x must = Dict nfa
PFIN1:              	        // ADDR-->char counted strings
	MOV	w, rb		// restore txt str addr
	LDRB    t, [x]  	// GET nfa LENGTH
//      LEN HAS TO BE smudged header byte for DFIND -> INTERPRET.
//      ELSE DO THIS HERE TO MAKE IT ACTUAL & REMOVE SECTION BELOW
//	MOVS	y, #0x3F
//	ANDS	t, y
	MOV	k, t		// Save search word length
	LDRB    n, [w]  	// TXT WORD LEN
	EORS	t, t, n
	MOVS	y, #0x3F
	ANDS	t, t, y
	BNE	PFIN5   // Len mismatch, GET NEXT LINK FIELD ADDR (lfa)

//	LENGTHS MATCH - CHECK EACH CHARACTER IN NAME
// 	TEST FIRST CHAR AND DETERMINE IF LAST
PFIN2:
	ADDS    w, w, #1        // str
	ADDS    x, x, #1        // nfa
	LDRB    t, [x]          // GET nfa 1RST CHAR
	LDRB    n, [w]          // GET TXT 1RST CHAR
	CMP	t, n	        // eor was working but this make 0x80 subb
	BEQ	PFIN2           // matched try next char
  
	MOVS	y, #0x80
	SUBS	t, t, y		// ascii only in t and n
	cmp	t, n
	BNE	PFIN51          // goto  WORD MISMATCH

//      FOUND END OF NAME (BIT 8 SET) - A MATCH
//      GET to pfa of found word
PFIN21:

	ADDS    x, x, #1	// Dict addr
	LDRB    t, [x]
	CMP     t, #0xFF
	BEQ     PFIN21

thispfa:

	ADDS    x, x, #8        // at cfa -> pfa
	PUSHx			// PUSH pfa
	MOVS    t, #1           // TRUE VALUE
	MOV     w, k		// RETURN Header Byte LENGTH
	DPUSH

//	NO NAME MATCH - TRY ANOTHER
//      Set NEXT LINK FIELD ADDR (lfa) to x
//      ( ZERO = FIRST WORD OF DICTIONARY )
//      GET TO END OF NAME BEFORE Padding IF ANY
//      ENTER HERE FROM LEN!= (RB->CNT)
PFIN5:
	ADDS     x, x, #1       // inc char in name addr

//      ENTER HERE FROM WORD MISMATCH
PFIN51:
	LDRB    t, [x]		// get Dict Word char
	MOVS	y, #0x80
	TST     t, y	        // is last char in word bit set
	BEQ     PFIN5

//      UNMATCHED NAME - POINTING AT LAST CHAR BEFORE Padding IF ANY
//      GET PAST FF Padding AND GET lfa
PFIN7:
// Code for ALIGNED
//	ADDS		x, x, #3
	ADDS		x, x, #4        // add 1 for entry
	LDR		t, =-4
	ANDS		x, x, t
	LDR     x, [x]          // GET lfa to next word
	CMP     x, #0    	// START OF DICT ( 0 ) ?
	BEQ		WORDNOTFOUND

// NOT A BRAN TARGET - This one used for breakpoint
PFIND_NEXT_NFA:
	B	PFIN1           // Search next word x must = Dict nfa

WORDNOTFOUND:             	// PFIND: DONE ( NO MATCH FOUND )
	MOVS     t, #0
	TPUSH
 LTORG

//:NONAME BRAN:	( -- ) Branch in definitions primitive
//      In IAR branch target MUST BE ON Next LINE!!!<<<<<<<<<<<<<<<<<<<<<<<<
//      The run-time proceedure to unconditionally branch. An in-line offset
//	is added to the interpretive pointer IP to branch ahead or back.
//	BRANCH is compiled by ELSE, AGAIN, REPEAT.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
BRAN:
	DC32	.+5
 SECTION .text : CODE (2)
BRAN1:
	LDR	x, [i]		// Get branck target
	ADDS	i, i, x         // use as offset
	NEXT

//:NONAME ZBRAN:	( f -- ) Branch if zero definition primitive.
//      In IAR branch target MUST BE ON Next LINE!!!<<<<<<<<<<<<<<<<<<<<<<<<
//	The run-time proceedure to conditionally branch. If f is false
//	(zero), the following in-line parameter is added to the interpretive
//	pointer to branch ahead or back. Compiled by IF, UNTIL, and WHILE.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
ZBRAN:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	CMP	t, #0		// ZERO?
#ifdef TOSCT                    // CMP CONSUMES t
        LDR     t, [p]          // REFRESH t
#endif
	beq	BRAN1		// YES, BRANCH

	ADDS	i, i, #4	// NO - CONTINUE...
	NEXT

//:NONAME XLOOP:	( -- ) Loop primitive in a definition.
//      The run-time proceedure compiled by LOOP which increments
//	the loop index by one and tests for loop completion.
//      See LOOP.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
XLOOP:
	DC32	.+5
 SECTION .text : CODE (2)
	MOVS		x, #1
XLOO1:
	// Get Limit and Index from r w/o popping them
	LDR     y, [r, #4]      // Limit
	LDR     w, [r]          // Index
	CMP	y, w            // If equal
	BEQ     XLOO2		// done

	adds    w, w, x         // INDEX = INDEX + INCR
	str     w, [r]		// Put it back by overwrite for I
	SUBS    y, y, w
	BNE     BRAN1		// End of `DO' LOOP
XLOO2:
	// Drop Limit and Index, increment i
	ADDS	r, r, #8
	ADDS	i, i, #4
	NEXT

//:NONAME XPLOOP:	( n -- ) +LOOP primitive in definitions.
//      The run-time proceedure compiled by +LOOP, which increments the loop
//      index by n and tests for loop completion. See +LOOP.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
XPLOOP:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2x			// GET LOOP VALUE
#ifdef TOSCT
        LDR     t, [p]
#endif
	b       XLOO1   	// see XLOO1 for why it's put in x

//:NONAME XDO:	( Limit Index -- ) The DO primitive in definitions.
//	( Limit = addr+cnt Index = addr  -- ) as in ( TIB+LEN TIB -- )
//      The run-time proceedure compiled by DO which moves the loop control
//      parameters to the return stack. See DO.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
XDO:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t			// INITIAL INDEX VALUE = ADDR
	POP2w			// LIMIT VALUE = ADDR+CNT
	PUSHw2r
	PUSHt2r			// i expects index TOP of RSTACK
#ifdef TOSCT
        LDR     t, [p]  // REFRESH t
#endif
	NEXT

//:NONAME CATLT7F  ( addr -- c ) Get char at addr then AND it with 7Fh
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
CATLT7F:
	DC32	.+5
 SECTION .text : CODE (2)
        POP2t
        LDRB    t, [t]
        LDR     n, =7Fh
        ANDS    t, t, n
        TPUSH

//:NONAME TIB_CHAR_SCAN   ( c -- f ) Scan TIB for c or until null found.
// Scan TIB, a null terminated string at TIB+IN for 'c' or null termination.
// Uses and adjusts IN to reflect offset to c or null in TIB.
// Return false flag if null found first, indicating end of input in TIB,
// ( usually from QUERY ) else return true flag, either with IN adjusted.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
TIB_CHAR_SCAN:
	DC32	.+5
 SECTION .text : CODE (2)
// t is char matched from TIB as flag to return
	POP2n   		// Char were lookin for
        LDR     w, = TIB
        MOV     ra, w           // Save for IN calculation.
        LDR     x, = IN
        LDR     y, [x]          // Get offset in to TIB
        ADDS    w, w, y         // Create TIB+IN
TCS_LOOP:
        LDRB    t, [w]          // Strting at TIB+IN ( ex space after ( .
        ADDS    w, w, #1        // When matched
        CMP     t, n            // Character passed to test
        BEQ     TCS_FOUND
        
        CMP     t, #0           // Null at end of TIB?
        BNE     TCS_LOOP        // no keep looking

TCS_FOUND:
        MOV     y, ra           // TIB
        SUBS    w, w, y         // TIB+Char offset
        STRB    w, [x]          // Set IN
        TPUSH                   // t is null or char

//:NONAME TOGGLE:	( addr b -- ) Complement BYTE at addr by the bit pattern b.
//      Byte operator for NFA count byte
//      Complement (FLIP) the BYTE contents of addr by the bit pattern b.
//	SEE SETBITS and CLRBITS
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
TOGGLE:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2n 		// Bit PATTERN
	POP2w		// ADDR
	LDRB	t, [w]
	EORS   	t, t, n
	STRB    t, [w]
	NEXT

#ifdef XON_XOFF
//:NONAME IF_EOL_SEND_XOFF: Get xoff out ASAP in EXPECT!
//      Must leave char in t!!!!!!!!!
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
IF_EOL_SEND_XOFF:
	DC32	.+5
 SECTION .text : CODE (2)
#ifdef  IO2TP
	NEXT
#else
        LDR     n, = 0Ah        // ^J
        CMP     t, n
        BNE     CR_CHECK

SUB_CR_4_NL:
        POP2t
        LDR     t, = 0Dh        // ^M
        PUSHt
        B       EOL_SEND_XOFF

CR_CHECK:
        LDR	n, = 0Dh
	CMP	t, n    // LEAVE CHAR IN T!!!!
	BNE	NOT_CR

EOL_SEND_XOFF:

#ifndef IO2TP
        BL      TXRDY_SUBR
        BL      XOFF_SUBR
#endif

NOT_CR:
	NEXT
#endif  // not IO2TP
#endif // XON_XOFF

//:NONAME NUMBERSUFFIX:      ( addr -- addr ) Number BASE suffix eval primitive.
//      In NUMBER temporarily change base if number has valid suffix.
//      IF LEN =1 LEAVE IT!
//      IF LEN =2 CHECK FOR .suffix!
//      addr is counted number striong. The count is ignored by NUMBER
//      If valid Base suffix replace suffix with null for NUMBER.
//      NUMBER must save and restore base outside of this call.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
NUMBERSUFFIX:
	DC32	.+5
 SECTION .text : CODE (2)
        NDPOP2t         // macro = copy tos to t, leave it on the stack
        LDRB    n, [t]  // get string lentgth
        CMP     n, #1
        BEQ     NSEXIT1
        
        CMP     n, #2
        BEQ     NSTWO

NSNOTTWO:
        ADD     n, n, t // point to end of string
        LDRB    w, [n]  // get possible suffix
        CMP     w, #'b' // 62h
        BEQ     USEBIN
        CMP     w, #'%'
        BEQ     USEBIN
        
        CMP     w, #'d' // 64h
        BEQ     USEDEC
        CMP     w, #'#'
        BEQ     USEDEC
        
        CMP     w, #'h' // 68h
        BEQ     USEHEX
        CMP     w, #'$'
        BEQ     USEHEX

NSEXIT1:
        NEXT

NSTWO:  // HERE BECAUSE INPUT LEN IS 2 SO IF . IS FIRST
        LDRB    w, [t, #1]      // GET 1RST CHAR
        CMP     w, #'.'         // IF IT'S A DOT
        BNE     NSNOTTWO
        NEXT

USEBIN:
        LDR     y, =2
        B       CLRSUFFIX
USEDEC:
        LDR     y, =10
        B       CLRSUFFIX
USEHEX:
        LDR     y, =16

CLRSUFFIX:
        LDR     w, =0X00        // =0x20
        STRB    w, [n]
        LDR     w, =NBASE       // BASE_SV in FISH
        STR     y, [w]
        NEXT
 LTORG

//:NONAME USTAR:	(u1 u2 --  ud=<LSW MSW> ) USED INTERNALLY - NOT UNSIGNED
//      Leave the unsigned double number product of two unsigned numbers.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
USTAR:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	POP2n
// IMPORT c_32by32to64mul	// LSW in t (r0), MSW in n (r1)
//	BL	c_32by32to64mul
//	MOV	w, t	// LSW
//	MOV	t, n	// MSW
// UMULL ilegal register R0 is not allowed here
        UMULL   w, t, t, n	; rdLO rdHi rn * rs
	DPUSH		//  --  LSW MSW )

//:NONAME	USLASH:	( d n --- u32REM u32QUO ) USED INTERNALLY - NOT UNSIGNED
//      Leave the unsigned remainder u2 and unsigned quotient u3 from the
//      unsigned double dividend ud and unsigned divisor u1.
//      RESULTS ARE NOT A TRADITOIONAL DOUBLE NUMBER ONE WOULD PRINT USING D.
//      Quotient = int ( Dividend / Divisor )
//      Remainder = Dividend - Quotient * Divisor
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
USLASH:
	DC32	.+5
 SECTION .text : CODE (2)
//	U/  USLASH: ( uLSW uMSW u32 --- u32REMAINDER u32QUOTIENT )
	POP2t			// U32  divisor
	POP2x     		// UMSW  dividendHI
	POP2w			// ULSW dividendLO
	CMP     t, #0           // divide by zero is divisor = 0
	BEQ     DZERO           // J Not Below
        TEQ     x, #0           ; fIG BEHAVIOR
        BNE     DZERO           ; TREAT DIVIDEN HI N=0 AS ERR

// IMPORT c_64by32div		// LSW of quo in t (r0), rem in w (r2)
//  	BL	c_64by32div
	MOV	n, t		; SAVE DIVISOR
        ;Quotient = int ( Dividend / Divisor ) ;UDIV only does this part
        ;UDIV RD = DIVIDEN / DIVISOR (rEV)          t                 w          t
// Error[438]: This instruction is not available in the selected cpu/core
        UDIV    t, w, t         ;DIV	AX,BX = QUOTIENT = INT ( dividendLO / DIVISOR )
        
        ;Rem =  Dividend - Quotient * Divisor ;This part you need to add;
        ;MLS RD, QUOTIENT, DIVISOR, DIVIDEND
        ;MLS RD = ( ARG1 * ARG2 ) - ARG3
        ;MLS    w, t, QUO  n, DIVOR  W DIVIDEND  I.E MLS RD = W - ( T * N ))
// Error[438]: This instruction is not available in the selected cpu/core
        MLS     w, t,      n,        w
	DPUSH			//  --  LSW MSW )

DZERO:
	EORS	t, t, t		// zero
	SUBS	t, t, #1        // 
	MOV     w, t            // 
	DPUSH			//  --  LSW MSW )

//:NONAME SPSTO:	( -- ) Initialize the stack pointer from INITSO.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
SPSTO:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR   	w, = INITSO	// SYSTEM VAR BASE ADDR, Initially set to PINIT
	LDR   	p, [w]		// RESET PARAMETER STACK PTR
	NEXT
// LTORG

//:NONAME RPSTO:	( -- ) Initialize the return stack pointer from INITR0.
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
RPSTO:
	DC32	.+5
 SECTION .text : CODE (2)
	LDR   	w, = INITRO	// SYSTEM VAR BASE ADDR, Initially set to RINIT
	LDR	r, [w]		// SET RETURN STACK PTR
	NEXT
 LTORG

//:NONAME BASE_TO_R12:    ( -- ) Copy BASE to r12
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
BASE_TO_R12:
	DC32	.+5
 SECTION .text : CODE (2)
        LDR     w, =NBASE
	LDR	n, [w]
        MOV     k, n
	NEXT
 LTORG
 
//:NONAME BASE_FROM_R12:    ( -- ) Set BASE from r12
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
BASE_FROM_R12:
	DC32	.+5
 SECTION .text : CODE (2)
        LDR     w, =NBASE
        MOV     n, k
	STR	n, [w]
        NEXT
 LTORG
 
//------------------------------------------------------------------------------
// SECTION HEADER TO RE ALIGN ALL CODE AFTER THIS INCLUDE
 SECTION .text : CONST (2)
 ALIGNROM 2,0xFFFFFFFF
//------------------------------------------------------------------------------
