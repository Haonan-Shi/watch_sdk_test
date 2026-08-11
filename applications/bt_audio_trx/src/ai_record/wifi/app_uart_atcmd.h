/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_UART_ATCMD_H_
#define _APP_UART_ATCMD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief AT Command Types
 */
#if (AT_CMD_VER == 1)
typedef enum
{
    ATCMD_WLCONN,       // WiFi Connect (AT+WLCONN=)
    ATCMD_WLDISCONN,    // WiFi Disconnect (AT+WLDISCONN)
    ATCMD_NUM
} T_ATCMD_TYPE;
#else  // AT_CMD_VER == 2
typedef enum
{
    ATCMD_ATPN,         // Set Profile (ATPN=)
    ATCMD_ATWD,         // WiFi Disconnect (ATWD)
    ATCMD_NUM
} T_ATCMD_TYPE;
#endif

/**
 * @brief AT Event Types
 */
typedef enum
{
    AT_EVT_CMD_RESPONSE,        // Command response (OK/ERROR)
    AT_EVT_WIFI_CONNECTED,      // WiFi connected
    AT_EVT_WIFI_DISCONNECTED,   // WiFi disconnected
    AT_EVT_WIFI_GOT_IP,         // Got IP address
    AT_EVT_UNKNOWN_DATA,        // Unknown/unsolicited data
} T_AT_EVT_TYPE;

/**
 * @brief AT Command Response State
 */
typedef enum
{
    AT_CMD_RSP_STATE_OK,
    AT_CMD_RSP_STATE_ERROR,
} T_AT_CMD_RSP_STATE;

/**
 * @brief IP Address Structure
 */
typedef union
{
    uint32_t addr;
    uint8_t  octets[4];  // [0]=192, [1]=168, [2]=1, [3]=100
} T_AT_IP_ADDR;

/**
 * @brief AT Command Response Callback
 * @param p_param Response string
 * @return true if handled, false otherwise
 */
typedef bool (*T_AT_CMD_RSP)(char *p_param);

/**
 * @brief AT Command Version
 * 1: SPI-like (no p_rsp, match by OK/ERROR)
 * 2: UART-like (with p_rsp, match by response prefix)
 */
#ifndef AT_CMD_VER
#define AT_CMD_VER  2
#endif

/**
 * @brief AT Command Table Entry
 */
#if (AT_CMD_VER == 1)
typedef struct
{
    const char      *p_cmd;         // Command string (e.g., "AT+WLCONN=")
    T_AT_CMD_RSP     rsp_func;      // Response handler function
    uint32_t         timeout_ms;    // Timeout in milliseconds
} T_AT_CMD_TABLE_ENTRY;
#else  // AT_CMD_VER == 2
typedef struct
{
    const char      *p_cmd;         // Command string (e.g., "ATPN=")
    const char      *p_rsp;         // Response prefix (e.g., "[ATPN]")
    T_AT_CMD_RSP     rsp_func;      // Response handler function
    uint32_t         timeout_ms;    // Timeout in milliseconds
} T_AT_CMD_TABLE_ENTRY;
#endif

/**
 * @brief AT Command State
 */
typedef struct
{
    T_ATCMD_TYPE    cur_cmd;        // Current command being executed
    uint8_t         resend_cnt;     // Resend counter
    uint8_t         rx_buf[512];    // RX buffer (increased from 100)
    uint16_t        rx_cnt;         // RX buffer count
} T_AT_CMD;

/**
 * @brief AT Command Queue Entry
 */
typedef struct t_at_cmd_queue
{
    struct t_at_cmd_queue  *p_next;
    T_ATCMD_TYPE            cmd;
    char                    param[0];  // Variable length parameter
} T_AT_CMD_QUEUE;

/**
 * @brief AT Event Callback
 * @param evt Event type
 * @param p_data Event data (can be NULL)
 * @param len Data length
 */
typedef void (*app_uart_atcmd_cb_t)(T_AT_EVT_TYPE evt, void *p_data, uint16_t len);

/**
 * @brief Initialize UART AT Command Module
 */
void app_uart_atcmd_init(void);

/**
 * @brief Register event callback
 * @param cb Callback function
 */
void app_uart_atcmd_register_callback(app_uart_atcmd_cb_t cb);

/**
 * @brief Add command to queue
 * @param cmd Command type
 * @param param Command parameter (can be NULL)
 * @return true if success, false if failed
 */
bool app_uart_atcmd_queue_fill(T_ATCMD_TYPE cmd, const char *param);

/**
 * @brief Trigger command send flow
 * Should be called after adding commands to queue
 */
void app_uart_atcmd_trigger_send_flow(void);

/**
 * @brief UART RX data handler
 * Should be called when UART data is received
 */
void app_uart_atcmd_rsp_handler(void);

/**
 * @brief Demo function
 */
void app_uart_atcmd_demo(uint8_t type);

#ifdef __cplusplus
}
#endif

#endif /* _APP_UART_ATCMD_H_ */
