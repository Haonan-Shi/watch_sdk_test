/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_WIFI_UART_CMD

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "trace.h"
#include "app_uart_atcmd.h"
#include "app_wifi_uart.h"
#include "os_mem.h"
#include "os_queue.h"
#include "app_timer.h"

// Configuration
#define MIN_RSP_LEN             2
#define MAX_RSP_LEN             512
#define ATCMD_RSP_OK            "OK"
#define ATCMD_RSP_ERROR         "ERROR"
#define ATCMD_RESEND_CNT        0       // 0 = no resend

// External UART interface (from wifi_uart.h)
extern bool atcmd_send(const uint8_t *tx_data, int size);
extern uint16_t atcmd_recv(uint8_t *recv_buf, uint16_t recv_len);

// Timer event
typedef enum
{
    UART_ATCMD_RESEND_TIMER = 0x00,
} T_UART_ATCMD_TIMER;

// Global variables
static T_AT_CMD             at_cmd_t = {.cur_cmd = ATCMD_NUM, .resend_cnt = 0, .rx_cnt = 0};
static T_OS_QUEUE           at_cmd_queue;
static uint8_t              atcmd_timer_id = 0;
static uint8_t              timer_handle_atcmd_resend = 0;
static app_uart_atcmd_cb_t  g_at_callback = NULL;

// Forward declarations
static bool cmd_send(T_ATCMD_TYPE cmd, const char *cmd_str);
static void atcmd_parser_process(void);
static void notify_at_evt(T_AT_EVT_TYPE evt, uint8_t *p_data, uint16_t len);
static void process_unsolicited_msg(char *p_param, uint16_t len);
static void *uart_atcmd_queue_peek(int offset);
static void uart_atcmd_queue_flush(uint16_t cnt);
static void uart_atcmd_timeout_cb(uint8_t timer_evt, uint16_t param);
static void uart_atcmd_flow_ctrl_handler(void);

// Response handlers
static bool rsp_WLCONN(char *p_param);
static bool rsp_WLDISCONN(char *p_param);

/**
 * @brief AT Command Table
 * AT_CMD_VER 1: {command, response_handler, timeout_ms}
 * AT_CMD_VER 2: {command, response_prefix, response_handler, timeout_ms}
 */
#ifndef AT_CMD_VER
#define AT_CMD_VER        2
#endif

#if (AT_CMD_VER == 1)
// SPI-like: No p_rsp, match by OK/ERROR
const T_AT_CMD_TABLE_ENTRY at_cmd_table[ATCMD_NUM] =
{
    {"AT+WLCONN=",      rsp_WLCONN,     20000},
    {"AT+WLDISCONN",    rsp_WLDISCONN,  10000},
};
#else  // AT_CMD_VER == 2
// UART-like: With p_rsp, match by response prefix
const T_AT_CMD_TABLE_ENTRY at_cmd_table[ATCMD_NUM] =
{
    {"ATPN=",   "[ATPN]",   rsp_WLCONN,     20000},
    {"ATWD",    "[ATWD]",   rsp_WLDISCONN,  10000},
};
#endif

/*****************************************************************************
 * Queue Management
 *****************************************************************************/

static void *uart_atcmd_queue_peek(int offset)
{
    void *cmd_queue_pkt = os_queue_peek(&at_cmd_queue, offset);
    return cmd_queue_pkt;
}

static void uart_atcmd_queue_flush(uint16_t cnt)
{
    T_AT_CMD_QUEUE *cmd_queue_pkt;
    APP_PRINT_TRACE1("[uart_atcmd] queue flush: %d", cnt);

    if (cnt > at_cmd_queue.count)
    {
        cnt = at_cmd_queue.count;
    }

    for (uint16_t i = 0; i < cnt; i++)
    {
        cmd_queue_pkt = os_queue_out(&at_cmd_queue);
        if (cmd_queue_pkt)
        {
            os_mem_free(cmd_queue_pkt);
        }
    }
}

