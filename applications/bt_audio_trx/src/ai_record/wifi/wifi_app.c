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
#include "app_uart_atcmd.h"
#if CONFIG_APP_WIFI_SDIO
#include "wifi_sdio.h"
#endif
#include "app_wifi_uart.h"
#include "wifi_bt_coexist.h"
#include <os_msg.h>
#include <os_task.h>
#include <os_sched.h>
#include "rtl876x_pinmux.h"
#include "wdg.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
//#include "app_module_init.h"

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
#if F_APP_WIFI_UART_CMD
                app_uart_atcmd_rsp_handler();
#endif
            }
            else if (wifi_msg.event == EVENT_UART_CMD_FLOW_CTRL)
            {
#if F_APP_WIFI_UART_CMD
                app_uart_atcmd_trigger_send_flow();
#endif
            }
#if CONFIG_APP_WIFI_SDIO
            else if (wifi_msg.event == EVENT_SDIO_INT)
            {
                wifi_sdio_msg_handler();
            }
#endif
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

        Pad_Config(RF_SWITCH_V1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE,
                   PAD_OUT_LOW);//low disable wifi ant
        Pad_Config(RF_SWITCH_V2, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_ENABLE,
                   PAD_OUT_HIGH);//high disable wifi ant
    }
}
#define  WIFI_DEBUG_EN 1
char  ip_test[] = "192.168.3.74";
int wifi_test_cfg(void);

/* Create the WiFi task + message queue exactly once. Quick and non-blocking,
 * so it is safe to call from a BLE/GATT callback. Idempotent: the second
 * caller (e.g. the 0x8401 power-on path vs. the CMD_WIFI_CONNECT path) is a
 * no-op, so the task is never double-created. */
void wifi_task_ensure(void)
{
    static bool task_created = false;

    if (task_created)
    {
        return;
    }
    task_created = true;

    wifi_bt_coexist_init();
    wifi_task_init();
}

/* Blocking module bring-up: chip_en timing (~2.2s in wifi_enable) + RF
 * switches + SDIO data path + upload port. MUST run on the WiFi task, never
 * in a BLE callback. */
void wifi_power_on(void)
{
    wifi_enable(true);
    wifi_sdio_init();
}

void wifi_init(void)
{
    wifi_task_ensure();

#if   WIFI_DEBUG_EN
    // extern void log_test(void);
    // log_test();
    wifi_power_on();
    //app_wifi_uart_init();
#endif

    //wifi_test_cfg();
    /* WiFi antenna control GPIO (disabled for now) */
    // wifi_ant_ctrl_gpio_init();
    // os_delay(1000);
    // cmd_list_demo();
    //  os_delay(17000);
//   cmd_wifi_upload_file(ip_test);
}

static void wifi_module_init(void)
{
    wifi_init();
}
//APP_MODULE_INIT(wifi_module_init);

#if 0
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "sdcard_wifi_cfg.h"

LOG_MODULE_REGISTER(wifi_app, LOG_LEVEL_INF);
char pre_ip[WIFI_CFG_MAX_VALUE_LEN];
int wifi_test_cfg(void)
{
    char value[WIFI_CFG_MAX_VALUE_LEN];

    /* ----------------------------------------------------------------
     * Demo 1: Dump all config entries
     * ---------------------------------------------------------------- */
    LOG_INF("---- Dump all config ----");
    sdcard_wifi_cfg_dump();

    /* ----------------------------------------------------------------
     * Demo 2: Read individual keys
     * ---------------------------------------------------------------- */
    LOG_INF("---- Read individual keys ----");

    if (sdcard_wifi_cfg_get(WIFI_CFG_KEY_SSID, value, sizeof(value)) == WIFI_CFG_OK)
    {
        LOG_INF("SSID     : %s", value);
    }

    if (sdcard_wifi_cfg_get(WIFI_CFG_KEY_PASSWORD, value, sizeof(value)) == WIFI_CFG_OK)
    {
        LOG_INF("Password : %s", value);
    }

    if (sdcard_wifi_cfg_get(WIFI_CFG_KEY_IP, pre_ip, sizeof(pre_ip)) == WIFI_CFG_OK)
    {
        LOG_INF("IP Addr  : %s", pre_ip);
    }

    /* ----------------------------------------------------------------
     * Demo 3: Update key values
     * ---------------------------------------------------------------- */
    LOG_INF("---- Update config values ----");

    // sdcard_wifi_cfg_set(WIFI_CFG_KEY_SSID, "NewSSID_2026");
    // sdcard_wifi_cfg_set(WIFI_CFG_KEY_PASSWORD, "newpassword99");
    // sdcard_wifi_cfg_set(WIFI_CFG_KEY_IP, "10.0.0.100");

    /* ----------------------------------------------------------------
     * Demo 4: Verify updated values
     * ---------------------------------------------------------------- */
    // LOG_INF("---- Verify updated config ----");
    // sdcard_wifi_cfg_dump();

    return 0;
}

#endif