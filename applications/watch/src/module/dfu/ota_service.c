/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                              Header Files
 *============================================================================*/

#include "app_cfg.h"
#include "app_main.h"
#include "bt_types.h"
#include "dfu_api.h"
#include "gap_conn_le.h"
#include "bt_gatt_svc.h"
#include "dfu_transport.h"
#include "dfu_common.h"
#include "gatt.h"
#include "ota_service.h"
#include "trace.h"

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

/* Indicate whether support ota image transfer during normal working mode */
#define OTA_NORMAL_MODE 1

/** End of OTA_SERVICE_Exported_Macros
  * @}
  */

/*============================================================================*
 *                              Constants
 *============================================================================*/
/** @defgroup OTA_SERVICE_Exported_Constants OTA service Exported Constants
  * @{
  */

/** @brief  OTA service UUID */
static const uint8_t GATT_UUID_OTA_SERVICE[16] = { 0x12, 0xA2, 0x4D, 0x2E, 0xFE, 0x14, 0x48, 0x8e, 0x93, 0xD2, 0x17, 0x3C, 0xFF, 0xD0, 0x00, 0x00};

/** @brief  OTA profile/service definition
*   @note   Here is an example of OTA service table including Write
*/
static const T_ATTRIB_APPL ota_service_table[] =
{
    /*--------------------------OTA Service ---------------------------*/
    /* <<Primary Service>>, .. 0 */
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),        /* flags */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),     /* type_value */
        },
        UUID_128BIT_SIZE,                           /* bValueLen */
        (void *)GATT_UUID_OTA_SERVICE,              /* p_value_context */
        GATT_PERM_READ                              /* permissions */
    },

    /* <<Characteristic1>>, .. 1 */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE_NO_RSP,            /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,     /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    /*  OTA characteristic value 2 */
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHAR_OTA),
            HI_WORD(GATT_UUID_CHAR_OTA),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ | GATT_PERM_WRITE            /* permissions */
    },

    /* <<Characteristic2>>, .. 3, MAC Address */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,     /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    /*  OTA characteristic value 4 */
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHAR_MAC),
            HI_WORD(GATT_UUID_CHAR_MAC),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ                              /* permissions */
    },

    /* <<Characteristic3>>, .. 5, Patch version */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,     /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    /*  OTA characteristic value 6 */
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHAR_PATCH_VERSION),
            HI_WORD(GATT_UUID_CHAR_PATCH_VERSION),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ                              /* permissions */
    },

    /* <<Characteristic4>>, .. 7 App version */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            //XXXXMJMJ GATT_CHAR_PROP_INDICATE,     /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },
    /*  OTA characteristic value 8 */
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHAR_APP_VERSION),
            HI_WORD(GATT_UUID_CHAR_APP_VERSION),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ                              /* permissions */
    },

    /* <<Characteristic5>>, .. 9, Extended Device information */
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* permissions */
    },

    /*  OTA characteristic value 10 */
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHAR_DEVICE_INFO),
            HI_WORD(GATT_UUID_CHAR_DEVICE_INFO),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ                              /* permissions */
    },

    /* <<Characteristic6>>, .. 11*/
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },

    /*  OTA characteristic value 12*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_IMAGE_VERSION_FIRST),
            HI_WORD(GATT_UUID_CHAR_IMAGE_VERSION_FIRST),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic7>>, .. 13*/
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },

    /*  OTA characteristic value 14*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_IMAGE_VERSION_SECOND),
            HI_WORD(GATT_UUID_CHAR_IMAGE_VERSION_SECOND),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic8>>, .. 15*/
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },

    /*  OTA characteristic value 16*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_PROTOCOL_INFO),
            HI_WORD(GATT_UUID_CHAR_PROTOCOL_INFO),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic9>>, .. 17*/
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },

    /*  OTA characteristic value 18*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_SECTION_SIZE_FIRST),
            HI_WORD(GATT_UUID_CHAR_SECTION_SIZE_FIRST),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic10>>, .. 19*/
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },

    /*  OTA characteristic value 20*/
    {
        ATTRIB_FLAG_VALUE_APPL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_SECTION_SIZE_SECOND),
            HI_WORD(GATT_UUID_CHAR_SECTION_SIZE_SECOND),
        },
        0,                                          /* variable size */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },
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
static T_SERVER_ID ota_srv_id_local;

