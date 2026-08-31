/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * @file    spi_file_upload.c
 * @brief   SPI file upload module (8773GTP + external Wi-Fi IC).
 *
 * Implements the Songs_Transfer V4.1 file upload/scan protocol over the
 * SPI transport, mirroring wifi_file_upload.c (SDIO path) but using
 * AT+SKTSENDRAW transparent sends for TX and receiving TCP data frames
 * from the SPI AT engine for RX.
 *
 * Protocol consistency with BLE and SDIO paths:
 *   - Same CMD/EVT IDs (0x680, 0x693-0x696)
 *   - Same outer frame format [0xAA][Seq][Len:2 LE][ID:2 LE][Params...]
 *   - Same CRC16 pair-XOR algorithm
 *   - Same EVT flag values (START=0x00, CONTINUE=0x01, END=0x02, ERROR=0xFE)
 *   - Same file-format-from-extension heuristic
 */

#if defined(CONFIG_WIFI_8711_CMD)

#include <zephyr/fs/fs.h>
#include <zephyr/devicetree.h>
#include <zephyr/linker/devicetree_regions.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "trace.h"
#include "os_mem.h"
#include "spi_file_upload.h"
#include "app_spi_atcmd.h"
#include "app_spi_api.h"
#include "wdg.h"
#include "app_dlps.h"
#include "clk_mgr.h"
#include "app_timer.h"
#include "os_sched.h"
#include "os_task.h"
#include "os_sync.h"

/* Static staging frame for the bulk data push. Building each SPI frame here and
 * pushing it directly (app_spi_master_send_raw_data) avoids the per-frame
 * os_mem_alloc(~16 KB) that FAILED on the tight/fragmented libc heap and left
 * bulk_push pushing 0 bytes (uploads timed out on the phone). Placed in the
 * dedicated PSRAM region (same as the SPI master TX/RX buffers and the tp_test
 * blast frame) so it costs no SRAM; the region is noinit and never zeroed at
 * boot, but bulk_push fully rewrites the frame every iteration.
 * Layout: [AT(2)][len(2)][ TCP frame: AA/Seq/inner_len/evt_id + 11B chunk hdr +
 *          data ][CRC(4)]  -> data starts at offset 4+6+11 = 21. */
#if DT_NODE_EXISTS(DT_NODELABEL(psram0_spi_buf))
#define BULK_FRAME_PSRAM __attribute__((__section__(LINKER_DT_NODE_REGION_NAME(DT_NODELABEL(psram0_spi_buf)))))
#else
#define BULK_FRAME_PSRAM
#endif
static uint8_t s_bulk_spi_frame[SPI_XMIT_SIZE] __aligned(32) BULK_FRAME_PSRAM;

/* SD read staging buffers: bulk_push reads the file in large blocks (N whole
 * chunks per fs_read) and memcpy's each frame's data out of them, instead of one
 * fs_read per 16K frame. Fewer/larger fs_read calls = fewer SD multi-block
 * commands + less FS-layer per-call overhead. PSRAM-resident (frees no SRAM).
 *
 * BULK_SD_PIPELINE: when 1 (default), the fs_read runs on a dedicated background
 * task (bulk_sd_reader_task) into a ping-pong PAIR of buffers while bulk_push
 * drains the OTHER buffer into the SPI ring - so a slow SD read is overlapped
 * with the SPI transfer instead of starving the bus. This is what closes the
 * occasional slow throughput window. Set to 0 to fall back to a single buffer
 * read synchronously inside bulk_push. NOTE: the pipeline path uses a second
 * task + semaphores; validate on hardware.
 *
 * Per-buffer size = BULK_SD_STAGE_CHUNKS * SPI_XMIT_SIZE (must be >= the region
 * budget in the psram0_spi_buf overlay - see snippets/wifi_8711/wifi_8711.overlay).
 * 2 chunks/buffer (~2 frames / ~40ms drain) is enough for the reader to refill
 * the other buffer within one drain; the ping-pong pair then costs 2*32K=64K
 * (only +32K vs the old single 32K buffer). Bump to 3-4 for more tolerance to
 * SD-latency spikes, growing psram0_spi_buf accordingly. */
#define BULK_SD_STAGE_CHUNKS   2     /* chunks per staging buffer */

#ifndef BULK_SD_PIPELINE
#define BULK_SD_PIPELINE       1
#endif

#if BULK_SD_PIPELINE
#define BULK_SD_NBUF           2     /* ping-pong */
#else
#define BULK_SD_NBUF           1     /* single synchronous buffer */
#endif

static uint8_t s_bulk_sd_stage[BULK_SD_NBUF][BULK_SD_STAGE_CHUNKS * SPI_XMIT_SIZE]
__aligned(4) BULK_FRAME_PSRAM;

#if BULK_SD_PIPELINE
/* --- SD read-ahead pipeline state --------------------------------------- *
 * A single reader task is created once (lazily, from bulk_push). Each bulk_push
 * call is one "session": the consumer (bulk_push, on the SPI TX task) primes the
 * free buffers, kicks the reader via s_sd_go_sem, then alternately waits on
 * s_sd_ready_sem[i] (buffer filled) / gives s_sd_free_sem[i] (buffer drained).
 * The reader owns ALL fs_read()s during a session; the consumer never touches
 * the file until it has torn the session down (abort + wait for s_sd_idle_sem),
 * which guarantees no fs_read is in flight when bulk_push calls fs_close().    */
#define BULK_SD_TASK_STACK     2048
#define BULK_SD_TASK_PRIORITY  1        /* same as the SPI TX blast task */
#define BULK_SD_SEM_MAX        4        /* > worst-case outstanding gives */
#define BULK_SD_WAIT_MS        5000     /* consumer safety timeout (not hit normally) */

static void    *s_sd_task;                       /* reader task handle (created once) */
static void    *s_sd_go_sem;                     /* consumer -> reader: start a session */
static void    *s_sd_idle_sem;                   /* reader -> consumer: parked (no I/O) */
static void    *s_sd_free_sem[BULK_SD_NBUF];     /* consumer -> reader: buffer i free */
static void    *s_sd_ready_sem[BULK_SD_NBUF];    /* reader -> consumer: buffer i filled */
static volatile ssize_t  s_sd_len[BULK_SD_NBUF]; /* bytes read into buffer i (<=0: EOF/err) */
static volatile uint32_t s_sd_read_off;          /* reader's file position (bytes read) */
static volatile uint32_t s_sd_read_want;         /* bytes per fs_read (= stage_cap) */
static volatile bool     s_sd_abort;             /* consumer -> reader: end the session */

static void bulk_sd_reader_task(void *p_param);

