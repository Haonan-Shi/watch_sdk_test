/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include "app_recording_user.h"
#include "app_recording_ui.h"
#include "gui_listener.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "gui_obj.h"
#include "gui_view.h"
#include "gui_text.h"
#include "gui_arc.h"
#include "gui_list.h"
#include "gui_rect.h"

#ifndef _HONEYGUI_SIMULATOR_
#include "bridge_recording.h"
#endif

#define WAVEFORM_FRAME_COUNT 30
#define RECORDING_DURATION_BUF_COUNT  24
#define RECORDING_DURATION_TEXT_LEN   12

static int recording_file_count = 0;
static int next_recording_number = 1;
static int selected_recording_index = -1;
static char recording_timer_text[16] = "00:00";
static char playback_current_time_text[16] = "00:00";
static char playback_total_time_text[16] = "/ 00:00";

static bool recording_active = false;
static int recording_elapsed_seconds = 0;
static char saved_recording_message[48] = "";
static int waveform_frame_index = 0;
static int waveform_tick_divider = 0;
static int recording_breath_phase = 0;
static uint8_t recording_status_opacity = 255;
static bool pending_saved_recording_message = false;

static int playback_elapsed_ticks = 0;
static int selected_playback_index = -1;

/*============================================================================*
 *                         Render State (from backend)
 *============================================================================*/
/*
 * gui_msg_publish() copies the payload into a heap buffer that gui_listener
 * frees immediately after the topic callback returns (see msg_task_exec()).
 * The list note_design callback runs LATER (deferred, keep_note_alive=true),
 * so cache the payload in persistent storage here instead of the transient
 * `data` pointer -- otherwise later dereferences read freed memory (garbled
 * file names/durations, wrong file count).
 */
static T_RECORDING_STATE s_state_storage;
static T_RECORDING_PLAYLIST s_playlist_storage;
static T_RECORDING_STATE *s_render_state;
static const T_RECORDING_PLAYLIST *s_render_playlist;
static char s_duration_bufs[RECORDING_DURATION_BUF_COUNT][RECORDING_DURATION_TEXT_LEN];

static bool is_current_view(const char *view_name);
static const T_RECORDING_FILE *get_selected_playback_file(void);
static void update_saved_recording_message_from_playlist(void);
static void update_recording_timer_display(void);
static void update_recording_status_display(void);
static void update_waveform_frame_display(void);
static void update_playback_progress_display(void);

/*============================================================================*
 *                         Render Helpers
 *============================================================================*/

static void render_recording_files_view(void)
{
    uint16_t total = 0;

    if (s_render_playlist != NULL)
    {
        total = s_render_playlist->file_count;
    }

    if (recording_files_list != NULL)
    {
        gui_list_set_note_num(recording_files_list, 0);
        gui_list_set_note_num(recording_files_list, total);
        gui_list_set_offset(recording_files_list, 0);
    }

    if (recording_files_empty_label != NULL)
    {
        gui_obj_hidden((gui_obj_t *)recording_files_list, total == 0);
        gui_obj_hidden((gui_obj_t *)recording_files_empty_label, total != 0);
    }
    gui_log("[app_recording] render recording files view, total %d", total);
}

static void render_recording_playback_view(void)
{
    (void)get_selected_playback_file();

    if (s_render_state != NULL)
    {
        playback_elapsed_ticks = (int)(s_render_state->duration_ms / 100);
    }

    update_playback_progress_display();

    if (playback_play_btn != NULL && s_render_state != NULL)
    {
        playback_play_btn_set_state(s_render_state->status == RECORDING_STATUS_PLAYING);
    }
}

static void render_recording_main_view(void)
{
    if (s_render_state != NULL)
    {
        recording_elapsed_seconds = (int)(s_render_state->duration_ms / 1000);
    }

    update_recording_timer_display();
    update_waveform_frame_display();
    update_recording_status_display();
}

/*============================================================================*
 *                         Topic Callbacks (GUI thread)
 *============================================================================*/
