/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/* 
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future. 
 * Kept to ensure backward compatibility with previous CubeMx versions when 
 * migrating projects. 
 * User code previously added there should be copied in the new user sections before 
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);  
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read, 
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */  
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
    Stat = STA_NOINIT;
    return Stat;
  /* USER CODE END INIT */
}
 
/**
  * @brief  Gets Disk Status 
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
    Stat = STA_NOINIT;
    return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s) 
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive number to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
	/*
	 * I created static unsigned char USER_read_buffer[512];
	 * Yet if this function also [returned] the source address, who needs the buffer?
	 * any who using memcopy to buffer ~ to be declared in caller, later.
	 */
	// FLASH            0x08000000         0x00100000         xr TOP of Flash = 0x8100000
	// using 0x80000 , 524288 bytes for flash at an offset of 0x8000, so 0x8080000 to 0x8100000
	uint8_t *flashSource = (uint8_t *) 0x8080000;	// Set to the start of the flash *sector* addresses.


	    /* Move to the start of the sector being read. */
	    flashSource += ( 512 * sector );

	    /* Hoping Flash data can be copied using memcpy(). */
	    memcpy( ( void * ) buff,
	            ( void * ) flashSource,
	            ( size_t ) ( count * 512 ) );
// does memcopy have a return value? Is this always a valid read?
    return RES_OK;
  /* USER CODE END READ */
}
/**
  * @brief  Writes Sector(s)  
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{ 
  /* USER CODE BEGIN WRITE */
  /* USER CODE HERE */
	// FLASH            0x08000000         0x00100000         xr TOP of Flash = 0x8100000
	// using 0x80000 , 524288 bytes for flash at an offset of 0x8000, so 0x8080000 to 0x8100000
	uint32_t FlashAddress = (uint32_t) 0x8080000;	// Set to the start of the flash *sector* addresses.
	uint64_t data = 0x5f;
	uint8_t eraseSector = FLASH_SECTOR_8;
    /* Move to the start of the sector being read. */
	FlashAddress += ( 512 * sector );

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );

    //For loop here for sector(s) to erase
//void FLASH_Erase_Sector(FLASH_SECTOR_6, VOLTAGE_RANGE_3);
    FLASH_Erase_Sector(eraseSector, VOLTAGE_RANGE_3);
    //for loop here to write bytes/halfword or words in a sector.
    			   // HAL_StatusTypeDef HAL_FLASH_Program(uint32_t TypeProgram, uint32_t Address, uint64_t Data)
    HAL_StatusTypeDef flashprogstatus = HAL_FLASH_Program(TYPEPROGRAM_WORD, FlashAddress, data);
    if (flashprogstatus)
    	while(1);
    HAL_FLASH_Lock();

    return RES_OK;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation  
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
    DRESULT res = RES_ERROR;
    return res;
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
