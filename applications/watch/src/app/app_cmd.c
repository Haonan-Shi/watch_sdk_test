/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "trace.h"
#include "app_cfg.h"
#include "console.h"
#include "app_timer.h"
#include "os_mem.h"
#include "os_sched.h"
#include "gap_br.h"
#include "app_cmd.h"
#include "app_main.h"
#include "app_audio_policy.h"
#include "app_transfer.h"
#include "app_report.h"
#include "app_ble_gap.h"
#if F_APP_CLI_BINARY_MP_SUPPORT
#include "mp_test.h"
#endif
#include "app_mmi.h"
#include "app_cfg.h"
#include "bt_hfp.h"
#include "bt_iap.h"
#include "tts.h"
#include "btm.h"
#include "bt_bond.h"
#include "remote.h"
#include "voice_prompt.h"
#include "audio_probe.h"
#include "patch_header_check.h"
#include "fmc_api.h"
#include "test_mode.h"
#include "rtl876x_pinmux.h"
#include "system_status_api.h"
#include "app_task.h"
#include "audio_record.h"
#include "auto_k_rf.h"
#include "audio_track.h"
#include "app_dlps.h"
#include "app_hfp.h"
#include "app_bt_policy_int.h"
#include "app_bt_policy_api.h"
#include "pm.h"
#include "audio.h"
#include "app_playback_update_file.h"
#include "app_gap.h"
#include "os_timer.h"
#include "app_audio_if.h"
#include "app_audio_route.h"
#include "img_ctrl_ext.h"
#if CONFIG_DFU_NORMAL_OTA
#include "dfu_common.h"
#endif
#include "dfu_transport.h"
#if CONFIG_REALTEK_APP_AUDIO_DATA_CAPTURE
#include "audio_data_capture.h"
#endif
#if CONFIG_REALTEK_APP_DSP_ONLINE_DEBUG
#include "audio_cmd.h"
#endif
/****************************************************
platfom api from system_status_api, follow RTL87x3E
*****************************************************/
//for CMD_TTS
#define TTS_TYPE_START                  0x00
#define TTS_TYPE_SEND_DATA              0x01

#define TTS_DATA_SINGLE                 0x00
#define TTS_DATA_START                  0x01
#define TTS_DATA_CONTINUE               0x02
#define TTS_DATA_END                    0x03

#define TTS_HEADER_LEN                  0x000A
#define TTS_SYNC_BYTE                   0xB8
#define TTS_INIT_SEQ                    0x00
#define TTS_FRAME_LEN                   0x02

//for CMD_SET_CFG
#define CFG_SET_LE_NAME                 0x00
#define CFG_SET_LEGACY_NAME             0x01
#define CFG_SET_AUDIO_LATENCY           0x02
#define CFG_SET_SUPPORT_CODEC           0x03
#define CFG_SET_DURIAN_ID               0x04
#define CFG_SET_DURIAN_SINGLE_ID        0x05
#define CFG_SET_HFP_REPORT_BATT         0x06

//for CMD_GET_CFG_SETTING
#define CFG_GET_LE_NAME                 0x00
#define CFG_GET_LEGACY_NAME             0x01
#define GET_GET_IC_NAME                 0x02
#define CFG_GET_MAX                     0x03

// for get FW version type
#define GET_PRIMARY_FW_VERSION          0x00
#define GET_SECONDARY_FW_VERSION        0x01
#define GET_PRIMARY_OTA_FW_VERSION      0x02
#define GET_SECONDARY_OTA_FW_VERSION    0x03


//for CMD_LINE_IN_CTRL
#define CFG_LINE_IN_STOP                0x00
#define CFG_LINE_IN_START               0x01

//for CMD_AUDIO_DSP_CTRL_SEND
#define CFG_H2D_DAC_GAIN                0x0C
#define CFG_H2D_VOICE_ADC_POST_GAIN     0x5D
#define CFG_H2D_APT_DAC_GAIN            0x4C
/* RTL87x3C specialized feature */
//for CMD_DSP_DEBUG_SIGNAL_IN
#define CFG_SET_DSP_DEBUG_SIGNAL_IN     0x71
#define DSP_DEBUG_SIGNAL_IN_PAYLOAD_LEN 16

//for CMD_INFO_REQ
#define CMD_INFO_STATUS_VALID       0x00
#define CMD_INFO_STATUS_ERROR       0x01
#define CMD_SUPPORT_VER_CHECK_LEN   3


