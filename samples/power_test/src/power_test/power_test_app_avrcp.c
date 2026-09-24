/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#include "string.h"
#include "trace.h"
#include "btm.h"
#include "bt_avrcp.h"
#include "power_test_link.h"
#include "power_test_app_avrcp.h"

static void app_power_test_avrcp_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    T_POWER_TEST_LINK *p_link;
    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_AVRCP_CONN_IND:
        {
            p_link = power_test_find_link(param->avrcp_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                bt_avrcp_connect_cfm(p_link->bd_addr, true);
            }
        }
        break;

    case BT_EVENT_AVRCP_BROWSING_CONN_IND:
        {
            p_link = power_test_find_link(param->avrcp_browsing_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                bt_avrcp_browsing_connect_cfm(p_link->bd_addr, true);
            }
        }
        break;

    case BT_EVENT_AVRCP_GET_CAPABILITIES_RSP:
        {
            p_link = power_test_find_link(param->avrcp_browsing_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                uint8_t  capability_count;
                uint8_t *capabilities;

                capability_count = param->avrcp_get_capabilities_rsp.capability_count;
                capabilities = param->avrcp_get_capabilities_rsp.capabilities;
                while (capability_count != 0)
                {
                    bt_avrcp_register_notification_req(p_link->bd_addr, *capabilities);
                    capability_count -= 1;
                    capabilities += 1;
                }
            }
        }
        break;

    case BT_EVENT_AVRCP_ABSOLUTE_VOLUME_SET:
        {
        }
        break;

    case BT_EVENT_AVRCP_VOLUME_UP:
        {
        }
        break;

    case BT_EVENT_AVRCP_VOLUME_DOWN:
        {
        }
        break;

    case BT_EVENT_AVRCP_REG_VOLUME_CHANGED:
        {
        }
        break;

    case BT_EVENT_AVRCP_CONN_CMPL:
        {
        }
        break;

    case BT_EVENT_AVRCP_DISCONN_CMPL:
        {
        }
        break;

    case BT_EVENT_AVRCP_PLAY_STATUS_RSP:
    case BT_EVENT_AVRCP_PLAY_STATUS_CHANGED:
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
        APP_PRINT_INFO1("app_power_test_avrcp_bt_cback: event_type 0x%04x", event_type);
    }
}

void app_power_test_avrcp_init(void)
{
    bt_avrcp_init(BT_AVRCP_FEATURE_CATEGORY_1, BT_AVRCP_FEATURE_CATEGORY_2);
    bt_mgr_cback_register(app_power_test_avrcp_bt_cback);
}

