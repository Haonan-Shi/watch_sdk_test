/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_control_center UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-14T03:23:12.107Z
 */
#include "app_control_center_ui.h"
#include "../callbacks/app_control_center_callbacks.h"
#include "../user/app_control_center_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *main_list = NULL;
gui_rounded_rect_t *bluetooth_item_bg = NULL;
gui_img_t *bluetooth_icon = NULL;
gui_text_t *bluetooth_label = NULL;
gui_rounded_rect_t *wifi_item_bg = NULL;
gui_img_t *wifi_icon = NULL;
gui_text_t *wifi_label = NULL;
gui_rounded_rect_t *settings_item_bg = NULL;
gui_img_t *settings_icon = NULL;
gui_text_t *settings_label = NULL;
gui_win_t *main_window = NULL;
gui_text_t *main_time_label = NULL;
gui_text_t *main_title_label = NULL;
gui_win_t *win_1 = NULL;
gui_img_t *main_back_btn = NULL;
gui_rounded_rect_t *bt_toggle_bg = NULL;
gui_text_t *bt_toggle_label = NULL;
gui_obj_t *bt_toggle_btn = NULL;
gui_rounded_rect_t *bt_headphones_entry_bg = NULL;
gui_img_t *headphones_entry_icon = NULL;
gui_text_t *headphones_entry_label = NULL;
gui_text_t *phone_section_label = NULL;
gui_list_t *bt_list = NULL;
gui_rounded_rect_t *phone_item_bg = NULL;
gui_text_t *phone_name_label = NULL;
gui_text_t *phone_status_label = NULL;
gui_img_t *phone_icon = NULL;
gui_win_t *bt_window = NULL;
gui_win_t *win_2 = NULL;
gui_img_t *img_3 = NULL;
gui_text_t *bt_time_label = NULL;
gui_text_t *bt_title_label = NULL;
gui_rounded_rect_t *search_item_bg = NULL;
gui_img_t *search_icon = NULL;
gui_text_t *search_label = NULL;
gui_text_t *headphones_section_label = NULL;
gui_list_t *headphone_list = NULL;
gui_rounded_rect_t *headphones_item1_bg = NULL;
gui_text_t *headphones1_name_label = NULL;
gui_text_t *headphones1_status_label = NULL;
gui_img_t *headphones1_icon = NULL;
gui_rounded_rect_t *headphones_item2_bg = NULL;
gui_text_t *headphones2_name_label = NULL;
gui_text_t *headphones2_status_label = NULL;
gui_img_t *headphones2_icon = NULL;
gui_rounded_rect_t *headphones_item3_bg = NULL;
gui_text_t *headphones3_name_label = NULL;
gui_text_t *headphones3_status_label = NULL;
gui_img_t *headphones3_icon = NULL;
gui_rounded_rect_t *headphones_item4_bg = NULL;
gui_text_t *headphones4_name_label = NULL;
gui_text_t *headphones4_status_label = NULL;
gui_img_t *headphones4_icon = NULL;
gui_rounded_rect_t *headphones_item5_bg = NULL;
gui_text_t *headphones5_name_label = NULL;
gui_text_t *headphones5_status_label = NULL;
gui_img_t *headphones5_icon = NULL;
gui_rounded_rect_t *headphones_item6_bg = NULL;
gui_text_t *headphones6_name_label = NULL;
gui_text_t *headphones6_status_label = NULL;
gui_img_t *headphones6_icon = NULL;
gui_rounded_rect_t *headphones_item7_bg = NULL;
gui_scroll_text_t *headphones7_name_label = NULL;
gui_text_t *headphones7_status_label = NULL;
gui_img_t *headphones7_icon = NULL;
gui_win_t *headphones_window = NULL;
gui_win_t *win_headphones_back = NULL;
gui_img_t *img_headphones_back = NULL;
gui_text_t *headphones_time_label = NULL;
gui_text_t *headphones_title_label = NULL;
gui_img_t *unbind_bt_icon = NULL;
gui_text_t *unbind_title_label = NULL;
gui_text_t *unbind_sub_label = NULL;
gui_rounded_rect_t *unbind_divider = NULL;
gui_rounded_rect_t *unbind_confirm_bg = NULL;
gui_text_t *unbind_confirm_label = NULL;
gui_rounded_rect_t *unbind_cancel_bg = NULL;
gui_text_t *unbind_cancel_label = NULL;
gui_text_t *found_devices_section_label = NULL;
gui_list_t *bt_search_list = NULL;
gui_rounded_rect_t *found_device1_bg = NULL;
gui_text_t *found_device1_name = NULL;
gui_text_t *found_device1_status = NULL;
gui_rounded_rect_t *found_device2_bg = NULL;
gui_text_t *found_device2_name = NULL;
gui_text_t *found_device2_status = NULL;
gui_win_t *bt_search_window = NULL;
gui_win_t *win_search_back = NULL;
gui_img_t *img_search_back = NULL;
gui_text_t *bt_search_time_label = NULL;
gui_text_t *bt_search_title_label = NULL;
gui_list_t *wifi_list = NULL;
gui_rounded_rect_t *wifi_toggle_bg = NULL;
gui_text_t *wifi_toggle_label = NULL;
gui_obj_t *wifi_toggle_btn = NULL;
gui_text_t *saved_networks_label = NULL;
gui_rounded_rect_t *saved_network_item_bg = NULL;
gui_text_t *saved_network_name_label = NULL;
gui_text_t *saved_network_status_label = NULL;
gui_img_t *saved_network_icon = NULL;
gui_win_t *wifi_window = NULL;
gui_text_t *wifi_time_label = NULL;
gui_text_t *wifi_title_label = NULL;
gui_win_t *win_3 = NULL;
gui_img_t *img_4 = NULL;
gui_list_t *settings_list = NULL;
gui_rounded_rect_t *device_name_bg = NULL;
gui_text_t *device_name_label = NULL;
gui_text_t *device_name_value = NULL;
gui_rounded_rect_t *bt_address_bg = NULL;
gui_text_t *bt_address_label = NULL;
gui_text_t *bt_address_value = NULL;
gui_rounded_rect_t *bt_version_bg = NULL;
gui_text_t *bt_version_label = NULL;
gui_text_t *bt_version_value = NULL;
gui_rounded_rect_t *wifi_ip_bg = NULL;
gui_text_t *wifi_ip_label = NULL;
gui_text_t *wifi_ip_value = NULL;
gui_rounded_rect_t *wifi_version_bg = NULL;
gui_text_t *wifi_version_label = NULL;
gui_text_t *wifi_version_value = NULL;
gui_win_t *settings_window = NULL;
gui_text_t *settings_time_label = NULL;
gui_text_t *settings_title_label = NULL;
gui_win_t *win_4 = NULL;
gui_img_t *img_5 = NULL;

