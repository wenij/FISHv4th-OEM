// FISH_STM32F4_FLASH.s
// FLASH_SAVE, FLASH_FORGET, FLASH_SCAN and support :NONAME's 

/* 2DO:
// FLASH_SAVE LEAVING HERE ON STACK
---
// SAVE PAGE IF ENOUGH ROOM LEFT FOR THIS SAVE
FLPT:
// NOT IMPLEMENTED YET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
---
// DETERMINE IF ASSUREPAGE NEEDED, IF SO REWRITE AND INCORPORATE
---
// MAKE IRQ Wordcat AND ADD:
//:NONAME CMSIS_DISABLE_IRQS ( -- )
//:NONAME CMSIS_ENABLE_IRQS ( -- )
*/

//	FLASH_SAVE FLASH_SAVE:	( -- ) Save RAM to Flash
//      EVERY SAVE HAS A HEADER SET FOR FLASH_SCAN TO USE
//      ALIGNED HERE IS BOUNDARY OF NEW WRITE
//      SAVES ARE CONTIGUOUS WORDS USING x32 WRITE SETTING
 SECTION .text : CONST (2)
FLASH_SAVE_NFA:
	DC8	0x80+10
	DC8	'FLASH_SAV'
	DC8	'E'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_IRQ_NFA
FLASH_SAVE:
	DC32	DOCOL
// SOMETHING TO SAVE TEST - IF LATEST IS NOT IN ROM
	DC32	LATEST, DBASE, LESSTHAN
	DC32	ZBRAN
        DC32     FLPT-.

        DC32    CR
        DC32    PDOTQ
        DC8     15D
        DC8     'NOTHING TO SAVE'
 ALIGNROM 2,0xFFFFFFFF
	DC32	SEMIS

// SINGLE ENTRY FROM ABOVE
// SAVE PAGE IF ENOUGH ROOM LEFT FOR THIS SAVE
FLPT:
// NOT IMPLEMENTED YET!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        DC32    FPADDR, AT
        DC32    LIT, FLASH_PPAGE
        DC32    EQUAL
// DC32 NOOP
        DC32    ZBRAN           // then save it
        DC32     FSAVE-.
        
FSLASTPAGE:
        DC32    CR
        DC32    PDOTQ
        DC8     20D
        DC8     'LAST FLASH AVAIL USED'  // 'NOT ENOUGH MEMORY'
 ALIGNROM 2,0xFFFFFFFF
        DC32    SEMIS

// Setup for saving - Assumes no error in relocation
// Based on relocating UP and Latest to FPVAR and FPCURR
FSAVE:
       // MAKE SURE HERE IS ALIGNED
        DC32    ALIGN32_DP_FF_PAD
// FPA SHOULD BE SET BY FLASH_SCAN IN WARM OR BY FISH OR FISH_ONLY
        // FPCURR AND FPVAR SAVE
	DC32	UP_SV, AT, FPVAR, STORE
	DC32	LATEST, TOFA
//	DC32	LATEST, TOFRA ALL RELOCS NEED TO ADD TO BASE!
        DC32    FPCURR, STORE

// KEEP RELOCATING ONE WORD AT A TIME UNTIL RECHING FIRST WORD (DBASE)
FS_BEGIN:
	DC32	LATEST, DBASE, EQUAL
        DC32	ZBRAN
        DC32     FS_RWORD-.

// AFTER LAST WORD RELOCATED
FS_WRITE:
	DC32	FLASHWRITE                      // Write page
	DC32	DBASE, DP_SV, STORE             // Reset DP
	DC32	FPVAR, AT, UP_SV, STORE         // and UP
	DC32	FPCURR, AT, CURRENT_SV, STORE   // and CURRENT to nothing in RAM

// FLASH_SAVE Finish
FS_EXIT:
        DC32    FLASH_SCAN      // Show Save done via Flash Page status
        DC32    CR, DOTVARSPACE, DOTDICTSPACE
	DC32	SEMIS

FS_RWORD:
	DC32	LATEST, RWORD   // Relocate this word, unlink it
	DC32	BRAN            // Repeat until LATEST = DBASE
        DC32     FS_BEGIN-.


//:NONAME	FPADDR:	( -- addr ) Flash Page Addreess to operate on.
 SECTION .text : CONST (2)
FPADDR:
	DC32	DOCON, FPA


