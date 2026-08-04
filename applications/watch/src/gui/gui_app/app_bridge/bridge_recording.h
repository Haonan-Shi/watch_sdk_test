/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _BRIDGE_RECORDING_H_
#define _BRIDGE_RECORDING_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>

/*============================================================================*
 *                         Macros
 *============================================================================*/
#define RECORDING_FILE_NAME_MAX_LEN  64
#define RECORDING_PLAYLIST_WINDOW_SIZE  128

/*gui topic*/
#define GUI_TOPIC_RECORDING_RECORDER_STATE      "recording_recorder_state"
#define GUI_TOPIC_RECORDING_RECORDER_FILE       "recording_recorder_file"
#define GUI_TOPIC_RECORDING_PLAYER_STATE        "recording_player_state"
#define GUI_TOPIC_RECORDING_PLAYLIST            "recording_playlist"
#define GUI_TOPIC_RECORDING_PROGRESS            "recording_progress"

/*app topic*/
#define EVENT_BUS_TOPIC_RECORDING_ALL_TOPIC "rec/*"
#define EVENT_BUS_TOPIC_REQ_RECORDING_RECORDER_STATE "rec/req/rec_st"
#define EVENT_BUS_TOPIC_REQ_RECORDING_RECORDER_FILE "rec/req/file"
#define EVENT_BUS_TOPIC_REQ_RECORDING_PLAYER_STATE "rec/req/pler_st"
#define EVENT_BUS_TOPIC_REQ_RECORDING_PLAYLIST_STATE "rec/req/pl"
#define EVENT_BUS_TOPIC_REQ_RECORDING_PROGRESS_STATE "rec/req/pg"
#define EVENT_BUS_TOPIC_RECORDING_CMD_ALL_TOPIC "rec/cmd/*"
#define EVENT_BUS_TOPIC_RECORDING_CMD_START_RECORDING_FROM_FS "rec/cmd/start_rec_fs"
#define EVENT_BUS_TOPIC_RECORDING_CMD_STOP_RECORDING_FROM_FS "rec/cmd/stop_rec_fs"
#define EVENT_BUS_TOPIC_RECORDING_CMD_START_PLAYING_FROM_FS "rec/cmd/start_play_fs"
#define EVENT_BUS_TOPIC_RECORDING_CMD_STOP_PLAYING_FROM_FS "rec/cmd/stop_play_fs"

/*============================================================================*
 *                         Types
 *============================================================================*/

typedef enum
{
    RECORDING_MODE_NONE        = 0x00,
    RECORDING_MODE_FILESYSTEM  = 0x01,
    RECORDING_MODE_LOOPBACK    = 0x02,
} T_RECORDING_MODE;

typedef enum
{
    RECORDING_STATUS_IDLE      = 0x00,
    RECORDING_STATUS_RECORDING = 0x01,
    RECORDING_STATUS_PLAYING   = 0x02,
} T_RECORDING_STATUS;

typedef struct
{
    T_RECORDING_MODE    mode;
    T_RECORDING_STATUS status;
    uint32_t            duration_ms;
    uint32_t            total_duration_ms;
    uint16_t            file_count;
    uint16_t            current_index;
} T_RECORDING_STATE;

typedef struct
{
    char     name[RECORDING_FILE_NAME_MAX_LEN];
    uint16_t name_len;
    uint32_t duration_ms;
    uint32_t file_size;
    uint32_t timestamp;
} T_RECORDING_FILE;

typedef struct
{
    uint16_t            current_index;
    uint16_t            file_count;
    T_RECORDING_FILE   *files;
} T_RECORDING_PLAYLIST;

typedef struct
{
    T_RECORDING_STATE state;
    T_RECORDING_PLAYLIST playlist;
} T_RECORDING;

typedef struct
{
    uint16_t current_index;
} T_RECORDING_GUI_TO_APP_EVENT_DATA;


/*============================================================================*
 *                         Constants
 *============================================================================*/


/*============================================================================*
 *                         Variables
 *============================================================================*/


/*============================================================================*
 *                         Functions
 *============================================================================*/

/**
* @brief Send recording command or data from GUI to App.
*
* @param  topic The topic of the message.
* @param  data Pointer to the data to be sent.
* @param  size Size of the data to be sent.
* @return true if the message was sent successfully, false otherwise.
*/

bool recording_gui_to_app(const char *topic, void *data, uint32_t size);
/**
 * @brief Initialize the recording bridge, including creating topics and subscribing to events.
 *        This function should be called in bridge_module_init().
 */
void bridge_recording_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _BRIDGE_RECORDING_H_ */