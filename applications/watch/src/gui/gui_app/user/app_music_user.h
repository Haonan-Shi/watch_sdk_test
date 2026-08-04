/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_MUSIC_USER_H
#define APP_MUSIC_USER_H

#include "../callbacks/app_music_callbacks.h"
#include "../ui/app_music_ui.h"

/**
 * User custom header file
 * This file is generated only once and can be freely modified
 */

// Add custom declarations here
void app_music_play_next(void *obj, gui_event_t *e);
void app_music_play_prev(void *obj, gui_event_t *e);
void app_music_play(void *obj, gui_event_t *e);
void app_music_pause(void *obj, gui_event_t *e);


#endif // APP_MUSIC_USER_H
