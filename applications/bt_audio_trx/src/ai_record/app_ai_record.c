/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */



#if CONFIG_REALTEK_APP_AI_RECORD

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "trace.h"
#include "app_main.h"
#include "os_msg.h"
#include "os_task.h"
#include "rtl876x.h"
#include "sysm.h"
#include "app_cmd.h"
#include "app_mmi.h"
#include "app_cfg.h"
#include "app_timer.h"
#include "app_auto_power_off.h"
#include "app_ipc.h"
#include "app_device.h"
#include "app_dlps.h"
#include "app_msg.h"
#include "hal_gpio.h"
#include "hal_gpio_int.h"
#include "app_hfp.h"
#include "app_a2dp.h"
#include "app_transfer.h"
#include "ai_record.h"
#include "app_ai_record.h"
#include "app_ai_record_process.h"
#include "app_ai_record_service.h"
#include "app_ai_record_file_trans.h"
#include "app_flags.h"
#if APP_AI_RECORD_PTA_SUPPORT
#include <gap_vendor.h>
#endif

#include "audio.h"
#include "app_audio_policy.h"
#if F_APP_DFU
#include "app_dfu.h"
#endif
#if CONFIG_REALTEK_APP_AI_AUTH
#include "app_rtk_auth.h"
#endif
#include "app_ai_record_rtc.h"
#include "app_sniff_mode_cs.h"

#if defined(CONFIG_WIFI_8711_CMD) && !F_APP_WIFI_UART_CMD
#include "app_spi_atcmd.h"
#endif
#if F_APP_WIFI_UART_CMD
#include "app_uart_atcmd.h"
#endif

#if (F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD)
#include "wifi_transport.h"
#endif

#define AI_RECORD_WIFI_MODE_IDLE            0
#define AI_RECORD_WIFI_MODE_STA             1

#define AI_RECORD_SIDE_ROLE_SINGLE      0
#define AI_RECORD_SIDE_ROLE_LEFT        1
#define AI_RECORD_SIDE_ROLE_RIGHT       2

typedef struct _app_ai_reocrd_db
{
    uint8_t scenario_state;
    uint8_t avrcp_play_status;
    bool vad_resume_flag;
    bool record_video_later_flag;
    bool record_audio_later_flag;
} T_APP_AI_RECORD_DB;

static T_APP_AI_RECORD_DB *app_ai_reocrd_db;

static void app_ai_record_check_and_quit_ai_record(void);
static bool app_ai_record_check_battery_level(void);

void app_ai_record_mmi_action(uint8_t action)
{
    APP_PRINT_INFO1("app_ai_record_mmi_action: action 0x%02x", action);

    switch (action)
    {
    case AI_RECORD_MMI_AI_VOICE_START:
        {
            app_ai_record_start_recording(0, false);
        }
        break;

    case AI_RECORD_MMI_AI_VOICE_STOP:
        {
            app_ai_record_stop_recording(0, false);
        }
        break;

    default:
        break;
    }
}

bool app_ai_record_mmi_handle(uint8_t action)
{
    bool mmi_handled = true;

    switch (action)
    {
    case MMI_AI_VOICE_START:
        {

            app_ai_record_mmi_action(AI_RECORD_MMI_AI_VOICE_START);

        }
        break;

    case MMI_AI_VOICE_STOP:
        {
            if (app_ai_record_is_recording())
            {
                app_ai_record_mmi_action(AI_RECORD_MMI_AI_VOICE_STOP);
            }
        }
        break;

    default:
        {
            mmi_handled = false;
        }
        break;
    }

    return mmi_handled;
}

bool app_ai_record_a2dp_check_pause(void)
{
    if (app_db.avrcp_play_status == BT_AVRCP_PLAY_STATUS_PLAYING)
    {
        app_ai_reocrd_db->avrcp_play_status = BT_AVRCP_PLAY_STATUS_PLAYING;
        app_mmi_handle_action(MMI_AV_PLAY_PAUSE);
        return false;
    }
    return true;
}

