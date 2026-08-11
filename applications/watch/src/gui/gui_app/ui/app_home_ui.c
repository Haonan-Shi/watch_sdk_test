/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_home UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.828Z
 */
#include "app_home_ui.h"
#include "../callbacks/app_home_callbacks.h"
#include "../user/app_home_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *home_main_list = NULL;
gui_rounded_rect_t *hg_rect_1770187079230_erq9 = NULL;
gui_text_t *hg_label_1770187314317_j193 = NULL;
gui_img_t *hg_image_1770187502585_1cbx = NULL;
gui_win_t *home_window = NULL;
gui_text_t *hg_time_label_1770174775962_9onz = NULL;
gui_text_t *hg_label_1770186977397_1rms = NULL;

// Time string global variables
char hg_time_label_1770174775962_9onz_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void home_main_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void home_main_list_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_rect_1770187079230_erq9 (hg_rect)
            hg_rect_1770187079230_erq9 = gui_rect_create((gui_obj_t *)note, "hg_rect_1770187079230_erq9", 0, 0,
                                                         352, 115, 20, gui_rgb(98, 101, 98));
            // Create hg_label_1770187314317_j193 (hg_label)
            hg_label_1770187314317_j193 = gui_text_create((gui_obj_t *)note, "hg_label_1770187314317_j193", 126,
                                                          38, 146, 50);
            gui_text_set((gui_text_t *)hg_label_1770187314317_j193, "Home", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 4, 36);
            gui_text_type_set((gui_text_t *)hg_label_1770187314317_j193,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1770187314317_j193, LEFT);
            // Create hg_image_1770187502585_1cbx (hg_image)
            hg_image_1770187502585_1cbx = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770187502585_1cbx", "/app_home/home_icon.bin", 15, 24, 80, 68);
            break;
        }
    default:
        break;
    }
}


// Create app_home_view (hg_view)
static void app_home_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_home_view_switch_in(gui_view_t *view)
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



    // Create home_main_list (hg_list)
    home_main_list = gui_list_create((gui_obj_t *)view, "home_main_list", 29, 115, 352, 387, 115, 5,
                                     VERTICAL, home_main_list_note_design, NULL, false);
    gui_list_set_style(home_main_list, LIST_CLASSIC);
    gui_list_set_note_num(home_main_list, 1);
    gui_list_set_out_scope(home_main_list, 80);

    // Create home_window (hg_window)
    home_window = gui_win_create((gui_obj_t *)view, "home_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)home_window, true);
    gui_win_set_blur_degree((gui_win_t *)home_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1770174775962_9onz_time_str,
                 sizeof(hg_time_label_1770174775962_9onz_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_time_label_1770174775962_9onz (hg_time_label)
    hg_time_label_1770174775962_9onz = gui_text_create(home_window, "hg_time_label_1770174775962_9onz",
                                                       305, 20, 80, 32);
    gui_text_set((gui_text_t *)hg_time_label_1770174775962_9onz,
                 hg_time_label_1770174775962_9onz_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1770174775962_9onz_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1770174775962_9onz,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1770174775962_9onz, RIGHT);

    // Create hg_label_1770186977397_1rms (hg_label)
    hg_label_1770186977397_1rms = gui_text_create(home_window, "hg_label_1770186977397_1rms", 244, 60,
                                                  141, 34);
    gui_text_set((gui_text_t *)hg_label_1770186977397_1rms, "My Home", GUI_FONT_SRC_BMP, gui_rgb(233,
                 143, 54), 7, 24);
    gui_text_type_set((gui_text_t *)hg_label_1770186977397_1rms,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1770186977397_1rms, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(home_window), (gui_event_cb_t)home_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)home_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1770174775962_9onz), 30000, true,
                         hg_time_label_1770174775962_9onz_time_update_cb);
}
GUI_VIEW_INSTANCE("app_home_view", false, app_home_view_switch_in, app_home_view_switch_out, false);
