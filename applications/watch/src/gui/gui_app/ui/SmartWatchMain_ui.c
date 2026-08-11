/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * SmartWatchMain UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:41.020Z
 */
#include "SmartWatchMain_ui.h"
#include "../callbacks/SmartWatchMain_callbacks.h"
#include "../user/SmartWatchMain_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_win_t *samrtWatch_window = NULL;
gui_rounded_rect_t *hg_rect_1766998925528_70wq = NULL;
gui_text_t *hg_label_point = NULL;
gui_text_t *hg_time_label_hh = NULL;
gui_text_t *hg_time_label_mm = NULL;
gui_rounded_rect_t *hg_rect = NULL;
gui_win_t *win_clock_big = NULL;
gui_img_t *hg_image_1772764949914_bwov = NULL;
gui_text_t *hg_time_label_1772765275313_pgx4 = NULL;
gui_text_t *text_date_big = NULL;
gui_list_t *list_card = NULL;
gui_img_t *bottom_View_weather = NULL;
gui_img_t *hg_image_1769134788793_og20 = NULL;
gui_text_t *bottomView_stand_text = NULL;
gui_arc_t *hg_arc_1768182825159_oh9l = NULL;
gui_arc_t *hg_arc_1768182974828_8nzt = NULL;
gui_arc_t *hg_arc_1768183009513_sibl = NULL;
gui_arc_t *hg_arc_1768183125777_bzej = NULL;
gui_arc_t *hg_arc_1768183158146_qutj = NULL;
gui_arc_t *hg_arc_1768183198408_5dw5 = NULL;
gui_text_t *bottomView_ext_text = NULL;
gui_text_t *bottomView_work_text = NULL;
gui_img_t *bottom_View_tag_bg1 = NULL;
gui_img_t *hg_image_1768183941920_4wxc = NULL;
gui_img_t *hg_image_1768184009679_rg6a = NULL;
gui_circle_t *hg_circle_1768184067471_7e0p = NULL;
gui_arc_t *hg_arc_1768184160265_b289 = NULL;
gui_arc_t *hg_arc_1768184103087_n36y = NULL;
gui_text_t *bottomView_charge_text = NULL;
gui_img_t *bottom_View_tag_bg_menu = NULL;
gui_img_t *hg_image_1768197237671_qmtd = NULL;
gui_text_t *bottomView_menu_text = NULL;
gui_win_t *win_clock_small = NULL;
gui_img_t *round_rect_bg = NULL;
gui_text_t *hg_time_label_1772765275313_pgx4_copy_1772765661189_2 = NULL;
gui_text_t *text_date_small = NULL;
gui_list_t *hg_list_1766999250662_ne76 = NULL;
gui_rounded_rect_t *hg_rect_1772778302433_yeu0 = NULL;
gui_text_t *hg_label_1768961345159_3hp5 = NULL;
gui_img_t *hg_image_1769134863025_7s7k = NULL;
gui_img_t *hg_image_1766999553786_6n5b = NULL;
gui_text_t *hg_label_activity = NULL;
gui_text_t *hg_label_activity_time = NULL;
gui_img_t *hg_image_1769134886591_ma8n = NULL;
gui_img_t *hg_image_1767000056743_qysv = NULL;
gui_text_t *hg_label_music = NULL;
gui_text_t *hg_label_activity_time_copy_1768961794007 = NULL;