//:NONAME	FPCURR:	( -- addr ) Flash Page CURRENT is NFA of last word saved in Flash.
 SECTION .text : CONST (2)
FPCURR:
	DC32	DOCON, FPC


//:NONAME	FPVAR:	( -- addr ) Flash Page VAR is UP value when Flash page saved.
 SECTION .text : CONST (2)
FPVAR:
	DC32	DOCON, FPSV

//:NONAME >FA TOFA:	( addr -- raddr ) Relocate address relative to FPADDR.
//	Convert WORD addresses to flash using FPADDR, which must be initialized.
 SECTION .text : CONST (2)
TOFA:
	DC32	.+5
 SECTION .text : CODE (2)
	POP2t
	LDR	w, = 0x0FFFFF   // 08100000 FLASH ADDR lIMIT
	ANDS	t, t, w         // Strip RAM address
	LDR	w, = FPA
	LDR	w, [w]          // Get Flash save start address
	ADDS	t, t, w         // add offset
	TPUSH
 LTORG	 //Always outside of code, else data in words


//:NONAME RWORD:	( nfa -- ) Relocate this word relative to FPADDR
//	Relocate word specified by it's NFA to FPADDR relative addresses
//	Handle case of CON/VAR, PDOTQ and LIT by not relocating them.
//      RLIT's WILL BE RELOCATED - LIT's WILL NOT.
 SECTION .text : CONST (2)
RWORD:
	DC32	DOCOL
	DC32	PFA, DUP, TOR	// SAVE FOR RELOCATING LFA
	DC32	CFA             // REAL CFA OF THSI WORD
// CHECK FOR DOCON AND DOVAR
        DC32    DUP, AT
	DC32	LIT, DOCON
	DC32	EQUAL

        DC32    OVER, AT        // COPY CFA AGAIN
	DC32	LIT, DOVAR
	DC32	EQUAL
        
        DC32    OR
        DC32	ZBRAN           // IF NOT DOCON OR DOVAR
        DC32     RW_BEGIN-.	// PROCESS WORD UNTIL SEMIS
// IS DOCON OR DOVAR
	DC32	DROP            // CFA
	DC32	BRAN
        DC32     RW_LFA-.	// THIS WORD DONE - FIX LFA

// CFA -- BECOMES EACH WORD IN BODY OF THIS DEFINITION
RW_BEGIN:
	DC32	FOURP, DUP, AT	// Next WORD in body to evaluate
CFA_LIT:
	DC32	LIT, LIT, EQUAL
	DC32	ZBRAN           // IF NOT LITERAL
        DC32     CFA_PDOTQ-.    // CHECK FOR PDOTQ
// IS LITERAL SO SKIP OVER IT'S DATA
	DC32	FOURP
	DC32	BRAN
        DC32     RW_BEGIN-.

CFA_PDOTQ:
	DC32	DUP, AT
	DC32	LIT, PDOTQ, EQUAL
	DC32	ZBRAN           // IF NOT PDOTQ
        DC32     CFA_SEMIS-.    // LOOK FOR SEMIS
// PDOTQ SO BYPASS TEXT AND LOOK FOR NEXT WORD IN THIS DEFINITION
	DC32	DUP, FOURP, CAT
	DC32	PLUS
	DC32	ALIGNED
	DC32	BRAN
        DC32     RW_BEGIN-.

CFA_SEMIS:
	DC32	DUP, AT         // USE THIS AS UNTIL FLAG TO FINISH
	DC32	LIT, SEMIS, EQUAL
	DC32	ZEQU, ZBRAN     // IF AT SEMIS
        DC32     RW_UNTIL-.     // DONE SO GO FIX LFA

RELOCATE_RAM_ADDR:
// NOT AT SEMIS - IF A RAM ADDRESS THEN IT's OK TO RELOCATE!
	DC32	DUP, AT         // IF THIS IS NOT IN RAM
	DC32	DBASE, GREATERTHAN
	DC32	ZBRAN           // THEN CHECK NEXT WORD IN THIS DEFINITION
        DC32     RW_BEGIN-.
// THIS IS A RAM ADDRESS THAT NEEDS TO BE RELOCATED TO A FLASH ADDRESS
// LIT's were left alone but RLIT value will be relocated here
	DC32	DUP, DUP, AT    // PICKUP RAM ADDRESS
	DC32	TOFA            // RELOCATE THIS ADDRESS TO A FLASH ADDRESS
        DC32    SWAP, STORE     // PUT IT BACK
	DC32	BRAN
        DC32     RW_BEGIN-.	// THEN CHECK NEXT WORD IN THIS DEFINITION

