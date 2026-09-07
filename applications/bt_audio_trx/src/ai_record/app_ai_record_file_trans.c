/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */



#if CONFIG_REALTEK_APP_AI_RECORD

/*============================================================================*
 *                              Header Files
 *============================================================================*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "trace.h"
#include "app_main.h"
#include "app_cfg.h"
#include "app_timer.h"
#include "gap.h"
#include "gap_conn_le.h"
#include "app_fs_if.h"
#include "fs_if.h"
#include "app_ai_record_service.h"
#include "app_ai_record_file_trans.h"
#include "app_bt_policy_api.h"
#include "app_flags.h"
#if CONFIG_REALTEK_APP_RTC_CALENDAR_SUPPORT
#include "app_ai_record_rtc.h"
#endif

#if (F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD)
#include "wifi_transport.h"
#endif

#if F_APP_WIFI_UART_CMD
#include <os_sched.h>           /* os_delay() - bring-up settle wait before ATPN */
#include "wifi/app_uart_atcmd.h"
#include "wifi/app_wifi_uart.h" /* app_wifi_uart_module_is_ready() - COMMAND READY gate */
#include "wifi/wifi_app.h"      /* wifi_task_ensure(), wifi_power_on(), wifi task msg */
#include "app_ai_record.h"      /* app_ai_record_wifi_power_on() */
#include "wifi/wifi_file_upload.h"  /* wifi_file_upload_is_ready() - WiFi module guard */

/* Gate flag (defined in app_cmd.c) that the WiFi UART RX handler checks
 * before forwarding module data to the AT parser. It must be set to 1 once
 * the module is powered, otherwise every AT response is discarded as
 * "unknown uart_mode" and connect commands silently time out. */
extern uint8_t wifi_enable_flag;

#elif F_APP_WIFI_SPI_CMD
#include "app_spi_atcmd.h"   /* app_spi_atcmd_register_callback() */
#include "spi_file_upload.h"  /* spi_file_upload_is_ready() - SPI WiFi module guard */
#include "app_spi_api.h"     /* SPI_XMIT_SIZE - SPI transport xfer size */
#endif

#include "app_dlps.h"
#include "pm.h"
#include "clk_mgr.h"

/*============================================================================*
 *                              Tunables
 *============================================================================*/

#define AI_REC_PROTOCOL_VERSION         1

#define AI_REC_BREDR_FIXED_MTU          512

/** @brief Spec ATT MTU floor (BLE 4.0 default before MTU exchange). */
#define AI_REC_MTU_SPEC_MIN             23

/** @brief ATT header overhead (opcode + handle) that the BLE stack adds
 *         internally to each gatt_svc_send_data() call.  The attribute value
 *         (p_data) passed to gatt_svc_send_data must therefore be limited to
 *         (ATT_MTU - ATT_HEADER_SIZE) bytes so the full ATT PDU fits in one
 *         LL packet without fragmentation. */
#define ATT_HEADER_SIZE                 3

/** @brief Smallest MTU we are willing to start an UPLOAD on.
 *
 *  Computed so the audio payload per chunk is at least 32 bytes after
 *  subtracting all framing overhead (ATT header + protocol framing).
 *  Below this threshold the per-packet overhead becomes ridiculous
 *  (e.g. MTU=23 => 4B audio/packet) and the transfer would crawl with
 *  no error indication. We refuse to start instead of pretending it'll
 *  work.
 *
 *  Effective audio per chunk = (MTU - ATT_HEADER_SIZE) - AI_REC_UPLOAD_HDR_TOTAL
 *  With MTU = 52: (52 - 3) - 17 = 32. */
#define AI_REC_MIN_USEFUL_MTU           (AI_REC_UPLOAD_HDR_TOTAL + ATT_HEADER_SIZE + 32)

/** @brief Watchdog: abort if no data activity for this long during
 *         a transfer. */
#define AI_REC_TRANS_WATCHDOG_MS        10000

/** @brief Live tail-follow poll cadence while caught up and waiting for
 *         the recorder to append more audio. Slower than the active
 *         streaming rate to avoid busy-spinning on an idle window. */
#define AI_REC_LIVE_POLL_INTERVAL_MS    500

/** @brief CRC pre-compute scratch (must be even-length so chunked XOR
 *         pair-alignment matches a single-pass result). */
#define AI_REC_CRC_SCRATCH_BYTES        512

/** @brief Preferred connection params for fast upload (units: 1.25 ms). */
#define AI_REC_CONN_INTERVAL_MIN        0x06   /* 7.5  ms */
#define AI_REC_CONN_INTERVAL_MAX        0x0C   /* 15   ms */
#define AI_REC_CONN_LATENCY             0
#define AI_REC_CONN_SUPERVISION_TIMEOUT 500    /* 5 s */

/** @brief Poll cadence (ms) while waiting for a connection parameter
 *         update to complete before sending the first data chunk.
 *
 *  After ble_set_prefer_conn_param() the actual LL connection update
 *  procedure may take several connection events.  We poll at this rate,
 *  checking le_get_conn_param(GAP_PARAM_CONN_INTERVAL), and only start
 *  the full-speed push once the interval settles inside the requested
 *  range.  The upload's 10 s watchdog guards against indefinite waits
 *  (e.g. phone rejects the update). */
#define AI_REC_CONN_UPDATE_POLL_MS      50

/*============================================================================*
 *                              Types
 *============================================================================*/

typedef enum
{
    AI_REC_TIMER_POLL              = 0x00,
    AI_REC_TIMER_TRANS_WATCHDOG    = 0x01,
} T_AI_REC_TRANS_TIMER;

typedef struct
{
    /* Link */
    uint8_t              conn_id;
    uint16_t             conn_handle;
    uint16_t             cid;
    T_GAP_CHANN_TYPE     chann_type;
    bool                 notify_enabled;

    /* Spec'd protocol state machine */
    T_AI_REC_TRANS_STATE state;

    /* Active upload state */
    T_FILE_HANDLE       *fs_handle;
    uint32_t             file_total_length;  /**< whole file size (incl. start_offset) */
    uint32_t             start_offset;       /**< resume base */
    uint32_t             cur_offset;         /**< absolute offset of NEXT byte to send */
    uint16_t             local_seq;
    uint16_t             whole_file_crc;     /**< pre-computed in OPEN */
    T_AI_REC_FILE_FORMAT file_format;
    uint32_t             file_create_ts;     /**< unix epoch, 0=unknown */

    /* MTU + tool-side cap */
    uint32_t             host_max_chunk;     /**< Tool-requested max chunk bytes */
    uint16_t             chunk_payload_max;  /**< effective audio bytes / chunk */

    /* Buffers */
    uint8_t             *p_chunk_buf;        /**< whole notify packet incl. headers */
    uint16_t             chunk_buf_size;     /**< == MTU */

    /* Recording mutex */
    bool                 is_recording;

    /* Filename currently held open by the recording pipeline. Empty
     * string ('\0' at index 0) when recording is idle. Used by
     * handle_upload_file to refuse same-file uploads. */
    char                 active_record_filename[AI_REC_FILENAME_MAX + 1];

    /* Live tail-follow state (live upload of the file still being
     * recorded). See the live-tail design doc. live_filename is a
     * separate copy from active_record_filename because the latter is
     * cleared the moment recording stops, but we must keep re-opening
     * this file by name to follow its growth and drain its tail. */
    bool                 live_mode;            /**< this upload follows a growing file */
    bool                 recording_finalized;  /**< writer stopped; size is now final */
    char                 live_filename[AI_REC_FILENAME_MAX + 1];  /**< reopen target */

    /* Protocol handshake flag.
     *
     *  Set to true when the host has successfully issued CMD_QUERY_INFO
     *  on the current BLE connection. CMD_UPLOAD_FILE is refused until
     *  this is true so the host always learns packet_size, protocol_ver
     *  and transport_cap before streaming begins.
     *
     *  Cleared on:
     *    - module init (memset above)
     *    - notify CCCD disable / disconnect (new BLE session must
     *      re-handshake)
     *    - explicit cancel paths */
    bool                 query_info_received;

    /* WiFi state (spec V4.1 x7).
     *
     *  Updated synchronously by the three CMD_WIFI_* handlers (entering
     *  CONNECTING / DISCONNECTING) and asynchronously by the AT-cmd
     *  callback (advancing to CONNECTED / DISCONNECTED / FAIL). All four
     *  fields zeroed by the memset of ai_rec_trans at module init -
     *  WIFI_STATE_DISCONNECTED is 0x00 by design. wifi_ssid is kept for
     *  EVT_WIFI_GET_STATUS body. wifi_ip stored in host order (BE on
     *  wire, see send_wifi_connect_evt). */
    T_AI_REC_WIFI_STATE  wifi_state;
    char                 wifi_ssid[AI_REC_WIFI_SSID_MAX + 1];
    uint32_t             wifi_ip;
    int8_t               wifi_rssi;
    /* True while a CMD_WIFI_CONNECT is in flight and the host has not yet
     * been given a connect-result EVT. Lets a disconnect that interrupts the
     * connect emit connect-fail before the disconnect EVT. */
    bool                 wifi_connect_pending;

    /** @brief Defer data-chunk push until the fast-conn-param update
     *         (requested by ai_rec_trans_apply_fast_conn_params())
     *         completes.
     *
     *  Set true in apply_fast_conn_params() so push_chunks_burst()
     *  polls the connection interval via le_get_conn_param() instead
     *  of sending data. Cleared (and normal streaming begins) once
     *  le_get_conn_param(GAP_PARAM_CONN_INTERVAL) returns a value
     *  within [AI_REC_CONN_INTERVAL_MIN, AI_REC_CONN_INTERVAL_MAX].
     *
     *  Rationale: the BLE controller can silently drop or stall a
     *  GATT notification queued while a connection parameter update
     *  is in progress, producing a SEND_DATA_COMPLETE that never fires
     *  and deadlocking the send-one-at-a-time pacing. By deferring
     *  data chunks until the update settles we avoid the problematic
     *  window entirely.
     *
     *  Zeroed in close_and_reset(). */
    bool                 conn_update_pending;

    /** @brief SPI TCP server setup progress (8773GTP only).
     *
     *  After GOT_IP, the external Wi-Fi IC needs explicit AT+SKTCFG and
     *  AT+SKTSERVER commands before it can accept TCP connections from the
     *  phone.  This counter tracks those setup steps:
     *    0 = waiting for SKTCFG OK
     *    1 = waiting for SKTSERVER OK
     *    2 = TCP server ready; EVT_WIFI_CONNECT may be sent.
     *
     *  Zeroed by memset at module init and in close_and_reset(). */
    uint8_t              spi_tcp_setup_step;
} T_AI_REC_TRANS_DB;

/*============================================================================*
 *                              Variables
 *============================================================================*/

static T_AI_REC_TRANS_DB ai_rec_trans = {0};

static uint8_t timer_id_poll           = 0;
static uint8_t timer_id_trans_watchdog = 0;
static uint8_t ai_rec_trans_timer_id   = 0;

/* clk_mgr user for boosting CPU + SPIC0 during file upload. */
static T_CLK_USER_HANDLE clk_mgr_upload_handle = NULL;

#if F_APP_WIFI_UART_CMD
/* The module is powered on the first connect and then left on, so the cold
 * bring-up is only paid once. wifi_pending_connect_param holds the
 * "<ssid>,<password>" AT payload until the WiFi task finishes bring-up and
 * issues the deferred ATPN. */
static bool    wifi_powered_on         = false;
static char    wifi_pending_connect_param[AI_REC_WIFI_SSID_MAX + AI_REC_WIFI_PASS_MAX + 4];
#endif

/*============================================================================*
 *                              Forward decls
 *============================================================================*/

static void ai_rec_trans_send_notify_raw(uint16_t evt_id,
                                         const uint8_t *data, uint16_t data_len);
static void ai_rec_trans_send_simple_resp(uint16_t evt_id, uint8_t result);
static void ai_rec_trans_send_upload_error(T_AI_REC_UPLOAD_ERR err);
static uint16_t ai_rec_trans_get_mtu(void);
static void ai_rec_trans_close_and_reset(T_AI_REC_TRANS_STATE final_state);
static void ai_rec_trans_arm_watchdog(void);
static void ai_rec_trans_disarm_watchdog(void);
static void ai_rec_trans_arm_timer_ms(uint32_t interval_ms);
static bool ai_rec_live_reopen_refresh(void);
static void ai_rec_trans_send_batch(void);
static void ai_rec_trans_timeout_cb(uint8_t timer_evt, uint16_t param);
static void ai_rec_trans_handle_event_ack(uint8_t *p, uint16_t plen);

/* Externally visible - called from app_ble_service when a GATT
 * notification send completes for the record-trans service. */
void ai_rec_trans_notify_send_complete(uint16_t service_id);

/*============================================================================*
 *                              CRC helpers (pair-XOR; spec uses CRC16)
 *============================================================================*/

/**
 * @brief Pair-XOR running checksum. Output matches a single-pass call
 *        as long as every chunk except the LAST is even-sized - we
 *        guarantee this by reading exact AI_REC_CRC_SCRATCH_BYTES.
 *
 *        Note: this is the same byte-pair XOR scheme used per-chunk in
 *        the on-wire data frames; the host re-implements it identically.
 */
static uint16_t ai_rec_crc16_step(uint16_t prev, const uint8_t *buf, uint32_t len)
{
    uint16_t cs = prev;
    const uint16_t *p16 = (const uint16_t *)buf;
    for (uint32_t i = 0; i < len / 2; ++i)
    {
        cs ^= p16[i];
    }
    if (len & 1)
    {
        cs ^= ((uint16_t)buf[len - 1]) << 8;
    }
    return cs;
}

/** @brief Final swap to big-endian for on-wire transport. */
static inline uint16_t ai_rec_crc16_finalize(uint16_t cs)
{
    return ((cs & 0xff) << 8) | ((cs & 0xff00) >> 8);
}

/** @brief One-pass per-chunk CRC for outbound data frames. */
static uint16_t ai_rec_crc16_chunk(const uint8_t *buf, uint32_t length)
{
    return ai_rec_crc16_finalize(ai_rec_crc16_step(0, buf, length));
}

/**
 * @brief  Compute whole-file CRC by streaming reads.
 *
 *         Leaves the file pointer at start_offset on return (regardless
 *         of where it was when we started). Needs a malloc'd scratch
 *         buffer; returns 0 with errno-style failure flag if alloc fails.
 *
 *         Cost: ~one extra full-file read pass before transmission begins.
 *         For a 100 MB file at ~5 MB/s effective SD throughput this is
 *         ~20 s - acceptable since the host is just waiting on the
 *         start frame anyway.
 *
 * @return false on read / malloc error.
 */
static bool ai_rec_compute_whole_file_crc(T_FILE_HANDLE *handle,
                                          uint32_t       seek_to_after,
                                          uint16_t      *out_crc)
{
    if (handle == NULL || out_crc == NULL)
    {
        return false;
    }

    uint8_t *scratch = malloc(AI_REC_CRC_SCRATCH_BYTES);
    if (scratch == NULL)
    {
        APP_PRINT_ERROR0("ai_rec_compute_whole_file_crc: malloc fail");
        return false;
    }

    if (app_fs_seek(handle, 0) != 0)
    {
        free(scratch);
        return false;
    }

    uint16_t cs = 0;
    while (true)
    {
        int rd = app_fs_read(handle, scratch, AI_REC_CRC_SCRATCH_BYTES);
        if (rd < 0)
        {
            free(scratch);
            return false;
        }
        if (rd == 0)
        {
            break;
        }
        cs = ai_rec_crc16_step(cs, scratch, (uint32_t)rd);
        if ((uint32_t)rd < AI_REC_CRC_SCRATCH_BYTES)
        {
            /* short read at EOF; loop will exit on next iteration */
        }
    }

    free(scratch);

    *out_crc = ai_rec_crc16_finalize(cs);

    /* Re-seek to the resume position the caller intends to start streaming
     * from. If this seek fails, the caller must abort. */
    if (app_fs_seek(handle, seek_to_after) != 0)
    {
        return false;
    }
    return true;
}

/*============================================================================*
 *                              Outer-packet helpers  (SINC/Seqn/LEN)
 *============================================================================*/

/** @brief Outgoing packet sequence number, increments per Notify.
 *         Range 1..255; wraps back to 1 after 255 (skips 0 by spec). */
static uint8_t g_pkt_tx_seqn = 0;

static uint8_t ai_rec_pkt_next_seqn(void)
{
    uint8_t s = (uint8_t)(g_pkt_tx_seqn + 1);
    if (s == 0)
    {
        s = 1;  /* wrap 0xFF => 0 => 1 (skip 0) */
    }
    g_pkt_tx_seqn = s;
    return s;
}



