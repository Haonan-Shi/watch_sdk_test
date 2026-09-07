/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */


/*============================================================================*
 * Header Files
 *============================================================================*/
#include <zephyr/fs/fs.h>
#include <zephyr/kernel.h>   /* k_busy_wait(): us-resolution wait, tick-independent */
#include <string.h>
#include "trace.h"
#include "section.h"
#include "wdg.h"
#include "wifi_sdio.h"
#include "os_queue.h"
#include "os_sched.h"
#include "app_fs_if.h"

/*============================================================================*
 * Macro Definitions
 *============================================================================*/
#define DATA_SEND_SIZE    (2048 - 44)  /* SDIO TX header size is 40 bytes */
#define SEND_TIMES         5
#define SEND_TOTAL_SIZE    100*1024*1024
#define SDIO_TX_EVENT_CNT   SEND_TOTAL_SIZE/SEND_TIMES/DATA_SEND_SIZE
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

/* microsecond timestamp (os_sys_time_get() is only 10 ms resolution because the
 * kernel tick is 100 Hz, so it is useless for splitting read-vs-write per event) */
extern uint32_t sys_timestamp_get_us(void);

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
    // app_fs_disk_power_down_disable(APP_DISK_CHECK_WIFI_TEST);
    return 0;
}

bool sdcard_test_end(void)
{
    fs_close(&test_dat);
    // app_fs_disk_power_down_enable(APP_DISK_CHECK_WIFI_TEST);
    return false;
}

/*============================================================================*
 * SDIO TX Test Function
 *============================================================================*/
