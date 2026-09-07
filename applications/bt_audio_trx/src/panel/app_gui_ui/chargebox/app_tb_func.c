/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_panel.h"
#if F_GUI_CHARGEBOX_DEMO
#include "trace.h"
#include "resource.h"
#include <gui_img.h>
#include "gui_win.h"
#include "gui_obj.h"
#include <gui_text.h>
#include <gui_button.h>
#include "font_mem.h"
#include "app_panel_le_msg.h"
#include "app_panel_db_common.h"
#include "app_panel_le_db.h"
#include "app_panel_device_db.h"
#include "app_panel_msg_util.h"
#include "app_chargebox.h"

gui_text_t *text_device_name;
gui_text_t *text_battery_level;
gui_text_t *text_bt_connect_status;
gui_text_t *text_volume;
gui_text_t *text_anc_status;
static char device_name[128];
static char battery_level[128];
static char bt_connect_status[32];
static char volume[32];
static char anc_status[32];

#define APP_CHARGEBOX_BASIC_INFO_FONT_SIZE 24

void app_gui_mmi_volume_up(void *obj, gui_event_t e, void *param);
void app_gui_mmi_volume_down(void *obj, gui_event_t e, void *paramid);

void page_tb_func(void *parent)
{
    gui_img_t *table = NULL;
    //gui_img_create_from_mem(parent, "page1", BACKGROUND_BIN, 0, 0, 0, 0);
    table = gui_img_create_from_mem(parent, "table2", TABLE2_BLANK_BIN, 10, 10, 0, 0);
    gui_img_set_mode(table, IMG_BYPASS_MODE);

    gui_font_mem_init(SIMKAI_SIZE24_BITS1_FONT_BIN);
    snprintf(device_name, sizeof(device_name), "Name:    %s", app_panel_device_db_get_device_name());
    text_device_name = gui_text_create(parent, "dev name txt", 20, 15, gui_get_screen_width(), 24);
    gui_text_set(text_device_name, device_name, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX,
                                                                          UINT8_MAX),
                 strlen(device_name),
                 APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_text_type_set(text_device_name, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);

    snprintf(battery_level, sizeof(battery_level), "Battery:  Case %d%%,L %%,R %%",
             app_panel_device_db_get_bettery_level());
    //char *text2 = "Battery:  Case %,L %,R %";
    APP_PRINT_INFO1("page_tb_func: %s", battery_level);
    text_battery_level = gui_text_create(parent, "battery level txt", 20, 55, gui_get_screen_width(),
                                         24);
    gui_text_set(text_battery_level, battery_level, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX,
                                                                              UINT8_MAX),
                 strlen(battery_level),
                 APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_text_type_set(text_battery_level, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);
    char *text3 = "BT:      Disconnected";
    text_bt_connect_status = gui_text_create(parent, "bt status txt", 20, 95, gui_get_screen_width(),
                                             24);
    gui_text_set(text_bt_connect_status, text3, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX,
                                                                          UINT8_MAX), strlen(text3),
                 APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_text_type_set(text_bt_connect_status, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);
    char *text4 = "Volume:    -%";
    text_volume = gui_text_create(parent, "volume txt", 20, 135, gui_get_screen_width(), 24);
    gui_text_set(text_volume, text4, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX, UINT8_MAX),
                 strlen(text4),
                 APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_text_type_set(text_volume, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);
    char *text5 = "ANC:      ";
    text_anc_status = gui_text_create(parent, "anc txt", 20, 175, gui_get_screen_width(), 24);
    gui_text_set(text_anc_status, text5, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX, UINT8_MAX),
                 strlen(text5),
                 APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_text_type_set(text_anc_status, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);

    gui_button_t *button1 = gui_button_create(parent, 80, 250, 106, 63, VOL_INC_BIN, VOL_INC_BLUE_BIN,
                                              "",
                                              BUTTON_BG_ICON, 0);
    gui_button_click(button1, app_gui_mmi_volume_up, NULL);
    gui_button_t *button2 = gui_button_create(parent, 220, 250, 106, 63, VOL_DEC_BIN, VOL_DEC_BLUE_BIN,
                                              "",
                                              BUTTON_BG_ICON, 0);
    gui_button_click(button2, app_gui_mmi_volume_down, NULL);
}

void app_tb_set_device_name(uint8_t *data, uint8_t length)
{
    if (text_device_name)
    {
        snprintf(device_name, length < sizeof(device_name) ? length : sizeof(device_name), "Name:     %s",
                 data);
        gui_text_content_set(text_device_name, device_name, strlen(device_name));
    }
}

