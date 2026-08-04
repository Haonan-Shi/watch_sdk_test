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
#include "app_spi_sd_source.h"
#include "wifi_8711_app.h"
#include <zephyr/devicetree.h>
#include "psram_section.h"

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
 * than SPI_RX_DUMP_THRESHOLD bytes as a hex ASCII string (set to 0 to disable). */
#ifndef SPI_RX_DUMP_SHORT_FRAME
#define SPI_RX_DUMP_SHORT_FRAME     1
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
 *  @brief format: [AT(2)][Len(2)][Data(N)][CRC32(4)]...[Padding]
 */
#define SPI_PROTO_MAGIC_0   'A'
#define SPI_PROTO_MAGIC_1   'T'
#define SPI_FRAME_HDR_LEN   4  // Magic(2) + Len(2)
#define SPI_FRAME_CRC_LEN   4  // Checksum(4)

/* Per-frame CRC32. The paired SPI slave (ameba atcmd, CHECKSUM disabled) does
 * NOT append a matching checksum, so the master's verify failed on every RX
 * frame (flooding "crc err"). Default OFF: skip the checksum compute on TX and
 * the verify on RX. The 4 CRC bytes stay reserved in the frame layout (so the
 * payload size is unchanged), they are just not filled/checked. Set to 1 only
 * if the slave is built to append a matching checksum_32_spi. */
#ifndef SPI_FRAME_CRC_EN
#define SPI_FRAME_CRC_EN    0
#endif

static app_spi_atcmd_cb_t g_at_callback = NULL;

/* downlink RX byte tap: when enabled, every valid received frame's payload
 * length is accumulated (see app_spi_atcmd_rx_bytes_* / spi_atcmd_rcv_cb). */
static volatile bool     s_rx_count_en      = false;
static volatile uint32_t s_rx_payload_bytes = 0;

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
} T_TP_TX_MSG;

#define TP_TX_QUEUE_DEPTH       2
#define TP_TX_TASK_STACK_SIZE   2048
#define TP_TX_TASK_PRIORITY     1   /* < APP task (2/3) so APP preempts to drain RX */
#define TP_TX_BUSY_RETRY_MS     1   /* yield while both ping-pong TX slots are full */

static void  *s_tp_tx_task    = NULL;
static void  *s_tp_tx_queue   = NULL;
static volatile bool     s_sendraw_stream = false;   /* this SENDRAW is a blast */
static volatile uint32_t s_stream_total   = 0;       /* total payload bytes     */

/* One SPI_XMIT_SIZE (16K) staging frame for the blast. Statically placed in the
 * dedicated PSRAM region (same as the SPI master's tx_dma_buf/rx_storage) rather
 * than malloc()'d: the libc heap is tight (esp. on the watch) and a one-shot 16K
 * malloc can fail, which would silently disable the whole blast. PSRAM is only
 * brought up in app_system_lower_init(), so this lives in the noinit SPI region
 * and is never zeroed at boot (the TX task fully rewrites it per frame). */
static uint8_t s_tp_tx_frame[SPI_XMIT_SIZE] __aligned(32) SECTION_PSRAM1_NC;
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

    // 4b. Downlink byte tap: count payload of every valid frame while enabled.
    if (s_rx_count_en)
    {
        s_rx_payload_bytes += payload_len;
    }

    // 4c. Short-frame debug: print payload (< 128 B) as a hex ASCII string.
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
    APP_PRINT_INFO1("[wifi] wifi_at_response_handler: %s", TRACE_STRING(p_data));

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
                if (s_sendraw_stream)
                {
                    /* The blast can run far longer than the default SENDRAW
                     * timeout (10 s); restart the resend/timeout guard with a
                     * generous window (30 s base + ~0.5 Mbps floor for the
                     * payload) so it is not torn down mid-stream, which would
                     * make us miss the terminal "OK". */
                    uint32_t stream_to_ms = 30000u + (s_stream_total >> 6);
                    app_start_timer(&timer_handle_atcmd_resend, "atcmd timer",
                                    atcmd_timer_id, UART_ATCMD_RESEND_TIMER, 0,
                                    false, stream_to_ms);

                    /* hand the long flood to the dedicated TX task so this
                     * (APP-task) context returns immediately and stays free to
                     * drain RX / service other tasks during the transfer. */
                    T_TP_TX_MSG msg = {.total_bytes = s_stream_total};
                    if (s_tp_tx_queue == NULL ||
                        os_msg_send(s_tp_tx_queue, &msg, 0) == false)
                    {
                        APP_PRINT_ERROR0("[wifi] sendraw stream: post to TX task fail");
                    }
                    else
                    {
                        APP_PRINT_INFO1("[wifi] sendraw stream: handed %u bytes to TX task",
                                        s_stream_total);
                    }
                }
                else
