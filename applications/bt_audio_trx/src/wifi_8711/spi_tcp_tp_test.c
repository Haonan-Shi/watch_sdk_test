/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * @file    spi_tcp_tp_test.c
 * @brief   SPI + TCP throughput test driver (master side): uplink + downlink,
 *          slave as TCP server or TCP client.
 *
 * See spi_tcp_tp_test.h for the topology and measurement description.
 *
 * Implementation notes
 * --------------------
 * - All AT commands are pushed through the existing AT-over-SPI engine
 *   (app_spi_atcmd.c). That engine is strictly serial: one command is sent,
 *   and the next is only sent after its "OK"/"ERROR" line is parsed. We piggy
 *   back on that: the uplink pump simply re-queues the next AT+SKTSEND from
 *   inside the "OK" callback, which keeps exactly one send in flight.
 * - app_spi_atcmd_trigger_send_flow() is a no-op while a command is in flight,
 *   so it is always safe to call after queue_fill(): if the engine is idle it
 *   starts immediately, otherwise the in-flight command's completion starts it.
 * - We reuse the generic ATCMD_RAW table entry (empty prefix) so any AT line
 *   can be sent without adding a dedicated table entry per command.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "trace.h"
#include "os_sched.h"
#include "app_timer.h"
#include "app_spi_atcmd.h"
#include "app_spi_sd_source.h"
#include "spi_tcp_tp_test.h"

/* TX scratch for one inline AT+SKTSEND line: prefix + up to TP_MAX_CHUNK
 * payload + CRLF + NUL. The full frame still must fit in SPI_XMIT_SIZE
 * (enforced by app_spi_master_send_raw_data). */
#define TP_SEND_BUF_SIZE        (TP_MAX_CHUNK + 32)

typedef enum
{
    TP_ST_IDLE = 0,
    TP_ST_WIFI_CONNECTING,  /* AT+WLCONN sent, waiting for "got ip"        */
    TP_ST_SETUP_CFG,        /* AT+SKTCFG sent, waiting for OK              */
    TP_ST_SETUP_SERVER,     /* AT+SKTSERVER sent, waiting for OK           */
    TP_ST_WAIT_CLIENT,      /* server up, waiting for the PC to connect    */
    TP_ST_SENDING,          /* uplink pump running                         */
    TP_ST_RECVING,          /* downlink measurement window running         */
    TP_ST_BLASTING,         /* uplink "blast" running (TX-task streaming)  */
    TP_ST_DONE,             /* finished, result printed                   */
} T_TP_STATE;

typedef struct
{
    T_TP_STATE  state;
    bool        initialized;
    bool        auto_flow;       /* drive the full sequence automatically  */

    /* configuration */
    uint16_t    port;
    uint16_t    chunk;           /* payload bytes per AT+SKTSENDRAW         */
    uint16_t    duration_s;
    uint8_t     link_id;         /* link id used for SKTSENDRAW             */
    bool        link_id_known;   /* seed link id resolved                   */

    /* accounting */
    uint64_t    start_time_ms;
    uint64_t    total_bytes;
    uint32_t    blast_total;     /* target bytes for the blast uplink (act 9) */
    uint32_t    ok_cnt;
    uint32_t    err_cnt;
    uint16_t    last_chunk;      /* bytes in the in-flight SD chunk (SD pump)   */

    uint8_t     send_buf[TP_SEND_BUF_SIZE];
} T_TP_CTX;

static T_TP_CTX g_tp;

static uint8_t  tp_timer_id = 0;
static uint8_t  tp_timer_duration_handle = 0;

typedef enum
{
    TP_TMR_DURATION = 0x00,
} T_TP_TIMER;

/* ------------------------------------------------------------------ */
/* helpers                                                            */
/* ------------------------------------------------------------------ */

static void tp_finish(const char *tag);

/**
 * @brief Queue one AT line as a raw command and kick the send engine.
 *        Calling trigger unconditionally is safe (no-op while busy).
 */
static void tp_send_at(const char *at_line)
{
    APP_PRINT_INFO1("[tp] >> %s", TRACE_STRING(at_line));
    app_spi_atcmd_queue_fill(ATCMD_RAW, (char *)at_line);
    app_spi_atcmd_trigger_send_flow();
}

