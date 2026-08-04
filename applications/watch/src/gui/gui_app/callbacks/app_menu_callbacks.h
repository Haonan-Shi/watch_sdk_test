/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_MENU_CALLBACKS_H
#define APP_MENU_CALLBACKS_H

#include "gui_api.h"
#include "gui_text.h"
#include "gui_obj_focus.h"

// Timer animation counters (defined in callbacks.c)
extern uint16_t app_menu_list_timer_cnt;

// Event callback function declarations
void app_menu_list_item_10_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_11_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_12_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_13_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_14_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_15_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_16_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_17_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_18_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_19_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_1_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_20_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_21_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_22_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_23_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_24_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_25_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_26_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_27_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_28_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_29_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_2_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_3_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_4_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_5_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_6_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_7_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_8_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_9_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_intercom_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_music_player_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_list_item_ota_switch_view_cb(void *obj, gui_event_t *e);
void app_menu_view_key_0_cb(void *obj, gui_event_t *e);
void app_menu_window_key_0_cb(void *obj, gui_event_t *e);
void hg_image_1766997222913_68q3_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1766997230487_yrdp_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1766997236346_95l4_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1766997242674_bw37_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1766997251460_wgoz_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1766997263436_j1j0_clicked_cb(void *obj, gui_event_t *e);
void hg_image_1766997276115_kywq_clicked_cb(void *obj, gui_event_t *e);
void hg_image_noise_clicked_cb(void *obj, gui_event_t *e);
void img_13_clicked_cb(void *obj, gui_event_t *e);

// User-configured timer callback function declarations
void app_menu_list_timer_0_cb(void *obj);

// Custom function declarations (auto-extracted from callbacks.c protected area)
void app_menu_list_timer_0_cb(void *obj);

#endif // APP_MENU_CALLBACKS_H