bool app_uart_atcmd_queue_fill(T_ATCMD_TYPE cmd, const char *param)
{
    if (cmd >= ATCMD_NUM)
    {
        APP_PRINT_ERROR1("[uart_atcmd] invalid cmd: %d", cmd);
        return false;
    }

    T_AT_CMD_QUEUE *cmd_queue_pkt;
    uint16_t param_len = 1;  // For null terminator

    if (param != NULL)
    {
        param_len += strlen(param);
    }

    cmd_queue_pkt = (T_AT_CMD_QUEUE *)os_mem_alloc(OS_MEM_TYPE_DATA,
                                                   sizeof(T_AT_CMD_QUEUE) + param_len);
    if (cmd_queue_pkt == NULL)
    {
        APP_PRINT_ERROR0("[uart_atcmd] queue fill: malloc failed");
        return false;
    }

    cmd_queue_pkt->cmd = cmd;
    memset(cmd_queue_pkt->param, 0, param_len);

    if (param != NULL && param_len > 1)
    {
        memcpy(cmd_queue_pkt->param, param, param_len - 1);
    }

    os_queue_in(&at_cmd_queue, cmd_queue_pkt);
    APP_PRINT_TRACE1("[uart_atcmd] queue fill: cmd %d", cmd);

    return true;
}

/*****************************************************************************
 * Command Send
 *****************************************************************************/

static bool cmd_send(T_ATCMD_TYPE cmd, const char *cmd_str)
{
    if (cmd >= ATCMD_NUM)
    {
        APP_PRINT_ERROR1("[uart_atcmd] cmd_send: invalid cmd %d", cmd);
        return false;
    }

    uint16_t cmd_len = strlen(at_cmd_table[cmd].p_cmd);
    uint16_t param_len = (cmd_str != NULL) ? strlen(cmd_str) : 0;
    uint16_t total_len = cmd_len + param_len;

    uint8_t *send_buf = (uint8_t *)os_mem_alloc(OS_MEM_TYPE_DATA, total_len);
    if (send_buf == NULL)
    {
        APP_PRINT_ERROR0("[uart_atcmd] cmd_send: malloc failed");
        return false;
    }

    // Build command: CMD + PARAM
    memcpy(send_buf, at_cmd_table[cmd].p_cmd, cmd_len);
    if (param_len > 0)
    {
        memcpy(send_buf + cmd_len, cmd_str, param_len);
    }

    APP_PRINT_INFO2("[uart_atcmd] TX: %s%s",
                    TRACE_STRING(at_cmd_table[cmd].p_cmd),
                    TRACE_STRING(cmd_str ? cmd_str : ""));

    bool ret = atcmd_send(send_buf, total_len);
    os_mem_free(send_buf);

    return ret;
}

/*****************************************************************************
 * Response Parser
 *****************************************************************************/

/**
 * @brief Parse received AT response
 * This is the optimized O(n) parser from SPI version
 */
