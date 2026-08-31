/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AI_RECORD_FILE_TRANS_H_
#define _APP_AI_RECORD_FILE_TRANS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "gap.h"
#include "profile_server_ext.h"

/** @defgroup AI_RECORD_FILE_TRANS AI Record File Transfer (BLE-only)
  *
  * @brief  One-Command + Multi-Event upload protocol per Songs_Transfer
  *         User Guide V4.1 x8 (CMD_UPLOAD_FILE = 0x694).
  *
  *         Wire format on Write 0xBA26 (host to Record_pen):
  *
  *             [cmd_id(2B, LE)] [payload ... ]
  *
  *         Wire format on Notify 0xBA27 (Record_pen to host):
  *
  *             [evt_id(2B, LE)] [payload ... ]
  *
  *         For EVT_UPLOAD_FILE the first payload byte is the Flag, and
  *         subsequent fields depend on the flag (see UPLOAD_FLAG_* below).
  * @{
  */

/*============================================================================*
 *                              Compile-time tunables
 *============================================================================*/

/** @brief Maximum filename length we accept. */
#define AI_REC_FILENAME_MAX                 64

/*============================================================================*
 *                              Outer Packet Framing  (per spec table)
 *============================================================================*
 *
 *  All Write/Notify payloads share the same outer wrapper:
 *
 *     ...................................................
 *     | SINC | Seqn | LENGTH   | op_code  | parameters  |
 *     | 0xAA |  1B  |   2B LE  |  2B LE   | LENGTH-2 B  |
 *     ...................................................
 *      byte0  byte1   2..3       4..5       6..(LEN+3)
 *
 *      LENGTH = op_code(2) + parameters_len
 *      Total wire bytes = 4 (header) + LENGTH = LENGTH + 4
 *
 *  Checksum has been removed from the spec table.
 */

#define PKT_SINC_WORD                       0xAA
#define PKT_HEADER_LEN                      4   /**< SINC + Seqn + LENGTH */
#define PKT_OP_CODE_LEN                     2   /**< cmd_id / evt_id */
#define PKT_OUTER_OVERHEAD                  PKT_HEADER_LEN                     /* 4 */

/** @brief Per-notify "fixed" header preceding business payload:
 *         outer header (4) + op_code (2) = 6 bytes before parameters. */
#define AI_REC_NOTIFY_HDR_LEN               (PKT_HEADER_LEN + PKT_OP_CODE_LEN)  /* 6 */

/** @brief EVT_UPLOAD_FILE chunk frame header (inside parameters):
 *         flag(1) + seq(2) + crc(2) + offset(4) + len(2) = 11 bytes. */
#define AI_REC_UPLOAD_FRAME_HDR_LEN         (1 + 2 + 2 + 4 + 2)  /* 11 */

/** @brief Total bytes occupied by header layers before audio data
 *         (outer + op_code + chunk header). No trailer - checksum
 *         has been removed. */
#define AI_REC_UPLOAD_HDR_TOTAL             (AI_REC_NOTIFY_HDR_LEN + \
                                             AI_REC_UPLOAD_FRAME_HDR_LEN)  /* 17 */

#define IC_TYPE_REC_PEN               0x13

/*============================================================================*
 *                              Spec'd Command / Event IDs
 *
 *  Two independent ID namespaces live on the same Write/Notify wire:
 *
 *    Direction           ID Namespace        Channel
 *    -----------------   -----------------   --------------------------
 *    Phone to Record_pen      CMD_*  (request)    Write  0xCA10 ([cmd_id][...])
 *    Record_pen to Phone      EVT_*  (response)   Notify 0xCA11 ([evt_id][...])
 *
 *  CMD and EVT id values may legitimately overlap in numeric value
 *  (e.g. CMD_UPLOAD_FILE = 0x0694 and EVT_UPLOAD_CANCEL = 0x0694)
 *  because the direction of the GATT operation already disambiguates
 *  them - the receiver knows whether it sees a Write or a Notify.
 *
 *  These four IDs come straight from Songs_Transfer User Guide V4.1 x8.2.
 *============================================================================*/

