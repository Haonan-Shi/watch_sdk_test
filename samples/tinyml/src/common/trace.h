/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Zephyr shim for the vendor SDK trace.h.
 * The vendor sources log through DBG_DIRECT()/APP_PRINT_*() which emit over the
 * Realtek trace UART.  On Zephyr we redirect everything to printk() (routed to
 * the RTK log backend), and auto-append the newline that DBG_DIRECT() implies.
 */

#ifndef TINYML_EDGE_TRACE_SHIM_H
#define TINYML_EDGE_TRACE_SHIM_H

#include <zephyr/sys/printk.h>

#define DBG_DIRECT(fmt, ...)   do { printk(fmt "\n", ##__VA_ARGS__); } while (0)

#define APP_PRINT_INFO0(fmt)                    DBG_DIRECT(fmt)
#define APP_PRINT_INFO1(fmt, a0)                DBG_DIRECT(fmt, a0)
#define APP_PRINT_INFO2(fmt, a0, a1)            DBG_DIRECT(fmt, a0, a1)
#define APP_PRINT_INFO3(fmt, a0, a1, a2)        DBG_DIRECT(fmt, a0, a1, a2)
#define APP_PRINT_ERROR0(fmt)                   DBG_DIRECT(fmt)
#define APP_PRINT_ERROR1(fmt, a0)               DBG_DIRECT(fmt, a0)

/* Only referenced by ts_queue_printf() (a debug helper that is never called on
 * the hot path).  Map it to the raw pointer so the file still compiles. */
#define TRACE_BINARY(len, pdata)                (pdata)

#endif /* TINYML_EDGE_TRACE_SHIM_H */