/* Create the reader task + semaphores once. Returns false if creation failed
 * (caller then falls back to the synchronous single-buffer read path). */
static bool bulk_sd_pipeline_ensure(void)
{
    if (s_sd_task != NULL)
    {
        return true;
    }
    bool ok = true;
    ok &= os_sem_create(&s_sd_go_sem,   "sd_go",   0, BULK_SD_SEM_MAX);
    ok &= os_sem_create(&s_sd_idle_sem, "sd_idle", 0, BULK_SD_SEM_MAX);
    for (uint32_t i = 0; i < BULK_SD_NBUF; i++)
    {
        ok &= os_sem_create(&s_sd_free_sem[i],  "sd_free",  0, BULK_SD_SEM_MAX);
        ok &= os_sem_create(&s_sd_ready_sem[i], "sd_ready", 0, BULK_SD_SEM_MAX);
    }
    if (!ok)
    {
        APP_PRINT_ERROR0("bulk_sd_pipeline: sem create failed");
        return false;
    }
    if (!os_task_create(&s_sd_task, "spi_sd_rd", bulk_sd_reader_task, NULL,
                        BULK_SD_TASK_STACK, BULK_SD_TASK_PRIORITY))
    {
        APP_PRINT_ERROR0("bulk_sd_pipeline: task create failed");
        s_sd_task = NULL;
        return false;
    }
    return true;
}

/* Drain any stale semaphore counts left by a previous (possibly aborted)
 * session, so each session starts from a known-zero state. */
static void bulk_sd_sem_drain(void)
{
    while (os_sem_take(s_sd_go_sem,   0)) { }
    while (os_sem_take(s_sd_idle_sem, 0)) { }
    for (uint32_t i = 0; i < BULK_SD_NBUF; i++)
    {
        while (os_sem_take(s_sd_free_sem[i],  0)) { }
        while (os_sem_take(s_sd_ready_sem[i], 0)) { }
    }
}

/* bulk_sd_reader_task() is defined after the s_upload context struct (it reads
 * s_upload.file / .total_len), just before spi_file_upload_bulk_push(). */
#endif /* BULK_SD_PIPELINE */

/* Byte offsets inside s_bulk_spi_frame (mirrors app_spi_atcmd.c framing;
 * SPI_FRAME_CRC_LEN there is not exported, so the 4-byte CRC field is local). */
#define BULK_SPI_HDR_LEN    4     /* ['A']['T'][len:2]                       */
#define BULK_SPI_CRC_LEN    4     /* trailing CRC32 (zeros when CRC disabled) */
#define BULK_TCP_HDR_LEN    6     /* [0xAA][Seq][inner_len:2][evt_id:2]      */
#define BULK_CHUNK_HDR_LEN  11    /* [flag][seq:2][crc:2][off:4][data_len:2] */
#define BULK_DATA_OFFSET    (BULK_SPI_HDR_LEN + BULK_TCP_HDR_LEN + BULK_CHUNK_HDR_LEN) /* 21 */

/*============================================================================*
 *                              Tunables
 *============================================================================*/

/** @brief TX staging buffer size (matches SPI_XMIT_SIZE from app_spi_api.h). */
#define SPI_UPLOAD_TX_BUF_SIZE      SPI_XMIT_SIZE

/** @brief SKTSENDRAW retry limit. */
#define SPI_SENDRAW_RETRY_MAX       3

/**
 * @brief Bulk-push throughput profiling (diagnostic only).
 *        When non-zero, spi_file_upload_bulk_push() logs the start/done
 *        timestamps and a per-window (~BULK_PROGRESS_FRAMES frames / ~2 MB)
 *        instantaneous rate ("[bulk] ... rate=...kbps"). Purely diagnostic - it
 *        has NO effect on the transfer - and is used to tell a WiFi-limited run
 *        (highly variable window rate) from an SPI-limited one (flat, near the
 *        ~800 KB/s per-frame ceiling). Default OFF for release; set to 1 to
 *        profile throughput.
 */
#ifndef SPI_UPLOAD_PROFILE
#define SPI_UPLOAD_PROFILE          0
#endif

/*============================================================================*
 *                              Module state
 *============================================================================*/

typedef enum
{
    STATE_IDLE = 0,
    STATE_UPLOADING,
    STATE_SCANNING,
} fsm_state_t;

static T_SPI_UPLOAD_CFG s_cfg;
static uint8_t          *s_tx_buf;
static fsm_state_t       s_state = STATE_IDLE;
static bool              s_inited;

/* clk_mgr user for boosting CPU + SPIC0 during file upload. */
static T_CLK_USER_HANDLE clk_mgr_upload_handle = NULL;

/* Timer IDs for upload/scan tick scheduling. */
static uint8_t s_timer_id         = 0;
static uint8_t s_timer_tick_handle = 0;

typedef enum
{
    SPI_UPLOAD_TIMER_TICK = 0x00,
} T_SPI_UPLOAD_TIMER;

/* Upload context --------------------------------------------------------- */
static struct
{
    struct fs_file_t  file;
    bool              file_open;
    char              path[SPI_UPLOAD_PATH_MAX];
    uint32_t          total_len;
    uint32_t          start_offset;
    uint32_t          cur_offset;
    uint16_t          tool_max_chunk;
    uint16_t          chunk_size;
    uint16_t          seq;
    uint16_t          whole_crc;
    uint8_t           format;
    bool              cancel_pending;
    bool              start_evt_sent;
    uint8_t           sendraw_retries;
    bool              bulk_active;        /* true between START-EVT OK and bulk OK */
} s_upload;

/* Scan context ----------------------------------------------------------- */
static struct
{
    uint8_t           filter;
    uint8_t           storage;
    uint8_t           batch_hint;
    struct fs_dir_t   dir;
    bool              dir_open;
    uint32_t          total_count;
    uint32_t          delivered;
    bool              cancel_pending;
    bool              start_evt_sent;
} s_scan;

/* TX staging buffer (shared, aligned for DMA compatibility). */
static uint8_t s_tx_buf_static[SPI_UPLOAD_TX_BUF_SIZE] __attribute__((aligned(32)));

/* TX sequence number (increments per EVT frame). */
static uint8_t s_tx_seq;

/*============================================================================*
 *                              Forward declarations
 *============================================================================*/

static void upload_tick(void);
static void scan_tick(void);
static void post_tick(void);
static bool send_spi_response(uint16_t evt_id, const uint8_t *body, uint16_t body_len);

/*============================================================================*
 *                              CRC16 pair-XOR helpers
 *============================================================================*/

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

/*============================================================================*
 *                              Byte-order helpers
 *============================================================================*/

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

/*============================================================================*
 *                              Frame protocol
 *============================================================================*/

