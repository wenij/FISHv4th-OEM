// FISH_NXP_M0_PRO_WORDSET.s
// In 11xx PRO Configuration Assembler Preprocessor Defines section:
// FISH_NXP_M0_PRO_WORDCAT  // Set CURRENT to WC_FISH_PRO_NFA
// Use defines to break out 11xx and 81x Wordset versions

//$FISH_STM32F4_PR0_CPUID.s

//=============================== I2C WORDCAT ====================================//

// >SLA TOSLA:  ( 1or0 mfg-sla -- sla _
//      Create a read (1) or write (0) I2c slave address
//      from the manufacturer raw slave address.

 SECTION .text : CONST (2)
TOSLA_NFA:
	DC8	0x84
	DC8	'>SL'
	DC8	'A'+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    WC_FISH_PubRel_NFA        // So far must be 1rst PRO WORSET
TOSLA:
	DC32	DOCOL
        DC32    TWOSTAR
        DC32    PLUS
	DC32	SEMIS

// Link Rest of Wordset between HERE


// and HERE

//=============================== I2C WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_I2C_PRO: = FISH I2C SYSTEM CATEGORY
 SECTION .text : CONST (2)
WC_FISH_I2C_PRO_NFA:
	DC8	0x80+4+13
        DC8     0x0D, 0x0A
	DC8	'FISH I2C PRO:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    TOSLA_NFA


//=============================== PRO WORDCAT ====================================//
//NOEXEC HEADERFORWORDCATEGORIES
//	WC_PRO: = FISH PRO WORDSET CATEGORY
 SECTION .text : CONST (2)
WC_FISH_PRO_NFA:
	DC8	0x80+4+14
        DC8     0x0D, 0x0A
	DC8	'FISH PRO WORDSET:'
        DC8     0x0D, 0x0A+0x80
 ALIGNROM 2,0xFFFFFFFF
        DC32    WC_FISH_I2C_PRO_NFA
