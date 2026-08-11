/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
*                        Header Files
*============================================================================*/

#include "event_bus.h"
#include "bridge_music_player.h"
#include "gui_listener.h"
#include "audio_resource.h"
#include "playback_playlist.h"
#include "app_mmi.h"
#include "app_main.h"
#include "app_audio_if.h"
#include "app_task.h"
#include "trace.h"

/*============================================================================*
 *                            Macros
 *============================================================================*/
#define DURATION_CACHE_MAX_SONGS  128
#define DURATION_UNKNOWN          0

/*============================================================================*
 *                           Types
 *============================================================================*/

/*============================================================================*
 *                           Constants
 *============================================================================*/

/*============================================================================*
 *                            Variables
 *============================================================================*/

static uint32_t s_duration_cache[DURATION_CACHE_MAX_SONGS];
static uint16_t s_duration_cache_count;   /* number of valid entries */
static bool     s_duration_cache_valid;

static T_MUSIC_PLAYER s_music_player;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_audio_handle;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_music_player_async_handle;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

/**
 * @brief  map app audio mode to music player mode
 *
 * @param[in]  mode The audio mode in app.
 * @return The corresponding music player mode.
 */
static T_MUSIC_PLAYER_MODE music_player_map_mode(T_APP_AUDIO_MODE mode)
{
    switch (mode)
    {
    case MODE_APP_PLAYBACK:
        return MUSIC_PLAYER_MODE_LOCAL_PLAYBACK;
    case MODE_APP_A2DP_SRC:
        return MUSIC_PLAYER_MODE_A2DP_SRC;
    case MODE_APP_A2DP_SNK:
        return MUSIC_PLAYER_MODE_A2DP_SINK;
    default:
        return MUSIC_PLAYER_MODE_NONE;
    }
}

/**
 * @brief  map app audio state to music player play status
 *
 * @param[in]  st The audio state in app.
 * @return The corresponding music player play status.
 */
static T_MUSIC_PLAYER_PLAY_STATUS music_player_map_play_status(T_APP_AUDIO_STATE st)
{
    switch (st)
    {
    case APP_AUDIO_STATE_PLAY:
    case APP_AUDIO_STATE_TRY_PLAYING:
        return MUSIC_PLAYER_STATUS_PLAYING;
    case APP_AUDIO_STATE_PAUSE:
    case APP_AUDIO_STATE_TRY_PAUSING:
        return MUSIC_PLAYER_STATUS_PAUSED;
    default:
        return MUSIC_PLAYER_STATUS_STOPPED;
    }
}

/**
 * @brief  fill song name for playlist item by index, truncates if exceeds max length.
 *         Uses a simple caching mechanism to avoid repeated flash reads for duration.
 *
 * @param[in] index The song index in playlist.
 * @param[out] out The output buffer for song name, should be at least MUSIC_PLAYER_SONG_NAME_MAX_LEN bytes.
 * @param[out] out_len The output length of the song name.
 */
static void music_player_fill_song_name(char *out, uint16_t *out_len, uint16_t index)
{
    T_HEAD_INFO *headers = playback_get_header_info_start();
    uint16_t count = playback_get_song_count();

    if (index < count && headers != NULL)
    {
        T_HEAD_INFO *hdr = &headers[index];
        uint8_t *name_ptr = (uint8_t *)(MUSIC_NAME_BIN_ADDR + hdr->offset);
        uint16_t len = hdr->length;
        APP_PRINT_TRACE3("name %s name-ptr: %p, len: %d", TRACE_STRING(name_ptr), name_ptr, len);
        if (len > MUSIC_PLAYER_SONG_NAME_MAX_LEN - 1)
        {
            len = MUSIC_PLAYER_SONG_NAME_MAX_LEN - 1;
        }
        memcpy(out, name_ptr, len);
        out[len] = '\0';
        *out_len = len;
    }
    else
    {
        out[0] = '\0';
        *out_len = 0;
    }
}

/**
 * @brief create playlist items array with specified capacity, and assign to global music player state.
 *
 * @param[in]  required_count The required capacity of the playlist items array.
 * @return true if success, false if failed to allocate memory.
 */