/**
 * @brief Issue one AT+SKTSENDRAW transparent send of g_tp.chunk bytes.
 *
 *        Two-phase (documented "Host Control Mode" transparent transmission):
 *          1. master -> "AT+SKTSENDRAW=<link_id>,<chunk>\r\n"
 *          2. slave  -> ">>>"              (consumed inside the AT engine)
 *          3. master -> <chunk> raw bytes  (engine pushes g_tp.send_buf)
 *          4. slave  -> "OK"               (bytes forwarded to TCP)
 *        Unlike inline AT+SKTSEND the payload is a real raw frame, so it is not
 *        bounded by the slave command-line buffer and needs no escaping.
 */
static void tp_pump_one(void)
{
    char     cmd_line[40];
    uint16_t send_len = g_tp.chunk;

#if TP_TX_DATA_FROM_SDCARD
    /* Refill send_buf with the next slice of the SD file. A read of 0 (EOF) or
     * <0 (error) means the whole file has been sent -> finish and report. */
    int rd = app_spi_sd_source_read(g_tp.send_buf, g_tp.chunk);
    if (rd <= 0)
    {
        APP_PRINT_INFO1("[tp] sdcard EOF/err (%d) - uplink complete", rd);
        tp_finish("uplink done (sdcard EOF)");
        return;
    }
    send_len = (uint16_t)rd;
    g_tp.last_chunk = send_len;
#endif

    snprintf(cmd_line, sizeof(cmd_line), "AT+SKTSENDRAW=%u,%u\r\n",
             g_tp.link_id, send_len);

    /* When TP_TX_DATA_FROM_SDCARD==0, send_buf is pre-filled with 'A' in
     * start_uplink; the engine pushes exactly send_len bytes once the slave
     * answers ">>>". */
    if (!app_spi_atcmd_sendraw(cmd_line, g_tp.send_buf, send_len))
    {
        APP_PRINT_ERROR1("[tp] sendraw queue fail (len %u)", send_len);
        return;
    }
    app_spi_atcmd_trigger_send_flow();
}

static void tp_report(const char *tag)
{
    uint64_t elapsed_ms = os_sys_time_get() - g_tp.start_time_ms;
    if (elapsed_ms == 0) { elapsed_ms = 1; }

    /* bytes * 8 / ms  ==  kbit/s */
    uint32_t kbps = (uint32_t)((g_tp.total_bytes * 8ULL) / elapsed_ms);

    APP_PRINT_INFO3("[tp] ===== %s : total %u bytes in %u ms =====",
                    TRACE_STRING(tag),
                    (uint32_t)g_tp.total_bytes, (uint32_t)elapsed_ms);
    APP_PRINT_INFO5("[tp] ===== sends ok=%u err=%u | rate=%u kbps (%u.%03u Mbps) =====",
                    g_tp.ok_cnt, g_tp.err_cnt, kbps, kbps / 1000, kbps % 1000);
}

static void tp_finish(const char *tag)
{
    if (g_tp.state != TP_ST_SENDING)
    {
        return;
    }
    g_tp.state = TP_ST_DONE;
    app_stop_timer(&tp_timer_duration_handle);
#if TP_TX_DATA_FROM_SDCARD
    app_spi_sd_source_close();
#endif
    tp_report(tag);

    /* tear the connection down so a follow-up run starts clean */
    tp_send_at("AT+SKTDEL=0\r\n");
}

/**
 * @brief Print the downlink result. Bytes come from the SPI RX byte tap, so the
 *        figure includes the slave's tiny per-line recv header overhead.
 */
static void tp_report_dl(const char *tag)
{
    uint64_t elapsed_ms = os_sys_time_get() - g_tp.start_time_ms;
    if (elapsed_ms == 0) { elapsed_ms = 1; }

    /* bytes * 8 / ms  ==  kbit/s */
    uint32_t kbps = (uint32_t)((g_tp.total_bytes * 8ULL) / elapsed_ms);

    APP_PRINT_INFO3("[tp] ===== %s : recv %u bytes in %u ms =====",
                    TRACE_STRING(tag),
                    (uint32_t)g_tp.total_bytes, (uint32_t)elapsed_ms);
    APP_PRINT_INFO3("[tp] ===== downlink rate=%u kbps (%u.%03u Mbps) =====",
                    kbps, kbps / 1000, kbps % 1000);
}

static void tp_finish_downlink(const char *tag)
{
    if (g_tp.state != TP_ST_RECVING)
    {
        return;
    }
    g_tp.state = TP_ST_DONE;
    app_stop_timer(&tp_timer_duration_handle);
    g_tp.total_bytes = app_spi_atcmd_rx_bytes_get();
    app_spi_atcmd_rx_bytes_stop();
    tp_report_dl(tag);
}

