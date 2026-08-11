/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_intercom UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-14T03:23:12.149Z
 */
#include "app_intercom_ui.h"
#include "../callbacks/app_intercom_callbacks.h"
#include "../user/app_intercom_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_text_t *available_devices_label = NULL;
gui_list_t *toggle_list = NULL;
gui_rounded_rect_t *device1_item_bg = NULL;
gui_text_t *device1_name_label = NULL;
gui_img_t *device1_status_dot = NULL;
gui_rounded_rect_t *device2_item_bg = NULL;
gui_text_t *device2_name_label = NULL;
gui_img_t *device2_status_dot = NULL;
gui_rounded_rect_t *device3_item_bg = NULL;
gui_text_t *device3_name_label = NULL;
gui_img_t *device3_status_dot = NULL;
gui_win_t *toggle_window = NULL;
gui_win_t *intercom_main_back_win = NULL;
gui_text_t *toggle_time_label = NULL;
gui_text_t *toggle_page_title = NULL;
gui_img_t *toggle_back_btn = NULL;
gui_obj_t *intercom_toggle_btn = NULL;
gui_win_t *talk_window = NULL;
gui_win_t *intercom_talk_back_win = NULL;
gui_text_t *talk_time_label = NULL;
gui_img_t *connection_dot = NULL;
gui_text_t *connection_label = NULL;
gui_text_t *intercom_device_name_label = NULL;
gui_img_t *waveform_image = NULL;
gui_text_t *status_text_label = NULL;
gui_img_t *talk_btn = NULL;
gui_obj_t *mute_btn = NULL;
gui_img_t *talk_back_btn = NULL;

// Time string global variables
char toggle_time_label_time_str[10] = {0};
char talk_time_label_time_str[10] = {0};

// Toggle button callback functions

// intercom_toggle_btn dual-state button callback
static bool intercom_toggle_btn_state = false;

void intercom_toggle_btn_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    intercom_toggle_btn_state = !intercom_toggle_btn_state;

    // Switch image based on state and call corresponding callback
    if (intercom_toggle_btn_state)
    {
        gui_img_set_src((gui_img_t *)intercom_toggle_btn,
                        (const uint8_t *)"/app_intercom/toggle_btn_on.bin", IMG_SRC_FILESYS);
        extern void intercom_toggle_on(void *obj, gui_event_t *e);
        intercom_toggle_on(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)intercom_toggle_btn,
                        (const uint8_t *)"/app_intercom/toggle_btn_off.bin", IMG_SRC_FILESYS);
        extern void intercom_toggle_on(void *obj, gui_event_t *e);
        intercom_toggle_off(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool intercom_toggle_btn_get_state(void)
{
    return intercom_toggle_btn_state;
}

// Set state (external call)
void intercom_toggle_btn_set_state(bool state)
{
    if (intercom_toggle_btn_state != state)
    {
        intercom_toggle_btn_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)intercom_toggle_btn,
                            (const uint8_t *)"/app_intercom/toggle_btn_on.bin", IMG_SRC_FILESYS);
            extern void intercom_toggle_on(void *obj, gui_event_t *e);
            intercom_toggle_on(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)intercom_toggle_btn,
                            (const uint8_t *)"/app_intercom/toggle_btn_off.bin", IMG_SRC_FILESYS);
            extern void intercom_toggle_on(void *obj, gui_event_t *e);
            intercom_toggle_off(NULL, NULL);
        }
    }
}

// mute_btn dual-state button callback
static bool mute_btn_state = false;

void mute_btn_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    mute_btn_state = !mute_btn_state;

    // Switch image based on state and call corresponding callback
    if (mute_btn_state)
    {
        gui_img_set_src((gui_img_t *)mute_btn, (const uint8_t *)"/app_intercom/mute_btn_active.bin",
                        IMG_SRC_FILESYS);
        extern void mute_btn_on(void *obj, gui_event_t *e);
        mute_btn_on(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)mute_btn, (const uint8_t *)"/app_intercom/mute_btn_normal.bin",
                        IMG_SRC_FILESYS);
        extern void mute_btn_on(void *obj, gui_event_t *e);
        mute_btn_off(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool mute_btn_get_state(void)
{
    return mute_btn_state;
}

// Set state (external call)
void mute_btn_set_state(bool state)
{
    if (mute_btn_state != state)
    {
        mute_btn_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)mute_btn, (const uint8_t *)"/app_intercom/mute_btn_active.bin",
                            IMG_SRC_FILESYS);
            extern void mute_btn_on(void *obj, gui_event_t *e);
            mute_btn_on(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)mute_btn, (const uint8_t *)"/app_intercom/mute_btn_normal.bin",
                            IMG_SRC_FILESYS);
            extern void mute_btn_on(void *obj, gui_event_t *e);
            mute_btn_off(NULL, NULL);
        }
    }
}
// List component note_design callback functions

