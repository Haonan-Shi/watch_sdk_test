/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_WIFI_UART_H_
#define _APP_WIFI_UART_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include "app_msg.h"

typedef enum
{
    IO_WIFI_UART_AT_CMD_IND,
    IO_WIFI_UART_EXIT_DLPS,
} IO_WIFI_UART_MSG_TYPE;

typedef void (*P_WIFI_UART_RX_PARSER)(void);

bool app_wifi_uart_msg_send(IO_WIFI_UART_MSG_TYPE subtype, void *param_buf);
void app_wifi_uart_msg_handle(T_IO_MSG *io_msg);
void app_wifi_uart_rx_parser_register(P_WIFI_UART_RX_PARSER cb_func);

void app_wifi_uart_init(void);
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
