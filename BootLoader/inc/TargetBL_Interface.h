//-----------------------------------------------------------------------------
// TargetBL_Interface.h
//-----------------------------------------------------------------------------

#ifndef _TARGETBL_INTERFACE_H_
#define _TARGETBL_INTERFACE_H_

//-----------------------------------------------------------------------------
// Global CONSTANTS - Available to all modules
//-----------------------------------------------------------------------------

// The following macros will allow inclusion of specific sections from Comm_Interface files
#define _INCLUDE_TARGET_ONLY_ITEMS_
#define _INCLUDE_TARGET_BL_ONLY_ITEMS_

//-----------------------------------------------------------------------------
// Enums, Structs, Unions and Typedefs - Available to all modules
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// "Public" Global Variables - Externs - Available to all modules
//-----------------------------------------------------------------------------

extern uint8_t Flash_Key0;
extern uint8_t Flash_Key1;

//-----------------------------------------------------------------------------
// "Public" Function Prototypes - Externs - Available to all modules
//-----------------------------------------------------------------------------

// This is usually defined in the STARTUP.A51 file.
void START_APPLICATION(void);

// ---------------------------------
// Device-Specific Functions
// Defined in "Fnnn_TargetBL_DevSpecific.c"
// ---------------------------------
void Device_Init(void);

// ---------------------------------
// Flash Read/Erase/Write Functions
// Defined in "Fnnn_Flash.c"
// ---------------------------------
void FLASH_Modify (uint32_t addr, uint8_t byte);
// Note: No flash read function is necessary for non-code banked devices
// (because flash reads are performed directly using a CODE pointer).

// ---------------------------------
// Comm Functions
// Defined in "Fnnn_Comm_*.c"
// Comm-specific interface constants defined in "Fxxx_BLddd_*.h"
// ---------------------------------
//void Comm_Wait_For_Rx_Bytes (U8);
//void Comm_Send_Tx_Bytes (U8);

// -------------------------------------
// Bootloader Command Processors
// Defined in "Fxxx_TargetBL_Commands.c"
// -------------------------------------
void TGT_Erase_Page(uint32_t Addr);
void TGT_Write_Flash(uint8_t *buf, uint32_t addr);

//-----------------------------------------------------------------------------
// Header file include-once preprocessor directive
//-----------------------------------------------------------------------------

#endif // _TARGETBL_INTERFACE_H_

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
