// Valid thru v1.8

//#define ss      // "single-step" switch for debug/shakedown testing

//------------------------------------------------------------------------------
//	Inner Interpreter Macros
//	Beauty is you can put any ISA in a macro and no prob till referenced!
NEXT	MACRO	
// ARMv7-M Thumb = .+5
	ldr	w, [i], #4      ; get IP->cfa to w, incr i after
// ARMv6-M Thumb = .+4
//        LDM	i!, {w}	// get IP->cfa to w, incr i after
	NEXT1
	ENDM
#ifndef ss
// ARMv7-M Thumb = .+5
NEXT1	MACRO	
	BIC	w, w, #3        ; Tested WORKS ALSO Thumb2 klooge: clear the 2 LSbits
	LDR	x, [w], #4	; contents of cfa -> x, bump w to cfa+4
	BX	x		; w preserves cfa+4 for DOCOL's benefit
// ARMv6-M Thumb = .+4
//	LDM	w!, {x}		// contents of cfa, (pfa), -> x, bump w to cfa+4
//	BLX	x		// w preserves cfa+4 (pfa) for DOCOL's benefit
	ENDM

// ELSE
#else
NEXT1	MACRO	
	LDR	x, =ssNEXT1	// meta-single-step for debugging
	BX	x
	ENDM
// LTORG
#endif
// ENDIF

TPUSH	MACRO	
	PUSHt	// push t to p, pre decrement p
	NEXT
	ENDM

DPUSH	MACRO	
	PUSHw	// push w to p, post decrement p
	TPUSH
	ENDM

//------------------------------------------------------------------------------
//      FISH STACK MACRO's
//	Beauty is you can put any ISA in a macro and no prob till referenced!
// 	Cortex M0 THUMB only does STMia and LDMia
//	I (rstack value to pstack) expects POP to be post increment
//	Meaning that TOS is = to current p or r
//	Therefore PUSH is pre decrement

PUSHt	MACRO
	STR	t, [p, #-4]!    // Pre-increment
//      SUBS	p, p, #4	// push t to p, pre decrement p
//	STR	t, [p]
	ENDM

POP2t	MACRO
	LDR	t, [p],#4       // Post-increment
//#ifndef TOSCT
//	LDR	t, [p]		// pop tos to t, post increment p
//#endif
//      ADDS	p, p, #4
	ENDM

NDPOP2t MACRO           	// macro = copy tos to t, leave it on the stack
	LDR	t, [p]
        ENDM

PUSHn	MACRO
	STR	n, [p, #-4]!
//	SUBS	p, p, #4	// push n to p, pre decrement p
//	STR	n, [p]
	ENDM

POP2n	MACRO
	LDR	n, [p],#4
//	LDR	n, [p]
//	ADDS	p, p, #4
	ENDM

PUSHw	MACRO
	STR	w, [p, #-4]!
//      SUBS	p, p, #4	//  push w to p, pre decrement p
//	STR	w, [p]
	ENDM

POP2w	MACRO
	LDR	w, [p],#4
//	LDR	w, [p]
//	ADDS	p, p, #4
	ENDM

NDPOP2w MACRO           	// macro = copy tos to w, leave it on the stack
	LDR	w, [p]
        ENDM

PUSHx	MACRO
	STR	x, [p, #-4]!
//	SUBS	p, p, #4	//  push x to p, pre decrement p
//	STR	x, [p]
	ENDM

POP2x	MACRO
	LDR	x, [p],#4
//	LDR	x, [p]
//	ADDS	p, p, #4
	ENDM

NDPOP2x MACRO           	// macro = copy tos to w, leave it on the stack
	LDR	x, [p]
        ENDM

PUSHi	MACRO
	STR	i, [p, #-4]!
//	SUBS	p, p, #4	// push i to p, pore increment p
//	STR	i, [p]
	ENDM

POP2i	MACRO
	LDR	i, [p],#4
//	LDR	i, [p]
//	ADDS	p, p, #4
	ENDM

PUSHt2r	MACRO
	STR	t, [r, #-4]!
//	SUBS	r, r, #4	//  push t to r, pre decrement r
//	STR	t, [r]
	ENDM

PUSHi2r	MACRO
        STR	i, [r, #-4]!	; save IP to Rstack
//	SUBS	r, r, #4	//  push i to r, pre decrement r
//	STR	i, [r]
	ENDM

POPr2i	MACRO
	LDR	i, [r],#4	; pop DOCOL'd IP from Rstack
//	LDR i, [r]		//  pop r to i, post increment r
//	ADDS	r, r, #4
	ENDM

//POP2p	MACRO
//	ENDM
//POP2PC	MACRO 
//	ENDM

POPr2t	MACRO 
	LDR	t, [r],#4
//	LDR	t, [r]
//	ADDS	r, r, #4
	ENDM

PUSHn2r	MACRO
	STR	n, [r, #-4]!
//	SUBS	r, r, #4	//  push t to r, pre decrement r
//	STR	n, [r]
	ENDM

PUSHw2r	MACRO
	STR	w, [r, #-4]!
//	SUBS	r, r, #4	//  push w to r, pre decrement r
//	STR	w, [r]
	ENDM

LIT2t	MACRO
	// as in xeq token at ToS setup for lit
 	LDR	t, [i], #4
//      LDM	i!, {t}	// fetch memory p points to into {t}, inc i
	ENDM

POPp2w	MACRO
	// as in xeq token on ToS setup for exec
        LDR	w, [p], #4	; xeq token at ToS
//      LDM	p!, {w}	// fetch memory p points to into {w}, inc p
	ENDM

ILK	MACRO	reg
; *generalized* v4th InLineKonstant
        ldr	reg, [i], #4	; pre-inc IP
	ENDM

