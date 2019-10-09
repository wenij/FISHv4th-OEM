//-----------------------------------------------------------------------------
// Target_Config.h
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Header file include-once preprocessor directive
//-----------------------------------------------------------------------------

#ifndef _TARGET_CONFIG_H_
#define _TARGET_CONFIG_H_

//-----------------------------------------------------------------------------
// Global CONSTANTS - Available to all modules
//-----------------------------------------------------------------------------

// ------------------------------------------
// Target Config Options (common to BL & App)
// ------------------------------------------

#define TGT_PRODUCT_CODE         0x36  // Company-specific product ID
#define TGT_BL_TYPE              0x1  // BL Type = UART
// => "Fxxx_BL132_SMB0_Interface.h" should be included.

#define TGT_FLASH_PAGE_SIZE      1024   // Number of bytes per flash page
#define TGT_FLASH_PAGE_SIZE_CODE 2     // Flash Page Size Code: 2 => 1024 bytes 1 => 512 bytes


// Note: For serial streams like UART where "packets" are not applicable, the packet size 
// should be set to be equal to the BL buffer size.

#define TGT_BL_PKT_SIZE          32     // Size of comm protocol's packet size (bytes; should be powers of 2)
// min: 2^0 = 1; max: 2^14 = 16384; 2^15 = RESERVED.

#define TGT_BL_PKT_SIZE_CODE     0x05  // BL Comm Protocol Packet Size Code. 
// If TGT_BL_BUF_SIZE is 2^n, TGT_BL_BUF_SIZE_CODE is n.
// See documentation for encoding details.

// Mask to clear out address bits within a page (useful for page boundary check)
// Example: For 512-byte pages, the lower 9 address bits [8:0] represent the bytes within
// a Flash page; so this mask's lower 9 bits would be set => 0x000001FF
#define TGT_FLASH_PAGE_ADDR_MASK 0x03FF

// Specify 24-bit linear addresses here (do not use banked 16-bit addresses)
#define APP_FW_START_ADDR        0x0400
#define APP_FW_END_ADDR          0x77FF

#define APP_FW_SIG0_ADDR         APP_FW_END_ADDR
#define APP_FW_SIG3_ADDR         (APP_FW_END_ADDR - 3)

// Target Serial Number (for multi-drop buses)
#define DEV_SERIAL_NUMBYTES      0     // 0=Disabled; Max: 7
// ** Ensure a UNIQUE Serial Number for each device on the same bus **
// - These are "don't care" if DEV_SERIAL_NUMBYTES is set to 0.
// - These need not be used if device addressing on a bus is handled 
//   at the comm protocol level. 
#define TGT_DEVICE_SERIAL0       0x01     
#define TGT_DEVICE_SERIAL1       0x00        



//-----------------------------------------------------------------------------
// Header file include-once preprocessor directive
//-----------------------------------------------------------------------------

#endif // #ifndef _ARGET_CONFIG_H_
//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
