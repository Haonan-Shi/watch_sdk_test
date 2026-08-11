/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_panel.h"
#if F_GUI_SDCARD_LIST_DEMO
#include "draw_font.h"
#include "resource.h"
#include "gui_tab.h"
#include "gui_obj.h"
#include "gui_win.h"
#include "gui_img.h"
#include "font_mem.h"
#include "gui_scroll_text.h"
#include "gui_pagelistview.h"
#include "gui_pagelist.h"
#include "app_gui.h"
#include "app_sdcard_list_demo.h"
#include "app_panel_msg.h"
#include "resource.h"
#include "app_panel_msg_util.h"
#include "trace.h"
#include "app_src_playback.h"

gui_pagelistview_t *pagelistview_sdcard = NULL;
gui_pagelist_t *pagelist_sdcard = NULL;
gui_switch_t *switch_pagelist_sdcard[MAX_SHOW_FILE_LIST_NUM] = {NULL};
gui_scroll_text_t *scroll_text_pagelist_sdcard[MAX_SHOW_FILE_LIST_NUM] = {NULL};
static bool music_status = false;
static uint8_t current_index = 0xff;
char *rect_text = "Music Files";

uint8_t music_files_num = 0;

static uint16_t **music_files_name = NULL;
T_HEAD_INFO *header_info = NULL;

int16_t index_first = 0;
int16_t index_last = 0;

bool pagelist_sdcard_init_music_name(void)
{
    music_files_num = *(uint16_t *)(MUSIC_HEADER_BIN_ADDR + FS_HEADER_COUNT_OFFSET);
    header_info = (T_HEAD_INFO *)(MUSIC_HEADER_BIN_ADDR + FS_HEADER_INFO_START);

    uint16_t res = audio_fs_check_init_result();
    if ((res != AUDIO_FS_OK) || (music_files_num == 0))
    {
        APP_PRINT_ERROR2("pagelist_sdcard_init_music_name, num %d, res 0x%x", music_files_num, res);
        return false;
    }

    for (uint8_t i = 0; i < music_files_num; i++)
    {
        uint8_t *name = (uint8_t *)(MUSIC_NAME_BIN_ADDR + ((header_info[i].offset)));
        APP_PRINT_INFO3("pagelist_sdcard_init_music_name: index %d, length %d, data %b", i,
                        header_info[i].length, TRACE_BINARY(header_info[i].length, name));
    }

    return true;
}

static void pagelist_sdcard_update_list_first_cb(gui_pagelist_t *this, gui_switch_t *list_first)
{
    bool change = true;
    gui_log("pagelist_sdcard_update_list_first_cb, list_first = 0x%x\n", list_first);
    gui_scroll_text_t *list_scroll_text = NULL;

    gui_list_t *node = NULL;
    gui_list_for_each(node, &list_first->base.child_list)
    {
        gui_obj_t *find_obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (find_obj->type == SCROLLTEXTBOX)
        {
            list_scroll_text = (gui_scroll_text_t *)find_obj;
            break;
        }
    }
    index_last ++;
    if (index_last > music_files_num - 1)
    {
        change = false;
        index_last = music_files_num - 1;
    }
    index_first ++;
    if (index_first > music_files_num  - MAX_SHOW_FILE_LIST_NUM)
    {
        change = false;
        index_first = music_files_num - MAX_SHOW_FILE_LIST_NUM ;
    }

    if (change)
    {
        if (current_index == (index_first - 1))
        {
            gui_switch_is_off(list_first);
        }
        else if (current_index == index_last)
        {
            gui_switch_is_on(list_first);
        }
    }

    uint16_t *name_buf = (uint16_t *)(MUSIC_NAME_BIN_ADDR + header_info[index_last].offset);
    gui_scroll_text_content_set(list_scroll_text, name_buf, (header_info[index_last].length) / 2 - 1);
}

