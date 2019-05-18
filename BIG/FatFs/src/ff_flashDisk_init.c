#include "ff_disk_t.h"
/* The size of each sector on the disk. */
#define flashSECTOR_SIZE                512UL

/* Only a single partition is used.  Partition numbers start at 0. */
#define flashPARTITION_NUMBER            0
  char diskName[] = {'p','S','T','A','T'};
  char *pcName = diskName;
  //make a const pointer of this?
  //FF_Disk_t *
 // FF_Disk_t *FF_FlashDiskInit
 // FF_Disk_t * (void *) p0Disk;
 // see if including FF_Disk_t header resolves
  FF_Disk_t * p0Disk;
  p0Disk = FF_FlashDiskInit( pcName,
		  //uint8_t *pucDataBuffer,
		  (uint8_t *) 0x8080000,	// beginning of 1000 sectors of flash disk
          //uint32_t ulSectorCount,
		  1000ul,
          //size_t xIOManagerCacheSize )
							  0);

/*
In this example:
  - pcName is the name to give the disk within FreeRTOS+FAT's virtual file system.
  - pucDataBuffer is the start of the RAM buffer used as the disk.
  - ulSectorCount is effectively the size of the disk, each sector is 512 bytes.
  - xIOManagerCacheSize is the size of the IO manager's cache, which must be a
   multiple of the sector size, and at least twice as big as the sector size.
*/
FF_Disk_t *FF_FlashDiskInit( char *pcName,
                          uint8_t *pucDataBuffer,
                          uint32_t ulSectorCount,
                          size_t xIOManagerCacheSize )
{
FF_Error_t xError;
FF_Disk_t *pxDisk = NULL;
FF_CreationParameters_t xParameters;

    /* Check the validity of the xIOManagerCacheSize parameter. */
    configASSERT( ( xIOManagerCacheSize % flashSECTOR_SIZE ) == 0 );
    configASSERT( ( xIOManagerCacheSize >= ( 2 * flashSECTOR_SIZE ) ) );

    /* Attempt to allocated the FF_Disk_t structure. */
    pxDisk = ( FF_Disk_t * ) pvPortMalloc( sizeof( FF_Disk_t ) );

    if( pxDisk != NULL )
    {
        /* It is advisable to clear the entire structure to zero after it has been
        allocated - that way the media driver will be compatible with future
        FreeRTOS+FAT versions, in which the FF_Disk_t structure may include
        additional members. */
        memset( pxDisk, '\0', sizeof( FF_Disk_t ) );

        /* The pvTag member of the FF_Disk_t structure allows the structure to be
        extended to also include media specific parameters.  The only media
        specific data that needs to be stored in the FF_Disk_t structure for a
        RAM disk is the location of the RAM buffer itself - so this is stored
        directly in the FF_Disk_t's pvTag member. */
        pxDisk->pvTag = ( void * ) pucDataBuffer;

        /* The signature is used by the disk read and disk write functions to
        ensure the disk being accessed is a RAM disk. */
        pxDisk->ulSignature = flashSIGNATURE;

        /* The number of sectors is recorded for bounds checking in the read and
        write functions. */
        pxDisk->ulNumberOfSectors = ulSectorCount;

        /* Create the IO manager that will be used to control the RAM disk -
        the FF_CreationParameters_t structure completed with the required
        parameters, then passed into the FF_CreateIOManager() function. */
        memset (&xParameters, '\0', sizeof xParameters);
        xParameters.pucCacheMemory = NULL;
        xParameters.ulMemorySize = xIOManagerCacheSize;
        xParameters.ulSectorSize = flashSECTOR_SIZE;
        xParameters.fnWriteBlocks = prvWriteFLASH;
        xParameters.fnReadBlocks = prvReadFLASH;
        xParameters.pxDisk = pxDisk;

        /* The driver is re-entrant as it just accesses RAM using memcpy(), so
        xBlockDeviceIsReentrant can be set to pdTRUE.  In this case the
        semaphore is only used to protect FAT data structures, and not the read
        and write function. */
        xParameters.pvSemaphore = ( void * ) xSemaphoreCreateRecursiveMutex();
        xParameters.xBlockDeviceIsReentrant = pdTRUE;


        pxDisk->pxIOManager = FF_CreateIOManger( &xParameters, &xError );

        if( ( pxDisk->pxIOManager != NULL ) && ( FF_isERR( xError ) == pdFALSE ) )
        {
            /* Record that the RAM disk has been initialised. */
            pxDisk->xStatus.bIsInitialised = pdTRUE;

            /* Create a partition on the RAM disk.  NOTE!  The disk is only
            being partitioned here because it is a new RAM disk.  It is
            known that the disk has not been used before, and cannot already
            contain any partitions.  Most media drivers will not perform
            this step because the media will already been partitioned and
            formatted. */
            xError = prvPartitionAndFormatDisk( pxDisk );

            if( FF_isERR( xError ) == pdFALSE )
            {
                /* Record the partition number the FF_Disk_t structure is, then
                mount the partition. */
                pxDisk->xStatus.bPartitionNumber = flashPARTITION_NUMBER;

                /* Mount the partition. */
                xError = FF_Mount( pxDisk, flashPARTITION_NUMBER );
            }

            if( FF_isERR( xError ) == pdFALSE )
            {
                /* The partition mounted successfully, add it to the virtual
                file system - where it will appear as a directory off the file
                system's root directory. */
                FF_FS_Add( pcName, pxDisk->pxIOManager );
            }
        }
        else
        {
            /* The disk structure was allocated, but the disk's IO manager could
            not be allocated, so free the disk again. */
            FF_FLASHDiskDelete( pxDisk );
            pxDisk = NULL;
        }
    }

    return pxDisk;
}
