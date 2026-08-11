/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_activity UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.751Z
 */
#include "app_activity_ui.h"
#include "../callbacks/app_activity_callbacks.h"
#include "../user/app_activity_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_img_t *hg_image_1768547548984_alny = NULL;
gui_img_t *hg_image_1768547556733_4p1o = NULL;
gui_img_t *hg_image_1768547565671_dhz0 = NULL;
gui_win_t *hg_window_arc = NULL;
gui_arc_t *hg_arc_1768547683171_qrb4 = NULL;
gui_arc_t *hg_arc_1768547680478_fdgw = NULL;
gui_arc_t *hg_arc_1768548096125_le9v = NULL;
gui_arc_t *hg_arc_1768548154198_qq35 = NULL;
gui_arc_t *hg_arc_1768548223121_16ut = NULL;
gui_arc_t *hg_arc_1768548288921_z6xj = NULL;
gui_text_t *hg_time_label_1768896458928_mps9 = NULL;

// Time string global variables
char hg_time_label_1768896458928_mps9_time_str[10] = {0};


// Create app_activity_view (hg_view)
static void app_activity_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_activity_view_switch_in(gui_view_t *view)
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
        snprintf(hg_time_label_1768896458928_mps9_time_str,
                 sizeof(hg_time_label_1768896458928_mps9_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }



    // Create hg_image_1768547548984_alny (hg_image)
    hg_image_1768547548984_alny = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1768547548984_alny", "/activity/activity_Left_Control0.bin", 17, 12, 72, 72);

    // Create hg_image_1768547556733_4p1o (hg_image)
    hg_image_1768547556733_4p1o = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1768547556733_4p1o", "/activity/activity_Left_Control.bin", 17, 414, 72, 72);

    // Create hg_image_1768547565671_dhz0 (hg_image)
    hg_image_1768547565671_dhz0 = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1768547565671_dhz0", "/activity/activity_Right_Control0.bin", 328, 414, 72, 72);

    // Create hg_window_arc (hg_window)
    hg_window_arc = gui_win_create((gui_obj_t *)view, "hg_window_arc", 0, 84, 410, 335);

    // Arc Group: bg_rings
    gui_arc_group_t *arc_group_bg_rings = gui_arc_group_create(hg_window_arc, "bg_rings", 50, 10, 329,
                                                               329);
    gui_arc_group_add_arc(arc_group_bg_rings, 164.5f, 164.5f, 150.0f, 0.0f, 360.0f, 25.0f, gui_rgba(44,
                          2, 13, 255));
    gui_arc_group_add_arc(arc_group_bg_rings, 164.5f, 164.5f, 120.0f, 0.0f, 360.0f, 25.0f, gui_rgba(26,
                          43, 0, 255));
    gui_arc_group_add_arc(arc_group_bg_rings, 164.5f, 164.5f, 90.0f, 0.0f, 360.0f, 25.0f, gui_rgba(0,
                          33, 38, 255));
    GUI_UNUSED(arc_group_bg_rings);

    // Create hg_arc_1768547680478_fdgw (hg_arc)
    hg_arc_1768547680478_fdgw = gui_arc_create(hg_window_arc, "hg_arc_1768547680478_fdgw", 215, 175,
                                               150, 270, 200, 25, gui_rgb(250, 17, 79));
    // Set angular gradient
    gui_arc_set_angular_gradient(hg_arc_1768547680478_fdgw, 270, 200);
    gui_arc_add_gradient_stop(hg_arc_1768547680478_fdgw, 0.0f, gui_rgba(250, 17, 79, 255));
    gui_arc_add_gradient_stop(hg_arc_1768547680478_fdgw, 1.0f, gui_rgba(255, 0, 0, 255));

    // Create hg_arc_1768548154198_qq35 (hg_arc)
    hg_arc_1768548154198_qq35 = gui_arc_create(hg_window_arc, "hg_arc_1768548154198_qq35", 215, 175,
                                               120, 270, 60, 25, gui_rgb(0, 122, 204));
    // Set angular gradient
    gui_arc_set_angular_gradient(hg_arc_1768548154198_qq35, 270, 60);
    gui_arc_add_gradient_stop(hg_arc_1768548154198_qq35, 0.0f, gui_rgba(164, 255, 0, 255));
    gui_arc_add_gradient_stop(hg_arc_1768548154198_qq35, 1.0f, gui_rgba(0, 255, 0, 255));

    // Create hg_arc_1768548288921_z6xj (hg_arc)
    hg_arc_1768548288921_z6xj = gui_arc_create(hg_window_arc, "hg_arc_1768548288921_z6xj", 215, 175, 90,
                                               270, 120, 25, gui_rgb(0, 122, 204));
    // Set angular gradient
    gui_arc_set_angular_gradient(hg_arc_1768548288921_z6xj, 270, 120);
    gui_arc_add_gradient_stop(hg_arc_1768548288921_z6xj, 0.0f, gui_rgba(0, 245, 234, 255));
    gui_arc_add_gradient_stop(hg_arc_1768548288921_z6xj, 1.0f, gui_rgba(0, 209, 255, 255));

    gui_obj_add_event_cb(GUI_BASE(hg_window_arc), (gui_event_cb_t)hg_window_arc_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)hg_window_arc);

    // Create hg_time_label_1768896458928_mps9 (hg_time_label)
    hg_time_label_1768896458928_mps9 = gui_text_create((gui_obj_t *)view,
                                                       "hg_time_label_1768896458928_mps9", 279, 20, 110, 40);
    gui_text_set((gui_text_t *)hg_time_label_1768896458928_mps9,
                 hg_time_label_1768896458928_mps9_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1768896458928_mps9_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1768896458928_mps9,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1768896458928_mps9, MID_RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1768896458928_mps9), 30000, true,
                         hg_time_label_1768896458928_mps9_time_update_cb);
}
GUI_VIEW_INSTANCE("app_activity_view", false, app_activity_view_switch_in,
                  app_activity_view_switch_out, false);
