/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdint.h>
#include <string.h>
#include "os_mem.h"
#include "os_sched.h"
#include "os_msg.h"
#include "os_task.h"
//#include "os_ext.h"
#include "system_status_api.h"
#include "rtl876x_pinmux.h"
#include "hal_gpio_int.h"
#include "hal_gpio.h"
#include "hal_i2c.h"
#include "dlps_util.h"
#include "trace.h"
#include "audio.h"
#include "audio_probe.h"
#include "sysm.h"
#include "gap_br.h"
#include "gap.h"
#include "test_mode.h"
#include "single_tone.h"
#include "fmc_api.h"
#include "spp_stream.h"
#include "ble_stream.h"
#include "iap_stream.h"
#include "app_console.h"
#include "app_cfg.h"
#include "app_ipc.h"
#include "app_dsp_cfg.h"
#include "app_cfg_nv.h"
#include "app_audio_cfg.h"
#include "app_charger_cfg.h"
#include "app_main.h"
#include "app_gap.h"
#include "app_io_msg.h"
#include "app_ble_cfg.h"
#include "app_ble_gap.h"
#include "app_ble_client.h"
#include "app_ble_service.h"
#include "app_dlps.h"
#include "app_bt_policy_cfg.h"
#include "app_bt_policy_api.h"
#include "app_sdp.h"
#include "app_transfer_cfg.h"
#include "app_transfer.h"
#include "app_timer.h"
#include "app_audio_policy.h"
#include "app_a2dp_cfg.h"
#include "app_a2dp.h"
#include "app_hfp_cfg.h"
#include "app_hfp.h"
#include "app_avrcp_cfg.h"
#include "app_avrcp.h"
#include "app_iap_cfg.h"
#include "app_iap.h"
#include "app_spp_cfg.h"
#include "app_spp.h"
#include "app_cmd.h"
#include "app_ble_device.h"


#include "app_mmi.h"
#include "app_customer.h"
#include "app_bond.h"

#include "app_bond.h"
#include "app_adp.h"

#include "app_pan.h"
#include "app_fs_if.h"

#if F_APP_EXT_AUDIO_AMP_SUPPORT
#include "app_ext_audio_amp.h"
#endif

#include "app_line_in.h"

#if CONFIG_REALTEK_APP_GUI
#include "app_panel_init.h"
#endif

#if F_APP_BT_PROFILE_PBAP_PCE_SUPPORT
#include "app_pbap_cfg.h"
#include "app_pbap.h"
#endif

#include "app_hfp_ag.h"

#if F_APP_BT_PROFILE_MAP_MCE_SUPPORT
#include "app_map.h"
#include "app_map_cfg.h"
#endif

#if F_APP_TEST_SUPPORT
#include "app_test.h"
#endif
#if F_APP_LINEIN_SUPPORT
#include "app_line_in.h"
#endif

#if F_APP_BT_HID_DEVICE_SUPPORT
#include "app_hid_cfg.h"
#include "app_hid.h"
#endif

#if F_APP_BT_HID_HOST_SUPPORT
#include "app_hid_cfg.h"
#include "app_hid_host.h"
#endif

#if F_APP_QDECODE_SUPPORT
#include "app_qdec.h"
#endif
#if F_APP_IAP_RTK_SUPPORT
#include "app_iap_rtk.h"
#endif

#if CONFIG_REALTEK_APP_BOND_MGR_SUPPORT
#include "bt_bond_api.h"
#if (F_APP_LE_AUDIO_ACCEPTOR_SUPPORT || F_APP_LE_AUDIO_INITIATOR_SUPPORT)
#include "ble_audio_bond.h"
#endif
#endif

#if F_APP_HIFI4_SUPPORT
#include "app_buck_tps62860.h"
#include "app_buck_apw7564.h"
#include "ipc.h"
#endif
#ifdef F_APP_DEBUG_TASK_PROFILING
#include "hal_debug.h"
#endif

#if (F_APP_A2DP_XMIT_SRC_SUPPORT || F_APP_A2DP_XMIT_SRC_LEA_SUPPORT)
#include "app_a2dp_xmit_mgr.h"
#endif

#if (F_APP_SCO_XMIT_AG_SUPPORT || F_APP_SCO_XMIT_HF_SUPPORT)
#include "app_sco_xmit_mgr.h"
#endif

#if F_SOURCE_PLAY_SUPPORT
#include "app_src_play.h"
#endif

#if BAP_BROADCAST_SOURCE
#include "app_lea_ini_profile.h"
#endif

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include "app_findmy_task.h"
#include "app_findmy_ble.h"
#include "app_findmy.h"
#endif

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "app_gfps_cfg.h"
#include "app_gfps.h"
#include "app_gfps_msg.h"
#include "ecc_enhanced.h"
#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
#include "app_dult.h"
#include "app_dult_device.h"
#endif
#endif

#if F_APP_DISABLE_NOTIFICATION_SUPPORT
#include "ringtone.h"
#include "voice_prompt.h"
#endif

#if F_APP_DATA_CAPTURE_SUPPORT
#include "app_data_capture_cs.h"
#endif