RW_UNTIL:
// ADDRESS OF SEMI IN THIS DEFINITION
	DC32	DROP            // COME FROM SEMIS MATCH
RW_LFA:	// RELOCATE LFA AND SET CURRENT/LATEST TO Next WORD
	DC32	RFROM           // PFA OF THIS DEFINITION
	DC32	LFA, DUP, AT    // GET ADDR OF LFA AND NFA IT POINTS TO
	DC32	DUP             // AND STORE NEXT NFA TO
	DC32	CURRENT_SV, STORE       // CURRENT
	DC32	TOFA, SWAP, STORE       // AND THIS DEF'S LFA
	DC32	SEMIS


//------------------------------------------------------------------------------
//:NONAME FLASH_SCAN:       ( -- ) SCAN FLASH FOR SAVED CODE
//      Patch in FLASH_SAVE'd Code.
 SECTION .text : CONST (2)
FLASH_SCAN:
	DC32	DOCOL

// FLASH_PPAGE can be reserved and Hard_fault if accessed!
        DC32    LIT, FLASH_SPAGE        // Start at first page available
CHECK_NEXT_FLASH_SAVE:
        DC32    DUP, LIT, FPA, STORE
        DC32    DUP, TOR                // EACH SAVE
        DC32    AT                      // Verify SYSCLK signature
        DC32    SYSCLK
        DC32    SUBB                    // ZBRAN IF SYSCLK FOUND
        DC32    ZBRAN                   //  = HEADER OF SAVED CODE
        DC32     LINK_FLASHCODE_SEGMENT-.

// NO CODE FOUND TO LINK EXIT
        DC32    RFROM, DROP, SEMIS

LINK_FLASHCODE_SEGMENT:
//      PAGE PASSES SYSCLK VERIFICATION, LINK IT
        DC32    R, FPADDR, STORE
        DC32    R, LIT, 8, PLUS         // FPCURR
        DC32    DUP, AT
        DC32    CURRENT_SV              // TO DP
        DC32    STORE
        DC32    FOURP, AT               // FPVAR
        DC32    UP_SV, STORE            // TO UP

        DC32    CR
        DC32    PDOTQ
        DC8     23D
        DC8     'ADDED WORDS FROM ADDR '
 ALIGNROM 2,0xFFFFFFFF
        DC32    R, DOTHEX

        DC32    RFROM, FOURP, AT        // NEXT FPADDR TO CHECK FOR CODE!
        DC32    BRAN
        DC32     CHECK_NEXT_FLASH_SAVE-.


//:NONAME ASSUREPAGE: ( fpa -- next-fpa) Verify this Flash page has code or is empty.
//      Make sure PAGE IN fpa page is erased and let user know what you do.
//      NOT TESTED YET: erase this page and tell user<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 2 STACK ITEMS LEFT ON EXIT, BOTH ARE fpa+1000H LIKE WHATS PUT ON r<<<<<<<<<<<

 SECTION .text : CONST (2)
ASSUREPAGE:
        DC32    DOCOL                   // verified ONLY fpa ON STACK Here
        DC32    DUP                     // -- fpa fpa
//        DC32    LIT, FLASH_WR_SIZE      // PAGE SIZE WERE WRITING
        DC32    PLUS
        DC32    TOR                     // End of this page
        DC32    DUP                     // Begin -- fpa fpa

APNEXT:
        DC32    AT                      // Should be -1 -- fpa
        DC32    ONEP                    // if so
        DC32    ZBRAN                   // -- addr 0|BADFLASHVALUE
        DC32     APCONT-.               // keep going

//      ERASE THIS PAGE                 // -- fpa
//        DC32    FLASHPREP
//        DC32    FLASHESEC
        DC32    CR
        DC32    PDOTQ
        DC8     23D
        DC8     'ERASED BAD FLASH PAGE  '
 ALIGNROM 2,0xFFFFFFFF
        DC32    DOTHEX

//        DC32    CR, DOTS // DEBUG
        DC32    DROP

        DC32    RFROM, DROP
        DC32    SEMIS

