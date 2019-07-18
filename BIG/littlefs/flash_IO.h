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
// Create wrapper prototypes for
// lfs_PSTAT_init that will call lsf_format and lfs_mount
//int lfs_PSTAT_init(lfs_t *lfs, const struct lfs_config *cfg);
// default 2nd arg workaround
//int lfs_PSTAT_init(lfs_t *lfs);
int lfs_PSTAT_init();

#endif /* FLASH_IO_H_ */
