/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_STOPWATCH_CALLBACKS_H
#define APP_STOPWATCH_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t app_stopwatch_view_timer_cnt;
extern uint16_t lst_stopwatch_timer_cnt;
extern uint16_t lst_stopwatch_item_0_timer_cnt;
extern uint16_t lst_stopwatch_item_1_timer_cnt;

// Event callback function declarations
void app_stopwatch_view_key_0_cb(void *obj, gui_event_t *e);
void bg_l_clicked_cb(void *obj, gui_event_t *e);
void bg_r_clicked_cb(void *obj, gui_event_t *e);
void tm_lbl_3_time_update_cb(void *p);

// User-configured timer callback function declarations
void app_stopwatch_view_timer_0_cb(void *obj);
void lst_stopwatch_timer_0_cb(void *obj);
void stopwatch_page_0_timer_cb(void *obj);
void stopwatch_page_1_timer_cb(void *obj);

#endif // APP_STOPWATCH_CALLBACKS_H
