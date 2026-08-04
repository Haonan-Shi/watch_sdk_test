/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_noise UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.888Z
 */
#include "app_noise_ui.h"
#include "../callbacks/app_noise_callbacks.h"
#include "../user/app_noise_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_img_t *Noise_ok_bg = NULL;
gui_img_t *hg_image_1769151866129_h3az = NULL;
gui_text_t *hg_time_label_1769151874591_kcfw = NULL;
gui_text_t *app_noise_ok_text = NULL;
gui_text_t *app_noise_data_text = NULL;
gui_img_t *Nois_Level_Meter_bg = NULL;
gui_rounded_rect_t *Nois_Level_Meter0 = NULL;
gui_rounded_rect_t *Nois_Level_Meter1 = NULL;
gui_rounded_rect_t *Nois_Level_Meter2 = NULL;
gui_rounded_rect_t *Nois_Level_Meter3 = NULL;
gui_rounded_rect_t *Nois_Level_Meter4 = NULL;
gui_rounded_rect_t *Nois_Level_Meter5 = NULL;
gui_rounded_rect_t *Nois_Level_Meter6 = NULL;
gui_rounded_rect_t *Nois_Level_Meter7 = NULL;
gui_rounded_rect_t *Nois_Level_Meter8 = NULL;
gui_rounded_rect_t *Nois_Level_Meter9 = NULL;
gui_rounded_rect_t *Nois_Level_Meter10 = NULL;
gui_rounded_rect_t *Nois_Level_Meter11 = NULL;
gui_rounded_rect_t *Nois_Level_Meter12 = NULL;
gui_rounded_rect_t *Nois_Level_Meter13 = NULL;
gui_rounded_rect_t *Nois_Level_Meter14 = NULL;
gui_text_t *app_noise_x30_text = NULL;
gui_text_t *app_noise_x80_text = NULL;
gui_text_t *app_noise_x120_text = NULL;
gui_img_t *hg_image_1769156756841_h11r = NULL;

// Time string global variables
char hg_time_label_1769151874591_kcfw_time_str[10] = {0};


