/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "os_sched.h"
#include "app_timer.h"
#include "gap_br.h"
#include "gap_le.h"
#include "bt_avrcp.h"
#include "bt_spp.h"
#include "bt_bond.h"
#include "bt_rdtp.h"
#include "bt_hfp.h"
#include "app_cfg.h"
#include "app_main.h"
#include "app_ble_gap.h"
#include "app_hfp.h"
#include "app_linkback.h"
#include "app_multilink.h"
#include "app_mmi.h"
#include "app_bond.h"
#include "app_report.h"
#include "audio_type.h"
#include "app_sdp.h"
#include "app_hfp.h"
#include "app_bt_policy_int.h"
#include "app_task.h"
#include "app_linkback.h"
#include "app_bond.h"
#include "module_font.h"
#include "os_timer.h"
#include "app_msg_handle.h"
#include "app_bt_policy_api.h"
#include "app_audio_if.h"
#include "event_bus.h"
#include "app_avrcp.h"
#include "app_audio_mode_switch.h"
#include "app_dlps.h"
#include "dfu_transport.h"
#include "gui_common.h"

T_DEVICE_TYPE acl_conn_ind_device_type = T_DEVICE_TYPE_DEFAULT;

typedef void (*P_BT_STATE_CHANGED_CBACK)(T_STATE old_state, T_STATE new_state);
P_BT_STATE_CHANGED_CBACK p_bt_state_changed_cback = NULL;

void bt_state_changed_cback_register(P_BT_STATE_CHANGED_CBACK cback)
{
    p_bt_state_changed_cback = cback;
}

uint32_t get_profs_by_bond_flag(uint32_t bond_flag)
{
    uint32_t profs = 0;

    if ((T_LINKBACK_SCENARIO)app_cfg_const.link_scenario == LINKBACK_SCENARIO_HFP_BASE)
    {
        if (bond_flag & BOND_FLAG_HFP)
        {
            profs |= HFP_PROFILE_MASK;

            if (app_cfg_const.supported_profile_mask & PBAP_PROFILE_MASK)
            {
                profs |= PBAP_PROFILE_MASK;
            }
        }
        else if (bond_flag & BOND_FLAG_HSP)
        {
            profs |= HSP_PROFILE_MASK;

            if (app_cfg_const.supported_profile_mask & PBAP_PROFILE_MASK)
            {
                profs |= PBAP_PROFILE_MASK;
            }
        }

        if (bond_flag & BOND_FLAG_SPP)
        {
            if (app_cfg_const.supported_profile_mask & SPP_PROFILE_MASK)
            {
                profs |= SPP_PROFILE_MASK;
            }
        }
        else if (bond_flag & BOND_FLAG_IAP)
        {
            if (app_cfg_const.supported_profile_mask & IAP_PROFILE_MASK)
            {
                profs |= IAP_PROFILE_MASK;
            }
        }
        if (bond_flag & (BOND_FLAG_GATT))
        {
            if (app_cfg_const.supported_profile_mask & GATT_PROFILE_MASK)
            {
                profs |= GATT_PROFILE_MASK;
            }
        }
    }
    else if ((T_LINKBACK_SCENARIO)app_cfg_const.link_scenario == LINKBACK_SCENARIO_A2DP_BASE)
    {
        if (bond_flag & (BOND_FLAG_A2DP))
        {
            profs |= A2DP_PROFILE_MASK;
            profs |= AVRCP_PROFILE_MASK;
        }

#if F_APP_HID_SUPPORT
        if (bond_flag & (BOND_FLAG_HID))
        {
            if (app_cfg_const.supported_profile_mask & HID_PROFILE_MASK)
            {
                profs |= HID_PROFILE_MASK;
            }
        }
#endif
        if (bond_flag & (BOND_FLAG_GATT))
        {
            if (app_cfg_const.supported_profile_mask & GATT_PROFILE_MASK)
            {
                profs |= GATT_PROFILE_MASK;
            }
        }
    }
    else if ((T_LINKBACK_SCENARIO)app_cfg_const.link_scenario == LINKBACK_SCENARIO_HF_A2DP_LAST_DEVICE)
    {
        if (bond_flag & BOND_FLAG_HFP)
        {
            profs |= HFP_PROFILE_MASK;

            if (app_cfg_const.supported_profile_mask & PBAP_PROFILE_MASK)
            {
                profs |= PBAP_PROFILE_MASK;
            }
        }
        else if (bond_flag & BOND_FLAG_HSP)
        {
            profs |= HSP_PROFILE_MASK;

            if (app_cfg_const.supported_profile_mask & PBAP_PROFILE_MASK)
            {
                profs |= PBAP_PROFILE_MASK;
            }
        }

        if (bond_flag & (BOND_FLAG_A2DP))
        {
            profs |= A2DP_PROFILE_MASK;
            profs |= AVRCP_PROFILE_MASK;
        }

#if F_APP_HID_SUPPORT
        if (bond_flag & (BOND_FLAG_HID))
        {
            profs |= HID_PROFILE_MASK;
        }
#endif

        if (bond_flag & BOND_FLAG_SPP)
        {
            if (app_cfg_const.supported_profile_mask & SPP_PROFILE_MASK)
            {
                profs |= SPP_PROFILE_MASK;
            }
        }
        else if (bond_flag & BOND_FLAG_IAP)
        {
            if (app_cfg_const.supported_profile_mask & IAP_PROFILE_MASK)
            {
                profs |= IAP_PROFILE_MASK;
            }
        }
        if (bond_flag & (BOND_FLAG_GATT))
        {
            if (app_cfg_const.supported_profile_mask & GATT_PROFILE_MASK)
            {
                profs |= GATT_PROFILE_MASK;
            }
        }
    }
    else if ((T_LINKBACK_SCENARIO)app_cfg_const.link_scenario == LINKBACK_SCENARIO_SPP_BASE)
    {
        if (bond_flag & BOND_FLAG_SPP)
        {
            if (app_cfg_const.supported_profile_mask & SPP_PROFILE_MASK)
            {
                profs |= SPP_PROFILE_MASK;
            }
        }
        else if (bond_flag & BOND_FLAG_IAP)
        {
            if (app_cfg_const.supported_profile_mask & IAP_PROFILE_MASK)
            {
                profs |= IAP_PROFILE_MASK;
            }
        }
    }
#if F_APP_HID_SUPPORT
    else if ((T_LINKBACK_SCENARIO)app_cfg_const.link_scenario == LINKBACK_SCENARIO_HID_BASE)
    {
        if (bond_flag & (BOND_FLAG_HID))
        {
            profs |= HID_PROFILE_MASK;
        }
    }
#endif

    return profs;
}


