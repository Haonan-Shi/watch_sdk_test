/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_podcasts UI Definition (Auto-generated, do not modify manually)
 * Generated at: 2026-04-02T09:54:52.827Z
 */
#ifndef APP_PODCASTS_UI_H
#define APP_PODCASTS_UI_H

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
#include "gui_arc.h"

// Component handle declarations
extern gui_list_t *app_podcast_menu_list;
extern gui_rounded_rect_t *podcats_menu_tag0;
extern gui_text_t *app_podcast_homepage_text;
extern gui_img_t *hg_image_1769566502906_2bxr;
extern gui_rounded_rect_t *podcats_menu_tag1;
extern gui_text_t *app_podcast_menu_lib_text;
extern gui_img_t *hg_image_1769566513496_qv6d;
extern gui_rounded_rect_t *podcats_menu_tag2;
extern gui_text_t *app_podcast_menu_search_text;
extern gui_img_t *hg_image_1769566525433_jtg5;
extern gui_win_t *app_podcast_menu_window;
extern gui_img_t *hg_image_window_podcast_ctr;
extern gui_text_t *app_podcast_menu_text0;
extern gui_text_t *app_podcast_menu_time_text;
extern gui_img_t *app_podcast_ctr_delete_icon;
extern gui_arc_t *app_podcast_ctr_arc;
extern gui_text_t *app_podcast_ctr_time_text;
extern gui_obj_t *app_podcast_ctr_button;
extern gui_img_t *app_podcast_cover_art;
extern gui_img_t *hg_image_1769564084391_reur;
extern gui_img_t *hg_image_1769564088845_muwb;
extern gui_text_t *hg_label_1769564186120_0wdb;
extern gui_text_t *hg_label_1769564290281_kezn;
extern gui_list_t *podcast_homepage_list;
extern gui_rounded_rect_t *hg_rect_1769592366392_pck6;
extern gui_img_t *podcast_homepage_list0_covert;
extern gui_img_t *podcast_homepage_list0_iconbg;
extern gui_img_t *podcast_homepage_list0_icon;
extern gui_text_t *podcast_homepage_list0_text0;
extern gui_text_t *podcast_homepage_list0_text1;
extern gui_rounded_rect_t *podcast_homepage_list_bg1;
extern gui_img_t *podcast_homepage_list1_iconbg;
extern gui_img_t *podcast_homepage_list1_icon;
extern gui_img_t *podcast_homepage_list1_covert;
extern gui_text_t *podcast_homepage_list1_text0;
extern gui_text_t *podcast_homepage_list1_text1;
extern gui_rounded_rect_t *podcast_homepage_list_bg2;
extern gui_img_t *podcast_homepage_list2_covert;
extern gui_img_t *podcast_homepage_list2_iconbg;
extern gui_img_t *podcast_homepage_list2_icon;
extern gui_text_t *podcast_homepage_list2_text0;
extern gui_text_t *podcast_homepage_list2_text1;
extern gui_win_t *app_podcast_homepage_window;
extern gui_text_t *app_podcast_homepage_text0;
extern gui_img_t *podcast_homepage_return_bg;
extern gui_img_t *podcast_homepage_return_icon;
extern gui_text_t *app_podcast_homepage_time_text;
extern gui_img_t *hg_image_window_podcast_homepage;

// Toggle button state management function declarations
extern bool app_podcast_ctr_button_get_state(void);
extern void app_podcast_ctr_button_set_state(bool state);

#endif // APP_PODCASTS_UI_H
