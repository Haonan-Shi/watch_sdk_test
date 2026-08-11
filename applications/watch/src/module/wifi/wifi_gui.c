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
#include "string.h"
#include "gap.h"
#include "app_version.h"
#include "app_ble_adv.h"
#include "app_msg.h"
#include "wdg.h"
#include "app_main.h"
#include <zephyr/drivers/rtc.h>
#include "hub_clock.h"
#include "wifi_app.h"
#include "app_task.h"
#include "gui_message.h"
#if CONFIG_WIFI_CAMERA
#include "rtsp/wifi_rtsp_app.h"
#endif
#include "autoconf.h"
#include "wifi_gui.h"


void wifi_gui_to_app(T_WIFI_GUI_TO_APP_TYPE type, void *data)
{
#if (CONFIG_WIFI_RTSP || CONFIG_WIFI_JPEG_PARSER)
    //(T_WIFI_EVENT)type;
    if (type == WIFI_GUI_CAMERA_ENTER)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;//EVENT_WIFI_CAM_ENTER;
        cmd_msg.u.buf = (void *)data;/*ip addrress*/
        cmd_msg.msg_cb = wifi_camera_enter_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi task msg send fail !");
        }
    }
    else if (type == WIFI_GUI_CAMERA_EXIT)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;//EVENT_WIFI_CAM_EXIT;
        cmd_msg.msg_cb = wifi_camera_exit_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi task msg send fail !");
        }
    }

    else if (type == WIFI_GUI_POWER_ON)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;//EVENT_WIFI_POWER_ON;
        cmd_msg.msg_cb = wifi_power_on_proc;
        // cmd_msg.u.buf = (void *)ip_addr;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi task msg send fail !");
        }
    }
#endif //#if (CONFIG_WIFI_RTSP || CONFIG_WIFI_JPEG_PARSER)

#if defined(CONFIG_WIFI_TEST) || defined(CONFIG_WIFI)
    /* WiFi Test GUI event handling */
    if (type == WIFI_GUI_TEST_ENTER)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.u.buf = data;  /* optional data */
        cmd_msg.msg_cb = wifi_test_enter_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test enter msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_EXIT)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.msg_cb = wifi_test_exit_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test exit msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_SCAN)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.msg_cb = wifi_test_scan_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test scan msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_IPERF_UPLOAD_START)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.u.buf = data;  /* target IP address */
        cmd_msg.msg_cb = wifi_test_iperf_upload_start_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test iperf upload start msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_IPERF_UPLOAD_STOP)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.msg_cb = wifi_test_iperf_upload_stop_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test iperf upload stop msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_IPERF_DOWNLOAD_START)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.u.buf = data;  /* target IP address */
        cmd_msg.msg_cb = wifi_test_iperf_download_start_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test iperf download start msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_IPERF_DOWNLOAD_STOP)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.msg_cb = wifi_test_iperf_download_stop_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test iperf download stop msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_FILE_UPLOAD_START)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.u.buf = data;  /* target IP address */
        cmd_msg.msg_cb = wifi_test_file_upload_start_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test file upload start msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_FILE_UPLOAD_STOP)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.msg_cb = wifi_test_file_upload_stop_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test file upload stop msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_FILE_DOWNLOAD_START)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.u.buf = data;  /* target IP address */
        cmd_msg.msg_cb = wifi_test_file_download_start_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test file download start msg send fail !");
        }
        APP_PRINT_INFO0("WIFI_GUI_TEST_FILE_DOWNLOAD_START  ");
    }
    else if (type == WIFI_GUI_TEST_FILE_DOWNLOAD_STOP)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.msg_cb = wifi_test_file_download_stop_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test file download stop msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_SSID_CONNECT)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.u.buf = data;  /* SSID info */
        cmd_msg.msg_cb = wifi_test_ssid_connect_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test ssid connect msg send fail !");
        }
    }
    else if (type == WIFI_GUI_TEST_DISCONNECT)
    {
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;
        cmd_msg.msg_cb = wifi_test_disconnect_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi test disconnect msg send fail !");
        }
    }
#endif /* CONFIG_WIFI_TEST || CONFIG_WIFI */
}


/* WiFi Test callback function implementations (stub) */
#if defined(CONFIG_WIFI_TEST) || defined(CONFIG_WIFI)
#include "wifi_atcmd.h"
void wifi_test_enter_proc(void *msg)
{
    (void)msg;
    APP_PRINT_INFO0("[wifi_test] enter proc");
    wifi_enable(true);
    wifi_sdio_init();
    wifi_uart_init();
    /* WiFi antenna control GPIO (disabled for now) */
    // wifi_ant_ctrl_gpio_init();
    // os_delay(1000);
    cmd_list_demo();
    //log_test();
}

