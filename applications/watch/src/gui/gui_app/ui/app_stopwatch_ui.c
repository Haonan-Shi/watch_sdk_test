/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_stopwatch UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.952Z
 */
#include "app_stopwatch_ui.h"
#include "../callbacks/app_stopwatch_callbacks.h"
#include "../user/app_stopwatch_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *lst_stopwatch = NULL;
gui_img_t *img_27 = NULL;
gui_img_t *img_28 = NULL;
gui_img_t *sec_hand_big_0 = NULL;
gui_img_t *min_hand_0 = NULL;
gui_text_t *lbl_14 = NULL;
gui_img_t *sec_hand_big_1 = NULL;
gui_img_t *img_29 = NULL;
gui_img_t *img_30 = NULL;
gui_img_t *img_31 = NULL;
gui_img_t *min_hand_1 = NULL;
gui_img_t *sec_hand_1 = NULL;
gui_img_t *milsec_hand_1 = NULL;
gui_text_t *lbl_15 = NULL;
gui_img_t *dot_0 = NULL;
gui_text_t *tm_lbl_3 = NULL;
gui_img_t *bg_l = NULL;
gui_img_t *icon_l = NULL;
gui_img_t *bg_r = NULL;
gui_img_t *icon_r = NULL;
gui_img_t *dot_1 = NULL;

