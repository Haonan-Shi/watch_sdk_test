/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#if CONFIG_AIRSYNC_SERVICE_SUPPORT

enum { __FILE_NUM__ = 0 };
#include "trace.h"
#include "string.h"
#include "app_task.h"
#include "airsync_ble_service.h"
#include "app_msg.h"
#include "app_main.h"
#include "gap_conn_le.h"
#include "bt_gatt_svc.h"
#include "module_global_data.h"
#include "gap_lib_common.h"
#include "gap_chann.h"
#include "gap.h"
#include "app_module_init.h"
#include "app_ble_service_info.h"

#if CONFIG_AIRSYNC_GATT_OVER_BREDR
#include "bt_sdp.h"
#endif

/*============================================================================*
 *                              Macros
 *============================================================================*/

#ifndef INVALID_CONN_ID
#define INVALID_CONN_ID                 0xFF
#endif

/*============================================================================*
 *                              Local Variables
 *============================================================================*/

/* Application callback registered by airsync_reg_srv(). */
static P_FUN_AIRSYNC_SERVER_APP_CB pfn_airsync_cb = NULL;

/* GATT service ID assigned by stack at registration time. */
static T_SERVER_ID airsync_ser_id = 0xFF;

/* ---- Simple BLE Protocol payloads (0xFEA1 / 0xFEA2 / 0xFEC9) ---- */

/** 0xFEA1 Pedometer Measurement - payload returned for ATT Read Request. */
static uint8_t s_measurement_read_payload[4]    = {0x01, 0x01, 0x00, 0x00};

/** 0xFEA1 Pedometer Measurement - initial Notify pushed right after first read. */
static uint8_t s_measurement_initial_notify[4]  = {0x01, 0x00, 0x00, 0x01};

/** 0xFEA2 Pedometer Target - payload returned for ATT Read Request. */
static uint8_t s_target_read_payload[4]         = {0x01, 0x03, 0x00, 0x03};

/** 0xFEA2 Pedometer Target - payload pushed via ATT Indication. */
static uint8_t s_target_indicate_payload[8]     = {8, 7, 6, 5, 4, 3, 2, 1};

/** 0xFEC9 Device MAC - read buffer; filled at runtime with local BD_ADDR (6 bytes). */
static uint8_t s_mac_read_payload[6]            = {0};

/*============================================================================*
 *                              GATT Service Table
 *
 *  Attribute layout (attrib_index : content):
 *    0 : Primary Service Declaration (UUID 0xFEE7)
 *
 *    --- Simple BLE Protocol ---
 *    1 : Char Declaration  - Pedometer Measurement (Read | Notify)
 *    2 : Char Value        - 0xFEA1
 *    3 : CCCD              - for 0xFEA1 Notify
 *    4 : Char Declaration  - Pedometer Target (Read | Write | Indicate)
 *    5 : Char Value        - 0xFEA2
 *    6 : CCCD              - for 0xFEA2 Indicate
 *    7 : Char Declaration  - Device MAC (Read)
 *    8 : Char Value        - 0xFEC9
 *
 *    --- AirSync Protocol (reserved, append here when implemented) ---
 *    // 9..  : 0xFEC7 AirSync Write
 *    // ..   : 0xFEC8 AirSync Indicate + CCCD
 *    // ..   : 0xFEC9 AirSync Read   (note: shares UUID with Device MAC,
 *    //                               must NOT coexist with the simple
 *    //                               protocol's MAC characteristic)
 *============================================================================*/

