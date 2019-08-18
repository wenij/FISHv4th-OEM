/*
 * flash_IO.h
 *
 *  Created on: Jul 7, 2019
 *      Author: Clyde W Phillips Jr
 */

#ifndef FLASH_IO_H_
#define FLASH_IO_H_

/// Version info ///

// Alpha version
#include "lfs.h"
/* Can statics be declared in an include file?
 * If you want a global variable, you need to use extern,
 *  not static and make the actual declaration (without extern)
 *   with the initial value in one of your c files.
static struct lfs_config lfs_cfg;
static lfs_t lfs_internal_flash;
*/
// Create wrapper prototypes for
// lfs_PSTAT_init that will call lsf_format and lfs_mount
//int lfs_PSTAT_init(lfs_t *lfs, const struct lfs_config *cfg);
// default 2nd arg workaround
//int lfs_PSTAT_init(lfs_t *lfs);
int lfs_PSTAT_init();
/* implement read
    // Read a region in a block. Negative error codes are propagated
    // to the user.
int (*read)(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
Debug an argument at a time
*/
int read_HAL(lfs_config, block, off, buffer, size);

#endif /* FLASH_IO_H_ */
