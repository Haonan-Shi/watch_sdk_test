/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * watchface_sport UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:41.034Z
 */
#include "watchface_sport_ui.h"
#include "../callbacks/watchface_sport_callbacks.h"
#include "../user/watchface_sport_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_img_t *hg_image_1766555010304_99ja = NULL;
gui_img_t *hg_image_1766555014041_zggx = NULL;
gui_img_t *hg_image_1766555023329_izzj = NULL;
gui_arc_t *hg_arc_1766555327383_kikf = NULL;
gui_arc_t *hg_arc_1766555341316_qd5r = NULL;
gui_arc_t *hg_arc_1766555343134_ku6q = NULL;
gui_arc_t *hg_arc_1766556753455_tg7q = NULL;
gui_arc_t *hg_arc_1766556813443_1zka = NULL;
gui_arc_t *hg_arc_1766556869456_faq0 = NULL;
gui_text_t *hg_label_1766558002262_3ych = NULL;
gui_text_t *hg_label_1766558226116_kswi = NULL;
gui_text_t *hg_label_1766558284986_myaw = NULL;
gui_text_t *hg_label_1766558608627_zo66 = NULL;
gui_text_t *hg_time_label_1768897114762_8ilu = NULL;

// Time string global variables
char hg_time_label_1768897114762_8ilu_time_str[10] = {0};


