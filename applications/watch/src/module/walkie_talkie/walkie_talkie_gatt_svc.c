/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "stdint.h"
#include "string.h"
#include "trace.h"
#include "walkie_talkie_gatt_svc.h"
#include "walkie_talkie_voice.h"
#include "walkie_talkie_app.h"
#include "walkie_talkie_adv.h"

/********************************************************************************************************
* local static variables defined here, only used in this source file.
********************************************************************************************************/
static T_SERVER_ID wts_srv_id_local;
const uint8_t gatt_uuid128_wts_service[16] = {GATT_UUID128_WALKIE_TALKIE_SERVICE};

/** @brief Index defines for Characteristic's value */
#define GATT_SVC_WTS_CONTROL_POINT_INDEX            2
#define GATT_SVC_WTS_CONTROL_POINT_CCCD_INDEX       3
#define GATT_SVC_WTS_WRITE_INDEX                    5
#define GATT_SVC_WTS_NOTIFY_INDEX                   7
#define GATT_SVC_WTS_NOTIFY_CCCD_INDEX              8
///@endcond

static P_FUN_WTS_SERVER_APP_CB pfn_wts_cb = NULL;

/**< @brief  profile/service definition.  */
const T_ATTRIB_APPL wts_attr_tbl[] =
{
    /*----------------- Walkie talkie Service -------------------*/
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),                /* flags */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),             /* type_value */
        },
        UUID_128BIT_SIZE,                                   /* bValueLen */
        (void *)gatt_uuid128_wts_service,                   /* p_value_context */
        GATT_PERM_READ                                      /* permissions  */
    },

    /* <<Characteristic>>, .. */
    {
        ATTRIB_FLAG_VALUE_INCL,                             /* flags */
        {   /* type_value */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE_NO_RSP |                  /* characteristic properties */
             GATT_CHAR_PROP_NOTIFY)
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                                  /* bValueLen */
        NULL,
        GATT_PERM_READ                                      /* permissions */
    },
    /*--- WTS Control Point value ---*/
    {
        (ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT),   /* flags */
        {   /* type_value */
            GATT_UUID128_WALKIE_TALKIE_CONTROL_POINT
        },
        0,                                                  /* bValueLen */
        NULL,
        (GATT_PERM_WRITE | GATT_PERM_NOTIF_IND)             /* permissions */
    },
    /* client characteristic configuration */
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL),   /* flags */
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
    /*--- wts write characteristic value ---*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,   /* flags */
        {   /* type_value */
            GATT_UUID128_WALKIE_TALKIE_WRITE
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
            GATT_CHAR_PROP_NOTIFY,                          /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                                  /* bValueLen */
        NULL,
        GATT_PERM_READ                                      /* permissions */
    },
    /*--- wts notify characteristic value ---*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,   /* flags */
        {   /* type_value */
            GATT_UUID128_WALKIE_TALKIE_NOTIFY
        },
        0,                                                  /* bValueLen */
        NULL,
        GATT_PERM_NOTIF_IND                                 /* permissions */
    },
    /* client characteristic configuration */
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL),   /* flags */
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
    },
};
/**< @brief  walkie talkie service size definition.  */
const int wts_attr_tbl_size = sizeof(wts_attr_tbl);

/**
 * @brief       Send voice data .
 *
 * @param[in]   conn_handle    Connection handle of the ACL link.
 * @param[in]   cid            Local CID assigned by Bluetooth stack.
 * @param[in]   p_data         voice data.
 * @param[in]   len            data length.
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 */
bool wts_send_voice_data_notify(uint16_t conn_handle, uint16_t cid, uint8_t *p_data, uint16_t len)
{
    return gatt_svc_send_data(conn_handle, cid, wts_srv_id_local,
                              GATT_SVC_WTS_NOTIFY_INDEX,
                              p_data, len, GATT_PDU_TYPE_NOTIFICATION);
}

/**
 * @brief       Send control point data .
 *
 * @param[in]   conn_handle    Connection handle of the ACL link.
 * @param[in]   cid            Local CID assigned by Bluetooth stack.
 * @param[in]   p_data         control point data.
 * @param[in]   len            data length.
 * @return Operation result.
 * @retval true Operation success.
 * @retval false Operation failure.
 */
bool wts_send_control_point_notify(uint16_t conn_handle, uint16_t cid, uint8_t *p_data,
                                   uint16_t len)
{
    return gatt_svc_send_data(conn_handle, cid, wts_srv_id_local,
                              GATT_SVC_WTS_CONTROL_POINT_INDEX,
                              p_data, len, GATT_PDU_TYPE_NOTIFICATION);
}

