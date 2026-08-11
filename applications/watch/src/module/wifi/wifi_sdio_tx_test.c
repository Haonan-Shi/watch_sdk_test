/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


/*============================================================================*
 * Header Files
 *============================================================================*/
#include <zephyr/fs/fs.h>
#include <string.h>
#include "trace.h"
#include "section.h"
#include "wdg.h"
#include "wifi_sdio.h"
#include "os_queue.h"
#include "os_sched.h"
#include "app_fs_if.h"
#include <os_sched.h>

/*============================================================================*
 * Macro Definitions
 *============================================================================*/
#define DATA_SEND_SIZE    (2048 - 44)  /* SDIO TX header size is 40 bytes */
#define SEND_TIMES         5
#define SEND_TOTAL_SIZE    100*1024*1024
#define SDIO_TX_EVENT_CNT   SEND_TOTAL_SIZE/SEND_TIMES/DATA_SEND_SIZE
#define RATE_WINDOW_PKTS    256        /* report a windowed throughput every N packets */
#define DATA_BUF_SIZE      (DATA_SEND_SIZE * SEND_TIMES)
#define PSRAM0_BASE_ADDR   0x22000000
#define TX_BUFFER_SIZE     DATA_SEND_SIZE * SEND_TIMES

/*============================================================================*
 * Global Variables
 *============================================================================*/

/** PSRAM buffer for SDIO TX test data */
static uint8_t *tx_buffer = (uint8_t *)PSRAM0_BASE_ADDR;

/** External file handle for SD card test */
extern struct fs_file_t test_dat;

/** External function for SDIO TX */
void sdio_tx_buf(uint8_t *buf, uint32_t len);

extern T_OS_QUEUE  sdio_write_queue;

/*============================================================================*
 *                          SD Card Test Functions
 *============================================================================*/

/** SD card read buffer size */
#define   SDCARD_READ_ONCE_LEN   128

/** Test file path on SD card */
static const char test_file[] = "/SD:/audio/test.bin";

/** File handle for SD card test */
struct fs_file_t test_dat;

/** Read buffer */
uint8_t  buf[SDCARD_READ_ONCE_LEN] = {0};

/**
 * @brief Test SD card read functionality
 * @return 0 on success, 1 on failure
 */
bool sdcard_test_start(void)
{
    APP_PRINT_INFO0("sdcard_test_start ");

    fs_file_t_init(&test_dat);
    int res = fs_open(&test_dat, (const char *)test_file, FS_O_READ);

    if (res < 0)
    {
        APP_PRINT_INFO1("sdcard_test_open fail res %d ", res);
        return 1;
    }

    APP_PRINT_INFO0("sdcard_test read start");
    uint32_t i = 0;
    for (i = 0; i < 100; i++)
    {
        fs_read(&test_dat, buf, SDCARD_READ_ONCE_LEN);
    }
    APP_PRINT_INFO1("sdcard_test read end i= %d", i);

    APP_PRINT_INFO1("sdcard_test= %b", TRACE_BINARY(128, buf));
    app_fs_disk_power_down_disable(APP_DISK_CHECK_WIFI_TEST);
    return 0;
}

bool sdcard_test_end(void)
{
    fs_close(&test_dat);
    app_fs_disk_power_down_enable(APP_DISK_CHECK_WIFI_TEST);
    return 1;
}

/*============================================================================*
 * SDIO TX Test Function
 *============================================================================*/
#include "wifi_app.h"
#include "wifi_sdio_tx_test.h"
int sdio_tx_proc(char *p_param);
int sdio_tx_test(char *p_param)
{
    APP_PRINT_INFO1("sdio_tx_test %s \n", TRACE_STRING(p_param));
    sdio_to_app_task(WIFI_SDIO_TX_EVENT, NULL);
    return 0;
}

void sdio_to_app_task(T_WIFI_SDIO_TO_APP_TYPE type, void *data)
{
    switch (type)
    {
    case WIFI_SDIO_TX_EVENT:
        T_WIFI_MSG cmd_msg;
        cmd_msg.event = EVENT_USER_APP_DEFINE;//EVENT_WIFI_CAM_ENTER;
        cmd_msg.u.buf = (void *)data;/*ip addrress*/
        cmd_msg.msg_cb = (wifi_msg_cb)sdio_tx_proc;
        if (app_send_msg_to_wifitask(&cmd_msg) == false)
        {
            APP_PRINT_ERROR0("[wifi] wifi task msg send fail !");
        }
        break;

    default:
        break;
    }

}

/**
 * @brief Report the average SDIO TX rate over a whole run.
 * @param pkt_cnt     number of packets actually sent
 * @param total_bytes total bytes pushed to SDIO during the run
 * @param start_ms    timestamp (ms) of the run's first packet
 */