#if F_APP_AUTO_POWER_TEST_LOG
#include "app_power_test.h"
#endif

#ifndef CONFIG_SOC_SERIES_RTL8773D
//#include "mem_config.h"
#endif

#include "bt_gatt_svc.h"
#include "app_ota.h"

#if F_APP_USB_AUDIO_SUPPORT | F_APP_USB_MSC_SUPPORT | F_APP_USB_HID_SUPPORT | F_APP_USB_CDC_SUPPORT
#include "app_usb.h"
#endif

#if F_APP_LE_AUDIO_ACCEPTOR_SUPPORT
#include "app_lea_acc_profile.h"
#endif


#if TRANSMIT_CLIENT_SUPPORT
#include "app_le_transfer.h"
#endif

#if F_APP_BT_AUDIO_TRI_DONGLE
#include "fmc_api_ext.h"
#include "app_tri_dongle_mgr.h"
#include "app_tri_dongle_cmd.h"
#if F_APP_UWB_SCENARIO_SUPPORT
#include "app_tri_dongle_uwb.h"
#endif
#if CONFIG_YYLX_DONGLE_FEATURE
#include "app_tri_dongle_yylx_multi_uhid.h"
#endif
#endif

#if CONFIG_REALTEK_APP_GUI
#include "app_panel_msg.h"
#include "app_panel_init.h"
#endif

#if F_APP_CFU_FEATURE_SUPPORT
#include "app_common_cfu.h"
#endif

#if F_APP_CHARGING_CASE_CMD_SUPPORT
#include "app_charging_case_cmd.h"
#endif

#if F_APP_MALLEUS_SUPPORT
#include "app_malleus.h"
#endif

#if F_APP_MULTILINK_ENABLE
#include "app_multilink.h"
#endif

#if F_APP_AI_RECORD_PEN_DEMO_SUPPORT
#include "app_ai_record.h"
#endif

#if CONFIG_REALTEK_APP_AI_AUTH
#include "app_rtk_auth.h"
#endif

#if CONFIG_SOC_SERIES_RTL8773E
#include "fmc_platform.h"
#endif

#if (F_APP_LOG2FLASH_SUPPORT || F_APP_CORE_DUMP_SUPPORT)
#include "app_flash_dump.h"
#endif

#include <gap_vendor.h>

#if F_APP_WIFI_PTA_SUPPORT
#include <app_pta.h>
#endif

#if (F_APP_SPI_ROLE_MASTER || F_APP_SPI_ROLE_SLAVE)
#include "app_spi_api.h"
#endif

#if F_APP_WIFI_SPI_CMD
#include "app_spi_atcmd.h"
#endif

#if F_APP_WIFI_UART_CMD
#include "app_wifi_uart.h"
#include "app_uart_atcmd.h"
#endif

#if (CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
#include "mi_config.h"
#include "mible_api.h"
#include "mible_log.h"

#include "common/mible_beacon_internal.h"
#include "common/mible_beacon.h"

#if (HAVE_MSC == 0) //MSC: Mijia Secure Chip
#include "standard_auth/mible_standard_auth.h"
#else
#include "secure_auth/mible_secure_auth.h"
#endif

#include "mijia_profiles/mi_service_server.h"
#include "mijia_profiles/stdio_service_server.h"

#include "mijia_uart.h"
#include "mible_mcu.h"
#include "miio_user_api.h"

#if MI_MANU_TEST_ENABLE
#include "mijia_mp_cmd.h"
#include "mijia_mp_cmd_parser.h"
#endif
#if MI_USER_CMD_ENABLE
#include "mi_cmd.h"
#endif

#include "rtk_common.h"
#include "mijia_sample.h"
// #include "data_uart.h"
// #include "user_cmd_parse.h"
// #include "app_mijia_uart_cmd.h"
#endif

#include <zephyr/shell/shell.h>
#include "app_lower_init.h"
#include "pm.h"
#include "app_key_button.h"

#define MAX_NUMBER_OF_GAP_MESSAGE       0x20    //!< indicate BT stack message queue size
#define MAX_NUMBER_OF_IO_MESSAGE        0x20    //!< indicate io queue size
#define MAX_NUMBER_OF_DSP_MSG           0x20    //!< number of dsp message reserved for DSP message handling.
#define MAX_NUMBER_OF_CODEC_MSG         0x10    //!< number of codec message reserved for CODEC message handling.
#define MAX_NUMBER_OF_ANC_MSG           0x10    //!< number of anc message reserved for ANC message handling.
#define MAX_NUMBER_OF_SYS_MSG           0x20    //!< indicate SYS timer queue size
#define MAX_NUMBER_OF_LOADER_MSG        0x10    //!< indicate Bin Loader queue size
#define MAX_NUMBER_OF_APP_TIMER_MODULE  0x30    //!< indicate app timer module size
#define MAX_NUMBER_OF_GUI_MODULE        0x20    //!< indicate app timer module size

