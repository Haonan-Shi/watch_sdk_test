/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_audio_mode_switch.h"
#include "trace.h"
#include "app_timer.h"
#include "app_linkback.h"
#include "app_bond.h"
#include "app_mmi.h"
#include "app_bt_policy_int.h"
#include "app_link_util.h"
#include "app_main.h"
#include "app_audio_if.h"
#include "app_cfg.h"
#include "app_sdp.h"
#include "event_bus.h"

bool is_audio_mode_switch = false;
bool is_audio_mode_switch_disconnect = false;
static uint8_t switch_to_mode = 0;

void app_audio_mode_switch_status_set(bool is_switch)
{
    is_audio_mode_switch = is_switch;
    event_bus_publish(EVENT_BUS_TOPIC_AUDIO_MODE, &app_db.audio_play_mode,
                      sizeof(app_db.audio_play_mode));
}

bool app_audio_mode_switch_status_get(void)
{
    APP_PRINT_INFO1("app_audio_mode_switch_status_get is_audio_mode_switch = %d", is_audio_mode_switch);
    return is_audio_mode_switch;
}

uint8_t app_audio_mode_switch_mode_get(void)
{
    return switch_to_mode;
}

void app_audio_mode_switch_linkback(void)
{
    if (!app_db.ble_is_ready || !app_db.bt_is_ready)
    {
        APP_PRINT_WARN0("app_audio_mode_switch_linkback bt and ble not ready !!!");
        return;
    }
    if (app_db.a2dp_cur_role == BT_A2DP_ROLE_SRC && app_db.bond_device[1].exist_addr_flag)
    {
        linkback_earphone_create_connection(app_db.bond_device[1].bd_addr);
    }
    if (app_db.bond_device[0].exist_addr_flag)
    {
        linkback_phone_create_connection(app_db.bond_device[0].bd_addr);
    }
    app_audio_mode_switch_status_set(false);
}

bool app_audio_mode_switch_disconnect(void)
{
    bool ret = false;

    if (is_audio_mode_switch_disconnect == false)
    {
        if ((app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_PHONE) != 0xff) ||
            (app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_EARPHONE) != 0xff))
        {
            ret = true;
            is_audio_mode_switch_disconnect = true;

            for (uint8_t i = 0; i < MAX_BR_LINK_NUM; i++)
            {
                app_bt_policy_discon_all_profiles_by_addr(app_db.br_link[i].bd_addr);
            }
        }
    }

    APP_PRINT_INFO1("app_audio_mode_switch_disconnect ret = %d", ret);

    return ret;
}

