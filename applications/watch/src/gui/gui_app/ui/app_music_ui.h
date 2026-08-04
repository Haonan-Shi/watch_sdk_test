/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_music UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-04-02T09:54:52.778Z
 */
#ifndef APP_MUSIC_UI_H
#define APP_MUSIC_UI_H

#include "guidef.h"
#include "gui_obj.h"
#include "gui_components_init.h"
#include "gui_view.h"
#include "gui_view_instance.h"
#include "gui_win.h"
#include "draw_font.h"
#include "font_types.h"
#include "gui_list.h"
#include "gui_rect.h"
#include "gui_img.h"
#include "gui_text.h"
#include "gui_arc.h"

// Component handle declarations
extern gui_list_t *hg_list_1768278317813_ni8t;
extern gui_rounded_rect_t *hg_rect_1768278391413_rzwv;
extern gui_img_t *hg_image_1768278459365_n8mq;
extern gui_text_t *music_homepage_text;
extern gui_rounded_rect_t *hg_rect_1768278391413_rzwv_copy_1768278443156;
extern gui_img_t *hg_image_1768278470059_6op4;
extern gui_text_t *music_broadcast_text;
extern gui_rounded_rect_t *hg_rect_1768278391413_rzwv_copy_1768278443156_copy_1768278446755;
extern gui_img_t *hg_image_1768278479908_yvdh;
extern gui_text_t *music_lib_text;
extern gui_win_t *hg_window_1768286822421_xea9;
extern gui_img_t *hg_image_window_music_ctr;
extern gui_text_t *hg_label_window_music;
extern gui_text_t *music_time_main_text;
extern gui_img_t *hg_image_1768283633747_exth;
extern gui_img_t *hg_image_1768283673697_8ra5;
extern gui_img_t *hg_image_1768283680575_lql6;
extern gui_img_t *hg_image_1768283684092_idby;
extern gui_arc_t *hg_arc_1768283890950_hrp0;
extern gui_text_t *music_time_ctr_text;
extern gui_obj_t *hg_button_1768981147980_ml83;

// Toggle button state management function declarations
extern bool hg_button_1768981147980_ml83_get_state(void);
extern void hg_button_1768981147980_ml83_set_state(bool state);

#endif // APP_MUSIC_UI_H
