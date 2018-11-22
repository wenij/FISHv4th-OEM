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
// Note sure why x_r3 style seems to cause problems.
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

// This is for 118MHz HCLK | SYSCLK PCLK2 = 134MHz PCLK1 = 42MHz
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

// ADDING PSTAT INIT HERE FOR A MINUTE>
// 1RST TRY DOCS HARD TO FIND~!
/*
typedef enum gpio_af {
    GPIO_AF_SPI_1_2              = 5,
    GPIO_AF_USART_1_2_3          = 7,
} gpio_af;
*/

/* Intialize PORTB_MODER pins this way.
Value at end is this init								...........
PB15    BT-RST	        is active high.	0x1
PB14    BT-CSn	        is active low. 	0x1 SANITYY CHECK CS-BT USES THIS!!!
PB13    DAC-CSn        	is active low. 	0x1
PB12		ADC-CSn					is active low. 	0x1
PB11		SDA2						I2C START (NOT USED)
PB10		SCL2
PB9			NC
PB8			NC
PB7			SDA1
PB6			SCL1						I2C END (NOT USED)
PB5			SPI1-MOSI				is AF, SPI1_MOSI 0x2
PB4			SPI1-MISO				is AF, SPI1_MISO 0x2
PB3			JTDO/SPI1-SCLK	is AF, SPI1_SCLK 0x2
PB2			BOOT0		        Do not program	 0x0
PB1			ADC-PDN	        is active low. 	 0x1
PB0     ADC-RSTn				is active high.	 0x1
*/
// NO MORE /*  ...  */ COMMENTS UNTIL THE END OF THIS SEGMENT:
// THI SEGMENT IS ABOUT THE SPI1 ISSUES. So a bit bang ATTEMPT WILL BE MADE~!

// Disco Board ? SWD PROBLEM STARTS GETTING BAD WITH THESE LAST 2 STATEMENT IN THIS SECTION
// AND THE
  DC32  LIT, RCC_AHB1ENR, LIT, 010b, SETBITS	; GPIO port B clk enable
#ifdef PSTAT_SPI1_TEST
  DC32	LIT, RCC_APB2ENR, LIT, 1000000000000b, SETBITS	; ENABLE SPI1 CLOCK
#endif
/* bIT bANG aTTEMPT
// POKE PB3. iT'S SPI1 CLK AND SPECIAL
// sET ALL 3 TO 01 AND SET THEIR ODR TO 1 NAD SEE IF ALL THREE ARE HIGH
// fix below!
// 01010101000000000000101010000101b 55000A85h - MODER IS TWO BITS PER PORT
  DC32  LIT, GPIOB_MODER, LIT, 055000A85h, SETBITS

  DC32  LIT, GPIOB_ODR, LIT, 011h, SETBITS

*/
// MISSING GPIOB_AFRL INIT FOR SPI1

// stm32cUBEMX CAME UP WITH THIS GPIOB_AFRL SETTING:
// DOES NOT HELP HERE NOW
// LETS SEE IF IT WORKS IN IT'S PROJECT.
#ifdef PSTAT_SPI1_TEST
  DC32	LIT, GPIOB_AFRL, LIT, 00555000h, SETBITS
#endif

// 01010101000000000000101010000101b 55000A85h - MODER IS TWO BITS PER PORT
  DC32  LIT, GPIOB_MODER, LIT, 055000A85h, SETBITS

// 01100111110b (033Eh) is Full Duplex WITH spe NOT ENABLED - CPOL=1 CPHA=0
// 01100111101b (033Dh) is Full Duplex WITH spe NOT ENABLED - CPOL=0 CPHA=1 = DAC, BT
#ifdef PSTAT_SPI1_TEST
  DC32	LIT, SPI1_CR1, LIT, 033Dh, SETBITS		// ENABLE CONFIG BEFORE
#endif
// 1100001100111101b (0C33Dh) is UNIDIRECTIONAL OUPUT WITH spe NOT ENABLED
//	DC32	LIT, SPI1_CR1, LIT, 0C33Dh, SETBITS		// ENABLE CONFIG BEFORE

//	1000000b 040h = ENABLE SPE
#ifdef PSTAT_SPI1_TEST
	DC32	LIT, SPI1_CR1, LIT, 040h, SETBITS	// ENABLING SPI
#endif
//	wHEN INPLEMENTED IN PSTAT BRANCH: DC32	lit, 4, LIT, SPI1_CR1, SETBIT

// Set pins according to logic.
  DC32  LIT, GPIOB_ODR, LIT, 00007001h, SETBITS
//#endif	// #if 1	// bIT bANG aTTEMPT

//--------------------------------PORTC-----------------------------------------
// GPIO PORT C Reset value: 0x0000 0000


