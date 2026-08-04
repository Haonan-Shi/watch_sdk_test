/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_OTA_CALLBACKS_H
#define APP_OTA_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t ota_ready_dots_timer_cnt;
extern uint16_t ota_starting_glow_timer_cnt;
extern uint16_t ota_starting_ring_bg_timer_cnt;
extern uint16_t ota_starting_dots_timer_cnt;
extern uint16_t ota_updating_glow_timer_cnt;
extern uint16_t ota_updating_ring_bg_timer_cnt;
extern uint16_t ota_updating_dots_timer_cnt;

// Event callback function declarations
void app_otaFailedView_key_0_cb(void *obj, gui_event_t *e);
void app_otaReadyView_key_0_cb(void *obj, gui_event_t *e);
void app_otaStartingView_key_0_cb(void *obj, gui_event_t *e);
void app_otaSuccessView_key_0_cb(void *obj, gui_event_t *e);
void app_otaUpdatingView_key_0_cb(void *obj, gui_event_t *e);
void ota_connect_btn_clicked_cb(void *obj, gui_event_t *e);
void ota_done_btn_clicked_cb(void *obj, gui_event_t *e);
void ota_failed_back_btn_clicked_cb(void *obj, gui_event_t *e);
void ota_ready_back_btn_clicked_cb(void *obj, gui_event_t *e);
void ota_retry_btn_clicked_cb(void *obj, gui_event_t *e);
void ota_success_back_btn_clicked_cb(void *obj, gui_event_t *e);

// User-configured timer callback function declarations
void ota_ready_dots_timer_0_cb(void *obj);
void ota_starting_glow_timer_0_cb(void *obj);
void ota_starting_timer_cb(void *obj);
void ota_starting_dots_timer_0_cb(void *obj);
void ota_updating_glow_timer_0_cb(void *obj);
void ota_progress_tick_cb(void *obj);
void ota_updating_dots_timer_0_cb(void *obj);

#endif // APP_OTA_CALLBACKS_H
