/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
*                              Header Files
*============================================================================*/

#include <gatt.h>
#include <bt_types.h>
#include "trace.h"
#include "app_main.h"
#include "app_cfg.h"
#include "playback_service.h"
#include "app_playback_update_file.h"
#include "gap_conn_le.h"
#include "gap.h"
#include "gap_chann.h"
#include "bt_gatt_svc.h"
#include "app_module_init.h"
#include "app_ble_service_info.h"

#if CONFIG_PLAYBACK_GATT_OVER_BREDR
#include "bt_sdp.h"
#endif

/** @defgroup  PLAYBACK_SERVICE PLAYBACK Service
 * @brief LE / BR-EDR Service to implement PLAYBACK feature
 * @{
 */

/*============================================================================*
*                              Constants
*============================================================================*/

#define INVALID_CONN_ID                 0xFF

#define LOG_CHANN_UNKNOWN               0
#define LOG_CHANN_LE                    1
#define LOG_CHANN_BREDR                 2

/** @brief  PLAYBACK service UUID (128-bit). */
static const uint8_t GATT_UUID_PLAYBACK_SERVICE[16] = {GATT_UUID128_PLAYBACK_SERVICE_ADV};

/*============================================================================*
*                              GATT Service Table
*============================================================================*/

static const T_ATTRIB_APPL gatt_extended_service_table[] =
{
    /* Primary Service, index 0 */
    {
#if CONFIG_PLAYBACK_GATT_OVER_BREDR
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE | ATTRIB_FLAG_BREDR),
#else
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),
#endif
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),
        },
        UUID_128BIT_SIZE,
        (void *)GATT_UUID_PLAYBACK_SERVICE,
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
            LO_WORD(GATT_UUID_CHAR_PLAYBACK_WRITE),
            HI_WORD(GATT_UUID_CHAR_PLAYBACK_WRITE),
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
            LO_WORD(GATT_UUID_CHAR_PLAYBACK_NOTIFY),
            HI_WORD(GATT_UUID_CHAR_PLAYBACK_NOTIFY),
        },
        0,
        (void *)NULL,
        GATT_PERM_NOTIF_IND
    },

    /* Client Characteristic Configuration Descriptor (CCCD) */
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

#if CONFIG_PLAYBACK_GATT_OVER_BREDR

/* NOTE: GATT service handle range below is hard-coded.
 *       The values MUST match the actual handle range assigned by
 *       gatt_svc_add(). Update manually if the service table or
 *       registration order changes.
 */
static uint8_t playback_sdp_record[] =
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
    /* Custom UUID: F5178291-8267-426C-BDA1-704AA9584D49 (little-endian on wire) */
    0x49, 0x4D, 0x58, 0xA9, 0x4A, 0x70, 0xA1, 0xBD,
    0x6C, 0x42, 0x67, 0x82, 0x91, 0x82, 0x17, 0xF5,

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

    /* Layer 2: ATT, with GATT service handle range */
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_ATT >> 8),
    (uint8_t)(UUID_ATT),
    /* Start handle */
    SDP_UNSIGNED_TWO_BYTE,
    0x00, 0x4D,
    /* End handle */
    SDP_UNSIGNED_TWO_BYTE,
    0x00, 0x52,

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

#endif /* CONFIG_PLAYBACK_GATT_OVER_BREDR */

/*============================================================================*
*                              Variables
*============================================================================*/

/* Public service ID, referenced by other modules (e.g. app layer to send
 * notifications via gatt_svc_send_data()). Defined here, declared extern
 * in playback_service.h. */
T_SERVER_ID srv_id_local = 0xFF;

/* Application callback registered through playback_reg_srv(). */
static P_FUN_PLAYBACK_SERVER_APP_CB p_fn_playback_service_cb = NULL;

/*============================================================================*
 *                              Functions
 *============================================================================*/

/**
 * @brief  GATT callback for playback service
 */
T_APP_RESULT playback_gatt_svc_callback(uint8_t type, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;

    switch (type)
    {
    case GATT_MSG_PLAYBACK_SERVER_WRITE:
        break;

    case GATT_MSG_PLAYBACK_SERVER_CCCD_UPDATE:
        break;

    default:
        break;
    }

    return app_result;
}

/*============================================================================*
*                              Internal Helpers
*============================================================================*/