static void atcmd_parser_process(void)
{
    APP_PRINT_INFO1("[uart_atcmd] atcmd_parser_process: rx_cnt=%d", at_cmd_t.rx_cnt);
    if (at_cmd_t.rx_cnt == 0)
    {
        return;
    }

    uint16_t processed_len = 0;
    uint16_t start_idx = 0;
    uint16_t i;

    // Single pass through buffer - O(n) complexity
    for (i = 0; i < at_cmd_t.rx_cnt; i++)
    {
        // Skip leading \r and \n at the start of a new line
        while (start_idx < at_cmd_t.rx_cnt &&
               (at_cmd_t.rx_buf[start_idx] == '\r' || at_cmd_t.rx_buf[start_idx] == '\n'))
        {
            start_idx++;
            i = start_idx;
        }

        // Check if we've consumed all data
        if (start_idx >= at_cmd_t.rx_cnt)
        {
            processed_len = at_cmd_t.rx_cnt;
            break;
        }

        // Find line ending
        if (at_cmd_t.rx_buf[i] == '\n')
        {
            uint16_t end_idx = i;

            // Remove \r if present
            if (end_idx > 0 && at_cmd_t.rx_buf[end_idx - 1] == '\r')
            {
                end_idx--;
            }

            uint16_t line_len = end_idx - start_idx;

            // Extract and process line
            if (line_len > 0)
            {
                // Allocate buffer for line (+1 for null terminator)
                uint8_t *p_line = (uint8_t *)os_mem_alloc(OS_MEM_TYPE_DATA, line_len + 1);
                if (p_line)
                {
                    memcpy(p_line, &at_cmd_t.rx_buf[start_idx], line_len);
                    p_line[line_len] = '\0';

                    APP_PRINT_INFO1("[uart_atcmd] RX: %s", TRACE_STRING(p_line));

                    // Process this line
                    bool is_command_finished = false;

                    // Check for OK/ERROR
                    if (strstr((char *)p_line, ATCMD_RSP_OK) ||
                        strstr((char *)p_line, ATCMD_RSP_ERROR))
                    {
                        is_command_finished = true;
                    }

#if (AT_CMD_VER == 1)
                    // AT_CMD_VER 1: Direct call to current command's rsp_func
                    if (at_cmd_t.cur_cmd < ATCMD_NUM)
                    {
                        // Call response handler for current command
                        if (at_cmd_table[at_cmd_t.cur_cmd].rsp_func)
                        {
                            at_cmd_table[at_cmd_t.cur_cmd].rsp_func((char *)p_line);
                        }
                    }
                    else
                    {
                        // No command pending - unsolicited message
                        process_unsolicited_msg((char *)p_line, line_len);
                    }
#else  // AT_CMD_VER == 2
                    // AT_CMD_VER 2: Match by p_rsp prefix
                    bool handled = false;
                    for (uint16_t cmd_idx = 0; cmd_idx < ATCMD_NUM; cmd_idx++)
                    {
                        const char *p_rsp = at_cmd_table[cmd_idx].p_rsp;
                        if (p_rsp && strstr((char *)p_line, p_rsp))
                        {
                            // Found matching response prefix
                            if (at_cmd_t.cur_cmd == cmd_idx)
                            {
                                // Expected response for current command
                                if (at_cmd_table[cmd_idx].rsp_func)
                                {
                                    at_cmd_table[cmd_idx].rsp_func((char *)p_line);
                                }
                                handled = true;
                                break;
                            }
                            else if (at_cmd_t.cur_cmd < ATCMD_NUM)
                            {
                                // Response for different command (unsolicited)
                                APP_PRINT_WARN2("[uart_atcmd] Unexpected rsp: expect cmd %d, got cmd %d",
                                                at_cmd_t.cur_cmd, cmd_idx);
                                if (at_cmd_table[cmd_idx].rsp_func)
                                {
                                    at_cmd_table[cmd_idx].rsp_func((char *)p_line);
                                }
                                handled = true;
                                // Don't break - continue waiting for expected response
                            }
                            else
                            {
                                // No command pending, but matched a response prefix
                                if (at_cmd_table[cmd_idx].rsp_func)
                                {
                                    at_cmd_table[cmd_idx].rsp_func((char *)p_line);
                                }
                                handled = true;
                                break;
                            }
                        }
                    }

                    if (!handled)
                    {
                        // No matching p_rsp found - unsolicited message
                        process_unsolicited_msg((char *)p_line, line_len);
                    }
#endif

                    // If command finished, trigger next command
                    if (is_command_finished && at_cmd_t.cur_cmd < ATCMD_NUM)
                    {
                        APP_PRINT_INFO0("[uart_atcmd] CMD finished, trigger next");

                        app_stop_timer(&timer_handle_atcmd_resend);
                        uart_atcmd_queue_flush(1);

                        at_cmd_t.cur_cmd = ATCMD_NUM;
                        at_cmd_t.resend_cnt = 0;

                        app_uart_atcmd_trigger_send_flow();
                    }

                    os_mem_free(p_line);
                }
            }

            // Update processed length (including \r\n)
            processed_len = i + 1;
            start_idx = processed_len;
        }
    }

    // Move unprocessed data to buffer start
    if (processed_len > 0)
    {
        uint16_t remain_len = at_cmd_t.rx_cnt - processed_len;
        if (remain_len > 0)
        {
            memmove(at_cmd_t.rx_buf, &at_cmd_t.rx_buf[processed_len], remain_len);
        }
        at_cmd_t.rx_cnt = remain_len;
    }

    // Handle incomplete line (no \n at the end)
    // If we have data remaining and it looks like a complete response, process it
    if (at_cmd_t.rx_cnt > 0 && start_idx < at_cmd_t.rx_cnt)
    {
        // Check if remaining data contains OK or ERROR (indicating complete response)
        at_cmd_t.rx_buf[at_cmd_t.rx_cnt] = '\0';  // Temporary null terminator
        if (strstr((char *)&at_cmd_t.rx_buf[start_idx], ATCMD_RSP_OK) ||
            strstr((char *)&at_cmd_t.rx_buf[start_idx], ATCMD_RSP_ERROR))
        {
            uint16_t line_len = at_cmd_t.rx_cnt - start_idx;

            // Allocate buffer for line (+1 for null terminator)
            uint8_t *p_line = (uint8_t *)os_mem_alloc(OS_MEM_TYPE_DATA, line_len + 1);
            if (p_line)
            {
                memcpy(p_line, &at_cmd_t.rx_buf[start_idx], line_len);
                p_line[line_len] = '\0';

                APP_PRINT_INFO1("[uart_atcmd] RX (no newline): %s", TRACE_STRING(p_line));

                // Process this line
                bool is_command_finished = true;  // OK/ERROR means finished

#if (AT_CMD_VER == 1)
                // AT_CMD_VER 1: Direct call to current command's rsp_func
                if (at_cmd_t.cur_cmd < ATCMD_NUM)
                {
                    // Call response handler for current command
                    if (at_cmd_table[at_cmd_t.cur_cmd].rsp_func)
                    {
                        at_cmd_table[at_cmd_t.cur_cmd].rsp_func((char *)p_line);
                    }

                    // Command finished, trigger next command
                    if (is_command_finished)
                    {
                        APP_PRINT_INFO0("[uart_atcmd] CMD finished (no newline), trigger next");

                        app_stop_timer(&timer_handle_atcmd_resend);
                        uart_atcmd_queue_flush(1);

                        at_cmd_t.cur_cmd = ATCMD_NUM;
                        at_cmd_t.resend_cnt = 0;

                        app_uart_atcmd_trigger_send_flow();
                    }
                }
                else
                {
                    // No command pending - unsolicited message
                    process_unsolicited_msg((char *)p_line, line_len);
                }
#else  // AT_CMD_VER == 2
                // AT_CMD_VER 2: Match by p_rsp prefix
                bool handled = false;
                for (uint16_t cmd_idx = 0; cmd_idx < ATCMD_NUM; cmd_idx++)
                {
                    const char *p_rsp = at_cmd_table[cmd_idx].p_rsp;
                    if (p_rsp && strstr((char *)p_line, p_rsp))
                    {
                        // Found matching response prefix
                        if (at_cmd_t.cur_cmd == cmd_idx)
                        {
                            // Expected response for current command
                            if (at_cmd_table[cmd_idx].rsp_func)
                            {
                                at_cmd_table[cmd_idx].rsp_func((char *)p_line);
                            }

                            // Command finished, trigger next command
                            if (is_command_finished)
                            {
                                APP_PRINT_INFO0("[uart_atcmd] CMD finished (no newline), trigger next");

                                app_stop_timer(&timer_handle_atcmd_resend);
                                uart_atcmd_queue_flush(1);

                                at_cmd_t.cur_cmd = ATCMD_NUM;
                                at_cmd_t.resend_cnt = 0;

                                app_uart_atcmd_trigger_send_flow();
                            }

                            handled = true;
                            break;
                        }
                        else
                        {
                            // Response for different command or no command pending
                            if (at_cmd_table[cmd_idx].rsp_func)
                            {
                                at_cmd_table[cmd_idx].rsp_func((char *)p_line);
                            }
                            handled = true;
                            break;
                        }
                    }
                }

                if (!handled)
                {
                    // No matching p_rsp found - unsolicited message
                    process_unsolicited_msg((char *)p_line, line_len);
                }
#endif

                os_mem_free(p_line);
            }

            // Clear buffer
            at_cmd_t.rx_cnt = 0;
        }
    }

    // Overflow protection
    if (at_cmd_t.rx_cnt >= sizeof(at_cmd_t.rx_buf) - 1)  // -1 for null terminator
    {
        APP_PRINT_WARN0("[uart_atcmd] RX buffer full without newline, clear!");
        at_cmd_t.rx_cnt = 0;
    }
}

