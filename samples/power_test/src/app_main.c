/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include <stdint.h>
#include <string.h>
#include "board.h"
#include "os_msg.h"
#include "os_task.h"
#include "os_queue.h"
#include "os_sched.h"
#include "rtl876x_pinmux.h"
#include "gap.h"
#include "gap_br.h"
#include "btm.h"
#include "sysm.h"
#include "remote.h"
#include "audio.h"
#include "trace.h"
#include "power_test.h"
#include "app_msg.h"
#include "app_io_msg.h"
#include "app_dlps.h"
#ifdef CONFIG_SOC_SERIES_RTL8773D
#if F_APP_EXTERNAL_BUCK_SUPPORT
#include "dvfs_api.h"
#include "ext_buck.h"
#include "app_buck_tps62860.h"
#else
#include "dvfs_api.h"
#include "pm.h"
#endif
#else
#include "pm.h"
#endif
#include "power_test_sdp.h"
#include "power_test_app.h"
#include "power_test_app_a2dp.h"
#include "power_test_app_avrcp.h"
#include "power_test_app_hfp.h"
#include "power_test_app_hfp_ag.h"
#include "power_test_flash.h"
#include "ble_mgr.h"
#ifdef CONFIG_SOC_SERIES_RTL87X3G
#include "rtl876x_uart.h"
#endif

#if (F_APP_PSRAM_ENABLE == 1)
#include "module_psram.h"
#endif

#define POWER_TEST_DEFAULT_PAGESCAN_WINDOW             0x12
#define POWER_TEST_DEFAULT_PAGESCAN_INTERVAL           0x800 //0x800
#define POWER_TEST_DEFAULT_PAGE_TIMEOUT                0xFA00 //40s
#define POWER_TEST_DEFAULT_SUPVISIONTIMEOUT            0x7D00 //20s
#define POWER_TEST_DEFAULT_INQUIRYSCAN_WINDOW          0x12
#define POWER_TEST_DEFAULT_INQUIRYSCAN_INTERVAL        0x1000 //0x1000

#define PLAYBACK_POOL_SIZE                  (13*1024)
#define VOICE_POOL_SIZE                     (2*1024)
#define RECORD_POOL_SIZE                    (1*1024)
#define NOTIFICATION_POOL_SIZE              (8*1024)

#define MAX_NUMBER_OF_GAP_MESSAGE       0x20    //!< indicate BT stack message queue size
#define MAX_NUMBER_OF_IO_MESSAGE        0x40    //!< indicate io queue size, extra 0x20 for data uart
#define MAX_NUMBER_OF_GAP_TIMER         0x10    //!< indicate gap timer queue size
#define MAX_NUMBER_OF_DSP_MSG           0x20    //!< number of dsp message reserved for DSP message handling.
#define MAX_NUMBER_OF_CODEC_MSG         0x20    //!< number of codec message reserved for CODEC message handling.
#define MAX_NUMBER_OF_SYS_MSG           0x20    //!< indicate SYS timer queue size
#define MAX_NUMBER_OF_LOADER_MSG        0x10    //!< indicate Bin Loader queue size
/** indicate rx event queue size*/
#define MAX_NUMBER_OF_RX_EVENT      \
    (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE  +  MAX_NUMBER_OF_DSP_MSG + MAX_NUMBER_OF_CODEC_MSG + MAX_NUMBER_OF_GAP_TIMER + MAX_NUMBER_OF_SYS_MSG + MAX_NUMBER_OF_LOADER_MSG)

void *power_test_queue_handle;
void *audio_evt_queue_handle;
void *audio_io_queue_handle;

/**
  * @brief  Initialize gap bond manager related parameters
  * @return void
  */
