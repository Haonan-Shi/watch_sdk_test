/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
/*
 * Downlink (host -> SoC) average-rate measurement for the wifi data test.
 * See wifi_sdio_rx.h for the design. The rate is computed here, in the
 * customer app, via the wifi_sdio read-callback interface -- module/wifi/
 * wifi_sdio.c is left untouched.
 */

#include <stdint.h>
#include <stdbool.h>
#include <zephyr/kernel.h>     /* k_uptime_get, k_timer */

#include "trace.h"
#include "wifi_sdio.h"         /* wifi_sdio_data_read_cb_reg/unreg, P_FUN_SDIO_DATA_CB */
#include "wifi_desc.h"         /* PRXDESC, pkt_len */

#include "wifi_sdio_rx.h"

/* The host streams back-to-back, so real inter-packet gaps are sub-millisecond;
 * this idle window only has to outlast normal jitter to detect "last packet". */
#define WIFI_RX_IDLE_TIMEOUT_MS   1000

/* ip/port passed to reg/unreg. wifi_sdio_find_data_read_cb() ignores the tuple
 * and returns the first registered callback, so any unique key works; (0, 0) is
 * distinct from the camera path's (0xC0A82701, 554). */
#define WIFI_RX_KEY_IP            0
#define WIFI_RX_KEY_PORT          0

static volatile bool  rx_measuring;     /* between first packet and idle timeout */
static bool           rx_registered;
static uint32_t       rx_total_bytes;   /* payload received this run */
static int64_t        rx_first_ms;      /* time of first packet */
static int64_t        rx_last_ms;       /* time of most recent packet */
static int64_t        rx_win_ms;        /* start of the current 1 s window */
static uint32_t       rx_win_bytes;     /* payload received within the window */
static struct k_timer rx_idle_timer;

/* Compute and print the average downlink rate over [first packet, last packet].
 * Runs either from the idle timer (stream ended) or from _stop() (teardown). */
static void rx_report(void)
{
    if (!rx_measuring)
    {
        return;
    }
    rx_measuring = false;

    uint32_t elapsed_ms = (uint32_t)(rx_last_ms - rx_first_ms);
    if (elapsed_ms == 0)
    {
        elapsed_ms = 1;
    }
    uint32_t avg_kbps = (uint32_t)(((uint64_t)rx_total_bytes * 8) / elapsed_ms);
    printk("[wifi] DOWNLINK done: total=%u bytes, time=%u ms, avg=%u kbit/s\n",
           rx_total_bytes, elapsed_ms, avg_kbps);
}

/* System-clock timer context: the stream has been idle for the timeout, so the
 * last packet has been seen. Freeze and report the measurement. */
static void rx_idle_expiry(struct k_timer *timer)
{
    (void)timer;
    rx_report();
}

/* SDIO read callback. Runs on the wifi task (via wifi_sdio_msg_handler) for each
 * forwarded packet. p_data points at the RXDESC; pkt_len is the payload size. */
static uint16_t rx_data_cb(void *p_data, uint16_t len)
{
    (void)len;
    PRXDESC  p_rxdesc = (PRXDESC)p_data;
    uint16_t pkt_len  = p_rxdesc->pkt_len;
    int64_t  now      = k_uptime_get();

    if (!rx_measuring)
    {
        rx_measuring   = true;
        rx_total_bytes = 0;
        rx_first_ms    = now;
        rx_win_ms      = now;
        rx_win_bytes   = 0;
        printk("[wifi] DOWNLINK first packet, start timing\n");
    }
    rx_total_bytes += pkt_len;
    rx_last_ms      = now;

    /* Print this second's average rate once a full second has elapsed. */
    rx_win_bytes += pkt_len;
    uint32_t win_ms = (uint32_t)(now - rx_win_ms);
    if (win_ms >= 1000)
    {
        uint32_t avg_kbps = (uint32_t)(((uint64_t)rx_win_bytes * 8) / win_ms);
        printk("[wifi] DOWNLINK 1s: bytes=%u, time=%u ms, avg=%u kbit/s\n",
               rx_win_bytes, win_ms, avg_kbps);
        rx_win_ms    = now;
        rx_win_bytes = 0;
    }

    /* Re-arm the idle detector; it fires only once packets stop arriving. */
    k_timer_start(&rx_idle_timer, K_MSEC(WIFI_RX_IDLE_TIMEOUT_MS), K_NO_WAIT);
    return 0;
}

void wifi_sdio_rx_start(void)
{
    rx_measuring   = false;
    rx_total_bytes = 0;
    rx_win_bytes   = 0;

    if (!rx_registered)
    {
        k_timer_init(&rx_idle_timer, rx_idle_expiry, NULL);
        if (wifi_sdio_data_read_cb_reg(WIFI_RX_KEY_IP, WIFI_RX_KEY_PORT, rx_data_cb))
        {
            rx_registered = true;
        }
        else
        {
            printk("[wifi] DOWNLINK: read cb register failed\n");
            return;
        }
    }
    printk("[wifi] DOWNLINK rx rate: armed, waiting for packets\n");
}

void wifi_sdio_rx_stop(void)
{
    k_timer_stop(&rx_idle_timer);
    rx_report();   /* flush a measurement still in progress at teardown */

    if (rx_registered)
    {
        wifi_sdio_data_read_cb_unreg(WIFI_RX_KEY_IP, WIFI_RX_KEY_PORT);
        rx_registered = false;
    }
    printk("[wifi] DOWNLINK rx rate: disarmed\n");
}