#if(CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
#define MI_TASK_QUEUE_SIZE              0x02
#undef  MAX_NUMBER_OF_IO_MESSAGE
#define MAX_NUMBER_OF_IO_MESSAGE        (0x20 + MI_TASK_QUEUE_SIZE + MIBLE_API_MSG_NUM + MI_UART_MSG_NUM)

#define EVENT_GROUP_MIJIA             0x08
#define EVENT_MI_TASK                 0x81
#define EVENT_MI_BLEAPI               0x82
#define EVENT_MI_UART                 0x83
#endif

/** indicate rx event queue size*/
#define MAX_NUMBER_OF_RX_EVENT      \
    (MAX_NUMBER_OF_GAP_MESSAGE + MAX_NUMBER_OF_IO_MESSAGE + \
     MAX_NUMBER_OF_DSP_MSG + MAX_NUMBER_OF_CODEC_MSG + MAX_NUMBER_OF_ANC_MSG + \
     MAX_NUMBER_OF_SYS_MSG + MAX_NUMBER_OF_LOADER_MSG + MAX_NUMBER_OF_GUI_MODULE)

#define DEFAULT_PAGESCAN_WINDOW         0x12
#define DEFAULT_PAGESCAN_INTERVAL       0x800
#define DEFAULT_PAGE_TIMEOUT            0x2000
#if F_APP_BT_AUDIO_TRI_DONGLE
#define DEFAULT_SUPVISIONTIMEOUT        0x1388
#else
#define DEFAULT_SUPVISIONTIMEOUT        0x1f40
#endif
#define DEFAULT_INQUIRYSCAN_WINDOW      0x12
#define DEFAULT_INQUIRYSCAN_INTERVAL    0x800

void *audio_evt_queue_handle;
void *audio_io_queue_handle;

T_APP_DB app_db = {};

#if (CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
extern mible_status_t mible_record_init(void);
extern void user_record_init(void);
#endif

