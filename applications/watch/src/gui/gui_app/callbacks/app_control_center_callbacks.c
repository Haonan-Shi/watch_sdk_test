/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_control_center_callbacks.h"
#include "../ui/app_control_center_ui.h"
#include "../user/app_control_center_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char main_time_label_time_str[10];
extern char bt_time_label_time_str[10];
extern char headphones_time_label_time_str[10];
extern char bt_search_time_label_time_str[10];
extern char wifi_time_label_time_str[10];
extern char settings_time_label_time_str[10];

// Timer animation counters
uint16_t app_control_centerBluetoothView_timer_cnt = 0;

// Event callback function implementations

void bluetooth_item_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBluetoothView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void wifi_item_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerWifiView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void settings_item_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerSettingsView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void main_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_1_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void bt_headphones_entry_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerHeadphonesView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void phone_item_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    phone_linkback_and_disconnect(obj, e);
}

void phone_item_bg_long_pressed_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtUnbindView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    phone_remove_paired_device(obj, e);
}

void bt_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_2_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerMainView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void search_item_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtSearchView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    bluetooth_search_devices(obj, e);
}

void headphones_item1_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone1_linkback_and_disconnect(obj, e);
}

void headphones_item1_bg_long_pressed_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtUnbindView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    headphone1_remove_paired_device(obj, e);
}

void headphones_item2_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone2_linkback_and_disconnect(obj, e);
}

void headphones_item2_bg_long_pressed_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtUnbindView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    headphone2_remove_paired_device(obj, e);
}

void headphones_item3_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone3_linkback_and_disconnect(obj, e);
}

void headphones_item3_bg_long_pressed_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtUnbindView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    headphone3_remove_paired_device(obj, e);
}

void headphones_item4_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone4_linkback_and_disconnect(obj, e);
}

void headphones_item4_bg_long_pressed_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtUnbindView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    headphone4_remove_paired_device(obj, e);
}

void headphones_item5_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone5_linkback_and_disconnect(obj, e);
}

void headphones_item5_bg_long_pressed_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtUnbindView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    headphone5_remove_paired_device(obj, e);
}

void headphones_item6_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone6_linkback_and_disconnect(obj, e);
}

void headphones_item6_bg_long_pressed_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtUnbindView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    headphone6_remove_paired_device(obj, e);
}

void headphones_item7_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone7_linkback_and_disconnect(obj, e);
}

void headphones_item7_bg_long_pressed_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBtUnbindView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    headphone7_remove_paired_device(obj, e);
}

void headphones_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_headphones_back_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBluetoothView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void unbind_confirm_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBluetoothView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    remove_paired_device_confirm(obj, e);
}

void unbind_cancel_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerBluetoothView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    remove_paired_device_cancel(obj, e);
}

void found_device1_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone1_connect(obj, e);
}

void found_device2_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    headphone2_connect(obj, e);
}

void bt_search_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_search_back_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerHeadphonesView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void wifi_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_3_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerMainView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void settings_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void win_4_clicked_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerMainView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void bt_list_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    update_phone_list(obj, topic, data, len);
}

void bt_list_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    update_phone_list(obj, topic, data, len);
}

void headphone_list_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    update_headphone_list(obj, topic, data, len);
}

void headphone_list_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    update_headphone_list(obj, topic, data, len);
}

void bt_search_list_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    update_search_list(obj, topic, data, len);
}

void bt_search_list_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    update_search_list(obj, topic, data, len);
}

void bt_search_list_msg_cb_2(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(topic);
    GUI_UNUSED(data);
    GUI_UNUSED(len);
    gui_view_switch_direct(gui_view_get_current(), "app_control_centerHeadphonesView",
                           SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
}

void main_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(main_time_label_time_str, sizeof(main_time_label_time_str), "%02d:%02d", t->tm_hour,
             t->tm_min);

    gui_text_content_set((gui_text_t *)main_time_label, main_time_label_time_str,
                         strlen(main_time_label_time_str));
}

void bt_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(bt_time_label_time_str, sizeof(bt_time_label_time_str), "%02d:%02d", t->tm_hour,
             t->tm_min);

    gui_text_content_set((gui_text_t *)bt_time_label, bt_time_label_time_str,
                         strlen(bt_time_label_time_str));
}

void headphones_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(headphones_time_label_time_str, sizeof(headphones_time_label_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)headphones_time_label, headphones_time_label_time_str,
                         strlen(headphones_time_label_time_str));
}

void bt_search_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(bt_search_time_label_time_str, sizeof(bt_search_time_label_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)bt_search_time_label, bt_search_time_label_time_str,
                         strlen(bt_search_time_label_time_str));
}

void wifi_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(wifi_time_label_time_str, sizeof(wifi_time_label_time_str), "%02d:%02d", t->tm_hour,
             t->tm_min);

    gui_text_content_set((gui_text_t *)wifi_time_label, wifi_time_label_time_str,
                         strlen(wifi_time_label_time_str));
}

void settings_time_label_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(settings_time_label_time_str, sizeof(settings_time_label_time_str), "%02d:%02d",
             t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)settings_time_label, settings_time_label_time_str,
                         strlen(settings_time_label_time_str));
}

// Preset timer callback functions

/**
 * Animation 1
 * Component: app_control_centerBluetoothView
 */
void app_control_centerBluetoothView_timer_0_cb(void *obj)
{
    GUI_UNUSED(obj);
    // Call the implementation function in protected area (if exists)
    // Define app_control_centerBluetoothView_timer_0_cb_impl() in custom_functions protected area for custom logic
#ifdef __cplusplus
    extern "C" {
#endif
    extern void app_control_centerBluetoothView_timer_0_cb_impl(void) __attribute__((weak));
#ifdef __cplusplus
}
#endif

if (app_control_centerBluetoothView_timer_0_cb_impl)
{
    app_control_centerBluetoothView_timer_0_cb_impl();
}
else
{
    // TODO: Implement timer callback logic
    // Or define app_control_centerBluetoothView_timer_0_cb_impl() in custom_functions protected area
}
}

// Toggle button state callback functions

/* USER CODE BEGIN bt_toggle_btn_on_callback */
/**
 * bt_toggle_btn ON state callback
 * Called when button switches to ON state
 */
void bt_toggle_btn_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END bt_toggle_btn_on_callback */

/* USER CODE BEGIN bt_toggle_btn_off_callback */
/**
 * bt_toggle_btn OFF state callback
 * Called when button switches to OFF state
 */
void bt_toggle_btn_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END bt_toggle_btn_off_callback */

/* USER CODE BEGIN wifi_toggle_btn_on_callback */
/**
 * wifi_toggle_btn ON state callback
 * Called when button switches to ON state
 */
void wifi_toggle_btn_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END wifi_toggle_btn_on_callback */

/* USER CODE BEGIN wifi_toggle_btn_off_callback */
/**
 * wifi_toggle_btn OFF state callback
 * Called when button switches to OFF state
 */
void wifi_toggle_btn_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END wifi_toggle_btn_off_callback */

/* @protected start custom_functions */
// Custom functions
// Note: After code regeneration from HML, fill in the USER CODE areas for
// bt_toggle_btn_on_callback  -> bluetooth_toggle_on(NULL, NULL);
// bt_toggle_btn_off_callback -> bluetooth_toggle_off(NULL, NULL);
// wifi_toggle_btn_on_callback  -> wifi_toggle_on(NULL, NULL);
// wifi_toggle_btn_off_callback -> wifi_toggle_off(NULL, NULL);
/* @protected end custom_functions */
