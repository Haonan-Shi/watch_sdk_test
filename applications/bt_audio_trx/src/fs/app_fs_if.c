/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_fs_if.h"
#include "trace.h"
#include "os_mem.h"
#include "wdg.h"
#include "app_report.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <zephyr/storage/disk_access.h>

typedef enum
{
    APP_TIMER_DISK_POWER_DOWN,
} T_APP_DISK_TIMER;

#define POWER_DOWN_DISK_TIME                                3500 //3s 

static struct fs_mount_t mp_fatfs =
{
    .type = FS_FATFS,
    .mnt_point = FATFS_ROOT_PATH,
};

void app_fs_if_report_file_name(T_FILE_HANDLE *fil_hdl)
{
    size_t full_len = strlen(fil_hdl->filename);

    if (full_len > (FF_MAX_LFN - 1))
    {
        full_len = FF_MAX_LFN - 1;
    }

    uint8_t rpt[FF_MAX_LFN];

    rpt[0] = (uint8_t)full_len;
    memcpy(&rpt[1], fil_hdl->filename, full_len);

    app_report_event(CMD_PATH_UART, EVENT_FS_FILE_INFO, 0, rpt, full_len + 1);
}

static bool audio_fs_mounted = false;

void app_fs_if_list_files(const char *filter_ext, FileFoundCallback cb, void *context)
{
    struct fs_dir_t dirp;
    struct fs_dirent entry;
    int res;

    APP_PRINT_INFO1("app_fs_if_list_files: Listing files in %s ---", TRACE_STRING(AUDIO_FILE_PATH));

    fs_dir_t_init(&dirp);
    res = fs_opendir(&dirp, AUDIO_FILE_PATH);
    if (res)
    {
        APP_PRINT_ERROR1("app_fs_if_list_files: opendir failed: %d", res);
        return;
    }

    while (1)
    {
        res = fs_readdir(&dirp, &entry);
        if (res || entry.name[0] == 0) { break; }

        if (entry.type != FS_DIR_ENTRY_FILE) { continue; }

        if (filter_ext != NULL)
        {
            char *dot = strrchr(entry.name, '.');
            if (!dot || strcasecmp(dot, filter_ext) != 0) { continue; }
        }

        APP_PRINT_INFO2("[FILE] %s (%d bytes)", TRACE_STRING(entry.name), entry.size);

        if (cb)
        {
            if (!cb(entry.name, entry.size, context)) { break; }
        }
    }
    fs_closedir(&dirp);
}

int app_fs_mkfs_mount(void)
{
    int res = 0;

    fs_deinit(&mp_fatfs);

    if (mp_fatfs.fs_data == NULL)
    {
        mp_fatfs.fs_data = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(FATFS));
        if (mp_fatfs.fs_data == NULL)
        {
            return -1;
        }
    }

    /* format "0:" */
    res = fs_mkfs(mp_fatfs.type, (uintptr_t)mp_fatfs.mnt_point, 0, 0);
    if (res != 0)
    {
        APP_PRINT_ERROR1("app_fs_mkfs_mount: mkfs fail, res:%d", res);
        os_mem_free(mp_fatfs.fs_data);
        mp_fatfs.fs_data = NULL;
    }
    else
    {
        res = fs_init(&mp_fatfs);
        if (res != 0)
        {
            APP_PRINT_ERROR1("app_fs_mkfs_mount: mount failed!, res:%d", res);
            return -2;
        }
        audio_fs_mounted = true;
    }
    APP_PRINT_INFO0("app_fs_mkfs_mount: mkfs ok");

    return 0;
}

int app_fs_free_space(uint32_t *pfreeSpace)
{
    int res = 0;
    struct fs_statvfs stat;
    res = fs_statvfs(mp_fatfs.mnt_point, &stat);
    if (res == 0)
    {
        *pfreeSpace = stat.f_bfree * stat.f_frsize;
    }
    else
    {
        APP_PRINT_ERROR1("ERROR: fs_free_space, res:%d", res);
    }
    APP_PRINT_INFO1("fs_free_space: freeSpace:0x%x", *pfreeSpace);
    return res;
}

