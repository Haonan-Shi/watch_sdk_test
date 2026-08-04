/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "bt_types.h"
#include "app_timer.h"
#include "btm.h"
#include "bt_bond.h"
#include "app_cfg.h"
#include "app_main.h"
#include "app_report.h"
#include "app_link_util.h"
#include "app_sdp.h"
#include "app_hfp.h"
#include "audio_hfp.h"
#include "app_audio_policy.h"
#include "audio_volume.h"
#include "app_bond.h"
#include "app_linkback.h"
#include "audio_track.h"
#include "app_bt_policy_api.h"
#include "app_audio_if.h"
#include "app_mmi.h"
#include "event_bus.h"
#include "nrec.h"

static uint8_t is_mic_mute = 0;
static uint8_t music_need_resume = 0;
static T_AUDIO_EFFECT_INSTANCE hfp_voice_nrec_instance;

static int32_t voice_nrec_attach(T_AUDIO_TRACK_HANDLE handle)
{
    T_AUDIO_EFFECT_INSTANCE nrec_instance;

    if (handle == NULL)
    {
        return -1;
    }

    nrec_instance = nrec_create(NREC_CONTENT_TYPE_VOICE, NREC_MODE_HIGH_SOUND_QUALITY, 0);
    if (nrec_instance == NULL)
    {
        return -2;
    }
    nrec_enable(nrec_instance);
    audio_track_effect_attach(handle, nrec_instance);
    hfp_voice_nrec_instance = nrec_instance;

    return 0;
}

static int32_t voice_nrec_detach(T_AUDIO_TRACK_HANDLE handle)
{
    if (handle == NULL)
    {
        return -1;
    }

    if (hfp_voice_nrec_instance == NULL)
    {
        return -2;
    }

    audio_track_effect_detach(handle, hfp_voice_nrec_instance);
    nrec_disable(hfp_voice_nrec_instance);
    nrec_release(hfp_voice_nrec_instance);

    hfp_voice_nrec_instance = NULL;
    return 0;
}

