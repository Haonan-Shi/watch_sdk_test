/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "bt_types.h"
#include "trace.h"
#include "os_mem.h"
//#include "bqb.h"
#include "app_mmi.h"
#include "app_main.h"
#include "gap_br.h"
//#include "cli_power.h"
#include "app_msg.h"
#include "app_console_msg.h"
#include "app_cmd.h"
#include "app_link_util.h"
#include "bt_a2dp.h"
#include "bt_hfp.h"
#include "bt_avrcp.h"
#include "btm.h"
#if ISOC_TEST_SUPPORT
#include "cli_isoc.h"
#include "app_isoc_test.h"
#endif

#if (F_APP_ONE_WIRE_UART_SUPPORT == 1)
#include "app_cfg.h"
#include "app_one_wire_uart.h"
#endif

#if F_APP_LE_AUDIO_TEST
#include "app_multilink.h"
#include "app_cfg.h"

#if F_APP_CCP_SUPPORT
#include "ccp_client.h"
#endif
#include "cli_power.h"
#if F_APP_MCP_SUPPORT
#include "mcp_client.h"
#endif
#if F_APP_VCS_SUPPORT
#include "vcs_mgr.h"
#endif

#if (F_APP_TMAP_BMR_SUPPORT)

#if F_APP_LE_AUDIO_CLI_TEST
#include "cli_le_audio.h"
#endif

#include "app_broadcast_sync.h"
#if F_APP_TMAP_BMR_SUPPORT
#include "gap_ext_scan.h"
#include "ble_isoch_def.h"
#include "app_le_audio_scan.h"
#endif

#endif
#endif

#define SCO_PKT_TYPES_HV3_EV3_2EV3          (GAP_PKT_TYPE_HV3 | GAP_PKT_TYPE_EV3 | GAP_PKT_TYPE_NO_3EV3 | GAP_PKT_TYPE_NO_2EV5 | GAP_PKT_TYPE_NO_3EV5)