// Time string global variables
char tm_lbl_3_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void lst_stopwatch_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void lst_stopwatch_note_design(gui_obj_t *obj, void *param)
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
            // Create img_27 (hg_image)
            img_27 = gui_img_create_from_fs((gui_obj_t *)note, "img_27",
                                            "/stopwatch/stopwatch_sec_container_big.bin", 30, 76, 350, 350);
            gui_img_set_mode((gui_img_t *)img_27, IMG_COVER_MODE);
            // Create img_28 (hg_image)
            img_28 = gui_img_create_from_fs((gui_obj_t *)note, "img_28",
                                            "/stopwatch/stopwatch_min_container.bin", 154, 128, 100, 100);
            gui_img_set_mode((gui_img_t *)img_28, IMG_SRC_OVER_MODE);
            // Create sec_hand_big_0 (hg_image)
            sec_hand_big_0 = gui_img_create_from_fs((gui_obj_t *)note, "sec_hand_big_0",
                                                    "/stopwatch/stopwatch_hand_big.bin", 196, 78, 16, 202);
            gui_img_set_mode((gui_img_t *)sec_hand_big_0, IMG_2D_SW_FIX_A8_FG);
            gui_img_a8_recolor((gui_img_t *)sec_hand_big_0, 0xFFEC602A);
            gui_img_set_quality((gui_img_t *)sec_hand_big_0, true);
            gui_img_translate((gui_img_t *)sec_hand_big_0, 7.0f, 170.0f);
            gui_img_set_focus((gui_img_t *)sec_hand_big_0, 7.0f, 170.0f);
            // Create min_hand_0 (hg_image)
            min_hand_0 = gui_img_create_from_fs((gui_obj_t *)note, "min_hand_0",
                                                "/stopwatch/stopwatch_hand_s.bin", 200, 132, 7, 50);
            gui_img_set_mode((gui_img_t *)min_hand_0, IMG_2D_SW_FIX_A8_FG);
            gui_img_a8_recolor((gui_img_t *)min_hand_0, 0xFFEC602A);
            gui_img_set_quality((gui_img_t *)min_hand_0, true);
            gui_img_translate((gui_img_t *)min_hand_0, 3.0f, 46.0f);
            gui_img_set_focus((gui_img_t *)min_hand_0, 3.0f, 46.0f);
            // Create lbl_14 (hg_label)
            lbl_14 = gui_text_create((gui_obj_t *)note, "lbl_14", 110, 282, 282, 48);
            gui_text_set((gui_text_t *)lbl_14, "00:00.00", GUI_FONT_SRC_BMP, gui_rgb(0, 0, 0), 8, 44);
            gui_text_type_set((gui_text_t *)lbl_14, "/font/Inter24pt_Medium_size44_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_14, LEFT);
            // Create sec_hand_big_1 (hg_image)
            sec_hand_big_1 = gui_img_create_from_fs((gui_obj_t *)note, "sec_hand_big_1",
                                                    "/stopwatch/stopwatch_hand_big.bin", 196, 78, 16, 202);
            gui_img_set_mode((gui_img_t *)sec_hand_big_1, IMG_2D_SW_FIX_A8_FG);
            gui_img_a8_recolor((gui_img_t *)sec_hand_big_1, 0xFF7B81DD);
            gui_img_set_quality((gui_img_t *)sec_hand_big_1, true);
            gui_img_translate((gui_img_t *)sec_hand_big_1, 14.0f, 340.0f);
            gui_img_set_focus((gui_img_t *)sec_hand_big_1, 7.0f, 170.0f);
            gui_obj_hidden((gui_obj_t *)sec_hand_big_1, true);
            // Bind timer: Animation 1
            gui_obj_create_timer((gui_obj_t *)note, 10, true, stopwatch_page_0_timer_cb);
            gui_obj_start_timer((gui_obj_t *)note);
            break;
        }
    case 1:
        {
            // Create img_29 (hg_image)
            img_29 = gui_img_create_from_fs((gui_obj_t *)note, "img_29",
                                            "/stopwatch/stopwatch_min_container.bin", 24, 100, 100, 100);
            // Create img_30 (hg_image)
            img_30 = gui_img_create_from_fs((gui_obj_t *)note, "img_30",
                                            "/stopwatch/stopwatch_sec_container.bin", 155, 100, 100, 100);
            // Create img_31 (hg_image)
            img_31 = gui_img_create_from_fs((gui_obj_t *)note, "img_31",
                                            "/stopwatch/stopwatch_milsec_container.bin", 286, 100, 100, 100);
            // Create min_hand_1 (hg_image)
            min_hand_1 = gui_img_create_from_fs((gui_obj_t *)note, "min_hand_1",
                                                "/stopwatch/stopwatch_hand_s.bin", 70, 104, 7, 50);
            gui_img_set_mode((gui_img_t *)min_hand_1, IMG_2D_SW_FIX_A8_FG);
            gui_img_a8_recolor((gui_img_t *)min_hand_1, 0xFFEC602A);
            gui_img_set_quality((gui_img_t *)min_hand_1, true);
            gui_img_translate((gui_img_t *)min_hand_1, 3.0f, 46.0f);
            gui_img_set_focus((gui_img_t *)min_hand_1, 3.0f, 46.0f);
            // Create sec_hand_1 (hg_image)
            sec_hand_1 = gui_img_create_from_fs((gui_obj_t *)note, "sec_hand_1",
                                                "/stopwatch/stopwatch_hand_s.bin", 201, 103, 7, 50);
            gui_img_set_mode((gui_img_t *)sec_hand_1, IMG_2D_SW_FIX_A8_FG);
            gui_img_a8_recolor((gui_img_t *)sec_hand_1, 0xFFEC602A);
            gui_img_set_quality((gui_img_t *)sec_hand_1, true);
            gui_img_translate((gui_img_t *)sec_hand_1, 3.0f, 46.0f);
            gui_img_set_focus((gui_img_t *)sec_hand_1, 3.0f, 46.0f);
            // Create milsec_hand_1 (hg_image)
            milsec_hand_1 = gui_img_create_from_fs((gui_obj_t *)note, "milsec_hand_1",
                                                   "/stopwatch/stopwatch_hand_s.bin", 332, 103, 7, 50);
            gui_img_set_mode((gui_img_t *)milsec_hand_1, IMG_2D_SW_FIX_A8_FG);
            gui_img_a8_recolor((gui_img_t *)milsec_hand_1, 0xFFEC602A);
            gui_img_set_quality((gui_img_t *)milsec_hand_1, true);
            gui_img_translate((gui_img_t *)milsec_hand_1, 3.0f, 46.0f);
            gui_img_set_focus((gui_img_t *)milsec_hand_1, 3.0f, 46.0f);
            // Create lbl_15 (hg_label)
            lbl_15 = gui_text_create((gui_obj_t *)note, "lbl_15", 36, 240, 350, 90);
            gui_text_set((gui_text_t *)lbl_15, "00:00.00", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8, 80);
            gui_text_type_set((gui_text_t *)lbl_15, "/font/Inter24pt_Medium_size80_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)lbl_15, MID_LEFT);
            // Bind timer: Animation 1
            gui_obj_create_timer((gui_obj_t *)note, 10, true, stopwatch_page_1_timer_cb);
            gui_obj_start_timer((gui_obj_t *)note);
            break;
        }
    default:
        break;
    }
}


