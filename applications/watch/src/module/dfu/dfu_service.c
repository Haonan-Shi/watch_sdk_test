/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/
#include "app_main.h"
#include "app_cfg.h"
#include "bt_types.h"
#include "dfu_transport.h"
#include "dfu_service.h"
#include "gatt.h"
#include "gap_conn_le.h"
#include "trace.h"
#include "bt_gatt_svc.h"

/** @defgroup  OTA_SERVICE OTA Service
    * @brief LE Service to implement OTA feature
    * @{
    */

/*============================================================================*
 *                              Macros
 *============================================================================*/
/** @defgroup OTA_SERVICE_Exported_Macros OTA service Exported Macros
    * @brief
    * @{
    */

/** End of OTA_SERVICE_Exported_Macros
    * @}
    */

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @defgroup OTA_SERVICE_Exported_Constants OTA service Exported Constants
    * @{
    */
const uint8_t gatt_uuid128_dfu_service[16] = {BLE_GATT_UUID128_DFU_SERVICE};
/** @brief  OTA profile/service definition
*   @note   Here is an example of OTA service table including Write
*/
static const T_ATTRIB_APPL dfu_service_table[] =
{
    /*-------------------------- DFU Service ---------------------------*/
    /* <<Primary Service>>, .. */
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),                /* flags */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),             /* type_value */
        },
        UUID_128BIT_SIZE,                                   /* bValueLen */
        (void *)gatt_uuid128_dfu_service,                   /* p_value_context */
        GATT_PERM_READ                                      /* permissions  */
    },

    /* <<Characteristic>>, .. */
    {
        ATTRIB_FLAG_VALUE_INCL,                             /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE_NO_RSP,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                                  /* bValueLen */
        NULL,
        GATT_PERM_READ                                      /* permissions */
    },
    /*--- DFU packet characteristic value ---*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,   /* flags */
        {   /* type_value */
            BLE_GATT_UUID128_DFU_PACKET
        },
        0,                                                  /* bValueLen */
        NULL,
        GATT_PERM_WRITE                                     /* permissions */
    },
    /* <<Characteristic>>, .. */
    {
        ATTRIB_FLAG_VALUE_INCL,                             /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE |                         /* characteristic properties */
             GATT_CHAR_PROP_NOTIFY)
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                                  /* bValueLen */
        NULL,
        GATT_PERM_READ                                      /* permissions */
    },
    /*--- DFU Control Point value ---*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,   /* flags */
        {   /* type_value */
            BLE_GATT_UUID128_DFU_CONTROL_POINT
        },
        0,                                                  /* bValueLen */
        NULL,
        GATT_PERM_WRITE                                     /* permissions */
    },
    /* client characteristic configuration */
    {
        (ATTRIB_FLAG_VALUE_INCL |                           /* flags */
         ATTRIB_FLAG_CCCD_APPL),
        {   /* type_value */
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            /* NOTE: this value has an instantiation for each client, a write to */
            /* this attribute does not modify this default value.                */
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT),       /* client char. config. bit field */
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,                                                  /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)                  /* permissions */
    }
};

/** End of OTA_SERVICE_Exported_Constants
    * @}
    */

/*============================================================================*
 *                              Variables
 *============================================================================*/
/** @defgroup OTA_SERVICE_Exported_Variables OTA service Exported Variables
    * @brief
    * @{
    */

/** @brief  Service ID only used in this file */
static T_SERVER_ID dfu_srv_id_local;

/** @brief  Function pointer used to send event to application from OTA service
*   @note   It is initiated in ota_add_service()
*/
static P_FUN_DFU_SERVER_APP_CB p_fn_dfu_service_cb = NULL;

/** End of OTA_SERVICE_Exported_Variables
    * @}
    */

/*============================================================================*
 *                              Private Functions
 *============================================================================*/
/** @defgroup OTA_SERVICE_Exported_Functions OTA service Exported Functions
    * @brief
    * @{
    */
