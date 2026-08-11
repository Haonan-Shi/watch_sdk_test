/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
*                        Header Files
*============================================================================*/

#include "event_bus.h"
#include "bridge_now_playing.h"
#include "gui_listener.h"
#include "app_audio_mode_switch.h"
#include "app_main.h"
#include "app_audio_if.h"
#include "app_task.h"
#include "trace.h"

/*============================================================================*
 *                            Macros
 *============================================================================*/

/*============================================================================*
 *                           Types
 *============================================================================*/

/*============================================================================*
 *                           Constants
 *============================================================================*/

/*============================================================================*
 *                            Variables
 *============================================================================*/

static T_NOW_PLAYING s_now_playing;
static bool s_now_playing_initialized;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_audio_mode_handle;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_now_playing_async_handle;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

/**
* @brief  map app audio mode to now playing mode
*
* @param[in]  mode The audio mode in app.
* @return The corresponding now playing mode.
*/
static T_NOW_PLAYING_MODE now_playing_map_mode(T_APP_AUDIO_MODE mode)
{
    switch (mode)
    {
    case MODE_APP_PLAYBACK:
        return NOW_PLAYING_MODE_LOCAL_PLAYBACK;
    case MODE_APP_A2DP_SNK:
        return NOW_PLAYING_MODE_A2DP_SINK;
    case MODE_APP_A2DP_SRC:
        return NOW_PLAYING_MODE_A2DP_SOURCE;
    default:
        return NOW_PLAYING_MODE_NONE;
    }
}

/**
 * @brief  collect current now playing state from app and update the global now playing variable.
 */
static void now_playing_collect_state(void)
{
    T_NOW_PLAYING *now_playing = &s_now_playing;

    now_playing->state.mode = now_playing_map_mode(app_db.audio_play_mode);
    now_playing->state.playback_enabled =
        (now_playing->state.mode == NOW_PLAYING_MODE_LOCAL_PLAYBACK);
    now_playing->state.a2dp_sink_enabled =
        (now_playing->state.mode == NOW_PLAYING_MODE_A2DP_SINK);
    now_playing->state.a2dp_source_enabled =
        (now_playing->state.mode == NOW_PLAYING_MODE_A2DP_SOURCE);

    APP_PRINT_INFO1("[now_playing] collect state mode %d", now_playing->state.mode);
}

/**
 * @brief  publish current now playing state to GUI.
 */
static void app_now_playing_publish_state_app_to_gui(void)
{
    now_playing_collect_state();
    gui_msg_publish(GUI_TOPIC_NOW_PLAYING_STATE, &s_now_playing.state,
                    sizeof(s_now_playing.state));
}

/**
 * @brief handle async events from GUI.
 *
 * @param[in] event_data The event data from event bus, should contain the topic and optional data for the command.
 * @return int32_t The result of handling the event, currently always returns 0.
 */
static int32_t app_now_playing_audio_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    if (strcmp(topic, EVENT_BUS_TOPIC_AUDIO_MODE) == 0)
    {
        app_now_playing_publish_state_app_to_gui();
    }

    return EVENT_BUS_OK;
}

/**
 * @brief handle async events from GUI.
 *
 * @param[in] event_data The event data from event bus, should contain the topic and optional data for the command.
 * @return int32_t The result of handling the event, currently always returns 0.
 */

static int32_t app_now_playing_async_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    APP_PRINT_TRACE1("[now_playing] app_now_playing_async_event_callback: %s", TRACE_STRING(topic));
    if (strcmp(topic, EVENT_BUS_TOPIC_NOW_PLAYING_SET_PLAYBACK_MODE) == 0)
    {
        app_audio_mode_switch(MODE_APP_PLAYBACK);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_NOW_PLAYING_SET_A2DP_SINK_MODE) == 0)
    {
        app_audio_mode_switch(MODE_APP_A2DP_SNK);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_NOW_PLAYING_SET_A2DP_SOURCE_MODE) == 0)
    {
        app_audio_mode_switch(MODE_APP_A2DP_SRC);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_NOW_PLAYING_REQ_AUDIO_MODE) == 0)
    {
        app_now_playing_publish_state_app_to_gui();
    }
    else
    {
        APP_PRINT_INFO1("[now_playing] received unknown async event topic: %s", TRACE_STRING(topic));
    }

    return EVENT_BUS_OK;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void bridge_now_playing_init(void)
{
    event_bus_topic_register(EVENT_BUS_TOPIC_NOW_PLAYING_ALL_TOPIC);
    event_bus_subscribe_async(&s_now_playing_async_handle,
                              EVENT_BUS_TOPIC_NOW_PLAYING_ALL_TOPIC,
                              event_bus_async_send_to_apptask,
                              NULL,
                              app_now_playing_async_event_callback);

    event_bus_subscribe(&s_audio_mode_handle,
                        EVENT_BUS_TOPIC_AUDIO_ALL_TOPIC,
                        app_now_playing_audio_event_callback);

    APP_PRINT_INFO0("[now_playing] initialized");
}

bool now_playing_gui_to_app(const char *topic, void *data, uint32_t size)
{
    return event_bus_publish(topic, data, size) == EVENT_BUS_OK;
}
