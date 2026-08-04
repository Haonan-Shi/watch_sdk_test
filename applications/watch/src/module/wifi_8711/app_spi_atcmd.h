/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _app_ATCMD_H_
#define _app_ATCMD_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum
{
    ATCMD_WLCONN,
    ATCMD_WLDISCONN,
    ATCMD_RAW,
    ATCMD_SENDRAW,
    ATCMD_NUM
} T_ATCMD_TYPE;

typedef bool (*T_AT_CMD_RSP)(char *p_param);

typedef struct
{
    char *p_cmd;
    T_AT_CMD_RSP  rsp_func;
    uint32_t timeout_ms;
} T_AT_CMD_TABLE_ENTRY;

typedef struct
{
    T_ATCMD_TYPE    cur_cmd;
    uint8_t         resend_cnt;
    uint8_t         rx_buf[1024];
    uint16_t        rx_cnt;
} T_AT_CMD;

typedef struct t_at_cmd_queue
{
    struct t_at_cmd_queue     *p_next;
    T_ATCMD_TYPE               cmd;
    char  param[0];
} T_AT_CMD_QUEUE;

typedef enum
{
    AT_EVT_CMD_RESPONSE,
    AT_EVT_WIFI_CONNECTED,
    AT_EVT_WIFI_GOT_IP,
    AT_EVT_WIFI_DISCONNECTED,
    AT_EVT_UNKNOWN_DATA
} T_AT_EVT_TYPE;

typedef enum
{
    AT_CMD_RSP_STATE_OK,
    AT_CMD_RSP_STATE_ERROR,
} T_AT_CMD_RSP_STATE;

typedef union
{
    uint32_t addr;
    uint8_t  octets[4]; // Sample: [0]=172, [1]=20, [2]=10, [3]=4
} T_AT_IP_ADDR;

typedef void (*app_spi_atcmd_cb_t)(T_AT_EVT_TYPE evt, void *p_data, uint16_t len);

void app_spi_atcmd_register_callback(app_spi_atcmd_cb_t cb);
void app_spi_atcmd_trigger_send_flow(void);
/* Run the queued-atcmd flow-control step. Runs on the dedicated wifi_8711 task,
 * driven by WIFI_8711_EVENT_ATCMD_FLOW_CTRL (see wifi_8711_app.c). */
void spi_atcmd_flow_ctrl_handler(void);
bool app_spi_atcmd_queue_fill(T_ATCMD_TYPE cmd, char *param);
bool app_spi_atcmd_sendraw(const char *cmd_line, const uint8_t *raw, uint16_t raw_len);
void app_spi_atcmd_init(void);

/* Fire a canned AT flow: type == ATCMD_WLCONN queues AT+WLCONN (connect to a
 * built-in test AP); type == ATCMD_WLDISCONN queues AT+WLDISCONN. Used by the
 * `wifi8711 wlconn` / `wldisconn` shell commands (was CMD_AT_CMD_WLCONN). */
void app_spi_atcmd_demo(uint8_t type);

/* --- downlink RX byte tap ----------------------------------------------- *
 * Sum the payload bytes of every valid SPI frame received from the slave,
 * used by the SPI+TCP downlink throughput test to measure how many bytes the
 * master actually received without depending on the slave's recv-line format.
 */
void     app_spi_atcmd_rx_bytes_start(void);  /* reset counter and enable it */
uint32_t app_spi_atcmd_rx_bytes_get(void);    /* bytes counted since start   */
void     app_spi_atcmd_rx_bytes_stop(void);   /* disable the counter         */

/* --- SPI+TCP throughput "blast" mode: dedicated SPI TX task ------------- *
 * Test-mode macro. When set, app_spi_atcmd_init() spawns a dedicated SPI TX
 * task (lower priority than the APP task so the APP task always preempts to
 * drain RX). app_spi_atcmd_sendraw_stream() issues one
 * "AT+SKTSENDRAW=<id>,<total>" header; on the slave's ">>>" the prompt handler
 * posts a message to that task, which then blasts <total> bytes to the slave
 * as back-to-back SPI_XMIT_SIZE frames carrying a repeating 0..127 pattern.
 * Because the long flood runs on its own task, other tasks (e.g. the APP task)
 * keep running while SPI data is continuously transferred.
 */
#ifndef SPI_TCP_TP_TX_TASK
#define SPI_TCP_TP_TX_TASK      1
#endif

#if SPI_TCP_TP_TX_TASK
/* Begin a streaming transparent send of total_bytes over link_id. The payload
 * (a continuous 0..127 ramp) is generated on the fly by the TX task; the send
 * completes when the slave answers "OK". Returns false if a SENDRAW is already
 * pending or the request could not be queued. */
bool app_spi_atcmd_sendraw_stream(uint8_t link_id, uint32_t total_bytes);
#endif /* SPI_TCP_TP_TX_TASK */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _app_ATCMD_H_ */