static void audio_voice_callback(T_AUDIO_EVENT event_type, void *event_buf, uint16_t buf_len)
{

    T_AUDIO_EVENT_PARAM *param = event_buf;
    bool handle = true;
    T_APP_BR_LINK *p_link;
    uint8_t active_hf_idx;

    active_hf_idx = app_hfp_get_active_hf_index();
    p_link = app_find_br_link(app_db.br_link[active_hf_idx].bd_addr);

    if (param->track_state_changed.handle != p_link->sco_track_handle)
    {
        return;
    }

    switch (event_type)
    {
    case AUDIO_EVENT_TRACK_STATE_CHANGED:
        {
            APP_PRINT_INFO1("audio_play_cback: track_state_changed.state %d",
                            param->track_state_changed.state);

            switch (param->track_state_changed.state)
            {
            case AUDIO_TRACK_STATE_STARTED:
                voice_nrec_attach(p_link->sco_track_handle);
                break;

            case AUDIO_TRACK_STATE_STOPPED:
                voice_nrec_detach(p_link->sco_track_handle);
                break;

            default:
                break;
            }
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_TRACE1("audio_voice_callback: event_type 0x%04x", event_type);
    }
}

void audio_coexist_handle(T_AUDIO_COEXIST_EVENT event)
{
    switch (event)
    {
    case AUDIO_COEXIST_CALL_STATUS_UPDATE:
        {
            if (app_hfp_get_call_status() != APP_HFP_CALL_IDLE)
            {
                if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
                {
                    app_mmi_handle_action(MMI_AV_PLAY_PAUSE);
                    music_need_resume = 1;
                }
            }
            else
            {
                if (music_need_resume)
                {
                    app_mmi_handle_action(MMI_AV_PLAY_PAUSE);
                    music_need_resume = 0;
                }
            }
        }
        break;
    case AUDIO_COEXIST_SCO_CONN_IND:
        {
            if (app_db.audio_play_mode != MODE_APP_A2DP_SNK)
            {
                if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
                {
                    app_mmi_handle_action(MMI_AV_PLAY_PAUSE);
                    music_need_resume = 1;
                }
            }
        }
        break;
    default:
        break;
    }
}

void audio_sco_conn_cmpl_handle(T_BT_EVENT_PARAM_SCO_CONN_CMPL *sco_conn_cmpl)
{
    uint8_t pair_idx;
    T_AUDIO_FORMAT_INFO format_info;
    T_APP_BR_LINK *p_link;
    int8_t temp_tx_power = 7;

    bt_link_tx_power_set(sco_conn_cmpl->bd_addr, temp_tx_power);

    if (sco_conn_cmpl->cause != 0)
    {
        return;
    }

    p_link = app_find_br_link(sco_conn_cmpl->bd_addr);
    if (p_link == NULL)
    {
        return;
    }

    if (bt_bond_index_get(sco_conn_cmpl->bd_addr, &pair_idx) == false)
    {
        APP_PRINT_ERROR0("sco_conn_cmpl get bond pair idx false");
        return;
    }

    p_link->sco_handle = sco_conn_cmpl->handle;

    if (app_find_sco_conn_num() == 1)
    {
        bt_sco_link_switch(sco_conn_cmpl->bd_addr);
    }
    else
    {
        APP_PRINT_TRACE2("app_multilink_bt_cback: active sco link %s, current link %s",
                         TRACE_BDADDR(app_db.br_link[app_hfp_get_active_hf_index()].bd_addr),
                         TRACE_BDADDR(sco_conn_cmpl->bd_addr));
        bt_sco_link_switch(app_db.br_link[app_hfp_get_active_hf_index()].bd_addr);

    }

    p_link->sco_seq_num = 0;

    format_info.frame_num = 1;
    if (sco_conn_cmpl->air_mode == 3) /**< Air mode transparent data. */
    {
        format_info.type = AUDIO_FORMAT_TYPE_MSBC;
        format_info.attr.msbc.chann_mode = AUDIO_SBC_CHANNEL_MODE_MONO;
        format_info.attr.msbc.sample_rate = 16000;
        format_info.attr.msbc.bitpool = 26;
        format_info.attr.msbc.allocation_method = 0;
        format_info.attr.msbc.subband_num = 8;
        format_info.attr.msbc.block_length = 15;
        format_info.attr.msbc.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
    }
    else if (sco_conn_cmpl->air_mode == 2) /**< Air mode CVSD. */
    {
        format_info.type = AUDIO_FORMAT_TYPE_CVSD;
        format_info.attr.cvsd.chann_num = 1;
        format_info.attr.cvsd.sample_rate = 8000;
        format_info.attr.cvsd.chann_location = AUDIO_CHANNEL_LOCATION_MONO;
        if (sco_conn_cmpl->rx_pkt_len == 30)
        {
            format_info.attr.cvsd.frame_duration = AUDIO_CVSD_FRAME_DURATION_3_75_MS;
        }
        else
        {
            format_info.attr.cvsd.frame_duration = AUDIO_CVSD_FRAME_DURATION_7_5_MS;
        }
    }

    if (p_link->sco_track_handle != NULL)
    {
        audio_track_release(p_link->sco_track_handle);
        p_link->sco_track_handle = NULL;
    }

    p_link->sco_track_handle = audio_track_create(AUDIO_STREAM_TYPE_VOICE,
                                                  AUDIO_STREAM_MODE_NORMAL,
                                                  AUDIO_STREAM_USAGE_SNOOP,
                                                  format_info,
                                                  app_cfg_nv.voice_gain_level[pair_idx],
                                                  15,
                                                  AUDIO_DEVICE_OUT_SPK | AUDIO_DEVICE_IN_MIC,
                                                  NULL,
                                                  NULL);
    if (p_link->sco_track_handle == NULL)
    {
        return;
    }

    if (app_find_b2s_link_num() > 1)
    {
        audio_track_latency_set(p_link->sco_track_handle, 150, false);
    }
    else
    {
        audio_track_latency_set(p_link->sco_track_handle, 100, false);
    }

    APP_PRINT_INFO0("voice audio track start");
    audio_track_start(p_link->sco_track_handle);
}

void audio_sco_discon_handle(T_BT_EVENT_PARAM_SCO_DISCONNECTED *sco_disconnected)
{
    T_APP_BR_LINK *p_link;
    p_link = app_find_br_link(sco_disconnected->bd_addr);
    bt_link_tx_power_set(sco_disconnected->bd_addr, 0);

    if (p_link != NULL)
    {
        if (p_link->sco_track_handle != NULL)
        {
            audio_track_release(p_link->sco_track_handle);
            p_link->sco_track_handle = NULL;
        }
        p_link->sco_handle = 0;
    }
}

void audio_sco_data_read(T_AUDIO_EVENT_PARAM *param)
{
    T_APP_BR_LINK *p_link;
    p_link = app_find_br_link(app_db.br_link[app_hfp_get_active_hf_index()].bd_addr);
    if (p_link == NULL)
    {
        return;
    }

    APP_PRINT_TRACE1("app_audio_policy_cback: data ind len %u", param->track_data_ind.len);
    uint32_t timestamp;
    uint16_t seq_num;
    uint8_t frame_num;
    uint16_t read_len;
    uint16_t send_size;
    uint8_t *buf;
    T_AUDIO_STREAM_STATUS status;

    buf = os_mem_alloc(OS_MEM_TYPE_DATA, param->track_data_ind.len);

    if (buf == NULL)
    {
        return;
    }

    if (audio_track_read(p_link->sco_track_handle,
                         &timestamp,
                         &seq_num,
                         &status,
                         &frame_num,
                         buf,
                         param->track_data_ind.len,
                         &read_len) == true)
    {
        APP_PRINT_INFO5("audio_track_read timestamp: %d, seq num: %d, frame num: %d, read length = %d, duplicate = %d",
                        timestamp, seq_num, frame_num, read_len, p_link->duplicate_fst_sco_data);
        APP_PRINT_INFO1("read mic data = %b", TRACE_BINARY(read_len, buf));

        send_size = read_len / frame_num;
        while (frame_num)
        {
            if (p_link->duplicate_fst_sco_data)
            {
                p_link->duplicate_fst_sco_data = false;
                bt_sco_data_send(app_db.br_link[app_hfp_get_active_hf_index()].bd_addr, seq_num - 1, buf,
                                 send_size);
            }
            bt_sco_data_send(app_db.br_link[app_hfp_get_active_hf_index()].bd_addr, seq_num,
                             buf + (read_len - frame_num * send_size), send_size);
            frame_num--;
        }
    }

    os_mem_free(buf);
}

void audio_sco_data_ind(T_BT_EVENT_PARAM_SCO_DATA_IND *sco_data_ind)
{
    uint16_t written_len;
    T_APP_BR_LINK *p_link;
    T_AUDIO_STREAM_STATUS status;

    p_link = app_find_br_link(sco_data_ind->bd_addr);
    if (p_link == NULL)
    {
        return;
    }

    p_link->sco_seq_num++;

    APP_PRINT_TRACE2("BT_EVENT_HFP_AG_SCO_DATA_IND:len %u status %u",
                     sco_data_ind->length, sco_data_ind->status);
    if (sco_data_ind->status == BT_SCO_PKT_STATUS_OK)
    {
        status = AUDIO_STREAM_STATUS_CORRECT;
    }
    else
    {
        status = AUDIO_STREAM_STATUS_LOST;
    }

    audio_track_write(p_link->sco_track_handle, sco_data_ind->bt_clk,
                      p_link->sco_seq_num,
                      status,
                      1,
                      sco_data_ind->p_data,
                      sco_data_ind->length,
                      &written_len);
}

bool audio_hfp_speaker_gain_level_report(uint8_t *bd_addr, uint8_t level)
{
    if (app_cfg_const.hfp_brsf_capability & BT_HFP_HF_LOCAL_REMOTE_VOLUME_CONTROL)
    {
        return bt_hfp_speaker_gain_level_report(bd_addr, level);
    }

    return true;
}

void audio_hfp_volume_up(void)
{
    uint8_t max_volume = 0;
    uint8_t curr_volume = 0;
    uint8_t active_idx;
    uint8_t pair_idx;
    T_AUDIO_STREAM_TYPE volume_type;
    volume_type = AUDIO_STREAM_TYPE_VOICE;
    active_idx = app_hfp_get_active_hf_index();

    if (app_db.br_link[active_idx].call_status == APP_HFP_CALL_INCOMING &&
        app_db.br_link[active_idx].is_inband_ring == false)
    {
        APP_PRINT_INFO0("app_volume_up: do not change vol when outband ringtone");
        return;
    }

    if (bt_bond_index_get(app_db.br_link[active_idx].bd_addr, &pair_idx) == false)
    {
        APP_PRINT_ERROR0("app_volume_up: find active hfp pair idx fail");
        return;
    }
    curr_volume = app_cfg_nv.voice_gain_level[pair_idx];
    max_volume = app_cfg_volume.voice_out_volume_max;

    if (curr_volume < max_volume)
    {
        curr_volume++;
    }

    audio_volume_out_set(volume_type, curr_volume);

    uint8_t level = (curr_volume * 0x0F + app_cfg_volume.voice_out_volume_max /
                     2) / app_cfg_volume.voice_out_volume_max;

    app_cfg_nv.voice_gain_level[pair_idx] = curr_volume;
    audio_hfp_speaker_gain_level_report(app_db.br_link[active_idx].bd_addr, level);

    // Notify phone GUI about volume change - pass actual gain level
    event_bus_publish(EVENT_BUS_TOPIC_HFP_AUDIO_VOLUME, &app_cfg_nv.voice_gain_level[pair_idx],
                      sizeof(app_cfg_nv.voice_gain_level[pair_idx]));
}

void audio_hfp_volume_down(void)
{
    bool already_vgs0 = false;
    uint8_t min_volume = 0;
    uint8_t curr_volume = 0;
    uint8_t active_idx;
    uint8_t pair_idx;
    T_AUDIO_STREAM_TYPE volume_type;
    volume_type = AUDIO_STREAM_TYPE_VOICE;
    active_idx = app_hfp_get_active_hf_index();

    if (app_db.br_link[active_idx].call_status == APP_HFP_CALL_INCOMING &&
        app_db.br_link[active_idx].is_inband_ring == false)
    {
        APP_PRINT_INFO0("app_volume_down: do not change vol when outband ringtone");
        return;
    }

    if (bt_bond_index_get(app_db.br_link[active_idx].bd_addr, &pair_idx) == false)
    {
        APP_PRINT_ERROR0("app_volume_down: find active hfp pair idx fail");
        return;
    }
    curr_volume = app_cfg_nv.voice_gain_level[pair_idx];
    min_volume = app_cfg_volume.voice_out_volume_min;

    if (curr_volume > min_volume)
    {
        curr_volume--;
    }
    else
    {
        curr_volume = min_volume;
        /*if ios version is 13, send AT+VGS=0 repeatedly will make voice mute*/
        if (curr_volume == 0)
        {
            already_vgs0 = true;
        }
    }

    audio_volume_out_set(volume_type, curr_volume);

    uint8_t level = (curr_volume * 0x0F + app_cfg_volume.voice_out_volume_max /
                     2) / app_cfg_volume.voice_out_volume_max;

    if (already_vgs0 == false)
    {
        audio_hfp_speaker_gain_level_report(app_db.br_link[active_idx].bd_addr, level);
    }
    app_cfg_nv.voice_gain_level[pair_idx] = curr_volume;

    // Notify phone GUI about volume change - pass actual gain level
    event_bus_publish(EVENT_BUS_TOPIC_HFP_AUDIO_VOLUME, &app_cfg_nv.voice_gain_level[pair_idx],
                      sizeof(app_cfg_nv.voice_gain_level[pair_idx]));
}

void audio_hfp_set_volume(uint8_t volume)
{
    uint8_t active_idx;
    uint8_t pair_idx;
    active_idx = app_hfp_get_active_hf_index();

    if (bt_bond_index_get(app_db.br_link[active_idx].bd_addr, &pair_idx) == false)
    {
        APP_PRINT_ERROR0("hfp spk volume change, get pair idx fail");
        return;
    }

    app_cfg_nv.voice_gain_level[pair_idx] = (volume * app_cfg_volume.voice_out_volume_max \
                                             + 0x0f / 2) / 0x0f;
    audio_volume_out_set(AUDIO_STREAM_TYPE_VOICE, app_cfg_nv.voice_gain_level[pair_idx]);

    // Notify phone GUI about volume change - pass actual gain level
    event_bus_publish(EVENT_BUS_TOPIC_HFP_AUDIO_VOLUME, &app_cfg_nv.voice_gain_level[pair_idx],
                      sizeof(app_cfg_nv.voice_gain_level[pair_idx]));
}

uint8_t audio_hfp_is_mic_mute(void)
{
    return is_mic_mute;
}

void audio_hfp_set_mic_mute_status(uint8_t status)
{
    is_mic_mute = status;
}

bool audio_hfp_check_mic_mute_enable(void)
{
    uint8_t i;
    bool enable_mic_mute = false;

    for (i = 0; i < MAX_BR_LINK_NUM; i++)
    {
        if (app_db.br_link[i].sco_handle != 0)
        {
            enable_mic_mute = true;
            break;
        }
        else
        {
            enable_mic_mute = false;
        }
    }

    return enable_mic_mute;
}

//mmi use
void app_mmi_mic_mute_set(void)
{
    uint8_t active_hf_idx = app_hfp_get_active_hf_index();
    T_APP_BR_LINK *p_link = NULL;

    p_link = app_find_br_link(app_db.br_link[active_hf_idx].bd_addr);
    if ((p_link != NULL) && p_link->sco_track_handle)
    {
        audio_track_volume_in_mute(p_link->sco_track_handle);
    }
}
//mmi use
void app_mmi_mic_unmute_set(void)
{
    uint8_t active_hf_idx = app_hfp_get_active_hf_index();
    T_APP_BR_LINK *p_link = NULL;

    p_link = app_find_br_link(app_db.br_link[active_hf_idx].bd_addr);
    if ((p_link != NULL) && p_link->sco_track_handle)
    {
        audio_track_volume_in_unmute(p_link->sco_track_handle);
    }
}

static void audio_hfp_set_mic_mute(void)
{
    uint8_t mic_mute = audio_hfp_is_mic_mute();
    if (audio_hfp_is_mic_mute())
    {
        audio_volume_in_unmute(AUDIO_STREAM_TYPE_VOICE);
        audio_hfp_set_mic_mute_status(0);
    }
    else
    {
        audio_volume_in_mute(AUDIO_STREAM_TYPE_VOICE);
        audio_hfp_set_mic_mute_status(1);
    }
    APP_PRINT_INFO2("audio_hfp_set_mic_mute is_mic_mute %d -> %d", mic_mute, audio_hfp_is_mic_mute());
}

void audio_hfp_mute_ctrl(void)
{
    uint8_t hf_active = 0;
    T_APP_BR_LINK *p_link;
    uint8_t app_idx;

    app_idx = app_hfp_get_active_hf_index();
    p_link = &(app_db.br_link[app_idx]);

    if (p_link->app_hf_state == APP_HF_STATE_CONNECTED)
    {
        if ((p_link->call_status == APP_HFP_CALL_ACTIVE) && (p_link->sco_handle != 0))
        {
            hf_active = 1;
        }
    }
    else
    {
        if (p_link->sco_handle)
        {
            hf_active = 1;
        }
    }

    if (hf_active)
    {
        audio_hfp_set_mic_mute();
    }
}

void audio_hfp_init(void)
{
    audio_mgr_cback_register(audio_voice_callback);
}
