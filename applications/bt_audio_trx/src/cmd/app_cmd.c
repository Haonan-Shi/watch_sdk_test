/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "console.h"
#include "gap_scan.h"
#include "app_timer.h"
#include "stdlib.h"
#include "ancs_gatt_client.h"
#include "gap.h"
#include "gap_br.h"
#include "audio.h"

#include "bt_hfp.h"
#include "bt_iap.h"
#include "btm.h"
#include "bt_avrcp.h"
#include "bt_map.h"
#include "bt_bond.h"
#include "bt_types.h"
#include "remote.h"
#include "stdlib.h"
#include "patch_header_check.h"
#include "test_mode.h"
#include "rtl876x_pinmux.h"
#include "app_cmd.h"
#include "app_main.h"
#include "app_audio_policy.h"
#include "app_pan.h"
#include "app_transfer.h"
#include "app_report.h"
#include "app_fs_test.h"

#include "app_bt_policy_api.h"
#include "app_mmi.h"
#include "app_cfg.h"
#include "app_ota.h"
#include "app_eq.h"

#include "app_hfp.h"
#include "app_linkback.h"

#include "app_device.h"
#include "wdg.h"

#include "app_hfp_ag.h"
#include "app_ble_common_adv.h"

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "app_gfps_cfg.h"
#endif

#if F_APP_PBAP_CMD_SUPPORT
#include "app_pbap.h"
#endif

#include "app_audio_policy.h"

#include "app_bt_policy_api.h"

#include "app_customer_cmd.h"
#include "app_bt_policy_api.h"


#if BAP_BROADCAST_SOURCE
#include "app_lea_ini_cmd.h"
#endif

#if F_SOURCE_PLAY_SUPPORT
#include "app_src_play_cmd.h"
#endif

#if F_APP_SD_CARD_LOCALPLAY
#include "app_src_playback.h"
#include "bt_types.h"
#endif

#include "app_map.h"


#if (F_APP_SPI_ROLE_MASTER || F_APP_SPI_ROLE_SLAVE)
#include "app_spi_cmd.h"
#endif

#if defined(CONFIG_WIFI_8711_CMD)
#include "app_spi_atcmd.h"
#include "spi_tcp_tp_test.h"
#endif

#if (F_APP_A2DP_XMIT_SRC_SUPPORT || F_APP_A2DP_XMIT_SNK_SUPPORT || F_APP_A2DP_XMIT_SRC_LEA_SUPPORT)
#include "app_a2dp_xmit_mgr.h"
#endif

#if (F_APP_SCO_XMIT_HF_SUPPORT || F_APP_SCO_XMIT_AG_SUPPORT)
#include "app_sco_xmit_mgr.h"
#endif

#if F_APP_BT_HID_HOST_SUPPORT
#include "app_hid_host.h"
#endif

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include "app_findmy.h"
#endif

#if F_APP_DATA_CAPTURE_SUPPORT
#include "app_data_capture_cs.h"
#endif

#if F_APP_SAIYAN_EQ_FITTING
#include "app_eq_fitting.h"
#endif

#if F_APP_UART_DFU
#include "app_uart_dfu.h"
#endif

#include "app_cmd_ble.h"
#include "app_cmd_br.h"
#include "app_cmd_customized.h"
#include "app_cmd_device.h"
#include "app_cmd_general.h"
#include "app_cmd_other.h"
#include "app_record.h"

#if (F_APP_TMAP_CT_SUPPORT || F_APP_TMAP_UMR_SUPPORT || F_APP_TMAP_BMR_SUPPORT)
#include "app_lea_acc_cmd.h"
#endif

#if TRANSMIT_CLIENT_SUPPORT
#include "app_le_transfer.h"
#endif

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
#include "app_gfps_finder.h"
#include "app_gfps_cfg.h"
#endif
#if F_APP_BT_AUDIO_TRI_DONGLE
#include "app_tri_dongle_cmd.h"
#endif

#if F_APP_BLE_HID_DEVICE_SUPPORT
#include "app_ble_hid_device.h"
#endif

#if F_APP_CHARGING_CASE_CMD_SUPPORT
#include "app_charging_case_cmd.h"
#endif

#if F_APP_MALLEUS_SUPPORT
#include "app_malleus.h"
#endif


#if F_APP_FLASH_DUMP_SUPPORT
#include "app_flash_dump.h"
#endif

#if (CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
#include "mijia_sample.h"
#endif

#if F_APP_WIFI_SPI_MAP_SUPPORT
#include "app_spi_data_parser.h"
#include "gui_img.h"
extern gui_img_t *map_display;
uint8_t *start_gui_show_buf0 = (uint8_t *)PSRAM_RX_BUF1_ADDR;
uint8_t *start_gui_show_buf1 = (uint8_t *)PSRAM_RX_BUF1_ADDR + (800 * 480 * 2);
uint8_t *start_gui_show_buf2 = (uint8_t *)PSRAM_RX_BUF1_ADDR + (800 * 480 * 2 * 3);
T_CURRENT_IMAGE current_image = {0};
T_SCREEN_DATA screen_data;
#endif

#if CONFIG_REALTEK_APP_AI_AUTH
#include "app_rtk_auth.h"
#endif

#if CONFIG_REALTEK_APP_AI_RECORD
#include "ai_record.h"
#endif

#include "app_reset_reason.h"

T_CMD_MGR app_cmd_mgr =
{
    .uart_rx_seqn = 0,
    .tmr =
    {
        .id = 0xff,
        .indices = {},
    },
    .dsp_spp_capture = {},
    .dlps =
    {
        .status = SET_DLPS_ENABLE,
    },

    .src_ver = {},
    .cmd_parse = {},
    .conn_req_flags =
    {
        .auto_reject_en = false,
        .auto_accept_en = true,
    },
    .sdp = {.rpt_attr_en = false},
    .flash = {},
};

typedef enum
{
    APP_TIMER_SWITCH_TO_HCI_DOWNLOAD_MODE,
    APP_TIMER_ENTER_DUT_FROM_SPP_WAIT_ACK,
    APP_TIMER_OTA_JIG_DELAY_POWER_OFF,
    APP_TIMER_OTA_JIG_DELAY_WDG_RESET,
    APP_TIMER_OTA_JIG_DLPS_ENABLE,
    APP_TIMER_DSP_SPP_CAPTRUE_CHECK_LINK,
    APP_TIMER_IO_PIN_PULL_HIGH,
    APP_TIMER_STOP_PERIODIC_INQUIRY,
} T_CMD_TIMER;

#define CMD_MAX_LENGTH                     1024

void app_start_timer_enter_dut_from_spp(uint8_t app_idx)
{
    app_start_timer(&app_cmd_mgr.tmr.indices.enter_dut_from_spp_wait_ack, "enter_dut_from_spp_wait_ack",
                    app_cmd_mgr.tmr.id, APP_TIMER_ENTER_DUT_FROM_SPP_WAIT_ACK, app_idx, false,
                    100);
}

void app_start_timer_switch_to_hci_mode(uint8_t app_idx)
{
    app_start_timer(&app_cmd_mgr.tmr.indices.switch_to_hci_mode, "switch_to_hci_mode",
                    app_cmd_mgr.tmr.id, APP_TIMER_SWITCH_TO_HCI_DOWNLOAD_MODE, app_idx, false,
                    100);
}

