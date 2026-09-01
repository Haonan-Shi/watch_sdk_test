/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include "trace.h"
#include "os_mem.h"
#include "btm.h"
#include "power_test_sdp.h"
#include "power_test_app_hfp.h"

static void app_power_test_hfp_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    bool handle = true;
    T_POWER_TEST_LINK *p_link;

    switch (event_type)
    {
    case BT_EVENT_HFP_CONN_IND:
        {
            p_link = power_test_find_link(param->hfp_conn_ind.bd_addr);
            if (p_link == NULL)
            {
                APP_PRINT_ERROR0("app_power_test_hfp_bt_cback: no acl link found");
                return;
            }
            bt_hfp_connect_cfm(p_link->bd_addr, true);
        }
        break;

    case BT_EVENT_HFP_CONN_CMPL:
        {
        }
        break;

    case BT_EVENT_HFP_CALL_STATUS:
        {
        }
        break;

    case BT_EVENT_HFP_SERVICE_STATUS:
        {
        }
        break;

    case BT_EVENT_HFP_CALL_WAITING_IND:
    case BT_EVENT_HFP_CALLER_ID_IND:
        {
        }
        break;

    case BT_EVENT_HFP_RING_ALERT:
        {
        }
        break;

    case BT_EVENT_HFP_SPK_VOLUME_CHANGED:
        {
        }
        break;

    case BT_EVENT_HFP_MIC_VOLUME_CHANGED:
        {
        }
        break;

    case BT_EVENT_HFP_DISCONN_CMPL:
        {
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_power_test_hfp_bt_cback: event_type 0x%04x", event_type);
    }
}

void app_power_test_hfp_init(void)
{
    bt_hfp_init(RFC_HFP_CHANN_NUM, RFC_HSP_CHANN_NUM, 0x2BF,
                BT_HFP_HF_CODEC_TYPE_CVSD | BT_HFP_HF_CODEC_TYPE_MSBC);

    bt_mgr_cback_register(app_power_test_hfp_bt_cback);
}
