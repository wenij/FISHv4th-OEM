//-----------------------------------------------------------------------------
// TargetBL_DevSpecific.c
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "main.h"

#include "Target_Config.h"
#include "Target_Interface.h"

#include "TargetBL_Config.h"
#include "TargetBL_Interface.h"

#include "UART_Interface.h"

//-----------------------------------------------------------------------------
// Global CONSTANTS
//-----------------------------------------------------------------------------

// ------------------------------------------
// *** Device-Specific *** 
// Change this as necessary based on MCU
// ------------------------------------------

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Function Prototypes (Local)
//-----------------------------------------------------------------------------
void OSCILLATOR_Init(void);
void PORT_Init(void);
void UART0_Init(void);

//=============================================================================
// Function Definitions
//=============================================================================
void Disable_Watchdog(void)
{
    PCA0MD &= ~0x40; // Disable Watchdog
}

void Device_Init(void)
{
	unsigned char SFRPAGE_save;
	Disable_Watchdog();
	
	SFRPAGE_save = SFRPAGE; // Save the current SFRPAGE
	SFRPAGE = CONFIG_PAGE;  // Switch to the necessary SFRPAGE

    OSCILLATOR_Init(); // Configure system clock
    PORT_Init(); // Initialize crossbar
    UART0_Init(); // Init UART0 & Timer1 (Data source comm)
	
	SFRPAGE = SFRPAGE_save;             // Restore the SFRPAGE
}

//-----------------------------------------------------------------------------
// OSCILLATOR_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// This routine initializes the system clock to use the internal oscillator
// at its maximum frequency.
// Also enables the Missing Clock Detector.
//-----------------------------------------------------------------------------
void OSCILLATOR_Init(void)
{
  /* disable F930 watchdog */
  PCA0MD &= (~M_WDTE);

  /* Init Internal Precision Oscillator (24.5MHz) */
  SFRPAGE = LEGACY_PAGE;
  FLSCL   = M_BYPASS;

  OSCICN |= M_IOSCEN; // p7: Internal Prec. Osc. enabled
  CLKSEL  = 0x00;     // Int. Prec. Osc. selected (24.5MHz)

	PCA0MD &= ~0x40;			// WDTE = 0 (clear watchdog timer enable)
    RSTSRC    = 0x04;
}

//-----------------------------------------------------------------------------
// PORT_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configure the Crossbar and GPIO ports.
//
// P0.1   digital   push-pull    UART0 TX
// P0.2   digital   open-drain   UART0 RX
//
// P3.2   digital  push-pull     LED
// P3.0   digital  open-drain    SW

//-----------------------------------------------------------------------------

void PORT_Init(void)
{
    /* disable F930 watchdog */
    PCA0MD &= (~M_WDTE);
    
    XBR2 |= M_XBARE;

        //P0MDOUT |= 0x5F;
        P0SKIP=0;
        
        P0SKIP  &= ~(0x30); //skip all on port, but UART

        

        P1SKIP=0;        
        
    P0MDOUT   = 0xDF;
    P1MDOUT   = 0xFD;        
        // All outputs are low.
        P1=0;
        P0=0;

        //XBR1      = 0x40;        // Wireless SPI enable


    XBR0 |= 0x01; // Enable UART
}


