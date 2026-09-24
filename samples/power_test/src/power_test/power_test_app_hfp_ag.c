/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "btm.h"
#include "trace.h"
#include "bt_bond.h"
#include "gap_br.h"
#include "bt_hfp_ag.h"
#include "power_test_link.h"
#include "power_test_sdp.h"

static void power_test_handle_sdp_discovery_info(uint8_t *bd_addr, T_BT_SDP_ATTR_INFO *sdp_attr);
static void power_test_handle_sdp_discovery_cmpl(uint8_t *bd_addr, uint16_t cause);

static void app_app_power_test_hfp_ag(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    T_POWER_TEST_LINK *p_link;

    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_SDP_ATTR_INFO:
        {
            power_test_handle_sdp_discovery_info(param->sdp_attr_info.bd_addr,
                                                 &(param->sdp_attr_info.info));
        }
        break;

    case BT_EVENT_SDP_DISCOV_CMPL:
        {
            power_test_handle_sdp_discovery_cmpl(param->sdp_discov_cmpl.bd_addr,
                                                 param->sdp_discov_cmpl.cause);
        }
        break;

    case BT_EVENT_HFP_AG_CONN_IND:
        {
            p_link = power_test_find_link(param->hfp_ag_conn_ind.bd_addr);
            if (p_link == NULL)
            {
                APP_PRINT_ERROR0("power_test_bt_cback: no acl link found");
                return;
            }

            bt_hfp_ag_connect_cfm(p_link->bd_addr, true);
        }
        break;

    case BT_EVENT_HFP_AG_CONN_CMPL:
        {
            APP_PRINT_INFO1("power_test_bt_cback BT_EVENT_HFP_AG_CONN_CMPL is_hfp:%d",
                            param->hfp_ag_conn_cmpl.is_hfp);
            p_link = power_test_find_link(param->hfp_ag_conn_cmpl.bd_addr);
            if (p_link != NULL)
            {
                if (param->hfp_ag_conn_cmpl.is_hfp)
                {
                    p_link->connected_profile |= HFP_PROFILE_MASK;
                }
                else
                {
                    p_link->connected_profile |= HSP_PROFILE_MASK;
                }
            }
        }
        break;

    case BT_EVENT_HFP_AG_DISCONN_CMPL:
        {
            p_link = power_test_find_link(param->hfp_ag_disconn_cmpl.bd_addr);
            if (p_link != NULL)
            {
                if (p_link->connected_profile & HFP_PROFILE_MASK)
                {
                    p_link->connected_profile &= ~HFP_PROFILE_MASK;
                }
                else
                {
                    p_link->connected_profile &= ~HSP_PROFILE_MASK;
                }
            }
        }
        break;

    case BT_EVENT_HFP_AG_INDICATORS_STATUS_REQ:
        {
            p_link = power_test_find_link(param->hfp_ag_indicators_status_req.bd_addr);
            if (p_link != NULL)
            {
                //Td App provide current network status.
                T_BT_HFP_AG_SERVICE_INDICATOR service_indicator = BT_HFP_AG_SERVICE_STATUS_AVAILABLE;
                T_BT_HFP_AG_CALL_INDICATOR call_indicator = BT_HFP_AG_NO_CALL_IN_PROGRESS;
                T_BT_HFP_AG_CALL_SETUP_INDICATOR call_setup_indicator = BT_HFP_AG_CALL_SETUP_STATUS_IDLE;
                T_BT_HFP_AG_CALL_HELD_INDICATOR call_held_indicator = BT_HFP_AG_CALL_HELD_STATUS_IDLE;

                //Td App provide current signal status.
                uint8_t signal_indicator = 5;
                //Td App provide current roaming status.
                T_BT_HFP_AG_ROAMING_INDICATOR roaming_indicator = BT_HFP_AG_ROAMING_STATUS_ACTIVE;
                //Td App provide current battery status.
                uint8_t batt_chg_indicator = 5;
                bt_hfp_ag_indicators_send(p_link->bd_addr,
                                          service_indicator,
                                          call_indicator,
                                          call_setup_indicator,
                                          call_held_indicator,
                                          signal_indicator,
                                          roaming_indicator,
                                          batt_chg_indicator);
                bt_hfp_ag_ok_send(param->hfp_ag_indicators_status_req.bd_addr);
            }
        }
        break;

    case BT_EVENT_HFP_AG_CURR_CALLS_LIST_QUERY:
        {
            p_link = power_test_find_link(param->hfp_ag_curr_calls_list_query.bd_addr);
            if (p_link != NULL)
            {
                bt_hfp_ag_ok_send(param->hfp_ag_curr_calls_list_query.bd_addr);
            }
        }
        break;

    case BT_EVENT_HFP_AG_VENDOR_CMD:
        {
            bt_hfp_ag_error_send(param->hfp_ag_vendor_cmd.bd_addr, BT_HFP_AG_ERR_INVALID_CHAR_IN_TSTR);
        }
        break;

    default:
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_app_power_test_hfp_ag: event_type 0x%04x", event_type);
    }
}

bool power_test_connect_hf_ag(uint8_t *bd_addr)
{
    T_GAP_UUID_DATA uuid;

    uuid.uuid_16 = UUID_HANDSFREE;

    if (gap_br_start_sdp_discov(bd_addr, GAP_UUID16, uuid) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }

    return false;
}

