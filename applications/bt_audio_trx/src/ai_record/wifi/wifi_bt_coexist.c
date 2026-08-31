/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sd/sd.h>
#include <zephyr/sd/sdio.h>
#include "trace.h"
#include "wifi_sdio.h"
#include <os_msg.h>
#include <os_task.h>
#include <os_sched.h>
#include "btm.h"
#include "bt_hfp.h"
#include "bt_a2dp.h"
//#include "app_link_util.h"

// #include "gap_lib_common.h"
// typedef struct HCI_EXT_SET_WIFI_CHANNEL_MAP_CMD_PARAM_
// {
//     uint8_t channel_map[10];
// } HCI_EXT_SET_WIFI_CHANNEL_MAP_CMD_PARAM;
// channel map setting
// HCI_EXT_SET_WIFI_CHANNEL_MAP_CMD_PARAM param;
// param.channel_map[0] = 0xFF;
// param.channel_map[1] = 0xFF;
// param.channel_map[2] = 0x0F;
// param.channel_map[3] = 0x00;
// param.channel_map[4] = 0x00;
// param.channel_map[5] = 0x00;
// param.channel_map[6] = 0x00;
// param.channel_map[7] = 0x00;
// param.channel_map[8] = 0x00;
// param.channel_map[9] = 0x00;
// gap_vendor_cmd_req(0xFDD6, sizeof(param), (uint8_t *)&param);

static void wifi_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
#if 0
    T_BT_EVENT_PARAM *param = event_buf;
    T_APP_BR_LINK *p_link;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_A2DP_STREAM_START_IND:
        {
            // wifi_atcmd_sleep_mode();
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

    case BT_EVENT_HFP_CALL_STATUS:
        {
            // if(param->hfp_call_status.curr_status != BT_HFP_CALL_IDLE)
            // {
            //     wifi_atcmd_sleep_mode();
            // }
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
        APP_PRINT_INFO1("wifi_bt_cback: event_type 0x%04x", event_type);
    }
#endif
}

int wifi_bt_coexist_init(void)
{

    //bt_mgr_cback_register(wifi_bt_cback);
    return 0;
}

