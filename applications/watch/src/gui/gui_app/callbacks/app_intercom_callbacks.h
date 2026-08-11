/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_INTERCOM_CALLBACKS_H
#define APP_INTERCOM_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Event callback function declarations
void app_intercomMainView_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void connection_label_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void connection_label_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void device1_item_bg_clicked_cb(void *obj, gui_event_t *e);
void device2_item_bg_clicked_cb(void *obj, gui_event_t *e);
void device3_item_bg_clicked_cb(void *obj, gui_event_t *e);
void intercom_device_name_label_msg_cb_0(gui_obj_t *obj, const char *topic, void *data,
                                         uint16_t len);
void intercom_main_back_win_clicked_0_cb(void *obj, gui_event_t *e);
void intercom_talk_back_win_clicked_0_cb(void *obj, gui_event_t *e);
void mute_btn_event_cb(void *obj, gui_event_t *e);
void talk_btn_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void talk_btn_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void talk_btn_pressed_cb(void *obj, gui_event_t *e);
void talk_btn_released_cb(void *obj, gui_event_t *e);
void talk_window_key_0_cb(void *obj, gui_event_t *e);
void toggle_list_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void toggle_window_key_0_cb(void *obj, gui_event_t *e);
void toggle_time_label_time_update_cb(void *p);
void talk_time_label_time_update_cb(void *p);

// Toggle button state callback function declarations
void intercom_toggle_btn_on_callback(void);
void intercom_toggle_btn_off_callback(void);
void mute_btn_on_callback(void);
void mute_btn_off_callback(void);

#endif // APP_INTERCOM_CALLBACKS_H