/*============================================================================*
 *                              Push-rate auto-tuning  (PHY + MTU aware)
 *============================================================================*/

/** @brief Sustainable LL packets/sec by PHY (251-byte payload, with ACK+IFS). */
#define AI_REC_LL_PER_SEC_1M            400
#define AI_REC_LL_PER_SEC_2M            900

/** @brief LE LL data PDU max payload (BLE 4.2+ DLE). */
#define AI_REC_LL_MAX_PAYLOAD           251
#define AI_REC_L2CAP_HDR_LEN            4

/** @brief Push-rate utilisation target: leave 10 % headroom for retx/jitter. */
#define AI_REC_PUSH_UTIL_PCT            90

/** @brief Hard caps on chosen knobs to keep timer behavior sane. */
#define AI_REC_PUSH_INTERVAL_MIN_MS     2
#define AI_REC_PUSH_INTERVAL_MAX_MS     50
#define AI_REC_PUSH_BURST_MIN           1
#define AI_REC_PUSH_BURST_MAX           8

/**
 * @brief Log estimated throughput based on current MTU for diagnostics.
 *        This is informational only - pacing is driven by LE credits
 *        via le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS), not by
 *        a computed timer interval.
 */
static void ai_rec_trans_log_throughput_est(void)
{
    uint16_t mtu = ai_rec_trans_get_mtu();
    uint16_t per_pkt = (mtu > (uint16_t)(AI_REC_UPLOAD_HDR_TOTAL + ATT_HEADER_SIZE)) ?
                       (uint16_t)((mtu - ATT_HEADER_SIZE) - AI_REC_UPLOAD_HDR_TOTAL) : 0;
    APP_PRINT_INFO2("throughput: mtu=%d payload/chunk=%d",
                    mtu, per_pkt);
}

/*============================================================================*
 *                              Notify helpers
 *============================================================================*/

/**
 * @brief Resolve the current ATT MTU for the active link.
 *
 *  Defensive contract:
 *    - Default to spec floor (23) before any branch runs, so an
 *      uninitialized chann_type gives a safe small value rather than
 *      a misleading "247" - callers must verify with AI_REC_MIN_USEFUL_MTU.
 *    - For LE: query the SDK with a *zero-initialized* output and only
 *      promote `mtu` if the returned value is sane (>= spec floor).
 *      This sidesteps any uncertainty about whether le_get_conn_param
 *      writes the output on failure.
 *    - For BR/EDR over GATT: fall back to the configured fixed value.
 *      TODO: replace with a real bt_gatt_get_mtu() lookup when
 *      CONFIG_RECORD_TRANS_GATT_OVER_BREDR is exercised.
 *    - Each branch has a single log line so trace files can pin down
 *      surprising MTU values quickly.
 */
static uint16_t ai_rec_trans_get_mtu(void)
{
    uint16_t mtu = AI_REC_MTU_SPEC_MIN;

    if (ai_rec_trans.chann_type == GAP_CHANN_TYPE_LE_ATT ||
        ai_rec_trans.chann_type == GAP_CHANN_TYPE_LE_ECFC)
    {
        uint16_t got = 0;
        le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE, &got, ai_rec_trans.conn_id);

        if (got >= AI_REC_MTU_SPEC_MIN)
        {
            mtu = got;
            APP_PRINT_INFO2("get_mtu: LE conn_id=%d mtu=%d",
                            ai_rec_trans.conn_id, mtu);
        }
        else
        {
            APP_PRINT_WARN2("get_mtu: le_get_conn_param returned bad mtu=%d "
                            "(conn_id=%d); fallback to spec floor 23",
                            got, ai_rec_trans.conn_id);
        }
    }
    else if (ai_rec_trans.chann_type == GAP_CHANN_TYPE_BREDR_ATT ||
             ai_rec_trans.chann_type == GAP_CHANN_TYPE_BREDR_ECFC)
    {
        mtu = AI_REC_BREDR_FIXED_MTU;
        APP_PRINT_INFO1("get_mtu: BR/EDR fixed mtu=%d (TODO: query stack)",
                        mtu);
    }
    else
    {
        APP_PRINT_WARN1("get_mtu: chann_type=%d is neither LE nor BR/EDR; "
                        "returning spec floor 23",
                        ai_rec_trans.chann_type);
    }

    /* Defense-in-depth: even if the branches above produced something
     * below the floor, do not return a sub-spec MTU. */
    if (mtu < AI_REC_MTU_SPEC_MIN)
    {
        mtu = AI_REC_MTU_SPEC_MIN;
    }
    return mtu;
}

/**
 * @brief  Send a notify packet with the full outer framing.
 *
 *         Wire:
 *          [SINC(1)] [Seqn(1)] [LEN(2)] [op_code(2)] [body(body_len)]
 *
 *         LEN = op_code(2) + body_len.
 */
static void ai_rec_trans_send_notify_raw(uint16_t op_code,
                                         const uint8_t *body, uint16_t body_len)
{
    uint16_t length = (uint16_t)(PKT_OP_CODE_LEN + body_len);   /* "LENGTH" field value */
    uint16_t total  = (uint16_t)(PKT_HEADER_LEN + length);

    uint8_t *buf = malloc(total);
    if (buf == NULL)
    {
        APP_PRINT_ERROR1("send_notify_raw: malloc fail, len=%d", total);
        return;
    }

    /* Outer header */
    buf[0] = PKT_SINC_WORD;
    buf[1] = ai_rec_pkt_next_seqn();
    buf[2] = (uint8_t)(length & 0xFF);
    buf[3] = (uint8_t)((length >> 8) & 0xFF);

    /* op_code */
    buf[4] = (uint8_t)(op_code & 0xFF);
    buf[5] = (uint8_t)((op_code >> 8) & 0xFF);

    /* parameters */
    if (body && body_len)
    {
        memcpy(buf + AI_REC_NOTIFY_HDR_LEN, body, body_len);
    }

    record_trans_service_send_notification(ai_rec_trans.conn_handle, ai_rec_trans.cid,
                                           buf, total);
    free(buf);
}

/** @brief [evt_id(2)][result(1)] for auxiliary cmds. */
static void ai_rec_trans_send_simple_resp(uint16_t evt_id, uint8_t result)
{
    uint8_t b = result;
    ai_rec_trans_send_notify_raw(evt_id, &b, 1);
}

/** @brief Send EVT_UPLOAD_FILE Flag=0xFE error frame. */
static void ai_rec_trans_send_upload_error(T_AI_REC_UPLOAD_ERR err)
{
    uint8_t body[2];
    body[0] = (uint8_t)UPLOAD_FLAG_ERROR;
    body[1] = (uint8_t)err;
    ai_rec_trans_send_notify_raw(EVT_UPLOAD_FILE, body, sizeof(body));
}

/** @brief Send EVT_UPLOAD_FILE Flag=0x00 start frame.
 *
 *         Layout (Byte0..Byte11): flag, crc(2), total_len(4),
 *                                 format(1), timestamp(4) = 12 bytes
 */
static void ai_rec_trans_send_upload_start(uint16_t crc, uint32_t total_len,
                                           uint8_t format, uint32_t timestamp)
{
    uint8_t body[12];
    body[0]  = (uint8_t)UPLOAD_FLAG_START;
    body[1]  = (uint8_t)(crc & 0xFF);
    body[2]  = (uint8_t)((crc >> 8) & 0xFF);
    body[3]  = (uint8_t)(total_len & 0xFF);
    body[4]  = (uint8_t)((total_len >> 8) & 0xFF);
    body[5]  = (uint8_t)((total_len >> 16) & 0xFF);
    body[6]  = (uint8_t)((total_len >> 24) & 0xFF);
    body[7]  = format;
    body[8]  = (uint8_t)(timestamp & 0xFF);
    body[9]  = (uint8_t)((timestamp >> 8) & 0xFF);
    body[10] = (uint8_t)((timestamp >> 16) & 0xFF);
    body[11] = (uint8_t)((timestamp >> 24) & 0xFF);
    ai_rec_trans_send_notify_raw(EVT_UPLOAD_FILE, body, sizeof(body));
}

/*============================================================================*
 *                              File scan  (spec V4.1 x9)
 *
 *  CMD_SCAN_FILES walks the FS directly (not Header.bin) so it can be
 *  used for initial discovery / list rebuild. Two-pass over
 *  app_fs_if_list_files: pass 1 counts matches for the Start frame,
 *  pass 2 emits up to batch_cap entries before the End frame.
 *============================================================================*/

/* Forward decl: ai_rec_format_from_name is defined later in this file. */
static T_AI_REC_FILE_FORMAT ai_rec_format_from_name(const char *name);

/** @brief Map CMD_SCAN_FILES filter type byte to a list_files extension. */
static const char *ai_rec_scan_filter_to_ext(uint8_t filter_type)
{
    switch ((T_AI_REC_FILE_FORMAT)filter_type)
    {
    case AI_REC_FORMAT_MP3:
        return ".mp3";
    case AI_REC_FORMAT_MP4:
        return ".mp4";
    case AI_REC_FORMAT_RTK:
        return ".rtk";
    case AI_REC_FORMAT_AAC:
        return ".aac";
    case AI_REC_FORMAT_FLAC:
        return ".flac";
    case AI_REC_FORMAT_OPUS:
        return ".opus";
    case AI_REC_FORMAT_TXT:
        return ".txt";
    case AI_REC_FORMAT_DAT:
        return ".dat";
    case AI_REC_FORMAT_BIN:
        return ".bin";
    default:
        return NULL;   /* AI_REC_FORMAT_ALL or unsupported */
    }
}

/** @brief EVT_SCAN_FILES Flag=0x00 start frame. Body: [flag][total(4)]. */
static void ai_rec_trans_send_scan_start(uint32_t total)
{
    uint8_t body[5];
    body[0] = (uint8_t)SCAN_FLAG_START;
    body[1] = (uint8_t)(total & 0xFF);
    body[2] = (uint8_t)((total >> 8) & 0xFF);
    body[3] = (uint8_t)((total >> 16) & 0xFF);
    body[4] = (uint8_t)((total >> 24) & 0xFF);
    ai_rec_trans_send_notify_raw(EVT_SCAN_FILES, body, sizeof(body));
}

/** @brief EVT_SCAN_FILES Flag=0x02 end frame. Body: [flag][delivered(4)]. */
static void ai_rec_trans_send_scan_end(uint32_t delivered)
{
    uint8_t body[5];
    body[0] = (uint8_t)SCAN_FLAG_END;
    body[1] = (uint8_t)(delivered & 0xFF);
    body[2] = (uint8_t)((delivered >> 8) & 0xFF);
    body[3] = (uint8_t)((delivered >> 16) & 0xFF);
    body[4] = (uint8_t)((delivered >> 24) & 0xFF);
    ai_rec_trans_send_notify_raw(EVT_SCAN_FILES, body, sizeof(body));
}

/** @brief EVT_SCAN_FILES Flag=0xFE error frame. */
static void ai_rec_trans_send_scan_error(T_AI_REC_SCAN_ERR err)
{
    uint8_t body[2];
    body[0] = (uint8_t)SCAN_FLAG_ERROR;
    body[1] = (uint8_t)err;
    ai_rec_trans_send_notify_raw(EVT_SCAN_FILES, body, sizeof(body));
}

/**
 * @brief EVT_SCAN_FILES Flag=0x01 entry frame.
 *
 *  Body (all multi-byte fields LE on wire):
 *    flag(1) idx(2) format(1) storage(1) size(4) modify_ts(4)
 *    crc16(2) name_len(2) name(N)
 *  Total = 17 + N bytes.
 *
 *  modify_ts and crc16 are spec-optional; firmware passes 0 (unknown)
 *  until the FS layer exposes per-file timestamps and we choose to
 *  pre-compute file CRCs at scan cost.
 */
static void ai_rec_trans_send_scan_entry(uint16_t idx, uint8_t format,
                                         uint8_t storage, uint32_t size,
                                         uint32_t modify_ts, uint16_t crc16,
                                         const char *filename)
{
    uint16_t name_len = (uint16_t)strlen(filename);
    if (name_len > AI_REC_FILENAME_MAX)
    {
        name_len = AI_REC_FILENAME_MAX;
    }

    uint16_t body_len = 1 + 2 + 1 + 1 + 4 + 4 + 2 + 2 + name_len;
    uint8_t *body = malloc(body_len);
    if (body == NULL)
    {
        APP_PRINT_ERROR1("scan_entry: malloc fail len=%d", body_len);
        return;
    }

    uint8_t *q = body;
    *q++ = (uint8_t)SCAN_FLAG_ENTRY;
    *q++ = (uint8_t)(idx & 0xFF);
    *q++ = (uint8_t)((idx >> 8) & 0xFF);
    *q++ = format;
    *q++ = storage;
    *q++ = (uint8_t)(size & 0xFF);
    *q++ = (uint8_t)((size >> 8) & 0xFF);
    *q++ = (uint8_t)((size >> 16) & 0xFF);
    *q++ = (uint8_t)((size >> 24) & 0xFF);
    *q++ = (uint8_t)(modify_ts & 0xFF);
    *q++ = (uint8_t)((modify_ts >> 8) & 0xFF);
    *q++ = (uint8_t)((modify_ts >> 16) & 0xFF);
    *q++ = (uint8_t)((modify_ts >> 24) & 0xFF);
    *q++ = (uint8_t)(crc16 & 0xFF);
    *q++ = (uint8_t)((crc16 >> 8) & 0xFF);
    *q++ = (uint8_t)(name_len & 0xFF);
    *q++ = (uint8_t)((name_len >> 8) & 0xFF);
    memcpy(q, filename, name_len);

    ai_rec_trans_send_notify_raw(EVT_SCAN_FILES, body, body_len);
    free(body);
}

/* Two-pass scan contexts. */

typedef struct
{
    uint32_t count;
} T_AI_REC_SCAN_COUNT_CTX;

typedef struct
{
    uint16_t cur_idx;
    uint16_t max_emit;
    uint32_t emitted;
    uint8_t  storage_byte;  /* placed in each entry's storage field */
} T_AI_REC_SCAN_EMIT_CTX;

static bool ai_rec_scan_count_cb(const char *filename, uint32_t filesize, void *ctx)
{
    T_AI_REC_SCAN_COUNT_CTX *c = (T_AI_REC_SCAN_COUNT_CTX *)ctx;
    (void)filename;
    (void)filesize;
    c->count++;
    return true;
}

static bool ai_rec_scan_emit_cb(const char *filename, uint32_t filesize, void *ctx)
{
    T_AI_REC_SCAN_EMIT_CTX *c = (T_AI_REC_SCAN_EMIT_CTX *)ctx;
    if (c->emitted >= c->max_emit)
    {
        return false;  /* tell list_files to stop */
    }
    uint8_t fmt = (uint8_t)ai_rec_format_from_name(filename);
    ai_rec_trans_send_scan_entry(c->cur_idx, fmt, c->storage_byte,
                                 filesize, 0 /* ts unknown */,
                                 0 /* crc16 not pre-computed */,
                                 filename);
    c->cur_idx++;
    c->emitted++;
    return true;
}

/**
 * @brief CMD_SCAN_FILES handler.
 *
 *  Sequence: pass1 (count) => start frame => pass2 (emit <= batch_cap) => end.
 *  Errors short-circuit with a single Flag=0xFE frame; no start/end pair.
 */
static void ai_rec_trans_handle_scan_files(uint8_t *p, uint16_t plen)
{
    if (plen < 3)
    {
        ai_rec_trans_send_scan_error(SCAN_ERR_PERMISSION_DENIED);
        return;
    }

    uint8_t filter_type = p[0];
    uint8_t storage     = p[1];
    uint8_t batch_cap   = p[2];

    APP_PRINT_INFO3("scan_files: filter=0x%02x storage=0x%02x batch=%d",
                    filter_type, storage, batch_cap);

    /* This firmware only owns SD; reject Flash explicitly, accept SD/ALL. */
    if (storage == SCAN_STORAGE_FLASH)
    {
        ai_rec_trans_send_scan_error(SCAN_ERR_STORAGE_NOT_MOUNTED);
        return;
    }
    if (storage != SCAN_STORAGE_ALL && storage != SCAN_STORAGE_SD)
    {
        ai_rec_trans_send_scan_error(SCAN_ERR_PERMISSION_DENIED);
        return;
    }

    /* filter_type==0 (ALL) => ext=NULL => list_files iterates all files */
    const char *ext = ai_rec_scan_filter_to_ext(filter_type);

    /* ---- Pass 1: count total matches ---- */
    T_AI_REC_SCAN_COUNT_CTX ccnt = {0};
    app_fs_if_list_files(ext, ai_rec_scan_count_cb, &ccnt);

    ai_rec_trans_send_scan_start(ccnt.count);

    /* ---- Pass 2: emit up to batch_cap entries ---- */
    uint16_t cap = batch_cap;
    if (cap == 0)
    {
        cap = AI_REC_SCAN_DEFAULT_BATCH;
    }
    if (cap > AI_REC_SCAN_MAX_BATCH)
    {
        cap = AI_REC_SCAN_MAX_BATCH;
    }
    if ((uint32_t)cap > ccnt.count)
    {
        cap = (uint16_t)ccnt.count;
    }

    T_AI_REC_SCAN_EMIT_CTX cemit = {0};
    cemit.cur_idx      = 0;
    cemit.max_emit     = cap;
    cemit.storage_byte = (uint8_t)SCAN_STORAGE_SD;

    if (cap > 0)
    {
        app_fs_if_list_files(ext, ai_rec_scan_emit_cb, &cemit);
    }

    ai_rec_trans_send_scan_end(cemit.emitted);
}

