/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "gap.h"
#include "gap_conn_le.h"
#include "bt_gatt_svc.h"
#include "le_throughput_service.h"

/********************************************************************************************************
* Local static variables
********************************************************************************************************/
static P_FUN_LE_THROUGHPUT_APP_CB pfn_le_throughput_app_cb = NULL;
static uint8_t le_throughput_srv_id = 0xFF;

/* 128-bit UUID for the LE Throughput Test Service (base: Realtek with UUID 0xA00D) */
static const uint8_t GATT_UUID128_LE_THROUGHPUT_SRV[16] =
{
    0x12, 0xA2, 0x4D, 0x2E, 0xFE, 0x14, 0x48, 0x8E,
    0x93, 0xD2, 0x17, 0x3C, 0x0D, 0xA0, 0x00, 0x00
};

/********************************************************************************************************
* Attribute table
********************************************************************************************************/
static const T_ATTRIB_APPL le_throughput_srv_tbl[] =
{
    /*------------------- LE Throughput Test Service (0xA00D) -------------------*/
    /* Index 0: <<Primary Service>> */
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE),        /* wFlags */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),      /* bTypeValue */
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
            LO_WORD(BLE_UUID_LE_THROUGHPUT_SERVICE),           /* service UUID */
            HI_WORD(BLE_UUID_LE_THROUGHPUT_SERVICE)
        },
        UUID_16BIT_SIZE,                            /* bValueLen */
        NULL,      /* pValueContext */
        GATT_PERM_READ                               /* wPermissions */
    },

    /* Index 1: <<Characteristic Declaration for CT_TX>> (Indication) */
    {
        ATTRIB_FLAG_VALUE_INCL,                      /* wFlags */
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_INDICATE,                  /* characteristic properties */
        },
        1,                                           /* bValueLen */
        NULL,
        GATT_PERM_READ                               /* wPermissions */
    },

    /* Index 2: CT_TX Characteristic Value (Indication, BTSOC -> Phone) */
    {
        ATTRIB_FLAG_VALUE_APPL,                      /* wFlags */
        {
            LO_WORD(BLE_UUID_CT_TX_CHAR),
            HI_WORD(BLE_UUID_CT_TX_CHAR),
        },
        0,                                           /* variable size */
        NULL,
        GATT_PERM_NOTIF_IND                          /* wPermissions: notification/indication */
    },

    /* Index 3: CT_TX Client Characteristic Configuration (CCCD for Indication) */
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL), /* wFlags */
        {
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT),  /* default */
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,                                           /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)           /* wPermissions */
    },

    /* Index 4: <<Characteristic Declaration for CT_RX>> (Write Request) */
    {
        ATTRIB_FLAG_VALUE_INCL,                      /* wFlags */
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE,                     /* characteristic properties: Write Request */
        },
        1,                                           /* bValueLen */
        NULL,
        GATT_PERM_READ                               /* wPermissions */
    },

    /* Index 5: CT_RX Characteristic Value (Write Request, Phone -> BTSOC) */
    {
        ATTRIB_FLAG_VALUE_APPL,                      /* wFlags */
        {
            LO_WORD(BLE_UUID_CT_RX_CHAR),
            HI_WORD(BLE_UUID_CT_RX_CHAR),
        },
        0,                                           /* variable size */
        NULL,
        GATT_PERM_WRITE                              /* wPermissions */
    },

    /* Index 6: <<Characteristic Declaration for DT_TX>> (Notification) */
    {
        ATTRIB_FLAG_VALUE_INCL,                      /* wFlags */
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_NOTIFY,                    /* characteristic properties: Notification */
        },
        1,                                           /* bValueLen */
        NULL,
        GATT_PERM_READ                               /* wPermissions */
    },

    /* Index 7: DT_TX Characteristic Value (Notification, BTSOC -> Phone) */
    {
        ATTRIB_FLAG_VALUE_APPL,                      /* wFlags */
        {
            LO_WORD(BLE_UUID_DT_TX_CHAR),
            HI_WORD(BLE_UUID_DT_TX_CHAR),
        },
        0,                                           /* variable size */
        NULL,
        GATT_PERM_NOTIF_IND                          /* wPermissions: notification/indication */
    },

    /* Index 8: DT_TX Client Characteristic Configuration (CCCD for Notification) */
    {
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL), /* wFlags */
        {
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT),  /* default */
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,                                           /* bValueLen */
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)           /* wPermissions */
    },

    /* Index 9: <<Characteristic Declaration for DT_RX>> (Write Command) */
    {
        ATTRIB_FLAG_VALUE_INCL,                      /* wFlags */
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE_NO_RSP,              /* characteristic properties: Write Without Response */
        },
        1,                                           /* bValueLen */
        NULL,
        GATT_PERM_READ                               /* wPermissions */
    },

    /* Index 10: DT_RX Characteristic Value (Write Command, Phone -> BTSOC) */
    {
        ATTRIB_FLAG_VALUE_APPL,                      /* wFlags */
        {
            LO_WORD(BLE_UUID_DT_RX_CHAR),
            HI_WORD(BLE_UUID_DT_RX_CHAR),
        },
        0,                                           /* variable size */
        NULL,
        GATT_PERM_WRITE                              /* wPermissions */
    },
};

