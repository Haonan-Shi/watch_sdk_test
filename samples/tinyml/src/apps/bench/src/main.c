/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * TinyML "edge" sample - Zephyr port.
 *
 * This is a host-driven test harness (not a standalone embedded demo): the
 * model and input data are streamed in over the data UART using the framed
 * protocol implemented in uart_packet_parser.c, and inference results / RAM
 * usage / operator lists / profiler traces are streamed back to the PC tool.
 *
 * The vendor SDK entry point (os_task_create / os_msg_queue_create /
 * os_timer_create) is replaced here with a dedicated Zephyr thread, a k_msgq
 * for inter-context messaging, and a k_timer + k_work backing the parser's
 * RX inactivity timeout.  See main.c comments below for the mapping.
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <string.h>

#include "trace.h"
#include "app_msg.h"
#include "tinyml_main.h"
#include "io_uart.h"
#include "ts_queue.h"
#include "ts_mem.h"
#include "uart_packet_parser.h"
#include "ts_realtek.h"
#include "tinyml_psram.h"

/*============================================================================*
 *                              Configuration
 *============================================================================*/
#define EDGE_TASK_PRIORITY        5
#define EDGE_TASK_STACK_SIZE      (20 * 1024)
#define EDGE_MSGQ_MAX_MSGS        0x30            /* mirrors MAX_NUMBER_OF_IO_MESSAGE */
#define EDGE_RX_TIMEOUT_MS        8000            /* mirrors vendor os_timer 8000ms */

/*============================================================================*
 *                              Kernel objects
 *============================================================================*/
K_THREAD_STACK_DEFINE(edge_task_stack, EDGE_TASK_STACK_SIZE);
static struct k_thread edge_task_data;

K_MSGQ_DEFINE(edge_msgq, sizeof(T_IO_MSG), EDGE_MSGQ_MAX_MSGS, 4);

/* RX byte queue shared with TinyML_main_task drain loop (was defined in the
 * vendor io_uart.c; on Zephyr the worker thread owns it). */
ts_queue_t *g_ts_uart_data_list = NULL;

/*============================================================================*
 *                       ts_timer (RX inactivity timeout)
 *============================================================================*/
/* The parser calls ts_timer_start(uart_timeout_cb) on every received byte to
 * (re)arm a single-shot timeout that frees partial model/data buffers if the
 * host stalls mid-transfer.  uart_timeout_cb() calls ts_free(), which is not
 * ISR-safe, so the k_timer expiry is deferred to the system workqueue. */
static struct k_timer  ts_timeout_timer;
static struct k_work   ts_timeout_work;
static pfunc           ts_timer_timeout_func;

static void ts_timeout_work_handler(struct k_work *work)
{
    ARG_UNUSED(work);

    pfunc func = ts_timer_timeout_func;
    ts_timer_timeout_func = NULL;
    if (func != NULL)
    {
        func(NULL);
    }
}

static void ts_timeout_timer_expiry(struct k_timer *timer)
{
    ARG_UNUSED(timer);
    k_work_submit(&ts_timeout_work);
}

void ts_timer_start(void (func)(void *xTimer))
{
    ts_timer_timeout_func = func;
    k_timer_start(&ts_timeout_timer, K_MSEC(EDGE_RX_TIMEOUT_MS), K_NO_WAIT);
}

/*============================================================================*
 *                          Inter-task messaging
 *============================================================================*/
bool tinyml_send_msg_to_task(T_IO_MSG *p_msg)
{
    /* Called from thread, ISR (UART) and timer context - K_NO_WAIT is safe. */
    if (k_msgq_put(&edge_msgq, p_msg, K_NO_WAIT) != 0)
    {
        DBG_DIRECT("tinyml_send_msg_to_task fail");
        return false;
    }
    return true;
}

/*============================================================================*
 *                          ts_realtek log sink
 *============================================================================*/
static void ts_log_cb(const char *msg)
{
    printk("%s", msg);
}

/*============================================================================*
 *                              Worker thread
 *============================================================================*/
static void edge_main_task(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    T_IO_MSG event;
    static uint8_t rx_chunk[2048];

    /* Route the inference engine's internal logging to printk. */
    ts_realtek_register_log(ts_log_cb);

    uart_init();

    DBG_DIRECT("tinyml edge task started (waiting for host over uart2)");

    while (true)
    {
        if (k_msgq_get(&edge_msgq, &event, K_FOREVER) != 0)
        {
            continue;
        }

        if (event.subtype == IO_MSG_UART_RX_DONE)
        {
            /* Snapshot the bytes accumulated by the RX ISR and enqueue them.
             * The framed parser is stateful across chunks, so frames split
             * over multiple idle windows still reassemble correctly. */
            uint16_t n = uart_drain_rx(rx_chunk, sizeof(rx_chunk));
            if (n > 0)
            {
                g_ts_uart_data_list = ts_queue_add_data(g_ts_uart_data_list, rx_chunk, n);
            }

            ts_queue_t *p_last_node = ts_queue_indexof_last(g_ts_uart_data_list);
            while (p_last_node != NULL)
            {
                uart_rx_data_parse(p_last_node->p_data, p_last_node->data_length);
                g_ts_uart_data_list = ts_queue_remove_last_node(g_ts_uart_data_list);
                p_last_node = ts_queue_indexof_last(g_ts_uart_data_list);
            }
        }
        else if (event.subtype == IO_MSG_UART_DATA_PARSER_SUCCESS)
        {
            DBG_DIRECT("IO_MSG_UART_DATA_PARSER_SUCCESS");
        }
        else if (event.subtype == IO_MSG_UART_DATA_PARSER_FAILED)
        {
            DBG_DIRECT("IO_MSG_UART_DATA_PARSER_FAILED, retry");
        }
    }
}

int main(void)
{
    /* Bring up external PSRAM before any ts_malloc (see src/common). */
    tinyml_psram_init();

    k_timer_init(&ts_timeout_timer, ts_timeout_timer_expiry, NULL);
    k_work_init(&ts_timeout_work, ts_timeout_work_handler);

    k_thread_create(&edge_task_data, edge_task_stack,
                    K_THREAD_STACK_SIZEOF(edge_task_stack),
                    edge_main_task, NULL, NULL, NULL,
                    EDGE_TASK_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&edge_task_data, "edge");

    return 0;
}