/* Phone to Record_pen: control-point commands */
#define CMD_EVENT_ACK                       0x0000  /**< host ACK for received EVT  */
#define CMD_QUERY_INFO                      0x0680  /**< spec V4.1 (no params) */
#define CMD_WIFI_CONNECT                    0x0691  /**< spec V4.1 x7.2 */
#define CMD_WIFI_DISCONNECT                 0x0692  /**< spec V4.1 x7.3 */
#define CMD_WIFI_GET_STATUS                 0x0693  /**< spec V4.1 x7.4 */
#define CMD_UPLOAD_FILE                     0x0694  /**< spec x8.2 */
#define CMD_UPLOAD_CANCEL                   0x0695  /**< spec x8.3 */
#define CMD_SCAN_FILES                      0x0696  /**< spec V4.1 x9.2 */

/* Record_pen to Phone: notify events */
#define EVT_QUERY_INFO                      0x0680  /**< spec V4.1 (32B device info) */
#define EVT_WIFI_CONNECT                    0x0690  /**< spec V4.1 x7.2 */
#define EVT_WIFI_DISCONNECT                 0x0691  /**< spec V4.1 x7.3 */
#define EVT_WIFI_GET_STATUS                 0x0692  /**< spec V4.1 x7.4 */
#define EVT_UPLOAD_FILE                     0x0693  /**< spec x8.2 (Multi-Event Flag) */
#define EVT_UPLOAD_CANCEL                   0x0694  /**< spec x8.3 */
#define EVT_SCAN_FILES                      0x0695  /**< spec V4.1 x9.2 (Multi-Event Flag) */

/*============================================================================*
 *                              EVT_UPLOAD_FILE Flags
 *============================================================================*/

/** @brief First byte of every EVT_UPLOAD_FILE notify body. */
typedef enum
{
    UPLOAD_FLAG_START      = 0x00,  /**< followed by file info */
    UPLOAD_FLAG_CONTINUE   = 0x01,  /**< intermediate data chunk */
    UPLOAD_FLAG_END        = 0x02,  /**< final data chunk */
    UPLOAD_FLAG_ERROR      = 0xFE,  /**< followed by 1B error code */
} T_AI_REC_UPLOAD_FLAG;

/** @brief Error codes used in UPLOAD_FLAG_ERROR frames. */
typedef enum
{
    UPLOAD_ERR_FILE_NOT_FOUND  = 0x10,
    UPLOAD_ERR_READ_ERROR      = 0x11,
    UPLOAD_ERR_PERMISSION      = 0x12,  /**< busy / recording / not allowed */
    UPLOAD_ERR_TRANSPORT       = 0x13,
} T_AI_REC_UPLOAD_ERR;

/** @brief CMD_UPLOAD_FILE Byte0 transport selector. */
typedef enum
{
    UPLOAD_TRANSPORT_CURRENT = 0x00,
    UPLOAD_TRANSPORT_BLE     = 0x01,
    UPLOAD_TRANSPORT_SPP     = 0x02,
    UPLOAD_TRANSPORT_WIFI    = 0x03,
} T_AI_REC_UPLOAD_TRANSPORT;

/** @brief CMD_UPLOAD_FILE Byte0 LIVE modifier bit (OR'd onto the
 *         transport selector). When set, the host requests a live
 *         tail-follow upload of the file currently being recorded: the
 *         device streams the file as it grows and only sends
 *         UPLOAD_FLAG_END after recording stops.
 *
 *         e.g. "BLE + live" = UPLOAD_TRANSPORT_BLE | UPLOAD_TRANSPORT_LIVE_FLAG = 0x81.
 *
 *         The firmware strips this bit before resolving the link, so the
 *         on-wire transport values and frame layout are unchanged.
 *
 *         START-frame sentinels in live mode: total_len = 0xFFFFFFFF and
 *         crc = 0x0000 (unknown total / streaming). The host derives the
 *         final size from END.offset + END.len. */
#define UPLOAD_TRANSPORT_LIVE_FLAG          0x80

/** @brief CMD_UPLOAD_CANCEL Byte0 reason. */
typedef enum
{
    UPLOAD_CANCEL_REASON_USER         = 0x01,
    UPLOAD_CANCEL_REASON_BUFFER_FULL  = 0x02,
    UPLOAD_CANCEL_REASON_VERIFY_FAIL  = 0x03,
} T_AI_REC_UPLOAD_CANCEL_REASON;

