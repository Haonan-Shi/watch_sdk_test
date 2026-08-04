/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
  *                   Define to prevent recursive inclusion
  *============================================================================*/
#ifndef _APP_FS_IF_H_
#define _APP_FS_IF_H_

/*============================================================================*
  *                               Header Files
  *============================================================================*/
#include <stdbool.h>
#include <stdint.h>
#include <zephyr/devicetree.h>
#include "remote.h"
#include "app_main.h"
#include "audio_playback.h"
#include "fs_if.h"

#define FILE_NAME_LEN       (FF_MAX_LFN * sizeof(uint16_t))

/**  @brief app disk power down check bit map*/
#define APP_DISK_CHECK_IDLE          0x0000
#define APP_DISK_CHECK_PLAYBACK      0x0001
#define APP_DISK_CHECK_TRANS_FILE    0x0002
#define APP_DISK_CHECK_USB           0x0004
#define APP_DISK_CHECK_MAP           0x0008
#define APP_DISK_CHECK_DISPLAY       0x0010
#define APP_DISK_CHECK_WIFI_TEST     0x0020
#define APP_DISK_CHECK_RECORD        0x0040

// cppcheck-suppress syntaxError
#if DT_NODE_HAS_STATUS(DT_NODELABEL(flash_disk0), okay)
#define FATFS_DISK_NAME                     DT_PROP(DT_NODELABEL(flash_disk0), disk_name)
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(sdcard_disk0), okay)
#define FATFS_DISK_NAME                     CONFIG_SDMMC_VOLUME_NAME
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(spi_nand_disk0), okay)
#define FATFS_DISK_NAME                     DT_PROP(DT_NODELABEL(spi_nand_disk0), disk_name)
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(nand_flash_disk0), okay)
#define FATFS_DISK_NAME                     DT_PROP(DT_NODELABEL(nand_flash_disk0), disk_name)
#else
#define FATFS_DISK_NAME                     "INVALID"
#endif

// cppcheck-suppress syntaxError
#if DT_NODE_HAS_STATUS(DT_NODELABEL(flash_disk1), okay)

#define ROMFS_DISK_NAME                     DT_PROP(DT_NODELABEL(spi_nand_disk1), disk_name)
#else
#define ROMFS_DISK_NAME                     "INVALID"
#endif



#define FATFS_ROOT_PATH                     "/"FATFS_DISK_NAME":"
#define AUDIO_ROOT_FOLDER                   FATFS_ROOT_PATH
#define AUDIO_FOLDER                        "/audio/"
#define AUDIO_FILE_PATH                     AUDIO_ROOT_FOLDER AUDIO_FOLDER

#define ALIPAY_ROOT_FOLDER                  FATFS_ROOT_PATH
#define ALIPAY_FOLDER                       "/alipay/"
#define ALIPAY_FILE_PATH                    ALIPAY_ROOT_FOLDER ALIPAY_FOLDER

#define RECORD_ROOT_FOLDER                  FATFS_ROOT_PATH
#define RECORD_FOLDER                       "/record/"
#define RECORD_FILE_PATH                    RECORD_ROOT_FOLDER RECORD_FOLDER

#define ROMFS_ROOT_PATH                     "/"ROMFS_DISK_NAME":"
#define RES_ROOT_FOLDER                     ROMFS_ROOT_PATH
#define RES_FOLDER                          "/res/"
#define RES_FILE_PATH                       RES_ROOT_FOLDER RES_FOLDER

extern T_FILE_SCAN_HANDLE scan_hdl;

uint8_t app_audio_fs_interface_init(void);

bool app_fs_disk_power_down_check_idle(void);
void app_fs_disk_power_down_enable(uint16_t bit);
void app_fs_disk_power_down_disable(uint16_t bit);

void app_fs_disk_power_down(void);
void app_fs_disk_power_on(void);

uint32_t app_fs_get_header_bin_size(void);
uint32_t app_fs_get_name_bin_size(void);
int app_fs_read_header_bin(uint32_t offset, uint8_t *readBuf, uint32_t readLen, uint32_t *len);
int app_fs_read_name_bin(uint32_t offset, uint8_t *readBuf, uint32_t readLen, uint32_t *len);

int app_fs_sd_mkfs(void);
int app_fs_free_space(uint32_t *pfreeSpace);
int app_fs_get_space_info(uint32_t *ptotalSpace, uint32_t *pfreeSpace);

T_FILE_HANDLE *app_fs_open_file(const char *fil_name, uint8_t mode);
int app_fs_close_file(T_FILE_HANDLE *fil_hdl);
int app_fs_read(T_FILE_HANDLE *fil_hdl, uint8_t *readBuf, uint32_t readLen);
ssize_t app_fs_write(T_FILE_HANDLE *fil_hdl, uint8_t *writeBuf, uint32_t writeLen);
uint32_t app_fs_size(T_FILE_HANDLE *fil_hdl);
uint8_t *app_fs_get_filename(T_FILE_HANDLE *fil_hdl);
uint16_t app_fs_get_filename_len(T_FILE_HANDLE *fil_hdl);
int app_fs_unlink_file(const char *filename);
int32_t app_fs_init(void);

#endif //_APP_FS_IF_H_