// Time string global variables
char main_time_label_time_str[10] = {0};
char bt_time_label_time_str[10] = {0};
char headphones_time_label_time_str[10] = {0};
char bt_search_time_label_time_str[10] = {0};
char wifi_time_label_time_str[10] = {0};
char settings_time_label_time_str[10] = {0};

// Toggle button callback functions

// bt_toggle_btn dual-state button callback
static bool bt_toggle_btn_state = true;

void bt_toggle_btn_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    bt_toggle_btn_state = !bt_toggle_btn_state;

    // Switch image based on state and call corresponding callback
    if (bt_toggle_btn_state)
    {
        gui_img_set_src((gui_img_t *)bt_toggle_btn, (const uint8_t *)"/app_control_center/toggle_on.bin",
                        IMG_SRC_FILESYS);
        extern void bluetooth_toggle_on(void *obj, gui_event_t *e);
        bluetooth_toggle_on(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)bt_toggle_btn, (const uint8_t *)"/app_control_center/toggle_off.bin",
                        IMG_SRC_FILESYS);
        extern void bluetooth_toggle_on(void *obj, gui_event_t *e);
        bluetooth_toggle_off(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool bt_toggle_btn_get_state(void)
{
    return bt_toggle_btn_state;
}

// Set state (external call)
void bt_toggle_btn_set_state(bool state)
{
    if (bt_toggle_btn_state != state)
    {
        bt_toggle_btn_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)bt_toggle_btn, (const uint8_t *)"/app_control_center/toggle_on.bin",
                            IMG_SRC_FILESYS);
            extern void bluetooth_toggle_on(void *obj, gui_event_t *e);
            bluetooth_toggle_on(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)bt_toggle_btn, (const uint8_t *)"/app_control_center/toggle_off.bin",
                            IMG_SRC_FILESYS);
            extern void bluetooth_toggle_on(void *obj, gui_event_t *e);
            bluetooth_toggle_off(NULL, NULL);
        }
    }
}

// wifi_toggle_btn dual-state button callback
static bool wifi_toggle_btn_state = false;

void wifi_toggle_btn_toggle_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    // Toggle state
    wifi_toggle_btn_state = !wifi_toggle_btn_state;

    // Switch image based on state and call corresponding callback
    if (wifi_toggle_btn_state)
    {
        gui_img_set_src((gui_img_t *)wifi_toggle_btn, (const uint8_t *)"/app_control_center/toggle_on.bin",
                        IMG_SRC_FILESYS);
        extern void wifi_toggle_on(void *obj, gui_event_t *e);
        wifi_toggle_on(obj, e);
    }
    else
    {
        gui_img_set_src((gui_img_t *)wifi_toggle_btn, (const uint8_t *)"/app_control_center/toggle_off.bin",
                        IMG_SRC_FILESYS);
        extern void wifi_toggle_on(void *obj, gui_event_t *e);
        wifi_toggle_off(obj, e);
    }
    gui_fb_change();
}

// Get current state
bool wifi_toggle_btn_get_state(void)
{
    return wifi_toggle_btn_state;
}

