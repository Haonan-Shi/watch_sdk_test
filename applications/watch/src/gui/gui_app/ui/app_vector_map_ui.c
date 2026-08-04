/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_vector_map UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.966Z
 */
#include "app_vector_map_ui.h"
#include "../callbacks/app_vector_map_callbacks.h"
#include "../user/app_vector_map_user.h"
#include <stddef.h>

// Component handle definitions


// Create app_vector_mapMainView (hg_view)
static void app_vector_mapMainView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_vector_mapMainView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_vector_mapMainView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_vector_mapMainView", false, app_vector_mapMainView_switch_in,
                  app_vector_mapMainView_switch_out, false);
