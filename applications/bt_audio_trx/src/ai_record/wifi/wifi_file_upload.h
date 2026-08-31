/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
#ifndef _WIFI_FILE_UPLOAD_H_
#define _WIFI_FILE_UPLOAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "wifi_app.h"

/* CMD / EVT IDs --------------------------------------------------------- */
#define WIFI_UPLOAD_CMD_QUERY_INFO     0x680
#define WIFI_UPLOAD_EVT_QUERY_INFO     0x680
#define WIFI_UPLOAD_CMD_UPLOAD_FILE    0x694
#define WIFI_UPLOAD_EVT_UPLOAD_FILE    0x693
#define WIFI_UPLOAD_CMD_UPLOAD_CANCEL  0x695
#define WIFI_UPLOAD_EVT_UPLOAD_CANCEL  0x694
#define WIFI_UPLOAD_CMD_SCAN_FILES     0x696
#define WIFI_UPLOAD_EVT_SCAN_FILES     0x695

#define WIFI_UPLOAD_PROTOCOL_VER       5


/* EVT flags (Multi-Event responses) ------------------------------------ */
#define WIFI_UPLOAD_FLAG_START         0x00
#define WIFI_UPLOAD_FLAG_CONTINUE      0x01
#define WIFI_UPLOAD_FLAG_END           0x02
#define WIFI_UPLOAD_FLAG_ERROR         0xFE

/* Upload error codes (EVT_UPLOAD_FILE Flag=0xFE Byte1) ------------------ */
#define WIFI_UPLOAD_ERR_NOT_FOUND      0x10
#define WIFI_UPLOAD_ERR_READ           0x11
#define WIFI_UPLOAD_ERR_PERMISSION     0x12
#define WIFI_UPLOAD_ERR_TRANSPORT      0x13

/* Scan error codes (EVT_SCAN_FILES Flag=0xFE Byte1) --------------------- */
#define WIFI_SCAN_ERR_STORAGE          0x10
#define WIFI_SCAN_ERR_PERMISSION       0x11
#define WIFI_SCAN_ERR_INTERNAL         0x12

/* file_format_t (per spec section 1.1) ---------------------------------- */
typedef enum
{
    WIFI_FILE_FORMAT_MP3  = 0,
    WIFI_FILE_FORMAT_MP4  = 1,
    WIFI_FILE_FORMAT_RTK  = 2,
    WIFI_FILE_FORMAT_AAC  = 3,
    WIFI_FILE_FORMAT_FLAC = 4,
    WIFI_FILE_FORMAT_TXT  = 5,
    WIFI_FILE_FORMAT_DAT  = 6,
    WIFI_FILE_FORMAT_BIN  = 7,
} wifi_file_format_t;

/* Storage IDs (only SD is supported on this device) -------------------- */
#define WIFI_STORAGE_ALL               0x00
#define WIFI_STORAGE_SD                0x02

/* Transport IDs (CMD_UPLOAD_FILE Byte0) --------------------------------- */
#define WIFI_TRANSPORT_BLE             0x01
#define WIFI_TRANSPORT_SPP             0x02
#define WIFI_TRANSPORT_WIFI            0x03

/* Internal subtype IDs used inside T_WIFI_MSG.subtype ------------------- */
#define WIFI_UPLOAD_SUBTYPE_TICK       0x10
#define WIFI_UPLOAD_SUBTYPE_CANCEL     0x11

/* Buffer sizing --------------------------------------------------------- */
#ifndef WIFI_UPLOAD_CHUNK_CAP
#define WIFI_UPLOAD_CHUNK_CAP          2000
#endif
#ifndef WIFI_UPLOAD_PATH_MAX
#define WIFI_UPLOAD_PATH_MAX           256
#endif

/* CRC calculation control ----------------------------------------------- */
#ifndef CONFIG_WIFI_UPLOAD_CRC_ENABLE
#define CONFIG_WIFI_UPLOAD_CRC_ENABLE  0   /* 1: real CRC per packet; 0: CRC field = 0 */
#endif

/**
 * Configuration passed at init by the caller. Values flow into
 * EVT_QUERY_INFO and define the listening port + SD mount.
 */
typedef struct
{
    uint16_t    packet_size;          /* EVT_QUERY_INFO Byte0..1 */
    uint16_t    buffer_check_size;    /* Byte2..3                */
    uint8_t     mode;                 /* Byte5  (RWS / bud role) */
    uint8_t     ic_type;              /* Byte6  (0x0B/0x11/0x13) */
    uint8_t     song_format_type;     /* Byte7  bitmask          */
    uint8_t     transport_cap;        /* Byte8  bitmask          */
    uint16_t    tcp_port;             /* listening TCP port      */
    const char *sd_mount;             /* e.g. "/SD:" - required  */
} T_WIFI_UPLOAD_CFG;

/**
 * Initialise the module. Must be called after WiFi stack is up.
 * Returns 0 on success, negative errno on failure.
 */
int  wifi_file_upload_init(const T_WIFI_UPLOAD_CFG *cfg);

/**
 * Tear down - closes any open file/dir, frees buffers, unregisters
 * the SDIO data callback.
 */
void wifi_file_upload_deinit(void);

/**
 * Hook called from the WiFi task whenever it dequeues a T_WIFI_MSG with
 * event == EVENT_USER_APP_DEFINE and msg_cb == wifi_file_upload_msg_handler.
 * The module installs itself as the msg_cb when posting its own ticks,
 * so the existing wifi_app.c dispatcher invokes this automatically and
 * no edit to wifi_app.c is required.
 */
void wifi_file_upload_msg_handler(void *msg);

/** @return true if the WiFi upload module was successfully initialised. */
bool wifi_file_upload_is_ready(void);

/**
 * @return true if a WiFi upload or scan is actively in progress.
 *         Used by the BLE command handler to avoid re-enabling DLPS
 *         while the WiFi upload module owns the INIT bit.
 */
bool wifi_file_upload_is_busy(void);

/**
 * @brief  Restore CPU + SPIC0 to normal frequency (40 MHz).
 *         Called on WiFi disconnect so the clock is kept boosted
 *         during the active upload and only lowered when the WiFi
 *         link is torn down.
 */
void wifi_file_upload_restore_clk(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_FILE_UPLOAD_H_ */
