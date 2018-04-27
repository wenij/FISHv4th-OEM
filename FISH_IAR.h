//------------------------------------------------------------------------------
// Valid thru v1.8
  COL 132           // required for dis-assembly output sanity
// M0 ISR:
    // Pretty much the same as STM except:
    // intvec placement in ram has to be done thru memory remap.
    // saving of hi registers convoluted because only r0-r5 savable by PUSH.
    // Not saving upper regs (r10-12) worked but shouldnt so they will be saved.
    // use of c coroutines us same stack as isr.

// PUSH in ISR uses cstack so:
    // C_CMSIS_DISABLE_IRQS and C_CMSIS_ENABLE_IRQS wrapper used in:
    // c_64by32div      :NONAME USLASH:
    // c_32by32to64mul  :NONAME USTAR:

// Next step is remap IntVecs to RAM. No VTOR!
// Cortex-M0 (e.g. NXP LPC11xx) has a memory remap feature on its memory system
// (not a part of the processor) to allow vector table accesses to be optionally redirected to the SRAM.
// On a Cortex-M0 (STM32F0) part, you also can't change the vector table, and the trick there is to either // map FLASH at zero, or copy a small vector table into RAM, and then map RAM at zero.

// Test:
// : .ST BEGIN STCTR ? ?KEY UNTIL ;
    // .ST skipping 8 on average

// 0x9D8: 3f b5 50 46 59 46 62 46 07 b4 01 4a 08 ca 98 47 00 0a 00 00 f1 09 00 00 35 68 36 1d 07 bc 82 46 8b 46 94 46 3f bd 00 00 01 06 00 00 18 22 00 00 98 35 00 00 14 19 00 00 ec 09 00 00
/*
DOCOL_ISR:
FM0_SYSTICK_ISR:
        0x9d8: 0xb53f         PUSH      {R0-R5, LR}
        MOV     r0, r10
        0x9da: 0x4650         MOV       R0, R10
        MOV     r1, r11
        0x9dc: 0x4659         MOV       R1, R11
        MOV     r2, r12
        0x9de: 0x4662         MOV       R2, R12
        PUSH    {r0-r2}
        0x9e0: 0xb407         PUSH      {R0-R2}
        LDR     w, [PC, #0X4] //= MY_LTORG The High Level Target
        0x9e2: 0x4a01         LDR.N     R2, [PC, #0x4]          ; [0x9e8] NONAME_STCTR_INCR
    NEXT1   // -> SEMIS_ISR RETURN required instead of SEMIS!
        0x9e4: 0xca08         LDM       R2!, {R3}
    NEXT1   // -> SEMIS_ISR RETURN required instead of SEMIS!
        0x9e6: 0x4798         BLX       R3
        0x9e8: 0x00000a00     DC32      NONAME_STCTR_INCR
MY_LTORG_ISR_SEMIS:
        0x9ec: 0x000009f1     DC32      2545                    ; 'ñ ..'
    POPr2i  // SEMIS to balance DOCOL!!!
        0x9f0: 0x6835         LDR       R5, [R6]
    POPr2i  // SEMIS to balance DOCOL!!!
        0x9f2: 0x1d36         ADDS      R6, R6, #4
        POP     {r0-r2}
        0x9f4: 0xbc07         POP       {R0-R2}
        MOV     r10, r0
        0x9f6: 0x4682         MOV       R10, R0
        MOV     r11, r1
        0x9f8: 0x468b         MOV       R11, R1
        MOV     r12, r2
        0x9fa: 0x4694         MOV       R12, R2
        POP     {r0-r5, pc }
        0x9fc: 0xbd3f         POP       {R0-R5, PC}
        0x9fe: 0x0000         MOVS      R0, R0
NONAME_STCTR_INCR:
        0xa00: 0x00000601     DC32      DOCOL
        0xa04: 0x00002218     DC32      ONE
        0xa08: 0x00003598     DC32      STCTR
        0xa0c: 0x00001914     DC32      PSTORE
        0xa10: 0x000009ec     DC32      MY_LTORG_ISR_SEMIS

*/
/*
Here is one implementation:

void SysTick_Handler(void) {
   msTicks++;
}

void Slow_SysTick_Handler(void) {
   static int slow;

   slow++;
   if((slow%4) == 3) {
      msTicks++;
      }
}
 
/* Declare pFunc_t as a function pointer to a function with no parameters typedef */
//   typedef void (*pFunc_t)(void);

/* Declare a pointer to an array of function pointers that points to RAM */
#define pfRAMVectors ((pFunc_t *)0x10000000)
 
//int main (void) {
 
//...
 
  // Step 1: modify linker script to leave 0x200 bytes of RAM free
  // In LPCXpresso this is done in the projectname_Debug_mem.ld file
  // MEMORY
  // {
  //   /* Define each memory region */
  //   MFlash32 (rx) : ORIGIN = 0x0, LENGTH = 0x8000 /* 32k */
  //   RamLoc8 (rwx) : ORIGIN = 0x10000200, LENGTH = 0x1E00 /* 8k */
  // }
 
  // 2: Copy current vectors to RAM.
  // memcpy( destination, source, length in bytes )
//  memcpy( (void *)0x10000000, (void *)0x00000000, 0x200);

  // 3: Modify the vector table- redirect SysTick interrupt
//  pfRAMVectors[15] = Slow_SysTick_Handler;

  // 4: Switch over to modified vector table
//  LPC_SYSCON->SYSMEMREMAP = 1;
 
//...

/*
    By default, the flash memory is mapped to address 0x0000 0000.
    When the MAP bits in the SYSMEMREMAP register are set to 0x0 or 0x1,
    the boot ROM or RAM respectively are mapped to the bottom 512 bytes
    of the memory map (addresses 0x0000 0000 to 0x0000 0200). 
*/