static void app_bt_gap_init(void)
{
    uint32_t class_of_device = (uint32_t)(0x18 | (0x04 << 8) | (0x24 << 16));
    uint16_t supervision_timeout = POWER_TEST_DEFAULT_SUPVISIONTIMEOUT;

    uint16_t link_policy = GAP_LINK_POLICY_DISABLE_ALL;

    uint8_t radio_mode = GAP_RADIO_MODE_NONE_DISCOVERABLE;
    bool limited_discoverable = false;
    bool auto_accept_acl = false;

    uint8_t pagescan_type = GAP_PAGE_SCAN_TYPE_INTERLACED;
    uint16_t pagescan_interval = POWER_TEST_DEFAULT_PAGESCAN_INTERVAL;
    uint16_t pagescan_window = POWER_TEST_DEFAULT_PAGESCAN_WINDOW;
    uint16_t page_timeout = POWER_TEST_DEFAULT_PAGE_TIMEOUT;

    uint8_t inquiryscan_type = GAP_INQUIRY_SCAN_TYPE_INTERLACED;
    uint16_t inquiryscan_window = POWER_TEST_DEFAULT_INQUIRYSCAN_WINDOW;
    uint16_t inquiryscan_interval = POWER_TEST_DEFAULT_INQUIRYSCAN_INTERVAL;
    uint8_t inquiry_mode = GAP_INQUIRY_MODE_EXTENDED_RESULT;

    uint8_t pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_GENERAL_BONDING_FLAG | GAP_AUTHEN_BIT_SC_FLAG;
    uint8_t io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t oob_enable = false;
    uint8_t bt_mode = GAP_BT_MODE_21ENABLED;

    gap_lib_init();

    gap_br_set_param(GAP_BR_PARAM_NAME, GAP_DEVICE_NAME_LEN, "power_test");

    //1: to be slave when accept the acl connect request by default.
    gap_br_cfg_accept_role(0);

    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(uint8_t), &pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(uint16_t), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(uint8_t), &io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(uint8_t), &oob_enable);

    gap_br_set_param(GAP_BR_PARAM_BT_MODE, sizeof(uint8_t), &bt_mode);
    gap_br_set_param(GAP_BR_PARAM_COD, sizeof(uint32_t), &class_of_device);
    gap_br_set_param(GAP_BR_PARAM_LINK_POLICY, sizeof(uint16_t), &link_policy);
    gap_br_set_param(GAP_BR_PARAM_SUPV_TOUT, sizeof(uint16_t), &supervision_timeout);
    gap_br_set_param(GAP_BR_PARAM_AUTO_ACCEPT_ACL, sizeof(bool), &auto_accept_acl);


    gap_br_set_param(GAP_BR_PARAM_RADIO_MODE, sizeof(uint8_t), &radio_mode);
    gap_br_set_param(GAP_BR_PARAM_LIMIT_DISCOV, sizeof(bool), &limited_discoverable);

    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_TYPE, sizeof(uint8_t), &pagescan_type);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_INTERVAL, sizeof(uint16_t), &pagescan_interval);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_WINDOW, sizeof(uint16_t), &pagescan_window);
    gap_br_set_param(GAP_BR_PARAM_PAGE_TIMEOUT, sizeof(uint16_t), &page_timeout);

    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_TYPE, sizeof(uint8_t), &inquiryscan_type);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_INTERVAL, sizeof(uint16_t), &inquiryscan_interval);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_WINDOW, sizeof(uint16_t), &inquiryscan_window);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_MODE, sizeof(uint8_t), &inquiry_mode);

    power_ble_gap_param_init();
}

/**
 * @brief    Contains the initialization of framework
 * @return   void
 */
void framework_init(void)
{
    /* System Manager */
    sys_mgr_init(audio_evt_queue_handle);

    /* RemoteController Manager */
    remote_mgr_init(REMOTE_SESSION_ROLE_SINGLE);

    /* Bluetooth Manager */
    bt_mgr_init();

    /* Audio Manager */
    audio_mgr_init(PLAYBACK_POOL_SIZE, VOICE_POOL_SIZE, RECORD_POOL_SIZE, NOTIFICATION_POOL_SIZE);
}