#define FRAME_SYNC_BYTE     0xAA
#define FRAME_PREFIX_LEN    4       /* sync(1) + seq(1) + length(2) */
#define FRAME_OVERHEAD      FRAME_PREFIX_LEN

/*============================================================================*
 *                              Format-from-extension
 *============================================================================*/

static uint8_t format_from_ext(const char *name, uint16_t name_len)
{
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
        return SPI_FILE_FORMAT_BIN;
    }
    const char *ext = name + dot + 1;
    uint16_t    ext_len = name_len - (uint16_t)dot - 1;
    char        lower[8];
    if (ext_len >= sizeof(lower))
    {
        return SPI_FILE_FORMAT_BIN;
    }
    for (uint16_t i = 0; i < ext_len; i++)
    {
        lower[i] = (char)((ext[i] >= 'A' && ext[i] <= 'Z') ? (ext[i] + 0x20) : ext[i]);
    }
    lower[ext_len] = '\0';

    if (!strcmp(lower, "mp3"))  { return SPI_FILE_FORMAT_MP3; }
    if (!strcmp(lower, "mp4"))  { return SPI_FILE_FORMAT_MP4; }
    if (!strcmp(lower, "rtk"))  { return SPI_FILE_FORMAT_RTK; }
    if (!strcmp(lower, "aac"))  { return SPI_FILE_FORMAT_AAC; }
    if (!strcmp(lower, "flac")) { return SPI_FILE_FORMAT_FLAC; }
    if (!strcmp(lower, "txt"))  { return SPI_FILE_FORMAT_TXT; }
    if (!strcmp(lower, "dat"))  { return SPI_FILE_FORMAT_DAT; }
    if (!strcmp(lower, "opus")) { return SPI_FILE_FORMAT_OPUS; }
    return SPI_FILE_FORMAT_BIN;
}

/*============================================================================*
 *                              Path resolution
 *============================================================================*/

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

/*============================================================================*
 *                              Message posting (timer-based tick)
 *============================================================================*/

static void post_tick(void)
{
    app_start_timer(&s_timer_tick_handle, "spi_upload_tick",
                    s_timer_id, SPI_UPLOAD_TIMER_TICK, 0, false, 10);
}

static void spi_upload_timer_cb(uint8_t timer_evt, uint16_t param)
{
    (void)param;
    switch (timer_evt)
    {
    case SPI_UPLOAD_TIMER_TICK:
        app_stop_timer(&s_timer_tick_handle);
        if (s_state == STATE_UPLOADING)
        {
            upload_tick();
        }
        else if (s_state == STATE_SCANNING)
        {
            scan_tick();
        }
        break;
    default:
        break;
    }
}

/*============================================================================*
 *                              EVT frame builders
 *============================================================================*/

static uint16_t build_query_info_evt(uint8_t *buf)
{
    uint8_t *b = buf;
    memset(b, 0, 32);
    put_u16_le(b + 0, s_cfg.packet_size);
    put_u16_le(b + 2, s_cfg.buffer_check_size);
    b[4] = SPI_UPLOAD_PROTOCOL_VER;
    b[5] = s_cfg.mode;
    b[6] = s_cfg.ic_type;
    b[7] = s_cfg.song_format_type;
    b[8] = s_cfg.transport_cap;
    b[9] = SPI_TRANSPORT_WIFI;
    return 32;
}

static uint16_t build_upload_start_evt(uint8_t *buf, uint16_t crc, uint32_t total_len,
                                       uint8_t format, uint32_t timestamp)
{
    uint8_t *b = buf;
    b[0] = SPI_UPLOAD_FLAG_START;
    put_u16_le(b + 1, crc);
    put_u32_le(b + 3, total_len);
    b[7] = format;
    put_u32_le(b + 8, timestamp);
    return 12;
}

static uint16_t build_upload_chunk_evt(uint8_t *buf, uint8_t flag, uint16_t seq,
                                       uint16_t crc, uint32_t off, uint16_t data_len)
{
    uint8_t *b = buf;
    b[0] = flag;
    put_u16_le(b + 1, seq);
    put_u16_le(b + 3, crc);
    put_u32_le(b + 5, off);
    put_u16_le(b + 9, data_len);
    return (uint16_t)(11 + data_len);
}

static uint16_t build_upload_error_evt(uint8_t *buf, uint8_t err_code)
{
    buf[0] = SPI_UPLOAD_FLAG_ERROR;
    buf[1] = err_code;
    return 2;
}

static uint16_t build_upload_cancel_evt(uint8_t *buf, uint8_t result)
{
    buf[0] = result;
    return 1;
}

static uint16_t build_scan_start_evt(uint8_t *buf, uint32_t total)
{
    buf[0] = SPI_UPLOAD_FLAG_START;
    put_u32_le(buf + 1, total);
    return 5;
}

static uint16_t build_scan_entry_evt(uint8_t *buf, uint16_t idx, uint8_t format,
                                     uint8_t storage, uint32_t size, uint32_t mtime,
                                     uint16_t crc, const char *name, uint16_t name_len)
{
    uint8_t *b = buf;
    b[0] = SPI_UPLOAD_FLAG_CONTINUE;
    put_u16_le(b + 1, idx);
    b[3] = format;
    b[4] = storage;
    put_u32_le(b + 5, size);
    put_u32_le(b + 9, mtime);
    put_u16_le(b + 13, crc);
    put_u16_le(b + 15, name_len);
    memcpy(b + 17, name, name_len);
    return (uint16_t)(17 + name_len);
}

static uint16_t build_scan_end_evt(uint8_t *buf, uint32_t delivered)
{
    buf[0] = SPI_UPLOAD_FLAG_END;
    put_u32_le(buf + 1, delivered);
    return 5;
}

static uint16_t build_scan_error_evt(uint8_t *buf, uint8_t err_code)
{
    buf[0] = SPI_UPLOAD_FLAG_ERROR;
    buf[1] = err_code;
    return 2;
}

/*============================================================================*
 *                              SKTSENDRAW TX
 *============================================================================*/

/**
 * @brief  Send an EVT frame via AT+SKTSENDRAW two-phase transparent send.
 *
 *         Builds [0xAA][Seq][Len][Evt_ID][Params] in s_tx_buf, then:
 *           1. app_spi_atcmd_sendraw("AT+SKTSENDRAW=1,<len>", s_tx_buf, len)
 *           2. AT engine handles ">>>" prompt internally
 *           3. AT engine pushes raw data frame
 *           4. AT engine waits for "OK" (spi_file_upload_on_sendraw_ok())
 *
 * @param  evt_id    16-bit event ID (LE on wire).
 * @param  body      Payload bytes (may be NULL if body_len == 0).
 * @param  body_len  Payload length in bytes.
 * @return true if the SKTSENDRAW command was successfully queued.
 */
