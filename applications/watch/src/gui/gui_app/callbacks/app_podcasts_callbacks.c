/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_podcasts_callbacks.h"
#include "../ui/app_podcasts_ui.h"
#include "../user/app_podcasts_user.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// Time string global variables (defined in UI file)
extern char app_podcast_menu_time_text_time_str[10];
extern char app_podcast_ctr_time_text_time_str[10];
extern char app_podcast_homepage_time_text_time_str[10];

// Event callback function implementations

void podcats_menu_tag0_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_podcast_homepage_view",
                           SWITCH_OUT_TO_LEFT_USE_TRANSLATION, SWITCH_IN_FROM_RIGHT_USE_TRANSLATION);
}

void app_podcast_menu_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void hg_image_window_podcast_ctr_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_podcast_ctr_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void app_podcast_ctr_delete_icon_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_podcast_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void app_podcast_cover_art_key_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void hg_rect_1769592366392_pck6_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_podcast_ctr_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void podcast_homepage_list_bg1_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_podcast_ctr_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void podcast_homepage_list_bg2_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_podcast_ctr_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void app_podcast_homepage_window_key_0_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    // Check key name
    if (strcmp(e->indev_name, "Home") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "SmartWatchTemplateMainView",
                               SWITCH_OUT_ANIMATION_FADE, SWITCH_IN_ANIMATION_FADE);
    }
    else if (strcmp(e->indev_name, "Menu") == 0)
    {
        gui_view_switch_direct(gui_view_get_current(), "app_menu_view", SWITCH_OUT_ANIMATION_FADE,
                               SWITCH_IN_ANIMATION_FADE);
    }
}

void podcast_homepage_return_bg_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_podcast_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void hg_image_window_podcast_homepage_clicked_cb(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
    gui_view_switch_direct(gui_view_get_current(), "app_podcast_ctr_view", SWITCH_OUT_ANIMATION_FADE,
                           SWITCH_IN_ANIMATION_FADE);
}

void app_podcast_menu_time_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(app_podcast_menu_time_text_time_str, sizeof(app_podcast_menu_time_text_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)app_podcast_menu_time_text, app_podcast_menu_time_text_time_str,
                         strlen(app_podcast_menu_time_text_time_str));
}

void app_podcast_ctr_time_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(app_podcast_ctr_time_text_time_str, sizeof(app_podcast_ctr_time_text_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)app_podcast_ctr_time_text, app_podcast_ctr_time_text_time_str,
                         strlen(app_podcast_ctr_time_text_time_str));
}

void app_podcast_homepage_time_text_time_update_cb(void *p)
{
    GUI_UNUSED(p);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL)
    {
        return;
    }

    snprintf(app_podcast_homepage_time_text_time_str, sizeof(app_podcast_homepage_time_text_time_str),
             "%02d:%02d", t->tm_hour, t->tm_min);

    gui_text_content_set((gui_text_t *)app_podcast_homepage_time_text,
                         app_podcast_homepage_time_text_time_str, strlen(app_podcast_homepage_time_text_time_str));
}

// Toggle button state callback functions

/* USER CODE BEGIN app_podcast_ctr_button_on_callback */
/**
 * app_podcast_ctr_button ON state callback
 * Called when button switches to ON state
 */
void app_podcast_ctr_button_on_callback(void)
{
    // TODO: Implement ON state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_play();
}
/* USER CODE END app_podcast_ctr_button_on_callback */

/* USER CODE BEGIN app_podcast_ctr_button_off_callback */
/**
 * app_podcast_ctr_button OFF state callback
 * Called when button switches to OFF state
 */
void app_podcast_ctr_button_off_callback(void)
{
    // TODO: Implement OFF state business logic
    // Hint: Set "Control Target" in button properties to specify control target
    // Example: music_player_pause();
}
/* USER CODE END app_podcast_ctr_button_off_callback */

/* @protected start custom_functions */
// Custom functions
/* @protected end custom_functions */
