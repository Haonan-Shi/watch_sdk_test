/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <trace.h>
#include <string.h>
#include <gap_msg.h>
#include <ble_mgr.h>
#include <gap_bond_le.h>
#include <app_peripheral_gap.h>
#include "app_peripheral_adv.h"
#include <gap_conn_le.h>
#if (THROUPUT_BT_BREDR_SUPPORT == 1)
#include "gap_br.h"
#endif
#include "tp_ctl.h"

/** @defgroup PERIPH_APP Peripheral Application
  * @brief Peripheral Application
  * @{
  */
/*============================================================================*
 *                              Variables
 *============================================================================*/

T_GAP_DEV_STATE gap_dev_state = {0, 0, 0, 0};/**< GAP device state */
T_GAP_CONN_STATE gap_conn_state = GAP_CONN_STATE_DISCONNECTED;/**< GAP connection state */
/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief app_peripheral_gap_ble_mgr_init
 * initialize ble manager lib which will enable ble extend advertising module.
 */
void app_peripheral_gap_ble_mgr_init(void)
{
    BLE_MGR_PARAMS param = {0};
    param.ble_ext_adv.enable = true;
    param.ble_ext_adv.adv_num = 1;
    ble_mgr_init(&param);
}

/**
  * @brief app_peripheral_gap_init
  * Initialize peripheral and gap bond manager related parameters
  * @return void
  */
void app_peripheral_gap_init(void)
{
    /* Device name and device appearance */
    uint8_t  device_name[GAP_DEVICE_NAME_LEN] = "TP_SERVER";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;

    le_set_gap_param(GAP_PARAM_DEVICE_NAME, GAP_DEVICE_NAME_LEN, device_name);
    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);

    /* slave initialize mtu request */
    uint8_t  slave_init_mtu_req = false;

    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ,
                     sizeof(slave_init_mtu_req), &slave_init_mtu_req);

    /* GAP Bond Manager parameters */
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint16_t auth_flags = GAP_AUTHEN_BIT_BONDING_FLAG;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  auth_oob = false;
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
#if F_APP_BT_ANCS_CLIENT_SUPPORT
    uint8_t  auth_sec_req_enable = true;
#else
    uint8_t  auth_sec_req_enable = false;
#endif
    uint16_t auth_sec_req_flags = GAP_AUTHEN_BIT_BONDING_FLAG;

    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
    gap_set_param(GAP_PARAM_BOND_AUTHEN_REQUIREMENTS_FLAGS, sizeof(auth_flags), &auth_flags);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(auth_oob), &auth_oob);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
                      &auth_use_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_REQUIREMENT, sizeof(auth_sec_req_flags),
                      &auth_sec_req_flags);