static bool send_spi_response(uint16_t evt_id, const uint8_t *body, uint16_t body_len)
{
    uint16_t inner_len = (uint16_t)(2 + body_len);   /* evt_id(2) + body */
    uint16_t frame_len = (uint16_t)(FRAME_PREFIX_LEN + inner_len);

    if (frame_len > SPI_UPLOAD_TX_BUF_SIZE)
    {
        APP_PRINT_ERROR2("send_spi_response: frame %d > buf %d",
                         frame_len, SPI_UPLOAD_TX_BUF_SIZE);
        return false;
    }

    /* Shift body up by 6 bytes to make room for header when body == s_tx_buf
     * (the common case from build_upload_* helpers).  memmove handles overlap
     * correctly; the old memcpy + memset destroyed the payload (self-overwrite). */
    if (body == s_tx_buf && body_len > 0)
    {
        memmove(s_tx_buf + 6, s_tx_buf, body_len);
    }
    memset(s_tx_buf, 0, 6);
    s_tx_buf[0] = FRAME_SYNC_BYTE;
    s_tx_buf[1] = s_tx_seq++;
    put_u16_le(s_tx_buf + 2, inner_len);
    put_u16_le(s_tx_buf + 4, evt_id);
    if (body != s_tx_buf && body && body_len)
    {
        memcpy(s_tx_buf + 6, body, body_len);
    }

    char cmd_line[64];
    snprintf(cmd_line, sizeof(cmd_line), "AT+SKTSENDRAW=1,%u\r\n",
             (unsigned)frame_len);

    WDG_Kick();

    if (!app_spi_atcmd_sendraw(cmd_line, s_tx_buf, frame_len))
    {
        s_upload.sendraw_retries++;
        if (s_upload.sendraw_retries >= SPI_SENDRAW_RETRY_MAX)
        {
            s_upload.sendraw_retries = 0;
            APP_PRINT_ERROR0("send_spi_response: SKTSENDRAW queue fail (max retry)");
            return false;
        }
        post_tick();
        return false;
    }
    app_spi_atcmd_trigger_send_flow();
    s_upload.sendraw_retries = 0;
    return true;
}

/*============================================================================*
 *                              Send error helpers
 *============================================================================*/

static void send_upload_error(uint8_t err)
{
    uint16_t n = build_upload_error_evt(s_tx_buf, err);
    (void)send_spi_response(SPI_UPLOAD_EVT_UPLOAD_FILE, s_tx_buf, n);
}

static void send_scan_error(uint8_t err)
{
    uint16_t n = build_scan_error_evt(s_tx_buf, err);
    (void)send_spi_response(SPI_UPLOAD_EVT_SCAN_FILES, s_tx_buf, n);
}

/*============================================================================*
 *                              Whole-file CRC
 *============================================================================*/

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
        *out_crc = crc16_pair_xor_finalize(cs);
    }
    if (out_size)
    {
        *out_size = total;
    }
    return 0;
}

/*============================================================================*
 *                              CMD: QUERY_INFO
 *============================================================================*/

static void handle_query_info(void)
{
    uint16_t n = build_query_info_evt(s_tx_buf);
    (void)send_spi_response(SPI_UPLOAD_EVT_QUERY_INFO, s_tx_buf, n);
}

/*============================================================================*
 *                              CMD: UPLOAD_FILE
 *============================================================================*/

static void handle_upload_file(const uint8_t *body, uint16_t body_len)
{
    if (body_len < 1 + 2)
    {
        send_upload_error(SPI_UPLOAD_ERR_PERMISSION);
        return;
    }

    uint8_t  transport = body[0];
    if (transport != SPI_TRANSPORT_WIFI && transport != SPI_TRANSPORT_BLE)
    {
        APP_PRINT_ERROR1("handle_upload_file: non-WiFi transport 0x%02x", transport);
        send_upload_error(SPI_UPLOAD_ERR_TRANSPORT);
        return;
    }
    if (s_state != STATE_IDLE)
    {
        APP_PRINT_ERROR1("handle_upload_file: busy state=%d", (int)s_state);
        send_upload_error(SPI_UPLOAD_ERR_TRANSPORT);
        return;
    }

    uint16_t name_len  = get_u16_le(body + 1);
    if (name_len == 0 || (uint32_t)body_len < 1U + 2U + name_len + 4U + 4U)
    {
        send_upload_error(SPI_UPLOAD_ERR_PERMISSION);
        return;
    }

    const uint8_t *name      = body + 3;
    uint32_t       start_off = get_u32_le(body + 3 + name_len);
    uint32_t       tool_max  = get_u32_le(body + 3 + name_len + 4);

    if (!resolve_path((const char *)name, name_len))
    {
        send_upload_error(SPI_UPLOAD_ERR_PERMISSION);
        return;
    }

    struct fs_dirent st;
    int rc = fs_stat(s_upload.path, &st);
    if (rc < 0 || st.type != FS_DIR_ENTRY_FILE)
    {
        APP_PRINT_ERROR2("handle_upload_file: fs_stat %s rc=%d",
                         TRACE_STRING(s_upload.path), rc);
        send_upload_error(SPI_UPLOAD_ERR_NOT_FOUND);
        return;
    }

    uint16_t whole_crc = 0;
#if CONFIG_WIFI_UPLOAD_CRC_ENABLE
    uint32_t verify_size = 0;
    if (compute_whole_crc(s_upload.path, &whole_crc, &verify_size) != 0)
    {
        APP_PRINT_WARN0("handle_upload_file: whole-file CRC skipped");
        whole_crc = 0;
    }
#endif

    s_state                 = STATE_UPLOADING;
    s_upload.total_len      = st.size;
    s_upload.start_offset   = (start_off > st.size) ? st.size : start_off;
    s_upload.cur_offset     = s_upload.start_offset;
    s_upload.tool_max_chunk = (tool_max > 0xFFFF) ? 0xFFFF : (uint16_t)tool_max;
    /* Use our own chunk cap (up to ~16 KB) regardless of the phone's
     * tool_max hint -- the SPI bus is the bottleneck, not the APK buffer. */
    s_upload.chunk_size     = SPI_UPLOAD_CHUNK_CAP;
    s_upload.seq            = 0;
    s_upload.whole_crc      = whole_crc;
    s_upload.format         = format_from_ext((const char *)name, name_len);
    s_upload.cancel_pending = false;
    s_upload.start_evt_sent = false;
    s_upload.file_open      = false;
    s_upload.sendraw_retries = 0;
    s_upload.bulk_active    = false;

    APP_PRINT_INFO3("handle_upload_file: path=%s size=%u start_off=%u",
                    TRACE_STRING(s_upload.path), s_upload.total_len, s_upload.start_offset);

    app_dlps_disable(APP_DLPS_ENTER_CHECK_INIT);

    if (clk_mgr_upload_handle)
    {
        clk_mgr_set_high_performance(clk_mgr_upload_handle);
    }

    post_tick();
}