void b2s_connected_add_prof(uint8_t *bd_addr, uint32_t prof)
{
    T_APP_BR_LINK *p_link;

    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        p_link->connected_profile |= prof;

        switch (prof)
        {
        case A2DP_PROFILE_MASK:
            bt_bond_flag_add(bd_addr, BOND_FLAG_A2DP);
            break;

        case HFP_PROFILE_MASK:
            {
                bt_bond_flag_remove(bd_addr, BOND_FLAG_HSP);
                bt_bond_flag_add(bd_addr, BOND_FLAG_HFP);
            }
            break;

        case HSP_PROFILE_MASK:
            {
                bt_bond_flag_remove(bd_addr, BOND_FLAG_HFP);
                bt_bond_flag_add(bd_addr, BOND_FLAG_HSP);
            }
            break;

        case SPP_PROFILE_MASK:
            bt_bond_flag_add(bd_addr, BOND_FLAG_SPP);
            break;

        case PBAP_PROFILE_MASK:
            bt_bond_flag_add(bd_addr, BOND_FLAG_PBAP);
            break;

#if F_APP_HID_SUPPORT
        case HID_PROFILE_MASK:
            bt_bond_flag_add(bd_addr, BOND_FLAG_HID);
            break;
#endif

        case IAP_PROFILE_MASK:
            bt_bond_flag_add(bd_addr, BOND_FLAG_IAP);
            break;

        case GATT_PROFILE_MASK:
            bt_bond_flag_add(bd_addr, BOND_FLAG_GATT);
            break;
        default:
            break;
        }
    }
}

static void b2s_connected_del_prof(uint8_t *bd_addr, uint32_t prof)
{
    T_APP_BR_LINK *p_link;

    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        p_link->connected_profile &= ~prof;
    }
}

static bool bt_connected_del_node(uint8_t *bd_addr)
{
    T_APP_BR_LINK *p_link;

    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        app_free_br_link(p_link);
        return true;
    }
    else
    {
        return false;
    }
}

static void connected_node_auth_suc(uint8_t *bd_addr)
{
    T_APP_BR_LINK *p_link;

    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        p_link->auth_flag = true;
    }
}

static bool bt_connected_add_node(uint8_t *bd_addr, uint8_t *id)
{
    T_APP_BR_LINK *p_link;

    p_link = app_find_br_link(bd_addr);
    if (p_link == NULL)
    {
        p_link = app_alloc_br_link(bd_addr);
    }
    if (p_link != NULL)
    {
        *id = p_link->id;
        return true;
    }
    else
    {
        return false;
    }
}

void app_bt_policy_discon_all_profiles_by_addr(uint8_t *bd_addr)
{
    APP_PRINT_INFO0("app_bt_policy_discon_all_profiles_by_addr start ");
    T_APP_BR_LINK *p_link = NULL;
    uint32_t plan_profs;

    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        plan_profs = p_link->connected_profile;
        if (plan_profs != 0)
        {
            p_link->disconn_acl_flg = true;
        }

        linkback_profile_disconnect_start(bd_addr, plan_profs);
    }
}

void app_bt_policy_disconnect_acl_handle(uint8_t *bd_addr, uint32_t prof)
{
    T_APP_BR_LINK *p_link = NULL;

    b2s_connected_del_prof(bd_addr, prof);

    p_link = app_find_br_link(bd_addr);
    if (p_link != NULL)
    {
        if (p_link->connected_profile == 0)
        {
            if (p_link->disconn_acl_flg)
            {
                p_link->disconn_acl_flg = false;
                gap_br_send_acl_disconn_req(bd_addr);
            }
        }
    }

}

