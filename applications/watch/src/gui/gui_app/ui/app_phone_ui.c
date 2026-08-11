/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_phone UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-14T03:23:12.214Z
 */
#include "app_phone_ui.h"
#include "../callbacks/app_phone_callbacks.h"
#include "../user/app_phone_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_win_t *phone_dialer_window = NULL;
gui_text_t *dialer_time_label = NULL;
gui_win_t *win_10 = NULL;
gui_img_t *dialer_back_btn = NULL;
gui_text_t *number_display_label = NULL;
gui_img_t *dial_key_1 = NULL;
gui_img_t *dial_key_2 = NULL;
gui_img_t *dial_key_3 = NULL;
gui_img_t *dial_key_4 = NULL;
gui_img_t *dial_key_5 = NULL;
gui_img_t *dial_key_6 = NULL;
gui_img_t *dial_key_7 = NULL;
gui_img_t *dial_key_8 = NULL;
gui_img_t *dial_key_9 = NULL;
gui_img_t *dial_key_star = NULL;
gui_img_t *dial_key_0 = NULL;
gui_img_t *dial_key_hash = NULL;
gui_img_t *delete_btn = NULL;
gui_img_t *call_btn = NULL;
gui_img_t *simulate_incoming_btn = NULL;
gui_win_t *phone_calling_window = NULL;
gui_text_t *calling_time_label = NULL;
gui_img_t *avatar_placeholder_img = NULL;
gui_text_t *calling_number_label = NULL;
gui_text_t *call_timer_label = NULL;
gui_img_t *phone_call_mute_btn = NULL;
gui_img_t *hangup_btn = NULL;
gui_img_t *volume_down_btn = NULL;
gui_img_t *volume_icon_img = NULL;
gui_text_t *volume_value_label = NULL;
gui_img_t *volume_up_btn = NULL;
gui_win_t *phone_incoming_window = NULL;
gui_text_t *incoming_time_label = NULL;
gui_text_t *incoming_status_label = NULL;
gui_text_t *incoming_name_label = NULL;
gui_text_t *incoming_number_label = NULL;
gui_img_t *incoming_ring_animation_img = NULL;
gui_img_t *decline_btn = NULL;
gui_img_t *incoming_btn = NULL;

// Time string global variables
char dialer_time_label_time_str[10] = {0};
char calling_time_label_time_str[10] = {0};
char incoming_time_label_time_str[10] = {0};