static const uint16_t LE_THROUGHPUT_SRV_TABLE_SIZE = sizeof(le_throughput_srv_tbl);

/********************************************************************************************************
* Forward declarations
********************************************************************************************************/
static T_APP_RESULT le_throughput_attr_read_cb(uint16_t conn_handle, uint16_t cid,
                                               T_SERVER_ID service_id,
                                               uint16_t attrib_index, uint16_t offset,
                                               uint16_t *p_length, uint8_t **pp_value);
static T_APP_RESULT le_throughput_attr_write_cb(uint16_t conn_handle, uint16_t cid,
                                                T_SERVER_ID service_id,
                                                uint16_t attrib_index, T_WRITE_TYPE write_type,
                                                uint16_t length, uint8_t *p_value,
                                                P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc);
static void le_throughput_cccd_update_cb(uint16_t conn_handle, uint16_t cid,
                                         T_SERVER_ID service_id,
                                         uint16_t index, uint16_t cccbits);

/********************************************************************************************************
* Callback implementations
********************************************************************************************************/

/**
 * @brief Read callback - handle read requests from peer.
 */
static T_APP_RESULT le_throughput_attr_read_cb(uint16_t conn_handle, uint16_t cid,
                                               T_SERVER_ID service_id,
                                               uint16_t attrib_index, uint16_t offset,
                                               uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;

    APP_PRINT_INFO1("le_throughput_attr_read_cb: attrib_index %d", attrib_index);

    switch (attrib_index)
    {
    default:
        APP_PRINT_ERROR1("le_throughput_attr_read_cb: attr not found, index %d", attrib_index);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    }

    return cause;
}

/**
 * @brief Write callback - handle write requests/commands from peer.
 *
 * CT_RX (index 5): Write Request - control signaling from Phone.
 * DT_RX (index 10): Write Command - test data from Phone.
 */
static T_APP_RESULT le_throughput_attr_write_cb(uint16_t conn_handle, uint16_t cid,
                                                T_SERVER_ID service_id,
                                                uint16_t attrib_index, T_WRITE_TYPE write_type,
                                                uint16_t length, uint8_t *p_value,
                                                P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;
    T_LE_THROUGHPUT_CALLBACK_DATA callback_data;
    uint8_t conn_id = 0xFF;

    if (p_value == NULL)
    {
        APP_PRINT_ERROR0("le_throughput_attr_write_cb: p_value is NULL");
        return APP_RESULT_INVALID_VALUE_SIZE;
    }

    /* Get connection ID */
    le_get_conn_id_by_handle(conn_handle, &conn_id);

    callback_data.conn_id = conn_id;
    callback_data.service_id = service_id;
    callback_data.conn_handle = conn_handle;
    callback_data.cid = cid;
    callback_data.attr_index = attrib_index;
    *p_write_ind_post_proc = NULL;

    APP_PRINT_INFO3("le_throughput_attr_write_cb: attrib_index %d, length %d, write_type %d",
                    attrib_index, length, write_type);

    switch (attrib_index)
    {
    case LE_THROUGHPUT_CT_RX_VALUE_INDEX:
        {
            /* CT_RX: Control signaling from Phone (Write Request with response) */
            APP_PRINT_INFO2("le_throughput CT_RX write: opcode 0x%02x, len %d",
                            (length > 0) ? p_value[0] : 0, length);

            /* Notify application with the received data */
            callback_data.msg_data.rx_data.len = length;
            callback_data.msg_data.rx_data.p_value = p_value;
            callback_data.msg_data.rx_data.attr_index = attrib_index;

            if (pfn_le_throughput_app_cb)
            {
                pfn_le_throughput_app_cb(GATT_MSG_LE_THROUGHPUT_WRITE,
                                         (void *)&callback_data);
            }
        }
        break;

    case LE_THROUGHPUT_DT_RX_VALUE_INDEX:
        {
            /* DT_RX: Test data from Phone (Write Command, no response) */
            /* Notify application for counting purposes */
            callback_data.msg_data.rx_data.len = length;
            callback_data.msg_data.rx_data.p_value = p_value;
            callback_data.msg_data.rx_data.attr_index = attrib_index;

            if (pfn_le_throughput_app_cb)
            {
                pfn_le_throughput_app_cb(GATT_MSG_LE_THROUGHPUT_WRITE,
                                         (void *)&callback_data);
            }
        }
        break;

    default:
        APP_PRINT_ERROR1("le_throughput_attr_write_cb: attrib_index %d not found", attrib_index);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    }

    return cause;
}

/**
 * @brief CCCD update callback - handle CCCD changes from peer.
 */
