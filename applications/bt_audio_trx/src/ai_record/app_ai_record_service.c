/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */



#if CONFIG_REALTEK_APP_AI_RECORD

#include <gatt.h>
#include <bt_types.h>
#include "trace.h"
#include "app_main.h"
#include "app_cfg.h"
#include "gap_conn_le.h"
#include "gap.h"
#include "gap_chann.h"
#include "bt_gatt_svc.h"
#include "app_ai_record_service.h"
#include "app_flags.h"

#if CONFIG_RECORD_TRANS_GATT_OVER_BREDR
#include "bt_sdp.h"
#endif

/*============================================================================*
 *                              Constants
 *============================================================================*/

#define INVALID_CONN_ID                 0xFF

#define LOG_CHANN_UNKNOWN               0
#define LOG_CHANN_LE                    1
#define LOG_CHANN_BREDR                 2

/** @brief  Record-Trans service UUID (128-bit, on-air little-endian). */
static const uint8_t GATT_UUID_RECORD_TRANS_SERVICE[16] =
{
    GATT_UUID128_RECORD_TRANS_SERVICE_ADV
};

/*============================================================================*
 *                              GATT Service Table
 *============================================================================*/

static const T_ATTRIB_APPL gatt_extended_service_table[] =
{
    /* Primary Service, index 0 */
    {
#if CONFIG_RECORD_TRANS_GATT_OVER_BREDR
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE | ATTRIB_FLAG_BREDR),
#else
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),
#endif
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
        },
        UUID_128BIT_SIZE,
        (void *)GATT_UUID_RECORD_TRANS_SERVICE,
        GATT_PERM_READ
    },

    /* Characteristic 1 declaration: Write / Write Without Response, index 1 */
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_WRITE | GATT_CHAR_PROP_WRITE_NO_RSP,
        },
        1,
        NULL,
        GATT_PERM_READ
    },

    /* Characteristic 1 value, index 2 */
    {
        ATTRIB_FLAG_VALUE_APPL,
        {
            LO_WORD(GATT_UUID_CHAR_RECORD_TRANS_WRITE),
            HI_WORD(GATT_UUID_CHAR_RECORD_TRANS_WRITE),
        },
        0,
        (void *)NULL,
        GATT_PERM_WRITE
    },

    /* Characteristic 2 declaration: Notify, index 3 */
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            (GATT_CHAR_PROP_NOTIFY),
        },
        1,
        NULL,
        GATT_PERM_READ
    },

    /* Characteristic 2 value, index 4 */
    {
        ATTRIB_FLAG_VALUE_APPL,
        {
            LO_WORD(GATT_UUID_CHAR_RECORD_TRANS_NOTIFY),
            HI_WORD(GATT_UUID_CHAR_RECORD_TRANS_NOTIFY),
        },
        0,
        (void *)NULL,
        GATT_PERM_NOTIF_IND
    },

    /* Client Characteristic Configuration Descriptor (CCCD), index 5 */
    {
        ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL,
        {
            LO_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            HI_WORD(GATT_UUID_CHAR_CLIENT_CONFIG),
            LO_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT),
            HI_WORD(GATT_CLIENT_CHAR_CONFIG_DEFAULT)
        },
        2,
        NULL,
        (GATT_PERM_READ | GATT_PERM_WRITE)
    }
};

/*============================================================================*
 *                              SDP Record (BR/EDR only)
 *============================================================================*/

#if CONFIG_RECORD_TRANS_GATT_OVER_BREDR

/* The GATT service handle range below is hard-coded.
 * The values MUST match the actual handle range assigned by gatt_svc_add().
 * Update manually if the service table or registration order changes. */