static void pagelist_sdcard_update_list_last_cb(gui_pagelist_t *obj, gui_switch_t *list_last)
{
    bool change = true;
    gui_log("pagelist_sdcard_update_list_last_cb, list_first = 0x%x\n", list_last);
    gui_scroll_text_t *list_scroll_text = NULL;

    gui_list_t *node = NULL;
    gui_list_for_each(node, &list_last->base.child_list)
    {
        gui_obj_t *find_obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (find_obj->type == SCROLLTEXTBOX)
        {
            list_scroll_text = (gui_scroll_text_t *)find_obj;
            break;
        }
    }
    index_first --;
    if (index_first < 0)
    {
        change = false;
        index_first = 0;
    }
    index_last --;
    if (index_last < MAX_SHOW_FILE_LIST_NUM - 1)
    {
        change = false;
        index_last = MAX_SHOW_FILE_LIST_NUM - 1;
    }

    if (change)
    {
        if (current_index == (index_last + 1))
        {
            gui_switch_is_off(list_last);
        }
        else if (current_index == index_first)
        {
            gui_switch_is_on(list_last);
        }
    }

    uint16_t *name_buf = (uint16_t *)(MUSIC_NAME_BIN_ADDR + header_info[index_first].offset);
    gui_scroll_text_content_set(list_scroll_text, name_buf, (header_info[index_first].length) / 2 - 1);
}

void app_sdcard_list_play_music(uint8_t index)
{
    T_APP_GUI_MSG gui_msg = {0};
    T_HEAD_INFO *header_info = (T_HEAD_INFO *)(MUSIC_HEADER_BIN_ADDR + FS_HEADER_INFO_START);

    gui_msg.type = EVENT_GUI_TO_DEVICE;
    gui_msg.subtype = GUI_DEVICE_SUBEVENT_PLAY_SD_MUSIC;
    gui_msg.u.param = (uint32_t)(header_info + index);
    app_panel_msg_send_to_app(&gui_msg);
}

void app_sdcard_list_stop_music(void)
{
    T_APP_GUI_MSG gui_msg = {0};

    gui_msg.type = EVENT_GUI_TO_DEVICE;
    gui_msg.subtype = GUI_DEVICE_SUBEVENT_STOP_SD_MUSIC;
    app_panel_msg_send_to_app(&gui_msg);
}

static void swtich_pagelist_touch_cb(void *obj, gui_event_t e)
{
    gui_switch_t *this = (gui_switch_t *)obj;
    uint8_t index = 0;
    //if (this->base.y + this->base.h > pagelist_sdcard->show_border_top &&
    //    this->base.y < pagelist_sdcard->show_border_bottom)
    {
        index = (this->base.y - SWITCH_INIT_Y) / (LIST_GAP + SWITCH_HEIGHT);
        APP_PRINT_INFO5("swtich_pagelist_touch_cb, event %d, y %d, h %d, index %d, current index %d\n", e,
                        this->base.y, this->base.h, index, current_index);

        if (e == GUI_EVENT_1)
        {
            if (current_index != 0xff)
            {
                if (current_index == index)
                {
                    return;
                }
                app_sdcard_list_stop_music();
            }
            current_index = index;
            app_sdcard_list_play_music(index);
        }
        else if (e == GUI_EVENT_2)
        {
            app_sdcard_list_stop_music();
            current_index = 0xff;
        }
        else
        {
            //not handle
        }
    }
}

