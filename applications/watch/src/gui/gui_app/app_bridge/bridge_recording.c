/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
*                        Header Files
*============================================================================*/
#include "event_bus.h"
#include "bridge_recording.h"
#include "gui_listener.h"
#include "trace.h"
#include "app_main.h"
#include "app_task.h"
#include "app_mmi.h"
#include "audio_record.h"
#include "record_playlist.h"
#include "audio_resource.h"

/*============================================================================*
 *                            Macros
 *============================================================================*/
#define RECORDING_FILE_PATH_BUF_LEN   (sizeof(RECORD_FILE_PATH) + RECORDING_FILE_NAME_MAX_LEN)

/*============================================================================*
 *                           Types
 *============================================================================*/

/*============================================================================*
 *                           Constants
 *============================================================================*/

/*============================================================================*
 *                            Variables
 *============================================================================*/

static T_RECORDING s_recording;
static int16_t s_recording_current_index = -1;
static T_EVENT_BUS_SUBSCRIBER_HANDLE s_recording_async_handle;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

/**
* @brief build file path from file name like /SD:/record/record0001.pcm
*
* @param[in] file_name The name of the recording file, should be null-terminated string.
* @param[out] path The buffer to store the built file path, should be at least RECORDING_FILE_PATH_BUF_LEN bytes.
* @param[in] path_len The length of the path buffer.
* @return true if the file path is built successfully, false otherwise.
*/
static bool recording_build_file_path_from_name(const char *file_name, char *path, size_t path_len)
{
    int written_len;

    if (file_name == NULL || file_name[0] == '\0' || path == NULL || path_len == 0)
    {
        return false;
    }

    written_len = snprintf(path, path_len, "%s%s", RECORD_FILE_PATH, file_name);
    if (written_len <= 0 || (size_t)written_len >= path_len)
    {
        return false;
    }

    return record_playlist_ensure_file_path(path);
}

static uint16_t recording_resolve_current_index(uint16_t file_count)
{
    uint16_t current_index;

    if (file_count == 0)
    {
        s_recording_current_index = -1;
        return 0;
    }

    if (s_recording_current_index >= 0 && (uint16_t)s_recording_current_index < file_count)
    {
        return (uint16_t)s_recording_current_index;
    }

    current_index = record_get_cur_play_index();
    if (current_index >= file_count)
    {
        current_index = 0;
    }

    return current_index;
}

