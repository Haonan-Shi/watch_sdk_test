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
#include "app_ble_service_info.h"
#include "app_le_throughput.h"
#include "event_bus.h"
#include "os_timer.h"
#include "os_mem.h"
#include "app_module_init.h"

/********************************************************************************************************
 * Local constants
 ********************************************************************************************************/

/** Event bus topic for starting the throughput TX test */
#define THROUGHPUT_EVENT_TEST_START     "le_throughput/test_start"

/** Event bus topic for triggering the next batch of data sends */
#define THROUGHPUT_EVENT_SEND_NEXT      "le_throughput/test_send_next"

/** Default ATT header overhead (opcode + handle) subtracted from MTU for payload */
#define ATT_HEADER_SIZE                 3

/** Maximum payload per packet (safe upper bound for ATT notification payload) */
#define THROUGHPUT_MAX_PACKET_SIZE      256

/********************************************************************************************************
 * Local types
 ********************************************************************************************************/

/** LE Throughput TX test context */
typedef struct
{
    bool                    active;             /**< Test is currently running */
    T_THROUGHPUT_TEST_MODE  mode;               /**< TX or RXTX test mode */
    uint16_t                conn_handle;        /**< BLE connection handle */
    uint16_t                cid;                /**< L2CAP channel ID */
    uint8_t                 conn_id;            /**< BLE connection ID */
    uint16_t                mtu_size;           /**< Current ATT MTU size */
    uint16_t                packet_len;         /**< Configured payload length per packet */
    uint16_t                duration;           /**< Test duration in seconds */
    uint32_t                total_bytes_sent;   /**< Total bytes sent during test */
    uint32_t                total_packets_sent; /**< Total packets sent during test */
    uint32_t                total_bytes_received;   /**< Total bytes received during test */
    uint32_t                total_packets_received; /**< Total packets received during test */
    void                   *timer_handle;       /**< Software timer handle for duration control */
    T_EVENT_BUS_SUBSCRIBER_HANDLE start_handle; /**< Eventbus handle for test_start */
    T_EVENT_BUS_SUBSCRIBER_HANDLE send_handle;  /**< Eventbus handle for send_next */
} T_LE_THROUGHPUT_TEST_CTX;

/********************************************************************************************************
 * Local variables
 ********************************************************************************************************/
T_SERVER_ID le_throughput_gatt_srv_id = 0xFF;

/** Test context instance (zero-initialised => idle) */
static T_LE_THROUGHPUT_TEST_CTX s_test_ctx = { 0 };

/** Static buffer for sending test data (avoids per-packet heap alloc/free) */
static uint8_t s_send_buffer[THROUGHPUT_MAX_PACKET_SIZE];

/********************************************************************************************************
 * Forward declarations
 ********************************************************************************************************/
static T_APP_RESULT app_le_throughput_gatt_svc_callback(uint8_t type, void *p_data);
static void app_le_throughput_send_data_cb(T_EXT_SEND_DATA_RESULT result);
static int32_t app_le_throughput_test_start_handler(T_EVENT_BUS_EVENT_DATA *p_data);
static int32_t app_le_throughput_send_next_handler(T_EVENT_BUS_EVENT_DATA *p_data);
static void app_le_throughput_timer_cb(void *p_handle);
static void le_throughput_send_data_packets(void);
static void le_throughput_stop_test(void);

/********************************************************************************************************
 * Internal helpers
 ********************************************************************************************************/

/**
 * @brief Send data packets to the Phone using available LE credits.
 *
 * Retrieves the remaining LE credits via le_get_gap_param() and sends exactly
 * that many packets in a single batch. Each packet carries a payload of
 * min(MTU - 3, configured packet_len) bytes filled with a pattern derived from
 * the packet sequence number.
 *
 * A static buffer is used to avoid per-packet heap allocation and freeing.
 * After exhausting all available credits, the function returns and relies on
 * the send-data-complete callback (app_le_throughput_send_data_cb) to trigger
 * the next batch via THROUGHPUT_EVENT_SEND_NEXT.
 */
