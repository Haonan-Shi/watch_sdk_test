/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_video_call UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-14T03:23:12.269Z
 */
#include "app_video_call_ui.h"
#include "../callbacks/app_video_call_callbacks.h"
#include "../user/app_video_call_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_img_t *video_call_idle_avatar = NULL;
gui_text_t *video_call_idle_name = NULL;
gui_text_t *video_call_idle_subtitle = NULL;
gui_img_t *video_call_btn = NULL;
gui_win_t *video_call_idle_window = NULL;
gui_img_t *video_call_idle_back_icon = NULL;
gui_text_t *video_call_idle_time_label = NULL;
gui_win_t *win_video_call_idle_back = NULL;
gui_img_t *video_call_ring_pulse_img = NULL;
gui_img_t *video_call_calling_avatar = NULL;
gui_text_t *video_call_calling_name = NULL;
gui_text_t *video_call_calling_status = NULL;
gui_obj_t *mic_btn = NULL;
gui_img_t *video_call_hangup_btn = NULL;
gui_obj_t *speaker_btn = NULL;
gui_win_t *video_call_calling_window = NULL;
gui_img_t *video_call_calling_back_icon = NULL;
gui_text_t *video_call_calling_time_label = NULL;
gui_win_t *win_video_calling_back = NULL;

// Time string global variables
char video_call_idle_time_label_time_str[10] = {0};
char video_call_calling_time_label_time_str[10] = {0};

// Toggle button callback functions

// mic_btn dual-state button callback
static bool mic_btn_state = false;

void mic_btn_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    mic_btn_state = !mic_btn_state;

    // Switch image based on state and call corresponding callback
    if (mic_btn_state)
    {
        gui_img_set_src((gui_img_t *)mic_btn, (const uint8_t *)"/app_video_call/mic_btn_active.bin",
                        IMG_SRC_FILESYS);
        extern void mic_toggle(void *obj, gui_event_t *e);
        mic_toggle(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)mic_btn, (const uint8_t *)"/app_video_call/mic_btn_normal.bin",
                        IMG_SRC_FILESYS);
        extern void mic_toggle(void *obj, gui_event_t *e);
        mic_toggle(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool mic_btn_get_state(void)
{
    return mic_btn_state;
}

// Set state (external call)
void mic_btn_set_state(bool state)
{
    if (mic_btn_state != state)
    {
        mic_btn_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)mic_btn, (const uint8_t *)"/app_video_call/mic_btn_active.bin",
                            IMG_SRC_FILESYS);
            extern void mic_toggle(void *obj, gui_event_t *e);
            mic_toggle(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)mic_btn, (const uint8_t *)"/app_video_call/mic_btn_normal.bin",
                            IMG_SRC_FILESYS);
            extern void mic_toggle(void *obj, gui_event_t *e);
            mic_toggle(NULL, NULL);
        }
    }
}

// speaker_btn dual-state button callback
static bool speaker_btn_state = false;

void speaker_btn_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    speaker_btn_state = !speaker_btn_state;

    // Switch image based on state and call corresponding callback
    if (speaker_btn_state)
    {
        gui_img_set_src((gui_img_t *)speaker_btn, (const uint8_t *)"/app_video_call/speaker_btn_active.bin",
                        IMG_SRC_FILESYS);
        extern void speaker_toggle(void *obj, gui_event_t *e);
        speaker_toggle(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)speaker_btn, (const uint8_t *)"/app_video_call/speaker_btn_normal.bin",
                        IMG_SRC_FILESYS);
        extern void speaker_toggle(void *obj, gui_event_t *e);
        speaker_toggle(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool speaker_btn_get_state(void)
{
    return speaker_btn_state;
}

// Set state (external call)
void speaker_btn_set_state(bool state)
{
    if (speaker_btn_state != state)
    {
        speaker_btn_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)speaker_btn, (const uint8_t *)"/app_video_call/speaker_btn_active.bin",
                            IMG_SRC_FILESYS);
            extern void speaker_toggle(void *obj, gui_event_t *e);
            speaker_toggle(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)speaker_btn, (const uint8_t *)"/app_video_call/speaker_btn_normal.bin",
                            IMG_SRC_FILESYS);
            extern void speaker_toggle(void *obj, gui_event_t *e);
            speaker_toggle(NULL, NULL);
        }
    }
}

