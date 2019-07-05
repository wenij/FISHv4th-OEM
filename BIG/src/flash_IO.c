/*
 * @file   flash_IO.c
 * Support for littlefs and fatfs io
 * Not building probably decause I haven't provided a flash_IO,h with prototypes?
 */

#ifdef STM32F205xx
#include "stm32f2xx_hal.h"
#else
#include "stm32f4xx_hal.h"
#endif
// define filesystem in use - MOVE TO PREPROCESSOR TO MAKE GLOBAL
#define littlefs
//#define fatfs
#ifdef fatfs
// Fatfs stuff
#include "fatfs.h"
#include "FF_Disk_t.h"
#include "FF_ioman.h"
#include "ff_flashDisk_Init.h"
// THis gets included in the init function.
//#include "ff_disk_t.h"
#endif
#ifdef littlefs
// littlefs includes
#include "lfs.h"
#endif
#ifdef fatfs
extern DRESULT USER_read (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
extern DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
extern void initialise_monitor_handles(void);
#endif
#ifdef fatfs
// Move to file that initializes the structures
/* FatFs stuff */
static FATFS fatfs;
static FIL fatfile;
static DIR fatdir;
static FILINFO fatfileinfo;
#endif
#ifdef littlefs
static lfs_t lfs_internal_flash;
// This works, static alone does not. Also can declare struct, but scope is unclear to me.
static struct lfs_config lfs_cfg;
// numerous sub structs to deal with
static lfs_cache_t lfs_read_cache;
static lfs_cache_t lfs_write_cache;
#endif
/* USER CODE BEGIN 2  This is tested and works */
/* 1meg Flash sector allocation
 * sector#	address		size	#512k sectors
 * 0			0x8000000	16k
 * 1			0x8004000	16k
 * 2			0x8008000	16k
 * 3			0x800C000	16k
 * 4			0x8010000	64k
 * 5			0x8020000	128k
 * 6			0x8040000	128k
 * 7			0x8060000	128k
 * 8			0x8080000	128k	250 512k sectors
 * 9			0x80A0000	128k	250 512k sectors
 * 10			0x80C0000	128k	250 512k sectors
 * 11			0x80E0000	128k	250 512k sectors
 */
#ifdef fatfs
/* Unit test USER_read */
//DRESULT readstatus = USER_read(	0,	USER_read_buffer,	0,	1); // Called in the main function.
//if (readstatus)
//	  while(1);
/* Unit test USER_write */
//  DRESULT writestatus = USER_write(	0,	USER_write_buffer,	0,	1);
//  if (writestatus)
//	  while(1);
// this prototype writes the same sector this reads.
//  readstatus = USER_read(	0,	USER_read_buffer,	0,	1); // Called in the main function.
//  if (readstatus)
//	  while(1);
/*
FF_Disk_t * p0Disk;
#define flashPARTITION_NUMBER            0
char diskName[] = {'p','F','L','A','S','H'};
char *pcName = diskName;

p0Disk = FF_FlashDiskInit( pcName,
		  //uint8_t *pucDataBuffer,
		  (uint8_t *) 0x8080000,	// beginning of 1000 sectors of flash disk
        //uint32_t ulSectorCount,
		  1000ul,
        //size_t xIOManagerCacheSize )
							  0);
*/
#endif
#ifdef littlefs
// Configure the lfs_config struct
lfs_cfg.read_size = 512;
lfs_cfg.prog_size = 512;
lfs_cfg.block_size = 512 * 250;	// 128k this is the  erase block size.
lfs_cfg.block_count = 4;
lfs_cfg.block_cycles = 10000; // Number of erase cycles before we should move data to another block.
lfs_cfg.cache_size = 512; // littlefs needs a read cache, a program cache, and one additional
		    // cache per file. Optional statically allocated read buffer. Must be cache_size.
lfs_cfg.lookahead_size = 8 * 64; // lookahead buffer is stored as a compact bitmap,
		  // so each byte of RAM can track 8 blocks. Blocks are 512k.
		  // The code says: must be multiple of 64-bits
		  // Don't know requirements for this yet. Aren't blocks allocated?
		  // So how many blocks should this be tracking? Right now allocating nominal values.
		  void *context;
		  int (*read)(const struct lfs_config *c, lfs_block_t block,
		              lfs_off_t off, void *buffer, lfs_size_t size);
		  int (*prog)(const struct lfs_config *c, lfs_block_t block,
		              lfs_off_t off, const void *buffer, lfs_size_t size);
		  int (*erase)(const struct lfs_config *c, lfs_block_t block);
		  int (*sync)(const struct lfs_config *c);
		  void *read_buffer; // Optional statically allocated read buffer. Must be cache_size.
		  void *prog_buffer; // Optional statically allocated program buffer. Must be cache_size.
		  void *lookahead_buffer; // Optional statically allocated lookahead buffer. Must be lookahead_size
		    // and aligned to a 64-bit boundary.

		  // Below are optional. file_max is useful. Name length handled by other attributes.
		  lfs_cfg.name_max; // This is optional. The size of the info struct
		  	// is controlled by the LFS_NAME_MAX define.
		  	// Defaults to LFS_NAME_MAX when zero. LFS_NAME_MAX is stored in
		    // superblock and must be respected by other littlefs drivers.
		  lfs_cfg.file_max;
		  lfs_cfg.attr_max;
// unit test the static struct lfs_cfg
// either this call or the function added below ruins the debug run - end up in HAL timer crap.
// commented both out, Need to replug in target?
//ut_lfs_cfg();
// configure the lookahead buffers

// Configure the read cache
lfs_read_cache.block = 1; // fudge
lfs_read_cache.buffer = USER_read_buffer; // rename
lfs_read_cache.off = 0; // fudge
lfs_read_cache.size = 512; // not sure caches are same size as 512k blocks~!
// Configure the write cache
lfs_write_cache.block = 1; // fudge
lfs_write_cache.buffer = USER_write_buffer; // rename
lfs_write_cache.off = 0; // fudge
lfs_write_cache.size = 512; // All the 512's need to be a symbol
// Add the read and write cache to lfs_internal_flash struct
lfs_internal_flash.pcache = lfs_write_cache; // Verify this is write cache
lfs_internal_flash.rcache = lfs_read_cache;
#endif

/* Will proceed with Format then mount. Static function lfs_init should be called by them.
*/
/* trying to build this file
  // Returns a negative error code on failure.
  // this call fails in lfs_init. The args seem valid going in but are wrong in it's scope.
  int lfs_format_status =  lfs_format(&lfs_internal_flash, &lfs_cfg);
  // Returns a negative error code on failure.
  int lfs_mount_status =  lfs_mount(&lfs_internal_flash, &lfs_cfg);
*/
