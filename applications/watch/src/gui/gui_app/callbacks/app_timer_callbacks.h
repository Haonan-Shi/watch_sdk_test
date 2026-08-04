/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_TIMER_CALLBACKS_H
#define APP_TIMER_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t active_timer_text_timer_cnt;
extern uint16_t active_arc_timer_cnt;

// Event callback function declarations
void app_timer_view_key_0_cb(void *obj, gui_event_t *e);
void bg_cancel_clicked_cb(void *obj, gui_event_t *e);
void bg_play_clicked_cb(void *obj, gui_event_t *e);
void img_15_clicked_cb(void *obj, gui_event_t *e);
void img_16_clicked_cb(void *obj, gui_event_t *e);
void img_17_clicked_cb(void *obj, gui_event_t *e);
void img_18_clicked_cb(void *obj, gui_event_t *e);
void img_19_clicked_cb(void *obj, gui_event_t *e);
void img_20_clicked_cb(void *obj, gui_event_t *e);
void img_21_clicked_cb(void *obj, gui_event_t *e);
void img_22_clicked_cb(void *obj, gui_event_t *e);
void img_25_clicked_cb(void *obj, gui_event_t *e);
void tm_lbl_1_time_update_cb(void *p);
void tm_lbl_2_time_update_cb(void *p);

// User-configured timer callback function declarations
void active_timer_update_cb(void *obj);
void active_arc_update_cb(void *obj);

// Custom function declarations (auto-extracted from callbacks.c protected area)
void active_timer_update_cb(void *obj);
void active_arc_update_cb(void *obj);

#endif // APP_TIMER_CALLBACKS_H