#include "wifi_app.h"
#include "wifi_sdio_tx_test.h"
int sdio_tx_proc(char *p_param);
int sdio_tx_test(char *p_param)
{
    if (sdcard_test_start())
    {
        APP_PRINT_ERROR0("[SDCARD] read SDCARD file fail !");
        return -1;
    }
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
        cmd_msg.msg_cb = sdio_tx_proc;
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

    /* Mbps printed digit-by-digit: the RTK trace decoder does NOT honour the
     * "%03u" zero-pad flag (it emits the '0' literally, e.g. 6.851 -> "6.0851"). */
    APP_PRINT_INFO8("sdio_tx DONE: %u pkts, %u bytes in %u ms, avg rate %u kbps (%u.%u%u%u Mbps)",
                    pkt_cnt, (uint32_t)total_bytes, (uint32_t)total_ms, avg_kbps,
                    avg_kbps / 1000, (avg_kbps % 1000) / 100,
                    (avg_kbps % 100) / 10, avg_kbps % 10);
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
    static uint64_t last_report_bytes = 0;   /* total_tx_bytes at the previous progress print */
    /* us-resolution split of where each event's wall-clock goes */
    static uint64_t total_read_us       = 0; /* cumulative us in fs_read() this run */
    static uint64_t total_push_us       = 0; /* cumulative us in the 5x sdio_tx_buf() this run */
    static uint32_t last_report_us      = 0; /* us timestamp of the previous progress print */
    static uint64_t last_report_read_us = 0;
    static uint64_t last_report_push_us = 0;

    int i = 0;
    ssize_t actual_read_len = 0;
    uint32_t rd_us;
    uint32_t push_us;
    uint32_t pkt_bytes = SEND_TIMES * DATA_SEND_SIZE;   /* bytes sent per event */

    sdio_tx_event_cnt++;

    /* (re)start statistics on the first packet of a run */
    if (sdio_tx_event_cnt == 1)
    {
        test_start_ms       = os_sys_time_get();
        total_tx_bytes      = 0;
        last_report_bytes   = 0;
        total_read_us       = 0;
        total_push_us       = 0;
        last_report_us      = sys_timestamp_get_us();
        last_report_read_us = 0;
        last_report_push_us = 0;
    }

    (void)p_param;

    wdg_kick();
    /* Pure-push TX throughput: read the source block from SD only ONCE, then
     * reuse it for every event. MEASURED here the SDIO write to the 8720c (WiFi
     * SDIO is 1-bit) is ~18.8 Mbps and DOMINATES the window (~70%); the SD read,
     * even at 4-bit (~47 Mbps), still adds ~28% when done per event, capping
     * real-streaming at ~13 Mbps. For a TX-throughput benchmark we take the SD
     * read out of the per-event path so every event is a pure SDIO push -> the
     * ~18.8 Mbps / TCP-capped ~17 ceiling. (Real per-event file streaming tops
     * out ~13 Mbps on this HW: 1-bit WiFi-SDIO write + no read/write overlap.) */
    if (sdio_tx_event_cnt == 1)
    {
        rd_us = sys_timestamp_get_us();
        actual_read_len = fs_read(&test_dat, tx_buffer, DATA_BUF_SIZE);
        total_read_us += (uint32_t)(sys_timestamp_get_us() - rd_us);

        if (actual_read_len <= 0)
        {
            /* Zephyr fs_read returns 0 at end of file -> nothing to send */
            APP_PRINT_ERROR1("sdio_tx_test: read data fail res = %d", actual_read_len);
            sdio_tx_report_avg(sdio_tx_event_cnt - 1, total_tx_bytes, test_start_ms);
            sdio_tx_event_cnt = 0;            /* reset for the next run */
            return -1;
        }
    }

    /* Time the SDIO push (all SEND_TIMES writes) in microseconds. */
    push_us = sys_timestamp_get_us();
    for (i = 0; i < SEND_TIMES; i++)
    {
        sdio_tx_buf((tx_buffer + i * DATA_SEND_SIZE), DATA_SEND_SIZE);
    }
    total_push_us += (uint32_t)(sys_timestamp_get_us() - push_us);

    total_tx_bytes += pkt_bytes;

    /* Throttle the progress print: only every 5th packet, plus the very first
     * and the very last one. The avg rate is the WINDOWED rate over the packets
     * since the previous print (i.e. "these 5"), not the cumulative run average. */
    if ((sdio_tx_event_cnt == 1) ||
        (sdio_tx_event_cnt == (uint32_t)SDIO_TX_EVENT_CNT) ||
        ((sdio_tx_event_cnt % 5) == 0))
    {
        /* Microsecond breakdown over this window (all deltas from sys_timestamp_get_us):
         *   win  = total wall-clock, rd = fs_read, push = 5x sdio_tx_buf,
         *   gap  = win - rd - push = message re-dispatch + scheduling + logging.
         * This tells us exactly where the ~7 ms/event goes: if rd+push ~= win the
         * transfers are serialized (fix: overlap SD read with SDIO write); if gap
         * dominates the cost is the per-event re-dispatch. */
        uint32_t now_us      = sys_timestamp_get_us();
        uint32_t win_us      = now_us - last_report_us;
        uint64_t win_read_us = total_read_us - last_report_read_us;
        uint64_t win_push_us = total_push_us - last_report_push_us;
        uint64_t win_bytes   = total_tx_bytes - last_report_bytes;
        uint32_t win_kbps;
        if (win_us == 0) { win_us = 1; }   /* avoid divide-by-zero */
        win_kbps = (uint32_t)((win_bytes * 8000ULL) / win_us);   /* bytes*8*1000/us = kbit/s */

        APP_PRINT_INFO7("sdio_tx %u/%u: win %u us = rd %u + push %u + gap %u us, avg %u kbps",
                        sdio_tx_event_cnt, (uint32_t)SDIO_TX_EVENT_CNT,
                        win_us, (uint32_t)win_read_us, (uint32_t)win_push_us,
                        (uint32_t)(win_us - win_read_us - win_push_us), win_kbps);

        last_report_us      = now_us;
        last_report_read_us = total_read_us;
        last_report_push_us = total_push_us;
        last_report_bytes   = total_tx_bytes;
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
    static uint32_t resend_cnt = 0;
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
            /* BD ring momentarily full. The system tick is 10 ms
             * (CONFIG_SYS_CLOCK_TICKS_PER_SEC=100), so os_delay() would sleep a
             * WHOLE 10 ms for a BD the 8720c frees in <1 ms -- that coarse sleep
             * was capping TX at ~10 Mbps (roughly half the packets hit it).
             * Busy-poll in microseconds instead so we re-send the instant a BD
             * frees; keep a safety valve so a truly wedged module still yields. */
            k_busy_wait(30);            /* 30 us */
            if (++resend_cnt >= 6000)   /* ~180 ms wedged: kick wdg then yield */
            {
                APP_PRINT_INFO0("z2plus sdio tx buf full >180ms, yield");
                wdg_kick();
                os_delay(10);
                resend_cnt = 0;
            }
            continue;
        }
    }
}