//for CMD_HA_SET_DSP_PARAM
#define CFG_SET_DSP_PARAM               0x70
#define HA_SET_DSP_PARAM_PAYLOAD_LEN    16
// end of RTL87x3C specialized feature
#define BOTH_SIDE_ADJUST                0x02

#define SET_CALIBRATION_NOISE_FLOOR           1
#define REPORT_CALIBRATION_NOISE_FLOOR        1
#define SET_IN_EAR_THRESHOLD                  2
#define REPORT_CALIBRATION_IN_EAR_THRESHOLD   2
#define SET_OUT_EAR_THRESHOLD                 3
#define REPORT_CALIBRATION_OUT_EAR_THRESHOLD  3
#define GET_PX318J_PARA                       4
#define REPORT_PX318J_PARA                    4

#if (F_APP_AUTO_SUPPORT == 1)
#if C_APP_DEVICE_CMD_SUPPORT
//for CMD_INQUIRY
#define NORMAL_INQUIRY                          0x00
#define PERIODIC_INQUIRY                        0x01
#define MAX_INQUIRY_TIME                        0x30
#endif

#define CLEAR_BOND_INFO_SUCCESS     0x00
#define CLEAR_BOND_INFO_FAIL        0x01

extern bool bt_hfp_dial_last_number_req(uint8_t *bd_addr);
extern T_APP_BOND_DEVICE *app_find_br_addr(uint8_t *bd_addr);
extern T_APP_BOND_DEVICE *app_unused_br_addr(uint8_t *bd_addr);
#endif

// for check audio dsp scenario
typedef enum
{
    DSP_SCENARIO_CHECK_SAMPLE_RATE = 0x0000,
} T_CMD_AUDIO_DSP_SCENARIO_CHECK_TYPE;

typedef enum
{
    DSP_SCENARIO_CHECK_EXISTENT     = 0x0000,
    DSP_SCENARIO_CHECK_NON_EXISTENT = 0x0001,
} T_CMD_AUDIO_DSP_SCENARIO_CHECK_STATUS;

typedef enum
{
    DSP_TOOL_OPERATION_DAC_GAIN,
    DSP_TOOL_OPERATION_ADC_POST_GAIN,
    DSP_TOOL_OPERATION_APT_DAC_GAIN,
} T_DSP_TOOL_OPERATION_GAIN_TYPE;


