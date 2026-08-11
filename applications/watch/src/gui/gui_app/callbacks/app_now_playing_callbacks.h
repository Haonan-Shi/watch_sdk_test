/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_NOW_PLAYING_CALLBACKS_H
#define APP_NOW_PLAYING_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Event callback function declarations
void now_paly_window_key_0_cb(void *obj, gui_event_t *e);
void now_play_back_window_clicked_0_cb(void *obj, gui_event_t *e);
void toggle_a2dp_sink_mode(void *obj, gui_event_t *e);
void toggle_a2dp_source_mode(void *obj, gui_event_t *e);
void toggle_playback_mode(void *obj, gui_event_t *e);
void hg_time_label_1770799292471_6q7p_time_update_cb(void *p);

#endif // APP_NOW_PLAYING_CALLBACKS_H
