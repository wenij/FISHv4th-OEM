// FISH_GPIO.h

// THIS IS BEING REMOVED
// FISH_RM_CONFIG_#DEFINES.h
// FISH_STM32F4_GPIO_WC is the #define

/* Define TOP of Dictionary:  
The top of the dictionary is defined in these places
In SLIBS SV_INIT_VALUES:
//        DC32    LIT, WC_FISH_PubRel_NFA // FISH in flash starts here
        DC32    LIT, WC_FISH_GPIO_NFA // FISH in flash starts here
IN FLASH RAMWORDS:
//        DC32    LIT, WC_FISH_PubRel_NFA // FISH in flash starts here
        DC32    LIT, WC_FISH_GPIO_NFA // FISH in flash starts here
In IAR FISH_ONLY:
//        LDR     n, = WC_FISH_PubRel_NFA
        LDR     n, = WC_FISH_GPIO_NFA

*/

// Conditional used in FISH_STM32F4_IAR.s to modify last dictionary link
// to include this wordcat.

//=============================== WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_FISH_SYS: = FISH System: CATEGORY

 SECTION .text : CONST (2)
WC_FISH_GPIO_NFA:
	DC8	0x80+4+5        // +4 is format chars constant
                                // +n is Name lenght
        DC8     0x0D, 0x0A
	DC8	'GPIO:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
	DC32	WC_FISH_PubRel_NFA        // Where this wordcat links in.