void app_ai_record_a2dp_check_resume(void)
{
    if ((app_db.avrcp_play_status == BT_AVRCP_PLAY_STATUS_PAUSED) &&
        (app_ai_reocrd_db->avrcp_play_status == BT_AVRCP_PLAY_STATUS_PLAYING))
    {
        app_ai_reocrd_db->avrcp_play_status = 0;
        app_mmi_handle_action(MMI_AV_PLAY_PAUSE);
    }
}

static void app_ai_record_check_and_quit_ai_record(void)
{
    app_ai_record_mmi_handle(MMI_AI_VOICE_STOP);
}


static bool app_ai_record_check_battery_level(void)
{
    bool ret = true;
    if (app_db.local_batt_level >= AI_RECORD_BATTERY_EN_WIFI_SOC_THRESHOLD)
    {
        return ret;
    }
    else
    {
//        ret = false;
    }
    return ret;
}

#if APP_AI_RECORD_PTA_SUPPORT
static void app_ai_record_set_pta(uint8_t type, uint8_t priority, uint16_t handle)
{
    uint8_t policy[4] = {0};
    policy[0] = type;
    policy[1] = priority;
    policy[2] = (handle) & 0xff;
    policy[3] = (handle >> 8) & 0xff;
    gap_vendor_cmd_req(0xFDC0, 4, policy);
    APP_PRINT_TRACE1("app_ai_record_set_pta, priority %d", priority);
}
#endif

void app_ai_record_connected_handle(uint8_t cmd_path, uint16_t mut_size)
{
    app_transfer_queue_reset(cmd_path);
    ai_record_cmd_ai_connected_handle(cmd_path, mut_size);
}

void app_ai_record_disconnected_handle(uint8_t cmd_path)
{
    uint8_t app_idx = ai_record_app_idx_get();
    app_sniff_mode_b2s_enable(app_db.br_link[app_idx].bd_addr, SNIFF_DISABLE_MASK_SPP_RECORD);

    app_transfer_queue_reset(cmd_path);
    ai_record_cmd_ai_disconnected_handle();
}

static void app_ai_record_audio_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    bool handle = true;
    uint8_t active_a2dp_idx = app_a2dp_get_active_idx();
    uint8_t active_hf_idx = app_hfp_get_active_idx();
    T_APP_BR_LINK *p_link;

    switch (event_type)
    {
    case BT_EVENT_SCO_CONN_IND:
        {
            p_link = app_link_find_br_link(param->sco_conn_ind.bd_addr);
            if (p_link == NULL)
            {
                break;
            }
            app_ai_record_check_and_quit_ai_record();
        }
        break;
    case BT_EVENT_SCO_CONN_RSP:
        {
            if (param->sco_conn_rsp.cause == 0)
            {
                app_ai_record_check_and_quit_ai_record();
            }
        }
        break;

    case BT_EVENT_SCO_CONN_CMPL:
        {

        }
        break;

    case BT_EVENT_SCO_DISCONNECTED:
        {

        }
        break;

    case BT_EVENT_A2DP_STREAM_START_IND:
        {

        }
        break;

    case BT_EVENT_A2DP_STREAM_STOP:
    case BT_EVENT_A2DP_STREAM_CLOSE:
        {

        }
        break;

    case BT_EVENT_SPP_CONN_CMPL:
        {
            app_ai_record_connected_handle(CMD_PATH_SPP, param->spp_conn_cmpl.frame_size);
        }
        break;

    case BT_EVENT_SPP_DISCONN_CMPL:
        {
            T_BT_EVENT_PARAM *param = event_buf;

            if (param->spp_disconn_cmpl.cause != (HCI_ERR | HCI_ERR_CONN_ROLESWAP))
            {
                app_ai_record_disconnected_handle(CMD_PATH_SPP);
            }
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_ai_record_audio_bt_cback: event_type 0x%04x", event_type);
    }
}

uint8_t app_ai_record_get_ai_conn_state(void)
{
    return ai_record_cmd_get_ai_conn_state();
}

static void app_ai_record_startup(void)
{
    if (app_ai_record_check_battery_level() == true)
    {
        ai_record_cmd_init_wifi_info();
    }
}