/** @brief  Function pointer used to send event to application from OTA service
*   @note   It is initiated in ota_add_service()
*/
static P_FUN_OTA_SERVER_APP_CB p_fn_ota_service_cb = NULL;

/** @brief  Array used to temporarily store BD Addr */
static uint8_t mac_addr[12];

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
/**
     * @brief    Callback function invoked after a client writes to an OTA service characteristic.
     *           This function processes the data received from a GATT Write Request or Command.
     *
     * @param    conn_handle   Connection handle identifying the BLE link.
     * @param    cid           Characteristic ID (local identifier) of the attribute that was written.
     * @param    service_id    ID of the service containing the written attribute.
     * @param    attrib_index  Attribute index within the service definition for the characteristic that was written.
     * @param    length        Length (in bytes) of the data in p_value.
     * @param    p_value       Pointer to the buffer containing the data written by the client.
     * @return   void
 */
void ota_service_write_post_callback(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                                     uint16_t attrib_index,
                                     uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("ota_service_write_post_callback: conn_handle %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_handle, service_id, attrib_index, length);
}

static T_APP_RESULT ota_service_attr_write_cb(uint16_t conn_handle, uint16_t cid,
                                              T_SERVER_ID service_id,
                                              uint16_t attr_index, T_WRITE_TYPE write_type, uint16_t length,
                                              uint8_t *p_value, P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_OTA_CALLBACK_DATA callback_data;
    T_APP_RESULT  cause = APP_RESULT_SUCCESS;

    uint8_t conn_id = 0xFF;
    le_get_conn_id_by_handle(conn_handle, &conn_id);

    APP_PRINT_INFO2("ota_service_attr_write_cb: attr_index 0x%02x, length %d", attr_index, length);

//    app_reg_le_link_disc_cb(conn_id, ble_ota_service_le_disconnect_cb);

    if (BLE_SERVICE_CHAR_OTA_INDEX == attr_index)
    {
        /* Make sure written value size is valid. */
        if ((length != sizeof(uint8_t)) || (p_value == NULL))
        {
            cause  = APP_RESULT_INVALID_VALUE_SIZE;
        }
        else
        {
            /* Notify Application. */
            callback_data.service_id = service_id;
            callback_data.msg_data.write.opcode = OTA_WRITE_CHAR_VAL;
            callback_data.conn_id = conn_id;
            callback_data.msg_data.write.value = p_value[0];

            if (p_fn_ota_service_cb)
            {
                p_fn_ota_service_cb(GATT_MSG_OTA_SERVER_WRITE, (void *)&callback_data);
            }
        }
    }
    else
    {
        APP_PRINT_ERROR0("ota_service_attr_write_cb: unknown attr_index");
        cause = APP_RESULT_ATTR_NOT_FOUND;
    }
    return cause;

}

/**
     * @brief    Callback function to handle read requests for OTA service characteristics.
     * @param    conn_handle  Connection handle identifying the BLE link.
     * @param    cid          Characteristic ID (local identifier) for the attribute being read.
     * @param    service_id   ID of the service containing the characteristic.
     * @param    attr_index   Attribute index within the service definition.
     * @param    offset       Offset for long/Blob Read operations.
     * @param    p_length     Pointer to variable for returning the data length.
     * @param    pp_value     Pointer to buffer pointer to return the address of the data to be read.
     * @return   T_APP_RESULT
     * @retval   Result of the profile read procedure.
 */
static T_APP_RESULT ota_service_attr_read_cb(uint16_t conn_handle, uint16_t cid,
                                             T_SERVER_ID service_id,
                                             uint16_t attr_index,
                                             uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT  cause = APP_RESULT_SUCCESS;

    uint8_t conn_id = 0xFF;
    le_get_conn_id_by_handle(conn_handle, &conn_id);

    APP_PRINT_INFO1("ota_service_attr_read_cb: attr_index 0x%02x", attr_index);

    switch (attr_index)
    {
    default:
        APP_PRINT_ERROR0("ota_service_attr_read_cb: unknown attr_index");
        cause  = APP_RESULT_ATTR_NOT_FOUND;
        break;
    case BLE_SERVICE_CHAR_MAC_ADDRESS_INDEX:
        {
            for (int i = 0; i < 6; i++)
            {
                mac_addr[i] = app_db.factory_addr[5 - i];
            }

            for (int i = 0; i < 6; i++)
            {
                mac_addr[i + 6] = app_cfg_nv.bud_local_addr[5 - i];
            }

            *pp_value  = (uint8_t *)mac_addr;
            *p_length = sizeof(mac_addr);
        }
        break;
    case BLE_SERVICE_CHAR_DEVICE_INFO_INDEX:
        {
            ota_info.ota_mode = BLE_OTA_MODE;
            ota_info.conn_id = conn_id;
            dfu_get_device_info(ota_info, &device_info);
            *pp_value  = (uint8_t *)&device_info;
            *p_length = sizeof(device_info);
        }
        break;
    case BLE_SERVICE_CHAR_IMAGE_VERSION_FIRST_INDEX:
        {
            img_ver[0] = ACTIVE_BANK;
            dfu_get_img_version(&img_ver[1], img_ver[0]);
            *pp_value  = img_ver;
            *p_length = img_ver[1] * 10 + 2;
            DFU_PRINT_INFO1("ota_service_attr_read_cb: version length=%d", *p_length);
        }
        break;
    case BLE_SERVICE_CHAR_IMAGE_VERSION_SECOND_INDEX:
        {
            *pp_value  = NULL;
            *p_length = 0;
        }
        break;
    case BLE_SERVICE_CHAR_PROTOCOL_INFO_INDEX:
        {
            *pp_value  = (uint8_t *)&protocol_info;
            *p_length = sizeof(protocol_info);
        }
        break;
    case BLE_SERVICE_CHAR_SECTION_SIZE_FIRST_INDEX:
        {
            dfu_get_section_size(section_size);
            *pp_value  = section_size;
            *p_length = section_size[0] * 6 + 1;
        }
        break;
    case BLE_SERVICE_CHAR_SECTION_SIZE_SECOND_INDEX:
        {
            *pp_value  = NULL;
            *p_length = 0;
        }
        break;
        /* TODO: add other version later */
    }

    APP_PRINT_INFO1("ota_service_attr_read_cb: %b", TRACE_BINARY(*p_length, *pp_value));

    return (cause);
}


/** @brief  OTA BLE Service Callbacks */
static const T_FUN_GATT_EXT_SERVICE_CBS  ota_service_cbs =
{
    ota_service_attr_read_cb,   /**< Read callback function pointer */
    ota_service_attr_write_cb,  /**< Write callback function pointer */
    NULL                        /**< CCCD update callback function pointer */
};
/**
    * @brief    Add OTA BLE service to application
    * @param    app_cb  Callback when service attribute was read, write or cccd update.
    * @return   Service ID auto generated by profile layer
    * @retval   A T_SERVER_ID type value
    */
T_SERVER_ID ota_reg_srv(P_FUN_OTA_SERVER_APP_CB app_cb)
{
    T_SERVER_ID service_id;
    if (false == gatt_svc_add(&service_id,
                              (uint8_t *)ota_service_table,
                              sizeof(ota_service_table),
                              &ota_service_cbs, NULL))
    {
        service_id = 0xff;
        APP_PRINT_ERROR1("ota_reg_srv: service_id %d", service_id);
    }
    p_fn_ota_service_cb = app_cb;
    return service_id;
}
