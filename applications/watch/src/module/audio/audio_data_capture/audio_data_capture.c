/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "app_timer.h"
#include "audio_route.h"
#include "app_audio_route.h"
#include "app_cmd.h"
#include "pm.h"
#include "trace.h"
#include "os_mem.h"
#include "stdlib.h"
#include "app_main.h"
#include "audio_probe.h"
#include "app_cfg.h"
#include "app_dsp_cfg.h"
#include "app_dlps.h"
#include "gap_br.h"
#include "bt_bond.h"
#include "app_bond.h"
#include "app_bt_policy_int.h"
#include "audio_track.h"
#include "audio.h"
#include "audio_data_capture.h"
#include "app_module_init.h"
#include "gap_br.h"
#include "app_report.h"
#include "os_sched.h"
#include "wdg.h"
#include "sysm.h"
#include "btm.h"
#include "app_bt_policy_api.h"
#include "app_transfer.h"
#include "audio_fake_sco_data.h"


#define APP_DATA_CAPTURE_POWER_OFF_TIME 600
//for CMD_AUDIO_DSP_CTRL_SEND to capture dsp data
#define VENDOR_SPP_CAPTURE_DSP_LOG      0x01
#define VENDOR_SPP_CAPTURE_DSP_RWA_DATA 0x02
#define H2D_CMD_DSP_DAC_ADC_DATA_TO_MCU 0x1F
#define H2D_SPPCAPTURE_SET              0x0F01
#define CHANGE_MODE_EXIST               0x00
#define CHANGE_MODE_TO_SCO              0x01

#define CAPTURE_MODE_MASK_A2DP          0x01
#define CAPTURE_MODE_MASK_HFP           0x02
#define CAPTURE_MODE_MASK_DSP_APT       0x04
#define CAPTURE_MODE_MASK_DATA_CAPTURE  0x08
#define CAPTURE_MODE_MASK_BUD_SIDE      0x10

static uint8_t audio_data_capture_timer_id = 0;
static uint8_t timer_idx_dsp_spp_captrue_check_link = 0;
static uint8_t dsp_capture_data_master_retry = 0;
static uint8_t *dsp_capture_data_cmd_ptr;
static uint8_t dsp_capture_data_cmd_len;
static uint8_t *dsp_capture_data_gain_level;
static uint8_t dsp_capture_data_app_idx;
static uint16_t dsp_capture_data_state = 0;
static uint8_t dsp_capture_data_path = CMD_PATH_SPP;
static uint16_t dsp_capture_data_seq = 0xFFFF;

static uint8_t dsp_capture_data_mode = 0;  // only for SPP2.0  , not for saiyan

T_AUDIO_TRACK_HANDLE dsp_capture_data_audio_track_handle;
T_AUDIO_TRACK_HANDLE dsp_capture_data_audio_record_handle;

enum
{
    CAPTRUE_TRACK_STOP       = 0x00,
    CAPTRUE_TRACK_START      = 0x01
};

typedef enum
{
    CAP_REPORT_ST_SUC = 0x0,
    CAP_REPORT_ST_NOT_SUP,
    CAP_REPORT_ST_LOAD_FAIL,
    CAP_REPORT_ST_EXIT_FAIL,
    CAP_REPORT_ST_OTHER_ERR = 0xFF

} T_CAPTURE_REPORT_ST;

typedef struct
{
    uint16_t    data_len;
    uint8_t     *p_data;
} T_PROBE_CB_MAILBOX_DATA;

typedef enum
{
    APP_TIMER_DATA_CAPTRUE_CHECK_LINK,
} T_CAPTURE_TIMER;

typedef enum
{
    APP_DATA_CAPTURE_SAIYAN         = 0x00,
    APP_DATA_CAPTURE_RAW_DATA       = 0x01,
    APP_DATA_CAPTURE_ENTER_SCO_MODE = 0x02,
    APP_DATA_CAPTURE_SCO_MODE_CTL   = 0x03,
    APP_DATA_CAPTURE_USER_MIC       = 0x04,
    APP_DATA_CAPTURE_DSP2_DATA      = 0x05,
    APP_DATA_CAPTURE_ACOUSTICS_MP   = 0x06,
} T_APP_CAPTURE_TYPE;

