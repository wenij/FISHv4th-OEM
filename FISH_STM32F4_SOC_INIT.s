//      FISH_STM32F4_SOC_INIT.s
// Valid thru v1.8

// This should only setup FISH I/O, RCC and Systick

// Other initializations should be in their own files:
// like FISH_STM32F4_GPIO_INIT.s , FISH_STM32F4_SPI_INIT.s etc

// ART init stuff
DCEN	EQU	00000400h
ICEN	EQU	00000200h
PRFTEN	EQU	00000100h
LATENCY_5WS	EQU	00000005h
initART:
	DC32	DOCOL, strva, (ICEN | DCEN | PRFTEN | LATENCY_5WS), FLASH_ACR
	DC32	SEMIS


 SECTION .text : CONST (2)
copyintvecs:
	DC32	.+5
 SECTION .text : CODE (2)
	mov32	y, RAM_INTVECS
	mov	x, #0    // BOOT REMAP IN MAIN FLASH ALIASED AT 0
//      mov32   x, = FLASH_START
	mov	k, #128
_copyintvecs
	ldr	w, [x], #4
	str	w, [y], #4
	subs	k, k, #1
	bne	_copyintvecs
	NEXT
 LTORG

//HEADERLESS SoCinit:        ( -- )
//      Initialize main subsystems here.
//      Initialize SYSCLOCK to system clock frequency in Hz.
//      Use of IRC, external xtal's and PLL done here.
//      Initialize SYSTICK only to use system clock.
//      SYSTICK eventually to be initiatlized for tasker IRQ.
//      Enable CLKOUT.

 SECTION .text : CONST (2)
SoCinit:
  DC32	DOCOL
  DC32  copyintvecs
// VTOR for 205 issue fixed in stm32F205RC.icf
// VTOR_PATCH set in 205 config Options Assembly Preprocessor
#ifndef VTOR_PATCH
  DC32  strkk, RAM_INTVECS, VTOR
#endif

#ifdef SLOW_POWERUP
// DELAY SIGNON MESSAGE OUTPUT UNTIL SERIAL BOARD POWERED UP
  DC32    LIT, 500, MS
#endif

//---------------------------XRC HCLK PCLK1 AND PCLK2---------------------------
#if STM32F4_XRC08_168MHZ | STM32F205RC_XRC10_118MHZ 
; 168MHz SYSCLK, 84MHz PCLK2, 42MHz PCLK1 for USB and RNG, 38.4MHz for I2S
  DC32	atk,RCC_CR, ork,1, strk,RCC_CR			; Set HSION bit
  DC32	strva,0,RCC_CFGR				; Reset CFGR register
  DC32	atk,RCC_CR, andk,0FEF6FFFFh, strk,RCC_CR	; Reset HSEON, CSSON and PLLON bits
  DC32	strva,24003010h,RCC_PLLCFGR			; jam PLLCFGR register to PUR value
  DC32	atk,RCC_CR, andk,0FFFBFFFFh, strk,RCC_CR	; Reset HSEBYP bit
  DC32	strva,0,RCC_CIR					; Disable all interrupts

RCC_CR_HSEON	EQU	00010000h

  DC32	atk,RCC_CR, ork,RCC_CR_HSEON, strk,RCC_CR	; Enable HSE 8MHz

RCC_CR_HSERDY	EQU	00020000h
  DC32	begin, atk,RCC_CR, andk,RCC_CR_HSERDY, until	; Wait till HSE is ready

RCC_APB1ENR_PWREN	EQU	10000000h
PWR_CR_PMODE	EQU	4000h

// Enable high performance mode, System frequency up to 168 MHz
  DC32	atk,RCC_APB1ENR, ork, RCC_APB1ENR_PWREN, strk,RCC_APB1ENR
  DC32	atk,PWR_CR, ork,PWR_CR_PMODE, strk,PWR_CR

RCC_CFGR_HPRE_DIV1	EQU	00000000h
RCC_CFGR_PPRE2_DIV2	EQU	00008000h
RCC_CFGR_PPRE1_DIV4	EQU	00001400h

// DEFINE OTHER SYSTEM CLOCKS FOR USER AND BAUD CALC
// PCLK2 = HCLK / 2 ; PCLK1 = HCLK / 4
// PCLK2 = USB PCLK1 = UART
  DC32	LIT, RCC_CFGR, LIT, RCC_CFGR_HPRE_DIV1, SETBITS		; HCLK = SYSCLK / 1
  DC32	LIT, RCC_CFGR, LIT, RCC_CFGR_PPRE2_DIV2, SETBITS	; PCLK2 = HCLK / 2
  DC32	LIT, RCC_CFGR, LIT, RCC_CFGR_PPRE1_DIV4, SETBITS	; PCLK1 = HCLK / 4
