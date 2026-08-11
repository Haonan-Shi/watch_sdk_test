/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_NOISE_USER_H
#define APP_NOISE_USER_H

#include "../callbacks/app_noise_callbacks.h"
#include "../ui/app_noise_ui.h"

/**
 * User custom header file
 * This file is generated only once and can be freely modified
 */

// Add custom declarations here
void app_noise_init(void);
void noise_simulation_timer_cb(void *obj);
void noise_display_timer_cb(void *obj);
void update_noise_meters(int noise_db);
gui_color_t get_noise_color(int noise_db);
void update_noise_status(int noise_db);
void app_noise_set_level(int noise_db);
int app_noise_get_level(void);

#endif // APP_NOISE_USER_H
