/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_VIDEO_CALL_CALLBACKS_H
#define APP_VIDEO_CALL_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t video_call_ring_pulse_img_timer_cnt;

// Event callback function declarations
void video_call_btn_clicked_cb(void *obj, gui_event_t *e);
void video_call_calling_avatar_msg_cb_0(gui_obj_t *obj, const char *topic, void *data,
                                        uint16_t len);
void video_call_calling_back_icon_clicked_cb(void *obj, gui_event_t *e);
void video_call_hangup_btn_clicked_cb(void *obj, gui_event_t *e);
void video_call_idle_back_icon_clicked_cb(void *obj, gui_event_t *e);
void win_video_call_idle_back_clicked_0_cb(void *obj, gui_event_t *e);
void win_video_calling_back_clicked_0_cb(void *obj, gui_event_t *e);
void video_call_idle_time_label_time_update_cb(void *p);
void video_call_calling_time_label_time_update_cb(void *p);

// User-configured timer callback function declarations
void ring_pulse_timer_cb(void *obj);

// Toggle button state callback function declarations
void mic_btn_on_callback(void);
void mic_btn_off_callback(void);
void speaker_btn_on_callback(void);
void speaker_btn_off_callback(void);

#endif // APP_VIDEO_CALL_CALLBACKS_H