static void board_init(void)
{
    /*use zephyr shell*/

#ifdef CONFIG_SOC_SERIES_RTL8773D
#if F_APP_EXTERNAL_BUCK_SUPPORT
    Pinmux_Config(ADC_1, I2C0_DAT);
    Pinmux_Config(ADC_0, I2C0_CLK);

    Pad_Config(ADC_1, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(ADC_0, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_PullConfigValue(ADC_1, PAD_STRONG_PULL);
    Pad_PullConfigValue(ADC_0, PAD_STRONG_PULL);
#else
    Pad_Config(ADC_1, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_Config(ADC_0, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
#endif
#endif
}

#ifdef CONFIG_SOC_SERIES_RTL8773D
#if !F_APP_EXTERNAL_BUCK_SUPPORT
bool ext_buck_set_voltage_empty_func(DVFSVDDMode dvfs_volt)
{
    return true;
}
#endif
#endif

static void driver_init(void)
{
#ifdef CONFIG_SOC_SERIES_RTL8773D
#if F_APP_EXTERNAL_BUCK_SUPPORT
    app_buck_tps62860_init(P5_1);
    ext_buck_vcore2_enable();
#else
    dvfs_register_voltage_func(DVFS_LOW_VDD, ext_buck_set_voltage_empty_func);
#endif
#endif

    /*
     * Console/CLI is provided by the Zephyr shell (see prj.conf + the board
     * overlay's zephyr,shell-uart). The RTK console_uart transport and
     * cli_cmd_register front-end have been replaced by SHELL_CMD_REGISTER.
     */
}

static void app_task(void *pvParameters)
{
    uint8_t event;

    gap_start_bt_stack(audio_evt_queue_handle, audio_io_queue_handle, MAX_NUMBER_OF_GAP_TIMER);

    while (true)
    {
        if (os_msg_recv(audio_evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
            //DBG_DIRECT("audio_evt_queue_handle 0x%x event 0x%x 0x%x", audio_evt_queue_handle, &event, event);
            if (EVENT_GROUP(event) == EVENT_GROUP_IO)
            {
                T_IO_MSG io_msg;

                if (os_msg_recv(audio_io_queue_handle, &io_msg, 0) == true)
                {
                    if (event == EVENT_IO_TO_APP)
                    {
                        app_io_msg_handler(io_msg);
                    }
                }
            }
            else if (EVENT_GROUP(event) == EVENT_GROUP_STACK)
            {
                gap_handle_msg(event);
            }
            else if (EVENT_GROUP(event) == EVENT_GROUP_FRAMEWORK)
            {
                //sys_mgr_event_handle(event);
            }

        }
    }
}

void app_ble_gap_init(void)
{
    le_register_app_cb(power_ble_gap_cb);

    BLE_MGR_PARAMS param      = {0};
    param.ble_ext_adv.enable  = true;
    param.ble_ext_adv.adv_num = 2;
    param.ble_conn.enable     = true;
    param.ble_conn.link_num   = 2;
    ble_mgr_init(&param);
    power_le_init_adv_param();
}

int main(void)
{
    __enable_irq();
    void *app_task_handle;

    DBG_DIRECT("power test project");

    os_msg_queue_create(&audio_io_queue_handle, "audio ioQ", MAX_NUMBER_OF_IO_MESSAGE,
                        sizeof(T_IO_MSG));
    os_msg_queue_create(&audio_evt_queue_handle, "audio evtQ", MAX_NUMBER_OF_RX_EVENT,
                        sizeof(unsigned char));

    board_init();
    driver_init();

    app_bt_gap_init();
    app_dlps_init();
    framework_init();
#if (F_APP_PSRAM_ENABLE == 1)
    app_psram_init();
#endif
    power_test_sdp_init();
    app_power_test_init();
    app_power_test_hfp_ag_init();
    app_power_test_hfp_init();
    app_power_test_avrcp_init();
    app_power_test_a2dp_init();
    app_ble_gap_init();
    os_task_create(&app_task_handle, "app_task", app_task, NULL, 1024 * 3, 1);

    return 0;
}