static void audio_data_capture_send_audio_probe(void);
static void audio_data_capture_unregister(void);
static void audio_data_capture_register(void);

static void audio_data_capture_report(uint8_t cmd_path, uint8_t app_idx, uint8_t track_status,
                                      uint8_t mask)
{
    uint8_t buf[2];
    buf[0] = track_status;
    buf[1] = mask;
    app_report_event(cmd_path, EVENT_DATA_CAPTURE_START_STOP, app_idx, buf, sizeof(buf));
}

static void audio_data_capture_audio_cback(T_AUDIO_EVENT event_type, void *event_buf,
                                           uint16_t buf_len)
{
    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;
    uint16_t seq_num;
    uint8_t send_pkt_num = 5;
    uint8_t track_status = 0;
    static uint16_t fake_sco_data_seq = 0;

    if ((param->track_state_changed.handle == dsp_capture_data_audio_track_handle) ||
        (param->track_state_changed.handle == dsp_capture_data_audio_record_handle))
    {
        switch (event_type)
        {
        case AUDIO_EVENT_TRACK_STATE_CHANGED:
            {
                uint8_t p_audio_buf[4] = {0, 0, 0, 0};

                APP_PRINT_INFO2("AUDIO_EVENT_TRACK_STATE_CHANGED handle %p track_state, 0x%x",
                                param->track_state_changed.handle,
                                param->track_state_changed.state);

                if ((param->track_state_changed.state == AUDIO_TRACK_STATE_STARTED) ||
                    (param->track_state_changed.state == AUDIO_TRACK_STATE_RELEASED))
                {
                    if (param->track_state_changed.handle == dsp_capture_data_audio_track_handle)
                    {
                        if (param->track_state_changed.state == AUDIO_TRACK_STATE_STARTED)
                        {
                            // for (seq_num = 0; seq_num < send_pkt_num; seq_num++)
                            // {
                            uint16_t written_len;
                            audio_track_write(dsp_capture_data_audio_track_handle,
                                              0,//              timestamp,
                                              fake_sco_data_seq,
                                              AUDIO_STREAM_STATUS_CORRECT,
                                              1,//            frame_num,
                                              (uint8_t *)fake_sco_data[fake_sco_data_seq],
                                              sizeof(fake_sco_data[fake_sco_data_seq]),
                                              &written_len);
                            // }
                            fake_sco_data_seq++;
                            if (fake_sco_data_seq > FAKE_SCO_DATA_LEN)
                            {
                                fake_sco_data_seq = 0;
                            }
                            track_status = CAPTRUE_TRACK_START;
                            //dsp_capture_data_state |= DATA_CAPTURE_DATA_START_SCO_MODE;
                        }
                        else if (param->track_state_changed.state == AUDIO_TRACK_STATE_RELEASED)
                        {
                            track_status = CAPTRUE_TRACK_STOP;
                            dsp_capture_data_audio_track_handle = NULL;
                            dsp_capture_data_state &= ~DATA_CAPTURE_DATA_START_SCO_MODE;
                        }
                    }

                    audio_data_capture_send_audio_probe();
                    audio_data_capture_report(dsp_capture_data_path, dsp_capture_data_app_idx, track_status, 0);
                }
            }
            break;
        case AUDIO_EVENT_TRACK_DATA_IND:
            {
                APP_PRINT_TRACE2("0x%x 0x%x", param->track_state_changed.handle, param->track_state_changed.state);
                if (param->track_data_ind.handle == dsp_capture_data_audio_track_handle)
                {
                    T_APP_BR_LINK *p_link;
                    p_link = app_find_br_link(app_db.br_link[0].bd_addr);
                    if (p_link == NULL)
                    {
                        break;
                    }
                    uint16_t written_len;
                    audio_track_write(dsp_capture_data_audio_track_handle,
                                      0,//              timestamp,
                                      fake_sco_data_seq,
                                      AUDIO_STREAM_STATUS_CORRECT,
                                      1,//            frame_num,
                                      (uint8_t *)fake_sco_data[fake_sco_data_seq],
                                      sizeof(fake_sco_data[fake_sco_data_seq]),
                                      &written_len);

                    APP_PRINT_INFO0("audio_data_capture_audio_cback audio_track_write");

                    fake_sco_data_seq++;
                    if (fake_sco_data_seq > FAKE_SCO_DATA_LEN)
                    {
                        fake_sco_data_seq = 0;
                    }

                    uint32_t timestamp;
                    uint16_t seq_num;
                    uint8_t frame_num;
                    uint16_t read_len;
                    uint8_t *buf;

                    buf = malloc(param->track_data_ind.len);

                    T_AUDIO_STREAM_STATUS status;
                    if (audio_track_read(dsp_capture_data_audio_track_handle,
                                         &timestamp,
                                         &seq_num,
                                         &status,
                                         &frame_num,
                                         buf,
                                         param->track_data_ind.len,
                                         &read_len) == true)
                    {
                        APP_PRINT_INFO1("[FAKE SCO]read mic data = %b", TRACE_BINARY(read_len, buf));
                    }
                    APP_PRINT_INFO0("audio_data_capture_audio_cback audio_track_read");
                    free(buf);
                    //dsp_capture_data_state |= DATA_CAPTURE_DATA_START_SCO_MODE;
                }
            }
            break;
        default:
            handle = false;
            break;
        }

        if (handle == true)
        {
            APP_PRINT_INFO1("audio_data_capture_audio_cback: event_type 0x%04x", event_type);
        }
    }
}

