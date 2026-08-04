/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include <trace.h>
#include <string.h>
#include <gap.h>
#include <gap_bond_le.h>
#include <bt_gatt_svc.h>
#include <gap_msg.h>
#include "dfu_common.h"
#include "dfu_transport.h"
#include <app_msg.h>
#include <gap_conn_le.h>
#include "app_ble_gap.h"
#include "communicate_protocol.h"
#include "module_global_data.h"
#include "app_a2dp.h"
#include "app_msg_handle.h"
#include "app_console_msg.h"
#include "app_transfer.h"
#include "app_report.h"
#include "app_cmd.h"
#include "audio_a2dp_src.h"
#include "app_main.h"
#include "ble_mgr.h"
#include "app_ble_adv.h"
#include "bt_bond_api.h"
#include "gap_bond_manager.h"
#include "os_mem.h"

#if CONFIG_GATT_OVER_BREDR
#include "app_att.h"
#endif

/** @defgroup  PERIPH_APP Peripheral Application
    * @brief This file handles BLE peripheral application routines.
    * @{
    */
/*============================================================================*
 *                              Variables
 *============================================================================*/

T_OS_QUEUE  adv_conn_queue;

/*============================================================================*
 *                              Functions
 *============================================================================*/
void app_handle_gap_msg(T_IO_MSG  *p_gap_msg);
/**
 * @brief    All the application messages are pre-handled in this function
 * @note     All the IO MSGs are sent to this function, then the event handling
 *           function shall be called according to the MSG type.
 * @param[in] io_msg  IO message data
 * @return   void
 */
