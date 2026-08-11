/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_NOISE_CALLBACKS_H
#define APP_NOISE_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t Nois_Level_Meter_bg_timer_cnt;
extern uint16_t Nois_Level_Meter0_timer_cnt;
extern uint16_t hg_image_1769156756841_h11r_timer_cnt;

// Event callback function declarations
void Noise_ok_bg_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1769151866129_h3az_key_cb(void *obj, gui_event_t *e);
void hg_time_label_1769151874591_kcfw_time_update_cb(void *p);

// User-configured timer callback function declarations
void Nois_Level_Meter_bg_timer_0_cb(void *obj);
void app_noise_view_init(void *obj);
void hg_image_1769156756841_h11r_timer_0_cb(void *obj);

#endif // APP_NOISE_CALLBACKS_H
