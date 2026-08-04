/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_OTA_USER_H
#define APP_OTA_USER_H

#include "def_event.h"

/* Event callbacks (triggered from HML callFunction) */
void ota_retry(void *obj, gui_event_t *e);
void ota_reset_to_ready(void *obj, gui_event_t *e);

/* Timer _impl callbacks (weak-linked from generated callbacks.c) */
void ota_starting_timer_cb_impl(void);
void ota_progress_tick_cb_impl(void);

#endif /* APP_OTA_USER_H */
