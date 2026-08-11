/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _UART_ATCMD_H_
#define _UART_ATCMD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "wifi_app.h"

/** AT command types */
typedef enum
{
    ATCMD_ATWS,    /**< WiFi scan command */
    ATCMD_ATW0,    /**< WiFi connect command (open) */
    ATCMD_ATW1,    /**< WiFi disconnect command */
    ATCMD_ATWC,    /**< WiFi connect command */
    ATCMD_ATWT,    /**< WiFi transfer command */
    ATCMD_ATPN,    /**< Ping command */
    ATCMD_ATSL,    /**< Sleep mode command */
    ATCMD_ATWO,    /**< WiFi OTA command */
    ATCMD_ATSD,    /**< SD card command */
    ATCMD_ATST,    /**< WiFi test command */
    ATCMD_ATWD,    /**< WiFi disconnect command (deassociate from AP) */
    ATCMD_ATWINFO, /**< WiFi info query command ("ATW?"): current IP and gateway */
    ATCMD_ATWQ,    /**< Lightweight local-IP query ("ATWQ"): one "[ATWQ] IP => a.b.c.d" line */
    ATCMD_ATPS,    /**< Start transport server ("ATPS=mode,port"): TCP downlink server */
    ATCMD_NUM,
} T_ATCMD_TYPE;

typedef bool (*T_AT_CMD_RSP_APP_CB)(char *p_param);
typedef bool (*T_AT_CMD_FUNC)(T_ATCMD_TYPE cmd, char *p_param);
typedef bool (*T_AT_CMD_RSP)(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);

typedef struct
{
    char *p_cmd;
    T_AT_CMD_FUNC cmd_func;
    char *p_rsp;
    T_AT_CMD_RSP  rsp_func;
} T_AT_CMD_TABLE_ENTRY;

typedef struct
{
    T_ATCMD_TYPE  cur_cmd;
    uint8_t resend_cnt;
    uint8_t rx_buf[100];
    uint16_t rx_cnt;
} T_AT_CMD;

typedef struct t_at_cmd_queue
{
    struct t_at_cmd_queue     *p_next;
    T_ATCMD_TYPE               cmd;
    T_AT_CMD_RSP_APP_CB        cb;
    char  param[0];
} T_AT_CMD_QUEUE;

void uart_atcmd_rsp_handler(void);
void uart_atcmd_flow_ctrl_handler(T_WIFI_MSG atcmd_msg);
bool uart_atcmd_queue_fill(T_ATCMD_TYPE cmd, char *param, T_AT_CMD_RSP_APP_CB app_cb);
void *uart_atcmd_queue_peek(int offset);
void uart_atcmd_queue_flush(uint16_t cnt);
void uart_atcmd_init(void);
void cmd_list_demo(void);
void wifi_atcmd_sleep_mode(void);
void cmd_wifi_scan(T_AT_CMD_RSP_APP_CB cb);
void cmd_wifi_query_info(T_AT_CMD_RSP_APP_CB cb);
/* Lightweight local-IP query ("ATWQ"). The single "[ATWQ] IP => a.b.c.d" reply
 * is delivered to cb via rsp_ATWQ. Preferred over cmd_wifi_query_info (ATW?),
 * which is heavy and can return no data line when the STA is momentarily down. */
void cmd_wifi_query_ip(T_AT_CMD_RSP_APP_CB cb);
/* iperf-path commands. <cb> is invoked with the actual "[ATWT]"/"[ATST]" reply
 * so the caller can confirm the command from the real result (pass NULL to
 * ignore). Both chip replies ("[ATWT] OK", "[ATST] OK <mode>") end with a
 * trailing CRLF, so they take the normal complete-line path in the parser. */
void cmd_wifi_set_server(T_AT_CMD_RSP_APP_CB cb);
void cmd_wifi_upload_file(char *str);
void cmd_wifi_upload_file_stop(void);
void cmd_wifi_set_client(char *str, T_AT_CMD_RSP_APP_CB cb);
void cmd_wifi_iperf_test_enable(T_AT_CMD_RSP_APP_CB cb);
/* SDIO data-path test: enter file-transfer mode + start ATPS TCP server on the
 * device (device is always the server). The uplink SDIO data pump itself lives
 * in the wifi_basic module (wifi_sdio_mem_tx.c). <cb> is invoked with the actual
 * "[ATPS] OK"/"[ATPS] ERROR:n" reply so the caller can set its "server up" flag
 * from the real result (pass NULL to ignore). */
void cmd_wifi_data_test_enter(uint16_t port, T_AT_CMD_RSP_APP_CB cb);
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _UART_ATCMD_H_ */
