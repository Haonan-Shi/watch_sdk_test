/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/** Add Includes here **/
#include <stdlib.h>
#include <trace.h>
#include <string.h>
#include "walkie_talkie_gatt_client.h"
#include "walkie_talkie_gatt_svc.h"
#include "walkie_talkie_voice.h"
#include "walkie_talkie_adv.h"
#include "walkie_talkie_app.h"

/********************************************************************************************************
* local static variables defined here, only used in this source file.
********************************************************************************************************/
const uint8_t walkie_talkie_service_uuid[16] = {GATT_UUID128_WALKIE_TALKIE_SERVICE};
const uint8_t control_point_uuid[16] = {GATT_UUID128_WALKIE_TALKIE_CONTROL_POINT};
const uint8_t write_uuid[16] = {GATT_UUID128_WALKIE_TALKIE_WRITE};
const uint8_t notify_uuid[16] = {GATT_UUID128_WALKIE_TALKIE_NOTIFY};
/**<  Callback used to send data to app from WTS client layer. */
static P_FUN_WTS_CLIENT_APP_CB wts_client_cb = NULL;

/**
  * @brief      Config the WTS service control point CCCD data.
  * @param[in]  conn_handle     Connection handle of the ACL link.
  * @param[in]  srv_instance_id Service instance id.
  * @param[in]  enable          Whether to enable CCCD.
  * \arg    true    Enable CCCD.
  * \arg    false   Disable CCCD.
  * @return Result of config the WTS service CCCD data.
  * @retval true  Config the WTS service CCCD data success.
  * @retval false Config the WTS service CCCD data failed.
  */
bool wts_client_cfg_control_point_cccd(uint16_t conn_handle, uint8_t srv_instance_id, bool enable)
{
    T_ATTR_UUID srv_uuid;
    T_ATTR_UUID char_uuid;
    T_GAP_CAUSE cause = GAP_CAUSE_INVALID_PARAM;
    srv_uuid.is_uuid16 = false;
    srv_uuid.instance_id = srv_instance_id;
    memcpy(srv_uuid.p.uuid128, walkie_talkie_service_uuid, 16);
    char_uuid.is_uuid16 = false;
    char_uuid.instance_id = 0;
    memcpy(char_uuid.p.uuid128, control_point_uuid, 16);
    uint8_t cccd_cfg;
    if (enable)
    {
        cccd_cfg = GATT_CLIENT_CONFIG_ALL;
    }
    else
    {
        cccd_cfg = GATT_CLIENT_CONFIG_DISABLE;
    }

    if (gatt_client_check_cccd_enabled(conn_handle, &srv_uuid, &char_uuid) != enable)
    {
        cause = gatt_client_enable_char_cccd(conn_handle, &srv_uuid, &char_uuid, cccd_cfg);
    }

    return (cause == GAP_CAUSE_SUCCESS) ? true : false;
}

/**
  * @brief      Config the WTS service notify CCCD data.
  * @param[in]  conn_handle     Connection handle of the ACL link.
  * @param[in]  srv_instance_id Service instance id.
  * @param[in]  enable          Whether to enable CCCD.
  * \arg    true    Enable CCCD.
  * \arg    false   Disable CCCD.
  * @return Result of config the WTS service CCCD data.
  * @retval true  Config the WTS service CCCD data success.
  * @retval false Config the WTS service CCCD data failed.
  */
bool wts_client_cfg_notify_cccd(uint16_t conn_handle, uint8_t srv_instance_id, bool enable)
{
    T_ATTR_UUID srv_uuid;
    T_ATTR_UUID char_uuid;
    T_GAP_CAUSE cause = GAP_CAUSE_INVALID_PARAM;
    srv_uuid.is_uuid16 = false;
    srv_uuid.instance_id = srv_instance_id;
    memcpy(srv_uuid.p.uuid128, walkie_talkie_service_uuid, 16);
    char_uuid.is_uuid16 = false;
    char_uuid.instance_id = 0;
    memcpy(char_uuid.p.uuid128, notify_uuid, 16);
    uint8_t cccd_cfg;
    if (enable)
    {
        cccd_cfg = GATT_CLIENT_CONFIG_ALL;
    }
    else
    {
        cccd_cfg = GATT_CLIENT_CONFIG_DISABLE;
    }

    if (gatt_client_check_cccd_enabled(conn_handle, &srv_uuid, &char_uuid) != enable)
    {
        cause = gatt_client_enable_char_cccd(conn_handle, &srv_uuid, &char_uuid, cccd_cfg);
    }

    return (cause == GAP_CAUSE_SUCCESS) ? true : false;
}