/*============================================================================*
 *                              CMD_SCAN_FILES Definitions  (spec V4.1 x9)
 *============================================================================*/

/** @brief First byte of every EVT_SCAN_FILES notify body. */
typedef enum
{
    SCAN_FLAG_START   = 0x00,
    SCAN_FLAG_ENTRY   = 0x01,
    SCAN_FLAG_END     = 0x02,
    SCAN_FLAG_ERROR   = 0xFE,
} T_AI_REC_SCAN_FLAG;

/** @brief CMD_SCAN_FILES Byte1 storage selector. */
typedef enum
{
    SCAN_STORAGE_ALL   = 0x00,
    SCAN_STORAGE_FLASH = 0x01,
    SCAN_STORAGE_SD    = 0x02,
} T_AI_REC_SCAN_STORAGE;

/** @brief Error codes used in EVT_SCAN_FILES Flag=0xFE frames. */
typedef enum
{
    SCAN_ERR_STORAGE_NOT_MOUNTED = 0x10,
    SCAN_ERR_PERMISSION_DENIED   = 0x11,
    SCAN_ERR_INTERNAL_ERROR      = 0x12,
} T_AI_REC_SCAN_ERR;

/** @brief Default batch cap if Tool sends 0 in CMD_SCAN_FILES Byte2. */
#define AI_REC_SCAN_DEFAULT_BATCH           16

/** @brief Hard upper bound on entries we ever emit per scan invocation. */
#define AI_REC_SCAN_MAX_BATCH               256

/** @brief Byte7 of UPLOAD_FLAG_START frame: file_format_t.
 *
 *  0xFF means no filter - all file types are supported.
 */
typedef enum
{
    AI_REC_FORMAT_MP3    = 0x00,
    AI_REC_FORMAT_MP4    = 0x01,
    AI_REC_FORMAT_RTK    = 0x02,
    AI_REC_FORMAT_AAC    = 0x03,
    AI_REC_FORMAT_FLAC   = 0x04,
    AI_REC_FORMAT_TXT    = 0x05,
    AI_REC_FORMAT_DAT    = 0x06,
    AI_REC_FORMAT_BIN    = 0x07,
    AI_REC_FORMAT_OPUS   = 0x08,
    AI_REC_FORMAT_ALL    = 0xFF,
} T_AI_REC_FILE_FORMAT;

/*============================================================================*
 *                              CMD/EVT_QUERY_INFO Definitions  (V4.1)
 *============================================================================*/

/** @brief Protocol version reported in EVT_QUERY_INFO. */
#define AI_REC_PROTOCOL_VER_V41             5

/** @brief EVT_QUERY_INFO Byte7 song_format_type bitmask. */
#define QUERY_FMT_AAC                       (1u << 0)
#define QUERY_FMT_MP3                       (1u << 1)
#define QUERY_FMT_FLAC                      (1u << 2)
#define QUERY_FMT_WAV                       (1u << 3)
#define QUERY_FMT_OPUS                      (1u << 4)

/** @brief EVT_QUERY_INFO Byte5 mode bitfield. */
#define QUERY_MODE_RWS_COUPLE               (1u << 0)  /**< 0=single, 1=couple */
#define QUERY_MODE_BUD_PRIMARY              (1u << 1)  /**< 0=secondary, 1=primary */

/** @brief EVT_QUERY_INFO Byte8 transport_cap bitmask (V4.1). */
#define QUERY_TRANSPORT_CAP_BLE             (1u << 0)
#define QUERY_TRANSPORT_CAP_SPP             (1u << 1)
#define QUERY_TRANSPORT_CAP_WIFI            (1u << 2)

/** @brief EVT_QUERY_INFO Byte9 active_transport (V4.1). */
typedef enum
{
    QUERY_ACTIVE_TRANSPORT_BLE  = 0x00,
    QUERY_ACTIVE_TRANSPORT_SPP  = 0x01,
    QUERY_ACTIVE_TRANSPORT_WIFI = 0x02,
} T_AI_REC_ACTIVE_TRANSPORT;

