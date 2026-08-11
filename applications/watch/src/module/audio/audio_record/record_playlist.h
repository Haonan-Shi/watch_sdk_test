/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _RECORD_PLAYLIST_H_
#define _RECORD_PLAYLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include "fs_if.h"
#include "app_fs_if.h"

/*============================================================================*
 *                           Constants
 *============================================================================*/

#define RECORD_MAX_NAME_LEN    64
/**
 * @brief Synchronizes recording playlist metadata from the file system.
 *
 * This routine refreshes the cached name and header bins, removes deleted
 * entries, and updates the stored recording count.
 */
void record_sync_playlist(void);

/**
 * @brief Initializes the file-system scan handle for recordings.
 *
 * @return Zero on success, negative value on failure.
 */
int32_t record_fs_init(void);

/**
 * @brief Gets the current number of recording files.
 *
 * @return Number of recording files currently cached.
 */
uint16_t record_get_file_count(void);

/**
 * @brief Gets the start address of the cached recording header list.
 *
 * @return Pointer to the first recording header entry.
 */
T_HEAD_INFO *record_get_header_info_start(void);

/**
 * @brief Gets the current recording playback index.
 *
 * @return Zero-based index of the current recording item.
 */
uint16_t record_get_cur_play_index(void);

/**
 * @brief Resets the current recording playback index to the first entry.
 */
void record_reset_cur_play_index(void);

/**
 * @brief Gets the header information for the current recording item.
 *
 * @return Pointer to the current recording header entry.
 */
T_HEAD_INFO *record_get_cur_play_header_info(void);

/**
 * @brief Gets the file name of the current recording item.
 *
 * @param[out] file_name Output buffer for the file name.
 * @param[in] buf_len Size of the output buffer in bytes.
 *
 * @return Zero on success, negative value on failure.
 */
int record_get_cur_file_name(char *file_name, uint16_t buf_len);

/**
 * @brief Loads the latest recording file name into a caller-provided buffer.
 *
 * @param[out] path Output buffer for the file name.
 * @param[in] path_len Size of the output buffer in bytes.
 *
 * @return Zero on success, negative value on failure.
 */
int32_t record_playlist_load_latest_file_name(char *path, size_t path_len);

/**
 * @brief Generates a new unique recording file name.
 *
 * @param[out] path Output buffer for the generated file name.
 * @param[in] path_len Size of the output buffer in bytes.
 *
 * @return Zero on success, negative value on failure.
 */
int32_t record_playlist_generate_new_file_name(char *path, size_t path_len);

/**
 * @brief Validates that a path is a record file path with a PCM extension.
 *
 * @param[in] path File path to validate.
 *
 * @retval true The path is valid for a recording file.
 * @retval false The path is invalid.
 */
bool record_playlist_ensure_file_path(char *path);

/**
 * @brief Updates the playlist after a recording file is created or modified.
 *
 * @param[in] file Pointer to the underlying file object.
 * @param[in] file_name Recording file name.
 * @param[in] str_len File name length, excluding the null terminator.
 *
 * @return Zero on success, negative value on failure.
 */
int record_playlist_update(void *file, const char *file_name, uint16_t str_len);

#ifdef __cplusplus
}
#endif

#endif /* _RECORD_PLAYLIST_H_ */