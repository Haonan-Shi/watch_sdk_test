/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include "app_ble_adv.h"
#include "gap_le_types.h"
#include "gap.h"
#include "string.h"
#include "gap_le.h"
#include "ble_ext_adv.h"
#include "os_queue.h"
#include "ble_mgr.h"
#include "app_ble_gap.h"
#include "ble_scan.h"
#include "gap_conn_le.h"
#include "walkie_talkie_scan.h"
#include "walkie_talkie_adv.h"
#include "trace.h"
#include "os_mem.h"
#include "event_bus.h"
#include "walkie_talkie_voice.h"
#include "walkie_talkie_gatt_svc.h"
#include "walkie_talkie_app.h"

BLE_SCAN_HDL walkie_talkie_scan_hdl = NULL;
static uint8_t data_map[32] = {0};
static uint8_t last_num = 0;

void walkie_talkie_scan_lost_clear(void)
{
    memset(data_map, 0, sizeof(data_map));
    last_num = 0;
}

void walkie_talkie_scan_lost_analyzer(uint8_t last_index, uint8_t cur_index, uint8_t cur_num)
{
    while (cur_index != last_index)
    {
        if (last_num > 0)
        {
            data_map[last_index / 8] &= ~BIT(last_index % 8);
            last_num--;
        }
        else
        {
            data_map[last_index / 8] |= BIT(last_index % 8);
        }
        last_index++;
    }
    last_num = cur_num;

    uint16_t lost_num = 0;
    for (uint8_t i = 0; i < 32; i++)
    {
        if (data_map[i])
        {
            for (uint8_t j = 0; j < 8; j++)
            {
                if (data_map[i] & BIT(j))
                {
                    lost_num++;
                }
            }
        }
    }
    APP_PRINT_INFO1("walkie talkie lost num %d in recent 256 packets", lost_num);
}

bool walkie_talkie_receiver_working(void)
{
    return false;
}

void walkie_talkie_scan_cb(BLE_SCAN_EVT evt, BLE_SCAN_EVT_DATA *p_data)
{
    switch (evt)
    {
    case BLE_SCAN_REPORT:
        {
            if (walkie_talkie_cfg.mode == WALKIE_TALKIE_ADV)
            {
                walkie_talkie_player_data_parser(p_data->report->p_data, p_data->report->data_len);
            }
            else if (walkie_talkie_cfg.mode == WALKIE_TALKIE_CONN)
            {
                T_WALKIE_TALKIE_DEV_INFO dev_info;
                memset(&dev_info, 0, sizeof(T_WALKIE_TALKIE_DEV_INFO));
                memcpy(dev_info.addr, p_data->report->bd_addr, 6);
                memcpy(dev_info.dev_name, p_data->report->p_data + USER_NAME_OFFSET, USER_NAME_LEN);
                APP_PRINT_INFO1("walkie_talkie_scan_cb: receive adv from %s", TRACE_STRING(dev_info.dev_name));
                event_bus_publish(EVENT_BUS_TOPIC_WALKIE_TALKIE_SCAN_REPORT, &dev_info,
                                  sizeof(T_WALKIE_TALKIE_DEV_INFO));
            }
        }
        break;
    default:
        break;
    }
}

T_GAP_CAUSE walkie_talkie_scan_connect(uint8_t *bd_addr)
{
    //set transmit_adv role and remote_bd for connection
    extern T_LE_ADV_CONN transmit_adv;
    transmit_adv.role = GAP_LINK_ROLE_MASTER;
    memcpy(transmit_adv.remote_bd, bd_addr, 6);

    T_GAP_LE_CONN_REQ_PARAM conn_req_param;

    conn_req_param.scan_interval = 0x10;
    conn_req_param.scan_window = 0x10;
    conn_req_param.conn_interval_min = 20;
    conn_req_param.conn_interval_max = 25;
    conn_req_param.conn_latency = 0;
    conn_req_param.supv_tout = 1000;
    conn_req_param.ce_len_min = 2 * (conn_req_param.conn_interval_min - 1);
    conn_req_param.ce_len_max = 2 * (conn_req_param.conn_interval_max - 1);
    le_set_conn_param(GAP_CONN_PARAM_CODED, &conn_req_param);

    T_GAP_CAUSE cause = le_connect(GAP_PHYS_CONN_INIT_CODED_BIT, bd_addr,
                                   GAP_REMOTE_ADDR_LE_PUBLIC,
                                   GAP_LOCAL_ADDR_LE_PUBLIC,
                                   1000);
    return cause;
}


bool walkie_talkie_scan_start(void)
{
    bool res = true;

    if (walkie_talkie_scan_hdl)
    {
        return res;
    }

    BLE_SCAN_PARAM param;
    BLE_SCAN_FILTER scan_filter;
    uint8_t ad_struct[17] = {GAP_ADTYPE_128BIT_MORE, GATT_UUID128_WALKIE_TALKIE_SERVICE};
    memset(&param, 0, sizeof(param));
    memset(&scan_filter, 0, sizeof(scan_filter));

    param.scan_param_coded.scan_type = GAP_SCAN_MODE_ACTIVE;
    param.scan_param_coded.scan_interval = 160;
    param.scan_param_coded.scan_window = 80;
    param.scan_param_1m.scan_type = GAP_SCAN_MODE_ACTIVE;
    param.scan_param_1m.scan_interval = 160;
    param.scan_param_1m.scan_window = 80;
    param.ext_filter_duplicate = GAP_SCAN_FILTER_DUPLICATE_DISABLE;
    param.ext_filter_policy = GAP_SCAN_FILTER_ANY;
    param.own_addr_type = GAP_LOCAL_ADDR_LE_PUBLIC;
    param.phys = GAP_EXT_SCAN_PHYS_CODED_BIT;

    scan_filter.filter_flags = BLE_SCAN_FILTER_ADV_DATA_BIT;//(BLE_SCAN_FILTER_ADV_DATA_BIT);

    //filter advertising data
    scan_filter.ad_len = 17;
    scan_filter.ad_struct = ad_struct;
    scan_filter.addr_num = 0;
    scan_filter.p_scan_addr = NULL;

    //filter ADV event property
    scan_filter.evt_type = LE_EXT_ADV_EXTENDED_ADV_NON_SCAN_NON_CONN_UNDIRECTED;

    res = ble_scan_start(&walkie_talkie_scan_hdl, walkie_talkie_scan_cb, &param, &scan_filter);
    return res;
}

bool walkie_talkie_scan_stop(void)
{
    walkie_talkie_scan_lost_clear();
    return ble_scan_stop(&walkie_talkie_scan_hdl);
}