/**
 * @brief Begin the socket setup phase (after WiFi is up).
 *        SKTCFG disables Nagle; SKTSERVER opens the listen socket with
 *        auto_rcv=1 so the slave pushes any downlink data to us.
 */
static void tp_begin_setup(void)
{
    char line[48];

    g_tp.state = TP_ST_SETUP_CFG;
    /* AT+SKTCFG=[snd],[rcv],[nodelay],...  -> nodelay=1 (Nagle off) */
    tp_send_at("AT+SKTCFG=,,1\r\n");

    /* Pre-queue the server command; the engine sends it after SKTCFG's OK. */
    snprintf(line, sizeof(line), "AT+SKTSERVER=0,1,,%u,1\r\n", g_tp.port);
    g_tp.state = TP_ST_SETUP_SERVER;
    tp_send_at(line);
}

/**
 * @brief Parse the seed (accepted) link id out of the connect event, e.g.
 *        "[SKT][EVENT]: A client[link_id:1,seed,tcp,...] connected to server[link_id:0]"
 */
static bool tp_parse_seed_link_id(const char *line, uint8_t *out_id)
{
    const char *p;

    if (strstr(line, "connected to server") == NULL)
    {
        return false;
    }
    p = strstr(line, "client[link_id:");
    if (p == NULL)
    {
        return false;
    }
    p += strlen("client[link_id:");

    int id = atoi(p);
    if (id < 0 || id > 255)
    {
        return false;
    }
    *out_id = (uint8_t)id;
    return true;
}

/* ------------------------------------------------------------------ */
/* timer                                                              */
/* ------------------------------------------------------------------ */

static void tp_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    switch (timer_evt)
    {
    case TP_TMR_DURATION:
        APP_PRINT_INFO0("[tp] duration elapsed");
        if (g_tp.state == TP_ST_RECVING)
        {
            tp_finish_downlink("downlink done");
        }
        else
        {
            tp_finish("uplink done");
        }
        break;
    default:
        break;
    }
}

/* ------------------------------------------------------------------ */
/* AT event callback                                                  */
/* ------------------------------------------------------------------ */

static void tp_at_evt_handler(T_AT_EVT_TYPE evt, void *p_data, uint16_t len)
{
    switch (evt)
    {
    case AT_EVT_WIFI_GOT_IP:
        {
            T_AT_IP_ADDR *ip = (T_AT_IP_ADDR *)p_data;
            if (ip != NULL)
            {
                APP_PRINT_INFO4("[tp] slave got ip %d.%d.%d.%d",
                                ip->octets[0], ip->octets[1], ip->octets[2], ip->octets[3]);
            }
            if (g_tp.state == TP_ST_WIFI_CONNECTING && g_tp.auto_flow)
            {
                /* Queue setup now; it is sent right after WLCONN's trailing OK. */
                tp_begin_setup();
            }
        }
        break;

    case AT_EVT_CMD_RESPONSE:
        {
            T_AT_CMD_RSP_STATE st = AT_CMD_RSP_STATE_OK;
            if (p_data != NULL && len >= 1)
            {
                st = (T_AT_CMD_RSP_STATE)(*(uint8_t *)p_data);
            }

            if (g_tp.state == TP_ST_SENDING)
            {
                if (st == AT_CMD_RSP_STATE_OK)
                {
#if TP_TX_DATA_FROM_SDCARD
                    g_tp.total_bytes += g_tp.last_chunk;   /* actual bytes sent */
#else
                    g_tp.total_bytes += g_tp.chunk;
#endif
                    g_tp.ok_cnt++;
                    /* Keep the pipe full: queue the next chunk. */
                    tp_pump_one();
                }
                else
                {
                    g_tp.err_cnt++;
                    APP_PRINT_WARN1("[tp] SKTSEND error (#%u) - stopping", g_tp.err_cnt);
                    tp_finish("uplink stopped (error)");
                }
            }
            else if (g_tp.state == TP_ST_BLASTING)
            {
                /* the whole 0..127 flood was pushed by the TX task; this OK is
                 * the slave's single terminal ack for the SKTSENDRAW. */
                g_tp.state = TP_ST_DONE;
                if (st == AT_CMD_RSP_STATE_OK)
                {
                    g_tp.total_bytes = g_tp.blast_total;
                    g_tp.ok_cnt = 1;
                    tp_report("uplink blast done");
                }
                else
                {
                    g_tp.err_cnt++;
                    APP_PRINT_WARN0("[tp] blast SKTSENDRAW error");
                    tp_report("uplink blast error");
                }
            }
            else if (g_tp.state == TP_ST_SETUP_SERVER && st == AT_CMD_RSP_STATE_ERROR)
            {
                APP_PRINT_ERROR0("[tp] server setup failed");
                g_tp.state = TP_ST_IDLE;
            }
        }
        break;

    case AT_EVT_UNKNOWN_DATA:
        {
            char *line = (char *)p_data;
            if (line == NULL) { break; }

            /* During the downlink flood the slave pushes a recv line per packet;
             * the bytes are already counted by the SPI RX tap, so skip the
             * per-line trace to avoid flooding the log and skewing timing. The
             * uplink blast is likewise timing-sensitive, so stay quiet there. */
            if (g_tp.state == TP_ST_RECVING || g_tp.state == TP_ST_BLASTING) { break; }

            uint8_t seed;
            if (tp_parse_seed_link_id(line, &seed))
            {
                APP_PRINT_INFO1("[tp] client connected, seed link_id=%d", seed);
                if (!g_tp.link_id_known || g_tp.link_id == TP_LINK_ID_AUTO)
                {
                    g_tp.link_id = seed;
                    g_tp.link_id_known = true;
                }
                if (g_tp.state == TP_ST_WAIT_CLIENT && g_tp.auto_flow)
                {
                    app_spi_tcp_tp_start_uplink(g_tp.link_id, g_tp.chunk, g_tp.duration_s);
                }
            }
            else
            {
                /* SKTSTATE rows and other async lines - just surface them. */
                APP_PRINT_INFO1("[tp] << %s", TRACE_STRING(line));
            }
        }
        break;

    default:
        break;
    }
}

