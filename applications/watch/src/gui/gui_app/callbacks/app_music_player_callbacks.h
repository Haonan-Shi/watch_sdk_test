/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_MUSIC_PLAYER_CALLBACKS_H
#define APP_MUSIC_PLAYER_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t progress_ring_timer_cnt;

// Event callback function declarations
void app_music_playerPlayerView_key_0_cb(void *obj, gui_event_t *e);
void list_btn_clicked_cb(void *obj, gui_event_t *e);
void play_pause_btn_clicked_cb(void *obj, gui_event_t *e);
void playlist_back_btn_clicked_cb(void *obj, gui_event_t *e);
void playlist_window_key_0_cb(void *obj, gui_event_t *e);
void skip_back_btn_clicked_cb(void *obj, gui_event_t *e);
void skip_forward_btn_clicked_cb(void *obj, gui_event_t *e);
void vol_close_btn_clicked_cb(void *obj, gui_event_t *e);
void vol_minus_btn_clicked_cb(void *obj, gui_event_t *e);
void vol_plus_btn_clicked_cb(void *obj, gui_event_t *e);
void volume_btn_clicked_cb(void *obj, gui_event_t *e);
void win_5_clicked_0_cb(void *obj, gui_event_t *e);
void win_6_clicked_0_cb(void *obj, gui_event_t *e);

// User-configured timer callback function declarations
void music_progress_timer_cb(void *obj);

#endif // APP_MUSIC_PLAYER_CALLBACKS_H