void dfu_service_write_post_callback(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                                     uint16_t attrib_index,
                                     uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("dfu_service_write_post_callback: conn_handle %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_handle, service_id, attrib_index, length);
}

static T_APP_RESULT dfu_service_attr_write_cb(uint16_t conn_handle, uint16_t cid,
                                              T_SERVER_ID service_id,
                                              uint16_t attr_index, T_WRITE_TYPE write_type, uint16_t length,
                                              uint8_t *p_value, P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    uint8_t conn_id = 0xFF;
    le_get_conn_id_by_handle(conn_handle, &conn_id);

    T_DFU_CALLBACK_DATA callback_data;
    T_APP_RESULT  cause = APP_RESULT_SUCCESS;

    APP_PRINT_INFO2("dfu_service_attr_write_cb: attr_index 0x%02x, length %d", attr_index, length);

//    app_reg_le_link_disc_cb(conn_id, dfu_service_le_disconnect_cb);

    if (INDEX_DFU_CHAR_DFU_PACKET_INDEX == attr_index)
    {
        return dfu_service_handle_packet(conn_id, length, p_value);
    }
    else if (INDEX_DFU_CHAR_DFU_CONTROL_POINT_INDEX == attr_index)
    {
        return dfu_service_handle_cp_req(conn_id, length, p_value);
    }
    else
    {
        APP_PRINT_ERROR0("dfu_service_attr_write_cb: unknown attr_index");
        cause = APP_RESULT_ATTR_NOT_FOUND;
    }

    /* Notify Application. */
    if (p_fn_dfu_service_cb)
    {
        cause = p_fn_dfu_service_cb(GATT_MSG_DFU_SERVER_WRITE, (void *)&callback_data);
    }

    return cause;

}

/**
 * @brief update CCCD bits from stack.
 *
 * @param ServiceId          Service ID.
 * @param Index          Attribute index of characteristic data.
 * @param wCCCBits         CCCD bits from stack.
 * @return None
*/
void dfu_service_cccd_update_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                                uint16_t index,
                                uint16_t ccc_bits)
{
    uint8_t conn_id = 0xFF;
    le_get_conn_id_by_handle(conn_handle, &conn_id);

    T_DFU_CALLBACK_DATA callback_data;
    callback_data.conn_id = conn_id;
    bool b_handle = true;
    DFU_PRINT_INFO2("dfu_cccd_update_cb: index=%d, ccc_bits=0x%x", index, ccc_bits);
    switch (index)
    {
    case INDEX_DFU_CHAR_DFU_CP_CCCD_INDEX:
        {
            if (ccc_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                // Enable Notification
                callback_data.msg_data.notification_indification_index = DFU_CP_NOTIFY_ENABLE;
                DFU_PRINT_INFO0("dfu_cccd_update_cb: Notification Enable");
            }
            else
            {
                callback_data.msg_data.notification_indification_index = DFU_CP_NOTIFY_DISABLE;
                DFU_PRINT_INFO0("dfu_cccd_update_cb: Notification Disable");
            }
            break;
        }
    default:
        {
            b_handle = false;
            break;
        }

    }
    /* Notify Application. */
    if (p_fn_dfu_service_cb && (b_handle == true))
    {
        T_APP_RESULT update_cause = p_fn_dfu_service_cb(GATT_MSG_DFU_SERVER_CCCD_UPDATE,
                                                        (void *)&callback_data);
        if (update_cause != APP_RESULT_SUCCESS)
        {
            return;
        }
    }

    return;
}

/**
    * @brief    Send notification to peer side
    * @param    conn_id  PID to identify the connection
    * @param    p_data  value to be send to peer
    * @param    data_len  data length of the value to be send
    * @return   void
    */
void dfu_service_send_notification(uint16_t conn_handle, uint16_t cid, uint8_t *p_data,
                                   uint16_t data_len)
{
    gatt_svc_send_data(conn_handle, cid, dfu_srv_id_local,
                       INDEX_DFU_CHAR_DFU_CONTROL_POINT_INDEX, p_data, data_len,
                       GATT_PDU_TYPE_NOTIFICATION);
}

/** @brief  OTA BLE Service Callbacks */
static const T_FUN_GATT_EXT_SERVICE_CBS dfu_service_cbs =
{
    NULL,   /**< Read callback function pointer */
    dfu_service_attr_write_cb,  /**< Write callback function pointer */
    dfu_service_cccd_update_cb,  /**< CCCD update callback function pointer */
};
/**
    * @brief    Add OTA BLE service to application
    * @param    app_cb  Callback when service attribute was read, write or cccd update.
    * @return   Service ID auto generated by profile layer
    * @retval   A T_SERVER_ID type value
    */
T_SERVER_ID dfu_reg_srv(P_FUN_DFU_SERVER_APP_CB app_cb)
{
    T_SERVER_ID service_id;

    if (false == gatt_svc_add(&service_id,
                              (uint8_t *)dfu_service_table,
                              sizeof(dfu_service_table),
                              &dfu_service_cbs, NULL))
    {
        APP_PRINT_ERROR1("dfu_add_service: service_id %d", service_id);
        service_id = 0xff;
    }
    p_fn_dfu_service_cb = app_cb;
    dfu_srv_id_local = service_id;
    return service_id;
}