/* ------------------------------------------------------------------ */
/* public API                                                         */
/* ------------------------------------------------------------------ */

void app_spi_tcp_tp_init(void)
{
    if (!g_tp.initialized)
    {
        memset(&g_tp, 0, sizeof(g_tp));
        g_tp.state = TP_ST_IDLE;
        g_tp.link_id = TP_LINK_ID_AUTO;
        if (tp_timer_id == 0)
        {
            app_timer_reg_cb(tp_timeout_cb, &tp_timer_id);
        }
        g_tp.initialized = true;
    }
    /* Take over the AT event callback for the duration of the test. */
    app_spi_atcmd_register_callback(tp_at_evt_handler);
    APP_PRINT_INFO0("[tp] spi+tcp throughput test ready");
}

void app_spi_tcp_tp_wifi_connect(const char *ssid, const char *pwd)
{
    char param[96];

    app_spi_tcp_tp_init();

    if (ssid == NULL) { ssid = ""; }
    if (pwd == NULL)  { pwd = ""; }

    /* AT+WLCONN= prefix is supplied by the ATCMD_WLCONN table entry. */
    snprintf(param, sizeof(param), "ssid,%s,pw,%s\r\n", ssid, pwd);

    g_tp.state = TP_ST_WIFI_CONNECTING;
    APP_PRINT_INFO1("[tp] connecting wifi: %s", TRACE_STRING(param));
    app_spi_atcmd_queue_fill(ATCMD_WLCONN, param);
    app_spi_atcmd_trigger_send_flow();
}

void app_spi_tcp_tp_create_server(uint16_t port)
{
    app_spi_tcp_tp_init();
    g_tp.port = (port != 0) ? port : TP_DEFAULT_PORT;
    g_tp.auto_flow = false;      /* manual stepping */
    tp_begin_setup();
    g_tp.state = TP_ST_WAIT_CLIENT;
    APP_PRINT_INFO1("[tp] tcp server on port %u (auto_rcv on). Connect the PC client now.",
                    g_tp.port);
}

void app_spi_tcp_tp_query_state(void)
{
    app_spi_tcp_tp_init();
    tp_send_at("AT+SKTSTATE\r\n");
}

