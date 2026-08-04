/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_alarms UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-04-02T09:54:52.696Z
 */
#ifndef APP_ALARMS_UI_H
#define APP_ALARMS_UI_H

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
#include "gui_text.h"
#include "gui_img.h"

// Component handle declarations
extern gui_list_t *alarm_list;
extern gui_rounded_rect_t *alarm_wake_up_bg;
extern gui_text_t *alarm_wake_up_Timetext;
extern gui_text_t *alarm_wake_up_text;
extern gui_img_t *hg_image_1769670275206_mvfs;
extern gui_rounded_rect_t *alarm_alarm_bg;
extern gui_text_t *alarm_alarm_text;
extern gui_text_t *alarm_alarm_Timetext;
extern gui_obj_t *alarm_buttom;
extern gui_img_t *hg_image_1769670641672_8war;
extern gui_img_t *hg_image_1769670646462_ezb4;
extern gui_win_t *alarm_top_window;
extern gui_text_t *hg_time_label_1769669369949_1hub;
extern gui_text_t *hg_label_1769669419002_bcvz;

// Toggle button state management function declarations
extern bool alarm_buttom_get_state(void);
extern void alarm_buttom_set_state(bool state);

#endif // APP_ALARMS_UI_H
