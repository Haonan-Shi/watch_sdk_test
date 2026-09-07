/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_GUI_BR_BLE_LINK_STATUS_DEMO
#include "draw_font.h"
#include "resource.h"
#include "gui_obj.h"
#include "gui_tab.h"
#include "gui_win.h"
#include "gui_img.h"
#include "gui_text.h"
#include "trace.h"
#include "app_panel_le_db.h"
#include "app_panel_bredr_db.h"

gui_text_t *text_bredr = NULL;
gui_text_t *text_ble = NULL;
char prefix_bredr[] = "bredr addr:";
char prefix_ble[] = "ble addr:";
char str_bredr[64] = {0};
char str_ble[64] = {0};

void app_panel_ble_text_animate_cback(void)
{
    T_APP_GUI_LE_LINK_DATA *p_app_gui_le_link = NULL;
    p_app_gui_le_link = app_panel_le_find_active_le_link();
    memset(str_ble, 0, sizeof(str_ble));

    if (p_app_gui_le_link)
    {
        sprintf(str_ble, "%s %x:%x:%x:%x:%x:%x", prefix_ble, p_app_gui_le_link->bd_addr[0],
                p_app_gui_le_link->bd_addr[1],
                p_app_gui_le_link->bd_addr[2], p_app_gui_le_link->bd_addr[3],
                p_app_gui_le_link->bd_addr[4], p_app_gui_le_link->bd_addr[5]);
    }
    else
    {
        sprintf(str_ble, "%s", prefix_ble);
    }

    gui_text_set(text_ble, str_ble, "rtk_font_mem", gui_rgb(0xff, 0xff, 0xff), strlen(str_ble), 32);
}

void app_panel_bredr_text_animate_cback(void)
{
    T_APP_GUI_BREDR_LINK_DATA *p_app_gui_bredr_link = NULL;
    p_app_gui_bredr_link = app_panel_bredr_find_active_br_link();
    memset(str_bredr, 0, sizeof(str_bredr));

    if (p_app_gui_bredr_link)
    {
        sprintf(str_bredr, "%s %x:%x:%x:%x:%x:%x", prefix_bredr, p_app_gui_bredr_link->bd_addr[0],
                p_app_gui_bredr_link->bd_addr[1],
                p_app_gui_bredr_link->bd_addr[2], p_app_gui_bredr_link->bd_addr[3],
                p_app_gui_bredr_link->bd_addr[4], p_app_gui_bredr_link->bd_addr[5]);
    }
    else
    {
        sprintf(str_bredr, "%s", prefix_bredr);
    }

    gui_text_set(text_bredr, str_bredr, "rtk_font_mem", gui_rgb(0xff, 0xff, 0xff), strlen(str_bredr),
                 32);
}

void app_bredr_ble_link_ui_design(gui_app_t *app)
{
#ifdef ENABLE_RTK_GUI_SCRIPT_AS_A_APP
#else
    gui_font_mem_init(SIMKAI_SIZE24_BITS1_FONT_BIN);

    gui_win_t *win = gui_win_create(&(app->screen), "win", 0, 0, 320, 320);
    gui_tabview_t *tv = gui_tabview_create(win, "tabview", 0, 0, 0, 0);
    gui_tab_t *tb1 = gui_tab_create(tv, "tb1",             0, 0, 0, 0, 0, 0);
    gui_img_t *bg1 = gui_img_create_from_mem(tb1, "bg1", BG_454_454_BIN, 0, 0, 0, 0);
    gui_img_set_mode(bg1, IMG_BYPASS_MODE);

    text_bredr = gui_text_create(tb1, "text_bredr", 15, 150, 400, 50);
    text_ble = gui_text_create(tb1, "text_ble", 15, 210, 400, 50);
    gui_text_type_set(text_bredr, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);
    gui_text_mode_set(text_bredr, LEFT);
    gui_text_type_set(text_ble, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);
    gui_text_mode_set(text_ble, LEFT);

    gui_text_set(text_bredr, prefix_bredr, GUI_FONT_SRC_BMP, gui_rgb(0xff, 0xff, 0xff),
                 strlen(prefix_bredr), 32);
    gui_text_set(text_ble, prefix_ble, GUI_FONT_SRC_BMP, gui_rgb(0xff, 0xff, 0xff), strlen(prefix_ble),
                 32);

    APP_PRINT_INFO1("app_bredr_ble_link_ui_design: %d", strlen(prefix_ble));


    gui_text_set_animate(text_bredr, 1000, -1, app_panel_bredr_text_animate_cback, NULL);
    gui_text_set_animate(text_ble, 1000, -1, app_panel_ble_text_animate_cback, NULL);
#endif
}
#endif
