/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
/*
 * Memory-sourced SDIO TX pump for the wifi data-uplink test. See the header
 * for the relationship to module/wifi/wifi_sdio_tx_test.c. The data source is a
 * fixed PSRAM pattern buffer (reuses the same scratch region as the reference
 * SDIO test, which is fine because the two tests never run concurrently).
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <os_sched.h>          /* os_delay */
#include <zephyr/kernel.h>     /* k_uptime_get (average-rate timing) */

#include "trace.h"
#include "wdg.h"
#include "os_queue.h"
#include "wifi_sdio.h"         /* wifi_sdio_data_write_queue_*, wifi_sdio_write_data */
#include "wifi_app.h"          /* T_WIFI_MSG, app_send_msg_to_wifitask, EVENT_USER_APP_DEFINE */

#include "wifi_sdio_mem_tx.h"
#include "psram_section.h"

/*============================================================================*
 * Macro Definitions (mirrors wifi_sdio_tx_test.c)
 *============================================================================*/
#define SDIO_DATA_SEND_SIZE    (2048 - 44)   /* SDIO TX header is 40 bytes */
#define SDIO_SEND_TIMES         5
/* SoC-controlled uplink total. The pump sends exactly this many bytes and then
 * reports one average rate, so the figure covers a fixed, known amount of data.
 * Bounded to keep the test well within the scenario timeout even on a slow link
 * (20 MB at ~3 Mbps ~= 55 s); raise it for a longer, steadier measurement. */
#define SDIO_SEND_TOTAL_SIZE    (512 * 1024)
#define SDIO_TX_EVENT_CNT       (SDIO_SEND_TOTAL_SIZE / SDIO_SEND_TIMES / SDIO_DATA_SEND_SIZE)
#define SDIO_DATA_BUF_SIZE      (SDIO_DATA_SEND_SIZE * SDIO_SEND_TIMES)

/*============================================================================*
 * Globals
 *============================================================================*/

/** In-RAM (PSRAM) data source; filled once with a pattern, no SD card. */
static uint8_t  sdio_tx_buffer[SDIO_SEND_TOTAL_SIZE]  SECTION_PSRAM1_NC ;
static bool     sdio_tx_running;

/* Uplink (SoC -> host) average-rate accounting. sdio_tx_buf() fully drains
 * each chunk into the chip before returning, so a byte counted here has really
 * been handed to the SDIO TX FIFO. Timing brackets the *actual* data flow: from
 * the first chunk handed off to the last one (mirrors wifi_sdio_rx.c, which
 * times first-packet -> last-packet), instead of from the pump kick -- so the
 * app->wifi-task handoff latency and any pre-flow scheduling gap are excluded. */
static volatile bool tx_measuring;    /* set once the first chunk is sent */
static uint32_t tx_total_bytes;       /* bytes handed to the chip while measuring */
static int64_t  tx_first_ms;          /* k_uptime_get() at first chunk */
static int64_t  tx_last_ms;           /* k_uptime_get() at most recent chunk */

/* Rolling 1-second window: bytes handed off since the current window opened, so
 * we can print this second's average rate once every second while the pump runs. */
static int64_t  tx_win_ms;            /* k_uptime_get() at the start of the window */
static uint32_t tx_win_bytes;         /* bytes handed off within the current window */

/* Print this second's average rate whenever a full second has elapsed since the
 * window opened, then reset the window. Called after each chunk hand-off. */
static void tx_persec(void)
{
    uint32_t win_ms = (uint32_t)(tx_last_ms - tx_win_ms);
    if (win_ms < 1000)
    {
        return;
    }
    uint32_t avg_kbps = (uint32_t)(((uint64_t)tx_win_bytes * 8) / win_ms);
    printk("[wifi] UPLINK 1s: bytes=%u, time=%u ms, avg=%u kbit/s\n",
           tx_win_bytes, win_ms, avg_kbps);
    tx_win_ms    = tx_last_ms;
    tx_win_bytes = 0;
}

/* Compute and print the average uplink rate over [first chunk, last chunk]. */
static void tx_report(const char *tag)
{
    if (!tx_measuring)
    {
        return;   /* nothing was ever sent this run */
    }
    tx_measuring = false;

    uint32_t elapsed_ms = (uint32_t)(tx_last_ms - tx_first_ms);
    if (elapsed_ms == 0)
    {
        elapsed_ms = 1;
    }
    uint32_t avg_kbps = (uint32_t)(((uint64_t)tx_total_bytes * 8) / elapsed_ms);
    printk("[wifi] UPLINK %s: total=%u bytes, time=%u ms, avg=%u kbit/s\n",
           tag, tx_total_bytes, elapsed_ms, avg_kbps);
}