//      SO FAR SO GOOD
APCONT:                                 // -- addr
        DC32    FOURP, DUP              // -- addr addr
        DC32    DUP, R, SWAP            // -- addr  r addr
        DC32    SUBB                    // End of this Page?
        DC32    ZBRAN                   // -- addr f
        DC32     APGOOD-.

        DC32    BRAN
        DC32     APNEXT-.

APGOOD:                                 // addr
        DC32    CR
        DC32    PDOTQ
        DC8     14D
        DC8     'FLASH PAGE OK '
 ALIGNROM 2,0xFFFFFFFF
//        DC32    DROP, LIT, FLASH_WR_SIZE      // PAGE SIZE WERE WRITING
        DC32    SUBB
        DC32    DOTHEX

APDONE:
        DC32    RFROM, DROP
        DC32    SEMIS

//------------------------------------------------------------------------------
//	FLASH_FORGET FLASH_FORGET:	( --  ) Erase All User Flash Pages

 SECTION .text : CONST (2)
FLASH_FORGET_NFA:
	DC8	0x80+12
	DC8	'FLASH_FORGE'
	DC8	'T'+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	FLASH_SAVE_NFA
FLASH_FORGET:
        DC32    DOCOL
        DC32    PDOTQ
        DC8     23D
        DC8     'TAKES ABOUT 6 SECONDS! '
 ALIGNROM 2,0xFFFFFFFF
        DC32    CMSIS_DISABLE_IRQS
        DC32    FLASH_UNLOCK
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
// Erase sector at a time
// ?Break this out to a primitive looping from sector 4/5 to 11
// 4 Words / 32 bytes per sector erase - 7 = 28 words / 112 bytes
// Sector 4 is 64k, sector 0-3 are 16k, together they equal 1rst 128k of flash.
// 5 to 11 are the 7 other 128k sectors that amount to the full 1mb of flash.

// 01 is start 02 is x32 2 is 4th sector 2 is sector erase
// 5 3 LSL 2 + .H 2Ah  ok, go fish in BASE 10d
// 6 3 LSL 2 + .H 32h  ok, go fish in BASE 10d
// 7 3 LSL 2 + .H 3Ah  ok, go fish in BASE 10d
// 8 3 LSL 2 + .H 42h  ok, go fish in BASE 10d

//        DC32    strva, 010222h, FLASH_CR        // erase sector 4 0x08010000h
//        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    strva, 01022Ah, FLASH_CR        // erase sector 5 0x08020000h
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    strva, 010232h, FLASH_CR        // erase sector 6 0x08040000h
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    strva, 01023Ah, FLASH_CR        // erase sector 7 0x08060000h
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    strva, 010242h, FLASH_CR        // erase sector 8 0x08080000h
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    strva, 01024Ah, FLASH_CR        // erase sector 9 0x080A0000h
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    strva, 010252h, FLASH_CR        // erase sector 10 0x080C0000h
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    strva, 01025Ah, FLASH_CR        // erase sector 11 0x080E0000h
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
// Lock Flash again
        DC32    FLASH_LOCK   // Top bit = Lock
        DC32    CMSIS_ENABLE_IRQS
// TEMP EXIT UNTIL the rest of this is fixed
//        DC32	SEMIS



//      RESET DICTIONARY POINTERS APPROPRIATELY 
FF_FINISH:
// IF NO WORDS IN RAM TEST
	DC32	RBASE, LATEST   // If Latest is GT RBASE ( Latest is in ram )
        DC32    GREATERTHAN     // This will be zero
        DC32    ZBRAN           // Meaning there are
        DC32     RAMWORDS-.     // words in Ram
// RESET ALL POINTERS
        DC32    FISH_ONLY       // No words so clean up
// RESET FPA
        DC32    BRAN
        DC32      FFDONE-.

//----------------------------- STM32F4 ----------------------------------------
// Common words for each Flash function - Save, Forget, etc.
// Flash TEST CODE relocation primitive:
// ASM CODE IS RELATIVE!!
// FISH CODE IS NOT!!! A DIFF LEVEL OF RWORD WOULD BE REQUIRED!!!
// THANK GOD FLASH CODE CAN EXECUTE IN FLASH AS LONG AS OP'S TARGET DIFF SECTOR!
// WORDS IN RAM NEED TO BE RELINKED TO FLASH
RAMWORDS:
//        DC32    LIT, TASK_NFA         // Fish WITH TASKER IN RAM starts here
#ifdef FISH_PubRel_WORDSET
        DC32    LIT, WC_FISH_PubRel_NFA // FISH in flash starts here
