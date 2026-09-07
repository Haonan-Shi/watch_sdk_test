/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/fs/fs.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "trace.h"
#include "os_mem.h"
#include "os_queue.h"
#include "wifi_file_upload.h"
#include "wifi_sdio.h"
#include "wifi_desc.h"
#include "wifi_app.h"
#include "wdg.h"
#include "app_dlps.h"
#include "clk_mgr.h"

/* ====================================================================== */
/* Module state                                                            */
/* ====================================================================== */

typedef enum
{
    STATE_IDLE = 0,
    STATE_UPLOADING,
    STATE_SCANNING,
} fsm_state_t;

static T_WIFI_UPLOAD_CFG s_cfg;
static uint8_t          *s_tx_buf;
static uint16_t          s_tx_buf_cap;
static fsm_state_t       s_state = STATE_IDLE;
static bool              s_inited;

/* clk_mgr user for boosting CPU + SPIC0 during WiFi file upload. */
static T_CLK_USER_HANDLE clk_mgr_upload_handle = NULL;

/* Upload context ------------------------------------------------------- */
static struct
{
    struct fs_file_t  file;
    bool              file_open;
    char              path[WIFI_UPLOAD_PATH_MAX];
    uint32_t          total_len;
    uint32_t          start_offset;
    uint32_t          cur_offset;
    uint16_t          tool_max_chunk;
    uint16_t          chunk_size;
    uint16_t          seq;
    uint16_t          whole_crc;
    uint8_t           format;
    uint32_t          mtime;
    uint32_t          peer_ip;
    uint16_t          peer_port;
    bool              cancel_pending;
    bool              start_evt_sent;
} s_upload;

/* Scan context --------------------------------------------------------- */
static struct
{
    uint8_t           filter;
    uint8_t           storage;
    uint8_t           batch_hint;
    struct fs_dir_t   dir;
    bool              dir_open;
    uint32_t          total_count;
    uint32_t          delivered;
    uint32_t          peer_ip;
    uint16_t          peer_port;
    bool              cancel_pending;
    bool              start_evt_sent;
} s_scan;

/* Forward decls -------------------------------------------------------- */
static uint16_t on_tcp_rx(void *p_data, uint16_t len);
static void     post_tick(void);
static void     post_cancel(void);

/* ====================================================================== */
/* Helpers                                                                 */
/* ====================================================================== */

/*
 * Pair-XOR running checksum - MUST match BLE path
 * (app_ai_record_file_trans.c ai_rec_crc16_step / finalize)
 * so the host APK uses one algorithm for both transports.
 *
 * XORs pairs of adjacent bytes interpreted as little-endian uint16,
 * then byte-swaps (big-endian on wire).  The cast-to-uint16_t
 * pattern is deliberately identical to the BLE reference so the
 * numeric result is bit-for-bit the same.
 */
static uint16_t crc16_pair_xor_step(uint16_t prev, const uint8_t *buf, uint32_t len)
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

static uint16_t crc16_pair_xor_finalize(uint16_t cs)
{
    return (uint16_t)(((cs & 0xff) << 8) | ((cs & 0xff00) >> 8));
}

static uint16_t crc16_pair_xor_chunk(const uint8_t *buf, uint32_t len)
{
    return crc16_pair_xor_finalize(crc16_pair_xor_step(0, buf, len));
}

static uint8_t format_from_ext(const char *name, uint16_t name_len)
{
    /* find the last '.' */
    int dot = -1;
    for (int i = (int)name_len - 1; i >= 0; i--)
    {
        if (name[i] == '.')
        {
            dot = i;
            break;
        }
        if (name[i] == '/' || name[i] == '\\')
        {
            break;
        }
    }
    if (dot < 0 || (uint16_t)dot == name_len - 1)
    {
        return WIFI_FILE_FORMAT_BIN;
    }
    const char *ext = name + dot + 1;
    uint16_t    ext_len = name_len - (uint16_t)dot - 1;
    char        lower[8];
    if (ext_len >= sizeof(lower))
    {
        return WIFI_FILE_FORMAT_BIN;
    }
    for (uint16_t i = 0; i < ext_len; i++)
    {
        lower[i] = (char)tolower((unsigned char)ext[i]);
    }
    lower[ext_len] = '\0';

    if (!strcmp(lower, "mp3"))
    {
        return WIFI_FILE_FORMAT_MP3;
    }
    if (!strcmp(lower, "mp4"))
    {
        return WIFI_FILE_FORMAT_MP4;
    }
    if (!strcmp(lower, "rtk"))
    {
        return WIFI_FILE_FORMAT_RTK;
    }
    if (!strcmp(lower, "aac"))
    {
        return WIFI_FILE_FORMAT_AAC;
    }
    if (!strcmp(lower, "flac"))
    {
        return WIFI_FILE_FORMAT_FLAC;
    }
    if (!strcmp(lower, "txt"))
    {
        return WIFI_FILE_FORMAT_TXT;
    }
    if (!strcmp(lower, "dat"))
    {
        return WIFI_FILE_FORMAT_DAT;
    }
    if (!strcmp(lower, "wav"))
    {
        return WIFI_FILE_FORMAT_BIN;     /* spec has no WAV - treat as BIN */
    }
    return WIFI_FILE_FORMAT_BIN;
}

