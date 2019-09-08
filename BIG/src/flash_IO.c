/*
 * @file   flash_IO.c
 * Support for littlefs and fatfs io
 * Not building probably decause I haven't provided a flash_IO,h with prototypes?
 */
#include "flash_IO.h"
#include <assert.h>
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
#include "lfs.h"
#include "lfs_util.h"
#define SECTOR_SIZE 0x20000
#define SECTOR08_ADDR 0x8080000
#define SECTOR09_ADDR 0x80A0000
#define SECTOR10_ADDR 0x80C0000
#define SECTOR11_ADDR 0x80E0000
#define LFS_BUFFERS_SIZE 512	// Read, write and cache buffer size.
								// .buffer the erase block size and is LFS_BUFFERS_SIZE * 250.
#define LOOKAHEAD_BUFFER_SIZE 8 * 64; // lookahead buffer is stored as a compact bitmap.
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

// This is a declaration of buffers shared by fatfs and littlefs for now.
// lfs will be feed these to/from the read and write cache.
// If need be separate later.

// The name can be changed if the fatfs dependency is resolved,
// Either by duplicating them for fatfs or deleting fatfs references.
static unsigned char USER_read_buffer[LFS_BUFFERS_SIZE];
static unsigned char USER_write_buffer[LFS_BUFFERS_SIZE] = { 0x5f, 0xc5 };

/* USER CODE BEGIN 2 */
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
 * 8			0x8080000	128k	250 512k sectors FLASH_SECTOR_8 (ENUMS)
 * 9			0x80A0000	128k	250 512k sectors FLASH_SECTOR_9
 * 10			0x80C0000	128k	250 512k sectors FLASH_SECTOR_10
 * 11			0x80E0000	128k	250 512k sectors FLASH_SECTOR_11
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
		  (uint8_t *) FLASH_SECTOR_8,	// beginning of 1000 sectors of flash disk
        //uint32_t ulSectorCount,
		  1000ul,
        //size_t xIOManagerCacheSize )
							  0);
*/
#endif

#ifdef littlefs
/*
 * These are function prototypes for the helper functions.
 * Declared static so they are only visible in this file.
 * The functions are defined down below.
 * Not in their original location which was inside another function.
 */
static int sync_HAL(const struct lfs_config *c);
static int erase_HAL(const struct lfs_config *c, lfs_block_t block);
static int prog_HAL(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, const void *buffer, lfs_size_t size);
static int read_HAL(const struct lfs_config *c, lfs_block_t block,
        lfs_off_t off, void *buffer, lfs_size_t size);

static lfs_cache_t lfs_read_cache;
static lfs_cache_t lfs_write_cache;

static struct lfs_config lfs_cfg;
static lfs_t lfs_internal_flash;
/*
extern lfs_internal_flash;	Carsten:
This is an incorrect declaration. Extern and static ONLY determine scope
 - extern tells the compiler to make the declaration visible outside the file.
 - static tells it not to.
The type is missing so it doesn't work.
 You get a compiler warning because in the "C" language standard a missing
  type declaration implies int.
  static lfs_t lfs_internal_flash; // the real deal
*/