static void le_throughput_cccd_update_cb(uint16_t conn_handle, uint16_t cid,
                                         T_SERVER_ID service_id,
                                         uint16_t index, uint16_t cccbits)
{
    uint8_t conn_id = 0xFF;
    T_LE_THROUGHPUT_CALLBACK_DATA callback_data;

    le_get_conn_id_by_handle(conn_handle, &conn_id);

    callback_data.conn_id = conn_id;
    callback_data.service_id = service_id;
    callback_data.conn_handle = conn_handle;
    callback_data.cid = cid;
    callback_data.attr_index = index;

    APP_PRINT_INFO2("le_throughput_cccd_update_cb: index %d, cccbits 0x%04x", index, cccbits);

    switch (index)
    {
    case LE_THROUGHPUT_CT_TX_CCCD_INDEX:
        {
            /* CT_TX CCCD update (Indication) */
            if (cccbits & GATT_CLIENT_CHAR_CONFIG_INDICATE)
            {
                callback_data.msg_data.notification_indification_value = 1;
            }
            else
            {
                callback_data.msg_data.notification_indification_value = 0;
            }

            if (pfn_le_throughput_app_cb)
            {
                pfn_le_throughput_app_cb(GATT_MSG_LE_THROUGHPUT_CT_TX_CCCD,
                                         (void *)&callback_data);
            }
        }
        break;

    case LE_THROUGHPUT_DT_TX_CCCD_INDEX:
        {
            /* DT_TX CCCD update (Notification) */
            if (cccbits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            {
                callback_data.msg_data.notification_indification_value = 1;
            }
            else
            {
                callback_data.msg_data.notification_indification_value = 0;
            }

            if (pfn_le_throughput_app_cb)
            {
                pfn_le_throughput_app_cb(GATT_MSG_LE_THROUGHPUT_DT_TX_CCCD,
                                         (void *)&callback_data);
            }
        }
        break;

    default:
        break;
    }
}

/********************************************************************************************************
* Service callbacks structure
********************************************************************************************************/
static const T_FUN_GATT_EXT_SERVICE_CBS le_throughput_srv_cbs =
{
    le_throughput_attr_read_cb,    /* read_attr_cb */
    le_throughput_attr_write_cb,   /* write_attr_cb */
    le_throughput_cccd_update_cb   /* cccd_update_cb */
};

/********************************************************************************************************
* Public API implementations
********************************************************************************************************/

/**
 * @brief Register the LE Throughput Test Service.
 */
uint8_t le_throughput_reg_srv(P_FUN_LE_THROUGHPUT_APP_CB app_cb,
                              P_FUN_GATT_EXT_SEND_DATA_CB send_cb)
{
    if (false == gatt_svc_add(&le_throughput_srv_id,
                              (uint8_t *)le_throughput_srv_tbl,
                              LE_THROUGHPUT_SRV_TABLE_SIZE,
                              &le_throughput_srv_cbs,
                              send_cb))
    {
        APP_PRINT_ERROR0("le_throughput_reg_srv: gatt_svc_add failed");
        le_throughput_srv_id = 0xFF;
        return le_throughput_srv_id;
    }

    pfn_le_throughput_app_cb = app_cb;
    APP_PRINT_INFO1("le_throughput_reg_srv: service_id %d", le_throughput_srv_id);
    return le_throughput_srv_id;
}

/**
 * @brief Send data via CT_TX (Indication).
 */
bool le_throughput_send_ct_tx_data(uint16_t conn_handle, uint16_t cid,
                                   uint8_t *p_value, uint16_t len)
{
    if (le_throughput_srv_id == 0xFF)
    {
        return false;
    }

    return gatt_svc_send_data(conn_handle, cid, le_throughput_srv_id,
                              LE_THROUGHPUT_CT_TX_VALUE_INDEX,
                              p_value, len, GATT_PDU_TYPE_INDICATION);
}

/**
 * @brief Send data via DT_TX (Notification).
 */
bool le_throughput_send_dt_tx_data(uint16_t conn_handle, uint16_t cid,
                                   uint8_t *p_value, uint16_t len)
{
    if (le_throughput_srv_id == 0xFF)
    {
        return false;
    }

    return gatt_svc_send_data(conn_handle, cid, le_throughput_srv_id,
                              LE_THROUGHPUT_DT_TX_VALUE_INDEX,
                              p_value, len, GATT_PDU_TYPE_NOTIFICATION);
}

/**
 * @brief Check if DT_TX notification is enabled.
 */
bool le_throughput_is_dt_tx_enabled(uint16_t conn_handle, uint16_t cid)
{
    /* Note: The CCCD state is managed internally by the stack.
     * This is a placeholder; actual CCCD state tracking can be added
     * in the application callback if needed.
     */
    (void)conn_handle;
    (void)cid;
    return true;
}

/**
 * @brief Check if CT_TX indication is enabled.
 */
bool le_throughput_is_ct_tx_enabled(uint16_t conn_handle, uint16_t cid)
{
    (void)conn_handle;
    (void)cid;
    return true;
}
