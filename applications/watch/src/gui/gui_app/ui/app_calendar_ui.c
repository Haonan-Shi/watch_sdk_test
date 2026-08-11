/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_calendar UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.780Z
 */
#include "app_calendar_ui.h"
#include "../callbacks/app_calendar_callbacks.h"
#include "../user/app_calendar_user.h"
#include <stddef.h>

// Component handle definitions
gui_text_t *text_calendar = NULL;


// Create app_calendar_view (hg_view)
static void app_calendar_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_calendar_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create text_calendar (hg_label)
    text_calendar = gui_text_create((gui_obj_t *)view, "text_calendar", 0, 220, 410, 100);
    gui_text_set((gui_text_t *)text_calendar, "Developing...", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 13, 40);
    gui_text_type_set((gui_text_t *)text_calendar, "/font/Inter24pt_Medium_size40_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)text_calendar, CENTER);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_calendar_view_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_calendar_view", false, app_calendar_view_switch_in,
                  app_calendar_view_switch_out, false);