static const T_ATTRIB_APPL airsync_ble_service_tbl[] =
{
    /* 0. <<Primary Service>> 0xFEE7 */
    {
#if CONFIG_AIRSYNC_GATT_OVER_BREDR
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE | ATTRIB_FLAG_BREDR),  /* dual transport */
#else
        (ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_LE),                       /* LE only        */
#endif
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
            LO_WORD(GATT_UUID_AIRSYNC_SERVICE),                 /* 0xFEE7 */
            HI_WORD(GATT_UUID_AIRSYNC_SERVICE)
        },
        UUID_16BIT_SIZE,
        NULL,
        GATT_PERM_READ
    },

    /* 1. <<Characteristic>> Pedometer Measurement decl (Read | Notify) */
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ | GATT_CHAR_PROP_NOTIFY
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    /* 2. Char Value: 0xFEA1 Pedometer Measurement */
    {
        ATTRIB_FLAG_VALUE_APPL,
        {
            LO_WORD(GATT_UUID_AIRSYNC_MEASUREMENT),             /* 0xFEA1 */
            HI_WORD(GATT_UUID_AIRSYNC_MEASUREMENT)
        },
        0,
        (void *)NULL,
        GATT_PERM_READ | GATT_PERM_WRITE
    },
    /* 3. CCCD for 0xFEA1 (Notify) */
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
    },

    /* 4. <<Characteristic>> Pedometer Target decl (Read | Write | Indicate) */
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ | GATT_CHAR_PROP_WRITE | GATT_CHAR_PROP_INDICATE
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    /* 5. Char Value: 0xFEA2 Pedometer Target */
    {
        ATTRIB_FLAG_VALUE_APPL,
        {
            LO_WORD(GATT_UUID_AIRSYNC_TARGET),                  /* 0xFEA2 */
            HI_WORD(GATT_UUID_AIRSYNC_TARGET)
        },
        0,
        (void *)NULL,
        GATT_PERM_READ | GATT_PERM_WRITE
    },
    /* 6. CCCD for 0xFEA2 (Indicate) */
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
    },

    /* 7. <<Characteristic>> Device MAC decl (Read only) */
    {
        ATTRIB_FLAG_VALUE_INCL,
        {
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ
        },
        1,
        NULL,
        GATT_PERM_READ
    },
    /* 8. Char Value: 0xFEC9 Device MAC */
    {
        ATTRIB_FLAG_VALUE_APPL,
        {
            LO_WORD(GATT_UUID_AIRSYNC_MAC),                     /* 0xFEC9 */
            HI_WORD(GATT_UUID_AIRSYNC_MAC)
        },
        0,
        (void *)NULL,
        GATT_PERM_READ | GATT_PERM_WRITE
    },
};

/*============================================================================*
 *                              SDP Record (BR/EDR only)
 *============================================================================*/

#if CONFIG_AIRSYNC_GATT_OVER_BREDR

/* SDP record published when AirSync GATT runs over BR/EDR ATT (PSM 0x001F).
 *
 *   - Service Class ID  : 0xFEE7
 *   - Protocol Stack    : L2CAP (PSM=ATT) -> ATT (start/end handle)
 *   - Browse Group      : Public Browse Root
 *
 * IMPORTANT: the ATT start/end handles embedded in this record describe the
 * GATT attribute range of THIS AirSync service. They are currently fixed
 * values that match the layout produced by gatt_svc_add() in the current
 * build. If the GATT database layout changes (new services added before/after
 * AirSync, attribute table modified, etc.), the values must be updated
 * accordingly, otherwise strict BR/EDR hosts will fail Service Discovery. */
static uint8_t airsync_sdp_record[] =
{
    SDP_DATA_ELEM_SEQ_HDR,
    0x2b,

    /* attribute SDP_ATTR_SRV_CLASS_ID_LIST */
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SRV_CLASS_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_SRV_CLASS_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(GATT_UUID_AIRSYNC_SERVICE >> 8),
    (uint8_t)(GATT_UUID_AIRSYNC_SERVICE),

    /* attribute SDP_ATTR_PROTO_DESC_LIST */
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROTO_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROTO_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x13,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_L2CAP >> 8),
    (uint8_t)(UUID_L2CAP),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(PSM_ATT >> 8),
    (uint8_t)(PSM_ATT),
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_ATT >> 8),
    (uint8_t)(UUID_ATT),
    /* GATT attribute handle range of the AirSync service (big-endian). */
    SDP_UNSIGNED_TWO_BYTE,
    0x00, 0x5B,                                     /* start handle */
    SDP_UNSIGNED_TWO_BYTE,
    0x00, 0x63,                                     /* end   handle */

    /* attribute SDP_ATTR_BROWSE_GROUP_LIST */
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_BROWSE_GROUP_LIST >> 8),
    (uint8_t)SDP_ATTR_BROWSE_GROUP_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP >> 8),
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP),
};

/**
 * @brief Submit the AirSync SDP record to the SDP server.
 */
static void airsync_sdp_register(void)
{
    if (bt_sdp_record_add((void *)airsync_sdp_record))
    {
        APP_PRINT_INFO0("airsync_sdp_register: ok");
    }
    else
    {
        APP_PRINT_ERROR0("airsync_sdp_register: bt_sdp_record_add failed");
    }
}

