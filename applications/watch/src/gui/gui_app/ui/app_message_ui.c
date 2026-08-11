/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_message UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.860Z
 */
#include "app_message_ui.h"
#include "../callbacks/app_message_callbacks.h"
#include "../user/app_message_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *hg_list_1768285628248_uckn = NULL;
gui_rounded_rect_t *app_message_list_bg1 = NULL;
gui_img_t *hg_image_1768356352082_xvk6 = NULL;
gui_text_t *app_message_list_telephoneNum1 = NULL;
gui_text_t *app_message_list_message1 = NULL;
gui_text_t *app_message_list_date1 = NULL;
gui_rounded_rect_t *app_message_list_bg2 = NULL;
gui_img_t *hg_image_1768356356559_kcug = NULL;
gui_text_t *app_message_list_message2 = NULL;
gui_text_t *app_message_list_telephoneNum2 = NULL;
gui_text_t *app_message_list_date2 = NULL;
gui_rounded_rect_t *app_message_list_bg3 = NULL;
gui_img_t *hg_image_1768356382959_8b09 = NULL;
gui_text_t *app_message_list_message3 = NULL;
gui_text_t *app_message_list_telephoneNum3 = NULL;
gui_text_t *app_message_list_date3 = NULL;
gui_win_t *hg_window_1768354016343_kf7g = NULL;
gui_img_t *hg_image_1768355370798_508r = NULL;
gui_text_t *app_message_title = NULL;
gui_text_t *app_message_time_text = NULL;

// Time string global variables
char app_message_time_text_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void hg_list_1768285628248_uckn_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void hg_list_1768285628248_uckn_note_design(gui_obj_t *obj, void *param)
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
            // Create app_message_list_bg1 (hg_rect)
            app_message_list_bg1 = gui_rect_create((gui_obj_t *)note, "app_message_list_bg1", 0, 0, 390, 218,
                                                   20, gui_rgba(44, 44, 46, 230));
            // Create hg_image_1768356352082_xvk6 (hg_image)
            hg_image_1768356352082_xvk6 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768356352082_xvk6", "/app_message/message_head_icon.bin", 20, 10, 72, 72);
            // Create app_message_list_telephoneNum1 (hg_label)
            app_message_list_telephoneNum1 = gui_text_create((gui_obj_t *)note,
                                                             "app_message_list_telephoneNum1", 118, 13, 268, 40);
            gui_text_set((gui_text_t *)app_message_list_telephoneNum1, "10692506157454911958", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 20, 24);
            gui_text_type_set((gui_text_t *)app_message_list_telephoneNum1,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_telephoneNum1, MULTI_LEFT);
            // Create app_message_list_message1 (hg_label)
            app_message_list_message1 = gui_text_create((gui_obj_t *)note, "app_message_list_message1", 20, 100,
                                                        360, 104);
            gui_text_set((gui_text_t *)app_message_list_message1,
                         "[Microsoft]Use verification code 112233 for ...", GUI_FONT_SRC_BMP, gui_rgb(180, 180, 180), 47,
                         28);
            gui_text_type_set((gui_text_t *)app_message_list_message1,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_message1, MULTI_LEFT);
            gui_text_wordwrap_set((gui_text_t *)app_message_list_message1, true);
            // Create app_message_list_date1 (hg_label)
            app_message_list_date1 = gui_text_create((gui_obj_t *)note, "app_message_list_date1", 118, 50, 186,
                                                     32);
            gui_text_set((gui_text_t *)app_message_list_date1, "2025/12/25", GUI_FONT_SRC_BMP, gui_rgb(180, 180,
                         180), 10, 24);
            gui_text_type_set((gui_text_t *)app_message_list_date1,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_date1, MULTI_LEFT);
            break;
        }
    case 1:
        {
            // Create app_message_list_bg2 (hg_rect)
            app_message_list_bg2 = gui_rect_create((gui_obj_t *)note, "app_message_list_bg2", 0, 0, 390, 218,
                                                   20, gui_rgba(44, 44, 46, 230));
            // Create hg_image_1768356356559_kcug (hg_image)
            hg_image_1768356356559_kcug = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768356356559_kcug", "/app_message/message_head_icon.bin", 20, 10, 72, 72);
            // Create app_message_list_message2 (hg_label)
            app_message_list_message2 = gui_text_create((gui_obj_t *)note, "app_message_list_message2", 20, 100,
                                                        360, 80);
            gui_text_set((gui_text_t *)app_message_list_message2,
                         "[Microsoft]Use verification code 112233 for ...", GUI_FONT_SRC_BMP, gui_rgb(180, 180, 180), 47,
                         28);
            gui_text_type_set((gui_text_t *)app_message_list_message2,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_message2, MULTI_LEFT);
            gui_text_wordwrap_set((gui_text_t *)app_message_list_message2, true);
            // Create app_message_list_telephoneNum2 (hg_label)
            app_message_list_telephoneNum2 = gui_text_create((gui_obj_t *)note,
                                                             "app_message_list_telephoneNum2", 118, 13, 268, 40);
            gui_text_set((gui_text_t *)app_message_list_telephoneNum2, "10692506157454911958", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 20, 24);
            gui_text_type_set((gui_text_t *)app_message_list_telephoneNum2,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_telephoneNum2, MULTI_LEFT);
            // Create app_message_list_date2 (hg_label)
            app_message_list_date2 = gui_text_create((gui_obj_t *)note, "app_message_list_date2", 118, 50, 250,
                                                     34);
            gui_text_set((gui_text_t *)app_message_list_date2, "2025/12/25", GUI_FONT_SRC_BMP, gui_rgb(180, 180,
                         180), 10, 24);
            gui_text_type_set((gui_text_t *)app_message_list_date2,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_date2, MULTI_LEFT);
            break;
        }
    case 2:
        {
            // Create app_message_list_bg3 (hg_rect)
            app_message_list_bg3 = gui_rect_create((gui_obj_t *)note, "app_message_list_bg3", 0, 0, 390, 218,
                                                   20, gui_rgba(44, 44, 46, 230));
            // Create hg_image_1768356382959_8b09 (hg_image)
            hg_image_1768356382959_8b09 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768356382959_8b09", "/app_message/message_head_icon.bin", 20, 10, 72, 72);
            // Create app_message_list_message3 (hg_label)
            app_message_list_message3 = gui_text_create((gui_obj_t *)note, "app_message_list_message3", 20, 123,
                                                        360, 80);
            gui_text_set((gui_text_t *)app_message_list_message3,
                         "[Microsoft]Use verification code 112233 for ...", GUI_FONT_SRC_BMP, gui_rgb(180, 180, 180), 47,
                         40);
            gui_text_type_set((gui_text_t *)app_message_list_message3,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_message3, MULTI_LEFT);
            gui_text_wordwrap_set((gui_text_t *)app_message_list_message3, true);
            // Create app_message_list_telephoneNum3 (hg_label)
            app_message_list_telephoneNum3 = gui_text_create((gui_obj_t *)note,
                                                             "app_message_list_telephoneNum3", 130, 0, 250, 80);
            gui_text_set((gui_text_t *)app_message_list_telephoneNum3, "10692506157454911958", GUI_FONT_SRC_BMP,
                         gui_rgb(255, 255, 255), 20, 40);
            gui_text_type_set((gui_text_t *)app_message_list_telephoneNum3,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_telephoneNum3, MULTI_LEFT);
            // Create app_message_list_date3 (hg_label)
            app_message_list_date3 = gui_text_create((gui_obj_t *)note, "app_message_list_date3", 130, 75, 250,
                                                     50);
            gui_text_set((gui_text_t *)app_message_list_date3, "2025/12/25", GUI_FONT_SRC_BMP, gui_rgb(180, 180,
                         180), 10, 40);
            gui_text_type_set((gui_text_t *)app_message_list_date3,
                              "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)app_message_list_date3, MULTI_LEFT);
            break;
        }
    default:
        break;
    }
}


