/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_ota UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.901Z
 */
#include "app_ota_ui.h"
#include "../callbacks/app_ota_callbacks.h"
#include "../user/app_ota_user.h"
#include <stddef.h>

// Component handle definitions
gui_img_t *ota_ready_back_btn = NULL;
gui_text_t *ota_ready_version = NULL;
gui_img_t *ota_ready_glow = NULL;
gui_arc_t *ota_ready_ring_bg = NULL;
gui_text_t *ota_ready_percent = NULL;
gui_text_t *ota_ready_pct_sym = NULL;
gui_text_t *ota_ready_status = NULL;
gui_img_t *ota_ready_dots = NULL;
gui_img_t *ota_connect_btn = NULL;
gui_img_t *ota_starting_back_btn = NULL;
gui_text_t *ota_starting_version = NULL;
gui_img_t *ota_starting_glow = NULL;
gui_arc_t *ota_starting_ring_bg = NULL;
gui_text_t *ota_starting_percent = NULL;
gui_text_t *ota_starting_pct_sym = NULL;
gui_text_t *ota_starting_status = NULL;
gui_img_t *ota_starting_dots = NULL;
gui_img_t *ota_updating_back_btn = NULL;
gui_text_t *ota_updating_version = NULL;
gui_img_t *ota_updating_glow = NULL;
gui_arc_t *ota_updating_ring_bg = NULL;
gui_arc_t *ota_updating_ring = NULL;
gui_text_t *ota_updating_percent = NULL;
gui_text_t *ota_updating_pct_sym = NULL;
gui_text_t *ota_updating_status = NULL;
gui_img_t *ota_updating_dots = NULL;
gui_img_t *ota_success_back_btn = NULL;
gui_text_t *ota_success_version = NULL;
gui_arc_t *ota_success_ring_bg = NULL;
gui_arc_t *ota_success_ring = NULL;
gui_text_t *ota_success_percent = NULL;
gui_text_t *ota_success_pct_sym = NULL;
gui_img_t *ota_success_check = NULL;
gui_text_t *ota_success_status = NULL;
gui_img_t *ota_done_btn = NULL;
gui_img_t *ota_failed_back_btn = NULL;
gui_text_t *ota_failed_version = NULL;
gui_arc_t *ota_failed_ring_bg = NULL;
gui_arc_t *ota_failed_ring = NULL;
gui_text_t *ota_failed_percent = NULL;
gui_text_t *ota_failed_pct_sym = NULL;
gui_text_t *ota_failed_status = NULL;
gui_img_t *ota_retry_btn = NULL;