#endif
#ifdef FISH_NXP_M0_PRO_WORDCAT
        DC32    LIT, WC_FISH_PRO_NFA    // OR HERE
#endif
        DC32    DBASE, PFA, LFA         // RELINK THIS lfa, i.e.
        DC32    STORE                   // repatch to Fish in flash

// FLASH_FORGET Finish
FFDONE:
        DC32    strva, FLASH_SPAGE, FPA 
        DC32    LIT, 11111111h, DUP, FPCURR, STORE, FPVAR, STORE
// Show results
        DC32    FLASH_SCAN      // Show Erase done via Flash Page status
        DC32    CR, DOTVARSPACE, DOTDICTSPACE
        DC32	SEMIS

//--------------------- Flash code copied to ram test --------------------------

//:NONAME FLASH_TCODE_COPY ( End Start -- ) Copy Flash code to ram
 SECTION .text : CONST (2)
FLASH_TCODE_COPY:
	DC32 .+5
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
        POP2t   // LDR     t, =FLASH_TCODE_START
        POP2n   // LDR     n, =FLASH_TCODE_END
        LDR     w, =FLASH_CODE_SRAM2_START
FLASH_COPY_LOOP:
        LDR     y, [t], #4
        STR     y, [w], #4
        cmp     t, n
        BNE     FLASH_COPY_LOOP
        NEXT

// Each one will use FLASH_RAMCODE_EXEC after copying it's code to ram..
//:NONAME FLASH_RAMCODE_EXEC ( -- )
 SECTION .text : CONST (2)
FLASH_RAMCODE_EXEC:
        DC32    LIT, FLASH_CODE_SRAM2_START
        DC32    EXEC
        DC32    SEMIS

// Prototype for each function
//:NONAME FLASH_TEST_RAMCODE ( -- )
 SECTION .text : CONST (2)
FLASH_TEST_CODE_IN_RAM:
        DC32    DOCOL
        DC32    LIT, FLASH_TCODE_END
        DC32    LIT, FLASH_TCODE_START
        DC32    FLASH_TCODE_COPY
	DC32	FLASH_RAMCODE_EXEC
        DC32    SEMIS

//:NONAME FLASHWRITE:	( --  ) Save Ram to Flash using FPADDR and valid HERE.
//	Header need to be completed at this point.
//      FPADDR needs to be Flash HERE, where the next save goes.
//      But current FPADDR must be uused for save
 SECTION .text : CONST (2)
FLASHWRITE:
	DC32	DOCOL
/* THIS WAS ADDED BECAUSE 2ND SAVE BOMBED WITH FPCURR -> HEADER NOT DBASE START
// SET FPCURR TO NEW LATEST
        DC32    FPADDR, AT
        DC32    LIT, 16, PLUS
        DC32    FPCURR, STORE
 DC32 NOOP
 */
        DC32    CMSIS_DISABLE_IRQS
        DC32    FLASH_UNLOCK
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
// Set PSIZE to x32 and the Program Start bit
        DC32    strva,  0201h, FLASH_CR // x32 & start programming
// SAVE FPA AS HEADER ADDR
        DC32    FPADDR, AT, TOR         /// SAVE FPA HEADER ADDR
// BUMP FPA TO DICT OFFSET
        DC32    LIT, 16, FPADDR, PSTORE  // SET FPA PAST HEADER
// COPY DBASE TO HERE TO FLASH
// AFTER RWORD IS HERE VALID????
	DC32	HERE, TOR		// length of save
	DC32	DBASE
//        DC32    RBASE
FLASH_WRITE_LOOP:
        DC32    DUP, AT                 // GET DICT
	DC32	FPADDR, AT              // FLASH
        DC32    STORE, FLASH_NOT_BSY    // WRITE & VERIFY NOT BUSY
        DC32    FOUR, FPADDR, PSTORE    // INC FPA
        DC32    FOURP                   // INC DICT
	DC32	DUP, R, SUBB            // EQUAL HERE YET?
	DC32	ZBRAN
        DC32     FLASH_WRITE_EXIT_LOOP-.
        DC32    BRAN
        DC32     FLASH_WRITE_LOOP-.
FLASH_WRITE_EXIT_LOOP:
        DC32    RFROM
        DC32    DROP, DROP     // THE FPADDR AND HERE LIMIT