/** @brief EVT_QUERY_INFO payload (32 bytes, packed).
 *
 *  Byte 0..1   packet_size       - max audio bytes per upload chunk
 *  Byte 2..3   buffer_check_size - host-side flow-control hint
 *  Byte 4      protocol_ver      - 5 for V4.1
 *  Byte 5      mode              - RWS/role bits (Recording-pen always 0)
 *  Byte 6      ic_type           - IC_TYPE
 *  Byte 7      song_format_type  - bitmask (QUERY_FMT_*)
 *  Byte 8      transport_cap     - bitmask (V4.1)
 *  Byte 9      active_transport  - current channel (V4.1)
 *  Byte 10..31 reserved          - must be zero
 */
typedef struct
{
    uint16_t packet_size;
    uint16_t buffer_check_size;
    uint8_t  protocol_ver;
    uint8_t  mode;
    uint8_t  ic_type;
    uint8_t  song_format_type;
    uint8_t  transport_cap;
    uint8_t  active_transport;
    uint8_t  rsv[22];
} __attribute__((packed)) T_QUERY_INFO_EVT;

/*============================================================================*
 *                              CMD_WIFI_*  Definitions  (spec V4.1 x7)
 *============================================================================*
 *
 *  Three control-plane commands for WiFi management. Recording-pen has
 *  an external WiFi MCU reached over UART AT commands (see
 *  applications/.../wifi/app_uart_atcmd.h); the file_trans handlers
 *  translate spec frames into AT calls and AT events back into spec
 *  EVT frames asynchronously.
 *
 *  KNOWN LIMITATION: this is control-plane only. The TCP file-transfer
 *  server promised by EVT_WIFI_CONNECT.port is NOT implemented yet -
 *  the reported port is reserved for a future server. Host should not
 *  attempt WiFi-channel file transfer until that arrives. transport_cap
 *  in EVT_QUERY_INFO is therefore left BLE-only for now.
 */

/** @brief WiFi result codes (EVT_WIFI_CONNECT / DISCONNECT byte 0). */
typedef enum
{
    WIFI_RESULT_SUCCESS             = 0x01,
    WIFI_RESULT_AP_NOT_FOUND        = 0x10,
    WIFI_RESULT_AUTH_FAIL           = 0x11,
    WIFI_RESULT_DHCP_FAIL           = 0x12,
    WIFI_RESULT_TIMEOUT             = 0x13,
    WIFI_RESULT_HARDWARE_ERROR      = 0x14,
    WIFI_RESULT_ALREADY_CONNECTED   = 0x15,
    WIFI_RESULT_NOT_SUPPORTED       = 0x16,
} T_AI_REC_WIFI_RESULT;

/** @brief WiFi state machine (EVT_WIFI_GET_STATUS byte 0).
 *         Initial value is DISCONNECTED (0x00) - works with the bss
 *         memset-zero initialisation of T_AI_REC_TRANS_DB. */
typedef enum
{
    WIFI_STATE_DISCONNECTED  = 0x00,
    WIFI_STATE_CONNECTING    = 0x01,
    WIFI_STATE_CONNECTED     = 0x02,
    WIFI_STATE_DISCONNECTING = 0x03,
    WIFI_STATE_FAIL          = 0x04,
    WIFI_STATE_ATPS_PENDING  = 0x05,  /**< ATPN OK received; waiting for ATPS */
    WIFI_STATE_ATPI_PENDING  = 0x06,  /**< ATPS OK received; waiting for ATPI */
} T_AI_REC_WIFI_STATE;

/** @brief Spec-imposed length caps. */
#define AI_REC_WIFI_SSID_MAX                32
#define AI_REC_WIFI_PASS_MAX                64

/*============================================================================*
 *                              State Machine
 *============================================================================*/

typedef enum
{
    AI_REC_TRANS_IDLE           = 0,
    AI_REC_TRANS_OPEN           = 1,  /**< file opened, CRC computing or done */
    AI_REC_TRANS_TRANSFERRING   = 2,  /**< streaming chunks */
    AI_REC_TRANS_VERIFY         = 3,  /**< end frame sent, awaiting host verdict */
    AI_REC_TRANS_DONE           = 4,
    AI_REC_TRANS_CANCELED       = 5,
    AI_REC_TRANS_ERROR          = 6,
} T_AI_REC_TRANS_STATE;