static uint16_t handle_all_cmds(T_CMD_PATH cmd_path, uint8_t *data, uint16_t len, uint8_t app_idx)
{
    uint16_t remains = len;
    while (remains > 0)
    {
        if (data[0] == CMD_SYNC_BYTE)
        {
            CMD_HDR *hdr = (CMD_HDR *)data;
            uint16_t tatol_cmd_len = hdr->cmd_len + offsetof(CMD_HDR, cmd);
            uint16_t cmd_id = hdr->cmd[0] | (hdr->cmd[1] << 8);
//            APP_PRINT_TRACE5("app_cmd handle_all_cmds: cmd_id 0x%04x, cmd_len %d, seq %d, remains %d, tatol_cmd_len %d",
//                                cmd_id, hdr->cmd_len, hdr->seq, remains, tatol_cmd_len);
            if (remains >= tatol_cmd_len)
            {
                app_handle_cmd_set(hdr->cmd, hdr->cmd_len, cmd_path, hdr->seq, app_idx);
                remains -= tatol_cmd_len;
                data += tatol_cmd_len;
            }
            else
            {
                break;
            }
        }
        else
        {
            remains--;
            data++;
        }
    }

//    APP_PRINT_TRACE1("app_cmd handle_all_cmds: remains %d", remains);

    return remains;
}

static uint8_t *handle_remains(uint8_t *p_data, uint16_t remains, uint16_t data_len)
{
    if (remains)
    {
        uint8_t *new_buf = calloc(1, remains);
        if (new_buf)
        {
            memcpy(new_buf, &p_data[data_len - remains], remains);
            return new_buf;
        }
    }

    return NULL;
}

static void concate_data_to_buf(uint8_t **p_buf, uint16_t *p_buf_len, uint8_t *p_data,
                                uint16_t data_len)
{
    if (data_len)
    {
        uint16_t new_buf_len = *p_buf_len + data_len, old_buf_len = *p_buf_len;
        uint8_t *new_buf = calloc(1, new_buf_len), *old_buf = *p_buf;
        if (new_buf)
        {
            memcpy(new_buf, old_buf, old_buf_len);
            memcpy(&new_buf[old_buf_len], p_data, data_len);

            *p_buf = new_buf;
            *p_buf_len = new_buf_len;

            free(old_buf);
        }
        else
        {
            *p_buf = old_buf;
            *p_buf_len = old_buf_len;
        }
    }
}

//p_buf is a buf contain remain data from last recv.
void app_cmd_proc_data(T_CMD_PATH cmd_path, uint8_t **p_buf, uint16_t *p_buf_len, uint8_t *p_data,
                       uint16_t data_len,
                       uint8_t app_link_id)
{
//    APP_PRINT_TRACE6("app_cmd_proc_data: p_buf %p, *p_buf %p, p_buf_len %p, *p_buf_len %d, data_len %d, data %b",
//                        p_buf, *p_buf, p_buf_len, *p_buf_len, data_len, TRACE_BINARY(data_len, p_data));

    if (*p_buf == NULL)
    {
        uint16_t remains = handle_all_cmds(cmd_path, p_data, data_len, app_link_id);
        *p_buf = handle_remains(p_data, remains, data_len); //NULL means no remains data
        *p_buf_len = remains;
    }
    else //"*p_buf != NULL" means need concate data with *p_buf
    {
        concate_data_to_buf(p_buf, p_buf_len, p_data, data_len);

        //ios will auto combine two cmd into one pkt
        uint16_t remains = handle_all_cmds(cmd_path, *p_buf, *p_buf_len, app_link_id);
        uint8_t *new_buf = handle_remains(*p_buf, remains, *p_buf_len);
        free(*p_buf); //free old p_buf
        *p_buf = new_buf;
        *p_buf_len = remains;

    }
}

static void app_cmd_bt_event_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_ACL_CONN_IND:
        {
            //Stop periodic inquiry when connecting
#if F_APP_DEVICE_CMD_SUPPORT
            app_stop_timer(&app_cmd_mgr.tmr.indices.stop_periodic_inquiry);
#endif
            bt_periodic_inquiry_stop();
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_cmd_bt_cback: event_type 0x%04x", event_type);
    }
}

static void app_cmd_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    switch (timer_evt)
    {
    case APP_TIMER_SWITCH_TO_HCI_DOWNLOAD_MODE:
        {
            app_stop_timer(&app_cmd_mgr.tmr.indices.switch_to_hci_mode);
            set_hci_mode_flag(true);
            wdg_reset_with_reason(RESET_ALL_EXCEPT_AON, APP_RESET_REASON_SWITCH_TO_HCI);
        }
        break;

    case APP_TIMER_ENTER_DUT_FROM_SPP_WAIT_ACK:
        {
            app_stop_timer(&app_cmd_mgr.tmr.indices.enter_dut_from_spp_wait_ack);
            app_mmi_handle_action(MMI_ENTER_DUT_FROM_SPP);
        }
        break;

    case APP_TIMER_IO_PIN_PULL_HIGH:
        {
            app_stop_timer(&app_cmd_mgr.tmr.indices.io_pin_pull_high);

            uint8_t pin_num = (uint8_t)param;

            Pad_Config(pin_num, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_NONE, PAD_OUT_ENABLE, PAD_OUT_HIGH);
        }
        break;

#if F_APP_DEVICE_CMD_SUPPORT
    case APP_TIMER_STOP_PERIODIC_INQUIRY:
        {
            app_stop_timer(&app_cmd_mgr.tmr.indices.stop_periodic_inquiry);
            bt_periodic_inquiry_stop();
        }
        break;
#endif

    default:
        break;
    }
}

void app_cmd_init(void)
{
    bt_mgr_cback_register(app_cmd_bt_event_cback);
    app_timer_reg_cb(app_cmd_timeout_cb, &app_cmd_mgr.tmr.id);
    app_cmd_ble_init();
}

bool app_cmd_cback_register(P_CMD_PARSE_CBACK parse_cb,
                            T_CMD_MODULE_TYPE module_type, uint8_t msg_max)
{
    T_CMD_PARSE_CBACK_ITEM *p_item;

    p_item = (T_CMD_PARSE_CBACK_ITEM *)app_cmd_mgr.cmd_parse.cb_list.p_first;

    while (p_item != NULL)
    {
        if (p_item->parse_cback == parse_cb)
        {
            return true;
        }

        p_item = p_item->p_next;
    }

    p_item = (T_CMD_PARSE_CBACK_ITEM *)malloc(sizeof(T_CMD_PARSE_CBACK_ITEM));

    if (p_item != NULL)
    {
        p_item->parse_cback = parse_cb;
        p_item->module_type = module_type;
        p_item->msg_type_max = msg_max;
        os_queue_in(&app_cmd_mgr.cmd_parse.cb_list, p_item);

        return true;
    }

    return false;
}

void app_cmd_set_event_broadcast(uint16_t event_id, uint8_t *buf, uint16_t len)
{
    T_APP_BR_LINK *br_link;
    T_APP_LE_LINK *le_link;
    uint8_t        i;

    for (i = 0; i < MAX_BR_LINK_NUM; i ++)
    {
        br_link = &app_db.br_link[i];

        if (br_link->cmd.enable == true)
        {
            if (br_link->connected_profile & SPP_PROFILE_MASK)
            {
                app_report_event(CMD_PATH_SPP, event_id, i, buf, len);
            }

            if (br_link->connected_profile & IAP_PROFILE_MASK)
            {
                app_report_event(CMD_PATH_IAP, event_id, i, buf, len);
            }

            if (br_link->connected_profile & GATT_PROFILE_MASK)
            {
                app_report_event(CMD_PATH_GATT_OVER_BREDR, event_id, i, buf, len);
            }
        }
    }

    for (i = 0; i < MAX_BLE_LINK_NUM; i++)
    {
        le_link = &app_db.le_link[i];

        if (le_link->state == LE_LINK_STATE_CONNECTED)
        {
            if (le_link->cmd.enable == true)
            {
                app_report_event(CMD_PATH_LE, event_id, i, buf, len);
            }
        }
    }
}

void app_cmd_set_event_ack(uint8_t cmd_path, uint8_t app_idx, uint8_t *buf)
{
    app_report_event(cmd_path, EVENT_ACK, app_idx, buf, 3);
}