/**
* @brief board_init() contains the initialization of pinmux settings and pad settings.
*
*   All the pinmux settings and pad settings shall be initiated in this function.
*   But if legacy driver is used, the initialization of pinmux setting and pad setting
*   should be peformed with the IO initializing.
*
* @return void
*/
static void board_init(void)
{
#if (CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
    mijia_sampe_rtc_init();
    hal_gpio_init_pin(MIJIA_PWR_PIN, GPIO_TYPE_AON, GPIO_DIR_OUTPUT, GPIO_PULL_NONE);
#endif

    if (app_cfg_const.dsp_log_output_select == DSP_OUTPUT_LOG_BY_UART)
    {
        Pinmux_Config(app_cfg_const.dsp_log_pin, UART2_TX);
        Pad_Config(app_cfg_const.dsp_log_pin,
                   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    }

#if F_APP_LINEIN_SUPPORT
    if (app_cfg_const.line_in_support)
    {
        app_line_in_driver_init();
        app_dlps_pad_wake_up_polarity_invert(app_cfg_const.line_in_pinmux);
    }
#endif

    //Config thermistor power pinmux
    if (app_cfg_const.thermistor_power_gpio_support && (app_cfg_const.thermistor_power_pinmux != 0xFF))
    {
        Pad_Config(app_cfg_const.thermistor_power_pinmux,
                   PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_LOW);
    }

#if F_APP_QDECODE_SUPPORT
    if (app_cfg_const.wheel_support)
    {
        app_qdec_pad_config();
    }
#endif

    if (app_cfg_const.dsp_gpio_num)
    {
        for (uint8_t i = 0; i < app_cfg_const.dsp_gpio_num; i++)
        {
            Pinmux_Config(app_cfg_const.dsp_gpio_pinmux[i], DSP_GPIO_OUT);
            Pad_Config(app_cfg_const.dsp_gpio_pinmux[i],
                       PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
        }
    }

    if (app_cfg_const.dsp_jtag_enable)
    {
        Pinmux_Config(app_cfg_const.dsp_jtck_pinmux, DSP_JTCK);
        Pad_Config(app_cfg_const.dsp_jtck_pinmux,
                   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

        Pinmux_Config(app_cfg_const.dsp_jtdi_pinmux, DSP_JTDI);
        Pad_Config(app_cfg_const.dsp_jtdi_pinmux,
                   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

        Pinmux_Config(app_cfg_const.dsp_jtdo_pinmux, DSP_JTDO);
        Pad_Config(app_cfg_const.dsp_jtdo_pinmux,
                   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

        Pinmux_Config(app_cfg_const.dsp_jtms_pinmux, DSP_JTMS);
        Pad_Config(app_cfg_const.dsp_jtms_pinmux,
                   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);

        Pinmux_Config(app_cfg_const.dsp_jtrst_pinmux, DSP_JTRST);
        Pad_Config(app_cfg_const.dsp_jtrst_pinmux,
                   PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP, PAD_OUT_DISABLE, PAD_OUT_LOW);
    }

#if F_APP_WIFI_PTA_SUPPORT
    Pinmux_Config(PIN_WL_ACT, BT_COEX_I_0);
    Pinmux_Config(PIN_BT_ACT, BT_COEX_O_1);
    Pinmux_Config(PIN_BT_STAT, BT_COEX_O_2);
    Pad_Config(PIN_WL_ACT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(PIN_BT_ACT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE, PAD_OUT_HIGH);
    Pad_Config(PIN_BT_STAT, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_DISABLE,
               PAD_OUT_HIGH);
    uint8_t param[1] = {1};
    gap_vendor_cmd_req(0xfdad, 1, param);
#endif
}

static void driver_init(void)
{
    app_adp_init();

#if F_APP_CONSOLE_SUPPORT
    if (app_cfg_const.enable_data_uart || app_cfg_const.one_wire_uart_support)
    {
        app_console_init();
    }
#endif

    app_charger_cfg_init();
    if (app_charger_cfg.charger_support || app_charger_cfg.discharger_support)
    {
        app_charger_init();
    }

#if F_APP_LINEIN_SUPPORT
    if (app_cfg_const.line_in_support)
    {
        app_line_in_driver_init();
        app_dlps_pad_wake_up_polarity_invert(app_cfg_const.line_in_pinmux);
    }
#endif

#if F_APP_EXT_RF_PA_SUPPORT
    if (app_cfg_const.ext_pa_enable)
    {
        Pinmux_Config(app_cfg_const.ext_pa_pinmux, EN_EXPA);
        Pad_Config(app_cfg_const.ext_pa_pinmux, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE,
                   PAD_OUT_DISABLE, PAD_OUT_LOW);
    }
    if (app_cfg_const.ext_lna_enable)
    {
        Pinmux_Config(app_cfg_const.ext_lna_pinmux, EN_EXLNA);
        Pad_Config(app_cfg_const.ext_lna_pinmux, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_NONE,
                   PAD_OUT_DISABLE, PAD_OUT_LOW);
    }
#endif

#if F_APP_EXT_AUDIO_AMP_SUPPORT
    app_ext_audio_amp_init();
#endif

#if F_APP_QDECODE_SUPPORT
    if (app_cfg_const.wheel_support)
    {
        app_qdec_init_status_read();
        app_qdec_driver_init();
    }
#endif

}

static void app_bt_gap_init(void)
{
    uint16_t supervision_timeout = DEFAULT_SUPVISIONTIMEOUT;
    uint16_t link_policy = GAP_LINK_POLICY_ROLE_SWITCH | GAP_LINK_POLICY_SNIFF_MODE;
    uint8_t radio_mode = GAP_RADIO_MODE_NONE_DISCOVERABLE;
    bool limited_discoverable = false;
    bool auto_accept_acl = false;

    uint8_t  pagescan_type = GAP_PAGE_SCAN_TYPE_INTERLACED;
    uint16_t pagescan_interval = DEFAULT_PAGESCAN_INTERVAL;
    uint16_t pagescan_window = DEFAULT_PAGESCAN_WINDOW;
    uint16_t page_timeout = DEFAULT_PAGE_TIMEOUT;
    uint8_t  page_scan_repetition_mode = GAP_PAGE_SCAN_REPETITION_R1;

    uint8_t inquiryscan_type = GAP_INQUIRY_SCAN_TYPE_INTERLACED;
    uint16_t inquiryscan_window = DEFAULT_INQUIRYSCAN_WINDOW;
    uint16_t inquiryscan_interval = DEFAULT_INQUIRYSCAN_INTERVAL;
    uint8_t inquiry_mode = GAP_INQUIRY_MODE_EXTENDED_RESULT;

    uint8_t pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_GENERAL_BONDING_FLAG |
                          GAP_AUTHEN_BIT_SC_FLAG | GAP_AUTHEN_BIT_FORCE_CENTRAL_ENCRYPT_FLAG;

#if F_APP_BREDR_SC_CTKD_SUPPORT
    auth_flags |= GAP_AUTHEN_BIT_SC_BR_FLAG;
#endif

#if (F_APP_AI_RECORD_PEN_DEMO_SUPPORT)
    uint8_t io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
#else
    uint8_t io_cap = GAP_IO_CAP_DISPLAY_YES_NO;
#endif

#if F_APP_BT_AUDIO_TRI_DONGLE
    if (app_cfg_nv.trigle_dongle_support_passkey == EVENT_SET_PASSKEY_SUPPORT_ENABLE)
    {
        io_cap = GAP_IO_CAP_DISPLAY_YES_NO;
    }
    else
    {
        io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    }
#endif

    uint8_t oob_enable = false;
    uint8_t bt_mode = GAP_BT_MODE_21ENABLED;

#if CONFIG_REALTEK_APP_BOND_MGR_SUPPORT
    bt_bond_init();
#if (F_APP_LE_AUDIO_ACCEPTOR_SUPPORT || F_APP_LE_AUDIO_INITIATOR_SUPPORT)
    ble_audio_bond_init();
#endif
#endif

    if ((app_cfg_const.bud_role == REMOTE_SESSION_ROLE_SINGLE) && (app_cfg_nv.factory_reset_done == 0))
    {
        /* change the scan interval to 100ms(0xA0 * 0.625) before factory reset */
        pagescan_interval = 0xA0;
        inquiryscan_interval = 0xA0;
    }

    gap_lib_init();

    //0: to be master
    gap_br_cfg_accept_role(1);

    gap_br_set_param(GAP_BR_PARAM_NAME, GAP_DEVICE_NAME_LEN, app_cfg_nv.device_name_legacy);


    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(uint8_t), &pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(uint16_t), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(uint8_t), &io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(uint8_t), &oob_enable);

    gap_br_set_param(GAP_BR_PARAM_BT_MODE, sizeof(uint8_t), &bt_mode);
    gap_br_set_param(GAP_BR_PARAM_COD, sizeof(uint32_t), &app_cfg_const.class_of_device);
    gap_br_set_param(GAP_BR_PARAM_LINK_POLICY, sizeof(uint16_t), &link_policy);
    gap_br_set_param(GAP_BR_PARAM_SUPV_TOUT, sizeof(uint16_t), &supervision_timeout);
    gap_br_set_param(GAP_BR_PARAM_AUTO_ACCEPT_ACL, sizeof(bool), &auto_accept_acl);

    gap_br_set_param(GAP_BR_PARAM_RADIO_MODE, sizeof(uint8_t), &radio_mode);
    gap_br_set_param(GAP_BR_PARAM_LIMIT_DISCOV, sizeof(bool), &limited_discoverable);

    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_TYPE, sizeof(uint8_t), &pagescan_type);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_INTERVAL, sizeof(uint16_t), &pagescan_interval);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_WINDOW, sizeof(uint16_t), &pagescan_window);
    gap_br_set_param(GAP_BR_PARAM_PAGE_TIMEOUT, sizeof(uint16_t), &page_timeout);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_REPETITION_MODE, sizeof(uint8_t),
                     &page_scan_repetition_mode);

    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_TYPE, sizeof(uint8_t), &inquiryscan_type);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_INTERVAL, sizeof(uint16_t), &inquiryscan_interval);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_WINDOW, sizeof(uint16_t), &inquiryscan_window);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_MODE, sizeof(uint8_t), &inquiry_mode);

    bt_pairing_tx_power_set(-2);

    app_ble_gap_param_init();
}

