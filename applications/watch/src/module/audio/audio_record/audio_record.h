/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _AUDIO_RECORD_H_
#define _AUDIO_RECORD_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*
 *                         Types
 *============================================================================*/
typedef enum
{
    AUDIO_RECORD_SAVE_FS        = 0x00,
    AUDIO_RECORD_SAVE_FLASH     = 0x01,
    AUDIO_RECORD_LOOP_BACK      = 0x02,
    AUDIO_RECORD_UART_REPORT    = 0x03,
} T_AUDIO_RECORD_MODE;

typedef enum _T_AUDIO_RECORD_FORMAT
{
    OPUS_16KHZ_16KBPS_CBR_0_20MS,
    MIC_PCM_16KHZ,

} T_AUDIO_RECORD_FORMAT;

/**
 * @brief Stops an active recording session.
 */
void audio_record_stop_recording(void);

/**
 * @brief Starts a prepared recording session.
 */
void audio_record_start_recording(void);

/**
 * @brief Indicates whether a recording session is active.
 *
 * @retval true Recording is active.
 * @retval false Recording is inactive.
 */
bool audio_record_is_recording(void);

/**
 * @brief Indicates whether a playback session is active.
 *
 * @retval true Playback is active.
 * @retval false Playback is inactive.
 */
bool audio_record_is_playing(void);

/**
 * @brief Gets the current elapsed time for the active record or playback session.
 *
 * @return Current elapsed time in milliseconds.
 */
uint32_t audio_record_get_current_time_ms(void);

/**
 * @brief Gets the total duration for the active record or playback session.
 *
 * @return Total duration in milliseconds.
 */
uint32_t audio_record_get_total_time_ms(void);

/**
 * @brief Converts a recorded file size to a PCM duration.
 *
 * @param[in] file_size File size in bytes.
 *
 * @return Calculated duration in milliseconds.
 */
uint32_t audio_record_get_duration_ms_by_file(uint32_t file_size);

/**
 * @brief Writes PCM data into the active playback track.
 *
 * @param[in] buf PCM data buffer.
 * @param[in] len Buffer length in bytes.
 */
void audio_record_play_data_write(uint8_t *buf, uint16_t len);

/**
 * @brief Stops an active playback session.
 */
void audio_record_stop_playing(void);

/**
 * @brief Initializes playback support resources.
 */
void audio_record_play_init(void);

/**
 * @brief Initializes the recording module.
 *
 * This routine should be called during application startup.
 */
void audio_record_init(void);

/**
 * @brief Prepares a new recording session.
 *
 * @param[in] record_mode Recording mode.
 */
void audio_record_init_recorder(T_AUDIO_RECORD_MODE record_mode);

/**
 * @brief Prepares a playback session for a recording source.
 *
 * @param[in] record_mode Playback source mode.
 * @param[in] play_path Optional file path. When NULL in file-system mode,
 *            the latest recording is used.
 *
 * @retval true Playback initialization succeeded.
 * @retval false Playback initialization failed.
 */
bool audio_record_init_player(T_AUDIO_RECORD_MODE record_mode, const char *play_path);

#ifdef __cplusplus
}
#endif

#endif /* _AUDIO_RECORD_H_ */