// Create app_intercomMainView (hg_view)
static void app_intercomMainView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_intercomMainView_switch_in(gui_view_t *view)
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



    // Create available_devices_label (hg_label)
    available_devices_label = gui_text_create((gui_obj_t *)view, "available_devices_label", 30, 161,
                                              200, 30);
    gui_text_set((gui_text_t *)available_devices_label, "Available Devices", GUI_FONT_SRC_BMP,
                 gui_rgb(242, 242, 242), 17, 24);
    gui_text_type_set((gui_text_t *)available_devices_label,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)available_devices_label, MID_LEFT);
    gui_obj_hidden((gui_obj_t *)available_devices_label, true);

    // Create toggle_list (hg_list)
    toggle_list = gui_list_create((gui_obj_t *)view, "toggle_list", -2, 192, 410, 300, 84, 5, VERTICAL,
                                  walkie_talkie_list_note_design, NULL, false);
    gui_list_set_style(toggle_list, LIST_CLASSIC);
    gui_list_set_note_num(toggle_list, 3);
    gui_list_set_out_scope(toggle_list, 80);
    gui_list_keep_note_alive(toggle_list, true);
    gui_obj_hidden((gui_obj_t *)toggle_list, true);
    gui_msg_subscribe((gui_obj_t *)toggle_list, "walkie_talkie_scan_report", toggle_list_msg_cb_0);

    // Create toggle_window (hg_window)
    toggle_window = gui_win_create((gui_obj_t *)view, "toggle_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)toggle_window, true);
    gui_win_set_blur_degree((gui_win_t *)toggle_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(toggle_time_label_time_str, sizeof(toggle_time_label_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create intercom_main_back_win (hg_window)
    intercom_main_back_win = gui_win_create(toggle_window, "intercom_main_back_win", 0, 0, 100, 100);

    gui_obj_add_event_cb(GUI_BASE(intercom_main_back_win),
                         (gui_event_cb_t)intercom_main_back_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create toggle_time_label (hg_time_label)
    toggle_time_label = gui_text_create(toggle_window, "toggle_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)toggle_time_label, toggle_time_label_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(toggle_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)toggle_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)toggle_time_label, RIGHT);

    // Create toggle_page_title (hg_label)
    toggle_page_title = gui_text_create(toggle_window, "toggle_page_title", 256, 57, 140, 28);
    gui_text_set((gui_text_t *)toggle_page_title, "Walkie-Talkie", GUI_FONT_SRC_BMP, gui_rgb(66, 211,
                 13), 13, 22);
    gui_text_type_set((gui_text_t *)toggle_page_title,
                      "/font/Inter_24pt_Regular_size22_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)toggle_page_title, CENTER);

    // Create toggle_back_btn (hg_image)
    toggle_back_btn = gui_img_create_from_fs(toggle_window, "toggle_back_btn",
                                             "/app_intercom/back_icon.bin", 32, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(toggle_window), (gui_event_cb_t)toggle_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)toggle_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(toggle_time_label), 30000, true, toggle_time_label_time_update_cb);

    // Create intercom_toggle_btn (hg_button)
    intercom_toggle_btn = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)view, "intercom_toggle_btn",
                                                              "/app_intercom/toggle_btn_off.bin", 105, 91, 200, 56);
    if (intercom_toggle_btn_state)
    {
        gui_img_set_src((gui_img_t *)intercom_toggle_btn,
                        (const uint8_t *)"/app_intercom/toggle_btn_on.bin", IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)intercom_toggle_btn, intercom_toggle_btn_toggle_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    gui_msg_subscribe((gui_obj_t *)view, "walkie_talkie_conn", app_intercomMainView_msg_cb_0);
}
GUI_VIEW_INSTANCE("app_intercomMainView", false, app_intercomMainView_switch_in,
                  app_intercomMainView_switch_out, false);

// Create app_intercomTalkView (hg_view)
static void app_intercomTalkView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_intercomTalkView_switch_in(gui_view_t *view)
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



    // Create talk_window (hg_window)
    talk_window = gui_win_create((gui_obj_t *)view, "talk_window", 0, 0, 410, 502);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(talk_time_label_time_str, sizeof(talk_time_label_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create intercom_talk_back_win (hg_window)
    intercom_talk_back_win = gui_win_create(talk_window, "intercom_talk_back_win", 0, 0, 100, 100);

    gui_obj_add_event_cb(GUI_BASE(intercom_talk_back_win),
                         (gui_event_cb_t)intercom_talk_back_win_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create talk_time_label (hg_time_label)
    talk_time_label = gui_text_create(talk_window, "talk_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)talk_time_label, talk_time_label_time_str, GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), strlen(talk_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)talk_time_label, "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)talk_time_label, RIGHT);

    // Create connection_dot (hg_image)
    connection_dot = gui_img_create_from_fs(talk_window, "connection_dot",
                                            "/app_intercom/status_dot.bin", 145, 115, 8, 8);

    // Create connection_label (hg_label)
    connection_label = gui_text_create(talk_window, "connection_label", 159, 104, 100, 30);
    gui_text_set((gui_text_t *)connection_label, "Connected", GUI_FONT_SRC_BMP, gui_rgb(128, 128, 128),
                 9, 20);
    gui_text_type_set((gui_text_t *)connection_label,
                      "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)connection_label, MID_LEFT);
    gui_msg_subscribe((gui_obj_t *)connection_label, "walkie_talkie_conn", connection_label_msg_cb_0);
    gui_msg_subscribe((gui_obj_t *)connection_label, "walkie_talkie_disconn",
                      connection_label_msg_cb_1);

    // Create intercom_device_name_label (hg_label)
    intercom_device_name_label = gui_text_create(talk_window, "intercom_device_name_label", 59, 140,
                                                 300, 50);
    gui_text_set((gui_text_t *)intercom_device_name_label, "Alex's Watch", GUI_FONT_SRC_BMP,
                 gui_rgb(242, 242, 242), 12, 40);
    gui_text_type_set((gui_text_t *)intercom_device_name_label,
                      "/font/Inter_24pt_Regular_size40_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)intercom_device_name_label, CENTER);
    gui_msg_subscribe((gui_obj_t *)intercom_device_name_label, "walkie_talkie_user_name",
                      intercom_device_name_label_msg_cb_0);

    // Create waveform_image (hg_image)
    waveform_image = gui_img_create_from_fs(talk_window, "waveform_image",
                                            "/app_intercom/waveform/idle/idle_frame_00.bin", 165, 192, 81, 40);

    // Create status_text_label (hg_label)
    status_text_label = gui_text_create(talk_window, "status_text_label", 134, 232, 150, 30);
    gui_text_set((gui_text_t *)status_text_label, "Hold to Talk", GUI_FONT_SRC_BMP, gui_rgb(102, 102,
                 102), 12, 20);
    gui_text_type_set((gui_text_t *)status_text_label,
                      "/font/Inter_24pt_Regular_size20_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)status_text_label, CENTER);

    // Create talk_btn (hg_image)
    talk_btn = gui_img_create_from_fs(talk_window, "talk_btn", "/app_intercom/talk_btn_normal.bin", 149,
                                      261, 120, 120);
    gui_obj_add_event_cb(talk_btn, (gui_event_cb_t)talk_btn_pressed_cb, GUI_EVENT_TOUCH_PRESSED, NULL);
    gui_obj_add_event_cb(talk_btn, (gui_event_cb_t)talk_btn_released_cb, GUI_EVENT_TOUCH_RELEASED,
                         NULL);
    gui_msg_subscribe((gui_obj_t *)talk_btn, "walkie_talkie_receive_start", talk_btn_msg_cb_0);
    gui_msg_subscribe((gui_obj_t *)talk_btn, "walkie_talkie_receive_stop", talk_btn_msg_cb_1);

    // Create mute_btn (hg_button)
    mute_btn = (gui_obj_t *)gui_img_create_from_fs(talk_window, "mute_btn",
                                                   "/app_intercom/mute_btn_normal.bin", 183, 396, 52, 52);
    if (mute_btn_state)
    {
        gui_img_set_src((gui_img_t *)mute_btn, (const uint8_t *)"/app_intercom/mute_btn_active.bin",
                        IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)mute_btn, mute_btn_toggle_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create talk_back_btn (hg_image)
    talk_back_btn = gui_img_create_from_fs(talk_window, "talk_back_btn", "/app_intercom/back_icon.bin",
                                           32, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(talk_window), (gui_event_cb_t)talk_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)talk_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(talk_time_label), 30000, true, talk_time_label_time_update_cb);
}
GUI_VIEW_INSTANCE("app_intercomTalkView", false, app_intercomTalkView_switch_in,
                  app_intercomTalkView_switch_out, false);