static void sdio_tx_report_avg(uint32_t pkt_cnt, uint64_t total_bytes, uint64_t start_ms)
{
    uint64_t total_ms = os_sys_time_get() - start_ms;
    if (total_ms == 0) { total_ms = 1; }            /* avoid divide-by-zero */

    /* bytes * 8 / ms == kbit/s */
    uint32_t avg_kbps = (uint32_t)((total_bytes * 8ULL) / total_ms);

    APP_PRINT_INFO6("sdio_tx DONE: %u pkts, %u bytes in %u ms, avg rate %u kbps (%u.%03u Mbps)",
                    pkt_cnt, (uint32_t)total_bytes, (uint32_t)total_ms,
                    avg_kbps, avg_kbps / 1000, avg_kbps % 1000);
}

/**
 * @brief SDIO TX test function
 * @param p_param Parameters (currently unused)
 * @return 0 on success, -1 on failure
 */
int sdio_tx_proc(char *p_param)
{
    static uint32_t sdio_tx_event_cnt = 0;
    static uint64_t test_start_ms     = 0;   /* timestamp of this run's 1st packet */
    static uint64_t total_tx_bytes    = 0;   /* bytes pushed to SDIO this run */
    static uint64_t win_start_ms      = 0;   /* start time of the current report window */
    static uint64_t win_bytes         = 0;   /* bytes pushed in the current window */

    int i = 0;
    ssize_t actual_read_len = 0;
    uint32_t pkt_bytes = SEND_TIMES * DATA_SEND_SIZE;   /* bytes sent per packet */

    sdio_tx_event_cnt++;

    /* (re)start statistics on the first packet of a run */
    if (sdio_tx_event_cnt == 1)
    {
        test_start_ms  = os_sys_time_get();
        total_tx_bytes = 0;
        win_start_ms   = test_start_ms;
        win_bytes      = 0;
    }

    (void)p_param;

    wdg_kick();
    actual_read_len = fs_read(&test_dat, tx_buffer, DATA_BUF_SIZE);

    if (actual_read_len <= 0)
    {
        /* Zephyr fs_read returns 0 when reaching end of file -> run finished */
        APP_PRINT_ERROR1("sdio_tx_test: read data fail res = %d", actual_read_len);
        sdio_tx_report_avg(sdio_tx_event_cnt - 1, total_tx_bytes, test_start_ms);
        sdio_tx_event_cnt = 0;                /* reset for the next run */
        return -1;
    }

    for (i = 0; i < SEND_TIMES; i++)
    {
        sdio_tx_buf((tx_buffer + i * DATA_SEND_SIZE), DATA_SEND_SIZE);
    }

    total_tx_bytes += pkt_bytes;
    win_bytes      += pkt_bytes;

    /* A single packet's time is NOT a meaningful rate here: sdio_tx_buf() mostly
     * just enqueues into the SDIO TX FIFO and returns immediately (~0 ms), and
     * only blocks on os_delay(10) when the FIFO is full. So per-packet timing is
     * bimodal (~0 ms -> tens of Mbps, ~10 ms -> a few Mbps) and measures enqueue
     * latency, not transfer rate. Report a WINDOWED average every RATE_WINDOW_PKTS
     * packets -- long enough that the ms clock and the occasional stall average
     * out into the real sustained throughput. */
    if ((sdio_tx_event_cnt % RATE_WINDOW_PKTS) == 0)
    {
        uint64_t now    = os_sys_time_get();
        uint64_t win_ms = now - win_start_ms;
        if (win_ms == 0) { win_ms = 1; }      /* avoid divide-by-zero */

        /* bytes * 8 / ms == kbit/s */
        uint32_t win_kbps = (uint32_t)((win_bytes * 8ULL) / win_ms);

        APP_PRINT_INFO7("sdio_tx %u/%u: window %u pkts in %u ms, rate %u kbps (%u.%03u Mbps)",
                        sdio_tx_event_cnt, (uint32_t)SDIO_TX_EVENT_CNT,
                        (uint32_t)RATE_WINDOW_PKTS, (uint32_t)win_ms,
                        win_kbps, win_kbps / 1000, win_kbps % 1000);

        win_start_ms = now;                   /* start a fresh window */
        win_bytes    = 0;
    }

    if (sdio_tx_event_cnt < SDIO_TX_EVENT_CNT)
    {
        sdio_to_app_task(WIFI_SDIO_TX_EVENT, NULL);
    }
    else
    {
        /* all packets sent: report the overall average rate, then reset */
        sdio_tx_report_avg(sdio_tx_event_cnt, total_tx_bytes, test_start_ms);
        sdio_tx_event_cnt = 0;
    }
    return 0;
}

void sdio_tx_buf(uint8_t *buf, uint32_t len)
{
    static uint8_t resend_cnt = 0;
    wifi_sdio_data_write_queue_fill(0, 0, buf, len);

    while (sdio_write_queue.count)
    {
        //  APP_PRINT_INFO1("sdio_write_queue.count = %d", sdio_write_queue.count);
        T_WIFI_SDIO_WRITE_QUEUE *write_pkt = wifi_sdio_data_write_queue_peek(0);
        if (!wifi_sdio_write_data(write_pkt))
        {
            wifi_sdio_data_write_queue_flush(1);
            resend_cnt = 0;
        }
        else
        {
            APP_PRINT_INFO1("z2plus sdio tx buf full os_delay 5ms resend_cnt %d ", resend_cnt);
            os_delay(2);
            resend_cnt ++;
            continue;
        }
    }
}