/*
 * @file   flash_IO.c
 * Support for littlefs and fatfs io
 * Not building probably decause I haven't provided a flash_IO,h with prototypes?
 */
#include "flash_IO.h"

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

// lfs will be feed these to/from the read and write cache

// The name can be changed if the fatfs dependency is resolved,
// Either by duplicating them for fatfs or deleting fatfs references.
static unsigned char USER_read_buffer[512];
static unsigned char USER_write_buffer[512] = { 0x5f, 0xc5 };

#ifdef littlefs
/*
extern lfs_internal_flash;
This is an incorrect declaration. Extern and static ONLY determine scope - extern tells the compiler to make the declaration visible outside the file. static tells it not to.
The type is missing so it doesn't work. You get a compiler warning because in the "C" language standard a missing type declaration implies int.

extern lfs_cfg;
*/
//error: initializer element is not constant static lfs_t lfs_internal_flash;
// This works, static alone does not. Also can declare struct, but scope is unclear to me.
//    struct <name> is a type called a structure. That is how it's declared. hence you must write struct lfs_config.
//static struct lfs_config lfs_cfg;
// 2nd arg workaround
//static struct lfs_config _config;
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

/*
 * These are function prototypes for the helper functions. Declared static so they are only visible in this file.
 * The functions are defined down below. Not in their original location which was inside another function.
 */
static int sync_HAL(const struct lfs_config *c);
static int erase_HAL(const struct lfs_config *c, lfs_block_t block);
static int prog_HAL(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size);
static int read_HAL(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size);

// I could not call lfs_format or lfs_mount from main
// int lfs_PSTAT_init(lfs_t *lfs, const struct lfs_config *cfg){
// So workaround is this wrapper I can call from main

static struct lfs_config lfs_cfg;
static lfs_t lfs_internal_flash;

// Initialize structs and call lfs_format then lfs_mount
int lfs_PSTAT_init(void)
{

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

    /*
		  int (*read)(const struct lfs_config *c, lfs_block_t block,
		              lfs_off_t off, void *buffer, lfs_size_t size);
Need to figure out how to assign the function as a pointer in this struct.
     */
    /* googled this as way to assign pointer
     * doesn't work
		  int (*functionPtr)(lfs_config, block, off, buffer, size);
//		  functionPtr = &read_hal;
//		  lfs_cfg.read = functionPtr;
     */
    /* from mbed examample?
     * doesn't work
lfs_cfg.read = read_HAL(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size);

        It's much simpler than that:
     */

    lfs_cfg.read = read_HAL;
    lfs_cfg.prog = prog_HAL;
    lfs_cfg.erase = erase_HAL;
    lfs_cfg.sync = sync_HAL;

    // Below are optional. file_max is useful. Name length handled by other attributes. Still need to be set to 0.
    lfs_cfg.name_max = 0; // This is optional. The size of the info struct
    // is controlled by the LFS_NAME_MAX define.
    // Defaults to LFS_NAME_MAX when zero. LFS_NAME_MAX is stored in
    // superblock and must be respected by other littlefs drivers.
    lfs_cfg.file_max = 0;
    lfs_cfg.attr_max = 0;
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




    /* Will proceed with Format then mount. Static function lfs_init should be called by them.
     */
    // Returns a negative error code on failure.
    // this call fails in lfs_init. The args seem valid going in but are wrong in it's scope.
    //  int lfs_format_status =  lfs_format(&lfs_internal_flash, &lfs_cfg);
    // Returns a negative error code on failure.
    int lfs_mount_status =  lfs_mount(&lfs_internal_flash, &lfs_cfg);
    return lfs_mount_status;
}


/*
 * Wrapper functions
 */

int lfsopen( lfs_file_t *file, const char *path, int flags)
{
    return( lfs_file_open( &lfs_internal_flash, file, path, flags) );
}

lfs_ssize_t lfsread( lfs_file_t *file, void *buffer, lfs_size_t size)
{

    return( lfs_file_read( &lfs_internal_flash, file, buffer, size) );
}

lfs_ssize_t lfswrite( lfs_file_t *file, void *buffer, lfs_size_t size)
{
    return( lfs_file_write( &lfs_internal_flash, file, buffer, size) );
}

int lfsflush( lfs_file_t *file)
{
    return( lfs_file_sync( &lfs_internal_flash, file) );
}

int lfsclose( lfs_file_t *file)
{
    return( lfs_file_close( &lfs_internal_flash, file) );
}

/*
 * Helper functions
 */
int read_HAL(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size){

    return LFS_ERR_OK;
};


/* implement prog
    // Program a region in a block. The block must have previously
    // been erased. Negative error codes are propagated to the user.
    // May return LFS_ERR_CORRUPT if the block should be considered bad.
int (*prog)(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size);
Debug an argument at a time
*/
int prog_HAL(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size){

    return LFS_ERR_OK;
};

/* implement erase
 *     // Erase a block. A block must be erased before being programmed.
    // The state of an erased block is undefined. Negative error codes
    // are propagated to the user.
     *     int (*erase)(const struct lfs_config *c, lfs_block_t block);
     *
    // May return LFS_ERR_CORRUPT if the block should be considered bad.
Debug an argument at a time
*/
int erase_HAL(const struct lfs_config *c, lfs_block_t block){

    return LFS_ERR_OK;
};

/* implement sync
    // Sync the state of the underlying block device. Negative error codes
    // are propogated to the user.
    int (*sync)(const struct lfs_config *c);
Debug an argument at a time
*/
int sync_HAL(const struct lfs_config *c){

    return LFS_ERR_OK;
};

#endif

