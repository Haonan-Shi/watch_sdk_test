/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_PHONE_CALLBACKS_H
#define APP_PHONE_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t calling_number_label_timer_cnt;
extern uint16_t call_timer_label_timer_cnt;
extern uint16_t incoming_ring_animation_img_timer_cnt;

// Event callback function declarations
void app_phoneDialerView_key_0_cb(void *obj, gui_event_t *e);
void call_btn_clicked_cb(void *obj, gui_event_t *e);
void calling_number_label_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void calling_number_label_msg_cb_1(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void decline_btn_clicked_cb(void *obj, gui_event_t *e);
void delete_btn_clicked_cb(void *obj, gui_event_t *e);
void dial_key_0_clicked_cb(void *obj, gui_event_t *e);
void dial_key_1_clicked_cb(void *obj, gui_event_t *e);
void dial_key_2_clicked_cb(void *obj, gui_event_t *e);
void dial_key_3_clicked_cb(void *obj, gui_event_t *e);
void dial_key_4_clicked_cb(void *obj, gui_event_t *e);
void dial_key_5_clicked_cb(void *obj, gui_event_t *e);
void dial_key_6_clicked_cb(void *obj, gui_event_t *e);
void dial_key_7_clicked_cb(void *obj, gui_event_t *e);
void dial_key_8_clicked_cb(void *obj, gui_event_t *e);
void dial_key_9_clicked_cb(void *obj, gui_event_t *e);
void dial_key_hash_clicked_cb(void *obj, gui_event_t *e);
void dial_key_star_clicked_cb(void *obj, gui_event_t *e);
void hangup_btn_clicked_cb(void *obj, gui_event_t *e);
void incoming_btn_clicked_cb(void *obj, gui_event_t *e);
void incoming_name_label_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void incoming_number_label_msg_cb_0(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void phone_call_mute_btn_clicked_cb(void *obj, gui_event_t *e);
void simulate_incoming_btn_clicked_cb(void *obj, gui_event_t *e);
void volume_down_btn_clicked_cb(void *obj, gui_event_t *e);
void volume_up_btn_clicked_cb(void *obj, gui_event_t *e);
void win_10_clicked_0_cb(void *obj, gui_event_t *e);
void dialer_time_label_time_update_cb(void *p);
void calling_time_label_time_update_cb(void *p);
void incoming_time_label_time_update_cb(void *p);

// User-configured timer callback function declarations
void calling_number_label_timer_0_cb(void *obj);
void call_timer_tick(void *obj);
void incoming_ring_timer_cb(void *obj);

#endif // APP_PHONE_CALLBACKS_H
