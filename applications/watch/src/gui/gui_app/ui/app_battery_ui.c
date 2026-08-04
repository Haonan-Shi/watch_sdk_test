/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_battery UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.777Z
 */
#include "app_battery_ui.h"
#include "../callbacks/app_battery_callbacks.h"
#include "../user/app_battery_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_arc_t *hg_arc_1768201544822_6743 = NULL;
gui_arc_t *hg_arc_1768201544822_6743_copy_1768202312078 = NULL;
gui_text_t *hg_label_1768201705164_a9tv_copy_1768202379798 = NULL;
gui_rounded_rect_t *hg_rect_1768203813618_f089 = NULL;
gui_text_t *hg_label_1768201705164_a9tv_copy_1768202650656 = NULL;
gui_win_t *app_battery_window = NULL;
gui_text_t *hg_label_1768201705164_a9tv_copy_1768201975824 = NULL;
gui_img_t *hg_image_1768201691713_mmkg = NULL;
gui_text_t *hg_time_label_1768896548520_ahhc = NULL;

// Time string global variables
char hg_time_label_1768896548520_ahhc_time_str[10] = {0};


// Create app_battery_view (hg_view)
static void app_battery_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_battery_view_switch_in(gui_view_t *view)
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



    // Create hg_arc_1768201544822_6743 (hg_arc)
    hg_arc_1768201544822_6743 = gui_arc_create((gui_obj_t *)view, "hg_arc_1768201544822_6743", 206, 251,
                                               100, 0, 360, 20, gui_rgb(73, 85, 73));

    // Create hg_arc_1768201544822_6743_copy_1768202312078 (hg_arc)
    hg_arc_1768201544822_6743_copy_1768202312078 = gui_arc_create((gui_obj_t *)view,
                                                                  "hg_arc_1768201544822_6743_copy_1768202312078", 206, 251, 100, 270, 185, 18, gui_rgb(50, 215, 75));

    // Create app_battery_window (hg_window)
    app_battery_window = gui_win_create((gui_obj_t *)view, "app_battery_window", 0, 0, 410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1768896548520_ahhc_time_str,
                 sizeof(hg_time_label_1768896548520_ahhc_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_image_1768201691713_mmkg (hg_image)
    hg_image_1768201691713_mmkg = gui_img_create_from_fs(app_battery_window,
                                                         "hg_image_1768201691713_mmkg", "/app_music/delete_icon.bin", 20, 10, 72, 72);
    gui_obj_add_event_cb(hg_image_1768201691713_mmkg,
                         (gui_event_cb_t)hg_image_1768201691713_mmkg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hg_label_1768201705164_a9tv_copy_1768201975824 (hg_label)
    hg_label_1768201705164_a9tv_copy_1768201975824 = gui_text_create(app_battery_window,
                                                                     "hg_label_1768201705164_a9tv_copy_1768201975824", 279, 55, 100, 42);
    gui_text_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768201975824, "Battery",
                 GUI_FONT_SRC_BMP, gui_rgb(74, 194, 30), 7, 28);
    gui_text_type_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768201975824,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768201975824, RIGHT);

    // Create hg_time_label_1768896548520_ahhc (hg_time_label)
    hg_time_label_1768896548520_ahhc = gui_text_create(app_battery_window,
                                                       "hg_time_label_1768896548520_ahhc", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)hg_time_label_1768896548520_ahhc,
                 hg_time_label_1768896548520_ahhc_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1768896548520_ahhc_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1768896548520_ahhc,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1768896548520_ahhc, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(app_battery_window), (gui_event_cb_t)app_battery_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_battery_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1768896548520_ahhc), 30000, true,
                         hg_time_label_1768896548520_ahhc_time_update_cb);

    // Create hg_label_1768201705164_a9tv_copy_1768202379798 (hg_label)
    hg_label_1768201705164_a9tv_copy_1768202379798 = gui_text_create((gui_obj_t *)view,
                                                                     "hg_label_1768201705164_a9tv_copy_1768202379798", 138, 213, 140, 95);
    gui_text_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768202379798, "77%", GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), 3, 72);
    gui_text_type_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768202379798,
                      "/font/Inter_24pt_Regular_size72_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768202379798, CENTER);

    // Create hg_rect_1768203813618_f089 (hg_rect)
    hg_rect_1768203813618_f089 = gui_rect_create((gui_obj_t *)view, "hg_rect_1768203813618_f089", 38,
                                                 400, 340, 80, 40, gui_rgba(200, 200, 200, 200));

    // Create hg_label_1768201705164_a9tv_copy_1768202650656 (hg_label)
    hg_label_1768201705164_a9tv_copy_1768202650656 = gui_text_create((gui_obj_t *)view,
                                                                     "hg_label_1768201705164_a9tv_copy_1768202650656", 89, 418, 237, 44);
    gui_text_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768202650656, "Low Power Mode",
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 14, 30);
    gui_text_type_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768202650656,
                      "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1768201705164_a9tv_copy_1768202650656, MID_RIGHT);
}
GUI_VIEW_INSTANCE("app_battery_view", false, app_battery_view_switch_in,
                  app_battery_view_switch_out, false);
