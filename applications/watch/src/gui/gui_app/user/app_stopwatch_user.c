/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_stopwatch_user.h"
#include "gui_message.h"
/**
 * 用户自定义实现
 * 此文件只生成一次，可自由修改
 */

// 在此添加自定义实现

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

STOPWATCH_STATUS status = DEFAULT;
uint32_t time_count = 0; //milsec
uint32_t time_count_array[TIME_CNT_NUM] = {0}; //milsec
uint8_t time_count_index = 0;
char count_str[10] = "00:00.00";
char count_str_array[TIME_CNT_NUM][10] = {0};

static void regenerate_current_view(void *msg)
{
    GUI_UNUSED(msg);

    gui_view_t *current_view = gui_view_get_current();
    const gui_view_descriptor_t *descriptor = current_view->descriptor;
    gui_obj_t *parent = current_view->base.parent;
    gui_obj_tree_free(GUI_BASE(current_view));

    gui_view_create(parent, descriptor->name, 0, 0, 0, 0);
}
void msg_2_regenerate_current_view(void)
{
    gui_msg_t msg =
    {
        .event = GUI_EVENT_USER_DEFINE,
        .cb = regenerate_current_view,
    };
    gui_send_msg_to_server(&msg);
}

void click_button_l(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    gui_obj_t *list = (void *)lst_stopwatch;
    if (list->child_list.next != list->child_list.prev) { return; }
    gui_obj_t *note = gui_list_entry(list->child_list.next, gui_obj_t, brother_list);
    GUI_UNUSED(note);
    switch (status)
    {
    case DEFAULT:
        return;
        break;
    case START:
        {
            if (time_count_index != TIME_CNT_NUM)
            {
                time_count_array[time_count_index] = time_count;
                if (time_count_index == 0) //create new hand
                {
                    gui_obj_hidden((gui_obj_t *)sec_hand_big_1, false);
                }
                time_count_index++;
            }
            // gui_log("time_count_array[%d] = %d\n", time_count_index - 1, time_count_array[time_count_index - 1]);
        }
        break;
    case STOP:
        msg_2_regenerate_current_view();
        break;

    default:
        break;
    }
}

void click_button_r(void *obj, gui_event_t *e)
{
    GUI_UNUSED(obj);
    GUI_UNUSED(e);

    gui_obj_t *bg_r = obj;
    gui_obj_t *list = (void *)lst_stopwatch;
    if (list->child_list.next != list->child_list.prev) { return; }
    gui_obj_t *note = gui_list_entry(list->child_list.next, gui_obj_t, brother_list);

    switch (status)
    {
    case DEFAULT:
        {
            status = START;
            gui_img_a8_recolor((void *)bg_r, COLOR_STOP);
            gui_img_set_src((void *)icon_r, "/stopwatch/stopwatch_button_stop.bin", IMG_SRC_FILESYS);
            gui_img_refresh_size((void *)icon_r);
            gui_obj_move((void *)icon_r, ICON_STOP_X, ICON_STOP_Y);
            gui_img_a8_recolor((void *)bg_l, COLOR_MARK);
            gui_obj_hidden((void *)icon_l, false);

            gui_obj_start_timer((void *)note);
        }
        break;
    case START:
        {
            status = STOP;
            gui_img_a8_recolor((void *)bg_r, COLOR_START);
            gui_img_set_src((void *)icon_r, "/stopwatch/stopwatch_button_start.bin", IMG_SRC_FILESYS);
            gui_img_refresh_size((void *)icon_r);
            gui_obj_move((void *)icon_r, ICON_START_X, ICON_START_Y);
            gui_img_a8_recolor((void *)bg_l, COLOR_RESET);
            gui_img_set_src((void *)icon_l, "/stopwatch/stopwatch_button_reset.bin", IMG_SRC_FILESYS);
            gui_img_refresh_size((void *)icon_l);
            gui_obj_move((void *)icon_l, ICON_RESET_X, ICON_RESET_Y);

            gui_obj_stop_timer((void *)note);
        }
        break;
    case STOP:
        {
            status = START;
            gui_img_a8_recolor((void *)bg_r, COLOR_STOP);
            gui_img_set_src((void *)icon_r, "/stopwatch/stopwatch_button_stop.bin", IMG_SRC_FILESYS);
            gui_img_refresh_size((void *)icon_r);
            gui_obj_move((void *)icon_r, ICON_STOP_X, ICON_STOP_Y);
            gui_img_a8_recolor((void *)bg_l, COLOR_MARK);
            gui_img_set_src((void *)icon_l, "/stopwatch/stopwatch_button_mark.bin", IMG_SRC_FILESYS);
            gui_img_refresh_size((void *)icon_l);
            gui_obj_move((void *)icon_l, ICON_MARK_X, ICON_MARK_Y);

            gui_obj_start_timer((void *)note);
        }
        break;
    default:
        break;
    }
}