static void app_ai_record_shutdown(void)
{
    if (app_ai_record_is_recording() == true)
    {
        app_ai_record_mmi_action(AI_RECORD_MMI_AI_VOICE_STOP);
    }

#if CONFIG_REALTEK_APP_RTC_CALENDAR_SUPPORT
    T_UTC_TIME utc_time;
    app_ai_record_rtc_get_utc_time(&utc_time);
    app_ai_record_rtc_info_save((uint8_t *)&utc_time, APP_RW_UTC_INFO_SIZE);
#endif
    ai_record_system_shutdown_handle();
}

static void app_ai_record_sysm_cback(T_SYS_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    bool handle = true;

    switch (event_type)
    {
    case SYS_EVENT_POWER_OFF:
        {
            app_ai_record_shutdown();
        }
        break;

    case SYS_EVENT_POWER_ON:
        {
            app_ai_record_startup();
        }
        break;

    default:
        {
            handle = false;
        }
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_ai_record_sysm_cback: event_type 0x%04x", event_type);
    }
}

static void app_ai_record_device_event_cback(uint32_t event, void *msg)
{
    if (msg)
    {
        bool *need_power_on = (bool *)msg;
        switch (event)
        {
        case APP_DEVICE_IPC_EVT_STACK_READY:
            {
                APP_PRINT_INFO0("app_ai_record_device_event_cback");
            }
            break;

        default:
            break;
        }
    }

}

static void app_ai_record_audio_cback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    uint8_t active_hf_idx = app_hfp_get_active_idx();

    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            T_APP_BR_LINK *p_link;
            T_AUDIO_STREAM_TYPE stream_type;

            switch (param->track_state_changed.state)
            {
            case AUDIO_TRACK_STATE_STARTED:
                {
                    if (audio_track_stream_type_get(param->track_state_changed.handle, &stream_type) == false)
                    {
                        break;
                    }

                    if (stream_type == AUDIO_STREAM_TYPE_VOICE)
                    {
                        p_link = &app_db.br_link[active_hf_idx];

                        if (p_link != NULL)
                        {
                            if (param->track_state_changed.state == AUDIO_TRACK_STATE_STARTED)
                            {
                            }
                        }
                    }
                    else if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
                    {
                        app_ai_reocrd_db->record_video_later_flag = false;
                        app_ai_reocrd_db->record_audio_later_flag = false;
                    }
                }
                break;

            case AUDIO_TRACK_STATE_STOPPED:
            case AUDIO_TRACK_STATE_PAUSED:
                {
                    if (audio_track_stream_type_get(param->track_state_changed.handle, &stream_type) == false)
                    {
                        break;
                    }

                    if (stream_type == AUDIO_STREAM_TYPE_VOICE)
                    {
                        p_link = &app_db.br_link[active_hf_idx];

                        if ((p_link != NULL) && app_ai_reocrd_db->vad_resume_flag)
                        {
                            // resume vad
                            app_ai_reocrd_db->vad_resume_flag  = false;
                            app_mmi_handle_action(MMI_AUDIO_VAD_OPEN);
                        }
                    }
                    else if (stream_type == AUDIO_STREAM_TYPE_PLAYBACK)
                    {
                        if (app_ai_reocrd_db->record_video_later_flag)
                        {
                            app_ai_reocrd_db->record_video_later_flag = false;
                        }
                        else if (app_ai_reocrd_db->record_audio_later_flag)
                        {
                            app_ai_reocrd_db->record_audio_later_flag = false;
                        }
                    }
                }
                break;

            default:
                break;
            }

        }
        break;

    default:
        break;
    }
}