void app_audio_mode_switch_role(void)
{
    APP_PRINT_INFO1("app_audio_mode_switch_role a2dp_cur_role %d", app_db.a2dp_cur_role);

    if (app_db.a2dp_cur_role == BT_A2DP_ROLE_SRC)
    {
        app_sdp_a2dp_record_reinit(BT_A2DP_ROLE_SNK);
        if (app_cfg_const.a2dp_codec_type_sbc)
        {
            T_BT_A2DP_STREAM_ENDPOINT sep;
            sep.role = BT_A2DP_ROLE_SNK;
            sep.codec_type = BT_A2DP_CODEC_TYPE_SBC;
            sep.u.codec_sbc.sampling_frequency_mask = BT_A2DP_SBC_SAMPLING_FREQUENCY_44_1KHZ |
                                                      BT_A2DP_SBC_SAMPLING_FREQUENCY_48KHZ;
            sep.u.codec_sbc.channel_mode_mask = app_cfg_const.sbc_channel_mode;
            sep.u.codec_sbc.block_length_mask = app_cfg_const.sbc_block_length;
            sep.u.codec_sbc.subbands_mask = app_cfg_const.sbc_subbands;
            sep.u.codec_sbc.allocation_method_mask = app_cfg_const.sbc_allocation_method;
            sep.u.codec_sbc.min_bitpool = app_cfg_const.sbc_min_bitpool;
            sep.u.codec_sbc.max_bitpool = app_cfg_const.sbc_max_bitpool;
            bt_a2dp_stream_endpoint_add(sep);
        }
        if (app_cfg_const.a2dp_codec_type_aac)
        {
            T_BT_A2DP_STREAM_ENDPOINT sep;
            sep.role = BT_A2DP_ROLE_SNK;
            sep.codec_type = BT_A2DP_CODEC_TYPE_AAC;
            sep.u.codec_aac.object_type_mask = app_cfg_const.aac_object_type;
            sep.u.codec_aac.sampling_frequency_mask = app_cfg_const.aac_sampling_frequency;
            sep.u.codec_aac.channel_number_mask = app_cfg_const.aac_channel_number;
            sep.u.codec_aac.vbr_supported = app_cfg_const.aac_vbr_supported;
            sep.u.codec_aac.bit_rate = app_cfg_const.aac_bit_rate;
            bt_a2dp_stream_endpoint_add(sep);
        }

        app_db.a2dp_cur_role = BT_A2DP_ROLE_SNK;
        app_db.audio_play_mode = MODE_APP_A2DP_SNK;
    }
    else if (app_db.a2dp_cur_role == BT_A2DP_ROLE_SNK)
    {
        app_sdp_a2dp_record_reinit(BT_A2DP_ROLE_SRC);

        T_BT_A2DP_STREAM_ENDPOINT sep;
        sep.role = BT_A2DP_ROLE_SRC;
        sep.codec_type = BT_A2DP_CODEC_TYPE_SBC;
        sep.u.codec_sbc.sampling_frequency_mask = BT_A2DP_SBC_SAMPLING_FREQUENCY_48KHZ;
        sep.u.codec_sbc.channel_mode_mask = app_cfg_const.sbc_channel_mode;
        sep.u.codec_sbc.block_length_mask = app_cfg_const.sbc_block_length;
        sep.u.codec_sbc.subbands_mask = app_cfg_const.sbc_subbands;
        sep.u.codec_sbc.allocation_method_mask = app_cfg_const.sbc_allocation_method;
        sep.u.codec_sbc.min_bitpool = app_cfg_const.sbc_min_bitpool;
        sep.u.codec_sbc.max_bitpool = app_cfg_const.sbc_max_bitpool;
        bt_a2dp_stream_endpoint_add(sep);

        app_db.a2dp_cur_role = BT_A2DP_ROLE_SRC;
        playback_db.source_buffer_state = APP_AUDIO_FS_BUF_LOW;
        playback_db.op_next_action = APP_AUDIO_STOPPED_IDLE_ACTION;
        playback_db.sd_play_state = APP_AUDIO_STATE_STOP;
        app_db.audio_play_mode = MODE_APP_A2DP_SRC;
    }
}

void app_audio_mode_switch_to_playback(void)
{
    if (app_audio_mode_switch_disconnect() == false)
    {
        playback_db.sd_play_state = APP_AUDIO_STATE_STOP;

        app_db.audio_play_mode = MODE_APP_PLAYBACK;
        playback_db.op_next_action = APP_AUDIO_STOPPED_IDLE_ACTION;

        if (app_db.bt_state != STATE_INIT)
        {
            uint8_t app_bond_phone_index = app_bt_bond_check_exist_device_info(T_DEVICE_TYPE_PHONE);
            if (app_bond_phone_index != 0xff)
            {
                linkback_phone_create_connection(app_db.bond_device[app_bond_phone_index].bd_addr);
            }
        }
        app_audio_mode_switch_status_set(false);
    }
}

void app_audio_mode_switch_to_src(void)
{
    if (app_db.a2dp_cur_role == BT_A2DP_ROLE_SNK)
    {
        if (app_audio_mode_switch_disconnect() == false)
        {
            app_audio_mode_switch_role();
        }
    }
    else
    {
        playback_db.source_buffer_state = APP_AUDIO_FS_BUF_LOW;
        app_db.audio_play_mode = MODE_APP_A2DP_SRC;
        if (app_bt_bond_check_exist_device_info(T_DEVICE_TYPE_EARPHONE) != 0xff)
        {
            app_audio_mode_switch_linkback();
        }
        else
        {
            app_audio_mode_switch_status_set(false);
        }
    }
}

void app_audio_mode_switch_to_sink(void)
{
    app_db.a2dp_control_switch = true;// Can be configured separately

    if (app_db.a2dp_cur_role == BT_A2DP_ROLE_SRC)
    {
        if (app_audio_mode_switch_disconnect() == false)
        {
            app_audio_mode_switch_role();
        }
    }
    else
    {
        app_db.audio_play_mode = MODE_APP_A2DP_SNK;
        if (app_bt_bond_check_exist_device_info(T_DEVICE_TYPE_PHONE) != 0xff)
        {
            app_audio_mode_switch_linkback();
        }
        else
        {
            app_audio_mode_switch_status_set(false);
        }
    }
}