int app_fs_get_space_info(uint32_t *ptotalSpace, uint32_t *pfreeSpace)
{
    int res = 0;
    struct fs_statvfs stat;
    res = fs_statvfs(mp_fatfs.mnt_point, &stat);
    if (res == 0)
    {
        *ptotalSpace = stat.f_blocks * stat.f_frsize;
        *pfreeSpace = stat.f_bfree * stat.f_frsize;
    }
    else
    {
        APP_PRINT_ERROR1("ERROR: fs_get_space_info, res:%d", res);
    }
    return res;
}

T_FILE_HANDLE *app_fs_open_file_with_timestamp(const char *file_name, uint8_t mode)
{
    char filename[64];
    uint32_t ts = sys_timestamp_get();
    T_FILE_HANDLE *check_hdl = NULL;

    snprintf(filename, sizeof(filename), "%u_%s", ts, file_name);

    check_hdl = app_fs_open_file(filename, FS_O_READ);

    if (check_hdl != NULL)
    {
        app_fs_close_file(check_hdl);
        check_hdl = NULL;

        uint16_t suffix = 1;

        do
        {
            if (suffix > 999)
            {
                APP_PRINT_ERROR1("app_fs_open_file_with_timestamp: Failed to generate unique name %s",
                                 TRACE_STRING(file_name));
                return NULL;
            }

            snprintf(filename, sizeof(filename), "%u_(%d)_%s", ts, suffix++, file_name);

            check_hdl = app_fs_open_file(filename, FS_O_READ);

            if (check_hdl != NULL)
            {
                app_fs_close_file(check_hdl);
            }
            else
            {
                break;
            }
        }
        while (1);
    }

    APP_PRINT_INFO1("Opening file: %s", TRACE_STRING(filename));
    return app_fs_open_file(filename, mode);
}

T_FILE_HANDLE *app_fs_open_file(const char *fil_name, uint8_t mode)
{
    int res = 0;

    char *full_file_path;
    char *dir_path = (char *)AUDIO_FILE_PATH;
    uint16_t total_len = strlen(fil_name) + strlen(dir_path) + 2; // for '/' and '\0'

    full_file_path = os_mem_zalloc(RAM_TYPE_DATA_ON, total_len);
    snprintf(full_file_path, total_len, "%s%s", dir_path, fil_name);

    T_FILE_HANDLE *fil_hdl = os_mem_alloc(RAM_TYPE_DATA_ON, sizeof(T_FILE_HANDLE));
    if (fil_hdl == NULL)
    {
        os_mem_free(full_file_path);
        return NULL;
    }
    fs_file_t_init(&fil_hdl->fil);
    res = fs_open(&fil_hdl->fil, full_file_path, mode);
    if (res != 0)
    {
        fs_close(&fil_hdl->fil);
        os_mem_free(fil_hdl);
        os_mem_free(full_file_path);
        return NULL;
    }
    else
    {
        uint16_t len = strlen(fil_name) + 1;// +1 for '\0'
        if (len > FF_MAX_LFN)
        {
            len = FF_MAX_LFN;
        }
        memcpy(fil_hdl->filename, fil_name, len);
        fil_hdl->namelen = len;
        os_mem_free(full_file_path);
        fil_hdl->bytes_since_sync = 0;
        return fil_hdl;
    }
}

int app_fs_close_file(T_FILE_HANDLE *fil_hdl)
{
    int res = 0;

    res = fs_close(&fil_hdl->fil);
    os_mem_free(fil_hdl);
    return res;
}

