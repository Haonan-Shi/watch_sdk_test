/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include <string.h>
#include "findmy_network_service.h"
#include "trace.h"
#include "gap.h"
#include "fmna_gatt.h"
#include "gap_conn_le.h"
#include "bt_gatt_svc.h"

T_SERVER_ID fns_id;
static P_FUN_EXT_SERVER_GENERAL_CB pfn_fns_data_cb = NULL;

const T_ATTRIB_APPL fns_attr_tbl[] =
{
    /*--------------------------FMN Service ---------------------------*/
    /* <<Primary Service>>, .. 0*/
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE),  /* wFlags     */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),     /* bTypeValue */
            LO_WORD(FINDMY_UUID_SERVICE),           /* service UUID */
            HI_WORD(FINDMY_UUID_SERVICE)
        },
        UUID_16BIT_SIZE,                            /* bValueLen     */
        NULL,                                       /* pValueContext */
        GATT_PERM_READ                              /* wPermissions  */
    },

    /* <<Characteristic1>>, .. 1, Pairing Control Point*/              //------------------------Pairing Control Point  1
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
    /*  FMN characteristic value 2*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,           /* flags */
        {                                            /* type_value */
            GATT_UUID128_PAIR_CTRL_POINT
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
    /* <<Characteristic2>>, .. 3, Configuration Control Point*/           //------------------------Configuration Control Point  2
    {
        ATTRIB_FLAG_VALUE_INCL, //ATTRIB_FLAG_VALUE_APPL                    /* wFlags */
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
    /*  FMN characteristic value 4*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,           /* flags */
        {                                            /* type_value */
            GATT_UUID128_CONF_CTRL_POINT
        },
        0,                                           /* bValueLen */
        (void *)NULL,
        GATT_PERM_WRITE                             /* wPermissions */
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
    /* <<Characteristic3>>, .. 5, Non Owner Control Point*/                //------------------------Non Owner Control Point  3
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
    /*  FMN characteristic value 6*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,           /* flags */
        {                                            /* type_value */
            GATT_UUID128_NON_OWNER_CTRL_POINT
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
    /* <<Characteristic4>>, .. 7, Paired owner Information Control Point*/   //------------------------Paired owner Information Control Point  4
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
    /*  FMN characteristic value 8*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,           /* flags */
        {                                            /* type_value */
            GATT_UUID128_PAIRED_OWNER_INFO_CTRL_POINT
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
#ifdef DEBUG
    /* <<Characteristic5>>, .. 9, Debug Control Point*/                        //------------------------Debug Control Point  5
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
    /*  FMN characteristic value 0x0a*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,           /* flags */
        {                                            /* type_value */
            GATT_UUID128_DEBUG_CTRL_POINT
        },
        0,                                           /* bValueLen */
        (void *)NULL,
        GATT_PERM_WRITE                              /* wPermissions */
    },
    /* client characteristic configuration  Index 0x0b */
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
    }
#endif //DEBUG
};

void fns_write_post_callback(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                             uint16_t attrib_index,
                             uint16_t length, uint8_t *p_value)
{
    APP_PRINT_INFO4("fns_write_post_callback: conn_handle %d, service_id %d, attrib_index 0x%x, length %d",
                    conn_handle, service_id, attrib_index, length);
}

T_APP_RESULT fns_attr_write_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                               uint16_t attrib_index, T_WRITE_TYPE write_type, uint16_t length, uint8_t *p_value,
                               P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;
    APP_PRINT_INFO1("fns_attr_write_cb write_type = 0x%x", write_type);
    *p_write_ind_post_proc = fns_write_post_callback;
    if (p_value == NULL)
    {
        cause  = APP_RESULT_INVALID_VALUE_SIZE;
    }
    else
    {
        if (pfn_fns_data_cb == NULL)
        {
            APP_PRINT_ERROR0("fns_attr_write_cb: service data handler missing.");
        }
        else
        {
            T_FNS_CALLBACK_DATA fns_cb_data;
            uint8_t conn_id;
            le_get_conn_id_by_handle(conn_handle, &conn_id);
            fns_cb_data.conn_id = conn_id;
            fns_cb_data.conn_handle = conn_handle;
            fns_cb_data.cid = cid;
            fns_cb_data.msg_type = SERVICE_CALLBACK_TYPE_WRITE_CHAR_VALUE;
            fns_cb_data.msg_data.write.len = length;
            fns_cb_data.msg_data.write.opcode = attrib_index;
            fns_cb_data.msg_data.write.write_type = write_type;
            fns_cb_data.msg_data.write.p_value = p_value;
            cause = pfn_fns_data_cb(fns_id, &fns_cb_data);
        }
    }
    return cause;
}

void fns_cccd_update_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id, uint16_t index,
                        uint16_t cccbits)
{
    APP_PRINT_INFO2("fns_cccd_update_cb: index = %d, cccbits 0x%x", index, cccbits);
    if (pfn_fns_data_cb == NULL)
    {
        APP_PRINT_ERROR0("fns_cccd_update_cb: service data handler missing.");
    }
    else
    {
        T_FNS_CALLBACK_DATA fns_cb_data;
        uint8_t conn_id;
        le_get_conn_id_by_handle(conn_handle, &conn_id);
        fns_cb_data.conn_id = conn_id;
        fns_cb_data.conn_handle = conn_handle;
        fns_cb_data.cid = cid;
        fns_cb_data.msg_type = SERVICE_CALLBACK_TYPE_INDIFICATION_NOTIFICATION;
        fns_cb_data.msg_data.nofity_indicate_update.notification_indification_index = index;
        fns_cb_data.msg_data.nofity_indicate_update.cccbits = cccbits;
        pfn_fns_data_cb(fns_id, &fns_cb_data);
    }
}

const T_FUN_GATT_EXT_SERVICE_CBS fns_ble_cbs =
{
    NULL,  // Read callback function pointer
    fns_attr_write_cb, // Write callback function pointer
    fns_cccd_update_cb // CCCD update callback function pointer
};

T_SERVER_ID findmy_network_add_service(void *p_func)
{
    if (false == gatt_svc_add(&fns_id,
                              (uint8_t *)fns_attr_tbl,
                              sizeof(fns_attr_tbl),
                              &fns_ble_cbs, NULL))
    {
        APP_PRINT_ERROR0("findmy_network_add_service: fail");
        fns_id = 0xff;
        return fns_id;
    }

    pfn_fns_data_cb = (P_FUN_EXT_SERVER_GENERAL_CB)p_func;
    return fns_id;
}
#endif
