/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "btm.h"
#include "trace.h"
#include <zephyr/sys/printk.h>
#include "bt_bond.h"
#include "gap_br.h"
#include "gap_storage_br.h"
#include "power_test_link.h"

static void app_power_test_bt_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    T_POWER_TEST_LINK *p_link;

    bool handle = true;

    switch (event_type)
    {
    case BT_EVENT_READY:
        {
            char temp_buff[100] = "BT_EVENT_READY\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_ACL_CONN_IND:
        {
            char temp_buff[100] = "BT_EVENT_ACL_CONN_IND\r\n";
            printk("%s", temp_buff);
            p_link = power_test_find_link(param->acl_conn_ind.bd_addr);
            if (p_link != NULL)
            {
                bt_acl_conn_reject(param->acl_conn_ind.bd_addr, BT_ACL_REJECT_UNACCEPTABLE_ADDR);
            }
            else
            {
                bt_acl_conn_accept(param->acl_conn_ind.bd_addr, BT_LINK_ROLE_SLAVE);
            }
        }
        break;

    case BT_EVENT_INQUIRY_RESULT:
        {
            char temp_buff[100] = "BT_EVENT_INQUIRY_RESULT\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_INQUIRY_RSP:
        {
            char temp_buff[100] = "BT_EVENT_INQUIRY_RSP\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_INQUIRY_CMPL:
        {
            char temp_buff[100] = "BT_EVENT_INQUIRY_CMPL\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_INQUIRY_CANCEL_RSP:
        {
            char temp_buff[100] = "BT_EVENT_INQUIRY_CANCEL_RSP\r\n";
            printk("%s", temp_buff);
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
            uint8_t pin_code[4] = {1, 2, 3, 4};
            bt_link_pin_code_cfm(param->link_pin_code_req.bd_addr, pin_code, 4, true);
        }
        break;

    case BT_EVENT_LINK_KEY_INFO:
        break;

    case BT_EVENT_LINK_USER_CONFIRMATION_REQ:
        {
            gap_br_user_cfm_req_cfm(param->link_user_confirmation_req.bd_addr, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case BT_EVENT_ACL_AUTHEN_SUCCESS:
        {
            char temp_buff[100] = "BT_EVENT_ACL_AUTHEN_SUCCESS\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_ACL_CONN_SUCCESS:
        {
            char temp_buff[100] = "BT_EVENT_ACL_CONN_SUCCESS\r\n";
            printk("%s", temp_buff);
            power_test_alloc_link(param->acl_conn_success.bd_addr);
        }
        break;

    case BT_EVENT_ACL_CONN_DISCONN:
        {
            char temp_buff[100] = "BT_EVENT_ACL_CONN_DISCONN\r\n";
            printk("%s", temp_buff);
            p_link = power_test_find_link(param->acl_conn_disconn.bd_addr);
            if (p_link != NULL)
            {
                power_test_free_link(p_link);
            }
        }
        break;

    case BT_EVENT_ACL_CONN_ACTIVE:
        {
            char temp_buff[100] = "BT_EVENT_ACL_CONN_ACTIVE\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_ACL_CONN_SNIFF:
        {
            char temp_buff[100] = "BT_EVENT_ACL_CONN_SNIFF\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_HFP_CONN_CMPL:
        {
            char temp_buff[100] = "BT_EVENT_HFP_CONN_CMPL\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_HFP_DISCONN_CMPL:
        {
            char temp_buff[100] = "BT_EVENT_HFP_DISCONN_CMPL\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_HFP_AG_CONN_CMPL:
        {
            char temp_buff[100] = "BT_EVENT_HFP_AG_CONN_CMPL\r\n";
            printk("%s", temp_buff);
        }
        break;

    case BT_EVENT_HFP_AG_DISCONN_CMPL:
        {
            char temp_buff[100] = "BT_EVENT_HFP_AG_DISCONN_CMPL\r\n";
            printk("%s", temp_buff);
        }
        break;

    default:
        {
            //avoid warning for CONFIG_SOC_SERIES_RTL87X3G
        }
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("app_power_test_bt_cback: event_type 0x%04x", event_type);
    }
}

void app_power_test_init(void)
{
    bt_mgr_cback_register(app_power_test_bt_cback);
}