/**
 * @brief UART RX handler - called when UART data received
 */
void app_uart_atcmd_rsp_handler(void)
{
    // Read available data from UART
    uint16_t free_space = sizeof(at_cmd_t.rx_buf) - at_cmd_t.rx_cnt;
    if (free_space == 0)
    {
        APP_PRINT_ERROR0("[uart_atcmd] RX buffer full!");
        at_cmd_t.rx_cnt = 0;  // Reset buffer
        free_space = sizeof(at_cmd_t.rx_buf);
    }

    uint16_t recv_len = atcmd_recv(at_cmd_t.rx_buf + at_cmd_t.rx_cnt, free_space);
    if (recv_len > 0)
    {
        at_cmd_t.rx_cnt += recv_len;
        // Process received data
        atcmd_parser_process();
    }
}

/*****************************************************************************
 * Flow Control
 *****************************************************************************/

static void uart_atcmd_flow_ctrl_handler(void)
{
    // Check if busy
    if (at_cmd_t.cur_cmd != ATCMD_NUM)
    {
        APP_PRINT_INFO1("[uart_atcmd] busy, cur cmd=0x%x", at_cmd_t.cur_cmd);
        return;
    }

    // Get next command from queue
    T_AT_CMD_QUEUE *cmd_queue_pkt = (T_AT_CMD_QUEUE *)uart_atcmd_queue_peek(0);
    if (cmd_queue_pkt)
    {
        if (cmd_queue_pkt->cmd >= ATCMD_NUM)
        {
            APP_PRINT_ERROR1("[uart_atcmd] invalid cmd: %d", cmd_queue_pkt->cmd);
            uart_atcmd_queue_flush(1);
            return;
        }

        APP_PRINT_INFO1("[uart_atcmd] sending cmd: %d", cmd_queue_pkt->cmd);

        // Send command
        cmd_send(cmd_queue_pkt->cmd, cmd_queue_pkt->param);

        // Update state
        at_cmd_t.cur_cmd = cmd_queue_pkt->cmd;
        at_cmd_t.resend_cnt = 0;

        // Start timeout timer
        app_start_timer(&timer_handle_atcmd_resend,
                        "uart_atcmd_timer",
                        atcmd_timer_id,
                        UART_ATCMD_RESEND_TIMER,
                        0,
                        false,
                        at_cmd_table[at_cmd_t.cur_cmd].timeout_ms);
    }
}