// Set state (external call)
void wifi_toggle_btn_set_state(bool state)
{
    if (wifi_toggle_btn_state != state)
    {
        wifi_toggle_btn_state = state;

        if (state)
        {
            gui_img_set_src((gui_img_t *)wifi_toggle_btn, (const uint8_t *)"/app_control_center/toggle_on.bin",
                            IMG_SRC_FILESYS);
            extern void wifi_toggle_on(void *obj, gui_event_t *e);
            wifi_toggle_on(NULL, NULL);
        }
        else
        {
            gui_img_set_src((gui_img_t *)wifi_toggle_btn, (const uint8_t *)"/app_control_center/toggle_off.bin",
                            IMG_SRC_FILESYS);
            extern void wifi_toggle_on(void *obj, gui_event_t *e);
            wifi_toggle_off(NULL, NULL);
        }
    }
}
// List component note_design callback functions
// note_design callback function declaration
static void main_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void main_list_note_design(gui_obj_t *obj, void *param)
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
            // Create bluetooth_item_bg (hg_rect)
            bluetooth_item_bg = gui_rect_create((gui_obj_t *)note, "bluetooth_item_bg", 24, 0, 362, 84, 16,
                                                gui_rgb(44, 44, 46));
            gui_obj_add_event_cb(bluetooth_item_bg, (gui_event_cb_t)bluetooth_item_bg_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create bluetooth_icon (hg_image)
            bluetooth_icon = gui_img_create_from_fs((gui_obj_t *)note, "bluetooth_icon",
                                                    "/app_control_center/bluetooth_icon.bin", 44, 20, 44, 44);
            // Create bluetooth_label (hg_label)
            bluetooth_label = gui_text_create((gui_obj_t *)note, "bluetooth_label", 104, 0, 200, 84);
            gui_text_set((gui_text_t *)bluetooth_label, "Bluetooth", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         9, 36);
            gui_text_type_set((gui_text_t *)bluetooth_label, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bluetooth_label, MID_LEFT);
            break;
        }
    case 1:
        {
            // Create wifi_item_bg (hg_rect)
            wifi_item_bg = gui_rect_create((gui_obj_t *)note, "wifi_item_bg", 24, 0, 362, 84, 16, gui_rgb(44,
                                           44, 46));
            gui_obj_add_event_cb(wifi_item_bg, (gui_event_cb_t)wifi_item_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED,
                                 NULL);
            // Create wifi_icon (hg_image)
            wifi_icon = gui_img_create_from_fs((gui_obj_t *)note, "wifi_icon",
                                               "/app_control_center/wifi_icon.bin", 44, 20, 44, 44);
            // Create wifi_label (hg_label)
            wifi_label = gui_text_create((gui_obj_t *)note, "wifi_label", 104, 0, 200, 84);
            gui_text_set((gui_text_t *)wifi_label, "Wi-Fi", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5, 36);
            gui_text_type_set((gui_text_t *)wifi_label, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)wifi_label, MID_LEFT);
            break;
        }
    case 2:
        {
            // Create settings_item_bg (hg_rect)
            settings_item_bg = gui_rect_create((gui_obj_t *)note, "settings_item_bg", 24, 0, 362, 84, 16,
                                               gui_rgb(44, 44, 46));
            gui_obj_add_event_cb(settings_item_bg, (gui_event_cb_t)settings_item_bg_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create settings_icon (hg_image)
            settings_icon = gui_img_create_from_fs((gui_obj_t *)note, "settings_icon",
                                                   "/app_control_center/settings_icon.bin", 44, 20, 44, 44);
            // Create settings_label (hg_label)
            settings_label = gui_text_create((gui_obj_t *)note, "settings_label", 104, 0, 200, 84);
            gui_text_set((gui_text_t *)settings_label, "Settings", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 8,
                         36);
            gui_text_type_set((gui_text_t *)settings_label, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)settings_label, MID_LEFT);
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void wifi_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void wifi_list_note_design(gui_obj_t *obj, void *param)
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
            // Create wifi_toggle_bg (hg_rect)
            wifi_toggle_bg = gui_rect_create((gui_obj_t *)note, "wifi_toggle_bg", 24, 0, 362, 84, 12,
                                             gui_rgb(44, 44, 46));
            // Create wifi_toggle_label (hg_label)
            wifi_toggle_label = gui_text_create((gui_obj_t *)note, "wifi_toggle_label", 40, 22, 200, 50);
            gui_text_set((gui_text_t *)wifi_toggle_label, "Wi-Fi", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                         36);
            gui_text_type_set((gui_text_t *)wifi_toggle_label,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)wifi_toggle_label, LEFT);
            // Create wifi_toggle_btn (hg_button)
            wifi_toggle_btn = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)note, "wifi_toggle_btn",
                                                                  "/app_control_center/toggle_off.bin", 316, 26, 56, 32);
            if (wifi_toggle_btn_state)
            {
                gui_img_set_src((gui_img_t *)wifi_toggle_btn, (const uint8_t *)"/app_control_center/toggle_on.bin",
                                IMG_SRC_FILESYS);
            }
            gui_obj_add_event_cb((gui_obj_t *)wifi_toggle_btn, wifi_toggle_btn_toggle_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 1:
        {
            // Create saved_networks_label (hg_label)
            saved_networks_label = gui_text_create((gui_obj_t *)note, "saved_networks_label", 32, 50, 250, 42);
            gui_text_set((gui_text_t *)saved_networks_label, "SAVED NETWORKS", GUI_FONT_SRC_BMP, gui_rgb(102,
                         102, 102), 14, 32);
            gui_text_type_set((gui_text_t *)saved_networks_label,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)saved_networks_label, LEFT);
            break;
        }
    case 2:
        {
            // Create saved_network_item_bg (hg_rect)
            saved_network_item_bg = gui_rect_create((gui_obj_t *)note, "saved_network_item_bg", 24, 0, 362, 84,
                                                    12, gui_rgb(44, 44, 46));
            // Create saved_network_name_label (hg_label)
            saved_network_name_label = gui_text_create((gui_obj_t *)note, "saved_network_name_label", 40, 10,
                                                       260, 50);
            gui_text_set((gui_text_t *)saved_network_name_label, "Home_WiFi_5G", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 12, 36);
            gui_text_type_set((gui_text_t *)saved_network_name_label,
                              "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)saved_network_name_label, LEFT);
            // Create saved_network_status_label (hg_label)
            saved_network_status_label = gui_text_create((gui_obj_t *)note, "saved_network_status_label", 40,
                                                         56, 200, 42);
            gui_text_set((gui_text_t *)saved_network_status_label, "Connected", GUI_FONT_SRC_BMP, gui_rgb(76,
                         217, 100), 9, 24);
            gui_text_type_set((gui_text_t *)saved_network_status_label,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)saved_network_status_label, LEFT);
            // Create saved_network_icon (hg_image)
            saved_network_icon = gui_img_create_from_fs((gui_obj_t *)note, "saved_network_icon",
                                                        "/app_control_center/wifi_icon.bin", 341, 30, 18, 18);
            gui_img_scale((gui_img_t *)saved_network_icon, 0.409091f, 0.409091f);
            break;
        }
    default:
        break;
    }
}