void app_spi_tcp_tp_start_uplink(uint8_t link_id, uint16_t chunk, uint16_t duration_s)
{
    app_spi_tcp_tp_init();

    if (link_id != TP_LINK_ID_AUTO)
    {
        g_tp.link_id = link_id;
        g_tp.link_id_known = true;
    }
    if (!g_tp.link_id_known)
    {
        APP_PRINT_WARN0("[tp] seed link_id unknown; defaulting to 1. "
                        "Use AT+SKTSTATE or pass link_id explicitly.");
        g_tp.link_id = 1;
        g_tp.link_id_known = true;
    }

    g_tp.chunk      = (chunk != 0) ? chunk : TP_DEFAULT_CHUNK;
    if (g_tp.chunk > TP_MAX_CHUNK) { g_tp.chunk = TP_MAX_CHUNK; }
    g_tp.duration_s = (duration_s != 0) ? duration_s : TP_DEFAULT_DURATION_S;

#if TP_TX_DATA_FROM_SDCARD
    /* Payload comes from the SD card: open the file now; tp_pump_one() reads the
     * next g_tp.chunk bytes per SKTSENDRAW and stops at EOF. */
    if (!app_spi_sd_source_open(TP_SD_FILE_PATH, NULL))
    {
        APP_PRINT_ERROR0("[tp] sdcard open fail - uplink aborted");
        g_tp.state = TP_ST_IDLE;
        return;
    }
    g_tp.last_chunk = 0;
#else
    /* pre-fill the raw payload once; every SKTSENDRAW pushes the same bytes */
    memset(g_tp.send_buf, 'A', g_tp.chunk);
#endif

    g_tp.total_bytes = 0;
    g_tp.ok_cnt = 0;
    g_tp.err_cnt = 0;
    g_tp.start_time_ms = os_sys_time_get();
    g_tp.state = TP_ST_SENDING;

    APP_PRINT_INFO3("[tp] start uplink: link_id=%d chunk=%u duration=%us",
                    g_tp.link_id, g_tp.chunk, g_tp.duration_s);

    app_start_timer(&tp_timer_duration_handle, "tp_dur",
                    tp_timer_id, TP_TMR_DURATION, 0, false,
                    (uint32_t)g_tp.duration_s * 1000);

    /* fire the first chunk; subsequent ones are queued on each OK */
    tp_pump_one();
}

void app_spi_tcp_tp_start_uplink_blast(uint8_t link_id, uint32_t total_bytes)
{
    app_spi_tcp_tp_init();

#if SPI_TCP_TP_TX_TASK
    if (link_id != TP_LINK_ID_AUTO)
    {
        g_tp.link_id = link_id;
        g_tp.link_id_known = true;
    }
    if (!g_tp.link_id_known)
    {
        APP_PRINT_WARN0("[tp] seed link_id unknown; defaulting to 1. "
                        "Use AT+SKTSTATE or pass link_id explicitly.");
        g_tp.link_id = 1;
        g_tp.link_id_known = true;
    }

#if TP_TX_DATA_FROM_SDCARD
    /* Read-once-and-stop: the SKTSENDRAW total MUST equal the number of bytes
     * pushed after ">>>" (the slave answers "OK" only after that many bytes),
     * so open the file up front and use its size as the total. The TX task
     * streams the file via app_spi_sd_source_read() and closes it when done. */
    uint32_t file_size = 0;
    if (!app_spi_sd_source_open(TP_SD_FILE_PATH, &file_size))
    {
        APP_PRINT_ERROR0("[tp] sdcard open fail - blast aborted");
        g_tp.state = TP_ST_IDLE;
        return;
    }
    total_bytes = file_size;
#else
    if (total_bytes == 0) { total_bytes = TP_CFG_BLAST_TOTAL_BYTES; }
#endif

    g_tp.blast_total   = total_bytes;
    g_tp.total_bytes   = 0;
    g_tp.ok_cnt        = 0;
    g_tp.err_cnt       = 0;
    g_tp.start_time_ms = os_sys_time_get();
    g_tp.state         = TP_ST_BLASTING;

    APP_PRINT_INFO2("[tp] start uplink blast: link_id=%d total=%u bytes "
                    "(streamed as 16K SPI frames by the TX task)",
                    g_tp.link_id, total_bytes);

    /* one handshake; the TX task pushes the whole payload on ">>>". The clock
     * runs until the slave's terminal "OK" (handled in tp_at_evt_handler). */
    if (!app_spi_atcmd_sendraw_stream(g_tp.link_id, total_bytes))
    {
        APP_PRINT_ERROR0("[tp] sendraw stream start fail");
#if TP_TX_DATA_FROM_SDCARD
        app_spi_sd_source_close();   /* TX task never runs -> release here */
#endif
        g_tp.state = TP_ST_IDLE;
    }
#else
    (void)link_id;
    (void)total_bytes;
    APP_PRINT_ERROR0("[tp] uplink blast not built (SPI_TCP_TP_TX_TASK == 0)");
#endif
}