void app_uart_atcmd_trigger_send_flow(void)
{
    uart_atcmd_flow_ctrl_handler();
}

/*****************************************************************************
 * Timer Callback
 *****************************************************************************/

static void uart_atcmd_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("[uart_atcmd] timeout: timer_id %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case UART_ATCMD_RESEND_TIMER:
        {
            app_stop_timer(&timer_handle_atcmd_resend);

            if (at_cmd_t.resend_cnt >= ATCMD_RESEND_CNT)
            {
                // Timeout - clear current command
                APP_PRINT_ERROR1("[uart_atcmd] timeout: cmd %d", at_cmd_t.cur_cmd);

                at_cmd_t.cur_cmd = ATCMD_NUM;
                at_cmd_t.resend_cnt = 0;
                at_cmd_t.rx_cnt = 0;
                uart_atcmd_queue_flush(1);

                // Notify timeout
                uint8_t state = AT_CMD_RSP_STATE_ERROR;
                notify_at_evt(AT_EVT_CMD_RESPONSE, &state, sizeof(state));
            }
            else
            {
                // Resend (if enabled)
                at_cmd_t.resend_cnt++;
                APP_PRINT_INFO1("[uart_atcmd] resend: cnt %d", at_cmd_t.resend_cnt);
            }

            // Trigger next command
            app_uart_atcmd_trigger_send_flow();
        }
        break;

    default:
        break;
    }
}

/*****************************************************************************
 * Event Notification
 *****************************************************************************/

static void notify_at_evt(T_AT_EVT_TYPE evt, uint8_t *p_data, uint16_t len)
{
    if (g_at_callback != NULL)
    {
        g_at_callback(evt, p_data, len);
    }
}

void app_uart_atcmd_register_callback(app_uart_atcmd_cb_t cb)
{
    g_at_callback = cb;
}

/*****************************************************************************
 * Unsolicited Message Handler
 *****************************************************************************/

