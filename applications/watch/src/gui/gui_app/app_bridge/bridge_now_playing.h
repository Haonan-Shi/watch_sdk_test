/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef _APP_NOW_PLAYING_H_
#define _APP_NOW_PLAYING_H_

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/

#include <stdbool.h>
#include <stdint.h>

/*============================================================================*
 *                         Macros
 *============================================================================*/

/*gui topic*/
#define GUI_TOPIC_NOW_PLAYING_STATE "now_playing_state"
/*app topic*/
#define EVENT_BUS_TOPIC_NOW_PLAYING_ALL_TOPIC "n_p/*"
#define EVENT_BUS_TOPIC_NOW_PLAYING_REQ_AUDIO_MODE "n_p/req/a_m"
#define EVENT_BUS_TOPIC_NOW_PLAYING_SET_PLAYBACK_MODE "n_p/set/pb"
#define EVENT_BUS_TOPIC_NOW_PLAYING_SET_A2DP_SINK_MODE "n_p/set/snk"
#define EVENT_BUS_TOPIC_NOW_PLAYING_SET_A2DP_SOURCE_MODE "n_p/set/src"

/*============================================================================*
 *                         Types
 *============================================================================*/
typedef enum
{
    NOW_PLAYING_CMD_SWITCH_PLAYBACK    = 0x10,
    NOW_PLAYING_CMD_SWITCH_A2DP_SINK,
    NOW_PLAYING_CMD_SWITCH_A2DP_SOURCE,
} T_NOW_PLAYING_CMD;

typedef enum
{
    NOW_PLAYING_MODE_NONE            = 0x00,
    NOW_PLAYING_MODE_LOCAL_PLAYBACK  = 0x01,
    NOW_PLAYING_MODE_A2DP_SINK       = 0x02,
    NOW_PLAYING_MODE_A2DP_SOURCE     = 0x03,
} T_NOW_PLAYING_MODE;

typedef struct
{
    T_NOW_PLAYING_MODE mode;
    bool playback_enabled;
    bool a2dp_sink_enabled;
    bool a2dp_source_enabled;
} T_NOW_PLAYING_STATE;

typedef struct
{
    T_NOW_PLAYING_STATE state;
} T_NOW_PLAYING;

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
* @brief Send now playing command or data from GUI to App.
*
* @param  topic The topic of the message.
* @param  data Pointer to the data to be sent.
* @param  size Size of the data to be sent.
* @return true if the message was sent successfully, false otherwise.
*/
bool now_playing_gui_to_app(const char *topic, void *data, uint32_t size);

/**
 * @brief Initialize the now playing bridge, including creating topics and subscribing to events.
 *        This function should be called in bridge_module_init().
 */
void bridge_now_playing_init(void);


#ifdef __cplusplus
}
#endif

#endif /* _APP_NOW_PLAYING_H_ */