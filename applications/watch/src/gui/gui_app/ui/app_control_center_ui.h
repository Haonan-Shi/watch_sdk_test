/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_control_center UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.802Z
 */
#ifndef APP_CONTROL_CENTER_UI_H
#define APP_CONTROL_CENTER_UI_H

#include "guidef.h"
#include "gui_obj.h"
#include "gui_components_init.h"
#include "gui_view.h"
#include "gui_view_instance.h"
#include "gui_win.h"
#include "draw_font.h"
#include "font_types.h"
#include "gui_scroll_text.h"
#include "gui_list.h"
#include "gui_rect.h"
#include "gui_img.h"
#include "gui_text.h"

// Component handle declarations
extern gui_list_t *main_list;
extern gui_rounded_rect_t *bluetooth_item_bg;
extern gui_img_t *bluetooth_icon;
extern gui_text_t *bluetooth_label;
extern gui_rounded_rect_t *wifi_item_bg;
extern gui_img_t *wifi_icon;
extern gui_text_t *wifi_label;
extern gui_rounded_rect_t *settings_item_bg;
extern gui_img_t *settings_icon;
extern gui_text_t *settings_label;
extern gui_win_t *main_window;
extern gui_text_t *main_time_label;
extern gui_text_t *main_title_label;
extern gui_win_t *win_1;
extern gui_img_t *main_back_btn;
extern gui_rounded_rect_t *bt_toggle_bg;
extern gui_text_t *bt_toggle_label;
extern gui_obj_t *bt_toggle_btn;
extern gui_rounded_rect_t *bt_headphones_entry_bg;
extern gui_img_t *headphones_entry_icon;
extern gui_text_t *headphones_entry_label;
extern gui_text_t *phone_section_label;
extern gui_list_t *bt_list;
extern gui_rounded_rect_t *phone_item_bg;
extern gui_text_t *phone_name_label;
extern gui_text_t *phone_status_label;
extern gui_img_t *phone_icon;
extern gui_win_t *bt_window;
extern gui_win_t *win_2;
extern gui_img_t *img_3;
extern gui_text_t *bt_time_label;
extern gui_text_t *bt_title_label;
extern gui_rounded_rect_t *search_item_bg;
extern gui_img_t *search_icon;
extern gui_text_t *search_label;
extern gui_text_t *headphones_section_label;
extern gui_list_t *headphone_list;
extern gui_rounded_rect_t *headphones_item1_bg;
extern gui_text_t *headphones1_name_label;
extern gui_text_t *headphones1_status_label;
extern gui_img_t *headphones1_icon;
extern gui_rounded_rect_t *headphones_item2_bg;
extern gui_text_t *headphones2_name_label;
extern gui_text_t *headphones2_status_label;
extern gui_img_t *headphones2_icon;
extern gui_rounded_rect_t *headphones_item3_bg;
extern gui_text_t *headphones3_name_label;
extern gui_text_t *headphones3_status_label;
extern gui_img_t *headphones3_icon;
extern gui_rounded_rect_t *headphones_item4_bg;
extern gui_text_t *headphones4_name_label;
extern gui_text_t *headphones4_status_label;
extern gui_img_t *headphones4_icon;
extern gui_rounded_rect_t *headphones_item5_bg;
extern gui_text_t *headphones5_name_label;
extern gui_text_t *headphones5_status_label;
extern gui_img_t *headphones5_icon;
extern gui_rounded_rect_t *headphones_item6_bg;
extern gui_text_t *headphones6_name_label;
extern gui_text_t *headphones6_status_label;
extern gui_img_t *headphones6_icon;
extern gui_rounded_rect_t *headphones_item7_bg;
extern gui_scroll_text_t *headphones7_name_label;
extern gui_text_t *headphones7_status_label;
extern gui_img_t *headphones7_icon;
extern gui_win_t *headphones_window;
extern gui_win_t *win_headphones_back;
extern gui_img_t *img_headphones_back;
extern gui_text_t *headphones_time_label;
extern gui_text_t *headphones_title_label;
extern gui_img_t *unbind_bt_icon;
extern gui_text_t *unbind_title_label;
extern gui_text_t *unbind_sub_label;
extern gui_rounded_rect_t *unbind_divider;
extern gui_rounded_rect_t *unbind_confirm_bg;
extern gui_text_t *unbind_confirm_label;
extern gui_rounded_rect_t *unbind_cancel_bg;
extern gui_text_t *unbind_cancel_label;
extern gui_text_t *found_devices_section_label;
extern gui_list_t *bt_search_list;
extern gui_rounded_rect_t *found_device1_bg;
extern gui_text_t *found_device1_name;
extern gui_text_t *found_device1_status;
extern gui_rounded_rect_t *found_device2_bg;
extern gui_text_t *found_device2_name;
extern gui_text_t *found_device2_status;
extern gui_win_t *bt_search_window;
extern gui_win_t *win_search_back;
extern gui_img_t *img_search_back;
extern gui_text_t *bt_search_time_label;
extern gui_text_t *bt_search_title_label;
extern gui_list_t *wifi_list;
extern gui_rounded_rect_t *wifi_toggle_bg;
extern gui_text_t *wifi_toggle_label;
extern gui_obj_t *wifi_toggle_btn;
extern gui_text_t *saved_networks_label;
extern gui_rounded_rect_t *saved_network_item_bg;
extern gui_text_t *saved_network_name_label;
extern gui_text_t *saved_network_status_label;
extern gui_img_t *saved_network_icon;
extern gui_win_t *wifi_window;
extern gui_text_t *wifi_time_label;
extern gui_text_t *wifi_title_label;
extern gui_win_t *win_3;
extern gui_img_t *img_4;
extern gui_list_t *settings_list;
extern gui_rounded_rect_t *device_name_bg;
extern gui_text_t *device_name_label;
extern gui_text_t *device_name_value;
extern gui_rounded_rect_t *bt_address_bg;
extern gui_text_t *bt_address_label;
extern gui_text_t *bt_address_value;
extern gui_rounded_rect_t *bt_version_bg;
extern gui_text_t *bt_version_label;
extern gui_text_t *bt_version_value;
extern gui_rounded_rect_t *wifi_ip_bg;
extern gui_text_t *wifi_ip_label;
extern gui_text_t *wifi_ip_value;
extern gui_rounded_rect_t *wifi_version_bg;
extern gui_text_t *wifi_version_label;
extern gui_text_t *wifi_version_value;
extern gui_win_t *settings_window;
extern gui_text_t *settings_time_label;
extern gui_text_t *settings_title_label;
extern gui_win_t *win_4;
extern gui_img_t *img_5;

// Toggle button state management function declarations
extern bool bt_toggle_btn_get_state(void);
extern void bt_toggle_btn_set_state(bool state);
extern bool wifi_toggle_btn_get_state(void);
extern void wifi_toggle_btn_set_state(bool state);

#endif // APP_CONTROL_CENTER_UI_H
