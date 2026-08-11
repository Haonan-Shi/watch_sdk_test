/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_VIDEO_CALL_USER_H
#define APP_VIDEO_CALL_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gui_api.h"

// Function declarations
void mic_toggle(void *obj, gui_event_t *e);
void speaker_toggle(void *obj, gui_event_t *e);
void hangup_reset(void *obj, gui_event_t *e);
void video_call_calling_view_init_cb_impl(void);
void ring_pulse_timer_cb_impl(void);

// Video call control functions
void video_call_start(void *obj, gui_event_t *e);
void video_call_end(void *obj, gui_event_t *e);

// Video stream message handlers
void video_call_update_stream(gui_obj_t *obj, const char *topic, void *data, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif // APP_VIDEO_CALL_USER_H