// Create sportView (hg_view)
static void sportView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void sportView_switch_in(gui_view_t *view)
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
        snprintf(hg_time_label_1768897114762_8ilu_time_str,
                 sizeof(hg_time_label_1768897114762_8ilu_time_str), "%02d:%02d:%02d", t->tm_hour, t->tm_min,
                 t->tm_sec);
    }



    // Create hg_image_1766555010304_99ja (hg_image)
    hg_image_1766555010304_99ja = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1766555010304_99ja", "/UI_app_list/app_weather_icon.bin", 20, 16, 80, 80);
    gui_img_scale((gui_img_t *)hg_image_1766555010304_99ja, 0.800000f, 0.800000f);

    // Create hg_image_1766555014041_zggx (hg_image)
    hg_image_1766555014041_zggx = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1766555014041_zggx", "/UI_app_list/app_workout_icon.bin", 312, 16, 80, 80);
    gui_img_scale((gui_img_t *)hg_image_1766555014041_zggx, 0.800000f, 0.800000f);
    gui_obj_add_event_cb(hg_image_1766555014041_zggx,
                         (gui_event_cb_t)hg_image_1766555014041_zggx_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hg_image_1766555023329_izzj (hg_image)
    hg_image_1766555023329_izzj = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1766555023329_izzj", "/UI_app_list/app_heart_rate_icon.bin", 35, 425, 60, 60);
    gui_img_scale((gui_img_t *)hg_image_1766555023329_izzj, 0.600000f, 0.600000f);

    // Create hg_arc_1766555327383_kikf (hg_arc)
    hg_arc_1766555327383_kikf = gui_arc_create((gui_obj_t *)view, "hg_arc_1766555327383_kikf", 135, 287,
                                               105, 0, 360, 20, gui_rgb(58, 23, 29));

    // Create hg_arc_1766555341316_qd5r (hg_arc)
    hg_arc_1766555341316_qd5r = gui_arc_create((gui_obj_t *)view, "hg_arc_1766555341316_qd5r", 135, 287,
                                               55, 0, 360, 20, gui_rgb(22, 50, 47));

    // Create hg_arc_1766555343134_ku6q (hg_arc)
    hg_arc_1766555343134_ku6q = gui_arc_create((gui_obj_t *)view, "hg_arc_1766555343134_ku6q", 135, 287,
                                               80, 0, 360, 20, gui_rgb(30, 55, 25));

    // Create hg_arc_1766556753455_tg7q (hg_arc)
    hg_arc_1766556753455_tg7q = gui_arc_create((gui_obj_t *)view, "hg_arc_1766556753455_tg7q", 135, 287,
                                               105, 270, 270, 20, gui_rgb(230, 64, 74));
    // Set angular gradient
    gui_arc_set_angular_gradient(hg_arc_1766556753455_tg7q, 270, 270);
    gui_arc_add_gradient_stop(hg_arc_1766556753455_tg7q, 0.0f, gui_rgba(233, 64, 46, 255));
    gui_arc_add_gradient_stop(hg_arc_1766556753455_tg7q, 1.0f, gui_rgba(234, 65, 143, 255));
    gui_msg_subscribe((gui_obj_t *)hg_arc_1766556753455_tg7q, "sport_move",
                      hg_arc_1766556753455_tg7q_msg_cb_0);

    // Create hg_arc_1766556813443_1zka (hg_arc)
    hg_arc_1766556813443_1zka = gui_arc_create((gui_obj_t *)view, "hg_arc_1766556813443_1zka", 135, 287,
                                               80, 270, 310, 20, gui_rgb(189, 254, 74));
    // Set angular gradient
    gui_arc_set_angular_gradient(hg_arc_1766556813443_1zka, 270, 270);
    gui_arc_add_gradient_stop(hg_arc_1766556813443_1zka, 0.0f, gui_rgba(171, 246, 76, 255));
    gui_arc_add_gradient_stop(hg_arc_1766556813443_1zka, 1.0f, gui_rgba(220, 249, 80, 255));

    // Create hg_arc_1766556869456_faq0 (hg_arc)
    hg_arc_1766556869456_faq0 = gui_arc_create((gui_obj_t *)view, "hg_arc_1766556869456_faq0", 135, 287,
                                               55, 270, 270, 20, gui_rgb(115, 230, 230));
    // Set angular gradient
    gui_arc_set_angular_gradient(hg_arc_1766556869456_faq0, 270, 270);
    gui_arc_add_gradient_stop(hg_arc_1766556869456_faq0, 0.0f, gui_rgba(95, 202, 216, 255));
    gui_arc_add_gradient_stop(hg_arc_1766556869456_faq0, 1.0f, gui_rgba(116, 248, 174, 255));

    // Create hg_label_1766558002262_3ych (hg_label)
    hg_label_1766558002262_3ych = gui_text_create((gui_obj_t *)view, "hg_label_1766558002262_3ych", 270,
                                                  178, 128, 90);
    gui_text_set((gui_text_t *)hg_label_1766558002262_3ych, "439", GUI_FONT_SRC_BMP, gui_rgb(230, 64,
                 74), 3, 64);
    gui_text_type_set((gui_text_t *)hg_label_1766558002262_3ych,
                      "/font/Inter_24pt_Regular_size64_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1766558002262_3ych, RIGHT);

    // Create hg_label_1766558226116_kswi (hg_label)
    hg_label_1766558226116_kswi = gui_text_create((gui_obj_t *)view, "hg_label_1766558226116_kswi", 270,
                                                  342, 128, 90);
    gui_text_set((gui_text_t *)hg_label_1766558226116_kswi, "07", GUI_FONT_SRC_BMP, gui_rgb(115, 230,
                 230), 2, 64);
    gui_text_type_set((gui_text_t *)hg_label_1766558226116_kswi,
                      "/font/Inter_24pt_Regular_size64_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1766558226116_kswi, RIGHT);

    // Create hg_label_1766558284986_myaw (hg_label)
    hg_label_1766558284986_myaw = gui_text_create((gui_obj_t *)view, "hg_label_1766558284986_myaw", 270,
                                                  252, 128, 90);
    gui_text_set((gui_text_t *)hg_label_1766558284986_myaw, "02", GUI_FONT_SRC_BMP, gui_rgb(189, 254,
                 74), 2, 64);
    gui_text_type_set((gui_text_t *)hg_label_1766558284986_myaw,
                      "/font/Inter_24pt_Regular_size64_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1766558284986_myaw, RIGHT);

    // Create hg_label_1766558608627_zo66 (hg_label)
    hg_label_1766558608627_zo66 = gui_text_create((gui_obj_t *)view, "hg_label_1766558608627_zo66", 107,
                                                  435, 250, 50);
    gui_text_set((gui_text_t *)hg_label_1766558608627_zo66, "80 bpm,1m ago", GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), 13, 32);
    gui_text_type_set((gui_text_t *)hg_label_1766558608627_zo66,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1766558608627_zo66, LEFT);

    // Create hg_time_label_1768897114762_8ilu (hg_time_label)
    hg_time_label_1768897114762_8ilu = gui_text_create((gui_obj_t *)view,
                                                       "hg_time_label_1768897114762_8ilu", 145, 103, 270, 80);
    gui_text_set((gui_text_t *)hg_time_label_1768897114762_8ilu,
                 hg_time_label_1768897114762_8ilu_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1768897114762_8ilu_time_str), 64);
    gui_text_type_set((gui_text_t *)hg_time_label_1768897114762_8ilu,
                      "/font/Inter_24pt_Regular_size64_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1768897114762_8ilu, LEFT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1768897114762_8ilu), 500, true,
                         hg_time_label_1768897114762_8ilu_time_update_cb);
}
GUI_VIEW_INSTANCE("sportView", false, sportView_switch_in, sportView_switch_out, false);