// Create app_noise_view (hg_view)
static void app_noise_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_noise_view_switch_in(gui_view_t *view)
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
        snprintf(hg_time_label_1769151874591_kcfw_time_str,
                 sizeof(hg_time_label_1769151874591_kcfw_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }



    // Create Noise_ok_bg (hg_image)
    Noise_ok_bg = gui_img_create_from_fs((gui_obj_t *)view, "Noise_ok_bg", "/app_noise/Noise_ok_bg.bin",
                                         0, 0, 410, 502);
    gui_obj_add_event_cb(Noise_ok_bg, (gui_event_cb_t)Noise_ok_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create hg_image_1769151866129_h3az (hg_image)
    hg_image_1769151866129_h3az = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1769151866129_h3az", "/heart/icon_information.bin", 25, 11, 72, 72);
    gui_obj_add_event_cb(hg_image_1769151866129_h3az,
                         (gui_event_cb_t)hg_image_1769151866129_h3az_key_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)hg_image_1769151866129_h3az);

    // Create hg_time_label_1769151874591_kcfw (hg_time_label)
    hg_time_label_1769151874591_kcfw = gui_text_create((gui_obj_t *)view,
                                                       "hg_time_label_1769151874591_kcfw", 287, 21, 100, 36);
    gui_text_set((gui_text_t *)hg_time_label_1769151874591_kcfw,
                 hg_time_label_1769151874591_kcfw_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1769151874591_kcfw_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1769151874591_kcfw,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1769151874591_kcfw, RIGHT);

    // Create app_noise_ok_text (hg_label)
    app_noise_ok_text = gui_text_create((gui_obj_t *)view, "app_noise_ok_text", 114, 324, 80, 42);
    gui_text_set((gui_text_t *)app_noise_ok_text, "OK", GUI_FONT_SRC_BMP, gui_rgb(104, 225, 113), 2,
                 32);
    gui_text_type_set((gui_text_t *)app_noise_ok_text,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_noise_ok_text, LEFT);

    // Create app_noise_data_text (hg_label)
    app_noise_data_text = gui_text_create((gui_obj_t *)view, "app_noise_data_text", 40, 397, 164, 58);
    gui_text_set((gui_text_t *)app_noise_data_text, "50 dB", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 5, 40);
    gui_text_type_set((gui_text_t *)app_noise_data_text,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_noise_data_text, LEFT);

    // Create Nois_Level_Meter_bg (hg_image)
    Nois_Level_Meter_bg = gui_img_create_from_fs((gui_obj_t *)view, "Nois_Level_Meter_bg",
                                                 "/app_noise/Nois_Level_Meter.bin", 30, 140, 342, 100);
    // Bind timer: Timer animation 1
    gui_obj_create_timer((gui_obj_t *)Nois_Level_Meter_bg, 1000, true, Nois_Level_Meter_bg_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)Nois_Level_Meter_bg);

    // Create Nois_Level_Meter0 (hg_rect)
    Nois_Level_Meter0 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter0", 30, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));
    // Bind timer: Initialize timer
    gui_obj_create_timer((gui_obj_t *)Nois_Level_Meter0, 100, false, app_noise_view_init);
    gui_obj_start_timer((gui_obj_t *)Nois_Level_Meter0);

    // Create Nois_Level_Meter1 (hg_rect)
    Nois_Level_Meter1 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter1", 49, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter2 (hg_rect)
    Nois_Level_Meter2 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter2", 68, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter3 (hg_rect)
    Nois_Level_Meter3 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter3", 88, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter4 (hg_rect)
    Nois_Level_Meter4 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter4", 108, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter5 (hg_rect)
    Nois_Level_Meter5 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter5", 128, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter6 (hg_rect)
    Nois_Level_Meter6 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter6", 148, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter7 (hg_rect)
    Nois_Level_Meter7 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter7", 168, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter8 (hg_rect)
    Nois_Level_Meter8 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter8", 188, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter9 (hg_rect)
    Nois_Level_Meter9 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter9", 208, 147, 12, 86, 2,
                                        gui_rgba(104, 225, 113, 0));

    // Create Nois_Level_Meter10 (hg_rect)
    Nois_Level_Meter10 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter10", 228, 147, 12, 86, 2,
                                         gui_rgba(255, 102, 0, 0));

    // Create Nois_Level_Meter11 (hg_rect)
    Nois_Level_Meter11 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter11", 248, 147, 12, 86, 2,
                                         gui_rgba(255, 102, 0, 0));

    // Create Nois_Level_Meter12 (hg_rect)
    Nois_Level_Meter12 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter12", 268, 147, 12, 86, 2,
                                         gui_rgba(255, 102, 0, 0));

    // Create Nois_Level_Meter13 (hg_rect)
    Nois_Level_Meter13 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter13", 288, 147, 12, 86, 2,
                                         gui_rgba(255, 102, 0, 0));

    // Create Nois_Level_Meter14 (hg_rect)
    Nois_Level_Meter14 = gui_rect_create((gui_obj_t *)view, "Nois_Level_Meter14", 308, 147, 12, 86, 2,
                                         gui_rgba(255, 102, 0, 0));

    // Create app_noise_x30_text (hg_label)
    app_noise_x30_text = gui_text_create((gui_obj_t *)view, "app_noise_x30_text", 29, 247, 50, 42);
    gui_text_set((gui_text_t *)app_noise_x30_text, "30", GUI_FONT_SRC_BMP, gui_rgb(104, 225, 113), 2,
                 28);
    gui_text_type_set((gui_text_t *)app_noise_x30_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_noise_x30_text, LEFT);

    // Create app_noise_x80_text (hg_label)
    app_noise_x80_text = gui_text_create((gui_obj_t *)view, "app_noise_x80_text", 214, 247, 50, 42);
    gui_text_set((gui_text_t *)app_noise_x80_text, "80", GUI_FONT_SRC_BMP, gui_rgb(104, 225, 113), 2,
                 28);
    gui_text_type_set((gui_text_t *)app_noise_x80_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_noise_x80_text, LEFT);

    // Create app_noise_x120_text (hg_label)
    app_noise_x120_text = gui_text_create((gui_obj_t *)view, "app_noise_x120_text", 337, 247, 50, 42);
    gui_text_set((gui_text_t *)app_noise_x120_text, "120", GUI_FONT_SRC_BMP, gui_rgb(104, 225, 113), 3,
                 28);
    gui_text_type_set((gui_text_t *)app_noise_x120_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_noise_x120_text, LEFT);

    // Create hg_image_1769156756841_h11r (hg_image)
    hg_image_1769156756841_h11r = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1769156756841_h11r", "/app_noise/Noise_ok_icon.bin", 40, 315, 60, 60);
    gui_img_scale((gui_img_t *)hg_image_1769156756841_h11r, 0.833333f, 0.833333f);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1769151874591_kcfw), 30000, true,
                         hg_time_label_1769151874591_kcfw_time_update_cb);
}
GUI_VIEW_INSTANCE("app_noise_view", false, app_noise_view_switch_in, app_noise_view_switch_out,
                  false);
