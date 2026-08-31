/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FS_IF_H_
#define _APP_FS_IF_H_

/*============================================================================*
  *                               Header Files
  *============================================================================*/
#include <stdbool.h>
#include <stdint.h>
#include "fs_if.h"

#define FILE_NAME_LEN       (FF_MAX_LFN * sizeof(uint16_t))
#define FS_SYNC_THRESHOLD   (4 * 1024)

/* FS mount/format status codes for EVENT_FS_MOUNT_STATUS */
#define FS_STATUS_MOUNT_OK      0
#define FS_STATUS_MOUNT_FAIL    1
#define FS_STATUS_FORMAT_OK     2
#define FS_STATUS_FORMAT_FAIL   3

/* Format type options for app_fs_format() */
#define APP_FS_FMT_AUTO         0x07  /* auto-detect (FM_ANY) */
#define APP_FS_FMT_EXFAT        0x04  /* force exFAT (FM_EXFAT) */
#define APP_FS_FMT_FAT32        0x02  /* force FAT32 (FM_FAT32) */

/**  @brief app disk power down check bit map*/
#define APP_DISK_CHECK_IDLE          0x0000
#define APP_DISK_CHECK_PLAYBACK      0x0001
#define APP_DISK_CHECK_TRANS_FILE    0x0002
#define APP_DISK_CHECK_USB           0x0004
#define APP_DISK_CHECK_MAP           0x0008
#define APP_DISK_CHECK_DISPLAY       0x0010
#define APP_DISK_CHECK_WIFI_TEST     0x0020
#define APP_DISK_CHECK_RECORD        0x0040

#if DT_NODE_HAS_STATUS(DT_NODELABEL(flash_disk0), okay)
#define FATFS_DISK_NAME                     DT_PROP(DT_NODELABEL(flash_disk0), disk_name)
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(sdcard_disk0), okay)
//#define FATFS_DISK_NAME                     CONFIG_SDMMC_VOLUME_NAME      // SD Card
#define FATFS_DISK_NAME                     CONFIG_MMC_VOLUME_NAME      // eMMC
#elif DT_NODE_HAS_STATUS(DT_NODELABEL(spi_nand_disk0), okay)
#define FATFS_DISK_NAME                     DT_PROP(DT_NODELABEL(spi_nand_disk0), disk_name)
#else
#define FATFS_DISK_NAME                     "INVALID"
#endif

#if DT_NODE_HAS_STATUS(DT_NODELABEL(flash_disk1), okay)

#define ROMFS_DISK_NAME                     DT_PROP(DT_NODELABEL(spi_nand_disk1), disk_name)
#else
#define ROMFS_DISK_NAME                     "INVALID"
#endif



#define FATFS_ROOT_PATH                     "/"FATFS_DISK_NAME":"
#define AUDIO_ROOT_FOLDER                   FATFS_ROOT_PATH
#define AUDIO_FOLDER                        "/audio/"
#define AUDIO_FILE_PATH                     AUDIO_ROOT_FOLDER AUDIO_FOLDER

#define ROMFS_ROOT_PATH                     "/"ROMFS_DISK_NAME":"
#define RES_ROOT_FOLDER                     ROMFS_ROOT_PATH
#define RES_FOLDER                          "/res/"
#define RES_FILE_PATH                       RES_ROOT_FOLDER RES_FOLDER

typedef bool (*FileFoundCallback)(const char *filename, uint32_t filesize, void *context);

/**
 * @brief Report the filename via UART.
 * @param fil_hdl Pointer to the file handle.
 */
void app_fs_if_report_file_name(T_FILE_HANDLE *fil_hdl);

/**
 * @brief Get the length of the filename.
 * @param fil_hdl Pointer to the file handle.
 * @return Length of the filename.
 */
uint16_t app_fs_get_filename_len(T_FILE_HANDLE *fil_hdl);

/**
 * @brief Get the filename from the file handle.
 * @param fil_hdl Pointer to the file handle.
 * @return Pointer to the filename string.
 */
uint8_t *app_fs_get_filename(T_FILE_HANDLE *fil_hdl);

/**
 * @brief Create a directory if it does not exist.
 *
 * Checks if the specified folder path exists. If a file exists with the same name,
 * it is deleted. Then it attempts to create the directory.
 *
 * @param folder_name Absolute path of the folder to create.
 * @return 0 on success, or negative error code (fs_mkdir result).
 */
int32_t app_fs_create_folder(const char *folder_name);

/**
 * @brief Delete a file from the audio folder.
 *
 * @note Ensure the file is closed before calling this function.
 *
 * @param filename Name of the file to delete (relative to AUDIO_FILE_PATH).
 * @return 0 on success, negative error code on failure.
 */
int app_fs_unlink_file(const char *filename);

/**
 * @brief Get the size of an open file.
 *
 * @note This function forces a filesystem sync before checking size.
 * @param fil_hdl Pointer to the file handle.
 * @return File size in bytes.
 */
uint32_t app_fs_size(T_FILE_HANDLE *fil_hdl);

/**
 * @brief Write data to an open file.
 *
 * @param fil_hdl  Pointer to the file handle.
 * @param writeBuf Buffer containing data to write.
 * @param writeLen Number of bytes to write.
 * @return Number of bytes written on success, negative error code on failure.
 */
