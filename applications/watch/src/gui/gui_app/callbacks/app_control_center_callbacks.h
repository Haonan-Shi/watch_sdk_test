/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_CONTROL_CENTER_CALLBACKS_H
#define APP_CONTROL_CENTER_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t app_control_centerBluetoothView_timer_cnt;

// Event callback function declarations
void bluetooth_item_bg_clicked_cb(void *obj, gui_event_t *e);
void bt_headphones_entry_bg_clicked_cb(void *obj, gui_event_t *e);
void bt_list_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void bt_list_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void bt_search_list_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void bt_search_list_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void bt_search_list_msg_cb_2(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void bt_search_window_key_0_cb(void *obj, gui_event_t *e);
void bt_window_key_0_cb(void *obj, gui_event_t *e);
void found_device1_bg_clicked_cb(void *obj, gui_event_t *e);
void found_device2_bg_clicked_cb(void *obj, gui_event_t *e);
void headphone_list_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void headphone_list_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void headphones_item1_bg_clicked_cb(void *obj, gui_event_t *e);
void headphones_item1_bg_long_pressed_cb(void *obj, gui_event_t *e);
void headphones_item2_bg_clicked_cb(void *obj, gui_event_t *e);
void headphones_item2_bg_long_pressed_cb(void *obj, gui_event_t *e);
void headphones_item3_bg_clicked_cb(void *obj, gui_event_t *e);
void headphones_item3_bg_long_pressed_cb(void *obj, gui_event_t *e);
void headphones_item4_bg_clicked_cb(void *obj, gui_event_t *e);
void headphones_item4_bg_long_pressed_cb(void *obj, gui_event_t *e);
void headphones_item5_bg_clicked_cb(void *obj, gui_event_t *e);
void headphones_item5_bg_long_pressed_cb(void *obj, gui_event_t *e);
void headphones_item6_bg_clicked_cb(void *obj, gui_event_t *e);
void headphones_item6_bg_long_pressed_cb(void *obj, gui_event_t *e);
void headphones_item7_bg_clicked_cb(void *obj, gui_event_t *e);
void headphones_item7_bg_long_pressed_cb(void *obj, gui_event_t *e);
void headphones_window_key_0_cb(void *obj, gui_event_t *e);
void main_window_key_0_cb(void *obj, gui_event_t *e);
void phone_item_bg_clicked_cb(void *obj, gui_event_t *e);
void phone_item_bg_long_pressed_cb(void *obj, gui_event_t *e);
void search_item_bg_clicked_cb(void *obj, gui_event_t *e);
void settings_item_bg_clicked_cb(void *obj, gui_event_t *e);
void settings_window_key_0_cb(void *obj, gui_event_t *e);
void unbind_cancel_bg_clicked_cb(void *obj, gui_event_t *e);
void unbind_confirm_bg_clicked_cb(void *obj, gui_event_t *e);
void wifi_item_bg_clicked_cb(void *obj, gui_event_t *e);
void wifi_window_key_0_cb(void *obj, gui_event_t *e);
void win_1_clicked_0_cb(void *obj, gui_event_t *e);
void win_2_clicked_0_cb(void *obj, gui_event_t *e);
void win_3_clicked_0_cb(void *obj, gui_event_t *e);
void win_4_clicked_0_cb(void *obj, gui_event_t *e);
void win_headphones_back_clicked_0_cb(void *obj, gui_event_t *e);
void win_search_back_clicked_0_cb(void *obj, gui_event_t *e);
void main_time_label_time_update_cb(void *p);
void bt_time_label_time_update_cb(void *p);
void headphones_time_label_time_update_cb(void *p);
void bt_search_time_label_time_update_cb(void *p);
void wifi_time_label_time_update_cb(void *p);
void settings_time_label_time_update_cb(void *p);

// User-configured timer callback function declarations
void app_control_centerBluetoothView_timer_0_cb(void *obj);

// Toggle button state callback function declarations
void bt_toggle_btn_on_callback(void);
void bt_toggle_btn_off_callback(void);
void wifi_toggle_btn_on_callback(void);
void wifi_toggle_btn_off_callback(void);

#endif // APP_CONTROL_CENTER_CALLBACKS_H
