/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "btm.h"
#include "bt_a2dp.h"
#include "power_test_link.h"
#include "power_test_app_a2dp.h"

static void app_power_test_a2dp_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    T_POWER_TEST_LINK *p_link;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_A2DP_CONN_IND:
        {
            p_link = power_test_find_link(param->a2dp_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                bt_a2dp_connect_cfm(p_link->bd_addr, 0, true);
            }
        }
        break;

    case BT_EVENT_A2DP_CONFIG_CMPL:
        {
            p_link = power_test_find_link(param->a2dp_config_cmpl.bd_addr);
            if (p_link != NULL)
            {
                if (param->a2dp_config_cmpl.role == BT_A2DP_ROLE_SNK)
                {
                    bt_a2dp_stream_delay_report_req(param->a2dp_config_cmpl.bd_addr, 200);
                }
            }
        }
        break;

    case BT_EVENT_A2DP_SNIFFING_CONN_CMPL:
        {
        }
        break;

    case BT_EVENT_A2DP_STREAM_OPEN:
        {
        }
        break;

    case BT_EVENT_A2DP_STREAM_START_IND:
        {
        }
        break;

    case BT_EVENT_A2DP_STREAM_START_RSP:
        {
        }
        break;

    case BT_EVENT_A2DP_STREAM_STOP:
        {
        }
        break;

    case BT_EVENT_A2DP_STREAM_CLOSE:
        {
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
        APP_PRINT_INFO1("app_power_test_a2dp_bt_cback: event_type 0x%04x", event_type);
    }
}

void app_power_test_a2dp_init(void)
{
    bt_a2dp_init(BT_A2DP_CAPABILITY_MEDIA_TRANSPORT | BT_A2DP_CAPABILITY_MEDIA_CODEC |
                 BT_A2DP_CAPABILITY_DELAY_REPORTING);

    T_BT_A2DP_STREAM_ENDPOINT sep;

    sep.role = BT_A2DP_ROLE_SNK;
    sep.codec_type = BT_A2DP_CODEC_TYPE_SBC;
    sep.u.codec_sbc.sampling_frequency_mask = 0xF0;
    sep.u.codec_sbc.channel_mode_mask = 0xF;
    sep.u.codec_sbc.block_length_mask = 0xF0;
    sep.u.codec_sbc.subbands_mask = 0xC;
    sep.u.codec_sbc.allocation_method_mask = 0x3;
    sep.u.codec_sbc.min_bitpool = 2;
    sep.u.codec_sbc.max_bitpool = 53;

    bt_a2dp_stream_endpoint_add(sep);

    T_BT_A2DP_STREAM_ENDPOINT sep_acc;

    sep_acc.role = BT_A2DP_ROLE_SNK;
    sep_acc.codec_type = BT_A2DP_CODEC_TYPE_AAC;
    sep_acc.u.codec_aac.object_type_mask = 0x1C;
    sep_acc.u.codec_aac.sampling_frequency_mask = 0x180;
    sep_acc.u.codec_aac.channel_number_mask = 12;
    sep_acc.u.codec_aac.vbr_supported = 1;
    sep_acc.u.codec_aac.bit_rate = 256000;
    bt_a2dp_stream_endpoint_add(sep_acc);
    bt_mgr_cback_register(app_power_test_a2dp_bt_cback);
}