void app_spi_tcp_tp_create_client(const char *ip, uint16_t port, uint8_t link_id)
{
    char line[64];

    app_spi_tcp_tp_init();

    if (ip == NULL || ip[0] == '\0') { ip = TP_CFG_CLIENT_IP; }
    if (port == 0)                   { port = TP_CFG_CLIENT_PORT; }

    g_tp.auto_flow = false;

    /* AT+SKTCFG=[snd],[rcv],[nodelay],...  -> nodelay=1 (Nagle off) */
    tp_send_at("AT+SKTCFG=,,1\r\n");

    /* AT+SKTCLIENT=<link_id>,1,,<ip>,<port>,,1
     *   arg0 = link id assigned to this client link (reused for SKTSENDRAW)
     *   1    = TCP
     *   <ip>,<port> = PC-hosted server to connect out to
     *   trailing 1 = auto-recv, so downlink data is pushed to us over SPI. */
    snprintf(line, sizeof(line), "AT+SKTCLIENT=%u,1,,%s,%u,,1\r\n",
             link_id, ip, port);
    tp_send_at(line);

    g_tp.link_id = link_id;
    g_tp.link_id_known = true;
    g_tp.state = TP_ST_IDLE;     /* link establishing; run uplink/downlink next */

    APP_PRINT_INFO2("[tp] tcp client -> %s:%u (auto_rcv on)",
                    TRACE_STRING(ip), port);
}

void app_spi_tcp_tp_start_downlink(uint8_t link_id, uint16_t duration_s)
{
    app_spi_tcp_tp_init();

    if (link_id != TP_LINK_ID_AUTO)
    {
        g_tp.link_id = link_id;
        g_tp.link_id_known = true;
    }

    g_tp.duration_s = (duration_s != 0) ? duration_s : TP_DEFAULT_DURATION_S;

    g_tp.total_bytes = 0;
    g_tp.ok_cnt = 0;
    g_tp.err_cnt = 0;

    /* tap SPI RX before starting the clock so no flood bytes are missed */
    app_spi_atcmd_rx_bytes_start();
    g_tp.start_time_ms = os_sys_time_get();
    g_tp.state = TP_ST_RECVING;

    APP_PRINT_INFO2("[tp] start downlink: link_id=%d duration=%us "
                    "(have the PC flood data now)", g_tp.link_id, g_tp.duration_s);

    app_start_timer(&tp_timer_duration_handle, "tp_dur",
                    tp_timer_id, TP_TMR_DURATION, 0, false,
                    (uint32_t)g_tp.duration_s * 1000);
}

void app_spi_tcp_tp_stop(void)
{
    if (g_tp.state == TP_ST_SENDING)
    {
        tp_finish("uplink stopped (user)");
    }
    else if (g_tp.state == TP_ST_RECVING)
    {
        tp_finish_downlink("downlink stopped (user)");
    }
    else if (g_tp.state == TP_ST_BLASTING)
    {
        /* the TX task keeps pushing until <total> is reached; a user stop here
         * just abandons the wait for "OK". Bytes actually delivered are unknown
         * mid-flight, so report only what completed (the real figure comes from
         * the terminal "OK"). */
        g_tp.state = TP_ST_DONE;
#if TP_TX_DATA_FROM_SDCARD
        /* if the ">>>" prompt never arrived the TX task never ran (nor closed
         * the file); release it here. No-op if the task already closed it. */
        app_spi_sd_source_close();
#endif
        tp_report("uplink blast stopped (user)");
    }
    else
    {
        g_tp.state = TP_ST_IDLE;
        app_stop_timer(&tp_timer_duration_handle);
    }
}

