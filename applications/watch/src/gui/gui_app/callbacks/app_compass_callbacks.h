/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_COMPASS_CALLBACKS_H
#define APP_COMPASS_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t compass_image_timer_cnt;

// Event callback function declarations
void compass_window_key_0_cb(void *obj, gui_event_t *e);
void compass_time_label_time_update_cb(void *p);

// User-configured timer callback function declarations
void compass_image_timer_0_cb(void *obj);

#endif // APP_COMPASS_CALLBACKS_H
