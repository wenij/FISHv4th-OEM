/* Prototype for disk initialization functions */

//FF_Disk_t
void * *FF_FlashDiskInit( char *pcName,
                          uint8_t *pucDataBuffer,
                          uint32_t ulSectorCount,
                          size_t xIOManagerCacheSize );
