/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include "fmna_gatt_platform.h"
#include "fmna_platform_includes.h"
#include "fmna_constants.h"
#include "fmna_gatt.h"
#include "fmna_version.h"
#include "fmna_state_machine.h"
#include "fmna_connection.h"
#include "fmna_config_control_point.h"
#include "fmna_util.h"
#include "fmna_adv_platform.h"
#include "app_findmy.h"
#include "trace.h"
#include "app_findmy_task.h"
#include "os_mem.h"
#include "os_msg.h"
#include "fmna_crypto.h"
#include "fmna_state_machine.h"
#include "gap_conn_le.h"
#include "fmna_peer_manager.h"
#include "fmna_battery_platform.h"
#include "accessory_info_service.h"
#include "bt_gatt_svc.h"
#include <stdlib.h>

#if CONFIG_REALTEK_FINDMY_USE_UARP
#include "fmna_uarp_control_point.h"
#endif

/*============================================================================*
 *                              Local Variables
 *============================================================================*/
static T_SERVER_ID ais_srv_id;  /**< Accessory information service id */
static T_SERVER_ID fns_srv_id;  /**< FindMy network service id */
static T_SERVER_ID tps_srv_id;  /**< Tx power service id */
#if CONFIG_REALTEK_FINDMY_USE_UARP
static T_SERVER_ID fwus_srv_id; /**< Firmware update service id */
#endif

static uint8_t recent_conn_handle = BLE_CONN_HANDLE_INVALID;
static uint8_t findmy_conn_handle;
static bool allow_send_indication = true;
static bool auth_reply_flag = true;

/*============================================================================*
 *                              Local Functions
 *============================================================================*/

/******************************************************************
 * @fn        fmna_ais_srv_cb
 * @brief     Accessory information service callbacks are handled in this function.
 * @param     service_id  Profile service ID
 * @param     p_data  - pointer to callback data
 * @return    cb_result
 * @retval    T_APP_RESULT
 */
static T_APP_RESULT fmna_ais_srv_cb(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_AIS_CALLBACK_DATA *p_ais_cb_data = (T_AIS_CALLBACK_DATA *)p_data;

    uint8_t bd_addr[6];
    uint8_t bd_type;
    le_get_conn_addr(p_ais_cb_data->conn_id, bd_addr, &bd_type);
    APP_PRINT_INFO3("fmna_ais_srv_cb: le_get_conn_addr bd_addr %s, bd_type %d, service_id %d",
                    TRACE_BDADDR(bd_addr), bd_type, ais_srv_id);

    switch (p_ais_cb_data->msg_type)
    {
    case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
        {
            if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_PRODUCT_DATA_INDEX)
            {
                uint8_t product_data[PRODUCT_DATA_BLEN] = PRODUCT_DATA_VAL;
                ais_set_parameter(AIS_PARAM_PROD_DATA, PRODUCT_DATA_BLEN, product_data);
            }
            else if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_MANU_NAME_INDEX)
            {
                uint8_t manu_name[MANU_NAME_MAX_LEN] = FMNA_MANUFACTURER_NAME;
                ais_set_parameter(AIS_PARAM_MANU_NAME, MANU_NAME_MAX_LEN, manu_name);
            }
            else if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_MODEL_NAME_INDEX)
            {
                uint8_t model_name[MODEL_NAME_MAX_LEN] = FMNA_MODEL_NAME;
                ais_set_parameter(AIS_PARAM_MODEL_NAME, MODEL_NAME_MAX_LEN, model_name);
            }
            else if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_ACCESSORY_CATEGORY_INDEX)
            {
                uint8_t accessory_category[ACC_CATEGORY_MAX_LEN] = {0};
                accessory_category[0] = ACCESSORY_CATEGORY;
                ais_set_parameter(AIS_PARAM_ACC_CATEGORY, ACC_CATEGORY_MAX_LEN, accessory_category);
            }
            else if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_ACCESSORY_CAPABILITY_INDEX)
            {
                uint32_t acc_capability = 0;
                SET_BIT(acc_capability, ACC_CAPABILITY_PLAY_SOUND_BIT_POS);
                SET_BIT(acc_capability, ACC_CAPABILITY_SRNM_LOOKUP_BLE_BIT_POS);
                SET_BIT(acc_capability, ACC_CAPABILITY_FW_UPDATE_SERVICE_BIT_POS);
                SET_BIT(acc_capability, ACC_CAPABILITY_SRNM_LOOKUP_NFC_BIT_POS);
                SET_BIT(acc_capability, ACC_CAPABILITY_UT_MOTION_DETECT_BIT_POS);
                ais_set_parameter(AIS_PARAM_ACC_CAP, ACC_CAP_MAX_LEN, &acc_capability);
            }
            else if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_FIRMWARE_VERSION_INDEX)
            {
                uint32_t fw_vers = fmna_version_get_fw_version();
                ais_set_parameter(AIS_PARAM_FW_VERS, FW_VERS_MAX_LEN, &fw_vers);
            }
            else if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_FINDMY_VERSION_INDEX)
            {
                uint32_t findmy_vers = 0x00010000;
                ais_set_parameter(AIS_PARAM_FINDMY_VERS, FINDMY_VERS_MAX_LEN, &findmy_vers);
            }
            else if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_BATTERY_TYPE_INDEX)
            {
                uint8_t bat_type = BAT_TYPE_POWERED;
                ais_set_parameter(AIS_PARAM_BATT_TYPE, BATT_TYPE_MAX_LEN, &bat_type);
            }
            else if (p_ais_cb_data->msg_data.read_value_index == FMNA_AIS_BATTERY_LEVEL_INDEX)
            {
                uint8_t bat_level = BAT_STATE_FULL;
                ais_set_parameter(AIS_PARAM_BATT_LVL, BATT_LVL_MAX_LEN, &bat_level);
            }

        }
        break;

    default:
        break;
    }

    return app_result;
}

