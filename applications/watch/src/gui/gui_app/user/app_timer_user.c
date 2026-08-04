/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_timer_user.h"

/**
 * User-defined implementation
 * This file is generated once only, feel free to modify
 */

// Add custom implementations here

/***
 * Template function
 * Distinguish development environments
 */
// void user_defined_func_called_by_event(void *obj, gui_event_t *e)
// {
//     GUI_UNUSED(obj);
//     GUI_UNUSED(e);
// #ifdef _HONEYGUI_SIMULATOR_
//     // TODO
// #else
//     // TODO
// #endif
// }

// void user_defined_func_called_by_msg(gui_obj_t *obj, const char *topic, void *data, uint16_t len)
// {
//     GUI_UNUSED(obj);
//     GUI_UNUSED(topic);
//     GUI_UNUSED(data);
//     GUI_UNUSED(len);
// #ifdef _HONEYGUI_SIMULATOR_
//     // TODO
// #else
//     // TODO
// #endif
// }

// void list_note_design(gui_obj_t *obj, void *param)
// {
//     GUI_UNUSED(param);
//     // Cast obj to gui_list_note_t * type
//     gui_list_note_t *note = (gui_list_note_t *)obj;
//     uint16_t index = note->index;
//     GUI_UNUSED(index);
// }
uint32_t preset_timer_val[12] = {60, 180, 300, 600, 900, 1800, 3600, 7200, 10800};
uint8_t preset_index = 0;
uint32_t active_timer_rec_val = 0;
char active_timer_rec_str[10];
char preset_timer_str[10];

void update_timer_str(char *str, uint32_t rec_val, uint32_t act_val)
{
    uint8_t min = (rec_val % 3600) / 60;
    uint8_t sec = rec_val % 60;
    if (act_val >= 3600)
    {
        uint8_t hour = rec_val / 3600;
        sprintf(str, "%02u:%02u:%02u", hour, min, sec);
    }
    else
    {
        sprintf(str, "%02u:%02u", min, sec);
    }
}
void click_button_page_all_timers(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
#ifdef _HONEYGUI_SIMULATOR_
    // TODO
#else
    // TODO
#endif
    gui_list_note_t *note = (void *)(GUI_BASE(obj)->parent);
    gui_obj_t *o = obj;
    int16_t index = (note->index - 1) * 2;
    if (o->x == 210) { index += 1; }
    preset_index = index;
    active_timer_rec_val = preset_timer_val[index];
    update_timer_str(preset_timer_str, preset_timer_val[index], preset_timer_val[index]);
    memcpy(active_timer_rec_str, preset_timer_str, strlen(active_timer_rec_str));
    gui_view_switch_direct(gui_view_get_current(), "timer_running_view", SWITCH_OUT_NONE_ANIMATION,
                           SWITCH_IN_NONE_ANIMATION);
}

void click_button_play_stop(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);
#ifdef _HONEYGUI_SIMULATOR_
    // TODO
#else
    // TODO
#endif
    if (active_timer_rec_val != 0)
    {
        if (icon_play->fg_color_set == 0xFF000000)
        {
            gui_img_a8_recolor(icon_play, 0xFFFFFFFF);
            gui_img_set_src(icon_play, "/app_timer/timer_button_start.bin", IMG_SRC_FILESYS);
            gui_obj_stop_timer((void *)active_timer_text);
            gui_obj_stop_timer((void *)active_arc);
        }
        else
        {
            gui_img_a8_recolor(icon_play, 0xFF000000);
            gui_img_set_src(icon_play, "/app_timer/timer_button_stop.bin", IMG_SRC_FILESYS);
            gui_obj_start_timer((void *)active_timer_text);
            gui_obj_start_timer((void *)active_arc);
        }
    }
    else
    {
        memcpy(active_timer_rec_str, preset_timer_str, strlen(active_timer_rec_str));
        gui_log("active_timer_rec_str: %s", active_timer_rec_str);
        active_timer_rec_val = preset_timer_val[preset_index];
        gui_text_content_set(active_timer_text, active_timer_rec_str, strlen(active_timer_rec_str));
        gui_img_a8_recolor(icon_play, 0xFF000000);
        gui_img_set_src(icon_play, "/app_timer/timer_button_stop.bin", IMG_SRC_FILESYS);
        gui_obj_start_timer((void *)active_timer_text);
        gui_obj_start_timer((void *)active_arc);
        gui_obj_move((void *)icon_play, 333, 422);
    }
}
