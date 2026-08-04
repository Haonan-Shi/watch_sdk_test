/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * watchface_select UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:41.029Z
 */
#include "watchface_select_ui.h"
#include "../callbacks/watchface_select_callbacks.h"
#include "../user/watchface_select_user.h"
#include <stddef.h>

// Component handle definitions
gui_list_t *watchface_select_list = NULL;
gui_text_t *hg_label_1770016204700_sw1h = NULL;
gui_img_t *hg_image_1770016357532_9lx9 = NULL;
gui_text_t *hg_label_1770016204700_sw1h_copy_1770016369560 = NULL;
gui_img_t *hg_image_1770016385884_x9w0 = NULL;
gui_img_t *hg_image_1770016418312_kzc4 = NULL;
gui_text_t *hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992 = NULL;
gui_img_t *hg_image_1770016451702_4n00 = NULL;
gui_text_t *hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992_copy_1770016460768 =
    NULL;
gui_win_t *watchface_select_window = NULL;
gui_rounded_rect_t *hg_rect_1770016833554_0smr = NULL;
gui_text_t *hg_label_1770016960792_xl9q = NULL;

// List component note_design callback functions
// note_design callback function declaration
static void watchface_select_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void watchface_select_list_note_design(gui_obj_t *obj, void *param)
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
            // Create hg_label_1770016204700_sw1h (hg_label)
            hg_label_1770016204700_sw1h = gui_text_create((gui_obj_t *)note, "hg_label_1770016204700_sw1h", 105,
                                                          50, 200, 50);
            gui_text_set((gui_text_t *)hg_label_1770016204700_sw1h, "Big Number", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 10, 40);
            gui_text_type_set((gui_text_t *)hg_label_1770016204700_sw1h,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1770016204700_sw1h, CENTER);
            // Create hg_image_1770016357532_9lx9 (hg_image)
            hg_image_1770016357532_9lx9 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770016357532_9lx9", "/app_home_list/watchface_big_number.bin", 105, 129, 200, 245);
            gui_obj_add_event_cb(hg_image_1770016357532_9lx9,
                                 (gui_event_cb_t)hg_image_1770016357532_9lx9_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            gui_obj_add_event_cb(obj, watchface_select_list_item_1_switch_view_cb, GUI_EVENT_KB_SHORT_PRESSED,
                                 NULL);
            break;
        }
    case 1:
        {
            // Create hg_label_1770016204700_sw1h_copy_1770016369560 (hg_label)
            hg_label_1770016204700_sw1h_copy_1770016369560 = gui_text_create((gui_obj_t *)note,
                                                                             "hg_label_1770016204700_sw1h_copy_1770016369560", 105, 50, 200, 50);
            gui_text_set((gui_text_t *)hg_label_1770016204700_sw1h_copy_1770016369560, "Sport",
                         GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 40);
            gui_text_type_set((gui_text_t *)hg_label_1770016204700_sw1h_copy_1770016369560,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1770016204700_sw1h_copy_1770016369560, CENTER);
            // Create hg_image_1770016385884_x9w0 (hg_image)
            hg_image_1770016385884_x9w0 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770016385884_x9w0", "/app_home_list/watchface_sport.bin", 105, 129, 200, 245);
            gui_obj_add_event_cb(hg_image_1770016385884_x9w0,
                                 (gui_event_cb_t)hg_image_1770016385884_x9w0_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 2:
        {
            // Create hg_image_1770016418312_kzc4 (hg_image)
            hg_image_1770016418312_kzc4 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770016418312_kzc4", "/app_home_list/watchface_video.bin", 105, 129, 200, 245);
            // Create hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992 (hg_label)
            hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992 = gui_text_create((
                                                                                    gui_obj_t *)note, "hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992", 105, 50, 200,
                                                                                50);
            gui_text_set((gui_text_t *)hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992,
                         "Video", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 40);
            gui_text_type_set((gui_text_t *)hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992,
                              CENTER);
            break;
        }
    case 3:
        {
            // Create hg_image_1770016451702_4n00 (hg_image)
            hg_image_1770016451702_4n00 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1770016451702_4n00", "/app_home_list/add_watchface.bin", 105, 129, 200, 245);
            // Create hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992_copy_1770016460768 (hg_label)
            hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992_copy_1770016460768 =
                gui_text_create((gui_obj_t *)note,
                                "hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992_copy_1770016460768", 80, 50, 251,
                                50);
            gui_text_set((gui_text_t *)
                         hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992_copy_1770016460768,
                         "Add Watchface", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 13, 40);
            gui_text_type_set((gui_text_t *)
                              hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992_copy_1770016460768,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)
                              hg_label_1770016204700_sw1h_copy_1770016369560_copy_1770016431992_copy_1770016460768, CENTER);
            break;
        }
    default:
        break;
    }
}


// Create watchface_select_view (hg_view)
static void watchface_select_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void watchface_select_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create watchface_select_list (hg_list)
    watchface_select_list = gui_list_create((gui_obj_t *)view, "watchface_select_list", 0, 0, 410, 502,
                                            410, 5, HORIZONTAL, watchface_select_list_note_design, NULL, false);
    gui_list_set_style(watchface_select_list, LIST_CLASSIC);
    gui_list_set_note_num(watchface_select_list, 4);
    gui_list_set_auto_align(watchface_select_list, true);

    // Create watchface_select_window (hg_window)
    watchface_select_window = gui_win_create((gui_obj_t *)view, "watchface_select_window", 0, 390, 410,
                                             100);


    // Create hg_rect_1770016833554_0smr (hg_rect)
    hg_rect_1770016833554_0smr = gui_rect_create(watchface_select_window, "hg_rect_1770016833554_0smr",
                                                 131, 15, 150, 70, 35, gui_rgb(98, 101, 98));
    gui_obj_hidden((gui_obj_t *)hg_rect_1770016833554_0smr, true);

    // Create hg_label_1770016960792_xl9q (hg_label)
    hg_label_1770016960792_xl9q = gui_text_create(watchface_select_window,
                                                  "hg_label_1770016960792_xl9q", 174, 34, 65, 42);
    gui_text_set((gui_text_t *)hg_label_1770016960792_xl9q, "Edit", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                 255), 4, 32);
    gui_text_type_set((gui_text_t *)hg_label_1770016960792_xl9q,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1770016960792_xl9q, MID_CENTER);
    gui_obj_hidden((gui_obj_t *)hg_label_1770016960792_xl9q, true);

    gui_obj_add_event_cb(GUI_BASE(watchface_select_window),
                         (gui_event_cb_t)watchface_select_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)watchface_select_window);
}
GUI_VIEW_INSTANCE("watchface_select_view", false, watchface_select_view_switch_in,
                  watchface_select_view_switch_out, false);
