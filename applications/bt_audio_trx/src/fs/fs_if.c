/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdio.h>
#include <zephyr/storage/disk_access.h>
#include "fs_if.h"
#include "pm.h"
#include "trace.h"
#include "string.h"
#include "os_mem.h"
#include "diskio.h"
#include "wdg.h"
#include "app_fs_if.h"

T_OS_MEM_TYPE    fs_ram_type = OS_MEM_TYPE_DATA;

int seek_read(struct fs_file_t *fp, off_t ofs, void *buff, size_t btr)
{
    int fr = 0;

    fr = fs_seek(fp, ofs, FS_SEEK_SET);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_read seek fail res %d, offset 0x%x, btr 0x%x, fp 0x%x",
                         fr, ofs, btr, fp);
        return fr;
    }

    fr = fs_read(fp, buff, btr);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_read read fail res %d, offset 0x%x, btr 0x%x, fp 0x%x",
                         fr, ofs, btr, fp);
        return fr;
    }

    return fr;
}

int seek_write(struct fs_file_t *fp, off_t ofs, void *buff, size_t btw)
{
    int fr = 0;
    size_t size = 0;

    //zephyr fat_fs.c does not allow ofs > file size when called fs_seek
    fr = fs_size(fp, &size);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write get size fail res %d, offset 0x%x, btw 0x%x, fp 0x%x",
                         fr, ofs, btw, fp);
        return fr;
    }

    if (ofs > size)
    {
        fr = fs_truncate(fp, ofs);
    }
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write truncate fail res %d, offset 0x%x, btw 0x%x, fp 0x%x",
                         fr, ofs, btw, fp);
    }

    fr = fs_seek(fp, ofs, FS_SEEK_SET);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write seek fail res %d, offset 0x%x, btw 0x%x, fp 0x%x",
                         fr, ofs, btw, fp);
        return fr;
    }

    fr = fs_write(fp, buff, btw);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write write fail res %d, offset 0x%x, btw 0x%x, fp 0x%x",
                         fr, ofs, btw, fp);
        return fr;
    }

    return fr;
}

size_t uint16_strlen(const uint16_t *str)
{
    const uint16_t *s = str;
    while (*s != 0)
    {
        ++s;
    }
    return s - str;
}

int uint16_strcmp(const uint16_t *str1, const uint16_t *str2)
{
    while (*str1 != 0 && *str2 != 0)
    {
        if (*str1 != *str2)
        {
            return *str1 - *str2;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}

int fs_init(struct fs_mount_t *mp)
{
    int res = 0;

    if (mp->fs_data == NULL)
    {
        mp->fs_data = os_mem_alloc(fs_ram_type, sizeof(FATFS));
        if (mp->fs_data == NULL)
        {
            res = -1;
            goto Error1;
        }
    }

    res = fs_mount(mp);
    if (0 != res)
    {
        res = -2;
        goto mount_fail;
    }

    if (0 != fs_chdrive(mp->mnt_point))
    {
        res = -3;
        goto mount_fail;
    }

    return res;

mount_fail:
    os_mem_free(mp->fs_data);
    mp->fs_data = NULL;
Error1:
    APP_PRINT_ERROR1("ERROR: fs_init res = %d", res);
    return res;
}

int fs_deinit(struct fs_mount_t *mp)
{

    if (0 != fs_unmount(mp))
    {
        return -1;
    }
    if (mp->fs_data)
    {
        os_mem_free(mp->fs_data);
        mp->fs_data = NULL;
    }
    return 0;
}


int32_t disk_power_off(void)
{
    if (disk_access_ioctl(FATFS_DISK_NAME, DISK_IOCTL_CTRL_DEINIT, NULL) != 0)
    {
        return -1;
    }
    return 0;
}

int32_t disk_power_on(void)
{
    if (disk_access_ioctl(FATFS_DISK_NAME, DISK_IOCTL_CTRL_INIT, NULL) != 0)
    {
        return -1;
    }
    return 0;
}