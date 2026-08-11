/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include "app_music_player_user.h"
#include "../ui/app_music_player_ui.h"
#include "gui_rect.h"
#include "gui_arc.h"
#include "gui_img.h"
#include "gui_view.h"
#include "gui_text.h"
#include "gui_list.h"
#include "gui_scroll_text.h"
#include "gui_listener.h"
#include <string.h>
#include <stdio.h>

#ifndef _HONEYGUI_SIMULATOR_
#include "bridge_music_player.h"
#endif

/*============================================================================*
 *                         Static State (render cache, not truth)
 *============================================================================*/

/*
 * gui_msg_publish() copies the payload into a heap buffer that gui_listener
 * frees immediately after the topic callback returns (see msg_task_exec()).
 * The list note_design callbacks run LATER (deferred, especially with
 * keep_note_alive=true), so we must copy the payload into persistent storage
 * here rather than caching the transient `data` pointer, otherwise later
 * dereferences read freed memory (garbled names/durations, wrong item count).
 */
static T_MUSIC_PLAYER_STATE s_state_storage;
static T_MUSIC_PLAYER_PLAYLIST s_playlist_storage;
static T_MUSIC_PLAYER_STATE *s_render_state;
static const T_MUSIC_PLAYER_PLAYLIST *s_render_playlist;

/* HoneyGUI stores text pointers, so visible note durations need stable backing storage. */
#define MUSIC_PLAYER_DURATION_BUF_COUNT  24
#define MUSIC_PLAYER_DURATION_TEXT_LEN   12
static char s_duration_bufs[MUSIC_PLAYER_DURATION_BUF_COUNT][MUSIC_PLAYER_DURATION_TEXT_LEN];
static char s_playlist_title_buf[24];

/*============================================================================*
 *                         Render Helpers
 *============================================================================*/

