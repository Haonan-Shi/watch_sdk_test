/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_now_playing_user.h"

#include <stdbool.h>
#include <string.h>
#include "gui_listener.h"

#ifndef _HONEYGUI_SIMULATOR_
#include "bridge_now_playing.h"
#endif

/**
 * User custom implementation
 * This file is generated only once and can be freely modified
 */

// Add custom implementation here

static T_NOW_PLAYING_STATE s_render_state;

static gui_color_t mode_status_enabled_color(void)
{
    return gui_rgb(90, 200, 250);
}

static gui_color_t mode_status_disabled_color(void)
{
    return gui_rgb(200, 200, 200);
}

static gui_color_t mode_card_default_color(void)
{
    return gui_rgb(44, 44, 46);
}

static gui_color_t mode_card_active_color(void)
{
    return gui_rgb(61, 53, 32);
}

static void render_now_playing_view(void)
{
    if (now_play_list == NULL)
    {
        return;
    }

    gui_list_set_note_num(now_play_list, 0);
    gui_list_set_note_num(now_play_list, 3);
    gui_fb_change();
}

static void now_playing_topic_state_cb(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);

    if (data == NULL || len < sizeof(T_NOW_PLAYING_STATE))
    {
        return;
    }

    gui_log("now_playing_topic_state_cb called with mode %d", ((T_NOW_PLAYING_STATE *)data)->mode);

    memcpy(&s_render_state, data, sizeof(s_render_state));
    render_now_playing_view();
}

void now_playing_view_reset_cb_impl(void)
{
    /* Keep this callback as a harmless hook because ui.c calls it. */
}