#endif /* CONFIG_AIRSYNC_GATT_OVER_BREDR */

/*============================================================================*
 *                              Transport Helper
 *============================================================================*/

/**
 * @brief  Resolve transport info from (conn_handle, cid).
 *
 *         Caller MUST pass a valid conn_handle/cid pair as delivered by the
 *         GATT callback context (or cached from a prior CCCD-update / write
 *         callback). This helper does NOT recover handles from the bond DB.
 *
 *         On success, *chann_type holds the resolved T_GAP_CHANN_TYPE,
 *         and *conn_id is filled when transport is LE
 *         (INVALID_CONN_ID on BR/EDR).
 *
 *         On failure (get_info failed, BR/EDR not allowed by build config,
 *         or unsupported chann_type), the function returns false. Caller
 *         MUST check the return value before using *chann_type / *conn_id.
 *
 * @param[in]  conn_handle  ACL handle from GATT cb context.
 * @param[in]  cid          ATT channel id from GATT cb context.
 * @param[out] chann_type   Resolved T_GAP_CHANN_TYPE on success.
 * @param[out] conn_id      LE conn_id on LE transport; INVALID_CONN_ID on BR/EDR.
 *
 * @retval true   resolved & allowed by current build config.
 * @retval false  resolve failed OR transport disabled by config.
 */
static bool airsync_resolve_link(uint16_t conn_handle, uint16_t cid,
                                 uint16_t *chann_type, uint8_t *conn_id)
{
    T_GAP_CHANN_INFO info;

    *conn_id = INVALID_CONN_ID;

    if (!gap_chann_get_info(conn_handle, cid, &info))
    {
        APP_PRINT_WARN2("[airsync_resolve_link] get_info failed, "
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
#if !CONFIG_AIRSYNC_GATT_OVER_BREDR
        APP_PRINT_WARN3("[airsync_resolve_link] BR/EDR not allowed "
                        "(CONFIG_AIRSYNC_GATT_OVER_BREDR=0), "
                        "conn_handle=0x%04x cid=0x%04x chann_type=%d",
                        conn_handle, cid, info.chann_type);
        return false;
#else
        /* allowed; conn_id stays INVALID_CONN_ID */
        break;
#endif

    default:
        APP_PRINT_WARN1("[airsync_resolve_link] unknown chann_type=%d",
                        info.chann_type);
        return false;
    }

    *chann_type = info.chann_type;

    APP_PRINT_INFO3("[airsync_resolve_link] conn_handle=0x%04x cid=0x%04x "
                    "-> chann_type=%d", conn_handle, cid, *chann_type);
    return true;
}

/*============================================================================*
 *                              GATT Callbacks
 *============================================================================*/

/**
 * @brief Read callback for AirSync (0xFEE7) characteristics.
 *
 *        Returns the static read payload to the stack. For 0xFEA1, an initial
 *        Notification is also pushed on the same link. The 0xFEA2 Indication
 *        is intentionally NOT triggered from this read path; it should be
 *        emitted by the upper-layer state machine via gatt_svc_send_data().
 */
static T_APP_RESULT airsync_service_attr_read_cb(uint16_t conn_handle, uint16_t cid,
                                                 T_SERVER_ID   service_id,
                                                 uint16_t      attrib_index,
                                                 uint16_t      offset,
                                                 uint16_t     *p_length,
                                                 uint8_t     **pp_value)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;

    switch (attrib_index)
    {
    case GATT_UUID_AIRSYNC_CHAR_MEASUREMENT_INDEX:
        *pp_value = s_measurement_read_payload;
        *p_length = sizeof(s_measurement_read_payload);
        APP_PRINT_INFO1("airsync read MEASUREMENT(0xFEA1): %b",
                        TRACE_BINARY(*p_length, *pp_value));

        /* Push an initial measurement notification on the same link. */
        airsync_service_send_measurement_notification(conn_handle, cid,
                                                      s_measurement_initial_notify,
                                                      sizeof(s_measurement_initial_notify));
        break;

    case GATT_UUID_AIRSYNC_CHAR_TARGET_INDEX:
        *pp_value = s_target_read_payload;
        *p_length = sizeof(s_target_read_payload);
        APP_PRINT_INFO1("airsync read TARGET(0xFEA2): %b",
                        TRACE_BINARY(*p_length, *pp_value));
        break;

    case GATT_UUID_AIRSYNC_CHAR_MAC_INDEX:
        /* Fill buffer with current local public BD_ADDR. */
        gap_get_param(GAP_PARAM_BD_ADDR, s_mac_read_payload);
        *pp_value = s_mac_read_payload;
        *p_length = sizeof(s_mac_read_payload);
        APP_PRINT_INFO1("airsync read MAC(0xFEC9): %b",
                        TRACE_BINARY(*p_length, *pp_value));
        break;

    default:
        APP_PRINT_ERROR1("airsync read: attr not found, attrib_index=%d",
                         attrib_index);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    }

    return cause;
}