void audio_data_capture_dsp_event_cback(uint32_t event, void *msg)
{
    switch (event)
    {
    case AUDIO_PROBE_DSP_EVT_MAILBOX_DSP_DATA:
        {
            //FOR SPP CAPTURE DSP DATA
            T_PROBE_CB_MAILBOX_DATA *p_info = (T_PROBE_CB_MAILBOX_DATA *)msg;
            uint16_t cur_seq = (p_info->p_data[2] | p_info->p_data[3] << 8);
            uint16_t pre_seq = (cur_seq == 0) ? 0xFFFF : cur_seq - 1;

            if (dsp_capture_data_seq != pre_seq)
            {
                APP_PRINT_INFO3("audio_data_capture_dsp_event_cback pre_seq %d curr_seq %d drop_seq %d",
                                dsp_capture_data_seq,
                                (p_info->p_data[2] | p_info->p_data[3] << 8),
                                pre_seq);
            }
            APP_PRINT_TRACE1("rcv_capture_data seq 0x%x", cur_seq);
            app_report_event(dsp_capture_data_path, EVENT_DATA_CAPTURE_DATA, dsp_capture_data_app_idx,
                             p_info->p_data, p_info->data_len);
            dsp_capture_data_seq = cur_seq;
            app_transfer_queue_recv_ack_check(EVENT_DATA_CAPTURE_DATA, dsp_capture_data_path);
        }
        break;

    default:
        break;
    }
}

bool audio_data_capture_get_record_handle(void)
{
    return (dsp_capture_data_audio_record_handle != NULL);
}

uint16_t audio_data_capture_get_state(void)
{
    return dsp_capture_data_state;
}

bool audio_data_capture_executing_check(void)
{
    return (dsp_capture_data_state & (DATA_CAPTURE_DATA_LOG_EXECUTING | DATA_CAPTURE_RAW_DATA_EXECUTING
                                      | DATA_CAPTURE_DATA_SAIYAN_EXECUTING | DATA_CAPTURE_DATA_USER_MIC_EXECUTING |
                                      DATA_CAPTURE_DATA_ACOUSTICS_MP_EXECUTING))
           ? true : false;
}