// MODER is a 2 bit init for each of the 16 ports in a bank.
// Enable PB0-1, and PB12-15 as Outputs for SPI BT, ADC and DAC BT RST
// PROGRAM SPI1 AF ON PB3, PB4 AND PB5
// USART3 inited on PORT C, in the first section, after system clocks setup.
// PORTC - Do std GPIO and level set init. Then do alternate function config.
/* Intialize PORTC pins this way.
Value at end is this init								...........
PC15
PC14
PC13
PC12
PC11    UART3_TX        is AF.
PC10    UART3_RX        is AF.
PC9     SWITCH4         is active high.
PC8     SWITCH3         is active high.
PC7     SWITCH2         is active high.
PC6     SWITCH1         is active high.
PC5     ADC_DATA_READY  is actve high.
PC4
PC3
PC2     GAIN_SWITCH2    is active low.
PC1     GAIN_SWITCH1    is active low.
PC0     GAIN_SWITCH0    is active low.

FOR PSTAT init add pc0-pc9 init here

*/

// FIX ADC_DATA_READY SO IT'S CORRECT TYPE OF INPUT?

//  DC32  LIT, RCC_AHB1ENR, LIT, 0100b, SETBITS	; GPIO port C clk enable

// Good MODER sets GAIN-S0-S2 and SW1-SW4 high AND LEAVES UART3 (PC10/11) ALONE
// This MODER ONLY sets GAIN-S0-S2 and SW1-SW4 high: 055415h
  DC32  LIT, GPIOC_MODER, LIT, 055415h, SETBITS
// Create lower speed OTYPER

// Good ODR is 03C0 = SWITCH1-4 ON.
  DC32  LIT, GPIOC_ODR, LIT, 03C0h, SETBITS
// 03C7 = SWITCH1-4 AND GAIN-SWITCH0-2 ON.
//  DC32  LIT, GPIOC_ODR, LIT, 03C7h, SETBITS

#ifdef PSTAT_BB_NOINIT_TEST
// NO GPIOB_MODER INIT. RESET INIT = 280h
// I.E. CLK (PB3) AND MISO (PB4) ARE 0X2 (AF)
// MOSI (PB5) IS 0, GPIO INPUT~!!!
// TRYING TO DUPLICATE CUBE BIT BANG TEST,
// THAT HAS SAME INIT (EVEN THOS CUBE SAYS IT'S
// PROGRAMMING OUTPUT ON SCLK AND MOSI, AND INPUT FOR MISO,
// THE RESULTS WHERE THAT SETTING CLK HI SET MISO HI~!!
// REPRODUCABLE IN:
// C:\Users\Clyde W Phillips Jr\Desktop\CUBE 205 SPI1 Bit-Bang\PSTAT SPI1 Bit-Bang
PSTAT_BB_NOINIT_INIT:
#endif // PSTAT_BB_NOINIT_TEST

#ifdef PSTAT_BB_TEST
// THE FULL PORTB PSTAT BB INIT:
// 01<BT-RST>01<BT-CSn>01<DAC-CSn>01<ADC-CSn>00.00.00.00.00
// 00<NOINIT>01<SPI1MOSI>00<SPI1-MISO>01<SPI1BB-CLK>00<BOOT0>
// 01<ADC-PDN>01<ADC-RSTn>
PSTAT_BB_INIT:
  DC32 NOOP
//  DC32	LIT, 010101000000000000010001000101b, LIT, GPIOB_MODER, SETBITS
//  DC32	LIT, 00000001b, LIT, GPIOB_ODR, SETBITS
// INIT FOR BIT BANG TEST, MODER is 2 bits wide per pin.
// PB3 OUT
// PB4 IN
// PB5 OUT
// 0440h 00,00,00,01,00,01B
// tHIS IS NOT SETTING THE REGISTER
//  DC32  LIT, GPIOB_MODER, LIT, 0440h, SETBITS

// this is FOR JUST SETTING UP BB, AND IS not working~!
// DC32	LIT, 00000001b, LIT, GPIOB_MODER, SETBITS
//  DC32  LIT, GPIOB_MODER, LIT, 0440h, SETBITS
// 10001,00,00,00b
//  DC32  LIT, GPIOB_MODER, LIT, 01000000b, SETBITS     // change jtag to spi clck
//  DC32  LIT, GPIOB_MODER, LIT, 0100000000b, CLRBITS   // change jtag to spi miso
//  DC32  LIT, GPIOB_MODER, LIT, 010000000000b, SETBITS // change jtag to spi miso
  
  DC32  LIT, 07FFFFFh, LIT, PSTATDACDATA, STORE
    DC32 NOOP
#endif // PSTAT_BB_TEST


  DC32	SEMIS