void app_console_handle_msg(T_IO_MSG console_msg)
{
    uint16_t  subtype;
//    uint16_t  id;
    uint16_t cmd_len;
    uint8_t rx_seqn;
//    uint8_t   action;
    uint8_t  *p;

    p       = console_msg.u.buf;
    subtype = console_msg.subtype;
    switch (subtype)
    {
    case IO_MSG_CONSOLE_STRING_RX:
#if F_APP_POWER_CMD_SUPPORT
        LE_STREAM_TO_UINT16(id, p);

        if (id == POWER_CMD)
        {
            LE_STREAM_TO_UINT8(action, p);

            if (action == POWER_ACTION_POWER_ON)
            {
                app_mmi_handle_action(MMI_DEV_POWER_ON);
            }
            else if (action == POWER_ACTION_POWER_OFF)
            {
                app_db.power_off_cause = POWER_OFF_CAUSE_CMD_SET;
                app_mmi_handle_action(MMI_DEV_POWER_OFF);
            }
        }
#endif
#if F_APP_BQB_CLI_SUPPORT
        else if (id == BQB_CMD_SDP)
        {
            T_GAP_UUID_DATA data;
            uint16_t uuid;

            LE_STREAM_TO_UINT8(action, p);
            LE_STREAM_TO_UINT16(uuid, p);
            data.uuid_16 = uuid;

            if (action == BQB_ACTION_SDP_SEARCH)
            {
                gap_br_start_sdp_discov(p, GAP_UUID16, data);
            }
        }
        else if (id == BQB_CMD_AVDTP)
        {
            LE_STREAM_TO_UINT8(action, p);

            if (action == BQB_ACTION_AVDTP_OPEN)
            {
                bt_a2dp_stream_open_req(p, BT_A2DP_ROLE_SNK);
            }
            else if (action == BQB_ACTION_AVDTP_START)
            {
                bt_a2dp_stream_start_req(p);
            }
            else if (action == BQB_ACTION_AVDTP_CLOSE)
            {
                bt_a2dp_stream_close_req(p);
            }
            else if (action == BQB_ACTION_AVDTP_ABORT)
            {
                bt_a2dp_stream_abort_req(p);
            }
            else if (action == BQB_ACTION_AVDTP_CONNECT_SIGNAL)
            {
                app_bt_policy_default_connect(p, A2DP_PROFILE_MASK, false);
            }
            else if (action == BQB_ACTION_AVDTP_CONNECT_STREAM)
            {
                //api removed, use bt_a2dp_stream_open_req() to connect_stream_chann
            }
            else if (action == BQB_ACTION_AVDTP_DISCONNECT)
            {
                app_bt_policy_disconnect(p, A2DP_PROFILE_MASK);
            }
        }
        else if (id == BQB_CMD_AVRCP)
        {
            LE_STREAM_TO_UINT8(action, p);

            if (action == BQB_ACTION_AVRCP_CONNECT)
            {
                bt_avrcp_connect_req(p);
            }
            else if (action == BQB_ACTION_AVRCP_CONNECT_CONTROLLER)
            {
                T_LINKBACK_SEARCH_PARAM search_param;
                search_param.is_target = true;
                app_bt_policy_special_connect(p, AVRCP_PROFILE_MASK, &search_param);
            }
            else if (action == BQB_ACTION_AVRCP_CONNECT_TARGET)
            {
                T_LINKBACK_SEARCH_PARAM search_param;
                search_param.is_target = false;
                app_bt_policy_special_connect(p, AVRCP_PROFILE_MASK, &search_param);
            }
            else if (action == BQB_ACTION_AVRCP_DISCONNECT)
            {
                app_bt_policy_disconnect(p, AVRCP_PROFILE_MASK);
            }
            else if (action == BQB_ACTION_AVRCP_GET_PLAY_STATUS)
            {
                bt_avrcp_get_play_status_req(p);
            }
            else if (action == BQB_ACTION_AVRCP_GET_ELEMENT_ATTR)
            {
                uint8_t attr = BT_AVRCP_ELEM_ATTR_TITLE;
                bt_avrcp_get_element_attr_req(p, 1, &attr);
            }
            else if (action == BQB_ACTION_AVRCP_PLAY)
            {
                bt_avrcp_play(p);
            }
            else if (action == BQB_ACTION_AVRCP_PAUSE)
            {
                bt_avrcp_pause(p);
            }
            else if (action == BQB_ACTION_AVRCP_STOP)
            {
                bt_avrcp_stop(p);
            }
            else if (action == BQB_ACTION_AVRCP_REWIND)
            {
                bt_avrcp_rewind_start(p);
                bt_avrcp_rewind_stop(p);
            }
            else if (action == BQB_ACTION_AVRCP_FASTFORWARD)
            {
                bt_avrcp_fast_forward_start(p);
                bt_avrcp_fast_forward_stop(p);
            }
            else if (action == BQB_ACTION_AVRCP_FORWARD)
            {
                bt_avrcp_forward(p);
            }
            else if (action == BQB_ACTION_AVRCP_BACKWARD)
            {
                bt_avrcp_backward(p);
            }
            else if (action == BQB_ACTION_AVRCP_NOTIFY_VOLUME)
            {
                uint8_t   vol;
                LE_STREAM_TO_UINT8(vol, p);
                bt_avrcp_volume_change_req(p, vol);
            }
        }
        else if (id == BQB_CMD_RFCOMM)
        {
            LE_STREAM_TO_UINT8(action, p);

            switch (action)
            {
            case BQB_ACTION_RFCOMM_CONNECT_SPP:
                app_bt_policy_default_connect(p, SPP_PROFILE_MASK, false);
                break;
            case BQB_ACTION_RFCOMM_CONNECT_HFP:
                app_bt_policy_default_connect(p, HFP_PROFILE_MASK, false);
                break;
            case BQB_ACTION_RFCOMM_CONNECT_HSP:
                app_bt_policy_default_connect(p, HSP_PROFILE_MASK, false);
                break;
            case BQB_ACTION_RFCOMM_CONNECT_PBAP:
                app_bt_policy_default_connect(p, PBAP_PROFILE_MASK, false);
                break;
            case BQB_ACTION_RFCOMM_DISCONNECT_SPP:
                app_bt_policy_disconnect(p, SPP_PROFILE_MASK);
                break;
            case BQB_ACTION_RFCOMM_DISCONNECT_HFP:
                app_bt_policy_disconnect(p, HFP_PROFILE_MASK);
                break;
            case BQB_ACTION_RFCOMM_DISCONNECT_HSP:
                app_bt_policy_disconnect(p, HSP_PROFILE_MASK);
                break;
            case BQB_ACTION_RFCOMM_DISCONNECT_PBAP:
                app_bt_policy_disconnect(p, PBAP_PROFILE_MASK);
                break;
            case BQB_ACTION_RFCOMM_DISCONNECT_ALL:
                break;
            default:

                break;
            }
        }
        else if (id == BQB_CMD_HFHS)
        {
            LE_STREAM_TO_UINT8(action, p);

            switch (action)
            {
            case BQB_ACTION_HFHS_CONNECT_SCO:
                bt_hfp_audio_connect_req(p);
                break;
            case BQB_ACTION_HFHS_DISCONNECT_SCO:
                bt_hfp_audio_disconnect_req(p);
                break;
            case BQB_ACTION_HFHS_CALL_ANSWER:
                bt_hfp_call_answer_req(p);
                break;
            case BQB_ACTION_HFHS_CALL_REDIAL:
                bt_hfp_dial_last_number_req(p);
                break;
            case BQB_ACTION_HFHS_CALL_ACTIVE:
                bt_hfp_release_active_call_accept_held_or_waiting_call_req(p);
                break;
            case BQB_ACTION_HFHS_CALL_END:
                bt_hfp_call_terminate_req(p);
                break;
            case BQB_ACTION_HFHS_CALL_REJECT:
                bt_hfp_call_terminate_req(p);
                break;
            case BQB_ACTION_HFHS_VOICE_RECOGNITION_ACTIVATE:
                bt_hfp_voice_recognition_enable_req(p);
                break;
            case BQB_ACTION_HFHS_VOICE_RECOGNITION_DEACTIVATE:
                bt_hfp_voice_recognition_disable_req(p);
                break;
            case BQB_ACTION_HFHS_SPK_GAIN_LEVEL_REPORT:
                {
                    uint8_t   level;
                    LE_STREAM_TO_UINT8(level, p);
                    bt_hfp_speaker_gain_level_report(p, level);
                }
                break;

            case BQB_ACTION_HFHS_MIC_GAIN_LEVEL_REPORT:
                {
                    uint8_t   level;
                    LE_STREAM_TO_UINT8(level, p);
                    bt_hfp_microphone_gain_level_report(p, level);
                }
                break;

            case BQB_ACTION_HFHS_SCO_CONN_REQ:
                {
                    uint16_t   voice_setting;
                    uint8_t    retrans_effort;
                    LE_STREAM_TO_UINT16(voice_setting, p);
                    LE_STREAM_TO_UINT8(retrans_effort, p);
                    gap_br_send_sco_conn_req(p, 8000, 8000, 12, voice_setting, retrans_effort,
                                             SCO_PKT_TYPES_HV3_EV3_2EV3);
                }
                break;

            default:
                break;
            }
        }
        else if (id == BQB_CMD_PBAP)
        {
            LE_STREAM_TO_UINT8(action, p);
            switch (action)
            {
            case BQB_ACTION_PBAP_VCARD_SRM:
            case BQB_ACTION_PBAP_VCARD_NOSRM:
                {
                    /*
                    uint8_t *bd_addr;
                    uint8_t *folder;
                    uint16_t folder_len;
                    T_PBAP_TAG_ID_ORDER_VALUE order;
                    uint8_t *search_value;
                    uint8_t value_len;
                    T_PBAP_TAG_ID_SEARCH_PROP_VALUE search_attr;
                    uint16_t max_list_count;
                    uint16_t start_offset;

                    pbap_pull_vcard_listing(bd_addr, folder, folder_len, order, search_value, value_len,
                                            search_attr, max_list_count, start_offset);
                    */
                }
                break;
            case BQB_ACTION_PBAP_VCARD_ENTRY:
                {
                    /*
                    uint8_t *bd_addr;
                    uint8_t *p_name;
                    uint8_t name_len;
                    uint8_t *filter;
                    T_PBAP_TAG_ID_FORMAT_VALUE format;

                    pbap_pull_vcard_entry(bd_addr, p_name, name_len, filter, format);
                    */
                }
                break;
            default:

                break;
            }
        }
#endif
#if F_APP_LE_AUDIO_TEST
#if F_APP_MCP_SUPPORT
        else if (id == BQB_CMD_MCP)
        {
            uint8_t conn_id;
            uint8_t srv_idx;
            uint8_t general;
            T_MCP_CLIENT_WRITE_MEDIA_CP_PARAM param;
            param.p_mcp_cb = NULL;

            LE_STREAM_TO_UINT8(action, p);
            LE_STREAM_TO_UINT8(conn_id, p);
            LE_STREAM_TO_UINT8(srv_idx, p);
            LE_STREAM_TO_UINT8(general, p);

            APP_PRINT_INFO4("app_console_handle_msg: action %x, conn_id %d, srv_idx %d, general %d",
                            action, conn_id, srv_idx, general);
            switch (action)
            {
            case BQB_ACTION_MCP_PLAY:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_PLAY;
                }
                break;

            case BQB_ACTION_MCP_PAUSE:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_PAUSE;
                }
                break;

            case BQB_ACTION_MCP_STOP:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_STOP;
                }
                break;

            case BQB_ACTION_MCP_FASTFORWARD:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_FAST_FORWARD;
                }
                break;

            case BQB_ACTION_MCP_FASTFORWARD_STOP:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_PLAY;
                }
                break;

            case BQB_ACTION_MCP_REWIND:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_FAST_REWIND;
                }
                break;

            case BQB_ACTION_MCP_REWIND_STOP:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_PLAY;
                }
                break;

            case BQB_ACTION_MCP_FORWARD:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_NEXT_TRACK;
                }
                break;

            case BQB_ACTION_MCP_BACKWARD:
                {
                    param.opcode = MCS_MEDIA_CONTROL_POINT_CHAR_OPCODE_PREVIOUS_TRACK;
                }
                break;

            default:
                break;
            }

            mcp_client_write_media_cp(le_get_conn_handle(conn_id), srv_idx, general, &param, true);
        }