void now_playing_view_sync_cb_impl(void *obj)
{
    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create now_paly_list_bg1 (hg_rect)
            now_paly_list_bg1 = gui_rect_create((gui_obj_t *)note, "now_paly_list_bg1", 0, 0, 390, 290, 40,
                                                gui_rgb(44, 44, 46));
            gui_rect_set_color(now_paly_list_bg1,
                               s_render_state.playback_enabled ? mode_card_active_color() : mode_card_default_color());
            // Create now_play_apple_watch_text (hg_label)
            now_play_apple_watch_text = gui_text_create((gui_obj_t *)note, "now_play_apple_watch_text", 32, 153,
                                                        220, 42);
            gui_text_set((gui_text_t *)now_play_apple_watch_text, "Playback", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 8, 32);
            gui_text_type_set((gui_text_t *)now_play_apple_watch_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)now_play_apple_watch_text, LEFT);
            // Create now_play_music_text (hg_label)
            now_play_music_text = gui_text_create((gui_obj_t *)note, "now_play_music_text", 32, 212, 120, 42);
            gui_text_set((gui_text_t *)now_play_music_text, "Disable", GUI_FONT_SRC_BMP, gui_rgb(200, 200, 200),
                         7, 32);
            gui_text_type_set((gui_text_t *)now_play_music_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)now_play_music_text, LEFT);
            gui_text_content_set(now_play_music_text, s_render_state.playback_enabled ? "Enable" : "Disable",
                                 s_render_state.playback_enabled ? (sizeof("Enable") - 1) : (sizeof("Disable") - 1));
            gui_text_color_set(now_play_music_text,
                               s_render_state.playback_enabled ? mode_status_enabled_color() : mode_status_disabled_color());
            // Create hg_image_1770801844379_si78 (hg_image)
            hg_image_1770801844379_si78 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770801844379_si78", "/app_now_playing/watch_icon.bin", 298, 15, 80, 80);
            gui_img_scale((gui_img_t *)hg_image_1770801844379_si78, 1.111111f, 1.111111f);
            gui_obj_hidden((gui_obj_t *)hg_image_1770801844379_si78, false);
            gui_obj_add_event_cb(obj, toggle_playback_mode, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 1:
        {
            // Create now_paly_now_paly_list_bg2 (hg_rect)
            now_paly_now_paly_list_bg2 = gui_rect_create((gui_obj_t *)note, "now_paly_now_paly_list_bg2", 0, 0,
                                                         390, 290, 40, gui_rgb(44, 44, 46));
            gui_rect_set_color(now_paly_now_paly_list_bg2,
                               s_render_state.a2dp_sink_enabled ? mode_card_active_color() : mode_card_default_color());
            // Create hg_image_1770802605750_j01q (hg_image)
            hg_image_1770802605750_j01q = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770802605750_j01q", "/app_now_playing/phone_icon.bin", 298, 15, 49, 72);
            gui_obj_hidden((gui_obj_t *)hg_image_1770802605750_j01q, false);
            // Create now_play_iphone_text (hg_label)
            now_play_iphone_text = gui_text_create((gui_obj_t *)note, "now_play_iphone_text", 32, 153, 220, 42);
            gui_text_set((gui_text_t *)now_play_iphone_text, "A2DP Sink", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 9, 32);
            gui_text_type_set((gui_text_t *)now_play_iphone_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)now_play_iphone_text, LEFT);
            // Create now_play_no_play_text (hg_label)
            now_play_no_play_text = gui_text_create((gui_obj_t *)note, "now_play_no_play_text", 32, 212, 120,
                                                    42);
            gui_text_set((gui_text_t *)now_play_no_play_text, "Disable", GUI_FONT_SRC_BMP, gui_rgb(200, 200,
                         200), 7, 32);
            gui_text_type_set((gui_text_t *)now_play_no_play_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)now_play_no_play_text, LEFT);
            gui_text_content_set(now_play_no_play_text, s_render_state.a2dp_sink_enabled ? "Enable" : "Disable",
                                 s_render_state.a2dp_sink_enabled ? (sizeof("Enable") - 1) : (sizeof("Disable") - 1));
            gui_text_color_set(now_play_no_play_text,
                               s_render_state.a2dp_sink_enabled ? mode_status_enabled_color() : mode_status_disabled_color());
            gui_obj_add_event_cb(obj, toggle_a2dp_sink_mode, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 2:
        {
            // Create now_play_list_bg3 (hg_rect)
            now_play_list_bg3 = gui_rect_create((gui_obj_t *)note, "now_play_list_bg3", 0, 0, 390, 290, 40,
                                                gui_rgb(44, 44, 46));
            gui_rect_set_color(now_play_list_bg3,
                               s_render_state.a2dp_source_enabled ? mode_card_active_color() : mode_card_default_color());
            // Create now_play_mode_icon3 (hg_image)
            now_play_mode_icon3 = gui_img_create_from_fs((gui_obj_t *)note, "now_play_mode_icon3",
                                                         "/app_now_playing/watch_icon.bin", 298, 15, 80, 80);
            gui_img_scale((gui_img_t *)now_play_mode_icon3, 1.111111f, 1.111111f);
            gui_obj_hidden((gui_obj_t *)now_play_mode_icon3, false);
            // Create now_play_a2dp_source_text (hg_label)
            now_play_a2dp_source_text = gui_text_create((gui_obj_t *)note, "now_play_a2dp_source_text", 32, 153,
                                                        250, 42);
            gui_text_set((gui_text_t *)now_play_a2dp_source_text, "A2DP Source", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 11, 32);
            gui_text_type_set((gui_text_t *)now_play_a2dp_source_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)now_play_a2dp_source_text, LEFT);
            // Create now_play_source_mode_text (hg_label)
            now_play_source_mode_text = gui_text_create((gui_obj_t *)note, "now_play_source_mode_text", 32, 212,
                                                        120, 42);
            gui_text_set((gui_text_t *)now_play_source_mode_text, "Disable", GUI_FONT_SRC_BMP, gui_rgb(200, 200,
                         200), 7, 32);
            gui_text_type_set((gui_text_t *)now_play_source_mode_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)now_play_source_mode_text, LEFT);
            gui_text_content_set(now_play_source_mode_text,
                                 s_render_state.a2dp_source_enabled ? "Enable" : "Disable",
                                 s_render_state.a2dp_source_enabled ? (sizeof("Enable") - 1) : (sizeof("Disable") - 1));
            gui_text_color_set(now_play_source_mode_text,
                               s_render_state.a2dp_source_enabled ? mode_status_enabled_color() : mode_status_disabled_color());
            gui_obj_add_event_cb(obj, toggle_a2dp_source_mode, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    default:
        break;
    }
}

void now_playing_view_init_cb_impl(void)
{
    gui_msg_subscribe((gui_obj_t *)now_play_list, GUI_TOPIC_NOW_PLAYING_STATE,
                      now_playing_topic_state_cb);

    now_playing_gui_to_app(EVENT_BUS_TOPIC_NOW_PLAYING_REQ_AUDIO_MODE, NULL, 0);
}

void toggle_playback_mode(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    now_playing_gui_to_app(EVENT_BUS_TOPIC_NOW_PLAYING_SET_PLAYBACK_MODE, NULL, 0);
#endif
}

void toggle_a2dp_sink_mode(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    now_playing_gui_to_app(EVENT_BUS_TOPIC_NOW_PLAYING_SET_A2DP_SINK_MODE, NULL, 0);
#endif
}

void toggle_a2dp_source_mode(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

#ifndef _HONEYGUI_SIMULATOR_
    now_playing_gui_to_app(EVENT_BUS_TOPIC_NOW_PLAYING_SET_A2DP_SOURCE_MODE, NULL, 0);
#endif
}