static void le_throughput_send_data_packets(void)
{
    uint16_t send_len;
    uint8_t credits;

    if (!s_test_ctx.active)
    {
        return;
    }

    /* Calculate send payload length: honour the configured packet_len,
     * but never exceed what fits in one ATT notification (MTU - 3).
     */
    send_len = (s_test_ctx.mtu_size > ATT_HEADER_SIZE)
               ? (s_test_ctx.mtu_size - ATT_HEADER_SIZE)
               : s_test_ctx.mtu_size;

    if ((s_test_ctx.packet_len > 0) && (s_test_ctx.packet_len < send_len))
    {
        send_len = s_test_ctx.packet_len;
    }

    /* Query remaining LE credits */
    le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS, &credits);

    APP_PRINT_INFO2("le_throughput: send_data_packets credits=%d, send_len=%d",
                    credits, send_len);

    /* Send exactly 'credits' number of packets, then wait for send complete callback */
    while (s_test_ctx.active && credits > 0)
    {
        /* Fill static buffer with a pattern based on packet sequence number */
        memset(s_send_buffer, (uint8_t)(s_test_ctx.total_packets_sent & 0xFF), send_len);

        if (!le_throughput_send_dt_tx_data(s_test_ctx.conn_handle,
                                           s_test_ctx.cid,
                                           s_send_buffer, send_len))
        {
            /* Queue full or unexpected failure: stop and wait for callback */
            break;
        }

        s_test_ctx.total_bytes_sent += send_len;
        s_test_ctx.total_packets_sent++;
        credits--;
    }
}

/**
 * @brief Stop the current throughput test and notify the Phone.
 */
static void le_throughput_stop_test(void)
{
    if (!s_test_ctx.active)
    {
        return;
    }

    APP_PRINT_INFO4("le_throughput: test STOPPED, tx_bytes=%lu, tx_pkgs=%lu, rx_bytes=%lu, rx_pkgs=%lu",
                    (unsigned long)s_test_ctx.total_bytes_sent,
                    (unsigned long)s_test_ctx.total_packets_sent,
                    (unsigned long)s_test_ctx.total_bytes_received,
                    (unsigned long)s_test_ctx.total_packets_received);

    s_test_ctx.active = false;

    /* Stop and delete the duration timer */
    if (s_test_ctx.timer_handle != NULL)
    {
        os_timer_stop(&s_test_ctx.timer_handle);
        os_timer_delete(&s_test_ctx.timer_handle);
        s_test_ctx.timer_handle = NULL;
    }

    /* Send TEST_COMPLETE to Phone via CT_TX (control channel) */
    {
        uint8_t opcode = LE_OPCODE_TEST_COMPLETE;
        le_throughput_send_ct_tx_data(s_test_ctx.conn_handle,
                                      s_test_ctx.cid,
                                      &opcode, sizeof(opcode));
    }

    APP_PRINT_INFO0("le_throughput: TEST_COMPLETE sent to Phone");
}

/********************************************************************************************************
 * Timer callback
 ********************************************************************************************************/

/**
 * @brief Timer callback invoked when the test duration expires.
 *
 * Stops the test and notifies the Phone that the test is complete.
 */
static void app_le_throughput_timer_cb(void *p_handle)
{
    (void)p_handle;

    APP_PRINT_INFO0("le_throughput: timer expired stopping test");
    le_throughput_stop_test();
}

/********************************************************************************************************
 * Event bus handlers
 ********************************************************************************************************/

/**
 * @brief Eventbus handler for THROUGHPUT_EVENT_TEST_START.
 *
 * Triggered after the Phone's CONFIG_TX_PARAMS / CONFIG_RXTX_PARAMS has been
 * processed. Starts sending data packets using the available LE credits.
 */
static int32_t app_le_throughput_test_start_handler(T_EVENT_BUS_EVENT_DATA *p_data)
{
    (void)p_data;

    APP_PRINT_INFO2("le_throughput: test_start event received, mode=%d, dur=%ds",
                    s_test_ctx.mode, s_test_ctx.duration);

    if (!s_test_ctx.active)
    {
        APP_PRINT_WARN0("le_throughput: test_start ignored (inactive)");
        return EVENT_BUS_OK;
    }

    /* Ensure DT_TX notification is enabled before sending */
    if (!le_throughput_is_dt_tx_enabled(s_test_ctx.conn_handle, s_test_ctx.cid))
    {
        APP_PRINT_WARN0("le_throughput: DT_TX not enabled by Phone, skipping TX");
        return EVENT_BUS_OK;
    }

    /* Kick off the first batch of data packets */
    le_throughput_send_data_packets();

    return EVENT_BUS_OK;
}

/**
 * @brief Eventbus handler for THROUGHPUT_EVENT_SEND_NEXT.
 *
 * Triggered by the send-data-complete callback when credits are released.
 * Continues sending more data packets until the queue is full again or
 * the test stops.
 */
