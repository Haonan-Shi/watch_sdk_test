/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * app_contacts UI Implementation (Auto-generated, do not modify manually)
 * Generated at: 2026-07-06T09:11:40.792Z
 */
#include "app_contacts_ui.h"
#include "../callbacks/app_contacts_callbacks.h"
#include "../user/app_contacts_user.h"
#include <stddef.h>
#include <time.h>

// Component handle definitions
gui_list_t *contact_main_list = NULL;
gui_rounded_rect_t *rect_1 = NULL;
gui_text_t *contact_name1 = NULL;
gui_rounded_rect_t *contact_main_list_bg2 = NULL;
gui_text_t *contact_name2 = NULL;
gui_rounded_rect_t *contact_main_list_bg3 = NULL;
gui_text_t *contact_name3 = NULL;
gui_win_t *app_contact_window = NULL;
gui_text_t *hg_time_label_1770691753179_vzi3 = NULL;
gui_text_t *hg_label_1770692164587_14y6 = NULL;
gui_img_t *app_contact_add_icon_bg = NULL;
gui_img_t *app_contact_add_icon = NULL;
gui_img_t *hg_image_1770691020367_567a = NULL;
gui_win_t *app_contacts_c_window = NULL;
gui_text_t *app_contacts_c_time_text = NULL;
gui_circle_t *app_contacts_c_back_icon_bg = NULL;
gui_img_t *app_contacts_c_back_icon = NULL;
gui_circle_t *app_contacts_c_phone_bg = NULL;
gui_circle_t *app_contacts_c_info_bg = NULL;
gui_img_t *app_contacts_c_info = NULL;
gui_img_t *app_contacts_c_phone_icon = NULL;
gui_text_t *contact_name_text = NULL;
gui_circle_t *app_contacts_c_message_bg = NULL;
gui_img_t *app_contacts_c_message_icon = NULL;

// Time string global variables
char hg_time_label_1770691753179_vzi3_time_str[10] = {0};
char app_contacts_c_time_text_time_str[10] = {0};

// List component note_design callback functions
// note_design callback function declaration
static void contact_main_list_note_design(gui_obj_t *obj, void *param);

// note_design callback function implementation
static void contact_main_list_note_design(gui_obj_t *obj, void *param)
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
            // Create rect_1 (hg_rect)
            rect_1 = gui_rect_create((gui_obj_t *)note, "rect_1", 15, 2, 380, 100, 20, gui_rgb(44, 44, 46));
            gui_obj_add_event_cb(rect_1, (gui_event_cb_t)rect_1_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create contact_name1 (hg_label)
            contact_name1 = gui_text_create((gui_obj_t *)note, "contact_name1", 42, 0, 204, 100);
            gui_text_set((gui_text_t *)contact_name1, "Taylor Swift", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         12, 32);
            gui_text_type_set((gui_text_t *)contact_name1, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)contact_name1, MID_LEFT);
            break;
        }
    case 1:
        {
            // Create contact_main_list_bg2 (hg_rect)
            contact_main_list_bg2 = gui_rect_create((gui_obj_t *)note, "contact_main_list_bg2", 15, 0, 380, 100,
                                                    20, gui_rgb(44, 44, 46));
            gui_obj_add_event_cb(contact_main_list_bg2, (gui_event_cb_t)contact_main_list_bg2_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create contact_name2 (hg_label)
            contact_name2 = gui_text_create((gui_obj_t *)note, "contact_name2", 42, 0, 204, 100);
            gui_text_set((gui_text_t *)contact_name2, "Taylor Swift", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         12, 32);
            gui_text_type_set((gui_text_t *)contact_name2, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)contact_name2, MID_LEFT);
            break;
        }
    case 2:
        {
            // Create contact_main_list_bg3 (hg_rect)
            contact_main_list_bg3 = gui_rect_create((gui_obj_t *)note, "contact_main_list_bg3", 15, 0, 380, 100,
                                                    20, gui_rgb(44, 44, 46));
            gui_obj_add_event_cb(contact_main_list_bg3, (gui_event_cb_t)contact_main_list_bg3_clicked_cb,
                                 GUI_EVENT_TOUCH_CLICKED, NULL);
            // Create contact_name3 (hg_label)
            contact_name3 = gui_text_create((gui_obj_t *)note, "contact_name3", 42, 0, 204, 100);
            gui_text_set((gui_text_t *)contact_name3, "Taylor Swift", GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                         12, 32);
            gui_text_type_set((gui_text_t *)contact_name3, "/font/Inter_24pt_Regular_size32_bits4_bitmap.bin",
                              FONT_SRC_FILESYS);
            gui_text_mode_set((gui_text_t *)contact_name3, MID_LEFT);
            break;
        }
    default:
        break;
    }
}


