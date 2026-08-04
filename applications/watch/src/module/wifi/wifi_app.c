/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sd/sd.h>
#include <zephyr/sd/sdio.h>
#include "wifi_app.h"
#include "trace.h"
#include "wifi_atcmd.h"
#include "wifi_sdio.h"
#include "wifi_uart.h"
#include "wifi_bt_coexist.h"
#include <os_msg.h>
#include <os_task.h>
#include <os_sched.h>
#include "rtl876x_pinmux.h"
#include "wdg.h"
#include "app_module_init.h"

//watch demo board
// #define WIFI_EN_PIN             P2_3
// #define RF_SWITCH_V2            P2_4
// #define RF_SWITCH_V1            P1_1

//EVB
// #define WIFI_EN_PIN             P2_7
// #define RF_SWITCH_V2            P1_1
// #define RF_SWITCH_V1            P4_5

//GBF
#define WIFI_EN_PIN             P6_4
#define RF_SWITCH_V2            P6_2
#define RF_SWITCH_V1            P6_0

static void *wifi_msg_queue_handle;
static void *wifi_task_handle;

static void wifi_task(void *param);

bool app_send_msg_to_wifitask(T_WIFI_MSG *p_msg)
{
    if (os_msg_send(wifi_msg_queue_handle, p_msg, 0) == false)
    {
        APP_PRINT_ERROR0("send_msg_to_wifi fail !");
        return false;
    }
    return true;
}

void wifi_task_init(void)
{
    os_msg_queue_create(&wifi_msg_queue_handle, "wifi msg queue", 0x10, sizeof(T_WIFI_MSG));
    os_task_create(&wifi_task_handle, "wifi task", wifi_task, NULL, 384 * 8, 2);


}

static void wifi_task(void *param)
{
    T_WIFI_MSG  wifi_msg;

    while (1)
    {
        wdg_kick();
        if (os_msg_recv(wifi_msg_queue_handle, &wifi_msg, 0xFFFFFFFF) == true)
        {
            if (wifi_msg.event == EVENT_UART_RX)
            {
                uart_atcmd_rsp_handler();
            }
            else if (wifi_msg.event == EVENT_UART_CMD_FLOW_CTRL)
            {
                uart_atcmd_flow_ctrl_handler(wifi_msg);
            }
            else if (wifi_msg.event == EVENT_SDIO_INT)
            {
                wifi_sdio_msg_handler();
            }
            else if (wifi_msg.event == EVENT_USER_APP_DEFINE)
            {
                if (wifi_msg.msg_cb)
                {
                    wifi_msg.msg_cb(&wifi_msg);
                }
            }
        }
    }
}


void wifi_enable(bool enable)
{
    if (enable)
    {
        APP_PRINT_INFO0(" wifi enable");
        Pad_Config(WIFI_EN_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
                   PAD_OUT_LOW);//high enable wifi
        os_delay(200);
        Pad_Config(WIFI_EN_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
                   PAD_OUT_HIGH);//high enable wifi
        os_delay(2000);
        // Pad_Config(RF_SWITCH_V1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
        //            PAD_OUT_HIGH);//high enable wifi
        // Pad_Config(RF_SWITCH_V2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE,
        //            PAD_OUT_LOW);//high enable wifi
        Pad_Config(RF_SWITCH_V1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
                   PAD_OUT_HIGH);
        Pinmux_Config(RF_SWITCH_V1, EN_EXLNA);

        Pad_Config(RF_SWITCH_V2, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE,
                   PAD_OUT_HIGH);
        Pinmux_Config(RF_SWITCH_V2, EN_EXPA);
    }
    else
    {
        APP_PRINT_INFO0(" wifi disable");
        Pad_Config(WIFI_EN_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
                   PAD_OUT_LOW);//high enable wifi
    }
}

void wifi_init(void)
{
    wifi_bt_coexist_init();
    wifi_task_init();
}

static void wifi_module_init(void)
{
    wifi_init();
}
APP_MODULE_INIT(wifi_module_init);