// Create app_control_centerMainView (hg_view)
static void app_control_centerMainView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_control_centerMainView_switch_in(gui_view_t *view)
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



    // Create main_list (hg_list)
    main_list = gui_list_create((gui_obj_t *)view, "main_list", 0, 110, 410, 392, 84, 16, VERTICAL,
                                main_list_note_design, NULL, false);
    gui_list_set_style(main_list, LIST_CLASSIC);
    gui_list_set_note_num(main_list, 3);
    gui_list_set_out_scope(main_list, 80);

    // Create main_window (hg_window)
    main_window = gui_win_create((gui_obj_t *)view, "main_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)main_window, true);
    gui_win_set_blur_degree((gui_win_t *)main_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(main_time_label_time_str, sizeof(main_time_label_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create main_time_label (hg_time_label)
    main_time_label = gui_text_create(main_window, "main_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)main_time_label, main_time_label_time_str, GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), strlen(main_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)main_time_label, "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)main_time_label, RIGHT);

    // Create main_title_label (hg_label)
    main_title_label = gui_text_create(main_window, "main_title_label", 260, 63, 120, 34);
    gui_text_set((gui_text_t *)main_title_label, "Control", GUI_FONT_SRC_BMP, gui_rgb(233, 143, 54), 7,
                 24);
    gui_text_type_set((gui_text_t *)main_title_label,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)main_title_label, RIGHT);

    // Create win_1 (hg_window)
    win_1 = gui_win_create(main_window, "win_1", 0, 0, 90, 90);


    // Create main_back_btn (hg_image)
    main_back_btn = gui_img_create_from_fs(win_1, "main_back_btn", "/app_control_center/back_icon.bin",
                                           32, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_1), (gui_event_cb_t)win_1_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    gui_obj_add_event_cb(GUI_BASE(main_window), (gui_event_cb_t)main_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)main_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(main_time_label), 30000, true, main_time_label_time_update_cb);
}
GUI_VIEW_INSTANCE("app_control_centerMainView", false, app_control_centerMainView_switch_in,
                  app_control_centerMainView_switch_out, false);