int app_fs_read(T_FILE_HANDLE *fil_hdl, uint8_t *readBuf, uint32_t readLen)
{
    int res = 0;
    res = fs_read(&fil_hdl->fil, readBuf, readLen);
    if (res < 0)
    {
        APP_PRINT_ERROR1("app_fs_read: read file failed! res:0x%x", res);
    }

    return res;
}

ssize_t app_fs_write(T_FILE_HANDLE *fil_hdl, uint8_t *writeBuf, uint32_t writeLen)
{
    int res = 0;

    res = fs_write(&fil_hdl->fil, writeBuf, writeLen);
    if (res < 0)
    {
        APP_PRINT_ERROR1("app_fs_write: write file failed! res:0x%x", res);
    }

    fil_hdl->bytes_since_sync += res;

    if (fil_hdl->bytes_since_sync >= FS_SYNC_THRESHOLD)
    {
        int sync_res = fs_sync(&fil_hdl->fil);
        if (sync_res == 0)
        {
            fil_hdl->bytes_since_sync = 0;
        }
        else
        {
            APP_PRINT_ERROR1("app_fs_write: fs_sync failed! res:%d", sync_res);
        }
    }

    return res;
}

int app_fs_seek(T_FILE_HANDLE *fil_hdl, uint32_t offset)
{
    int res = 0;

    if (fil_hdl == NULL)
    {
        return -1;
    }

    res = fs_seek(&fil_hdl->fil, offset, FS_SEEK_SET);

    if (res < 0)
    {
        APP_PRINT_ERROR2("app_fs_seek: fail res:%d, offset:%u", res, offset);
    }

    return res;
}

uint32_t app_fs_tell(T_FILE_HANDLE *fil_hdl)
{
    off_t pos = 0;

    if (fil_hdl == NULL)
    {
        return 0;
    }

    pos = fs_tell(&fil_hdl->fil);

    if (pos < 0)
    {
        APP_PRINT_ERROR1("app_fs_tell: fail res:%d", (int)pos);
        return 0;
    }

    return (uint32_t)pos;
}

uint32_t app_fs_size(T_FILE_HANDLE *fil_hdl)
{
    size_t size = 0;
    fs_sync(&fil_hdl->fil);
    int res = fs_size(&fil_hdl->fil, &size);
    if (res != 0)
    {
        APP_PRINT_ERROR1("Failed to get file size: %d", res);
    }
    return size;

}

uint8_t *app_fs_get_filename(T_FILE_HANDLE *fil_hdl)
{
    return (uint8_t *)fil_hdl->filename;
}

uint16_t app_fs_get_filename_len(T_FILE_HANDLE *fil_hdl)
{
    return fil_hdl->namelen;
}

int app_fs_unlink_file(const char *filename)
{
    int res = 0;
    char *full_file_path;
    char *dir_path = (char *)AUDIO_FILE_PATH;
    uint16_t total_len = strlen(filename) + strlen(dir_path) + 2; // for '/' and '\0'

    full_file_path = os_mem_zalloc(RAM_TYPE_DATA_ON, total_len);
    snprintf(full_file_path, total_len, "%s%s", dir_path, filename);

    WDG_Kick();
    res = fs_unlink(full_file_path);
    os_mem_free(full_file_path);
    return res;
}

int32_t app_fs_create_folder(const char *folder_name)
{
    struct fs_dirent entry;
    int32_t ret;

    ret = fs_stat(folder_name, &entry);

    if (ret == 0)
    {
        if (entry.type == FS_DIR_ENTRY_DIR)
        {
            APP_PRINT_INFO1("app_fs_create_folder: '%s' exists", TRACE_STRING(folder_name));
            return ret;
        }
        else
        {
            APP_PRINT_INFO1("app_fs_create_folder: '%s' exists but is a FILE, deleting...",
                            TRACE_STRING(folder_name));
            fs_unlink(folder_name);
        }
    }

    APP_PRINT_INFO1("app_fs_create_folder: create '%s'", TRACE_STRING(folder_name));
    return fs_mkdir(folder_name);
}