#endif
#if F_APP_CCP_SUPPORT
        else if (id == BQB_CMD_CCP)
        {
            uint8_t conn_id;
            uint8_t srv_idx;
            uint8_t call_idx;
            uint8_t is_cmd;
            uint8_t general;
            uint16_t conn_handle;
            T_CCP_CLIENT_WRITE_CALL_CP_PARAM write_call_cp_param = {0};

            LE_STREAM_TO_UINT8(action, p);
            LE_STREAM_TO_UINT8(conn_id, p);
            LE_STREAM_TO_UINT8(srv_idx, p);
            LE_STREAM_TO_UINT8(call_idx, p);
            LE_STREAM_TO_UINT8(is_cmd, p);
            LE_STREAM_TO_UINT8(general, p);

            conn_handle = le_get_conn_handle(conn_id);

            APP_PRINT_INFO6("app_console_handle_msg: action %x, conn_id %d, srv_idx %d, call_idx %d, is_cmd %d, general %d",
                            action, conn_id, srv_idx, call_idx, is_cmd, general);
            switch (action)
            {
            case BQB_ACTION_CAP_ACCEPT:
                {
                    write_call_cp_param.opcode = TBS_CALL_CONTROL_POINT_CHAR_OPCODE_ACCEPT;
                    write_call_cp_param.param.accept_opcode_call_index = call_idx;

                    ccp_client_write_call_cp(conn_handle, srv_idx, general, (!is_cmd), &write_call_cp_param);
                }
                break;

            case BQB_ACTION_CAP_TERMINATE:
                {
                    write_call_cp_param.opcode = TBS_CALL_CONTROL_POINT_CHAR_OPCODE_TERMINATE;
                    write_call_cp_param.param.terminate_opcode_call_index = call_idx;

                    ccp_client_write_call_cp(conn_handle, srv_idx, general, (!is_cmd), &write_call_cp_param);
                }
                break;

            case BQB_ACTION_CAP_LOCAL_HOLD:
                {
                    write_call_cp_param.opcode = TBS_CALL_CONTROL_POINT_CHAR_OPCODE_LOCAL_HOLD;
                    write_call_cp_param.param.local_hold_opcode_call_index = call_idx;

                    ccp_client_write_call_cp(conn_handle, srv_idx, general, (!is_cmd), &write_call_cp_param);
                }
                break;

            case BQB_ACTION_CAP_LOCAL_RETRIEVE:
                {
                    write_call_cp_param.opcode = TBS_CALL_CONTROL_POINT_CHAR_OPCODE_LOCAL_RETRIEVE;
                    write_call_cp_param.param.local_retrieve_opcode_call_index = call_idx;

                    ccp_client_write_call_cp(conn_handle, srv_idx, general, (!is_cmd), &write_call_cp_param);
                }
                break;

            default:
                break;
            }
        }
