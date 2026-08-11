/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_compass UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.785Z
 */
#include "app_compass_ui.h"
#include "../callbacks/app_compass_callbacks.h"
#include "../user/app_compass_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_rounded_rect_t *compass_view_bg = NULL;
gui_img_t *compass_image = NULL;
gui_win_t *compass_window = NULL;
gui_img_t *compass_info_icon = NULL;
gui_text_t *compass_time_label = NULL;

// Time string global variables
char compass_time_label_time_str[10] = {0};


// Create app_compass_view (hg_view)
static void app_compass_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_compass_view_switch_in(gui_view_t *view)
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



    // Create compass_view_bg (hg_rect)
    compass_view_bg = gui_rect_create((gui_obj_t *)view, "compass_view_bg", 0, 0, 410, 502, 0,
                                      gui_rgb(222, 222, 222));

    // Create compass_window (hg_window)
    compass_window = gui_win_create((gui_obj_t *)view, "compass_window", 0, 0, 410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(compass_time_label_time_str, sizeof(compass_time_label_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create compass_time_label (hg_time_label)
    compass_time_label = gui_text_create(compass_window, "compass_time_label", 300, 20, 80, 40);
    gui_text_set((gui_text_t *)compass_time_label, compass_time_label_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(0, 0, 0), strlen(compass_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)compass_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)compass_time_label, RIGHT);

    // Create compass_info_icon (hg_image)
    compass_info_icon = gui_img_create_from_fs(compass_window, "compass_info_icon",
                                               "/app_compass/compass_infor_icon.bin", 20, 10, 72, 73);
    gui_img_scale((gui_img_t *)compass_info_icon, 1.000000f, 1.013889f);

    gui_obj_add_event_cb(GUI_BASE(compass_window), (gui_event_cb_t)compass_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)compass_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(compass_time_label), 30000, true, compass_time_label_time_update_cb);

    // Create compass_image (hg_image)
    compass_image = gui_img_create_from_fs((gui_obj_t *)view, "compass_image",
                                           "/app_compass/compass_icon.bin", 66, 92, 138, 183);
    gui_img_translate((gui_img_t *)compass_image, 138.0f, 184.0f);
    gui_img_set_focus((gui_img_t *)compass_image, 69.0f, 92.0f);
    // Bind timer: compass_sim
    gui_obj_create_timer((gui_obj_t *)compass_image, 1000, true, compass_image_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)compass_image);
}
GUI_VIEW_INSTANCE("app_compass_view", false, app_compass_view_switch_in,
                  app_compass_view_switch_out, false);