static bool music_player_playlist_items_create(uint16_t required_count)
{
    if (required_count == 0)
    {
        return true;
    }

    T_MUSIC_PLAYER_PLAYLIST_ITEM *new_items = (T_MUSIC_PLAYER_PLAYLIST_ITEM *)malloc(
                                                  sizeof(T_MUSIC_PLAYER_PLAYLIST_ITEM) * required_count);
    if (new_items == NULL)
    {
        APP_PRINT_ERROR0("[music_player] allocate playlist items failed");
        return false;
    }

    memset(new_items, 0, sizeof(T_MUSIC_PLAYER_PLAYLIST_ITEM) * required_count);

    s_music_player.playlist.items = new_items;
    s_music_player.playlist.item_capacity = required_count;
    return true;
}

/**
 * @brief delete playlist items array and reset the global music player state.
 *
 * @return true if success, false if failed to allocate memory.
 */
static bool music_player_playlist_items_delete(void)
{
    if (s_music_player.playlist.items != NULL)
    {
        free(s_music_player.playlist.items);
        s_music_player.playlist.items = NULL;
        s_music_player.playlist.item_capacity = 0;
    }
    return true;
}

/**
 * @brief Lazy-load duration for a single song by index.
 *        Opens the mp3 file, reads total play time, and caches it.
 *
 * @param[in] index The song index in playlist.
 * @return The duration in ms, or DURATION_UNKNOWN if failed to get.
 */
static uint32_t music_player_duration_cache_load(uint16_t index)
{
    T_HEAD_INFO *headers = playback_get_header_info_start();
    uint16_t count = playback_get_song_count();

    if (index >= count || headers == NULL)
    {
        return DURATION_UNKNOWN;
    }

    T_HEAD_INFO *hdr = &headers[index];
    char *name_ptr = (char *)(MUSIC_NAME_BIN_ADDR + hdr->offset);

    EMp3Res res;
    Mp3Hdl_t hdl = Mp3_CreateHandle(name_ptr, &res);
    if (hdl == NULL || res != MP3RES_OK)
    {
        APP_PRINT_INFO1("[music_player] duration cache: failed to open song %d", index);
        if (hdl != NULL)
        {
            Mp3_FreeHandle(hdl);
        }
        return DURATION_UNKNOWN;
    }

    float total_ms = Mp3_GetTotalPlayTime_ms(hdl);
    Mp3_FreeHandle(hdl);

    uint32_t duration = (total_ms > 0.0f) ? (uint32_t)total_ms : DURATION_UNKNOWN;

    if (index < DURATION_CACHE_MAX_SONGS)
    {
        s_duration_cache[index] = duration;
        if (index >= s_duration_cache_count)
        {
            s_duration_cache_count = index + 1;
        }
    }

    return duration;
}

/**
 * @brief get duration for a song by index, with caching. If the duration is not in cache, it will be loaded and cached.
 *
 * @param[in]  index The song index in playlist.
 * @return The duration in ms, or DURATION_UNKNOWN if failed to get.
 */
static uint32_t music_player_duration_cache_get(uint16_t index)
{
    if (!s_duration_cache_valid)
    {
        memset(s_duration_cache, 0, sizeof(s_duration_cache));
        s_duration_cache_count = 0;
        s_duration_cache_valid = true;
    }

    if (index < DURATION_CACHE_MAX_SONGS && index < s_duration_cache_count &&
        s_duration_cache[index] != DURATION_UNKNOWN)
    {
        return s_duration_cache[index];
    }

    return music_player_duration_cache_load(index);
}

/**
 * @brief collect current music player state from app and update the global music player state variable.
 */
