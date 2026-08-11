/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_photos UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.920Z
 */
#include "app_photos_ui.h"
#include "../callbacks/app_photos_callbacks.h"
#include "../user/app_photos_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *photo_main_list = NULL;
gui_img_t *hg_image_1770628433295_aqnj = NULL;
gui_text_t *app_photos_total_num_text = NULL;
gui_text_t *app_photos_feature_label = NULL;
gui_text_t *app_photos_total_u_text = NULL;
gui_win_t *photo_window = NULL;
gui_text_t *app_photos_tital_time_text = NULL;
gui_text_t *app_photos_tital = NULL;

// Time string global variables
char app_photos_tital_time_text_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void photo_main_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void photo_main_list_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create hg_image_1770628433295_aqnj (hg_image)
            hg_image_1770628433295_aqnj = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770628433295_aqnj", "/app_photo/photo_demo.bin", 0, 0, 352, 320);
            // Create app_photos_total_num_text (hg_label)
            app_photos_total_num_text = gui_text_create((gui_obj_t *)note, "app_photos_total_num_text", 19, 276,
                                                        20, 34);
            gui_text_set((gui_text_t *)app_photos_total_num_text, "1", GUI_FONT_SRC_BMP, gui_rgb(200, 200, 200),
                         1, 24);
            gui_text_type_set((gui_text_t *)app_photos_total_num_text,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_photos_total_num_text, LEFT);
            // Create app_photos_feature_label (hg_label)
            app_photos_feature_label = gui_text_create((gui_obj_t *)note, "app_photos_feature_label", 19, 241,
                                                       145, 34);
            gui_text_set((gui_text_t *)app_photos_feature_label, "Featured", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 8, 24);
            gui_text_type_set((gui_text_t *)app_photos_feature_label,
                              "/font/Inter24pt_SemiBold_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_photos_feature_label, LEFT);
            // Create app_photos_total_u_text (hg_label)
            app_photos_total_u_text = gui_text_create((gui_obj_t *)note, "app_photos_total_u_text", 43, 276, 80,
                                                      34);
            gui_text_set((gui_text_t *)app_photos_total_u_text, "Photos", GUI_FONT_SRC_BMP, gui_rgb(200, 200,
                         200), 6, 24);
            gui_text_type_set((gui_text_t *)app_photos_total_u_text,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_photos_total_u_text, LEFT);
            break;
        }
    default:
        break;
    }
}


// Create app_photo_view (hg_view)
static void app_photo_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_photo_view_switch_in(gui_view_t *view)
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



    // Create photo_main_list (hg_list)
    photo_main_list = gui_list_create((gui_obj_t *)view, "photo_main_list", 29, 115, 352, 387, 320, 5,
                                      VERTICAL, photo_main_list_note_design, NULL, false);
    gui_list_set_style(photo_main_list, LIST_CLASSIC);
    gui_list_set_note_num(photo_main_list, 1);
    gui_list_set_out_scope(photo_main_list, 80);

    // Create photo_window (hg_window)
    photo_window = gui_win_create((gui_obj_t *)view, "photo_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)photo_window, true);
    gui_win_set_blur_degree((gui_win_t *)photo_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(app_photos_tital_time_text_time_str, sizeof(app_photos_tital_time_text_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create app_photos_tital_time_text (hg_time_label)
    app_photos_tital_time_text = gui_text_create(photo_window, "app_photos_tital_time_text", 305, 20,
                                                 80, 32);
    gui_text_set((gui_text_t *)app_photos_tital_time_text, app_photos_tital_time_text_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(app_photos_tital_time_text_time_str), 28);
    gui_text_type_set((gui_text_t *)app_photos_tital_time_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_photos_tital_time_text, RIGHT);

    // Create app_photos_tital (hg_label)
    app_photos_tital = gui_text_create(photo_window, "app_photos_tital", 234, 60, 151, 34);
    gui_text_set((gui_text_t *)app_photos_tital, "My photos", GUI_FONT_SRC_BMP, gui_rgb(233, 143, 54),
                 9, 24);
    gui_text_type_set((gui_text_t *)app_photos_tital,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_photos_tital, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(photo_window), (gui_event_cb_t)photo_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)photo_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(app_photos_tital_time_text), 30000, true,
                         app_photos_tital_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_photo_view", false, app_photo_view_switch_in, app_photo_view_switch_out,
                  false);