// Create app_message_view (hg_view)
static void app_message_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_message_view_switch_in(gui_view_t *view)
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



    // Create hg_list_1768285628248_uckn (hg_list)
    hg_list_1768285628248_uckn = gui_list_create((gui_obj_t *)view, "hg_list_1768285628248_uckn", 10,
                                                 110, 390, 392, 218, 5, VERTICAL, hg_list_1768285628248_uckn_note_design, NULL, false);
    gui_list_set_style(hg_list_1768285628248_uckn, LIST_CLASSIC);
    gui_list_set_note_num(hg_list_1768285628248_uckn, 3);
    gui_list_set_out_scope(hg_list_1768285628248_uckn, 80);

    // Create hg_window_1768354016343_kf7g (hg_window)
    hg_window_1768354016343_kf7g = gui_win_create((gui_obj_t *)view, "hg_window_1768354016343_kf7g", 0,
                                                  0, 410, 100);
    gui_win_enable_blur((gui_win_t *)hg_window_1768354016343_kf7g, true);
    gui_win_set_blur_degree((gui_win_t *)hg_window_1768354016343_kf7g, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(app_message_time_text_time_str, sizeof(app_message_time_text_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create hg_image_1768355370798_508r (hg_image)
    hg_image_1768355370798_508r = gui_img_create_from_fs(hg_window_1768354016343_kf7g,
                                                         "hg_image_1768355370798_508r", "/app_message/message_write_icon.bin", 27, 9, 72, 72);

    // Create app_message_title (hg_label)
    app_message_title = gui_text_create(hg_window_1768354016343_kf7g, "app_message_title", 215, 53, 172,
                                        50);
    gui_text_set((gui_text_t *)app_message_title, "Messages", GUI_FONT_SRC_BMP, gui_rgb(66, 148, 244),
                 8, 32);
    gui_text_type_set((gui_text_t *)app_message_title,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_message_title, RIGHT);

    // Create app_message_time_text (hg_time_label)
    app_message_time_text = gui_text_create(hg_window_1768354016343_kf7g, "app_message_time_text", 287,
                                            20, 100, 42);
    gui_text_set((gui_text_t *)app_message_time_text, app_message_time_text_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(app_message_time_text_time_str), 28);
    gui_text_type_set((gui_text_t *)app_message_time_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_message_time_text, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(hg_window_1768354016343_kf7g),
                         (gui_event_cb_t)hg_window_1768354016343_kf7g_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)hg_window_1768354016343_kf7g);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(app_message_time_text), 30000, true,
                         app_message_time_text_time_update_cb);
}
GUI_VIEW_INSTANCE("app_message_view", false, app_message_view_switch_in,
                  app_message_view_switch_out, false);