//ai record lib callback
static void app_ai_record_cb_wifi_power_on(void)
{
    //enable pin
    APP_PRINT_TRACE0("app_ai_record_cb_wifi_power_on");
    Pad_Config(PIN_WIFI_POWER,
               PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    /* PIN_G_SENSOR_POWER (P5_4) is shared with UART3_TX (wifi AT uart); driving it
     * as a GPIO corrupts the module's UART RX. G-sensor unused for now -> disabled. */
    // Pad_Config(PIN_G_SENSOR_POWER,
    //            PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_HIGH);
    app_transfer_queue_reset(CMD_PATH_UART);
}

void app_ai_record_wifi_power_on(void)
{
    app_ai_record_cb_wifi_power_on();
}

static void app_ai_record_cb_wifi_power_down(bool disable_pin)
{
    APP_PRINT_TRACE1("app_ai_record_cb_wifi_power_down: disable_pin %d", disable_pin);

    if (disable_pin)
    {
        Pad_Config(PIN_WIFI_POWER,
                   PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
        /* PIN_G_SENSOR_POWER (P5_4) == UART3_TX; leave it to uart3 (G-sensor unused). */
        // Pad_Config(PIN_G_SENSOR_POWER,
        //            PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_DOWN, PAD_OUT_ENABLE, PAD_OUT_LOW);
    }
}

static void app_ai_record_cb_report_send(uint8_t cmd_path, uint16_t event_id, uint8_t app_index,
                                         uint8_t *data, uint16_t len)
{
    if (event_id == EVENT_ACK)
    {
        APP_PRINT_TRACE1("ai_record_ack_to_app===>, cmd_id 0x%x",  data[1] << 8 | data[0]);
    }
    else
    {
        APP_PRINT_TRACE2("ai_record_tx_to_app===>, event_id 0x%04x, len 0x%x", event_id, len);
    }

    app_report_event(cmd_path, event_id, app_index, data, len);
}

#if (defined(CONFIG_WIFI_8711_CMD) || F_APP_WIFI_UART_CMD)
static void app_ai_record_cb_atcmd_send(uint16_t event_id, uint8_t *data, uint16_t len)
{
    APP_PRINT_TRACE2("ai_record_tx_to_wifi===>, cmd_id 0x%04x, len 0x%x", event_id, len);
    switch (event_id)
    {
    case EVENT_AI_RECORD_WIFI_SET_STA_MODE:
        {
            if (data[0] == AI_RECORD_WIFI_MODE_IDLE)
            {
#if defined(CONFIG_WIFI_8711_CMD) && !F_APP_WIFI_UART_CMD
                app_spi_atcmd_queue_fill(ATCMD_WLDISCONN, "\r\n");
                app_spi_atcmd_trigger_send_flow();
#elif F_APP_WIFI_UART_CMD
                app_uart_atcmd_queue_fill(ATCMD_ATWD, "\r\n");
                app_uart_atcmd_trigger_send_flow();
#endif
            }
            else if (data[0] == AI_RECORD_WIFI_MODE_STA)
            {
                struct
                {
                    uint8_t ssid_len;         // 0x06
                    uint8_t ssid[33];         // SSID buffer (33 bytes)
                    uint8_t resv1;            // 0x00
                    uint32_t security;        // 0x00400004 (Little Endian)
                    uint8_t channel;          // 0x00
                    uint8_t pwd_len;          // 0x08
                    uint8_t pwd[65];          // Password buffer (65 bytes)
                    uint8_t resv2[5];         // Reserved
                } __attribute__((packed)) *pkt = (__typeof__(pkt))&data[1];

                if (pkt->ssid_len > 32)
                {
                    break;
                }
                if (pkt->pwd_len > 64)
                {
                    break;
                }
                char ssid_str[33] = {0};
                char pwd_str[65] = {0};
                memcpy(ssid_str, pkt->ssid, pkt->ssid_len);
                ssid_str[pkt->ssid_len] = '\0';
                memcpy(pwd_str, pkt->pwd, pkt->pwd_len);
                pwd_str[pkt->pwd_len] = '\0';

                char cmd_buffer[128] = {0};

#if defined(CONFIG_WIFI_8711_CMD) && !F_APP_WIFI_UART_CMD
                snprintf(cmd_buffer, sizeof(cmd_buffer), "ssid,%s,pw,%s\r\n", ssid_str, pwd_str);
                app_spi_atcmd_queue_fill(ATCMD_WLCONN, cmd_buffer);
                app_spi_atcmd_trigger_send_flow();
#elif F_APP_WIFI_UART_CMD
                snprintf(cmd_buffer, sizeof(cmd_buffer), "%s,%s\r\n", ssid_str, pwd_str);
                app_uart_atcmd_queue_fill(ATCMD_ATPN, cmd_buffer);
                app_uart_atcmd_trigger_send_flow();
#endif
            }
        }
        break;

    default:
        break;
    }
}
#endif

static void app_ai_record_cb_ai_record_start(void)
{
    app_ai_record_mmi_handle(MMI_AI_VOICE_START);
}

static void app_ai_record_cb_ai_record_stop(void)
{
    app_ai_record_mmi_handle(MMI_AI_VOICE_STOP);
}

static void app_ai_record_cb_ai_get_next_file(uint8_t *data, uint16_t len)
{
    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
    {
        ai_record_cmd_get_next_file_resp(0);
    }
}

static void app_ai_record_cb_set_wifi_sta_mode(uint8_t *data, uint16_t len)
{
    ai_record_cmd_set_wifi_sta_mode(data, len);
}

//wifi soc notify ip info, it only wifi set sta mode, there are the event;
//byte0:status, byt1 role, byte2~byte5 ip, byte6 channel
static void app_ai_record_cb_wifi_sta_resp(uint8_t *data, uint16_t length)
{
    uint8_t resp_data[16] = {0};

    memcpy(resp_data + 2, data + 1, (sizeof(resp_data) - 2));

    resp_data[0] = data[0];
    resp_data[1] = AI_RECORD_SIDE_ROLE_SINGLE;

    {
        ai_record_cmd_set_wifi_sta_mode_resp(resp_data, length);
    }
}

static void app_ai_record_cb_set_gps_real(uint8_t *data, uint16_t len)
{
#if CONFIG_REALTEK_APP_RTC_CALENDAR_SUPPORT
    T_UTC_TIME utc_time;
    bool ret = false;
    uint32_t gps_week;
    uint32_t gps_sec;

    memcpy(&gps_week, data + 12, sizeof(gps_week));
    memcpy(&gps_sec, data + 16, sizeof(gps_sec));
    utc_time = app_ai_record_rtc_gps_to_date(gps_week, gps_sec);
    ret = app_ai_record_rtc_set_utc_time(&utc_time);
    APP_PRINT_TRACE6("app_ai_record_cb_set_gps_real %d-%d-%d-%d-%d-%d",
                     utc_time.year, utc_time.month, utc_time.day,
                     utc_time.hour, utc_time.minutes, utc_time.seconds);
    if (ret)
    {
        app_ai_record_rtc_info_save((uint8_t *)&utc_time, APP_RW_UTC_INFO_SIZE);
    }
#endif
}

static void app_ai_record_cb_set_gps(uint8_t *data, uint16_t len)
{
    if (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE)
    {
        app_ai_record_cb_set_gps_real(data, len);
    }
}

static void app_ai_record_cb_app_get_file_cnt(void)
{
    uint16_t snapshot_cnt;
    uint16_t video_cnt;

    if (app_ai_record_get_ai_conn_state() == AI_RECORD_AI_STATE_CONN)
    {
        ai_record_cmd_get_file_cnt(&snapshot_cnt, &video_cnt);
        ai_record_cmd_report_file_cnt(false, snapshot_cnt, video_cnt);
    }
}

static void app_ai_record_cb_wifi_get_file_cnt(void)
{
    uint16_t snapshot_cnt;
    uint16_t video_cnt;

    ai_record_cmd_get_file_cnt(&snapshot_cnt, &video_cnt);
    {
        if (app_ai_record_get_ai_conn_state() == AI_RECORD_AI_STATE_CONN)
        {
            ai_record_cmd_report_file_cnt(false, snapshot_cnt, video_cnt);
        }
    }
}

static void app_ai_record_cb_check_battery_state(uint8_t *p_state)
{
    uint8_t *p_battery_state = p_state;
    *p_battery_state = app_ai_record_check_battery_level();
}

static void app_ai_record_cb_ai_update_wifi(uint8_t *p_data, uint16_t length, uint8_t *results)
{
    *results = ai_record_cmd_ai_update_wifi_handle(p_data);
}

#if F_APP_DFU
static void app_ai_record_cb_wifi_soc_ota_status(uint8_t status)
{
    uint8_t app_idx = 0;
    uint8_t path = CMD_PATH_UART;
    uint8_t state = CMD_SET_STATUS_COMPLETE;

#if APP_AI_RECORD_OTA_USE_EMMC
    if (status == AI_RECORD_ERR_SUCCESS)
    {
        app_dfu_process_set(false);
        // notify 8735B

        if ((app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE) ||
            (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_PRIMARY))
        {
            app_notify_app_ota_status(EVENT_NOTIFY_APP_OTA_FINISHED, DFU_STATUS_SUCCESS);
        }

    }
#else //without emmc
    if ((status == AI_RECORD_WIFI_SOC_OTA_SUCCESS) ||
        (status == AI_RECORD_WIFI_SOC_OTA_SUCCESS_BT_OTA_IS_READY))
    {
        app_dfu_process_set(false);
        // notify 8735B
        if ((app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_SINGLE) ||
            (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_PRIMARY))
        {
            app_notify_app_ota_status(EVENT_NOTIFY_APP_OTA_FINISHED, DFU_STATUS_SUCCESS);
            if (status == AI_RECORD_WIFI_SOC_OTA_SUCCESS)
            {
                app_dfu_delay_reset();
            }
        }
    }
#endif
    else
    {
        // notify APP
        state = CMD_SET_STATUS_PROCESS_FAIL;
        app_dfu_error_handle(EVENT_NOTIFY_APP_OTA_FINISHED, DFU_STATUS_BT_START_FW_UPGRADE_ERROR);
    }
    ai_record_scenario_set_state(AI_RECORD_STATE_IDLE);
}

static void app_ai_record_dfu_start_sys_upgrade(uint8_t *ota_info, uint8_t length)
{
    ai_record_cmd_sys_upgrade_start(ota_info, length);
    ai_record_scenario_set_state(AI_RECORD_STATE_OTA);
}
#endif



#if (F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD)

/* Uses T_AT_EVT_TYPE from the WiFi AT header (app_spi_atcmd.h / app_uart_atcmd.h),
 * which is only included when a WiFi transport is enabled, and is only registered
 * in that same case below. Guard the definition so no-transport builds compile. */
static void app_ai_record_wifi_atevt_handle(T_AT_EVT_TYPE evt, void *p_data, uint16_t len)
{
    ai_cmd_wifi_atevt_handle(evt, (uint8_t *)p_data, len);
}

void app_ai_record_cb_init(void)
{
    //ai record callback handle
    //ai_record_register_cb(AI_RECORD_CB_IDX_WIFI_POWER_ON_HANDLE, app_ai_record_cb_wifi_power_on);
    //ai_record_register_cb(AI_RECORD_CB_IDX_WIFI_POWER_DOWN_HANDLE, (AI_RECORD_FUNC_CB)app_ai_record_cb_wifi_power_down);
    ai_record_register_cb(AI_RECORD_CB_IDX_REPORT_SEND,
                          (AI_RECORD_FUNC_CB)app_ai_record_cb_report_send);
    wifi_transport_register_callback((wifi_at_evt_cb_t)app_ai_record_wifi_atevt_handle);
    ai_record_register_cb(AI_RECORD_CB_IDX_ATCMD_SEND, (AI_RECORD_FUNC_CB)app_ai_record_cb_atcmd_send);
    ai_record_register_cb(AI_RECORD_CB_IDX_AI_RECORDING_START, app_ai_record_cb_ai_record_start);
    ai_record_register_cb(AI_RECORD_CB_IDX_AI_RECORDING_STOP, app_ai_record_cb_ai_record_stop);

    ai_record_register_cb(AI_RECORD_CB_IDX_SET_WIFI_STA_MODE,
                          (AI_RECORD_FUNC_CB)app_ai_record_cb_set_wifi_sta_mode);
    ai_record_register_cb(AI_RECORD_CB_IDX_WIFI_STA_RESP,
                          (AI_RECORD_FUNC_CB)app_ai_record_cb_wifi_sta_resp);
    ai_record_register_cb(AI_RECORD_CB_IDX_SET_GPS, (AI_RECORD_FUNC_CB)app_ai_record_cb_set_gps);
    ai_record_register_cb(AI_RECORD_CB_IDX_APP_GET_FILE_CNT,
                          (AI_RECORD_FUNC_CB)app_ai_record_cb_app_get_file_cnt);

#if F_APP_DFU
    // ota
    ai_record_register_cb(AI_RECORD_CB_IDX_WIFI_SOC_OTA_STATUS,
                          (AI_RECORD_FUNC_CB)app_ai_record_cb_wifi_soc_ota_status);
#endif
}

#endif /* F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD */

static void app_ai_record_set_side_role(void)
{
    uint8_t side_role = AI_RECORD_SIDE_ROLE_SINGLE;

    if (app_cfg_const.bud_role == REMOTE_SESSION_ROLE_SINGLE)
    {
        side_role = AI_RECORD_SIDE_ROLE_SINGLE;
    }

    ai_record_side_role_set(side_role);
}

/**
 * @brief Central dispatcher for record-trans service events.
 *
 *  This is the single entry point all upstream events from the GATT
 *  layer flow through. It owns the routing policy: today every WRITE
 *  goes to file_trans (the only consumer); future modules can branch
 *  here on cmd_id without touching service.c.
 *
 *  WRITE event:
 *    p_data->msg_data.write.{p_value, length} carries the raw payload.
 *    Pointer is valid only within this call - file_trans copies what
 *    it needs internally before returning.
 *
 *  CCCD_UPDATE event:
 *    p_data->msg_data.notification_index identifies which notify char
 *    the CCCD belongs to (record-trans only has one).
 */
static T_APP_RESULT app_ai_record_service_cb(uint8_t type, void *p_data)
{
    if (p_data == NULL)
    {
        return APP_RESULT_APP_ERR;
    }

    T_RECORD_TRANS_CALLBACK_DATA *d = (T_RECORD_TRANS_CALLBACK_DATA *)p_data;

    switch (type)
    {
    case GATT_MSG_RECORD_TRANS_SERVER_WRITE:
        /* Forward raw payload + link info to file_trans. Future hook:
         * peek at cmd_id (d->msg_data.write.p_value[0..1]) here to
         * route different cmd_id ranges to different modules. */
        return app_ai_record_file_handle_cp_req(d->conn_id, d->conn_handle,
                                                d->cid, d->chann_type,
                                                d->msg_data.write.length,
                                                d->msg_data.write.p_value);

    case GATT_MSG_RECORD_TRANS_SERVER_CCCD_UPDATE:
        /* Service-layer log already prints whether notify is enabled;
         * we just propagate the link info. The `true` here mirrors the
         * historical assumption - refine to the actual cccd_bits if we
         * ever add a real notify_enabled field to the union. */
        app_ai_record_file_trans_on_cccd(d->conn_id, d->conn_handle,
                                         d->cid, d->chann_type,
                                         true);
        return APP_RESULT_SUCCESS;

    default:
        APP_PRINT_WARN1("app_ai_record_service_cb: unknown type 0x%02x", type);
        return APP_RESULT_APP_ERR;
    }
}

void app_ai_record_init(void)
{
    app_ai_reocrd_db = calloc(1, sizeof(T_APP_AI_RECORD_DB));

    sys_mgr_cback_register(app_ai_record_sysm_cback);
    app_ipc_subscribe(APP_DEVICE_IPC_TOPIC, app_ai_record_device_event_cback);
    bt_mgr_cback_register(app_ai_record_audio_bt_cback);
    audio_mgr_cback_register(app_ai_record_audio_cback);

    ai_record_init();

    app_ai_record_set_side_role();

    // app_ai_record_cb_wifi_power_down(true);

#if (F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD)
    app_ai_record_cb_init();
#endif
#if F_APP_DFU
    //ota
    app_dfu_cback_register(ai_record_dfu_fail_handle, DFU_ERROR);
    app_dfu_cback_register(ai_record_dfu_finish_handle, DFU_FINISH);
    app_dfu_start_sys_upgrade_cback_register(app_ai_record_dfu_start_sys_upgrade);
#endif

    app_ai_record_process_init();

#if CONFIG_REALTEK_APP_RTC_CALENDAR_SUPPORT
    app_ai_record_rtc_calendar_init();
#endif

    /* BLE file-transfer service: register GATT service, then init the
     * application-layer state machine. SDP record (BR/EDR over GATT)
     * needs the GATT handle range to already be assigned, so it goes
     * after record_trans_reg_srv(). */
    app_ai_record_file_trans_init();
    record_trans_reg_srv(app_ai_record_service_cb);
#if CONFIG_RECORD_TRANS_GATT_OVER_BREDR
    record_trans_sdp_register();
#endif
}

#endif //F_APP_AI_AI_RECORD_SUPPORT