static void app_bt_policy_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    T_APP_BR_LINK *p_link = NULL;
    APP_PRINT_INFO1("app_bt_policy_cback  event_type = 0x%x", event_type);

    switch (event_type)
    {
    case BT_EVENT_ACL_CONN_SUCCESS:
        {
            uint8_t id;
            bool res = true;
            //need to skip gui fb when lcd has not been powerd off
            if (app_dlps_check_enter_bits(APP_DLPS_ENTER_CHECK_DISPLAY))
            {
                gui_task_delay_enable(3000);
            }
            if (app_bt_bond_get_temp_search_device(param->acl_conn_success.bd_addr) !=
                NULL) //inquiry scan connect
            {
                app_bt_bond_add_search_device(param->acl_conn_success.bd_addr);

                //do not update for RTL87x3EP

            }
#if CONFIG_REALTEK_APP_DSP_ONLINE_DEBUG
            if (acl_conn_ind_device_type == T_DEVICE_TYPE_PC)
            {
                APP_PRINT_INFO0("only for DSP online debugging, do not add to bondlist");
            }
#endif
            else//case 1 linkback, case 2 device indicate
            {
                res = app_bt_bond_add_device(param->acl_conn_success.bd_addr, acl_conn_ind_device_type);
            }
            app_bt_bond_save_device_info_to_ftl(app_db.bond_device);

            bt_connected_add_node(param->acl_conn_success.bd_addr, &id);
            if (!res)
            {
                app_bt_policy_discon_all_profiles_by_addr(param->acl_conn_success.bd_addr);
            }
            acl_conn_ind_device_type = T_DEVICE_TYPE_DEFAULT;
            app_bt_policy_event_handle(EVENT_BT_CONN, param);
            bt_active_link_set(param->acl_conn_success.bd_addr);
        }
        break;

    case BT_EVENT_ACL_CONN_FAIL:
        {
            APP_PRINT_INFO1("BT_EVENT_ACL_CONN_FAIL acl_conn_fail.cause =%x ",
                            param->acl_conn_fail.cause);
            acl_conn_ind_device_type = T_DEVICE_TYPE_DEFAULT;
            app_bt_policy_event_handle(EVENT_BT_CONN_FAIL, param);
        }
        break;

    case BT_EVENT_ACL_AUTHEN_SUCCESS:
        {
            connected_node_auth_suc(param->acl_authen_success.bd_addr);
            if (app_bt_bond_get_cod_type(param->acl_authen_success.bd_addr) == T_DEVICE_TYPE_PHONE)
            {
                T_LINKBACK_RETRY_PARAM retry_param =
                {
                    .conn_retry_timeout = 0,
                    .conn_retry_cnt = 0,
                    .prof_retry_timeout = 0,
                    .prof_retry_cnt = 0,
                    .delay_timeout = 5000
                };
                linkback_create_connection(param->acl_authen_success.bd_addr, HFP_PROFILE_MASK, T_DEVICE_TYPE_PHONE,
                                           retry_param);
            }
        }
        break;

    case BT_EVENT_ACL_AUTHEN_FAIL:
        {
            APP_PRINT_INFO1("BT_EVENT_ACL_AUTHEN_FAIL acl_authen_fail.cause =%x ",
                            param->acl_authen_fail.cause);
            if ((param->acl_authen_fail.cause == (HCI_ERR | HCI_ERR_AUTHEN_FAIL)) ||
                (param->acl_authen_fail.cause == (HCI_ERR | HCI_ERR_KEY_MISSING)))
            {
                T_LINKBACK_RETRY_PARAM retry_param =
                {
                    .conn_retry_timeout = 0,
                    .conn_retry_cnt = 0,
                    .prof_retry_timeout = 0,
                    .prof_retry_cnt = 0,
                    .delay_timeout = 0
                };
                bt_bond_delete(param->acl_authen_fail.bd_addr);

                if (app_bt_bond_get_cod_type(param->acl_authen_fail.bd_addr) == T_DEVICE_TYPE_EARPHONE)
                {
                    linkback_create_connection(param->acl_authen_fail.bd_addr, A2DP_PROFILE_MASK,
                                               T_DEVICE_TYPE_EARPHONE,
                                               retry_param);
                }
            }
            else
            {
                APP_PRINT_INFO0("BT_EVENT_ACL_AUTHEN_FAIL happened");
                //app_bt_bond_inactive_device(param->acl_authen_fail.bd_addr);//add for test
            }
        }
        break;

    case BT_EVENT_ACL_CONN_DISCONN:
        {
            bt_connected_del_node(param->acl_conn_disconn.bd_addr);
            APP_PRINT_INFO1("BT_EVENT_ACL_CONN_DISCONN reason = %d", param->acl_conn_disconn.cause);
            uint8_t pair_idx;
            uint8_t cur_index = app_bt_bond_get_index_by_addr(param->acl_conn_disconn.bd_addr);
            if (cur_index < MAX_BOND_INFO_NUM)
            {
                if (app_db.bond_device[cur_index].used == true)
                {
                    if (bt_bond_index_get(param->acl_conn_disconn.bd_addr, &pair_idx))
                    {
                        app_bt_bond_inactive_device_by_index(cur_index);
                    }
                    else
                    {
                        app_bt_bond_del_bond_device(param->acl_conn_disconn.bd_addr);
                    }
                    app_bt_bond_save_device_info_to_ftl(app_db.bond_device);
                }

                if (app_db.bond_device[cur_index].device_type == T_DEVICE_TYPE_PHONE &&
                    param->acl_conn_disconn.cause == (HCI_ERR | HCI_ERR_CONN_TIMEOUT))
                {
                    T_LINKBACK_RETRY_PARAM retry_param =
                    {
                        .conn_retry_timeout = 0,
                        .conn_retry_cnt = 0,
                        .prof_retry_timeout = 0,
                        .prof_retry_cnt = 0,
                        .delay_timeout = 0
                    };
                    linkback_create_connection(param->acl_conn_disconn.bd_addr, HFP_PROFILE_MASK, T_DEVICE_TYPE_PHONE,
                                               retry_param);
                }
            }
            app_bt_policy_event_handle(EVENT_BT_DISCONN, param);
            if (dfu_switch_to_ota_mode_pending)
            {
                dfu_switch_to_ota_mode();
            }
            else if (dfu_active_reset_pending)
            {
                if (dfu_active_reset_to_ota_mode)
                {
                    dfu_switch_to_ota_mode();
                }
                else
                {
                    dfu_fw_reboot(RESET_ALL, DFU_ACTIVE_RESET);
                }
            }
        }
        break;

    case BT_EVENT_ACL_CONN_READY:
        {
            T_APP_BR_LINK *p_link;

            /* TODO not set pkt 2m in DUT Test Mode */
            bt_acl_pkt_type_set(param->acl_conn_ready.bd_addr, BT_ACL_PKT_TYPE_2M);

            p_link = app_find_br_link(param->acl_conn_ready.bd_addr);
            if (p_link == NULL)
            {
                return;
            }
            if (app_bt_bond_get_cod_type(param->acl_conn_ready.bd_addr) == T_DEVICE_TYPE_PHONE
                && p_link->acl_link_role != BT_LINK_ROLE_SLAVE)
            {
                bt_link_role_switch(param->acl_conn_ready.bd_addr, false);
            }
        }
        break;

    case BT_EVENT_ACL_CONN_IND:
        {
            uint8_t link_key[16];
            T_BT_LINK_KEY_TYPE type;
            p_link = app_find_br_link(param->acl_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                bt_acl_conn_reject(param->acl_conn_ind.bd_addr, BT_ACL_REJECT_UNACCEPTABLE_ADDR);
            }
            else
            {
                if (((param->acl_conn_ind.cod & 0x1F00) >> 8) == 0x02)
                {
                    APP_PRINT_INFO1("connect to phone cod = 0x%x", param->acl_conn_ind.cod);
                    if (app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_PHONE) != 0xff)
                    {
                        APP_PRINT_INFO0("exist bredr link with phone, reject new link");
                        bt_acl_conn_reject(param->acl_conn_ind.bd_addr, BT_ACL_REJECT_UNACCEPTABLE_ADDR);
                    }
                    else
                    {
                        bt_acl_conn_accept(param->acl_conn_ind.bd_addr, BT_LINK_ROLE_SLAVE);
                        acl_conn_ind_device_type = T_DEVICE_TYPE_PHONE;
                    }

                }
                else if (((param->acl_conn_ind.cod & 0x1F00) >> 8) == 0x04)
                {
                    APP_PRINT_INFO1("connect to audio/video cod = 0x%x", param->acl_conn_ind.cod);
                    if (!bt_bond_key_get(param->acl_conn_ind.bd_addr, link_key, (uint8_t *)&type))
                    {
                        APP_PRINT_INFO0("app_bt_policy reject no linkkey device");
                        bt_acl_conn_reject(param->acl_conn_ind.bd_addr, BT_ACL_REJECT_UNACCEPTABLE_ADDR);
                    }
                    else
                    {
                        if (app_db.a2dp_cur_role == BT_A2DP_ROLE_SNK)
                        {
                            APP_PRINT_INFO0("now is sink role, reject earphone link");
                            bt_acl_conn_reject(param->acl_conn_ind.bd_addr, BT_ACL_REJECT_UNACCEPTABLE_ADDR);
                        }
                        else if (app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_EARPHONE) != 0xff)
                        {
                            APP_PRINT_INFO0("exist bredr link with audio/video device, reject new link");
                            bt_acl_conn_reject(param->acl_conn_ind.bd_addr, BT_ACL_REJECT_UNACCEPTABLE_ADDR);
                        }
                        else
                        {
                            bt_acl_conn_accept(param->acl_conn_ind.bd_addr, BT_LINK_ROLE_MASTER);
                            acl_conn_ind_device_type = T_DEVICE_TYPE_EARPHONE;
                        }
                    }
                }
#if CONFIG_REALTEK_APP_DSP_ONLINE_DEBUG
                else if (((param->acl_conn_ind.cod & 0x1F00) >> 8) == 0x01)
                {
                    APP_PRINT_INFO1("connect to pc cod = 0x%x", param->acl_conn_ind.cod);
                    bt_acl_conn_accept(param->acl_conn_ind.bd_addr, BT_LINK_ROLE_SLAVE);
                    acl_conn_ind_device_type = T_DEVICE_TYPE_PC;
                }
#endif
                else
                {
                    APP_PRINT_INFO1("unknown device cod = 0x%x", param->acl_conn_ind.cod);
                    bt_acl_conn_reject(param->acl_conn_ind.bd_addr, BT_ACL_REJECT_UNACCEPTABLE_ADDR);
                }
            }
        }
        break;

    case BT_EVENT_LINK_KEY_INFO:
        {
            bt_bond_key_set(param->link_key_info.bd_addr, param->link_key_info.link_key,
                            param->link_key_info.key_type);

#if CONFIG_SC_KEY_DERIVE
            APP_PRINT_INFO3("app_bt_policy_cback bd_addr: %s, linkkey: %b key_type: %d",
                            TRACE_BDADDR(param->link_key_info.bd_addr), TRACE_BINARY(16, param->link_key_info.link_key),
                            param->link_key_info.key_type);

            if (app_bt_bond_get_cod_type(param->link_key_info.bd_addr) != T_DEVICE_TYPE_EARPHONE)
            {
                T_LINKBACK_RETRY_PARAM retry_param =
                {
                    .conn_retry_timeout = 0,
                    .conn_retry_cnt = 0,
                    .prof_retry_timeout = 0,
                    .prof_retry_cnt = 0,
                    .delay_timeout = 5000
                };
                linkback_create_connection(param->link_key_info.bd_addr, HFP_PROFILE_MASK, T_DEVICE_TYPE_PHONE,
                                           retry_param);

                acl_conn_ind_device_type = T_DEVICE_TYPE_PHONE;
            }
#endif
        }
        break;

    case BT_EVENT_LINK_KEY_REQ:
        {
            uint8_t link_key[16];
            T_BT_LINK_KEY_TYPE type;

            if (bt_bond_key_get(param->link_key_req.bd_addr, link_key, (uint8_t *)&type))
            {
                bt_link_key_cfm(param->link_key_req.bd_addr, true, type, link_key);
            }
            else
            {
                bt_link_key_cfm(param->link_key_req.bd_addr, false, type, link_key);
            }
        }
        break;

    case BT_EVENT_LINK_PIN_CODE_REQ:
        {
            bt_link_pin_code_cfm(param->link_pin_code_req.bd_addr, app_cfg_nv.pin_code,
                                 app_cfg_nv.pin_code_size, true);
        }
        break;

    case BT_EVENT_SCO_CONN_CMPL:
        {

        }
        break;

    case BT_EVENT_SCO_DISCONNECTED:
        {
            bt_sniff_mode_enable(param->sco_disconnected.bd_addr, 0, 0, 0, 0);
        }
        break;

    case BT_EVENT_SDP_ATTR_INFO:
        {
            linkback_handle_sdp_attr_info(param);
        }
        break;

    case BT_EVENT_SDP_DISCOV_CMPL:
        {
            linkback_handle_sdp_discov_cmpl(param);
#if 0
            // alipay pan create
            static int flag = 0;
            bool alipay_bt_pan_connect(void);
            if (flag == 0)
            {
                if (false == alipay_bt_pan_connect())
                {
                    DBG_DIRECT("[Alipay] alipay_bt_pan_connect, failed!");
                    //return -1;
                }
                else
                {
                    DBG_DIRECT("[Alipay] alipay_bt_pan_connect, success!");
                    flag = 1;
                }
            }
#endif
        }
        break;

    case  BT_EVENT_DID_ATTR_INFO:
        {

        }
        break;

    case BT_EVENT_SDP_DISCOV_STOP:
        {

        }
        break;

    case BT_EVENT_HFP_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;
            uint32_t prof;

            p_link = app_find_br_link(param->hfp_conn_cmpl.bd_addr);
            if (p_link)
            {
                if (param->hfp_conn_cmpl.is_hfp)
                {
                    prof = HFP_PROFILE_MASK;
                }
                else
                {
                    prof = HSP_PROFILE_MASK;
                }
                b2s_connected_add_prof(param->hfp_conn_cmpl.bd_addr, prof);
                linkback_handle_profile_conn(param->hfp_conn_cmpl.bd_addr, prof);
            }
        }
        break;

    case BT_EVENT_HFP_SNIFFING_DISCONN_CMPL:
    case BT_EVENT_HFP_DISCONN_CMPL:
        {
            uint32_t prof;
            if (param->hfp_conn_cmpl.is_hfp)
            {
                prof = HFP_PROFILE_MASK;
            }
            else
            {
                prof = HSP_PROFILE_MASK;
            }

            app_bt_policy_disconnect_acl_handle(param->hfp_conn_cmpl.bd_addr, prof);
        }
        break;

    case BT_EVENT_PBAP_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->pbap_conn_cmpl.bd_addr);
            if (p_link)
            {
                b2s_connected_add_prof(param->pbap_conn_cmpl.bd_addr, PBAP_PROFILE_MASK);
                linkback_handle_profile_conn(param->pbap_conn_cmpl.bd_addr, PBAP_PROFILE_MASK);
            }
        }
        break;

    case BT_EVENT_PBAP_SNIFFING_DISCONN_CMPL:
    case BT_EVENT_PBAP_DISCONN_CMPL:
        {
            app_bt_policy_disconnect_acl_handle(param->pbap_disconn_cmpl.bd_addr, PBAP_PROFILE_MASK);
        }
        break;

    case BT_EVENT_HID_DEVICE_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->hid_device_conn_cmpl.bd_addr);
            if (p_link)
            {
                b2s_connected_add_prof(param->hid_device_conn_cmpl.bd_addr, HID_PROFILE_MASK);
                linkback_handle_profile_conn(param->hid_device_conn_cmpl.bd_addr, HID_PROFILE_MASK);
            }
        }
        break;

    case BT_EVENT_HID_DEVICE_SNIFFING_DISCONN_CMPL:
    case BT_EVENT_HID_DEVICE_DISCONN_CMPL:
        {
            app_bt_policy_disconnect_acl_handle(param->hid_device_disconn_cmpl.bd_addr, HID_PROFILE_MASK);
        }
        break;

    case BT_EVENT_A2DP_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->a2dp_conn_cmpl.bd_addr);
            if (p_link)
            {
                b2s_connected_add_prof(param->a2dp_conn_cmpl.bd_addr, A2DP_PROFILE_MASK);
                linkback_handle_profile_conn(param->a2dp_conn_cmpl.bd_addr, A2DP_PROFILE_MASK);
            }
        }
        break;

    case BT_EVENT_A2DP_DISCONN_CMPL:
        {
            app_bt_policy_disconnect_acl_handle(param->a2dp_disconn_cmpl.bd_addr, A2DP_PROFILE_MASK);
        }
        break;

    case BT_EVENT_AVRCP_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->avrcp_conn_cmpl.bd_addr);
            if (p_link)
            {
                b2s_connected_add_prof(param->avrcp_conn_cmpl.bd_addr, AVRCP_PROFILE_MASK);
                linkback_handle_profile_conn(param->avrcp_conn_cmpl.bd_addr, AVRCP_PROFILE_MASK);
            }
        }
        break;

    case BT_EVENT_AVRCP_SNIFFING_DISCONN_CMPL:
    case BT_EVENT_AVRCP_DISCONN_CMPL:
        {
            app_bt_policy_disconnect_acl_handle(param->avrcp_disconn_cmpl.bd_addr, AVRCP_PROFILE_MASK);
            app_avrcp_set_abs_vol_support(false);
        }
        break;

    case BT_EVENT_SPP_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->spp_conn_cmpl.bd_addr);
            if (p_link)
            {
                b2s_connected_add_prof(param->spp_conn_cmpl.bd_addr, SPP_PROFILE_MASK);
                linkback_handle_profile_conn(param->spp_conn_cmpl.bd_addr, SPP_PROFILE_MASK);
            }
        }
        break;

    case BT_EVENT_SPP_SNIFFING_DISCONN_CMPL:
    case BT_EVENT_SPP_DISCONN_CMPL:
        {
            app_bt_policy_disconnect_acl_handle(param->spp_disconn_cmpl.bd_addr, SPP_PROFILE_MASK);
        }
        break;

    case BT_EVENT_IAP_CONN_CMPL:
        {
            T_APP_BR_LINK *p_link;

            p_link = app_find_br_link(param->iap_conn_cmpl.bd_addr);
            if (p_link)
            {
                b2s_connected_add_prof(param->iap_conn_cmpl.bd_addr, IAP_PROFILE_MASK);
                linkback_handle_profile_conn(param->iap_conn_cmpl.bd_addr, IAP_PROFILE_MASK);
            }
        }
        break;

    case BT_EVENT_IAP_SNIFFING_DISCONN_CMPL:
    case BT_EVENT_IAP_DISCONN_CMPL:
        {
            app_bt_policy_disconnect_acl_handle(param->iap_disconn_cmpl.bd_addr, IAP_PROFILE_MASK);
        }
        break;

    case BT_EVENT_SPP_CONN_FAIL:
    case BT_EVENT_A2DP_CONN_FAIL:
    case BT_EVENT_AVRCP_CONN_FAIL:
    case BT_EVENT_IAP_CONN_FAIL:
    case BT_EVENT_HFP_CONN_FAIL:
    case BT_EVENT_PBAP_CONN_FAIL:
    case BT_EVENT_HID_DEVICE_CONN_FAIL:

        if (param->spp_conn_fail.cause == (L2C_ERR | L2C_CONN_RSP_SECURITY_BLOCK) ||
            param->a2dp_conn_fail.cause == (L2C_ERR | L2C_CONN_RSP_SECURITY_BLOCK) ||
            param->avrcp_conn_fail.cause == (L2C_ERR | L2C_CONN_RSP_SECURITY_BLOCK) ||
            param->iap_conn_fail.cause == (L2C_ERR | L2C_CONN_RSP_SECURITY_BLOCK) ||
            param->hfp_conn_fail.cause == (L2C_ERR | L2C_CONN_RSP_SECURITY_BLOCK) ||
            param->pbap_conn_fail.cause == (L2C_ERR | L2C_CONN_RSP_SECURITY_BLOCK) ||
            param->hid_device_conn_fail.cause == (L2C_ERR | L2C_CONN_RSP_SECURITY_BLOCK))
        {
            linkback_active_node_step_fail_adjust_remain_profs();//need to reconnect
        }
        else
        {
            linkback_active_node_step_fail_adjust_remain_profs();
        }
        linkback_run();
        break;


    case BT_EVENT_REMOTE_NAME_RSP:
        {
            uint16_t unicode_name[MAX_DEVICE_NAME_NUM];
            uint8_t name_len = 0;
            APP_PRINT_INFO1("<-- remote name rsp: name = %s", TRACE_STRING(param->remote_name_rsp.name));
            name_len = watch_utf8_to_unicode((uint8_t *)param->remote_name_rsp.name,
                                             strlen(param->remote_name_rsp.name), \
                                             unicode_name, MAX_DEVICE_NAME_NUM);
            T_APP_BOND_DEVICE *device_temp = app_bt_bond_get_device_by_addr(param->remote_name_rsp.bd_addr);
            if (device_temp != NULL)
            {
                if (device_temp->device_type == T_DEVICE_TYPE_PHONE)
                {
                    memcpy((uint8_t *)device_temp->device_name, (uint8_t *)unicode_name,
                           sizeof(device_temp->device_name));
                    device_temp->device_name_len = name_len;
                    app_bt_bond_save_device_info_to_ftl(app_db.bond_device);
                }
                else
                {
                    APP_PRINT_INFO1("<-- error device_temp->device_type = ", device_temp->device_type);
                }
            }
            else
            {
                app_bt_bond_add_temp_search_device_name_info(param->remote_name_rsp.bd_addr, unicode_name,
                                                             name_len);
                if (app_bt_bond_get_temp_search_device(param->acl_conn_success.bd_addr) !=
                    NULL) //inquiry scan connect
                {
                    app_bt_bond_add_search_device(param->acl_conn_success.bd_addr);
                    app_bt_bond_save_device_info_to_ftl(app_db.bond_device);
                }
            }
        }
        break;

    default:
        break;
    }
}

