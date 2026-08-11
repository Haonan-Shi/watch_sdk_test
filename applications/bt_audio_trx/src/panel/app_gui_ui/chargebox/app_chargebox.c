/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_panel.h"
#if F_GUI_CHARGEBOX_DEMO
#include <gui_tabview.h>
#include "gui_card.h"
#include <gui_obj.h>
#include <gui_win.h>
#include <gui_text.h>
#include <gui_curtain.h>
#include "gui_tab.h"
#include "app_chargebox.h"
#include "gui_server.h"
#include "gui_components_init.h"
#include "gui_img.h"
#include "gui_page.h"
#include "draw_font.h"
#include "gui_button.h"
#include <stdio.h>
#include "resource.h"


gui_tabview_t *tv;

void app_chargebox_ui_design(void)
{
    gui_log("app_chargebox_ui_design");
    tv = gui_tabview_create(&(app->screen), "tabview", 0, 0, 0, 0);
    gui_win_t *win = gui_win_create(&(app->screen), "window", 0, 0, 0, 0);
    gui_tabview_set_style(tv, CLASSIC);

    gui_tab_t *tb_conn = gui_tab_create(tv, "tb_conn",     0, 0, 0, 0, 1, 0);
    gui_tab_t *tb_func = gui_tab_create(tv, "tb_func",     0, 0, 0, 0, 0, 0);
    gui_tab_t *tb_music = gui_tab_create(tv, "tb_music",     0, 0, 0, 0, 2, 0);

    page_tb_conn(tb_conn);
    page_tb_func(tb_func);
    page_tb_music(tb_music);

}

#endif