static int32_t app_le_throughput_send_next_handler(T_EVENT_BUS_EVENT_DATA *p_data)
{
    (void)p_data;

    if (!s_test_ctx.active)
    {
        return EVENT_BUS_OK;
    }

    le_throughput_send_data_packets();

    return EVENT_BUS_OK;
}

/********************************************************************************************************
 * Send data complete callback
 ********************************************************************************************************/

/**
 * @brief Send data complete callback registered via le_throughput_reg_srv.
 *
 * Called by the BLE stack when a previously queued notification has been
 * sent (or failed). If the test is still active and credits are available,
 * publishes THROUGHPUT_EVENT_SEND_NEXT to continue sending.
 */
static void app_le_throughput_send_data_cb(T_EXT_SEND_DATA_RESULT result)
{
    APP_PRINT_INFO3("le_throughput: send_cb credits=%d, cause=0x%04x, idx=%d",
                    result.credits, result.cause, result.attrib_idx);

    if (result.attrib_idx != LE_THROUGHPUT_DT_TX_VALUE_INDEX)
    {
        return;
    }

    if (!s_test_ctx.active)
    {
        return;
    }

    /* If the send succeeded and credits are available, continue sending */
    if (result.cause == GAP_SUCCESS)
    {
        event_bus_publish(THROUGHPUT_EVENT_SEND_NEXT, NULL, 0);
    }
    else
    {
        APP_PRINT_WARN1("le_throughput: send failed cause=0x%04x", result.cause);
    }
}

/********************************************************************************************************
 * Application callback (GATT service events)
 ********************************************************************************************************/

