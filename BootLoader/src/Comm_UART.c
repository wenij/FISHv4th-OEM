//-----------------------------------------------------------------------------
// Comm_UART.c
//-----------------------------------------------------------------------------
//
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
#include "CRC.h"



extern TGT_BL_InfoBlock[TGT_BL_FW_INFOBLOCK_LENGTH], const U8, SEG_CODE);
extern uint8_t data rx_buf[32];
//-----------------------------------------------------------------------------
// Function Prototypes (Local)
//-----------------------------------------------------------------------------

//=============================================================================
// Function Definitions
//=============================================================================
//-----------------------------------------------------------------------------
// SRC_Get_Info
//-----------------------------------------------------------------------------
//
// Return Value:  Response
// Parameters:    None
//
//
//
//-----------------------------------------------------------------------------
uint8_t SRC_Get_Info(void)
{
	uint8_t data cmd = SRC_CMD_GET_INFO;

    // Command Format:
    // [0] Command
	uart_send(&cmd, 1);
    // Response:
    // [0] Return code (ACK/ERROR etc)
    // [1] InfoBlock Length
    // [2] MCU Code
    // [3] BL Type
    // [4] Flash Page Size Code
    // [5] Flash Key0
    // [6] Flash Key1
    // [7] SMB Device Address
    // [8] Application FW Start Addr Low (0)
    // [9] Application FW Start Addr (1)
    // [10] Application FW Start Addr High (2)
    // [11] Application FW End Addr Low (0)
    // [12] Application FW End Addr (1)
    // [13] Application FW End Addr High (2)
	uart_receive(rx_buf, SRC_CMD_GET_INFO_RX_SZ);

    return (rx_buf[0]);
}

//-----------------------------------------------------------------------------
// SRC_Get_Page_Info
//-----------------------------------------------------------------------------
//
// Return Value:  Reponse
// Parameters:    buf
//
//
//
//-----------------------------------------------------------------------------
uint8_t SRC_Get_Page_Info(void)
{
	uint8_t cmd = SRC_CMD_GET_PAGE_INFO;

    // Command Format:
    // [0] Command
	uart_send(&cmd, 1);

    // Response:
    // [0] Return code (ACK/ERROR etc)
    // [1] Page address byte 0 {or Num Pages, if return code = SRC_RSP_DATA_END}
    // [2] Page address byte 1
    // [3] Page address byte 2 <-- don't care for MCUs 64k and smaller
    // [4] Page CRC byte 0
    // [5] Page CRC byte 1
    // Note: If Num Pages is provided, add 1 to get actual num pages
	uart_receive(rx_buf, SRC_CMD_GET_PAGE_INFO_RX_SZ);

    return (rx_buf[0]);
}

//-----------------------------------------------------------------------------
// SRC_Get_Page
//-----------------------------------------------------------------------------
//
// Return Value:  Response cmd
// Parameters:    buf
//
//
//
//-----------------------------------------------------------------------------
uint8_t SRC_Get_Page(U8 xdata *buf)
{
	uint8_t cmd = SRC_CMD_GET_PAGE;
    // Command Format:
    // [0] Command
	uart_send(&cmd, 1);

    // Response:
    // [0] Return code (ACK/ERROR etc)
    // [1..512] Page data
    // [513] Return code (ACK/ERROR etc)
	uart_receive(&cmd, 1);
	uart_receive(buf, SRC_CMD_GET_PAGE_RX_SZ);
	uart_receive(&cmd, 1);

    return (cmd);
}

//-----------------------------------------------------------------------------
// SRC_Disp_TGT_Info
//-----------------------------------------------------------------------------
//
// Return Value:  Response
// Parameters:    None
//
//
//
//-----------------------------------------------------------------------------
uint8_t SRC_Disp_TGT_Info(void)
{
	uint8_t cmd;

    // Command Format:
    // [0] Command
	uart_send(TGT_BL_InfoBlock, TGT_BL_FW_INFOBLOCK_LENGTH);

    // Response:
    // [0] Return code (ACK/ERROR etc)
	uart_receive(&cmd, SRC_CMD_DISP_TGT_INFO_RX_SZ);


    return (cmd);
}

//-----------------------------------------------------------------------------
// SRC_Disp_Info_Code
//-----------------------------------------------------------------------------
//
// Return Value:  None
// Parameters:    info_code
//
//
//
//-----------------------------------------------------------------------------
uint8_t SRC_Disp_Info_Code(U8 info_code)
{

    // Command Format:
    // [0] Command
    // [1] Info Code
	rx_buf[0] = SRC_CMD_DISP_INFO_CODE;
	rx_buf[1] = info_code;
	uart_send(rx_buf, 2);

    // Response:
    // [0] Return code (ACK/ERROR etc)
	uart_receive(rx_buf, SRC_CMD_DISP_INFO_CODE_RX_SZ);
    return (rx_buf[0]);
}

//-----------------------------------------------------------------------------
// UART wrapper functions
//-----------------------------------------------------------------------------

//  uart send, count max number is 255, limited by XRAM size
void uart_send(U8 *buf, U8 count)
{
	uint8_t *ptr = buf;
	do{
	    SBUF0 = *ptr++;
		while(TI0 == 0);
		TI0 = 0;
	}while(--count);
}

// uart receive, count max 1024, limited by XRAM size
void uart_receive(U8 *buf, U16 count)
{
	uint8_t * ptr = buf;
	do{
		if(RI0 == 1)
		{
			*ptr++ = SBUF0;
			RI0 = 0;
			count--;
		}
	}while(count);
}

//-----------------------------------------------------------------------------
// End Of File
//-----------------------------------------------------------------------------
