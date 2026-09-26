/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Zephyr shim for the vendor SDK app_msg.h.  Only the subset used by the tinyml
 * edge sources is provided: the inter-task message envelope (T_IO_MSG) and the
 * UART message subtypes.  On Zephyr these envelopes travel through a k_msgq
 * (see main.c).
 */

#ifndef TINYML_EDGE_APP_MSG_SHIM_H
#define TINYML_EDGE_APP_MSG_SHIM_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Message top-level types */
#define IO_MSG_TYPE_UART                    0x01

/* UART message subtypes */
#define IO_MSG_UART_RX_DONE                 0x10
#define IO_MSG_UART_DATA_PARSER_SUCCESS     0x11
#define IO_MSG_UART_DATA_PARSER_FAILED      0x12

typedef struct
{
    uint16_t type;      /* IO_MSG_TYPE_* */
    uint16_t subtype;   /* IO_MSG_UART_* */
    uint16_t len;       /* payload length (RX_DONE): bytes at u.buf */
    union
    {
        uint32_t param;
        void    *buf;
    } u;
} T_IO_MSG;

#ifdef __cplusplus
}
#endif

#endif /* TINYML_EDGE_APP_MSG_SHIM_H */
