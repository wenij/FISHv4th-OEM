// FISH_GPIO.h
// Valid in v1.8
// FISH_RM_CONFIG_#DEFINES.h
// FISH_STM32F4_GPIO_WC is the #define


// This has to be defined in FISH_STM32F4_FLASH.s
// #ifdef FISH_PubRel_WORDSET
//        DC32    LIT, WC_FISH_PubRel_NFA // FISH in flash starts here
// #endif


// Conditional used in FISH_STM32F4_IAR.s to modify last dictionary link
// to include this wordcat.

//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_SYS: = FISH System: CATEGORY

 SECTION .text : CONST (2)
WC_FISH_GPIO:
	DC8	0x80+4+5        // +4 is format chars constant
                                // +n is Name lenght
        DC8     0x0D, 0x0A
	DC8	'GPIO:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_FISH_PubRel_NFA        // Where this wordcat links in.