// Create app_video_callIdleView (hg_view)
static void app_video_callIdleView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_video_callIdleView_switch_in(gui_view_t *view)
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



    // Create video_call_idle_avatar (hg_image)
    video_call_idle_avatar = gui_img_create_from_fs((gui_obj_t *)view, "video_call_idle_avatar",
                                                    "/app_video_call/avatar_large.bin", 125, 123, 160, 160);

    // Create video_call_idle_name (hg_label)
    video_call_idle_name = gui_text_create((gui_obj_t *)view, "video_call_idle_name", 0, 295, 410, 50);
    gui_text_set((gui_text_t *)video_call_idle_name, "Alex Johnson", GUI_FONT_SRC_BMP, gui_rgb(224, 224,
                 224), 12, 36);
    gui_text_type_set((gui_text_t *)video_call_idle_name,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)video_call_idle_name, MID_CENTER);

    // Create video_call_idle_subtitle (hg_label)
    video_call_idle_subtitle = gui_text_create((gui_obj_t *)view, "video_call_idle_subtitle", 0, 339,
                                               410, 34);
    gui_text_set((gui_text_t *)video_call_idle_subtitle, "Video Call", GUI_FONT_SRC_BMP, gui_rgb(100,
                 210, 255), 10, 24);
    gui_text_type_set((gui_text_t *)video_call_idle_subtitle,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)video_call_idle_subtitle, MID_CENTER);

    // Create video_call_btn (hg_image)
    video_call_btn = gui_img_create_from_fs((gui_obj_t *)view, "video_call_btn",
                                            "/app_video_call/video_call_btn.bin", 173, 388, 64, 64);
    gui_obj_add_event_cb(video_call_btn, (gui_event_cb_t)video_call_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create video_call_idle_window (hg_window)
    video_call_idle_window = gui_win_create((gui_obj_t *)view, "video_call_idle_window", 0, 0, 410, 80);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(video_call_idle_time_label_time_str, sizeof(video_call_idle_time_label_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create video_call_idle_back_icon (hg_image)
    video_call_idle_back_icon = gui_img_create_from_fs(video_call_idle_window,
                                                       "video_call_idle_back_icon", "/app_video_call/back_icon.bin", 32, 28, 32, 32);
    gui_obj_add_event_cb(video_call_idle_back_icon,
                         (gui_event_cb_t)video_call_idle_back_icon_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create video_call_idle_time_label (hg_time_label)
    video_call_idle_time_label = gui_text_create(video_call_idle_window, "video_call_idle_time_label",
                                                 300, 20, 80, 32);
    gui_text_set((gui_text_t *)video_call_idle_time_label, video_call_idle_time_label_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(video_call_idle_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)video_call_idle_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)video_call_idle_time_label, RIGHT);

    // Create win_video_call_idle_back (hg_window)
    win_video_call_idle_back = gui_win_create(video_call_idle_window, "win_video_call_idle_back", 0, 0,
                                              100, 100);

    gui_obj_add_event_cb(GUI_BASE(win_video_call_idle_back),
                         (gui_event_cb_t)win_video_call_idle_back_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(video_call_idle_time_label), 30000, true,
                         video_call_idle_time_label_time_update_cb);
}
GUI_VIEW_INSTANCE("app_video_callIdleView", false, app_video_callIdleView_switch_in,
                  app_video_callIdleView_switch_out, false);

// Create app_video_callCallingView (hg_view)
static void app_video_callCallingView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_video_callCallingView_switch_in(gui_view_t *view)
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



    // Create video_call_ring_pulse_img (hg_image)
    video_call_ring_pulse_img = gui_img_create_from_fs((gui_obj_t *)view, "video_call_ring_pulse_img",
                                                       "/app_video_call/ring_pulse/frame_00.bin", 105, 103, 200, 200);
    // Bind timer: ring_pulse
    gui_obj_create_timer((gui_obj_t *)video_call_ring_pulse_img, 33, true, ring_pulse_timer_cb);
    gui_obj_start_timer((gui_obj_t *)video_call_ring_pulse_img);

    // Create video_call_calling_avatar (hg_image)
    video_call_calling_avatar = gui_img_create_from_fs((gui_obj_t *)view, "video_call_calling_avatar",
                                                       "/app_video_call/avatar_small.bin", 145, 143, 120, 120);
    gui_msg_subscribe((gui_obj_t *)video_call_calling_avatar, "video_update",
                      video_call_calling_avatar_msg_cb_0);

    // Create video_call_calling_name (hg_label)
    video_call_calling_name = gui_text_create((gui_obj_t *)view, "video_call_calling_name", 0, 295, 410,
                                              50);
    gui_text_set((gui_text_t *)video_call_calling_name, "Alex Johnson", GUI_FONT_SRC_BMP, gui_rgb(224,
                 224, 224), 12, 36);
    gui_text_type_set((gui_text_t *)video_call_calling_name,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)video_call_calling_name, MID_CENTER);

    // Create video_call_calling_status (hg_label)
    video_call_calling_status = gui_text_create((gui_obj_t *)view, "video_call_calling_status", 0, 339,
                                                410, 34);
    gui_text_set((gui_text_t *)video_call_calling_status, "Calling...", GUI_FONT_SRC_BMP, gui_rgb(48,
                 209, 88), 10, 24);
    gui_text_type_set((gui_text_t *)video_call_calling_status,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)video_call_calling_status, MID_CENTER);

    // Create mic_btn (hg_button)
    mic_btn = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)view, "mic_btn",
                                                  "/app_video_call/mic_btn_normal.bin", 76, 388, 64, 64);
    if (mic_btn_state)
    {
        gui_img_set_src((gui_img_t *)mic_btn, (const uint8_t *)"/app_video_call/mic_btn_active.bin",
                        IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)mic_btn, mic_btn_toggle_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create video_call_hangup_btn (hg_image)
    video_call_hangup_btn = gui_img_create_from_fs((gui_obj_t *)view, "video_call_hangup_btn",
                                                   "/app_video_call/hangup_btn.bin", 173, 388, 64, 64);
    gui_obj_add_event_cb(video_call_hangup_btn, (gui_event_cb_t)video_call_hangup_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create speaker_btn (hg_button)
    speaker_btn = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)view, "speaker_btn",
                                                      "/app_video_call/speaker_btn_normal.bin", 266, 388, 64, 64);
    if (speaker_btn_state)
    {
        gui_img_set_src((gui_img_t *)speaker_btn, (const uint8_t *)"/app_video_call/speaker_btn_active.bin",
                        IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)speaker_btn, speaker_btn_toggle_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create video_call_calling_window (hg_window)
    video_call_calling_window = gui_win_create((gui_obj_t *)view, "video_call_calling_window", 0, 0,
                                               410, 80);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(video_call_calling_time_label_time_str, sizeof(video_call_calling_time_label_time_str),
                 "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create video_call_calling_back_icon (hg_image)
    video_call_calling_back_icon = gui_img_create_from_fs(video_call_calling_window,
                                                          "video_call_calling_back_icon", "/app_video_call/back_icon.bin", 32, 28, 32, 32);
    gui_obj_add_event_cb(video_call_calling_back_icon,
                         (gui_event_cb_t)video_call_calling_back_icon_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create video_call_calling_time_label (hg_time_label)
    video_call_calling_time_label = gui_text_create(video_call_calling_window,
                                                    "video_call_calling_time_label", 300, 20, 80, 32);
    gui_text_set((gui_text_t *)video_call_calling_time_label, video_call_calling_time_label_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(video_call_calling_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)video_call_calling_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)video_call_calling_time_label, RIGHT);

    // Create win_video_calling_back (hg_window)
    win_video_calling_back = gui_win_create(video_call_calling_window, "win_video_calling_back", 0, 0,
                                            100, 100);

    gui_obj_add_event_cb(GUI_BASE(win_video_calling_back),
                         (gui_event_cb_t)win_video_calling_back_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(video_call_calling_time_label), 30000, true,
                         video_call_calling_time_label_time_update_cb);
}
GUI_VIEW_INSTANCE("app_video_callCallingView", false, app_video_callCallingView_switch_in,
                  app_video_callCallingView_switch_out, false);