#endif /* SPI_TCP_TP_TX_TASK */
                {
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
        process_ok_event();
    }
    else if (strstr(p_param, "ERROR"))
    {
        s_sendraw_phase = SENDRAW_IDLE;
#if SPI_TCP_TP_TX_TASK
        s_sendraw_stream = false;
#endif
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
 * @brief Build one [AT][len][payload][CRC] frame and hand it to the SPI master.
 *        The payload source is chosen at runtime: if an SD-card file was opened
 *        by the higher-level test (app_spi_sd_source_is_open()), up to
 *        @p payload_len bytes are read from it; otherwise the payload is a
 *        continuous 0..127 ramp (`*p_ramp` carries the running value across
 *        frames). Retries while both ping-pong TX slots are full, yielding the
 *        CPU so higher-priority tasks keep running.
 *
 * @param frame        staging buffer (>= SPI_XMIT_SIZE).
 * @param payload_len  requested payload bytes for this frame.
 * @param p_ramp       running ramp value (used only on the ramp path).
 * @return bytes actually pushed (>=0; may be < payload_len, or 0, at SD EOF),
 *         or -1 on a non-recoverable SPI master / SD read error.
 */
static int tp_tx_blast_frame(uint8_t *frame, uint16_t payload_len, uint32_t *p_ramp)
{
    uint16_t i;

    if (app_spi_sd_source_is_open())
    {
        int rd = app_spi_sd_source_read(&frame[SPI_FRAME_HDR_LEN], payload_len);
        if (rd < 0)
        {
            APP_PRINT_ERROR1("[wifi] blast sdcard read fail: %d", rd);
            return -1;
        }
        if (rd == 0)
        {
            return 0;   /* EOF: no payload for this frame */
        }
        payload_len = (uint16_t)rd;   /* honour a short (final) read */
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
    }

    frame[0] = SPI_PROTO_MAGIC_0;
    frame[1] = SPI_PROTO_MAGIC_1;
    frame[2] = payload_len & 0xFF;
    frame[3] = (payload_len >> 8) & 0xFF;
#if SPI_FRAME_CRC_EN
    uint32_t crc = checksum_32_spi(0, frame, SPI_FRAME_HDR_LEN + payload_len);
    memcpy(&frame[SPI_FRAME_HDR_LEN + payload_len], &crc, 4);
#endif

    uint16_t frame_len = SPI_FRAME_HDR_LEN + payload_len + SPI_FRAME_CRC_LEN;
    uint8_t  ret;
    while ((ret = app_spi_master_send_raw_data(frame, frame_len)) == SPI_SEND_ERR_BUSY)
    {
        /* both ping-pong slots in use: yield so the SPI completion - and any
         * other task, e.g. the APP task draining RX - can run, then retry. */
        os_delay(TP_TX_BUSY_RETRY_MS);
    }
    if (ret != SPI_SEND_SUC)
    {
        APP_PRINT_ERROR1("[wifi] blast frame hw send fail: %d", ret);
        return -1;
    }
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
    uint8_t       *frame = s_tp_tx_frame;   /* static PSRAM buffer, no malloc() */
    T_TP_TX_MSG    msg;

    (void)p_param;

    for (;;)
    {
        if (os_msg_recv(s_tp_tx_queue, &msg, 0xFFFFFFFF) == false)
        {
            continue;
        }

        uint32_t total = msg.total_bytes;
        uint32_t sent  = 0;
        uint32_t ramp  = 0;
        uint32_t total_pkts = (total + payload_max - 1) / payload_max;   /* ceil */

        APP_PRINT_INFO3("[wifi] blast start: total %u bytes, %u packets, %u bytes/packet",
                        total, total_pkts, (uint32_t)payload_max);

        /* arm the per-trigger "cur/total" progress log in the SPI master */
        app_spi_master_tx_progress_begin(total_pkts);

        while (sent < total)
        {
            uint32_t remain = total - sent;
            uint16_t pl = (remain > payload_max) ? payload_max : (uint16_t)remain;
            int pushed = tp_tx_blast_frame(frame, pl, &ramp);
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
    if (g_at_callback != NULL)
    {
        g_at_callback(evt, p_data, len);
    }
}

void app_spi_atcmd_register_callback(app_spi_atcmd_cb_t cb)
{
    g_at_callback = cb;
}

void app_spi_atcmd_rx_bytes_start(void)
{
    s_rx_payload_bytes = 0;
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

    /* Flow-control is handled on the shared wifi_8711 task (see wifi_8711_app.c);
     * no self-contained flow task is created here anymore. */

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
