/*
 * flash_IO.h
 *
 *  Created on: Jul 7, 2019
 *      Author: Clyde W Phillips Jr
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FLASH_IO_H_
#define FLASH_IO_H_

 /* Scheduler includes.
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "pstat.h"
*/
#include "lfs.h"

// Wrappers for calling from main, before bundling into tasks.
extern int lfs_PSTAT_STATIC_init(void);
extern int lfs_PSTAT_format(void);
extern int lfs_PSTAT_mount(void);
extern int lfsopen( lfs_file_t *file, const char *path, int flags);
extern lfs_ssize_t lfsread( lfs_file_t *file, void *buffer, lfs_size_t size);
extern lfs_ssize_t lfswrite( lfs_file_t *file, void *buffer, lfs_size_t size);
extern int lfsflush( lfs_file_t *file);
extern int lfsclose( lfs_file_t *file);

#endif /* FLASH_IO_H_ */