static void framework_init(void)
{
    /* System Manager */
    sys_mgr_init(audio_evt_queue_handle);

    /* RemoteController Manager */
    remote_mgr_init((T_REMOTE_SESSION_ROLE)app_cfg_nv.bud_role);
    remote_local_addr_set(app_cfg_nv.bud_local_addr);

    /* Bluetooth Manager */
    bt_mgr_init();

    /* Audio Manager */
    audio_mgr_init(PLAYBACK_POOL_SIZE, VOICE_POOL_SIZE, RECORD_POOL_SIZE, NOTIFICATION_POOL_SIZE);

    if ((app_cfg_const.dsp_jtag_enable) || (app_cfg_const.dsp2_jtag_enable) ||
        (app_cfg_const.adsp_jtag_enable))
    {
        audio_probe_disable_dsp_powerdown();
    }

#if F_APP_DISABLE_NOTIFICATION_SUPPORT
    ringtone_mode_set(RINGTONE_MODE_SILENT);
    voice_prompt_mode_set(VOICE_PROMPT_MODE_SILENT);
#endif

#if F_APP_MULTI_CHANNEL_SUPPORT
    audio_track_policy_set(AUDIO_TRACK_POLICY_MULTI_STREAM);
#endif
}
extern void app_spi_console_init(void);
extern bool app_cmd_parser_register(void);
extern void app_ant_init(void);
extern void app_sdio_console_init(void);

static void app_task(void *pvParameters)
{
    uint8_t event;
#if F_APP_WIFI_SPI_MAP_SUPPORT
    //open when wifi bin is support atcmd version2
//   app_ant_init();
#endif
    app_adp_detect();
    gap_start_bt_stack(audio_evt_queue_handle, audio_io_queue_handle, MAX_NUMBER_OF_GAP_MESSAGE);
#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
    app_gfps_msg_queue_init();
#endif

#if(CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
    mible_api_init(EVENT_MI_BLEAPI, audio_evt_queue_handle);
    mi_task_start(EVENT_MI_TASK, audio_evt_queue_handle);
    /* Add mijia auth code */
    mible_record_init();
    user_record_init();

    user_app_init();
#endif

#if F_APP_WIFI_SPI_MAP_SUPPORT
    app_cmd_parser_register();
#endif
#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
    app_findmy_crypto_init();
#endif
    os_mem_peek_printf();
    APP_PRINT_WARN1("app_task: cpu freq %d MHz", pm_cpu_freq_get());

    while (true)
    {
        if (os_msg_recv(audio_evt_queue_handle, &event, 0xFFFFFFFF) == true)
        {
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
                sys_mgr_event_handle(event);
            }
            else if (EVENT_GROUP(event) == EVENT_GROUP_APP)
            {
                app_timer_handle_msg(event);
            }
#if CONFIG_REALTEK_APP_GUI
            else if (EVENT_GROUP(event) == EVENT_GROUP_GUI)
            {
                app_panel_msg_handle(event);
            }
#endif
#if(CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
            else if (EVENT_GROUP(event) == EVENT_GROUP_MIJIA)
            {
                if (event == EVENT_MI_BLEAPI)
                {
                    mible_api_inner_msg_handle(event);
                }
                else if (event == EVENT_MI_TASK)
                {
                    mible_tasks_exec();
                }
            }
#endif
        }
#if(CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
        user_app_main_thread();
#endif
    }
}

