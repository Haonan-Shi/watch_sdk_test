/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#ifndef APP_RECORDING_USER_H
#define APP_RECORDING_USER_H

#include <stdbool.h>
#include <stdint.h>
#include "def_event.h"
#include "gui_obj.h"


void recording_start(void *obj, gui_event_t *e);
void recording_stop(void *obj, gui_event_t *e);
void playback_play(void *obj, gui_event_t *e);
void playback_pause(void *obj, gui_event_t *e);

void recording_main_init_cb_impl(void);
void recording_files_init_cb_impl(void);
void recording_playback_init_cb_impl(void);
void recording_timer_tick_impl(void);
void recording_waveform_timer_cb_impl(void);
void playback_timer_tick_impl(void);
// User implementation function declarations
void recording_files_note_design_impl(gui_obj_t *obj);
void recording_files_init_cb_impl(void);
void recording_playback_init_cb_impl(void);
void recording_file_note_clicked(void *obj, gui_event_t *e);

#endif // APP_RECORDING_USER_H