static void render_player_state(const T_MUSIC_PLAYER_STATE *state)
{
    /* Song title */
    if (state->song_name_len > 0)
    {
        gui_scroll_text_set((gui_scroll_text_t *)song_title_label, (char *)state->song_name,
                            GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242),
                            state->song_name_len, 36);
    }
    else
    {
        gui_scroll_text_set((gui_scroll_text_t *)song_title_label, "No Song", GUI_FONT_SRC_BMP,
                            gui_rgb(102, 102, 102), 7, 36);
    }
    gui_scroll_text_type_set((gui_scroll_text_t *)song_title_label,
                             "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);

    /* Artist label: show mode info */
    const char *mode_str = "";
    switch (state->mode)
    {
    case MUSIC_PLAYER_MODE_LOCAL_PLAYBACK:
        mode_str = "Local Playback";
        break;
    case MUSIC_PLAYER_MODE_A2DP_SRC:
        mode_str = "A2DP Source";
        break;
    case MUSIC_PLAYER_MODE_A2DP_SINK:
        mode_str = "A2DP Sink";
        break;
    default:
        mode_str = "No Mode";
        break;
    }
    gui_text_set(artist_label, (char *)mode_str, GUI_FONT_SRC_BMP,
                 gui_rgb(102, 102, 102), strlen(mode_str), 28);
    gui_text_type_set(artist_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(artist_label, MID_CENTER);

    /* Play/Pause button icon */
    if (state->play_status == MUSIC_PLAYER_STATUS_PLAYING)
    {
        gui_img_set_src((gui_img_t *)play_pause_btn,
                        (const uint8_t *)"/app_music_player/pause_btn.bin", IMG_SRC_FILESYS);
    }
    else
    {
        gui_img_set_src((gui_img_t *)play_pause_btn,
                        (const uint8_t *)"/app_music_player/play_btn.bin", IMG_SRC_FILESYS);
    }

    /* Cover: use a single mock cover */
    gui_img_set_src((gui_img_t *)album_cover,
                    (const uint8_t *)"/app_music_player/cover_1.bin", IMG_SRC_FILESYS);

    /* Progress ring */
    float pct = 0.0f;
    if (state->total_time_ms > 0)
    {
        pct = (float)state->pos_time_ms / (float)state->total_time_ms;
    }
    float angle = -90.0f + pct * 360.0f;
    gui_arc_set_end_angle(progress_ring, angle);

    gui_fb_change();
}

static void render_volume(const T_MUSIC_PLAYER_STATE *state)
{
    int vol_pct = state->volume;
    if (vol_pct > state->max_volume)
    {
        vol_pct = state->max_volume;
    }
    int bar_width = 240 * vol_pct / state->max_volume;
    gui_rect_set_size(volume_bar_fill, bar_width, 10);

    static char buf[8];
    snprintf(buf, sizeof(buf), "%d", vol_pct);
    gui_text_set(volume_percent_label, buf, GUI_FONT_SRC_BMP, gui_rgb(249, 211, 66),
                 strlen(buf), 36);
    gui_text_type_set(volume_percent_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(volume_percent_label, MID_CENTER);
    gui_fb_change();
}

static void render_play_progress(const T_MUSIC_PLAYER_STATE *state)
{
    /* Progress ring */
    float pct = 0.0f;
    if (state->total_time_ms > 0)
    {
        pct = (float)state->pos_time_ms / (float)state->total_time_ms;
    }
    float angle = -90.0f + pct * 360.0f;
    gui_log("render_play_progress: pos_time_ms=%u, total_time_ms=%u, pct=%.2f",
            (unsigned)state->pos_time_ms,
            (unsigned)state->total_time_ms, pct);
    gui_arc_set_end_angle(progress_ring, angle);
}

static void format_duration(uint32_t ms, char *buf, size_t buf_size)
{
    uint32_t total_sec = ms / 1000;
    uint32_t min = total_sec / 60;
    uint32_t sec = total_sec % 60;
    gui_log("format_duration" " ms: %u, min: %u, sec: %u", (unsigned)ms, (unsigned)min, (unsigned)sec);
    snprintf(buf, buf_size, "%u:%02u", (unsigned)min, (unsigned)sec);
}

static bool get_playlist_item_data(uint16_t actual_index,
                                   const T_MUSIC_PLAYER_PLAYLIST_ITEM **item_out,
                                   const char **title_out,
                                   uint16_t *title_len_out,
                                   uint32_t *duration_ms_out)
{
    if (item_out != NULL)
    {
        *item_out = NULL;
    }
    *title_out = "Unknown";
    *title_len_out = (uint16_t)strlen("Unknown");
    *duration_ms_out = 0;

    if (s_render_playlist != NULL
        && s_render_playlist->items != NULL
        && actual_index < s_render_playlist->item_capacity)
    {
        const T_MUSIC_PLAYER_PLAYLIST_ITEM *item =
            &s_render_playlist->items[actual_index];

        if (item_out != NULL)
        {
            *item_out = item;
        }

        if (item->name_len > 0)
        {
            *title_out = item->name;
            *title_len_out = item->name_len;
        }
        *duration_ms_out = item->duration_ms;
        return true;
    }

    return false;
}

static void render_playlist(const T_MUSIC_PLAYER_PLAYLIST *pl)
{
    uint16_t total = s_render_playlist->item_capacity;

    if (playlist_title_label != NULL)
    {
        snprintf(s_playlist_title_buf, sizeof(s_playlist_title_buf), "Playlist (%u) Songs",
                 (unsigned)total);
        gui_text_set(playlist_title_label, s_playlist_title_buf, GUI_FONT_SRC_BMP,
                     gui_rgb(242, 242, 242), strlen(s_playlist_title_buf), 36);
        gui_text_type_set(playlist_title_label,
                          "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
        gui_text_mode_set(playlist_title_label, MID_LEFT);
    }

    if (playlist_list != NULL)
    {
        gui_list_set_note_num(playlist_list, 0);
        gui_list_set_note_num(playlist_list, total);
        gui_list_set_offset(playlist_list, 0);
    }

    gui_fb_change();
}

void playlist_note_design_impl(gui_obj_t *obj)
{
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t actual_index = (uint16_t)note->index;
    uint16_t total = s_render_playlist->item_capacity;
    uint16_t buf_index = actual_index % MUSIC_PLAYER_DURATION_BUF_COUNT;
    const T_MUSIC_PLAYER_PLAYLIST_ITEM *item = NULL;
    const char *title_text = NULL;
    uint16_t title_len = 0;
    uint32_t duration_ms = 0;

    if (actual_index >= total)
    {
        return;
    }

    gui_obj_child_free((gui_obj_t *)note);

    if (!get_playlist_item_data(actual_index, &item, &title_text, &title_len, &duration_ms))
    {
        return;
    }

    bool is_current = (s_render_playlist != NULL &&
                       actual_index == s_render_playlist->current_index);
    gui_color_t title_color = is_current ? gui_rgb(249, 211, 66) : gui_rgb(242, 242, 242);
    gui_color_t duration_color = is_current ? gui_rgb(200, 170, 50) : gui_rgb(153, 153, 153);
    gui_color_t bg_color = is_current ? gui_rgb(61, 53, 32) : gui_rgba(0, 0, 0, 0);

    gui_rect_create((gui_obj_t *)note, "playlist_item_bg", 15, 0, 380, 84, 16, bg_color);

    gui_scroll_text_t *title = gui_scroll_text_create((gui_obj_t *)note, "playlist_item_title", 40, 19,
                                                      260, 40);
    gui_scroll_text_set((gui_scroll_text_t *)title, (char *)title_text, GUI_FONT_SRC_BMP, title_color,
                        title_len, 36);
    gui_scroll_text_type_set((gui_scroll_text_t *)title,
                             "/font/NotoSansSC_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_scroll_text_scroll_set((gui_scroll_text_t *)title, SCROLL_X, 0, 0, 5000, 0);
    if (duration_ms > 0)
    {
        format_duration(duration_ms, s_duration_bufs[buf_index], sizeof(s_duration_bufs[buf_index]));
    }
    else
    {
        snprintf(s_duration_bufs[buf_index], sizeof(s_duration_bufs[buf_index]), "--:--");
    }

    gui_text_t *duration = gui_text_create((gui_obj_t *)note, "playlist_item_duration", 40, 52, 200,
                                           38);
    gui_text_set(duration, s_duration_bufs[buf_index], GUI_FONT_SRC_BMP, duration_color,
                 strlen(s_duration_bufs[buf_index]), 28);
    gui_text_type_set(duration, "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set(duration, MID_LEFT);

    gui_obj_add_event_cb((gui_obj_t *)note, playlist_note_clicked, GUI_EVENT_TOUCH_CLICKED, NULL);
}

/*============================================================================*
 *                         Topic Callbacks (GUI thread)
 *============================================================================*/
void music_player_topic_state_cb(gui_obj_t *obj, const char *topic, void *data,
                                 uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    if (data != NULL && len >= sizeof(T_MUSIC_PLAYER_STATE))
    {
        memcpy(&s_state_storage, data, sizeof(s_state_storage));
        s_render_state = &s_state_storage;

        gui_log("[music_player] topic state update: mode %d, play_status %d, volume %d, total_songs %d, current_index %d",
                s_render_state->mode, s_render_state->play_status, s_render_state->volume,
                s_render_state->total_songs, s_render_state->current_index);

        render_player_state(s_render_state);
        render_volume(s_render_state);
    }
}

static void music_player_topic_playlist_cb(gui_obj_t *obj, const char *topic, void *data,
                                           uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    if (data != NULL && len >= sizeof(T_MUSIC_PLAYER_PLAYLIST))
    {
        memcpy(&s_playlist_storage, data, sizeof(s_playlist_storage));
        s_render_playlist = &s_playlist_storage;
        render_playlist(s_render_playlist);
    }
}

static void music_player_topic_progress_cb(gui_obj_t *obj, const char *topic, void *data,
                                           uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    if (data != NULL && len >= sizeof(T_MUSIC_PLAYER_STATE))
    {
        memcpy(&s_state_storage, data, sizeof(s_state_storage));
        s_render_state = &s_state_storage;

        /* Update progress ring */
        render_play_progress(s_render_state);

        /* Update play/pause button */
        if (s_render_state->play_status == MUSIC_PLAYER_STATUS_PLAYING)
        {
            gui_img_set_src((gui_img_t *)play_pause_btn,
                            (const uint8_t *)"/app_music_player/pause_btn.bin", IMG_SRC_FILESYS);
        }
        else
        {
            gui_img_set_src((gui_img_t *)play_pause_btn,
                            (const uint8_t *)"/app_music_player/play_btn.bin", IMG_SRC_FILESYS);
        }
        gui_fb_change();
    }
}

/*============================================================================*
 *                         VolumeOverlay visibility
 *============================================================================*/
static void set_volume_overlay_visible(bool visible)
{
    gui_obj_hidden((gui_obj_t *)volume_overlay_window, !visible);
    gui_fb_change();
}

/*============================================================================*
 *                         View Init Callbacks
 *============================================================================*/

void player_view_init_cb_impl(void)
{
    /* Subscribe to topics */
    gui_msg_subscribe((gui_obj_t *)song_title_label, GUI_TOPIC_MUSIC_PLAYER_STATE,
                      music_player_topic_state_cb);
    gui_msg_subscribe((gui_obj_t *)progress_ring, GUI_TOPIC_MUSIC_PLAYER_PROGRESS,
                      music_player_topic_progress_cb);

    /* Request before the first frame is rendered */
    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PLAYER_STATE, NULL, 0);

}

void playlist_view_init_cb_impl(void)
{
    /* Subscribe to playlist topic */
    gui_msg_subscribe((gui_obj_t *)playlist_title_label, GUI_TOPIC_MUSIC_PLAYER_PLAYLIST,
                      music_player_topic_playlist_cb);

    /* Request fresh playlist snapshot */
    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PLAYLIST_STATE, NULL, 0);
}

/*============================================================================*
 *                         Button Callback Implementations
 *============================================================================*/

void music_toggle_play(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PLAY_PAUSE, NULL, 0);
}

void music_next(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_NEXT, NULL, 0);
}

void music_prev(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PREV, NULL, 0);
}

void music_show_volume(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    set_volume_overlay_visible(true);
}

void music_close_volume(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    set_volume_overlay_visible(false);
}

void music_volume_down(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_VOL_DOWN, NULL, 0);
}

void music_volume_up(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_VOL_UP, NULL, 0);
}

void playlist_note_clicked(void *obj, gui_event_t *e)
{
    GUI_UNUSED(e);

    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = (uint16_t)note->index;
    uint16_t total = s_render_playlist->item_capacity;

    if (index >= total)
    {
        return;
    }

    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_CMD_PLAY_BY_INDEX, &index, sizeof(index));
    gui_view_switch_direct(gui_view_get_current(), "app_music_playerPlayerView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

/* Progress timer callback: request progress from backend instead of self-incrementing */
void music_progress_timer_cb_impl(void)
{
    music_player_gui_to_app(EVENT_BUS_TOPIC_MUSIC_PLAYER_REQ_PROGRESS_STATE, NULL, 0);
}