static void audio_data_capture_send_audio_probe(void)
{
    if (dsp_capture_data_cmd_ptr)
    {
        audio_probe_dsp_send(dsp_capture_data_cmd_ptr, dsp_capture_data_cmd_len);
        free(dsp_capture_data_cmd_ptr);
        dsp_capture_data_cmd_ptr = NULL;
    }
}

void audio_data_capture_start_process(T_CAPTURE_HEADER *param, uint8_t cmd_path, uint8_t app_idx)
{
    uint32_t actual_mhz;
    uint16_t pkt_type = GAP_PKT_TYPE_DM1 | GAP_PKT_TYPE_DH1 | \
                        GAP_PKT_TYPE_DM3 | GAP_PKT_TYPE_DH3 | \
                        GAP_PKT_TYPE_DM5 | GAP_PKT_TYPE_DH5 | \
                        GAP_PKT_TYPE_NO_2DH1 | GAP_PKT_TYPE_NO_2DH3 | GAP_PKT_TYPE_NO_2DH5;

    dsp_capture_data_state |= DATA_CAPTURE_DATA_ENTER_START;

    app_dlps_disable(APP_DLPS_ENTER_CHECK_SPP_CAPTURE);
    for (uint8_t i = 0; i < MAX_BR_LINK_NUM; i++)
    {
        if (app_db.br_link[i].used)
        {
            bt_sniff_mode_disable(app_db.br_link[i].bd_addr);
        }
    }

    gap_br_cfg_acl_pkt_type(app_db.br_link[app_idx].bd_addr, pkt_type); // set 3M only

    bt_link_qos_set(app_db.br_link[app_idx].bd_addr, BT_QOS_TYPE_GUARANTEED, param->tpoll);

    if (param->flush_tout)
    {
        gap_br_cfg_acl_link_flush_tout(app_db.br_link[app_idx].bd_addr, param->flush_tout);
    }
}

void audio_data_capture_stop_process(uint8_t cmd_path, uint8_t app_idx)
{
    uint32_t actual_mhz;
    uint32_t plan_profs;
    uint32_t bond_flag;

    dsp_capture_data_state &= ~DATA_CAPTURE_DATA_ENTER_START;

    if (dsp_capture_data_audio_record_handle)
    {
        audio_track_release(dsp_capture_data_audio_record_handle);
    }

    if (dsp_capture_data_state & DATA_CAPTURE_DATA_CHANGE_TO_SCO_MODE)
    {
        if (dsp_capture_data_audio_track_handle != NULL)
        {
            audio_track_release(dsp_capture_data_audio_track_handle);
        }
        bt_bond_flag_get(app_db.br_link[app_idx].bd_addr, &bond_flag);
        if (bond_flag & (BOND_FLAG_HFP | BOND_FLAG_HSP | BOND_FLAG_A2DP))
        {
            //TODO: app_bt_policy_get_profs_by_bond_flag api not found
            // plan_profs = app_bt_policy_get_profs_by_bond_flag(bond_flag);
            // app_bt_policy_default_connect(app_db.br_link[app_idx].bd_addr, plan_profs, false);
        }
    }

    app_dlps_enable(APP_DLPS_ENTER_CHECK_SPP_CAPTURE);

    bt_sniff_mode_enable(app_db.br_link[app_idx].bd_addr, 784, 816, 0, 0);
    bt_acl_pkt_type_set(app_db.br_link[app_idx].bd_addr, BT_ACL_PKT_TYPE_3M);
//    pm_cpu_freq_set(APP_BASE_CPU_FREQ, &actual_mhz);

    //bt_link_role_switch(app_db.br_link[app_idx].bd_addr, false);
    bt_link_qos_set(app_db.br_link[app_idx].bd_addr, BT_QOS_TYPE_GUARANTEED, 40);

    if (dsp_capture_data_state & DATA_CAPTURE_DATA_ENTER_TEST_MODE)
    {
        dsp_capture_data_state = 0;
        dsp_capture_data_state |= DATA_CAPTURE_DATA_ENTER_TEST_MODE;
    }
    else
    {
        dsp_capture_data_state = 0;
    }
}