#if (THROUPUT_BT_BREDR_SUPPORT == 1)
    static uint8_t bd_addr_server[6]  = {0x11, 0x22, 0x66, 0x77, 0x77, 0x88};
    uint32_t class_of_device = (uint32_t)(0x18 | (0x04 << 8) | (0x24 << 16));
    uint16_t supervision_timeout = 0x1f40;

    uint16_t link_policy = GAP_LINK_POLICY_ROLE_SWITCH | GAP_LINK_POLICY_SNIFF_MODE;

    uint8_t radio_mode = GAP_RADIO_MODE_VISIBLE_CONNECTABLE;
    bool limited_discoverable = false;
    bool auto_accept_acl = false;

    uint8_t pagescan_type = GAP_RADIO_MODE_VISIBLE_CONNECTABLE;
    uint16_t pagescan_interval = 0x800;
    uint16_t pagescan_window = 0x12;
    uint16_t page_timeout = 0x4000;

    uint8_t inquiryscan_type = GAP_INQUIRY_SCAN_TYPE_STANDARD;
    uint16_t inquiryscan_window = 0x12;
    uint16_t inquiryscan_interval = 0x800;
    uint8_t inquiry_mode = GAP_INQUIRY_MODE_EXTENDED_RESULT;
    uint8_t bt_mode = GAP_BT_MODE_21ENABLED;

    gap_br_set_param(GAP_BR_PARAM_BT_MODE, sizeof(uint8_t), &bt_mode);
    gap_br_set_param(GAP_BR_PARAM_COD, sizeof(uint32_t), &class_of_device);
    gap_br_set_param(GAP_BR_PARAM_LINK_POLICY, sizeof(uint16_t), &link_policy);
    gap_br_set_param(GAP_BR_PARAM_SUPV_TOUT, sizeof(uint16_t), &supervision_timeout);
    gap_br_set_param(GAP_BR_PARAM_AUTO_ACCEPT_ACL, sizeof(bool), &auto_accept_acl);

    gap_br_set_param(GAP_BR_PARAM_RADIO_MODE, sizeof(uint8_t), &radio_mode);
    gap_br_set_param(GAP_BR_PARAM_LIMIT_DISCOV, sizeof(bool), &limited_discoverable);

    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_TYPE, sizeof(uint8_t), &pagescan_type);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_INTERVAL, sizeof(uint16_t), &pagescan_interval);
    gap_br_set_param(GAP_BR_PARAM_PAGE_SCAN_WINDOW, sizeof(uint16_t), &pagescan_window);
    gap_br_set_param(GAP_BR_PARAM_PAGE_TIMEOUT, sizeof(uint16_t), &page_timeout);

    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_TYPE, sizeof(uint8_t), &inquiryscan_type);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_INTERVAL, sizeof(uint16_t), &inquiryscan_interval);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_SCAN_WINDOW, sizeof(uint16_t), &inquiryscan_window);
    gap_br_set_param(GAP_BR_PARAM_INQUIRY_MODE, sizeof(uint8_t), &inquiry_mode);

    gap_set_bd_addr(bd_addr_server);
    gap_br_set_param(GAP_BR_PARAM_NAME, GAP_DEVICE_NAME_LEN, "server");
    gap_br_cfg_accept_role(0);
#endif
    /* register gap message callback */

    /* ble manager module initialize*/
    app_peripheral_gap_ble_mgr_init();

    /*advertising parameters initialize*/
    app_peripheral_adv_init_conn_public();
}

/**
 * @brief app_peripheral_gap_handle_conn_param_update_evt
 * Handle msg GAP_MSG_LE_CONN_PARAM_UPDATE
 * @note All the connection parameter update change  events are pre-handled in this function.
 * @param[in] conn_id Connection ID
 * @param[in] status  New update state
 * @param[in] cause Use this cause when status is GAP_CONN_PARAM_UPDATE_STATUS_FAIL
 * @return   void
 */
void app_peripheral_gap_handle_conn_param_update_evt(uint8_t conn_id, uint8_t status,
                                                     uint16_t cause)
{
    switch (status)
    {
    case GAP_CONN_PARAM_UPDATE_STATUS_SUCCESS:
        {
            uint16_t conn_interval;
            uint16_t conn_slave_latency;
            uint16_t conn_supervision_timeout;

            le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &conn_interval, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_LATENCY, &conn_slave_latency, conn_id);
            le_get_conn_param(GAP_PARAM_CONN_TIMEOUT, &conn_supervision_timeout, conn_id);
            APP_PRINT_INFO3("app_handle_conn_param_update_evt update success:conn_interval 0x%x, conn_slave_latency 0x%x, conn_supervision_timeout 0x%x",
                            conn_interval, conn_slave_latency, conn_supervision_timeout);
            para_mode = UPDATE_PARAM_RESULT_ACCEPT_CHANGE;
            tp_update_conn_para_callback(le_get_conn_handle(conn_id), L2C_FIXED_CID_ATT);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_FAIL:
        {
            APP_PRINT_ERROR0("app_handle_conn_param_update_evt update failed");
            para_mode = UPDATE_PARAM_RESULT_REJECT;
            tp_update_conn_para_callback(le_get_conn_handle(conn_id), L2C_FIXED_CID_ATT);
        }
        break;

    case GAP_CONN_PARAM_UPDATE_STATUS_PENDING:
        {
            APP_PRINT_INFO0("app_handle_conn_param_update_evt update pending.");
            para_mode = UPDATE_PARAM_RESULT_ACCEPT_NO_CHANGE;
        }
        break;

    default:
        break;
    }
}

/** End of PERIPH_APP
* @}
*/