bool app_cmd_get_tool_connect_status(void)
{
    bool tool_connect_status = false;
    T_APP_BR_LINK *br_link;
    T_APP_LE_LINK *le_link;
    uint8_t        i;

    for (i = 0; i < MAX_BR_LINK_NUM; i ++)
    {
        br_link = &app_db.br_link[i];

        if (br_link->connected_profile & (SPP_PROFILE_MASK | IAP_PROFILE_MASK))
        {
            if (br_link->cmd.enable == true)
            {
                tool_connect_status = true;
            }
        }
    }

    for (i = 0; i < MAX_BLE_LINK_NUM; i++)
    {
        le_link = &app_db.le_link[i];
        if (le_link->state == LE_LINK_STATE_CONNECTED)
        {
            if (le_link->cmd.enable == true)
            {
                tool_connect_status = true;
            }
        }
    }

    return tool_connect_status;
}

void app_cmd_update_eq_ctrl(uint8_t value, bool is_need_relay)
{
    if (app_db.eq_ctrl_by_src != value)
    {
        app_db.eq_ctrl_by_src = value;

        if (is_need_relay)
        {

        }
    }

    APP_PRINT_TRACE1("app_cmd_update_eq_ctrl: connect_status %d", value);
}

T_SRC_SUPPORT_VER_FORMAT *app_cmd_get_src_version(uint8_t cmd_path, uint8_t app_idx)
{
    T_SRC_SUPPORT_VER_FORMAT *version = NULL;

    if (cmd_path == CMD_PATH_UART)
    {
        version = &app_cmd_mgr.src_ver.uart;
    }
    else if (cmd_path == CMD_PATH_LE)
    {
        version = &app_cmd_mgr.src_ver.le_link[app_idx];
    }
    else if ((cmd_path == CMD_PATH_SPP) || (cmd_path == CMD_PATH_IAP) ||
             (cmd_path == CMD_PATH_GATT_OVER_BREDR))
    {
        version = &app_cmd_mgr.src_ver.br_link[app_idx];
    }

    return version;
}

bool app_cmd_check_specific_version(T_SRC_SUPPORT_VER_FORMAT *version, uint8_t ver_major,
                                    uint8_t ver_minor)
{
    if (version->cmd_set_ver_major > ver_major ||
        (version->cmd_set_ver_major == ver_minor && version->cmd_set_ver_minor >= ver_minor))
    {
        // SRC support version is compatible.
        return true;
    }

    return false;
}

bool app_cmd_check_src_cmd_version(uint8_t cmd_path, uint8_t app_idx)
{
    T_SRC_SUPPORT_VER_FORMAT *version = app_cmd_get_src_version(cmd_path, app_idx);

    if (version)
    {
        if (app_cmd_check_specific_version(version, CMD_SET_VER_MAJOR, CMD_SET_VER_MINOR))
        {
            // SRC support version is new, which is valid.
            return true;
        }
        else if (version->cmd_set_ver_major == 0 && version->cmd_set_ver_minor == 0)
        {
            // SRC never update support version
            return true;
        }
    }

    return false;
}

bool app_cmd_check_src_eq_spec_version(uint8_t cmd_path, uint8_t app_idx)
{
    T_SRC_SUPPORT_VER_FORMAT *version = app_cmd_get_src_version(cmd_path, app_idx);

    if (version)
    {
        uint8_t eq_spec_minor = EQ_SPEC_VER_MINOR;

        if (version->eq_spec_ver_major > EQ_SPEC_VER_MAJOR ||
            (version->eq_spec_ver_major == EQ_SPEC_VER_MAJOR && version->eq_spec_ver_minor >= eq_spec_minor))
        {
            // SRC support version is new, which is valid.
            return true;
        }
        else if (version->eq_spec_ver_major == 0 && version->eq_spec_ver_minor == 0)
        {
            // SRC never update support version
            return true;
        }
    }

    return false;
}