static void music_player_collect_state(void)
{
    s_music_player.state.mode = music_player_map_mode(app_db.audio_play_mode);
    s_music_player.state.play_status = music_player_map_play_status(app_audio_get_play_status());
    s_music_player.state.volume = app_audio_get_volume();
    s_music_player.state.total_songs = playback_get_song_count();
    s_music_player.state.current_index = playback_get_cur_play_index();
    s_music_player.state.max_volume = 15;

    APP_PRINT_INFO5("[music_player] collect state: mode %d, play_status %d, volume %d, total_songs %d, current_index %d",
                    s_music_player.state.mode, s_music_player.state.play_status, s_music_player.state.volume,
                    s_music_player.state.total_songs, s_music_player.state.current_index);

    /* Progress: only available for local/src mode with active song */
    if ((s_music_player.state.mode == MUSIC_PLAYER_MODE_LOCAL_PLAYBACK ||
         s_music_player.state.mode == MUSIC_PLAYER_MODE_A2DP_SRC) &&
        g_curr_song != NULL)
    {
        uint32_t pos_ms = 0, total_ms = 0;
        playback_get_source_song_time(g_curr_song, &pos_ms, &total_ms);
        s_music_player.state.pos_time_ms = pos_ms;
        s_music_player.state.total_time_ms = total_ms;
    }
    else
    {
        s_music_player.state.pos_time_ms = 0;
        s_music_player.state.total_time_ms = 0;
    }

    /* Song name */
    if (s_music_player.state.total_songs > 0 &&
        (s_music_player.state.mode == MUSIC_PLAYER_MODE_LOCAL_PLAYBACK ||
         s_music_player.state.mode == MUSIC_PLAYER_MODE_A2DP_SRC))
    {
        music_player_fill_song_name(s_music_player.state.song_name, &s_music_player.state.song_name_len,
                                    s_music_player.state.current_index);
    }
    else
    {
        s_music_player.state.song_name[0] = '\0';
        s_music_player.state.song_name_len = 0;
    }
}

/**
 * @brief collect current music player playlist from app and update the global music player playlist variable.
 */
static void music_player_collect_playlist(void)
{
    uint16_t count = playback_get_song_count();
    uint16_t cur = playback_get_cur_play_index();

    if (!music_player_playlist_items_delete())
    {
        return;
    }

    s_music_player.playlist.current_index = cur;
    s_music_player.playlist.item_capacity = (count > MUSIC_PLAYER_PLAYLIST_WINDOW_SIZE) ?
                                            MUSIC_PLAYER_PLAYLIST_WINDOW_SIZE : count;

    if (!music_player_playlist_items_create(s_music_player.playlist.item_capacity))
    {
        s_music_player.playlist.item_capacity = 0;
        return;
    }

    for (uint16_t i = 0; i < s_music_player.playlist.item_capacity; i++)
    {
        music_player_fill_song_name(s_music_player.playlist.items[i].name,
                                    &s_music_player.playlist.items[i].name_len,
                                    i);
        s_music_player.playlist.items[i].duration_ms = music_player_duration_cache_get(i);
    }
}

/**
 * @brief collect current music player progress from app and update the global music player state variable.
 */
static void music_player_collect_progress(void)
{
    /* Reuse state struct for progress-only update */
    if ((s_music_player.state.mode == MUSIC_PLAYER_MODE_LOCAL_PLAYBACK ||
         s_music_player.state.mode == MUSIC_PLAYER_MODE_A2DP_SRC) &&
        g_curr_song != NULL)
    {
        uint32_t pos_ms = 0, total_ms = 0;
        playback_get_source_song_time(g_curr_song, &pos_ms, &total_ms);
        s_music_player.state.pos_time_ms = pos_ms;
        s_music_player.state.total_time_ms = total_ms;
    }
    else
    {
        s_music_player.state.pos_time_ms = 0;
        s_music_player.state.total_time_ms = 0;
    }
}

/**
 * @brief publish current music player state to GUI.
 */
static void music_player_publish_state_app_to_gui(void)
{
    music_player_collect_state();
    gui_msg_publish(GUI_TOPIC_MUSIC_PLAYER_STATE, &s_music_player.state, sizeof(s_music_player.state));
}

/**
 * @brief publish current music player playlist to GUI.
 */
static void music_player_publish_playlist_app_to_gui(void)
{
    music_player_collect_playlist();
    gui_msg_publish(GUI_TOPIC_MUSIC_PLAYER_PLAYLIST, &s_music_player.playlist,
                    sizeof(s_music_player.playlist));
}

/**
 * @brief publish current music player progress to GUI.
 */
static void music_player_publish_progress_app_to_gui(void)
{
    music_player_collect_progress();
    gui_msg_publish(GUI_TOPIC_MUSIC_PLAYER_PROGRESS, &s_music_player.state,
                    sizeof(s_music_player.state));
}

/**
 * @brief handle async events from GUI, such as state/playlist/progress request and play control commands.
 *
 * @param[in] event_data The event data from event bus, should contain the topic and optional data for the command.
 * @return int32_t The result of handling the event, currently always returns 0.
 */
