/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _BRIDGE_MUSIC_PLAYER_H_
#define _BRIDGE_MUSIC_PLAYER_H_

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
#define MUSIC_PLAYER_SONG_NAME_MAX_LEN  64
#define MUSIC_PLAYER_PLAYLIST_WINDOW_SIZE  128

/*gui topic*/
#define GUI_TOPIC_MUSIC_PLAYER_STATE      "music_player_state"
#define GUI_TOPIC_MUSIC_PLAYER_PLAYLIST   "music_player_playlist"
#define GUI_TOPIC_MUSIC_PLAYER_PROGRESS   "music_player_progress"

/*app topic*/
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_ALL_TOPIC                  "music_player/*"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PLAYER_STATE           "music_player/req/pl_st"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PLAYLIST_STATE         "music_player/req/pll_st"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PROGRESS_STATE         "music_player/req/pg_st"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PLAY_PAUSE             "music_player/cmd/pp"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_NEXT                   "music_player/cmd/next"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PREV                   "music_player/cmd/prev"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_VOL_UP                 "music_player/cmd/vol_up"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_VOL_DOWN               "music_player/cmd/vol_down"
#define EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PLAY_BY_INDEX          "music_player/cmd/pbi"

/*============================================================================*
 *                         Types
 *============================================================================*/

typedef enum
{
    MUSIC_PLAYER_MODE_NONE            = 0x00,
    MUSIC_PLAYER_MODE_LOCAL_PLAYBACK  = 0x01,
    MUSIC_PLAYER_MODE_A2DP_SRC        = 0x02,
    MUSIC_PLAYER_MODE_A2DP_SINK       = 0x03,
} T_MUSIC_PLAYER_MODE;

typedef enum
{
    MUSIC_PLAYER_STATUS_STOPPED  = 0x00,
    MUSIC_PLAYER_STATUS_PLAYING  = 0x01,
    MUSIC_PLAYER_STATUS_PAUSED   = 0x02,
} T_MUSIC_PLAYER_PLAY_STATUS;

typedef struct
{
    T_MUSIC_PLAYER_MODE          mode;
    T_MUSIC_PLAYER_PLAY_STATUS   play_status;
    uint8_t                     volume;        /* 0~15*/
    uint8_t                     max_volume;
    uint16_t                    current_index;
    uint16_t                    total_songs;   /* local playlist count */
    uint32_t                    pos_time_ms;
    uint32_t                    total_time_ms;
    char                        song_name[MUSIC_PLAYER_SONG_NAME_MAX_LEN];
    uint16_t                    song_name_len;
} T_MUSIC_PLAYER_STATE;

typedef struct
{
    char     name[MUSIC_PLAYER_SONG_NAME_MAX_LEN];
    uint16_t name_len;
    uint32_t duration_ms;  /* total duration in ms, 0 if unknown */
} T_MUSIC_PLAYER_PLAYLIST_ITEM;

typedef struct
{
    uint16_t                    current_index;
    uint16_t                    item_capacity;   /* capacity of items array */
    T_MUSIC_PLAYER_PLAYLIST_ITEM *items;
} T_MUSIC_PLAYER_PLAYLIST;

typedef struct
{
    T_MUSIC_PLAYER_STATE state;
    T_MUSIC_PLAYER_PLAYLIST playlist;
} T_MUSIC_PLAYER;

typedef struct
{
    uint32_t current_index;
} T_MUSIC_PLAYER_GUI_TO_APP_EVENT_DATA;

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
* @brief Send music player command or data from GUI to App.
*
* @param  topic The topic of the message.
* @param  data Pointer to the data to be sent.
* @param  size Size of the data to be sent.
* @return true if the message was sent successfully, false otherwise.
*/
bool music_player_gui_to_app(const char *topic, void *data, uint32_t size);
/**
 * @brief Initialize the music player bridge, including creating topics and subscribing to events.
 *        This function should be called in bridge_module_init().
 */
void bridge_music_player_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _BRIDGE_MUSIC_PLAYER_H_ */