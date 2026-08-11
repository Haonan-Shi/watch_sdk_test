/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_ALARMS_CALLBACKS_H
#define APP_ALARMS_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Event callback function declarations
void alarm_top_window_key_0_cb(void *obj, gui_event_t *e);
void hg_time_label_1769669369949_1hub_time_update_cb(void *p);

// Toggle button state callback function declarations
void alarm_buttom_on_callback(void);
void alarm_buttom_off_callback(void);

#endif // APP_ALARMS_CALLBACKS_H