// Create app_control_centerBluetoothView (hg_view)
static void app_control_centerBluetoothView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_control_centerBluetoothView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    // Bind timer: Animation 1
    gui_obj_create_timer((gui_obj_t *)view, 100, false, app_control_centerBluetoothView_timer_0_cb);

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create bt_toggle_bg (hg_rect)
    bt_toggle_bg = gui_rect_create((gui_obj_t *)view, "bt_toggle_bg", 24, 110, 362, 84, 12, gui_rgb(44,
                                   44, 46));

    // Create bt_headphones_entry_bg (hg_rect)
    bt_headphones_entry_bg = gui_rect_create((gui_obj_t *)view, "bt_headphones_entry_bg", 24, 199, 362,
                                             84, 12, gui_rgb(44, 44, 46));
    gui_obj_add_event_cb(bt_headphones_entry_bg, (gui_event_cb_t)bt_headphones_entry_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create phone_section_label (hg_label)
    phone_section_label = gui_text_create((gui_obj_t *)view, "phone_section_label", 29, 293, 200, 42);
    gui_text_set((gui_text_t *)phone_section_label, "PHONE", GUI_FONT_SRC_BMP, gui_rgb(102, 102, 102),
                 5, 32);
    gui_text_type_set((gui_text_t *)phone_section_label,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)phone_section_label, LEFT);

    // Create bt_list (hg_list)
    bt_list = gui_list_create((gui_obj_t *)view, "bt_list", 0, 330, 410, 172, 84, 5, VERTICAL,
                              bt_phone_list_note_design, NULL, false);
    gui_list_set_style(bt_list, LIST_CLASSIC);
    gui_list_set_note_num(bt_list, 1);
    gui_list_set_out_scope(bt_list, 80);
    gui_list_keep_note_alive(bt_list, true);
    gui_msg_subscribe((gui_obj_t *)bt_list, "bt/phone_conn", bt_list_msg_cb_0);
    gui_msg_subscribe((gui_obj_t *)bt_list, "bt/phone_disconn", bt_list_msg_cb_1);

    // Create bt_toggle_label (hg_label)
    bt_toggle_label = gui_text_create((gui_obj_t *)view, "bt_toggle_label", 44, 132, 200, 50);
    gui_text_set((gui_text_t *)bt_toggle_label, "Bluetooth", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 9, 36);
    gui_text_type_set((gui_text_t *)bt_toggle_label, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)bt_toggle_label, LEFT);

    // Create headphones_entry_icon (hg_image)
    headphones_entry_icon = gui_img_create_from_fs((gui_obj_t *)view, "headphones_entry_icon",
                                                   "/app_control_center/headphones_icon_connected.bin", 44, 219, 44, 44);
    gui_img_scale((gui_img_t *)headphones_entry_icon, 2.444444f, 2.444444f);

    // Create headphones_entry_label (hg_label)
    headphones_entry_label = gui_text_create((gui_obj_t *)view, "headphones_entry_label", 104, 221, 258,
                                             50);
    gui_text_set((gui_text_t *)headphones_entry_label, "Headphones", GUI_FONT_SRC_BMP, gui_rgb(90, 200,
                 250), 10, 36);
    gui_text_type_set((gui_text_t *)headphones_entry_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)headphones_entry_label, LEFT);

    // Create bt_window (hg_window)
    bt_window = gui_win_create((gui_obj_t *)view, "bt_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)bt_window, true);
    gui_win_set_blur_degree((gui_win_t *)bt_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(bt_time_label_time_str, sizeof(bt_time_label_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create win_2 (hg_window)
    win_2 = gui_win_create(bt_window, "win_2", 0, -3, 90, 90);


    // Create img_3 (hg_image)
    img_3 = gui_img_create_from_fs(win_2, "img_3", "/app_control_center/back_icon.bin", 31, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_2), (gui_event_cb_t)win_2_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create bt_time_label (hg_time_label)
    bt_time_label = gui_text_create(bt_window, "bt_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)bt_time_label, bt_time_label_time_str, GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), strlen(bt_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)bt_time_label, "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)bt_time_label, RIGHT);

    // Create bt_title_label (hg_label)
    bt_title_label = gui_text_create(bt_window, "bt_title_label", 245, 63, 135, 34);
    gui_text_set((gui_text_t *)bt_title_label, "Bluetooth", GUI_FONT_SRC_BMP, gui_rgb(233, 143, 54), 9,
                 24);
    gui_text_type_set((gui_text_t *)bt_title_label, "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)bt_title_label, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(bt_window), (gui_event_cb_t)bt_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)bt_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(bt_time_label), 30000, true, bt_time_label_time_update_cb);

    // Create bt_toggle_btn (hg_button)
    bt_toggle_btn = (gui_obj_t *)gui_img_create_from_fs((gui_obj_t *)view, "bt_toggle_btn",
                                                        "/app_control_center/toggle_off.bin", 300, 136, 56, 32);
    if (bt_toggle_btn_state)
    {
        gui_img_set_src((gui_img_t *)bt_toggle_btn, (const uint8_t *)"/app_control_center/toggle_on.bin",
                        IMG_SRC_FILESYS);
    }
    gui_obj_add_event_cb((gui_obj_t *)bt_toggle_btn, bt_toggle_btn_toggle_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);
}
GUI_VIEW_INSTANCE("app_control_centerBluetoothView", false,
                  app_control_centerBluetoothView_switch_in, app_control_centerBluetoothView_switch_out, false);

// Create app_control_centerHeadphonesView (hg_view)
static void app_control_centerHeadphonesView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_control_centerHeadphonesView_switch_in(gui_view_t *view)
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



    // Create search_item_bg (hg_rect)
    search_item_bg = gui_rect_create((gui_obj_t *)view, "search_item_bg", 24, 114, 362, 84, 12,
                                     gui_rgb(44, 44, 46));
    gui_obj_add_event_cb(search_item_bg, (gui_event_cb_t)search_item_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create headphone_list (hg_list)
    headphone_list = gui_list_create((gui_obj_t *)view, "headphone_list", 0, 283, 410, 219, 84, 5,
                                     VERTICAL, bt_headphone_list_note_design, NULL, false);
    gui_list_set_style(headphone_list, LIST_CLASSIC);
    gui_list_set_note_num(headphone_list, 7);
    gui_list_set_out_scope(headphone_list, 80);
    gui_list_keep_note_alive(headphone_list, true);
    gui_msg_subscribe((gui_obj_t *)headphone_list, "bt/headphone_conn", headphone_list_msg_cb_0);
    gui_msg_subscribe((gui_obj_t *)headphone_list, "bt/headphone_disconn", headphone_list_msg_cb_1);

    // Create search_icon (hg_image)
    search_icon = gui_img_create_from_fs((gui_obj_t *)view, "search_icon",
                                         "/app_control_center/search_icon.bin", 44, 130, 32, 32);
    gui_img_scale((gui_img_t *)search_icon, 1.777778f, 1.777778f);

    // Create search_label (hg_label)
    search_label = gui_text_create((gui_obj_t *)view, "search_label", 88, 130, 290, 50);
    gui_text_set((gui_text_t *)search_label, "Search Devices", GUI_FONT_SRC_BMP, gui_rgb(90, 200, 250),
                 14, 36);
    gui_text_type_set((gui_text_t *)search_label, "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)search_label, LEFT);

    // Create headphones_section_label (hg_label)
    headphones_section_label = gui_text_create((gui_obj_t *)view, "headphones_section_label", 28, 240,
                                               300, 42);
    gui_text_set((gui_text_t *)headphones_section_label, "HEADPHONES", GUI_FONT_SRC_BMP, gui_rgb(102,
                 102, 102), 10, 32);
    gui_text_type_set((gui_text_t *)headphones_section_label,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)headphones_section_label, LEFT);

    // Create headphones_window (hg_window)
    headphones_window = gui_win_create((gui_obj_t *)view, "headphones_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)headphones_window, true);
    gui_win_set_blur_degree((gui_win_t *)headphones_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(headphones_time_label_time_str, sizeof(headphones_time_label_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create win_headphones_back (hg_window)
    win_headphones_back = gui_win_create(headphones_window, "win_headphones_back", 0, -3, 90, 90);


    // Create img_headphones_back (hg_image)
    img_headphones_back = gui_img_create_from_fs(win_headphones_back, "img_headphones_back",
                                                 "/app_control_center/back_icon.bin", 31, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_headphones_back),
                         (gui_event_cb_t)win_headphones_back_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create headphones_time_label (hg_time_label)
    headphones_time_label = gui_text_create(headphones_window, "headphones_time_label", 305, 20, 80,
                                            32);
    gui_text_set((gui_text_t *)headphones_time_label, headphones_time_label_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(headphones_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)headphones_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)headphones_time_label, RIGHT);

    // Create headphones_title_label (hg_label)
    headphones_title_label = gui_text_create(headphones_window, "headphones_title_label", 230, 63, 155,
                                             34);
    gui_text_set((gui_text_t *)headphones_title_label, "Headphones", GUI_FONT_SRC_BMP, gui_rgb(233, 143,
                 54), 10, 24);
    gui_text_type_set((gui_text_t *)headphones_title_label,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)headphones_title_label, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(headphones_window), (gui_event_cb_t)headphones_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)headphones_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(headphones_time_label), 30000, true,
                         headphones_time_label_time_update_cb);
}
GUI_VIEW_INSTANCE("app_control_centerHeadphonesView", false,
                  app_control_centerHeadphonesView_switch_in, app_control_centerHeadphonesView_switch_out, false);

// Create app_control_centerBtUnbindView (hg_view)
static void app_control_centerBtUnbindView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_control_centerBtUnbindView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);


    // Create unbind_bt_icon (hg_image)
    unbind_bt_icon = gui_img_create_from_fs((gui_obj_t *)view, "unbind_bt_icon",
                                            "/app_control_center/bluetooth_icon.bin", 183, 72, 44, 44);

    // Create unbind_title_label (hg_label)
    unbind_title_label = gui_text_create((gui_obj_t *)view, "unbind_title_label", 0, 148, 410, 52);
    gui_text_set((gui_text_t *)unbind_title_label, "Remove Device Pairing?", GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), 22, 36);
    gui_text_type_set((gui_text_t *)unbind_title_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)unbind_title_label, CENTER);

    // Create unbind_sub_label (hg_label)
    unbind_sub_label = gui_text_create((gui_obj_t *)view, "unbind_sub_label", 24, 210, 362, 75);
    gui_text_set((gui_text_t *)unbind_sub_label,
                 "This device will be removed from your paired devices list.", GUI_FONT_SRC_BMP, gui_rgb(142, 142,
                         147), 58, 24);
    gui_text_type_set((gui_text_t *)unbind_sub_label,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)unbind_sub_label, MULTI_CENTER);
    gui_text_extra_line_spacing_set((gui_text_t *)unbind_sub_label, 8);

    // Create unbind_divider (hg_rect)
    unbind_divider = gui_rect_create((gui_obj_t *)view, "unbind_divider", 24, 298, 362, 1, 0,
                                     gui_rgb(58, 58, 60));

    // Create unbind_confirm_bg (hg_rect)
    unbind_confirm_bg = gui_rect_create((gui_obj_t *)view, "unbind_confirm_bg", 24, 318, 362, 80, 16,
                                        gui_rgb(255, 59, 48));
    gui_obj_add_event_cb(unbind_confirm_bg, (gui_event_cb_t)unbind_confirm_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create unbind_cancel_bg (hg_rect)
    unbind_cancel_bg = gui_rect_create((gui_obj_t *)view, "unbind_cancel_bg", 24, 414, 362, 80, 16,
                                       gui_rgb(44, 44, 46));
    gui_obj_add_event_cb(unbind_cancel_bg, (gui_event_cb_t)unbind_cancel_bg_clicked_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create unbind_confirm_label (hg_label)
    unbind_confirm_label = gui_text_create((gui_obj_t *)view, "unbind_confirm_label", 24, 338, 362, 50);
    gui_text_set((gui_text_t *)unbind_confirm_label, "Remove", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 6, 36);
    gui_text_type_set((gui_text_t *)unbind_confirm_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)unbind_confirm_label, CENTER);

    // Create unbind_cancel_label (hg_label)
    unbind_cancel_label = gui_text_create((gui_obj_t *)view, "unbind_cancel_label", 24, 434, 362, 50);
    gui_text_set((gui_text_t *)unbind_cancel_label, "Cancel", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 6, 36);
    gui_text_type_set((gui_text_t *)unbind_cancel_label,
                      "/font/Inter_24pt_Regular_size36_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)unbind_cancel_label, CENTER);
}
GUI_VIEW_INSTANCE("app_control_centerBtUnbindView", false, app_control_centerBtUnbindView_switch_in,
                  app_control_centerBtUnbindView_switch_out, false);

// Create app_control_centerBtSearchView (hg_view)
static void app_control_centerBtSearchView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_control_centerBtSearchView_switch_in(gui_view_t *view)
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



    // Create found_devices_section_label (hg_label)
    found_devices_section_label = gui_text_create((gui_obj_t *)view, "found_devices_section_label", 30,
                                                  162, 260, 43);
    gui_text_set((gui_text_t *)found_devices_section_label, "FOUND DEVICES", GUI_FONT_SRC_BMP,
                 gui_rgb(102, 102, 102), 13, 32);
    gui_text_type_set((gui_text_t *)found_devices_section_label,
                      "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)found_devices_section_label, LEFT);

    // Create bt_search_list (hg_list)
    bt_search_list = gui_list_create((gui_obj_t *)view, "bt_search_list", 0, 199, 410, 303, 84, 5,
                                     VERTICAL, bt_search_list_note_design, NULL, false);
    gui_list_set_style(bt_search_list, LIST_CLASSIC);
    gui_list_set_note_num(bt_search_list, 2);
    gui_list_set_out_scope(bt_search_list, 80);
    gui_list_keep_note_alive(bt_search_list, true);
    gui_msg_subscribe((gui_obj_t *)bt_search_list, "bt/inquiry_result", bt_search_list_msg_cb_0);
    gui_msg_subscribe((gui_obj_t *)bt_search_list, "bt/inquiry_cmpl", bt_search_list_msg_cb_1);
    gui_msg_subscribe((gui_obj_t *)bt_search_list, "bt/headphone_conn", bt_search_list_msg_cb_2);

    // Create bt_search_window (hg_window)
    bt_search_window = gui_win_create((gui_obj_t *)view, "bt_search_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)bt_search_window, true);
    gui_win_set_blur_degree((gui_win_t *)bt_search_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(bt_search_time_label_time_str, sizeof(bt_search_time_label_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create win_search_back (hg_window)
    win_search_back = gui_win_create(bt_search_window, "win_search_back", 0, -3, 90, 90);


    // Create img_search_back (hg_image)
    img_search_back = gui_img_create_from_fs(win_search_back, "img_search_back",
                                             "/app_control_center/back_icon.bin", 31, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_search_back), (gui_event_cb_t)win_search_back_clicked_0_cb,
                         GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create bt_search_time_label (hg_time_label)
    bt_search_time_label = gui_text_create(bt_search_window, "bt_search_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)bt_search_time_label, bt_search_time_label_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(bt_search_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)bt_search_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)bt_search_time_label, RIGHT);

    // Create bt_search_title_label (hg_label)
    bt_search_title_label = gui_text_create(bt_search_window, "bt_search_title_label", 186, 63, 199,
                                            34);
    gui_text_set((gui_text_t *)bt_search_title_label, "Found Devices", GUI_FONT_SRC_BMP, gui_rgb(233,
                 143, 54), 13, 24);
    gui_text_type_set((gui_text_t *)bt_search_title_label,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)bt_search_title_label, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(bt_search_window), (gui_event_cb_t)bt_search_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)bt_search_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(bt_search_time_label), 30000, true,
                         bt_search_time_label_time_update_cb);
}
GUI_VIEW_INSTANCE("app_control_centerBtSearchView", false, app_control_centerBtSearchView_switch_in,
                  app_control_centerBtSearchView_switch_out, false);

// Create app_control_centerWifiView (hg_view)
static void app_control_centerWifiView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_control_centerWifiView_switch_in(gui_view_t *view)
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



    // Create wifi_list (hg_list)
    wifi_list = gui_list_create((gui_obj_t *)view, "wifi_list", -16, 110, 410, 392, 84, 5, VERTICAL,
                                wifi_list_note_design, NULL, false);
    gui_list_set_style(wifi_list, LIST_CLASSIC);
    gui_list_set_note_num(wifi_list, 3);
    gui_list_set_out_scope(wifi_list, 80);

    // Create wifi_window (hg_window)
    wifi_window = gui_win_create((gui_obj_t *)view, "wifi_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)wifi_window, true);
    gui_win_set_blur_degree((gui_win_t *)wifi_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(wifi_time_label_time_str, sizeof(wifi_time_label_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create wifi_time_label (hg_time_label)
    wifi_time_label = gui_text_create(wifi_window, "wifi_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)wifi_time_label, wifi_time_label_time_str, GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), strlen(wifi_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)wifi_time_label, "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)wifi_time_label, RIGHT);

    // Create win_3 (hg_window)
    win_3 = gui_win_create(wifi_window, "win_3", 0, -3, 90, 90);


    // Create img_4 (hg_image)
    img_4 = gui_img_create_from_fs(win_3, "img_4", "/app_control_center/back_icon.bin", 31, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_3), (gui_event_cb_t)win_3_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create wifi_title_label (hg_label)
    wifi_title_label = gui_text_create(wifi_window, "wifi_title_label", 280, 63, 100, 34);
    gui_text_set((gui_text_t *)wifi_title_label, "Wi-Fi", GUI_FONT_SRC_BMP, gui_rgb(233, 143, 54), 5,
                 24);
    gui_text_type_set((gui_text_t *)wifi_title_label,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)wifi_title_label, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(wifi_window), (gui_event_cb_t)wifi_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)wifi_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(wifi_time_label), 30000, true, wifi_time_label_time_update_cb);
}
GUI_VIEW_INSTANCE("app_control_centerWifiView", false, app_control_centerWifiView_switch_in,
                  app_control_centerWifiView_switch_out, false);

// Create app_control_centerSettingsView (hg_view)
static void app_control_centerSettingsView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_control_centerSettingsView_switch_in(gui_view_t *view)
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



    // Create settings_list (hg_list)
    settings_list = gui_list_create((gui_obj_t *)view, "settings_list", 0, 110, 410, 392, 84, 10,
                                    VERTICAL, setting_list_note_design, NULL, false);
    gui_list_set_style(settings_list, LIST_CLASSIC);
    gui_list_set_note_num(settings_list, 5);
    gui_list_set_out_scope(settings_list, 80);

    // Create settings_window (hg_window)
    settings_window = gui_win_create((gui_obj_t *)view, "settings_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)settings_window, true);
    gui_win_set_blur_degree((gui_win_t *)settings_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(settings_time_label_time_str, sizeof(settings_time_label_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create settings_time_label (hg_time_label)
    settings_time_label = gui_text_create(settings_window, "settings_time_label", 305, 20, 80, 32);
    gui_text_set((gui_text_t *)settings_time_label, settings_time_label_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(255, 255, 255), strlen(settings_time_label_time_str), 28);
    gui_text_type_set((gui_text_t *)settings_time_label,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)settings_time_label, RIGHT);

    // Create win_4 (hg_window)
    win_4 = gui_win_create(settings_window, "win_4", 0, -3, 90, 90);


    // Create img_5 (hg_image)
    img_5 = gui_img_create_from_fs(win_4, "img_5", "/app_control_center/back_icon.bin", 31, 28, 32, 32);

    gui_obj_add_event_cb(GUI_BASE(win_4), (gui_event_cb_t)win_4_clicked_0_cb, GUI_EVENT_TOUCH_CLICKED,
                         NULL);

    // Create settings_title_label (hg_label)
    settings_title_label = gui_text_create(settings_window, "settings_title_label", 265, 63, 115, 34);
    gui_text_set((gui_text_t *)settings_title_label, "Settings", GUI_FONT_SRC_BMP, gui_rgb(233, 143,
                 54), 8, 24);
    gui_text_type_set((gui_text_t *)settings_title_label,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)settings_title_label, RIGHT);

    gui_obj_add_event_cb(GUI_BASE(settings_window), (gui_event_cb_t)settings_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)settings_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(settings_time_label), 30000, true,
                         settings_time_label_time_update_cb);
}
GUI_VIEW_INSTANCE("app_control_centerSettingsView", false, app_control_centerSettingsView_switch_in,
                  app_control_centerSettingsView_switch_out, false);