// Create app_phoneDialerView (hg_view)
static void app_phoneDialerView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_phoneDialerView_switch_in(gui_view_t *view)
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



    // Create phone_dialer_window (hg_window)
    phone_dialer_window = gui_win_create((gui_obj_t *)view, "phone_dialer_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)phone_dialer_window, true);
    gui_win_set_blur_degree((gui_win_t *)phone_dialer_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(dialer_time_label_time_str, sizeof(dialer_time_label_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create dialer_time_label (hg_time_label)
    dialer_time_label = gui_text_create(phone_dialer_window, "dialer_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)dialer_time_label, dialer_time_label_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(242, 242, 242), strlen(dialer_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)dialer_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)dialer_time_label, MID_RIGHT);

    // Create win_10 (hg_window)
    win_10 = gui_win_create(phone_dialer_window, "win_10", 0, 0, 100, 100);


    // Create dialer_back_btn (hg_image)
    dialer_back_btn = gui_img_create_from_fs(win_10, "dialer_back_btn", "/app_phone/back_icon.bin", 32,
                                             28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_10), (gui_event_cb_t)win_10_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(dialer_time_label), 30000, true, dialer_time_label_time_update_cb);

    // Create number_display_label (hg_label)
    number_display_label = gui_text_create((gui_obj_t *)view, "number_display_label", 30, 116, 350, 40);
    gui_text_set((gui_text_t *)number_display_label, "", GUI_FONT_SRC_BMP, gui_rgb(242, 242, 242), 0,
                 30);
    gui_text_type_set((gui_text_t *)number_display_label,
                      "/font/Inter_24pt_Regular_size30_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)number_display_label, MID_CENTER);
    gui_text_extra_letter_spacing_set((gui_text_t *)number_display_label, 2);

    // Create dial_key_1 (hg_image)
    dial_key_1 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_1", "/app_phone/dial_key_1.bin",
                                        48, 162, 100, 60);
    gui_obj_add_event_cb(dial_key_1, (gui_event_cb_t)dial_key_1_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_2 (hg_image)
    dial_key_2 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_2", "/app_phone/dial_key_2.bin",
                                        155, 162, 100, 60);
    gui_obj_add_event_cb(dial_key_2, (gui_event_cb_t)dial_key_2_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_3 (hg_image)
    dial_key_3 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_3", "/app_phone/dial_key_3.bin",
                                        262, 162, 100, 60);
    gui_obj_add_event_cb(dial_key_3, (gui_event_cb_t)dial_key_3_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_4 (hg_image)
    dial_key_4 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_4", "/app_phone/dial_key_4.bin",
                                        48, 228, 100, 60);
    gui_obj_add_event_cb(dial_key_4, (gui_event_cb_t)dial_key_4_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_5 (hg_image)
    dial_key_5 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_5", "/app_phone/dial_key_5.bin",
                                        155, 228, 100, 60);
    gui_obj_add_event_cb(dial_key_5, (gui_event_cb_t)dial_key_5_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_6 (hg_image)
    dial_key_6 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_6", "/app_phone/dial_key_6.bin",
                                        262, 228, 100, 60);
    gui_obj_add_event_cb(dial_key_6, (gui_event_cb_t)dial_key_6_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_7 (hg_image)
    dial_key_7 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_7", "/app_phone/dial_key_7.bin",
                                        48, 294, 100, 60);
    gui_obj_add_event_cb(dial_key_7, (gui_event_cb_t)dial_key_7_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_8 (hg_image)
    dial_key_8 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_8", "/app_phone/dial_key_8.bin",
                                        155, 294, 100, 60);
    gui_obj_add_event_cb(dial_key_8, (gui_event_cb_t)dial_key_8_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_9 (hg_image)
    dial_key_9 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_9", "/app_phone/dial_key_9.bin",
                                        262, 294, 100, 60);
    gui_obj_add_event_cb(dial_key_9, (gui_event_cb_t)dial_key_9_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_star (hg_image)
    dial_key_star = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_star",
                                           "/app_phone/dial_key_star.bin", 48, 360, 100, 60);
    gui_obj_add_event_cb(dial_key_star, (gui_event_cb_t)dial_key_star_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create dial_key_0 (hg_image)
    dial_key_0 = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_0", "/app_phone/dial_key_0.bin",
                                        155, 360, 100, 60);
    gui_obj_add_event_cb(dial_key_0, (gui_event_cb_t)dial_key_0_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create dial_key_hash (hg_image)
    dial_key_hash = gui_img_create_from_fs((gui_obj_t *)view, "dial_key_hash",
                                           "/app_phone/dial_key_hash.bin", 262, 360, 100, 60);
    gui_obj_add_event_cb(dial_key_hash, (gui_event_cb_t)dial_key_hash_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create delete_btn (hg_image)
    delete_btn = gui_img_create_from_fs((gui_obj_t *)view, "delete_btn", "/app_phone/delete_btn.bin",
                                        56, 434, 48, 48);
    gui_obj_add_event_cb(delete_btn, (gui_event_cb_t)delete_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create call_btn (hg_image)
    call_btn = gui_img_create_from_fs((gui_obj_t *)view, "call_btn", "/app_phone/call_btn.bin", 175,
                                      428, 60, 60);
    gui_obj_add_event_cb(call_btn, (gui_event_cb_t)call_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create simulate_incoming_btn (hg_image)
    simulate_incoming_btn = gui_img_create_from_fs((gui_obj_t *)view, "simulate_incoming_btn",
                                                   "/app_phone/simulate_incoming_btn.bin", 306, 434, 48, 48);
    gui_obj_add_event_cb(simulate_incoming_btn, (gui_event_cb_t)simulate_incoming_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)app_phoneDialerView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);
}
GUI_VIEW_INSTANCE("app_phoneDialerView", false, app_phoneDialerView_switch_in,
                  app_phoneDialerView_switch_out, false);

// Create app_phoneCallingView (hg_view)
static void app_phoneCallingView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_phoneCallingView_switch_in(gui_view_t *view)
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



    // Create phone_calling_window (hg_window)
    phone_calling_window = gui_win_create((gui_obj_t *)view, "phone_calling_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)phone_calling_window, true);
    gui_win_set_blur_degree((gui_win_t *)phone_calling_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(calling_time_label_time_str, sizeof(calling_time_label_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create calling_time_label (hg_time_label)
    calling_time_label = gui_text_create(phone_calling_window, "calling_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)calling_time_label, calling_time_label_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(242, 242, 242), strlen(calling_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)calling_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)calling_time_label, MID_RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(calling_time_label), 30000, true, calling_time_label_time_update_cb);

    // Create avatar_placeholder_img (hg_image)
    avatar_placeholder_img = gui_img_create_from_fs((gui_obj_t *)view, "avatar_placeholder_img",
                                                    "/app_phone/avatar_placeholder.bin", 145, 138, 120, 120);

    // Create calling_number_label (hg_label)
    calling_number_label = gui_text_create((gui_obj_t *)view, "calling_number_label", 48, 273, 314, 50);
    gui_text_set((gui_text_t *)calling_number_label, "Unknown", GUI_FONT_SRC_BMP, gui_rgb(242, 242,
                 242), 7, 36);
    gui_text_type_set((gui_text_t *)calling_number_label,
                      "/font/NotoSansSC_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)calling_number_label, MID_CENTER);
    // Bind timer: Animation 1
    gui_obj_create_timer((gui_obj_t *)calling_number_label, 100, false,
                         calling_number_label_timer_0_cb);
    gui_msg_subscribe((gui_obj_t *)calling_number_label, "phone/caller_id",
                      calling_number_label_msg_cb_0);
    gui_msg_subscribe((gui_obj_t *)calling_number_label, "phone/number", calling_number_label_msg_cb_1);

    // Create call_timer_label (hg_label)
    call_timer_label = gui_text_create((gui_obj_t *)view, "call_timer_label", 48, 326, 314, 42);
    gui_text_set((gui_text_t *)call_timer_label, "00:00", GUI_FONT_SRC_BMP, gui_rgb(102, 102, 102), 5,
                 32);
    gui_text_type_set((gui_text_t *)call_timer_label,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)call_timer_label, MID_CENTER);
    // Bind timer: call_timer
    gui_obj_create_timer((gui_obj_t *)call_timer_label, 1000, true, call_timer_tick);
    gui_obj_start_timer((gui_obj_t *)call_timer_label);

    // Create phone_call_mute_btn (hg_image)
    phone_call_mute_btn = gui_img_create_from_fs((gui_obj_t *)view, "phone_call_mute_btn",
                                                 "/app_phone/mute_btn_normal.bin", 135, 366, 56, 56);
    gui_obj_add_event_cb(phone_call_mute_btn, (gui_event_cb_t)phone_call_mute_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create hangup_btn (hg_image)
    hangup_btn = gui_img_create_from_fs((gui_obj_t *)view, "hangup_btn", "/app_phone/hangup_btn.bin",
                                        219, 366, 56, 56);
    gui_obj_add_event_cb(hangup_btn, (gui_event_cb_t)hangup_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create volume_down_btn (hg_image)
    volume_down_btn = gui_img_create_from_fs((gui_obj_t *)view, "volume_down_btn",
                                             "/app_phone/volume_down_btn.bin", 128, 450, 36, 36);
    gui_obj_add_event_cb(volume_down_btn, (gui_event_cb_t)volume_down_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create volume_icon_img (hg_image)
    volume_icon_img = gui_img_create_from_fs((gui_obj_t *)view, "volume_icon_img",
                                             "/app_phone/volume_icon.bin", 180, 456, 24, 24);
    gui_img_scale((gui_img_t *)volume_icon_img, 0.500000f, 0.500000f);

    // Create volume_value_label (hg_label)
    volume_value_label = gui_text_create((gui_obj_t *)view, "volume_value_label", 205, 448, 48, 40);
    gui_text_set((gui_text_t *)volume_value_label, "5", GUI_FONT_SRC_BMP, gui_rgb(102, 102, 102), 1,
                 28);
    gui_text_type_set((gui_text_t *)volume_value_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)volume_value_label, MID_CENTER);

    // Create volume_up_btn (hg_image)
    volume_up_btn = gui_img_create_from_fs((gui_obj_t *)view, "volume_up_btn",
                                           "/app_phone/volume_up_btn.bin", 252, 450, 36, 36);
    gui_obj_add_event_cb(volume_up_btn, (gui_event_cb_t)volume_up_btn_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);
}
GUI_VIEW_INSTANCE("app_phoneCallingView", false, app_phoneCallingView_switch_in,
                  app_phoneCallingView_switch_out, false);

// Create app_phoneIncomingView (hg_view)
static void app_phoneIncomingView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_phoneIncomingView_switch_in(gui_view_t *view)
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



    // Create phone_incoming_window (hg_window)
    phone_incoming_window = gui_win_create((gui_obj_t *)view, "phone_incoming_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)phone_incoming_window, true);
    gui_win_set_blur_degree((gui_win_t *)phone_incoming_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(incoming_time_label_time_str, sizeof(incoming_time_label_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create incoming_time_label (hg_time_label)
    incoming_time_label = gui_text_create(phone_incoming_window, "incoming_time_label", 305, 20, 80,
                                          32);
    gui_text_set((gui_text_t *)incoming_time_label, incoming_time_label_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(242, 242, 242), strlen(incoming_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)incoming_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)incoming_time_label, MID_RIGHT);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(incoming_time_label), 30000, true,
                         incoming_time_label_time_update_cb);

    // Create incoming_status_label (hg_label)
    incoming_status_label = gui_text_create((gui_obj_t *)view, "incoming_status_label", 48, 119, 314,
                                            42);
    gui_text_set((gui_text_t *)incoming_status_label, "Incoming Call", GUI_FONT_SRC_BMP, gui_rgb(115,
                 115, 115), 13, 28);
    gui_text_type_set((gui_text_t *)incoming_status_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)incoming_status_label, MID_CENTER);

    // Create incoming_name_label (hg_label)
    incoming_name_label = gui_text_create((gui_obj_t *)view, "incoming_name_label", 48, 156, 314, 50);
    gui_text_set((gui_text_t *)incoming_name_label, "John Doe", GUI_FONT_SRC_BMP, gui_rgb(242, 242,
                 242), 8, 36);
    gui_text_type_set((gui_text_t *)incoming_name_label,
                      "/font/NotoSansSC_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)incoming_name_label, MID_CENTER);
    gui_msg_subscribe((gui_obj_t *)incoming_name_label, "phone/caller_id",
                      incoming_name_label_msg_cb_0);

    // Create incoming_number_label (hg_label)
    incoming_number_label = gui_text_create((gui_obj_t *)view, "incoming_number_label", 48, 198, 314,
                                            50);
    gui_text_set((gui_text_t *)incoming_number_label, "+1 (555) 123-4567", GUI_FONT_SRC_BMP, gui_rgb(77,
                 77, 77), 17, 32);
    gui_text_type_set((gui_text_t *)incoming_number_label,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)incoming_number_label, MID_CENTER);
    gui_msg_subscribe((gui_obj_t *)incoming_number_label, "phone/number",
                      incoming_number_label_msg_cb_0);

    // Create incoming_ring_animation_img (hg_image)
    incoming_ring_animation_img = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "incoming_ring_animation_img", "/app_phone/incoming_ring_animation/frame_00.bin", 157, 250, 96, 96);
    // Bind timer: incoming_ring
    gui_obj_create_timer((gui_obj_t *)incoming_ring_animation_img, 33, true, incoming_ring_timer_cb);
    gui_obj_start_timer((gui_obj_t *)incoming_ring_animation_img);

    // Create decline_btn (hg_image)
    decline_btn = gui_img_create_from_fs((gui_obj_t *)view, "decline_btn", "/app_phone/decline_btn.bin",
                                         117, 390, 64, 64);
    gui_obj_add_event_cb(decline_btn, (gui_event_cb_t)decline_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create incoming_btn (hg_image)
    incoming_btn = gui_img_create_from_fs((gui_obj_t *)view, "incoming_btn",
                                          "/app_phone/incoming_btn.bin", 229, 390, 64, 64);
    gui_obj_add_event_cb(incoming_btn, (gui_event_cb_t)incoming_btn_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);
}
GUI_VIEW_INSTANCE("app_phoneIncomingView", false, app_phoneIncomingView_switch_in,
                  app_phoneIncomingView_switch_out, false);