/*============================================================================*
 *                              CMD: UPLOAD_CANCEL
 *============================================================================*/

static void handle_upload_cancel(const uint8_t *body, uint16_t body_len)
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
        s_state                = STATE_UPLOADING;
        s_upload.cancel_pending = true;
        post_tick();
        return;
    }
    post_tick();
}

/*============================================================================*
 *                              CMD: SCAN_FILES
 *============================================================================*/

static bool storage_supported(uint8_t filter_storage)
{
    return filter_storage == SPI_STORAGE_ALL || filter_storage == SPI_STORAGE_SD;
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

static void handle_scan_files(const uint8_t *body, uint16_t body_len)
{
    if (body_len < 3)
    {
        send_scan_error(SPI_SCAN_ERR_INTERNAL);
        return;
    }
    if (s_state != STATE_IDLE)
    {
        send_scan_error(SPI_SCAN_ERR_INTERNAL);
        return;
    }

    uint8_t filter     = body[0];
    uint8_t storage    = body[1];
    uint8_t batch_hint = body[2];

    if (!storage_supported(storage))
    {
        send_scan_error(SPI_SCAN_ERR_STORAGE);
        return;
    }

    s_state               = STATE_SCANNING;
    s_scan.filter         = filter;
    s_scan.storage        = storage;
    s_scan.batch_hint     = batch_hint;
    s_scan.dir_open       = false;
    s_scan.total_count    = 0;
    s_scan.delivered      = 0;
    s_scan.cancel_pending = false;
    s_scan.start_evt_sent = false;

    APP_PRINT_INFO2("handle_scan_files: filter=0x%02x storage=0x%02x", filter, storage);

    app_dlps_disable(APP_DLPS_ENTER_CHECK_INIT);
    post_tick();
}

/*============================================================================*
 *                              RX callback
 *============================================================================*/

bool spi_file_upload_on_tcp_rx(const uint8_t *frame, uint16_t frame_len)
{
    if (frame == NULL || frame_len < FRAME_PREFIX_LEN + 2)
    {
        return false;
    }

    /* Outer frame: [0xAA][Seq][Length:2][cmd_id:2][Params] */
    if (frame[0] != FRAME_SYNC_BYTE)
    {
        return false;
    }

    uint16_t inner_len = get_u16_le(frame + 2);
    if (inner_len < 2 || (uint32_t)frame_len < (uint32_t)FRAME_PREFIX_LEN + inner_len)
    {
        return false;
    }

    uint16_t       cmd_id = get_u16_le(frame + FRAME_PREFIX_LEN);
    const uint8_t *body   = frame + FRAME_PREFIX_LEN + 2;
    uint16_t       blen   = (uint16_t)(inner_len - 2);

    APP_PRINT_INFO2("spi_file_upload_on_tcp_rx: cmd=0x%04x len=%d", cmd_id, inner_len);

    switch (cmd_id)
    {
    case SPI_UPLOAD_CMD_QUERY_INFO:
        handle_query_info();
        break;
    case SPI_UPLOAD_CMD_UPLOAD_FILE:
        handle_upload_file(body, blen);
        break;
    case SPI_UPLOAD_CMD_UPLOAD_CANCEL:
        handle_upload_cancel(body, blen);
        break;
    case SPI_UPLOAD_CMD_SCAN_FILES:
        handle_scan_files(body, blen);
        break;
    default:
        APP_PRINT_WARN1("spi_file_upload_on_tcp_rx: unknown cmd 0x%04x", cmd_id);
        return false;
    }
    return true;
}

/*============================================================================*
 *                              SKTSENDRAW result callbacks
 *============================================================================*/

void spi_file_upload_on_sendraw_ok(void)
{
    APP_PRINT_TRACE0("spi_file_upload_on_sendraw_ok");

    if (s_state != STATE_UPLOADING)
    {
        return;
    }

    if (!s_upload.bulk_active)
    {
        /* START EVT was confirmed by the Wi-Fi module.
         * Open the file and begin the bulk data SKTSENDRAW. */
        s_upload.bulk_active = true;

        /* -- calculate the combined TCP-frame size of every chunk -- */
        uint32_t total_tcp = 0;
        uint32_t rem       = s_upload.total_len;
        while (rem > 0)
        {
            uint16_t chunk = (rem > s_upload.chunk_size)
                             ? s_upload.chunk_size : (uint16_t)rem;
            total_tcp += (uint32_t)(17 + chunk);   /* 6-byte header + 11-byte body + data */
            rem -= chunk;
        }

        /* queue a single AT+SKTSENDRAW that covers the whole file */
        app_spi_atcmd_set_bulk_mode(true, total_tcp);
        char cmd_line[40];
        snprintf(cmd_line, sizeof(cmd_line), "AT+SKTSENDRAW=1,%u\r\n",
                 (unsigned)total_tcp);
        app_spi_atcmd_queue_fill(ATCMD_SENDRAW, cmd_line);
        app_spi_atcmd_trigger_send_flow();
    }
    else
    {
        /* Bulk-data SKTSENDRAW confirmed -- send the END EVT. */
        s_upload.bulk_active = false;

        uint16_t m = build_upload_chunk_evt(s_tx_buf, SPI_UPLOAD_FLAG_END,
                                            s_upload.seq, 0,
                                            s_upload.cur_offset, 0);
        if (!send_spi_response(SPI_UPLOAD_EVT_UPLOAD_FILE, s_tx_buf, m))
        {
            post_tick();                     /* retry via timer */
            return;
        }

        /* file already closed by bulk_push */
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        if (clk_mgr_upload_handle)
        {
            clk_mgr_set_normal_performance(clk_mgr_upload_handle);
        }
        s_state = STATE_IDLE;
    }
}

void spi_file_upload_on_sendraw_error(void)
{
    APP_PRINT_WARN0("spi_file_upload_on_sendraw_error");

    if (s_state == STATE_UPLOADING)
    {
        s_upload.sendraw_retries++;
        if (s_upload.sendraw_retries >= SPI_SENDRAW_RETRY_MAX)
        {
            APP_PRINT_ERROR0("spi_file_upload: SKTSENDRAW error (max retry), abort");
            send_upload_error(SPI_UPLOAD_ERR_TRANSPORT);
            if (s_upload.file_open)
            {
                fs_close(&s_upload.file);
                s_upload.file_open = false;
            }
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            if (clk_mgr_upload_handle)
            {
                clk_mgr_set_normal_performance(clk_mgr_upload_handle);
            }
            s_state = STATE_IDLE;
            return;
        }
        APP_PRINT_INFO1("spi_file_upload: SKTSENDRAW retry %d",
                        s_upload.sendraw_retries);
        post_tick();
    }
}

/*============================================================================*
 *                              Bulk data push (stream mode)
 *============================================================================*/

#if BULK_SD_PIPELINE
/* Reader task: waits for a session (s_sd_go_sem), then fills the ping-pong
 * buffers ahead of the consumer until EOF / read error / abort, parks on
 * s_sd_idle_sem, and loops back to wait for the next session. Defined here (not
 * with the other pipeline helpers) because it dereferences the s_upload context
 * struct, which is declared later in the file. */
static void bulk_sd_reader_task(void *p_param)
{
    (void)p_param;
    for (;;)
    {
        os_sem_take(s_sd_go_sem, 0xFFFFFFFFUL);

        uint32_t ri = 0;
        while (!s_sd_abort)
        {
            /* Wait for buffer ri to be free (consumer drained it). */
            if (!os_sem_take(s_sd_free_sem[ri], 0xFFFFFFFFUL) || s_sd_abort)
            {
                break;
            }

            uint32_t remain = (s_sd_read_off < s_upload.total_len)
                              ? (s_upload.total_len - s_sd_read_off) : 0;
            if (remain == 0)
            {
                s_sd_len[ri] = 0;                 /* EOF marker */
                os_sem_give(s_sd_ready_sem[ri]);
                break;
            }

            uint32_t want = (remain > s_sd_read_want) ? s_sd_read_want : remain;
            ssize_t  got  = fs_read(&s_upload.file, s_bulk_sd_stage[ri], want);
            s_sd_len[ri] = got;
            if (got > 0)
            {
                s_sd_read_off += (uint32_t)got;
            }
            os_sem_give(s_sd_ready_sem[ri]);
            if (got <= 0)                          /* error or EOF: end session */
            {
                break;
            }
            ri ^= 1;
        }

        os_sem_give(s_sd_idle_sem);                /* parked: no fs_read in flight */
    }
}
#endif /* BULK_SD_PIPELINE */

void spi_file_upload_bulk_push(uint32_t total_bytes)
{
    (void)total_bytes;   /* used only for the resend timeout (set in app_spi_atcmd) */

    if (!s_upload.file_open)
    {
        APP_PRINT_ERROR0("bulk_push: file not open");
        return;
    }

#if SPI_UPLOAD_PROFILE
    /* Lightweight throughput probe: log offset + elapsed + instantaneous rate
     * once every BULK_PROGRESS_FRAMES frames (~10 lines for a 20 MB file), so we
     * can plot the rate curve without the per-frame log flood. Diagnostic only,
     * gated by SPI_UPLOAD_PROFILE. */
#define BULK_PROGRESS_FRAMES   128
    uint32_t probe_frames   = 0;
    uint32_t probe_last_ms  = os_sys_time_get();
    uint32_t probe_last_off = s_upload.cur_offset;

    APP_PRINT_INFO2("bulk_push: start, offset=%u t=%ums",
                    s_upload.cur_offset, probe_last_ms);
#endif

    /* payload cap per frame: keep the whole SPI frame within SPI_XMIT_SIZE */
    const uint16_t payload_cap =
        (uint16_t)(SPI_XMIT_SIZE - BULK_DATA_OFFSET - BULK_SPI_CRC_LEN);
    const uint16_t chunk = (s_upload.chunk_size > payload_cap) ? payload_cap
                           : s_upload.chunk_size;

    /* SD read staging: fill with N whole chunks per fs_read (so no runt frames
     * mid-file), then slice each frame's data out with a memcpy. stage_cap is a
     * multiple of `chunk`; the final read at EOF returns a shorter tail. */
    const uint32_t stage_cap = (uint32_t)BULK_SD_STAGE_CHUNKS * chunk;
    uint32_t stage_len = 0;   /* valid bytes currently in the active staging buffer */
    uint32_t stage_pos = 0;   /* bytes already consumed from the active staging buffer */
    uint8_t *stage_buf = s_bulk_sd_stage[0];   /* active staging buffer */

#if BULK_SD_PIPELINE
    /* Start a read-ahead session: a background task fills one ping-pong buffer
     * while we drain the other, so a slow fs_read is overlapped with the SPI
     * transfer instead of starving the bus. Falls back to synchronous single-
     * buffer reads if the task/sem setup failed. */
    const bool pipeline = bulk_sd_pipeline_ensure();
    uint32_t   ci = 0;                          /* consumer's ping-pong index */
    if (pipeline)
    {
        bulk_sd_sem_drain();                    /* clear any stale session state */
        s_sd_abort     = false;
        s_sd_read_off  = s_upload.cur_offset;   /* reader starts where we are */
        s_sd_read_want = stage_cap;
        for (uint32_t i = 0; i < BULK_SD_NBUF; i++)
        {
            s_sd_len[i] = 0;
            os_sem_give(s_sd_free_sem[i]);      /* both buffers free to fill */
        }
        os_sem_give(s_sd_go_sem);               /* wake the reader */
    }
#else
    const bool pipeline = false;
#endif

    while (s_upload.cur_offset < s_upload.total_len)
    {
        WDG_Kick();   /* long loop on the TX task: keep the watchdog fed */

        /* Refill the active staging buffer when drained. */
        if (stage_pos >= stage_len)
        {
#if BULK_SD_PIPELINE
            if (pipeline)
            {
                /* Take the buffer the reader has filled ahead of us. */
                if (!os_sem_take(s_sd_ready_sem[ci], BULK_SD_WAIT_MS))
                {
                    APP_PRINT_ERROR0("bulk_push: SD read-ahead timeout");
                    break;
                }
                ssize_t got = s_sd_len[ci];
                if (got <= 0)
                {
                    if (got < 0)
                    {
                        APP_PRINT_ERROR1("bulk_push: fs_read rc=%d", (int)got);
                    }
                    break;   /* EOF or error */
                }
                stage_buf = s_bulk_sd_stage[ci];
                stage_len = (uint32_t)got;
                stage_pos = 0;
            }
            else
#endif
            {
                uint32_t remain_file = s_upload.total_len - s_upload.cur_offset;
                uint32_t want_read   = (remain_file > stage_cap) ? stage_cap : remain_file;
                ssize_t  got_stage   = fs_read(&s_upload.file, s_bulk_sd_stage[0], want_read);
                if (got_stage <= 0)
                {
                    if (got_stage < 0)
                    {
                        APP_PRINT_ERROR1("bulk_push: fs_read rc=%d", (int)got_stage);
                    }
                    break;
                }
                stage_buf = s_bulk_sd_stage[0];
                stage_len = (uint32_t)got_stage;
                stage_pos = 0;
            }
        }

        uint32_t avail = stage_len - stage_pos;
        uint16_t want  = (avail > chunk) ? chunk : (uint16_t)avail;

        /* Slice this frame's data out of the staging buffer into the SPI frame. */
        memcpy(s_bulk_spi_frame + BULK_DATA_OFFSET, stage_buf + stage_pos, want);
        stage_pos += want;

        uint16_t n_data = want;
        uint8_t  flag   = ((s_upload.cur_offset + n_data) >= s_upload.total_len)
                          ? SPI_UPLOAD_FLAG_END : SPI_UPLOAD_FLAG_CONTINUE;

        /* chunk header at [10..20]; data already at [21..] */
        (void)build_upload_chunk_evt(s_bulk_spi_frame + BULK_SPI_HDR_LEN + BULK_TCP_HDR_LEN,
                                     flag, s_upload.seq, 0, s_upload.cur_offset, n_data);

        /* TCP frame header [AA][Seq][inner_len][evt_id=0x693] at [4..9] */
        uint16_t body_len  = (uint16_t)(BULK_CHUNK_HDR_LEN + n_data);
        uint16_t inner_len = (uint16_t)(2 + body_len);                 /* evt_id(2) + body   */
        uint16_t tcp_len   = (uint16_t)(BULK_TCP_HDR_LEN + body_len);  /* AA..evt_id + body  */
        uint8_t *tcp = s_bulk_spi_frame + BULK_SPI_HDR_LEN;
        tcp[0] = FRAME_SYNC_BYTE;        /* 0xAA */
        tcp[1] = (uint8_t)(s_tx_seq++);
        put_u16_le(tcp + 2, inner_len);
        put_u16_le(tcp + 4, SPI_UPLOAD_EVT_UPLOAD_FILE);

        /* SPI frame header [AT][tcp_len] at [0..3] + CRC(4, zeros) after the TCP frame */
        uint16_t spi_len = (uint16_t)(BULK_SPI_HDR_LEN + tcp_len + BULK_SPI_CRC_LEN);
        s_bulk_spi_frame[0] = 'A';       /* SPI_PROTO_MAGIC_0 */
        s_bulk_spi_frame[1] = 'T';       /* SPI_PROTO_MAGIC_1 */
        put_u16_le(s_bulk_spi_frame + 2, tcp_len);
        memset(s_bulk_spi_frame + BULK_SPI_HDR_LEN + tcp_len, 0, BULK_SPI_CRC_LEN);

        uint8_t ret;
        while ((ret = app_spi_master_send_raw_data(s_bulk_spi_frame, spi_len)) == SPI_SEND_ERR_BUSY)
        {
            os_delay(1);
        }

        if (ret != SPI_SEND_SUC)
        {
            APP_PRINT_ERROR1("bulk_push: SPI send fail %d", ret);
            break;
        }

        s_upload.cur_offset += n_data;
        s_upload.seq++;

#if SPI_UPLOAD_PROFILE
        /* Every N frames: dump offset + elapsed + instantaneous rate over the
         * last window. Cheap (once per ~128 frames), no per-frame flood. */
        if (++probe_frames >= BULK_PROGRESS_FRAMES)
        {
            uint32_t now   = os_sys_time_get();
            uint32_t dt_ms = now - probe_last_ms;
            if (dt_ms == 0) { dt_ms = 1; }
            uint32_t dbytes = s_upload.cur_offset - probe_last_off;
            /* rate in kbit/s = dbytes * 8 / dt_ms (both /1000 cancel) */
            uint32_t kbps_bit = (uint32_t)(((uint64_t)dbytes * 8) / dt_ms); /* kbit/s */
            APP_PRINT_INFO4("[bulk] off=%u dt=%ums win=%uB rate=%ukbps",
                            s_upload.cur_offset, dt_ms, dbytes, kbps_bit);
            probe_frames   = 0;
            probe_last_ms  = now;
            probe_last_off = s_upload.cur_offset;
        }
#endif

        if (flag == SPI_UPLOAD_FLAG_END)
        {
            break;
        }

#if BULK_SD_PIPELINE
        /* Buffer fully drained: hand it back to the reader and flip to the one
         * it has been filling in the meantime. */
        if (pipeline && stage_pos >= stage_len)
        {
            os_sem_give(s_sd_free_sem[ci]);
            ci ^= 1;
        }
#endif

        /* Yield periodically so higher-priority tasks (BLE, timers) stay alive */
        if ((s_upload.seq & 0x3F) == 0)
        {
            os_delay(0);
        }
    }

#if BULK_SD_PIPELINE
    /* Tear the read-ahead session down BEFORE closing the file: signal abort,
     * unblock the reader if it is waiting on a free buffer, then wait until it
     * has parked (guaranteeing no fs_read is in flight on s_upload.file). */
    if (pipeline)
    {
        s_sd_abort = true;
        for (uint32_t i = 0; i < BULK_SD_NBUF; i++)
        {
            os_sem_give(s_sd_free_sem[i]);
        }
        if (!os_sem_take(s_sd_idle_sem, BULK_SD_WAIT_MS))
        {
            APP_PRINT_ERROR0("bulk_push: SD reader did not park");
        }
    }
#endif

    /* Close file (the END-EVT SPI frame was already pushed if flag==END) */
    if (s_upload.file_open)
    {
        fs_close(&s_upload.file);
        s_upload.file_open = false;
    }

#if SPI_UPLOAD_PROFILE
    APP_PRINT_INFO2("bulk_push: done, off=%u t=%ums",
                    s_upload.cur_offset, os_sys_time_get());
#endif
}

/*============================================================================*
 *                              Tick handlers
 *============================================================================*/

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
    WDG_Kick();

    if (s_upload.cancel_pending)
    {
        close_upload();
        uint16_t n = build_upload_cancel_evt(s_tx_buf, 0x01);
        (void)send_spi_response(SPI_UPLOAD_EVT_UPLOAD_CANCEL, s_tx_buf, n);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        if (clk_mgr_upload_handle)
        {
            clk_mgr_set_normal_performance(clk_mgr_upload_handle);
        }
        s_state = STATE_IDLE;
        return;
    }

    if (!s_upload.start_evt_sent)
    {
        uint32_t deliver = (s_upload.start_offset >= s_upload.total_len)
                           ? 0
                           : (s_upload.total_len - s_upload.start_offset);

        uint16_t n = build_upload_start_evt(s_tx_buf, s_upload.whole_crc,
                                            deliver, s_upload.format, 0);
        if (!send_spi_response(SPI_UPLOAD_EVT_UPLOAD_FILE, s_tx_buf, n))
        {
            post_tick();
            return;
        }

        fs_file_t_init(&s_upload.file);
        int rc = fs_open(&s_upload.file, s_upload.path, FS_O_READ);
        if (rc < 0)
        {
            APP_PRINT_ERROR2("upload_tick: fs_open %s rc=%d",
                             TRACE_STRING(s_upload.path), rc);
            send_upload_error(SPI_UPLOAD_ERR_READ);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            if (clk_mgr_upload_handle)
            {
                clk_mgr_set_normal_performance(clk_mgr_upload_handle);
            }
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
                send_upload_error(SPI_UPLOAD_ERR_READ);
                app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
                if (clk_mgr_upload_handle)
                {
                    clk_mgr_set_normal_performance(clk_mgr_upload_handle);
                }
                s_state = STATE_IDLE;
                return;
            }
        }
        s_upload.cur_offset     = s_upload.start_offset;
        s_upload.seq            = 0;
        s_upload.start_evt_sent = true;

        if (deliver == 0)
        {
            uint16_t m = build_upload_chunk_evt(s_tx_buf, SPI_UPLOAD_FLAG_END,
                                                s_upload.seq, 0,
                                                s_upload.cur_offset, 0);
            (void)send_spi_response(SPI_UPLOAD_EVT_UPLOAD_FILE, s_tx_buf, m);
            close_upload();
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            if (clk_mgr_upload_handle)
            {
                clk_mgr_set_normal_performance(clk_mgr_upload_handle);
            }
            s_state = STATE_IDLE;
            return;
        }
        return;
    }

    /* START EVT was sent -- on_sendraw_ok() will begin the bulk push */
}