T_SNK_CAPABILITY app_cmd_get_system_capability(void)
{
    T_SNK_CAPABILITY snk_capability;
    memset(&snk_capability, 0, sizeof(T_SNK_CAPABILITY));
    snk_capability.snk_support_local_playback = 1;
#if CONFIG_REALTEK_APP_AUDIO_DATA_CAPTURE
    snk_capability.snk_support_data_capture = 1;
    snk_capability.snk_support_3bin_scenario = 1;
#endif
    return snk_capability;
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

    APP_PRINT_TRACE4("==>app_handle_cmd_set: cmd_id 0x%04x, cmd_len 0x%04x, cmd_path %u, rx_seqn 0x%02x",
                     cmd_id, cmd_len, cmd_path, rx_seqn);
    APP_PRINT_INFO4("cmd_ptr[0] = 0x%02x, cmd_ptr[1] = 0x%02x, cmd_ptr[2] = 0x%02x, cmd_ptr[3] = 0x%02x",
                    cmd_ptr[0], cmd_ptr[1], cmd_ptr[2], cmd_ptr[3]);

    /* check duplicated seq num */
    if (cmd_id != CMD_ACK && rx_seqn != 0)
    {
        if (cmd_path == CMD_PATH_LE)
        {
            if (app_db.le_link[app_idx].rx_cmd_seqn == rx_seqn)
            {
                app_report_event(CMD_PATH_LE, EVENT_ACK, app_idx, &ack_pkt[0], 3);
                return;
            }
            app_db.le_link[app_idx].rx_cmd_seqn = rx_seqn;
        }
        else if (cmd_path == CMD_PATH_SPP)
        {
            if (app_db.br_link[app_idx].rx_cmd_seqn == rx_seqn)
            {
                app_report_event(CMD_PATH_SPP, EVENT_ACK, app_idx, &ack_pkt[0], 3);
                return;
            }
            app_db.br_link[app_idx].rx_cmd_seqn = rx_seqn;
        }
        else if (cmd_path == CMD_PATH_IAP)
        {
            if (app_db.br_link[app_idx].rx_cmd_seqn == rx_seqn)
            {
                app_report_event(CMD_PATH_IAP, EVENT_ACK, app_idx, &ack_pkt[0], 3);
                return;
            }
            app_db.br_link[app_idx].rx_cmd_seqn = rx_seqn;
        }
    }

    if (cmd_path == CMD_PATH_SPP)
    {
        app_db.br_link[app_idx].cmd_set_enable = true;
    }
    else if (cmd_path == CMD_PATH_LE)
    {
        if (app_db.le_link[app_idx].state == LE_LINK_STATE_CONNECTED)
        {
            app_db.le_link[app_idx].cmd_set_enable = true;
        }
    }

    switch (cmd_id)
    {
    default:
        ack_pkt[2] = CMD_SET_STATUS_UNKNOW_CMD;
        app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        break;
    case CMD_ACK:
        {
            uint16_t event_id = (uint16_t)(cmd_ptr[2] | (cmd_ptr[3] << 8));
            uint8_t status = cmd_ptr[4];

            if (cmd_path == CMD_PATH_UART)
            {
                app_pop_data_transfer_queue(CMD_PATH_UART, true);
            }
            else if ((cmd_path == CMD_PATH_LE) || (cmd_path == CMD_PATH_SPP) || (cmd_path == CMD_PATH_IAP))
            {
//                if (!app_cfg_const.enable_dsp_capture_data_by_spp)
                {
                    app_transfer_queue_recv_ack_check(event_id, cmd_path);
                }
            }

            if ((event_id == EVENT_PLAYBACK_GET_LIST_DATA) && (status == CMD_SET_STATUS_COMPLETE))
            {
                if (cmd_path == CMD_PATH_SPP || cmd_path == CMD_PATH_IAP)
                {
                    app_playback_trans_list_data_handle();
                }
            }
            if (event_id == EVENT_OTA_ACTIVE_ACK)
            {
                if (cmd_path == CMD_PATH_SPP)
                {
                    app_ota_cmd_ack_handle(event_id, status);
                }
            }
        }
        break;
    case CMD_INFO_REQ:
        {
            uint8_t info_type = cmd_ptr[2];
            uint8_t report_to_phone_len = 6;
            uint8_t buf[report_to_phone_len];

            if (info_type == CMD_SET_INFO_TYPE_VERSION)
            {
                app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

                buf[0] = info_type;
                buf[1] = CMD_INFO_STATUS_VALID;

                buf[2] = CMD_SET_VER_MAJOR;
                buf[3] = CMD_SET_VER_MINOR;
                buf[4] = EQ_SPEC_VER_MAJOR;
                buf[5] = EQ_SPEC_VER_MINOR_0;

                if (report_to_phone_len > 0)
                {
                    app_report_event(cmd_path, EVENT_INFO_RSP, app_idx, buf, report_to_phone_len);
                }
            }
            else if (info_type == CMD_INFO_GET_CAPABILITY)
            {
                T_SNK_CAPABILITY current_snk_cap;
                uint8_t evt_param[11];
                evt_param[0] = info_type;
                evt_param[1] = CMD_INFO_STATUS_VALID;
                current_snk_cap = app_cmd_get_system_capability();
                memcpy(&evt_param[2], (uint8_t *)&current_snk_cap, sizeof(T_SNK_CAPABILITY));
                app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
                app_report_event(cmd_path, EVENT_INFO_RSP, app_idx, evt_param, sizeof(evt_param));
            }
            else
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
                app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            }
        }
        break;
    case CMD_GET_STATUS:
        {
            uint8_t buf[3];
            uint8_t report_len = 2;
            buf[0] = cmd_ptr[2]; //status_index
            APP_PRINT_TRACE1("==>app_handle_cmd_set: get status index 0x%02x", cmd_ptr[2]);
            switch (cmd_ptr[2])
            {
            case GET_STATUS_BATTERY_STATUS:
                {
                    buf[1] = app_db.local_batt_level;
                    buf[2] = app_db.remote_batt_level;
                    report_len = 3;
                }
                break;
            case GET_STATUS_VOLUME:
                {
                    T_AUDIO_STREAM_TYPE volume_type;

                    if (app_hfp_get_call_status() != APP_HFP_CALL_IDLE)
                    {
                        volume_type = AUDIO_STREAM_TYPE_VOICE;
                    }
                    else
                    {
                        volume_type = AUDIO_STREAM_TYPE_PLAYBACK;
                    }

                    buf[1] = app_audio_get_volume();
                    buf[2] = audio_volume_out_max_get(volume_type);
                    report_len = 3;
                }
                break;
            default:
                break;
            }
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            app_report_event(cmd_path, EVENT_REPORT_STATUS, app_idx, buf, report_len);
        }
        break;
    case CMD_GET_IMAGE_INFO:
        {
            uint8_t image_feature_info[IMG_FEATURE_STR_LEN] = {0};
            uint16_t image_id;
            LE_ARRAY_TO_UINT16(image_id, &cmd_ptr[2])

            if ((image_id < IMG_OTA) || (image_id >= IMAGE_MAX))
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }
            else
            {
                if (get_image_feature_info((IMG_ID)image_id, image_feature_info, IMG_FEATURE_STR_LEN) != 0)
                {
                    ack_pkt[2] = CMD_SET_STATUS_DISALLOW;
                }
            }

            uint8_t evt_buf[IMG_FEATURE_STR_LEN + 2];
            memcpy(&evt_buf[0], &cmd_ptr[2], 2);
            memcpy(&evt_buf[2], image_feature_info, IMG_FEATURE_STR_LEN);
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            app_report_event(cmd_path, EVENT_REPORT_IMAGE_INFO, app_idx, evt_buf,
                             (IMG_FEATURE_STR_LEN + 2));
        }
        break;
    case CMD_AUDIO_DSP_SCENARIO_CHECK:
        {
            uint8_t category = cmd_ptr[2];
            uint8_t type = cmd_ptr[3];

            switch (type)
            {
            case DSP_SCENARIO_CHECK_SAMPLE_RATE:
                {
                    uint8_t report[9];
                    uint32_t sample_rate = 0;
                    uint16_t payload_len = sizeof(sample_rate);

                    report[0] = DSP_SCENARIO_CHECK_NON_EXISTENT;

                    switch (category)
                    {
                    case AUDIO_CATEGORY_AUDIO:
                    case AUDIO_CATEGORY_VOICE:
                    case AUDIO_CATEGORY_RECORD:
                        {
                            T_AUDIO_FORMAT_INFO format_info;
                            T_AUDIO_TRACK_HANDLE handle;
                            T_AUDIO_TRACK_STATE state = AUDIO_TRACK_STATE_RELEASED;
                            T_APP_BR_LINK *p_link;

                            if (category == AUDIO_CATEGORY_AUDIO)
                            {
                                p_link = &app_db.br_link[app_a2dp_get_active_idx()];
                                handle = p_link->a2dp_track_handle;
                            }
                            else if (category == AUDIO_CATEGORY_VOICE)
                            {
                                p_link = &app_db.br_link[app_hfp_get_active_hf_index()];
                                handle = p_link->sco_track_handle;
                            }
                            else // category record/line in/apt/etc.
                            {
                                //TODO: we need to save other category audio track handle
                                //handle = (T_AUDIO_TRACK_HANDLE)app_audio_track_handle_get(AUDIO_STREAM_TYPE_RECORD);
                                handle = NULL;
                            }

                            if (handle != NULL)
                            {
                                audio_track_format_info_get(handle, &format_info);
                                audio_track_state_get(handle, (T_AUDIO_TRACK_STATE *)&state);

                                if (state == AUDIO_TRACK_STATE_STARTED)
                                {
                                    switch (format_info.type)
                                    {
                                    case AUDIO_FORMAT_TYPE_PCM:
                                        {
                                            sample_rate = format_info.attr.pcm.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_CVSD:
                                        {
                                            sample_rate = format_info.attr.cvsd.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_MSBC:
                                        {
                                            sample_rate = format_info.attr.msbc.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_SBC:
                                        {
                                            sample_rate = format_info.attr.sbc.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_AAC:
                                        {
                                            sample_rate = format_info.attr.aac.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_OPUS:
                                        {
                                            sample_rate = format_info.attr.opus.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_FLAC:
                                        {
                                            sample_rate = format_info.attr.flac.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_MP3:
                                        {
                                            sample_rate = format_info.attr.mp3.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_LC3:
                                        {
                                            sample_rate = format_info.attr.lc3.sample_rate;
                                        }
                                        break;

                                    case AUDIO_FORMAT_TYPE_LDAC:
                                        {
                                            sample_rate = format_info.attr.ldac.sample_rate;
                                        }
                                        break;

                                    default:
                                        break;
                                    }

                                    report[0] = DSP_SCENARIO_CHECK_EXISTENT;
                                }
                            }
                        }
                        break;
                    default:
                        break;
                    }

                    report[1] = category;
                    report[2] = type;

                    app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

                    LE_UINT16_TO_ARRAY(&report[3], payload_len);
                    LE_UINT32_TO_ARRAY(&report[5], sample_rate);

                    app_report_event(cmd_path, EVENT_AUDIO_DSP_SCENARIO_INFO, app_idx, report, sizeof(report));
                }
                break;

            default:
                {
                    ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
                    app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
                }
                break;
            }
        }
        break;
    case CMD_DSP_TOOL_OPERATION:
        {
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

            uint8_t event_data[3];
            uint16_t function_type;

            LE_ARRAY_TO_UINT16(function_type, &cmd_ptr[2]);

            memcpy(event_data, &cmd_ptr[2], 2);
            event_data[2] = CMD_SET_STATUS_COMPLETE;

            switch (function_type)
            {
            case DSP_TOOL_OPCODE_BRIGHTNESS:
                {
                }
                break;

            case DSP_TOOL_OPCODE_HW_EQ:
                {
                    uint8_t eq_type = cmd_ptr[4];
                    uint8_t eq_channel = cmd_ptr[5];
                    uint16_t eq_len;

                    LE_ARRAY_TO_UINT16(eq_len, &cmd_ptr[10]);

                    audio_route_hw_eq_apply((T_AUDIO_ROUTE_HW_EQ_TYPE)eq_type, eq_channel, &cmd_ptr[12], eq_len);
                }
                break;

            case DSP_TOOL_OPCODE_GAIN:
                {
                    T_AUDIO_STREAM_TYPE stream_type;
                    T_AUDIO_CATEGORY category = (T_AUDIO_CATEGORY)cmd_ptr[4];
                    uint8_t gain_type = cmd_ptr[5];
                    uint8_t gain_level = cmd_ptr[6];
                    int16_t gain = (cmd_ptr[7] | cmd_ptr[8] << 8);

                    switch (category)
                    {
                    case AUDIO_CATEGORY_TONE:
                    case AUDIO_CATEGORY_VP:
                    case AUDIO_CATEGORY_AUDIO:
                        {
                            stream_type = AUDIO_STREAM_TYPE_PLAYBACK;
                        }
                        break;

                    case AUDIO_CATEGORY_APT:
                    case AUDIO_CATEGORY_LLAPT:
                    case AUDIO_CATEGORY_ANC:
                    case AUDIO_CATEGORY_LINE:
                    case AUDIO_CATEGORY_VOICE:
                        {
                            stream_type = AUDIO_STREAM_TYPE_VOICE;
                        }
                        break;
                    case AUDIO_CATEGORY_RECORD:
                        {
                            stream_type = AUDIO_STREAM_TYPE_RECORD;
                        }
                        break;

                    default:
                        {
                            stream_type = AUDIO_STREAM_TYPE_PLAYBACK;
                        }
                        break;
                    }

                    switch (gain_type)
                    {
                    case DSP_TOOL_OPERATION_DAC_GAIN:
                        {
                            app_audio_route_dac_gain_set(category, gain_level, gain);
                            audio_volume_out_set(stream_type, gain_level);
                        }
                        break;

                    case DSP_TOOL_OPERATION_APT_DAC_GAIN:
                        {
                            app_audio_route_dac_gain_set(category, gain_level, gain);
                        }
                        break;

                    case DSP_TOOL_OPERATION_ADC_POST_GAIN:
                        {
                            app_audio_route_adc_gain_set(category, gain_level, gain);

                            if ((category != AUDIO_CATEGORY_APT) && (category != AUDIO_CATEGORY_LLAPT))
                            {
                                audio_volume_in_set(stream_type, gain_level);
                            }
                        }
                        break;

                    default:
                        break;
                    }
                }
                break;

            case DSP_TOOL_OPCODE_SW_EQ:
                {
                    APP_PRINT_TRACE0("not support sw eq opration!");
                }
                break;

            default:
                {
                    event_data[2] = CMD_SET_STATUS_UNKNOW_CMD;
                }
                break;
            }

            app_report_event(cmd_path, EVENT_DSP_TOOL_OPERATION, app_idx, event_data,
                             sizeof(event_data));
        }
        break;
#if (F_APP_AUTO_SUPPORT == 1)
#if C_APP_DEVICE_CMD_SUPPORT
    case CMD_INQUIRY:
        {
            if ((cmd_ptr[2] == START_INQUIRY) && (cmd_ptr[3] <= MAX_INQUIRY_TIME))
            {
                if (cmd_ptr[4] == NORMAL_INQUIRY)
                {
                    if (gap_br_start_inquiry(false, cmd_ptr[3]) != GAP_CAUSE_SUCCESS)
                    {
                        ack_pkt[2] = CMD_SET_STATUS_PROCESS_FAIL;
                    }
                }
                else
                {
                    if (get_search_status() != SEARCH_START)
                    {
                        T_IO_MSG inquiry_msg;
                        inquiry_msg.type = IO_MSG_TYPE_WRISTBNAD;
                        inquiry_msg.subtype = IO_MSG_INQUIRY_START;
                        app_send_msg_to_apptask(&inquiry_msg);
                        app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
                    }
                    else
                    {
                        ack_pkt[2] = CMD_SET_STATUS_PROCESS_FAIL;
                    }
                }
            }
            else if (cmd_ptr[2] == STOP_INQUIRY)
            {
                if (cmd_ptr[4] == NORMAL_INQUIRY)
                {
                    if (gap_br_stop_inquiry() != GAP_CAUSE_SUCCESS)
                    {
                        ack_pkt[2] = CMD_SET_STATUS_PROCESS_FAIL;
                    }
                }
                else
                {
                    T_IO_MSG dis_inquiry_msg;
                    dis_inquiry_msg.type = IO_MSG_TYPE_WRISTBNAD;
                    dis_inquiry_msg.subtype = IO_MSG_INQUIRY_STOP;
                    app_send_msg_to_apptask(&dis_inquiry_msg);

                    app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
                }
            }
            else
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }
        }
        break;
#endif
    case CMD_BT_CREATE_CONNECTION:
        {
            if (cmd_ptr[2] == 1)
            {
                T_APP_BOND_DEVICE *temp_search = app_bt_bond_get_temp_cache_device_by_index(0);
                if (temp_search)
                {
                    if (app_bt_bond_check_active_device_info_by_addr(temp_search->bd_addr))
                    {
                        APP_PRINT_INFO0("connect active device error");
                    }
                    else
                    {
                        {
                            app_bt_bond_temp_cache_save_to_search();
                            T_IO_MSG dis_inquiry_msg;
                            dis_inquiry_msg.type = IO_MSG_TYPE_WRISTBNAD;
                            dis_inquiry_msg.subtype = IO_MSG_INQUIRY_STOP;
                            app_send_msg_to_apptask(&dis_inquiry_msg);
                            DBG_DIRECT("Connect start");
                            T_IO_MSG con_msg;
                            con_msg.type = IO_MSG_TYPE_WRISTBNAD;
                            con_msg.subtype = IO_MSG_CONNECT_BREDR_DEVICE;
                            con_msg.u.buf = temp_search->bd_addr;
                            app_send_msg_to_apptask(&con_msg);

                            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
                        }
                    }
                }
                else
                {
                    APP_PRINT_INFO0("get temp_search error");
                }
            }
            else if (cmd_ptr[2] == 0)
            {
                uint8_t *reconn_addr = (uint8_t *)(&cmd_ptr[3]);
                for (uint8_t y = 0; y < 3; y++)
                {
                    uint8_t temp;
                    temp = reconn_addr[y];
                    reconn_addr[y] = reconn_addr[5 - y];
                    reconn_addr[5 - y] = temp;
                }

                DBG_DIRECT("Reconnect start");
                T_IO_MSG con_msg;
                con_msg.type = IO_MSG_TYPE_WRISTBNAD;
                con_msg.subtype = IO_MSG_CONNECT_BREDR_DEVICE;
                con_msg.u.buf = reconn_addr;
                app_send_msg_to_apptask(&con_msg);

                app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            }
        }
        break;
    case CMD_BT_DISCONNECT:
        {
            T_APP_BOND_DEVICE *temp_search;

            uint8_t *disconn_addr = (uint8_t *)(&cmd_ptr[3]);
            temp_search = app_find_br_addr(disconn_addr);

            if (temp_search != NULL)
            {
                app_bt_policy_disconnect(temp_search->bd_addr, cmd_ptr[2]);
            }
            else
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    case CMD_BT_BOND_INFO_CLEAR:
        {
            uint8_t *clear_addr = (uint8_t *)(&cmd_ptr[2]);
            for (uint8_t y = 0; y < 3; y++)
            {
                uint8_t temp;
                temp = clear_addr[y];
                clear_addr[y] = clear_addr[5 - y];
                clear_addr[5 - y] = temp;
            }
            uint8_t temp_buff = CLEAR_BOND_INFO_FAIL;

            if (app_find_br_link(clear_addr) == NULL)
            {
                APP_PRINT_INFO0("delete start");
                if (bt_bond_delete(clear_addr))
                {
                    APP_PRINT_INFO0("bt_bond_delete success");
                    temp_buff = CLEAR_BOND_INFO_SUCCESS;
                }
                app_bt_bond_del_bond_device(clear_addr);
            }
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            app_report_event(CMD_PATH_UART, EVENT_BT_BOND_INFO_CLEAR, 0, &temp_buff, sizeof(temp_buff));
        }
        break;
    case CMD_MMI:
        {
            T_IO_MSG mmi_msg;
            mmi_msg.type = IO_MSG_TYPE_WRISTBNAD;
            mmi_msg.subtype = IO_MSG_MMI;
            mmi_msg.u.param = cmd_ptr[2];
            app_send_msg_to_apptask(&mmi_msg);

            app_report_event(CMD_PATH_UART, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    case CMD_WDG_RESET:
        {
            uint8_t wdg_status = 0x00;

            app_report_event(CMD_PATH_UART, EVENT_ACK, app_idx, ack_pkt, 3);
            APP_PRINT_INFO1("ack_pkt info: ack_pkt %s", TRACE_BDADDR(ack_pkt));
            app_report_event(cmd_path, EVENT_WDG_RESET, app_idx, &wdg_status, 1);

            os_delay(20);
            chip_reset(RESET_ALL);
        }
        break;
    case CMD_BT_HFP_DIAL_WITH_NUMBER:
        {
            uint8_t app_index = app_hfp_get_active_hf_index();

            if ((app_db.br_link[app_index].app_hf_state == APP_HF_STATE_CONNECTED) &&
                (app_hfp_get_call_status() == APP_HFP_CALL_IDLE))
            {
                uint8_t callnumber_len = cmd_len - 2;
                static uint8_t dial_num[21];
                for (uint8_t i = 0; i < callnumber_len; i++)
                {
                    dial_num[i] = 0x30 + cmd_ptr[i + 2];
                }
                dial_num[callnumber_len] = 0;
                bt_hfp_dial_with_number_req(app_db.br_link[app_index].bd_addr, (char *)dial_num);
            }
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    case CMD_HONEYGUI_BENCHMARK_START:
        {
            //gui_update_by_event(GUI_EVENT_BENCHMARK_START, NULL, true);
        }
        app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        break;
#endif
    case CMD_START_RECORD:
        {
            T_IO_MSG record_msg;
            record_msg.type = IO_MSG_TYPE_WRISTBNAD;
            record_msg.subtype = IO_MSG_MMI;
            record_msg.u.param = MMI_RECORD_START;
            app_send_msg_to_apptask(&record_msg);

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    case CMD_STOP_RECORD:
        {
            T_IO_MSG record_msg;
            record_msg.type = IO_MSG_TYPE_WRISTBNAD;
            record_msg.subtype = IO_MSG_MMI;
            record_msg.u.param = MMI_RECORD_STOP;
            app_send_msg_to_apptask(&record_msg);

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    case CMD_START_RECORD_PLAY:
        {
            T_IO_MSG record_msg;
            record_msg.type = IO_MSG_TYPE_WRISTBNAD;
            record_msg.subtype = IO_MSG_MMI;
            record_msg.u.param = MMI_RECORD_PLAY_START;
            app_send_msg_to_apptask(&record_msg);

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    case CMD_STOP_RECORD_PLAY:
        {
            T_IO_MSG record_msg;
            record_msg.type = IO_MSG_TYPE_WRISTBNAD;
            record_msg.subtype = IO_MSG_MMI;
            record_msg.u.param = MMI_RECORD_PLAY_STOP;
            app_send_msg_to_apptask(&record_msg);

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
        }
        break;
    case CMD_RECORD_PLAY_DATA:
        {
            audio_record_play_data_write(cmd_ptr + 2, cmd_len - 2);
        }
        break;
    case CMD_RF_XTAK_K:
        {
            uint8_t xtal_k_result[2] = {0};
            uint8_t rf_channel = cmd_ptr[2];
            uint8_t freq_upperbound = cmd_ptr[3];
            uint8_t freq_lowerbound = cmd_ptr[4];
            uint8_t measure_offset = cmd_ptr[5];
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            /* start rf xtal K */
            hci_cmd_vendor_auto_k_rf(rf_channel, freq_upperbound, freq_lowerbound, measure_offset);

            /* get xtal value */
            get_xtal_cap_value(xtal_k_result);
            app_report_event(cmd_path, EVENT_RF_XTAL_K, app_idx, xtal_k_result, 2);
            APP_PRINT_INFO0("CMD_RF_XTAK_K");
        }
        break;

    case CMD_RF_XTAL_K_GET_RESULT:
        {
            uint8_t xtal_k_result[2] = {0};

            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

            /* get xtal value */
            get_xtal_cap_value(xtal_k_result);

            APP_PRINT_TRACE2("CMD_RF_XTAL_K_GET_RESULT: %02x %02x",
                             xtal_k_result[0], xtal_k_result[1]);

            app_report_event(cmd_path, EVENT_RF_XTAL_K_GET_RESULT, app_idx, xtal_k_result,
                             sizeof(xtal_k_result));
        }
        break;
    /* RTL87x3C specialized feature */
    case CMD_PLAYBACK_QUERY_INFO:
    case CMD_PLAYBACK_GET_LIST_DATA:
    case CMD_PLAYBACK_TRANS_START:
    case CMD_PLAYBACK_TRANS_CONTINUE:
    case CMD_PLAYBACK_REPORT_BUFFER_CHECK:
    case CMD_PLAYBACK_VALID_SONG:
    case CMD_PLAYBACK_TRIGGER_ROLE_SWAP:
    case CMD_PLAYBACK_TRANS_CANCEL:
    case CMD_PLAYBACK_PERMANENT_DELETE_SONG:
    case CMD_PLAYBACK_PERMANENT_DELETE_ALL_SONG:
    case CMD_PLAYBACK_PLAYLIST_ADD_SONG:
    case CMD_PLAYBACK_PLAYLIST_DELETE_SONG:
    case CMD_PLAYBACK_EXIT_TRANS:
    case CMD_PLAYBACK_GET_SD_SPACE_INFO:
    case CMD_PLAYBACK_GET_FLASH_SPACE_INFO:
    case CMD_PERMANENT_DELETE_ALL_FILE_BY_FORMAT:
    case CMD_TRANS_SET_SCENARIO:
        {
            app_playback_trans_cmd_handle(cmd_len, cmd_ptr, app_idx);
        }
        break;
#if CONFIG_REALTEK_APP_DSP_ONLINE_DEBUG
    case CMD_AUDIO_DSP_CTRL_SEND:
    case CMD_AUDIO_CODEC_CTRL_SEND:
        {
            app_audio_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif
#if CONFIG_REALTEK_APP_AUDIO_DATA_CAPTURE
    case CMD_DATA_CAPTURE_START_STOP:
        {
            audio_data_capture_cmd_handle(cmd_ptr, cmd_len, cmd_path, app_idx, ack_pkt);
        }
        break;

    case CMD_DATA_CAPTURE_ENTER_EXIT:
        {
            audio_data_capture_mode_ctl(&cmd_ptr[2], cmd_len - 2, cmd_path, app_idx, ack_pkt);
        }
        break;
#endif
    case CMD_SWITCH_TO_HCI_DOWNLOAD_MODE:
        {
            app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);
            os_delay(10);
            set_hci_mode_flag(true);
            chip_reset(RESET_ALL_EXCEPT_AON);
        }
        break;

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
    case CMD_OTA_KEY_CHECK:
    case CMD_OTA_REPORT_IMAGE_NUM:
    case CMD_ENTER_NORMAL_OTA:
    case CMD_NORMAL_OTA_CHECK:
        {
            app_ota_cmd_handle(cmd_path, cmd_len, cmd_ptr, app_idx);
        }
        break;
    }

    APP_PRINT_TRACE1("<==app_handle_cmd_set: ack_status 0x%02x", ack_pkt[2]);
}
