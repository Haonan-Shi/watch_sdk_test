/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _APP_AUDIO_IF_H_
#define _APP_AUDIO_IF_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdbool.h>
#include <stdint.h>
#include "remote.h"
#include "app_main.h"
#include "app_fs_if.h"

/*============================================================================*
 *                           Types
 *============================================================================*/

#define EVENT_BUS_TOPIC_AUDIO_ALL_TOPIC "audio/*"
#define EVENT_BUS_TOPIC_AUDIO_VOLUME_UP "audio/vol/up"
#define EVENT_BUS_TOPIC_AUDIO_VOLUME_DOWN "audio/vol/down"
#define EVENT_BUS_TOPIC_AUDIO_PLAY_STATUS_CHANGED "audio/pl_st"
#define EVENT_BUS_TOPIC_AUDIO_PLAY_INDEX_UPDATE "audio/pl_idx"
typedef enum
{
APP_AUDIO_STATE_STOP         = 0x00,
APP_AUDIO_STATE_PLAY         = 0x01,
APP_AUDIO_STATE_PAUSE        = 0x02,

/* Indicate that the command is still in progress. */
APP_AUDIO_STATE_TRY_STOPPING = 0x10,
APP_AUDIO_STATE_TRY_PLAYING  = 0x11,
APP_AUDIO_STATE_TRY_PAUSING  = 0x12,
} T_APP_AUDIO_STATE;

typedef enum
{
    APP_AUDIO_SUCCESS                    = 0x00,
    APP_AUDIO_READ_ERROR                 = 0x01,
    APP_AUDIO_IDX_ERROR                  = 0x02,
    APP_AUDIO_OPEN_FILE_ERROR            = 0x03,
    APP_AUDIO_CRC_ERROR                  = 0x04,
    APP_AUDIO_CLOSE_FILE_ERROR           = 0x05,
    APP_AUDIO_PLAYLIST_ERROR             = 0x06,
    APP_AUDIO_WRITE_ERROR                = 0x07,
    APP_AUDIO_TYPE_ERROR                 = 0x08,
    APP_AUDIO_MALLOC_ERROR               = 0x09,
    APP_AUDIO_PIPE_DRAIN_ERROR           = 0x0A,
    APP_AUDIO_PIPE_FILL_ERROR            = 0x0B,
    APP_AUDIO_A2DP_ENC_RX_ERROR          = 0x0C,
    APP_AUDIO_A2DP_ENC_PEEK_ERROR        = 0x0D,
    APP_AUDIO_A2DP_DATA_SEND             = 0x0E,
    APP_AUDIO_MODE_ERROR                 = 0x0F,
    APP_AUDIO_END_OF_FILE                = 0x10,
} T_APP_AUDIO_ERROR;

typedef enum
{
    APP_AUDIO_STOPPED_IDLE_ACTION             = 0x00,
    APP_AUDIO_STOPPED_SWITCH_FWD_ACTION       = 0x01,
    APP_AUDIO_STOPPED_SWITCH_BWD_ACTION       = 0x02,
    APP_AUDIO_STOPPED_FILE_END_TO_NEXT_ACTION = 0x03,
    APP_AUDIO_STOPPED_SWITCH_BY_NAME          = 0x04,
    APP_AUDIO_STOPPED_SWITCH_LOCAL_PLAY       = 0x05,
    APP_AUDIO_STOPPED_SWITCH_A2DP_SRC         = 0x06,
    APP_AUDIO_STOPPED_SWITCH_A2DP_SNK         = 0x07,
} T_APP_AUDIO_STOPPED_NEXT_ACTION;

typedef enum
{
    APP_AUDIO_FS_BUF_NORMAL = 0x00,
    APP_AUDIO_FS_BUF_LOW    = 0x01,
    APP_AUDIO_FS_BUF_HIGH   = 0x02,
} T_APP_AUDIO_FS_BUF_STATE;

typedef struct
{
    T_APP_AUDIO_STATE sd_play_state;
    T_APP_AUDIO_FS_BUF_STATE local_buffer_state;
    T_APP_AUDIO_FS_BUF_STATE source_buffer_state;
    uint8_t op_next_action;
    uint8_t track_state;
    uint8_t volume;
    uint8_t playlist_idx;
    uint16_t file_idx;
    uint16_t seq_num;
    uint8_t *file_name;
    uint16_t name_length;
} T_APP_AUDIO_FS_DATA;

/*============================================================================*
 *                         Variables
 *============================================================================*/
extern T_APP_AUDIO_FS_DATA playback_db;

/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
 * @brief Opens the specified file and starts playback or schedules a switch.
 *
 * @param[in] file_name Pointer to the file name buffer.
 * @param[in] length File name length.
 *
 * @return Audio operation result.
 */
T_APP_AUDIO_ERROR app_audio_play_by_name(uint8_t *file_name, uint16_t length);

/**
 * @brief Gets the current audio playback state for the active mode.
 *
 * @return Current audio state.
 */
T_APP_AUDIO_STATE app_audio_get_play_status(void);

/**
 * @brief Increases the current playback volume.
 */
void app_volume_up(void);

/**
 * @brief Decreases the current playback volume.
 */
void app_volume_down(void);

/**
 * @brief Gets the current volume of the active audio route.
 *
 * @return Current volume level.
 */
uint8_t app_audio_get_volume(void);

/**
 * @brief Starts audio playback or sends the corresponding active-mode command.
 *
 * @return Audio operation result.
 */
uint16_t app_audio_start(void);

/**
 * @brief Pauses audio playback or sends the corresponding active-mode command.
 *
 * @return Audio operation result.
 */
uint16_t app_audio_pause(void);

/**
 * @brief Stops audio playback or sends the corresponding active-mode command.
 *
 * @return Audio operation result.
 */
uint16_t app_audio_stop(void);

/**
 * @brief Skips to the next audio item or forwards the active remote source.
 *
 * @return Audio operation result.
 */
uint16_t app_audio_next(void);

/**
 * @brief Returns to the previous audio item or rewinds the active remote source.
 *
 * @return Audio operation result.
 */
uint16_t app_audio_prev(void);

/**
 * @brief Initializes the audio interface and selects the initial audio mode.
 */
void app_audio_interface_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_AUDIO_IF_H_ */
