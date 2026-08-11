/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef SMARTWATCHMAIN_CALLBACKS_H
#define SMARTWATCHMAIN_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t win_clock_big_timer_cnt;
extern uint16_t text_date_big_timer_cnt;
extern uint16_t text_date_small_timer_cnt;

// Event callback function declarations
void SmartWatchTemplateMainView_key_0_cb(void *obj, gui_event_t *e);
void bottom_View_tag_bg_menu_clicked_cb(void *obj, gui_event_t *e);
void bottom_View_weather_clicked_cb(void *obj, gui_event_t *e);
void hg_arc_1768184103087_n36y_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1768183941920_4wxc_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1768184009679_rg6a_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1769134788793_og20_clicked_cb(void *obj, gui_event_t *e);
void samrtWatch_window_key_0_cb(void *obj, gui_event_t *e);
void hg_time_label_hh_time_update_cb(void *p);
void hg_time_label_mm_time_update_cb(void *p);
void hg_time_label_1772765275313_pgx4_time_update_cb(void *p);
void hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_update_cb(void *p);

// User-configured timer callback function declarations
void win_clock_big_timer_0_cb(void *obj);
void text_date_big_timer_0_cb(void *obj);
void text_date_small_timer_0_cb(void *obj);

// Custom function declarations (auto-extracted from callbacks.c protected area)
void text_date_big_timer_0_cb(void *obj);
void text_date_small_timer_0_cb(void *obj);
void win_clock_big_timer_0_cb(void *obj);

#endif // SMARTWATCHMAIN_CALLBACKS_H