// Create app_contacts_view (hg_view)
static void app_contacts_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_contacts_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create contact_main_list (hg_list)
    contact_main_list = gui_list_create((gui_obj_t *)view, "contact_main_list", 0, 110, 410, 392, 102,
                                        5, VERTICAL, contact_main_list_note_design, NULL, false);
    gui_list_set_style(contact_main_list, LIST_CLASSIC);
    gui_list_set_note_num(contact_main_list, 3);
    gui_list_set_out_scope(contact_main_list, 80);

    // Create app_contact_window (hg_window)
    app_contact_window = gui_win_create((gui_obj_t *)view, "app_contact_window", 0, 0, 410, 110);
    gui_win_enable_blur((gui_win_t *)app_contact_window, true);
    gui_win_set_blur_degree((gui_win_t *)app_contact_window, 225);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(hg_time_label_1770691753179_vzi3_time_str,
                 sizeof(hg_time_label_1770691753179_vzi3_time_str), "%02d:%02d", t->tm_hour, t->tm_min);
    }


    // Create hg_time_label_1770691753179_vzi3 (hg_time_label)
    hg_time_label_1770691753179_vzi3 = gui_text_create(app_contact_window,
                                                       "hg_time_label_1770691753179_vzi3", 300, 20, 80, 40);
    gui_text_set((gui_text_t *)hg_time_label_1770691753179_vzi3,
                 hg_time_label_1770691753179_vzi3_time_str, GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255),
                 strlen(hg_time_label_1770691753179_vzi3_time_str), 28);
    gui_text_type_set((gui_text_t *)hg_time_label_1770691753179_vzi3,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_time_label_1770691753179_vzi3, RIGHT);

    // Create hg_label_1770692164587_14y6 (hg_label)
    hg_label_1770692164587_14y6 = gui_text_create(app_contact_window, "hg_label_1770692164587_14y6",
                                                  280, 63, 100, 34);
    gui_text_set((gui_text_t *)hg_label_1770692164587_14y6, "Contacts", GUI_FONT_SRC_BMP, gui_rgb(255,
                 255, 255), 8, 24);
    gui_text_type_set((gui_text_t *)hg_label_1770692164587_14y6,
                      "/font/Inter_24pt_Regular_size24_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)hg_label_1770692164587_14y6, RIGHT);

    // Create app_contact_add_icon_bg (hg_image)
    app_contact_add_icon_bg = gui_img_create_from_fs(app_contact_window, "app_contact_add_icon_bg",
                                                     "/app_phone/icon_bg.bin", 20, 10, 72, 72);

    // Create app_contact_add_icon (hg_image)
    app_contact_add_icon = gui_img_create_from_fs(app_contact_window, "app_contact_add_icon",
                                                  "/app_phone/phone_add_icon.bin", 36, 26, 40, 40);

    gui_obj_add_event_cb(GUI_BASE(app_contact_window), (gui_event_cb_t)app_contact_window_key_0_cb,
                         GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_contact_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(hg_time_label_1770691753179_vzi3), 30000, true,
                         hg_time_label_1770691753179_vzi3_time_update_cb);
}
GUI_VIEW_INSTANCE("app_contacts_view", false, app_contacts_view_switch_in,
                  app_contacts_view_switch_out, false);

// Create app_contacts_c_view (hg_view)
static void app_contacts_c_view_switch_out(gui_view_t *view)
{
    GUI_UNUSED(view);
}

