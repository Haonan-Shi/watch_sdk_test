/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_panel.h"
#if F_GUI_CHARGEBOX_DEMO
#include "resource.h"
#include <gui_img.h>
#include "gui_win.h"
#include <gui_button.h>
#include "gui_switch.h"
#include "gui_grid.h"
#include <gui_page.h>
#include "font_mem.h"
#include "app_panel_le_msg.h"
#include "app_panel_le_db.h"
#include "app_panel_msg_util.h"

gui_switch_t *scan_switch = NULL;
gui_page_t *page_device_list = NULL;
gui_grid_t    *grid = NULL ;
gui_button_t *select_button = NULL;
char str_select[] = "(sync)";
uint8_t *device_origin_name = NULL;
char device_select_name[64] = {0};

void scan_switch_on_cback(void *obj, gui_event_t e, void *param)
{
    grid = gui_grid_create(page_device_list, 0, 0, 100, 1, 0, 50);

    T_APP_GUI_MSG gui_msg = {0};
    gui_msg.type = EVENT_GUI_TO_BLE;
    gui_msg.subtype = GUI_LE_SUBEVENT_BASS_SCAN;
    gui_msg.u.param = (uint32_t)GUI_BASS_SCAN_START;
    app_panel_msg_send_to_app(&gui_msg);
}

void scan_switch_off_cback(void *obj, gui_event_t e, void *param)
{
    gui_obj_tree_free((gui_obj_t *)grid);
    select_button = NULL;
    gui_fb_change();

    T_APP_GUI_MSG gui_msg = {0};
    gui_msg.type = EVENT_GUI_TO_BLE;
    gui_msg.subtype = GUI_LE_SUBEVENT_BASS_SCAN;
    gui_msg.u.param = (uint32_t)GUI_BASS_SCAN_STOP;
    app_panel_msg_send_to_app(&gui_msg);
}

void conn_button_click_cback(void *obj, gui_event_t e, void *param)
{
    T_APP_GUI_MSG gui_msg = {0};
    gui_msg.type = EVENT_GUI_TO_BLE;
    gui_msg.subtype = GUI_LE_SUBEVENT_BST_RECEPTION;
    gui_msg.u.param = (uint32_t)GUI_LE_BST_RECEPTION_START;
    app_panel_msg_send_to_app(&gui_msg);
}

void disc_button_click_cback(void *obj, gui_event_t e, void *param)
{
    T_APP_GUI_MSG gui_msg = {0};
    gui_msg.type = EVENT_GUI_TO_BLE;
    gui_msg.subtype = GUI_LE_SUBEVENT_BST_RECEPTION;
    gui_msg.u.param = (uint32_t)GUI_LE_BST_RECEPTION_STOP;
    app_panel_msg_send_to_app(&gui_msg);
}

void page_tb_conn(void *parent)
{
    gui_img_t *overlap1 = NULL;
    gui_img_t *overlap2 = NULL;
    //gui_img_create_from_mem(parent, "page0", BACKGROUND_BIN, 0, 0, 0, 0);
    overlap1 = gui_img_create_from_mem(parent, "overlap", OVERLAP_BIN, 0, -30, 0, 0);
    gui_img_set_mode(overlap1, IMG_BYPASS_MODE);
    overlap2 = gui_img_create_from_mem(parent, "overlap", OVERLAP_BIN, 0, 240, 0, 0);
    gui_img_set_mode(overlap2, IMG_BYPASS_MODE);
    gui_img_create_from_mem(parent, "AURACAST", AURACAST_BIN, 0, 0, 0, 0);

    gui_font_mem_init(SIMKAI_SIZE24_BITS1_FONT_BIN);

    page_device_list = gui_page_create(parent, "BT", 0, 50, 385, 200);
    gui_page_rebound(page_device_list, true);

    scan_switch = gui_switch_create(parent, 20, 250, 106, 63, SCAN_BIN, STOP_BIN);
    gui_obj_add_event_cb(scan_switch, (gui_event_cb_t)scan_switch_on_cback, GUI_EVENT_1, NULL);
    gui_obj_add_event_cb(scan_switch, (gui_event_cb_t)scan_switch_off_cback, GUI_EVENT_2, NULL);

    gui_button_t *button_conn = gui_button_create(parent, 140, 250, 106, 63, CONN_BIN, CONN_BLUE_BIN,
                                                  "",
                                                  BUTTON_BG_ICON, 0);
    gui_button_click(button_conn, conn_button_click_cback, NULL);

    gui_button_t *button_disc = gui_button_create(parent, 260, 250, 106, 63, DISC_BIN, DISC_BLUE_BIN,
                                                  "",
                                                  BUTTON_BG_ICON, 0);
    gui_button_click(button_disc, disc_button_click_cback, NULL);
}

void app_tb_dev_list_click_cback(void *obj, gui_event_t e, void *param)
{
    gui_button_t *button = (gui_button_t *)obj;
    gui_log("app_tb_dev_list_click_cback: button y %d", button->base.y);
    if (select_button)
    {
        gui_text_content_set(select_button->text, device_origin_name, strlen((char *)device_origin_name));
    }
    select_button = button;
    device_origin_name = GUI_TYPE(gui_button_t, button)->text->content;
    snprintf(device_select_name, sizeof(device_select_name), "%s%s", device_origin_name, str_select);
    gui_text_content_set(button->text, device_select_name, strlen((char *)device_select_name));
    gui_fb_change();

    T_APP_GUI_MSG gui_msg = {0};
    gui_msg.type = EVENT_GUI_TO_BLE;
    gui_msg.subtype = GUI_LE_SUBEVENT_PA_SYNC;
    gui_msg.u.param = (uint32_t)button->base.y / 50;
    app_panel_msg_send_to_app(&gui_msg);
}

void app_tb_add_scan_dev(uint8_t idx, uint8_t *device_name)
{
    gui_button_t *button = gui_button_create(grid, 0, 0, 0, 50, BUTTON_BIN, BUTTON_ON_BIN, "",
                                             BUTTON_BG_ICON, 0);

    if (button)
    {
        gui_text_type_set(button->text, SIMKAI_SIZE24_BITS1_FONT_BIN,
                          FONT_SRC_MEMADDR);
        gui_text_size_set(button->text, 24, 0);
        gui_text_mode_set(button->text, LEFT);
        gui_text_content_set(button->text, (void *)device_name, strlen((char *)device_name));
        gui_button_text_move(button, 0, 0);
        gui_button_click(button, app_tb_dev_list_click_cback, button);
        gui_log("app_tb_add_scan_dev: device name %s, len %d, y %d", device_name,
                strlen((char *)device_name), button->base.y);
    }
    else
    {
        gui_log("app_tb_add_scan_dev: button alloc failed");
    }
}

#endif
