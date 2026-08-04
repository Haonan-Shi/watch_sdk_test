/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include <trace.h>
#include <upay_service.h>
#include <gap.h>
#include "alipay_config.h"
#include "gap_conn_le.h"

#if (CONFIG_ALIPAY)

static T_SERVER_ID upay_service_id;

/**<  Function pointer used to send event to application from simple profile. Initiated in simp_ble_service_add_service. */
static P_FUN_UPAY_SERVER_APP_CB pfn_upay_service_cb = NULL;

uint8_t alipay_mac[6] = {0};

/**< @brief  profile/service definition.  */
const T_ATTRIB_APPL upay_service_tbl[] =
{
    /* <<Primary Service>>, .. */
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE),  /* flags     */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
            LO_WORD(GATT_UUID_UPAY_SERVICE),              /* service UUID */
            HI_WORD(GATT_UUID_UPAY_SERVICE)
        },
        UUID_16BIT_SIZE,                            /* bValueLen     */
        NULL,                                       /* p_value_context */
        GATT_PERM_READ                              /* permissions  */
    },
    /* <<Characteristic>>, .. */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE |                   /* characteristic properties */
             GATT_CHAR_PROP_NOTIFY | GATT_CHAR_PROP_READ)
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },

    {
        ATTRIB_FLAG_VALUE_APPL,                              /* flags */
        {                                                           /* type_value */
            LO_WORD(GATT_UPAY_CHARACTERISTIC),
            HI_WORD(GATT_UPAY_CHARACTERISTIC)
        },
        0,                                                 /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)           /* permissions */
    },
    /* client characteristic configuration */
    {
        (ATTRIB_FLAG_VALUE_INCL |                   /* flags */
         ATTRIB_FLAG_CCCD_APPL),
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            /* NOTE: this value has an instantiation for each client, a write to */
            /* this attribute does not modify this default value:                */
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT), /* client char. config. bit field */
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,                                          /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)          /* permissions */
    }
    ,
    /* client characteristic configuration 0x4b02*/
    {
        (ATTRIB_FLAG_VOID),
        {                                           /* type_value */
            LO_WORD(GATT_UUID_CHAR_DESCRIPTOR_ALIPAY),
            HI_WORD(GATT_UUID_CHAR_DESCRIPTOR_ALIPAY),

        },
        2,                                          /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)          /* permissions */
    }
};

/**
 * @brief read characteristic data from service.
 *
 * @param service_id          ServiceID of characteristic data.
 * @param attrib_index        Attribute index of getting characteristic data.
 * @param offset              Used for Blob Read.
 * @param p_length            length of getting characteristic data.
 * @param pp_value            data got from service.
 * @return Profile procedure result
*/
T_APP_RESULT  upay_service_attr_read_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                                        uint16_t attrib_index, uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT  cause  = APP_RESULT_SUCCESS;

    switch (attrib_index)
    {
    default:
        APP_PRINT_ERROR1("[alipay] upay service read, Attr not found, index %d", attrib_index);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    case INDEX_UPAY_READ:
        {
//            T_UPAYS_CALLBACK_DATA callback_data;
//            callback_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
//            callback_data.msg_data.read_value_index = UPAY_READ;
//            callback_data.conn_id = conn_id;
//            if (pfn_upay_service_cb)
//            {
//                pfn_upay_service_cb(service_id, (void *)&callback_data);
//            }
//            *pp_value = (uint8_t *)0x00283144;
//            *p_length = 0x06;
        }
        break;
    case INDEX_UPAY_CHAR_DESCRIPTOR:
        {
            uint8_t *p = (uint8_t *)ALIPAY_BT_MAC_RAM_ARRD;
            alipay_mac[0] = p[5];
            alipay_mac[1] = p[4];
            alipay_mac[2] = p[3];
            alipay_mac[3] = p[2];
            alipay_mac[4] = p[1];
            alipay_mac[5] = p[0];

            *pp_value = (uint8_t *)alipay_mac;
            *p_length = 0x06;
        }
        break;

    }

    return (cause);
}