static uint8_t record_trans_sdp_record[] =
{
    /* Outer sequence header. Length: 0x4F for 128-bit UUID. */
    SDP_DATA_ELEM_SEQ_HDR,
    0x4F,

    /* Attribute: ServiceClassIDList (0x0001) */
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SRV_CLASS_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_SRV_CLASS_ID_LIST,

    SDP_DATA_ELEM_SEQ_HDR,
    0x11,
    SDP_UUID128_HDR,
    /* Custom UUID: 4E5A7C2F-9B3E-4F8A-A6D1-3C7E8F9A2B5D
     * SDP transports UUID big-endian (MSB first on wire). */
    0x4E, 0x5A, 0x7C, 0x2F, 0x9B, 0x3E, 0x4F, 0x8A,
    0xA6, 0xD1, 0x3C, 0x7E, 0x8F, 0x9A, 0x2B, 0x5D,

    /* Attribute: ProtocolDescriptorList (0x0004) */
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROTO_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROTO_DESC_LIST,

    SDP_DATA_ELEM_SEQ_HDR,
    0x13,

    /* Layer 1: L2CAP, PSM = ATT */
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_L2CAP >> 8),
    (uint8_t)(UUID_L2CAP),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(PSM_ATT >> 8),
    (uint8_t)(PSM_ATT),

    /* Layer 2: ATT, with GATT service handle range
     *          Update both fields if registration order changes. */
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_ATT >> 8),
    (uint8_t)(UUID_ATT),
    /* Start handle (placeholder) */
    SDP_UNSIGNED_TWO_BYTE,
    0x00, 0x53,
    /* End handle (placeholder) */
    SDP_UNSIGNED_TWO_BYTE,
    0x00, 0x58,

    /* Attribute: BrowseGroupList (0x0005) */
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_BROWSE_GROUP_LIST >> 8),
    (uint8_t)SDP_ATTR_BROWSE_GROUP_LIST,

    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP >> 8),
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP),
};

#endif /* CONFIG_RECORD_TRANS_GATT_OVER_BREDR */

/*============================================================================*
 *                              Variables
 *============================================================================*/

T_SERVER_ID rt_srv_id_local = 0xFF;

static P_FUN_RECORD_TRANS_SERVER_APP_CB p_fn_record_trans_service_cb = NULL;

/*============================================================================*
 *                              Internal Helpers
 *============================================================================*/

/**
 * @brief  Resolve transport info from (conn_handle, cid).
 *
 *         On success, *chann_type is the resolved T_GAP_CHANN_TYPE,
 *         and *conn_id is filled when transport is LE (INVALID_CONN_ID
 *         on BR/EDR).
 *
 * @retval true   resolved & allowed.
 * @retval false  resolve failed OR transport not allowed by config.
 */
static bool record_trans_resolve_link(uint16_t conn_handle, uint16_t cid,
                                      uint16_t *chann_type, uint8_t *conn_id)
{
    T_GAP_CHANN_INFO info;

    *conn_id = INVALID_CONN_ID;

    if (!gap_chann_get_info(conn_handle, cid, &info))
    {
        APP_PRINT_WARN2("[record_trans_resolve_link] get_info failed, "
                        "conn_handle=0x%04x cid=0x%04x", conn_handle, cid);
        return false;
    }

    switch (info.chann_type)
    {
    case GAP_CHANN_TYPE_LE_ATT:
    case GAP_CHANN_TYPE_LE_ECFC:
        le_get_conn_id_by_handle(conn_handle, conn_id);
        break;

    case GAP_CHANN_TYPE_BREDR_ATT:
    case GAP_CHANN_TYPE_BREDR_ECFC:
#if !CONFIG_RECORD_TRANS_GATT_OVER_BREDR
        APP_PRINT_WARN3("[record_trans_resolve_link] BR/EDR not allowed "
                        "(CONFIG_RECORD_TRANS_GATT_OVER_BREDR=0), "
                        "conn_handle=0x%04x cid=0x%04x chann_type=%d",
                        conn_handle, cid, info.chann_type);
        return false;
#else
        break;
#endif

    default:
        APP_PRINT_WARN1("[record_trans_resolve_link] unknown chann_type=%d",
                        info.chann_type);
        return false;
    }

    *chann_type = info.chann_type;

    APP_PRINT_INFO3("[record_trans_resolve_link] conn_handle=0x%04x cid=0x%04x "
                    "chann_type=%d", conn_handle, cid, *chann_type);
    return true;
}

