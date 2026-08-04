/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_INTERCOM_USER_H
#define APP_INTERCOM_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gui_api.h"

void intercom_toggle_on(void *obj, gui_event_t *e);
void intercom_toggle_off(void *obj, gui_event_t *e);
void intercom_update_scan_result(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void intercom_connect_dev(void *obj, gui_event_t *e);
void intercom_update_connect_status(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void intercom_update_user_name(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void intercom_update_receive_status(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void talk_btn_press(void *obj, gui_event_t *e);
void talk_btn_release(void *obj, gui_event_t *e);
void mute_btn_on(void *obj, gui_event_t *e);
void mute_btn_off(void *obj, gui_event_t *e);
void intercom_disconnect(void *obj, gui_event_t *e);
void talking_timer_cb(void *obj);
void receive_timer_cb(void *obj);
void walkie_talkie_list_note_design(gui_obj_t *obj, void *param);

#ifdef __cplusplus
}
#endif

#endif // APP_INTERCOM_USER_H