/******************************************************************
 * @fn        fmna_fns_srv_cb
 * @brief     Find my network service callbacks are handled in this function.
 * @param     service_id  Profile service ID
 * @param     p_data  - pointer to callback data
 * @return    cb_result
 * @retval    T_APP_RESULT
 */
static T_APP_RESULT fmna_fns_srv_cb(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    fmna_ret_code_t ret_code = FMNA_SUCCESS;

    T_FNS_CALLBACK_DATA *p_srv_data = (T_FNS_CALLBACK_DATA *)p_data;
    uint8_t conn_id = p_srv_data->conn_id;
    recent_conn_handle = conn_id;
    findmy_conn_handle = p_srv_data->conn_handle;
    FMNA_LOG_INFO("fmna_fns_srv_cb: type %d, opcode %d, valid %d", p_srv_data->msg_type,
                  p_srv_data->msg_data.write.opcode, fmna_connection_is_valid_connection(conn_id));

    if (p_srv_data->msg_type == SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE)
    {
        //TBD
//        if (fmna_connection_is_valid_connection(conn_id))
        {
            switch (p_srv_data->msg_data.write.opcode)
            {
            case FMNA_FNS_PAIRING_CP_INDEX:
                ret_code = fmna_gatt_pairing_char_authorized_write_handler(conn_id, FINDMY_UUID_PAIRING_CHAR,
                                                                           p_srv_data->msg_data.write.len,
                                                                           p_srv_data->msg_data.write.p_value);

                break;
            case FMNA_FNS_CONFIG_CP_INDEX:
                ret_code = fmna_gatt_config_char_write_handler(conn_id, FINDMY_UUID_CONFIG_CHAR,
                                                               p_srv_data->msg_data.write.len,
                                                               p_srv_data->msg_data.write.p_value);
                break;
            case FMNA_FNS_NON_OWNER_CP_INDEX:
                ret_code = fmna_gatt_nonown_char_write_handler(conn_id, FINDMY_UUID_NONOWN_CHAR,
                                                               p_srv_data->msg_data.write.len,
                                                               p_srv_data->msg_data.write.p_value);
                break;
            case FMNA_FNS_PAIRED_OWNER_CP_INDEX:
                ret_code = fmna_gatt_paired_owner_char_write_handler(conn_id, FINDMY_UUID_PAIRED_OWNER_CHAR,
                                                                     p_srv_data->msg_data.write.len,
                                                                     p_srv_data->msg_data.write.p_value);
                break;
#ifdef DEBUG
            case FMNA_FNS_DEBUG_CP_INDEX:
                ret_code = fmna_gatt_debug_char_write_handler(conn_id, FINDMY_UUID_DEBUG_CHAR,
                                                              p_srv_data->msg_data.write.len,
                                                              p_srv_data->msg_data.write.p_value);
                break;
#endif //DEBUG
            default:
                APP_PRINT_ERROR2("fmna_fns_srv_cb: attrib_index 0x%x, length %d",
                                 p_srv_data->msg_data.write.opcode, p_srv_data->msg_data.write.len);
                break;
            }

            if (ret_code != FMNA_SUCCESS)
            {
                FMNA_LOG_INFO("fmna_fns_srv_cb: attrib_index 0x%x, error code 0x%x",
                              p_srv_data->msg_data.write.opcode, ret_code);
            }

            if (!auth_reply_flag)
            {
                result = APP_RESULT_REJECT;
                auth_reply_flag = true;
            }
        }
    }
    else if (p_srv_data->msg_type == SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION)
    {
        switch (p_srv_data->msg_data.nofity_indicate_update.notification_indification_index)
        {
        case FMNA_FNS_PAIRING_CP_CCCD_INDEX:
            {
                if (p_srv_data->msg_data.nofity_indicate_update.cccbits)
                {
                    fmna_pm_conn_sec_handle(p_srv_data->conn_id);
                }
            }
            break;
        case FMNA_FNS_CONFIG_CP_CCCD_INDEX:
            {
            }
            break;
        case FMNA_FNS_NON_OWNER_CP_CCCD_INDEX:
            {
            }
            break;
        case FMNA_FNS_PAIRED_OWNER_CP_CCCD_INDEX:
            {
            }
            break;
#ifdef DEBUG
        case FMNA_DEBUG_CP_CCCD_INDEX:
            {
            }
            break;
#endif //DEBUG
        default:
            break;
        }
    }
    return result;
}

