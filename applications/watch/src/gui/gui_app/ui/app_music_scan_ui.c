/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_music_scan UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.880Z
 */
#include "app_music_scan_ui.h"
#include "../callbacks/app_music_scan_callbacks.h"
#include "../user/app_music_scan_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_text_t *music_scan_time_text = NULL;
gui_img_t *music_scan_icon_bg = NULL;
gui_img_t *music_scan_icon = NULL;
gui_text_t *hg_label_1770793098712_0862 = NULL;

// Time string global variables
char music_scan_time_text_time_str[10] = {0};


// Create app_music_scan_view (hg_view)
static void app_music_scan_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_music_scan_view_switch_in(gui_view_t *view)
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
    if (t != NULL)
    {
        snprintf(music_scan_time_text_time_str, sizeof(music_scan_time_text_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }



    // Create music_scan_time_text (hg_time_label)
    music_scan_time_text = gui_text_create((gui_obj_t *)view, "music_scan_time_text", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)music_scan_time_text, music_scan_time_text_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(music_scan_time_text_time_str), 28);
    gui_text_type_set((gui_text_t *)music_scan_time_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)music_scan_time_text, RIGHT);

    // Create music_scan_icon_bg (hg_image)
    music_scan_icon_bg = gui_img_create_from_fs((gui_obj_t *)view, "music_scan_icon_bg",
                                                "/app_music/music_scan_icon_bg.bin", 106, 101, 200, 200);
    gui_img_translate((gui_img_t *)music_scan_icon_bg, 100.0f, 100.0f);
    gui_img_set_focus((gui_img_t *)music_scan_icon_bg, 100.0f, 100.0f);

    // Create hg_label_1770793098712_0862 (hg_label)
    hg_label_1770793098712_0862 = gui_text_create((gui_obj_t *)view, "hg_label_1770793098712_0862", 94,
                                                  375, 225, 42);
    gui_text_set((gui_text_t *)hg_label_1770793098712_0862, "Tap to Scan", GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), 11, 32);
    gui_text_type_set((gui_text_t *)hg_label_1770793098712_0862,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1770793098712_0862, MID_CENTER);

    // Create music_scan_icon (hg_image)
    music_scan_icon = gui_img_create_from_fs((gui_obj_t *)view, "music_scan_icon",
                                             "/app_music/music_scan_icon.bin", 106, 101, 200, 200);
    gui_obj_add_event_cb(music_scan_icon, (gui_event_cb_t)music_scan_icon_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);
    gui_obj_add_event_cb(music_scan_icon, (gui_event_cb_t)music_scan_icon_key_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)music_scan_icon);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_music_scan_view_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(music_scan_time_text), 30000, true,
                         music_scan_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_music_scan_view", false, app_music_scan_view_switch_in,
                  app_music_scan_view_switch_out, false);