static void recording_fill_file_name(char *out, uint16_t *out_len, uint16_t index)
{
    T_HEAD_INFO *headers = record_get_header_info_start();
    uint16_t count = record_get_file_count();

    if (index < count && headers != NULL)
    {
        T_HEAD_INFO *hdr = &headers[index];
        uint8_t *name_ptr = (uint8_t *)(RECORD_NAME_BIN_ADDR + hdr->offset);
        uint16_t len = hdr->length;

        APP_PRINT_TRACE3("[recording] file name: %s, name_ptr: %p, len: %d",
                         TRACE_STRING(name_ptr), name_ptr, len);

        if (len > RECORDING_FILE_NAME_MAX_LEN - 1)
        {
            len = RECORDING_FILE_NAME_MAX_LEN - 1;
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


static bool recording_build_file_path(uint16_t index, char *path, size_t path_len)
{
    char file_name[RECORDING_FILE_NAME_MAX_LEN] = {0};
    uint16_t file_name_len = 0;

    if (path == NULL || path_len == 0)
    {
        return false;
    }

    recording_fill_file_name(file_name, &file_name_len, index);
    if (file_name_len == 0)
    {
        return false;
    }

    return recording_build_file_path_from_name(file_name, path, path_len);
}

static bool recording_start_playback(uint16_t index)
{
    char play_path[RECORDING_FILE_PATH_BUF_LEN] = {0};
    uint16_t file_count = record_get_file_count();

    if (index >= file_count)
    {
        APP_PRINT_ERROR2("[recording] invalid play index %d, file_count %d", index, file_count);
        return false;
    }

    if (!recording_build_file_path(index, play_path, sizeof(play_path)))
    {
        APP_PRINT_ERROR1("[recording] build play path failed for index %d", index);
        return false;
    }

    if (audio_record_is_playing())
    {
        audio_record_stop_playing();
    }

    if (!audio_record_init_player(AUDIO_RECORD_SAVE_FS, play_path))
    {
        APP_PRINT_ERROR1("[recording] init player failed for %s", TRACE_STRING(play_path));
        return false;
    }

    s_recording_current_index = (int16_t)index;
    return true;
}

static uint32_t recording_calc_duration_ms(uint32_t file_size)
{
    return audio_record_get_duration_ms_by_file(file_size);
}

static void recording_fill_file_meta(T_RECORDING_FILE *file)
{
    char file_path[RECORDING_FILE_PATH_BUF_LEN] = {0};
    struct fs_file_t file_handle;
    int fs_res;
    size_t file_size = 0;

    if (file == NULL || file->name_len == 0)
    {
        return;
    }

    if (!recording_build_file_path_from_name(file->name, file_path, sizeof(file_path)))
    {
        APP_PRINT_WARN1("[recording] build record file path failed: %s", TRACE_STRING(file->name));
        return;
    }

    fs_file_t_init(&file_handle);
    fs_res = fs_open(&file_handle, file_path, FS_O_READ);
    if (fs_res < 0)
    {
        APP_PRINT_WARN1("[recording] open record file failed: %s", TRACE_STRING(file_path));
        return;
    }

    if (fs_size(&file_handle, &file_size) != 0)
    {
        file_size = 0;
    }

    file->file_size = (uint32_t)file_size;
    file->duration_ms = recording_calc_duration_ms(file->file_size);
    fs_close(&file_handle);
}

static bool recording_playlist_items_create(uint16_t required_count)
{
    T_RECORDING_FILE *new_items;

    if (required_count == 0)
    {
        return true;
    }

    new_items = (T_RECORDING_FILE *)malloc(sizeof(T_RECORDING_FILE) * required_count);
    if (new_items == NULL)
    {
        APP_PRINT_ERROR0("[recording] allocate playlist items failed");
        return false;
    }

    memset(new_items, 0, sizeof(T_RECORDING_FILE) * required_count);

    s_recording.playlist.files = new_items;
    s_recording.playlist.file_count = required_count;
    return true;
}

static bool recording_playlist_items_delete(void)
{
    if (s_recording.playlist.files != NULL)
    {
        free(s_recording.playlist.files);
        s_recording.playlist.files = NULL;
        s_recording.playlist.file_count = 0;
    }

    return true;
}

static void recording_collect_state(void)
{
    s_recording.state.file_count = record_get_file_count();
    s_recording.state.current_index = recording_resolve_current_index(s_recording.state.file_count);
    s_recording.state.duration_ms = 0;
    s_recording.state.total_duration_ms = 0;

    if (audio_record_is_recording())
    {
        s_recording.state.status = RECORDING_STATUS_RECORDING;
        s_recording.state.mode = RECORDING_MODE_FILESYSTEM;
        s_recording.state.duration_ms = audio_record_get_current_time_ms();
        s_recording.state.total_duration_ms = audio_record_get_total_time_ms();
    }
    else if (audio_record_is_playing())
    {
        s_recording.state.status = RECORDING_STATUS_PLAYING;
        s_recording.state.mode = RECORDING_MODE_FILESYSTEM;
        s_recording.state.duration_ms = audio_record_get_current_time_ms();
        s_recording.state.total_duration_ms = audio_record_get_total_time_ms();
    }
    else
    {
        s_recording.state.status = RECORDING_STATUS_IDLE;
        s_recording.state.mode = RECORDING_MODE_NONE;
    }

    APP_PRINT_INFO3("[recording] collect state: status %d, mode %d, file_count %d",
                    s_recording.state.status, s_recording.state.mode, s_recording.state.file_count);
}

static void recording_collect_playlist(void)
{
    uint16_t count = record_get_file_count();
    uint16_t cur = recording_resolve_current_index(count);
    uint16_t playlist_count = count;

    if (playlist_count > RECORDING_PLAYLIST_WINDOW_SIZE)
    {
        playlist_count = RECORDING_PLAYLIST_WINDOW_SIZE;
    }

    if (!recording_playlist_items_delete())
    {
        return;
    }

    s_recording.playlist.current_index = cur;

    if (!recording_playlist_items_create(playlist_count))
    {
        return;
    }

    for (uint16_t index = 0; index < playlist_count; index++)
    {
        recording_fill_file_name(s_recording.playlist.files[index].name,
                                 &s_recording.playlist.files[index].name_len,
                                 index);
        recording_fill_file_meta(&s_recording.playlist.files[index]);
    }

    APP_PRINT_INFO2("[recording] collect playlist: count %d, current_index %d",
                    playlist_count, cur);
}

static void recording_publish_recorder_state_app_to_gui(void)
{
    recording_collect_state();
    gui_msg_publish(GUI_TOPIC_RECORDING_RECORDER_STATE, &s_recording.state, sizeof(s_recording.state));
}

static void recording_publish_recoder_file_app_to_gui(void)
{
    recording_collect_playlist();
    gui_msg_publish(GUI_TOPIC_RECORDING_RECORDER_FILE, &s_recording.playlist,
                    sizeof(s_recording.playlist));
}

static void recording_publish_player_state_app_to_gui(void)
{
    recording_collect_state();
    gui_msg_publish(GUI_TOPIC_RECORDING_PLAYER_STATE, &s_recording.state, sizeof(s_recording.state));
}

static void recording_publish_playlist_app_to_gui(void)
{
    recording_collect_playlist();
    gui_msg_publish(GUI_TOPIC_RECORDING_PLAYLIST, &s_recording.playlist, sizeof(s_recording.playlist));
}

static void recording_publish_progress_app_to_gui(void)
{
    recording_collect_state();
    gui_msg_publish(GUI_TOPIC_RECORDING_PROGRESS, &s_recording.state, sizeof(s_recording.state));
}

static int32_t recording_async_event_callback(T_EVENT_BUS_EVENT_DATA *event_data)
{
    const char *topic = event_data->topic;

    APP_PRINT_TRACE1("[recording] received async event topic: %s", TRACE_STRING(topic));
    if (strcmp(topic, EVENT_BUS_TOPIC_REQ_RECORDING_RECORDER_STATE) == 0)
    {
        recording_publish_recorder_state_app_to_gui();
    }
    else if (strcmp(topic, (const char *)EVENT_BUS_TOPIC_REQ_RECORDING_PLAYLIST_STATE) == 0)
    {
        recording_publish_playlist_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_REQ_RECORDING_RECORDER_FILE) == 0)
    {
        recording_publish_recoder_file_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_REQ_RECORDING_PROGRESS_STATE) == 0)
    {
        recording_publish_progress_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_REQ_RECORDING_PLAYER_STATE) == 0)
    {
        recording_publish_player_state_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_RECORDING_CMD_START_RECORDING_FROM_FS) == 0)
    {
        app_mmi_handle_action(MMI_RECORD_START);
        recording_publish_recorder_state_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_RECORDING_CMD_STOP_RECORDING_FROM_FS) == 0)
    {
        app_mmi_handle_action(MMI_RECORD_STOP);
        recording_publish_recorder_state_app_to_gui();
        /* The recording main view (where the "Saved to <file>" message lives)
         * subscribes to RECORDER_FILE, not PLAYLIST. Publish RECORDER_FILE so
         * the freshly-saved file reaches it; otherwise the message never leaves
         * "Saving...". */
        recording_publish_recoder_file_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_RECORDING_CMD_START_PLAYING_FROM_FS) == 0)
    {
        T_RECORDING_GUI_TO_APP_EVENT_DATA data;
        data.current_index = *(uint16_t *)event_data->data;
        recording_start_playback(data.current_index);
        recording_publish_playlist_app_to_gui();
        recording_publish_player_state_app_to_gui();
        recording_publish_progress_app_to_gui();
    }
    else if (strcmp(topic, EVENT_BUS_TOPIC_RECORDING_CMD_STOP_PLAYING_FROM_FS) == 0)
    {
        app_mmi_handle_action(MMI_RECORD_PLAY_STOP);
        recording_publish_player_state_app_to_gui();
        recording_publish_progress_app_to_gui();
    }
    else
    {
        APP_PRINT_INFO1("[recording] received unknown async event topic: %s", TRACE_STRING(topic));
    }


    return EVENT_BUS_OK;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void bridge_recording_init(void)
{
    event_bus_topic_register(EVENT_BUS_TOPIC_RECORDING_ALL_TOPIC);
    event_bus_subscribe_async(&s_recording_async_handle,
                              EVENT_BUS_TOPIC_RECORDING_ALL_TOPIC,
                              event_bus_async_send_to_apptask,
                              NULL,
                              recording_async_event_callback);

    APP_PRINT_INFO0("[recording] initialized");
}

bool recording_gui_to_app(const char *topic, void *data, uint32_t size)
{
    return event_bus_publish(topic, data, size) == EVENT_BUS_OK;
}