void app_recording_topic_recorder_state_cb(gui_obj_t *obj, const char *topic, void *data,
                                           uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);

    if (data != NULL && len >= sizeof(T_RECORDING_STATE))
    {
        memcpy(&s_state_storage, data, sizeof(s_state_storage));
        s_render_state = &s_state_storage;
        gui_log("[app_recording] topic recorder state: status %d, mode %d, file_count %d",
                s_render_state->status, s_render_state->mode, s_render_state->file_count);

        if (is_current_view("app_recordingMainView"))
        {
            render_recording_main_view();
            gui_fb_change();
        }

        if (is_current_view("app_recordingPlaybackView"))
        {
            render_recording_playback_view();
            gui_fb_change();
        }

    }
}

static void app_recording_topic_playlist_cb(gui_obj_t *obj, const char *topic, void *data,
                                            uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    if (data != NULL && len >= sizeof(T_RECORDING_PLAYLIST))
    {
        memcpy(&s_playlist_storage, data, sizeof(s_playlist_storage));
        s_render_playlist = &s_playlist_storage;
        gui_log("[app_recording] topic playlist: file_count %d, current_index %d",
                s_render_playlist->file_count, s_render_playlist->current_index);

        update_saved_recording_message_from_playlist();
        render_recording_files_view();
        gui_fb_change();
    }
}

static void app_recording_topic_recorder_file_cb(gui_obj_t *obj, const char *topic, void *data,
                                                 uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    if (data != NULL && len >= sizeof(T_RECORDING_PLAYLIST))
    {
        memcpy(&s_playlist_storage, data, sizeof(s_playlist_storage));
        s_render_playlist = &s_playlist_storage;
        gui_log("app_recording_topic_recorder_file_cb");
        update_saved_recording_message_from_playlist();
    }
}

static void app_recording_topic_progress_cb(gui_obj_t *obj, const char *topic, void *data,
                                            uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);

    if (data != NULL && len >= sizeof(T_RECORDING_STATE))
    {
        memcpy(&s_state_storage, data, sizeof(s_state_storage));
        s_render_state = &s_state_storage;

        if (is_current_view("app_recordingMainView"))
        {
            render_recording_main_view();
        }

        if (is_current_view("app_recordingPlaybackView"))
        {
            render_recording_playback_view();
        }
        gui_fb_change();
    }
}

void app_recording_topic_player_state_cb(gui_obj_t *obj, const char *topic, void *data,
                                         uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);

    if (data != NULL && len >= sizeof(T_RECORDING_STATE))
    {
        memcpy(&s_state_storage, data, sizeof(s_state_storage));
        s_render_state = &s_state_storage;
        gui_log("[app_recording] topic player state: status %d, mode %d, file_count %d",
                s_render_state->status, s_render_state->mode, s_render_state->file_count);

        render_recording_playback_view();
        gui_fb_change();
    }
}

/*============================================================================*
 *                         Note Design Implementation
 *============================================================================*/