void app_audio_mode_switch(uint8_t new_mode)
{
    APP_PRINT_INFO4("app_audio_mode_switch from 0x%x to 0x%x, support_sink %d, support_local_source %d",
                    app_db.audio_play_mode, new_mode, app_audio_cfg.support_sink, app_audio_cfg.support_local_source);

    switch_to_mode = new_mode;
    app_audio_mode_switch_status_set(true);

    if ((app_db.audio_play_mode == new_mode)
        || ((new_mode == MODE_APP_A2DP_SNK) && (!app_audio_cfg.support_sink))
        || (((new_mode == MODE_APP_PLAYBACK) || (new_mode == MODE_APP_A2DP_SRC)) && \
            (!app_audio_cfg.support_local_source)))
    {
        app_audio_mode_switch_status_set(false);
        return;
    }

    if (app_db.bt_state == STATE_LINKBACK)
    {
        linkback_cancel_connection(true, true);
        return;
    }

    if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
    {
        app_audio_pause();
    }

    switch (app_db.audio_play_mode)
    {
    case MODE_APP_PLAYBACK:
        {
            if (new_mode == MODE_APP_A2DP_SRC)
            {
                app_audio_mode_switch_to_src();
            }
            else if (new_mode == MODE_APP_A2DP_SNK)
            {
                app_audio_mode_switch_to_sink();
            }
        }
        break;

    case MODE_APP_A2DP_SRC:
        {
            if (new_mode == MODE_APP_PLAYBACK)
            {
                app_audio_mode_switch_to_playback();
            }
            else if (new_mode == MODE_APP_A2DP_SNK)
            {
                app_audio_mode_switch_to_sink();
            }
        }
        break;

    case MODE_APP_A2DP_SNK:
        {
            if (new_mode == MODE_APP_PLAYBACK)
            {
                app_audio_mode_switch_to_playback();
            }
            else if (new_mode == MODE_APP_A2DP_SRC)
            {
                app_audio_mode_switch_to_src();
            }
        }
        break;

    case MODE_NONE:
        {
            if (new_mode == MODE_APP_PLAYBACK)
            {
                app_audio_mode_switch_to_playback();
            }
            else if (new_mode == MODE_APP_A2DP_SRC)
            {
                app_audio_mode_switch_to_src();
            }
            else if (new_mode == MODE_APP_A2DP_SNK)
            {
                app_audio_mode_switch_to_sink();
            }
        }
        break;

    default:
        break;
    }
}

static void app_audio_mode_switch_cback(T_BT_EVENT event_type, void *event_buf,
                                        uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_ACL_CONN_DISCONN:
        {
            if ((app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_PHONE) == 0xff) &&
                (app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_EARPHONE) == 0xff))
            {
                is_audio_mode_switch_disconnect = false;

                if (app_audio_mode_switch_status_get() && (switch_to_mode != MODE_NONE))
                {
                    if (switch_to_mode == MODE_APP_A2DP_SRC)
                    {
                        app_audio_mode_switch_to_src();
                    }
                    else if (switch_to_mode == MODE_APP_A2DP_SNK)
                    {
                        app_audio_mode_switch_to_sink();
                    }
                    else if (switch_to_mode == MODE_APP_PLAYBACK)
                    {
                        app_audio_mode_switch_to_playback();
                    }
                }
            }
        }
        break;

    case BT_EVENT_ACL_CONN_SUCCESS:
        {
            //UI
        }
        break;

    case BT_EVENT_A2DP_CONN_CMPL:
        {
            if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
            {
                app_audio_pause();
            }
        }
        break;

    case BT_EVENT_A2DP_CONN_FAIL:
    case BT_EVENT_A2DP_DISCONN_CMPL:
        {
            //UI
        }
        break;

    case BT_EVENT_ADD_SDP_RECORD_RSP:
        {
            if (((param->sdp_add_record.cause == 0) && app_audio_mode_switch_status_get())//switching mode
                || ((app_db.audio_play_mode == MODE_APP_A2DP_SNK) &&
                    (!app_audio_cfg.support_local_source))) //sink only
            {
                app_audio_mode_switch_linkback();
            }
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_audio_mode_switch_cback  event_type = 0x%x", event_type);
    }
}

void app_audio_mode_switch_init(void)
{
    bt_mgr_cback_register(app_audio_mode_switch_cback);
}
