/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_now_playing UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.894Z
 */
#include "app_now_playing_ui.h"
#include "../callbacks/app_now_playing_callbacks.h"
#include "../user/app_now_playing_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *now_play_list = NULL;
gui_rounded_rect_t *now_paly_list_bg1 = NULL;
gui_text_t *now_play_apple_watch_text = NULL;
gui_text_t *now_play_music_text = NULL;
gui_img_t *hg_image_1770801844379_si78 = NULL;
gui_rounded_rect_t *now_paly_now_paly_list_bg2 = NULL;
gui_img_t *hg_image_1770802605750_j01q = NULL;
gui_text_t *now_play_iphone_text = NULL;
gui_text_t *now_play_no_play_text = NULL;
gui_rounded_rect_t *now_play_list_bg3 = NULL;
gui_img_t *now_play_mode_icon3 = NULL;
gui_text_t *now_play_a2dp_source_text = NULL;
gui_text_t *now_play_source_mode_text = NULL;
gui_win_t *now_paly_window = NULL;
gui_win_t *now_play_back_window = NULL;
gui_img_t *now_play_back_btn = NULL;
gui_text_t *hg_time_label_1770799292471_6q7p = NULL;
gui_text_t *hg_label_1770799326271_mh23 = NULL;

// Time string global variables
char hg_time_label_1770799292471_6q7p_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void now_play_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void now_play_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    now_playing_view_sync_cb_impl(obj);
}


// Create app_now_play_view (hg_view)
static void app_now_play_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_now_play_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create now_play_list (hg_list)
    now_play_list = gui_list_create((gui_obj_t *)view, "now_play_list", 10, 110, 390, 392, 290, 5,
                                    VERTICAL, now_play_list_note_design, NULL, false);
    gui_list_set_style(now_play_list, LIST_ZOOM);
    gui_list_set_note_num(now_play_list, 3);
    gui_list_set_out_scope(now_play_list, 80);

    // Create now_paly_window (hg_window)
    now_paly_window = gui_win_create((gui_obj_t *)view, "now_paly_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)now_paly_window, true);
    gui_win_set_blur_degree((gui_win_t *)now_paly_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1770799292471_6q7p_time_str,
                 sizeof(hg_time_label_1770799292471_6q7p_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create now_play_back_window (hg_window)
    now_play_back_window = gui_win_create(now_paly_window, "now_play_back_window", 0, 0, 100, 100);


    // Create now_play_back_btn (hg_image)
    now_play_back_btn = gui_img_create_from_fs(now_play_back_window, "now_play_back_btn",
                                               "/app_phone/back_icon.bin", 32, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(now_play_back_window),
                         (gui_event_cb_t)now_play_back_window_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hg_time_label_1770799292471_6q7p (hg_time_label)
    hg_time_label_1770799292471_6q7p = gui_text_create(now_paly_window,
                                                       "hg_time_label_1770799292471_6q7p", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)hg_time_label_1770799292471_6q7p,
                 hg_time_label_1770799292471_6q7p_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1770799292471_6q7p_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1770799292471_6q7p,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1770799292471_6q7p, RIGHT);

    // Create hg_label_1770799326271_mh23 (hg_label)
    hg_label_1770799326271_mh23 = gui_text_create(now_paly_window, "hg_label_1770799326271_mh23", 208,
                                                  59, 172, 42);
    gui_text_set((gui_text_t *)hg_label_1770799326271_mh23, "Now Playing", GUI_FONT_SRC_BMP, gui_rgb(61,
                 147, 240), 11, 28);
    gui_text_type_set((gui_text_t *)hg_label_1770799326271_mh23,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1770799326271_mh23, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(now_paly_window), (gui_event_cb_t)now_paly_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)now_paly_window);

    now_playing_view_init_cb_impl();

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1770799292471_6q7p), 30000, true,
                         hg_time_label_1770799292471_6q7p_time_update_cb);
}
GUI_VIEW_INSTANCE("app_now_play_view", false, app_now_play_view_switch_in,
                  app_now_play_view_switch_out, false);
