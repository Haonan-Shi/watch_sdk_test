/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_UART_H_
#define _WIFI_UART_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "wifi_app.h"
#include <stdio.h>

void wifi_uart_init(void);
void wifi_uart_console_recv(uint8_t *recv_buf, uint16_t recv_len);
bool atcmd_send(const uint8_t *tx_data, int size);
uint16_t atcmd_recv(uint8_t *recv_buf, uint16_t recv_len);
int mp_open(void);
int mp_close(void);
void mp_send(uint8_t *cmd_buf, uint16_t cmd_len);
uint16_t mp_recv(uint8_t *recv_buf, uint16_t recv_len);
int mp_set_baudrate(uint32_t baud);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIFI_UART_H_ */