/*============================================================================*
 *                              CMD_EVENT_ACK  (0x0000)
 *
 *  Host sends this to acknowledge a previously-received EVT notification.
 *  Parameters (plen == 3):
 *    Byte0..1  ack_evt_id   - EVT id being acknowledged (LE)
 *    Byte2     result       - 0x00 = OK, non-zero = error from host
 *
 *  The device does NOT send a response; this is a fire-and-forget ACK.
 *  If plen < 3 we silently ignore (the host may send a shorter ACK in
 *  future protocol revisions).
 *============================================================================*/

static void ai_rec_trans_handle_event_ack(uint8_t *p, uint16_t plen)
{
    uint16_t ack_evt_id = 0;
    uint8_t  result     = 0;

    if (plen >= 3)
    {
        ack_evt_id = (uint16_t)p[0] | ((uint16_t)p[1] << 8);
        result     = p[2];
    }

    APP_PRINT_INFO3("event_ack: evt_id=0x%04x result=0x%02x plen=%d",
                    ack_evt_id, result, plen);
}

/*============================================================================*
 *                              CMD_QUERY_INFO  (spec V4.1)
 *
 *  Same role as PLAYBACK's CMD_PLAYBACK_QUERY_INFO. Mirrors the filling
 *  pattern of app_playback_trans_get_device_info() but:
 *    - This firmware is BLE-only => transport_cap = BLE bit only,
 *      active_transport = BLE.
 *    - packet_size is derived from the negotiated ATT MTU at this very
 *      moment (le_get_conn_param GAP_PARAM_CONN_MTU_SIZE), minus the
 *      per-chunk framing overhead, so the host can size its receive
 *      buffer correctly.
 *    - Recording-pen is single-bud only => mode = 0x00.
 *    - song_format_type advertises Opus (recorder writes .opus files).
 *
 *  The 32-byte response easily fits in a single Notify under any sane
 *  MTU (>= 40 bytes wire), so we reuse send_notify_raw - which already
 *  handles outer framing (SINC/Seqn/LENGTH) - instead of the
 *  multi-notify "prepare" splitter that PLAYBACK uses for oversized
 *  list payloads.
 *============================================================================*/

/**
 * @brief Fill a T_QUERY_INFO_EVT struct with the current device snapshot.
 *
 *        MTU is sampled at call time so the value reflects whatever the
 *        host most recently negotiated. Caller (handle_query_info)
 *        sends it immediately, before the link can re-negotiate.
 */
static void ai_rec_trans_fill_device_info(T_QUERY_INFO_EVT *info)
{
    if (info == NULL)
    {
        return;
    }
    memset(info, 0, sizeof(*info));

    /* MTU negotiation: sample whatever the host most recently agreed on.
     * For LE this is le_get_conn_param(GAP_PARAM_CONN_MTU_SIZE);
     * BR/EDR-over-GATT is fixed-MTU. The helper centralizes both cases. */
    uint16_t mtu = ai_rec_trans_get_mtu();

    /* Effective audio bytes per upload chunk =
     *   MTU - AI_REC_UPLOAD_HDR_TOTAL (outer + op_code + chunk hdr = 17)
     * Clamp to 0 if MTU is too small to carry any audio. */
    uint16_t per_pkt = 0;
    if (mtu > AI_REC_UPLOAD_HDR_TOTAL)
    {
        per_pkt = (uint16_t)((mtu - ATT_HEADER_SIZE) - AI_REC_UPLOAD_HDR_TOTAL);
    }

    info->packet_size       = per_pkt;
    info->buffer_check_size = (uint16_t)(per_pkt * 3);   /* hint: 3 packets worth */
    info->protocol_ver      = AI_REC_PROTOCOL_VER_V41;
    info->mode              = 0x00;                       /* single-bud, no RWS */
    info->ic_type           = IC_TYPE_REC_PEN;
    info->song_format_type  = QUERY_FMT_OPUS;             /* recorder writes .opus */
    info->transport_cap     = QUERY_TRANSPORT_CAP_BLE | QUERY_TRANSPORT_CAP_WIFI;   /* BLE + WiFi */
    info->active_transport  = QUERY_TRANSPORT_CAP_BLE |
                              QUERY_TRANSPORT_CAP_WIFI;   /* BLE + WiFi active */
    /* info->rsv[22] kept zeroed by memset above */
}

/**
 * @brief CMD_QUERY_INFO (0x680) handler. Builds the 32-byte device
 *        info snapshot and emits EVT_QUERY_INFO (0x680). Carries no
 *        request parameters per spec.
 */
static void ai_rec_trans_handle_query_info(uint8_t *p, uint16_t plen)
{
    (void)p;
    (void)plen;   /* CMD_QUERY_INFO has no parameters */

    T_QUERY_INFO_EVT info;
    ai_rec_trans_fill_device_info(&info);

    APP_PRINT_INFO5("query_info: pkt=%d buf_chk=%d ver=%d cap=0x%02x active=%d",
                    info.packet_size, info.buffer_check_size,
                    info.protocol_ver, info.transport_cap, info.active_transport);

    /* 32B + outer framing (4 hdr + 2 op_code = 6) = 38B wire
     * => fits any negotiated MTU we'd accept. Single Notify is fine. */
    ai_rec_trans_send_notify_raw(EVT_QUERY_INFO,
                                 (uint8_t *)&info, sizeof(info));

    /* Mark the per-session protocol handshake as complete. CMD_UPLOAD_FILE
     * is gated on this so the host never streams without first learning
     * packet_size / protocol_ver / transport_cap. Cleared on disconnect. */
    ai_rec_trans.query_info_received = true;
}

/*============================================================================*
 *                              CMD_WIFI_*  (spec V4.1 x7)
 *
 *  Three control-plane commands. Connect / disconnect map onto the
 *  external WiFi MCU's AT command queue (ATCMD_WLCONN / WLDISCONN in
 *  V1, ATCMD_ATPN / ATWD in V2). Status query is purely synchronous -
 *  reports whatever state the AT-callback last observed.
 *
 *  A single async callback (ai_rec_trans_wifi_atcmd_cb) registered with
 *  the AT module bridges its events into our spec EVT frames. Re-uses
 *  ai_rec_trans.conn_handle / cid for the notify path; this is sound
 *  because the firmware supports a single BLE link at a time.
 *============================================================================*/

/** @brief TCP port the WiFi module serves on. After a successful connect
 *         we auto-issue ATPS=0,<port> (protocol 0 = TCP) to make the module
 *         a TCP server, and report the same port in EVT_WIFI_CONNECT.port so
 *         the host opens its file-transfer socket to the right place. */
#define AI_REC_WIFI_TCP_PORT                5001

/* Stringify the port so the on-wire ATPS param can never drift from the
 * port reported to the host. The proven phone-driven demo path sends the
 * param WITH a trailing "\r\n" and cmd_send() appends ANOTHER "\r\n", so the
 * module actually receives a double terminator ("0,<port>\r\n\r\n"). The
 * single-terminator form this path used before got no response, so we now
 * replicate the proven byte sequence exactly by keeping the "\r\n" here. */
#define AI_REC_STR_(x)                      #x
#define AI_REC_STR(x)                       AI_REC_STR_(x)
#define AI_REC_WIFI_ATPS_PARAM              "0," AI_REC_STR(AI_REC_WIFI_TCP_PORT) "\r\n"

/** @brief Map our cmd path to the AT module's command-type enum. The
 *         AT module ships in two cmd-set versions (V1=AT+WLCONN style,
 *         V2=ATPN style) with different enum names for the same act. */
#if F_APP_WIFI_UART_CMD
#if (AT_CMD_VER == 1)
#define AI_REC_WIFI_AT_CMD_CONNECT          ATCMD_WLCONN
#define AI_REC_WIFI_AT_CMD_DISCONNECT       ATCMD_WLDISCONN
#else
#define AI_REC_WIFI_AT_CMD_CONNECT          ATCMD_ATPN
#define AI_REC_WIFI_AT_CMD_DISCONNECT       ATCMD_ATWD
#endif
#endif

/* ---- EVT senders ---------------------------------------------------------*/

/**
 * @brief Send EVT_WIFI_CONNECT (8B body).
 *
 *  Spec x7.2 marks IPv4 as big-endian. Port lacks an explicit annotation
 *  but pairs naturally with IP - we use BE for both (network order),
 *  diverging from this protocol's otherwise-LE convention. Document
 *  prominently so the host parser doesn't byte-swap accidentally.
 *
 *  ip_host: address in host order (high byte first as a uint32). Use 0
 *  for non-success results. RSSI is int8 dBm; 0 if unknown.
 */
static void ai_rec_trans_send_wifi_connect_evt(uint8_t result, uint32_t ip_host,
                                               uint16_t port, int8_t rssi)
{
    uint8_t body[8];
    body[0] = result;

    if (result == WIFI_RESULT_SUCCESS ||
        result == WIFI_RESULT_ALREADY_CONNECTED)
    {

        body[1] = (uint8_t)(ip_host >> 24);   /* IP big-endian */
        body[2] = (uint8_t)(ip_host >> 16);
        body[3] = (uint8_t)(ip_host >> 8);
        body[4] = (uint8_t)(ip_host);
        body[5] = (uint8_t)(port >> 8);       /* port big-endian */
        body[6] = (uint8_t)(port);
        body[7] = (uint8_t)rssi;
    }
    else
    {
        memset(body + 1, 0, 7);
    }

    ai_rec_trans_send_notify_raw(EVT_WIFI_CONNECT, body, sizeof(body));
}

/** @brief Send EVT_WIFI_DISCONNECT (1B body). */
static void ai_rec_trans_send_wifi_disconnect_evt(uint8_t result)
{
    ai_rec_trans_send_notify_raw(EVT_WIFI_DISCONNECT, &result, 1);
}

/**
 * @brief Send EVT_WIFI_GET_STATUS. Body length depends on state:
 *
 *    Connected:    state(1) + ssid_len(1) + ssid(N) + ip(4 BE) + rssi(1)
 *    Other states: state(1) + ssid_len=0(1) + ip(4 BE, all 0) + rssi(1, 0)
 */
static void ai_rec_trans_send_wifi_status_evt(void)
{
    uint8_t  body[1 + 1 + AI_REC_WIFI_SSID_MAX + 4 + 1];
    uint8_t *q = body;
    bool     connected = (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTED);

    *q++ = (uint8_t)ai_rec_trans.wifi_state;

    if (connected)
    {
        uint8_t nlen = (uint8_t)strlen(ai_rec_trans.wifi_ssid);
        if (nlen > AI_REC_WIFI_SSID_MAX)
        {
            nlen = AI_REC_WIFI_SSID_MAX;
        }
        *q++ = nlen;
        memcpy(q, ai_rec_trans.wifi_ssid, nlen);
        q += nlen;

        *q++ = (uint8_t)(ai_rec_trans.wifi_ip >> 24);   /* IP big-endian */
        *q++ = (uint8_t)(ai_rec_trans.wifi_ip >> 16);
        *q++ = (uint8_t)(ai_rec_trans.wifi_ip >> 8);
        *q++ = (uint8_t)(ai_rec_trans.wifi_ip);
        *q++ = (uint8_t)ai_rec_trans.wifi_rssi;
    }
    else
    {
        *q++ = 0;       /* ssid_len */
        memset(q, 0, 5);
        q += 5;         /* ip(4) + rssi(1) */
    }

    ai_rec_trans_send_notify_raw(EVT_WIFI_GET_STATUS, body,
                                 (uint16_t)(q - body));
}

/* ---- Async AT callback ---------------------------------------------------*/

#if (F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD)
/**
 * @brief Called when ATPN link-up is confirmed (ATPN OK or wifi got ip).
 *
 *        Stores the IP, moves to ATPS_PENDING, and issues the ATPS command
 *        to configure the TCP server. EVT_WIFI_CONNECT is NOT sent yet -
 *        it waits for the ATPS OK response (ai_rec_trans_wifi_atps_ok_evt).
 *
 *        ip_host is 0 when the IP is unknown (OK carries no address);
 *        the host re-polls via CMD_WIFI_GET_STATUS.
 */
static void ai_rec_trans_wifi_on_link_up(uint32_t ip_host)
{
    ai_rec_trans.wifi_ip    = ip_host;
    ai_rec_trans.wifi_rssi  = 0;   /* TODO: pull RSSI when AT exposes it */
    ai_rec_trans.wifi_state = WIFI_STATE_ATPS_PENDING;
    /* wifi_connect_pending stays true until ATPS OK fires. */

#if (AT_CMD_VER == 2)
    /* Configure the module as a TCP server on AI_REC_WIFI_TCP_PORT
     * so the host can open the file-transfer socket.
     * EVT_WIFI_CONNECT is deferred until ATPS returns OK. */
    const wifi_transport_ops_t *tport = wifi_transport_get();
    if (app_uart_atcmd_queue_fill(ATCMD_ATPS, AI_REC_WIFI_ATPS_PARAM))
    {
        tport->trigger_send();
    }
    else
    {
        APP_PRINT_ERROR0("wifi_on_link_up: ATPS queue_fill failed");
        /* ATPS cannot be queued - report connect failure. */
        ai_rec_trans.wifi_state = WIFI_STATE_FAIL;
        ai_rec_trans.wifi_connect_pending = false;
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
    }
#endif
}

/**
 * @brief Called when ATPS returns OK - query real IP/port via ATPI before
 *        emitting EVT_WIFI_CONNECT.
 *
 *        Moves state to ATPI_PENDING and issues ATPI. If ATPI cannot be
 *        queued, fall back to sending EVT with whatever IP we already have.
 */
#if (AT_CMD_VER == 2)
static void ai_rec_trans_wifi_atps_ok_evt(void)
{
    ai_rec_trans.wifi_state = WIFI_STATE_ATPI_PENDING;
    /* wifi_connect_pending stays true until ATPI OK fires. */

    const wifi_transport_ops_t *tport = wifi_transport_get();
    if (app_uart_atcmd_queue_fill(ATCMD_ATPI, "\r\n"))
    {
        tport->trigger_send();
        APP_PRINT_INFO0("wifi_atps_ok: ATPI queued, waiting for real IP/port");
    }
    else
    {
        APP_PRINT_ERROR0("wifi_atps_ok: ATPI queue_fill failed, fallback");
        /* ATPI cannot be queued - send EVT with whatever we have. */
        ai_rec_trans.wifi_state = WIFI_STATE_CONNECTED;
        ai_rec_trans.wifi_connect_pending = false;
        uint16_t real_port = app_uart_atcmd_get_atpi_port();
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_SUCCESS, ai_rec_trans.wifi_ip,
                                           real_port ? real_port : AI_REC_WIFI_TCP_PORT,
                                           ai_rec_trans.wifi_rssi);
    }
}
#endif /* (AT_CMD_VER == 2) */

/**
 * @brief Bridges AT module events to spec EVT frames.
 *
 *        AT_EVT_WIFI_GOT_IP carries a T_AT_IP_ADDR; we treat octets[0..3]
 *        as MSB..LSB (e.g. 192.168.1.100 => octets {192,168,1,100}) and
 *        repack into host-order uint32 for storage.
 *
 *        AT_EVT_WIFI_DISCONNECTED while CONNECTED (not
 * DISCONNECTING) is
 *        treated as an unsolicited disconnect (e.g. AP went away) and
 *        propagated to host so the UI can update.
 */
