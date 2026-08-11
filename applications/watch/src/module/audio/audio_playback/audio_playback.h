/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _APP_PLAYBACK_H_
#define _APP_PLAYBACK_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <string.h>
#include "rtl876x.h"
#include "mp3_parser.h"

/*============================================================================*
 *                         Constants
 *============================================================================*/
#define PLAYBACK_TRACK_STATE_CLEAR                  0xFF

/*============================================================================*
 *                         Variables
 *============================================================================*/
extern Mp3Hdl_t g_curr_song;

/*============================================================================*
 *                           Types
 *============================================================================*/

/*============================================================================*
 *                         Macros
 *============================================================================*/

/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
 * @brief Starts playback for the currently opened audio file.
 *
 * @return Playback execution status.
 */
uint8_t audio_playback_start(void);

/**
 * @brief Stops local playback and releases playback resources.
 *
 * @return Playback execution status.
 */
uint8_t audio_playback_stop(void);

/**
 * @brief Closes the currently opened song handle.
 *
 * @return Playback execution status.
 */
uint8_t audio_playback_close_flie(void);

/**
 * @brief Increases the playback volume by one step.
 */
void audio_playback_volume_up(void);

/**
 * @brief Decreases the playback volume by one step.
 */
void audio_playback_volume_down(void);

bool playback_get_source_song_time(Mp3Hdl_t curr_song, uint32_t *pos_time_ms,
                                   uint32_t *total_time_ms);

/**
 * @brief Initializes the playback module and registers callbacks.
 */
void audio_playback_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_PLAYBACK_H_ */