void app_handle_cmd_set(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t rx_seqn,
                        uint8_t app_idx)
{
    uint16_t cmd_id;
    uint8_t  ack_pkt[3];
    cmd_id     = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));
    ack_pkt[0] = cmd_ptr[0];
    ack_pkt[1] = cmd_ptr[1];
    ack_pkt[2] = CMD_SET_STATUS_COMPLETE;

    APP_PRINT_TRACE4("app_handle_cmd_set++: cmd_id 0x%04x, cmd_len 0x%04x, cmd_path %u, rx_seqn 0x%02x",
                     cmd_id, cmd_len, cmd_path, rx_seqn);
    if (cmd_len > CMD_MAX_LENGTH)
    {
        APP_PRINT_ERROR1("app_handle_cmd_set: cmd_len error len %d", cmd_len);
        return;
    }
    /* check duplicated seq num */
    if ((cmd_id != CMD_ACK) && (rx_seqn != 0))
    {
        uint8_t *check_rx_seqn = NULL;
        uint16_t *check_cmd_id = NULL;
        uint8_t  report_app_idx = app_idx;
        bool drop_data = false;

        if (cmd_path == CMD_PATH_UART)
        {
            check_rx_seqn = &app_cmd_mgr.uart_rx_seqn;
            check_cmd_id = &app_cmd_mgr.uart_last_cmd_id;
            report_app_idx = 0;
        }
        else if (cmd_path == CMD_PATH_LE)
        {
            check_rx_seqn = &app_db.le_link[app_idx].cmd.rx_seqn;
        }
        else if ((cmd_path == CMD_PATH_SPP) || (cmd_path == CMD_PATH_IAP) ||
                 (cmd_path == CMD_PATH_GATT_OVER_BREDR))
        {
            check_rx_seqn = &app_db.br_link[app_idx].cmd.rx_seqn;
        }
        else if (cmd_path == CMD_PATH_SPI)
        {
            check_rx_seqn = &app_cmd_mgr.spi_rx_seq;
        }

        if (check_rx_seqn)
        {
            if (*check_rx_seqn == rx_seqn)
            {
                if (cmd_path != CMD_PATH_SPI)
                {
                    if (cmd_path == CMD_PATH_SPP && cmd_id == CMD_BLUETALKIE_RECORD_DATA)
                    {
                        APP_PRINT_WARN0("app_handle_cmd_set: receive same data");
                    }
                }
                if (check_cmd_id)
                {
                    if (*check_cmd_id == cmd_id)
                    {
                        drop_data = true;
                    }
                }
                else
                {
                    drop_data = true;
                }
                if (drop_data)
                {
                    APP_PRINT_WARN0("app_handle_cmd_set: seq_duplicated");
                    app_report_event(cmd_path, EVENT_ACK, report_app_idx, ack_pkt, 3);
                    return;
                }
            }
            *check_rx_seqn = rx_seqn;
        }
        if (check_cmd_id)
        {
            *check_cmd_id = cmd_id;
        }
    }

    if ((cmd_path == CMD_PATH_SPP) || (cmd_path == CMD_PATH_IAP) ||
        (cmd_path == CMD_PATH_GATT_OVER_BREDR))
    {
        app_db.br_link[app_idx].cmd.enable = true;
        bt_sniff_mode_exit(app_db.br_link[app_idx].bd_addr, true);
    }
    else if (cmd_path == CMD_PATH_LE)
    {
        if (app_db.le_link[app_idx].state == LE_LINK_STATE_CONNECTED)
        {
            app_db.le_link[app_idx].cmd.enable = true;
        }
    }

    switch (cmd_id)
    {
#if F_APP_WIFI_SPI_MAP_SUPPORT
        static  uint8_t  buf_used = 0;
        static uint8_t start = 0;
        static uint32_t     data_len = 0;
        static uint32_t write_data_len = 0;
    case CMD_SCREEN_CAST_CMD:

        APP_PRINT_INFO2("CMD_SCREEN_CAST_CMD len from console cmd_ptr= 0x%x  cmd_len = 0x%x", cmd_ptr,
                        cmd_len);
        cmd_ptr += 2;
        LE_STREAM_TO_UINT8(start, cmd_ptr);

        if (start == 0)
        {
            APP_PRINT_INFO0("CMD_SCREEN_CAST_CMD len map transfer start");
        }
        else
        {
            APP_PRINT_INFO0("CMD_SCREEN_CAST_CMD len map transfer stop");
            map_display->base.not_show = true;
        }
        break;

    case CMD_SCREEN_CAST_DATA:

        if (screen_data.packet_flag == 0x01)
        {
            APP_PRINT_INFO3("CMD_SCREEN_CAST_DATA screen_cast_index = 0x%x  , packet_flag = 0x%x  buf_to_be_use = 0x%x",
                            screen_data.screen_cast_index, screen_data.packet_flag, buf_used);

            write_data_len = 0;
            memset(&current_image, 0, sizeof(T_CURRENT_IMAGE));
            current_image.is_image_ok = true;
            current_image.screen_cast_start = true;
            current_image.screen_cast_index = screen_data.screen_cast_index;
            if (buf_used == 0)
            {
                current_image.packet_begin_buf = start_gui_show_buf0;
                current_image.packet_op_buf = start_gui_show_buf0;
            }
            else if (buf_used == 1)
            {
                current_image.packet_begin_buf = start_gui_show_buf1;
                current_image.packet_op_buf = start_gui_show_buf1;
            }
            else if (buf_used == 2)
            {
                current_image.packet_begin_buf = start_gui_show_buf2;
                current_image.packet_op_buf = start_gui_show_buf2;
            }
        }
        else
        {
            if (current_image.screen_cast_start != true)
            {
                APP_PRINT_WARN0("app_handle_cmd_set screen_cast_start false");
                return;
            }
        }
        if (screen_data.screen_cast_index != current_image.screen_cast_index)
        {
            APP_PRINT_WARN2("app_handle_cmd_set  image index error screen_cast_index =0x%x  packet_index = %d",
                            screen_data.screen_cast_index, screen_data.packet_index);
            break;
        }
        if (current_image.is_image_ok == false)
        {
            APP_PRINT_WARN2("app_handle_cmd_set  image broken screen_cast_index =0x%x  packet_index = %d",
                            screen_data.screen_cast_index, screen_data.packet_index);
            break;
        }

        if (cmd_len - 11 > 0)
        {
            data_len = cmd_len - 11;
        }
        else
        {
            APP_PRINT_ERROR1("cmd_len <11 cmd_len = %d  ", cmd_len);
            break;
        }
        memcpy(current_image.packet_op_buf, (cmd_ptr + 11), data_len);
        write_data_len += data_len;
        current_image.packet_op_buf += data_len;

        if (screen_data.packet_flag == 0x03)
        {
            APP_PRINT_INFO6("last paket  current_image.screen_cast_index:%d packet_op_buf= 0x%x packet_begin_buf = 0x%x write_data_len = 0x%x buf_used %d  packet_flag = %d ",
                            current_image.screen_cast_index,
                            current_image.packet_op_buf,
                            current_image.packet_begin_buf,
                            write_data_len, buf_used,
                            screen_data.packet_flag);

            if (write_data_len == screen_data.total_size)
            {
                gui_img_set_attribute(map_display, "map_display", current_image.packet_begin_buf, 0, 0);
                map_display->base.not_show = false;
            }
            else
            {
                APP_PRINT_WARN2(" screen_data.total_size !!!!==== write_data_len total_size  =0x%x  write_data_len= 0x%x",
                                screen_data.total_size, write_data_len);
                break;
            }
            buf_used++;
            if (buf_used == 3)
            {
                buf_used = 0;
            }
        }
        break;
#endif

#if F_APP_BT_PROFILE_MAP_MCE_SUPPORT
    case CMD_MAP_SDP_REQUEST...CMD_MAP_PUSH_MESSAGE:
        app_map_handle_cmd(app_idx, (T_CMD_PATH)cmd_path, cmd_ptr, cmd_len, ack_pkt);

        break;
#endif

    case CMD_ACK:
        {
            uint16_t event_id = (uint16_t)(cmd_ptr[2] | (cmd_ptr[3] << 8));
            if (cmd_path == CMD_PATH_UART)
            {
                app_transfer_switch(cmd_path, event_id);
            }
            else if ((cmd_path == CMD_PATH_LE) || (cmd_path == CMD_PATH_SPP) || (cmd_path == CMD_PATH_IAP) ||
                     (cmd_path == CMD_PATH_GATT_OVER_BREDR))
            {
                uint8_t status = cmd_ptr[4];

                app_transfer_switch(cmd_path, event_id);

                if (event_id == EVENT_AUDIO_EQ_PARAM_REPORT)
                {
                    if (cmd_path == CMD_PATH_LE)
                    {
                        if (status != CMD_SET_STATUS_COMPLETE)
                        {
                            app_eq_report_terminate_param_report(cmd_path, app_idx);
                        }
                        else
                        {
                            app_eq_report_eq_param(cmd_path, app_idx);
                        }
                    }
                }
                else if (event_id == EVENT_OTA_ACTIVE_ACK || event_id == EVENT_OTA_ROLESWAP)
                {
                    if ((cmd_path == CMD_PATH_SPP) || (cmd_path == CMD_PATH_IAP) ||
                        (cmd_path == CMD_PATH_GATT_OVER_BREDR))
                    {
                        app_ota_cmd_ack_handle(event_id, status);
                    }
                }
#if F_APP_FLASH_DUMP_SUPPORT
                else if (event_id == EVENT_DUMP_FLASH || event_id == EVENT_REPORT_DUMP_STATE)
                {
                    app_flash_dump_handle_event(event_id, cmd_path, app_idx);
                }
#endif
            }
        }
        break;

    case CMD_BT_READ_PAIRED_RECORD:
    case CMD_BT_CREATE_CONNECTION:
    case CMD_BT_DISCONNECT:
    case CMD_BT_READ_LINK_INFO:
    case CMD_BT_GET_REMOTE_NAME:
    case CMD_BT_IAP_LAUNCH_APP:
    case CMD_BT_SEND_AT_CMD:
    case CMD_BT_HFP_DIAL_WITH_NUMBER:
    case CMD_GET_BD_ADDR:
    case CMD_BT_GET_LOCAL_ADDR:
    case CMD_GET_LEGACY_RSSI:
    case CMD_BT_BOND_INFO_CLEAR:
    case CMD_GET_NUM_OF_CONNECTION:
    case CMD_LINK_USER_PASSKEY_INPUT:
    case CMD_SUPPORT_MULTILINK:
    case CMD_LEGACY_DATA_TRANSFER:
        br_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        break;

    case CMD_LE_START_ADVERTISING...CMD_LE_ATT_MTU_EXCHANGE:
    case CMD_XM_LE_START_ADVERTISING...CMD_XM_LE_GET_PHY:
    case CMD_LE_ADV_CREATE:
    case CMD_LE_MODIFY_WHITELIST...CMD_LE_USER_PASSKEY_INPUT:
        app_cmd_ble_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        break;

    case CMD_MMI:
    case CMD_INFO_REQ:
    case CMD_SET_CFG:
    case CMD_GET_CFG_SETTING:
    case CMD_INDICATION:
    case CMD_LANGUAGE_GET:
    case CMD_LANGUAGE_SET:
    case CMD_GET_STATUS:
    case CMD_GET_BUD_INFO:
    case CMD_GET_FW_VERSION:
    case CMD_WDG_RESET:
    case CMD_GET_FLASH_DATA:
    case CMD_GET_PACKAGE_ID:
    case CMD_GET_EAR_DETECTION_STATUS:
    case CMD_REG_ACCESS:
    case CMD_SEND_RAW_PAYLOAD:
    case CMD_AUDIO_DSP_SCENARIO_CHECK:
    case CMD_GET_IMAGE_INFO:
#if F_APP_SPDIF_SUPPORT
    case CMD_SET_PLAYBACK_DEVICE:
#endif
        {
            app_cmd_general_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;

    case CMD_SET_VP_VOLUME:
    case CMD_AUDIO_DSP_CTRL_SEND:
    case CMD_AUDIO_CODEC_CTRL_SEND:
    case CMD_SET_VOLUME:

    case CMD_DSP_TOOL_OPERATION:
    case CMD_MIC_SWITCH:
    case CMD_DUAL_MIC_MP_VERIFY:
    case CMD_SOUND_PRESS_CALIBRATION:
    case CMD_GET_LOW_LATENCY_MODE_STATUS:
    case CMD_SET_LOW_LATENCY_LEVEL:
        {
            app_audio_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;

    case CMD_AUDIO_EQ_QUERY:
    case CMD_AUDIO_EQ_QUERY_PARAM:
    case CMD_AUDIO_EQ_PARAM_SET:
    case CMD_AUDIO_EQ_PARAM_GET:
    case CMD_AUDIO_EQ_INDEX_SET:
    case CMD_AUDIO_EQ_INDEX_GET:
#if F_APP_USER_EQ_SUPPORT
    case CMD_RESET_EQ_DATA:
#endif
        {
            if (!app_cmd_check_src_eq_spec_version(cmd_path, app_idx))
            {
                ack_pkt[2] = CMD_SET_STATUS_VERSION_INCOMPATIBLE;
                app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
                return;
            }

            app_eq_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;

#if F_APP_DATA_CAPTURE_SUPPORT
    case CMD_DATA_CAPTURE_START_STOP:
        {
            app_data_capture_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
    case CMD_DATA_CAPTURE_ENTER_EXIT:
        {
            app_data_capture_mode_ctl(&cmd_ptr[2], cmd_len - 2, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif

#if F_APP_DEVICE_CMD_SUPPORT
    //case CMD_INQUIRY:
    case CMD_SERVICES_SEARCH:
    case CMD_SET_PAIRING_CONTROL:
    case CMD_SET_PIN_CODE:
    case CMD_GET_PIN_CODE:
    case CMD_PAIR_REPLY:
    case CMD_SSP_CONFIRMATION:
    case CMD_GET_CONNECTED_DEV_ID:
    case CMD_GET_REMOTE_DEV_ATTR_INFO:
        {
            app_cmd_device_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif

#if F_APP_HFP_CMD_SUPPORT
    case CMD_SEND_DTMF:
    case CMD_GET_SUBSCRIBER_NUM:
    case CMD_GET_OPERATOR:
    case CMD_SEND_VGM:
    case CMD_SEND_VGS:
        {
            app_hfp_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif

#if F_APP_AVRCP_CMD_SUPPORT
    case CMD_AVRCP_LIST_SETTING_ATTR:
    case CMD_AVRCP_LIST_SETTING_VALUE:
    case CMD_AVRCP_GET_CURRENT_VALUE:
    case CMD_AVRCP_SET_VALUE:
    case CMD_AVRCP_ABORT_DATA_TRANSFER:
    case CMD_AVRCP_SET_ABSOLUTE_VOLUME:
    case CMD_AVRCP_GET_PLAY_STATUS:
    case CMD_AVRCP_GET_ELEMENT_ATTR:
        {
            app_avrcp_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif

    case CMD_OTA_DEV_INFO:
    case CMD_OTA_IMG_VER:
    case CMD_OTA_INACTIVE_BANK_VER:
    case CMD_OTA_START:
    case CMD_OTA_PACKET:
    case CMD_OTA_VALID:
    case CMD_OTA_RESET:
    case CMD_OTA_ACTIVE_RESET:
    case CMD_OTA_BUFFER_CHECK_ENABLE:
    case CMD_OTA_BUFFER_CHECK:
    case CMD_OTA_IMG_INFO:
    case CMD_OTA_SECTION_SIZE:
    case CMD_OTA_DEV_EXTRA_INFO:
    case CMD_OTA_PROTOCOL_TYPE:
    case CMD_OTA_GET_RELEASE_VER:
    case CMD_OTA_COPY_IMG:
    case CMD_OTA_CHECK_SHA256:
    case CMD_OTA_TEST_EN:
    case CMD_OTA_ROLESWAP:
    case CMD_OTA_CHECK_KEY:
    case CMD_OTA_REPORT_IMAGE_NUM:
        {
            app_ota_cmd_handle(cmd_path, cmd_len, cmd_ptr, app_idx);
        }
        break;

#if F_APP_PBAP_CMD_SUPPORT
    case CMD_PBAP_DOWNLOAD...CMD_PBAP_DISCONNECT:
        {
            app_pbap_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif

#if F_APP_PAN_SUPPORT
    case CMD_PAN_CONN...CMD_PAN_HTTPS:
        {
            app_pan_cmd(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif

    case CMD_ASSIGN_BUFFER_SIZE:
    case CMD_TONE_GEN:
    case CMD_STRING_MODE:
    case CMD_SET_AND_READ_DLPS:
#if F_APP_BT_ANCS_CLIENT_SUPPORT
    case CMD_ANCS_REGISTER ... CMD_ANCS_PERFORM_NOTIFICATION_ACTION:
#endif
    case CMD_LED_TEST:
    case CMD_SWITCH_TO_HCI_DOWNLOAD_MODE:
#if F_APP_ADC_SUPPORT
    case CMD_GET_PAD_VOLTAGE:
#endif
    case CMD_RF_XTAK_K:
    case CMD_RF_XTAL_K_GET_RESULT:

    case CMD_MEMORY_DUMP:
    case CMD_AMS_REGISTER ... CMD_AMS_READ_ENTITY_ATTR:
    case CMD_MP_TEST:
        {
            app_cmd_other_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;

    case CMD_IO_PIN_PULL_HIGH:
    case CMD_ENTER_BAT_OFF_MODE:
    case CMD_MIC_MP_VERIFY_BY_HFP:
    case CMD_USBH_AUDIO_SET_PARAM:
    case CMD_USBH_AUDIO_CONTROL:
        {
            app_cmd_customized_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;

    case CMD_DSP_DEBUG_SIGNAL_IN:
        {

        }
        break;

    /* customer customer command handler*/
    case CMD_XM_ENTER_HCI_MODE:
    case CMD_XM_BT_SET_ADDR:
    case CMD_XM_SPP_DATA_TRANSFER:
    case CMD_XM_SNIFF_MODE_CTRL:
    case CMD_XM_USER_CFM_REQ:
    case CMD_INQUIRY:
    case CMD_BT_HFP_SCO_MAG:
    case CMD_XM_SET_MODE:
    case CMD_XM_DEVICE_REBOOT:
    case CMD_XM_MMI:
    case CMD_XM_GATT_DATA_TRANSFER:
    case CMD_XM_GET_FACTORY_RESET:
    case CMD_XM_SET_XTAL:
    case CMD_XM_SET_GAIN_K:
    case CMD_XM_SET_VOLUME_LEVEL:
    case CMD_XM_AUDIO_TEST_GET_PA_ID...CMD_XM_SET_PA_MODE:
    case CMD_XM_MUSIC:
    case CMD_XM_TTS:
    case CMD_XM_TEST:
    case CMD_XM_SET_CPU_FREQ:
        app_customer_handle_cmd_set(cmd_ptr, cmd_len, cmd_path, rx_seqn, app_idx);
        break;

#if F_APP_CUSTOMER_RECORD_SUPPORT
    case CMD_XM_START_RECORD...CMD_XM_RECORD_THEN_PLAY_VP:
    case CMD_XM_SWITCH_MIC_RECORD:
        app_record_handle_cmd(app_idx, (T_CMD_PATH)cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

    case CMD_GCSS_ADD...CMD_GCSS_WRITE_RSP:
        break;

    case CMD_GCSC_SERV_DISCOVER_ALL...CMD_GCSC_WRITE:
        break;

    case CMD_HFP_AG_CONNECT_SCO...CMD_HFP_AG_INDICATORS_STATUS_SET:
        {
            app_hfp_ag_handle_cmd(cmd_ptr, cmd_len, cmd_path, rx_seqn, app_idx);
        }
        break;

#if (F_APP_SPI_ROLE_MASTER || F_APP_SPI_ROLE_SLAVE)
    case CMD_SPI_INIT:
        app_spi_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if (F_APP_SPI_ROLE_MASTER || F_APP_SPI_ROLE_SLAVE)
#if (F_APP_A2DP_XMIT_SRC_SUPPORT || F_APP_A2DP_XMIT_SRC_LEA_SUPPORT || F_APP_A2DP_XMIT_SNK_SUPPORT || F_APP_A2DP_XMIT_SNK_LEA_SUPPORT)
    case CMD_A2DP_XMIT_ROUTE_OUT_CTRL:
    case CMD_A2DP_XMIT_AUDIO:
    case CMD_A2DP_XMIT_CONFIG:
    case CMD_A2DP_XMIT_SET_ROUTE_OUT:
    case CMD_A2DP_XMIT_NEED_FORMAT:
        app_a2dp_xmit_mgr_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif
#endif

#if (F_APP_SCO_XMIT_HF_SUPPORT || F_APP_SCO_XMIT_AG_SUPPORT)
    case CMD_SCO_XMIT_CONFIG:
    case CMD_SCO_XMIT_AUDIO:
    case CMD_SCO_XMIT_NEED_FORMAT:
        app_sco_xmit_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if BAP_BROADCAST_SOURCE
    case CMD_LEA_BSRC_INIT...CMD_LEA_BST_REMOVE_ALL_SOURCE:
        app_lea_ini_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if F_APP_LE_AUDIO_ACCEPTOR_SUPPORT
    case CMD_LEA_ADV_START...CMD_LEA_VCS_SET:
        app_lea_acc_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if F_SOURCE_PLAY_SUPPORT
    case CMD_SRC_PLAY_SET_SRC_ROUTE...CMD_SRC_PLAY_GET_PLAY_STATE:
    case CMD_SRC_PLAY_SET_MULTI_ESCO:
    case CMD_SRC_PLAY_ATTACH_FEATURE:
    case CMD_SRC_PLAY_DETACH_FEATURE:
    case CMD_SRC_PLAY_SET_RECORD_FORMAT:
    case CMD_SRC_PLAY_A2DP_VOLUME_UP:
    case CMD_SRC_PLAY_A2DP_VOLUME_DOWN:
        app_src_play_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#if F_APP_SD_CARD_LOCALPLAY
    case CMD_SRC_SD_PLAYPACK_PAUSE...CMD_SRC_SD_PLAYBACK_BWD_PLAYLIST:
    case CMD_SRC_PLAY_SET_SRC_FILE_INDEX:
    case CMD_SRC_PLAY_SET_PCM_FORMAT:
        app_src_playback_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif
#endif

    case CMD_FS_LIST_FILES...CMD_FS_END:
        app_fs_test_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;

#if F_APP_UART_DFU
    case CMD_UART_DFU:
        app_uart_dfu_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if F_APP_BT_HID_HOST_SUPPORT
    case CMD_HID_HOST_CONNECT...CMD_HID_HOST_DISCONNECT:
        app_hid_host_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
    case CMD_FINDMY_FEATURE:
        app_findmy_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if CONFIG_REALTEK_GFPS_FINDER_SUPPORT
    case CMD_GFPS_FINDER_FEATURE:
        app_gfps_finder_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if F_APP_SAIYAN_MODE
    case CMD_SAIYAN_GAIN_CTL:
        app_data_capture_gain_ctl(&cmd_ptr[2], cmd_len - 2, cmd_path, app_idx, ack_pkt);
        break;
#endif

#if TRANSMIT_CLIENT_SUPPORT
    case CMD_CHARGING_CASE_LE_CONTROL:
        app_le_transfer_control_cmd_handle(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if F_APP_CHARGING_CASE_CMD_SUPPORT
    case CMD_CHARGER_CASE_FIND_CHARGERBOX:
    case CMD_CHARGER_CASE_REPORT_STATUS:
    case CMD_CHARGER_CASE_BUD_AUTO_PAIR_SUC:
        app_charging_case_cmd_handle(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        break;
#endif

#if F_APP_BT_AUDIO_TRI_DONGLE
    case CMD_TRI_DONGLE_CMD_LIST:
        {
            T_TRI_DONGLE_CMD tri_dongle_cmd;

            tri_dongle_cmd.cmd_ptr = cmd_ptr;
            tri_dongle_cmd.cmd_len = cmd_len;
            tri_dongle_cmd.cmd_path = cmd_path;
            tri_dongle_cmd.rx_seqn = rx_seqn;
            tri_dongle_cmd.app_idx = app_idx;
            tri_dongle_cmd.ack_pkt = ack_pkt;
#if F_APP_LEAUDIO_SPP_CMD_SUPPORT
            if (cmd_path == CMD_PATH_SPP)
            {
                app_tri_dongle_le_audio_spp_cmd(tri_dongle_cmd, true);
            }
            else
#endif
            {
                app_tri_dongle_handle_cmd(tri_dongle_cmd, true);
            }
        }
        break;
#endif

#if F_APP_BLE_HID_DEVICE_SUPPORT
    case CMD_LE_HID_CONTROL:
        {
            app_ble_hid_device_cmd_handle(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        }
        break;
#endif

#if F_APP_SAIYAN_EQ_FITTING
    case CMD_HW_EQ_TEST_MODE:
    case CMD_HW_EQ_START_SET:
    case CMD_HW_EQ_CONTINUE_SET:
    case CMD_HW_EQ_CLEAR_CALIBRATE_FLAG:
    case CMD_HW_EQ_SET_TEST_MODE_TMP_EQ:
    case CMD_HW_EQ_SET_TEST_MODE_TMP_EQ_ADVANCED:
        {
            app_eq_fitting_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif

#if F_APP_MALLEUS_SUPPORT
    case CMD_MALLEUS_CUSTOMIZED_FEATURE:
        {
            app_malleus_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif

#if CONFIG_REALTEK_APP_AI_RECORD
    case CMD_AI_RECORD_WIFI_QUERY_INFO:
    case CMD_AI_RECORD_WIFI_POWER_ON:
    case CMD_AI_RECORD_WIFI_POWER_DOWN_RESP:
    case CMD_AI_RECORD_WIFI_GET_POWER_STATE:
    case CMD_AI_RECORD_WIFI_UPDATE_INFO_STATE:
    case CMD_AI_RECORD_WIFI_SNAPSHOT:
    case CMD_AI_RECORD_WIFI_GET_FILE_NAME:
    case CMD_AI_RECORD_WIFI_GET_FILE_DATA:
    case CMD_AI_RECORD_WIFI_TRANS_STOP:
    case CMD_AI_RECORD_WIFI_RECORD_START:
    case CMD_AI_RECORD_WIFI_RECORD_CONTINUE:
    case CMD_AI_RECORD_WIFI_SYNC_TIMESTAMP:
    case CMD_AI_RECORD_WIFI_RECORD_STOP:
    case CMD_AI_RECORD_WIFI_GET_FILE_CNT:
    case CMD_AI_RECORD_WIFI_DELETE_FILE:
    case CMD_AI_RECORD_WIFI_DELETE_ALL:
    case CMD_AI_RECORD_WIFI_GET_SD_INFO:
    case CMD_AI_RECORD_WIFI_SET_WIFI_RESP:
    case CMD_AI_RECORD_WIFI_GET_DATA_TCP:
    case CMD_AI_RECORD_WIFI_DATA_TCP_ACK:
    case CMD_AI_RECORD_WIFI_FW_ROLLBACK:
    case CMD_AI_RECORD_WIFI_SET_STA_MODE_RESP:
    case CMD_AI_RECORD_WIFI_LIVE_START_RESP:
    case CMD_AI_RECORD_WIFI_LIVE_STOP_RESP:
    case CMD_AI_RECORD_WIFI_LIVE_STREAM_RESP:
    case CMD_AI_RECORD_WIFI_AUDIO_START:
    case CMD_AI_RECORD_WIFI_AUDIO_CONTINUE:
    case CMD_AI_RECORD_WIFI_AUDIO_STOP:
    case CMD_AI_RECORD_WIFI_RTSP_START_RESP:
    case CMD_AI_RECORD_WIFI_RTSP_STOP_RESP:
    case CMD_AI_RECORD_WIFI_SDIO_TX_TEST:
        {
#if F_APP_WIFI_RTL8720C
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            ai_record_cmd_wifi_8720c_handle(cmd_path, cmd_len, cmd_ptr, app_idx);

#else
            ai_record_cmd_wifi_handle(cmd_path, cmd_len, cmd_ptr, app_idx);
#endif
        }
        break;

    // AI APP cmd
    case CMD_AI_RECORD_APP_QUERY_INFO:
    case CMD_AI_RECORD_APP_GET_SD_INFO:
    case CMD_AI_RECORD_APP_UPDTAE_WIFI_INFO:
    case CMD_AI_RECORD_APP_SNAPSHOT:
    case CMD_AI_RECORD_APP_GET_FILE_INFO:
    case CMD_AI_RECORD_APP_GET_FILE_DATA:
    case CMD_AI_RECORD_APP_TRANS_STOP:
    case CMD_AI_RECORD_APP_VOICE_START:
    case CMD_AI_RECORD_APP_VOICE_STOP:
    case CMD_AI_RECORD_APP_SET_GPS:
    case CMD_AI_RECORD_APP_SET_WIFI_MODE:
    case CMD_AI_RECORD_APP_GET_FILE_CNT:
    case CMD_AI_RECORD_APP_GET_DATA_TCP:
    case CMD_AI_RECORD_APP_DATA_TCP_ACK:
    case CMD_AI_RECORD_APP_GET_NEXT_FILE:
    case CMD_AI_RECORD_APP_SET_WIFI_STA_INFO:
    case CMD_AI_RECORD_APP_LIVE_START:
    case CMD_AI_RECORD_APP_LIVE_STOP:
    case CMD_AI_RECORD_APP_RTSP_START:
    case CMD_AI_RECORD_APP_RTSP_STOP:
        {
            ai_record_cmd_app_handle(cmd_path, cmd_len, cmd_ptr, app_idx);
        }
        break;

    case CMD_AI_RECORD_APP_SET_MODE:
        {
            bool fast_mode = cmd_ptr[2];
            //app_ai_record_set_mode_cmd_handle(fast_mode);
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
#endif

#if CONFIG_REALTEK_APP_AI_AUTH
    case CMD_START_AUTH:
    case CMD_AUTH_KEY_EXCHANGE:
    case CMD_DYNAMIC_AUTH_DATA_EXCHANGE:
    case CMD_ENCRYPTED_AUTH_DATA_VERIFY:
        {
            app_rtk_auth_cmd_handle(cmd_path, cmd_len, cmd_ptr, app_idx);
        }
        break;
#endif

#if (defined(CONFIG_WIFI_8711_CMD) || F_APP_WIFI_UART_CMD)
    case CMD_AT_CMD_WLCONN:
        {
#if defined(CONFIG_WIFI_8711_CMD)
            extern void app_spi_atcmd_demo(uint8_t type);
            app_spi_atcmd_demo(cmd_ptr[2]);
#elif F_APP_WIFI_UART_CMD
            /* bt_audio_trx keeps the UART AT-cmd path for rtl8783gbf; watch is
             * SPI-only and has no #elif branch here. */
            extern void app_uart_atcmd_demo(uint8_t *ptr);
            app_uart_atcmd_demo(&(cmd_ptr[2]));
#endif
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
#endif

#if defined(CONFIG_WIFI_8711_CMD)
    /*
     * SPI + TCP throughput test (uplink + downlink; slave as server or client).
     * Payload layout (cmd_ptr):
     *   [0..1] = CMD_SPI_TCP_TP_TEST
     *   [2]    = sub-action:
     *            0 INIT
     *            1 WIFI_CONNECT   [3..] = ASCII "ssid,password"
     *            2 CREATE_SERVER  [3..4] = port (uint16 LE)
     *            3 START_UPLINK   [3]=link_id(0xFF=auto) [4..5]=chunk LE [6..7]=duration_s LE
     *            4 STOP
     *            5 QUERY_STATE
     *            6 AUTO_RUN       [3..4]=port LE [5..6]=chunk LE [7..8]=duration_s LE
     *                             [9..] = ASCII "ssid,password"
     *            7 START_DOWNLINK [3]=link_id(0xFF=auto) [4..5]=duration_s LE
     *            8 CREATE_CLIENT  [3..6]=remote ip octets [7..8]=port LE [9]=link_id
     *            9 START_UPLINK_BLAST [3]=link_id(0xFF=auto) [4..7]=total bytes LE
     */
    case CMD_SPI_TCP_TP_TEST:
        {
            app_spi_tcp_tp_handle_cmd(cmd_ptr, cmd_len);
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
#endif

#if F_APP_FLASH_DUMP_SUPPORT
    case CMD_FLASH_CHECK_RESULT...CMD_FEATURE_STATE_GET:
        {
            app_flash_dump_handle_cmd(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        }
        break;
#endif

#if (CONFIG_REALTEK_APP_DASHBOARD_WITH_MIJIA_SUPPORT == 1)
    case CMD_MIJIA_INPUT_OOB...CMD_MIJIA_GET_OTP:
        {
            app_mijia_sample_handle_cmd(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        }
        break;
#endif

    case CMD_LE_COMMON_ADV_CONTROL:
    case CMD_LE_COMMON_ADV_UPDATE_INTERVAL:
        {
            app_ble_common_adv_handle_cmd_set(app_idx, cmd_path, cmd_ptr, cmd_len, ack_pkt);
        }
        break;

    default:
        {
            ack_pkt[2] = CMD_SET_STATUS_UNKNOW_CMD;
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;
    }

    APP_PRINT_TRACE1("app_handle_cmd_set--: ack_status 0x%02x", ack_pkt[2]);
}




// wifi test
// 8720c wifi
#if CONFIG_REALTEK_APP_AI_RECORD
#if CONFIG_APP_WIFI_SDIO
/* WiFi module SDIO transport helpers (wifi_init/wifi_enable/wifi_sdio_init/
 * sdio_tx_test) live in src/ai_record/wifi/ and are compiled only when
 * CONFIG_APP_WIFI_SDIO is set. The SPI-only build (gtp record_pen) has no SDIO
 * and skips the wifi/ dir, so these must be compiled out even when
 * the SPI command path (CONFIG_WIFI_8711_CMD) is enabled (the guard here was
 * previously (SPI||UART), which wrongly pulled them into the gtp build and
 * broke the link). */
#include "wifi_app.h"
uint8_t wifi_enable_flag = 0;
extern int sdio_tx_test(char *p_param);
#endif
// static void //wifi_8720c_uart_send(uint16_t event_id, uint8_t *data, uint16_t len)
// {
//     if (ai_record_func_cb[AI_RECORD_CB_IDX_UART_SEND])
//     {
//         ai_record_func_cb[AI_RECORD_CB_IDX_UART_SEND](event_id, data, len);
//     }
// }
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
void ai_record_cmd_wifi_8720c_handle(uint8_t path, uint16_t length, uint8_t *p_value,
                                     uint8_t app_idx)
{
    uint8_t ack_pkt[3];

    uint16_t cmd_id = *(uint16_t *)p_value;
    uint8_t *p;
    bool ack_flag = false;

    ack_pkt[0] = p_value[0];
    ack_pkt[1] = p_value[1];
    ack_pkt[2] = CMD_SET_STATUS_COMPLETE;

    if (length < 2)
    {
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
        APP_PRINT_ERROR0("ai_record_cmd_wifi_8720c_handle: error length");
        return;
    }

    length = length - 2;
    p = p_value + 2;

    APP_PRINT_TRACE3("===>ai_record_rx_from_wifi_8720c, cmd_id 0x%x, len 0x%x, data %b",
                     cmd_id, length, TRACE_BINARY(MIN(8, length), p));

    switch (cmd_id)
    {
    case CMD_AI_RECORD_WIFI_QUERY_INFO:
        {
            // if (length == AI_RECORD_WIFI_LEN_GET_INFO)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: clear local, handle query info */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_POWER_ON:
        {
#if CONFIG_APP_WIFI_SDIO
            /* p_value[2] is the 2nd payload byte (p[1]); the top-of-function
             * guard only ensures length >= 2, so require >= 2 remaining
             * payload bytes here before reading it to avoid an OOB read. */
            if (length < 1)
            {
                ack_flag = true;
                break;
            }
            if (p_value[2])
            {
                wifi_init();
                wifi_enable(true);
                wifi_sdio_init();
                wifi_enable_flag = 1;
            }
            else
            {
                wifi_enable(false);
            }
#endif
        }
        break;

    case CMD_AI_RECORD_WIFI_SDIO_TX_TEST:
        {
#if CONFIG_APP_WIFI_SDIO
            // Default handler when no OK/ERROR
            if (wifi_enable_flag == 1)
            {
                sdio_tx_test(NULL);
            }
            else
            {
                APP_PRINT_WARN0("wifi_enable_flag false , power on wifi first");
            }
#endif
        }

        break;
    case CMD_AI_RECORD_WIFI_POWER_DOWN_RESP:
        {
            uint8_t dev_power_state = 0;
            // if (length == AI_RECORD_WIFI_LEN_POWER_DOWN_STATE)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     dev_power_state = p[0];
            //     /* TODO: handle wifi power down (dev_power_state) */
            //     (void)dev_power_state;
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_UPDATE_INFO_STATE:
        {
            // if (length == AI_RECORD_WIFI_LEN_UPDATE_PARAM)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_GET_POWER_STATE:
        {
            // if (length == AI_RECORD_WIFI_LEN_GET_POWER_STATE)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: handle notify state (p) */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_SNAPSHOT:
        {
            // uint8_t snapshot_state = 0;
            // if (length == AI_RECORD_WIFI_LEN_SNAPSHOT_STATE)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     snapshot_state = p[0];
            //     /* TODO: handle snapshot state per wifi_process and app_snapshot_state */
            //     (void)snapshot_state;
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_GET_FILE_NAME:
        {
            // uint16_t file_name_len;
            // uint16_t param_len;
            // LE_ARRAY_TO_UINT16(file_name_len, p + 1);
            // param_len = 1 + 2 + 4 + file_name_len;
            // if (length == param_len)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: handle get file name, report AI snapshot complete */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_GET_FILE_DATA:
        {
            // uint16_t pkt_len;
            // uint16_t param_len;
            // LE_ARRAY_TO_UINT16(pkt_len, p + 1);
            // param_len = 1 + 2 + pkt_len;
            // if (length == param_len)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: handle get file data */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_GET_DATA_TCP:
        {
            /* TODO: handle rx wifi -> tx AI data (p, length) */
        }
        break;
    case CMD_AI_RECORD_WIFI_TRANS_STOP:
        {
            // if (length == AI_RECORD_WIFI_LEN_TRANS_STOP)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: stop ai_snapshot timer, handle trans done, set lp mode */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_RECORD_START:
        {
            // uint8_t record_state = 0;
            // if (length == AI_RECORD_WIFI_LEN_RECORD_START)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     record_state = p[0];
            //     /* TODO: if record_state != 0 call record_stop */
            //     (void)record_state;
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_SYNC_TIMESTAMP:
        {
            // //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            // if (length != AI_RECORD_WIFI_LEN_SYNC_TIMESTAMP)
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_RECORD_CONTINUE:
        {
            // uint8_t record_state = 0;
            // //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            // if (length == AI_RECORD_WIFI_LEN_RECORD_CONTINUE)
            // {
            //     record_state = p[0];
            //     /* TODO: if record_state != 0 call record_stop, else restart record video timer */
            //     (void)record_state;
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_RECORD_STOP:
        {
            // if (length == AI_RECORD_WIFI_LEN_RECORD_STOP)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: stop record video timer, call record_stop, wifi power down */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_AUDIO_START:
        {
            //     uint8_t audio_state = 0;
            //     if (length == AI_RECORD_WIFI_LEN_AUDIO_START)
            //     {
            //         //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //         audio_state = p[0];
            //         /* TODO: if audio_state != 0 call record_stop */
            //         (void)audio_state;
            //     }
            //     else
            //     {
            //         ack_flag = true;
            //     }
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_AUDIO_CONTINUE:
        {
            // uint8_t audio_state = 0;
            // //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            // if (length == AI_RECORD_WIFI_LEN_AUDIO_CONTINUE)
            // {
            //     audio_state = p[0];
            //     /* TODO: if audio_state != 0 call record_stop, else restart record audio timer */
            //     (void)audio_state;
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_AUDIO_STOP:
        {
            // if (length == AI_RECORD_WIFI_LEN_AUDIO_STOP)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: stop record audio timer, call record_stop, wifi power down */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_GET_FILE_CNT:
        {
            // if (length == AI_RECORD_WIFI_LEN_GET_FILE_CNT)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: parse result/snapshot_cnt/video_cnt from p, call get_file_cnt_cb */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_DELETE_FILE:
    case CMD_AI_RECORD_WIFI_DELETE_ALL:
        break;
    case CMD_AI_RECORD_WIFI_GET_SD_INFO:
        {
            // if (length == AI_RECORD_WIFI_LEN_GET_SD_INFO)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: copy sd_info from p */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_SET_WIFI_RESP:
        {
            // if (length == AI_RECORD_WIFI_LEN_SET_WIFI_RESQ)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: stop set_wifi_mode timer, report to app, handle fail case */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_SET_STA_MODE_RESP:
        {
            // if (length == AI_RECORD_WIFI_LEN_SET_STA_MODE_RESP)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: stop sta_mode timer, handle fail case, call sta_resp_cb */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_LIVE_START_RESP:
        {
            // uint8_t live_state = 0;
            // if (length == AI_RECORD_WIFI_LEN_LIVE_START_RESP)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     live_state = p[0];
            //     /* TODO: if live_state != 0 call live_stop_cb, else stop timer & set hp mode;
            //      *       report EVENT_AI_RECORD_APP_LIVE_START_RESP to app */
            //     (void)live_state;
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_LIVE_STOP_RESP:
        {
            // if (length == AI_RECORD_WIFI_LEN_LIVE_STOP_RESP)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: report EVENT_AI_RECORD_APP_LIVE_STOP_RESP to app,
            //      *       set scenario idle, wifi power down */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_LIVE_STREAM_RESP:
        {
            /* TODO: handle rx wifi -> tx stream data (p, length) */
        }
        break;
    case CMD_AI_RECORD_WIFI_RTSP_START_RESP:
        {
            // uint8_t rtsp_state = 0;
            // if (length == AI_RECORD_WIFI_LEN_RTSP_START_RESP)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     rtsp_state = p[0];
            //     /* TODO: if rtsp_state != 0 call rtsp_stop_cb, else stop timer;
            //      *       report EVENT_AI_RECORD_APP_RTSP_START_RESP to app */
            //     (void)rtsp_state;
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    case CMD_AI_RECORD_WIFI_RTSP_STOP_RESP:
        {
            // if (length == AI_RECORD_WIFI_LEN_RTSP_STOP_RESP)
            // {
            //     //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
            //     /* TODO: report EVENT_AI_RECORD_APP_RTSP_STOP_RESP to app,
            //      *       set scenario idle */
            // }
            // else
            // {
            //     ack_flag = true;
            // }
        }
        break;
    default:
        ack_pkt[2] = CMD_SET_STATUS_UNKNOW_CMD;
        //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
        break;
    }

    if (ack_flag == true)
    {
        APP_PRINT_TRACE0("ai_record_cmd_wifi_8720c_handle: invalid length");
        ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
        //wifi_8720c_uart_send(EVENT_ACK, ack_pkt, 3);
    }
}
#endif /* CONFIG_REALTEK_APP_AI_RECORD */
