/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_TIMER_USER_H
#define APP_TIMER_USER_H

#include "../callbacks/app_timer_callbacks.h"
#include "../ui/app_timer_ui.h"

/**
 * User-defined header file
 * This file is generated once only, feel free to modify
 */

// Add custom declarations here

// List note_design function declarations
extern uint32_t preset_timer_val[12];
extern uint32_t active_timer_rec_val;
extern uint8_t preset_index;
extern char active_timer_rec_str[10];
extern char preset_timer_str[10];

void click_button_page_all_timers(void *obj, gui_event_t *e);
void click_button_play_stop(void *obj, gui_event_t *e);

void update_timer_str(char *str, uint32_t rec_val, uint32_t act_val);

#endif // APP_TIMER_USER_H
