/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_MUSIC_SCAN_CALLBACKS_H
#define APP_MUSIC_SCAN_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t music_scan_icon_bg_timer_cnt;

// Event callback function declarations
void app_music_scan_view_key_0_cb(void *obj, gui_event_t *e);
void music_scan_icon_clicked_cb(void *obj, gui_event_t *e);
void music_scan_icon_key_cb(void *obj, gui_event_t *e);
void music_scan_time_text_time_update_cb(void *p);

// User-configured timer callback function declarations
void music_scan_icon_bg_timer_0_cb(void *obj);

#endif // APP_MUSIC_SCAN_CALLBACKS_H