void app_bt_policy_enter_state(T_STATE state)
{
    APP_PRINT_INFO2("app_bt_policy_enter_state = 0x%x, old_state = 0x%x", state, app_db.bt_state);
    switch (state)
    {
    case STATE_INIT:
        {
            if (app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_PHONE) != 0xff ||
                app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_EARPHONE) != 0xff)
            {
                for (uint8_t i = 0; i < MAX_BR_LINK_NUM; i++)
                {
                    app_bt_policy_discon_all_profiles_by_addr(app_db.br_link[i].bd_addr);
                }
            }
            bt_device_mode_set(BT_DEVICE_MODE_IDLE);
        }
        break;
    case STATE_STANDBY:
        {
            bt_device_mode_set(BT_DEVICE_MODE_DISCOVERABLE_CONNECTABLE);
        }
        break;
    case STATE_LINKBACK:
        {

        }
        break;
    case STATE_CONNECTED_PHONE:
        {
#if (CONFIG_REALTEK_APP_DSP_ONLINE_DEBUG == 0)
            if (app_db.a2dp_cur_role == BT_A2DP_ROLE_SRC &&
                app_bt_bond_check_exist_device_info(T_DEVICE_TYPE_EARPHONE) != 0xff)
            {
                bt_device_mode_set(BT_DEVICE_MODE_CONNECTABLE);
            }
            else
            {
                bt_device_mode_set(BT_DEVICE_MODE_IDLE);
            }
#endif
        }
        break;
    case STATE_CONNECTED_EARPHONE:
        {
            bt_device_mode_set(BT_DEVICE_MODE_DISCOVERABLE_CONNECTABLE);
        }
        break;
    case STATE_CONNECTED_TWO:
        {
#if (CONFIG_REALTEK_APP_DSP_ONLINE_DEBUG == 0)
            bt_device_mode_set(BT_DEVICE_MODE_IDLE);
#endif
        }
        break;
    default:
        break;
    }
    if (app_db.bt_state != state && p_bt_state_changed_cback != NULL)
    {
        p_bt_state_changed_cback(app_db.bt_state, state);
    }
    app_db.bt_state = state;
}