ssize_t app_fs_write(T_FILE_HANDLE *fil_hdl, uint8_t *writeBuf, uint32_t writeLen);

/**
 * @brief Read data from an open file.
 *
 * @param fil_hdl Pointer to the file handle.
 * @param readBuf Buffer to store read data.
 * @param readLen Number of bytes to read.
 * @return Number of bytes read on success, negative error code on failure.
 */
int app_fs_read(T_FILE_HANDLE *fil_hdl, uint8_t *readBuf, uint32_t readLen);

/**
 * @brief Move the file pointer to a specific location.
 *
 * @param fil_hdl Pointer to the file handle.
 * @param offset  The number of bytes from the beginning of the file (absolute position).
 * @return 0 on success, negative error code on failure.
 */
int app_fs_seek(T_FILE_HANDLE *fil_hdl, uint32_t offset);

/**
 * @brief Get the current position of the file pointer.
 *
 * @param fil_hdl Pointer to the file handle.
 * @return Current offset from the beginning of the file in bytes, or 0 on error.
 */
uint32_t app_fs_tell(T_FILE_HANDLE *fil_hdl);

/**
 * @brief Close an open file and free associated resources.
 *
 * @param fil_hdl Pointer to the file handle structure.
 * @return 0 on success, negative error code on failure.
 */
int app_fs_close_file(T_FILE_HANDLE *fil_hdl);

/**
 * @brief Open a file with a timestamp-based name, handling duplicates.
 *
 * This function generates a filename using the current system timestamp.
 * If a file with the generated name already exists, it appends a suffix
 * (e.g., _(1), _(2)) to ensure uniqueness.
 *
 * @param file_name Base name suffix (e.g., "record.opus").
 * @param mode      File access mode (e.g., FS_O_CREATE | FS_O_RDWR).
 * @return Pointer to the file handle, or NULL if creation failed.
 */
T_FILE_HANDLE *app_fs_open_file_with_timestamp(const char *file_name, uint8_t mode);

/**
 * @brief Open a file located in the audio folder.
 *
 * This function constructs the full path by prepending AUDIO_FILE_PATH,
 * allocates memory for the file handle structure, and opens the file.
 *
 * @param fil_name The name of the file (relative to AUDIO_FILE_PATH).
 * @param mode     File access mode flags (e.g., FS_O_READ, FS_O_WRITE).
 * @return Pointer to T_FILE_HANDLE, or NULL on failure.
 */
T_FILE_HANDLE *app_fs_open_file(const char *fil_name, uint8_t mode);

/**
 * @brief Get both total and free space of the filesystem.
 *
 * @param[out] ptotalSpace Pointer to store total space in bytes.
 * @param[out] pfreeSpace Pointer to store free space in bytes.
 * @return 0 on success, negative error code on failure.
 */
int app_fs_get_space_info(uint32_t *ptotalSpace, uint32_t *pfreeSpace);

/**
 * @brief Get the free space of the mounted filesystem.
 *
 * @param[out] pfreeSpace Pointer to store the free space in bytes.
 * @return 0 on success, negative error code on failure.
 */
int app_fs_free_space(uint32_t *pfreeSpace);

/**
 * @brief List and process files in the configured audio directory.
 *
 * This function iterates through the directory specified by AUDIO_FILE_PATH.
 * If a filter extension is provided, only files matching that extension are processed.
 * For each matching file, the provided callback function is invoked to pass
 * the file name and size to the caller.
 *
 * @param filter_ext The file extension to filter by (e.g., ".mp3").
 *                   Pass NULL to list all files. (Case-insensitive check is recommended).
 * @param cb         Pointer to the callback function (FileFoundCallback).
 *                   This function is called for each file found.
 *                   - Return true to continue the iteration.
 *                   - Return false to stop the iteration immediately.
 *                   - Can be NULL if you only want to print logs.
 * @param context    A user-defined pointer passed transparently to the callback function.
 *                   Useful for maintaining state (e.g., a counter or a list buffer).
 */
void app_fs_if_list_files(const char *filter_ext, FileFoundCallback cb, void *context);

/**
 * @brief Format and mount the FATFS filesystem.
 *
 * @note This function performs a file system format (mkfs) before mounting.
 *       Use with caution as this will erase data on the target partition.
 *
 * @return 0 on success, negative error code on failure.
 */
int app_fs_mkfs_mount(void);

/**
 * @brief Initialize the file system subsystem.
 *
 * Mounts the FATFS filesystem and ensures the audio directory exists.
 *
 * @return 0 on success, -1 on failure.
 */
int32_t app_fs_init(void);

#if F_APP_FS_FORMAT_SUPPORT
/**
 * @brief Format and re-mount the filesystem.
 *
 * Calls fs_deinit, fs_mkfs with the specified format option, then fs_init.
 * On success, creates the default audio folder.
 *
 * @param opt Format option: APP_FS_FMT_AUTO, APP_FS_FMT_EXFAT, or APP_FS_FMT_FAT32.
 * @return 0 on success, negative error code on failure.
 */
int32_t app_fs_format(uint32_t opt);
#endif

#endif //_APP_FS_IF_H_