static void audio_data_capture_enter_sco_pre_handle(uint8_t app_idx)
{
    uint32_t plan_profs;
    dsp_capture_data_state |= DATA_CAPTURE_DATA_CHANGE_TO_SCO_MODE;
    plan_profs = (app_db.br_link[app_idx].connected_profile & (~RDTP_PROFILE_MASK) &
                  (~SPP_PROFILE_MASK));
    if (plan_profs)
    {
        app_bt_policy_disconnect(app_db.br_link[app_idx].bd_addr, plan_profs);
    }
    app_start_timer(&timer_idx_dsp_spp_captrue_check_link, "dsp_spp_captrue_check_link",
                    audio_data_capture_timer_id, APP_TIMER_DATA_CAPTRUE_CHECK_LINK, app_idx, false,
                    1500);
}

static uint8_t audio_data_capture_a2dp_hfp_mask_check(uint8_t app_idx)
{
    T_AUDIO_TRACK_STATE state;
    uint8_t mask = 0;
    for (uint8_t app_idx = 0; app_idx < MAX_BR_LINK_NUM; app_idx++)
    {
        if (app_db.br_link[app_idx].a2dp_track_handle)
        {
            audio_track_state_get(app_db.br_link[app_idx].a2dp_track_handle, &state);
            if ((state == AUDIO_TRACK_STATE_CREATED) ||
                (state == AUDIO_TRACK_STATE_STARTED) ||
                (state == AUDIO_TRACK_STATE_RESTARTED))
            {
                mask |= CAPTURE_MODE_MASK_A2DP;
            }
        }
        if (app_db.br_link[app_idx].sco_track_handle)
        {
            audio_track_state_get(app_db.br_link[app_idx].sco_track_handle, &state);
            if ((state == AUDIO_TRACK_STATE_CREATED) ||
                (state == AUDIO_TRACK_STATE_STARTED) ||
                (state == AUDIO_TRACK_STATE_RESTARTED))
            {
                mask |= CAPTURE_MODE_MASK_HFP;
            }
        }
    }

    return mask;
}

