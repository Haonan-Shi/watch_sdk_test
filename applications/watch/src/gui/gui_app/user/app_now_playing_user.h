/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_NOW_PLAYING_USER_H
#define APP_NOW_PLAYING_USER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gui_api.h"
#include "../ui/app_now_playing_ui.h"

/**
 * User custom header file
 * This file is generated only once and can be freely modified
 */

void toggle_playback_mode(void *obj, gui_event_t *e);
void toggle_a2dp_sink_mode(void *obj, gui_event_t *e);
void toggle_a2dp_source_mode(void *obj, gui_event_t *e);
void now_playing_view_init_cb_impl(void);
void now_playing_view_reset_cb_impl(void);
void now_playing_view_sync_cb_impl(void *obj);

#ifdef __cplusplus
}
#endif

#endif // APP_NOW_PLAYING_USER_H