static void scan_tick(void)
{
    WDG_Kick();

    if (s_scan.cancel_pending)
    {
        close_scan();
        uint16_t n = build_upload_cancel_evt(s_tx_buf, 0x01);
        (void)send_spi_response(SPI_UPLOAD_EVT_UPLOAD_CANCEL, s_tx_buf, n);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        s_state = STATE_IDLE;
        return;
    }

    if (!s_scan.start_evt_sent)
    {
        uint32_t total = count_matching();
        s_scan.total_count = total;
        uint16_t n = build_scan_start_evt(s_tx_buf, total);
        (void)send_spi_response(SPI_UPLOAD_EVT_SCAN_FILES, s_tx_buf, n);

        s_scan.start_evt_sent = true;

        if (total == 0)
        {
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        if (!open_sd_dir())
        {
            n = build_scan_end_evt(s_tx_buf, 0);
            (void)send_spi_response(SPI_UPLOAD_EVT_SCAN_FILES, s_tx_buf, n);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        post_tick();
        return;
    }

    if (!s_scan.dir_open)
    {
        uint16_t n = build_scan_end_evt(s_tx_buf, s_scan.delivered);
        (void)send_spi_response(SPI_UPLOAD_EVT_SCAN_FILES, s_tx_buf, n);
        app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
        s_state = STATE_IDLE;
        return;
    }

    bool found = false;
    struct fs_dirent ent;
    uint16_t name_len;

    while (!found)
    {
        int rc = fs_readdir(&s_scan.dir, &ent);
        if (rc < 0 || ent.name[0] == '\0')
        {
            close_scan();
            uint16_t n = build_scan_end_evt(s_tx_buf, s_scan.delivered);
            (void)send_spi_response(SPI_UPLOAD_EVT_SCAN_FILES, s_tx_buf, n);
            app_dlps_enable(APP_DLPS_ENTER_CHECK_INIT);
            s_state = STATE_IDLE;
            return;
        }
        if (ent.type != FS_DIR_ENTRY_FILE)
        {
            continue;
        }
        name_len = (uint16_t)strlen(ent.name);
        if (!ext_matches(ent.name, name_len, s_scan.filter))
        {
            continue;
        }
        found = true;
    }

    uint16_t hdr_total = (uint16_t)(FRAME_PREFIX_LEN + 19);
    uint16_t max_name  = (SPI_UPLOAD_TX_BUF_SIZE > hdr_total)
                         ? (uint16_t)(SPI_UPLOAD_TX_BUF_SIZE - hdr_total) : 0;
    if (name_len > max_name)
    {
        name_len = max_name;
    }

    uint8_t format = s_scan.filter;
    uint16_t n = build_scan_entry_evt(s_tx_buf, (uint16_t)s_scan.delivered,
                                      format, SPI_STORAGE_SD,
                                      (uint32_t)ent.size, 0, 0,
                                      ent.name, name_len);
    (void)send_spi_response(SPI_UPLOAD_EVT_SCAN_FILES, s_tx_buf, n);
    s_scan.delivered++;
    post_tick();
}

/*============================================================================*
 *                              Public API
 *============================================================================*/

int spi_file_upload_init(const T_SPI_UPLOAD_CFG *cfg)
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
    s_tx_buf = s_tx_buf_static;

    if (s_timer_id == 0)
    {
        app_timer_reg_cb(spi_upload_timer_cb, &s_timer_id);
    }

    memset(&s_upload, 0, sizeof(s_upload));
    memset(&s_scan, 0, sizeof(s_scan));
    s_tx_seq = 0;
    s_state  = STATE_IDLE;

    if (clk_mgr_upload_handle == NULL)
    {
        U_CLK_BITMAP bitmap;
        bitmap.data = BIT(T_CLK_TYPE_CPU) | BIT(T_CLK_TYPE_SPIC0);
        clk_mgr_upload_handle = clk_mgr_user_create("spi_upload", bitmap);
    }

    s_inited = true;
    APP_PRINT_INFO1("spi_file_upload_init: ok, port=%u", s_cfg.tcp_port);
    return 0;
}

void spi_file_upload_deinit(void)
{
    if (!s_inited)
    {
        return;
    }
    close_upload();
    close_scan();
    app_stop_timer(&s_timer_tick_handle);
    s_state  = STATE_IDLE;
    s_inited = false;
    APP_PRINT_INFO0("spi_file_upload_deinit");
}

bool spi_file_upload_is_ready(void)
{
    return s_inited;
}

bool spi_file_upload_is_busy(void)
{
    return (s_state == STATE_UPLOADING || s_state == STATE_SCANNING);
}

void spi_file_upload_restore_clk(void)
{
    if (clk_mgr_upload_handle)
    {
        clk_mgr_set_normal_performance(clk_mgr_upload_handle);
    }
}

#endif /* CONFIG_WIFI_8711_CMD */
