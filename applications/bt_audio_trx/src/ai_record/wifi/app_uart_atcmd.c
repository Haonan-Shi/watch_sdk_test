/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_WIFI_UART_CMD

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/fs/fs.h>
#include "trace.h"
#include "app_uart_atcmd.h"
#include "app_wifi_uart.h"
#include <zephyr/drivers/uart.h>
#include "os_mem.h"
#include "wifi_app.h"
#include "os_queue.h"
#include "app_timer.h"
#include "app_wifi_uart.h"
#include "wifi_transport.h"


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
/* ---- Broadcast dispatch array (replaces single-callback slot) ------------- *
 * Multiple modules (file_trans, app_ai_record, etc.) can independently
 * register for AT events; notify_at_evt() broadcasts to the whole array. */
static app_uart_atcmd_cb_t  g_at_cb_array[WIFI_TRANSPORT_MAX_CBS] = {NULL};
static uint8_t              g_at_cb_count = 0;
static char                 s_cur_cmd_str[64] = {0};  // last AT cmd string sent (for TX/RSP pairing)

/* Stashed by rsp_ATPI from the ATPI server line, consumed by file_trans
 * when building EVT_WIFI_CONNECT so the real listening port is reported. */
static uint16_t             g_atpi_port = 0;

// Forward declarations
static bool cmd_send(T_ATCMD_TYPE cmd, const char *cmd_str);
static void atcmd_parser_process(void);
static void notify_at_evt(T_AT_EVT_TYPE evt, uint8_t *p_data, uint16_t len);
static void process_unsolicited_msg(char *p_param, uint16_t len);
static void *uart_atcmd_queue_peek(int offset);
static void uart_atcmd_queue_flush(uint16_t cnt);
static void uart_atcmd_timeout_cb(uint8_t timer_evt, uint16_t param);
static void uart_atcmd_flow_ctrl_handler(void);

// Response handlers - using original app_uart_atcmd mechanism (without app_cb)
static bool rsp_WLCONN(char *p_param);
static bool rsp_WLDISCONN(char *p_param);
static bool rsp_ATWS(char *p_param);
static bool rsp_ATPN(char *p_param);
static bool rsp_ATPW(char *p_param);
static bool rsp_ATPA(char *p_param);
static bool rsp_ATPI(char *p_param);
static bool rsp_ATPS(char *p_param);
static bool rsp_ATWT(char *p_param);
static bool rsp_ATST(char *p_param);
static bool rsp_ATWD(char *p_param);


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
    [ATCMD_ATWS] = {.p_cmd = "ATWS",    .p_rsp = "[ATWS]",  .rsp_func = rsp_ATWS,    .timeout_ms = 20000},
    [ATCMD_ATW0] = {.p_cmd = "ATW0=",   .p_rsp = "[ATW0]",  .rsp_func = NULL,        .timeout_ms = 10000},
    [ATCMD_ATW1] = {.p_cmd = "ATW1=",   .p_rsp = "[ATW1]",  .rsp_func = NULL,        .timeout_ms = 10000},
    [ATCMD_ATWC] = {.p_cmd = "ATWC",    .p_rsp = "[ATWC]",  .rsp_func = NULL,        .timeout_ms = 30000},
    [ATCMD_ATWT] = {.p_cmd = "ATWT=",   .p_rsp = "[ATWT]",  .rsp_func = rsp_ATWT,   .timeout_ms = 60000},
    [ATCMD_ATPN] = {.p_cmd = "ATPN=",   .p_rsp = "[ATPN]",  .rsp_func = rsp_ATPN,   .timeout_ms = 20000},
    [ATCMD_ATPW] = {.p_cmd = "ATPW=",   .p_rsp = "[ATPW]",  .rsp_func = rsp_ATPW,   .timeout_ms = 20000},
    [ATCMD_ATPA] = {.p_cmd = "ATPA=",   .p_rsp = "[ATPA]",  .rsp_func = rsp_ATPA,   .timeout_ms = 20000},
    [ATCMD_ATPI] = {.p_cmd = "ATPI",    .p_rsp = "[ATPI]",  .rsp_func = rsp_ATPI,   .timeout_ms = 20000},
    [ATCMD_ATPS] = {.p_cmd = "ATPS=",   .p_rsp = "[ATPS]",  .rsp_func = rsp_ATPS,   .timeout_ms = 20000},
    [ATCMD_ATSL] = {.p_cmd = "ATSL=",   .p_rsp = "[ATSL]",  .rsp_func = NULL,        .timeout_ms = 10000},
    [ATCMD_ATWO] = {.p_cmd = "ATWO=",   .p_rsp = "[ATWO]",  .rsp_func = NULL,        .timeout_ms = 10000},
    [ATCMD_ATSD] = {.p_cmd = "ATSD=",   .p_rsp = "[ATSD]",  .rsp_func = NULL,        .timeout_ms = 30000},
    [ATCMD_ATST]   = {.p_cmd = "ATST=", .p_rsp = "[ATST]",  .rsp_func = rsp_ATST,    .timeout_ms = 10000},
    [ATCMD_ATWD]   = {.p_cmd = "ATWD",  .p_rsp = "[ATWD]",  .rsp_func = rsp_ATWD,    .timeout_ms = 10000},
};
#endif