// Time string global variables
char hg_time_label_hh_time_str[4] = {0};
char hg_time_label_mm_time_str[4] = {0};
char hg_time_label_1772765275313_pgx4_time_str[10] = {0};
char hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void list_card_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void list_card_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create bottom_View_weather (hg_image)
            bottom_View_weather = gui_img_create_from_fs((gui_obj_t *)note, "bottom_View_weather",
                                                         "/weather/ui_card_weather.bin", 0, 0, 352, 157);
            gui_obj_add_event_cb(bottom_View_weather, (gui_event_cb_t)bottom_View_weather_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            break;
        }
    case 1:
        {
            // Create hg_image_1769134788793_og20 (hg_image)
            hg_image_1769134788793_og20 = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769134788793_og20", "/tab_bg/bottom_tab_bg.bin", 0, 0, 352, 157);
            gui_obj_add_event_cb(hg_image_1769134788793_og20,
                                 (gui_event_cb_t)hg_image_1769134788793_og20_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create bottomView_stand_text (hg_label)
            bottomView_stand_text = gui_text_create((gui_obj_t *)note, "bottomView_stand_text", 178, 88, 165,
                                                    42);
            gui_text_set((gui_text_t *)bottomView_stand_text, "5/6 h", GUI_FONT_SRC_BMP, gui_rgb(0, 237, 255),
                         5, 26);
            gui_text_type_set((gui_text_t *)bottomView_stand_text,
                              "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bottomView_stand_text, LEFT);
            // Create hg_arc_1768182825159_oh9l (hg_arc)
            hg_arc_1768182825159_oh9l = gui_arc_create((gui_obj_t *)note, "hg_arc_1768182825159_oh9l", 80, 79,
                                                       50, 0, 360, 8, gui_rgb(58, 23, 29));
            // Create hg_arc_1768182974828_8nzt (hg_arc)
            hg_arc_1768182974828_8nzt = gui_arc_create((gui_obj_t *)note, "hg_arc_1768182974828_8nzt", 80, 79,
                                                       50, 270, 60, 8, gui_rgb(250, 17, 79));
            // Create hg_arc_1768183009513_sibl (hg_arc)
            hg_arc_1768183009513_sibl = gui_arc_create((gui_obj_t *)note, "hg_arc_1768183009513_sibl", 80, 79,
                                                       40, 0, 360, 8, gui_rgb(30, 55, 25));
            // Create hg_arc_1768183125777_bzej (hg_arc)
            hg_arc_1768183125777_bzej = gui_arc_create((gui_obj_t *)note, "hg_arc_1768183125777_bzej", 80, 79,
                                                       40, 270, 30, 8, gui_rgb(146, 255, 1));
            // Create hg_arc_1768183158146_qutj (hg_arc)
            hg_arc_1768183158146_qutj = gui_arc_create((gui_obj_t *)note, "hg_arc_1768183158146_qutj", 80, 79,
                                                       30, 0, 360, 8, gui_rgb(22, 50, 47));
            // Create hg_arc_1768183198408_5dw5 (hg_arc)
            hg_arc_1768183198408_5dw5 = gui_arc_create((gui_obj_t *)note, "hg_arc_1768183198408_5dw5", 80, 79,
                                                       30, 270, 180, 8, gui_rgb(0, 237, 255));
            // Create bottomView_ext_text (hg_label)
            bottomView_ext_text = gui_text_create((gui_obj_t *)note, "bottomView_ext_text", 178, 21, 165, 42);
            gui_text_set((gui_text_t *)bottomView_ext_text, "208/500 kcol", GUI_FONT_SRC_BMP, gui_rgb(250, 17,
                         79), 12, 26);
            gui_text_type_set((gui_text_t *)bottomView_ext_text,
                              "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bottomView_ext_text, LEFT);
            // Create bottomView_work_text (hg_label)
            bottomView_work_text = gui_text_create((gui_obj_t *)note, "bottomView_work_text", 178, 53, 165, 42);
            gui_text_set((gui_text_t *)bottomView_work_text, "10/60 min", GUI_FONT_SRC_BMP, gui_rgb(146, 255,
                         1), 9, 26);
            gui_text_type_set((gui_text_t *)bottomView_work_text,
                              "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bottomView_work_text, LEFT);
            break;
        }
    case 2:
        {
            // Create bottom_View_tag_bg1 (hg_image)
            bottom_View_tag_bg1 = gui_img_create_from_fs((gui_obj_t *)note, "bottom_View_tag_bg1",
                                                         "/tab_bg/bottom_tab_bg.bin", 0, 0, 352, 157);
            // Create hg_image_1768183941920_4wxc (hg_image)
            hg_image_1768183941920_4wxc = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768183941920_4wxc", "/UI_app_list/app_music_icon.bin", 21, 29, 100, 100);
            gui_img_scale((gui_img_t *)hg_image_1768183941920_4wxc, 0.850000f, 0.850000f);
            gui_obj_add_event_cb(hg_image_1768183941920_4wxc,
                                 (gui_event_cb_t)hg_image_1768183941920_4wxc_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_image_1768184009679_rg6a (hg_image)
            hg_image_1768184009679_rg6a = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768184009679_rg6a", "/UI_app_list/app_message_icon.bin", 244, 29, 100, 100);
            gui_img_scale((gui_img_t *)hg_image_1768184009679_rg6a, 0.850000f, 0.850000f);
            gui_obj_add_event_cb(hg_image_1768184009679_rg6a,
                                 (gui_event_cb_t)hg_image_1768184009679_rg6a_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_circle_1768184067471_7e0p (hg_circle)
            hg_circle_1768184067471_7e0p = gui_circle_create((gui_obj_t *)note, "hg_circle_1768184067471_7e0p",
                                                             174, 69, 40, gui_rgb(0, 0, 0));
            // Create hg_arc_1768184160265_b289 (hg_arc)
            hg_arc_1768184160265_b289 = gui_arc_create((gui_obj_t *)note, "hg_arc_1768184160265_b289", 174, 69,
                                                       34, 0, 360, 8, gui_rgb(91, 125, 89));
            // Create hg_arc_1768184103087_n36y (hg_arc)
            hg_arc_1768184103087_n36y = gui_arc_create((gui_obj_t *)note, "hg_arc_1768184103087_n36y", 174, 69,
                                                       34, 270, 185, 8, gui_rgb(58, 195, 40));
            gui_obj_add_event_cb(hg_arc_1768184103087_n36y,
                                 (gui_event_cb_t)hg_arc_1768184103087_n36y_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create bottomView_charge_text (hg_label)
            bottomView_charge_text = gui_text_create((gui_obj_t *)note, "bottomView_charge_text", 159, 56, 40,
                                                     42);
            gui_text_set((gui_text_t *)bottomView_charge_text, "77", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         2, 32);
            gui_text_type_set((gui_text_t *)bottomView_charge_text,
                              "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bottomView_charge_text, LEFT);
            break;
        }
    case 3:
        {
            // Create bottom_View_tag_bg_menu (hg_image)
            bottom_View_tag_bg_menu = gui_img_create_from_fs((gui_obj_t *)note, "bottom_View_tag_bg_menu",
                                                             "/tab_bg/bottom_list_menu_bg.bin", 47, 32, 258, 76);
            gui_obj_add_event_cb(bottom_View_tag_bg_menu, (gui_event_cb_t)bottom_View_tag_bg_menu_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create hg_image_1768197237671_qmtd (hg_image)
            hg_image_1768197237671_qmtd = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1768197237671_qmtd", "/UI_app_list/cellular_icon.bin", 79, 46, 48, 49);
            // Create bottomView_menu_text (hg_label)
            bottomView_menu_text = gui_text_create((gui_obj_t *)note, "bottomView_menu_text", 140, 60, 200, 34);
            gui_text_set((gui_text_t *)bottomView_menu_text, "APP MENU", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), 8, 24);
            gui_text_type_set((gui_text_t *)bottomView_menu_text,
                              "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)bottomView_menu_text, LEFT);
            break;
        }
    default:
        break;
    }
}

// note_design callback function declaration
static void hg_list_1766999250662_ne76_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void hg_list_1766999250662_ne76_note_design(gui_obj_t *obj, void *param)
{
    GUI_UNUSED(param);

    // Cast obj to gui_list_note_t * type
    gui_list_note_t *note = (gui_list_note_t *)obj;
    uint16_t index = note->index;

    // Create different list_item content based on index
    switch (index)
    {
    case 0:
        {
            // Create hg_rect_1772778302433_yeu0 (hg_rect)
            hg_rect_1772778302433_yeu0 = gui_rect_create((gui_obj_t *)note, "hg_rect_1772778302433_yeu0", 46,
                                                         35, 260, 80, 40, gui_rgb(127, 127, 127));
            // Create hg_label_1768961345159_3hp5 (hg_label)
            hg_label_1768961345159_3hp5 = gui_text_create((gui_obj_t *)note, "hg_label_1768961345159_3hp5", 46,
                                                          35, 260, 80);
            gui_text_set((gui_text_t *)hg_label_1768961345159_3hp5, "Clear All", GUI_FONT_SRC_BMP, gui_rgb(255,
                         255, 255), 9, 28);
            gui_text_type_set((gui_text_t *)hg_label_1768961345159_3hp5,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_1768961345159_3hp5, MID_CENTER);
            break;
        }
    case 1:
        {
            // Create hg_image_1769134863025_7s7k (hg_image)
            hg_image_1769134863025_7s7k = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769134863025_7s7k", "/tab_bg/top_tab_bg.bin", 0, 25, 352, 120);
            // Create hg_image_1766999553786_6n5b (hg_image)
            hg_image_1766999553786_6n5b = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1766999553786_6n5b", "/UI_app_list/app_activity_icon.bin", 18, 0, 100, 106);
            gui_img_scale((gui_img_t *)hg_image_1766999553786_6n5b, 0.800000f, 0.800000f);
            // Create hg_label_activity (hg_label)
            hg_label_activity = gui_text_create((gui_obj_t *)note, "hg_label_activity", 30, 95, 199, 46);
            gui_text_set((gui_text_t *)hg_label_activity, "Come On", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         7, 26);
            gui_text_type_set((gui_text_t *)hg_label_activity,
                              "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_activity, LEFT);
            // Create hg_label_activity_time (hg_label)
            hg_label_activity_time = gui_text_create((gui_obj_t *)note, "hg_label_activity_time", -10, 29, 352,
                                                     44);
            gui_text_set((gui_text_t *)hg_label_activity_time, "45 minute ago", GUI_FONT_SRC_BMP, gui_rgb(200,
                         200, 200), 13, 28);
            gui_text_type_set((gui_text_t *)hg_label_activity_time,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_activity_time, RIGHT);
            break;
        }
    case 2:
        {
            // Create hg_image_1769134886591_ma8n (hg_image)
            hg_image_1769134886591_ma8n = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1769134886591_ma8n", "/tab_bg/top_tab_bg.bin", 0, 25, 352, 120);
            // Create hg_image_1767000056743_qysv (hg_image)
            hg_image_1767000056743_qysv = gui_img_create_from_fs((gui_obj_t *)note,
                                                                 "hg_image_1767000056743_qysv", "/UI_app_list/app_music_icon.bin", 18, 0, 100, 100);
            gui_img_scale((gui_img_t *)hg_image_1767000056743_qysv, 0.800000f, 0.800000f);
            // Create hg_label_music (hg_label)
            hg_label_music = gui_text_create((gui_obj_t *)note, "hg_label_music", 30, 92, 108, 46);
            gui_text_set((gui_text_t *)hg_label_music, "Music", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                         26);
            gui_text_type_set((gui_text_t *)hg_label_music, "/font/Inter_24pt_Regular_size26_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_music, LEFT);
            // Create hg_label_activity_time_copy_1768961794007 (hg_label)
            hg_label_activity_time_copy_1768961794007 = gui_text_create((gui_obj_t *)note,
                                                                        "hg_label_activity_time_copy_1768961794007", -10, 35, 352, 44);
            gui_text_set((gui_text_t *)hg_label_activity_time_copy_1768961794007, "1 hour ago",
                         GUI_FONT_SRC_BMP, gui_rgb(200, 200, 200), 10, 28);
            gui_text_type_set((gui_text_t *)hg_label_activity_time_copy_1768961794007,
                              "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)hg_label_activity_time_copy_1768961794007, RIGHT);
            break;
        }
    default:
        break;
    }
}


// Create SmartWatchTemplateMainView (hg_view)
static void SmartWatchTemplateMainView_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void SmartWatchTemplateMainView_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    gui_view_switch_on_event(view, "app_top_view", SWITCH_OUT_STILL_USE_BLUR,
                             SWITCH_IN_FROM_TOP_USE_TRANSLATION, GUI_EVENT_TOUCH_MOVE_DOWN);
    gui_view_switch_on_event(view, "app_bottom_view", SWITCH_INIT_STATE,
                             SWITCH_IN_FROM_BOTTOM_USE_TRANSLATION, GUI_EVENT_TOUCH_MOVE_UP);
    gui_view_switch_on_event(view, "watchface_select_view", SWITCH_OUT_ANIMATION_FADE,
                             SWITCH_IN_ANIMATION_FADE, GUI_EVENT_TOUCH_LONG);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t != NULL)
    {
        snprintf(hg_time_label_hh_time_str, sizeof(hg_time_label_hh_time_str), "%02d", t->tm_hour);
        snprintf(hg_time_label_mm_time_str, sizeof(hg_time_label_mm_time_str), "%02d", t->tm_min);
    }



    // Create samrtWatch_window (hg_window)
    samrtWatch_window = gui_win_create((gui_obj_t *)view, "samrtWatch_window", 0, 0, 410, 110);

    gui_obj_add_event_cb(GUI_BASE(samrtWatch_window), (gui_event_cb_t)samrtWatch_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)samrtWatch_window);

    // Create hg_rect_1766998925528_70wq (hg_rect)
    hg_rect_1766998925528_70wq = gui_rect_create((gui_obj_t *)view, "hg_rect_1766998925528_70wq", 0, 0,
                                                 410, 502, 0, gui_rgb(222, 121, 148));
    gui_obj_hidden((gui_obj_t *)hg_rect_1766998925528_70wq, true);

    // Create hg_label_point (hg_label)
    hg_label_point = gui_text_create((gui_obj_t *)view, "hg_label_point", 25, 211, 100, 350);
    gui_text_set((gui_text_t *)hg_label_point, ":", GUI_FONT_SRC_BMP, gui_rgb(254, 249, 116), 1, 250);
    gui_text_type_set((gui_text_t *)hg_label_point, "/font/Inter_24pt_Regular_size250_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_point, LEFT);

    // Create hg_time_label_hh (hg_time_label)
    hg_time_label_hh = gui_text_create((gui_obj_t *)view, "hg_time_label_hh", 50, 0, 360, 300);
    gui_text_set((gui_text_t *)hg_time_label_hh, hg_time_label_hh_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(254, 249, 116), strlen(hg_time_label_hh_time_str), 250);
    gui_text_type_set((gui_text_t *)hg_time_label_hh,
                      "/font/Inter_24pt_Regular_size250_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_hh, RIGHT);

    // Create hg_time_label_mm (hg_time_label)
    hg_time_label_mm = gui_text_create((gui_obj_t *)view, "hg_time_label_mm", 50, 225, 360, 300);
    gui_text_set((gui_text_t *)hg_time_label_mm, hg_time_label_mm_time_str, GUI_FONT_SRC_BMP,
                 gui_rgb(254, 249, 116), strlen(hg_time_label_mm_time_str), 250);
    gui_text_type_set((gui_text_t *)hg_time_label_mm,
                      "/font/Inter_24pt_Regular_size250_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_mm, MID_RIGHT);

    gui_obj_add_event_cb((gui_obj_t *)view, (gui_event_cb_t)SmartWatchTemplateMainView_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)view);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_hh), 30000, true, hg_time_label_hh_time_update_cb);
    gui_obj_create_timer(GUI_BASE(hg_time_label_mm), 30000, true, hg_time_label_mm_time_update_cb);
}
GUI_VIEW_INSTANCE("SmartWatchTemplateMainView", false, SmartWatchTemplateMainView_switch_in,
                  SmartWatchTemplateMainView_switch_out, false);

// Create app_bottom_view (hg_view)
static void app_bottom_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_bottom_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    gui_view_switch_on_event(view, "SmartWatchTemplateMainView", SWITCH_OUT_TO_BOTTOM_USE_TRANSLATION,
                             SWITCH_INIT_STATE, GUI_EVENT_TOUCH_MOVE_DOWN);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create hg_rect (hg_rect)
    hg_rect = gui_rect_create((gui_obj_t *)view, "hg_rect", 0, 0, 410, 502, 0, gui_rgb(222, 121, 148));

    // Create win_clock_big (hg_window)
    win_clock_big = gui_win_create((gui_obj_t *)view, "win_clock_big", 38, 30, 372, 190);
    // Bind timer: Animation 1
    gui_obj_create_timer((gui_obj_t *)win_clock_big, 10, true, win_clock_big_timer_0_cb);
    gui_obj_start_timer((gui_obj_t *)win_clock_big);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1772765275313_pgx4_time_str,
                 sizeof(hg_time_label_1772765275313_pgx4_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_image_1772764949914_bwov (hg_image)
    hg_image_1772764949914_bwov = gui_img_create_from_fs(win_clock_big, "hg_image_1772764949914_bwov",
                                                         "/others/ui_card_clockcircle.bin", 163, 0, 190, 190);

    // Create hg_time_label_1772765275313_pgx4 (hg_time_label)
    hg_time_label_1772765275313_pgx4 = gui_text_create(win_clock_big,
                                                       "hg_time_label_1772765275313_pgx4", 163, 0, 190, 190);
    gui_text_set((gui_text_t *)hg_time_label_1772765275313_pgx4,
                 hg_time_label_1772765275313_pgx4_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1772765275313_pgx4_time_str), 56);
    gui_text_type_set((gui_text_t *)hg_time_label_1772765275313_pgx4,
                      "/font/Inter24pt_Medium_size56_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1772765275313_pgx4, MID_CENTER);
    gui_text_extra_letter_spacing_set((gui_text_t *)hg_time_label_1772765275313_pgx4, 3);

    // Create text_date_big (hg_label)
    text_date_big = gui_text_create(win_clock_big, "text_date_big", 0, 22, 200, 168);
    gui_text_set((gui_text_t *)text_date_big, "FRI\nMAR\n6", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 9, 42);
    gui_text_type_set((gui_text_t *)text_date_big, "/font/Inter24pt_Medium_size42_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)text_date_big, MULTI_LEFT);
    gui_text_extra_line_spacing_set((gui_text_t *)text_date_big, 4);
    // Bind timer:
    gui_obj_create_timer((gui_obj_t *)text_date_big, 1000, true, text_date_big_timer_0_cb);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1772765275313_pgx4), 30000, true,
                         hg_time_label_1772765275313_pgx4_time_update_cb);

    // Create list_card (hg_list)
    list_card = gui_list_create((gui_obj_t *)view, "list_card", 29, 0, 352, 502, 157, 10, VERTICAL,
                                list_card_note_design, NULL, false);
    gui_list_set_style(list_card, LIST_CARD);
    gui_list_set_note_num(list_card, 4);
    gui_list_set_offset(list_card, 250);
    gui_list_set_card_stack_location(list_card, 20);

    // Create win_clock_small (hg_window)
    win_clock_small = gui_win_create((gui_obj_t *)view, "win_clock_small", 0, 0, 410, 60);
    gui_win_enable_blur((gui_win_t *)win_clock_small, true);
    gui_win_set_blur_degree((gui_win_t *)win_clock_small, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str,
                 sizeof(hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str), "%02d:%02d", t->tm_hour,
                 t->tm_min);
    }


    // Create round_rect_bg (hg_image)
    round_rect_bg = gui_img_create_from_fs(win_clock_small, "round_rect_bg",
                                           "/others/win_clock_small_bg.bin", 35, 0, 340, 60);

    // Create hg_time_label_1772765275313_pgx4_copy_1772765661189_2 (hg_time_label)
    hg_time_label_1772765275313_pgx4_copy_1772765661189_2 = gui_text_create(win_clock_small,
                                                                            "hg_time_label_1772765275313_pgx4_copy_1772765661189_2", -10, 8, 375, 60);
    gui_text_set((gui_text_t *)hg_time_label_1772765275313_pgx4_copy_1772765661189_2,
                 hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                         255), strlen(hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_str), 24);
    gui_text_type_set((gui_text_t *)hg_time_label_1772765275313_pgx4_copy_1772765661189_2,
                      "/font/Inter24pt_Medium_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1772765275313_pgx4_copy_1772765661189_2, MID_RIGHT);

    // Create text_date_small (hg_label)
    text_date_small = gui_text_create(win_clock_small, "text_date_small", 50, 8, 410, 60);
    gui_text_set((gui_text_t *)text_date_small, "FRI 6", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), 5,
                 24);
    gui_text_type_set((gui_text_t *)text_date_small, "/font/Inter24pt_Medium_size24_bits4_bitmap.bin",
                      FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)text_date_small, MULTI_MID_LEFT);
    gui_text_extra_line_spacing_set((gui_text_t *)text_date_small, 10);
    // Bind timer:
    gui_obj_create_timer((gui_obj_t *)text_date_small, 1000, true, text_date_small_timer_0_cb);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1772765275313_pgx4_copy_1772765661189_2), 30000, true,
                         hg_time_label_1772765275313_pgx4_copy_1772765661189_2_time_update_cb);
}
GUI_VIEW_INSTANCE("app_bottom_view", false, app_bottom_view_switch_in, app_bottom_view_switch_out,
                  false);

// Create app_top_view (hg_view)
static void app_top_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_top_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    gui_view_switch_on_event(view, "SmartWatchTemplateMainView", SWITCH_OUT_TO_TOP_USE_TRANSLATION,
                             SWITCH_IN_STILL_USE_BLUR, GUI_EVENT_TOUCH_MOVE_UP);


    // Create hg_list_1766999250662_ne76 (hg_list)
    hg_list_1766999250662_ne76 = gui_list_create((gui_obj_t *)view, "hg_list_1766999250662_ne76", 29, 0,
                                                 352, 491, 157, 10, VERTICAL, hg_list_1766999250662_ne76_note_design, NULL, false);
    gui_list_set_style(hg_list_1766999250662_ne76, LIST_CLASSIC);
    gui_list_set_note_num(hg_list_1766999250662_ne76, 3);
}
GUI_VIEW_INSTANCE("app_top_view", false, app_top_view_switch_in, app_top_view_switch_out, false);
