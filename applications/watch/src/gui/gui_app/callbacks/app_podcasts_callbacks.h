/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_PODCASTS_CALLBACKS_H
#define APP_PODCASTS_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Event callback function declarations
void app_podcast_cover_art_key_cb(void *obj, gui_event_t *e);
void app_podcast_ctr_delete_icon_clicked_cb(void *obj, gui_event_t *e);
void app_podcast_homepage_window_key_0_cb(void *obj, gui_event_t *e);
void app_podcast_menu_window_key_0_cb(void *obj, gui_event_t *e);
void hg_image_window_podcast_ctr_clicked_cb(void *obj, gui_event_t *e);
void hg_image_window_podcast_homepage_clicked_cb(void *obj, gui_event_t *e);
void hg_rect_1769592366392_pck6_clicked_cb(void *obj, gui_event_t *e);
void podcast_homepage_list_bg1_clicked_cb(void *obj, gui_event_t *e);
void podcast_homepage_list_bg2_clicked_cb(void *obj, gui_event_t *e);
void podcast_homepage_return_bg_clicked_cb(void *obj, gui_event_t *e);
void podcats_menu_tag0_clicked_cb(void *obj, gui_event_t *e);
void app_podcast_menu_time_text_time_update_cb(void *p);
void app_podcast_ctr_time_text_time_update_cb(void *p);
void app_podcast_homepage_time_text_time_update_cb(void *p);

// Toggle button state callback function declarations
void app_podcast_ctr_button_on_callback(void);
void app_podcast_ctr_button_off_callback(void);

#endif // APP_PODCASTS_CALLBACKS_H
