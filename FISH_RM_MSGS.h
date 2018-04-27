//------------------------------------------------------------------------------
// Valid thru v1.8
 SECTION .text : CONST (2)

// DC8 "?" IS A NULL TERMINATED STRING
// DC8 '?' IS NOT
// #define EOL_DLE // DLE 0x10/16d in QUIT>CR after last word interpreted.
// #define EOL_NAK // NAK 0x15/21d in error messages for STDLED editor highlight
// The EOL_NAK makes SPACE NAK end of string in FISH_RM_MSGS.h

 ALIGNROM 2,0xFFFFFFFF
// ALIGN THIS ONE FOR PFIND          
msg_RUN:
        DC8     3
        DC8     "RUN "
msg_questionmark:
	DC8     " ? "

msg_cr:
	DC8     " <CR> "

msg_uvspace:
	DC8     "VAR CELLS "

msg_dictspace:
	DC8     "DICTIONARY BYTES "

msg_paren_err:
#ifdef EOL_NAK
        DC8     ' COMMENT LINE CANNOT BE EMPTY '
        DC8     0x15, 0
#else
        DC8     " COMMENT LINE CANNOT BE EMPTY "
#endif
          
msg_forget_fish:
#ifdef EOL_NAK
        DC8     ' CANT FORGET FISH IN FLASH '
        DC8     0x15, 0
#else
        DC8     " CANT FORGET FISH IN FLASH "
#endif

msg_forget_saved:
#ifdef EOL_NAK
        DC8     ' IN FLASH - SEE FLASH_FORGET '
        DC8     0x15, 0
#else
        DC8     " IN FLASH - SEE FLASH_FORGET "
#endif

msg_dictfull:
#ifdef EOL_NAK
	DC8     ' DICTIONARY FULL - SEE .DS '
        DC8     0x15, 0
#else
	DC8     " DICTIONARY FULL - SEE .DS "
#endif

msg_uvfull:
#ifdef EOL_NAK
	DC8     ' RAM VAR SPACE FULL - SEE .VS '
        DC8     0x15, 0
#else
	DC8     " RAM VAR SPACE FULL - SEE .VS "
#endif

msg_wordexists:
#ifdef EOL_NAK
	DC8     ' IS ALREADY DEFINED '
        DC8     0x15, 0
#else
	DC8     " IS ALREADY DEFINED "
#endif

msg_word_error:
#ifdef EOL_NAK
        DC8     ' ? NOT WORD '
        DC8     0x15, 0
#else
        DC8     " ? NOT WORD "
#endif

msg_number_error:
#ifdef EOL_NAK
	DC8     ' ? NOT WORD OR NUMBER '
        DC8     0x15, 0
#else
	DC8     " ? NOT WORD OR NUMBER "
#endif

msg_qstack:
#ifdef EOL_NAK
	DC8     ' CAUSED A STACK ERROR '
        DC8     0x15, 0
#else
	DC8     " CAUSED A STACK ERROR "
#endif

msg_qpair:
#ifdef EOL_NAK
	DC8     ' STACK ERROR WHILE EXECUTING OR COMPILING '
        DC8     0x15, 0
#else
	DC8     " STACK ERROR WHILE EXECUTING OR COMPILING "
#endif

msg_qcomp:
#ifdef EOL_NAK
	DC8     ' ERROR - NOT COMPILING '
        DC8     0x15, 0
#else
	DC8     " ERROR - NOT COMPILING "
#endif

msg_qexec:
#ifdef EOL_NAK
	DC8     ' ERROR - EXECUTED WHILE COMPILING '
        DC8     0x15, 0
#else
	DC8     " ERROR - EXECUTED WHILE COMPILING "
#endif

