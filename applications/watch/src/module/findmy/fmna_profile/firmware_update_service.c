/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_FINDMY_USE_UARP
#include "firmware_update_service.h"
#include "findmy_network_service.h"
#include "trace.h"
#include <string.h>
#include "gap.h"
#include "fmna_gatt.h"
#include "fmna_constants.h"

static P_FUN_FWUS_SERVER_APP_CB pfn_fwus_data_cb = NULL;

const T_ATTRIB_APPL fwus_attr_tbl[] =
{
    /*--------------------------FMN Service ---------------------------*/
    /* <<Primary Service>>, .. 0*/
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE),  /* wFlags     */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),     /* bTypeValue */
            LO_WORD(UARP_UUID_SERVICE),           /* service UUID */
            HI_WORD(UARP_UUID_SERVICE)
        },
        UUID_16BIT_SIZE,                            /* bValueLen     */
        NULL,                                       /* pValueContext */
        GATT_PERM_READ                              /* wPermissions  */
    },

    /* <<Characteristic1>>, .. 1, Data Control Point*/              //------------------------Data Control Point  1
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_WRITE |                 /* characteristic properties */
             GATT_CHAR_PROP_INDICATE)
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  FWUS characteristic value 2*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,           /* flags */
        {                                            /* type_value */
            GATT_UUID128_DATA_CTRL_POINT
        },
        0,                                           /* bValueLen */
        (void *)NULL,
        GATT_PERM_WRITE                              /* wPermissions */
    },
    {
        (ATTRIB_FLAG_VALUE_INCL |                   /* wFlags */
         ATTRIB_FLAG_CCCD_APPL),
        {                                           /* bTypeValue */
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            /* NOTE: this value has an instantiation for each client, a write to */
            /* this attribute does not modify this default value:                */
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT), /* client char. config. bit field */
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,                                          /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)          /* wPermissions */
    },
};

void fwus_write_post_callback(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                              uint16_t attrib_index,
                              uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("fwus_write_post_callback: conn_handle %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_handle, service_id, attrib_index, length);
}

T_APP_RESULT fwus_attr_write_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                                uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                                P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;
    APP_PRINT_INFO1("fwus_attr_write_cb: write_type 0x%x", write_type);
    *p_write_ind_post_proc = fwus_write_post_callback;
    if (p_value == NULL)
    {
        cause  = APP_RESULT_INVALID_VALUE_SIZE;
    }
    else
    {
        if (pfn_fwus_data_cb == NULL)
        {
            APP_PRINT_ERROR0("fwus_attr_write_cb: service data handler missing");
        }
        else
        {
            T_FWUS_CALLBACK_DATA fwus_cb_data;
            uint8_t conn_id;
            le_get_conn_id_by_handle(conn_handle, &conn_id);
            fwus_cb_data.conn_id = conn_id;
            fwus_cb_data.conn_handle = conn_handle;
            fwus_cb_data.cid = cid;
            fwus_cb_data.msg_data.write.len = length;
            fwus_cb_data.msg_data.write.opcode = attrib_index;
            fwus_cb_data.msg_data.write.write_type = write_type;
            fwus_cb_data.msg_data.write.p_value = p_value;
            cause = pfn_fwus_data_cb(GATT_MSG_FWUS_SERVER_WRTIE, &fwus_cb_data);
        }
    }
    return cause;
}

void fwus_cccd_update_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id, uint16_t index,
                         uint16_t cccbits)
{
    APP_PRINT_INFO2("fwus_cccd_update_cb: index %d, cccbits 0x%x", index, cccbits);

    if (pfn_fwus_data_cb == NULL)
    {
        APP_PRINT_ERROR0("fwus_cccd_update_cb: service data handler missing");
    }
    else
    {
        T_FWUS_CALLBACK_DATA fwus_cb_data;
        uint8_t conn_id;
        le_get_conn_id_by_handle(conn_handle, &conn_id);
        fwus_cb_data.conn_id = conn_id;
        fns_cb_data.conn_handle = conn_handle;
        fns_cb_data.cid = cid;
        fwus_cb_data.msg_data.nofity_indicate_update.notification_indification_index = index;
        fwus_cb_data.msg_data.nofity_indicate_update.cccbits = cccbits;
        pfn_fwus_data_cb(GATT_MSG_FWUS_SERVER_CCCD, &fwus_cb_data);
    }
}

const T_FUN_GATT_EXT_SERVICE_CBS fwus_ble_cbs =
{
    NULL,               // Read callback function pointer
    fwus_attr_write_cb, // Write callback function pointer
    fwus_cccd_update_cb // CCCD update callback function pointer
};

T_SERVER_ID fwus_reg_srv(P_FUN_FWUS_SERVER_APP_CB app_cb, P_FUN_GATT_EXT_SEND_DATA_CB send_cb)
{
    T_SERVER_ID service_id;

    if (false == gatt_svc_add(&service_id,
                              (uint8_t *)fwus_attr_tbl,
                              sizeof(fwus_attr_tbl),
                              &fwus_ble_cbs, send_cb))
    {
        APP_PRINT_ERROR0("fwus_reg_srv: fail");
        service_id = 0xff;
        return service_id;
    }

    pfn_fwus_data_cb = app_cb;
    return service_id;
}
#endif