void app_bt_policy_set_state(void)
{
    uint8_t phone_connected = app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_PHONE);
    uint8_t earphone_connected = app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_EARPHONE);
    APP_PRINT_INFO2("app_bt_policy_set_state: phone_connected %x, earphone_connected %x",
                    phone_connected, earphone_connected);

    if ((phone_connected != 0xff) && (earphone_connected == 0xff))
    {
        app_bt_policy_enter_state(STATE_CONNECTED_PHONE);
    }
    else if ((phone_connected == 0xff) && (earphone_connected != 0xff))
    {
        app_bt_policy_enter_state(STATE_CONNECTED_EARPHONE);
    }
    else if ((phone_connected != 0xff) && (earphone_connected != 0xff))
    {
        app_bt_policy_enter_state(STATE_CONNECTED_TWO);
    }
    else
    {
        app_bt_policy_enter_state(STATE_STANDBY);
    }
}

void app_bt_policy_event_handle(T_EVENT event, void *param)
{
    T_BT_EVENT_PARAM *bt_event_param = param;
    APP_PRINT_INFO1("app_bt_policy_event_handle = 0x%x", event);
    switch (event)
    {
    case EVENT_BT_STARTUP:
        {
            linkback_todo_queue_init();
            linkback_active_node_init();
            app_bt_policy_enter_state(STATE_STANDBY);
        }
        break;
    case EVENT_LINKBACK_START:
        {
            app_bt_policy_enter_state(STATE_LINKBACK);
        }
        break;
    case EVENT_LINKBACK_STOP:
        {
            app_bt_policy_set_state();

            if (app_audio_mode_switch_status_get())
            {
                app_audio_mode_switch(app_audio_mode_switch_mode_get());
            }
        }
        break;
    case EVENT_BT_CONN:
        {
            //the profile will linkback, to set BT state after EVENT_LINKBACK_STOP
        }
        break;
    case EVENT_BT_CONN_FAIL:
        {
            linkback_run();
        }
        break;
    case EVENT_BT_DISCONN:
        {
            if (app_db.bt_state != STATE_INIT)
            {
                uint8_t cur_index = app_bt_bond_get_index_by_addr(bt_event_param->acl_conn_disconn.bd_addr);
                if (cur_index == MAX_BOND_INFO_NUM)
                {
                    return;
                }
                bool linkback_condition = false;
                T_LINKBACK_RETRY_PARAM retry_param =
                {
                    .conn_retry_timeout = 0,
                    .conn_retry_cnt = 0,
                    .prof_retry_timeout = 1000,
                    .prof_retry_cnt = 3,
                    .delay_timeout = 0
                };

                if (app_db.bond_device[cur_index].device_type == T_DEVICE_TYPE_PHONE &&
                    bt_event_param->acl_conn_disconn.cause == (HCI_ERR | HCI_ERR_CONN_TIMEOUT))
                {
                    linkback_condition = true;
                }

                if (app_db.bt_state == STATE_LINKBACK) //ACL is connected, but profiles are not connected
                {
                    if (linkback_active_node_judge_cur_conn_addr(bt_event_param->acl_conn_disconn.bd_addr))
                    {
                        linkback_cancel_connection_by_addr(bt_event_param->acl_conn_disconn.bd_addr);
                        if (linkback_condition)
                        {
                            linkback_create_connection(bt_event_param->acl_conn_disconn.bd_addr, HFP_PROFILE_MASK,
                                                       T_DEVICE_TYPE_PHONE,
                                                       retry_param);
                        }
                        linkback_run();
                    }
                    else
                    {
                        if (linkback_condition)
                        {
                            linkback_create_connection(bt_event_param->acl_conn_disconn.bd_addr, HFP_PROFILE_MASK,
                                                       T_DEVICE_TYPE_PHONE,
                                                       retry_param);
                        }
                    }
                }
                else
                {
                    if (linkback_condition)
                    {
                        linkback_create_connection(bt_event_param->acl_conn_disconn.bd_addr, HFP_PROFILE_MASK,
                                                   T_DEVICE_TYPE_PHONE,
                                                   retry_param);
                    }
                    else
                    {
                        app_bt_policy_set_state();
                    }
                }
            }

            if (app_db.is_bond_clear)
            {
                app_bond_bt_bond_clear();
            }
        }
        break;
    case EVENT_BT_IDLE:
        {
            app_bt_policy_enter_state(STATE_INIT);
        }
        break;
    default:
        break;
    }
}