void app_sdcard_list_ui_design(void)
{
#ifdef ENABLE_RTK_GUI_SCRIPT_AS_A_APP
#else
    gui_font_mem_init(SIMKAI_SIZE24_BITS1_FONT_BIN);
    uint16_t *name_buf = NULL;

    gui_win_t *win = gui_win_create(app, "win_pagelist_sdcard", 0, 0, LCD_WIDTH, LCD_HIGHT);

    pagelistview_sdcard = gui_pagelistview_create(win, "pagelistview", 0, 0, LCD_WIDTH, LCD_HIGHT);
    gui_img_t *img_top_mask = gui_img_create_from_mem(win, "top_mask", PAGELIST_BASE_MASK_BLACK_BIN, 0,
                                                      0,
                                                      LCD_WIDTH, 40);
    gui_img_t *img_bottom_mask = gui_img_create_from_mem(win, "bottom_mask",
                                                         PAGELIST_BASE_MASK_BLACK_BIN,
                                                         0, 280, LCD_WIDTH, 40);
    gui_img_set_mode(img_top_mask, IMG_BYPASS_MODE);
    gui_img_set_mode(img_bottom_mask, IMG_BYPASS_MODE);

    gui_text_t *rect_top_text = gui_text_create(img_top_mask, "rect_top_text", 0, 0, LCD_WIDTH, 41);
    gui_text_set(rect_top_text, rect_text, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX,
                                                                     UINT8_MAX),
                 strlen(rect_text),
                 24);
    gui_text_type_set(rect_top_text, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);
    gui_text_mode_set(rect_top_text, CENTER);

    gui_pagelistview_add_top_mask(pagelistview_sdcard, img_top_mask);
    gui_pagelistview_add_bottom_mask(pagelistview_sdcard, img_bottom_mask);

    if (pagelist_sdcard_init_music_name() == false)
    {
        gui_log("app_sdcard_list_ui_design: music file init failed");
        return;
    }

    pagelist_sdcard = gui_pagelist_create(pagelistview_sdcard, "pagelist", 0, 0, LCD_WIDTH, LCD_HIGHT);
    for (int8_t i = 0;
         i < (MAX_SHOW_FILE_LIST_NUM < music_files_num ? MAX_SHOW_FILE_LIST_NUM : music_files_num); i++)
    {
        switch_pagelist_sdcard[i] = gui_switch_create(pagelist_sdcard, SWITCH_INIT_X,
                                                      SWITCH_INIT_Y + i * (LIST_GAP + SWITCH_HEIGHT), SWITCH_WIDTH,
                                                      SWITCH_HEIGHT,
                                                      ICON_TEXT_BASE_DARK_BIN, ICON_TEXT_BASE_BLUE_BIN);
        //switch_pagelist_sdcard[i]->off_hl_pic_addr = ICON_TEXT_BASE_BLUE_BIN;
        //switch_pagelist_sdcard[i]->on_hl_pic_addr = ICON_TEXT_BASE_BLUE_BIN;
        gui_img_set_mode(switch_pagelist_sdcard[i]->switch_picture, IMG_BYPASS_MODE);

        scroll_text_pagelist_sdcard[i] = gui_scroll_text_create(switch_pagelist_sdcard[i],
                                                                "scroll_sdcard_files", 0, 0, SWITCH_WIDTH, FONT_SIZE);
        gui_scroll_text_encoding_set(scroll_text_pagelist_sdcard[i], UNICODE_ENCODING);
        name_buf = (uint16_t *)(MUSIC_NAME_BIN_ADDR + header_info[i].offset);
        gui_scroll_text_set(scroll_text_pagelist_sdcard[i], name_buf,
                            GUI_FONT_SRC_BMP, gui_rgb(0xff, 0xff, 0xff), (header_info[index_first].length / 2 - 1), FONT_SIZE);
        gui_text_type_set((gui_text_t *)scroll_text_pagelist_sdcard[i], SIMKAI_SIZE24_BITS1_FONT_BIN,
                          FONT_SRC_MEMADDR);
        gui_scroll_text_scroll_set(scroll_text_pagelist_sdcard[i], SCROLL_X, 255, 255, 5000, 0);
        gui_obj_add_event_cb(switch_pagelist_sdcard[i], (gui_event_cb_t)swtich_pagelist_touch_cb,
                             GUI_EVENT_1,
                             NULL);
        gui_obj_add_event_cb(switch_pagelist_sdcard[i], (gui_event_cb_t)swtich_pagelist_touch_cb,
                             GUI_EVENT_2,
                             NULL);
    }

    index_first = 0;
    index_last = MAX_SHOW_FILE_LIST_NUM - 1;
    gui_pagelist_set_att(pagelist_sdcard, music_files_num, MAX_SHOW_FILE_LIST_NUM, LIST_GAP,
                         switch_pagelist_sdcard[0],
                         switch_pagelist_sdcard[MAX_SHOW_FILE_LIST_NUM - 1]);

    gui_pagelist_add_list_update_cb(pagelist_sdcard,
                                    (gui_pagelist_update_cb_t)pagelist_sdcard_update_list_first_cb,
                                    (gui_pagelist_update_cb_t)pagelist_sdcard_update_list_last_cb);
#endif
}
#endif
