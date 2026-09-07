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

typedef struct
{
    uint16_t event;
    uint16_t subtype;
    union
    {
        uint32_t  param;
        void     *buf;
    } u;
} T_WIFI_MSG;

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
bool app_spi_atcmd_queue_fill(T_ATCMD_TYPE cmd, char *param);
void app_spi_atcmd_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _app_ATCMD_H_ */