/*============================================================================*
 *                              Public APIs
 *============================================================================*/

/**
 * @brief  Initialize timers + reset state. Call once at boot.
 *         Does NOT register the GATT service; that is done from
 *         app_ai_record_init() via record_trans_reg_srv().
 */
void app_ai_record_file_trans_init(void);

/**
 * @brief  Forward a Write request from the GATT service layer.
 *
 * @param  conn_id      LE conn id; 0xFF for BR/EDR.
 * @param  conn_handle  ACL handle.
 * @param  cid          ATT channel id.
 * @param  chann_type   Resolved transport (T_GAP_CHANN_TYPE).
 * @param  length       Total length of p_value (>= 2).
 * @param  p_value      [cmd_id(2B LE)][payload].
 *
 * @return APP_RESULT_SUCCESS unless GATT layer must signal protocol error.
 */
T_APP_RESULT app_ai_record_file_handle_cp_req(uint8_t conn_id, uint16_t conn_handle,
                                              uint16_t cid,
                                              T_GAP_CHANN_TYPE chann_type,
                                              uint16_t length, uint8_t *p_value);

/**
 * @brief  True iff we are currently uploading a file.
 *         Other modules use this to avoid concurrent SD access.
 */
bool app_ai_record_file_trans_is_busy(void);

/**
 * @brief  Local abort (e.g. on disconnect). Safe to call when idle.
 */
void app_ai_record_file_trans_cancel(void);

/**
 * @brief  Tell the module about CCCD enable/disable. The module gates
 *         its first push on notify being enabled.
 */
void app_ai_record_file_trans_on_cccd(uint8_t conn_id, uint16_t conn_handle,
                                      uint16_t cid, uint16_t chann_type,
                                      bool notify_enabled);

/**
 * @brief  Notify the module that a GATT notification send has completed
 *         (or failed) for the given service_id.  The module uses this to
 *         un-stall its push timer when the GATT TX queue had been full.
 *
 *         Safe to call from any context (timer, BLE callback, etc.)
 *         and safe to call when idle (no-op).
 *
 * @param  service_id  GATT service ID from the send-complete event.
 */
void ai_rec_trans_notify_send_complete(uint16_t service_id);

/**
 * @brief  Register the filename currently being written by the
 *         recording pipeline. Used by handle_upload_file to refuse
 *         CMD_UPLOAD_FILE requests targeting the same file (which would
 *         hit a FATFS write-lock or read a growing snapshot).
 *
 *         Pass NULL or empty string to clear (e.g. when recording stops).
 *
 *         The string is copied internally; caller may free its buffer
 *         after this call returns. Pass NULL to clear.
 *
 *         Wiring: ideally called by the recording module right after
 *         it opens the output file (and again with NULL on close).
 *         If never called, behavior degrades to the documented
 *         "host must avoid same-file" caveat in set_recording().
 */
void app_ai_record_file_trans_set_active_record_file(const char *filename);

/**
 * @brief  Inform the module that mic recording has started / stopped.
 *
 *         Concurrent recording + upload is supported: this call is
 *         status-only - the flag does NOT gate uploads. Recording (mic
 *         to SD) and upload (SD to BLE) own independent FATFS handles and
 *         coexist freely.
 *
 *         Same-file race caveat: if the host issues CMD_UPLOAD_FILE for
 *         the file currently being recorded, the result is undefined
 *         (FATFS may refuse re-open or hand back a partial snapshot
 *         whose size grows during transfer). The host is expected to
 *         only request previously-finalized files. A future hardening
 *         step would expose the active recording filename through this
 *         module so the upload entry can refuse a same-file request.
 *
 *         Wiring: typically called from app_ai_record_mmi_action() when
 *         AI_RECORD_MMI_AI_VOICE_START / STOP is processed.
 */
void app_ai_record_file_trans_set_recording(bool recording);

/** @} */ /* End of group AI_RECORD_FILE_TRANS */

#ifdef __cplusplus
}
#endif

#endif /* _APP_AI_RECORD_FILE_TRANS_H_ */