/******************************************************************
 * @fn        fmna_tps_srv_cb
 * @brief     Tx power service callbacks are handled in this function.
 * @param     service_id  Profile service ID
 * @param     p_data  - pointer to callback data
 * @return    cb_result
 * @retval    T_APP_RESULT
 */
static T_APP_RESULT fmna_tps_srv_cb(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_TPS_CALLBACK_DATA *p_tps_cb_data = (T_TPS_CALLBACK_DATA *)p_data;

    uint8_t bd_addr[6];
    uint8_t bd_type;
    le_get_conn_addr(p_tps_cb_data->conn_id, bd_addr, &bd_type);
    APP_PRINT_INFO3("fmna_ais_srv_cb: le_get_conn_addr bd_addr %s, bd_type %d, service_id %d",
                    TRACE_BDADDR(bd_addr), bd_type, tps_srv_id);

    switch (p_tps_cb_data->msg_type)
    {
    case SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE:
        {
            if (p_tps_cb_data->msg_data.read_value_index == TPS_READ_TX_POWER_VALUE)
            {
                uint8_t tps_value = FINDMY_TPS_VALUE;
                tps_set_parameter(TPS_PARAM_TX_POWER, 1, (void *)&tps_value);
            }
        }
        break;

    default:
        break;
    }

    return app_result;
}

/******************************************************************
 * @fn        fmna_fwus_srv_cb
 * @brief     Firmware update service callbacks are handled in this function.
 * @param     service_id  Profile service ID
 * @param     p_data  - pointer to callback data
 * @return    cb_result
 * @retval    T_APP_RESULT
 */