/**
 * @brief  Resolve transport info from (conn_handle, cid).
 *
 *         Caller MUST pass a valid conn_handle that is currently active.
 *         This function does NOT recover handles from bond DB; that
 *         responsibility belongs to the caller (see send_notification).
 *
 *         On success, *chann_type is the resolved T_GAP_CHANN_TYPE,
 *         and *conn_id is filled when transport is LE.
 *
 *         On failure (get_info failed, BR/EDR not allowed by config, or
 *         unsupported chann_type), the function returns false. Caller
 *         MUST check the return value before using *chann_type / *conn_id.
 *
 * @param[in]  conn_handle  ACL handle from GATT cb context.
 * @param[in]  cid          ATT channel id from GATT cb context.
 * @param[out] chann_type   Resolved T_GAP_CHANN_TYPE on success.
 * @param[out] conn_id      LE conn_id on LE transport; INVALID_CONN_ID on BR/EDR.
 *
 * @retval true   resolved & allowed.
 * @retval false  resolve failed OR transport not allowed by config.
 */
static bool playback_resolve_link(uint16_t conn_handle, uint16_t cid,
                                  uint16_t *chann_type, uint8_t *conn_id)
{
    T_GAP_CHANN_INFO info;

    *conn_id = INVALID_CONN_ID;

    if (!gap_chann_get_info(conn_handle, cid, &info))
    {
        APP_PRINT_WARN2("[playback_resolve_link] get_info failed, "
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
#if !CONFIG_PLAYBACK_GATT_OVER_BREDR
        APP_PRINT_WARN3("[playback_resolve_link] BR/EDR not allowed "
                        "(CONFIG_PLAYBACK_GATT_OVER_BREDR=0), "
                        "conn_handle=0x%04x cid=0x%04x chann_type=%d",
                        conn_handle, cid, info.chann_type);
        return false;
#else
        /* allowed; nothing extra to fetch for BR/EDR transport */
        break;
#endif

    default:
        APP_PRINT_WARN1("[playback_resolve_link] unknown chann_type=%d",
                        info.chann_type);
        return false;
    }

    *chann_type = info.chann_type;

    APP_PRINT_INFO3("[playback_resolve_link] conn_handle=0x%04x cid=0x%04x "
                    "-> chann_type=%d", conn_handle, cid, *chann_type);
    return true;
}

/**
 * @brief  Map channel type to a short identifier for logging.
 * @return 1 = LE, 2 = BR/EDR, 0 = Unknown.
 */
static inline uint8_t playback_log_chann(uint16_t chann_type)
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
 */
static T_APP_RESULT playback_service_attr_write_cb(uint16_t conn_handle, uint16_t cid,
                                                   T_SERVER_ID service_id,
                                                   uint16_t attr_index, T_WRITE_TYPE write_type,
                                                   uint16_t length, uint8_t *p_value,
                                                   P_FUN_EXT_WRITE_IND_POST_PROC *p_write_ind_post_proc)
{
    uint8_t  conn_id;
    uint16_t chann_type;

    if (!playback_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR2("playback_service_attr_write_cb: resolve failed, "
                         "conn_handle=0x%04x cid=0x%04x", conn_handle, cid);
        return APP_RESULT_APP_ERR;
    }

    APP_PRINT_INFO4("playback_service_attr_write_cb: chann_type=%d, write_type=%d, attr_idx=0x%02x, len=%d",
                    playback_log_chann(chann_type), write_type, attr_index, length);

    if (attr_index != BLE_SERVICE_CHAR_PLAYBACK_WRITE_INDEX)
    {
        APP_PRINT_ERROR1("playback_service_attr_write_cb: unknown attr_index 0x%x", attr_index);
        return APP_RESULT_ATTR_NOT_FOUND;
    }

    return app_playback_ble_handle_cp_req(conn_id, conn_handle, cid,
                                          chann_type, length, p_value);
}

/**
 * @brief    CCCD update callback from GATT layer.
 */
void playback_cccd_update_cb(uint16_t conn_handle, uint16_t cid,
                             T_SERVER_ID service_id,
                             uint16_t attrib_idx, uint16_t cccd_bits)
{
    uint8_t  conn_id;
    uint16_t chann_type;
    T_PLAYBACK_CALLBACK_DATA callback_data;

    if (!playback_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR2("playback_cccd_update_cb: resolve failed, "
                         "conn_handle=0x%04x cid=0x%04x", conn_handle, cid);
        return;
    }

    APP_PRINT_INFO3("playback_cccd_update_cb: chann_type=%d, attrib_idx=%d, cccd_bits=0x%04x",
                    playback_log_chann(chann_type), attrib_idx, cccd_bits);

    if (attrib_idx != BLE_SERVICE_CHAR_PLAYBACK_DATA_CCCD_INDEX)
    {
        return;
    }

    if (cccd_bits & GATT_CLIENT_CHAR_CONFIG_NOTIFY)
    {
        APP_PRINT_INFO0("playback_cccd_update_cb: NOTIFY ENABLED");
    }
    else
    {
        APP_PRINT_INFO0("playback_cccd_update_cb: NOTIFY DISABLED");
    }

    callback_data.conn_id     = conn_id;
    callback_data.conn_handle = conn_handle;
    callback_data.cid         = cid;
    callback_data.chann_type  = chann_type;
    callback_data.msg_data.notification_indification_index = BLE_SERVICE_CHAR_PLAYBACK_NOTIFY_INDEX;

    if (p_fn_playback_service_cb)
    {
        p_fn_playback_service_cb(GATT_MSG_PLAYBACK_SERVER_CCCD_UPDATE, (void *)&callback_data);
    }
}

/** @brief  PLAYBACK Service Callback table */
static const T_FUN_GATT_EXT_SERVICE_CBS playback_service_cbs =
{
    NULL,                           /**< Read callback  */
    playback_service_attr_write_cb, /**< Write callback */
    playback_cccd_update_cb         /**< CCCD update callback */
};

/*============================================================================*
*                              SDP Helper (BR/EDR only)
*============================================================================*/

#if CONFIG_PLAYBACK_GATT_OVER_BREDR

/**
 * @brief Submit the playback SDP record to the SDP server.
 */
void playback_sdp_register(void)
{
    if (bt_sdp_record_add((void *)playback_sdp_record))
    {
        APP_PRINT_INFO0("playback_sdp_register: ok");
    }
    else
    {
        APP_PRINT_ERROR0("playback_sdp_register: bt_sdp_record_add failed");
    }
}

#endif /* CONFIG_PLAYBACK_GATT_OVER_BREDR */

/*============================================================================*
*                              Public Functions
*============================================================================*/

/**
 * @brief    Send notification on PLAYBACK Notify characteristic.
 *
 *           Caller MUST provide a valid conn_handle/cid pair obtained
 *           from a prior CCCD-enable event or write callback.
 */
void playback_service_send_notification(uint16_t conn_handle, uint16_t cid,
                                        uint8_t *p_data, uint16_t data_len)
{
    uint8_t  conn_id;
    uint16_t chann_type;

    if (!playback_resolve_link(conn_handle, cid, &chann_type, &conn_id))
    {
        APP_PRINT_ERROR3("playback_service_send_notification: resolve failed, "
                         "drop notify. conn_handle=0x%04x cid=0x%04x len=%d",
                         conn_handle, cid, data_len);
        return;
    }

    APP_PRINT_INFO3("playback_service_send_notification: chann_type=%d, conn_handle=0x%04x, len=%d",
                    playback_log_chann(chann_type), conn_handle, data_len);

    gatt_svc_send_data(conn_handle, cid, srv_id_local,
                       BLE_SERVICE_CHAR_PLAYBACK_NOTIFY_INDEX,
                       p_data, data_len,
                       GATT_PDU_TYPE_NOTIFICATION);
}

/**
 * @brief    Register PLAYBACK service to GATT layer.
 * @param    app_cb  Application-level callback for write/cccd events.
 * @return   Service ID assigned by the profile layer (0xFF on failure).
 */
T_SERVER_ID playback_reg_srv(P_FUN_PLAYBACK_SERVER_APP_CB app_cb)
{
    T_SERVER_ID service_id;

    if (!gatt_svc_add(&service_id,
                      (uint8_t *)gatt_extended_service_table,
                      sizeof(gatt_extended_service_table),
                      &playback_service_cbs, NULL))
    {
        APP_PRINT_ERROR0("playback_reg_srv: gatt_svc_add failed");
        return 0xFF;
    }

    p_fn_playback_service_cb = app_cb;
    srv_id_local             = service_id;

    APP_PRINT_INFO1("playback_reg_srv: success, service_id=0x%x", service_id);
    return service_id;
}

/** @} */ /* End of group PLAYBACK_SERVICE */

/* Register playback service to ELF section */
APP_BLE_SERVICE_INFO(playback, 1);
