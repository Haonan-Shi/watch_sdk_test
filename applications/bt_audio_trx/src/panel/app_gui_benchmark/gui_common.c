/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_GUI_BENCHMARK_SUPPORT
#include "gui_common.h"
#include "gui_interface.h"
#include "gui_server.h"
#include "def_list.h"

extern void win_count_frame_update_frame_cost_cb(void *obj, gui_event_user_t event);
extern void win_count_frame_switch_scenario_cb(void *obj, gui_event_user_t event);

static void gui_update_event_cb(gui_msg_t *msg)
{
    gui_log("gui_update_event_cb: event %d, sub_event %d", msg->event,
            (gui_event_user_t)msg->sub_event);

    if (msg->event == GUI_EVENT_USER_INIT)
    {
        switch (msg->sub_event)
        {
        case GUI_EVENT_BENCHMARK_UPDATE_FRAME_COST:
            {
                win_count_frame_update_frame_cost_cb(msg->payload, (gui_event_user_t)msg->sub_event);
                break;
            }
        case GUI_EVENT_BENCHMARK_NEXT_SCENRAIO:
            {
                win_count_frame_switch_scenario_cb(msg->payload, (gui_event_user_t)msg->sub_event);
                break;
            }
        default:
            break;
        }
    }
}

void gui_update_by_event(gui_event_user_t event, void *data, bool force_update)
{
    extern bool gui_send_msg_to_server(gui_msg_t *msg);

    gui_msg_t p_msg;
    if (event < GUI_EVENT_USER_INIT)
    {
        p_msg.event = event;
        gui_send_msg_to_server(&p_msg);
    }
    else if (event > GUI_EVENT_USER_INIT)
    {
        p_msg.event = GUI_EVENT_USER_INIT;
        p_msg.sub_event = event;
        p_msg.payload = data;
        p_msg.cb = (gui_msg_cb)gui_update_event_cb;
        gui_send_msg_to_server(&p_msg);
    }
    else
    {
        gui_log("do nothing when send GUI_EVENT_USER_INIT");
        //do nothing
    }

    if (force_update)
    {
        gui_msg_t p_msg_on;
        p_msg_on.event = GUI_EVENT_DISPLAY_ON;
        gui_send_msg_to_server(&p_msg_on);
    }
}

gui_img_t *app_gui_img_create(void       *parent,
                              const char *name,
                              void       *file,
                              int16_t     x,
                              int16_t     y,
                              int16_t     w,
                              int16_t     h)
{
    gui_img_t *img = NULL;

    img = gui_img_create_from_mem(parent, name, file, x, y, w, h);

    return img;
}

gui_text_t *app_gui_text_create(void       *parent,
                                const char *name,
                                int16_t     x,
                                int16_t     y,
                                int16_t     w,
                                int16_t     h)
{
    gui_text_t *text = NULL;

    text = gui_text_create(parent, name, x, y, w, h);
    gui_text_font_mode_set(text, FONT_SRC_MEMADDR);

    return text;
}

gui_scroll_text_t *app_gui_scroll_text_create(void       *parent,
                                              const char *name,
                                              int16_t     x,
                                              int16_t     y,
                                              int16_t     w,
                                              int16_t     h)
{
    gui_scroll_text_t *scroll_text = NULL;

    scroll_text = gui_scroll_text_create(parent, name, x, y, w, h);

    return scroll_text;
}
#endif