void app_bt_state_changed_cback(T_STATE old_state, T_STATE new_state)
{
    DBG_DIRECT("app_bt_state_cback old_state 0x%x to new_state 0x%x", old_state, new_state);

    /* Publish phone connection status to GUI */
    if (new_state == STATE_CONNECTED_PHONE || new_state == STATE_CONNECTED_TWO)
    {
        /* Phone connected */
        event_bus_publish(EVENT_BUS_TOPIC_BT_EVT_PHONE_CONN, NULL, 0);
    }
    else if ((old_state == STATE_CONNECTED_PHONE && new_state == STATE_STANDBY) ||
             (old_state == STATE_CONNECTED_TWO && new_state == STATE_STANDBY) ||
             (old_state == STATE_CONNECTED_TWO && new_state == STATE_CONNECTED_EARPHONE))
    {
        /* Phone disconnected */
        event_bus_publish(EVENT_BUS_TOPIC_BT_EVT_PHONE_DISCONN, NULL, 0);
    }

    /* Publish headphone connection status to GUI */
    if (new_state == STATE_CONNECTED_EARPHONE || new_state == STATE_CONNECTED_TWO)
    {
        /* Headphone connected: carry the active earphone's address as payload so
           subscribers can identify which device connected. Payload is optional;
           subscribers that only need the notification ignore it. */
        T_APP_BOND_DEVICE *ear = app_bt_bond_get_active_device_by_type(T_DEVICE_TYPE_EARPHONE);
        event_bus_publish(EVENT_BUS_TOPIC_BT_EVT_HEADPHONE_CONN,
                          ear ? ear->bd_addr : NULL, ear ? sizeof(ear->bd_addr) : 0);
    }
    else if ((old_state == STATE_CONNECTED_EARPHONE && new_state == STATE_STANDBY) ||
             (old_state == STATE_CONNECTED_TWO && new_state == STATE_STANDBY) ||
             (old_state == STATE_CONNECTED_TWO && new_state == STATE_CONNECTED_PHONE))
    {
        /* Headphone disconnected */
        event_bus_publish(EVENT_BUS_TOPIC_BT_EVT_HEADPHONE_DISCONN, NULL, 0);
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void app_bt_policy_init(void)
{
    APP_PRINT_INFO0("app_bt_policy_init");
    bt_mgr_cback_register(app_bt_policy_cback);
    app_bt_bond_load_device_info_from_ftl(app_db.bond_device);
    app_bt_bond_free_all_device();
    bt_state_changed_cback_register(app_bt_state_changed_cback);
}

void app_bt_policy_disconnect(uint8_t *bd_addr, uint32_t plan_profs)
{
    linkback_profile_disconnect_start(bd_addr, plan_profs);
}

void app_bt_policy_set_enabled(bool enable)
{
    APP_PRINT_INFO1("app_bt_policy_set_enabled: enable=%d", enable);
    app_bt_policy_event_handle(enable ? EVENT_BT_STARTUP : EVENT_BT_IDLE, NULL);
}

void app_bt_policy_connect_phone(uint8_t *bd_addr)
{
    linkback_phone_create_connection(bd_addr);
}

void app_bt_policy_connect_bredr(uint8_t *bd_addr)
{
    if (bd_addr == NULL) { return; }

    if (app_audio_get_play_status() == APP_AUDIO_STATE_PLAY)
    {
        set_play_flag(true);
    }

    linkback_cancel_connection_by_device_type(T_DEVICE_TYPE_EARPHONE);

    uint8_t *active_addr = app_bt_bond_check_exist_other_active_device(bd_addr,
                                                                       T_DEVICE_TYPE_EARPHONE);
    if (active_addr != NULL)
    {
        APP_PRINT_INFO0("app_bt_policy_connect_bredr: disconnect other active first");
        app_bt_policy_discon_all_profiles_by_addr(active_addr);
    }
    else
    {
        APP_PRINT_INFO0("app_bt_policy_connect_bredr: create connection");
        linkback_earphone_create_connection(bd_addr);
    }
}

void app_bt_policy_disconnect_bredr(uint8_t *bd_addr)
{
    if (bd_addr == NULL) { return; }
    app_bt_policy_discon_all_profiles_by_addr(bd_addr);
}

void app_bt_policy_default_connect(uint8_t *bd_addr, uint32_t plan_profs, bool check_bond_flag)
{
    // to be done
//    T_BT_PARAM bt_param;

//    memset(&bt_param, 0, sizeof(T_BT_PARAM));

//    bt_param.bd_addr = bd_addr;
//    bt_param.prof = plan_profs;
//    bt_param.is_special = false;
//    bt_param.check_bond_flag = check_bond_flag;
//    state_machine(EVENT_DEDICATED_CONNECT, &bt_param);
}