#if CONFIG_REALTEK_FINDMY_USE_UARP
static T_APP_RESULT fmna_fwus_srv_cb(T_SERVER_ID service_id, void *p_data)
{
    T_APP_RESULT result = APP_RESULT_SUCCESS;
    fmna_ret_code_t ret_code = FMNA_SUCCESS;

    T_FWUS_CALLBACK_DATA *p_srv_data = (T_FWUS_CALLBACK_DATA *)p_data;
    uint8_t conn_id = p_srv_data->conn_id;

    if (fmna_connection_is_valid_connection(conn_id))
    {
        if (p_srv_data->msg_type == SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE)
        {
            switch (p_srv_data->msg_data.write.opcode)
            {
            case FMNA_FWUS_DATA_CTRL_INDEX:
                ret_code = fmna_gatt_uarp_char_write_handler(conn_id, UARP_UUID_DATA_CHAR,
                                                             p_srv_data->msg_data.write.len,
                                                             p_srv_data->msg_data.write.p_value);
                break;
            default:
                APP_PRINT_ERROR2("fmna_fwus_srv_cb: no such attrib_index 0x%x, length %d",
                                 p_srv_data->msg_data.write.opcode, p_srv_data->msg_data.write.len);
                break;
            }
            if (ret_code != FMNA_SUCCESS)
            {
                FMNA_LOG_INFO("fmna_fwus_srv_cb: attrib_index 0x%x, error code 0x%x",
                              p_srv_data->msg_data.write.opcode, ret_code);
            }
            if (!auth_reply_flag)
            {
                result = APP_RESULT_REJECT;
                auth_reply_flag = true;
            }

        }
        else if (p_srv_data->msg_type == SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION)
        {
            switch (p_srv_data->msg_data.nofity_indicate_update.notification_indification_index)
            {
            case FMNA_FWUS_DATA_CTRL_CCCD_INDEX:
                if (p_srv_data->msg_data.nofity_indicate_update.cccbits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
                {
                    APP_PRINT_INFO0("fmna_fwus_srv_cb: FW update data ctrl notification enable");
                    fmna_uarp_connect(conn_id);
                }
                else
                {
                    APP_PRINT_INFO0("fmna_fwus_srv_cb: FW update data ctrl notification disable");
                }
                break;
            default:
                break;
            }
        }
    }
    return result;
}
#endif

/**
  * @brief  Initialize peripheral and gap bond manager related parameters
  * @return void
  */
static void fmna_gap_init(void)
{
    /* Device name and device appearance */
    uint8_t  device_name[] = "RTK_D";
    uint16_t appearance = GAP_GATT_APPEARANCE_UNKNOWN;
    uint8_t  slave_init_mtu_req = false;

    /* Advertising parameters */
    uint8_t  adv_direct_addr[8] = {0};

    /* GAP Bond Manager parameters */
    uint8_t  auth_pair_mode = GAP_PAIRING_MODE_PAIRABLE;
    uint8_t  auth_io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT;
    uint8_t  auth_oob = false;
    uint8_t  auth_use_fix_passkey = false;
    uint32_t auth_fix_passkey = 0;
    uint8_t gap_param_key_manager = 2;
    uint8_t  auth_sec_req_enable = false;

    if (fmna_connection_is_fmna_paired())
    {
        uint8_t *p = NULL;
        uint8_t *q = NULL;
        ftl_load_from_storage(adv_direct_addr, FTL_SAVE_BT_MAC_ADDR, FTL_SAVE_BT_MAC_SIZE);
        le_cfg_local_identity_address(adv_direct_addr, GAP_IDENT_ADDR_RAND);
        le_set_gap_param(GAP_PARAM_RANDOM_ADDR, 6, adv_direct_addr);
        p = (uint8_t *)malloc(FTL_SAVE_PRI_KEY_SIZE);
        q = (uint8_t *)malloc(FTL_SAVE_SEC_KEY_SIZE);
        ftl_load_from_storage(p, FTL_SAVE_PRI_KEY_ADDR, FTL_SAVE_PRI_KEY_SIZE);
        ftl_load_from_storage(q, FTL_SAVE_SEC_KEY_ADDR, FTL_SAVE_SEC_KEY_SIZE);
        memcpy(&m_fmna_current_primary_key, p, sizeof(m_fmna_current_primary_key));
        memcpy(&m_fmna_current_secondary_key, q, sizeof(m_fmna_current_secondary_key));
        uint32_t for_load = 0;
        ftl_load_from_storage(&for_load, FTL_SAVE_NEXT_PW_ROT_INDEX_ADDR, FTL_SAVE_NEXT_PW_ROT_INDEX_SIZE);
        m_fmna_secondary_keys_info.next_secondary_key_rotation_index = for_load;
        free(p);
        free(q);
    }

    /* Set device name and device appearance */
    le_set_gap_param(GAP_PARAM_DEVICE_NAME, sizeof(device_name), device_name);
    le_set_gap_param(GAP_PARAM_APPEARANCE, sizeof(appearance), &appearance);
    le_set_gap_param(GAP_PARAM_SLAVE_INIT_GATT_MTU_REQ, sizeof(slave_init_mtu_req),
                     &slave_init_mtu_req);

    /* Setup the GAP Bond Manager */
    gap_set_param(GAP_PARAM_BOND_PAIRING_MODE, sizeof(auth_pair_mode), &auth_pair_mode);
    gap_set_param(GAP_PARAM_BOND_IO_CAPABILITIES, sizeof(auth_io_cap), &auth_io_cap);
    gap_set_param(GAP_PARAM_BOND_OOB_ENABLED, sizeof(auth_oob), &auth_oob);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY, sizeof(auth_fix_passkey), &auth_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_FIXED_PASSKEY_ENABLE, sizeof(auth_use_fix_passkey),
                      &auth_use_fix_passkey);
    le_bond_set_param(GAP_PARAM_BOND_SEC_REQ_ENABLE, sizeof(auth_sec_req_enable), &auth_sec_req_enable);
    le_bond_set_param(GAP_PARAM_BOND_KEY_MANAGER, sizeof(gap_param_key_manager),
                      &gap_param_key_manager);
}