/**
 * @brief Write callback for AirSync (0xFEE7) characteristics.
 */
static T_APP_RESULT airsync_service_attr_write_cb(uint16_t conn_handle, uint16_t cid,
                                                  T_SERVER_ID                    service_id,
                                                  uint16_t                       attrib_index,
                                                  T_WRITE_TYPE                   write_type,
                                                  uint16_t                       length,
                                                  uint8_t                       *p_value,
                                                  P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    uint8_t  conn_id;
    uint16_t chann_type;

    if (p_value == NULL)
    {
        APP_PRINT_ERROR0("airsync write: p_value is NULL");
        return APP_RESULT_INVALID_VALUE_SIZE;
    }

    if (!airsync_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR2("airsync write: resolve link failed, "
                         "conn_handle=0x%04x cid=0x%04x", conn_handle, cid);
        return APP_RESULT_APP_ERR;
    }

    switch (attrib_index)
    {
    case GATT_UUID_AIRSYNC_CHAR_TARGET_INDEX:
        APP_PRINT_INFO3("airsync write TARGET(0xFEA2): chann_type=%d len=%d data=%b",
                        chann_type, length, TRACE_BINARY(length, p_value));
        return APP_RESULT_SUCCESS;

    default:
        APP_PRINT_INFO2("airsync write: unsupported attrib_index=0x%x len=%d",
                        attrib_index, length);
        return APP_RESULT_ATTR_NOT_FOUND;
    }
}

/**
 * @brief CCCD update callback for AirSync (0xFEE7) characteristics.
 *
 *        Resolves transport, then forwards an enable/disable event to the
 *        registered application callback.
 */
