/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_news UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.884Z
 */
#include "app_news_ui.h"
#include "../callbacks/app_news_callbacks.h"
#include "../user/app_news_user.h"
#include <stddef.h>

// Component handle definitions
gui_text_t *hg_label_1773478579329_kkq0 = NULL;


// Create app_news_view (hg_view)
static void app_news_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_news_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create hg_label_1773478579329_kkq0 (hg_label)
    hg_label_1773478579329_kkq0 = gui_text_create((gui_obj_t *)view, "hg_label_1773478579329_kkq0", 0,
                                                  220, 410, 100);
    gui_text_set((gui_text_t *)hg_label_1773478579329_kkq0, "Developing...", GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), 13, 40);
    gui_text_type_set((gui_text_t *)hg_label_1773478579329_kkq0,
                      "/font/Inter24pt_Medium_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1773478579329_kkq0, CENTER);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_news_view_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_news_view", false, app_news_view_switch_in, app_news_view_switch_out, false);