void app_handle_io_msg(T_IO_MSG io_msg)
{
    uint16_t msg_type = io_msg.type;

    switch (msg_type)
    {
    case IO_MSG_TYPE_BT_STATUS:
        {
            app_handle_gap_msg(&io_msg);
        }
        break;
    case IO_MSG_TYPE_WRISTBNAD:
#if (F_APP_AUTO_SUPPORT == 1)
    case IO_MSG_TYPE_GPIO:
#endif
        {
            watch_handle_io_message(&io_msg);
        }
        break;
    case IO_MSG_TYPE_A2DP_SRC:
        {
            audio_a2dp_src_handle_msg(&io_msg);
        }
        break;
    case IO_MSG_TYPE_CONSOLE:
        {
            app_console_handle_msg(io_msg);
        }
        break;
    case IO_MSG_TYPE_ECC:
        {
            event_bus_publish("ecc/relay", NULL, 0);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_DEV_STATE_CHANGE
 * @note     All the gap device state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] new_state  New gap device state
 * @param[in] cause GAP device state change cause
 * @return   void
 */
void app_handle_dev_state_evt(T_GAP_DEV_STATE new_state, uint16_t cause)
{
    if (new_state.gap_init_state == GAP_INIT_STATE_STACK_READY)
    {
#if CONFIG_GATT_OVER_BREDR
        app_att_init();
#endif
    }
    for (uint8_t i = 0; i < adv_conn_queue.count; i++)
    {
        T_ADV_CONN_QUEUE_PKT *queue_pkt = os_queue_peek(&adv_conn_queue, i);
        if (queue_pkt)
        {
            queue_pkt->adv_conn->cb_tbl->device_state_cb(new_state, cause);
        }
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_STATE_CHANGE
 * @note     All the gap conn state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New gap connection state
 * @param[in] disc_cause Use this cause when new_state is GAP_CONN_STATE_DISCONNECTED
 * @return   void
 */
void app_handle_conn_state_evt(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause)
{
    APP_PRINT_INFO3("app_handle_conn_state_evt: conn_id %d new_state %d, disc_cause 0x%x",
                    conn_id, new_state, disc_cause);
    T_APP_LE_LINK *p_link;
    p_link = app_find_le_link_by_conn_id(conn_id);

    T_LE_ADV_CONN *adv_conn = app_get_adv_conn_hdr_by_conn_id(conn_id);
    if (adv_conn)
    {
        if (adv_conn->cb_tbl->conn_state_cb)
        {
            adv_conn->cb_tbl->conn_state_cb(conn_id, new_state, disc_cause);
        }
    }

    switch (new_state)
    {
    case GAP_CONN_STATE_CONNECTING:
        {
            if (p_link == NULL)
            {
                p_link = app_alloc_le_link_by_conn_id(conn_id);
                if (p_link != NULL)
                {
                    p_link->state = LE_LINK_STATE_CONNECTING;
                }
            }
        }
        break;
    case GAP_CONN_STATE_DISCONNECTED:
        {
            app_free_le_link(p_link);
        }
        break;
    default:
        break;
    }

}

/**
 * @brief    Handle msg GAP_MSG_LE_AUTHEN_STATE_CHANGE
 * @note     All the gap authentication state events are pre-handled in this function.
 *           Then the event handling function shall be called according to the new_state
 * @param[in] conn_id Connection ID
 * @param[in] new_state  New authentication state
 * @param[in] cause Use this cause when new_state is GAP_AUTHEN_STATE_COMPLETE
 * @return   void
 */
void app_handle_authen_state_evt(uint8_t conn_id, uint8_t new_state, uint16_t cause)
{
    T_LE_ADV_CONN *adv_conn = app_get_adv_conn_hdr_by_conn_id(conn_id);
    if (adv_conn)
    {
        if (adv_conn->cb_tbl->authen_state_cb)
        {
            adv_conn->cb_tbl->authen_state_cb(conn_id, new_state, cause);
        }
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_MTU_INFO
 * @note     This msg is used to inform APP that exchange mtu procedure is completed.
 * @param[in] conn_id Connection ID
 * @param[in] mtu_size  New mtu size
 * @return   void
 */
void app_handle_conn_mtu_info_evt(uint8_t conn_id, uint16_t mtu_size)
{
    T_LE_ADV_CONN *adv_conn = app_get_adv_conn_hdr_by_conn_id(conn_id);
    if (adv_conn)
    {
        if (adv_conn->cb_tbl->mtu_info_cb)
        {
            adv_conn->cb_tbl->mtu_info_cb(conn_id, mtu_size);
        }
    }
}

/**
 * @brief    Handle msg GAP_MSG_LE_CONN_PARAM_UPDATE
 * @note     All the connection parameter update change  events are pre-handled in this function.
 * @param[in] conn_id Connection ID
 * @param[in] status  New update state
 * @param[in] cause Use this cause when status is GAP_CONN_PARAM_UPDATE_STATUS_FAIL
 * @return   void
 */
void app_handle_conn_param_update_evt(uint8_t conn_id, uint8_t status, uint16_t cause)
{
    T_LE_ADV_CONN *adv_conn = app_get_adv_conn_hdr_by_conn_id(conn_id);
    if (adv_conn)
    {
        if (adv_conn->cb_tbl->conn_param_cb)
        {
            adv_conn->cb_tbl->conn_param_cb(conn_id, status, cause);
        }
    }
}

/**
 * @brief    All the BT GAP MSG are pre-handled in this function.
 * @note     Then the event handling function shall be called according to the
 *           subtype of T_IO_MSG
 * @param[in] p_gap_msg Pointer to GAP msg
 * @return   void
 */
void app_handle_gap_msg(T_IO_MSG *p_gap_msg)
{
    T_LE_GAP_MSG gap_msg;
    uint8_t conn_id;
    memcpy(&gap_msg, &p_gap_msg->u.param, sizeof(p_gap_msg->u.param));
    ble_mgr_handle_gap_msg(p_gap_msg->subtype, &gap_msg);
    APP_PRINT_TRACE1("app_handle_gap_msg: subtype %d", p_gap_msg->subtype);
    switch (p_gap_msg->subtype)
    {
    case GAP_MSG_LE_DEV_STATE_CHANGE:
        {
            app_handle_dev_state_evt(gap_msg.msg_data.gap_dev_state_change.new_state,
                                     gap_msg.msg_data.gap_dev_state_change.cause);
        }
        break;

    case GAP_MSG_LE_CONN_STATE_CHANGE:
        {
            app_handle_conn_state_evt(gap_msg.msg_data.gap_conn_state_change.conn_id,
                                      (T_GAP_CONN_STATE)gap_msg.msg_data.gap_conn_state_change.new_state,
                                      gap_msg.msg_data.gap_conn_state_change.disc_cause);
        }
        break;

    case GAP_MSG_LE_CONN_MTU_INFO:
        {
            app_handle_conn_mtu_info_evt(gap_msg.msg_data.gap_conn_mtu_info.conn_id,
                                         gap_msg.msg_data.gap_conn_mtu_info.mtu_size);
        }
        break;

    case GAP_MSG_LE_CONN_PARAM_UPDATE:
        {
            app_handle_conn_param_update_evt(gap_msg.msg_data.gap_conn_param_update.conn_id,
                                             gap_msg.msg_data.gap_conn_param_update.status,
                                             gap_msg.msg_data.gap_conn_param_update.cause);
        }
        break;

    case GAP_MSG_LE_AUTHEN_STATE_CHANGE:
        {
            app_handle_authen_state_evt(gap_msg.msg_data.gap_authen_state.conn_id,
                                        gap_msg.msg_data.gap_authen_state.new_state,
                                        gap_msg.msg_data.gap_authen_state.status);
        }
        break;

    case GAP_MSG_LE_BOND_JUST_WORK:
        {
            conn_id = gap_msg.msg_data.gap_bond_just_work_conf.conn_id;
            le_bond_just_work_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_JUST_WORK");
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_DISPLAY:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_passkey_display.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_DISPLAY:passkey %d", display_value);
            le_bond_passkey_display_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_USER_CONFIRMATION:
        {
            uint32_t display_value = 0;
            conn_id = gap_msg.msg_data.gap_bond_user_conf.conn_id;
            le_bond_get_display_key(conn_id, &display_value);
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_USER_CONFIRMATION: passkey %d", display_value);
            le_bond_user_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_PASSKEY_INPUT:
        {
            uint32_t passkey = 888888;
            conn_id = gap_msg.msg_data.gap_bond_passkey_input.conn_id;
            APP_PRINT_INFO1("GAP_MSG_LE_BOND_PASSKEY_INPUT: conn_id %d", conn_id);
            le_bond_passkey_input_confirm(conn_id, passkey, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    case GAP_MSG_LE_BOND_OOB_INPUT:
        {
            uint8_t oob_data[GAP_OOB_LEN] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
            conn_id = gap_msg.msg_data.gap_bond_oob_input.conn_id;
            APP_PRINT_INFO0("GAP_MSG_LE_BOND_OOB_INPUT");
            le_bond_set_param(GAP_PARAM_BOND_OOB_DATA, GAP_OOB_LEN, oob_data);
            le_bond_oob_input_confirm(conn_id, GAP_CFM_CAUSE_ACCEPT);
        }
        break;

    default:
        APP_PRINT_ERROR1("app_handle_gap_msg: unknown subtype %d", p_gap_msg->subtype);
        break;
    }
}

/** @} */ /* End of group PERIPH_GAP_MSG */

/** @defgroup  PERIPH_GAP_CALLBACK GAP Callback Event Handler
    * @brief Handle GAP callback event
    * @{
    */
/**
  * @brief Callback for gap le to notify app
  * @param[in] cb_type callback msy type @ref GAP_LE_MSG_Types.
  * @param[in] p_cb_data point to callback data @ref T_LE_CB_DATA.
  * @retval result @ref T_APP_RESULT
  */
T_APP_RESULT app_gap_callback(uint8_t cb_type, void *p_cb_data)
{
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;

    ble_mgr_handle_gap_cb(cb_type, p_cb_data);

    switch (cb_type)
    {
    case GAP_MSG_LE_DATA_LEN_CHANGE_INFO:
        APP_PRINT_INFO3("GAP_MSG_LE_DATA_LEN_CHANGE_INFO: conn_id %d, tx octets 0x%x, max_tx_time 0x%x",
                        p_data->p_le_data_len_change_info->conn_id,
                        p_data->p_le_data_len_change_info->max_tx_octets,
                        p_data->p_le_data_len_change_info->max_tx_time);
        break;

    case GAP_MSG_LE_MODIFY_WHITE_LIST:
        APP_PRINT_INFO2("GAP_MSG_LE_MODIFY_WHITE_LIST: operation %d, cause 0x%x",
                        p_data->p_le_modify_white_list_rsp->operation,
                        p_data->p_le_modify_white_list_rsp->cause);
        break;

    case GAP_MSG_APP_BOND_MANAGER_INFO:
        result = bt_bond_mgr_handle_gap_msg(p_data->p_le_cb_data);
        break;

    default:
        APP_PRINT_ERROR1("app_gap_callback: unhandled cb_type 0x%x", cb_type);
        break;
    }
    return result;
}
/** @} */ /* End of group PERIPH_GAP_CALLBACK */

bool app_ble_gap_msg_handle_register(T_LE_ADV_CONN *adv_conn)
{
    if (adv_conn == NULL)
    {
        return false;
    }
    T_ADV_CONN_QUEUE_PKT *queue_pkt;
    queue_pkt = (T_ADV_CONN_QUEUE_PKT *)os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(T_ADV_CONN_QUEUE_PKT));
    if (queue_pkt == NULL)
    {
        APP_PRINT_ERROR0("app_ble_gap_msg_handle_register fail");
        return false;
    }

    queue_pkt->adv_conn = adv_conn;
    os_queue_in(&adv_conn_queue, queue_pkt);

    return true;
}

T_LE_ADV_CONN *app_get_adv_conn_hdr_by_conn_id(uint8_t conn_id)
{
    T_ADV_CONN_QUEUE_PKT *queue_pkt = NULL;
    T_GAP_CONN_INFO conn_info;

    le_get_conn_info(conn_id, &conn_info);
    for (uint8_t i = 0; i < adv_conn_queue.count; i++)
    {
        queue_pkt = os_queue_peek(&adv_conn_queue, i);
        if (queue_pkt)
        {
            if (queue_pkt->adv_conn->role == GAP_LINK_ROLE_SLAVE)
            {
                if (queue_pkt->adv_conn->conn_id == conn_id)
                {
                    return queue_pkt->adv_conn;//used for slave role case, we can get conn id in adv stop event
                }
            }
            else if (queue_pkt->adv_conn->role == GAP_LINK_ROLE_MASTER)
            {
                if (!memcmp(conn_info.remote_bd, queue_pkt->adv_conn->remote_bd, 6) ||
                    (queue_pkt->adv_conn->conn_id == conn_id))
                {
                    return queue_pkt->adv_conn;//used for master role case, we can set remote address before connect
                }
            }

        }
    }

    return NULL;
}

void app_ble_gap_ble_mgr_init(void)
{
    BLE_MGR_PARAMS param = {0};
    param.ble_ext_adv.enable = true;
    param.ble_ext_adv.adv_num = CONFIG_BLE_LINKS;
    param.ble_conn.enable = true;
    param.ble_conn.link_num = CONFIG_BLE_LINKS;
    param.ble_scan.enable = true;
    ble_mgr_init(&param);

    app_ble_gap_msg_handle_register(&le_common_adv);
}


/** @} */ /* End of group PERIPH_APP */
