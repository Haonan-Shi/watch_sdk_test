/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_CONTROL_CENTER_USER_H
#define APP_CONTROL_CENTER_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gui_api.h"

void bluetooth_toggle_on(void *obj, gui_event_t *e);
void bluetooth_toggle_off(void *obj, gui_event_t *e);
void bluetooth_search_devices(void *obj, gui_event_t *e);
void wifi_toggle_on(void *obj, gui_event_t *e);
void wifi_toggle_off(void *obj, gui_event_t *e);
void phone_linkback_and_disconnect(void *obj, gui_event_t *e);
void headphone1_linkback_and_disconnect(void *obj, gui_event_t *e);
void headphone2_linkback_and_disconnect(void *obj, gui_event_t *e);
void headphone3_linkback_and_disconnect(void *obj, gui_event_t *e);
void headphone4_linkback_and_disconnect(void *obj, gui_event_t *e);
void headphone5_linkback_and_disconnect(void *obj, gui_event_t *e);
void headphone6_linkback_and_disconnect(void *obj, gui_event_t *e);
void headphone7_linkback_and_disconnect(void *obj, gui_event_t *e);
void phone_remove_paired_device(void *obj, gui_event_t *e);
void headphone1_remove_paired_device(void *obj, gui_event_t *e);
void headphone2_remove_paired_device(void *obj, gui_event_t *e);
void headphone3_remove_paired_device(void *obj, gui_event_t *e);
void headphone4_remove_paired_device(void *obj, gui_event_t *e);
void headphone5_remove_paired_device(void *obj, gui_event_t *e);
void headphone6_remove_paired_device(void *obj, gui_event_t *e);
void headphone7_remove_paired_device(void *obj, gui_event_t *e);
void remove_paired_device_confirm(void *obj, gui_event_t *e);
void remove_paired_device_cancel(void *obj, gui_event_t *e);
void headphone1_connect(void *obj, gui_event_t *e);
void headphone2_connect(void *obj, gui_event_t *e);
void update_phone_list(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void update_headphone_list(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void update_search_list(gui_obj_t *obj, const char *topic, void *data, uint16_t len);
void bt_phone_list_note_design(gui_obj_t *obj, void *param);
void bt_headphone_list_note_design(gui_obj_t *obj, void *param);
void bt_search_list_note_design(gui_obj_t *obj, void *param);
void setting_list_note_design(gui_obj_t *obj, void *param);

#ifdef __cplusplus
}
#endif

#endif