/**
 * @brief write characteristic data from service.
 *
 * @param conn_id
 * @param service_id        ServiceID to be written.
 * @param attrib_index      Attribute index of characteristic.
 * @param length            length of value to be written.
 * @param p_value           value to be written.
 * @return Profile procedure result
*/
T_APP_RESULT upay_service_attr_write_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                                        uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                                        P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_UPAYS_CALLBACK_DATA callback_data;
    T_APP_RESULT  cause = APP_RESULT_SUCCESS;

    uint8_t conn_id = 0xFF;
    le_get_conn_id_by_handle(conn_handle, &conn_id);
    APP_PRINT_INFO1("[alipay] upay service write_type = 0x%x", write_type);
    if (INDEX_UPAY_WRITE == attrib_index)
    {
        /* Make sure written value size is valid. */
        if (p_value == NULL)
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            void  alipay_ble_set_connect_id(uint8_t connect_id);
            alipay_ble_set_connect_id(conn_id);

            /* Notify Application. */
            callback_data.conn_id  = conn_id;
            callback_data.msg_data.write.opcode = UPAY_WRITE;
            callback_data.msg_data.write.write_type = write_type;
            callback_data.msg_data.write.len = length;
            callback_data.msg_data.write.p_value = p_value;

            if (pfn_upay_service_cb)
            {
                pfn_upay_service_cb(GATT_MSG_UPAY_SERVER_WRITE, (void *)&callback_data);
            }
        }
    }
    else
    {
        APP_PRINT_ERROR2("[alipay] upay_service_attr_write_cb Error: attrib_index 0x%x, length %d",
                         attrib_index,
                         length);
        cause = APP_RESULT_ATTR_NOT_FOUND;
    }
    return cause;
}

/**
 * @brief update CCCD bits from stack.
 *
 * @param conn_id           connection id.
 * @param service_id          Service ID.
 * @param index          Attribute index of characteristic data.
 * @param cccbits         CCCD bits from stack.
 * @return None
*/
void upay_service_cccd_update_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                                 uint16_t index,
                                 uint16_t cccbits)
{
    uint8_t conn_id = 0xFF;
    T_UPAYS_CALLBACK_DATA callback_data;
    bool is_handled = false;
    le_get_conn_id_by_handle(conn_handle, &conn_id);
    callback_data.conn_id = conn_id;
    APP_PRINT_INFO2("[alipay] upay_service_cccd_update_cb: index = %d, cccbits 0x%x", index, cccbits);
    switch (index)
    {
    case INDEX_UPAY_CHAR_CCCD:
        {
            if (cccbits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                // Enable Notification
                callback_data.msg_data.notification_indification_index = UPAY_NOTIFY_INDICATE_ENABLE;
            }
            else
            {
                // Disable Notification
                callback_data.msg_data.notification_indification_index = UPAY_NOTIFY_INDICATE_DISABLE;
            }
            is_handled =  true;
        }
        break;

    default:
        break;
    }
    /* Notify Application. */
    if (pfn_upay_service_cb && (is_handled == true))
    {
        pfn_upay_service_cb(GATT_MSG_UPAY_SERVER_CCCD_UPDATE, (void *)&callback_data);
        upay_data_notify(conn_handle, cid, NULL, 0);
    }
}

/**
 * @brief       Send notify upay data .
 *
 * @param[in]   conn_id  Connection id.
 * @param[in]   service_id  Service id.
 * @param[in]   battery_level  Battery level value.
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 * \endcode
 */
bool upay_data_notify(uint16_t conn_handle, uint16_t cid, uint8_t *data, uint32_t data_len)
{

    return gatt_svc_send_data(conn_handle, cid, upay_service_id, INDEX_UPAY_NOTIFY, data,
                              data_len, GATT_PDU_TYPE_NOTIFICATION);
}

/**
 * @brief Simple ble Service Callbacks.
*/
const T_FUN_GATT_EXT_SERVICE_CBS upay_service_cbs =
{
    upay_service_attr_read_cb,  // Read callback function pointer
    upay_service_attr_write_cb, // Write callback function pointer
    upay_service_cccd_update_cb // CCCD update callback function pointer
};


/**
  * @brief Add simple BLE service to the BLE stack database.
  *
  * @param[in]   p_func  Callback when service attribute was read, write or cccd update.
  * @return Service id generated by the BLE stack: @ref T_SERVER_ID.
  * @retval 0xFF Operation failure.
  * @retval others Service id assigned by stack.
  *
  */
T_SERVER_ID upay_reg_srv(P_FUN_UPAY_SERVER_APP_CB app_cb, P_FUN_GATT_EXT_SEND_DATA_CB send_cb)
{
    T_SERVER_ID service_id;
    if (false == gatt_svc_add(&service_id,
                              (uint8_t *)upay_service_tbl,
                              sizeof(upay_service_tbl),
                              &upay_service_cbs, send_cb))
    {
        APP_PRINT_ERROR0("[alipay] server_add_service: fail");
        service_id = 0xff;
        return service_id;
    }

    pfn_upay_service_cb = app_cb;
    upay_service_id = service_id;
    return service_id;
}

#endif // CONFIG_ALIPAY