void app_tb_set_battery(uint8_t case_battery, uint8_t left_battery, uint8_t right_battery)
{
    APP_PRINT_INFO4("app_tb_set_battery: case %d, left %d, right %d, text_battery_level %p",
                    case_battery, left_battery, right_battery, text_battery_level);
    if (text_battery_level)
    {
        if (left_battery == BATTERY_INVALIE_VALUE && right_battery == BATTERY_INVALIE_VALUE)
        {
            snprintf(battery_level, sizeof(battery_level), "Battery:   Case %d%%,L %%,R %%",
                     case_battery);
        }
        else if (left_battery == BATTERY_INVALIE_VALUE)
        {
            snprintf(battery_level, sizeof(battery_level), "Battery:   Case %d%%,L %%,R %d%%",
                     case_battery, right_battery);
        }
        else if (right_battery == BATTERY_INVALIE_VALUE)
        {
            snprintf(battery_level, sizeof(battery_level), "Battery:   Case %d%%,L %d%%,R %%",
                     case_battery, left_battery);
        }
        else
        {
            snprintf(battery_level, sizeof(battery_level), "Battery:   Case %d%%,L %d%%,R %d%%",
                     case_battery, left_battery, right_battery);
        }

        APP_PRINT_INFO1("app_tb_set_battery: %s", battery_level);

        gui_text_content_set(text_battery_level, battery_level, strlen(battery_level));
    }
}

void app_tb_set_bt_link_status(uint8_t *bd_addr,  T_APP_GUI_LE_LINK_STATUS link_status)
{
    if (link_status == BLE_LINK_CONNECT)
    {
        snprintf(bt_connect_status, sizeof(bt_connect_status), "BT:       Connected");
    }
    else
    {
        snprintf(bt_connect_status, sizeof(bt_connect_status), "BT:       Disconnected");
    }

    if (text_bt_connect_status)
    {
        gui_text_content_set(text_bt_connect_status, bt_connect_status, strlen(bt_connect_status));
    }
}

void app_tb_set_bt_volume_value(uint8_t *bd_addr,  uint8_t volume_value)
{
    if (text_volume)
    {
        if (volume_value == 0xff)
        {
            snprintf(volume, sizeof(volume), "Volume:    -%");
        }
        else
        {
            snprintf(volume, sizeof(volume), "Volume:    %d%%", volume_value);
        }
        gui_text_content_set(text_volume, volume, strlen(volume));
    }
}

void app_tb_set_bt_anc_status(uint8_t *bd_addr,  T_APP_GUI_LISTENING_MODE anc_enable)
{
    if (anc_enable == GUI_LISTENING_OFF)
    {
        snprintf(anc_status, sizeof(anc_status), "ANC:      OFF");
    }
    else if (anc_enable == GUI_LISTENING_NORMAL_APT)
    {
        snprintf(anc_status, sizeof(anc_status), "ANC:      Normal APT");
    }
    else if (anc_enable == GUI_LISTENING_ANC)
    {
        snprintf(anc_status, sizeof(anc_status), "ANC:      ANC");
    }
    else if (anc_enable == GUI_LISTENING_LLAPT)
    {
        snprintf(anc_status, sizeof(anc_status), "ANC:      LLAPT");
    }

    if (text_anc_status)
    {
        gui_text_content_set(text_anc_status, anc_status, strlen(anc_status));
    }
}

void app_gui_mmi_volume_up(void *obj, gui_event_t e, void *param)
{
    T_APP_GUI_MSG gui_msg = {0};
    gui_msg.type = EVENT_GUI_TO_BLE;
    gui_msg.subtype = GUI_LE_SUBEVENT_ADJUST_VOLUME;
    gui_msg.u.param = (uint32_t)GUI_ADJUST_VOLUME_UP;
    APP_PRINT_INFO0("app_gui_mmi_volume_up");
    app_panel_msg_send_to_app(&gui_msg);
}

void app_gui_mmi_volume_down(void *obj, gui_event_t e, void *param)
{
    T_APP_GUI_MSG gui_msg = {0};
    gui_msg.type = EVENT_GUI_TO_BLE;
    gui_msg.subtype = GUI_LE_SUBEVENT_ADJUST_VOLUME;
    gui_msg.u.param = (uint32_t)GUI_ADJUST_VOLUME_DOWN;
    APP_PRINT_INFO0("app_gui_mmi_volume_down");
    app_panel_msg_send_to_app(&gui_msg);
}

#endif