/**
  * @brief  Used by application, to write control point data characteristic value.
  * @param[in]  conn_handle  Connection handle of the ACL link.
  * @param[in]  buf  data buf for control point data.
  * @param[in]  len  data length for control point data.
  * @retval true send write command to upper stack success.
  * @retval false send write command to upper stack failed.
  */
bool wts_client_write_control_point_data(uint16_t conn_handle, uint8_t *buf, uint16_t len)
{
    T_ATTR_UUID srv_uuid;
    T_ATTR_UUID char_uuid;
    T_GAP_CAUSE cause = GAP_CAUSE_INVALID_PARAM;
    uint16_t handle = 0;
    T_GATT_WRITE_TYPE type = GATT_WRITE_TYPE_CMD;

    srv_uuid.is_uuid16 = false;
    srv_uuid.instance_id = 0;
    memcpy(srv_uuid.p.uuid128, walkie_talkie_service_uuid, 16);
    char_uuid.is_uuid16 = false;
    char_uuid.instance_id = 0;
    memcpy(char_uuid.p.uuid128, control_point_uuid, 16);

    if (gatt_client_find_char_handle(conn_handle, &srv_uuid, &char_uuid, &handle))
    {
        cause = gatt_client_write(conn_handle, type, handle, len, buf, NULL);
    }

    if (cause == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**
  * @brief  Used by application, to write voice data characteristic value.
  * @param[in]  conn_handle  Connection handle of the ACL link.
  * @param[in]  buf  data buf for voice data.
  * @param[in]  len  data length for voice data.
  * @retval true send write command to upper stack success.
  * @retval false send write command to upper stack failed.
  */
bool wts_client_write_voice_data(uint16_t conn_handle, uint8_t *buf, uint16_t len)
{
    T_ATTR_UUID srv_uuid;
    T_ATTR_UUID char_uuid;
    T_GAP_CAUSE cause = GAP_CAUSE_INVALID_PARAM;
    uint16_t handle = 0;
    T_GATT_WRITE_TYPE type = GATT_WRITE_TYPE_CMD;

    srv_uuid.is_uuid16 = false;
    srv_uuid.instance_id = 0;
    memcpy(srv_uuid.p.uuid128, walkie_talkie_service_uuid, 16);
    char_uuid.is_uuid16 = false;
    char_uuid.instance_id = 0;
    memcpy(char_uuid.p.uuid128, write_uuid, 16);

    if (gatt_client_find_char_handle(conn_handle, &srv_uuid, &char_uuid, &handle))
    {
        cause = gatt_client_write(conn_handle, type, handle, len, buf, NULL);
    }

    if (cause == GAP_CAUSE_SUCCESS)
    {
        return true;
    }
    else
    {
        return false;
    }
}

T_APP_RESULT wts_client_cbs(uint16_t conn_handle, T_GATT_CLIENT_EVENT type, void *p_data)
{
    T_APP_RESULT  result = APP_RESULT_SUCCESS;
    T_GATT_CLIENT_DATA *p_client_cb_data = (T_GATT_CLIENT_DATA *)p_data;

    APP_PRINT_INFO1("wts_client_cbs type = %d", type);

    // 1) service discovery done,
    // 2) enable notify cccd
    // 3) enable control point cccd
    // 4) write control point data to sync information

    switch (type)
    {
    case GATT_CLIENT_EVENT_DIS_DONE:
        {
            T_WTS_CLIENT_DIS_DONE dis_done = {0};
            dis_done.is_found = p_client_cb_data->dis_done.is_found;
            dis_done.load_from_ftl = p_client_cb_data->dis_done.load_from_ftl;
            dis_done.srv_instance_num = p_client_cb_data->dis_done.srv_instance_num;

            wts_client_cfg_notify_cccd(conn_handle, 0, true);

            if (wts_client_cb)
            {
                result = wts_client_cb(conn_handle, GATT_MSG_WTS_CLIENT_DIS_DONE, &dis_done);
            }
        }
        break;

    case GATT_CLIENT_EVENT_NOTIFY_IND:
        {
            if (!memcmp(control_point_uuid, p_client_cb_data->notify_ind.char_uuid.p.uuid128, 16))
            {
                walkie_talkie_service_handle_cp_req(conn_handle, p_client_cb_data->notify_ind.cid,
                                                    p_client_cb_data->notify_ind.value_size, p_client_cb_data->notify_ind.p_value);
                // if (wts_client_cb)
                // {
                //     result = wts_client_cb(conn_handle, GATT_MSG_WTS_CLIENT_CONTROL_POINT_NOTIFY, &(p_client_cb_data->notify_ind));
                // }
            }
            else if (!memcmp(notify_uuid, p_client_cb_data->notify_ind.char_uuid.p.uuid128, 16))
            {
                APP_PRINT_INFO2("wts notify p_value = 0x%x, length = %d", p_client_cb_data->notify_ind.p_value,
                                p_client_cb_data->notify_ind.value_size);
                walkie_talkie_player_data_parser(p_client_cb_data->notify_ind.p_value,
                                                 p_client_cb_data->notify_ind.value_size);
                // if (wts_client_cb)
                // {
                //     result = wts_client_cb(conn_handle, GATT_MSG_WTS_CLIENT_NOTIFY, &(p_client_cb_data->notify_ind));
                // }
            }
        }
        break;
#if 0
    case GATT_CLIENT_EVENT_WRITE_RESULT:
        {
            if (!memcmp(write_uuid, p_client_cb_data->write_result.char_uuid.uuid128, 16))
            {
                if (wts_client_cb)
                {
                    result = wts_client_cb(conn_handle, GATT_MSG_WTS_CLIENT_WRITE_RESULT, &notify_data);
                }
            }
        }
        break;
#endif

    case GATT_CLIENT_EVENT_CCCD_CFG:
        {
            T_WTS_CLIENT_CCCD_CFG_RESULT cccd_info = {0};

            cccd_info.srv_instance_id = p_client_cb_data->cccd_cfg.srv_instance_id;
            cccd_info.cause = p_client_cb_data->cccd_cfg.cause;

            if (!memcmp(control_point_uuid, p_client_cb_data->cccd_cfg.char_uuid.p.uuid128, 16))
            {
                if (p_client_cb_data->cccd_cfg.cccd_cfg & GATT_CLIENT_CONFIG_NOTIFICATION)
                {
                    cccd_info.enable = true;
                }
                else
                {
                    cccd_info.enable = false;
                }
                if (wts_client_cb)
                {
                    result = wts_client_cb(conn_handle, GATT_MSG_WTS_CLIENT_CONTROL_POINT_CCCD_CFG_RESULT, &cccd_info);
                }
                uint8_t user_name_exchange[USER_NAME_LEN + 1] = {0};
                user_name_exchange[0] = WALKIE_TALKIE_CP_NAME;
                memcpy(user_name_exchange + 1, transmit_adv_data + USER_NAME_OFFSET, USER_NAME_LEN);
                wts_client_write_control_point_data(conn_handle, user_name_exchange, USER_NAME_LEN + 1);
            }
            else if (!memcmp(notify_uuid, p_client_cb_data->cccd_cfg.char_uuid.p.uuid128, 16))
            {
                if (p_client_cb_data->cccd_cfg.cccd_cfg & GATT_CLIENT_CONFIG_NOTIFICATION)
                {
                    cccd_info.enable = true;
                }
                else
                {
                    cccd_info.enable = false;
                }
                if (wts_client_cb)
                {
                    result = wts_client_cb(conn_handle, GATT_MSG_WTS_CLIENT_NOTIFY_CCCD_CFG_RESULT, &cccd_info);
                }

                wts_client_cfg_control_point_cccd(conn_handle, 0, true);
            }
        }
        break;

    default:
        break;
    }

    return result;
}

/**
  * @brief      Add wts client to application.
  * @param[in]  app_cb   pointer of app callback function to handle specific client module data.
  * @return Result of add the specific client module.
  * @retval true  Add client module success.
  * @retval false Add client module failed.
  */
bool wts_client_init(P_FUN_WTS_CLIENT_APP_CB app_cb)
{
    T_ATTR_UUID srv_uuid = {0};
    srv_uuid.is_uuid16 = false;
    memcpy(srv_uuid.p.uuid128, walkie_talkie_service_uuid, 16);

    if (gatt_client_spec_register(&srv_uuid, wts_client_cbs) == GAP_CAUSE_SUCCESS)
    {
        /* register callback for profile to inform application that some events happened. */
        wts_client_cb = app_cb;
        return true;
    }

    return false;
}

