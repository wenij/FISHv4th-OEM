
/* Structure that contains fields common to all media drivers, and can be
extended to contain additional fields to tailor it for use with a specific media
type. */
struct xFFDisk
{
    struct
    {
        /* Flags that can optionally be used by the media driver to ensure the
        disk has been initialised, registered and mounted before it is accessed. */
        uint32_t bIsInitialised : 1;
        uint32_t bIsRegistered : 1;
        uint32_t bIsMounted : 1;
        uint32_t spare0 : 5;

        /* The partition number on the media described by this structure. */
        uint32_t bPartitionNumber : 8;
        uint32_t spare1 : 16;
    } xStatus;

    /* Provided to allow this structure to be extended to include additional
    attributes that are specific to a media type. */
    void *pvTag;

    /* Points to input and output manager used by the disk described by this
    structure. */
    FF_IOManager_t *pxIOManager;

    /* The number of sectors on the disk. */
    uint32_t ulNumberOfSectors;

    /* Field that can optionally be set to a signature that is unique to the
    media.  Read and write functions can check the ulSignature field to validate
    the media type before they attempt to access the pvTag field, or perform any
    read and write operations. */
    uint32_t ulSignature;
};

typedef struct xFFDisk FF_Disk_t;