// External functions from wifi_atcmd.c

extern bool sdcard_test_start(void);
extern bool sdcard_test_end(void);

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
    uint16_t total_len = cmd_len + param_len + 2;  // +2 for \r\n

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
    send_buf[total_len - 2] = '\r';
    send_buf[total_len - 1] = '\n';

    // Remember the full command string so the matching response can be paired with it
    snprintf(s_cur_cmd_str, sizeof(s_cur_cmd_str), "%s%s",
             at_cmd_table[cmd].p_cmd, cmd_str ? cmd_str : "");

    APP_PRINT_INFO1("[uart_atcmd] TX cmd: %s", TRACE_STRING(s_cur_cmd_str));

    bool ret = atcmd_send(send_buf, total_len);
    os_mem_free(send_buf);

    return ret;
}

/*****************************************************************************
 * Response Parser (original app_uart_atcmd mechanism)
 *****************************************************************************/

/**
 * @brief Parse received AT response
 * This is the optimized O(n) parser from app_uart_atcmd
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
                    /* Points at the recognized response prefix within p_line once one
                     * is matched below. Used so the handler and logs skip any leftover
                     * garbage (e.g. the module's newline-less "AT COMMAND READY" prompt
                     * that got glued to the front of "[ATPN] OK") preceding the real
                     * response on this physical line. */
                    char *p_clean = (char *)p_line;

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
                        char *p_match;
                        if (p_rsp && (p_match = strstr((char *)p_line, p_rsp)) != NULL)
                        {
                            // Found response prefix; drop any garbage before it
                            p_clean = p_match;
                            if (at_cmd_t.cur_cmd == cmd_idx)
                            {
                                // Expected response for current command: pair it with the sent cmd
                                APP_PRINT_INFO2("[uart_atcmd] CMD: %s  <==  RSP: %s",
                                                TRACE_STRING(s_cur_cmd_str), TRACE_STRING(p_clean));
                                if (at_cmd_table[cmd_idx].rsp_func)
                                {
                                    at_cmd_table[cmd_idx].rsp_func(p_clean);
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
                                    at_cmd_table[cmd_idx].rsp_func(p_clean);
                                }
                                handled = true;
                                // Don't break - continue waiting for expected response
                            }
                            else
                            {
                                // No command pending, but matched a response prefix
                                if (at_cmd_table[cmd_idx].rsp_func)
                                {
                                    at_cmd_table[cmd_idx].rsp_func(p_clean);
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
                        APP_PRINT_INFO2("[uart_atcmd] CMD: %s  finished, RSP: %s",
                                        TRACE_STRING(s_cur_cmd_str), TRACE_STRING(p_clean));

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
        /* Buffer was compacted: the leftover now starts at index 0, so the
         * no-newline fallback below must scan from 0 (start_idx still holds the
         * stale pre-memmove offset, which would skip a trailing OK/ERROR line
         * such as "...socket:0\r\n[ATPI] OK"). */
        start_idx = 0;
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
                    char *p_match;
                    if (p_rsp && (p_match = strstr((char *)p_line, p_rsp)) != NULL)
                    {
                        // Found response prefix; drop any garbage before it
                        if (at_cmd_t.cur_cmd == cmd_idx)
                        {
                            // Expected response for current command: pair it with the sent cmd
                            APP_PRINT_INFO2("[uart_atcmd] CMD: %s  <==  RSP: %s",
                                            TRACE_STRING(s_cur_cmd_str), TRACE_STRING(p_match));
                            if (at_cmd_table[cmd_idx].rsp_func)
                            {
                                at_cmd_table[cmd_idx].rsp_func(p_match);
                            }

                            // Command finished, trigger next command
                            if (is_command_finished)
                            {
                                APP_PRINT_INFO1("[uart_atcmd] CMD finished (no newline): %s",
                                                TRACE_STRING(s_cur_cmd_str));

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
                                at_cmd_table[cmd_idx].rsp_func(p_match);
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
 * Using original app_uart_atcmd mechanism
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

/** @brief Store the TCP server port parsed from ATPI response.
 *  Called by rsp_ATPI; consumed by file_trans when building EVT. */
void app_uart_atcmd_set_atpi_port(uint16_t port)
{
    g_atpi_port = port;
}

/** @brief Return the TCP server port from the last ATPI response.
 *  Returns 0 if ATPI has not yet been executed or parsing failed. */
uint16_t app_uart_atcmd_get_atpi_port(void)
{
    return g_atpi_port;
}

bool app_uart_atcmd_is_busy(void)
{
    return at_cmd_t.cur_cmd != ATCMD_NUM;
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
    /* Broadcast to all registered listeners. An empty array is a silent no-op. */
    for (uint8_t i = 0; i < g_at_cb_count; i++)
    {
        if (g_at_cb_array[i] != NULL)
        {
            g_at_cb_array[i](evt, p_data, len);
        }
    }
}

bool app_uart_atcmd_register_callback(app_uart_atcmd_cb_t cb)
{
    if (cb == NULL)
    {
        return false;
    }
    if (g_at_cb_count >= WIFI_TRANSPORT_MAX_CBS)
    {
        APP_PRINT_ERROR1("app_uart_atcmd_register_callback: table full (%d max)",
                         WIFI_TRANSPORT_MAX_CBS);
        return false;
    }
    g_at_cb_array[g_at_cb_count++] = cb;
    APP_PRINT_INFO1("app_uart_atcmd_register_callback: registered (%d listeners)",
                    g_at_cb_count);
    return true;
}

/*****************************************************************************
 * Unsolicited Message Handler
 *****************************************************************************/

/**
 * @brief Parse the IP out of a "wifi got ip" line and emit AT_EVT_WIFI_GOT_IP.
 *
 *        Module format is: wifi got ip "192.168.1.100" - the address sits
 *        inside the first pair of double quotes. On a parse miss we still
 *        emit the event with p_data=NULL so the connect flow (and the
 *        auto-ATPS that follows) isn't stalled; the host can re-poll the
 *        address later via its status command.
 */
static void process_got_ip_event(char *p_str)
{
    T_AT_IP_ADDR ip_data = {0};
    int ip[4] = {0};
    char *p_start = strchr(p_str, '"');

    if (p_start != NULL &&
        sscanf(p_start + 1, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == 4)
    {
        ip_data.octets[0] = (uint8_t)ip[0];
        ip_data.octets[1] = (uint8_t)ip[1];
        ip_data.octets[2] = (uint8_t)ip[2];
        ip_data.octets[3] = (uint8_t)ip[3];

        APP_PRINT_INFO4("[uart_atcmd] got ip %d.%d.%d.%d",
                        ip_data.octets[0], ip_data.octets[1],
                        ip_data.octets[2], ip_data.octets[3]);

        notify_at_evt(AT_EVT_WIFI_GOT_IP, (uint8_t *)&ip_data, sizeof(ip_data));
    }
    else
    {
        APP_PRINT_WARN1("[uart_atcmd] got ip parse failed: %s",
                        TRACE_STRING(p_str));
        notify_at_evt(AT_EVT_WIFI_GOT_IP, NULL, 0);
    }
}

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
        process_got_ip_event(p_param);
    }
    else if (at_cmd_t.cur_cmd == ATCMD_ATPI && strstr(p_param, "server,tcp"))
    {
        /* ATPI data line (not matched by any p_rsp prefix).
         * Parse: con_id:1,server,tcp,address:192.168.99.143,port:5001,socket:0 */
        char *addr_str = strstr(p_param, "address:");
        char *port_str = strstr(p_param, ",port:");

        if (addr_str && port_str)
        {
            addr_str += strlen("address:");
            port_str += strlen(",port:");

            char ip_buf[16] = {0};
            size_t ip_len = (size_t)(port_str - addr_str);
            if (ip_len > sizeof(ip_buf) - 1)
            {
                ip_len = sizeof(ip_buf) - 1;
            }
            memcpy(ip_buf, addr_str, ip_len);
            ip_buf[ip_len] = '\0';

            T_AT_IP_ADDR ip = {0};
            uint16_t port = 0;
            sscanf(ip_buf, "%hhu.%hhu.%hhu.%hhu",
                   &ip.octets[0], &ip.octets[1],
                   &ip.octets[2], &ip.octets[3]);
            sscanf(port_str, "%hu", &port);

            app_uart_atcmd_set_atpi_port(port);
            APP_PRINT_INFO2("[uart_atcmd] ATPI data line: ip=%s port=%u",
                            ip_buf, port);

            notify_at_evt(AT_EVT_WIFI_GOT_IP, (uint8_t *)&ip, sizeof(ip));
        }
    }
    else
    {
        // Unknown data
        notify_at_evt(AT_EVT_UNKNOWN_DATA, (uint8_t *)p_param, len);
    }
}

/*****************************************************************************
 * Response Handlers - using original app_uart_atcmd mechanism
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
        process_got_ip_event(p_param);
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

static bool rsp_ATWS(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATWS: %s", TRACE_STRING(p_param));

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

    return true;
}

static bool rsp_ATPN(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATPN: %s", TRACE_STRING(p_param));

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
        process_got_ip_event(p_param);
    }

    return true;
}

static bool rsp_ATPW(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATPW: %s", TRACE_STRING(p_param));

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

    return true;
}

static bool rsp_ATPA(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATPA: %s", TRACE_STRING(p_param));

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
        notify_at_evt(AT_EVT_WIFI_GOT_IP, NULL, 0);
    }

    return true;
}

static bool rsp_ATPI(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATPI: %s", TRACE_STRING(p_param));

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

    return true;
}

static bool rsp_ATPS(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATPS: %s", TRACE_STRING(p_param));

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

    return true;
}

static bool rsp_ATWT(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATWT: %s", TRACE_STRING(p_param));

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

    return true;
}

static bool rsp_ATST(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATST: %s", TRACE_STRING(p_param));

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

    return true;
}

static bool rsp_ATWD(char *p_param)
{
    APP_PRINT_INFO1("[uart_atcmd] rsp_ATWD: %s", TRACE_STRING(p_param));

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
 * Demo and Business Functions (from wifi_atcmd.c)
 * Using original app_uart_atcmd mechanism
 *****************************************************************************/

void cmd_list_demo(void)
{
    APP_PRINT_INFO0(" [wifi] cmd_list_demo");
    app_uart_atcmd_queue_fill(ATCMD_ATPN, "mytest,12345678");
    app_uart_atcmd_trigger_send_flow();
}

void wifi_atcmd_sleep_mode(void)
{
    app_uart_atcmd_queue_fill(ATCMD_ATSL, "r[0]");
    app_uart_atcmd_trigger_send_flow();
}

void cmd_wifi_set_server(void)
{
    APP_PRINT_INFO0("[wifi]  cmd_wifi_set_server !");
    app_uart_atcmd_queue_fill(ATCMD_ATWT, "-s");
    app_uart_atcmd_trigger_send_flow();
}


void cmd_wifi_download_file(void)
{
    APP_PRINT_INFO0("[wifi]  cmd_wifi_download_file !");
    app_uart_atcmd_queue_fill(ATCMD_ATWT, "-s");
    app_uart_atcmd_trigger_send_flow();
}

/**
 * @brief Replace IP address in command string
 */
int replace_ip_address(const char *cmd_str, const char *new_ip,
                       char *output, size_t output_size)
{
    if (!cmd_str || !new_ip || !output || output_size == 0)
    {
        return -1;
    }

    // Find IP address after "-c,"
    const char *ip_start = strstr(cmd_str, "-c,");
    if (!ip_start)
    {
        return -1;  // "-c," not found
    }

    ip_start += 3;  // Skip "-c,"

    // Find the end of IP address (next comma or end of string)
    const char *ip_end = strchr(ip_start, ',');
    if (!ip_end)
    {
        ip_end = cmd_str + strlen(cmd_str);
    }

    // Calculate new string length
    size_t prefix_len = ip_start - cmd_str;
    size_t new_ip_len = strlen(new_ip);
    size_t suffix_len = strlen(ip_end);
    size_t total_len = prefix_len + new_ip_len + suffix_len;

    if (total_len >= output_size)
    {
        return -1;  // Output buffer too small
    }

    // Build new string
    memcpy(output, cmd_str, prefix_len);
    memcpy(output + prefix_len, new_ip, new_ip_len);
    memcpy(output + prefix_len + new_ip_len, ip_end, suffix_len);
    output[total_len] = '\0';

    return 0;
}

void cmd_wifi_upload_file(char *str)
{
    APP_PRINT_INFO0("cmd_wifi_upload_file \n");

    if (sdcard_test_start())
    {
        APP_PRINT_ERROR0("[SDCARD] read SDCARD file fail !");
        return;
    }
    char original[] = "-c,192.168.0.0,-i,1,-t,20";
    char result[30];

    if (str != NULL)
    {
        APP_PRINT_INFO1("upload input IP str: %s\n", TRACE_STRING(str));

        if (replace_ip_address(original, str, result, sizeof(result)) == 0)
        {
            APP_PRINT_INFO1("After replacement: %s\n", TRACE_STRING(result));
        }
        else
        {
            APP_PRINT_INFO0("Replacement failed\n");
        }
    }

    app_uart_atcmd_queue_fill(ATCMD_ATWT, result);
    app_uart_atcmd_trigger_send_flow();
}

void cmd_wifi_upload_file_stop(void)
{
    sdcard_test_end();
}

void cmd_wifi_set_client(char *str)
{
    APP_PRINT_INFO0("[wifi]  cmd_wifi_set_client !");
    char original[] = "-c,192.168.0.0,-i,1,-t,15";
    char result[30];

    if (str != NULL)
    {
        APP_PRINT_INFO1("input ip str: %s\n", TRACE_STRING(str));

        if (replace_ip_address(original, str, result, sizeof(result)) == 0)
        {
            APP_PRINT_INFO1("After replacement: %s\n", TRACE_STRING(result));
        }
        else
        {
            APP_PRINT_INFO0("Replacement failed\n");
        }
    }

    app_uart_atcmd_queue_fill(ATCMD_ATWT, result);
    app_uart_atcmd_trigger_send_flow();
}
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app_uart_atcmd, LOG_LEVEL_DBG);

/*****************************************************************************
 * Demo (AT_CMD_VER 1 backward compatibility)
 *****************************************************************************/
void app_uart_atcmd_demo(uint8_t *ptr)
{
    uint8_t type = 0;
    type = *ptr;
    uint8_t subcmd = *(ptr + 1);
    APP_PRINT_INFO2("app_uart_atcmd_demo type =%d subcmd =%d ", type, subcmd);
    LOG_INF("app_uart_atcmd_demo type =%d subcmd =%d ", type, subcmd);
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
    case ATCMD_ATPN://5
        {
            app_uart_atcmd_queue_fill(ATCMD_ATPN, "mytest,12345678\r\n");
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_ATPW:
        {
            app_uart_atcmd_queue_fill(ATCMD_ATPW, "2");
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_ATPA:
        {
            app_uart_atcmd_queue_fill(ATCMD_ATPA, "8783mytest,12345678,1,0");
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_ATPI:
        {
            app_uart_atcmd_queue_fill(ATCMD_ATPI, NULL);
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_ATPS:
        {
            if (subcmd == 0) // -s
            {
                app_uart_atcmd_queue_fill(ATCMD_ATPS, "0,5001\r\n");
            }
            else
            {
                app_uart_atcmd_queue_fill(ATCMD_ATPS, "1,5002\r\n");
            }
            app_uart_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_ATWT://5
        {
            if (subcmd == 0) // -s
            {
                app_uart_atcmd_queue_fill(ATCMD_ATWT, "-s\r\n");
            }
            else
            {
                app_uart_atcmd_queue_fill(ATCMD_ATST, "4\r\n");
                app_uart_atcmd_queue_fill(ATCMD_ATWT, "-c,192.168.3.74,-i,1,-t,20 \r\n");
            }

            app_uart_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_ATST://5
        {
            app_uart_atcmd_queue_fill(ATCMD_ATST, "4\r\n");
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

#endif /* F_APP_WIFI_UART_CMD */