static void airsync_service_cccd_update_cb(uint16_t conn_handle, uint16_t cid,
                                           T_SERVER_ID service_id,
                                           uint16_t    attrib_index,
                                           uint16_t    cccd_bits)
{
    uint8_t  conn_id;
    uint16_t chann_type;
    bool     is_handled = false;
    T_AIRSYNC_CALLBACK_DATA callback_data;

    if (!airsync_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR3("airsync cccd_update: resolve link failed, "
                         "conn_handle=0x%04x cid=0x%04x attrib_index=%d",
                         conn_handle, cid, attrib_index);
        return;
    }

    callback_data.conn_id    = conn_id;
    callback_data.chann_type = chann_type;

    APP_PRINT_INFO2("airsync cccd_update: attrib_index=%d cccd_bits=0x%x",
                    attrib_index, cccd_bits);

    switch (attrib_index)
    {
    case GATT_UUID_AIRSYNC_CHAR_MEASUREMENT_CCCD_INDEX:
        callback_data.msg_data.notification_indification_index =
            (cccd_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
            ? GATT_UUID_AIRSYNC_CHAR_NOTIFY_ENABLE
            : GATT_UUID_AIRSYNC_CHAR_NOTIFY_DISABLE;
        is_handled = true;
        break;

    case GATT_UUID_AIRSYNC_CHAR_TARGET_CCCD_INDEX:
        callback_data.msg_data.notification_indification_index =
            (cccd_bits & GATT_CLIENT_CHAR_CONFIG_INDICATE)
            ? GATT_UUID_AIRSYNC_CHAR_INDICATION_ENABLE
            : GATT_UUID_AIRSYNC_CHAR_INDICATION_DISABLE;
        is_handled = true;
        break;

    default:
        break;
    }

    if (pfn_airsync_cb && is_handled)
    {
        pfn_airsync_cb(GATT_MSG_AIRSYNC_SERVER_CCCD_UPDATE, (void *)&callback_data);
    }
}

static const T_FUN_GATT_EXT_SERVICE_CBS airsync_service_cbs =
{
    airsync_service_attr_read_cb,
    airsync_service_attr_write_cb,
    airsync_service_cccd_update_cb,
};

/*============================================================================*
 *                              App Callback
 *============================================================================*/

/**
 * @brief Default application-level callback for AirSync events.
 */
static T_APP_RESULT airsync_app_callback(uint8_t type, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_AIRSYNC_CALLBACK_DATA *p_cb_data = (T_AIRSYNC_CALLBACK_DATA *)p_data;
    (void)p_cb_data;

    APP_PRINT_INFO1("airsync_app_callback: type=%d", type);

    switch (type)
    {
    case GATT_MSG_AIRSYNC_SERVER_CCCD_UPDATE:
        APP_PRINT_INFO0("airsync_app_callback: CCCD update event");
        break;

    default:
        break;
    }
    return app_result;
}

/*============================================================================*
 *                              Public Functions
 *============================================================================*/

/**
 * @brief  Register the AirSync (0xFEE7) GATT service to the stack.
 * @param  app_cb  Application callback for read/write/CCCD events.
 * @return Service ID assigned by stack, or 0xFF on failure.
 */
T_SERVER_ID airsync_reg_srv(P_FUN_AIRSYNC_SERVER_APP_CB app_cb)
{
    T_SERVER_ID service_id;

    if (false == gatt_svc_add(&service_id,
                              (uint8_t *)airsync_ble_service_tbl,
                              sizeof(airsync_ble_service_tbl),
                              &airsync_service_cbs, NULL))
    {
        return 0xFF;
    }

    pfn_airsync_cb = app_cb;
    airsync_ser_id = service_id;
    APP_PRINT_INFO1("airsync: service_id=0x%x", service_id);
    return service_id;
}

/**
 * @brief  Send a Notification on the Pedometer Measurement (0xFEA1) characteristic.
 *
 *         Caller MUST pass a valid (conn_handle, cid) pair, typically cached
 *         from a prior CCCD-enable / write callback. The function resolves
 *         the transport via airsync_resolve_link() and drops the request if
 *         the link is no longer valid or the transport is disabled by config.
 *
 * @param[in] conn_handle  ACL handle of the target link.
 * @param[in] cid          ATT channel id of the target bearer.
 * @param[in] p_data       Notification payload.
 * @param[in] data_len     Payload length in bytes.
 */
void airsync_service_send_measurement_notification(uint16_t conn_handle, uint16_t cid,
                                                   uint8_t *p_data, uint16_t data_len)
{
    uint8_t  conn_id;
    uint16_t chann_type;

    if (!airsync_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR3("airsync notify MEASUREMENT: resolve failed, drop. "
                         "conn_handle=0x%04x cid=0x%04x len=%d",
                         conn_handle, cid, data_len);
        return;
    }

    APP_PRINT_INFO3("airsync notify MEASUREMENT(0xFEA1): chann_type=%d len=%d data=%b",
                    chann_type, data_len, TRACE_BINARY(data_len, p_data));

    gatt_svc_send_data(conn_handle, cid,
                       airsync_ser_id,
                       GATT_UUID_AIRSYNC_CHAR_MEASUREMENT_INDEX,
                       p_data, data_len,
                       GATT_PDU_TYPE_NOTIFICATION);
}

/*============================================================================*
 *                              Module Auto Init
 *============================================================================*/

/**
 * @brief Auto-init entry registered via APP_MODULE_INIT().
 *        Called by app_module_init_all() at startup.
 *
 *        Steps:
 *          1) Register GATT service (always)
 *          2) Register SDP record (only when BR/EDR transport is enabled).
 *             Must run after step 1.
 */
static void airsync_module_init(void)
{
    /* 1. Register GATT service. */
    airsync_reg_srv(airsync_app_callback);

#if CONFIG_AIRSYNC_GATT_OVER_BREDR
    /* 2. Register SDP record. */
    if (airsync_ser_id != 0xFF)
    {
        airsync_sdp_register();
    }
    else
    {
        APP_PRINT_ERROR0("airsync_module_init: GATT register failed, skip SDP");
    }
#endif
}
APP_MODULE_INIT(airsync_module_init);

APP_BLE_SERVICE_INFO(airsync, 1);

#endif /* CONFIG_AIRSYNC_SERVICE_SUPPORT */
