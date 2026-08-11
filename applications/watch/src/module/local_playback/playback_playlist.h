/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _PLAYBACK_PLAYLIST_H_
#define _PLAYBACK_PLAYLIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "rtl876x.h"
#include "app_fs_if.h"

/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
 * @brief Synchronizes playlist metadata from the file system to the cache.
 *
 * This routine refreshes the cached name and header bins, removes deleted
 * entries from the header buffer, and updates the stored song count.
 */
void playback_sync_playlist(void);

/**
 * @brief Gets the current song count in the playlist.
 *
 * @return Number of songs currently available in the cached playlist.
 */
uint16_t playback_get_song_count(void);

/**
 * @brief Gets the start address of the cached song header list.
 *
 * @return Pointer to the first song header entry.
 */
T_HEAD_INFO *playback_get_header_info_start(void);

/**
 * @brief Gets the current playback index.
 *
 * @return Zero-based index of the current song.
 */
uint16_t playback_get_cur_play_index(void);

/**
 * @brief Resets the current playback index to the first song.
 */
void playback_reset_cur_play_index(void);

/**
 * @brief Gets the header information for the current song.
 *
 * @return Pointer to the header entry of the current playback index.
 */
T_HEAD_INFO *playback_get_cur_play_header_info(void);

/**
 * @brief Advances to the next song and sends a play request.
 *
 * When the current index reaches the end of the playlist, it wraps back to 0.
 */
void playback_play_next_music(void);

/**
 * @brief Moves to the previous song and sends a play request.
 *
 * When the current index would become negative, it is clamped to 0.
 */
void playback_play_prev_music(void);

/**
 * @brief Selects a song by index and sends a play request.
 *
 * @param[in] index Zero-based playlist index to play.
 *
 * If the index is out of range, the current playback index is reset to 0.
 */
void playback_play_select_music(uint16_t index);

#ifdef __cplusplus
}
#endif

#endif /* _PLAYBACK_PLAYLIST_H_ */