bool power_test_connect_hs_ag(uint8_t *bd_addr)
{
    T_GAP_UUID_DATA uuid;

    uuid.uuid_16 = UUID_HEADSET;

    if (gap_br_start_sdp_discov(bd_addr, GAP_UUID16, uuid) == GAP_CAUSE_SUCCESS)
    {
        return true;
    }

    return false;
}

bool app_power_test_connect_hfp_ag(uint8_t *bd_addr, bool is_hfp)
{
    T_POWER_TEST_LINK *p_link;
    bool ret = false;

    p_link = power_test_find_link(bd_addr);
    bt_bond_delete(bd_addr);

    if (p_link == NULL)
    {
        p_link = power_test_alloc_link(bd_addr);
    }

    if (p_link != NULL)
    {
        if (is_hfp)
        {
            //HANDSFREE profile SDP.
            if ((p_link->connected_profile & HFP_PROFILE_MASK) == 0)
            {
                ret = power_test_connect_hf_ag(p_link->bd_addr);
                if (ret == true)
                {
                    p_link->sdp_hfp_ag_hf_record_num = 0;
                    p_link->sdp_active_inquire_profile = HFP_PROFILE_MASK;
                }
            }
        }
        else
        {
            //HEADSET profile SDP.
            if ((p_link->connected_profile & HSP_PROFILE_MASK) == 0)
            {
                ret = power_test_connect_hs_ag(p_link->bd_addr);
                if (ret == true)
                {
                    p_link->sdp_hfp_ag_hs_record_num = 0;
                    p_link->sdp_active_inquire_profile = HSP_PROFILE_MASK;
                }
            }
        }
    }

    return ret;
}

bool app_power_test_disconnect_hfp_ag(uint8_t *bd_addr, bool is_hfp)
{
    T_POWER_TEST_LINK *p_link;
    bool ret = false;

    p_link = power_test_find_link(bd_addr);

    if (p_link == NULL)
    {
        p_link = power_test_alloc_link(bd_addr);
    }

    if (p_link != NULL)
    {
        if (is_hfp)
        {
            //HANDSFREE profile disconnect.
            if ((p_link->connected_profile & HFP_PROFILE_MASK) != 0)
            {
                ret = bt_hfp_ag_disconnect_req(bd_addr);
            }
        }
        else
        {
            //HEADSET profile disconnect.
            if ((p_link->connected_profile & HSP_PROFILE_MASK) != 0)
            {
                ret = bt_hfp_ag_disconnect_req(bd_addr);
            }
        }
    }

    return ret;
}

void power_test_handle_sdp_discovery_info(uint8_t *bd_addr, T_BT_SDP_ATTR_INFO *sdp_attr)
{
    T_POWER_TEST_LINK *p_link;
    T_BT_SDP_ATTR_INFO *p_info = (T_BT_SDP_ATTR_INFO *)sdp_attr;

    p_link = power_test_find_link(bd_addr);
    if (p_link != NULL)
    {
        if (p_link->sdp_active_inquire_profile == HFP_PROFILE_MASK &&
            p_info->srv_class_uuid_data.uuid_16 == UUID_HANDSFREE)
        {
            if (bt_hfp_ag_connect_req(bd_addr, p_info->server_channel, true) == true)
            {
                p_link->sdp_hfp_ag_hf_record_num++;
            }
        }
        else if (p_link->sdp_active_inquire_profile == HSP_PROFILE_MASK &&
                 p_info->srv_class_uuid_data.uuid_16 == UUID_HEADSET)
        {
            if (bt_hfp_ag_connect_req(bd_addr, p_info->server_channel, false) == true)
            {
                p_link->sdp_hfp_ag_hs_record_num++;
            }
        }
    }
}

void power_test_handle_sdp_discovery_cmpl(uint8_t *bd_addr, uint16_t cause)
{
    T_POWER_TEST_LINK *p_link;

    p_link = power_test_find_link(bd_addr);
    if (p_link != NULL && cause == 0)
    {
        if (p_link->sdp_active_inquire_profile == HFP_PROFILE_MASK)
        {
            if (p_link->sdp_hfp_ag_hf_record_num != 0)
            {
                //HANDSFREE SDP result is not null, just connect HANDSFREE profile and skip HEADSET SDP and connect.
                p_link->sdp_hfp_ag_hf_record_num = 0;
            }
            else
            {
                //HANDSFREE SDP info is NULL, then try to HEADSET SDP.
                p_link->sdp_active_inquire_profile = HSP_PROFILE_MASK;
                power_test_connect_hs_ag(p_link->bd_addr);
            }
        }
        else if (p_link->sdp_active_inquire_profile == HSP_PROFILE_MASK)
        {
            //HEADSET SDP info is NULL, Terminate SDP.
            p_link->sdp_hfp_ag_hs_record_num = 0;
        }
    }
}

void app_power_test_hfp_ag_init(void)
{
    bt_hfp_ag_init(RFC_HFP_AG_CHANN_NUM, RFC_HSP_AG_CHANN_NUM, 0xf69,
                   BT_HFP_AG_CODEC_TYPE_CVSD | BT_HFP_AG_CODEC_TYPE_MSBC);

    bt_mgr_cback_register(app_app_power_test_hfp_ag);
}