#endif
#if F_APP_VCS_SUPPORT
        else if (id == BQB_CMD_VCS)
        {
            uint8_t volume_setting;
            uint8_t mute;
            uint8_t change_counter;
            uint8_t volume_flags;
            uint8_t step_size;

            LE_STREAM_TO_UINT8(action, p);
            LE_STREAM_TO_UINT8(volume_setting, p);
            LE_STREAM_TO_UINT8(mute, p);
            LE_STREAM_TO_UINT8(change_counter, p);
            LE_STREAM_TO_UINT8(volume_flags, p);
            LE_STREAM_TO_UINT8(step_size, p);

            APP_PRINT_INFO6("app_console_handle_msg: action %x, volume_setting %d, mute %d, change_counter %d, volume_flags %d, step_size %d",
                            action, volume_setting, mute, change_counter, volume_flags, step_size);
            switch (action)
            {
            case BQB_ACTION_VCS_UP:
                {
                    T_VCS_PARAM vcs_param;

                    vcs_param.volume_setting = volume_setting;
                    vcs_param.mute = mute;
                    vcs_param.change_counter = change_counter;
                    vcs_param.volume_flags = volume_flags;
                    vcs_param.step_size = step_size;
                    vcs_set_param(&vcs_param);
                }
                break;

            case BQB_ACTION_VCS_DOWN:
                {
                    T_VCS_PARAM vcs_param;

                    vcs_param.volume_setting = volume_setting;
                    vcs_param.mute = mute;
                    vcs_param.change_counter = change_counter;
                    vcs_param.volume_flags = volume_flags;
                    vcs_param.step_size = step_size;
                    vcs_set_param(&vcs_param);
                }
                break;

            default:
                break;
            }
        }
