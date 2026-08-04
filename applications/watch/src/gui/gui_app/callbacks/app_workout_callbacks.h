/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_WORKOUT_CALLBACKS_H
#define APP_WORKOUT_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t app_workout_icon_bg1_timer_cnt;
extern uint16_t app_workout_icon1_timer_cnt;
extern uint16_t app_workout_start_icon_timer_cnt;
extern uint16_t workout_countdown_arc_timer_cnt;

// Event callback function declarations
void app_workout_menu_topwindow_key_0_cb(void *obj, gui_event_t *e);
void app_workout_start_engine_bg_clicked_cb(void *obj, gui_event_t *e);
void app_workout_start_icon_clicked_cb(void *obj, gui_event_t *e);
void workout_countdown_arc_clicked_cb(void *obj, gui_event_t *e);
void app_workout_time_text_time_update_cb(void *p);
void hg_time_label_1769406976200_a8x9_time_update_cb(void *p);

// User-configured timer callback function declarations
void hg_image_1769161039267_t63r_timer_0_cb(void *obj);
void app_workout_icon_bg1_timer_1_cb(void *obj);
void app_workout_start_icon_timer_0_cb(void *obj);

#endif // APP_WORKOUT_CALLBACKS_H