void wifi_test_exit_proc(void *msg)
{
    (void)msg;
    APP_PRINT_INFO0("[wifi_test] exit proc");
    // TODO: Implement WiFi test exit logic
    // - Stop any ongoing test
    // - Clean up test state
}

void wifi_test_scan_proc(void *msg)
{
    (void)msg;
    APP_PRINT_INFO0("[wifi_test] scan proc");
    // TODO: Implement WiFi scan logic
    // - Trigger WiFi scan
    // - Update UI with scan results
}

void wifi_test_iperf_upload_start_proc(void *msg)
{
    T_WIFI_MSG *wifi_msg = (T_WIFI_MSG *)msg;
    char *target_ip = (wifi_msg && wifi_msg->u.buf) ? (char *)wifi_msg->u.buf : NULL;
    (void)target_ip;

    APP_PRINT_INFO0("[wifi_test] iperf upload start proc");
    cmd_wifi_set_client(target_ip, NULL);
    // TODO: Implement iperf upload start logic
    // - Start iperf client (upload mode)
    // - Connect to target IP
    // - Start throughput test
}

void wifi_test_iperf_upload_stop_proc(void *msg)
{
    (void)msg;
    APP_PRINT_INFO0("[wifi_test] iperf upload stop proc");
    // TODO: Implement iperf upload stop logic
    // - Stop iperf client
    // - Display results
}

void wifi_test_iperf_download_start_proc(void *msg)
{
    T_WIFI_MSG *wifi_msg = (T_WIFI_MSG *)msg;
    char *target_ip = (wifi_msg && wifi_msg->u.buf) ? (char *)wifi_msg->u.buf : NULL;
    (void)target_ip;

    APP_PRINT_INFO0("[wifi_test] iperf download start proc");
    cmd_wifi_set_server(NULL);
    // TODO: Implement iperf download start logic
    // - Start iperf server mode or connect to server
    // - Start download throughput test
}

void wifi_test_iperf_download_stop_proc(void *msg)
{
    (void)msg;
    APP_PRINT_INFO0("[wifi_test] iperf download stop proc");
    // TODO: Implement iperf download stop logic
    // - Stop iperf test
    // - Display results
}

void wifi_test_file_upload_start_proc(void *msg)
{
    T_WIFI_MSG *wifi_msg = (T_WIFI_MSG *)msg;
    char *target_ip = (wifi_msg && wifi_msg->u.buf) ? (char *)wifi_msg->u.buf : NULL;
    (void)target_ip;

    APP_PRINT_INFO0("[wifi_test] file upload start proc");
    //char *ip_addr = "192.168.3.74";
    cmd_wifi_upload_file(target_ip);
    // TODO: Implement file upload start logic
    // - Start file upload to target IP
    // - Show progress
}

void wifi_test_file_upload_stop_proc(void *msg)
{
    (void)msg;
    APP_PRINT_INFO0("[wifi_test] file upload stop proc");

    // TODO: Implement file upload stop logic
    cmd_wifi_upload_file_stop();
    // - Display results
}

void wifi_test_file_download_start_proc(void *msg)
{
    T_WIFI_MSG *wifi_msg = (T_WIFI_MSG *)msg;
    char *target_ip = (wifi_msg && wifi_msg->u.buf) ? (char *)wifi_msg->u.buf : NULL;
    (void)target_ip;

    APP_PRINT_INFO0("[wifi_test] file download start proc");
    cmd_wifi_set_server(NULL);
    // TODO: Implement file download start logic
    // - Start file download from target IP
    // - Show progress
}

void wifi_test_file_download_stop_proc(void *msg)
{
    (void)msg;
    APP_PRINT_INFO0("[wifi_test] file download stop proc");
    // TODO: Implement file download stop logic
    // - Stop file download
    // - Display results
}

void wifi_test_ssid_connect_proc(void *msg)
{
    T_WIFI_MSG *wifi_msg = (T_WIFI_MSG *)msg;
    (void)wifi_msg;

    APP_PRINT_INFO0("[wifi_test] ssid connect proc");
    // TODO: Implement SSID connect logic
    // - Connect to specified SSID
    // - Update connection status
}

void wifi_test_disconnect_proc(void *msg)
{
    (void)msg;
    APP_PRINT_INFO0("[wifi_test] disconnect proc");
    // TODO: Implement disconnect logic
    // - Disconnect from current AP
    // - Update UI status
}

#endif /* CONFIG_WIFI_TEST || CONFIG_WIFI */