void app_spi_tcp_tp_run(const char *ssid, const char *pwd, uint16_t port,
                        uint16_t chunk, uint16_t duration_s)
{
    app_spi_tcp_tp_init();

    g_tp.auto_flow  = true;
    g_tp.port       = (port != 0) ? port : TP_DEFAULT_PORT;
    g_tp.chunk      = (chunk != 0) ? chunk : TP_DEFAULT_CHUNK;
    if (g_tp.chunk > TP_MAX_CHUNK) { g_tp.chunk = TP_MAX_CHUNK; }
    g_tp.duration_s = (duration_s != 0) ? duration_s : TP_DEFAULT_DURATION_S;
    g_tp.link_id    = TP_LINK_ID_AUTO;
    g_tp.link_id_known = false;

    APP_PRINT_INFO2("[tp] auto run: port=%u chunk=%u", g_tp.port, g_tp.chunk);
    APP_PRINT_INFO0("[tp] step1: connect wifi -> step2: server -> step3: wait PC -> step4: pump");

    app_spi_tcp_tp_wifi_connect(ssid, pwd);   /* sets state, GOT_IP drives the rest */
}

/* ------------------------------------------------------------------ */
/* binary CMD payload decoder                                         */
/* ------------------------------------------------------------------ */

typedef enum
{
    TP_ACT_INIT          = 0,
    TP_ACT_WIFI_CONNECT  = 1,
    TP_ACT_CREATE_SERVER = 2,
    TP_ACT_START_UPLINK  = 3,
    TP_ACT_STOP          = 4,
    TP_ACT_QUERY_STATE   = 5,
    TP_ACT_AUTO_RUN      = 6,
    TP_ACT_START_DOWNLINK = 7,  /* measure SPI RX bytes for duration_s     */
    TP_ACT_CREATE_CLIENT  = 8,  /* slave connects out to a PC-hosted server */
    TP_ACT_START_UPLINK_BLAST = 9, /* TX-task streaming blast (act 9)       */
} T_TP_ACTION;

