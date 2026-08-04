/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_MUSIC_CALLBACKS_H
#define APP_MUSIC_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Event callback function declarations
void hg_image_1768283673697_8ra5_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1768283673697_8ra5_key_cb(void *obj, gui_event_t *e);
void hg_image_1768283680575_lql6_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1768283684092_idby_clicked_cb(void *obj, gui_event_t *e);
void hg_image_window_music_ctr_clicked_cb(void *obj, gui_event_t *e);
void hg_window_1768286822421_xea9_key_0_cb(void *obj, gui_event_t *e);
void music_time_ctr_text_key_cb(void *obj, gui_event_t *e);
void music_time_main_text_time_update_cb(void *p);
void music_time_ctr_text_time_update_cb(void *p);

// Toggle button state callback function declarations
void hg_button_1768981147980_ml83_on_callback(void);
void hg_button_1768981147980_ml83_off_callback(void);

#endif // APP_MUSIC_CALLBACKS_H