PCLK2   EQU     SYSTEMCLOCK / 2
PCLK1   EQU     SYSTEMCLOCK / 4
//STM32F205RC_XRC10_118MHZ
//PCLK2   EQU     59000000        // 38444C0h
//PCLK1   EQU     29500000        // 1C22260h
// STM32F4_XRC08_168MHZ
//PCLK2   EQU     84000000        // 501BD00h
//PCLK1   EQU     42000000        // 280DE80h


/* VIC: OK, I figured out what I was trying to do... 't'was a failed experiment.
"Magic" 157 MHz won't work (even with a different xtal)
because it would bugger-up the USB, which *needs* 48 MHz.
Right now WITH THESE PLL SETTINGS the clock is in fact 144 MHz.
PLL_Mbits	EQU	4;8	; 8MHz/4 = 2MHz
PLL_N		EQU	144;157;336
PLL_Nbits	EQU	(PLL_N << 6)
PLL_P		EQU	2
PLL_Pbits	EQU	(((PLL_P >> 1) -1) << 16)
PLL_Q		EQU	6;7
PLL_Qbits	EQU	(PLL_Q << 24)
*/

// This is for 168MHz HCLK | SYSCLK PCLK2 = 134MHz PCLK1 = 42MHz
// pROTOTYPING STP RPM 205 CLOCK
#ifdef STM32F205RC_XRC10_118MHZ

