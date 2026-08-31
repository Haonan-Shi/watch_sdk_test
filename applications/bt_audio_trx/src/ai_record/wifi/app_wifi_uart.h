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
    /* Posted by the WiFi task after it finishes the blocking module bring-up,
     * so the deferred ATPN connect is actually sent from the main app task
     * (same task context as the proven phone-driven 0x8440 demo), not the
     * WiFi task. Handled via the registered deferred-send callback below. */
    IO_WIFI_UART_SEND_CONNECT,
} IO_WIFI_UART_MSG_TYPE;

typedef void (*P_WIFI_UART_RX_PARSER)(void);
typedef void (*P_WIFI_UART_DEFERRED_CB)(void);

bool app_wifi_uart_msg_send(IO_WIFI_UART_MSG_TYPE subtype, void *param_buf);
void app_wifi_uart_msg_handle(T_IO_MSG *io_msg);
void app_wifi_uart_rx_parser_register(P_WIFI_UART_RX_PARSER cb_func);

/* Register the callback run on the main app task when IO_WIFI_UART_SEND_CONNECT
 * is received. Lets the connect owner (app_ai_record_file_trans) keep its send
 * logic without app_wifi_uart depending on it, mirroring the rx parser hook. */
void app_wifi_uart_deferred_connect_register(P_WIFI_UART_DEFERRED_CB cb_func);

void app_wifi_uart_init(void);
void wifi_uart_console_recv(uint8_t *recv_buf, uint16_t recv_len);

/* "COMMAND READY" boot-banner gate. Reset before powering the module, then
 * poll app_wifi_uart_module_is_ready() to know the module finished booting and
 * the UART link is alive before issuing the first AT command. */
void app_wifi_uart_reset_module_ready(void);
bool app_wifi_uart_module_is_ready(void);

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
