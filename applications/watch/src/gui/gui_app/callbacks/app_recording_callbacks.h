/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_RECORDING_CALLBACKS_H
#define APP_RECORDING_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t recording_timer_label_timer_cnt;
extern uint16_t recording_waveform_image_timer_cnt;
extern uint16_t playback_current_time_label_timer_cnt;

// Event callback function declarations
void recording_file_0_selected(void *obj, gui_event_t *e);
void recording_file_1_selected(void *obj, gui_event_t *e);
void recording_file_2_selected(void *obj, gui_event_t *e);
void recording_file_3_selected(void *obj, gui_event_t *e);
void recording_file_4_selected(void *obj, gui_event_t *e);
void recording_files_btn_clicked_cb(void *obj, gui_event_t *e);
void recording_files_window_key_0_cb(void *obj, gui_event_t *e);
void recording_main_window_key_0_cb(void *obj, gui_event_t *e);
void recording_playback_window_key_0_cb(void *obj, gui_event_t *e);
void win_7_clicked_0_cb(void *obj, gui_event_t *e);
void win_8_clicked_0_cb(void *obj, gui_event_t *e);
void win_9_clicked_0_cb(void *obj, gui_event_t *e);
void recording_main_time_label_time_update_cb(void *p);
void recording_files_time_label_time_update_cb(void *p);
void recording_playback_time_label_time_update_cb(void *p);

// User-configured timer callback function declarations
void recording_timer_tick(void *obj);
void recording_waveform_timer_cb(void *obj);
void playback_timer_tick(void *obj);

// Toggle button state callback function declarations
void recording_record_btn_on_callback(void);
void recording_record_btn_off_callback(void);
void playback_play_btn_on_callback(void);
void playback_play_btn_off_callback(void);

#endif // APP_RECORDING_CALLBACKS_H