extern T_OS_QUEUE sdio_write_queue;   /* defined in module/wifi/wifi_sdio.c */

/*============================================================================*
 * SDIO TX
 *============================================================================*/

/* Push one chunk into the SDIO write queue and drain it (blocking on the wifi
 * task, retrying with a short delay when the chip's TX buffer is full). */
static void sdio_tx_buf(uint8_t *buf, uint32_t len)
{
    static uint8_t resend_cnt = 0;
    wifi_sdio_data_write_queue_fill(0, 0, buf, len);

    while (sdio_write_queue.count)
    {
        T_WIFI_SDIO_WRITE_QUEUE *write_pkt = wifi_sdio_data_write_queue_peek(0);
        if (!wifi_sdio_write_data(write_pkt))
        {
            wifi_sdio_data_write_queue_flush(1);
            resend_cnt = 0;
        }
        else
        {
            APP_PRINT_INFO1("[wifi] sdio mem tx buf full, os_delay 10ms resend_cnt %d",
                            resend_cnt);
            os_delay(10);
            resend_cnt++;
            continue;
        }
    }
}

static int sdio_tx_proc(char *p_param);   /* forward decl (wifi-task callback) */

/* Post the pump callback to the wifi task (mirrors sdio_to_app_task). */
static void sdio_tx_kick(void)
{
    T_WIFI_MSG cmd_msg;
    cmd_msg.event  = EVENT_USER_APP_DEFINE;
    cmd_msg.u.buf  = NULL;
    cmd_msg.msg_cb = (wifi_msg_cb)sdio_tx_proc;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] sdio mem tx: wifi task msg send fail !");
    }
}

/* Runs on the wifi task: send SEND_TIMES chunks, then re-arm until the total
 * byte target is reached or the pump is stopped. */
static int sdio_tx_proc(char *p_param)
{
    (void)p_param;
    static uint32_t evt_cnt = 0;

    if (!sdio_tx_running)
    {
        evt_cnt = 0;
        return 0;
    }

    evt_cnt++;
    APP_PRINT_INFO1("[wifi] sdio mem tx evt_cnt = %d", evt_cnt);

    wdg_kick();

    /* Start the stopwatch on the first chunk actually sent (mirrors rx.c's
     * first-packet start), not at the pump kick. */
    if (!tx_measuring)
    {
        tx_measuring   = true;
        tx_total_bytes = 0;
        tx_first_ms    = k_uptime_get();
        tx_win_ms      = tx_first_ms;
        tx_win_bytes   = 0;
        printk("[wifi] UPLINK first chunk, start timing\n");
    }

    for (int i = 0; i < SDIO_SEND_TIMES; i++)
    {
        sdio_tx_buf(sdio_tx_buffer + i * SDIO_DATA_SEND_SIZE,
                    SDIO_DATA_SEND_SIZE);
    }
    tx_total_bytes += SDIO_SEND_TIMES * SDIO_DATA_SEND_SIZE;
    tx_win_bytes   += SDIO_SEND_TIMES * SDIO_DATA_SEND_SIZE;
    tx_last_ms = k_uptime_get();   /* advance to the most recent chunk */
    tx_persec();                   /* emit this second's rate if a second elapsed */

    if (evt_cnt < SDIO_TX_EVENT_CNT)
    {
        sdio_tx_kick();   /* re-arm */
    }
    else
    {
        sdio_tx_running = false;
        evt_cnt = 0;
        tx_report("done");   /* SoC-controlled total reached */
    }
    return 0;
}

/*============================================================================*
 * Public API
 *============================================================================*/

void wifi_sdio_mem_tx_start(void)
{
    if (sdio_tx_running)
    {
        APP_PRINT_WARN0("[wifi] sdio mem tx: already running");
        return;
    }

    /* Fill the RAM source once with a recognizable byte pattern. */
    for (uint32_t i = 0; i < SDIO_DATA_BUF_SIZE; i++)
    {
        sdio_tx_buffer[i] = (uint8_t)i;
    }

    tx_measuring   = false;   /* first chunk in sdio_tx_proc() starts timing */
    tx_total_bytes = 0;

    sdio_tx_running = true;
    APP_PRINT_INFO0("[wifi] sdio mem tx: start (memory source)");
    sdio_tx_kick();
}

void wifi_sdio_mem_tx_stop(void)
{
    bool was_running = sdio_tx_running;
    sdio_tx_running = false;
    if (was_running)
    {
        tx_report("stopped");   /* early stop: report what was sent so far */
    }
}