/**
 * @brief write characteristic data from service.
 *
 * @param[in] conn_handle       Connection handle.
 * @param[in] cid               Local CID assigned by Bluetooth stack.
 * @param[in] service_id        ServiceID to be written.
 * @param[in] attrib_index      Attribute index of characteristic.
 * @param[in] write_type        Write type.
 * @param[in] length            Length of writing characteristic data.
 * @param[in] p_value           Pointer to characteristic data.
 * @param[in] p_write_ind_post_proc      Function pointer after ias_attr_write_cb.
 * @return TProfileResult
*/
T_APP_RESULT wts_attr_write_cb(uint16_t conn_handle, uint16_t cid, uint8_t service_id,
                               uint16_t attrib_index, T_WRITE_TYPE write_type,
                               uint16_t length, uint8_t *p_value,
                               P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_APP_RESULT cause  = APP_RESULT_SUCCESS;

    if (p_value == NULL)
    {
        PROFILE_PRINT_ERROR2("wts_attr_write_cb: p_value %p length= 0x%x", p_value, length);
        cause = APP_RESULT_INVALID_PDU;
        return cause;
    }

    switch (attrib_index)
    {
    default:
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;

    case GATT_SVC_WTS_CONTROL_POINT_INDEX:
        {
            T_WTS_SERVER_WRITE_DATA write_ind;
            write_ind.len = length;
            write_ind.p_value = p_value;

            walkie_talkie_service_handle_cp_req(conn_handle, cid, length, p_value);

            if (pfn_wts_cb && (cause == APP_RESULT_SUCCESS))
            {
                cause = pfn_wts_cb(conn_handle, cid, GATT_MSG_WTS_SERVER_WRITE_CONTROL_POINT, (void *)&write_ind);
            }
        }
        break;

    case GATT_SVC_WTS_WRITE_INDEX:
        {
            T_WTS_SERVER_WRITE_DATA write_ind;
            write_ind.len = length;
            write_ind.p_value = p_value;
            APP_PRINT_INFO2("wts write p_value = 0x%x, length = %d", p_value, length);

            walkie_talkie_player_data_parser(p_value, length);

            if (pfn_wts_cb && (cause == APP_RESULT_SUCCESS))
            {
                cause = pfn_wts_cb(conn_handle, cid, GATT_MSG_WTS_SERVER_WRITE, (void *)&write_ind);
            }
        }
        break;

    }

    return cause;
}

/**
 * @brief update CCCD bits from stack.
 *
 * @param conn_handle       Connection handle of the ACL link.
 * @param cid               Local CID assigned by Bluetooth stack.
 * @param[in] service_id    Service ID.
 * @param[in] index         Attribute index of characteristic data.
 * @param[in] ccc_bits      CCCD bits from stack.
 * @return None
*/
void wts_cccd_update_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                        uint16_t index, uint16_t ccc_bits)
{
    bool bHandle = true;

    PROFILE_PRINT_INFO2("wts_cccd_update_cb: index 0x%x, ccc_bits 0x%x", index, ccc_bits);

    switch (index)
    {
    case GATT_SVC_WTS_CONTROL_POINT_CCCD_INDEX:
        {
            if (ccc_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                PROFILE_PRINT_INFO0("wts control point cccd enable");
            }
            else
            {
                PROFILE_PRINT_INFO0("wts control point cccd disable");
            }
        }
        break;

    case GATT_SVC_WTS_NOTIFY_CCCD_INDEX:
        {
            if (ccc_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                PROFILE_PRINT_INFO0("wts notify cccd enable");
            }
            else
            {
                PROFILE_PRINT_INFO0("wts notify cccd disable");
            }
        }
        break;

    default:
        {
            bHandle = false;
            PROFILE_PRINT_ERROR1("hrs_cccd_update_cb: index 0x%x not found", index);
        }
        break;
    }

    return;
}

/*********************************************************************
 * SERVICE CALLBACKS
 */
// walkie talkie service related Service Callbacks
const T_FUN_GATT_EXT_SERVICE_CBS wts_cbs =
{
    NULL,  // Read callback function pointer
    wts_attr_write_cb, // Write callback function pointer
    wts_cccd_update_cb  // CCCD update callback function pointer
};

/**
 * @brief Add walkie talkie service to the BLE stack database.
 *
 * @param[in]   app_cb  Callback when service attribute was read, write or cccd update.
 * @return Service id generated by the BLE stack: @ref T_SERVER_ID.
 * @retval 0xFF Operation failure.
 * @retval others Service id assigned by stack.
 */
T_SERVER_ID wts_reg_srv(P_FUN_WTS_SERVER_APP_CB app_cb)
{
    T_SERVER_ID service_id;
    if (false == gatt_svc_add(&service_id,
                              (uint8_t *)wts_attr_tbl,
                              wts_attr_tbl_size,
                              &wts_cbs, NULL))
    {
        PROFILE_PRINT_ERROR1("wts_reg_srv: service_id %d", service_id);
        service_id = 0xff;
    }
    wts_srv_id_local = service_id;
    pfn_wts_cb = app_cb;
    return service_id;
}