//PLL_Mbits       EQU     5       ; 10MHz/5 = 2MHz
//PLL_N           EQU     118     ; vco = (2 MHz * 118) = 236 MHz
// STMCUBE says 10 * 236
PLL_Mbits       EQU     10      ; 10MHz/10 = 1MHz
PLL_N           EQU     236     ; vco = (1 MHz * 236) = 236 MHz
PLL_Nbits       EQU     (PLL_N << 6)
PLL_P           EQU     2       ; 236/2 = 118 MHz
PLL_Pbits       EQU     (((PLL_P >> 1) -1) << 16)
PLL_Q           EQU     5       ; 236/5 = 47.2 MHz ( 48 MHz req'd? ) for USB
PLL_Qbits       EQU     (PLL_Q << 24)
#endif
#ifdef STM32F4_XRC08_168MHZ
PLL_Mbits       EQU     8       ; 8MHz/8 = 1MHz
PLL_N           EQU     336     ; vco = (1 MHz * 336) = 336 MHz
//or
//PLL_Mbits       EQU     4       ; 8MHz/4 = 2MHz
//PLL_N           EQU     168     ; vco = (2MHz * 168) = 336 MHz

PLL_Nbits       EQU     (PLL_N << 6)
PLL_P           EQU     2       ; 336/2 = 168 MHz
PLL_Pbits       EQU     (((PLL_P >> 1) -1) << 16)
PLL_Q           EQU     7       ; 336/7 = 48 MHz for USB
PLL_Qbits       EQU     (PLL_Q << 24)
#endif

RCC_PLLCFGR_PLLSRC_HSE	EQU	00400000h

  DC32	strkk,(PLL_Mbits | PLL_Nbits | PLL_Pbits | RCC_PLLCFGR_PLLSRC_HSE | PLL_Qbits), RCC_PLLCFGR

RCC_CR_PLLON	EQU	01000000h

  DC32	atk,RCC_CR, ork,RCC_CR_PLLON, strk,RCC_CR	; Enable the main PLL

RCC_CR_PLLRDY	EQU	02000000h
  DC32	begin, atk,RCC_CR, andk,RCC_CR_PLLRDY, until	; Wait till the main PLL is ready
  DC32	initART	; init flash accelerator

RCC_CFGR_SW	EQU	00000003h
RCC_CFGR_SW_PLL	EQU	00000002h

; Select main PLL as sysclk
  DC32	atk,RCC_CFGR, andk,(~RCC_CFGR_SW), strk,RCC_CFGR
  DC32	atk,RCC_CFGR, ork,RCC_CFGR_SW_PLL, strk,RCC_CFGR

RCC_CFGR_SWS		EQU	0000000Ch
RCC_CFGR_SWS_PLL	EQU	00000008h

; wait for clk changeover
  DC32	begin, atk,RCC_CFGR,andk,RCC_CFGR_SWS, LIT,RCC_CFGR_SWS_PLL, EQUAL, until

RCC_CFGR_I2SSRC	EQU	00800000h
//* PLLI2S clock used as I2S clock source */
//RCC->CFGR &= ~RCC_CFGR_I2SSRC;

PLLI2S_N	EQU	192
PLLI2S_Nbits	EQU	(PLLI2S_N << 6)
PLLI2S_R	EQU	5
PLLI2S_Rbits	EQU	(PLLI2S_R << 28)

  DC32	strkk,(PLLI2S_Nbits | PLLI2S_Rbits), RCC_PLLI2SCFGR

RCC_CR_PLLI2SON		EQU	04000000h
RCC_CR_PLLI2SRDY	EQU	08000000h

  DC32	atk,RCC_CR, ork,RCC_CR_PLLI2SON, strk,RCC_CR
  DC32	begin, atk,RCC_CR, andk,RCC_CR_PLLI2SRDY, until	; wait for PLL ready

#endif  // #if STM32F4_XRC08_168MHZ | STM32F205RC_XRC10_118MHZ 

// Set SYSCLOCK = SYSTEMCLOCK defined in FISH_STM32F4_EQUATES.s
// in Assembler -> Preprocessor -> Defined Symbols
  DC32  strva, SYSTEMCLOCK, SYSCLOCK    // HCLK
  
//------------------------USART3 & GPIO I/O-------------------------------------
// SETUP USART3 I/O FOR UART3_INIT
//      SETBITS SETBITS:	( addr val -- ) OR val bits into addr.
//  DC32  LIT, RCC_AHB1ENR, LIT, 5h, SETBITS	; GPIO port A,C clk enable
  DC32  LIT, RCC_AHB1ENR, LIT, 10000111b, SETBITS ; GPIO port A,B,C & H clk enable

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// ADD GPIO CLK to C include GPIOH to use XRC as GPIO clk
  DC32  LIT, RCC_APB1ENR, LIT, 40000h, SETBITS	; USART3 clk enable
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


  ; PC10 as TX / PC11 as RX
  DC32  strva, 00007700h, GPIOC_AFRH

// rmwamd standalone low level ILK macro and rmwamd prim.
// STM DISCO BOARD:    Pc10 TX, pc11 RX, PC0 GND
  DC32 rmwamd, GPIOC_MODER,00F00003h,00A00001h	
  DC32 strva, 200Ch, USART3_CR1         ; Enable USART, TX and RX over8=0=16x

//---------------------------USART3_INIT TEST-----------------------------------
/*
// SET BAUD HERE FOR TESTING CHANGE OF BAUD IN UART3_INIT
// OVER8=0 = 16x OVERSAMPLING
#ifdef STM32F4_XRC08_168MHZ
BAUDRATE	SET	((273 << 4 | 7)); 9600: 42MHz / ( 16 * 273.4375 = 1117h)
//BAUDRATE	SET	((136 << 8 | 11)); 19200: 42MHz / ( 16 * 136.75 )
//BAUDRATE	SET	((45 << 6 | 13)); 57600: 42MHz / ( 16 * 45.5625 )
//BAUDRATE	SET	((22 << 4 | 8)); 115200: 42MHz / ( 16 * 22.8125 = 168h)
#endif
#ifdef STM32F4_IRC16_16MHZ
// 682h DIV_Mantissa = 68H DIV_fraction = 2
BAUDRATE        SET     ((104 << 4 | 2)); 9600: 16MHz / ( 16 * 104.1875 )
#endif
  DC32 strva, BAUDRATE, USART3_BRR	; BRG, x16 oversampling
*/

#ifdef XON_XOFF
// Asign CTS signal pin
#endif

#ifdef CTS_RTS
// Assign RTS = OUTPUT CTS = INPUT signals to pins
#endif

#ifdef CLKOUT
// Turn CLKOUT on and SET divide by...
#endif

// SYSTICK SECTION: (SysTick uses the processor clock.)
// SYST_RVR (Reload value) not set until user does with MS or DELAY.
// SYSTICK TIMER ENABLE: Bit 0 = 1
// SYSTICK CLKSOURCE: Bit 2 = 0 (SYSTEM CLOCK/2 OR 8)
// SYSTICK CLKSOURCE: Bit 2 = 1 (SYSTEM CLOCK)
//  DC32    strva, 5, SYST_CSR    // SYSCLK NO IRQ
  DC32    strva, 7, SYST_CSR    // SYSCLK + SYSTICK IRQ 

  DC32	SEMIS