void audio_data_capture_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path,
                                   uint8_t app_idx, uint8_t *ack_pkt)
{
    uint8_t data_idx = 0;
    uint8_t *p_data = &cmd_ptr[8];
    uint8_t capture_type = 0;
    uint8_t capture_len = 0;
    bool stop_process = false;
    uint8_t capture_mask = 0;

    T_CAPTURE_HEADER throughput_param;

    APP_PRINT_TRACE3("audio_data_capture_cmd_handle capture_mode %d data_state %d %b",
                     dsp_capture_data_mode, dsp_capture_data_state,
                     TRACE_BINARY(cmd_len, cmd_ptr));

    memcpy(&throughput_param, &cmd_ptr[2], sizeof(T_CAPTURE_HEADER));
    app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

    dsp_capture_data_app_idx = app_idx;
    dsp_capture_data_path = cmd_path;

    if (audio_data_capture_executing_check() == throughput_param.capture_enable)
    {
        audio_data_capture_report(dsp_capture_data_path, dsp_capture_data_app_idx,
                                  throughput_param.capture_enable, 0);
        APP_PRINT_TRACE2("audio_data_capture_cmd_handle check state %d %d",
                         dsp_capture_data_state,
                         throughput_param.capture_enable);
    }

    if (throughput_param.capture_enable)
    {
        capture_mask = audio_data_capture_a2dp_hfp_mask_check(app_idx);
    }

    if (throughput_param.type_num)
    {
        for (uint8_t i = 0; i < throughput_param.type_num; i++)
        {
            capture_type = p_data[data_idx++];
            capture_len  = p_data[data_idx++];
            APP_PRINT_TRACE4("audio_data_capture_cmd_handle enable %d type %d len %d data_idx %d",
                             throughput_param.capture_enable,
                             capture_type,
                             capture_len,
                             data_idx);

            if (throughput_param.capture_enable)
            {
                /*need check capture_mask when enter fack sco mode*/
                if (capture_type != APP_DATA_CAPTURE_ENTER_SCO_MODE)
                {
                    audio_data_capture_start_process(&throughput_param, cmd_path, app_idx);
                }

                switch (capture_type)
                {
                case APP_DATA_CAPTURE_RAW_DATA:
                    {
                        dsp_capture_data_state |= DATA_CAPTURE_RAW_DATA_EXECUTING;
                    }
                    break;

                case APP_DATA_CAPTURE_ENTER_SCO_MODE:
                    {
                        if (capture_mask == 0)
                        {
                            audio_data_capture_start_process(&throughput_param, cmd_path, app_idx);
                            audio_data_capture_enter_sco_pre_handle(app_idx);
                        }
                        else
                        {
                            audio_data_capture_report(dsp_capture_data_path, dsp_capture_data_app_idx, CAPTRUE_TRACK_STOP,
                                                      capture_mask);
                        }
                    }
                    break;

                case APP_DATA_CAPTURE_SCO_MODE_CTL:
                    {
                        dsp_capture_data_state |= DATA_CAPTURE_DATA_START_SCO_MODE;
                    }
                    break;
                }
            }
            else
            {
                if ((capture_type == APP_DATA_CAPTURE_SAIYAN) ||
                    (capture_type == APP_DATA_CAPTURE_RAW_DATA) ||
                    (capture_type == APP_DATA_CAPTURE_ENTER_SCO_MODE) ||
                    (capture_type == APP_DATA_CAPTURE_USER_MIC) ||
                    (capture_type == APP_DATA_CAPTURE_ACOUSTICS_MP))
                {
                    stop_process = true;
                }
            }
            data_idx += capture_len;
        }
    }
    else
    {
        stop_process = true;
    }

    if (capture_type != APP_DATA_CAPTURE_DSP2_DATA)
    {
        dsp_capture_data_seq = 0xFFFF;
    }

    dsp_capture_data_cmd_len = cmd_len - data_idx - sizeof(T_CAPTURE_HEADER) - 2;
    dsp_capture_data_cmd_ptr = malloc(dsp_capture_data_cmd_len);
    memcpy(dsp_capture_data_cmd_ptr, &p_data[data_idx], dsp_capture_data_cmd_len);

    if ((dsp_capture_data_state & DATA_CAPTURE_RAW_DATA_EXECUTING) ||
        (dsp_capture_data_state & DATA_CAPTURE_DATA_START_SCO_MODE) ||
        ((dsp_capture_data_state & DATA_CAPTURE_DATA_SAIYAN_EXECUTING) && (stop_process)) ||
        ((dsp_capture_data_state & DATA_CAPTURE_DATA_USER_MIC_EXECUTING) && (stop_process)) ||
        ((dsp_capture_data_state & DATA_CAPTURE_DATA_ACOUSTICS_MP_EXECUTING) && (stop_process)))
    {
        audio_data_capture_send_audio_probe();
    }

    if (stop_process)
    {
        audio_data_capture_stop_process(cmd_path, app_idx);
    }
}

