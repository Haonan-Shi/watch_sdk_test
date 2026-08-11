/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_timer UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.960Z
 */
#include "app_timer_ui.h"
#include "../callbacks/app_timer_callbacks.h"
#include "../user/app_timer_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *lst_all_timers = NULL;
gui_text_t *lbl_5 = NULL;
gui_img_t *img_15 = NULL;
gui_text_t *lbl_6 = NULL;
gui_img_t *img_16 = NULL;
gui_text_t *lbl_7 = NULL;
gui_img_t *img_17 = NULL;
gui_img_t *img_18 = NULL;
gui_text_t *lbl_8 = NULL;
gui_text_t *lbl_9 = NULL;
gui_img_t *img_19 = NULL;
gui_img_t *img_20 = NULL;
gui_text_t *lbl_10 = NULL;
gui_text_t *lbl_11 = NULL;
gui_img_t *img_21 = NULL;
gui_img_t *img_22 = NULL;
gui_text_t *lbl_12 = NULL;
gui_text_t *lbl_13 = NULL;
gui_win_t *win_top = NULL;
gui_text_t *tm_lbl_1 = NULL;
gui_text_t *lbl_timer_app = NULL;
gui_img_t *bg_cancel = NULL;
gui_img_t *icon_cancel = NULL;
gui_img_t *img_25 = NULL;
gui_img_t *img_26 = NULL;
gui_img_t *bg_play = NULL;
gui_img_t *icon_play = NULL;
gui_text_t *tm_lbl_2 = NULL;
gui_text_t *active_preset_text = NULL;
gui_text_t *active_timer_text = NULL;
gui_arc_t *active_arc = NULL;