static void ai_rec_trans_wifi_atcmd_cb(T_AT_EVT_TYPE evt, void *p_data,
                                       uint16_t len)
{
    (void)len;
    APP_PRINT_INFO2("wifi_atcmd_cb: evt=%d state=%d",
                    evt, ai_rec_trans.wifi_state);

    switch (evt)
    {
    case AT_EVT_WIFI_GOT_IP:
        if (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTING)
        {
            /* Both the UART and SPI AT modules pass a T_AT_IP_ADDR here,
             * parsed from the module's `wifi got ip "x.x.x.x"` line.
             * p_data is NULL only when that parse failed; we then advance
             * with ip=0 so the host still gets an EVT and can re-poll via
             * CMD_WIFI_GET_STATUS rather than stalling in CONNECTING. */
            uint32_t ip_host = 0;
            if (p_data)
            {
                T_AT_IP_ADDR *ip = (T_AT_IP_ADDR *)p_data;
                ip_host = ((uint32_t)ip->octets[0] << 24) |
                          ((uint32_t)ip->octets[1] << 16) |
                          ((uint32_t)ip->octets[2] << 8) |
                          ((uint32_t)ip->octets[3]);
            }
#if F_APP_WIFI_SPI_CMD
            /* SPI path: GOT_IP -> store IP -> queue TCP server setup.
             *
             * Unlike the UART path (where the RTL8720C's TCP server is created
             * by explicit ATPS/ATPI commands), the external Wi-Fi IC needs two
             * AT+SKT* raw commands before it can accept TCP connections:
             *   1. AT+SKTCFG=,,1       -- disable Nagle algorithm
             *   2. AT+SKTSERVER=0,1,,<port>,1 -- listen + auto-recv
             *
             * EVT_WIFI_CONNECT + spi_file_upload_init() are deferred until
             * both commands have returned OK (tracked by spi_tcp_setup_step
             * in the AT_EVT_CMD_RESPONSE handler below). */
            ai_rec_trans.wifi_state = WIFI_STATE_CONNECTED;
            ai_rec_trans.wifi_connect_pending = false;
            ai_rec_trans.wifi_ip = ip_host;
            ai_rec_trans.wifi_rssi = 0;
            ai_rec_trans.spi_tcp_setup_step = 0;

            /* Queue TCP server setup via raw AT commands.  The AT engine
             * processes them sequentially after the current WLCONN command
             * finishes.  Both are queued before the OK from WLCONN has been
             * fully processed, so they execute in order: WLCONN -> SKTCFG ->
             * SKTSERVER.  spi_tcp_setup_step tracks completion: step 0 is
             * SKTCFG OK, step 1 is SKTSERVER OK, step 2 means ready. */
            {
                const wifi_transport_ops_t *tport = wifi_transport_get();
                if (tport)
                {
                    char cmd_line[48];
                    tport->queue_fill(ATCMD_RAW, "AT+SKTCFG=,,1\r\n");
                    snprintf(cmd_line, sizeof(cmd_line),
                             "AT+SKTSERVER=0,1,,%u,1\r\n", (unsigned int)AI_REC_WIFI_TCP_PORT);
                    tport->queue_fill(ATCMD_RAW, cmd_line);
                    tport->trigger_send();
                }
            }
#else
            ai_rec_trans_wifi_on_link_up(ip_host);
#endif
        }
        else if (ai_rec_trans.wifi_state == WIFI_STATE_ATPI_PENDING)
        {
            /* ATPI response delivered GOT_IP with real IP from the module.
             * Update stored IP - the ATPI OK handler will emit EVT. */
            if (p_data)
            {
                T_AT_IP_ADDR *ip = (T_AT_IP_ADDR *)p_data;
                ai_rec_trans.wifi_ip = ((uint32_t)ip->octets[0] << 24) |
                                       ((uint32_t)ip->octets[1] << 16) |
                                       ((uint32_t)ip->octets[2] << 8) |
                                       ((uint32_t)ip->octets[3]);
                APP_PRINT_INFO1("wifi_atcmd_cb: ATPI GOT_IP updated ip=0x%08x",
                                ai_rec_trans.wifi_ip);
            }
        }
        break;

    case AT_EVT_WIFI_CONNECTED:
        /* Raw association up; IP not yet leased. Stay in CONNECTING and
         * wait for AT_EVT_WIFI_GOT_IP to emit the success EVT. */
        APP_PRINT_INFO1("wifi_atcmd_cb: AT_EVT_WIFI_CONNECTED - link up, "
                        "waiting for DHCP (state=%d)",
                        ai_rec_trans.wifi_state);
        break;

    case AT_EVT_WIFI_DISCONNECTED:
        if (ai_rec_trans.wifi_state == WIFI_STATE_DISCONNECTING ||
            ai_rec_trans.wifi_state == WIFI_STATE_CONNECTED)
        {
            /* If a connect was still in flight (a disconnect interrupted it),
             * the host has an unanswered CMD_WIFI_CONNECT: report connect-fail
             * first, then the disconnect it asked for. From CONNECTED the flag
             * is already clear, so a normal disconnect emits only disconnect. */
            bool was_connecting = ai_rec_trans.wifi_connect_pending;
            ai_rec_trans.wifi_state   = WIFI_STATE_DISCONNECTED;
            ai_rec_trans.wifi_ip      = 0;
            ai_rec_trans.wifi_rssi    = 0;
            ai_rec_trans.wifi_ssid[0] = '\0';
            ai_rec_trans.wifi_connect_pending = false;
            if (was_connecting)
            {
                ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_AP_NOT_FOUND,
                                                   0, 0, 0);
            }
            ai_rec_trans_send_wifi_disconnect_evt(WIFI_RESULT_SUCCESS);
            /* CMD_WIFI_DISCONNECT (or unsolicited disconnect while
             * connected) completed - safe to re-enable DLPS. */
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            /* WiFi link is down - restore CPU+SPIC0 to normal frequency. */
#if F_APP_WIFI_SPI_CMD
            spi_file_upload_restore_clk();
#elif F_APP_WIFI_UART_CMD
            wifi_file_upload_restore_clk();
#endif
        }
        else if (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTING ||
                 ai_rec_trans.wifi_state == WIFI_STATE_ATPS_PENDING ||
                 ai_rec_trans.wifi_state == WIFI_STATE_ATPI_PENDING)
        {
            /* Connect attempt failed (AP gone, deauth, or link dropped while
             * waiting for ATPS). wifi_connect_pending covers both CONNECTING
             * and ATPS_PENDING. */
            ai_rec_trans.wifi_state = WIFI_STATE_FAIL;
            ai_rec_trans.wifi_connect_pending = false;
            ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_AP_NOT_FOUND,
                                               0, 0, 0);
            /* Connect lifecycle ended in FAIL - re-enable DLPS. */
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        }
        break;

    case AT_EVT_CMD_RESPONSE:
        if (p_data &&
            *(T_AT_CMD_RSP_STATE *)p_data == AT_CMD_RSP_STATE_ERROR)
        {
            /* AT module reports a generic ERROR - dispatched by current
             * state so each phase gets a sensible result code. */
            if (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTING)
            {
                /* ATPN (or WLCONN) failed. */
                ai_rec_trans.wifi_state = WIFI_STATE_FAIL;
                ai_rec_trans.wifi_connect_pending = false;
                ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_AUTH_FAIL,
                                                   0, 0, 0);
                /* Connect lifecycle ended in FAIL - re-enable DLPS. */
                app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            }
#if F_APP_WIFI_SPI_CMD
            else if (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTED &&
                     ai_rec_trans.spi_tcp_setup_step < 2)
            {
                /* SPI path: SKTCFG or SKTSERVER failed.
                 * TCP server cannot be created; report HARDWARE_ERROR. */
                APP_PRINT_ERROR1("wifi_atcmd_cb: SPI TCP setup failed at step %d",
                                 ai_rec_trans.spi_tcp_setup_step);
                ai_rec_trans.wifi_state = WIFI_STATE_FAIL;
                ai_rec_trans.wifi_connect_pending = false;
                ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR,
                                                   0, 0, 0);
                app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            }
#endif /* F_APP_WIFI_SPI_CMD */
#if F_APP_WIFI_UART_CMD
            else if (ai_rec_trans.wifi_state == WIFI_STATE_ATPS_PENDING)
            {
                /* ATPS failed - TCP server not configured. */
                ai_rec_trans.wifi_state = WIFI_STATE_FAIL;
                ai_rec_trans.wifi_connect_pending = false;
                ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR,
                                                   0, 0, 0);
                /* Connect lifecycle ended in FAIL - re-enable DLPS. */
                app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            }
            else if (ai_rec_trans.wifi_state == WIFI_STATE_ATPI_PENDING)
            {
                /* ATPI failed - could not query IP/port.
                 * Fall back: emit EVT with whatever we already have. */
                APP_PRINT_ERROR0("wifi_atcmd_cb: ATPI failed, fallback");
                ai_rec_trans.wifi_state = WIFI_STATE_CONNECTED;
                ai_rec_trans.wifi_connect_pending = false;
                uint16_t real_port = app_uart_atcmd_get_atpi_port();
                ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_SUCCESS,
                                                   ai_rec_trans.wifi_ip,
                                                   real_port ? real_port : AI_REC_WIFI_TCP_PORT,
                                                   ai_rec_trans.wifi_rssi);
                /* ATPI fallback => CONNECTED - re-enable DLPS. */
                app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            }
#endif /* F_APP_WIFI_UART_CMD - ATPS/ATPI are UART-only states */
            else if (ai_rec_trans.wifi_state == WIFI_STATE_DISCONNECTING)
            {
                ai_rec_trans.wifi_state   = WIFI_STATE_DISCONNECTED;
                ai_rec_trans.wifi_ssid[0] = '\0';
                ai_rec_trans_send_wifi_disconnect_evt(WIFI_RESULT_SUCCESS);
                /* Disconnect completed - re-enable DLPS. */
                app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
                /* WiFi link is down - restore CPU+SPIC0 to normal frequency. */
#if F_APP_WIFI_SPI_CMD
                spi_file_upload_restore_clk();
#elif F_APP_WIFI_UART_CMD
                wifi_file_upload_restore_clk();
#endif
            }
        }
#if F_APP_WIFI_SPI_CMD
        /* ---- SPI path: TCP server setup (SKTCFG -> SKTSERVER) ---------------
         * After GOT_IP, the GOT_IP handler (above) queues AT+SKTCFG and
         * AT+SKTSERVER as raw AT commands, then sets state = CONNECTED and
         * resets spi_tcp_setup_step = 0.  Each OK response advances the step:
         *   step 0 -> SKTCFG OK (Nagle disabled)
         *   step 1 -> SKTSERVER OK (TCP server listening)
         *   step 2 -> both done: init file upload + send EVT_WIFI_CONNECT
         *
         * Note: we do NOT check `p_data` here because the OK arm of the
         * ERROR check above means p_data is valid and carries OK state. */
        else if (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTED &&
                 ai_rec_trans.spi_tcp_setup_step < 2)
        {
            ai_rec_trans.spi_tcp_setup_step++;
            APP_PRINT_INFO1("wifi_atcmd_cb: SPI TCP setup step %d OK",
                            ai_rec_trans.spi_tcp_setup_step);

            if (ai_rec_trans.spi_tcp_setup_step >= 2)
            {
                APP_PRINT_INFO0("wifi_atcmd_cb: SPI TCP server ready, "
                                "sending EVT_WIFI_CONNECT");
                T_SPI_UPLOAD_CFG upload_cfg =
                {
                    .packet_size       = SPI_XMIT_SIZE,
                    .buffer_check_size = SPI_XMIT_SIZE * 3,
                    .mode              = 0x00,
                    .ic_type           = 0x13,
                    .song_format_type  = 0x0F,
                    .transport_cap     = 0x07,
                    .tcp_port          = AI_REC_WIFI_TCP_PORT,
                    .sd_mount          = "/SD:/audio",
                };
                spi_file_upload_init(&upload_cfg);

                /* BLE coexistence: widen interval to 500 ms */
                extern void ble_set_prefer_conn_param(uint8_t conn_id,
                                                      uint16_t min_interval,
                                                      uint16_t max_interval,
                                                      uint16_t latency,
                                                      uint16_t supervision_timeout);
                ble_set_prefer_conn_param(ai_rec_trans.conn_id,
                                          0x0190, 0x0190, 0, 500);

                ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_SUCCESS,
                                                   ai_rec_trans.wifi_ip,
                                                   AI_REC_WIFI_TCP_PORT,
                                                   ai_rec_trans.wifi_rssi);
                /* WiFi + TCP connect lifecycle complete - re-enable DLPS. */
                app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            }
        }
#endif /* F_APP_WIFI_SPI_CMD */
#if (AT_CMD_VER == 2)
        else if (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTING)
        {
            /* ATPN returned OK. Link is up (association + DHCP done).
             * Advance to ATPS_PENDING and issue ATPS.
             * EVT_WIFI_CONNECT is deferred until ATPS OK. */
            ai_rec_trans_wifi_on_link_up(0);
        }
        else if (ai_rec_trans.wifi_state == WIFI_STATE_ATPS_PENDING)
        {
            /* ATPS returned OK - TCP server is ready.
             * Query real IP/port via ATPI before emitting EVT. */
            ai_rec_trans_wifi_atps_ok_evt();
        }
        else if (ai_rec_trans.wifi_state == WIFI_STATE_ATPI_PENDING)
        {
            /* ATPI returned OK - we now have real IP and port.
             * Final step: move to CONNECTED and emit EVT_WIFI_CONNECT. */
            ai_rec_trans.wifi_state = WIFI_STATE_CONNECTED;
            ai_rec_trans.wifi_connect_pending = false;

            /* Initialize WiFi file upload module (registers SDIO data callback
             * on_tcp_rx so incoming TCP commands e.g. 0x694 are processed). */
            {
                T_WIFI_UPLOAD_CFG upload_cfg =
                {
                    .packet_size       = 1024,
                    .buffer_check_size = 20480,
                    .mode              = 0x00,
                    .ic_type           = 0x13,            /* RTL87x3EP */
                    .song_format_type  = 0x0F,            /* AAC|MP3|FLAC|WAV */
                    .transport_cap     = 0x07,            /* BLE|SPP|WiFi */
                    .tcp_port          = AI_REC_WIFI_TCP_PORT,
                    .sd_mount          = "/SD:/audio",
                };
                wifi_file_upload_init(&upload_cfg);
            }

            uint16_t real_port = app_uart_atcmd_get_atpi_port();
            APP_PRINT_INFO2("wifi_atpi_ok: EVT_WIFI_CONNECT ip=0x%08x port=%u",
                            ai_rec_trans.wifi_ip,
                            real_port ? real_port : AI_REC_WIFI_TCP_PORT);
            ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_SUCCESS,
                                               ai_rec_trans.wifi_ip,
                                               real_port ? real_port : AI_REC_WIFI_TCP_PORT,
                                               ai_rec_trans.wifi_rssi);

            /* WiFi upload is about to start via SDIO/TCP.  BLE and WiFi share
             * the same antenna on RTL87x3EP; a short BLE connection interval
             * (default 47.5 ms) causes frequent antenna pre-emption, filling
             * the SDIO Tx BD pool and throttling upload throughput by ~65 %.
             *
             * Set the BLE connection interval to 500 ms so BLE events are
             * sparse enough that WiFi can drain its Tx BDs between them.
             * 500 ms = 400 x 1.25 ms = 0x0190.
             *
             * This is a best-effort request - the phone (central) may accept
             * or reject it.  The supervision timeout stays at 5 s (500 x
             * 10 ms) so the link survives a missed event at the longer
             * interval. */
            APP_PRINT_INFO0("[wifi_upload] lowering BLE conn interval to 500 ms "
                            "for WiFi upload coexistence");
            extern void ble_set_prefer_conn_param(uint8_t conn_id,
                                                  uint16_t min_interval,
                                                  uint16_t max_interval,
                                                  uint16_t latency,
                                                  uint16_t supervision_timeout);
            ble_set_prefer_conn_param(ai_rec_trans.conn_id,
                                      0x0190, 0x0190,
                                      0,
                                      500);

            /* WiFi connect lifecycle completed successfully -
             * safe to re-enable DLPS.  Actual WiFi file uploads
             * manage DLPS independently via wifi_file_upload.c. */
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        }
#endif
        /* (AT_CMD_VER==1 / WLCONN: OK is command-accept only; keep waiting for
         * the unsolicited GOT_IP to confirm the link.) */
        break;

    default:
        break;
    }
}
#endif /* F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD */

/* ---- Command handlers ----------------------------------------------------*/

#if F_APP_WIFI_UART_CMD
/** @brief Queue the connect command and kick the AT send flow.
 *  Uses the vtable from wifi_transport_get(). Shared by the connect handler
 *  (module already powered) and the cold-start bring-up callback. */
static bool ai_rec_trans_wifi_send_connect_cmd(const char *param)
{
    const wifi_transport_ops_t *tport = wifi_transport_get();
    if (!tport)
    {
        return false;
    }
    tport->queue_fill(AI_REC_WIFI_AT_CMD_CONNECT, param);
    tport->trigger_send();
    return true;
}