static void audio_data_capture_bt_event_cback(T_BT_EVENT event_type, void *event_buf,
                                              uint16_t buf_len)
{
    bool handle = true;
    T_BT_EVENT_PARAM *param = event_buf;

    switch (event_type)
    {
    case BT_EVENT_SPP_DISCONN_CMPL:
    case BT_EVENT_ACL_CONN_DISCONN:
        {
            if ((dsp_capture_data_state & DATA_CAPTURE_DATA_ENTER_TEST_MODE) != 0)
            {
                if (dsp_capture_data_mode)
                {
                    if (event_type == BT_EVENT_ACL_CONN_DISCONN)
                    {
                        audio_data_capture_stop_process(dsp_capture_data_path, 0);
                        dsp_capture_data_mode = 0;
                        audio_probe_dsp_test_bin_set(false);
                        audio_data_capture_unregister();
                    }
                }
            }
        }
        break;

    case BT_EVENT_ACL_ROLE_MASTER:
        {
            if ((dsp_capture_data_state & DATA_CAPTURE_DATA_ENTER_TEST_MODE) != 0)
            {
                if (dsp_capture_data_state & DATA_CAPTURE_DATA_SWAP_TO_MASTER)
                {
                    audio_data_capture_send_audio_probe();
                    dsp_capture_data_state |= DATA_CAPTURE_DATA_START_MASK;
                    dsp_capture_data_state &= ~DATA_CAPTURE_DATA_SWAP_TO_MASTER;
                }
            }
        }
        break;

    case BT_EVENT_ACL_ROLE_SWITCH_FAIL:
        {
            if ((dsp_capture_data_state & DATA_CAPTURE_DATA_ENTER_TEST_MODE) != 0)
            {
                if (dsp_capture_data_state & DATA_CAPTURE_DATA_SWAP_TO_MASTER)
                {
                    if (dsp_capture_data_master_retry < 3)
                    {
                        bt_link_role_switch(param->acl_role_switch_fail.bd_addr, true);
                        dsp_capture_data_master_retry++;
                    }
                    else
                    {
                        ///if ((dsp_capture_data_state & DATA_CAPTURE_DATA_SAIYAN_EXECUTING) == 0)
                        {
                            audio_data_capture_send_audio_probe();
                        }
                        dsp_capture_data_state |= DATA_CAPTURE_DATA_START_MASK;
                        dsp_capture_data_state &= ~DATA_CAPTURE_DATA_SWAP_TO_MASTER;
                        dsp_capture_data_master_retry = 0;
                    }
                }
            }
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("audio_data_capture_bt_event_cback: event_type 0x%04x", event_type);
    }
}

static void audio_data_capture_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("audio_data_capture_timeout_cb %d %d", timer_evt, param);
    switch (timer_evt)
    {
    case APP_TIMER_DATA_CAPTRUE_CHECK_LINK:
        {
            app_stop_timer(&timer_idx_dsp_spp_captrue_check_link);

            if ((app_db.br_link[param].connected_profile & (~RDTP_PROFILE_MASK) & (~SPP_PROFILE_MASK)))
            {
                app_start_timer(&timer_idx_dsp_spp_captrue_check_link, "dsp_spp_captrue_check_link",
                                audio_data_capture_timer_id, APP_TIMER_DATA_CAPTRUE_CHECK_LINK, param, false,
                                1500);
            }
            else
            {
                T_AUDIO_FORMAT_INFO format_info;

                format_info.type = AUDIO_FORMAT_TYPE_MSBC;
                // format_info.frame_num = 1;
                format_info.attr.msbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_MONO;
                format_info.attr.msbc.sample_rate = 16000;
                format_info.attr.msbc.bitpool = 26;
                format_info.attr.msbc.allocation_method = 0;
                format_info.attr.msbc.subband_num = 8;
                format_info.attr.msbc.block_length = 15;
                format_info.attr.msbc.chann_location = AUDIO_CHANNEL_LOCATION_MONO;

                if (dsp_capture_data_audio_track_handle)
                {
                    audio_track_release(dsp_capture_data_audio_track_handle);
                }

                APP_PRINT_TRACE2("audio_track_create %d db %d db", app_dsp_cfg_vol.voice_out_volume_default,
                                 app_dsp_cfg_vol.voice_volume_in_default);
                dsp_capture_data_audio_track_handle = audio_track_create(AUDIO_STREAM_TYPE_VOICE,
                                                                         AUDIO_STREAM_MODE_NORMAL,
                                                                         AUDIO_STREAM_USAGE_LOCAL,
                                                                         format_info,
                                                                         8,
                                                                         8,
                                                                         AUDIO_DEVICE_OUT_SPK | AUDIO_DEVICE_IN_MIC,
                                                                         NULL,
                                                                         NULL);

                audio_track_latency_set(dsp_capture_data_audio_track_handle, 15, false);
                audio_track_start(dsp_capture_data_audio_track_handle);
            }
        }
        break;

    default:
        break;
    }
}