// Create app_otaReadyView (hg_view)
static void app_otaReadyView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_otaReadyView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create ota_ready_glow (hg_image)
    ota_ready_glow = gui_img_create_from_fs((gui_obj_t *)view, "ota_ready_glow",
                                            "/app_ota/glow_pulse/frame_00.bin", 75, 120, 260, 260);

    // Create ota_ready_back_btn (hg_image)
    ota_ready_back_btn = gui_img_create_from_fs((gui_obj_t *)view, "ota_ready_back_btn",
                                                "/app_ota/back_icon.bin", 32, 28, 32, 32);
    gui_obj_add_event_cb(ota_ready_back_btn, (gui_event_cb_t)ota_ready_back_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create ota_ready_version (hg_label)
    ota_ready_version = gui_text_create((gui_obj_t *)view, "ota_ready_version", 130, 76, 150, 20);
    gui_text_set((gui_text_t *)ota_ready_version, "Firmware v2.4.1", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                 255), 15, 14);
    gui_text_type_set((gui_text_t *)ota_ready_version,
                      "/font/Inter_24pt_Regular_size14_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_ready_version, MID_CENTER);

    // Create ota_ready_ring_bg (hg_arc)
    ota_ready_ring_bg = gui_arc_create((gui_obj_t *)view, "ota_ready_ring_bg", 205, 250, 105, 0, 360,
                                       10, gui_rgba(255, 255, 255, 8));

    // Create ota_ready_status (hg_label)
    ota_ready_status = gui_text_create((gui_obj_t *)view, "ota_ready_status", 95, 385, 220, 24);
    gui_text_set((gui_text_t *)ota_ready_status, "Waiting for Phone", GUI_FONT_SRC_BMP, gui_rgb(0, 229,
                 160), 17, 18);
    gui_text_type_set((gui_text_t *)ota_ready_status,
                      "/font/Inter_24pt_Regular_size18_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_ready_status, MID_CENTER);

    // Create ota_ready_dots (hg_image)
    ota_ready_dots = gui_img_create_from_fs((gui_obj_t *)view, "ota_ready_dots",
                                            "/app_ota/pulsing_dots/frame_00.bin", 175, 413, 50, 20);
    // Bind timer: dots_anim
    gui_obj_create_timer((gui_obj_t *)ota_ready_dots, 33, true, ota_ready_dots_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)ota_ready_dots);

    // Create ota_connect_btn (hg_image)
    ota_connect_btn = gui_img_create_from_fs((gui_obj_t *)view, "ota_connect_btn",
                                             "/app_ota/install_btn.bin", 100, 430, 220, 52);
    gui_obj_add_event_cb(ota_connect_btn, (gui_event_cb_t)ota_connect_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create ota_ready_percent (hg_label)
    ota_ready_percent = gui_text_create((gui_obj_t *)view, "ota_ready_percent", 150, 220, 100, 60);
    gui_text_set((gui_text_t *)ota_ready_percent, "0", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1, 54);
    gui_text_type_set((gui_text_t *)ota_ready_percent,
                      "/font/Inter_24pt_Regular_size54_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_ready_percent, MID_CENTER);

    // Create ota_ready_pct_sym (hg_label)
    ota_ready_pct_sym = gui_text_create((gui_obj_t *)view, "ota_ready_pct_sym", 230, 230, 30, 40);
    gui_text_set((gui_text_t *)ota_ready_pct_sym, "%", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1, 26);
    gui_text_type_set((gui_text_t *)ota_ready_pct_sym,
                      "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_ready_pct_sym, MID_LEFT);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_otaReadyView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_otaReadyView", false, app_otaReadyView_switch_in,
                  app_otaReadyView_switch_out, false);

// Create app_otaStartingView (hg_view)
static void app_otaStartingView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_otaStartingView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create ota_starting_glow (hg_image)
    ota_starting_glow = gui_img_create_from_fs((gui_obj_t *)view, "ota_starting_glow",
                                               "/app_ota/glow_pulse/frame_00.bin", 75, 120, 260, 260);
    // Bind timer: glow_anim
    gui_obj_create_timer((gui_obj_t *)ota_starting_glow, 33, true, ota_starting_glow_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)ota_starting_glow);

    // Create ota_starting_back_btn (hg_image)
    ota_starting_back_btn = gui_img_create_from_fs((gui_obj_t *)view, "ota_starting_back_btn",
                                                   "/app_ota/back_icon.bin", 32, 28, 32, 32);

    // Create ota_starting_version (hg_label)
    ota_starting_version = gui_text_create((gui_obj_t *)view, "ota_starting_version", 130, 76, 150, 20);
    gui_text_set((gui_text_t *)ota_starting_version, "Firmware v2.4.1", GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), 15, 14);
    gui_text_type_set((gui_text_t *)ota_starting_version,
                      "/font/Inter_24pt_Regular_size14_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_starting_version, MID_CENTER);

    // Create ota_starting_ring_bg (hg_arc)
    ota_starting_ring_bg = gui_arc_create((gui_obj_t *)view, "ota_starting_ring_bg", 205, 250, 105, 0,
                                          360, 10, gui_rgba(255, 255, 255, 8));
    // Bind timer: starting_delay
    gui_obj_create_timer((gui_obj_t *)ota_starting_ring_bg, 2000, false, ota_starting_timer_cb);
    gui_obj_start_timer((gui_obj_t *)ota_starting_ring_bg);

    // Create ota_starting_status (hg_label)
    ota_starting_status = gui_text_create((gui_obj_t *)view, "ota_starting_status", 95, 385, 220, 24);
    gui_text_set((gui_text_t *)ota_starting_status, "Preparing...", GUI_FONT_SRC_BMP, gui_rgb(0, 229,
                 160), 12, 18);
    gui_text_type_set((gui_text_t *)ota_starting_status,
                      "/font/Inter_24pt_Regular_size18_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_starting_status, MID_CENTER);

    // Create ota_starting_dots (hg_image)
    ota_starting_dots = gui_img_create_from_fs((gui_obj_t *)view, "ota_starting_dots",
                                               "/app_ota/pulsing_dots/frame_00.bin", 175, 413, 50, 20);
    // Bind timer: dots_anim
    gui_obj_create_timer((gui_obj_t *)ota_starting_dots, 33, true, ota_starting_dots_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)ota_starting_dots);

    // Create ota_starting_percent (hg_label)
    ota_starting_percent = gui_text_create((gui_obj_t *)view, "ota_starting_percent", 150, 220, 100,
                                           60);
    gui_text_set((gui_text_t *)ota_starting_percent, "0", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1,
                 54);
    gui_text_type_set((gui_text_t *)ota_starting_percent,
                      "/font/Inter_24pt_Regular_size54_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_starting_percent, MID_CENTER);

    // Create ota_starting_pct_sym (hg_label)
    ota_starting_pct_sym = gui_text_create((gui_obj_t *)view, "ota_starting_pct_sym", 230, 230, 30, 40);
    gui_text_set((gui_text_t *)ota_starting_pct_sym, "%", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1,
                 26);
    gui_text_type_set((gui_text_t *)ota_starting_pct_sym,
                      "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_starting_pct_sym, MID_LEFT);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_otaStartingView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_otaStartingView", false, app_otaStartingView_switch_in,
                  app_otaStartingView_switch_out, false);

// Create app_otaUpdatingView (hg_view)
static void app_otaUpdatingView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_otaUpdatingView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create ota_updating_glow (hg_image)
    ota_updating_glow = gui_img_create_from_fs((gui_obj_t *)view, "ota_updating_glow",
                                               "/app_ota/glow_pulse/frame_00.bin", 75, 120, 260, 260);
    // Bind timer: glow_anim
    gui_obj_create_timer((gui_obj_t *)ota_updating_glow, 33, true, ota_updating_glow_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)ota_updating_glow);

    // Create ota_updating_back_btn (hg_image)
    ota_updating_back_btn = gui_img_create_from_fs((gui_obj_t *)view, "ota_updating_back_btn",
                                                   "/app_ota/back_icon.bin", 32, 28, 32, 32);

    // Create ota_updating_version (hg_label)
    ota_updating_version = gui_text_create((gui_obj_t *)view, "ota_updating_version", 130, 76, 150, 20);
    gui_text_set((gui_text_t *)ota_updating_version, "Firmware v2.4.1", GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), 15, 14);
    gui_text_type_set((gui_text_t *)ota_updating_version,
                      "/font/Inter_24pt_Regular_size14_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_updating_version, MID_CENTER);

    // Create ota_updating_ring_bg (hg_arc)
    ota_updating_ring_bg = gui_arc_create((gui_obj_t *)view, "ota_updating_ring_bg", 205, 250, 105, 0,
                                          360, 10, gui_rgba(255, 255, 255, 8));
    // Bind timer: progress_tick
    gui_obj_create_timer((gui_obj_t *)ota_updating_ring_bg, 50, true, ota_progress_tick_cb);
    gui_obj_start_timer((gui_obj_t *)ota_updating_ring_bg);

    // Create ota_updating_status (hg_label)
    ota_updating_status = gui_text_create((gui_obj_t *)view, "ota_updating_status", 95, 385, 220, 24);
    gui_text_set((gui_text_t *)ota_updating_status, "Updating...", GUI_FONT_SRC_BMP, gui_rgb(0, 229,
                 160), 11, 18);
    gui_text_type_set((gui_text_t *)ota_updating_status,
                      "/font/Inter_24pt_Regular_size18_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_updating_status, MID_CENTER);

    // Create ota_updating_dots (hg_image)
    ota_updating_dots = gui_img_create_from_fs((gui_obj_t *)view, "ota_updating_dots",
                                               "/app_ota/pulsing_dots/frame_00.bin", 175, 413, 50, 20);
    // Bind timer: dots_anim
    gui_obj_create_timer((gui_obj_t *)ota_updating_dots, 33, true, ota_updating_dots_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)ota_updating_dots);

    // Create ota_updating_ring (hg_arc)
    ota_updating_ring = gui_arc_create((gui_obj_t *)view, "ota_updating_ring", 205, 250, 105, -90, -90,
                                       10, gui_rgb(0, 229, 160));

    // Create ota_updating_percent (hg_label)
    ota_updating_percent = gui_text_create((gui_obj_t *)view, "ota_updating_percent", 150, 220, 100,
                                           60);
    gui_text_set((gui_text_t *)ota_updating_percent, "0", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1,
                 54);
    gui_text_type_set((gui_text_t *)ota_updating_percent,
                      "/font/Inter_24pt_Regular_size54_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_updating_percent, MID_CENTER);

    // Create ota_updating_pct_sym (hg_label)
    ota_updating_pct_sym = gui_text_create((gui_obj_t *)view, "ota_updating_pct_sym", 230, 230, 30, 40);
    gui_text_set((gui_text_t *)ota_updating_pct_sym, "%", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1,
                 26);
    gui_text_type_set((gui_text_t *)ota_updating_pct_sym,
                      "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_updating_pct_sym, MID_LEFT);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_otaUpdatingView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_otaUpdatingView", false, app_otaUpdatingView_switch_in,
                  app_otaUpdatingView_switch_out, false);

// Create app_otaSuccessView (hg_view)
static void app_otaSuccessView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_otaSuccessView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create ota_success_back_btn (hg_image)
    ota_success_back_btn = gui_img_create_from_fs((gui_obj_t *)view, "ota_success_back_btn",
                                                  "/app_ota/back_icon.bin", 32, 28, 32, 32);
    gui_obj_add_event_cb(ota_success_back_btn, (gui_event_cb_t)ota_success_back_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create ota_success_version (hg_label)
    ota_success_version = gui_text_create((gui_obj_t *)view, "ota_success_version", 130, 76, 150, 20);
    gui_text_set((gui_text_t *)ota_success_version, "Firmware v2.4.1", GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), 15, 14);
    gui_text_type_set((gui_text_t *)ota_success_version,
                      "/font/Inter_24pt_Regular_size14_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_success_version, MID_CENTER);

    // Create ota_success_ring_bg (hg_arc)
    ota_success_ring_bg = gui_arc_create((gui_obj_t *)view, "ota_success_ring_bg", 205, 250, 105, 0,
                                         360, 10, gui_rgba(255, 255, 255, 8));

    // Create ota_success_check (hg_image)
    ota_success_check = gui_img_create_from_fs((gui_obj_t *)view, "ota_success_check",
                                               "/app_ota/check_icon.bin", 103, 382, 36, 36);

    // Create ota_success_status (hg_label)
    ota_success_status = gui_text_create((gui_obj_t *)view, "ota_success_status", 147, 385, 180, 24);
    gui_text_set((gui_text_t *)ota_success_status, "Update Complete", GUI_FONT_SRC_BMP, gui_rgb(0, 229,
                 160), 15, 18);
    gui_text_type_set((gui_text_t *)ota_success_status,
                      "/font/Inter_24pt_Regular_size18_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_success_status, MID_LEFT);

    // Create ota_done_btn (hg_image)
    ota_done_btn = gui_img_create_from_fs((gui_obj_t *)view, "ota_done_btn", "/app_ota/done_btn.bin",
                                          100, 430, 220, 52);
    gui_obj_add_event_cb(ota_done_btn, (gui_event_cb_t)ota_done_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create ota_success_ring (hg_arc)
    ota_success_ring = gui_arc_create((gui_obj_t *)view, "ota_success_ring", 205, 250, 105, -90, -90,
                                      10, gui_rgb(0, 229, 160));

    // Create ota_success_percent (hg_label)
    ota_success_percent = gui_text_create((gui_obj_t *)view, "ota_success_percent", 150, 220, 100, 60);
    gui_text_set((gui_text_t *)ota_success_percent, "100", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3,
                 54);
    gui_text_type_set((gui_text_t *)ota_success_percent,
                      "/font/Inter_24pt_Regular_size54_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_success_percent, MID_CENTER);

    // Create ota_success_pct_sym (hg_label)
    ota_success_pct_sym = gui_text_create((gui_obj_t *)view, "ota_success_pct_sym", 245, 230, 30, 40);
    gui_text_set((gui_text_t *)ota_success_pct_sym, "%", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1,
                 26);
    gui_text_type_set((gui_text_t *)ota_success_pct_sym,
                      "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_success_pct_sym, MID_LEFT);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_otaSuccessView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_otaSuccessView", false, app_otaSuccessView_switch_in,
                  app_otaSuccessView_switch_out, false);

// Create app_otaFailedView (hg_view)
static void app_otaFailedView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_otaFailedView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create ota_failed_back_btn (hg_image)
    ota_failed_back_btn = gui_img_create_from_fs((gui_obj_t *)view, "ota_failed_back_btn",
                                                 "/app_ota/back_icon.bin", 32, 28, 32, 32);
    gui_obj_add_event_cb(ota_failed_back_btn, (gui_event_cb_t)ota_failed_back_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create ota_failed_version (hg_label)
    ota_failed_version = gui_text_create((gui_obj_t *)view, "ota_failed_version", 130, 76, 150, 20);
    gui_text_set((gui_text_t *)ota_failed_version, "Firmware v2.4.1", GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), 15, 14);
    gui_text_type_set((gui_text_t *)ota_failed_version,
                      "/font/Inter_24pt_Regular_size14_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_failed_version, MID_CENTER);

    // Create ota_failed_ring_bg (hg_arc)
    ota_failed_ring_bg = gui_arc_create((gui_obj_t *)view, "ota_failed_ring_bg", 205, 250, 105, 0, 360,
                                        10, gui_rgba(255, 255, 255, 8));

    // Create ota_failed_status (hg_label)
    ota_failed_status = gui_text_create((gui_obj_t *)view, "ota_failed_status", 95, 385, 220, 24);
    gui_text_set((gui_text_t *)ota_failed_status, "Update Failed", GUI_FONT_SRC_BMP, gui_rgb(255, 77,
                 106), 13, 18);
    gui_text_type_set((gui_text_t *)ota_failed_status,
                      "/font/Inter_24pt_Regular_size18_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_failed_status, MID_CENTER);

    // Create ota_retry_btn (hg_image)
    ota_retry_btn = gui_img_create_from_fs((gui_obj_t *)view, "ota_retry_btn", "/app_ota/retry_btn.bin",
                                           100, 430, 220, 52);
    gui_obj_add_event_cb(ota_retry_btn, (gui_event_cb_t)ota_retry_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create ota_failed_ring (hg_arc)
    ota_failed_ring = gui_arc_create((gui_obj_t *)view, "ota_failed_ring", 205, 250, 105, -90, 270, 10,
                                     gui_rgb(255, 77, 106));

    // Create ota_failed_percent (hg_label)
    ota_failed_percent = gui_text_create((gui_obj_t *)view, "ota_failed_percent", 150, 220, 100, 60);
    gui_text_set((gui_text_t *)ota_failed_percent, "100", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 3,
                 54);
    gui_text_type_set((gui_text_t *)ota_failed_percent,
                      "/font/Inter_24pt_Regular_size54_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_failed_percent, MID_CENTER);

    // Create ota_failed_pct_sym (hg_label)
    ota_failed_pct_sym = gui_text_create((gui_obj_t *)view, "ota_failed_pct_sym", 245, 230, 30, 40);
    gui_text_set((gui_text_t *)ota_failed_pct_sym, "%", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 1,
                 26);
    gui_text_type_set((gui_text_t *)ota_failed_pct_sym,
                      "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)ota_failed_pct_sym, MID_LEFT);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_otaFailedView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_otaFailedView", false, app_otaFailedView_switch_in,
                  app_otaFailedView_switch_out, false);