/** @brief Settle window between module power-on (chip_en + SDIO bring-up) and
 *         the first ATPN, executed on the WiFi task.
 *
 *  Root cause of the cold-start connect failure (COM8 15:21 log): the module
 *  booted to "COMMAND READY" but ATPN, fired ~1s after wifi_sdio_init, got zero
 *  bytes back for the full 20s ATPN timeout. The proven phone-driven 0x8401 path
 *  (COM8 15:32 log) issues the same TX code successfully - its only material
 *  difference is that ATPN arrives ~23s after SDIO init, i.e. the module is
 *  fully idle. The module is not ready to process AT commands immediately after
 *  the SDIO handshake, so we wait here before sending. Runs on the WiFi task
 *  (never a BLE callback), so a blocking os_delay is safe. */
#define AI_REC_WIFI_BRINGUP_SETTLE_MS       3000
/* Max time to wait for the module's "COMMAND READY" boot banner, and the poll
 * granularity while waiting. If the banner never arrives (missed/garbled) we
 * fall through after the timeout and try ATPN anyway rather than wedging. */
#define AI_REC_WIFI_READY_TIMEOUT_MS        8000
#define AI_REC_WIFI_READY_POLL_MS           50

/**
 * @brief Cold-start bring-up, executed on the WiFi task.
 *
 *  Posted by the connect handler (see ai_rec_trans_handle_wifi_connect) so the
 *  multi-second module power-on never blocks the BLE/GATT callback. Mirrors the
 *  proven CMD_AI_RECORD_WIFI_POWER_ON (0x8401) sequence:
 *    - wifi_power_on(): chip_en timing (~2.2s) + RF switch + SDIO + upload port,
 *    - wifi_enable_flag = 1: open the UART RX gate so AT responses reach the
 *      parser (otherwise they are dropped as "unknown uart_mode"),
 *    - wait for the module's "COMMAND READY" boot banner (with a fallback
 *      timeout), then a short settle, so the first AT command is not raced
 *      against module bring-up (see AI_REC_WIFI_READY_TIMEOUT_MS),
 *  then issues the deferred ATPN connect - unless a disconnect arrived during
 *  bring-up and moved us out of CONNECTING, in which case the stale connect is
 *  dropped. Runs in the same task that drives the AT send/parse flow, so the
 *  queue_fill / EVT-notify calls here follow the established threading model. */
static void ai_rec_trans_wifi_bringup_connect(void *p_msg)
{
    (void)p_msg;

    /* Clear the latched banner before powering the module so we observe THIS
     * boot's "COMMAND READY", never a stale one from a previous power cycle. */
    app_wifi_uart_reset_module_ready();

    wifi_power_on();
    wifi_enable_flag = 1;

    /* Wait for the module's "COMMAND READY" banner instead of a blind delay:
     * the banner proves the module finished booting and the UART link is alive.
     * Poll with a fallback timeout so a missed/garbled banner never wedges us. */
    {
        uint32_t waited = 0;
        while (!app_wifi_uart_module_is_ready() &&
               waited < AI_REC_WIFI_READY_TIMEOUT_MS)
        {
            os_delay(AI_REC_WIFI_READY_POLL_MS);
            waited += AI_REC_WIFI_READY_POLL_MS;
        }
        if (app_wifi_uart_module_is_ready())
        {
            APP_PRINT_INFO1("wifi_connect: module COMMAND READY after %d ms", waited);
        }
        else
        {
            APP_PRINT_WARN1("wifi_connect: COMMAND READY not seen in %d ms, send anyway",
                            waited);
        }
    }

    /* COMMAND READY only marks that the boot banner printed; the COM8 logs show
     * ATPN can still get no response if issued right after it (the proven
     * phone-driven flow waited far longer). Keep a settle after the banner. */
    os_delay(AI_REC_WIFI_BRINGUP_SETTLE_MS);

    if (ai_rec_trans.wifi_state != WIFI_STATE_CONNECTING)
    {
        APP_PRINT_INFO1("wifi_connect: bring-up done but state=%d, drop connect",
                        ai_rec_trans.wifi_state);
        /* CMD_WIFI_CONNECT lifecycle ends here without a terminal AT
         * callback - re-enable DLPS so we don't leak it disabled. */
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        return;
    }

    APP_PRINT_INFO1("wifi_connect: bring-up done, flag=%d, posting ATPN to app task",
                    wifi_enable_flag);

    /* Do NOT send ATPN here on the WiFi task. The cold-start connect failure
     * (COM8 logs) was isolated to the send TASK CONTEXT: the identical ATPN
     * sent from the WiFi task gets zero response from the module, while the
     * proven phone-driven 0x8440 demo - which sends from the main app task -
     * works every time (all other factors: \r\n, timing, DLPS, SDIO bring-up,
     * wifi_power_on(), wifi_enable_flag, are byte/sequence identical). So hand
     * the actual send off to the main app task (same context as the demo) via
     * an IO message; ai_rec_trans_wifi_send_connect_deferred() runs it there. */
    if (!app_wifi_uart_msg_send(IO_WIFI_UART_SEND_CONNECT, NULL))
    {
        APP_PRINT_ERROR0("wifi_connect: post ATPN-send msg to app task failed");
        ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTED;
        ai_rec_trans.wifi_connect_pending = false;
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        /* No AT callback will fire - re-enable DLPS now. */
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
    }
}

/** @brief Send the deferred ATPN connect. Registered with
 *  app_wifi_uart_deferred_connect_register() and invoked on the MAIN APP TASK
 *  when IO_WIFI_UART_SEND_CONNECT arrives - i.e. the exact task context of the
 *  proven 0x8440 demo, never the WiFi task. Uses the param stashed at connect
 *  time (wifi_pending_connect_param). Drops the send if a disconnect moved us
 *  out of CONNECTING during bring-up. */
static void ai_rec_trans_wifi_send_connect_deferred(void)
{
    if (ai_rec_trans.wifi_state != WIFI_STATE_CONNECTING)
    {
        APP_PRINT_INFO1("wifi_connect: deferred send but state=%d, drop connect",
                        ai_rec_trans.wifi_state);
        /* CMD_WIFI_CONNECT lifecycle ends here - no AT callback
         * will fire, so re-enable DLPS now. */
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        return;
    }

    APP_PRINT_INFO0("wifi_connect: sending ATPN from app task");

    if (!ai_rec_trans_wifi_send_connect_cmd(wifi_pending_connect_param))
    {
        ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTED;
        ai_rec_trans.wifi_connect_pending = false;
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        /* No AT callback will fire - re-enable DLPS now. */
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
    }
}
#endif /* F_APP_WIFI_UART_CMD */

/**
 * @brief CMD_WIFI_CONNECT (0x0691) handler.
 *
 *  Wire layout (same for SPI and UART):
 *    Byte0       SSID length (1..32)
 *    Byte1..M    SSID (UTF-8, not NUL-terminated on wire)
 *    ByteM+1     Password length (0..64; 0 means open network)
 *    ByteM+2..N  Password (UTF-8)
 *
 *  UART path: sends ATPN=<ssid>,<pwd> via app_uart_atcmd, multi-step
 *             state machine (ATPN -> ATPS -> ATPI -> CONNECTED).
 *  SPI path:  sends AT+WLCONN=ssid,<ssid>,pw,<pwd> via app_spi_atcmd,
 *             single-step state machine (AT+WLCONN -> GOT_IP -> CONNECTED).
 */
static void ai_rec_trans_handle_wifi_connect(uint8_t *p, uint16_t plen)
{
    /* ---------- Parameter validation and parsing (common) ---------- */
    if (plen < 2)
    {
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        return;
    }

    /* Already linked: spec asks for ALREADY_CONNECTED with valid IP/port. */
    if (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTED)
    {
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_ALREADY_CONNECTED,
                                           ai_rec_trans.wifi_ip,
                                           AI_REC_WIFI_TCP_PORT,
                                           ai_rec_trans.wifi_rssi);
        return;
    }

    /* Mid-flight connect / disconnect: refuse new request. */
    if (ai_rec_trans.wifi_state == WIFI_STATE_CONNECTING ||
        ai_rec_trans.wifi_state == WIFI_STATE_DISCONNECTING
#if F_APP_WIFI_UART_CMD
        || ai_rec_trans.wifi_state == WIFI_STATE_ATPS_PENDING
#endif
       )
    {
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        return;
    }

    uint8_t ssid_len = p[0];
    if (ssid_len == 0 || ssid_len > AI_REC_WIFI_SSID_MAX ||
        plen < (uint16_t)(1 + ssid_len + 1))
    {
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        return;
    }

    char ssid[AI_REC_WIFI_SSID_MAX + 1];
    memcpy(ssid, p + 1, ssid_len);
    ssid[ssid_len] = '\0';

    uint8_t pass_len = p[1 + ssid_len];
    if (pass_len > AI_REC_WIFI_PASS_MAX ||
        plen < (uint16_t)(1 + ssid_len + 1 + pass_len))
    {
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        return;
    }

    char pass[AI_REC_WIFI_PASS_MAX + 1];
    if (pass_len > 0)
    {
        memcpy(pass, p + 1 + ssid_len + 1, pass_len);
    }
    pass[pass_len] = '\0';

    APP_PRINT_INFO2("wifi_connect: ssid=%s pass_len=%d",
                    TRACE_STRING(ssid), pass_len);

    /* Stash SSID for later EVT_WIFI_GET_STATUS responses. */
    strncpy(ai_rec_trans.wifi_ssid, ssid, AI_REC_WIFI_SSID_MAX);
    ai_rec_trans.wifi_ssid[AI_REC_WIFI_SSID_MAX] = '\0';
    ai_rec_trans.wifi_state = WIFI_STATE_CONNECTING;
    ai_rec_trans.wifi_connect_pending = true;

    /* ---------- Transport-specific AT command send ---------- */
#if (F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD)
    const wifi_transport_ops_t *tport = wifi_transport_get();
#if F_APP_WIFI_SPI_CMD
    /* SPI path: AT+WLCONN=ssid,<ssid>,pw,<pwd> via SPI AT engine.
     * The SPI AT engine handles the command asynchronously; the
     * ai_rec_trans_wifi_atcmd_cb() callback drives the state machine
     * forward on GOT_IP -> CONNECTED (single-step, no ATPS/ATPI). */
    char spi_param[AI_REC_WIFI_SSID_MAX + AI_REC_WIFI_PASS_MAX + 16];
    snprintf(spi_param, sizeof(spi_param), "ssid,%s,pw,%s\r\n", ssid, pass);
    if (!tport || !tport->queue_fill(ATCMD_WLCONN, spi_param))
    {
        ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTED;
        ai_rec_trans.wifi_connect_pending = false;
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        APP_PRINT_ERROR0("wifi_connect: SPI queue_fill failed");
        return;
    }
    tport->trigger_send();
    /* EVT_WIFI_CONNECT sent asynchronously from ai_rec_trans_wifi_atcmd_cb()
     * on AT_EVT_WIFI_GOT_IP (see GOT_IP handler, #if F_APP_WIFI_SPI_CMD). */

#elif F_APP_WIFI_UART_CMD
    /* UART path: ATPN=<ssid>,<pwd> via UART AT engine.
     * Cold-start bring-up defers the actual ATPN to the WiFi task;
     * subsequent connects send immediately. */
    char param[AI_REC_WIFI_SSID_MAX + AI_REC_WIFI_PASS_MAX + 6];
    snprintf(param, sizeof(param), "%s,%s\r\n", ssid, pass);

    if (!wifi_powered_on)
    {
        wifi_task_ensure();
        wifi_powered_on = true;

        strncpy(wifi_pending_connect_param, param,
                sizeof(wifi_pending_connect_param) - 1);
        wifi_pending_connect_param[sizeof(wifi_pending_connect_param) - 1] = '\0';

        T_WIFI_MSG msg = {0};
        msg.event  = EVENT_USER_APP_DEFINE;
        msg.msg_cb = ai_rec_trans_wifi_bringup_connect;
        if (!app_send_msg_to_wifitask(&msg))
        {
            APP_PRINT_ERROR0("wifi_connect: post bring-up msg failed");
            ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTED;
            ai_rec_trans.wifi_connect_pending = false;
            ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        }
        return;
    }

    if (!ai_rec_trans_wifi_send_connect_cmd(param))
    {
        ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTED;
        ai_rec_trans.wifi_connect_pending = false;
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
        return;
    }
    /* EVT emitted asynchronously by ai_rec_trans_wifi_atcmd_cb(). */
#endif /* F_APP_WIFI_UART_CMD / F_APP_WIFI_SPI_CMD */
#else
    /* No WiFi transport available on this board - report connect failure. */
    ai_rec_trans.wifi_state = WIFI_STATE_FAIL;
    ai_rec_trans.wifi_connect_pending = false;
    ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_HARDWARE_ERROR, 0, 0, 0);
#endif /* F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD */
}

/**
 * @brief CMD_WIFI_DISCONNECT (0x0692) handler.
 *
 *  Wire layout:
 *    Byte0   Flags (Bit0 = forget credential - informational only;
 *                   the AT module doesn't currently expose credential
 *                   storage, so this firmware always behaves as if
 *                   credentials are forgotten)
 *
 *  UART path: sends ATWD via app_uart_atcmd.
 *  SPI path:  sends AT+WLDISCONN via app_spi_atcmd.
 */
static void ai_rec_trans_handle_wifi_disconnect(uint8_t *p, uint16_t plen)
{
    uint8_t flags = (plen >= 1) ? p[0] : 0;
    APP_PRINT_INFO2("wifi_disconnect: flags=0x%02x state=%d",
                    flags, ai_rec_trans.wifi_state);

    /* Already idle - report success idempotently. */
    if (ai_rec_trans.wifi_state == WIFI_STATE_DISCONNECTED ||
        ai_rec_trans.wifi_state == WIFI_STATE_FAIL)
    {
        ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTED;
        ai_rec_trans_send_wifi_disconnect_evt(WIFI_RESULT_SUCCESS);
        return;
    }

#if (F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD)
    const wifi_transport_ops_t *tport = wifi_transport_get();
#if F_APP_WIFI_SPI_CMD
    /* SPI path: AT+WLDISCONN via SPI AT engine.
     * A disconnect can land while STILL CONNECTING (AT+WLCONN sent but
     * GOT_IP not yet received). The state check in the async callback
     * (ai_rec_trans_wifi_atcmd_cb) handles this: the GOT_IP handler checks
     * wifi_state == CONNECTING, so moving us out of CONNECTING here drops
     * the stale connect. */
    if (!tport || !tport->queue_fill(ATCMD_WLDISCONN, "\r\n"))
    {
        ai_rec_trans_send_wifi_disconnect_evt(0x00);
        return;
    }
    ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTING;
    tport->trigger_send();
    /* EVT emitted asynchronously on AT_EVT_WIFI_DISCONNECTED. */

#elif F_APP_WIFI_UART_CMD
    /* ATPS_PENDING: link is up but EVT_WIFI_CONNECT hasn't fired yet.
     * A disconnect here aborts the pending connect entirely. */
    if (ai_rec_trans.wifi_state == WIFI_STATE_ATPS_PENDING)
    {
        ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTED;
        ai_rec_trans.wifi_ip = 0;
        ai_rec_trans.wifi_ssid[0] = '\0';
        ai_rec_trans.wifi_connect_pending = false;
        ai_rec_trans_send_wifi_connect_evt(WIFI_RESULT_AP_NOT_FOUND, 0, 0, 0);
        ai_rec_trans_send_wifi_disconnect_evt(WIFI_RESULT_SUCCESS);
        return;
    }

    /* A disconnect can land while the WiFi task is still doing the blocking
     * bring-up (state CONNECTING, ATPN not yet sent). No timer to cancel: the
     * bring-up callback re-checks wifi_state == CONNECTING before it issues the
     * deferred ATPN, so moving us out of CONNECTING here drops the stale
     * connect on its own. */
    if (!tport || !tport->queue_fill(AI_REC_WIFI_AT_CMD_DISCONNECT, NULL))
    {
        ai_rec_trans_send_wifi_disconnect_evt(0x00);
        return;
    }

    ai_rec_trans.wifi_state = WIFI_STATE_DISCONNECTING;
    tport->trigger_send();
    /* EVT emitted asynchronously on AT_EVT_WIFI_DISCONNECTED. */
#endif /* F_APP_WIFI_SPI_CMD / F_APP_WIFI_UART_CMD */
#else
    /* No WiFi transport available - report disconnect success. */
    ai_rec_trans_send_wifi_disconnect_evt(WIFI_RESULT_SUCCESS);
#endif /* F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD */
}

