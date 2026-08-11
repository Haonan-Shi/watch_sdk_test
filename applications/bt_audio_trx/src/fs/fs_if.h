/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _FS_IF_H_
#define _FS_IF_H_

/*============================================================================*
  *                               Header Files
  *============================================================================*/
#include "rtl876x.h"
#include "ff.h"
#include <zephyr/fs/fs.h>
#include "diskio.h"
#include "os_mem.h"
#include <stdbool.h>
/*============================================================================*
  *                                  Variables
  *============================================================================*/

typedef struct
{
    struct fs_file_t         fil;
    char              filename[FF_MAX_LFN];
    uint16_t          namelen;
    uint32_t          bytes_since_sync;
} T_FILE_HANDLE;


/*============================================================================*
  *                                Functions
  *============================================================================*/
void fs_set_mem_alloc_type(T_OS_MEM_TYPE ram_type);

int seek_read(struct fs_file_t *fp, off_t ofs, void *buff, UINT btr);

int seek_write(struct fs_file_t *fp, off_t ofs, void *buff, UINT btw);

size_t uint16_strlen(const uint16_t *str);

int uint16_strcmp(const uint16_t *str1, const uint16_t *str2);

int fs_deinit(struct fs_mount_t *mp);

int fs_init(struct fs_mount_t *mp);

int32_t disk_power_on(void);

int32_t disk_power_off(void);

#endif //_FS_IF_H_