// Initialize structs and call lfs_format then lfs_mount
// I could not call lfs_format or lfs_mount from main,
// so workaround is this wrapper.
int lfs_PSTAT_init(void)
{
    // Configure the lfs_config struct
    lfs_cfg.read_size = LFS_BUFFERS_SIZE;
    lfs_cfg.prog_size = LFS_BUFFERS_SIZE;
    lfs_cfg.block_size = LFS_BUFFERS_SIZE * 250;	// 128k this is the sector erase size.
    lfs_cfg.block_count = 4;
    lfs_cfg.block_cycles = 10000; // Number of erase cycles before we should move data to another block.
    lfs_cfg.cache_size = LFS_BUFFERS_SIZE; // littlefs needs a read cache, a program cache, and one additional
    // cache per file. Optional statically allocated read buffer. Must be cache_size.
    lfs_cfg.lookahead_size = LOOKAHEAD_BUFFER_SIZE; // stored as a compact bitmap,
    // so each byte of RAM can track 8 blocks. Blocks are 512k. There are 250 in a erasable sector.
    // The code says: must be multiple of 64-bits
    // Don't know requirements for this yet. Aren't blocks allocated?
    // So how many blocks should this be tracking? Right now allocating nominal values.

    // The buffers read, prog and lookahead are not populated in the structure,
    // so they are malloced in the functions that use them.

    /* It's much simpler than that: */
    lfs_cfg.read = read_HAL;
    lfs_cfg.prog = prog_HAL;
    lfs_cfg.erase = erase_HAL;
    lfs_cfg.sync = sync_HAL;

#if 0
    printf("lfs_cfg.read_size =  %x\n", lfs_cfg.read_size);

    printf("Debug/Output.map =.text.read_HAL = 0x08006df2\n");
    printf("lfs_cfg.read =  %x\n", lfs_cfg.read);

    printf("Debug/Output.map =.text.prog_HAL = 0x08006de6\n");
    printf("lfs_cfg.read =  %x\n", lfs_cfg.prog);

    printf("Debug/Output.map =.text.erase_HAL = 0x08006dea\n");
    printf("lfs_cfg.read =  %x\n", lfs_cfg.erase);

    printf("Debug/Output.map =.text.sync_HAL = 0x08006dee\n");
    printf("lfs_cfg.read =  %x\n", lfs_cfg.sync);
#endif

    // Below are optional. file_max is useful. Name length handled by other attributes.
    // Still needs to be set to 0.
    lfs_cfg.name_max = 0; // This is optional. The size of the info struct
    // is controlled by the LFS_NAME_MAX define.
    // Defaults to LFS_NAME_MAX when zero. LFS_NAME_MAX is stored in
    // superblock and must be respected by other littlefs drivers.
    lfs_cfg.file_max = 0;
    lfs_cfg.attr_max = 0;

    // Configure the lookahead buffers, do they need to be static?

    // Configure the read cache
    lfs_read_cache.block = 1; // fudge
    lfs_read_cache.buffer = USER_read_buffer; // rename. NOTWORKING
    lfs_read_cache.off = 0; // fudge
    lfs_read_cache.size = LFS_BUFFERS_SIZE; // not sure caches are same size as 512k blocks~!

    // Configure the write cache
    lfs_write_cache.block = 1; // fudge
    lfs_write_cache.buffer = USER_write_buffer; // rename
    lfs_write_cache.off = 0; // fudge
    lfs_write_cache.size = LFS_BUFFERS_SIZE;

    // Add the read and write cache to lfs_internal_flash struct
    lfs_internal_flash.pcache = lfs_write_cache; // Verify this is write cache
    lfs_internal_flash.rcache = lfs_read_cache;

// Make a test to see if lfs has been created.
    // Returns a negative error code on failure.
    int lfs_format_status =  lfs_format(&lfs_internal_flash, &lfs_cfg);
    if (lfs_format_status) return lfs_format_status;

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
	/*
	 * I created static unsigned char USER_read_buffer[LFS_BUFFERS_SIZE];
	 * It can be sent thru a queue to the fatfs API managing task.
	 * Yet a read function could also return the source address,
	 * for the read function only, so who needs the buffer?
	 * any who using memcpy to buffer ~ to be declared in caller, later.
	 */
	// FLASH            0x08000000         0x00100000         xr TOP of Flash = 0x8100000
	// using 524288 bytes for flash at an offset of 0x0 to 0x8000, so FLASH_SECTOR_8 to 0x8100000
	uint8_t *flashSource = (uint8_t *) FLASH_SECTOR_8;	// Set to the start of the flash *sector* addresses.


	    /* Move to the start of the sector being read. */
	    flashSource += (( LFS_BUFFERS_SIZE * block) + off );

//void * memcpy (void *__restrict, const void *__restrict, size_t);
	    memcpy( ( void * ) buffer,
	            ( void * ) flashSource,
	            ( size_t ) ( size ) );
// memcopy does not have a return value. A compare would be needed to verify a valid read?
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
/*
 * Block should = 512 byte section of a sector?
 * Erase a block. A block must be erased before being programmed.
   The state of an erased block is undefined. Negative error codes
   are propogated to the user.
   May return LFS_ERR_CORRUPT if the block should be considered bad.
   So check whole sector for correct erasure.
 *
 */
	// ASSERT that block is between 0 (first) and last block (999)
	LFS_ASSERT(block <= 999);
	int block_in_Sector;
	if (block <= 0 && block <= 249)
		block_in_Sector = 0;
	else if (block <= 250 && block <= 499)
		block_in_Sector = 1;
	else if (block <= 500 && block <= 749)
		block_in_Sector = 2;
	else if (block <= 750 && block <= 999)
		block_in_Sector = 3;

	int sector;			// int * used to verify sector is erased.
	int sector_number;	// enum'd arg to FLASH_Erase_Sector().
	// Use a tool to write something and then verify it gets erased.
	switch(block_in_Sector) {

	   case (0)  :
		   sector = SECTOR08_ADDR;	// FLASH_SECTOR_8 to FLASH_SECTOR_9
	   	   sector_number = FLASH_SECTOR_8;
	      break; /* optional */

	   case (1)  :
		   sector = SECTOR09_ADDR;	// FLASH_SECTOR_9 to FLASH_SECTOR_10
	   	   sector_number = FLASH_SECTOR_9;
	      break; /* optional */

	   case (2)  :
		   sector = SECTOR10_ADDR;	// FLASH_SECTOR_10 to FLASH_SECTOR_11
	   	   sector_number = FLASH_SECTOR_10;
	      break; /* optional */

	   case (3)  :
		   sector = SECTOR11_ADDR;	// FLASH_SECTOR_11 to 0x8100000
	   	   sector_number = FLASH_SECTOR_11;
	      break; /* optional */

	   /* you can have any number of case statements */
	   default : /* Optional */
	   ;
	}

	    // HAL_FLASH_Unlock(); The lock is hanging. The flash is unprotected so should be fine.
	    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGSERR );
	    FLASH_Erase_Sector(sector_number, VOLTAGE_RANGE_3);	// sector_number is a enum.
	    // HAL_FLASH_Lock();	// this hangs.

	    int *word = (int *) sector;
	    printf("Erase verify started at %x\n", (unsigned int) word);
	    for( word; word < (int *) (sector + SECTOR_SIZE); word++ ){
/* unit test passed
	    	printf("word in for loop = %x\n", (unsigned int) word);

	    	if (*word != ( int * )0xFFFFFFFF)
	    	   return LFS_ERR_CORRUPT;
	    	if (word == ( int * ) FLASH_SECTOR_9)
	    	   // loop is wrong
	    	   printf("Reached end of erase verify loop %x/n", (unsigned int) word);
*/
	    }
	    printf("Erase verify loop ended at word = %x\n", (unsigned int) word);

	    /*	Block 0 erase results
	     * 	Erase verify started at 8080000
			Erase verify loop ended at word = 80a0000


	     */
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