/**
 * @brief CMD_WIFI_GET_STATUS (0x0693) handler. No params.
 *
 *  Synchronous: reports whatever state the AT-callback last observed.
 *  RSSI is best-effort; this firmware doesn't poll RSSI via AT yet, so
 *  the value is whatever was sampled at connect time (0 on cold boot).
 */
static void ai_rec_trans_handle_wifi_get_status(uint8_t *p, uint16_t plen)
{
    (void)p;
    (void)plen;
    APP_PRINT_INFO1("wifi_get_status: state=%d", ai_rec_trans.wifi_state);
    ai_rec_trans_send_wifi_status_evt();
}

/*============================================================================*
 *                              Upload flow - open / cleanup
 *============================================================================*/

static void ai_rec_trans_close_and_reset(T_AI_REC_TRANS_STATE final_state)
{
    /* Re-enable DLPS now that the upload has ended (done / canceled / error).
     * Paired with the app_dlps_disable() in handle_upload_file. */
    app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);

    /* Restore CPU + SPIC0 to normal frequency (40 MHz). */
    if (clk_mgr_upload_handle)
    {
        clk_mgr_set_normal_performance(clk_mgr_upload_handle);
    }

    if (ai_rec_trans.fs_handle)
    {
        app_fs_close_file(ai_rec_trans.fs_handle);
        ai_rec_trans.fs_handle = NULL;
    }
    if (ai_rec_trans.p_chunk_buf)
    {
        free(ai_rec_trans.p_chunk_buf);
        ai_rec_trans.p_chunk_buf = NULL;
    }
    ai_rec_trans.chunk_buf_size    = 0;
    ai_rec_trans.file_total_length = 0;
    ai_rec_trans.start_offset      = 0;
    ai_rec_trans.cur_offset        = 0;
    ai_rec_trans.local_seq         = 0;
    ai_rec_trans.whole_file_crc    = 0;
    ai_rec_trans.host_max_chunk    = 0;
    ai_rec_trans.chunk_payload_max = 0;
    ai_rec_trans.file_format       = AI_REC_FORMAT_ALL;     /* sentinel: no file loaded */
    ai_rec_trans.file_create_ts    = 0;
    ai_rec_trans.live_mode           = false;
    ai_rec_trans.recording_finalized = false;
    ai_rec_trans.live_filename[0]    = '\0';

    ai_rec_trans.conn_update_pending     = false;

    /* Eventual stable state: IDLE if normal cleanup, otherwise the
     * caller-specified terminal (CANCELED / ERROR / DONE). */
    ai_rec_trans.state = final_state;

    ai_rec_trans_disarm_watchdog();

    /* Final-state stickiness is for logging only; immediately re-arm
     * IDLE so a fresh upload can start. */
    ai_rec_trans.state = AI_REC_TRANS_IDLE;
}

/**
 * @brief  Determine file format from filename suffix.
 *         Heuristic; falls back to OPUS (since recorder writes .opus).
 */
static T_AI_REC_FILE_FORMAT ai_rec_format_from_name(const char *name)
{
    size_t n = strlen(name);
    if (n >= 5 && strcmp(name + n - 5, ".opus") == 0)
    {
        return AI_REC_FORMAT_OPUS;
    }
    if (n >= 4 && strcmp(name + n - 4, ".mp3")  == 0)
    {
        return AI_REC_FORMAT_MP3;
    }
    if (n >= 4 && strcmp(name + n - 4, ".mp4")  == 0)
    {
        return AI_REC_FORMAT_MP4;
    }
    if (n >= 4 && strcmp(name + n - 4, ".rtk")  == 0)
    {
        return AI_REC_FORMAT_RTK;
    }
    if (n >= 4 && strcmp(name + n - 4, ".aac")  == 0)
    {
        return AI_REC_FORMAT_AAC;
    }
    if (n >= 5 && strcmp(name + n - 5, ".flac") == 0)
    {
        return AI_REC_FORMAT_FLAC;
    }
    if (n >= 4 && strcmp(name + n - 4, ".txt")  == 0)
    {
        return AI_REC_FORMAT_TXT;
    }
    if (n >= 4 && strcmp(name + n - 4, ".dat")  == 0)
    {
        return AI_REC_FORMAT_DAT;
    }
    if (n >= 4 && strcmp(name + n - 4, ".bin")  == 0)
    {
        return AI_REC_FORMAT_BIN;
    }
    return AI_REC_FORMAT_OPUS;
}

#if CONFIG_REALTEK_APP_RTC_CALENDAR_SUPPORT
/** @brief Approximate unix-epoch from RTC. 0 if nothing useful available. */
static uint32_t ai_rec_get_now_unix_ts(void)
{
    T_UTC_TIME utc;
    app_ai_record_rtc_get_utc_time(&utc);
    /* Watch RTC's epoch is 1900-01-01 in this codebase; unix epoch is
     * 1970-01-01. Days between: 70*365 + 17 leap = 25,567 => 2208988800 s.
     * Computing exact unix would need a full date arithmetic helper -
     * for now we just hand back a monotonic timestamp the host can
     * interpret as "watch local timestamp", with 0 reserved for unknown. */
    if (utc.year < 1970)
    {
        return 0;
    }
    return 0;  /* TODO: full year/month/day => unix epoch conversion */
}
#else
static uint32_t ai_rec_get_now_unix_ts(void)
{
    return 0;
}
#endif

/**
 * @brief  Apply preferred connection params for upload throughput.
 *         Best-effort; failures are logged but not fatal.
 */
static void ai_rec_trans_apply_fast_conn_params(void)
{
    if (ai_rec_trans.chann_type != GAP_CHANN_TYPE_LE_ATT &&
        ai_rec_trans.chann_type != GAP_CHANN_TYPE_LE_ECFC)
    {
        return;  /* not LE; nothing to tune */
    }
    /* These symbols come from app_ble_gap.c / SDK. Both signatures exist
     * in the codebase: ble_set_prefer_conn_param(conn_id, min, max,
     * latency, supervision_timeout). Best effort. */
    extern void ble_set_prefer_conn_param(uint8_t conn_id,
                                          uint16_t min_interval,
                                          uint16_t max_interval,
                                          uint16_t latency,
                                          uint16_t supervision_timeout);
    ble_set_prefer_conn_param(ai_rec_trans.conn_id,
                              AI_REC_CONN_INTERVAL_MIN,
                              AI_REC_CONN_INTERVAL_MAX,
                              AI_REC_CONN_LATENCY,
                              AI_REC_CONN_SUPERVISION_TIMEOUT);
    APP_PRINT_INFO2("ai_rec_trans: pref CI %d-%d (units 1.25ms)",
                    AI_REC_CONN_INTERVAL_MIN, AI_REC_CONN_INTERVAL_MAX);

    /* Mark the update as pending so push_chunks_burst() waits for it
     * to complete before sending the first data chunk. Prevents the
     * "stuck GATT notification" scenario where a notification queued
     * during the connection update procedure never completes. */
    ai_rec_trans.conn_update_pending = true;
}

/**
 * @brief  Handle CMD_UPLOAD_FILE (0x694). Validates input, opens file,
 *         pre-computes whole-file CRC, then sends EVT Flag=0x00 and
 *         arms timers to begin streaming.
 *
 *         On any failure path emits EVT Flag=0xFE with appropriate
 *         error code and stays in ERROR/IDLE.
 *
 *         Wire layout (host to record_pen):
 *           Byte0           transport (0=current, 1=BLE; others rejected)
 *           Byte1..2        filename length (LE)
 *           Byte3..N        filename
 *           N+1..N+4        start offset (LE)
 *           N+5..N+8        max chunk size (LE)
 */
static void ai_rec_trans_handle_upload_file(uint8_t *p, uint16_t plen)
{
    if (ai_rec_trans.state != AI_REC_TRANS_IDLE)
    {
        APP_PRINT_WARN1("upload_file: bad state %d", ai_rec_trans.state);
        ai_rec_trans_send_upload_error(UPLOAD_ERR_PERMISSION);
        return;
    }

    /* Protocol handshake guard.
     *
     *  Spec V4.1 expects the host to issue CMD_QUERY_INFO at the start
     *  of every BLE session so it learns packet_size, protocol_ver and
     *  transport_cap before streaming. We refuse UPLOAD until this has
     *  happened - otherwise the host would be guessing the chunk size
     *  the device is sized for, leading to over- or under-runs.
     *
     *  The flag is sticky for the duration of the current notify-enabled
     *  session and is cleared in on_cccd() when the host disables notify
     *  or disconnects. */
    if (!ai_rec_trans.query_info_received)
    {
        APP_PRINT_ERROR0("upload_file: refused, host must issue "
                         "CMD_QUERY_INFO first");
        ai_rec_trans_send_upload_error(UPLOAD_ERR_PERMISSION);
        return;
    }

    /* Concurrent recording + upload is supported.
     *
     *  Recording (mic => SD) and upload (SD => BLE) own independent
     *  T_FILE_HANDLE objects, so FATFS serializes the two streams
     *  internally. Upload reads a previously-finalized file while
     *  recording appends to a new (different) file.
     *
     *  KNOWN LIMITATION: if the host requests upload of the file
     *  currently being recorded, app_fs_open_file may either fail
     *  (FATFS write-lock) or succeed with a partial snapshot whose
     *  size grows during transfer. We log the recording state for
     *  diagnosis but do NOT block - the host is expected not to do
     *  this. A future hardening would expose the active record
     *  filename and refuse same-file uploads here. */
    if (ai_rec_trans.is_recording)
    {
        APP_PRINT_INFO0("upload_file: starting while recording is active "
                        "(concurrent operation supported)");
    }

    if (!ai_rec_trans.notify_enabled)
    {
        APP_PRINT_WARN0("upload_file: notify not enabled");
        ai_rec_trans_send_upload_error(UPLOAD_ERR_TRANSPORT);
        return;
    }

    /* Minimum payload: transport(1) + nlen(2) + name(>=1) + off(4) + max(4) = 12 */
    if (plen < 12)
    {
        ai_rec_trans_send_upload_error(UPLOAD_ERR_PERMISSION);
        return;
    }

    uint8_t transport_raw = p[0];
    bool    live_req      = (transport_raw & UPLOAD_TRANSPORT_LIVE_FLAG) != 0;
    uint8_t transport     = (uint8_t)(transport_raw & ~UPLOAD_TRANSPORT_LIVE_FLAG);

    /* Accept BLE (0x01) and current-channel (0x00).
     * WiFi transport (0x03) commands arrive via the WiFi TCP socket
     * and are handled by wifi_file_upload.c - not this BLE path. */
    if (transport != UPLOAD_TRANSPORT_CURRENT && transport != UPLOAD_TRANSPORT_BLE)
    {
        APP_PRINT_WARN1("upload_file: unsupported transport 0x%02x", transport);
        ai_rec_trans_send_upload_error(UPLOAD_ERR_TRANSPORT);
        return;
    }

    uint16_t name_len = (uint16_t)p[1] | ((uint16_t)p[2] << 8);
    if (name_len == 0 || name_len > AI_REC_FILENAME_MAX ||
        plen < (uint16_t)(3 + name_len + 4 + 4))
    {
        ai_rec_trans_send_upload_error(UPLOAD_ERR_PERMISSION);
        return;
    }

    char fname[AI_REC_FILENAME_MAX + 1];
    memcpy(fname, p + 3, name_len);
    fname[name_len] = '\0';

    /* LIVE-bit truth table (see live-tail design x4):
     *
     *   LIVE | name == active recording | action
     *   -----+--------------------------+---------------------------------
     *     1  | yes                      | enter live tail-follow
     *     1  | no / nothing recording   | refuse PERMISSION
     *     0  | yes                      | refuse PERMISSION (fixed-length
     *        |                          |   upload of a growing file is
     *        |                          |   unsafe - host must set LIVE)
     *     0  | no                       | normal fixed-length upload
     *
     * name_is_active also requires is_recording so a stale registered
     * filename can never be mistaken for an active recording. */
    bool name_is_active = (ai_rec_trans.is_recording &&
                           ai_rec_trans.active_record_filename[0] != '\0' &&
                           strcmp(fname, ai_rec_trans.active_record_filename) == 0);

    if (live_req)
    {
        if (!name_is_active)
        {
            APP_PRINT_WARN1("upload_file: LIVE requested but %s is not the "
                            "active recording", TRACE_STRING(fname));
            ai_rec_trans_send_upload_error(UPLOAD_ERR_PERMISSION);
            return;
        }
    }
    else
    {
        if (name_is_active)
        {
            APP_PRINT_WARN1("upload_file: refused, %s is being recorded "
                            "(use LIVE)", TRACE_STRING(fname));
            ai_rec_trans_send_upload_error(UPLOAD_ERR_PERMISSION);
            return;
        }
    }

    uint32_t start_off = ((uint32_t)p[3 + name_len + 0])
                         | ((uint32_t)p[3 + name_len + 1] << 8)
                         | ((uint32_t)p[3 + name_len + 2] << 16)
                         | ((uint32_t)p[3 + name_len + 3] << 24);
    uint32_t host_max  = ((uint32_t)p[3 + name_len + 4])
                         | ((uint32_t)p[3 + name_len + 5] << 8)
                         | ((uint32_t)p[3 + name_len + 6] << 16)
                         | ((uint32_t)p[3 + name_len + 7] << 24);

    APP_PRINT_INFO4("upload_file: name=%s start_off=%d max=%d transport=%d",
                    TRACE_STRING(fname), start_off, host_max, transport);

    /* ---- Open file ---- */
    ai_rec_trans.state = AI_REC_TRANS_OPEN;

    ai_rec_trans.fs_handle = app_fs_open_file(fname, FS_O_READ);
    if (ai_rec_trans.fs_handle == NULL)
    {
        APP_PRINT_ERROR1("upload_file: open fail, name=%s", TRACE_STRING(fname));
        ai_rec_trans_send_upload_error(UPLOAD_ERR_FILE_NOT_FOUND);
        ai_rec_trans.state = AI_REC_TRANS_IDLE;
        return;
    }

    uint32_t total_len = app_fs_size(ai_rec_trans.fs_handle);
    if (start_off > total_len)
    {
        APP_PRINT_ERROR2("upload_file: start_off %d > size %d", start_off, total_len);
        ai_rec_trans_send_upload_error(UPLOAD_ERR_PERMISSION);
        app_fs_close_file(ai_rec_trans.fs_handle);
        ai_rec_trans.fs_handle = NULL;
        ai_rec_trans.state = AI_REC_TRANS_IDLE;
        return;
    }

    /* ---- Whole-file CRC (skipped for LIVE) ----
     *
     *  Live mode streams a growing file, so a whole-file CRC is both
     *  impossible (the file is not yet complete) and wasteful (it would
     *  read the whole current window before the first byte ships).
     *  Integrity is per-chunk only. We still must position the read
     *  cursor at the resume offset, which the CRC pass would otherwise
     *  have done. */
    uint16_t whole_crc = 0;
    if (live_req)
    {
        if (app_fs_seek(ai_rec_trans.fs_handle, start_off) != 0)
        {
            ai_rec_trans_send_upload_error(UPLOAD_ERR_READ_ERROR);
            app_fs_close_file(ai_rec_trans.fs_handle);
            ai_rec_trans.fs_handle = NULL;
            ai_rec_trans.state = AI_REC_TRANS_IDLE;
            return;
        }
    }
    else if (!ai_rec_compute_whole_file_crc(ai_rec_trans.fs_handle, start_off, &whole_crc))
    {
        ai_rec_trans_send_upload_error(UPLOAD_ERR_READ_ERROR);
        app_fs_close_file(ai_rec_trans.fs_handle);
        ai_rec_trans.fs_handle = NULL;
        ai_rec_trans.state = AI_REC_TRANS_IDLE;
        return;
    }

    /* ---- MTU + tool max-chunk negotiation ----
     *
     *  Per-chunk overhead = AI_REC_UPLOAD_HDR_TOTAL (header before data)
     *                     = 17 bytes.
     *
     *  e.g. MTU 247 => 230 audio bytes per chunk; MTU 512 => 495.
     *
     *  Refuse to start the upload if the negotiated MTU is
     too small
     *  to make meaningful progress (audio < 32 B/packet). At MTU=23
     *  the ratio of headers to audio is about 5:1; the host gets a
     *  TRANSPORT error and can re-trigger the MTU exchange. */
    uint16_t mtu = ai_rec_trans_get_mtu();
    if (mtu < AI_REC_MIN_USEFUL_MTU)
    {
        APP_PRINT_ERROR2("upload_file: MTU %d < min useful %d, abort",
                         mtu, (int)AI_REC_MIN_USEFUL_MTU);
        ai_rec_trans_send_upload_error(UPLOAD_ERR_TRANSPORT);
        app_fs_close_file(ai_rec_trans.fs_handle);
        ai_rec_trans.fs_handle = NULL;
        ai_rec_trans.state = AI_REC_TRANS_IDLE;
        return;
    }
    uint16_t mtu_payload = (uint16_t)((mtu - ATT_HEADER_SIZE) - AI_REC_UPLOAD_HDR_TOTAL);
    uint16_t eff_payload = mtu_payload;
    if (host_max > 0 && host_max <= eff_payload)
    {
        eff_payload = (uint16_t)host_max;
    }

    ai_rec_trans.p_chunk_buf = malloc(mtu);
    if (ai_rec_trans.p_chunk_buf == NULL)
    {
        ai_rec_trans_send_upload_error(UPLOAD_ERR_PERMISSION);
        app_fs_close_file(ai_rec_trans.fs_handle);
        ai_rec_trans.fs_handle = NULL;
        ai_rec_trans.state = AI_REC_TRANS_IDLE;
        return;
    }

    /* ---- Commit upload state ---- */
    ai_rec_trans.chunk_buf_size    = mtu;
    ai_rec_trans.file_total_length = total_len;
    ai_rec_trans.start_offset      = start_off;
    ai_rec_trans.cur_offset        = start_off;
    ai_rec_trans.local_seq         = 0;
    ai_rec_trans.whole_file_crc    = whole_crc;
    ai_rec_trans.host_max_chunk    = host_max;
    ai_rec_trans.chunk_payload_max = eff_payload;
    ai_rec_trans.file_format       = ai_rec_format_from_name(fname);
    ai_rec_trans.file_create_ts    = ai_rec_get_now_unix_ts();

    /* Live tail-follow bookkeeping. live_filename is a separate copy
     * because active_record_filename is cleared the moment recording
     * stops, yet we must keep re-opening this file by name to follow its
     * growth and later drain its tail. file_total_length stays the real
     * (open-time) window size set above - never the 0xFFFFFFFF wire
     * sentinel, or `remain` would overflow. */
    ai_rec_trans.live_mode           = live_req;
    ai_rec_trans.recording_finalized = false;
    if (live_req)
    {
        memcpy(ai_rec_trans.live_filename, fname, name_len);
        ai_rec_trans.live_filename[name_len] = '\0';
    }
    else
    {
        ai_rec_trans.live_filename[0] = '\0';
    }

    /* Disable DLPS for the duration of this upload (file data is being
     * streamed over BLE). Re-enabled in close_and_reset() regardless
     * of whether the upload completes, is canceled, or errors. */
    app_dlps_disable(APP_DLPS_ENTER_CHECK_INIT);

    /* Boost CPU + SPIC0 to high performance (200 MHz) for upload
     * throughput. Restored to 40 MHz in close_and_reset(). */
    if (clk_mgr_upload_handle)
    {
        clk_mgr_set_high_performance(clk_mgr_upload_handle);
    }

    /* ---- Apply fast conn params (best effort, LE only) ---- */
    ai_rec_trans_apply_fast_conn_params();

    /* ---- Log throughput estimate for diagnostics ----
     *
     *  Pacing is entirely credit-driven via LE credits
     *  (le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS));
     *  no timer-interval computation needed. */
    ai_rec_trans_log_throughput_est();

    /* ---- Emit start frame ----
     *
     *  Live mode advertises an unknown total: total_len = 0xFFFFFFFF and
     *  crc = 0x0000 are the streaming sentinels. The host derives the
     *  true size from the END frame (END.offset + END.len). */
    if (live_req)
    {
        ai_rec_trans_send_upload_start(0x0000, 0xFFFFFFFF,
                                       (uint8_t)ai_rec_trans.file_format,
                                       ai_rec_trans.file_create_ts);
    }
    else
    {
        ai_rec_trans_send_upload_start(whole_crc, total_len,
                                       (uint8_t)ai_rec_trans.file_format,
                                       ai_rec_trans.file_create_ts);
    }

    /* ---- Begin streaming ----
     *
     *  If a fast-conn-param update was requested, defer the push timer
     *  to a slow poll so push_chunks_burst() waits for the update to
     *  complete before sending the first data chunk.  This avoids the
     *  "GATT notification stuck during connection update" scenario.
     *  If no update is pending (already at target params or non-LE),
     *  arm the push timer at the full-speed interval immediately. */
    ai_rec_trans.state = AI_REC_TRANS_TRANSFERRING;
    ai_rec_trans_arm_watchdog();
    if (ai_rec_trans.conn_update_pending)
    {
        ai_rec_trans_arm_timer_ms(AI_REC_CONN_UPDATE_POLL_MS);
    }
    else
    {
        ai_rec_trans_send_batch();
    }

    APP_PRINT_INFO4("upload_file OPEN ok: total=%d resume=%d crc=0x%04x mtu_pl=%d",
                    total_len, start_off, whole_crc, eff_payload);
}