static uint16_t tp_rd_u16(const uint8_t *p)
{
    return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t tp_rd_u32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/**
 * @brief Split an ASCII "ssid,password" blob (length n, not NUL terminated)
 *        into two NUL-terminated strings. Splits on the first comma.
 */
static void tp_split_ssid_pwd(const uint8_t *src, uint16_t n,
                              char *ssid, uint16_t ssid_sz,
                              char *pwd, uint16_t pwd_sz)
{
    char tmp[96];

    ssid[0] = '\0';
    pwd[0] = '\0';
    if (n == 0) { return; }
    if (n >= sizeof(tmp)) { n = sizeof(tmp) - 1; }

    memcpy(tmp, src, n);
    tmp[n] = '\0';

    char *comma = strchr(tmp, ',');
    if (comma == NULL)
    {
        strncpy(ssid, tmp, ssid_sz - 1);
        ssid[ssid_sz - 1] = '\0';
        return;
    }

    *comma = '\0';
    strncpy(ssid, tmp, ssid_sz - 1);
    ssid[ssid_sz - 1] = '\0';
    strncpy(pwd, comma + 1, pwd_sz - 1);
    pwd[pwd_sz - 1] = '\0';
}

void app_spi_tcp_tp_handle_cmd(uint8_t *cmd_ptr, uint16_t cmd_len)
{
    if (cmd_ptr == NULL || cmd_len < 3)
    {
        APP_PRINT_ERROR1("[tp] cmd too short: %d", cmd_len);
        return;
    }

    uint8_t  action = cmd_ptr[2];
    uint8_t *p      = &cmd_ptr[3];      /* parameters start here          */
    uint16_t plen   = cmd_len - 3;      /* parameter bytes available      */
    char ssid[48], pwd[48];

    switch (action)
    {
    case TP_ACT_INIT:
        app_spi_tcp_tp_init();
        break;

    case TP_ACT_WIFI_CONNECT:
#if TP_PARAMS_FROM_MACRO
        app_spi_tcp_tp_wifi_connect(TP_CFG_WIFI_SSID, TP_CFG_WIFI_PWD);
#else
        tp_split_ssid_pwd(p, plen, ssid, sizeof(ssid), pwd, sizeof(pwd));
        app_spi_tcp_tp_wifi_connect(ssid, pwd);
#endif
        break;

    case TP_ACT_CREATE_SERVER:
#if TP_PARAMS_FROM_MACRO
        app_spi_tcp_tp_create_server(TP_CFG_SERVER_PORT);
#else
        app_spi_tcp_tp_create_server((plen >= 2) ? tp_rd_u16(p) : 0);
#endif
        break;

    case TP_ACT_START_UPLINK:
        {
#if TP_PARAMS_FROM_MACRO
            uint8_t  link_id    = TP_CFG_LINK_ID;
            uint16_t chunk      = TP_CFG_CHUNK;
            uint16_t duration_s = TP_CFG_DURATION_S;
#else
            uint8_t  link_id    = (plen >= 1) ? p[0] : TP_LINK_ID_AUTO;
            uint16_t chunk      = (plen >= 3) ? tp_rd_u16(&p[1]) : 0;
            uint16_t duration_s = (plen >= 5) ? tp_rd_u16(&p[3]) : 0;
#endif
#if TP_UPLINK_USE_BLAST
            /* Auto-blast: stream TP_CFG_BLAST_TOTAL_BYTES (10 MB) to the slave as
             * back-to-back SPI frames, each carrying (SPI_XMIT_SIZE - 8) payload
             * bytes of a continuous 0..127 ramp, via one SKTSENDRAW handshake. */
            (void)chunk;
            (void)duration_s;
            app_spi_tcp_tp_start_uplink_blast(link_id, TP_CFG_BLAST_TOTAL_BYTES);
#else
            app_spi_tcp_tp_start_uplink(link_id, chunk, duration_s);
#endif
        }
        break;

    case TP_ACT_STOP:
        app_spi_tcp_tp_stop();
        break;

    case TP_ACT_QUERY_STATE:
        app_spi_tcp_tp_query_state();
        break;

    case TP_ACT_AUTO_RUN:
        {
            uint16_t port       = (plen >= 2) ? tp_rd_u16(&p[0]) : 0;
            uint16_t chunk      = (plen >= 4) ? tp_rd_u16(&p[2]) : 0;
            uint16_t duration_s = (plen >= 6) ? tp_rd_u16(&p[4]) : 0;
            if (plen > 6)
            {
                tp_split_ssid_pwd(&p[6], plen - 6, ssid, sizeof(ssid), pwd, sizeof(pwd));
            }
            else
            {
                ssid[0] = '\0';
                pwd[0]  = '\0';
            }
            app_spi_tcp_tp_run(ssid, pwd, port, chunk, duration_s);
        }
        break;

    case TP_ACT_START_DOWNLINK:
        {
#if TP_PARAMS_FROM_MACRO
            uint8_t  link_id    = TP_CFG_LINK_ID;
            uint16_t duration_s = TP_CFG_DURATION_S;
#else
            uint8_t  link_id    = (plen >= 1) ? p[0] : TP_LINK_ID_AUTO;
            uint16_t duration_s = (plen >= 3) ? tp_rd_u16(&p[1]) : 0;
#endif
            app_spi_tcp_tp_start_downlink(link_id, duration_s);
        }
        break;

    case TP_ACT_CREATE_CLIENT:
        {
#if TP_PARAMS_FROM_MACRO
            app_spi_tcp_tp_create_client(TP_CFG_CLIENT_IP, TP_CFG_CLIENT_PORT,
                                         TP_CFG_CLIENT_LINK_ID);
#else
            char     ip[16];
            uint16_t cport   = 0;
            uint8_t  link_id = TP_CFG_CLIENT_LINK_ID;
            if (plen >= 6)
            {
                /* [0..3]=ip octets  [4..5]=port LE  [6]=link_id (optional) */
                snprintf(ip, sizeof(ip), "%u.%u.%u.%u",
                         p[0], p[1], p[2], p[3]);
                cport   = tp_rd_u16(&p[4]);
                link_id = (plen >= 7) ? p[6] : TP_CFG_CLIENT_LINK_ID;
                app_spi_tcp_tp_create_client(ip, cport, link_id);
            }
            else
            {
                /* no params: fall back to the configured defaults */
                app_spi_tcp_tp_create_client(NULL, 0, TP_CFG_CLIENT_LINK_ID);
            }
#endif
        }
        break;

    case TP_ACT_START_UPLINK_BLAST:
        {
#if TP_PARAMS_FROM_MACRO
            uint8_t  link_id = TP_CFG_LINK_ID;
            uint32_t total   = TP_CFG_BLAST_TOTAL_BYTES;
#else
            /* [0]=link_id  [1..4]=total bytes LE (0/absent -> default 10 MB) */
            uint8_t  link_id = (plen >= 1) ? p[0] : TP_LINK_ID_AUTO;
            uint32_t total   = (plen >= 5) ? tp_rd_u32(&p[1]) : 0;
#endif
            app_spi_tcp_tp_start_uplink_blast(link_id, total);
        }
        break;

    default:
        APP_PRINT_ERROR1("[tp] unknown action %d", action);
        break;
    }
}