/** @brief  Map channel type to a short identifier for logging. */
static inline uint8_t record_trans_log_chann(uint16_t chann_type)
{
    if (chann_type == GAP_CHANN_TYPE_LE_ATT ||
        chann_type == GAP_CHANN_TYPE_LE_ECFC)
    {
        return LOG_CHANN_LE;
    }

    if (chann_type == GAP_CHANN_TYPE_BREDR_ATT ||
        chann_type == GAP_CHANN_TYPE_BREDR_ECFC)
    {
        return LOG_CHANN_BREDR;
    }

    return LOG_CHANN_UNKNOWN;
}

/*============================================================================*
 *                              GATT Callbacks
 *============================================================================*/

/**
 * @brief    Write characteristic data callback from GATT layer.
 *
 *           Phone to Watch control commands. The service layer does NOT
 *           parse payload format; it wraps (link info + raw pointer) in
 *           T_RECORD_TRANS_CALLBACK_DATA and dispatches to the registered
 *           application callback. The app then routes by cmd_id (or by
 *           any other policy) to the appropriate business handler.
 */
static T_APP_RESULT record_trans_service_attr_write_cb(uint16_t conn_handle, uint16_t cid,
                                                       T_SERVER_ID service_id,
                                                       uint16_t attr_index, T_WRITE_TYPE write_type,
                                                       uint16_t length, uint8_t *p_value,
                                                       P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    uint8_t  conn_id;
    uint16_t chann_type;
    T_RECORD_TRANS_CALLBACK_DATA callback_data;

    if (!record_trans_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR2("record_trans_service_attr_write_cb: resolve failed, "
                         "conn_handle=0x%04x cid=0x%04x", conn_handle, cid);
        return APP_RESULT_APP_ERR;
    }

    APP_PRINT_INFO4("record_trans_service_attr_write_cb: chann_type=%d, write_type=%d, "
                    "attr_idx=0x%02x, len=%d",
                    record_trans_log_chann(chann_type), write_type, attr_index, length);

    if (attr_index != BLE_SERVICE_CHAR_RECORD_TRANS_WRITE_INDEX)
    {
        APP_PRINT_ERROR1("record_trans_service_attr_write_cb: unknown attr_index 0x%x",
                         attr_index);
        return APP_RESULT_ATTR_NOT_FOUND;
    }

    if (length < 2 || p_value == NULL)
    {
        APP_PRINT_ERROR1("record_trans_service_attr_write_cb: invalid payload, len=%d",
                         length);
        return APP_RESULT_INVALID_VALUE_SIZE;
    }

    /* Build callback payload. p_value lifetime is the cb context only;
     * the app must copy if it wants to defer (see header doc). */
    callback_data.conn_id              = conn_id;
    callback_data.conn_handle          = conn_handle;
    callback_data.cid                  = cid;
    callback_data.chann_type           = chann_type;
    callback_data.msg_data.write.p_value = p_value;
    callback_data.msg_data.write.length  = length;

    if (p_fn_record_trans_service_cb == NULL)
    {
        APP_PRINT_WARN0("record_trans_service_attr_write_cb: no app_cb registered, drop");
        return APP_RESULT_APP_ERR;
    }

    return p_fn_record_trans_service_cb(GATT_MSG_RECORD_TRANS_SERVER_WRITE,
                                        (void *)&callback_data);
}

/**
 * @brief    CCCD update callback from GATT layer.
 */