#if F_APP_PSRAM_ADD_TO_SYSTEM_RAM_MANAGER
#include "psram_heap.h"
void prepare_psram_data_to_ram(void)
{
#ifdef __CC_ARM
    extern unsigned int Load$$PSRAM_DATA$$RW$$Base;
    extern unsigned int Image$$PSRAM_DATA$$RW$$Base;
    extern unsigned int Image$$PSRAM_DATA$$RW$$Length;
    extern unsigned int Load$$PSRAM_DATA$$ZI$$Base;
    extern unsigned int Image$$PSRAM_DATA$$ZI$$Base;
    extern unsigned int Image$$PSRAM_DATA$$ZI$$Length;

    uint32_t load_addr = (uint32_t)&Load$$PSRAM_DATA$$RW$$Base;
    uint32_t dest_addr = (uint32_t)&Image$$PSRAM_DATA$$RW$$Base;
    uint32_t len = (uint32_t)&Image$$PSRAM_DATA$$RW$$Length;
    memcpy((uint8_t *)dest_addr, (uint8_t *)load_addr, len);

    dest_addr = (uint32_t)&Image$$PSRAM_DATA$$ZI$$Base;
    len = (uint32_t)&Image$$PSRAM_DATA$$ZI$$Length;
    memset((uint8_t *)dest_addr, 0, len);
#elif defined(__GNUC__)
    extern uint32_t __psram_data_length__[];
    extern uint32_t __psram_data_load_addr__[];
    extern uint32_t __psram_data_dst_addr__[];

    memcpy((uint8_t *)__psram_data_dst_addr__, (uint8_t *)__psram_data_load_addr__,
           (uint32_t)__psram_data_length__);

    extern uint32_t __psram_bss_length__[];
    extern uint32_t __psram_bss_start_addr__[];
    memset((uint8_t *)__psram_bss_start_addr__, 0, (uint32_t)__psram_bss_length__);
#endif
}

void prepare_psram_text_to_ram(void)
{
#ifdef __CC_ARM
    extern unsigned int Image$$PSRAM_TEXT$$Base;
    extern unsigned int Load$$PSRAM_TEXT$$Base;
    extern unsigned int Image$$PSRAM_TEXT$$Length;

    void *image_base  = (void *)&Image$$PSRAM_TEXT$$Base;
    void *load_base   = (void *)&Load$$PSRAM_TEXT$$Base;
    unsigned int size = (unsigned int)&Image$$PSRAM_TEXT$$Length;

    memcpy(image_base, load_base, size);
#elif defined(__GNUC__)
    extern uint32_t __psram_text_length__[];
    extern uint32_t __psram_text_load_addr__[];
    extern uint32_t __psram_text_dst_addr__[];

    memcpy((uint8_t *)__psram_text_dst_addr__, (uint8_t *)__psram_text_load_addr__,
           (uint32_t) __psram_text_length__);
#endif
}

#endif

