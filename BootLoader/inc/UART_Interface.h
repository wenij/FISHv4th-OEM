//-----------------------------------------------------------------------------
// UART_Interface.h
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Header file include-once preprocessor directive
//-----------------------------------------------------------------------------

#ifndef _UART_INTERFACE_H_
#define _UART_INTERFACE_H_

//-----------------------------------------------------------------------------
// Global CONSTANTS - Available to all modules
//-----------------------------------------------------------------------------
#define BAUDRATE 115200


//-----------------------------------------------------
// Constants common to Master / Target BL / Target App
//-----------------------------------------------------

// *** Add constants here to be accessed by the Master, Target BL & Target App ***

// Note: 'TGT_BL_TYPE' should be defined as 131 (0x81) elsewhere.

//---------------------------------------------
// MASTER-only Constants
//---------------------------------------------

#ifdef _INCLUDE_MASTER_ONLY_ITEMS_

// *** Add constants here to be accessed only by the master ***

#endif

//-----------------------------------------------------
// Constants common to Target BL and Target App
//-----------------------------------------------------

#ifdef _INCLUDE_TARGET_ONLY_ITEMS_

// *** Add constants here to be accessed by the target BL & App ***

#endif

//---------------------------------------------
// TARGET BL-only Constants
//---------------------------------------------

#ifdef _INCLUDE_TARGET_BL_ONLY_ITEMS_

// *** Add constants here to be accessed only by the Target BL ***

#endif

//---------------------------------------------
// TARGET App-only Constants
//---------------------------------------------

#ifdef _INCLUDE_TARGET_APP_ONLY_ITEMS_

// *** Add constants here to be accessed only by the Target App ***

//-------------------------------
// Support Function Prototypes
//-------------------------------
#endif
uint8_t SRC_Get_Info(void);
uint8_t SRC_Get_Page_Info(void);
uint8_t SRC_Get_Page(uint8_t *buf);
uint8_t SRC_Disp_TGT_Info(void);
uint8_t SRC_Disp_Info_Code(uint8_t info_code);
void uart_send(uint8_t *buf, uint8_t count);
void uart_receive(uint8_t *buf, uint8_t count);

//-----------------------------------------------------------------------------
// Header file include-once preprocessor directive
//-----------------------------------------------------------------------------

#endif // _UART_INTERFACE_H_
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