static void app_contacts_c_view_switch_in(gui_view_t *view)
{
    // Set animation step
    gui_view_set_animate_step(view, 50);

    // Set opacity
    gui_view_set_opacity(view, 255);

    // Set background color
    gui_set_bg_color(gui_rgb(0, 0, 0));

    GUI_UNUSED(view);

    // Initialize time strings
    time_t now = time(NULL);
    struct tm *t = localtime(&now);



    // Create hg_image_1770691020367_567a (hg_image)
    hg_image_1770691020367_567a = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "hg_image_1770691020367_567a", "/app_contacts/Taylor_Swift.bin", 0, 0, 410, 502);

    // Create app_contacts_c_window (hg_window)
    app_contacts_c_window = gui_win_create((gui_obj_t *)view, "app_contacts_c_window", 0, 0, 410, 110);

    // Initialize time strings (using now and t variables declared in view)
    if (t != NULL)
    {
        snprintf(app_contacts_c_time_text_time_str, sizeof(app_contacts_c_time_text_time_str), "%02d:%02d",
                 t->tm_hour, t->tm_min);
    }


    // Create app_contacts_c_time_text (hg_time_label)
    app_contacts_c_time_text = gui_text_create(app_contacts_c_window, "app_contacts_c_time_text", 300,
                                               20, 80, 40);
    gui_text_set((gui_text_t *)app_contacts_c_time_text, app_contacts_c_time_text_time_str,
                 GUI_FONT_SRC_BMP, gui_rgb(255, 255, 255), strlen(app_contacts_c_time_text_time_str), 28);
    gui_text_type_set((gui_text_t *)app_contacts_c_time_text,
                      "/font/Inter_24pt_Regular_size28_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)app_contacts_c_time_text, RIGHT);

    // Create app_contacts_c_back_icon_bg (hg_circle)
    app_contacts_c_back_icon_bg = gui_circle_create(app_contacts_c_window,
                                                    "app_contacts_c_back_icon_bg", 60, 50, 40, gui_rgba(44, 44, 46, 168));
    gui_obj_add_event_cb(app_contacts_c_back_icon_bg,
                         (gui_event_cb_t)app_contacts_c_back_icon_bg_clicked_cb, GUI_EVENT_TOUCH_CLICKED, NULL);

    // Create app_contacts_c_back_icon (hg_image)
    app_contacts_c_back_icon = gui_img_create_from_fs(app_contacts_c_window, "app_contacts_c_back_icon",
                                                      "/app_reminder/reminders_icon5.bin", 48, 27, 25, 47);

    gui_obj_add_event_cb(GUI_BASE(app_contacts_c_window),
                         (gui_event_cb_t)app_contacts_c_window_key_0_cb, GUI_EVENT_KB_SHORT_PRESSED, NULL);
    gui_obj_focus_set((gui_obj_t *)app_contacts_c_window);

    // Create time update timer
    gui_obj_create_timer(GUI_BASE(app_contacts_c_time_text), 30000, true,
                         app_contacts_c_time_text_time_update_cb);

    // Create app_contacts_c_phone_bg (hg_circle)
    app_contacts_c_phone_bg = gui_circle_create((gui_obj_t *)view, "app_contacts_c_phone_bg", 211, 449,
                                                40, gui_rgba(44, 44, 46, 168));

    // Create app_contacts_c_info_bg (hg_circle)
    app_contacts_c_info_bg = gui_circle_create((gui_obj_t *)view, "app_contacts_c_info_bg", 350, 450,
                                               40, gui_rgba(44, 44, 46, 168));

    // Create app_contacts_c_info (hg_image)
    app_contacts_c_info = gui_img_create_from_fs((gui_obj_t *)view, "app_contacts_c_info",
                                                 "/app_phone/phone_etc_icon.bin", 332, 446, 36, 9);

    // Create app_contacts_c_phone_icon (hg_image)
    app_contacts_c_phone_icon = gui_img_create_from_fs((gui_obj_t *)view, "app_contacts_c_phone_icon",
                                                       "/app_phone/phone_icon.bin", 186, 426, 50, 51);

    // Create contact_name_text (hg_label)
    contact_name_text = gui_text_create((gui_obj_t *)view, "contact_name_text", 115, 347, 180, 46);
    gui_text_set((gui_text_t *)contact_name_text, "Taylor Swift", GUI_FONT_SRC_BMP, gui_rgb(255, 255,
                 255), 12, 32);
    gui_text_type_set((gui_text_t *)contact_name_text,
                      "/font/Inter24pt_SemiBold_size32_bits4_bitmap.bin", FONT_SRC_FILESYS);
    gui_text_mode_set((gui_text_t *)contact_name_text, CENTER);

    // Create app_contacts_c_message_bg (hg_circle)
    app_contacts_c_message_bg = gui_circle_create((gui_obj_t *)view, "app_contacts_c_message_bg", 60,
                                                  450, 40, gui_rgba(44, 44, 46, 168));

    // Create app_contacts_c_message_icon (hg_image)
    app_contacts_c_message_icon = gui_img_create_from_fs((gui_obj_t *)view,
                                                         "app_contacts_c_message_icon", "/app_contacts/message_s_icon.bin", 35, 425, 50, 50);
}
GUI_VIEW_INSTANCE("app_contacts_c_view", false, app_contacts_c_view_switch_in,
                  app_contacts_c_view_switch_out, false);