int32_t app_fs_init(void)
{
    uint8_t status = FS_STATUS_MOUNT_OK;

    APP_PRINT_INFO1("AUDIO_FILE_PATH is %s", TRACE_STRING(AUDIO_FILE_PATH));
    if (fs_init(&mp_fatfs) != 0)
    {
        APP_PRINT_ERROR1("app fs mount disk error: disk root path %s", TRACE_STRING(mp_fatfs.mnt_point));
#if F_APP_FS_FORMAT_SUPPORT
        status = FS_STATUS_MOUNT_FAIL;
        app_report_event(CMD_PATH_UART, EVENT_FS_MOUNT_STATUS, 0, &status, sizeof(status));
#endif
        return -1;
    }
    audio_fs_mounted = true;
    app_fs_create_folder(AUDIO_FILE_PATH);
#if F_APP_FS_FORMAT_SUPPORT
    app_report_event(CMD_PATH_UART, EVENT_FS_MOUNT_STATUS, 0, &status, sizeof(status));
#endif
    return 0;
}

#if F_APP_FS_FORMAT_SUPPORT
int32_t app_fs_format(uint32_t opt)
{
    int res;
    uint8_t status;
    void *fs_buf = NULL;

    /* allocate FATFS work area if not already present */
    if (mp_fatfs.fs_data == NULL)
    {
        fs_buf = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(FATFS));
        if (fs_buf == NULL)
        {
            status = FS_STATUS_FORMAT_FAIL;
            app_report_event(CMD_PATH_UART, EVENT_FS_MOUNT_STATUS, 0, &status, sizeof(status));
            return -1;
        }
        mp_fatfs.fs_data = fs_buf;
    }

    MKFS_PARM mkfs_parm =
    {
        .fmt     = (uint8_t)opt,
        .n_fat   = 1,
        .align   = 0,
        .n_root  = 0,
        .au_size = 0,
    };

    res = fs_mkfs(mp_fatfs.type, (uintptr_t)mp_fatfs.mnt_point, &mkfs_parm, 0);
    if (res != 0)
    {
        APP_PRINT_ERROR2("app_fs_format: mkfs fail, opt=0x%x res=%d", opt, res);
        os_mem_free(mp_fatfs.fs_data);
        mp_fatfs.fs_data = NULL;
        audio_fs_mounted = false;
        status = FS_STATUS_FORMAT_FAIL;
        app_report_event(CMD_PATH_UART, EVENT_FS_MOUNT_STATUS, 0, &status, sizeof(status));
        return -2;
    }

    fs_deinit(&mp_fatfs);

    mp_fatfs.fs_data = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(FATFS));
    if (mp_fatfs.fs_data == NULL)
    {
        status = FS_STATUS_FORMAT_FAIL;
        app_report_event(CMD_PATH_UART, EVENT_FS_MOUNT_STATUS, 0, &status, sizeof(status));
        return -3;
    }

    res = fs_init(&mp_fatfs);
    if (res != 0)
    {
        APP_PRINT_ERROR2("app_fs_format: mount after format failed, opt=0x%x res=%d", opt, res);
        os_mem_free(mp_fatfs.fs_data);
        mp_fatfs.fs_data = NULL;
        audio_fs_mounted = false;
        status = FS_STATUS_FORMAT_FAIL;
        app_report_event(CMD_PATH_UART, EVENT_FS_MOUNT_STATUS, 0, &status, sizeof(status));
        return -4;
    }

    audio_fs_mounted = true;
    app_fs_create_folder(AUDIO_FILE_PATH);
    APP_PRINT_INFO0("app_fs_format: format and mount ok");

    status = FS_STATUS_FORMAT_OK;
    app_report_event(CMD_PATH_UART, EVENT_FS_MOUNT_STATUS, 0, &status, sizeof(status));
    return 0;
}
#endif