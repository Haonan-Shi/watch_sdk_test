/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_HEART_CALLBACKS_H
#define APP_HEART_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t hg_image_1769146380658_kvde_timer_cnt;
extern uint16_t app_heart_circel0_timer_cnt;

// Event callback function declarations
void app_heart_window_key_0_cb(void *obj, gui_event_t *e);
void hg_time_label_heart_time_update_cb(void *p);

// User-configured timer callback function declarations
void hg_image_1769146380658_kvde_timer_0_cb(void *obj);
void app_heart_circel0_timer_0_cb(void *obj);

// Custom function declarations (auto-extracted from callbacks.c protected area)
void app_heart_circel0_timer_0_cb(void *obj);

#endif // APP_HEART_CALLBACKS_H