/*============================================================================*
 *                              Global Functions
 *============================================================================*/

void fmna_gatt_platform_services_init(void)
{
#if CONFIG_REALTEK_FINDMY_USE_UARP
    fwus_srv_id = firmware_update_add_service(fmna_fwus_srv_cb);
#endif
    ais_srv_id = accessory_info_add_service(fmna_ais_srv_cb);
    fns_srv_id = findmy_network_add_service(fmna_fns_srv_cb);
    tps_srv_id = tps_add_service(fmna_tps_srv_cb);
}

void fmna_gatt_platform_init(void)
{
    fmna_gap_init();
    fmna_gatt_services_init();
    for (uint8_t i = 0; i < MAX_SUPPORTED_CONNECTIONS; i++)
    {
        m_fmna_active_connections[i].conn_handle = CONN_HANDLE_INVALID;
    }
}

void fmna_gatt_platform_send_authorized_write_reply(bool accept)
{
    //TODO: need check what to send, nordic -> sd_ble_gatts_rw_authorize_reply();
    auth_reply_flag = accept;
}

uint16_t fmna_gatt_platform_get_most_recent_conn_handle(void)
{
    APP_PRINT_INFO1("fmna_gatt_platform_get_most_recent_conn_handle: conn handle %#x",
                    recent_conn_handle);
    return recent_conn_handle;
}

void fmna_gatt_indication_state_set(bool new_state)
{
    allow_send_indication = new_state;
}

fmna_ret_code_t fmna_gatt_platform_send_indication(uint16_t conn_handle,
                                                   FMNA_Service_Opcode_t *p_opcode,
                                                   uint8_t *data, uint16_t length)
{
    T_SERVER_ID srv_id = 0xFF;
    uint16_t attr_idx = 0xFFFF;
    bool return_val;
    // If indication needs to be fragmented   TODO, check
    if (*p_opcode == FMNA_SERVICE_OPCODE_PACKET_EXTENSION)
    {
        *p_opcode = fmna_service_current_extended_packet_tx.opcode;
    }

    switch (*p_opcode & FMNA_SERVICE_OPCODE_BASE_MASK)
    {
    case FMNA_SERVICE_OPCODE_PAIRING_CONTROL_POINT_BASE:
        srv_id = fns_srv_id;
        attr_idx = FMNA_FNS_PAIRING_CP_INDEX;
        break;

    case FMNA_SERVICE_OPCODE_CONFIG_CONTROL_POINT_BASE:
        srv_id = fns_srv_id;
        attr_idx = FMNA_FNS_CONFIG_CP_INDEX;
        break;

    case FMNA_SERVICE_OPCODE_NON_OWNER_CONTROL_POINT_BASE:
        srv_id = fns_srv_id;
        attr_idx = FMNA_FNS_NON_OWNER_CP_INDEX;
        break;

    case FMNA_SERVICE_OPCODE_PAIRED_OWNER_CONTROL_POINT_BASE:
        srv_id = fns_srv_id;
        attr_idx = FMNA_FNS_PAIRED_OWNER_CP_INDEX;
        break;

#ifdef DEBUG
    case FMNA_SERVICE_OPCODE_DEBUG_CONTROL_POINT_BASE:
        srv_id = fns_srv_id;
        attr_idx = FMNA_FNS_DEBUG_CP_INDEX;
        break;
#endif // DEBUG

#if CONFIG_REALTEK_FINDMY_USE_UARP
    case FMNA_SERVICE_OPCODE_INTERNAL_UARP_BASE:
        srv_id = fwus_srv_id;
        attr_idx = FMNA_FWUS_DATA_CTRL_INDEX;
        break;
#endif

    default:
        APP_PRINT_INFO1("fmna_gatt_platform_send_indication: Unknown opcode 0x%x", *p_opcode);
        break;
    }
    DBG_BUFFER_RETARGET("fmna_gatt_platform_send_indication: srv 0x%x, attr 0x%x, len %d", srv_id,
                        attr_idx,
                        length);
    return_val = gatt_svc_send_data(le_get_conn_handle(conn_handle), L2C_FIXED_CID_ATT, srv_id,
                                    attr_idx, data,
                                    length, GATT_PDU_TYPE_INDICATION);
    return !return_val;
}