static void process_unsolicited_msg(char *p_param, uint16_t len)
{
    // Check for known unsolicited messages
    if (strstr(p_param, "wifi connected"))
    {
        notify_at_evt(AT_EVT_WIFI_CONNECTED, NULL, 0);
    }
    else if (strstr(p_param, "wifi disconnected"))
    {
        notify_at_evt(AT_EVT_WIFI_DISCONNECTED, NULL, 0);
    }
    else if (strstr(p_param, "wifi got ip"))
    {
        // Parse IP address if needed
        notify_at_evt(AT_EVT_WIFI_GOT_IP, NULL, 0);
    }
    else
    {
        // Unknown data
        notify_at_evt(AT_EVT_UNKNOWN_DATA, (uint8_t *)p_param, len);
    }
}

/*****************************************************************************
 * Response Handlers
 *****************************************************************************/

static bool rsp_WLCONN(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_WLCONN: %s", TRACE_STRING(p_param));

    if (strstr(p_param, ATCMD_RSP_OK))
    {
        uint8_t state = AT_CMD_RSP_STATE_OK;
        notify_at_evt(AT_EVT_CMD_RESPONSE, &state, sizeof(state));
    }
    else if (strstr(p_param, ATCMD_RSP_ERROR))
    {
        uint8_t state = AT_CMD_RSP_STATE_ERROR;
        notify_at_evt(AT_EVT_CMD_RESPONSE, &state, sizeof(state));
    }
    else if (strstr(p_param, "wifi connected"))
    {
        notify_at_evt(AT_EVT_WIFI_CONNECTED, NULL, 0);
    }
    else if (strstr(p_param, "wifi got ip"))
    {
        // TODO: Parse IP address if needed
        notify_at_evt(AT_EVT_WIFI_GOT_IP, NULL, 0);
    }

    return true;
}

static bool rsp_WLDISCONN(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_WLDISCONN: %s", TRACE_STRING(p_param));

    if (strstr(p_param, ATCMD_RSP_OK))
    {
        uint8_t state = AT_CMD_RSP_STATE_OK;
        notify_at_evt(AT_EVT_CMD_RESPONSE, &state, sizeof(state));
    }
    else if (strstr(p_param, ATCMD_RSP_ERROR))
    {
        uint8_t state = AT_CMD_RSP_STATE_ERROR;
        notify_at_evt(AT_EVT_CMD_RESPONSE, &state, sizeof(state));
    }
    else if (strstr(p_param, "wifi disconnected"))
    {
        notify_at_evt(AT_EVT_WIFI_DISCONNECTED, NULL, 0);
    }

    return true;
}

/*****************************************************************************
 * Initialization
 *****************************************************************************/

void app_uart_atcmd_init(void)
{
    // Register timer callback
    if (atcmd_timer_id == 0)
    {
        app_timer_reg_cb(uart_atcmd_timeout_cb, &atcmd_timer_id);
    }

    app_wifi_uart_rx_parser_register(app_uart_atcmd_rsp_handler);

    // Initialize queue
    memset(&at_cmd_queue, 0, sizeof(at_cmd_queue));

    // Initialize state
    at_cmd_t.cur_cmd = ATCMD_NUM;
    at_cmd_t.resend_cnt = 0;
    at_cmd_t.rx_cnt = 0;

    APP_PRINT_INFO0("[uart_atcmd] module initialized");
}

/*****************************************************************************
 * Demo
 *****************************************************************************/
void app_uart_atcmd_demo(uint8_t type)
{
    APP_PRINT_INFO0("app_uart_atcmd_demo");

#if (AT_CMD_VER == 1)
    switch ((T_ATCMD_TYPE)type)
    {
    case ATCMD_WLCONN:
        {
            app_uart_atcmd_queue_fill(ATCMD_WLCONN, "ssid,ZOTEST,pw,AAAAAAAA\r\n");
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_WLDISCONN:
        {
            app_uart_atcmd_queue_fill(ATCMD_WLDISCONN, "\r\n");
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    default:
        break;
    }
#else  // AT_CMD_VER == 2
    switch ((T_ATCMD_TYPE)type)
    {
    case ATCMD_ATPN:
        {
            app_uart_atcmd_queue_fill(ATCMD_ATPN, "ZOTEST,AAAAAAAA\r\n");
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_ATWD:
        {
            app_uart_atcmd_queue_fill(ATCMD_ATWD, "\r\n");
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    default:
        break;
    }
#endif
}

#endif
