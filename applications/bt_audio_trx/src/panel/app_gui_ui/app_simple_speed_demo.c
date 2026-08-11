/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_panel.h"
#if F_GUI_SIMPLE_SPEED_DEMO
#include "draw_font.h"
#include "resource.h"
#include "gui_win.h"
#include "gui_img.h"
#include "gui_text.h"
#include "gui_obj.h"
#include "gui_fb.h"
#include "font_mem.h"

gui_text_t *text = NULL;
uint8_t num = 100;
char str[4] = {0};

void app_panel_text_animate_cback(void)
{
    num++;
    sprintf(str, "%d", num);
    gui_text_content_set(text, str, strlen(str));
    //gui_text_set(text, str, "rtk_font_mem", gui_rgb(0xff, 0xff, 0xff), strlen(str), 32);
    //gui_text_type_set(text, SIMKAI_SIZE24_BITS1_FONT_BIN);
    //gui_text_mode_set(text, CENTER);
}

extern void gui_set_keep_active_time(uint32_t active_time);

void app_simple_speed_ui_design(void)
{
#ifdef ENABLE_RTK_GUI_SCRIPT_AS_A_APP
#else
    gui_log("app_simple_speed_ui_design");
    //gui_font_mem_init(SIMKAI_SIZE24_BITS1_FONT_BIN);

    gui_win_t *win = gui_win_create(gui_obj_get_root(), "win", 0, 0, 320, 320);
    gui_img_t *bg1 = gui_img_create_from_mem(win, "bg1", BG_410_502_BIN, 0, 0, 0, 0);
    gui_img_set_mode(bg1, IMG_BYPASS_MODE);

    gui_set_keep_active_time(0xffffffff);
#endif
}
#endif