static int32_t app_music_player_async_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    APP_PRINT_TRACE1("[music_player] received async event topic: %s", TRACE_STRING(topic));
    if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PLAYER_STATE) == 0)
    {
        music_player_publish_state_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PLAYLIST_STATE) == 0)
    {
        music_player_publish_playlist_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PROGRESS_STATE) == 0)
    {
        music_player_publish_progress_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PLAY_PAUSE) == 0)
    {
        app_mmi_handle_action(MMI_AV_PLAY_PAUSE);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_NEXT) == 0)
    {
        app_mmi_handle_action(MMI_AV_FWD);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PREV) == 0)
    {
        app_mmi_handle_action(MMI_AV_BWD);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_VOL_UP) == 0)
    {
        app_mmi_handle_action(MMI_DEV_SPK_VOL_UP);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_VOL_DOWN) == 0)
    {
        app_mmi_handle_action(MMI_DEV_SPK_VOL_DOWN);
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PLAY_BY_INDEX) == 0)
    {
        /* GUI publishes the selected index as a raw uint16_t (see
         * playlist_note_clicked); read it width-tolerantly instead of demanding
         * the full T_MUSIC_PLAYER_GUI_TO_APP_EVENT_DATA struct size, otherwise a
         * size mismatch silently drops the command and list switching does
         * nothing. Mirrors the recording bridge START_PLAYING handler. */
        if (event_data->data != NULL &&
            event_data->data_len >= sizeof(uint16_t))
        {
            T_MUSIC_PLAYER_GUI_TO_APP_EVENT_DATA data;
            data.current_index = *(uint16_t *)event_data->data;
            playback_play_select_music(data.current_index);
        }
    }
    return 0;
}

/**
 * @brief handle audio-related events from app, such as volume/play status/index changes, and publish updated state to GUI.
 *
 * @param[in] event_data The event data from event bus, should contain the topic and optional data for the command.
 * @return int32_t The result of handling the event, currently always returns 0.
 */
static int32_t app_music_player_audio_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    if (strcmp(topic, EVENT_BUS_TOPIC_AUDIO_VOLUME_UP) == 0
        || strcmp(topic, EVENT_BUS_TOPIC_AUDIO_VOLUME_DOWN) == 0)
    {
        s_music_player.state.volume = *(uint8_t *)event_data->data;
        gui_msg_publish(GUI_TOPIC_MUSIC_PLAYER_STATE, &s_music_player.state, sizeof(s_music_player.state));
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_AUDIO_PLAY_STATUS_CHANGED) == 0)
    {
        s_music_player.state.play_status = music_player_map_play_status(*(T_APP_AUDIO_STATE *)
                                                                        event_data->data);
        gui_msg_publish(GUI_TOPIC_MUSIC_PLAYER_STATE, &s_music_player.state, sizeof(s_music_player.state));
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_AUDIO_PLAY_INDEX_UPDATE) == 0)
    {
        s_music_player.state.current_index = *(uint16_t *)event_data->data;
        gui_msg_publish(GUI_TOPIC_MUSIC_PLAYER_STATE, &s_music_player.state, sizeof(s_music_player.state));
    }
    else
    {
        APP_PRINT_INFO1("[music_player] received unknown audio event topic: %s", TRACE_STRING(topic));
    }

    return EVENT_BUS_OK;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/
bool music_player_gui_to_app(const char *topic, void *data, uint32_t size)
{
    return event_bus_publish(topic, data, size) == EVENT_BUS_OK;
}

void bridge_music_player_init(void)
{
    /*create music player topic*/
    event_bus_topic_register(EVENT_BUS_TOPIC_MUSIC_PLAYER_ALL_TOPIC);
    /*subscribe to music player topic asynchronously*/
    event_bus_subscribe_async(&s_music_player_async_handle,
                              EVENT_BUS_TOPIC_MUSIC_PLAYER_ALL_TOPIC,
                              event_bus_async_send_to_apptask,
                              NULL,
                              app_music_player_async_event_callback);

    /*subscribe to audio topic*/
    event_bus_subscribe(&s_audio_handle,
                        EVENT_BUS_TOPIC_AUDIO_ALL_TOPIC,
                        app_music_player_audio_event_callback);
    APP_PRINT_INFO0("[music_player] initialized");
}