/*
 * Resolve the inbound filename to an absolute path in s_upload.path.
 * If the filename starts with '/', it is taken verbatim. Otherwise it is
 * resolved against sd_mount. Returns true on success, false if the
 * result would not fit.
 */
static bool resolve_path(const char *name, uint16_t name_len)
{
    if (name_len == 0)
    {
        return false;
    }
    if (name[0] == '/')
    {
        if (name_len + 1 > sizeof(s_upload.path))
        {
            return false;
        }
        memcpy(s_upload.path, name, name_len);
        s_upload.path[name_len] = '\0';
        return true;
    }
    if (s_cfg.sd_mount == NULL)
    {
        return false;
    }
    size_t mlen = strlen(s_cfg.sd_mount);
    /* mount + '/' + name + '\0' */
    if (mlen + 1 + name_len + 1 > sizeof(s_upload.path))
    {
        return false;
    }
    memcpy(s_upload.path, s_cfg.sd_mount, mlen);
    s_upload.path[mlen] = '/';
    memcpy(s_upload.path + mlen + 1, name, name_len);
    s_upload.path[mlen + 1 + name_len] = '\0';
    return true;
}

static inline void put_u16_le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
}

static inline void put_u32_le(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)(v & 0xFF);
    p[1] = (uint8_t)((v >> 8) & 0xFF);
    p[2] = (uint8_t)((v >> 16) & 0xFF);
    p[3] = (uint8_t)((v >> 24) & 0xFF);
}

static inline uint16_t get_u16_le(const uint8_t *p)
{
    return (uint16_t)(p[0] | ((uint16_t)p[1] << 8));
}

