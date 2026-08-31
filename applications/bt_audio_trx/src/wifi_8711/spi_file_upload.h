/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * @file    spi_file_upload.h
 * @brief   SPI file upload module (8773GTP + external Wi-Fi IC).
 *
 * Mirrors wifi_file_upload.h for the SPI transport path.  Protocol-level
 * CMD/EVT IDs, EVT flags, error codes, frame format [0xAA][Seq][Len][ID]
 * and CRC16 pair-XOR are all IDENTICAL to the BLE path
 * (app_ai_record_file_trans.c) and the SDIO path (wifi_file_upload.c) so
 * the host APK uses one codec for all transports.
 *
 * The TX path uses AT+SKTSENDRAW two-phase transparent send (>>> prompt +
 * raw data + OK) through the SPI AT engine (app_spi_atcmd.c).  The RX
 * path receives TCP data frames from the external Wi-Fi IC via the SPI
 * AT engine's transport dispatcher.
 */

#ifndef _SPI_FILE_UPLOAD_H_
#define _SPI_FILE_UPLOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/*============================================================================*
 *                              CMD / EVT IDs
 *============================================================================*/

#define SPI_UPLOAD_CMD_QUERY_INFO     0x680
#define SPI_UPLOAD_EVT_QUERY_INFO     0x680
#define SPI_UPLOAD_CMD_UPLOAD_FILE    0x694
#define SPI_UPLOAD_EVT_UPLOAD_FILE    0x693
#define SPI_UPLOAD_CMD_UPLOAD_CANCEL  0x695
#define SPI_UPLOAD_EVT_UPLOAD_CANCEL  0x694
#define SPI_UPLOAD_CMD_SCAN_FILES     0x696
#define SPI_UPLOAD_EVT_SCAN_FILES     0x695

#define SPI_UPLOAD_PROTOCOL_VER       5

/*============================================================================*
 *                              EVT flags
 *============================================================================*/

#define SPI_UPLOAD_FLAG_START         0x00
#define SPI_UPLOAD_FLAG_CONTINUE      0x01
#define SPI_UPLOAD_FLAG_END           0x02
#define SPI_UPLOAD_FLAG_ERROR         0xFE

/* Upload error codes (EVT_UPLOAD_FILE Flag=0xFE Byte1) */
#define SPI_UPLOAD_ERR_NOT_FOUND      0x10
#define SPI_UPLOAD_ERR_READ           0x11
#define SPI_UPLOAD_ERR_PERMISSION     0x12
#define SPI_UPLOAD_ERR_TRANSPORT      0x13

/* Scan error codes (EVT_SCAN_FILES Flag=0xFE Byte1) */
#define SPI_SCAN_ERR_STORAGE          0x10
#define SPI_SCAN_ERR_PERMISSION       0x11
#define SPI_SCAN_ERR_INTERNAL         0x12

/*============================================================================*
 *                              File format enum
 *============================================================================*/

typedef enum
{
    SPI_FILE_FORMAT_MP3  = 0,
    SPI_FILE_FORMAT_MP4  = 1,
    SPI_FILE_FORMAT_RTK  = 2,
    SPI_FILE_FORMAT_AAC  = 3,
    SPI_FILE_FORMAT_FLAC = 4,
    SPI_FILE_FORMAT_TXT  = 5,
    SPI_FILE_FORMAT_DAT  = 6,
    SPI_FILE_FORMAT_BIN  = 7,
    SPI_FILE_FORMAT_OPUS = 8,
} spi_file_format_t;

/*============================================================================*
 *                              Constants
 *============================================================================*/

#define SPI_STORAGE_ALL               0x00
#define SPI_STORAGE_SD                0x02

#define SPI_TRANSPORT_BLE             0x01
#define SPI_TRANSPORT_SPP             0x02
#define SPI_TRANSPORT_WIFI            0x03

