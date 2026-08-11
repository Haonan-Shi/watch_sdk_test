/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "guidef.h"
#include "gui_port.h"
#include "gui_api.h"
#include "trace.h"
#include "string.h"
#include "platform_utils.h"
#include "os_sched.h"
#include "trace.h"
#include "os_mem.h"
#include "rtl876x_pinmux.h"
#include "rtl_lcdc.h"
#include "system_status_api.h"
#include "fmc_api_ext.h"
#include "section.h"
#include "lcd_sh8601z_410_502_qspi.h"
#include "app_dlps.h"
#include "clk_mgr.h"


#define DRV_LCD_WIDTH   410
#define DRV_LCD_HIGHT   502

#ifdef DRV_LCD_WIDTH
#undef DRV_LCD_WIDTH
#define DRV_LCD_WIDTH   410
#endif

#ifdef DRV_LCD_HIGHT
#undef DRV_LCD_HIGHT
#define DRV_LCD_HIGHT   502
#endif

#define LCD_SECTION_HEIGHT                      5


#include <rtl876x_rcc.h>
#include <rtl876x_gdma.h>
#include <dma_channel.h>

static T_CLK_USER_HANDLE clk_user_gui;

static bool gui_task_work = true;

bool gui_port_dc_lcd_is_work(void)
{
    return gui_task_work;
}

uint32_t rt_tick_get_millisecond(void)
{
    return sys_timestamp_get();
}

void gui_dc_lcd_power_on(void)
{
    gui_log("gui_dc_lcd_power_on\n");
    gui_task_work = true;

    if (app_dlps_check_enter_bits(APP_DLPS_ENTER_CHECK_DISPLAY))
    {
        return;
    }

    app_dlps_disable(APP_DLPS_ENTER_CHECK_DISPLAY);

    rtk_lcd_hal_init();
    sys_hall_auto_sleep_in_idle(false);
    clk_mgr_set_high_performance(clk_user_gui);
}

void gui_dc_lcd_power_off(void)
{
    gui_log("gui_dc_lcd_power_off\n");
    gui_task_work = false;

    if (!app_dlps_check_enter_bits(APP_DLPS_ENTER_CHECK_DISPLAY))
    {
        return;
    }
    SH8601Z_qspi_power_off();
    rtk_lcd_hal_lcd_enter_dlps();
    sys_hall_auto_sleep_in_idle(true);
    clk_mgr_set_normal_performance(clk_user_gui);
    app_dlps_enable(APP_DLPS_ENTER_CHECK_DISPLAY);
}

void port_gui_lcd_update(struct gui_dispdev *dc)
{
    uint32_t total_section_cnt = (rtk_lcd_hal_get_height() / LCD_SECTION_HEIGHT + ((
            rtk_lcd_hal_get_height() % LCD_SECTION_HEIGHT) ? 1 : 0));

    if (dc->section_count == 0)
    {
        rtk_lcd_hal_set_TE_type(LCDC_TE_TYPE_NO_TE);
        rtk_lcd_hal_set_window(0, dc->fb_height * dc->section_count, dc->fb_width, dc->fb_height);
        rtk_lcd_hal_start_transfer(dc->frame_buf, dc->fb_width * dc->fb_height);
    }
    else if (dc->section_count == total_section_cnt - 1)
    {
        uint32_t last_height = dc->screen_height - dc->section_count * dc->fb_height;
        rtk_lcd_hal_transfer_done();
        rtk_lcd_hal_set_TE_type(LCDC_TE_TYPE_NO_TE);
        rtk_lcd_hal_set_window(0, dc->fb_height * dc->section_count, dc->fb_width, last_height);
        rtk_lcd_hal_start_transfer(dc->frame_buf, dc->fb_width * last_height);
        rtk_lcd_hal_transfer_done();
    }
    else
    {
        rtk_lcd_hal_transfer_done();
        rtk_lcd_hal_set_TE_type(LCDC_TE_TYPE_NO_TE);
        rtk_lcd_hal_set_window(0, dc->fb_height * dc->section_count, dc->fb_width, dc->fb_height);
        rtk_lcd_hal_start_transfer(dc->frame_buf, dc->fb_width * dc->fb_height);
    }

}

static struct gui_dispdev dc =
{
    .type = DC_RAMLESS,
    .section = {0, 0, 0, 0},
    .section_count = 0,

    .lcd_update = port_gui_lcd_update,
    .lcd_power_off = gui_dc_lcd_power_off,
    .lcd_power_on = gui_dc_lcd_power_on,

    .reset_lcd_timer = NULL,
    .get_lcd_us = NULL,

    .lcd_te_wait = NULL,

};

static uint8_t __attribute__((aligned(4))) __attribute__((
                                                             used)) disp_write_buff1_port[DRV_LCD_WIDTH *
                                                                                   LCD_SECTION_HEIGHT * 2];
static uint8_t __attribute__((aligned(4))) __attribute__((
                                                             used)) disp_write_buff2_port[DRV_LCD_WIDTH *
                                                                                   LCD_SECTION_HEIGHT * 2];

void gui_port_dc_init(void)
{
    rtk_lcd_hal_init();
    dc.frame_buf = NULL;
    dc.fb_height = LCD_SECTION_HEIGHT;
    dc.fb_width = rtk_lcd_hal_get_width();
    dc.disp_buf_1 = disp_write_buff1_port;
    dc.disp_buf_2 = disp_write_buff2_port;
    dc.bit_depth = rtk_lcd_hal_get_pixel_bits();

    dc.screen_width =  rtk_lcd_hal_get_width();
    dc.screen_height = rtk_lcd_hal_get_height();

    gui_dc_info_register(&dc);
    gui_log("gui_port_dc_init ");
    gui_log("dc addr is 0x%x,line is %d", &dc, __LINE__);
    app_dlps_disable(APP_DLPS_ENTER_CHECK_DISPLAY);

    U_CLK_BITMAP bitmap;
    bitmap.data = BIT(T_CLK_TYPE_CPU) | BIT(T_CLK_TYPE_SPIC0) | BIT(T_CLK_TYPE_SPIC1) | BIT(
                      T_CLK_TYPE_SPIC3);
    clk_user_gui = clk_mgr_user_create("gui", bitmap);
    clk_mgr_set_high_performance(clk_user_gui);
}