//-----------------------------------------------------------------------------
// UART0_Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Configure the UART0 using Timer1, for <BAUDRATE> and 8-N-1.
//-----------------------------------------------------------------------------
void UART0_Init(void)
{
	SCON0 = 0x10;  		    // SCON0: 8-bit variable bit rate + read
                     	    //        level of STOP bit is ignored
                     	    //        RX enabled
                     	    //        ninth bits are zeros
                     	    //        clear RI0 and TI0 bits
   /*
	SCON0: Serial Port 0 Control Register (0x98)
	+----------------------------------------------------+
	| S0MODE | - | MCE0 | REN0 | TB80 | RB80 | TI0 | RI0 |
	+----------------------------------------------------+
	REN0: Receive Enable
		This bit enables/diseables the UART receiver.
		0: UART0 reception disabled.
		1: UART0 reception enabled.

	TI0: Transmit Interrupt Flag

	RI0: Receive Interrupt Flag
	*/

	//
	// BaudRate = T1CLK/(256-TH1)/2
	// BaudRate = 19140 bps
	//
 	TMOD &= ~0x30;		// clear T1M1,T1M0 bits
   	TMOD |= 0x20;		// Timer 1: Mode 2: 8-bit counter/timer with auto-reload
   /*
	TMOD: Timer Mode Register
	+---------------------------------------------------------+
	| GATE1 | C/T1 | T1M1 | T1M0 | GATE0 | C/T0 | T0M1 | T0M0 |
	+---------------------------------------------------------+
 	C/T1: Counter/Timer 1 Select
	0: Timer function: Timer1 incremented by clock defined by T1M bit(CKCON.4).
	1: Counter function: Timer1 incremented by high-to-low transitions 
	   on external input pin (T1).

    T1M1-T1M0: Timer 1 Mode Select.
	These bits select the Timer 1 operation mode.
		0     0    - Mode 0: 13-bit counter/timer 
		0     1    - Mode 1: 16-bit counter/timer 
		1     0    - Mode 2: 8-bit counter/timer with auto-reload
		1     1	   - Mode 3: two 8-bit counter/timers 

	*/


 	CKCON &= ~0x03;		// Clear SCA1-0 bits	- baudrate 19200
  	CKCON |= 0x01;		// system clock divided by 4
 	CKCON &= ~0x08;		// T1M=0:  use SCA1:SCA0 timebase

 //	CKCON &= ~0x03;		// Clear SCA1-0 bits	- baudrate 9600
 // 	CKCON |= 0x00;		// system clock divided by 12
 //	CKCON &= ~0x08;		// T1M=0:  use SCA1:SCA0 timebase
  /*
 	CKCON: Clock Control Register
	+-----------------------------------------------------+
	| T3MH | T3ML | T2MH | T2ML | T1M | T0M | SCA1 | SCA0 |
	+-----------------------------------------------------+
												0     0    - SYSCLK/12 
												0     1    - SYSCLK/4
												1     0    - SYSCLK/48
												1     1	   - SYSCLK/8
													SYSCLK=24.5 MHz
	T1M: Timer 1 Clock Select
	This bit selects the clock source suppplied to Timer 1. T1M is ignored
	when C/T1 is set to logic 1.
	0: Counter/Timer1 uses the clock defined by the prescale bits, SCA1-0
	1: Counter/Timer1 uses the system clock.
	
	*/

   	TH1 = 0x60;			// timer 1 reload value	- baudrate 19200
//	TH1 = 0x96;			// baudrate 9600

 	TR1 = 1;			// START Timer1 --- TCON: Timer Control Register
  	TI0 = 1;       	 	// Indicate TX0 ready --- SCON0: Serial Port 0 Control Register


 #if 0
    SCON0 = 0x10; // SCON0: 8-bit variable bit rate
    //        level of STOP bit is ignored
    //        RX enabled
    //        ninth bits are zeros
    //        clear RI0 and TI0 bits
    if (SYSCLK / BAUDRATE / 2 / 256 < 1)
    {
        TH1 = -(SYSCLK / BAUDRATE / 2); // 0x96
        CKCON &= ~0x0B; // T1M = 1; SCA1:0 = xx
        CKCON |= 0x08;
    }
    else if (SYSCLK / BAUDRATE / 2 / 256 < 4)
    {
        TH1 = -(SYSCLK / BAUDRATE / 2 / 4);
        CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 01
        CKCON |= 0x01;
    }
    else if (SYSCLK / BAUDRATE / 2 / 256 < 12)
    {
        TH1 = -(SYSCLK / BAUDRATE / 2 / 12);
        CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 00
    }
    else
    {
        TH1 = -(SYSCLK / BAUDRATE / 2 / 48);
        CKCON &= ~0x0B; // T1M = 0; SCA1:0 = 10
        CKCON |= 0x02;
    }

    TL1 = TH1; // init Timer1
    TMOD &= ~0xf0; // TMOD: timer 1 in 8-bit autoreload
    TMOD |= 0x20;
    TR1 = 1; // START Timer1
#endif

}


//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
