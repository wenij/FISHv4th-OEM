// Valid only on v1.8 onward
//------------------------------------------------------------------------------

 SECTION .text : CONST (2)

// DC8 "?" IS A NULL TERMINATED STRING
// DC8 '?' IS NOT

 ALIGNROM 2,0xFFFFFFFF

// IF this fits in 1114 move to FISH_RM_MSGS.h 

msg_CO:
	DC8     " Error Halt (EHON/EHOFF) - To COntinue Type CO then Enter: "