static void record_trans_cccd_update_cb(uint16_t conn_handle, uint16_t cid,
                                        T_SERVER_ID service_id,
                                        uint16_t attrib_idx, uint16_t cccd_bits)
{
    uint8_t  conn_id;
    uint16_t chann_type;
    T_RECORD_TRANS_CALLBACK_DATA callback_data;

    if (!record_trans_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR2("record_trans_cccd_update_cb: resolve failed, "
                         "conn_handle=0x%04x cid=0x%04x", conn_handle, cid);
        return;
    }

    APP_PRINT_INFO3("record_trans_cccd_update_cb: chann_type=%d, attrib_idx=%d, cccd_bits=0x%04x",
                    record_trans_log_chann(chann_type), attrib_idx, cccd_bits);

    if (attrib_idx != BLE_SERVICE_CHAR_RECORD_TRANS_NOTIFY_CCCD_INDEX)
    {
        return;
    }

    if (cccd_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
    {
        APP_PRINT_INFO0("record_trans_cccd_update_cb: NOTIFY ENABLED");
    }
    else
    {
        APP_PRINT_INFO0("record_trans_cccd_update_cb: NOTIFY DISABLED");
    }

    callback_data.conn_id                       = conn_id;
    callback_data.conn_handle                   = conn_handle;
    callback_data.cid                           = cid;
    callback_data.chann_type                    = chann_type;
    callback_data.msg_data.notification_index = BLE_SERVICE_CHAR_RECORD_TRANS_NOTIFY_INDEX;

    if (p_fn_record_trans_service_cb)
    {
        p_fn_record_trans_service_cb(GATT_MSG_RECORD_TRANS_SERVER_CCCD_UPDATE,
                                     (void *)&callback_data);
    }
}

/** @brief  Record-Trans Service Callback table */
static const T_FUN_GATT_EXT_SERVICE_CBS record_trans_service_cbs =
{
    NULL,                                  /**< Read callback (unused) */
    record_trans_service_attr_write_cb,    /**< Write callback */
    record_trans_cccd_update_cb            /**< CCCD update callback */
};

/*============================================================================*
 *                              SDP Helper (BR/EDR only)
 *============================================================================*/

#if CONFIG_RECORD_TRANS_GATT_OVER_BREDR

void record_trans_sdp_register(void)
{
    if (bt_sdp_record_add((void *)record_trans_sdp_record))
    {
        APP_PRINT_INFO0("record_trans_sdp_register: ok");
    }
    else
    {
        APP_PRINT_ERROR0("record_trans_sdp_register: bt_sdp_record_add failed");
    }
}

#endif /* CONFIG_RECORD_TRANS_GATT_OVER_BREDR */

/*============================================================================*
 *                              Public Functions
 *============================================================================*/

bool record_trans_service_send_notification(uint16_t conn_handle, uint16_t cid,
                                            uint8_t *p_data, uint16_t data_len)
{
    uint8_t  conn_id;
    uint16_t chann_type;

    if (!record_trans_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR3("record_trans_service_send_notification: resolve failed, "
                         "drop notify. conn_handle=0x%04x cid=0x%04x len=%d",
                         conn_handle, cid, data_len);
        return false;
    }

    APP_PRINT_INFO3("record_trans_service_send_notification: chann_type=%d, "
                    "conn_handle=0x%04x, len=%d",
                    record_trans_log_chann(chann_type), conn_handle, data_len);

    return gatt_svc_send_data(conn_handle, cid, rt_srv_id_local,
                              BLE_SERVICE_CHAR_RECORD_TRANS_NOTIFY_INDEX,
                              p_data, data_len,
                              GATT_PDU_TYPE_NOTIFICATION);
}

T_SERVER_ID record_trans_reg_srv(P_FUN_RECORD_TRANS_SERVER_APP_CB app_cb)
{
    T_SERVER_ID service_id;

    if (!gatt_svc_add(&service_id,
                      (uint8_t *)gatt_extended_service_table,
                      sizeof(gatt_extended_service_table),
                      &record_trans_service_cbs, NULL))
    {
        APP_PRINT_ERROR0("record_trans_reg_srv: gatt_svc_add failed");
        return 0xFF;
    }

    p_fn_record_trans_service_cb = app_cb;
    rt_srv_id_local              = service_id;

    APP_PRINT_INFO1("record_trans_reg_srv: success, service_id=0x%x", service_id);
    return service_id;
}

#endif /* CONFIG_REALTEK_APP_AI_RECORD */