// WRITE HEADER WITH ORIGINAL FPA ( 4 WORDS )
// NEW FPA SET HERE - DOES IT NEED 4+ BUMP HERE?
// FPVAR AND FPCURR SET IN CALLING FUNCTION - FLASH_SAVE
// DEFINE SRBASE        DC32    SRBASE, AT, R           // WRITE SYSCLK
        DC32    LIT, SRAM1, AT, R        // GET SYSCLK AND
        DC32    STORE, FLASH_NOT_BSY    // WRITE & VERIFY NOT BUSY
        DC32    FPADDR, AT              // NEW FPADDR
        DC32    R, FOURP
        DC32    STORE, FLASH_NOT_BSY    // WRITE & VERIFY NOT BUSY
        DC32    FPCURR, AT              // HAVE TO MAKE FPCURR
//        DC32    LIT, 16, PLUS           // PAST HEADER
        DC32    R, EIGHT, PLUS
        DC32    STORE, FLASH_NOT_BSY    // WRITE & VERIFY NOT BUSY
        DC32    FPVAR, AT, RFROM, LIT, 12, PLUS
        DC32    STORE, FLASH_NOT_BSY    // WRITE & VERIFY NOT BUSY
FLASH_WRITE_FINISH:
        DC32    FLASH_LOCK   // Top bit = Lock
        DC32    CMSIS_ENABLE_IRQS
	DC32	SEMIS

//--------------------- Flash code copied to ram test --------------------------
FLASH_TCODE_START:
// WORDS and CODE to evaluate for copying to RAM ( for each flash function ).

//:NONAME FLASH_TEST_CODE_IN_FLASH: ( -- )
 SECTION .text : CONST (2)
FLASH_TEST_CODE_IN_FLASH:
        DC32    DOCOL
        DC32    FLASH_FORGET    // Atomic Primitive: erase all user sectors
        DC32    CMSIS_DISABLE_IRQS
        DC32    FLASH_UNLOCK
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
// Set PSIZE to x32 and the Program Start bit
        DC32    strva,  0201h, FLASH_CR // x32 & start programming
        DC32    strva, 08020000h, FPA
        DC32    LIT, 16, DP_SV, PSTORE
        DC32    NOOP
        DC32    FLASHWRITE
        DC32    NOOP
        DC32    FLASH_FORGET    // Atomic Primitive: erase all user sectors
/* Write and erase test
// User Code Flash Save starts in sector 5, the 1rst of 7 128k sectors
        DC32    strva,  -3, 08020000h   // The beginning of sector 5
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    strva,  -4, 08040000h   // The beginning of sector 6
        DC32    FLASH_NOT_BSY   // VERIFY NOT BUSY
        DC32    NOOP
        DC32    FLASH_FORGET    // Atomic Primitive: erase all user sectors
*/
        DC32    FLASH_LOCK   // Top bit = Lock
        DC32    CMSIS_ENABLE_IRQS
// Lock Flash again
        DC32    SEMIS


 SECTION .text : CONST (2)
FLASH_NOT_BSY:
	DC32 .+5
 SECTION .text : CODE (2)
 ALIGNROM 2,0xFFFFFFFF
        LDR     n, =FLASH_SR    // 40023C00h is FLASH CTRL 40023C0Ch is FLASH_SR
_WaitBsy10:
        LDR     t, [n]
        LSLS    t, t, #16       // BSY - Bit 16
        BCS     _WaitBsy10
        NEXT
// Commit literal constants to pool to be copied!
 LTORG

//:NONAME FLASH_UNLOCK ( -- ) // Unlock Flash Programming Functionality
 SECTION .text : CONST (2)
FLASH_UNLOCK:
        DC32    DOCOL
        DC32    strva,  045670123h, FLASH_KEYR // Key1
        DC32    strva,  0CDEF89ABh, FLASH_KEYR // Key2
        DC32    SEMIS

//:NONAME FLASH_LOCK ( -- ) // Lock Flash Programming Functionality
 SECTION .text : CONST (2)
FLASH_LOCK:
        DC32    DOCOL
//      SETBITS SETBITS:	( addr val -- ) OR val bits into addr.
//        DC32    LIT, FLASH_CR, LIT, 080000000h, SETBITS // Preserve other bits
        DC32    strva,   080000000h, FLASH_CR   // Top bit = Lock
        DC32    SEMIS

FLASH_TCODE_END:

