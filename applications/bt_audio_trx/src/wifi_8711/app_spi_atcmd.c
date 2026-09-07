/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include <stdio.h>
#include "trace.h"
#include "app_spi_atcmd.h"
#include "os_mem.h"
#include "os_queue.h"
#include "os_task.h"
#include "os_msg.h"
#include "os_sched.h"
#include "app_timer.h"
#include "app_util.h"
#include "app_spi_api.h"
#include "app_spi_common.h"
#include "app_spi_sd_source.h"
#include "wifi_8711_app.h"
#include "spi_file_upload.h"   /* spi_file_upload_on_tcp_rx(), on_sendraw_ok/error() */
#include "pm.h"              /* pm_cpu_freq_req/clear - boost AHB clock for the blast */
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>   /* k_cycle_get_32() for the TX producer profiler */

/* ---- TX producer (blast) phase profiler -----------------------------------
 * The app-side tp-prof already showed the per-frame budget as hs + hop + xfer
 * + a ~2.6ms "producer gap" that period-minus-the-measured-phases leaves as a
 * black box. That gap is the single-buffer producer serialising the next
 * frame's fill behind the current frame's wire (one SRAM DMA buffer). Split it
 * into the three steps of tp_tx_blast_frame() so we can see WHICH step stalls:
 *   wait  time blocked in tx_wait_free()  (overlaps the in-flight wire)
 *   fill  building the 16K frame          (SD read OR the 0..127 ramp loop)
 *   send  app_spi_master_send_prefilled() (just submits the transfer)
 * Averages print once per SPI_TX_PROFILE_N frames. Set 0 to compile out. */
#define SPI_TX_PROFILE      0
#define SPI_TX_PROFILE_N    100
#if SPI_TX_PROFILE
static uint32_t s_txp_n, s_txp_wait, s_txp_fill, s_txp_send;
static inline uint32_t txp_us(uint32_t cyc)
{
    return (uint32_t)(((uint64_t)cyc * 1000000u) / CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC);
}
#endif

/* payload_len of the ramp frame currently resident in the single SRAM DMA
 * buffer. The 0..127 ramp payload is identical for every full-size frame, so
 * tp_tx_blast_frame() builds it once and reuses the resident buffer on the next
 * same-size frame instead of re-running the ~2.6ms byte-by-byte fill that
 * stalled SCK between frames. Reset to 0 at each blast start (the buffer may
 * have been overwritten by other SPI traffic between blasts). */
static uint16_t s_blast_built_len;

/* CPU/system-clock boost for the duration of a blast.
 *
 * The SPI1 SCK is sourced from PLL1 (200MHz / div2 = 100MHz, prescaler 2 =>
 * 50MHz) and is independent of the CPU clock. But the GDMA + AHB bus that feed
 * the 32-entry SPI FIFO run on the *system/CPU* clock, which the app pins at
 * 40MHz (F_APP_MULTI_CHANNEL_SUPPORT=0). A full-duplex 50MHz frame needs the
 * DMA to refill TX and drain RX every ~16 bytes; at 40MHz each request->
 * arbitrate->burst round trip costs ~2.35us, and the master auto-stalls SCK
 * (no error flag) until the FIFO is serviced - the ~50% SCK duty cycle seen on
 * the LA (2.35us on / 2.35us off) and the ~2x wire time (5031us vs 2621us
 * ideal). Raising the system clock speeds the AHB/GDMA so the FIFO stays fed.
 *
 * Held via a pm_cpu_freq_req() floor (coexists with DLPS, which is also held
 * off for the blast) and cleared at blast end. This is a per-blast floor - NOT
 * the global clk_mgr normal baseline (clk_modules_normal_freq[CPU], which would
 * run the whole app at this clock and gets overridden by app_main's
 * pm_cpu_freq_set(40) anyway). 200MHz is clk_modules_high_freq[CPU] and the
 * rate ai_record file transfer already uses, so it is a validated CPU point;
 * SCK stays 50MHz (independent PLL1). The pm framework reports the granted
 * clock in `actual` (logged below) in case it is clamped. */
#define SPI_BLAST_CPU_MHZ   200
static uint8_t s_blast_cpu_freq_handle;

/* spi_atcmd_flow_ctrl_handler() is deferred onto the dedicated wifi_8711 task
 * (see wifi_8711_app.c) rather than run inline in the caller's context (the SPI
 * RX parse callback / the app-timer callback): app_spi_atcmd_trigger_send_flow()
 * just posts WIFI_8711_EVENT_ATCMD_FLOW_CTRL and the wifi_8711 task runs the
 * handler. The blast TX streaming task (SPI_TCP_TP_TX_TASK, below) still runs on
 * its own low-priority task so a long flood never blocks RX. */
#define MIN_RSP_LEN         2
#define MAX_RSP_LEN         1024
#define ATCMD_RSP_OK        "OK"
#define ATCMD_RSP_ERR       "ERROR"
#define ATCMD_RESEND_CNT    0

/* Debug: dump every frame received from the SPI slave whose payload is shorter
 * than SPI_RX_DUMP_THRESHOLD bytes as a hex ASCII string (set to 0 to disable).
 * Off by default: the throughput test only wants the per-packet count + size,
 * not the payload content (see the "rx pkt #" trace in spi_atcmd_rcv_cb). */
#ifndef SPI_RX_DUMP_SHORT_FRAME
#define SPI_RX_DUMP_SHORT_FRAME     0
#endif
#define SPI_RX_DUMP_THRESHOLD       128
#define SPI_RX_DUMP_BYTES_PER_LINE  32

typedef struct
{
    uint8_t     magic[2];   // "AT"
    uint16_t    len;        // Payload Length
    uint8_t     payload[];  // Variable length data
} __attribute__((packed)) spi_frame_hdr_t;

/**
 *  @brief SPI frame with FrameType discriminator.
 *
 *  Format: [AT(2)][Len(2)][FrameType(1)][Payload(N-1)][CRC32(4)]
 *
 *  FrameType (first byte of Payload):
 *    0x00 -- AT command response -> atcmd_parser_process()
 *    0x01 -- TCP data frame      -> spi_file_upload_on_tcp_rx()
 *    other -- legacy (no FrameType) -> treated as AT response
 */
#define SPI_PROTO_MAGIC_0   'A'
#define SPI_PROTO_MAGIC_1   'T'
#define SPI_FRAME_HDR_LEN   4  // Magic(2) + Len(2)
#define SPI_FRAME_CRC_LEN   4  // Checksum(4)

/** @brief FrameType: AT response (goes to AT command parser). */
#define SPI_FRAMETYPE_AT_RSP       0x00
/** @brief FrameType: TCP data (goes to spi_file_upload_on_tcp_rx()). */
#define SPI_FRAMETYPE_TCP_DATA     0x01

/* Per-frame CRC32. The paired SPI slave (ameba atcmd, CHECKSUM disabled) does
 * NOT append a matching checksum, so the master's verify failed on every RX
 * frame (flooding "crc err"). Default OFF: skip the checksum compute on TX and
 * the verify on RX. The 4 CRC bytes stay reserved in the frame layout (so the
 * payload size is unchanged), they are just not filled/checked. Set to 1 only
 * if the slave is built to append a matching checksum_32_spi. */
#ifndef SPI_FRAME_CRC_EN
#define SPI_FRAME_CRC_EN    0
#endif

/* ---- Broadcast dispatch array (replaces single-callback slot) ---------- *
 * Multiple modules (file_trans, spi_file_upload, etc.) can independently
 * register for AT events; notify_at_evt() broadcasts to the whole array. */
static app_spi_atcmd_cb_t g_at_cb_array[WIFI_TRANSPORT_MAX_CBS] = {NULL};
static uint8_t            g_at_cb_count = 0;

/* downlink RX byte tap: when enabled, every valid received frame's payload
 * length is accumulated (see app_spi_atcmd_rx_bytes_* / spi_atcmd_rcv_cb). */
static volatile bool     s_rx_count_en      = false;
static volatile uint32_t s_rx_payload_bytes = 0;
/* downlink packet counter: incremented for every valid frame received from the
 * slave. Reset at app_spi_atcmd_rx_bytes_start() so numbering restarts per
 * downlink test. Printed per frame (pkt # + size) in place of the payload
 * content, so a gap in the sequence flags a dropped/garbled SPI frame. */
static volatile uint32_t s_rx_frame_cnt     = 0;

typedef enum
{
    UART_ATCMD_RESEND_TIMER           = 0x00,
} T_UART_ATCMD_TIMER;

T_AT_CMD    at_cmd_t = {.cur_cmd = ATCMD_NUM, .resend_cnt = 0, .rx_cnt = 0};
T_OS_QUEUE  at_cmd_queue;
static uint8_t atcmd_timer_id = 0;
static uint8_t timer_handle_atcmd_resend = 0;

static void process_unsolicited_msg(char *p_param, uint16_t len);
static bool cmd_send(T_ATCMD_TYPE cmd, const char *cmd_str);
static void atcmd_parser_process(void);
static void notify_at_evt(T_AT_EVT_TYPE evt, uint8_t *p_data, uint16_t len);

static bool rsp_WLCONN(char *p_param);
static bool rsp_WLDISCONN(char *p_param);
static bool rsp_RAW(char *p_param);
static bool rsp_SENDRAW(char *p_param);

/* --- AT+SKTSENDRAW transparent (raw) two-phase send state --- */
typedef enum
{
    SENDRAW_IDLE = 0,
    SENDRAW_WAIT_PROMPT,    /* command sent, waiting for the ">>>" prompt  */
    SENDRAW_WAIT_OK,        /* raw data frame sent, waiting for "OK"       */
} T_SENDRAW_PHASE;

static const uint8_t *s_sendraw_data  = NULL;   /* payload pushed after ">>>" */
static uint16_t       s_sendraw_len   = 0;
static uint8_t        s_sendraw_phase = SENDRAW_IDLE;
/** @brief When true, the next AT+SKTSENDRAW ">>>" hands the file-upload bulk
 *         push to the dedicated TX task (spi_file_upload_bulk_push) instead of
 *         pushing a single frame inline via send_raw_frame(). Set by
 *         app_spi_atcmd_set_bulk_mode(), consumed (cleared) inside the >>>
 *         handler. Offloading to the TX task is required: running the bulk push
 *         inline would block the parser (which holds an RX ping-pong slot) and
 *         deadlock the full-duplex SPI ring. */
static bool           s_sendraw_bulk_mode = false;
static bool send_raw_frame(const uint8_t *data, uint16_t len);

#if SPI_TCP_TP_TX_TASK
/* --- SPI+TCP "blast" streaming-send state + dedicated TX task ------------ *
 * When app_spi_atcmd_sendraw_stream() is used, the data phase of SKTSENDRAW is
 * NOT pushed inline from the parser (which runs on the APP task); instead the
 * ">>>" prompt handler posts a message to s_tp_tx_queue and the dedicated TX
 * task (s_tp_tx_task) generates and pushes the whole <total> payload. */
typedef struct
{
    uint32_t total_bytes;   /* number of payload bytes to blast to the slave */
    bool     upload_bulk;   /* true: run spi_file_upload_bulk_push() instead of
                             * the ramp/SD blast. Routes the file-upload bulk
                             * push onto this dedicated TX task so it does NOT run
                             * inline in the ">>>" parser (which holds an RX slot
                             * and would deadlock the full-duplex SPI ring). */
} T_TP_TX_MSG;

#define TP_TX_QUEUE_DEPTH       2
#define TP_TX_TASK_STACK_SIZE   2048
#define TP_TX_TASK_PRIORITY     1   /* < APP task (2/3) so APP preempts to drain RX */

static void  *s_tp_tx_task    = NULL;
static void  *s_tp_tx_queue   = NULL;
static volatile bool     s_sendraw_stream = false;   /* this SENDRAW is a blast */
static volatile uint32_t s_stream_total   = 0;       /* total payload bytes     */

/* The blast no longer keeps its own staging frame. It builds each frame straight
 * into the SPI master's single SRAM DMA buffer (app_spi_master_tx_dma_buf()) via
 * the zero-copy prefill path, gated by app_spi_master_tx_wait_free(). This drops
 * the previous 3x-per-frame PSRAM traffic (fill s_tp_tx_frame -> copy into the TX
 * ring -> copy ring into tx_dma_buf) down to a single direct fill, which was the
 * dominant per-frame cost capping the uplink at ~9Mbps. */
#endif /* SPI_TCP_TP_TX_TASK */

const T_AT_CMD_TABLE_ENTRY at_cmd_table[ATCMD_NUM] =
{
    {"AT+WLCONN=", rsp_WLCONN, 20000},
    {"AT+WLDISCONN", rsp_WLDISCONN, 10000},
    {"", rsp_RAW, 10000},
    {"", rsp_SENDRAW, 10000},
};

static void *spi_atcmd_queue_peek(int offset)
{
    void *cmd_queue_pkt = os_queue_peek(&at_cmd_queue, offset);
    return cmd_queue_pkt;
}

static void spi_atcmd_queue_flush(uint16_t cnt)
{
    T_AT_CMD_QUEUE *cmd_queue_pkt;
    APP_PRINT_TRACE1("[wifi] uart atcmd queue flush: %d", cnt);
    if (cnt > at_cmd_queue.count)
    {
        cnt = at_cmd_queue.count;
    }
    for (uint16_t i = 0; i < cnt; i++)
    {
        cmd_queue_pkt = os_queue_out(&at_cmd_queue);
        os_mem_free(cmd_queue_pkt);
    }
}

/* Only referenced when SPI_FRAME_CRC_EN; keep it defined either way so toggling
 * the macro needs no other edit, and silence the unused warning when CRC is off. */
__attribute__((unused))
static uint32_t checksum_32_spi(uint32_t start_value, uint8_t *data, int len)
{
    uint32_t checksum32 = start_value;
    uint16_t data16 = 0;
    int i;

    for (i = 0; i < (len / 2 * 2); i += 2)
    {
        data16 = (data[i] << 8) | data[i + 1];
        checksum32 += data16;
    }

    if (len % 2)
    {
        data16 = data[len - 1] << 8;
        checksum32 += data16;
    }

    return checksum32;
}

void app_spi_atcmd_trigger_send_flow(void)
{
    /* Defer onto the dedicated wifi_8711 task; the handler must not run in the
     * SPI RX parse / app-timer callback context. */
    T_WIFI_8711_MSG msg = {.event = WIFI_8711_EVENT_ATCMD_FLOW_CTRL, .buf = NULL};
    if (!app_send_msg_to_wifi_8711_task(&msg))
    {
        APP_PRINT_ERROR0("[wifi] atcmd flow msg send fail !");
    }
}

#if SPI_RX_DUMP_SHORT_FRAME
/**
 * @brief Dump a received payload as a hex ASCII string, e.g. "41 54 0d 0a".
 *        Chunked at SPI_RX_DUMP_BYTES_PER_LINE so each trace line stays within
 *        the trace message limit (MAX_LOG_MESSAGE_LEN, 252).
 */
static void spi_rx_dump_ascii(const uint8_t *data, uint16_t len)
{
    char hexline[SPI_RX_DUMP_BYTES_PER_LINE * 3 + 1];
    char txtline[SPI_RX_DUMP_BYTES_PER_LINE + 1];

    for (uint16_t off = 0; off < len; off += SPI_RX_DUMP_BYTES_PER_LINE)
    {
        uint16_t n = len - off;
        if (n > SPI_RX_DUMP_BYTES_PER_LINE) { n = SPI_RX_DUMP_BYTES_PER_LINE; }

        int pos = 0;
        for (uint16_t i = 0; i < n; i++)
        {
            uint8_t c = data[off + i];
            pos += snprintf(&hexline[pos], sizeof(hexline) - pos, "%02x ", c);
            /* printable ASCII as-is, everything else as '.' */
            txtline[i] = (c >= 0x20 && c <= 0x7e) ? (char)c : '.';
        }
        txtline[n] = '\0';
        APP_PRINT_INFO3("[wifi] spi rx hex (%d/%d): %s", off, len, TRACE_STRING(hexline));
        APP_PRINT_INFO3("[wifi] spi rx txt (%d/%d): %s", off, len, TRACE_STRING(txtline));
    }
}
#endif /* SPI_RX_DUMP_SHORT_FRAME */

/**
 * @brief Parse SPI RX data
 *        Format: [AT(2)][Len(2)][Data(N)][CRC32(4)]...[Padding]
 */
static void spi_atcmd_rcv_cb(uint8_t *p_data, uint16_t data_len)
{
    spi_frame_hdr_t *hdr;
    uint16_t payload_len;
#if SPI_FRAME_CRC_EN
    uint32_t cal_crc, rcv_crc;
#endif

    if (p_data == NULL || data_len < (SPI_FRAME_HDR_LEN + SPI_FRAME_CRC_LEN))
    {
        return;
    }

    hdr = (spi_frame_hdr_t *)p_data;

    // 1. Verify Magic Number "AT"
    if (hdr->magic[0] != SPI_PROTO_MAGIC_0  || hdr->magic[1] != SPI_PROTO_MAGIC_1)
    {
        return;
    }

    // 2. Get Payload Len
    payload_len = hdr->len;

    // 3. Boundary Check
    if (SPI_FRAME_HDR_LEN + payload_len + SPI_FRAME_CRC_LEN > data_len)
    {
        APP_PRINT_ERROR2("[wifi] spi_atcmd_rcv_cb: req %d, act %d",
                         SPI_FRAME_HDR_LEN + payload_len + SPI_FRAME_CRC_LEN, data_len);
        return;
    }
    // APP_PRINT_INFO2("[wifi] spi_atcmd_rcv_cb: len %d, %b", payload_len,
    //                 TRACE_BINARY(SPI_FRAME_HDR_LEN + payload_len + SPI_FRAME_CRC_LEN, p_data));
    // 4. Checksum
#if SPI_FRAME_CRC_EN
    cal_crc = checksum_32_spi(0, p_data, SPI_FRAME_HDR_LEN + payload_len);
    memcpy(&rcv_crc, &p_data[SPI_FRAME_HDR_LEN + payload_len], 4);
    if (cal_crc != rcv_crc)
    {
        APP_PRINT_ERROR0("[wifi] spi_atcmd_rcv_cb: crc err");
        return;
    }
#endif

    /* 4b. Classify the frame by NATURE, not by the measurement window. A bulk
     *     downlink data frame is (a) any frame while the throughput test window is
     *     open, OR (b) any frame too big to be a single AT line (> rx_buf). Such a
     *     frame carries raw TCP payload (no newline): we trace it as "pkt #N,
     *     size" and RETURN - it must never reach the AT line parser, which would
     *     only overflow rx_buf and leak the raw bytes to the log as content. This
     *     is why downlink data kept printing as "[wifi] << <rawdata>" after the
     *     10s window closed: the window gate turned off while data still flowed.
     *     Small frames outside the window are AT status/responses and fall through
     *     to wifi_at_response_handler, so the slave's boot/link status stays
     *     visible. s_rx_payload_bytes is still only accumulated inside the window
     *     so the throughput math is unaffected. */
    bool is_bulk_data = s_rx_count_en || (payload_len > sizeof(at_cmd_t.rx_buf));
    if (is_bulk_data)
    {
        if (s_rx_count_en) { s_rx_payload_bytes += payload_len; }
        s_rx_frame_cnt++;
        APP_PRINT_INFO2("[wifi] rx pkt #%u, size=%u", s_rx_frame_cnt, payload_len);
        return;
    }

    // 4c. FrameType-based routing: first byte of payload discriminates
    //     AT responses (0x00) from TCP data frames (0x01).
    //     Legacy frames without a FrameType byte fall through to AT parser.
    if (payload_len >= 1)
    {
        uint8_t frametype = hdr->payload[0];

        if (frametype == SPI_FRAMETYPE_TCP_DATA)
        {
            /* TCP data frame: forward remaining payload to file upload.
             * payload[1..payload_len-1] is the [0xAA][Seq][Len][Cmd_ID] frame. */
            uint16_t data_len = (payload_len > 0) ? (uint16_t)(payload_len - 1) : 0;
            if (data_len > 0)
            {
                spi_file_upload_on_tcp_rx(&hdr->payload[1], data_len);
            }
            return;  /* consumed, skip AT parser */
        }
        /* Legacy [$][SKT][DATA] text format fallback -- some 8711 atcmd firmware
         * versions emit inbound TCP data wrapped in:
         *     [$][SKT][DATA][link_id][len]:<binary>\r\n
         * instead of the binary FrameType=0x01 frame.  Scan the full payload
         * (not just the start) because a single SPI frame may batch SKT EVENT
         * and SKT DATA lines together.
         *
         * Extract the <binary> payload and hand it to spi_file_upload_on_tcp_rx()
         * so the same [0xAA][Seq][Len][Cmd_ID] protocol codepath is shared.
         * This MUST be done here, BEFORE the data hits the AT line parser:
         * binary payload may contain 0x0A ('\n') bytes and the parser splits
         * on '\n', which would truncate the binary data and skip the upload. */
        {
            const uint8_t *const payload_end = hdr->payload + payload_len;
            const uint8_t *scan = hdr->payload;

            while (scan + 14 <= payload_end)
            {
                /* Look for the pattern "[$][SKT][DATA]" at any offset */
                if (scan[0] == '[' && scan[1] == '$' && scan[2] == ']' &&
                    memcmp(scan + 3, "[SKT][DATA]", 11) == 0)
                {
                    uint16_t pos = (uint16_t)(scan - hdr->payload) + 14;

                    /* Skip [link_id] -- scan to ']' */
                    while (pos < payload_len && hdr->payload[pos] != ']') { pos++; }
                    if (pos >= payload_len) { break; }
                    pos++; /* skip ']' */

                    /* Parse [len] -- expect '[' then digits then ']' */
                    if (pos >= payload_len || hdr->payload[pos] != '[') { break; }
                    pos++; /* skip '[' */

                    uint16_t expected = 0;
                    while (pos < payload_len &&
                           hdr->payload[pos] >= '0' && hdr->payload[pos] <= '9')
                    {
                        expected = expected * 10 + (uint16_t)(hdr->payload[pos] - '0');
                        pos++;
                    }
                    if (pos >= payload_len || hdr->payload[pos] != ']') { break; }
                    pos++; /* skip ']' */

                    /* Expect ':' */
                    if (pos >= payload_len || hdr->payload[pos] != ':') { break; }
                    pos++; /* skip ':' -- now at binary payload */

                    /* Clamp expected length to available bytes in this frame */
                    uint16_t avail    = payload_len - pos;
                    uint16_t copy_len = (expected < avail) ? expected : avail;

                    if (copy_len > 0)
                    {
                        APP_PRINT_INFO2("[wifi] legacy [$][SKT][DATA] at %d len=%d -> on_tcp_rx",
                                        (uint16_t)(scan - hdr->payload), copy_len);
                        spi_file_upload_on_tcp_rx(&hdr->payload[pos], copy_len);
                    }

                    /* Skip over this SKT DATA entry and continue scanning */
                    pos += copy_len;
                    scan = hdr->payload + pos;
                    continue;
                }
                scan++;
            }
        }

        /* FrameType == 0x00 or any other legacy text: fall through to
         * the existing AT response line-parsing below.  The AT engine
         * uses text-based matching (OK / ERROR / "wifi got ip" etc.) so
         * old firmware that does not set FrameType still works. */
    }

    // 4d. Short-frame debug: print payload (< 128 B) as a hex ASCII string.
#if SPI_RX_DUMP_SHORT_FRAME
    if (payload_len > 0 && payload_len < SPI_RX_DUMP_THRESHOLD)
    {
        spi_rx_dump_ascii(hdr->payload, payload_len);
    }
#endif

    // 5. Copy Payload to rx_buf
    if (payload_len > 0)
    {
        // Check Free Size
        if (at_cmd_t.rx_cnt + payload_len > sizeof(at_cmd_t.rx_buf))
        {
            APP_PRINT_ERROR2("[wifi] at rx buf full! cur:%d, add:%d", at_cmd_t.rx_cnt, payload_len);
            at_cmd_t.rx_cnt = 0; // TODO: Reset?
        }

        /* Clamp to the space left in rx_buf. A single downlink frame can carry up
         * to SPI_XMIT_SIZE-8 (~16K) payload bytes - far more than rx_buf (1024) -
         * so copying payload_len unconditionally overflowed the buffer and
         * corrupted adjacent globals (spinlock assert / reboot). The byte tap
         * above already counted the full payload for the throughput test; any
         * bytes past the buffer are dropped for line parsing. */
        uint16_t copy_len = payload_len;
        if (copy_len > sizeof(at_cmd_t.rx_buf) - at_cmd_t.rx_cnt)
        {
            copy_len = sizeof(at_cmd_t.rx_buf) - at_cmd_t.rx_cnt;
        }

        // Copy Payload
        memcpy(at_cmd_t.rx_buf + at_cmd_t.rx_cnt, hdr->payload, copy_len);
        at_cmd_t.rx_cnt += copy_len;

        // 6. Trigger Process
        atcmd_parser_process();
    }
}

static void wifi_at_response_handler(uint8_t *p_data, uint16_t len)
{
    /* Print the slave's status/response line during boot and link-establishment
     * so the master log shows what the SPI slave reports (WiFi/BT connect state,
     * OK/ERROR, IP, SKT rows, ...). It is suppressed ONLY while the downlink
     * throughput test is running (s_rx_count_en), where each line is the raw data
     * flood; there the per-packet count + size is traced in spi_atcmd_rcv_cb. */
    if (!s_rx_count_en)
    {
        APP_PRINT_INFO1("[wifi] << %s", TRACE_STRING(p_data));
    }

    bool is_command_finished = false;

    if (strstr((char *)p_data, ATCMD_RSP_OK) ||
        strstr((char *)p_data, ATCMD_RSP_ERR))
    {
        is_command_finished = true;
    }

    if (at_cmd_t.cur_cmd < ATCMD_NUM)
    {
        if (at_cmd_table[at_cmd_t.cur_cmd].rsp_func)
        {
            at_cmd_table[at_cmd_t.cur_cmd].rsp_func((char *)p_data);
        }
    }
    else
    {
        process_unsolicited_msg((char *)p_data, len);
    }

    if (is_command_finished && at_cmd_t.cur_cmd < ATCMD_NUM)
    {
        APP_PRINT_INFO0("[wifi] CMD Finished, Trigger Next");

        app_stop_timer(&timer_handle_atcmd_resend);

        spi_atcmd_queue_flush(1);

        at_cmd_t.cur_cmd = ATCMD_NUM;
        at_cmd_t.resend_cnt = 0;

        app_spi_atcmd_trigger_send_flow();
    }
}

/**
 * @brief AT Parser
 */
static void atcmd_parser_process(void)
{
    if (at_cmd_t.rx_cnt == 0) { return; }

    /* AT+SKTSENDRAW phase 1: the ">>>" prompt may arrive without a trailing
     * CRLF, so scan for it directly. Once seen, consume it and push the raw
     * data frame; then fall through so any later OK/ERROR line is parsed. */
    if (at_cmd_t.cur_cmd == ATCMD_SENDRAW && s_sendraw_phase == SENDRAW_WAIT_PROMPT)
    {
        for (uint16_t k = 0; k + 3 <= at_cmd_t.rx_cnt; k++)
        {
            if (at_cmd_t.rx_buf[k] == '>' && at_cmd_t.rx_buf[k + 1] == '>' &&
                at_cmd_t.rx_buf[k + 2] == '>')
            {
                uint16_t consumed = k + 3;
                uint16_t remain   = at_cmd_t.rx_cnt - consumed;
                if (remain > 0)
                {
                    memmove(at_cmd_t.rx_buf, &at_cmd_t.rx_buf[consumed], remain);
                }
                at_cmd_t.rx_cnt = remain;
                s_sendraw_phase = SENDRAW_WAIT_OK;
                APP_PRINT_INFO2("[wifi] SENDRAW '>>>' prompt seen (stream=%d, total=%u)",
                                s_sendraw_stream, s_stream_total);
#if SPI_TCP_TP_TX_TASK
                if (s_sendraw_stream || s_sendraw_bulk_mode)
                {
                    /* Both the tp_test blast (s_sendraw_stream) and the file-upload
                     * bulk push (s_sendraw_bulk_mode) can run far longer than the
                     * default SENDRAW timeout (10 s); restart the resend/timeout
                     * guard with a generous window (30 s base + ~0.5 Mbps floor for
                     * the payload) so it is not torn down mid-stream, which would
                     * make us miss the terminal "OK". */
                    uint32_t stream_to_ms = 30000u + (s_stream_total >> 6);
                    app_start_timer(&timer_handle_atcmd_resend, "atcmd timer",
                                    atcmd_timer_id, UART_ATCMD_RESEND_TIMER, 0,
                                    false, stream_to_ms);

                    bool is_upload = s_sendraw_bulk_mode;
                    s_sendraw_bulk_mode = false;

                    /* Hand the long push to the dedicated TX task so this
                     * (wifi/APP-task) parser context returns IMMEDIATELY: the
                     * ">>>" is an RX frame, so this context holds an RX ping-pong
                     * slot until it returns. The full-duplex SPI ring can only
                     * trigger the next TX once an RX slot is free, so pushing the
                     * whole file inline here fills the ring, then spins waiting
                     * for an RX slot that this same blocked parser is holding
                     * -> deadlock (the SPI msg pump on this task also stalls).
                     * Offloading to the low-priority TX task keeps this task free
                     * to drain RX and cycle slots. */
                    T_TP_TX_MSG msg = {.total_bytes = s_stream_total,
                                       .upload_bulk = is_upload
                                      };
                    if (s_tp_tx_queue == NULL ||
                        os_msg_send(s_tp_tx_queue, &msg, 0) == false)
                    {
                        APP_PRINT_ERROR0("[wifi] sendraw: post to TX task fail");
                    }
                    else
                    {
                        APP_PRINT_INFO2("[wifi] sendraw: handed %u bytes to TX task (upload=%d)",
                                        s_stream_total, is_upload);
                    }
                }
                else
#endif /* SPI_TCP_TP_TX_TASK */
                {
                    /* single small frame (e.g. START/END EVT): safe inline */
                    send_raw_frame(s_sendraw_data, s_sendraw_len);
                }
                break;
            }
        }
    }

    uint16_t processed_len = 0;
    uint16_t start_idx = 0;
    uint16_t i;

    for (i = 0; i < at_cmd_t.rx_cnt; i++)
    {
        // 1. Find tail
        if (at_cmd_t.rx_buf[i] == '\n')
        {
            uint16_t end_idx = i;
            if (end_idx > 0 && at_cmd_t.rx_buf[end_idx - 1] == '\r')
            {
                end_idx--;
            }

            uint16_t line_len = end_idx - start_idx;

            // 2. extract
            if (line_len > 0)
            {
                // +1 for null terminator
                uint8_t *p_line = malloc(line_len + 1);
                if (p_line)
                {
                    memcpy(p_line, &at_cmd_t.rx_buf[start_idx], line_len);
                    p_line[line_len] = '\0';

                    // APP_PRINT_INFO1("[wifi] atcmd_parser_process: %s", TRACE_STRING(p_line));
                    wifi_at_response_handler(p_line, line_len);

                    os_mem_free(p_line);
                }
            }

            // update legnth(including \r\n)
            processed_len = i + 1;
            start_idx = processed_len;
        }
    }

    // 3. Move unhandled data to header
    if (processed_len > 0)
    {
        uint16_t remain_len = at_cmd_t.rx_cnt - processed_len;
        if (remain_len > 0)
        {
            memmove(at_cmd_t.rx_buf, &at_cmd_t.rx_buf[processed_len], remain_len);
        }
        at_cmd_t.rx_cnt = remain_len;
    }

    // 4. Overflow handler
    if (at_cmd_t.rx_cnt >= sizeof(at_cmd_t.rx_buf))
    {
        APP_PRINT_WARN0("[wifi] RX Buf Full without newline, clear!");
        at_cmd_t.rx_cnt = 0;
    }
}


void spi_atcmd_flow_ctrl_handler(void)
{
    if (at_cmd_t.cur_cmd != ATCMD_NUM)
    {
        APP_PRINT_INFO1("[wifi] busy, cur cmd=0x%x", at_cmd_t.cur_cmd);
        return;
    }

    T_AT_CMD_QUEUE *cmd_queue_pkt = (T_AT_CMD_QUEUE *)spi_atcmd_queue_peek(0);
    if (cmd_queue_pkt)
    {
        if (cmd_queue_pkt->cmd >= ATCMD_NUM)
        {
            spi_atcmd_queue_flush(1);
            return;
        }

        APP_PRINT_INFO1("[wifi] sending cmd type: %d", cmd_queue_pkt->cmd);

        cmd_send(cmd_queue_pkt->cmd, cmd_queue_pkt->param);

        at_cmd_t.cur_cmd = cmd_queue_pkt->cmd;
        at_cmd_t.resend_cnt = 0;
        app_start_timer(&timer_handle_atcmd_resend, "atcmd timer",
                        atcmd_timer_id, UART_ATCMD_RESEND_TIMER, 0, false, at_cmd_table[at_cmd_t.cur_cmd].timeout_ms);
    }
}

/**
 * @brief Send AT Cmd
 */
static bool cmd_send(T_ATCMD_TYPE cmd, const char *cmd_str)
{
    if (cmd_str == NULL) { return false; }

    if (cmd == ATCMD_SENDRAW)
    {
        /* phase 1: the command line below makes the slave answer ">>>" */
        s_sendraw_phase = SENDRAW_WAIT_PROMPT;
    }

    uint16_t raw_data_len = strlen(at_cmd_table[cmd].p_cmd) + strlen(cmd_str);
    uint16_t packet_len = SPI_FRAME_HDR_LEN + raw_data_len + SPI_FRAME_CRC_LEN;

    uint8_t *send_buf = (uint8_t *)malloc(packet_len);
    if (send_buf == NULL)
    {
        APP_PRINT_ERROR0("Send Malloc Fail");
        return false;
    }

    send_buf[0] = SPI_PROTO_MAGIC_0;
    send_buf[1] = SPI_PROTO_MAGIC_1;
    send_buf[2] = raw_data_len & 0xFF;        // Len Low
    send_buf[3] = (raw_data_len >> 8) & 0xFF; // Len High
    memcpy(&send_buf[SPI_FRAME_HDR_LEN], at_cmd_table[cmd].p_cmd, strlen(at_cmd_table[cmd].p_cmd));
    memcpy(&send_buf[SPI_FRAME_HDR_LEN + strlen(at_cmd_table[cmd].p_cmd)], cmd_str, strlen(cmd_str));
#if SPI_FRAME_CRC_EN
    uint32_t crc = checksum_32_spi(0, send_buf, SPI_FRAME_HDR_LEN + raw_data_len);
    memcpy(&send_buf[packet_len - 4], &crc, 4);
#endif

    uint8_t ret = app_spi_master_send_raw_data(send_buf, packet_len);

    os_mem_free(send_buf);

    if (ret != 0)
    {
        APP_PRINT_ERROR1("SPI Hardware Send Fail: %d", ret);
        return false;
    }

    return true;
}

//**************************Parse Specific AT EVT***************************//

/**
 * @brief parse ip address from str
 */
static void process_got_ip_event(char *p_str)
{
    T_AT_IP_ADDR ip_data = {0};
    int ip[4] = {0};

    char *p_start = strchr(p_str, '"');

    if (p_start != NULL)
    {
        p_start++;

        if (sscanf(p_start, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == 4)
        {
            ip_data.octets[0] = (uint8_t)ip[0];
            ip_data.octets[1] = (uint8_t)ip[1];
            ip_data.octets[2] = (uint8_t)ip[2];
            ip_data.octets[3] = (uint8_t)ip[3];

            APP_PRINT_INFO4("[wifi] process_got_ip_event: IP %d.%d.%d.%d",
                            ip_data.octets[0], ip_data.octets[1],
                            ip_data.octets[2], ip_data.octets[3]);

            notify_at_evt(AT_EVT_WIFI_GOT_IP, (uint8_t *)&ip_data, sizeof(T_AT_IP_ADDR));
            return;
        }
    }

    APP_PRINT_WARN1("[wifi] process_got_ip_event: Parse IP Failed: %s", TRACE_STRING(p_str));
}

static void process_ok_event(void)
{
    uint8_t state = AT_CMD_RSP_STATE_OK;
    notify_at_evt(AT_EVT_CMD_RESPONSE, (uint8_t *)&state, sizeof(state));
}

static void process_error_event(void)
{
    uint8_t state = AT_CMD_RSP_STATE_ERROR;
    notify_at_evt(AT_EVT_CMD_RESPONSE, (uint8_t *)&state, sizeof(state));
}

static void process_unsolicited_msg(char *p_param, uint16_t len)
{
    if (strstr(p_param, "wifi got ip"))
    {
        process_got_ip_event(p_param);
    }
    else if (strstr((char *)p_param, "wifi connected"))
    {
        notify_at_evt(AT_EVT_WIFI_CONNECTED, NULL, 0);
    }
    else if (strstr((char *)p_param, "wifi disconnected"))
    {
        notify_at_evt(AT_EVT_WIFI_DISCONNECTED, NULL, 0);
    }
    else
    {
        notify_at_evt(AT_EVT_UNKNOWN_DATA, p_param, len);
    }
}

static bool rsp_WLCONN(char *p_param)
{
    APP_PRINT_INFO1("rsp_WLCONN %s", TRACE_STRING(p_param));
    if (strstr(p_param, "OK"))
    {
        process_ok_event();
    }
    else if (strstr(p_param, "ERROR"))
    {
        process_error_event();
    }
    else if (strstr((char *)p_param, "wifi connected"))
    {
        notify_at_evt(AT_EVT_WIFI_CONNECTED, NULL, 0);
    }
    else if (strstr(p_param, "wifi got ip"))
    {
        process_got_ip_event(p_param);
    }

    return true;
}

static bool rsp_WLDISCONN(char *p_param)
{
    APP_PRINT_INFO1("rsp_WLDISCONN %s", TRACE_STRING(p_param));
    if (strstr(p_param, "OK"))
    {
        process_ok_event();
    }
    else if (strstr(p_param, "ERROR"))
    {
        process_error_event();
    }
    else if (strstr((char *)p_param, "wifi disconnected"))
    {
        notify_at_evt(AT_EVT_WIFI_DISCONNECTED, NULL, 0);
    }
    return true;
}

static bool rsp_RAW(char *p_param)
{
    APP_PRINT_INFO1("rsp_RAW %s", TRACE_STRING(p_param));
    if (strstr(p_param, "OK"))
    {
        process_ok_event();
    }
    else if (strstr(p_param, "ERROR"))
    {
        process_error_event();
    }
    else
    {
        notify_at_evt(AT_EVT_UNKNOWN_DATA, (uint8_t *)p_param, strlen(p_param));
    }
    return true;
}

/**
 * @brief Send one SPI frame whose payload is exactly `len` raw bytes (no "AT+"
 *        prefix, no CRLF) - the data phase of AT+SKTSENDRAW. Unlike cmd_send()
 *        the length is explicit, so the payload may hold any byte value.
 */
static bool send_raw_frame(const uint8_t *data, uint16_t len)
{
    if (data == NULL || len == 0) { return false; }

    uint16_t packet_len = SPI_FRAME_HDR_LEN + len + SPI_FRAME_CRC_LEN;
    uint8_t *send_buf = (uint8_t *)malloc(packet_len);
    if (send_buf == NULL)
    {
        APP_PRINT_ERROR0("[wifi] sendraw malloc fail");
        return false;
    }

    send_buf[0] = SPI_PROTO_MAGIC_0;
    send_buf[1] = SPI_PROTO_MAGIC_1;
    send_buf[2] = len & 0xFF;
    send_buf[3] = (len >> 8) & 0xFF;
    memcpy(&send_buf[SPI_FRAME_HDR_LEN], data, len);
#if SPI_FRAME_CRC_EN
    uint32_t crc = checksum_32_spi(0, send_buf, SPI_FRAME_HDR_LEN + len);
    memcpy(&send_buf[packet_len - 4], &crc, 4);
#endif

    uint8_t ret = app_spi_master_send_raw_data(send_buf, packet_len);
    os_mem_free(send_buf);

    if (ret != 0)
    {
        APP_PRINT_ERROR1("[wifi] sendraw hw send fail: %d", ret);
        return false;
    }
    return true;
}

/**
 * @brief Response handler for AT+SKTSENDRAW. The ">>>" prompt is consumed in
 *        atcmd_parser_process(); here we only handle the terminal OK/ERROR.
 *
 *        On OK: notifies spi_file_upload so it can send the next chunk.
 *        On ERROR: notifies spi_file_upload which may retry or abort.
 *        In both cases the AT_EVT_CMD_RESPONSE is still broadcast to all
 *        dispatcher listeners (file_trans etc.) via process_ok/error_event().
 */
static bool rsp_SENDRAW(char *p_param)
{
    APP_PRINT_INFO1("rsp_SENDRAW %s", TRACE_STRING(p_param));
    if (strstr(p_param, "OK"))
    {
        s_sendraw_phase = SENDRAW_IDLE;
#if SPI_TCP_TP_TX_TASK
        s_sendraw_stream = false;
#endif
        /* Notify SPI file upload that the previous SKTSENDRAW completed
         * so it can advance to the next chunk. */
        spi_file_upload_on_sendraw_ok();
        process_ok_event();
    }
    else if (strstr(p_param, "ERROR"))
    {
        s_sendraw_phase = SENDRAW_IDLE;
#if SPI_TCP_TP_TX_TASK
        s_sendraw_stream = false;
#endif
        /* Notify SPI file upload of the error; it may retry or abort. */
        spi_file_upload_on_sendraw_error();
        process_error_event();
    }
    return true;
}

/**
 * @brief Queue an AT+SKTSENDRAW transparent send. `cmd_line` is the
 *        "AT+SKTSENDRAW=<id>,<size>\r\n" line; `raw`/`raw_len` is the payload
 *        pushed to the slave once it answers ">>>".
 *
 * @note  `raw` must stay valid until the send completes. The AT engine runs one
 *        command at a time, so a single caller-owned buffer is sufficient.
 */
bool app_spi_atcmd_sendraw(const char *cmd_line, const uint8_t *raw, uint16_t raw_len)
{
    if (cmd_line == NULL || raw == NULL || raw_len == 0) { return false; }

    s_sendraw_data = raw;
    s_sendraw_len  = raw_len;
    return app_spi_atcmd_queue_fill(ATCMD_SENDRAW, (char *)cmd_line);
}

#if SPI_TCP_TP_TX_TASK
/**
 * @brief Build one [AT][len][payload][CRC] frame directly into the SPI master's
 *        SRAM DMA buffer and hand it off (zero-copy prefill path). The payload
 *        source is chosen at runtime: if an SD-card file was opened by the
 *        higher-level test (app_spi_sd_source_is_open()), up to @p payload_len
 *        bytes are read from it; otherwise the payload is a continuous 0..127
 *        ramp (`*p_ramp` carries the running value across frames).
 *
 *        Flow control is the master's single-buffer semaphore: tx_wait_free()
 *        blocks (yielding the CPU) until the previous frame has finished on the
 *        wire, then this fills the buffer in place and send_prefilled() clocks it
 *        out with no PSRAM staging and no ring copy. Every path that has taken
 *        the buffer MUST return it - send_prefilled() returns it on success/len
 *        error, and tx_release() returns it on the EOF / read-error early-outs -
 *        or the next tx_wait_free() would block forever.
 *
 * @param payload_len  requested payload bytes for this frame.
 * @param p_ramp       running ramp value (used only on the ramp path).
 * @return bytes actually pushed (>=0; may be < payload_len, or 0, at SD EOF),
 *         or -1 on a non-recoverable SPI master / SD read error.
 */
static int tp_tx_blast_frame(uint16_t payload_len, uint32_t *p_ramp)
{
    uint16_t i;
#if SPI_TX_PROFILE
    uint32_t t_wait0 = k_cycle_get_32();
#endif

    /* Acquire the single SRAM DMA buffer (blocks until the previous prefilled
     * frame has finished on the wire), then build straight into it. */
    app_spi_master_tx_wait_free();
    uint8_t *frame = app_spi_master_tx_dma_buf();
#if SPI_TX_PROFILE
    uint32_t t_fill0 = k_cycle_get_32();
#endif

    bool rebuild = true;

    if (app_spi_sd_source_is_open())
    {
        int rd = app_spi_sd_source_read(&frame[SPI_FRAME_HDR_LEN], payload_len);
        if (rd < 0)
        {
            APP_PRINT_ERROR1("[wifi] blast sdcard read fail: %d", rd);
            app_spi_master_tx_release();   /* return the buffer token, no send */
            return -1;
        }
        if (rd == 0)
        {
            app_spi_master_tx_release();
            return 0;   /* EOF: no payload for this frame */
        }
        payload_len = (uint16_t)rd;   /* honour a short (final) read */
        s_blast_built_len = 0;        /* SD payload differs every frame - never reuse */
    }
    else if (payload_len == s_blast_built_len)
    {
        /* Same-size ramp frame already resident in the single DMA buffer from a
         * previous send (DMA only reads it, nothing overwrites it between blast
         * frames). Skip the byte-by-byte refill AND the header rewrite - this is
         * the ~2.6ms "producer gap" that stalled SCK between frames. */
        rebuild = false;
    }
    else
    {
        uint32_t ramp = *p_ramp;
        for (i = 0; i < payload_len; i++)
        {
            frame[SPI_FRAME_HDR_LEN + i] = (uint8_t)(ramp & 0x7F);   /* 0..127 cycle */
            ramp++;
        }
        *p_ramp = ramp;
        s_blast_built_len = payload_len;
    }

    if (rebuild)
    {
        frame[0] = SPI_PROTO_MAGIC_0;
        frame[1] = SPI_PROTO_MAGIC_1;
        frame[2] = payload_len & 0xFF;
        frame[3] = (payload_len >> 8) & 0xFF;
#if SPI_FRAME_CRC_EN
        uint32_t crc = checksum_32_spi(0, frame, SPI_FRAME_HDR_LEN + payload_len);
        memcpy(&frame[SPI_FRAME_HDR_LEN + payload_len], &crc, 4);
#endif
    }

    uint16_t frame_len = SPI_FRAME_HDR_LEN + payload_len + SPI_FRAME_CRC_LEN;
#if SPI_TX_PROFILE
    uint32_t t_send0 = k_cycle_get_32();
#endif
    uint8_t  ret = app_spi_master_send_prefilled(frame_len);
    if (ret != SPI_SEND_SUC)
    {
        /* send_prefilled() already returned the buffer token on the len error. */
        APP_PRINT_ERROR1("[wifi] blast frame hw send fail: %d", ret);
        return -1;
    }
#if SPI_TX_PROFILE
    {
        uint32_t t_end = k_cycle_get_32();
        s_txp_wait += (t_fill0 - t_wait0);
        s_txp_fill += (t_send0 - t_fill0);
        s_txp_send += (t_end   - t_send0);
        if (++s_txp_n >= SPI_TX_PROFILE_N)
        {
            APP_PRINT_INFO3("[tx-prof] avg us/frame: wait=%u fill=%u send=%u",
                            txp_us(s_txp_wait / s_txp_n),
                            txp_us(s_txp_fill / s_txp_n),
                            txp_us(s_txp_send / s_txp_n));
            s_txp_n = 0; s_txp_wait = 0; s_txp_fill = 0; s_txp_send = 0;
        }
    }
#endif
    return (int)payload_len;
}

/**
 * @brief Dedicated SPI TX "blast" task. Idle until the ">>>" prompt handler
 *        posts a T_TP_TX_MSG, then pushes total_bytes to the slave as
 *        back-to-back SPI_XMIT_SIZE frames (payload = continuous 0..127 ramp).
 *        Running on its own (low-priority) task means the long flood never
 *        blocks the APP task, which stays free to drain RX and catch the
 *        slave's terminal "OK" that ends the SKTSENDRAW.
 */
static void app_spi_tp_tx_task(void *p_param)
{
    const uint16_t payload_max = SPI_XMIT_SIZE - SPI_FRAME_HDR_LEN - SPI_FRAME_CRC_LEN;
    T_TP_TX_MSG    msg;   /* frames are built into the master's SRAM DMA buffer */

    (void)p_param;

    for (;;)
    {
        if (os_msg_recv(s_tp_tx_queue, &msg, 0xFFFFFFFF) == false)
        {
            continue;
        }

        /* File-upload bulk push: build+push the AA-framed protocol frames on this
         * (low-priority) task instead of inline in the ">>>" parser, so the wifi
         * task stays free to drain RX and cycle the full-duplex ring's RX slots.
         * spi_file_upload_bulk_push() owns its own framing/staging buffer and
         * signals completion via its END EVT; total_bytes is only informational. */
        if (msg.upload_bulk)
        {
            APP_PRINT_INFO1("[wifi] tx task: file-upload bulk push (%u bytes)",
                            msg.total_bytes);
            spi_file_upload_bulk_push(msg.total_bytes);
            continue;
        }

        uint32_t total = msg.total_bytes;
        uint32_t sent  = 0;
        uint32_t ramp  = 0;
        uint32_t total_pkts = (total + payload_max - 1) / payload_max;   /* ceil */

        s_blast_built_len = 0;   /* force a fresh fill for this blast's first frame */

        APP_PRINT_INFO3("[wifi] blast start: total %u bytes, %u packets, %u bytes/packet",
                        total, total_pkts, (uint32_t)payload_max);

        /* arm the per-trigger "cur/total" progress log in the SPI master */
        app_spi_master_tx_progress_begin(total_pkts);

        /* Hold DLPS off for the WHOLE blast with a single session lock instead of
         * letting the master driver enable/disable it per frame. The refcount
         * (app_spi_common.c) keeps the hardware lock held even across the brief
         * moments the TX ring drains between frames, so the SPI1 clock is never
         * reconfigured mid-stream. Released once below when the blast ends; any
         * still-in-flight frames keep it held via APP_SPI_XFER_BIT until they
         * complete. */
        app_spi_disable_dlps(APP_SPI_BLAST_BIT);

        /* Boost the system/AHB clock so the GDMA can keep the 50MHz SPI FIFO
         * fed (see SPI_BLAST_CPU_MHZ). Floor is held for the whole blast and
         * cleared once all bytes are pushed. */
        {
            uint32_t boost_mhz = 0;
            pm_cpu_freq_req(&s_blast_cpu_freq_handle, SPI_BLAST_CPU_MHZ, &boost_mhz);
            APP_PRINT_INFO2("[wifi] blast cpu boost: req %u MHz, actual %u MHz",
                            (uint32_t)SPI_BLAST_CPU_MHZ, boost_mhz);
        }

        while (sent < total)
        {
            uint32_t remain = total - sent;
            uint16_t pl = (remain > payload_max) ? payload_max : (uint16_t)remain;
            int pushed = tp_tx_blast_frame(pl, &ramp);
            if (pushed < 0)
            {
                break;                       /* hw send / SD read error */
            }
            sent += (uint32_t)pushed;
            if ((uint16_t)pushed < pl)
            {
                /* SD EOF (0) or short final read: the file is fully sent. total
                 * was set to the file size, so this coincides with sent==total;
                 * break defensively in case the file changed under us. */
                break;
            }
        }

        /* release the SD source (no-op if the ramp path was used) */
        app_spi_sd_source_close();

        app_spi_master_tx_progress_end();

        /* Blast finished (all bytes pushed): release the session DLPS lock. Any
         * frames still draining from the TX ring keep the hardware lock held via
         * APP_SPI_XFER_BIT until their completion callback runs, so DLPS is only
         * truly re-allowed once the very last frame has gone out. */
        app_spi_enable_dlps(APP_SPI_BLAST_BIT);

        /* Drop the CPU/AHB clock floor back to the app default. */
        {
            uint32_t rest_mhz = 0;
            pm_cpu_freq_clear(&s_blast_cpu_freq_handle, &rest_mhz);
        }

        APP_PRINT_INFO2("[wifi] blast done: %u/%u bytes pushed", sent, total);
    }
}

/**
 * @brief Start a streaming transparent send (the SPI+TCP "blast" test). Issues
 *        one "AT+SKTSENDRAW=<id>,<total>" header; the ">>>" prompt handler then
 *        wakes the dedicated TX task, which generates and pushes <total> bytes.
 */
bool app_spi_atcmd_sendraw_stream(uint8_t link_id, uint32_t total_bytes)
{
    char cmd_line[40];

    if (total_bytes == 0)
    {
        return false;
    }
    if (s_tp_tx_queue == NULL || s_tp_tx_task == NULL)
    {
        APP_PRINT_ERROR0("[wifi] sendraw stream: TX task not started");
        return false;
    }
    if (s_sendraw_phase != SENDRAW_IDLE)
    {
        APP_PRINT_ERROR0("[wifi] sendraw stream: a SENDRAW is already pending");
        return false;
    }

    s_sendraw_stream = true;
    s_stream_total   = total_bytes;
    s_sendraw_data   = NULL;     /* payload is generated by the TX task */
    s_sendraw_len    = 0;

    snprintf(cmd_line, sizeof(cmd_line), "AT+SKTSENDRAW=%u,%u\r\n",
             (unsigned)link_id, (unsigned)total_bytes);

    if (app_spi_atcmd_queue_fill(ATCMD_SENDRAW, cmd_line) == false)
    {
        s_sendraw_stream = false;
        return false;
    }
    app_spi_atcmd_trigger_send_flow();
    return true;
}

void app_spi_atcmd_set_bulk_mode(bool enable, uint32_t total)
{
    s_sendraw_bulk_mode = enable;
    if (enable)
    {
        s_stream_total = total;
    }
}
#endif /* SPI_TCP_TP_TX_TASK */

static void spi_atcmd_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("spi_atcmd_timeout_cb: timer_id %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case UART_ATCMD_RESEND_TIMER:
        {
            app_stop_timer(&timer_handle_atcmd_resend);
            if (at_cmd_t.resend_cnt >= ATCMD_RESEND_CNT)
            {
                /* Notify the file upload module if a SKTSENDRAW timed out,
                 * so it can retry or abort instead of hanging forever. */
                if (at_cmd_t.cur_cmd == ATCMD_SENDRAW &&
                    s_sendraw_phase != SENDRAW_IDLE)
                {
                    spi_file_upload_on_sendraw_error();
                }
                at_cmd_t.cur_cmd = ATCMD_NUM;
                at_cmd_t.resend_cnt = 0;
                at_cmd_t.rx_cnt = 0;
                s_sendraw_phase = SENDRAW_IDLE;
#if SPI_TCP_TP_TX_TASK
                s_sendraw_stream = false;
#endif
                spi_atcmd_queue_flush(1);
            }
            else
            {
                at_cmd_t.resend_cnt++;
                // TODO: Not implemented
            }

            app_spi_atcmd_trigger_send_flow();
        }
        break;
    default:
        break;
    }
}

bool app_spi_atcmd_queue_fill(T_ATCMD_TYPE cmd, char *param)
{
    APP_PRINT_TRACE1("[wifi] app_spi_atcmd_queue_fill: cmd %d", cmd);
    T_AT_CMD_QUEUE *cmd_queue_pkt;
    uint16_t param_len = 1;
    if (param != NULL)
    {
        param_len += strlen(param);
    }

    cmd_queue_pkt = (T_AT_CMD_QUEUE *)os_mem_alloc(OS_MEM_TYPE_DATA,
                                                   sizeof(T_AT_CMD_QUEUE) + param_len);
    if (cmd_queue_pkt == NULL)
    {
        APP_PRINT_ERROR0("[wifi] uart atcmd queue fill get buffer error");
        return false;
    }

    cmd_queue_pkt->cmd = cmd;
    memset(cmd_queue_pkt->param, 0, param_len);
    if (param_len > 1)
    {
        memcpy(cmd_queue_pkt->param, param, param_len - 1);
    }
    os_queue_in(&at_cmd_queue, cmd_queue_pkt);

    return true;
}

static void notify_at_evt(T_AT_EVT_TYPE evt, uint8_t *p_data, uint16_t len)
{
    /* Broadcast to all registered listeners.  An empty array is a silent no-op
     * (the AT engine functions without any listener, e.g. during testing). */
    for (uint8_t i = 0; i < g_at_cb_count; i++)
    {
        if (g_at_cb_array[i] != NULL)
        {
            g_at_cb_array[i](evt, p_data, len);
        }
    }
}

bool app_spi_atcmd_register_callback(app_spi_atcmd_cb_t cb)
{
    if (cb == NULL)
    {
        return false;
    }
    if (g_at_cb_count >= WIFI_TRANSPORT_MAX_CBS)
    {
        APP_PRINT_ERROR1("app_spi_atcmd_register_callback: table full (%d max)",
                         WIFI_TRANSPORT_MAX_CBS);
        return false;
    }
    g_at_cb_array[g_at_cb_count++] = cb;
    APP_PRINT_INFO1("app_spi_atcmd_register_callback: registered (%d listeners)",
                    g_at_cb_count);
    return true;
}

void app_spi_atcmd_rx_bytes_start(void)
{
    s_rx_payload_bytes = 0;
    s_rx_frame_cnt = 0;
    s_rx_count_en = true;
}

uint32_t app_spi_atcmd_rx_bytes_get(void)
{
    return s_rx_payload_bytes;
}

void app_spi_atcmd_rx_bytes_stop(void)
{
    s_rx_count_en = false;
}

void app_spi_atcmd_init(void)
{
    if (atcmd_timer_id == 0)
    {
        app_timer_reg_cb(spi_atcmd_timeout_cb, &atcmd_timer_id);
    }

    app_spi_master_module_register(spi_atcmd_rcv_cb, 0);

#if SPI_TCP_TP_TX_TASK
    /* Create the queue before the task so the task's os_msg_recv() always has a
     * valid handle, then spawn the dedicated SPI TX "blast" task. */
    if (s_tp_tx_queue == NULL)
    {
        os_msg_queue_create(&s_tp_tx_queue, "spi_tp_tx_q",
                            TP_TX_QUEUE_DEPTH, sizeof(T_TP_TX_MSG));
    }
    if (s_tp_tx_task == NULL)
    {
        os_task_create(&s_tp_tx_task, "spi_tp_tx", app_spi_tp_tx_task, NULL,
                       TP_TX_TASK_STACK_SIZE, TP_TX_TASK_PRIORITY);
    }
    APP_PRINT_INFO0("[wifi] atcmd SPI TX blast task started");
#endif

    APP_PRINT_INFO0("[wifi] atcmd module initialized (SPI Mode)");
}

void app_spi_atcmd_demo(uint8_t type)
{
    APP_PRINT_INFO0("app_spi_atcmd_demo");
    switch ((T_ATCMD_TYPE)type)
    {
    case ATCMD_WLCONN:
        {
            APP_PRINT_INFO0("[wifi] atcmd  ATCMD_WLCONN ");
            app_spi_atcmd_queue_fill(ATCMD_WLCONN, "ssid,mytest,pw,12345678\r\n");
            app_spi_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_WLDISCONN:
        {
            app_spi_atcmd_queue_fill(ATCMD_WLDISCONN, "\r\n");
            app_spi_atcmd_trigger_send_flow();
        }
        break;

    default:
        break;
    }

}
