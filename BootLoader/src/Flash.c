//-----------------------------------------------------------------------------
// Flash.c
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "main.h"


//=============================================================================
// Function Definitions
//=============================================================================

//-----------------------------------------------------------------------------
// FLASH_Modify
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   :
//   1) U16 addr - address of the byte to write to
//   2) U8 byte - byte to write to Flash.
//
// This routine writes <byte> to the linear FLASH address <addr> if PSCTL == 1.
// This routine erases the FLASH page containing the linear FLASH address is
// PSCTL == 3.
//-----------------------------------------------------------------------------
void FLASH_Modify(uint32_t addr, uint8_t byte)
{

    // *** Device-Specific ***
    // ** IMPORTANT ** When modifying flash primitive functions to fit in this
    // template, it is important to remove the literal flash key values and use
    // the global variables Flash_Key0 and Flash_Key1 instead.

    if((addr > APP_FW_END_ADDR) || (addr < APP_FW_START_ADDR))
    {
		PSCTL &= ~0x03; // PSWE = 0 which disable writes
    	return ;
    }
    VDM0CN = 0x80; // Enable VDD monitor
    RSTSRC = 0x02; // Enable VDD monitor as a reset source

    FLKEY = Flash_Key0; // Key Sequence 1
    FLKEY = Flash_Key1; // Key Sequence 2
    *((U8 xdata *) addr) = byte;
    PSCTL &= ~0x03; // PSWE = 0 which disable writes
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