static inline uint32_t get_u32_le(const uint8_t *p)
{
    return (uint32_t)(p[0]) |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

/* ---------- SDIO outer frame ------------------------------------------
 * [0xAA][Seq:1][Length:2 LE][cmd/event:2 LE][parameters:N]
 * Length covers (cmd/event + parameters) = 2 + N (N may be 0).
 * Builders write the inner content (cmd/event + parameters) at
 * s_tx_buf + FRAME_PREFIX_LEN; frame_and_send() prepends the sync/seq/length
 * and queues the whole frame on SDIO (no trailing checksum - mirrors BLE
 * protocol for consistency).
 */
#define FRAME_SYNC_BYTE     0xAAu
#define FRAME_PREFIX_LEN    4u    /* sync(1) + seq(1) + length(2) */
#define FRAME_SUFFIX_LEN    0u    /* no checksum - mirroring BLE protocol */
#define FRAME_OVERHEAD      (FRAME_PREFIX_LEN + FRAME_SUFFIX_LEN)

static uint8_t s_tx_seq;

/*
 * Wrap inner content already written at s_tx_buf + FRAME_PREFIX_LEN with
 * the outer frame header (0xAA / Seq / Length) and queue it for SDIO TX.
 * No trailing checksum - mirrors the BLE protocol convention.
 * Returns false if the SDIO write queue is full - caller should retry.
 *
 * NOTE: when the cached tx_bd_num is 0, we refresh it from the hardware
 * register and only bail out if it is STILL 0 after refresh.
 */
static bool frame_and_send(uint32_t ip, uint16_t port, uint16_t inner_len)
{
    uint8_t seq = s_tx_seq++;
    s_tx_buf[0] = FRAME_SYNC_BYTE;
    s_tx_buf[1] = seq;
    put_u16_le(s_tx_buf + 2, inner_len);

    extern uint16_t tx_bd_num;
    if (tx_bd_num == 0)
    {
        /* Poll the hardware register multiple times - the WiFi module may
         * free TX BDs shortly after the cached value hit 0.  Each iteration
         * waits 3 ms then refreshes from hardware; bail after 5 attempts
         * so the caller can schedule a retry via post_tick(). */
        for (int i = 0; i < 5; i++)
        {
            k_msleep(3);
            tx_bd_num = wifi_sdio_tx_bd_update();
            if (tx_bd_num != 0)
            {
                break;
            }
        }
        if (tx_bd_num == 0)
        {
            APP_PRINT_ERROR0("[wifi_upload] frame_and_send: SDIO tx buffer full");
            return false;
        }
    }

    if (wifi_sdio_data_write_queue_fill(ip, port, s_tx_buf,
                                        (uint16_t)(FRAME_PREFIX_LEN + inner_len + FRAME_SUFFIX_LEN)))
    {
        extern T_OS_QUEUE  sdio_write_queue;
        while (sdio_write_queue.count)
        {
            //  APP_PRINT_INFO1("sdio_write_queue.count = %d", sdio_write_queue.count);
            T_WIFI_SDIO_WRITE_QUEUE *write_pkt = wifi_sdio_data_write_queue_peek(0);
            if (!wifi_sdio_write_data(write_pkt))
            {
                wifi_sdio_data_write_queue_flush(1);
            }
            else
            {
                APP_PRINT_INFO0("[wifi_upload] sdio tx buf full");
                return false;
            }
        }
        return true;
    }
    else
    {
        APP_PRINT_ERROR0("[wifi_upload] frame_and_send: SDIO write queue full");
        return false;
    }
}

/* ====================================================================== */
/* Frame builders - write into s_tx_buf, return total length              */
/* ====================================================================== */

static uint16_t build_query_info_evt(uint8_t *buf)
{
    /* EVT_ID(2) + 32-byte body */
    put_u16_le(buf, WIFI_UPLOAD_EVT_QUERY_INFO);
    uint8_t *b = buf + 2;
    memset(b, 0, 32);
    put_u16_le(b + 0, s_cfg.packet_size);
    put_u16_le(b + 2, s_cfg.buffer_check_size);
    b[4] = WIFI_UPLOAD_PROTOCOL_VER;
    b[5] = s_cfg.mode;
    b[6] = s_cfg.ic_type;
    b[7] = s_cfg.song_format_type;
    b[8] = s_cfg.transport_cap;
    b[9] = WIFI_TRANSPORT_WIFI;          /* active_transport */
    /* b[10..31] reserved (zero) */
    return 2 + 32;
}

static uint16_t build_upload_start_evt(uint8_t *buf, uint16_t crc, uint32_t total_len,
                                       uint8_t format, uint32_t timestamp)
{
    put_u16_le(buf, WIFI_UPLOAD_EVT_UPLOAD_FILE);
    uint8_t *b = buf + 2;
    b[0] = WIFI_UPLOAD_FLAG_START;
    put_u16_le(b + 1, crc);
    put_u32_le(b + 3, total_len);
    b[7] = format;
    put_u32_le(b + 8, timestamp);
    return 2 + 12;
}

/*
 * Header-only builder: writes EVT_ID + 11-byte chunk header into buf.
 * Caller must have already placed `data_len` data bytes at buf+13.
 * Returns total frame length (header + data).
 */
static uint16_t build_upload_chunk_evt(uint8_t *buf, uint8_t flag, uint16_t seq,
                                       uint16_t crc, uint32_t off, uint16_t data_len)
{
    put_u16_le(buf, WIFI_UPLOAD_EVT_UPLOAD_FILE);
    uint8_t *b = buf + 2;
    b[0] = flag;
    put_u16_le(b + 1, seq);
    put_u16_le(b + 3, crc);
    put_u32_le(b + 5, off);
    put_u16_le(b + 9, data_len);
    return (uint16_t)(2 + 11 + data_len);
}

static uint16_t build_upload_error_evt(uint8_t *buf, uint8_t err_code)
{
    put_u16_le(buf, WIFI_UPLOAD_EVT_UPLOAD_FILE);
    buf[2] = WIFI_UPLOAD_FLAG_ERROR;
    buf[3] = err_code;
    return 4;
}

static uint16_t build_upload_cancel_evt(uint8_t *buf, uint8_t result)
{
    put_u16_le(buf, WIFI_UPLOAD_EVT_UPLOAD_CANCEL);
    buf[2] = result;
    return 3;
}

static uint16_t build_scan_start_evt(uint8_t *buf, uint32_t total)
{
    put_u16_le(buf, WIFI_UPLOAD_EVT_SCAN_FILES);
    buf[2] = WIFI_UPLOAD_FLAG_START;
    put_u32_le(buf + 3, total);
    return 7;
}

static uint16_t build_scan_entry_evt(uint8_t *buf, uint16_t idx, uint8_t format,
                                     uint8_t storage, uint32_t size, uint32_t mtime,
                                     uint16_t crc, const char *name, uint16_t name_len)
{
    put_u16_le(buf, WIFI_UPLOAD_EVT_SCAN_FILES);
    uint8_t *b = buf + 2;
    b[0] = WIFI_UPLOAD_FLAG_CONTINUE;
    put_u16_le(b + 1, idx);
    b[3] = format;
    b[4] = storage;
    put_u32_le(b + 5, size);
    put_u32_le(b + 9, mtime);
    put_u16_le(b + 13, crc);
    put_u16_le(b + 15, name_len);
    memcpy(b + 17, name, name_len);
    return (uint16_t)(2 + 17 + name_len);
}

static uint16_t build_scan_end_evt(uint8_t *buf, uint32_t delivered)
{
    put_u16_le(buf, WIFI_UPLOAD_EVT_SCAN_FILES);
    buf[2] = WIFI_UPLOAD_FLAG_END;
    put_u32_le(buf + 3, delivered);
    return 7;
}

static uint16_t build_scan_error_evt(uint8_t *buf, uint8_t err_code)
{
    put_u16_le(buf, WIFI_UPLOAD_EVT_SCAN_FILES);
    buf[2] = WIFI_UPLOAD_FLAG_ERROR;
    buf[3] = err_code;
    return 4;
}

/* ====================================================================== */
/* Message posting                                                         */
/* ====================================================================== */

static void post_tick(void)
{
    T_WIFI_MSG msg;
    msg.event   = EVENT_USER_APP_DEFINE;
    msg.subtype = WIFI_UPLOAD_SUBTYPE_TICK;
    msg.msg_cb  = wifi_file_upload_msg_handler;
    msg.u.param = 0;
    if (!app_send_msg_to_wifitask(&msg))
    {
        APP_PRINT_ERROR0("[wifi_upload] post_tick failed");
    }
}

static void post_cancel(void)
{
    T_WIFI_MSG msg;
    msg.event   = EVENT_USER_APP_DEFINE;
    msg.subtype = WIFI_UPLOAD_SUBTYPE_CANCEL;
    msg.msg_cb  = wifi_file_upload_msg_handler;
    msg.u.param = 0;
    (void)app_send_msg_to_wifitask(&msg);
}

/* ====================================================================== */
/* Pre-pass CRC of whole file (for EVT Flag=0x00)                          */
/* ====================================================================== */

static int compute_whole_crc(const char *path, uint16_t *out_crc, uint32_t *out_size)
{
    struct fs_file_t f;
    fs_file_t_init(&f);
    int rc = fs_open(&f, path, FS_O_READ);
    if (rc < 0)
    {
        return rc;
    }
    uint16_t cs = 0;
    uint32_t total = 0;
    uint8_t buf[256];
    for (;;)
    {
        ssize_t n = fs_read(&f, buf, sizeof(buf));
        if (n < 0)
        {
            fs_close(&f);
            return (int)n;
        }
        if (n == 0)
        {
            break;
        }
        cs = crc16_pair_xor_step(cs, buf, (uint32_t)n);
        total += (uint32_t)n;
    }
    fs_close(&f);
    if (out_crc)
    {
        *out_crc  = crc16_pair_xor_finalize(cs);
    }
    if (out_size)
    {
        *out_size = total;
    }
    return 0;
}

/* ====================================================================== */
/* CMD parsers (called from RX callback)                                   */
/* ====================================================================== */

static void send_upload_error(uint32_t ip, uint16_t port, uint8_t err)
{
    uint16_t n = build_upload_error_evt(s_tx_buf + FRAME_PREFIX_LEN, err);
    (void)frame_and_send(ip, port, n);
}

static void send_scan_error(uint32_t ip, uint16_t port, uint8_t err)
{
    uint16_t n = build_scan_error_evt(s_tx_buf + FRAME_PREFIX_LEN, err);
    (void)frame_and_send(ip, port, n);
}

static void handle_query_info(uint32_t ip, uint16_t port)
{
    uint16_t n = build_query_info_evt(s_tx_buf + FRAME_PREFIX_LEN);
    (void)frame_and_send(ip, port, n);
}

static void handle_upload_file(const uint8_t *body, uint16_t body_len,
                               uint32_t ip, uint16_t port)
{
    /* body layout (after CMD_ID): Transport(1) NameLen(2) Name(N)
     *                             StartOff(4) MaxChunk(4) */
    if (body_len < 1 + 2)
    {
        send_upload_error(ip, port, WIFI_UPLOAD_ERR_PERMISSION);
        return;
    }
    uint8_t  transport = body[0];
    uint16_t name_len  = get_u16_le(body + 1);

    if (transport != WIFI_TRANSPORT_WIFI)
    {
        APP_PRINT_ERROR1("[wifi_upload] non-WiFi transport 0x%02x", transport);
        send_upload_error(ip, port, WIFI_UPLOAD_ERR_TRANSPORT);
        return;
    }
    if (s_state != STATE_IDLE)
    {
        APP_PRINT_ERROR1("[wifi_upload] busy state=%d", (int)s_state);
        send_upload_error(ip, port, WIFI_UPLOAD_ERR_TRANSPORT);
        return;
    }
    if (name_len == 0 || (uint32_t)body_len < 1U + 2U + name_len + 4U + 4U)
    {
        send_upload_error(ip, port, WIFI_UPLOAD_ERR_PERMISSION);
        return;
    }
    const uint8_t *name      = body + 3;
    uint32_t       start_off = get_u32_le(body + 3 + name_len);
    uint32_t       tool_max  = get_u32_le(body + 3 + name_len + 4);

    if (!resolve_path((const char *)name, name_len))
    {
        send_upload_error(ip, port, WIFI_UPLOAD_ERR_PERMISSION);
        return;
    }

    /* fs_stat for size + mtime */
    struct fs_dirent st;
    int rc = fs_stat(s_upload.path, &st);
    if (rc < 0)
    {
        APP_PRINT_ERROR2("[wifi_upload] fs_stat %s rc=%d", TRACE_STRING(s_upload.path), rc);
        send_upload_error(ip, port, WIFI_UPLOAD_ERR_NOT_FOUND);
        return;
    }
    if (st.type != FS_DIR_ENTRY_FILE)
    {
        send_upload_error(ip, port, WIFI_UPLOAD_ERR_NOT_FOUND);
        return;
    }

    /*
     * Pre-compute whole-file CRC (pair-XOR, matching BLE path).
     * Failure is non-fatal: crc stays 0 and the host skips
     * whole-file integrity checks.
     * Disabled when CONFIG_WIFI_UPLOAD_CRC_ENABLE==0.
     */
    uint16_t whole_crc = 0;
#if CONFIG_WIFI_UPLOAD_CRC_ENABLE
    {
        uint32_t verify_size = 0;
        (void)compute_whole_crc(s_upload.path, &whole_crc, &verify_size);
    }
#endif

    /* lock state */
    s_state                 = STATE_UPLOADING;
    s_upload.total_len      = st.size;
    s_upload.start_offset   = (start_off > st.size) ? st.size : start_off;
    s_upload.cur_offset     = s_upload.start_offset;
    s_upload.tool_max_chunk = (tool_max > 0xFFFF) ? 0xFFFF : (uint16_t)tool_max;
    if (s_upload.tool_max_chunk == 0 || s_upload.tool_max_chunk > WIFI_UPLOAD_CHUNK_CAP)
    {
        s_upload.chunk_size = WIFI_UPLOAD_CHUNK_CAP;
    }
    else
    {
        s_upload.chunk_size = s_upload.tool_max_chunk;
    }
    s_upload.seq            = 0;
    s_upload.whole_crc      = whole_crc;
    s_upload.format         = format_from_ext((const char *)name, name_len);
    s_upload.mtime          = 0;
    s_upload.peer_ip        = ip;
    s_upload.peer_port      = port;
    s_upload.cancel_pending = false;
    s_upload.start_evt_sent = false;
    s_upload.file_open      = false;

    APP_PRINT_INFO3("[wifi_upload] start path=%s size=%u start_off=%u",
                    TRACE_STRING(s_upload.path), s_upload.total_len, s_upload.start_offset);

    /* Disable DLPS for the duration of this WiFi upload. Re-enabled
     * when upload_tick() transitions the state back to IDLE. */
    app_dlps_disable(APP_DLPS_ENTER_CHECK_INIT);

    /* Boost CPU + SPIC0 to high performance (200 MHz) for upload
     * throughput. Restored when upload_tick() reaches STATE_IDLE. */
    if (clk_mgr_upload_handle)
    {
        clk_mgr_set_high_performance(clk_mgr_upload_handle);
    }

    post_tick();
}

static void handle_upload_cancel(uint32_t ip, uint16_t port, const uint8_t *body, uint16_t body_len)
{
    (void)body;
    (void)body_len;

    if (s_state == STATE_UPLOADING)
    {
        s_upload.cancel_pending = true;
    }
    else if (s_state == STATE_SCANNING)
    {
        s_scan.cancel_pending = true;
    }
    else
    {
        /*
         * IDLE state - the previous upload already finished and
         * reset the state machine.  The host sends CMD_UPLOAD_CANCEL
         * as the normal end-of-file signal so it can start the next
         * file.  We must still respond with EVT_UPLOAD_CANCEL so the
         * host knows the channel is ready.
         *
         * Instead of a one-shot frame_and_send from the RX callback
         * (which has no retry - its return value is discarded), we
         * ride the same post_cancel/upload_tick path that the
         * uploading-state cancel uses: set cancel_pending on the
         * upload context and let upload_tick() handle the send with
         * its built-in k_msleep(2) + post_tick() retry loop.
         */
        s_state                = STATE_UPLOADING;
        s_upload.cancel_pending = true;
        s_upload.peer_ip        = ip;
        s_upload.peer_port      = port;
        post_cancel();
        return;
    }

    /* peer for the cancel-ack uses the same connection that issued the cancel */
    if (s_state == STATE_UPLOADING)
    {
        s_upload.peer_ip   = ip;
        s_upload.peer_port = port;
    }
    else
    {
        s_scan.peer_ip   = ip;
        s_scan.peer_port = port;
    }
    post_cancel();
}

/* ----- Scan ---------------------------------------------------------- */

/* Storage filter accepts only ALL or SD on this device. */
static bool storage_supported(uint8_t filter_storage)
{
    return filter_storage == WIFI_STORAGE_ALL || filter_storage == WIFI_STORAGE_SD;
}

static bool ext_matches(const char *name, uint16_t name_len, uint8_t filter)
{
    if (filter == 0x00)
    {
        return true;
    }
    return format_from_ext(name, name_len) == filter;
}

static uint32_t count_matching(void)
{
    uint32_t total = 0;
    struct fs_dir_t d;
    fs_dir_t_init(&d);
    if (fs_opendir(&d, s_cfg.sd_mount) < 0)
    {
        return 0;
    }

    for (;;)
    {
        struct fs_dirent ent;
        int rc = fs_readdir(&d, &ent);
        if (rc < 0 || ent.name[0] == '\0')
        {
            break;
        }
        if (ent.type != FS_DIR_ENTRY_FILE)
        {
            continue;
        }
        if (!ext_matches(ent.name, (uint16_t)strlen(ent.name), s_scan.filter))
        {
            continue;
        }
        total++;
    }
    fs_closedir(&d);
    return total;
}

static bool open_sd_dir(void)
{
    fs_dir_t_init(&s_scan.dir);
    if (fs_opendir(&s_scan.dir, s_cfg.sd_mount) == 0)
    {
        s_scan.dir_open = true;
        return true;
    }
    s_scan.dir_open = false;
    return false;
}

static void handle_scan_files(const uint8_t *body, uint16_t body_len,
                              uint32_t ip, uint16_t port)
{
    if (body_len < 3)
    {
        send_scan_error(ip, port, WIFI_SCAN_ERR_INTERNAL);
        return;
    }
    if (s_state != STATE_IDLE)
    {
        send_scan_error(ip, port, WIFI_SCAN_ERR_INTERNAL);
        return;
    }

    uint8_t filter     = body[0];
    uint8_t storage    = body[1];
    uint8_t batch_hint = body[2];

    if (!storage_supported(storage))
    {
        send_scan_error(ip, port, WIFI_SCAN_ERR_STORAGE);
        return;
    }

    s_state                 = STATE_SCANNING;
    s_scan.filter           = filter;
    s_scan.storage          = storage;
    s_scan.batch_hint       = batch_hint;
    s_scan.dir_open         = false;
    s_scan.total_count      = 0;
    s_scan.delivered        = 0;
    s_scan.peer_ip          = ip;
    s_scan.peer_port        = port;
    s_scan.cancel_pending   = false;
    s_scan.start_evt_sent   = false;

    APP_PRINT_INFO2("[wifi_upload] scan filter=0x%02x storage=0x%02x", filter, storage);

    /* Disable DLPS for the duration of this scan operation.
     * Re-enabled in scan_tick() when state transitions back to IDLE. */
    app_dlps_disable(APP_DLPS_ENTER_CHECK_INIT);

    post_tick();
}

/* ====================================================================== */
/* RX callback (runs on WiFi task, payload includes RXDESC header)         */
/* ====================================================================== */

static uint16_t on_tcp_rx(void *p_data, uint16_t len)
{
    if (p_data == NULL || len < SIZE_RX_DESC)
    {
        return 0;
    }
    PRXDESC  rx       = (PRXDESC)p_data;
    uint16_t pkt_len  = (uint16_t)rx->pkt_len;
    uint16_t hdr_off  = (uint16_t)rx->offset;
    if (hdr_off + pkt_len > len)
    {
        return 0;
    }
    uint8_t *payload = (uint8_t *)p_data + hdr_off;
    uint32_t peer_ip = 0;
    uint16_t peer_pt = 5001;

    /* Outer frame: [0xAA][Seq][Length:2][cmd_id:2][params:N].
     * Length covers cmd_id + params (>= 2). Total = 4 + Length. */
    if (pkt_len < FRAME_PREFIX_LEN + 2u ||
        payload[0] != FRAME_SYNC_BYTE)
    {
        APP_PRINT_WARN1("[wifi_upload] bad frame head pkt_len=%d", pkt_len);
        return len;
    }
    uint8_t  rx_seq    = payload[1];
    uint16_t inner_len = get_u16_le(payload + 2);
    if (inner_len < 2u ||
        (uint32_t)pkt_len < (uint32_t)FRAME_PREFIX_LEN + inner_len)
    {
        APP_PRINT_WARN2("[wifi_upload] bad frame len pkt_len=%d inner=%d",
                        pkt_len, inner_len);
        return len;
    }

    uint16_t       cmd_id = get_u16_le(payload + FRAME_PREFIX_LEN);
    const uint8_t *body   = payload + FRAME_PREFIX_LEN + 2;
    uint16_t       blen   = (uint16_t)(inner_len - 2u);

    APP_PRINT_INFO4("[wifi_upload] rx cmd=0x%04x seq=%d inner=%d port=%d",
                    cmd_id, rx_seq, inner_len, peer_pt);

    switch (cmd_id)
    {
    case WIFI_UPLOAD_CMD_QUERY_INFO:
        handle_query_info(peer_ip, peer_pt);
        break;
    case WIFI_UPLOAD_CMD_UPLOAD_FILE:
        handle_upload_file(body, blen, peer_ip, peer_pt);
        break;
    case WIFI_UPLOAD_CMD_UPLOAD_CANCEL:
        handle_upload_cancel(peer_ip, peer_pt, body, blen);
        break;
    case WIFI_UPLOAD_CMD_SCAN_FILES:
        handle_scan_files(body, blen, peer_ip, peer_pt);
        break;
    default:
        APP_PRINT_WARN1("[wifi_upload] unknown cmd 0x%04x", cmd_id);
        break;
    }
    return len;
}

/* ====================================================================== */
/* Tick handlers (run on WiFi task via msg_cb dispatch)                    */
/* ====================================================================== */

static void close_upload(void)
{
    if (s_upload.file_open)
    {
        fs_close(&s_upload.file);
        s_upload.file_open = false;
    }
}

static void close_scan(void)
{
    if (s_scan.dir_open)
    {
        fs_closedir(&s_scan.dir);
        s_scan.dir_open = false;
    }
}

static void upload_tick(void)
{
    if (s_upload.cancel_pending)
    {
        close_upload();
        uint16_t n = build_upload_cancel_evt(s_tx_buf + FRAME_PREFIX_LEN, 0x01);
        (void)frame_and_send(s_upload.peer_ip, s_upload.peer_port, n);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        s_state = STATE_IDLE;
        return;
    }

    /* First tick: pre-pass CRC + Flag=0x00 */
    if (!s_upload.start_evt_sent)
    {
        uint32_t deliver = (s_upload.start_offset >= s_upload.total_len)
                           ? 0
                           : (s_upload.total_len - s_upload.start_offset);

        uint16_t n = build_upload_start_evt(s_tx_buf + FRAME_PREFIX_LEN,
                                            s_upload.whole_crc, deliver,
                                            s_upload.format, s_upload.mtime);
        if (!frame_and_send(s_upload.peer_ip, s_upload.peer_port, n))
        {
            /* queue full - retry next tick */
            k_msleep(2);
            post_tick();
            return;
        }

        /* open + seek for data phase */
        fs_file_t_init(&s_upload.file);
        int rc = fs_open(&s_upload.file, s_upload.path, FS_O_READ);
        if (rc < 0)
        {
            send_upload_error(s_upload.peer_ip, s_upload.peer_port, WIFI_UPLOAD_ERR_READ);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        s_upload.file_open = true;

        if (s_upload.start_offset > 0)
        {
            rc = fs_seek(&s_upload.file, (off_t)s_upload.start_offset, FS_SEEK_SET);
            if (rc < 0)
            {
                close_upload();
                send_upload_error(s_upload.peer_ip, s_upload.peer_port, WIFI_UPLOAD_ERR_READ);
                app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
                s_state = STATE_IDLE;
                return;
            }
        }
        s_upload.cur_offset     = s_upload.start_offset;
        s_upload.seq            = 0;
        s_upload.start_evt_sent = true;

        if (deliver == 0)
        {
            /* nothing to deliver - send empty Flag=0x02 and finish */
            uint16_t m = build_upload_chunk_evt(s_tx_buf + FRAME_PREFIX_LEN, WIFI_UPLOAD_FLAG_END,
                                                s_upload.seq, 0,
                                                s_upload.cur_offset, 0);
            (void)frame_and_send(s_upload.peer_ip, s_upload.peer_port, m);
            close_upload();
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        post_tick();
        return;
    }

    while (1)
    {
        WDG_Kick();
        /* Data phase: one chunk per tick */
        uint32_t remaining = s_upload.total_len - s_upload.cur_offset;
        uint16_t want = (remaining > s_upload.chunk_size) ? s_upload.chunk_size
                        : (uint16_t)remaining;

        uint8_t *data_dst = s_tx_buf + FRAME_PREFIX_LEN + 2 + 11;
        ssize_t  got = fs_read(&s_upload.file, data_dst, want);
        if (got < 0)
        {
            APP_PRINT_ERROR1("[wifi_upload] fs_read rc=%d", (int)got);
            close_upload();
            send_upload_error(s_upload.peer_ip, s_upload.peer_port, WIFI_UPLOAD_ERR_READ);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        uint16_t n_data = (uint16_t)got;
#if CONFIG_WIFI_UPLOAD_CRC_ENABLE
        uint16_t chunk_crc = crc16_pair_xor_chunk(data_dst, n_data);
#else
        uint16_t chunk_crc = 0;
#endif
        uint8_t  flag = ((s_upload.cur_offset + n_data) >= s_upload.total_len)
                        ? WIFI_UPLOAD_FLAG_END : WIFI_UPLOAD_FLAG_CONTINUE;

        uint16_t total = build_upload_chunk_evt(s_tx_buf + FRAME_PREFIX_LEN, flag, s_upload.seq, chunk_crc,
                                                s_upload.cur_offset, n_data);
        if (!frame_and_send(s_upload.peer_ip, s_upload.peer_port, total))
        {
            /* queue full - file pos has advanced by n_data; rewind so we resend the same chunk */
            (void)fs_seek(&s_upload.file, (off_t)s_upload.cur_offset, FS_SEEK_SET);
            // k_msleep(2);
            post_tick();
            return;
        }

        s_upload.cur_offset += n_data;
        s_upload.seq++;
        if (flag == WIFI_UPLOAD_FLAG_END)
        {
            APP_PRINT_INFO1("[wifi_upload] done off=%u", s_upload.cur_offset);
            close_upload();
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            if (clk_mgr_upload_handle)
            {
                clk_mgr_set_normal_performance(clk_mgr_upload_handle);
            }
            s_state = STATE_IDLE;
            return;
        }
    }
    // post_tick();
}

static void scan_tick(void)
{
    if (s_scan.cancel_pending)
    {
        close_scan();
        uint16_t n = build_upload_cancel_evt(s_tx_buf + FRAME_PREFIX_LEN, 0x01);
        (void)frame_and_send(s_scan.peer_ip, s_scan.peer_port, n);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        s_state = STATE_IDLE;
        return;
    }

    if (!s_scan.start_evt_sent)
    {
        uint32_t total = count_matching();
        s_scan.total_count = total;
        uint16_t n = build_scan_start_evt(s_tx_buf + FRAME_PREFIX_LEN, total);
        (void)frame_and_send(s_scan.peer_ip, s_scan.peer_port, n);

        s_scan.start_evt_sent = true;

        if (total == 0)
        {
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        if (!open_sd_dir())
        {
            n = build_scan_end_evt(s_tx_buf + FRAME_PREFIX_LEN, 0);
            (void)frame_and_send(s_scan.peer_ip, s_scan.peer_port, n);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        post_tick();
        return;
    }

    /* one entry per tick */
    if (!s_scan.dir_open)
    {
        uint16_t n = build_scan_end_evt(s_tx_buf + FRAME_PREFIX_LEN, s_scan.delivered);
        (void)frame_and_send(s_scan.peer_ip, s_scan.peer_port, n);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        s_state = STATE_IDLE;
        return;
    }

    bool find_next = false;
    struct fs_dirent ent;
    uint16_t name_len;
    while (!find_next)
    {

        int rc = fs_readdir(&s_scan.dir, &ent);
        if (rc < 0 || ent.name[0] == '\0')
        {
            /* end of SD root - finish */
            close_scan();
            uint16_t end_n = build_scan_end_evt(s_tx_buf + FRAME_PREFIX_LEN, s_scan.delivered);
            (void)frame_and_send(s_scan.peer_ip, s_scan.peer_port, end_n);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        find_next = true;
        if (ent.type != FS_DIR_ENTRY_FILE)
        {
            find_next = false;
        }
        else
        {
            name_len = (uint16_t)strlen(ent.name);
            if (!ext_matches(ent.name, name_len, s_scan.filter))
            {
                find_next = false;
            }
        }
    }
    /* tx_buf_cap must hold: outer prefix(4) + EVT_ID(2) + entry hdr(17) + name.
     * Clamp name_len so the framed packet fits. */
    uint16_t hdr_total = (uint16_t)(FRAME_PREFIX_LEN + 19u + FRAME_SUFFIX_LEN);
    uint16_t max_name  = (s_tx_buf_cap > hdr_total) ? (uint16_t)(s_tx_buf_cap - hdr_total) : 0u;
    if (name_len > max_name)
    {
        name_len = max_name;
    }

    uint8_t format = s_scan.filter;
    uint16_t n = build_scan_entry_evt(s_tx_buf + FRAME_PREFIX_LEN, (uint16_t)s_scan.delivered,
                                      format, WIFI_STORAGE_SD,
                                      (uint32_t)ent.size, 0, 0,
                                      ent.name, name_len);
    if (!frame_and_send(s_scan.peer_ip, s_scan.peer_port, n))
    {
        APP_PRINT_WARN0("[wifi_upload] scan_tick: queue full, retrying");
    }
    s_scan.delivered++;
    post_tick();
}

void wifi_file_upload_msg_handler(void *msg)
{
    T_WIFI_MSG *m = (T_WIFI_MSG *)msg;
    if (m == NULL)
    {
        return;
    }
    switch (m->subtype)
    {
    case WIFI_UPLOAD_SUBTYPE_TICK:
    case WIFI_UPLOAD_SUBTYPE_CANCEL:
        if (s_state == STATE_UPLOADING)
        {
            upload_tick();
        }
        else if (s_state == STATE_SCANNING)
        {
            scan_tick();
        }
        /* else IDLE: drop */
        break;
    default:
        break;
    }
}

/* ====================================================================== */
/* Init / deinit                                                           */
/* ====================================================================== */

int wifi_file_upload_init(const T_WIFI_UPLOAD_CFG *cfg)
{
    if (s_inited)
    {
        return -EALREADY;
    }
    if (cfg == NULL || cfg->tcp_port == 0 || cfg->sd_mount == NULL)
    {
        return -EINVAL;
    }

    memcpy(&s_cfg, cfg, sizeof(s_cfg));

    /* tx buffer: outer frame(5) + EVT_ID(2) + chunk hdr(11) + data + slack */
    s_tx_buf_cap = (uint16_t)(WIFI_UPLOAD_CHUNK_CAP + 16u + FRAME_OVERHEAD);
    s_tx_buf = (uint8_t *)os_mem_alloc(OS_MEM_TYPE_DATA, s_tx_buf_cap);
    if (s_tx_buf == NULL)
    {
        return -ENOMEM;
    }

    if (!wifi_sdio_data_read_cb_reg(0, s_cfg.tcp_port, on_tcp_rx))
    {
        os_mem_free(s_tx_buf);
        s_tx_buf = NULL;
        return -EBUSY;
    }

    memset(&s_upload, 0, sizeof(s_upload));
    memset(&s_scan, 0, sizeof(s_scan));
    s_tx_seq = 0;                /* first frame will use seq=1 */
    s_state  = STATE_IDLE;

    /* Create clk_mgr user for boosting CPU+SPIC0 during WiFi upload. */
    if (clk_mgr_upload_handle == NULL)
    {
        U_CLK_BITMAP bitmap;
        bitmap.data = BIT(T_CLK_TYPE_CPU) | BIT(T_CLK_TYPE_SPIC0);
        clk_mgr_upload_handle = clk_mgr_user_create("wifi_upload", bitmap);
    }
    s_inited = true;

    APP_PRINT_INFO1("[wifi_upload] init port=%u", s_cfg.tcp_port);
    return 0;
}

void wifi_file_upload_deinit(void)
{
    if (!s_inited)
    {
        return;
    }

    close_upload();
    close_scan();

    (void)wifi_sdio_data_read_cb_unreg(0, s_cfg.tcp_port);
    if (s_tx_buf)
    {
        os_mem_free(s_tx_buf);
        s_tx_buf = NULL;
    }
    s_state  = STATE_IDLE;
    s_inited = false;
}

bool wifi_file_upload_is_ready(void)
{
    return s_inited;
}

bool wifi_file_upload_is_busy(void)
{
    return (s_state == STATE_UPLOADING || s_state == STATE_SCANNING);
}

void wifi_file_upload_restore_clk(void)
{
    if (clk_mgr_upload_handle)
    {
        clk_mgr_set_normal_performance(clk_mgr_upload_handle);
    }
}

void wifi_file_upload_test(void)
{
    T_WIFI_UPLOAD_CFG cfg =
    {
        .packet_size       = 1024,
        .buffer_check_size = 20480,
        .mode              = 0x00,
        .ic_type           = 0x13,            // RTL87x3EP
        .song_format_type  = 0x0F,            // AAC|MP3|FLAC|WAV
        .transport_cap     = 0x07,            // BLE|SPP|WiFi
        .tcp_port          = 5001,
        .sd_mount          = "/SD:",          // or NULL if no SD
    };
    wifi_file_upload_init(&cfg);
}