/**
 * @brief Handle CMD_UPLOAD_CANCEL (0x695). Tears down active transfer.
 *        Always responds with EVT_UPLOAD_CANCEL/result=0x01 so the host
 *        gets a deterministic ack.
 */
static void ai_rec_trans_handle_upload_cancel(uint8_t *p, uint16_t plen)
{
    uint8_t reason = (plen >= 1) ? p[0] : 0;
    APP_PRINT_INFO2("upload_cancel: reason=0x%02x state=%d",
                    reason, ai_rec_trans.state);

    if (ai_rec_trans.state == AI_REC_TRANS_TRANSFERRING ||
        ai_rec_trans.state == AI_REC_TRANS_OPEN ||
        ai_rec_trans.state == AI_REC_TRANS_VERIFY)
    {
        ai_rec_trans_close_and_reset(AI_REC_TRANS_CANCELED);
    }
    /* Spec says result is just success/0x01; we don't differentiate. */
    ai_rec_trans_send_simple_resp(EVT_UPLOAD_CANCEL, 0x01);
}

/*============================================================================*
 *                              Upload streaming
 *============================================================================*/

/**
 * @brief  Live tail-follow window refresh: re-open the growing file to
 *         pick up bytes the recorder appended after our read handle was
 *         opened.
 *
 *         A read-only FATFS handle freezes its size at open; f_sync is a
 *         no-op for it and f_read never returns past that frozen size. A
 *         fresh open reads the current directory-entry size (the writer
 *         refreshes it on every 4 KB f_sync), so we close, re-open, and
 *         seek back to cur_offset. cur_offset is preserved, so streaming
 *         resumes from the exact same byte - nothing is skipped or
 *         duplicated.
 *
 * @return false if the re-open or seek fails (file deleted / FS error);
 *         in that case the transfer is fully torn down (error frame sent
 *         + state reset) before returning.
 */
static bool ai_rec_live_reopen_refresh(void)
{
    if (ai_rec_trans.fs_handle)
    {
        app_fs_close_file(ai_rec_trans.fs_handle);
        ai_rec_trans.fs_handle = NULL;
    }

    ai_rec_trans.fs_handle = app_fs_open_file(ai_rec_trans.live_filename, FS_O_READ);
    if (ai_rec_trans.fs_handle == NULL)
    {
        APP_PRINT_ERROR1("live_reopen: open fail %s",
                         TRACE_STRING(ai_rec_trans.live_filename));
        ai_rec_trans_send_upload_error(UPLOAD_ERR_READ_ERROR);
        ai_rec_trans_close_and_reset(AI_REC_TRANS_ERROR);
        return false;
    }

    if (app_fs_seek(ai_rec_trans.fs_handle, ai_rec_trans.cur_offset) != 0)
    {
        APP_PRINT_ERROR1("live_reopen: seek fail off=%d", ai_rec_trans.cur_offset);
        ai_rec_trans_send_upload_error(UPLOAD_ERR_READ_ERROR);
        ai_rec_trans_close_and_reset(AI_REC_TRANS_ERROR);
        return false;
    }

    ai_rec_trans.file_total_length = app_fs_size(ai_rec_trans.fs_handle);
    return true;
}

/**
 * @brief  Build & send exactly one continue/end frame.
 *
 *         Frame body bytes 0..N (after evt_id prefix):
 *           [flag(1)][seq(2)][crc(2)][offset(4)][len(2)][data...]
 *
 *         Returns false on read error or send failure.
 */
static bool ai_rec_trans_send_one_chunk(void)
{
    if (ai_rec_trans.fs_handle == NULL || ai_rec_trans.p_chunk_buf == NULL)
    {
        return false;
    }

    uint32_t remain = ai_rec_trans.file_total_length - ai_rec_trans.cur_offset;
    if (remain == 0)
    {
        if (ai_rec_trans.live_mode && !ai_rec_trans.recording_finalized)
        {
            /* Caught up to the current readable window but recording is
             * still active. Feed the watchdog (recording alive = healthy)
             * and poll slowly for the writer to extend the file; the next
             * burst's reopen will pick up any new bytes. */
            ai_rec_trans_arm_watchdog();
            ai_rec_trans_arm_timer_ms(AI_REC_LIVE_POLL_INTERVAL_MS);
            return false;
        }
        if (!ai_rec_trans.live_mode)
        {
            return false;  /* fixed-length path: EOF already handled elsewhere */
        }
        /* live_mode && recording_finalized: fall through with want=0 to
         * emit the zero-length END frame that closes the stream. */
    }

    uint16_t want = (remain > ai_rec_trans.chunk_payload_max) ?
                    ai_rec_trans.chunk_payload_max : (uint16_t)remain;
    bool is_last = ai_rec_trans.live_mode
                   ? (ai_rec_trans.recording_finalized &&
                      (ai_rec_trans.cur_offset + want) == ai_rec_trans.file_total_length)
                   : ((ai_rec_trans.cur_offset + want) == ai_rec_trans.file_total_length);

    uint8_t *p           = ai_rec_trans.p_chunk_buf;
    uint8_t *frame       = p + AI_REC_NOTIFY_HDR_LEN;
    uint8_t *p_data_area = p + AI_REC_UPLOAD_HDR_TOTAL;

    if (want > 0)
    {
        int rd = app_fs_read(ai_rec_trans.fs_handle, p_data_area, want);
        if (rd < 0 || (uint16_t)rd != want)
        {
            APP_PRINT_ERROR2("send_one_chunk: read err want=%d got=%d", want, rd);
            ai_rec_trans_send_upload_error(UPLOAD_ERR_READ_ERROR);
            ai_rec_trans_close_and_reset(AI_REC_TRANS_ERROR);
            return false;
        }
    }

    /* ......... Outer header: SINC / Seqn / LENGTH .........
     * LENGTH = op_code(2) + chunk_hdr(11) + data(want) */
    uint16_t length = (uint16_t)(PKT_OP_CODE_LEN + AI_REC_UPLOAD_FRAME_HDR_LEN + want);
    p[0] = PKT_SINC_WORD;
    p[1] = ai_rec_pkt_next_seqn();
    p[2] = (uint8_t)(length & 0xFF);
    p[3] = (uint8_t)((length >> 8) & 0xFF);

    /* ......... op_code (cmd/event id) ......... */
    p[4] = (uint8_t)(EVT_UPLOAD_FILE & 0xFF);
    p[5] = (uint8_t)((EVT_UPLOAD_FILE >> 8) & 0xFF);

    /* ......... chunk frame header at offset AI_REC_NOTIFY_HDR_LEN (=6) ......... */
    frame[0] = is_last ? (uint8_t)UPLOAD_FLAG_END : (uint8_t)UPLOAD_FLAG_CONTINUE;

    frame[1] = (uint8_t)(ai_rec_trans.local_seq & 0xFF);
    frame[2] = (uint8_t)((ai_rec_trans.local_seq >> 8) & 0xFF);

    uint16_t crc = ai_rec_crc16_chunk(p_data_area, want);
    frame[3] = (uint8_t)(crc & 0xFF);
    frame[4] = (uint8_t)((crc >> 8) & 0xFF);

    uint32_t off = ai_rec_trans.cur_offset;
    frame[5] = (uint8_t)(off & 0xFF);
    frame[6] = (uint8_t)((off >> 8) & 0xFF);
    frame[7] = (uint8_t)((off >> 16) & 0xFF);
    frame[8] = (uint8_t)((off >> 24) & 0xFF);

    frame[9]  = (uint8_t)(want & 0xFF);
    frame[10] = (uint8_t)((want >> 8) & 0xFF);

    /* ......... data already read into p_data_area ......... */

    /* No checksum - send header + op_code + frame + data only. */
    uint16_t total = (uint16_t)(PKT_HEADER_LEN + length);   /* = 4 + length = 17 + want */
    bool ok = record_trans_service_send_notification(ai_rec_trans.conn_handle, ai_rec_trans.cid,
                                                     p, total);
    if (!ok)
    {
        APP_PRINT_WARN2("send_one_chunk: GATT TX full, seq=%d off=%lu dropped",
                        ai_rec_trans.local_seq, off);
        /* Do NOT advance local_seq / cur_offset / window - caller will
         * back off and retry the SAME chunk on the next timer fire. */
        return false;
    }

    ai_rec_trans.local_seq++;
    ai_rec_trans.cur_offset += want;

    if (is_last)
    {
        APP_PRINT_INFO2("upload EOF: total=%d crc=0x%04x",
                        ai_rec_trans.file_total_length,
                        ai_rec_trans.whole_file_crc);
        ai_rec_trans.state = AI_REC_TRANS_VERIFY;
        ai_rec_trans_close_and_reset(AI_REC_TRANS_DONE);
        return false;  /* no more pushes */
    }
    return true;
}

/**
 * @brief Send ONE data chunk using available LE credits.
 *
 * Core pacing principle - credit-gated, send-one-at-a-time:
 *
 *   1. Query LE credits via le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS).
 *      If credits == 0, return immediately - the controller TX queue is
 *      full and gatt_svc_send_data() would silently discard the packet.
 *
 *   2. Send exactly ONE chunk (GATT notification per attribute allows
 *      at most one pending notification; a second call while the first
 *      is in flight is guaranteed to return false).
 *
 *   3. SEND_DATA_COMPLETE (delivered to notify_send_complete())
 *      replenishes a credit and calls send_batch() again to send the
 *      next chunk.
 *
 *   This eliminates the push-timer deadlock entirely: if a GATT
 *   notification never completes, no SEND_DATA_COMPLETE fires, no
 *   further chunks are queued, and the watchdog (10 s) eventually
 *   aborts.  There is no timer that re-arms the watchdog while making
 *   no progress.
 *
 *   The credit query is a gate, not a batch-size knob - it prevents
 *   useless SD-card reads and packet assembly when the controller has
 *   no room.  Actual send pacing is entirely event-driven from
 *   SEND_DATA_COMPLETE.
 */
static void ai_rec_trans_send_batch(void)
{
    if (ai_rec_trans.state != AI_REC_TRANS_TRANSFERRING)
    {
        return;
    }

    /* ...... Live tail-follow drain check ...... */
    if (ai_rec_trans.live_mode &&
        ai_rec_trans.cur_offset >= ai_rec_trans.file_total_length)
    {
        if (!ai_rec_live_reopen_refresh())
        {
            return;
        }
    }

    /* ...... Defer until the fast-conn-param update completes ...... */
    if (ai_rec_trans.conn_update_pending)
    {
        uint16_t interval = 0;
        le_get_conn_param(GAP_PARAM_CONN_INTERVAL, &interval,
                          ai_rec_trans.conn_id);
        if (interval >= AI_REC_CONN_INTERVAL_MIN &&
            interval <= AI_REC_CONN_INTERVAL_MAX)
        {
            ai_rec_trans.conn_update_pending = false;
            APP_PRINT_INFO1("send_batch: conn_update done, interval=%d",
                            interval);
        }
        else
        {
            APP_PRINT_INFO1("send_batch: conn_update pending, "
                            "curr_interval=%d", interval);
            ai_rec_trans_arm_timer_ms(AI_REC_CONN_UPDATE_POLL_MS);
            return;
        }
    }

    /* ...... Gate: only proceed when the controller has TX credits ......
     *
     *  le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS) returns the number
     *  of available TX buffers in the BLE controller.  When credits == 0
     *  the controller queue is full and gatt_svc_send_data() will either
     *  return false or (worse) return true but silently drop the packet
     *  because the BTIF command queue cannot accept it.  To avoid silent
     *  data loss we gate here and wait for SEND_DATA_COMPLETE to replenish
     *  a credit before trying again. */
    uint8_t credits = 0;
    le_get_gap_param(GAP_PARAM_LE_REMAIN_CREDITS, &credits);

    if (credits == 0)
    {
        return;
    }

    /* ...... Send up to 'credits' chunks ......
     *
     *  With gatt_svc_cfg_pending_num default notify_num=10, the GATT
     *  service can queue up to 10 notifications.  Each notification
     *  carries exactly (ATT_MTU - ATT_HEADER_SIZE) bytes of attribute
     *  value (= 244 for MTU=247), which fits in a single LL packet
     *  without ATT-layer fragmentation.
     *
     *  The credit query fills the pipeline: send 'credits' chunks,
     *  each consuming one credit.  SEND_DATA_COMPLETE returns one
     *  credit and triggers the next send_batch() to keep the pipeline
     *  topped up.
     *
     *  NOTE: retransmission window and CMD_UPLOAD_ACK have been removed
     *  because the phone's cumulative ACK (seq=N) and the device's
     *  retransmission logic were found to conflict: the phone might ACK
     *  seq=32 (meaning chunks 0-32 received) and expect seq=33 as the
     *  next NEW chunk, but the device would retransmit seq=33 because
     *  it was still in the unACKed window.  Removing the window means
     *  the device always sends forward-only chunks driven purely by
     *  credit availability and SEND_DATA_COMPLETE - no retransmission
     *  confusion. */
    bool progress = false;

    for (uint8_t i = 0; i < credits; i++)
    {
        if (ai_rec_trans_send_one_chunk())
        {
            progress = true;
        }
    }

    if (progress)
    {
        ai_rec_trans_arm_watchdog();
    }
}