void recording_files_note_design_impl(gui_obj_t *obj)
{
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t actual_index = (uint16_t)note->index;
    uint16_t total = 0;

    if (s_render_playlist != NULL)
    {
        total = s_render_playlist->file_count;
    }

    if (actual_index >= total)
    {
        return;
    }

    gui_obj_child_free((gui_obj_t *)note);

    /* Get file data from playlist */
    const char *title_text = "Unknown";
    uint16_t title_len = 7;
    uint32_t duration_ms = 0;

    if (s_render_playlist != NULL && s_render_playlist->file_count > actual_index)
    {
        title_text = s_render_playlist->files[actual_index].name;
        title_len = s_render_playlist->files[actual_index].name_len;
        duration_ms = s_render_playlist->files[actual_index].duration_ms;
        gui_log("[app_recording] note design: index %d, title %s, duration_ms %d",
                actual_index, title_text, duration_ms);
    }

    bool is_current = (s_render_playlist != NULL && actual_index == s_render_playlist->current_index);
    gui_color_t title_color = is_current ? gui_rgb(48, 209, 88) : gui_rgb(242, 242, 242);
    gui_color_t duration_color = is_current ? gui_rgb(48, 209, 88) : gui_rgb(153, 153, 153);
    gui_color_t bg_color = is_current ? gui_rgba(48, 209, 88, 38) : gui_rgba(0, 0, 0, 0);

    /* Create background */
    gui_rect_create((gui_obj_t *)note, "recording_file_bg", 32, 0, 346, 84, 16, bg_color);

    /* Create file name */
    gui_text_t *name = gui_text_create((gui_obj_t *)note, "recording_file_name", 52, 10, 280, 50);
    gui_text_set(name, (char *)title_text, GUI_FONT_SRC_BMP, title_color, title_len, 36);
    gui_text_type_set(name, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(name, LEFT);

    /* Format duration */
    uint16_t buf_index = actual_index % RECORDING_DURATION_BUF_COUNT;
    if (duration_ms > 0)
    {
        uint32_t total_sec = duration_ms / 1000;
        snprintf(s_duration_bufs[buf_index], sizeof(s_duration_bufs[buf_index]),
                 "%02u:%02u", total_sec / 60, total_sec % 60);
    }
    else
    {
        snprintf(s_duration_bufs[buf_index], sizeof(s_duration_bufs[buf_index]), "--:--");
    }

    /* Create duration */
    gui_text_t *duration = gui_text_create((gui_obj_t *)note, "recording_file_duration", 52, 56, 120,
                                           38);
    gui_text_set(duration, s_duration_bufs[buf_index], GUI_FONT_SRC_BMP, duration_color,
                 strlen(s_duration_bufs[buf_index]), 28);
    gui_text_type_set(duration, "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(duration, LEFT);

    /* Add click event */
    gui_obj_add_event_cb(obj, recording_file_note_clicked, GUI_EVENT_TOUCH_CLICKED, NULL);
}

/*============================================================================*
 *                         View Init Callbacks
 *============================================================================*/
void recording_files_init_cb_impl(void)
{
    /* Subscribe to topics */
    gui_msg_subscribe((gui_obj_t *)recording_files_list, GUI_TOPIC_RECORDING_RECORDER_STATE,
                      app_recording_topic_recorder_state_cb);
    gui_msg_subscribe((gui_obj_t *)recording_files_list, GUI_TOPIC_RECORDING_PLAYLIST,
                      app_recording_topic_playlist_cb);

#ifndef _HONEYGUI_SIMULATOR_
    /* Request fresh playlist snapshot from backend */
    recording_gui_to_app(EVENT_BUS_TOPIC_REQ_RECORDING_PLAYLIST_STATE, NULL, 0);
#else
    render_recording_files_view();
    gui_fb_change();
#endif
}

void recording_playback_init_cb_impl(void)
{
    gui_msg_subscribe((gui_obj_t *)playback_current_time_label, GUI_TOPIC_RECORDING_PLAYER_STATE,
                      app_recording_topic_player_state_cb);
    /* Subscribe to progress topic */
    gui_msg_subscribe((gui_obj_t *)playback_current_time_label, GUI_TOPIC_RECORDING_PROGRESS,
                      app_recording_topic_progress_cb);

#ifndef _HONEYGUI_SIMULATOR_
    recording_gui_to_app(EVENT_BUS_TOPIC_REQ_RECORDING_PLAYER_STATE, NULL, 0);
#else
    render_recording_playback_view();
    gui_fb_change();
#endif
}

static void set_text_content(gui_text_t *text_obj, const char *text)
{
    if (!text_obj || !text)
    {
        return;
    }

    gui_text_content_set(text_obj, (char *)text, strlen(text));
}

static void format_time_text(int seconds, char *buffer, size_t buffer_size)
{
    unsigned int total_seconds;

    if (!buffer || buffer_size == 0)
    {
        return;
    }

    total_seconds = seconds > 0 ? (unsigned int)seconds : 0U;
    snprintf(buffer, buffer_size, "%02u:%02u", total_seconds / 60U, total_seconds % 60U);
}

static bool is_current_view(const char *view_name)
{
    gui_view_t *current_view = gui_view_get_current();

    /* main view -> playlist view, exe in playlist swtich in
     * call gui_view_get_current(), will still gei main view*/

    return current_view != NULL && current_view->base.name != NULL &&
           strcmp(current_view->base.name, view_name) == 0;
}

static const T_RECORDING_FILE *get_selected_playback_file(void)
{
    if (s_render_playlist == NULL || s_render_playlist->file_count == 0 ||
        s_render_playlist->files == NULL)
    {
        return NULL;
    }

    if (selected_playback_index < 0 || selected_playback_index >= s_render_playlist->file_count)
    {
        if (s_render_playlist->current_index < s_render_playlist->file_count)
        {
            selected_playback_index = s_render_playlist->current_index;
        }
        else
        {
            return NULL;
        }
    }

    return &s_render_playlist->files[selected_playback_index];
}

static void update_saved_recording_message_from_playlist(void)
{
    const T_RECORDING_FILE *latest_file;

    if (!pending_saved_recording_message || s_render_playlist == NULL ||
        s_render_playlist->files == NULL || s_render_playlist->file_count == 0)
    {
        return;
    }

    latest_file = &s_render_playlist->files[s_render_playlist->file_count - 1];
    if (latest_file->name_len == 0)
    {
        return;
    }

    snprintf(saved_recording_message, sizeof(saved_recording_message),
             "Saved to %.*s", (int)latest_file->name_len, latest_file->name);
    pending_saved_recording_message = false;
    gui_log("update_saved_recording_message_from_playlist %s ", saved_recording_message);
}

static void update_recording_timer_display(void)
{
    format_time_text(recording_elapsed_seconds, recording_timer_text, sizeof(recording_timer_text));
    set_text_content((gui_text_t *)recording_timer_label, recording_timer_text);
}

static void update_recording_status_display(void)
{
    if (!recording_status_label)
    {
        return;
    }

#ifndef _HONEYGUI_SIMULATOR_
    if (s_render_state != NULL && s_render_state->status == 1)
    {
        set_text_content((gui_text_t *)recording_status_label, "Recording...");
        ((gui_obj_t *)recording_status_label)->opacity_value = recording_status_opacity;
        return;
    }
#else
    if (false)
    {
        return;
    }
#endif

    ((gui_obj_t *)recording_status_label)->opacity_value = 255;
    set_text_content((gui_text_t *)recording_status_label,
                     saved_recording_message[0] != '\0' ? saved_recording_message : "Ready");
}

static void update_waveform_frame_display(void)
{
    char frame_path[96];

    if (!recording_waveform_image)
    {
        return;
    }

    bool is_recording = (s_render_state != NULL && s_render_state->status == 1);
    snprintf(frame_path,
             sizeof(frame_path),
             "/app_recording/waveform/%s/frame_%02d.bin",
             is_recording ? "active" : "inactive",
             waveform_frame_index);
    gui_img_set_src((gui_img_t *)recording_waveform_image, (const uint8_t *)frame_path,
                    IMG_SRC_FILESYS);
}

static void update_playback_progress_display(void)
{
    const T_RECORDING_FILE *selected_file;
    int playback_current_seconds = playback_elapsed_ticks / 10;
    float progress = 0.0f;
    uint32_t duration_ms = 0;

    selected_file = get_selected_playback_file();
    if (selected_file != NULL)
    {
        set_text_content((gui_text_t *)playback_file_name_label, selected_file->name);
    }
    else
    {
        set_text_content((gui_text_t *)playback_file_name_label, "");
    }

    if (s_render_state != NULL && s_render_state->total_duration_ms > 0)
    {
        duration_ms = s_render_state->total_duration_ms;
    }
    else if (selected_file != NULL)
    {
        duration_ms = selected_file->duration_ms;
    }

    if (duration_ms == 0)
    {
        strcpy(playback_current_time_text, "00:00");
        strcpy(playback_total_time_text, selected_file != NULL ? "/ --:--" : "/ 00:00");
        set_text_content((gui_text_t *)playback_current_time_label, playback_current_time_text);
        set_text_content((gui_text_t *)playback_total_time_label, playback_total_time_text);
        if (playback_progress_fg)
        {
            gui_arc_set_end_angle((gui_arc_t *)playback_progress_fg, -90.0f);
        }
        return;
    }

    uint32_t duration_sec = duration_ms / 1000;
    if (duration_sec > 0)
    {
        progress = (float)playback_current_seconds / (float)duration_sec;
        if (progress < 0.0f)
        {
            progress = 0.0f;
        }
        if (progress > 1.0f)
        {
            progress = 1.0f;
        }
    }

    format_time_text(playback_current_seconds, playback_current_time_text,
                     sizeof(playback_current_time_text));
    snprintf(playback_total_time_text,
             sizeof(playback_total_time_text),
             "/ %02u:%02u",
             duration_sec / 60,
             duration_sec % 60);

    set_text_content((gui_text_t *)playback_current_time_label, playback_current_time_text);
    set_text_content((gui_text_t *)playback_total_time_label, playback_total_time_text);

    if (playback_progress_fg)
    {
        gui_arc_set_end_angle((gui_arc_t *)playback_progress_fg, -90.0f + 360.0f * progress);
    }
}

/*============================================================================*
 *                         Public List API (for UI compatibility)
 *============================================================================*/

void recording_main_init_cb_impl(void)
{
    gui_msg_subscribe((gui_obj_t *)recording_timer_label, GUI_TOPIC_RECORDING_RECORDER_STATE,
                      app_recording_topic_recorder_state_cb);
    /*for update saved recording file name*/
    gui_msg_subscribe((gui_obj_t *)recording_timer_label, GUI_TOPIC_RECORDING_RECORDER_FILE,
                      app_recording_topic_recorder_file_cb);
    gui_msg_subscribe((gui_obj_t *)recording_timer_label, GUI_TOPIC_RECORDING_PROGRESS,
                      app_recording_topic_progress_cb);

    waveform_tick_divider = 0;
    recording_breath_phase = 0;
    recording_status_opacity = 255;
    playback_elapsed_ticks = 0;

    if (recording_record_btn_get_state())
    {
        recording_record_btn_set_state(false);
    }

#ifndef _HONEYGUI_SIMULATOR_
    recording_gui_to_app(EVENT_BUS_TOPIC_REQ_RECORDING_RECORDER_STATE, NULL, 0);
    recording_gui_to_app(EVENT_BUS_TOPIC_REQ_RECORDING_RECORDER_FILE, NULL, 0);
#else
    saved_recording_message[0] = '\0';
    pending_saved_recording_message = false;
    recording_elapsed_seconds = 0;
    waveform_frame_index = 0;
    selected_playback_index = -1;
    render_recording_main_view();
    gui_fb_change();
#endif
}

void recording_timer_tick_impl(void)
{
#ifndef _HONEYGUI_SIMULATOR_
    if (s_render_state == NULL || s_render_state->status != 1)
    {
        return;
    }

    recording_gui_to_app(EVENT_BUS_TOPIC_REQ_RECORDING_PROGRESS_STATE, NULL, 0);
#else
    recording_elapsed_seconds++;
    update_recording_timer_display();
    gui_fb_change();
#endif
}

void recording_waveform_timer_cb_impl(void)
{
#ifndef _HONEYGUI_SIMULATOR_
    bool is_recording = (s_render_state != NULL && s_render_state->status == 1);
#else
    bool is_recording = false;
#endif

    if (is_recording)
    {
        waveform_frame_index = (waveform_frame_index + 1) % WAVEFORM_FRAME_COUNT;
        recording_breath_phase = (recording_breath_phase + 1) % 60;
        if (recording_breath_phase < 30)
        {
            recording_status_opacity = (uint8_t)(110 + (recording_breath_phase * 145) / 29);
        }
        else
        {
            recording_status_opacity = (uint8_t)(110 + ((59 - recording_breath_phase) * 145) / 29);
        }
    }
    else
    {
        waveform_tick_divider = (waveform_tick_divider + 1) % 2;
        if (waveform_tick_divider == 0)
        {
            waveform_frame_index = (waveform_frame_index + 1) % WAVEFORM_FRAME_COUNT;
        }
        recording_status_opacity = 255;
    }

    update_waveform_frame_display();
    update_recording_status_display();
    gui_fb_change();
}

void playback_timer_tick_impl(void)
{
#ifndef _HONEYGUI_SIMULATOR_
    if (s_render_state == NULL || s_render_state->status != 2)
    {
        return;
    }

    recording_gui_to_app(EVENT_BUS_TOPIC_REQ_RECORDING_PROGRESS_STATE, NULL, 0);
#endif
}

void recording_start(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    recording_gui_to_app(EVENT_BUS_TOPIC_RECORDING_CMD_START_RECORDING_FROM_FS, NULL, 0);
#endif

    recording_elapsed_seconds = 0;
    saved_recording_message[0] = '\0';
    pending_saved_recording_message = false;
    waveform_frame_index = 0;
    waveform_tick_divider = 0;
    recording_breath_phase = 0;
    recording_status_opacity = 255;

    update_recording_timer_display();
    update_waveform_frame_display();
    update_recording_status_display();
    gui_fb_change();
}

void recording_stop(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    recording_gui_to_app(EVENT_BUS_TOPIC_RECORDING_CMD_STOP_RECORDING_FROM_FS, NULL, 0);
#endif

    waveform_frame_index = 0;
    waveform_tick_divider = 0;
    recording_status_opacity = 255;

    if (recording_elapsed_seconds > 0)
    {
        strcpy(saved_recording_message, "Saving...");
        pending_saved_recording_message = true;
    }
    else
    {
        saved_recording_message[0] = '\0';
        pending_saved_recording_message = false;
    }

    /* Do NOT resolve the saved message here: s_render_playlist still holds the
     * pre-recording snapshot, so files[file_count-1] is the PREVIOUS file, not
     * the one just recorded (this produced the off-by-one file number, e.g. new
     * file 0009 shown as 0008). Leave pending set; the fresh RECORDER_FILE
     * published by the bridge on stop drives update via the recorder_file
     * callback with the correct, newly-added file. */
    update_waveform_frame_display();
    update_recording_status_display();
    gui_fb_change();
}

void playback_play(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    if (selected_playback_index < 0)
    {
        return;
    }
    recording_gui_to_app(EVENT_BUS_TOPIC_RECORDING_CMD_START_PLAYING_FROM_FS, &selected_playback_index,
                         sizeof(uint16_t));
#endif

    playback_elapsed_ticks = 0;
    update_playback_progress_display();
    gui_fb_change();
}

void playback_pause(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    recording_gui_to_app(EVENT_BUS_TOPIC_RECORDING_CMD_STOP_PLAYING_FROM_FS, NULL, 0);
#endif

    update_playback_progress_display();
    gui_fb_change();
}

void recording_file_note_clicked(void *obj, gui_event_t *e)
{
    GUI_UNUSED(e);

    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = (uint16_t)note->index;
    uint16_t total = 0;

    if (s_render_playlist != NULL)
    {
        total = s_render_playlist->file_count;
    }

    if (index >= total)
    {
        return;
    }

    selected_playback_index = index;
    gui_view_switch_direct(gui_view_get_current(), "app_recordingPlaybackView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    gui_fb_change();
}
