/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdio.h>
#include "app_fs_if.h"
#include "trace.h"
#include "app_mmi.h"
#include "app_timer.h"
#include "app_usb.h"
#include "app_task.h"
#include "playback_playlist.h"
#include "os_mem.h"
#include "app_dlps.h"
#include "audio_resource.h"
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

#if DT_NODE_HAS_STATUS(DT_NODELABEL(flash_disk1), okay)
static struct fs_mount_t mp_romfs =
{
    .type = FS_TYPE_EXTERNAL_BASE,
    .mnt_point = ROMFS_ROOT_PATH,
    .fs_data = CONFIG_FLASH_BASE_ADDRESS + DT_REG_ADDR(DT_NODELABEL(partition_userdata2)),
    .flags = FS_MOUNT_FLAG_READONLY,
};
#endif

const char ext_MP3[] = ".MP3";
const char ext_mp3[] = ".mp3";
const char ext_TXT[] = ".TXT";
const char ext_txt[] = ".txt";
const char ext_DAT[] = ".DAT";
const char ext_dat[] = ".dat";
const char ext_BIN[] = ".BIN";
const char ext_bin[] = ".bin";

const T_EXTENSION_DEF ext_def_array[8] =
{
    {0x00, ext_MP3, ext_mp3},
    /*0x01~0x04 the audio function is not supported */
    // {0x01, _T(".MP4"), _T(".mp4")},
    // {0x02, _T(".RTK"), _T(".rtk")},
    // {0x03, _T(".AAC"), _T(".aac")},
    // {0x04, _T(".FLAC"), _T(".flac")},
    {0x05, ext_TXT, ext_txt},
    {0x06, ext_DAT, ext_dat},
    {0x07, ext_BIN, ext_bin},
    /*need to sync with mobile APP*/
};

const T_FILE_EXTENSION scan_ext =
{
    .file_ext_num = 8,
    .file_ext = ext_def_array,
};
T_FILE_SCAN_HANDLE scan_hdl;

static uint8_t timer_idx_disk_power_down = 0;
static uint8_t app_disk_timer_id = 0;
uint16_t DiskPowerBitmap = 0;
static bool audio_fs_mounted = false;

bool app_fs_disk_power_down_check_idle(void)
{
    if (DiskPowerBitmap == APP_DISK_CHECK_IDLE)
    {
        return true;
    }
    return false;
}

static void app_disk_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("app_disk_timeout_cb: timer_evt 0x%02x, param 0x%x", timer_evt, param);

    switch (timer_evt)
    {
    case APP_TIMER_DISK_POWER_DOWN:
        {
            app_stop_timer(&timer_idx_disk_power_down);
            if (app_fs_disk_power_down_check_idle())
            {
                app_fs_disk_power_down();
            }
        }
        break;
    default:
        break;
    }
}

void app_fs_disk_power_down_enable(uint16_t bit)
{
    if (DiskPowerBitmap & bit)
    {
        APP_PRINT_TRACE3("app_disk_power_down_enable: %08x %08x -> %08x", bit, DiskPowerBitmap,
                         (DiskPowerBitmap & ~bit));
    }
    DiskPowerBitmap &= ~bit;

    app_start_timer(&timer_idx_disk_power_down, "disk_power_down",
                    app_disk_timer_id, APP_TIMER_DISK_POWER_DOWN, 0, false,
                    POWER_DOWN_DISK_TIME);
}

void app_fs_disk_power_down_disable(uint16_t bit)
{
    app_stop_timer(&timer_idx_disk_power_down);
    if ((DiskPowerBitmap & bit) == 0)
    {
        APP_PRINT_TRACE3("app_disk_power_down_disable: %08x %08x -> %08x", bit, DiskPowerBitmap,
                         (DiskPowerBitmap | bit));
    }
    DiskPowerBitmap |= bit;
}

void app_fs_disk_power_down(void)
{
    if (disk_power_off() != 0)
    {
        APP_PRINT_INFO1("app_fs_disk_power_down disk %s do not support power ctrl!",
                        mp_fatfs.mnt_point);
    }
    APP_PRINT_INFO1("app_fs_disk_power_down disk %s", TRACE_STRING(mp_fatfs.mnt_point));
    app_dlps_enable(APP_DLPS_ENTER_CHECK_DISK);
}