#endif
#endif//LE_AUDIO_TESTend                
#if ISOC_TEST_SUPPORT
        else if (id & ISOC_CMD_SET)
        {
            APP_PRINT_TRACE1("app_console_handle_msg: ISOC_CMD_SET id 0x%04x", id);
            switch (id)
            {
            case ISOC_CMD_CONFIG:
                {
                    uint8_t session_id;
                    uint8_t scenario_idx;
                    uint8_t setting_group;
                    uint8_t setting_idx;
                    LE_STREAM_TO_UINT8(session_id, p);
                    LE_STREAM_TO_UINT8(scenario_idx, p);
                    LE_STREAM_TO_UINT8(setting_group, p);
                    LE_STREAM_TO_UINT8(setting_idx, p);
                    app_isoc_cmd_config(session_id, scenario_idx, setting_group, setting_idx);
                }
                break;
            case ISOC_CMD_ENABLE:
                {
                    uint8_t session_id;
                    LE_STREAM_TO_UINT8(session_id, p);
                    app_isoc_cmd_enable(session_id);
                }
                break;
            case ISOC_CMD_DISABLE:
                {
                    uint8_t session_id;
                    LE_STREAM_TO_UINT8(session_id, p);
                    app_isoc_cmd_disable(session_id);
                }
                break;
            case ISOC_CMD_RELEASE:
                {
                    uint8_t session_id;
                    LE_STREAM_TO_UINT8(session_id, p);
                    app_isoc_cmd_release(session_id);
                }
                break;
            case ISOC_CMD_STARTADV:
                {
                    uint8_t always_adv;
                    LE_STREAM_TO_UINT8(always_adv, p);
                    app_isoc_cmd_startadv(always_adv);
                }
                break;
            case ISOC_CMD_STOPADV:
                {
                    app_isoc_cmd_stopadv();
                }
                break;
            case ISOC_CMD_CON:
                {
                    uint8_t bt_addr[6];
                    memcpy(bt_addr, p, 6);
                    app_isoc_cmd_con(bt_addr);
                }
                break;
            case ISOC_CMD_DISC:
                {
                    uint8_t conn_id;
                    LE_STREAM_TO_UINT8(conn_id, p);
                    app_isoc_cmd_disc(conn_id);
                }
                break;
            case ISOC_CMD_DUMP:
                {
                    app_isoc_cmd_dump();
                }
                break;
#if ISOC_BIS_TEST
            case ISOC_CMD_BSENABLE:
                {
                    uint8_t mode;
                    uint8_t item;
                    LE_STREAM_TO_UINT8(mode, p);
                    LE_STREAM_TO_UINT8(item, p);
                    app_isoc_cmd_bsenable(mode, (T_CODEC_CFG_ITEM)item);
                }
                break;
            case ISOC_CMD_BSDISABLE:
                {
                    app_isoc_cmd_bsdisable();
                }
                break;
            case ISOC_CMD_BSYNCENABLE:
                {
                    uint8_t mode;
                    uint8_t item;
                    uint8_t bt_addr[6];
                    LE_STREAM_TO_UINT8(mode, p);
                    LE_STREAM_TO_UINT8(item, p);
                    memcpy(bt_addr, p, 6);
                    app_isoc_cmd_bsyncenable(mode, (T_CODEC_CFG_ITEM)item,  bt_addr);
                }
                break;
            case ISOC_CMD_BSYNCDISABLE:
                {
                    app_isoc_cmd_bsyncdisable();
                }
                break;
#endif
            default:
                break;
            }
        }