int main(void)
{
    /* Dlps init and disable should invoke before any other initialization,
       or cpu may enter low power state unexpectedly */
    app_dlps_init();
    app_dlps_disable(APP_DLPS_ENTER_CHECK_INIT);

    app_system_lower_init();  //init flash, psram, cpu and spic clock

    uint32_t time_entry_app;
    void *app_task_handle;
    uint8_t wake_up_reason;
    uint16_t stack_size = 1024 * 3 - 256;

    log_module_trace_set(MODULE_HCI, LEVEL_TRACE, false);
    log_module_trace_set(MODULE_BTIF, LEVEL_TRACE, false);
    log_module_trace_set(MODULE_GAP, LEVEL_TRACE, false);

    time_entry_app = sys_timestamp_get();
    wake_up_reason = power_down_check_wake_up_reason();
    APP_PRINT_INFO2("TIME FROM PATCH TO APP: %d ms, wake_up_reason: 0x%x", time_entry_app,
                    wake_up_reason);
    APP_PRINT_INFO2("APP COMPILE TIME: [%s - %s]", TRACE_STRING(__DATE__), TRACE_STRING(__TIME__));

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
    stack_size = 1024 * 6;
#endif

    {
        uint8_t freq_handle = 0;
        uint32_t actual_mhz = 0;

#if F_APP_MULTI_CHANNEL_SUPPORT
        pm_cpu_freq_set(100, &actual_mhz);
#else
        pm_cpu_freq_set(40, &actual_mhz);
#endif

    }

#if F_APP_PSRAM_ADD_TO_SYSTEM_RAM_MANAGER
    prepare_psram_data_to_ram();
    prepare_psram_text_to_ram();
    psram_heap_init((uint8_t *)PSRAM_HEAP_ADDR, PSRAM_HEAP_SIZE);
#endif
    if (sys_hall_get_reset_status())
    {
        APP_PRINT_INFO0("APP RESTART FROM WDT_SOFTWARE_RESET");
    }
    else
    {
        //APP power off reboot also in this case
        APP_PRINT_INFO0("APP START FROM HW_RESET");
    }

    os_msg_queue_create(&audio_io_queue_handle, "ioQ", MAX_NUMBER_OF_IO_MESSAGE, sizeof(T_IO_MSG));
    os_msg_queue_create(&audio_evt_queue_handle, "evtQ", MAX_NUMBER_OF_RX_EVENT, sizeof(unsigned char));
#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
    os_msg_queue_create(&app_findmy_queue_handle, "findmy_msg", MAX_NUMBER_OF_IO_MESSAGE,
                        sizeof(T_FINDMY_BLE_INDICATION));
#endif
#if CONFIG_REALTEK_APP_GUI
    app_panel_msg_channel_register(audio_evt_queue_handle, audio_io_queue_handle,
                                   MAX_NUMBER_OF_GUI_MODULE);
#endif
    app_init_timer(audio_evt_queue_handle, MAX_NUMBER_OF_APP_TIMER_MODULE);
    pm_cpu_freq_init();

    app_ipc_init();

    app_mcu_cfg_init();

    //app_key_cfg_init();

    app_dsp_cfg_init(app_audio_cfg.normal_apt_support);
    app_mcu_cfg_nv_load();

    app_transfer_cfg_init();
    board_init();
    app_bt_gap_init();
    framework_init();

    //Callback provider for other modules MUST init fisrt
    app_ota_init();

    //driver init MUST after callback provider init
    driver_init();

    app_key_gpio_button_init();

#if F_APP_CORE_DUMP_SUPPORT
    app_core_dump_init(CORE_DUMP_ADDR_OFFSET, CORE_DUMP_SIZE);
#endif

#if F_APP_LOG2FLASH_SUPPORT
    uint32_t time_1 = sys_timestamp_get();
    app_log2flash_init(LOG2FLASH_ADDR_OFFSET, LOG2FLASH_SIZE);
    uint32_t time_2 = sys_timestamp_get();
    APP_PRINT_INFO2("init log2flash %d ~ %d", time_1, time_2);
#endif

    //Other app module MUST init after here
    app_auto_power_off_init();
    app_audio_cfg_init();

    app_mmi_init();

    if (is_single_tone_test_mode()) //DUT test mode
    {
        reset_single_tone_test_mode();
        mp_hci_test_init(MP_HCI_TEST_DUT_MODE);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
    }
    else //Normal mode
    {

#if F_APP_TEST_SUPPORT
        app_test_init();
#endif

        app_gap_init();
        app_ble_cfg_init();
        app_ble_gap_init();
        app_bt_policy_cfg_init();
        app_bt_policy_init();
        app_ble_client_init();

        gatt_svc_init(GATT_SVC_USE_EXT_SERVER, 0);
        T_GATT_SVC_PENDING_NUM num;
        num.notify_num = 20;
        num.ind_num = 10;
        gatt_svc_cfg_pending_num(num);

        app_hfp_cfg_init();

#if F_APP_HFP_AG_SUPPORT
        app_hfp_ag_init();
#endif

#if F_APP_HFP_HF_SUPPORT
        app_hfp_hf_init();
#endif

        app_avrcp_cfg_init();
        app_avrcp_init();
        app_a2dp_cfg_init();
        app_a2dp_init();
        app_sdp_init();
        app_spp_cfg_init();
        app_spp_init();

        app_audio_init();

#if F_APP_BT_PROFILE_PBAP_PCE_SUPPORT
        app_pbap_cfg_init();
        app_pbap_init();
#endif

#if F_APP_BT_HID_DEVICE_SUPPORT
        app_hid_cfg_init();
        app_hid_init();
#endif

#if F_APP_BT_HID_HOST_SUPPORT
        app_hid_cfg_init();
        app_hid_host_init();
#endif

#if F_APP_IAP_SUPPORT
        app_iap_cfg_init();
        app_iap_init();
#endif

#if F_APP_BT_PROFILE_MAP_MCE_SUPPORT
        app_map_cfg_init();
        app_map_init();
#endif

#if F_APP_PAN_SUPPORT
        app_pan_init();
#endif

#if F_APP_MULTILINK_ENABLE
        app_multilink_init();
#endif

        app_ble_service_init();

        app_device_init();
        app_ble_device_init();
        app_transfer_init();

#if F_APP_LINEIN_SUPPORT
        app_line_in_init();
#endif

        app_cmd_init();
        app_bond_init();

#if F_APP_DATA_CAPTURE_SUPPORT
        app_data_capture_init();
#endif

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
        app_findmy_ble_bond_sync_init();
#endif

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
        app_gfps_module_init();
#endif
        app_customer_init();

#if F_APP_IAP_RTK_SUPPORT
        spp_stream_init(0xff);
        ble_stream_init(0xff);
        iap_stream_init(0xff);
        app_iap_rtk_init();
#endif

#if TRANSMIT_CLIENT_SUPPORT
        app_le_transfer_init();
#endif

#if F_APP_CHARGING_CASE_CMD_SUPPORT
        app_charging_case_init();
#endif

#if F_APP_QDECODE_SUPPORT
        if (app_cfg_const.wheel_support)
        {
            app_qdec_init();
        }
#endif

#if F_APP_AUTO_POWER_TEST_LOG
        app_power_test_init();
#endif

#if F_APP_BT_AUDIO_TRI_DONGLE
        uint32_t spic0_freq = 0;
        bool ret = fmc_flash_nor_clock_switch(FMC_SPIC_ID_0, 160, &spic0_freq);
        APP_PRINT_INFO1("fmc_flash_nor_clock_switch: set flash clock result %d", ret);
        app_tri_dongle_mgr_init();

#if F_APP_NXP_UWB_DRIVER_SUPPORT
        extern void uwb_task_init(void);
        uwb_task_init();
#endif
#if F_APP_UWB_SCENARIO_SUPPORT
        app_tri_dongle_uwb_init();
#endif
#if CONFIG_YYLX_DONGLE_FEATURE
        app_tri_dongle_yylx_multi_uhid_init();
#endif
#endif

        //increase app task priority to 2 so that user could implement some background daemon task
#if F_APP_BT_AUDIO_TRI_DONGLE
        os_task_create(&app_task_handle, "app_task", app_task, NULL, stack_size, 3);
#else
        os_task_create(&app_task_handle, "app_task", app_task, NULL, stack_size, 2);
#endif

#if F_APP_HIFI4_SUPPORT
        void *ipc_task_handle = NULL;
        os_task_create(&ipc_task_handle, "ipc_task", ipc_task, NULL, 1024 * 2, 2);
#endif

#if F_APP_DEBUG_HIT_RATE_PRINT
        cache_hit_count_init(10000);
#endif
    }

#if F_APP_DEBUG_TASK_PROFILING
    hal_debug_init();
    hal_debug_task_time_proportion_init(5000);
#endif

#if (F_APP_A2DP_XMIT_SRC_SUPPORT || F_APP_A2DP_XMIT_SRC_LEA_SUPPORT)
    app_a2dp_xmit_mgr_init();
#endif

#if (F_APP_SCO_XMIT_AG_SUPPORT || F_APP_SCO_XMIT_HF_SUPPORT)
    app_sco_xmit_init();
#endif

#if BAP_BROADCAST_SOURCE
    app_lea_profile_init();
#endif

#if (F_APP_TMAP_CT_SUPPORT || F_APP_TMAP_UMR_SUPPORT || F_APP_TMAP_BMR_SUPPORT)
    app_lea_acc_profile_init();
#endif

#if F_APP_SD_CARD_SUPPORT
    app_sd_card_init();
#endif

    app_fs_init();

#if F_SOURCE_PLAY_SUPPORT
    app_src_play_init();
#endif

#if CONFIG_REALTEK_APP_GUI
    app_gui_init();
#endif

#if F_APP_USB_AUDIO_SUPPORT | F_APP_USB_MSC_SUPPORT | F_APP_USB_HID_SUPPORT | F_APP_USB_CDC_SUPPORT
    app_usb_init();
#endif
#if F_APP_USB_HOST_SUPPORT
    extern void app_usbh_audio_init(void);
    app_usbh_audio_init();
#endif
#if F_APP_CFU_FEATURE_SUPPORT
    app_cfu_init();
#endif

#if F_APP_MONITOR_MEMORY_AND_TIMER
    monitor_memory_and_timer(app_cfg_const.timer_monitor_heap_and_timer_timeout);
#endif

#if F_APP_MALLEUS_SUPPORT
    app_malleus_init(MALLEUS_FULL_CYCLE, MALLEUS_FULL_CYCLE);
#endif

#if F_APP_WIFI_SPI_CMD
    app_spi_atcmd_init();
#endif

#if F_APP_WIFI_UART_CMD
    app_wifi_uart_init();
    app_uart_atcmd_init();
#endif

#if CONFIG_REALTEK_APP_AI_RECORD
    app_ai_record_init();
#endif

#if CONFIG_REALTEK_APP_AI_AUTH
    app_rtk_auth_init();
#endif

#if F_APP_SPI_ROLE_MASTER
    app_spi_master_init();
#endif

#if F_APP_SPI_ROLE_SLAVE
    app_spi_slave_init();
#endif
    /* log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_TRACE, true);
     log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_INFO, true);
     log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_WARN, true);
     log_module_bitmap_trace_set(0xFFFFFFFFFFFFFFFF, LEVEL_ERROR, true);
    */
    os_mem_peek_printf();
    APP_PRINT_INFO1("TIME FROM APP TO OS SCHED: %d ms", sys_timestamp_get() - time_entry_app);
    app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);

    return 0;
}