void app_fs_disk_power_on(void)
{
    app_dlps_disable(APP_DLPS_ENTER_CHECK_DISK);
    if (disk_power_on() != 0)
    {
        APP_PRINT_INFO1("app_fs_disk_power_on disk %s do not support power ctrl!",
                        mp_fatfs.mnt_point);
    }
    APP_PRINT_INFO1("app_fs_disk_power_on disk %s", TRACE_STRING(mp_fatfs.mnt_point));
}

static void app_audio_fs_interface_timer_init(void)
{
    app_timer_reg_cb(app_disk_timeout_cb, &app_disk_timer_id);
}

uint32_t app_fs_get_header_bin_size(void)
{
    size_t size = 0;
    int res = fs_size(scan_hdl.header_fil, &size);
    if (res != 0)
    {
        APP_PRINT_ERROR1("Failed to get file size: %d", res);
    }
    return size;
}

uint32_t app_fs_get_name_bin_size(void)
{
    size_t size = 0;
    int res = fs_size(scan_hdl.name_fil, &size);
    if (res != 0)
    {
        APP_PRINT_ERROR1("Failed to get file size: %d", res);
    }
    return size;
}

int app_fs_read_header_bin(uint32_t offset, uint8_t *readBuf, uint32_t readLen, uint32_t *len)
{
    int fs_res = 0;

    if (readBuf == NULL)
    {
        return FR_INVALID_PARAMETER;
    }

    fs_res = seek_read(scan_hdl.header_fil, offset, readBuf, readLen);
    if (fs_res < 0)
    {
        return fs_res;
    }
    *len = fs_res;

    return 0;
}

int app_fs_read_name_bin(uint32_t offset, uint8_t *readBuf, uint32_t readLen, uint32_t *len)
{
    int fs_res = 0;

    if (readBuf == NULL)
    {
        return FR_INVALID_PARAMETER;
    }

    fs_res = seek_read(scan_hdl.name_fil, offset, readBuf, readLen);
    if (fs_res < 0)
    {
        return fs_res;
    }
    *len = fs_res;

    return 0;
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

    //APP_PRINT_TRACE2("file %s ptr 0x%x", TRACE_STRING(fil_hdl->filename), fs_tell(&fil_hdl->fil));
    res = fs_write(&fil_hdl->fil, writeBuf, writeLen);
    if (res < 0)
    {
        APP_PRINT_ERROR1("app_fs_write: write file failed! res:0x%x", res);
    }

    return res;
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

uint8_t app_audio_fs_interface_init(void)
{
    if (audio_fs_mounted)
    {
        if (fs_create_scan_handle(AUDIO_FILE_PATH, &scan_ext, &scan_hdl) == 0)
        {
            bool playlist_changed;
            uint16_t count;
            fs_scan_file_list(&scan_hdl, &playlist_changed);
            if (fs_read_header_bin_count(&count, &scan_hdl) != 0)
            {
                APP_PRINT_ERROR0("app read head bin count error !!! ");
            }
            //check sd card file and nor flash saved info
            if (playlist_changed || (count != *(uint16_t *)MUSIC_HEADER_BIN_ADDR))
            {
                playback_sync_playlist();
            }
            audio_playback_init();
            app_audio_fs_interface_timer_init();
            app_fs_disk_power_down_enable(APP_DISK_CHECK_PLAYBACK);
#if F_APP_SUPPORT_USB
            usb_app_init();
#endif
            return 0;
        }
    }

    APP_PRINT_ERROR0("app audio fs init fail!");
    return 1;
}

int32_t app_fs_init(void)
{
    app_dlps_disable(APP_DLPS_ENTER_CHECK_DISK);
#if DT_NODE_HAS_STATUS(DT_NODELABEL(flash_disk1), okay)
    if (fs_mount(&mp_romfs) != 0)
    {
        APP_PRINT_ERROR1("app fs mount disk error: disk root path %s", mp_romfs.mnt_point);
    }
#endif
    //fix sdio issue, remove on b-cut
    *((uint32_t *) 0x400002ac) |= (0xf << 19);
    if (fs_init(&mp_fatfs) != 0)
    {
        APP_PRINT_ERROR1("app fs mount disk error: disk root path %s", mp_fatfs.mnt_point);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_DISK);
        return -1;
    }
    audio_fs_mounted = true;
    return 0;
}