/*============================================================================*
 *                        Send-Complete Notification
 *============================================================================*/

/*============================================================================*
 *                        Send-Complete Notification
 *============================================================================*/

void ai_rec_trans_notify_send_complete(uint16_t service_id)
{
    if (service_id != rt_srv_id_local)
    {
        return;
    }

    if (ai_rec_trans.state == AI_REC_TRANS_TRANSFERRING)
    {
        /* A SEND_DATA_COMPLETE event means a GATT notification was
         * fully transmitted - real progress. Re-arm the watchdog so
         * the 10 s timeout is measured from the LAST completed send,
         * then try to send the next chunk (which may gate on credits). */
        ai_rec_trans_arm_watchdog();
        ai_rec_trans_send_batch();
    }
}

/*============================================================================*
 *                              Timers
 *============================================================================*/

static void ai_rec_trans_arm_timer_ms(uint32_t interval_ms)
{
    app_stop_timer(&timer_id_poll);
    app_start_timer(&timer_id_poll, "ai_rec_poll",
                    ai_rec_trans_timer_id, AI_REC_TIMER_POLL, 0, false,
                    interval_ms);
}

static void ai_rec_trans_arm_watchdog(void)
{
    app_stop_timer(&timer_id_trans_watchdog);
    app_start_timer(&timer_id_trans_watchdog, "ai_rec_wd",
                    ai_rec_trans_timer_id, AI_REC_TIMER_TRANS_WATCHDOG, 0, false,
                    AI_REC_TRANS_WATCHDOG_MS);
}

static void ai_rec_trans_disarm_watchdog(void)
{
    app_stop_timer(&timer_id_trans_watchdog);
}

static void ai_rec_trans_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    switch (timer_evt)
    {
    case AI_REC_TIMER_POLL:
        app_stop_timer(&timer_id_poll);
        ai_rec_trans_send_batch();
        break;

    case AI_REC_TIMER_TRANS_WATCHDOG:
        APP_PRINT_WARN0("ai_rec_trans: watchdog fired, abort");
        app_stop_timer(&timer_id_trans_watchdog);
        if (ai_rec_trans.state == AI_REC_TRANS_TRANSFERRING ||
            ai_rec_trans.state == AI_REC_TRANS_OPEN ||
            ai_rec_trans.state == AI_REC_TRANS_VERIFY)
        {
            ai_rec_trans_send_upload_error(UPLOAD_ERR_TRANSPORT);
            ai_rec_trans_close_and_reset(AI_REC_TRANS_ERROR);
        }
        break;

    default:
        break;
    }
}

/*============================================================================*
 *                              Command Dispatcher
 *============================================================================*/

T_APP_RESULT app_ai_record_file_handle_cp_req(uint8_t conn_id, uint16_t conn_handle,
                                              uint16_t cid,
                                              T_GAP_CHANN_TYPE chann_type,
                                              uint16_t length, uint8_t *p_value)
{
    /* Minimum packet: SINC(1) + Seqn(1) + LENGTH(2) + op_code(2) = 6 */
    if (p_value == NULL ||
        length < (uint16_t)(PKT_HEADER_LEN + PKT_OP_CODE_LEN))
    {
        APP_PRINT_ERROR1("cp_req: too short, len=%d", length);
        return APP_RESULT_INVALID_PDU;
    }

    /* ......... Outer header validation ......... */
    if (p_value[0] != PKT_SINC_WORD)
    {
        APP_PRINT_ERROR1("cp_req: bad SINC 0x%02x", p_value[0]);
        return APP_RESULT_INVALID_PDU;
    }

    uint8_t  pkt_seqn = p_value[1];
    uint16_t pkt_len  = (uint16_t)p_value[2] | ((uint16_t)p_value[3] << 8);
    uint32_t actual_mhz = 40;

    /* total wire bytes must equal 4 (header) + LENGTH */
    if (length != (uint16_t)(PKT_HEADER_LEN + pkt_len))
    {
        APP_PRINT_ERROR2("cp_req: len mismatch declared=%d wire=%d",
                         pkt_len, length);
        return APP_RESULT_INVALID_PDU;
    }
    if (pkt_len < PKT_OP_CODE_LEN)
    {
        APP_PRINT_ERROR1("cp_req: LENGTH too small=%d", pkt_len);
        return APP_RESULT_INVALID_PDU;
    }

    /* ......... Extract op_code and parameters ......... */
    uint16_t cmd_id = (uint16_t)p_value[4] | ((uint16_t)p_value[5] << 8);
    uint8_t *p      = p_value + AI_REC_NOTIFY_HDR_LEN;       /* params start at 6 */
    uint16_t plen   = (uint16_t)(pkt_len - PKT_OP_CODE_LEN); /* params length */

    /* Memorize link for notify dispatch. If we reach here the phone has
     * successfully written a command to our service, which implies a valid
     * BLE session. The CCCD callback may not fire reliably for all
     * Write-Request-with-Response writes from the phone (GATT stack may
     * handle CCCD writes internally without calling our cccd_cb). So use
     * every valid write as a fallback to arm notify_enabled. */
    ai_rec_trans.conn_id           = conn_id;
    ai_rec_trans.conn_handle       = conn_handle;
    ai_rec_trans.cid               = cid;
    ai_rec_trans.chann_type        = chann_type;
    ai_rec_trans.notify_enabled    = true;

    APP_PRINT_INFO4("cmd: id=0x%04x seqn=%d plen=%d state=%d",
                    cmd_id, pkt_seqn, plen, ai_rec_trans.state);

    /* Refresh watchdog whenever host shows a sign of life during a
     * live transfer. */
    if (ai_rec_trans.state == AI_REC_TRANS_TRANSFERRING ||
        ai_rec_trans.state == AI_REC_TRANS_OPEN)
    {
        ai_rec_trans_arm_watchdog();
    }

    /* Disable DLPS during any BLE CMD/EVENT interaction to prevent
     * deep sleep from disrupting the protocol handshake.
     *
     * CMD_WIFI_CONNECT and CMD_WIFI_DISCONNECT are async - the handler
     * returns before the operation completes.  If a WiFi async lifecycle
     * is in progress (any terminal state not yet reached), the
     * app_dlps_enable() below is skipped and left to the WiFi AT callback
     * (ai_rec_trans_wifi_atcmd_cb) which fires when the connect/disconnect
     * reaches CONNECTED / FAIL / DISCONNECTED.  The same guard also
     * prevents an unrelated synchronous command (e.g. CMD_EVENT_ACK) from
     * inadvertently clearing the INIT bit while the WiFi lifecycle is
     * still active. */
    app_dlps_disable(APP_DLPS_ENTER_CHECK_INIT);

    switch (cmd_id)
    {
    /* --------- Host ACK (silently consumed) --------- */
    case CMD_EVENT_ACK:
        ai_rec_trans_handle_event_ack(p, plen);
        break;

    /* --------- Spec'd commands --------- */
    case CMD_QUERY_INFO:
        ai_rec_trans_handle_query_info(p, plen);
        break;

    case CMD_UPLOAD_FILE:
        pm_cpu_freq_set(200, &actual_mhz);
        app_bt_policy_enter_pairing_mode(false, false);
        ai_rec_trans_handle_upload_file(p, plen);
        break;

    case CMD_UPLOAD_CANCEL:
        pm_cpu_freq_set(40, &actual_mhz);
        app_bt_policy_enter_pairing_mode(false, true);
        ai_rec_trans_handle_upload_cancel(p, plen);
        break;

    case CMD_SCAN_FILES:
        ai_rec_trans_handle_scan_files(p, plen);
        break;



    case CMD_WIFI_CONNECT:
        pm_cpu_freq_set(200, &actual_mhz);
        app_bt_policy_enter_pairing_mode(false, false);
        ai_rec_trans_handle_wifi_connect(p, plen);
        break;

    case CMD_WIFI_DISCONNECT:
        pm_cpu_freq_set(40, &actual_mhz);
        app_bt_policy_enter_pairing_mode(false, true);
        ai_rec_trans_handle_wifi_disconnect(p, plen);
        break;

    case CMD_WIFI_GET_STATUS:
        ai_rec_trans_handle_wifi_get_status(p, plen);
        break;

    default:
        APP_PRINT_WARN1("unknown cmd_id 0x%04x", cmd_id);
        ai_rec_trans_send_simple_resp(cmd_id, 0x02);
        break;
    }

    /* Re-enable DLPS after synchronous command processing, UNLESS a
     * WiFi async lifecycle (connect/disconnect) is still in progress,
     * or a BLE upload is active, or a WiFi upload/scan is in progress.
     * In those cases the lifecycle owner (WiFi AT callback or
     * close_and_reset or wifi_file_upload tick) re-enables DLPS when
     * the operation reaches a terminal state. */
    if (ai_rec_trans.wifi_state != WIFI_STATE_CONNECTING &&
        ai_rec_trans.wifi_state != WIFI_STATE_ATPS_PENDING &&
        ai_rec_trans.wifi_state != WIFI_STATE_ATPI_PENDING &&
        ai_rec_trans.wifi_state != WIFI_STATE_DISCONNECTING &&
        ai_rec_trans.state != AI_REC_TRANS_OPEN &&
        ai_rec_trans.state != AI_REC_TRANS_TRANSFERRING &&
        ai_rec_trans.state != AI_REC_TRANS_VERIFY
#if F_APP_WIFI_UART_CMD
        && !wifi_file_upload_is_busy()
#elif F_APP_WIFI_SPI_CMD
        && !spi_file_upload_is_busy()
#endif
       )
    {
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
    }

    return APP_RESULT_SUCCESS;
}

/*============================================================================*
 *                              Public APIs
 *============================================================================*/

void app_ai_record_file_trans_init(void)
{
    memset(&ai_rec_trans, 0, sizeof(ai_rec_trans));
    ai_rec_trans.state = AI_REC_TRANS_IDLE;
    if (ai_rec_trans_timer_id == 0)
    {
        app_timer_reg_cb(ai_rec_trans_timeout_cb, &ai_rec_trans_timer_id);
    }

    /* Create clk_mgr user for boosting CPU+SPIC0 during file upload.
     * Only these two clock domains matter for upload throughput;
     * others (SPIC1/SPIC2/SPIC3) are left at their normal rate. */
    if (clk_mgr_upload_handle == NULL)
    {
        U_CLK_BITMAP bitmap;
        bitmap.data = BIT(T_CLK_TYPE_CPU) | BIT(T_CLK_TYPE_SPIC0);
        clk_mgr_upload_handle = clk_mgr_user_create("ai_rec_upload", bitmap);
    }

    /* Register the AT-event callback so async WiFi connect/disconnect events
     * surface as spec EVT_WIFI_* notifies. The wifi_transport layer broadcasts
     * to all registered listeners, so each module registers independently. */
#if (F_APP_WIFI_UART_CMD || F_APP_WIFI_SPI_CMD)
    wifi_transport_register_callback((wifi_at_evt_cb_t)ai_rec_trans_wifi_atcmd_cb);
#endif

#if F_APP_WIFI_UART_CMD
    /* Send the cold-start ATPN from the main app task (this callback runs there
     * via IO_WIFI_UART_SEND_CONNECT), matching the proven 0x8440 demo context
     * instead of the WiFi task. See ai_rec_trans_wifi_send_connect_deferred(). */
    app_wifi_uart_deferred_connect_register(ai_rec_trans_wifi_send_connect_deferred);
#endif

    APP_PRINT_INFO0("app_ai_record_file_trans_init: ok");
}

bool app_ai_record_file_trans_is_busy(void)
{
    return (ai_rec_trans.state == AI_REC_TRANS_TRANSFERRING ||
            ai_rec_trans.state == AI_REC_TRANS_OPEN ||
            ai_rec_trans.state == AI_REC_TRANS_VERIFY);
}

void app_ai_record_file_trans_cancel(void)
{
    if (app_ai_record_file_trans_is_busy())
    {
        ai_rec_trans_send_upload_error(UPLOAD_ERR_TRANSPORT);
        ai_rec_trans_close_and_reset(AI_REC_TRANS_CANCELED);
    }
}

void app_ai_record_file_trans_on_cccd(uint8_t conn_id, uint16_t conn_handle,
                                      uint16_t cid, uint16_t chann_type,
                                      bool notify_enabled)
{
    ai_rec_trans.conn_id        = conn_id;
    ai_rec_trans.conn_handle    = conn_handle;
    ai_rec_trans.cid            = cid;
    ai_rec_trans.chann_type     = (T_GAP_CHANN_TYPE)chann_type;
    ai_rec_trans.notify_enabled = notify_enabled;

    if (!notify_enabled)
    {
        /* Host disabled notify (or link dropped). Treat this as the
         * end of a BLE session: clear the QUERY_INFO handshake so the
         * next session must re-handshake before any upload. */
        ai_rec_trans.query_info_received = false;
        APP_PRINT_INFO0("on_cccd: notify disabled, query_info handshake cleared");

        if (app_ai_record_file_trans_is_busy())
        {
            APP_PRINT_WARN0("ai_rec_trans: notify disabled mid-transfer, abort");
            ai_rec_trans_close_and_reset(AI_REC_TRANS_CANCELED);
        }
    }
}

void app_ai_record_file_trans_set_recording(bool recording)
{
    ai_rec_trans.is_recording = recording;

    /* Live tail-follow finalize: if recording of the file we are live-
     * streaming has just stopped, mark the size as final and kick the
     * push timer once at the fast cadence. The drain loop then reopens
     * (picking up the writer's final flushed bytes), drains the tail,
     * and emits the END frame. */
    if (!recording && ai_rec_trans.live_mode &&
        ai_rec_trans.state == AI_REC_TRANS_TRANSFERRING)
    {
        ai_rec_trans.recording_finalized = true;
        ai_rec_trans_send_batch();
    }

    /* Concurrent recording + upload is supported.
     *
     *  Mic capture (mic => Opus encoder => SD write) and upload streaming
     *  (SD read => BLE notify) are independent pipelines that share only
     *  the SD bus and CPU. FATFS serializes the per-handle I/O, and the
     *  CPU has enough headroom to run both at our 60-120 KB/s upload
     *  rates plus the comparatively cheap Opus encoder.
     *
     *  We therefore do NOT abort an in-flight upload when recording
     *  starts. The flag is kept only for status / diagnostics so this
     *  module knows the SD is "shared" and could be used in future to:
     *    - de-prioritize push timer if CPU saturates
     *    - refuse upload of the file currently being recorded
     *      (requires recording side to register its filename)
     *
     *  See handle_upload_file for the same-file race note. */
    APP_PRINT_INFO2("set_recording=%d (transfer state=%d)",
                    recording, ai_rec_trans.state);
}

void app_ai_record_file_trans_set_active_record_file(const char *filename)
{
    if (filename == NULL || filename[0] == '\0')
    {
        ai_rec_trans.active_record_filename[0] = '\0';
        APP_PRINT_INFO0("active_record_file: cleared");
        return;
    }

    /* Truncating snprintf - anything longer than AI_REC_FILENAME_MAX is
     * already untransferable on the wire anyway (see handle_upload_file
     * length checks), so silently capping is fine. */
    size_t n = strlen(filename);
    if (n > AI_REC_FILENAME_MAX)
    {
        n = AI_REC_FILENAME_MAX;
    }
    memcpy(ai_rec_trans.active_record_filename, filename, n);
    ai_rec_trans.active_record_filename[n] = '\0';

    APP_PRINT_INFO1("active_record_file: %s",
                    TRACE_STRING(ai_rec_trans.active_record_filename));
}

#endif /* CONFIG_REALTEK_APP_AI_RECORD */