uint8_t fmna_gatt_platform_send_indication_busy(uint16_t conn_handle, FMNA_Service_Opcode_t opcode,
                                                void *data, uint16_t length)
{
    uint8_t queue_msg = 1;
// TODO: get cridits;fmna_gatt_platform_send_indication_busy
    if (allow_send_indication)
    {
        allow_send_indication = false;
        fmna_gatt_send_indication_internal(conn_handle, opcode, data, length);
    }
    else
    {
        //queue message
        T_FINDMY_BLE_INDICATION indication_msg;
        indication_msg.length = length;
        indication_msg.conn_handle = conn_handle;
        indication_msg.opcode = opcode;
        indication_msg.buf = malloc(length);
        memcpy(indication_msg.buf, data, length);
        os_msg_send(app_findmy_queue_handle, &indication_msg, 0);
    }

    return queue_msg;
}

void fmna_gatt_platform_reset_indication_queue(void)
{
//    le_bond_clear_all_keys();
    allow_send_indication = true;

//    CRITICAL_REGION_ENTER();
//TODO: clear send buffer/cridits
//    CRITICAL_REGION_EXIT();
}

uint8_t fmna_gatt_platform_get_next_command_response_index(void)
{
//TODO: check if necessary
    uint8_t index;
//    CRITICAL_REGION_ENTER();
    index = m_command_response_index;
//    m_command_response_index++;
//    if (m_command_response_index <= MAX_CONTROL_POINT_RSP)
//    {
//        m_command_response_index = 0;
//    }
//    CRITICAL_REGION_EXIT();
    return index;
}

void fmna_gatt_platform_send_next_indication(void)
{
// TODO: create a queue to store packets when cridits exceeds 9
    T_FINDMY_BLE_INDICATION msg;
    if (os_msg_recv(app_findmy_queue_handle, &msg, 0) == true)
    {
        fmna_gatt_send_indication_internal(msg.conn_handle, msg.opcode, msg.buf, msg.length);
        APP_PRINT_INFO0("fmna_gatt_platform_send_next_indication: Send next packet");
        os_mem_free(msg.buf);
    }
}

/// Function for handling the Connect event.
/// @param conn_id Event received from the BLE stack.
void fmna_on_connect(uint8_t conn_id)
{
    // The config handle servers as the most recently connected device.
    // This allows for easier commnuication with the device during:
    // Pairing lock
    // UT Play sound
    // connected / not encrypted devices
    recent_conn_handle = conn_id;
    findmy_conn_handle = le_get_conn_handle(conn_id);
#if CONFIG_REALTEK_FINDMY_USE_UARP
    fmna_uarp_connect(conn_id);
#endif
}

/// Function for handling the Disconnect event.
/// @param conn_id Event received from the BLE stack.
void fmna_on_disconnect(uint8_t conn_id)
{
#if CONFIG_REALTEK_FINDMY_USE_UARP
    fmna_uarp_disconnect(conn_id);
#endif
    if (recent_conn_handle == conn_id)
    {
        recent_conn_handle = BLE_CONN_HANDLE_INVALID;
        findmy_conn_handle = BLE_CONN_HANDLE_INVALID;
    }

    app_findmy_disconnect();
}

bool fmna_is_connect_by_handle(uint16_t conn_handle)
{
    return findmy_conn_handle == conn_handle;
}
#endif