// Create app_stopwatch_view (hg_view)
static void app_stopwatch_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_stopwatch_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    // Bind timer: Animation 1
    gui_obj_create_timer((gui_obj_t *)view, 10, true, app_stopwatch_view_timer_0_cb);

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t != NULL)
    {
        snprintf(tm_lbl_3_time_str, sizeof(tm_lbl_3_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }



    // Create lst_stopwatch (hg_list)
    lst_stopwatch = gui_list_create((gui_obj_t *)view, "lst_stopwatch", 0, 0, 410, 502, 502, 0,
                                    VERTICAL, lst_stopwatch_note_design, NULL, false);
    gui_list_set_style(lst_stopwatch, LIST_FADE);
    gui_list_set_note_num(lst_stopwatch, 2);
    gui_list_set_auto_align(lst_stopwatch, true);
    gui_list_set_inertia(lst_stopwatch, false);
    // Bind timer: list timer
    gui_obj_create_timer((gui_obj_t *)lst_stopwatch, 10, true, lst_stopwatch_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)lst_stopwatch);

    // Create dot_0 (hg_image)
    dot_0 = gui_img_create_from_fs((gui_obj_t *)view, "dot_0", "/scrollbar/scrollbar_s.bin", 394, 60,
                                   10, 10);
    gui_img_set_mode((gui_img_t *)dot_0, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)dot_0, 0xFFFFFFFF);

    // Create dot_1 (hg_image)
    dot_1 = gui_img_create_from_fs((gui_obj_t *)view, "dot_1", "/scrollbar/scrollbar_s.bin", 394, 74,
                                   10, 10);
    gui_img_set_mode((gui_img_t *)dot_1, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)dot_1, 0xFF808080);

    // Create tm_lbl_3 (hg_time_label)
    tm_lbl_3 = gui_text_create((gui_obj_t *)view, "tm_lbl_3", 0, 18, 390, 42);
    gui_text_set((gui_text_t *)tm_lbl_3, tm_lbl_3_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(tm_lbl_3_time_str), 32);
    gui_text_type_set((gui_text_t *)tm_lbl_3, "/font/Inter24pt_SemiBold_size32_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)tm_lbl_3, MID_RIGHT);

    // Create bg_l (hg_image)
    bg_l = gui_img_create_from_fs((gui_obj_t *)view, "bg_l", "/stopwatch/stopwatch_button_bg.bin", 18,
                                  414, 72, 72);
    gui_img_set_mode((gui_img_t *)bg_l, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)bg_l, 0xFFB7B7B7);
    gui_obj_add_event_cb(bg_l, (gui_event_cb_t)bg_l_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create icon_l (hg_image)
    icon_l = gui_img_create_from_fs((gui_obj_t *)view, "icon_l", "/stopwatch/stopwatch_button_mark.bin",
                                    39, 435, 35, 30);
    gui_img_set_mode((gui_img_t *)icon_l, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)icon_l, 0xFF000000);
    gui_obj_hidden((gui_obj_t *)icon_l, true);

    // Create bg_r (hg_image)
    bg_r = gui_img_create_from_fs((gui_obj_t *)view, "bg_r", "/stopwatch/stopwatch_button_bg.bin", 320,
                                  414, 72, 72);
    gui_img_set_mode((gui_img_t *)bg_r, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)bg_r, 0xFF65DC7B);
    gui_obj_add_event_cb(bg_r, (gui_event_cb_t)bg_r_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create icon_r (hg_image)
    icon_r = gui_img_create_from_fs((gui_obj_t *)view, "icon_r",
                                    "/stopwatch/stopwatch_button_start.bin", 345, 435, 26, 30);
    gui_img_set_mode((gui_img_t *)icon_r, IMG_2D_SW_FIX_A8_FG);
    gui_img_a8_recolor((gui_img_t *)icon_r, 0xFF000000);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_stopwatch_view_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(tm_lbl_3), 30000, true, tm_lbl_3_time_update_cb);
}
GUI_VIEW_INSTANCE("app_stopwatch_view", false, app_stopwatch_view_switch_in,
                  app_stopwatch_view_switch_out, false);
