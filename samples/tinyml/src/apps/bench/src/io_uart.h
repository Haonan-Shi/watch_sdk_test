/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Zephyr port of the vendor io_uart.h.  The RTK register-level typedefs are gone;
 * the data UART is driven through the Zephyr interrupt-driven UART API against
 * the board's uart2 node (P3_0/P3_1), enabled by the sample overlay.
 */

#ifndef TINYML_EDGE_IO_UART_H
#define TINYML_EDGE_IO_UART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bring up the data UART, register the RX interrupt callback and enable RX. */
void uart_init(void);

/* Blocking send of vCount bytes (polling TX). */
void uart_senddata(uint8_t *pSend_Buf, uint16_t vCount);

/* Copy the bytes accumulated by the RX ISR into caller-supplied storage and
 * reset the accumulator.  Returns the number of bytes copied.  Runs in thread
 * context; briefly locks interrupts to snapshot the shared buffer. */
uint16_t uart_drain_rx(uint8_t *dst, uint16_t dst_size);

#ifdef __cplusplus
}
#endif

#endif /* TINYML_EDGE_IO_UART_H */