static T_APP_RESULT app_le_throughput_gatt_svc_callback(uint8_t type, void *p_data)
{
    T_APP_RESULT app_result = APP_RESULT_SUCCESS;
    T_LE_THROUGHPUT_CALLBACK_DATA *p_cb_data = (T_LE_THROUGHPUT_CALLBACK_DATA *)p_data;

    switch (type)
    {
    case GATT_MSG_LE_THROUGHPUT_WRITE:
        {
            uint8_t conn_id = p_cb_data->conn_id;
            uint8_t *p_value = p_cb_data->msg_data.rx_data.p_value;
            uint16_t len = p_cb_data->msg_data.rx_data.len;
            uint16_t conn_handle = p_cb_data->conn_handle;
            uint16_t cid = p_cb_data->cid;
            uint16_t mtu_size;

            APP_PRINT_INFO3("app_le_throughput: WRITE attr_idx %d, len %d, conn_id %d",
                            p_cb_data->attr_index, len, conn_id);

            if (p_cb_data->attr_index == LE_THROUGHPUT_CT_RX_VALUE_INDEX)
            {
                /* Control channel (CT_RX) - parse opcode */
                if (len < 1)
                {
                    break;
                }

                uint8_t opcode = p_value[0];

                switch (opcode)
                {
                case LE_OPCODE_GET_MTU:
                    {
                        /* Return current ATT MTU value */
                        T_GET_MTU_RESPONSE rsp;
                        rsp.opcode = LE_OPCODE_GET_MTU_RSP;
                        le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &mtu_size, conn_id);
                        rsp.mtu = mtu_size;

                        le_throughput_send_ct_tx_data(conn_handle, cid,
                                                      (uint8_t *)&rsp, sizeof(rsp));
                        APP_PRINT_INFO1("app_le_throughput: GET_MTU rsp mtu=%d", mtu_size);
                    }
                    break;

                case LE_OPCODE_CONFIG_TX_PARAMS:
                case LE_OPCODE_CONFIG_RX_PARAMS:
                case LE_OPCODE_CONFIG_RXTX_PARAMS:
                    {
                        /* Parse test configuration parameters */
                        if (len < sizeof(T_TEST_START_REQUEST))
                        {
                            APP_PRINT_ERROR0("app_le_throughput: config params too short");
                            break;
                        }

                        T_TEST_START_REQUEST *p_req = (T_TEST_START_REQUEST *)p_value;

                        APP_PRINT_INFO4("app_le_throughput: CONFIG opcode=0x%02x, pkt_len=%d, dur=%ds, flag=0x%02x",
                                        p_req->opcode, p_req->packet_len, p_req->duration, p_req->flag);

                        /* Build config response */
                        T_CONFIG_PARAMS_RESPONSE rsp;
                        rsp.opcode = LE_OPCODE_CONFIGPARAMS_RSP;
                        rsp.interval = 0;
                        rsp.flag = 0;

                        /* Request connection parameter update */
                        T_GAP_CAUSE cause = le_update_conn_param(conn_id,
                                                                 p_req->min_interval,
                                                                 p_req->max_interval,
                                                                 0,      /* slave latency */
                                                                 500,    /* supervision timeout *10ms */
                                                                 2 * (p_req->min_interval - 1),      /* ce_length_min */
                                                                 2 * (p_req->max_interval - 1));     /* ce_length_max */

                        if (cause == GAP_CAUSE_SUCCESS)
                        {
                            rsp.status = THROUGHPUT_STATUS_SUCCESS;
                            APP_PRINT_INFO0("app_le_throughput: conn param update requested");
                        }
                        else
                        {
                            rsp.status = THROUGHPUT_STATUS_TIMEOUT;
                            APP_PRINT_WARN1("app_le_throughput: conn param update failed: 0x%x", cause);
                        }

                        /* Enable DLE if requested */
                        if (p_req->flag & THROUGHPUT_FLAG_DLE)
                        {
                            le_set_data_len(conn_id, 251, 2120);
                            rsp.flag |= THROUGHPUT_FLAG_DLE;
                        }

                        /* Enable 2M PHY if requested */
                        if (p_req->flag & THROUGHPUT_FLAG_2M_PHY)
                        {
                            le_set_phy(conn_id, 0, GAP_PHYS_PREFER_2M_BIT,
                                       GAP_PHYS_PREFER_2M_BIT, 0);
                            rsp.flag |= THROUGHPUT_FLAG_2M_PHY;
                        }

                        /* Get actual connection interval after update */
                        le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &rsp.interval, conn_id);

                        /* Send configuration response via CT_TX */
                        le_throughput_send_ct_tx_data(conn_handle, cid,
                                                      (uint8_t *)&rsp, sizeof(rsp));

                        APP_PRINT_INFO2("app_le_throughput: CONFIG RSP status=%d, interval=%d",
                                        rsp.status, rsp.interval);

                        /* --- Start TX test if the config includes a TX direction --- */
                        if ((opcode == LE_OPCODE_CONFIG_TX_PARAMS) ||
                            (opcode == LE_OPCODE_CONFIG_RXTX_PARAMS))
                        {
                            /* Stop any previously running test first */
                            le_throughput_stop_test();

                            /* Determine test mode */
                            T_THROUGHPUT_TEST_MODE test_mode = THROUGHPUT_MODE_TX_TEST;
                            if (opcode == LE_OPCODE_CONFIG_RXTX_PARAMS)
                            {
                                test_mode = THROUGHPUT_MODE_RXTX_TEST;
                            }

                            /* Populate test context */
                            s_test_ctx.active          = true;
                            s_test_ctx.mode            = test_mode;
                            s_test_ctx.conn_handle     = conn_handle;
                            s_test_ctx.cid             = cid;
                            s_test_ctx.conn_id         = conn_id;
                            s_test_ctx.packet_len      = p_req->packet_len;
                            s_test_ctx.duration        = p_req->duration;
                            s_test_ctx.total_bytes_sent     = 0;
                            s_test_ctx.total_packets_sent   = 0;
                            s_test_ctx.total_bytes_received   = 0;
                            s_test_ctx.total_packets_received = 0;

                            /* Get current ATT MTU size */
                            le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE,
                                              &s_test_ctx.mtu_size, conn_id);

                            /* Create and start the duration timer (one-shot) */
                            if (s_test_ctx.timer_handle == NULL)
                            {
                                os_timer_create(&s_test_ctx.timer_handle,
                                                "throughput_timer",
                                                0,
                                                p_req->duration * 1000,
                                                false,          /* one-shot */
                                                app_le_throughput_timer_cb);
                            }
                            os_timer_start(&s_test_ctx.timer_handle);

                            APP_PRINT_INFO5("le_throughput: TX test started mode=%d, dur=%ds, "
                                            "mtu=%d, pkt_len=%d, conn_id=%d",
                                            test_mode, p_req->duration,
                                            s_test_ctx.mtu_size, p_req->packet_len, conn_id);

                            /* Kick off the test via eventbus */
                            event_bus_publish(THROUGHPUT_EVENT_TEST_START, NULL, 0);
                        }
                        else if (opcode == LE_OPCODE_CONFIG_RX_PARAMS)
                        {
                            /* RX-only test: just update connection parameters and wait for incoming data */
                            APP_PRINT_INFO0("app_le_throughput: RX-only test configured, waiting for data...");
                            T_THROUGHPUT_TEST_MODE test_mode = THROUGHPUT_MODE_RX_TEST;

                            s_test_ctx.mode            = test_mode;
                            s_test_ctx.conn_handle     = conn_handle;
                            s_test_ctx.cid             = cid;
                            s_test_ctx.conn_id         = conn_id;
                            s_test_ctx.packet_len      = p_req->packet_len;
                            s_test_ctx.duration        = p_req->duration;
                            s_test_ctx.total_bytes_sent     = 0;
                            s_test_ctx.total_packets_sent   = 0;
                            s_test_ctx.total_bytes_received   = 0;
                            s_test_ctx.total_packets_received = 0;
                        }
                    }
                    break;

                case LE_OPCODE_TEST_COMPLETE:
                    {
                        APP_PRINT_INFO0("app_le_throughput: TEST_COMPLETE from Phone");
                        /* Stop our local test if still running */
                        le_throughput_stop_test();
                    }
                    break;

                case LE_OPCODE_GET_TEST_REPORT:
                    {
                        T_TEST_REPORT_RESPONSE rsp;
                        rsp.opcode = LE_OPCODE_GET_TEST_REPORT_RSP;
                        rsp.tx_rate = 0;
                        rsp.rx_rate = 0;

                        if (s_test_ctx.duration > 0)
                        {
                            /* Calculate TX rate in bytes per second */
                            rsp.tx_rate = s_test_ctx.total_bytes_sent / s_test_ctx.duration;

                            /* Calculate RX rate in bytes per second */
                            rsp.rx_rate = s_test_ctx.total_bytes_received / s_test_ctx.duration;
                        }

                        le_throughput_send_ct_tx_data(conn_handle, cid,
                                                      (uint8_t *)&rsp, sizeof(rsp));
                        APP_PRINT_INFO4("app_le_throughput: GET_TEST_REPORT rsp "
                                        "tx_rate=%lu (tx_bytes=%lu), rx_rate=%lu (rx_bytes=%lu)",
                                        (unsigned long)rsp.tx_rate,
                                        (unsigned long)s_test_ctx.total_bytes_sent,
                                        (unsigned long)rsp.rx_rate,
                                        (unsigned long)s_test_ctx.total_bytes_received);
                    }
                    break;

                default:
                    APP_PRINT_WARN1("app_le_throughput: unknown opcode 0x%02x", opcode);
                    break;
                }
            }
            else if (p_cb_data->attr_index == LE_THROUGHPUT_DT_RX_VALUE_INDEX)
            {
                /* Data channel (DT_RX) - test data from Phone */
                APP_PRINT_INFO2("app_le_throughput: DT_RX data len=%d, conn_id=%d",
                                len, conn_id);

                /* Accumulate RX statistics */
                s_test_ctx.total_bytes_received += len;
                s_test_ctx.total_packets_received++;
            }
        }
        break;

    case GATT_MSG_LE_THROUGHPUT_CT_TX_CCCD:
        {
            APP_PRINT_INFO1("app_le_throughput: CT_TX CCCD update, val=%d",
                            p_cb_data->msg_data.notification_indification_value);
        }
        break;

    case GATT_MSG_LE_THROUGHPUT_DT_TX_CCCD:
        {
            APP_PRINT_INFO1("app_le_throughput: DT_TX CCCD update, val=%d",
                            p_cb_data->msg_data.notification_indification_value);
        }
        break;

    default:
        break;
    }

    return app_result;
}

/********************************************************************************************************
 * Public API
 ********************************************************************************************************/

void app_le_throughput_init(void)
{
    /* Register the GATT service with both the application callback and
     * the send-data-complete callback for credit-based flow control.
     */
    le_throughput_gatt_srv_id = le_throughput_reg_srv(app_le_throughput_gatt_svc_callback,
                                                      app_le_throughput_send_data_cb);

    /* Register eventbus topics */
    event_bus_topic_register(THROUGHPUT_EVENT_TEST_START);
    event_bus_topic_register(THROUGHPUT_EVENT_SEND_NEXT);

    /* Subscribe to throughput test events */
    event_bus_subscribe(&s_test_ctx.start_handle,
                        THROUGHPUT_EVENT_TEST_START,
                        app_le_throughput_test_start_handler);

    event_bus_subscribe(&s_test_ctx.send_handle,
                        THROUGHPUT_EVENT_SEND_NEXT,
                        app_le_throughput_send_next_handler);

    APP_PRINT_INFO1("app_le_throughput: init done, srv_id=%d", le_throughput_gatt_srv_id);
}
APP_MODULE_INIT(app_le_throughput_init);
APP_BLE_SERVICE_INFO(le_throughput, 1);