static void audio_data_capture_dm_cback(T_SYS_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    switch (event_type)
    {
    case SYS_EVENT_POWER_ON:
        {
            //do nothing
        }
        break;
    default:
        break;
    }
    APP_PRINT_TRACE1("audio_data_capture_dm_cback %d", event_type);
}

static void audio_data_capture_register(void)
{
    APP_PRINT_TRACE0("audio_data_capture_register");
    dsp_capture_data_state |= DATA_CAPTURE_DATA_ENTER_TEST_MODE;

    audio_mgr_cback_register(audio_data_capture_audio_cback);
    audio_probe_dsp_evt_cback_register(audio_data_capture_dsp_event_cback);
    sys_mgr_cback_register(audio_data_capture_dm_cback);
}

static void audio_data_capture_unregister(void)
{
    APP_PRINT_TRACE0("audio_data_capture_unregister");
    dsp_capture_data_state &= ~DATA_CAPTURE_DATA_ENTER_TEST_MODE;

    audio_mgr_cback_unregister(audio_data_capture_audio_cback);
    audio_probe_dsp_evt_cback_unregister(audio_data_capture_dsp_event_cback);
    sys_mgr_cback_unregister(audio_data_capture_dm_cback);
}

void audio_data_capture_mode_ctl(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path,
                                 uint8_t app_idx, uint8_t *ack_pkt)
{
    uint8_t dsp_data_capture_scenario = cmd_ptr[0];
    uint8_t report_data[2] = {0};
    uint8_t report_mask = 0;

    app_report_event(cmd_path, EVENT_ACK, app_idx, ack_pkt, 3);

    if (dsp_data_capture_scenario != 0)
    {
        if (dsp_capture_data_state & DATA_CAPTURE_DATA_ENTER_TEST_MODE)
        {
            dsp_capture_data_mode = dsp_data_capture_scenario;
            report_data[0] = CAP_REPORT_ST_SUC;
            report_data[1] = 0;
            goto capture_mode_report;
        }
    }

    report_mask = audio_data_capture_a2dp_hfp_mask_check(app_idx);

    if (audio_data_capture_executing_check())
    {
        report_mask |= CAPTURE_MODE_MASK_DATA_CAPTURE;
    }

    report_data[1] = report_mask;

    if (report_mask == 0)
    {
        dsp_capture_data_mode = dsp_data_capture_scenario;
        report_data[0] = CAP_REPORT_ST_SUC;
    }
    else
    {
        report_data[0] = (dsp_capture_data_mode) ? CAP_REPORT_ST_EXIT_FAIL : CAP_REPORT_ST_LOAD_FAIL;
    }

    if ((dsp_capture_data_mode != 0) && (dsp_data_capture_scenario != 0))
    {
        audio_data_capture_register();
    }
    else
    {
        audio_data_capture_unregister();
    }

capture_mode_report:
    APP_PRINT_TRACE5("audio_data_capture_mode_ctl %d %d 0x%04x %d 0x%02x",
                     dsp_data_capture_scenario, dsp_capture_data_mode, dsp_capture_data_state,
                     report_data[0],
                     report_data[1]);

    app_report_event(cmd_path, EVENT_DATA_CAPTURE_ENTER_EXIT, app_idx, report_data, 2);

}

void audio_data_capture_init(void)
{
    bt_mgr_cback_register(audio_data_capture_bt_event_cback);
    app_timer_reg_cb(audio_data_capture_timeout_cb, &audio_data_capture_timer_id);
}

static void audio_data_capture_module_init(void)
{
    audio_data_capture_init();
}
APP_MODULE_INIT(audio_data_capture_module_init);