#endif
#if F_APP_LE_AUDIO_CLI_TEST
        else if (id & AUDIO_CMD_SET)
        {
            APP_PRINT_TRACE1("app_console_handle_msg: AUDIO_CMD_SET id 0x%04x", id);
            switch (id)
            {
            case AUDIO_CMD_STARTESCAN:
                {
                    uint8_t bis_channel;
                    LE_STREAM_TO_UINT8(bis_channel, p);
                    app_cfg_const.iso_mode = 1;
                    app_cfg_const.subgroup = bis_channel;
                    app_le_audio_scan_start(LE_AUDIO_SCAN_TIME);
                    app_le_audio_bis_state_change(LE_AUDIO_BIS_STATE_SCAN);
                    app_bt_sniffing_param_update(APP_BT_SNIFFING_EVENT_ISO_SUSPEND);
                }
                break;
            case AUDIO_CMD_STOPESCAN:
                {
                    le_ext_scan_stop();
                    app_le_audio_bis_state_change(LE_AUDIO_BIS_STATE_IDLE);
#if 0
                    T_APP_BR_LINK *p_link = NULL;
                    p_link = &app_db.br_link[app_a2dp_get_active_idx()];
                    if (p_link && (app_cfg_nv.bud_role == REMOTE_SESSION_ROLE_PRIMARY) &&
                        (app_db.remote_session_state == REMOTE_SESSION_STATE_CONNECTED))
                    {

                        app_db.sniffing_stop = 1;
                        bt_sniffing_link_disconnect(p_link->bd_addr);

                    }
#endif
                }
                break;
            case AUDIO_CMD_PASYNC:
                {
                    uint8_t dev_idx, i;

                    LE_STREAM_TO_UINT8(dev_idx, p);
                    app_bc_set_active(dev_idx);
                    if (app_bc_sync_pa_sync(dev_idx) == false)
                    {
                        APP_PRINT_ERROR1("app_audio_cmd_pasync: failed: dev_idx: 0x%x", dev_idx);
                    }
                }
                break;
            case AUDIO_CMD_BIGSYNC:
                {
                    uint8_t dev_idx;

                    LE_STREAM_TO_UINT8(dev_idx, p);

                    if (app_bc_sync_big_establish(dev_idx) == false)
                    {
                        APP_PRINT_ERROR0("app_audio_cmd_bigsync: failed");
                    }
                }
                break;

            case AUDIO_CMD_PATERMINATE:
                {
                    uint8_t dev_idx;

                    LE_STREAM_TO_UINT8(dev_idx, p);

                    if (app_bc_sync_pa_terminate(dev_idx) == false)
                    {
                        APP_PRINT_ERROR0("app_audio_cmd_paterminate: failed");
                    }
                }
                break;

            case AUDIO_CMD_BIGTERMINATE:
                {
                    uint8_t dev_idx;

                    LE_STREAM_TO_UINT8(dev_idx, p);

                    if (app_bc_sync_big_terminate(dev_idx) == false)
                    {
                        APP_PRINT_ERROR0("app_audio_cmd_bigterminate: failed");
                    }
                }
                break;

            case AUDIO_CMD_BISOTERMINATE:
                {
                    uint8_t source_id;

                    LE_STREAM_TO_UINT8(source_id, p);

                    if (app_sink_iso_terminate(source_id) == false)
                    {
                        APP_PRINT_ERROR0("app_sink_iso_terminate: failed");
                    }
                }
                break;

            case AUDIO_CMD_BPATERMINATE:
                {
                    uint8_t source_id;

                    LE_STREAM_TO_UINT8(source_id, p);

                    if (app_sink_pa_terminate(source_id) == false)
                    {
                        APP_PRINT_ERROR0("app_sink_pa_terminate: failed");
                    }
                }
                break;

            case AUDIO_CMD_A2DP_PAUSE:
                {
                    app_big_handle_switch(LE_AUDIO_STREAMING, NULL);
                }
                break;

            case AUDIO_CMD_BIGSPATH:
                {
                    uint16_t bis_conn_handle;
                    uint8_t data_path_direction = DATA_PATH_ADD_OUTPUT;
                    uint8_t data_path_id = 0;
                    uint8_t codec_id[5] = {1, 0, 0, 0, 0};

                    LE_STREAM_TO_UINT16(bis_conn_handle, p);

                    if (gap_big_mgr_setup_data_path(bis_conn_handle, data_path_direction, data_path_id, codec_id,
                                                    0x1122,
                                                    0, NULL) != GAP_CAUSE_SUCCESS)
                    {
                        APP_PRINT_ERROR0("app_audio_cmd_bigspath: failed");
                    }
                }
                break;

            case AUDIO_CMD_BIGRPATH:
                {
                    uint16_t bis_conn_handle;
                    uint8_t data_path_direction = DATA_PATH_OUTPUT_FLAG;

                    LE_STREAM_TO_UINT16(bis_conn_handle, p);

                    if (gap_big_mgr_remove_data_path(bis_conn_handle, data_path_direction) != GAP_CAUSE_SUCCESS)
                    {
                        APP_PRINT_ERROR0("app_audio_cmd_bigrpath: failed");
                    }
                }
                break;

            case AUDIO_CMD_CIS_ADV:
                {
                    app_cfg_const.iso_mode = 0;
                    app_le_audio_device_sm(LE_AUDIO_AFE_SHAKING_DONE, NULL);

                }
                break;

            default:
                break;
            }
        }
#endif
        free(console_msg.u.buf);
        break;

    case  IO_MSG_CONSOLE_BINARY_RX:
#if (F_APP_ONE_WIRE_UART_SUPPORT == 1)
        if (app_cfg_const.one_wire_uart_support)
        {
            p += 1;
            LE_STREAM_TO_UINT8(rx_seqn, p);
            LE_STREAM_TO_UINT16(cmd_len, p);

            app_one_wire_cmd_set_handle(p, cmd_len, CMD_PATH_UART, rx_seqn, 0);
        }
        else
#endif
        {
            p += 1;
            LE_STREAM_TO_UINT8(rx_seqn, p);
            LE_STREAM_TO_UINT16(cmd_len, p);

            app_handle_cmd_set(p, cmd_len, CMD_PATH_UART, rx_seqn, 0);
        }
        free(console_msg.u.buf);
        break;

    default:
        break;
    }
}
