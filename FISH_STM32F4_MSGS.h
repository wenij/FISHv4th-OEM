//------------------------------------------------------------------------------

 SECTION .text : CONST (2)

// DC8 "?" IS A NULL TERMINATED STRING
// DC8 '?' IS NOT

 ALIGNROM 2,0xFFFFFFFF
// ALIGN THIS ONE FOR PFIND          
msg_RUN:
        DC8     3
        DC8     "RUN "
msg_questionmark:
	DC8     " ? "

msg_cr:
	DC8     " <CR> "

msg_paren_err:
        DC8     " COMMENT LINE CANNOT BE EMPTY "
//        DC8     ' COMMENT LINE CANNOT BE EMPTY '
//        DC8     0xA, 0xD, 0

msg_forget_fish:
        DC8     " CANT FORGET FISH IN FLASH "

msg_forget_saved:
        DC8     " IN FLASH - SEE FLASH_FORGET "

msg_uvspace:
	DC8     "VAR CELLS "

msg_dictspace:
	DC8     "DICTIONARY BYTES "

msg_dictfull:
	DC8     " DICTIONARY FULL - SEE .DS "

msg_uvfull:
	DC8     " RAM VAR SPACE FULL - SEE .VS "

msg_wordexists:
	DC8     " IS ALREADY DEFINED "

msg_word_error:
        DC8     " ? NOT WORD "

msg_number_error:
	DC8     " ? NOT WORD OR NUMBER "

msg_qstack:
	DC8     " CAUSED A STACK ERROR "

msg_qpair:
	DC8     " STACK ERROR WHILE EXECUTING OR COMPILING "

msg_qcomp:
	DC8     " ERROR - NOT COMPILING "

msg_qexec:
	DC8     " ERROR - EXECUTED WHILE COMPILING "