#ifndef SPI_UPLOAD_CHUNK_CAP
/** @brief Maximum payload bytes per data chunk. The whole SPI frame
 *         [AT(2)][tcp_len(2)][AA-hdr(6)][chunk-hdr(11)][data][CRC(4)] must be
 *         <= SPI_XMIT_SIZE(16384), so data <= 16384-25 = 16359. Filling the
 *         frame (16359) makes each upload frame the same on-wire size as the
 *         tp_test blast (16376-byte SPI payload) and minimises frame count. */
#define SPI_UPLOAD_CHUNK_CAP          16359
#endif

#ifndef SPI_UPLOAD_PATH_MAX
#define SPI_UPLOAD_PATH_MAX           256
#endif

/*============================================================================*
 *                              Config struct
 *============================================================================*/

typedef struct
{
    uint16_t    packet_size;
    uint16_t    buffer_check_size;
    uint8_t     mode;
    uint8_t     ic_type;
    uint8_t     song_format_type;
    uint8_t     transport_cap;
    uint16_t    tcp_port;
    const char *sd_mount;
} T_SPI_UPLOAD_CFG;

/*============================================================================*
 *                              CRC control
 *============================================================================*/

#ifndef CONFIG_WIFI_UPLOAD_CRC_ENABLE
#define CONFIG_WIFI_UPLOAD_CRC_ENABLE  0   /* 1: real CRC per packet; 0: CRC field = 0 */
#endif

/*============================================================================*
 *                              Public API
 *============================================================================*/

/**
 * @brief  Initialise the SPI file upload module.
 *         Must be called after the Wi-Fi IC is connected and has an IP.
 * @param  cfg  Configuration (packet size, port, sd_mount, ...).
 * @return 0 on success, negative errno on failure.
 */
int  spi_file_upload_init(const T_SPI_UPLOAD_CFG *cfg);

/**
 * @brief  Tear down - close any open file/dir, free buffers.
 */
void spi_file_upload_deinit(void);

/**
 * @brief  Process an inbound TCP data frame from the Wi-Fi IC.
 *         Called from the SPI AT engine's spi_atcmd_rcv_cb() when a
 *         frame with FrameType=0x01 (TCP data) arrives.
 *
 *         Frame payload (after [AT][Len][FrameType=0x01]) is the standard
 *         [0xAA][Seq][Len:2 LE][Cmd_ID:2 LE][Params...] protocol frame.
 *
 * @param  frame      Pointer to the TCP data frame payload.
 * @param  frame_len  Length of the frame payload in bytes.
 * @return true if the frame was recognised and consumed.
 */
bool spi_file_upload_on_tcp_rx(const uint8_t *frame, uint16_t frame_len);

/**
 * @brief  Called when an AT+SKTSENDRAW completes successfully ("OK").
 *         Triggers upload_tick() to send the next file chunk.
 */
void spi_file_upload_on_sendraw_ok(void);

/**
 * @brief  Called when an AT+SKTSENDRAW completes with "ERROR".
 *         Triggers error handling / retry.
 */
void spi_file_upload_on_sendraw_error(void);

/** @return true if the module was successfully initialised. */
bool spi_file_upload_is_ready(void);

/**
 * @return true if a WiFi upload or scan is actively in progress.
 *         Used by the BLE command handler to avoid re-enabling DLPS
 *         while the SPI file upload module owns the INIT bit.
 */
bool spi_file_upload_is_busy(void);

/**
 * @brief  Push all file data chunks in one continuous SKTSENDRAW stream.
 *         Called by the AT engine's ">>>" handler when bulk mode is set.
 *         Reads file data, builds AA-framed protocol packets, and pushes
 *         each as a raw SPI frame via app_spi_master_send_raw_data().
 *
 * @param  total_bytes  Total TCP bytes declared in the AT+SKTSENDRAW command
 *                      (used only for the resend timeout).
 */
void spi_file_upload_bulk_push(uint32_t total_bytes);

/**
 * @brief  Restore CPU + SPIC0 to normal frequency (40 MHz).
 *         Called on WiFi disconnect so the clock is kept boosted
 *         during the active upload and only lowered when the WiFi
 *         link is torn down.
 */
void spi_file_upload_restore_clk(void);

#ifdef __cplusplus
}
#endif

#endif /* _SPI_FILE_UPLOAD_H_ */