// Time string global variables
char tm_lbl_1_time_str[10] = {0};
char tm_lbl_2_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void lst_all_timers_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void lst_all_timers_note_design(gui_obj_t *obj, void *param)
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
            // Create lbl_5 (hg_label)
            lbl_5 = gui_text_create((gui_obj_t *)note, "lbl_5", 28, 125, 300, 40);
            gui_text_set((gui_text_t *)lbl_5, "All Timers", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 10, 30);
            gui_text_type_set((gui_text_t *)lbl_5, "/font/Inter24pt_SemiBold_size30_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_5, MID_LEFT);
            break;
        }
    case 1:
        {
            // Create img_15 (hg_image)
            img_15 = gui_img_create_from_fs((gui_obj_t *)note, "img_15", "/app_timer/timer_stroke_bg.bin", 28,
                                            0, 172, 172);
            gui_obj_add_event_cb(img_15, (gui_event_cb_t)img_15_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create lbl_6 (hg_label)
            lbl_6 = gui_text_create((gui_obj_t *)note, "lbl_6", 28, 0, 172, 172);
            gui_text_set((gui_text_t *)lbl_6, "01:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 32);
            gui_text_type_set((gui_text_t *)lbl_6, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_6, MID_CENTER);
            // Create img_16 (hg_image)
            img_16 = gui_img_create_from_fs((gui_obj_t *)note, "img_16", "/app_timer/timer_stroke_bg.bin", 210,
                                            0, 172, 172);
            gui_obj_add_event_cb(img_16, (gui_event_cb_t)img_16_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create lbl_7 (hg_label)
            lbl_7 = gui_text_create((gui_obj_t *)note, "lbl_7", 210, 0, 172, 172);
            gui_text_set((gui_text_t *)lbl_7, "03:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 32);
            gui_text_type_set((gui_text_t *)lbl_7, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_7, MID_CENTER);
            break;
        }
    case 2:
        {
            // Create img_17 (hg_image)
            img_17 = gui_img_create_from_fs((gui_obj_t *)note, "img_17", "/app_timer/timer_stroke_bg.bin", 28,
                                            0, 172, 172);
            gui_obj_add_event_cb(img_17, (gui_event_cb_t)img_17_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create img_18 (hg_image)
            img_18 = gui_img_create_from_fs((gui_obj_t *)note, "img_18", "/app_timer/timer_stroke_bg.bin", 210,
                                            0, 172, 172);
            gui_obj_add_event_cb(img_18, (gui_event_cb_t)img_18_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create lbl_8 (hg_label)
            lbl_8 = gui_text_create((gui_obj_t *)note, "lbl_8", 28, 0, 172, 172);
            gui_text_set((gui_text_t *)lbl_8, "05:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 32);
            gui_text_type_set((gui_text_t *)lbl_8, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_8, MID_CENTER);
            // Create lbl_9 (hg_label)
            lbl_9 = gui_text_create((gui_obj_t *)note, "lbl_9", 210, 0, 172, 172);
            gui_text_set((gui_text_t *)lbl_9, "10:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 32);
            gui_text_type_set((gui_text_t *)lbl_9, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_9, MID_CENTER);
            break;
        }
    case 3:
        {
            // Create img_19 (hg_image)
            img_19 = gui_img_create_from_fs((gui_obj_t *)note, "img_19", "/app_timer/timer_stroke_bg.bin", 28,
                                            0, 172, 172);
            gui_obj_add_event_cb(img_19, (gui_event_cb_t)img_19_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create img_20 (hg_image)
            img_20 = gui_img_create_from_fs((gui_obj_t *)note, "img_20", "/app_timer/timer_stroke_bg.bin", 210,
                                            0, 172, 172);
            gui_obj_add_event_cb(img_20, (gui_event_cb_t)img_20_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create lbl_10 (hg_label)
            lbl_10 = gui_text_create((gui_obj_t *)note, "lbl_10", 28, 0, 172, 172);
            gui_text_set((gui_text_t *)lbl_10, "15:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 32);
            gui_text_type_set((gui_text_t *)lbl_10, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_10, MID_CENTER);
            // Create lbl_11 (hg_label)
            lbl_11 = gui_text_create((gui_obj_t *)note, "lbl_11", 210, 0, 172, 172);
            gui_text_set((gui_text_t *)lbl_11, "30:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 32);
            gui_text_type_set((gui_text_t *)lbl_11, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_11, MID_CENTER);
            break;
        }
    case 4:
        {
            // Create img_21 (hg_image)
            img_21 = gui_img_create_from_fs((gui_obj_t *)note, "img_21", "/app_timer/timer_stroke_bg.bin", 28,
                                            0, 172, 172);
            gui_obj_add_event_cb(img_21, (gui_event_cb_t)img_21_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create img_22 (hg_image)
            img_22 = gui_img_create_from_fs((gui_obj_t *)note, "img_22", "/app_timer/timer_stroke_bg.bin", 210,
                                            0, 172, 172);
            gui_obj_add_event_cb(img_22, (gui_event_cb_t)img_22_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create lbl_12 (hg_label)
            lbl_12 = gui_text_create((gui_obj_t *)note, "lbl_12", 28, 0, 172, 172);
            gui_text_set((gui_text_t *)lbl_12, "01:00:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8, 32);
            gui_text_type_set((gui_text_t *)lbl_12, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_12, MID_CENTER);
            // Create lbl_13 (hg_label)
            lbl_13 = gui_text_create((gui_obj_t *)note, "lbl_13", 210, 0, 172, 172);
            gui_text_set((gui_text_t *)lbl_13, "02:00:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8, 32);
            gui_text_type_set((gui_text_t *)lbl_13, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_13, MID_CENTER);
            break;
        }
    default:
        break;
    }
}


// Create app_timer_view (hg_view)
static void app_timer_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_timer_view_switch_in(gui_view_t *view)
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



    // Create lst_all_timers (hg_list)
    lst_all_timers = gui_list_create((gui_obj_t *)view, "lst_all_timers", 0, 0, 410, 502, 172, 10,
                                     VERTICAL, lst_all_timers_note_design, NULL, false);
    gui_list_set_style(lst_all_timers, LIST_CLASSIC);
    gui_list_set_note_num(lst_all_timers, 5);

    // Create win_top (hg_window)
    win_top = gui_win_create((gui_obj_t *)view, "win_top", 0, 0, 410, 100);
    gui_win_enable_blur((gui_win_t *)win_top, true);
    gui_win_set_blur_degree((gui_win_t *)win_top, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(tm_lbl_1_time_str, sizeof(tm_lbl_1_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create tm_lbl_1 (hg_time_label)
    tm_lbl_1 = gui_text_create(win_top, "tm_lbl_1", 0, 30, 382, 40);
    gui_text_set((gui_text_t *)tm_lbl_1, tm_lbl_1_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(tm_lbl_1_time_str), 34);
    gui_text_type_set((gui_text_t *)tm_lbl_1, "/font/Inter24pt_SemiBold_size34_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)tm_lbl_1, MID_RIGHT);

    // Create lbl_timer_app (hg_label)
    lbl_timer_app = gui_text_create(win_top, "lbl_timer_app", 0, 66, 382, 36);
    gui_text_set((gui_text_t *)lbl_timer_app, "Timers", GUI_FONT_SRC_BMP, gui_rgb(241, 154, 56), 6, 32);
    gui_text_type_set((gui_text_t *)lbl_timer_app, "/font/Inter24pt_SemiBold_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)lbl_timer_app, MID_RIGHT);

    // Create bg_cancel (hg_image)
    bg_cancel = gui_img_create_from_fs(win_top, "bg_cancel", "/app_timer/bg_circle_72.bin", 28, 25, 72,
                                       72);
    gui_img_set_mode((gui_img_t *)bg_cancel, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)bg_cancel, 0xFF1f1f1f);
    gui_obj_add_event_cb(bg_cancel, (gui_event_cb_t)bg_cancel_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create icon_cancel (hg_image)
    icon_cancel = gui_img_create_from_fs(win_top, "icon_cancel", "/app_timer/timer_button_cancel.bin",
                                         49, 46, 30, 30);
    gui_img_set_mode((gui_img_t *)icon_cancel, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)icon_cancel, 0xFFFFFFFF);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(tm_lbl_1), 30000, true, tm_lbl_1_time_update_cb);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_timer_view_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_timer_view", false, app_timer_view_switch_in, app_timer_view_switch_out,
                  false);

// Create timer_running_view (hg_view)
static void timer_running_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void timer_running_view_switch_in(gui_view_t *view)
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
        snprintf(tm_lbl_2_time_str, sizeof(tm_lbl_2_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }



    // Create tm_lbl_2 (hg_time_label)
    tm_lbl_2 = gui_text_create((gui_obj_t *)view, "tm_lbl_2", 0, 30, 410, 40);
    gui_text_set((gui_text_t *)tm_lbl_2, tm_lbl_2_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(tm_lbl_2_time_str), 34);
    gui_text_type_set((gui_text_t *)tm_lbl_2, "/font/Inter24pt_SemiBold_size34_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)tm_lbl_2, MID_CENTER);

    // Create active_preset_text (hg_label)
    active_preset_text = gui_text_create((gui_obj_t *)view, "active_preset_text", 0, 300, 410, 36);
    gui_text_set((gui_text_t *)active_preset_text, "01:00", GUI_FONT_SRC_BMP, gui_rgb(241, 154, 56), 5,
                 26);
    gui_text_type_set((gui_text_t *)active_preset_text,
                      "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)active_preset_text, MID_CENTER);

    // Create active_timer_text (hg_label)
    active_timer_text = gui_text_create((gui_obj_t *)view, "active_timer_text", 0, 0, 410, 502);
    gui_text_set((gui_text_t *)active_timer_text, "01:00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 72);
    gui_text_type_set((gui_text_t *)active_timer_text,
                      "/font/Inter_24pt_Regular_size72_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)active_timer_text, MID_CENTER);
    // Bind timer: active timer update
    gui_obj_create_timer((gui_obj_t *)active_timer_text, 1000, true, active_timer_update_cb);

    // Create active_arc (hg_arc)
    active_arc = gui_arc_create((gui_obj_t *)view, "active_arc", 210, 256, 167, -89, 270, 10,
                                gui_rgb(241, 154, 56));
    // Bind timer: active arc update
    gui_obj_create_timer((gui_obj_t *)active_arc, 1000, true, active_arc_update_cb);

    // Create img_25 (hg_image)
    img_25 = gui_img_create_from_fs((gui_obj_t *)view, "img_25", "/app_timer/bg_circle_72.bin", 28, 401,
                                    72, 72);
    gui_img_set_mode((gui_img_t *)img_25, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)img_25, 0xFF1f1f1f);
    gui_obj_add_event_cb(img_25, (gui_event_cb_t)img_25_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create bg_play (hg_image)
    bg_play = gui_img_create_from_fs((gui_obj_t *)view, "bg_play", "/app_timer/bg_circle_72.bin", 310,
                                     402, 72, 72);
    gui_img_set_mode((gui_img_t *)bg_play, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)bg_play, 0xFFf19a38);
    gui_obj_add_event_cb(bg_play, (gui_event_cb_t)bg_play_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create img_26 (hg_image)
    img_26 = gui_img_create_from_fs((gui_obj_t *)view, "img_26", "/app_timer/timer_button_cancel.bin",
                                    49, 423, 30, 30);
    gui_img_set_mode((gui_img_t *)img_26, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)img_26, 0xFFFFFFFF);

    // Create icon_play (hg_image)
    icon_play = gui_img_create_from_fs((gui_obj_t *)view, "icon_play",
                                       "/app_timer/timer_button_stop.bin", 333, 422, 26, 32);
    gui_img_set_mode((gui_img_t *)icon_play, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)icon_play, 0xFF000000);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(tm_lbl_2), 30000, true, tm_lbl_2_time_update_cb);
}
GUI_VIEW_INSTANCE("timer_running_view", false, timer_running_view_switch_in,
                  timer_running_view_switch_out, false);
