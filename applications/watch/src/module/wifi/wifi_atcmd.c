/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/fs/fs.h>
#include "trace.h"
#include "wifi_atcmd.h"
#include <zephyr/drivers/uart.h>
#include "os_mem.h"
#include "wifi_app.h"
#include "os_queue.h"
#include "app_timer.h"
#include "wifi_uart.h"

#define MIN_RSP_LEN         5
#define MAX_RSP_LEN         100
#define ATCMD_RESEND_CNT    0

typedef enum
{
    UART_ATCMD_RESEND_TIMER           = 0x00,
} T_UART_ATCMD_TIMER;

T_AT_CMD    at_cmd_t = {.cur_cmd = ATCMD_NUM, .resend_cnt = 0, .rx_cnt = 0};
T_OS_QUEUE  at_cmd_queue;
static uint8_t atcmd_timer_id = 0;
static uint8_t timer_handle_atcmd_resend = 0;

//functions
static bool cmd_send(T_ATCMD_TYPE cmd, char *p_param);
static bool rsp_ATWS(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);
static bool rsp_ATWINFO(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);
static bool rsp_ATPN(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);
static bool rsp_ATWT(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);
void uart_atcmd_timeout_cb(uint8_t timer_evt, uint16_t param);
static bool rsp_ATST(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);
static bool rsp_ATWD(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);
static bool rsp_ATWQ(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);
static bool rsp_ATPS(char *p_param, T_AT_CMD_RSP_APP_CB app_cb);

/** @brief  User command table */
const T_AT_CMD_TABLE_ENTRY at_cmd_table[ATCMD_NUM] =
{
    [ATCMD_ATWS] = {.p_cmd = "ATWS",    .cmd_func = cmd_send,   .p_rsp = "[ATWS]",  .rsp_func = rsp_ATWS},
    [ATCMD_ATW0] = {.p_cmd = "ATW0=",   .cmd_func = cmd_send,   .p_rsp = "[ATW0]",  .rsp_func = NULL},
    [ATCMD_ATW1] = {.p_cmd = "ATW1=",   .cmd_func = cmd_send,   .p_rsp = "[ATW1]",  .rsp_func = NULL},
    [ATCMD_ATWC] = {.p_cmd = "ATWC",    .cmd_func = cmd_send,   .p_rsp = "[ATWC]",  .rsp_func = NULL},
    [ATCMD_ATWT] = {.p_cmd = "ATWT=",   .cmd_func = cmd_send,   .p_rsp = "[ATWT]",  .rsp_func = rsp_ATWT},
    [ATCMD_ATPN] = {.p_cmd = "ATPN=",   .cmd_func = cmd_send,   .p_rsp = "[ATPN]",  .rsp_func = rsp_ATPN},
    [ATCMD_ATSL] = {.p_cmd = "ATSL=",   .cmd_func = cmd_send,   .p_rsp = "[ATSL]",  .rsp_func = NULL},
    [ATCMD_ATWO] = {.p_cmd = "ATWO=",   .cmd_func = cmd_send,   .p_rsp = "[ATWO]",  .rsp_func = NULL},
    [ATCMD_ATSD] = {.p_cmd = "ATSD=",   .cmd_func = cmd_send,   .p_rsp = "[ATSD]",  .rsp_func = NULL},
    [ATCMD_ATST]   = {.p_cmd = "ATST=", .cmd_func = cmd_send,   .p_rsp = "[ATST]",   .rsp_func = rsp_ATST},
    [ATCMD_ATWD] = {.p_cmd = "ATWD",    .cmd_func = cmd_send,   .p_rsp = "[ATWD]",  .rsp_func = rsp_ATWD},
    /* ATW? -- query current STA info. The "STA,...,<ip>,<gw>" data line carries no
     * "[ATxx]" prefix and is routed via the data-line hook above; rsp_func must be
     * non-NULL so cur_cmd stays set as ATCMD_ATWINFO while that line is parsed. */
    [ATCMD_ATWINFO] = {.p_cmd = "ATW?", .cmd_func = cmd_send,   .p_rsp = "[ATW?]",  .rsp_func = rsp_ATWINFO},
    /* ATWQ -- lightweight local-IP query. Single reply line "[ATWQ] IP => a.b.c.d"
     * carries the "[ATWQ]" prefix, so it flows through the normal prefix match to
     * rsp_ATWQ (no data-line hook needed) and doubles as the terminator. */
    [ATCMD_ATWQ] = {.p_cmd = "ATWQ",    .cmd_func = cmd_send,   .p_rsp = "[ATWQ]",  .rsp_func = rsp_ATWQ},
    /* ATPS=mode,port -- start transport server (sdio2tcp). mode 0=TCP/1=UDP/2=SSL.
     * The chip's server_start task replies ~1s later with "[ATPS] OK" +
     * "[ATPS] con_id=n" once it is actually listening, or "[ATPS] ERROR:n" on
     * bind/listen failure. rsp_func is now non-NULL so this command WAITS for
     * that reply (flow-control stays on ATPS until it arrives / times out) and
     * forwards it to the caller's cb, letting the shell set its "server up" flag
     * from the real reply. A later async "[ATPS] A client connected to server[n]"
     * arrives after the command has completed (cur_cmd cleared) and is ignored. */
    [ATCMD_ATPS] = {.p_cmd = "ATPS=",   .cmd_func = cmd_send,   .p_rsp = "[ATPS]",  .rsp_func = rsp_ATPS},
};

extern int sdio_tx_test(char *p_param);
extern bool sdcard_test_start(void);
extern bool sdcard_test_end(void);

void uart_atcmd_rsp_handler(void)
{
cmdbuf_read:
    uint16_t recv_len = atcmd_recv(at_cmd_t.rx_buf + at_cmd_t.rx_cnt,
                                   sizeof(at_cmd_t.rx_buf) - at_cmd_t.rx_cnt);
    at_cmd_t.rx_cnt += recv_len;

    if (at_cmd_t.rx_cnt < MIN_RSP_LEN)
    {
        APP_PRINT_ERROR1("[wifi] at cmd rx buf len less than min size, len = %d", at_cmd_t.rx_cnt);
        return;
    }

    uint16_t parser_ofs = 0;
    while (parser_ofs < at_cmd_t.rx_cnt)
    {
        uint16_t start_ofs = at_cmd_t.rx_cnt;
        uint16_t end_ofs = 0;

        for (uint16_t i = parser_ofs; i < at_cmd_t.rx_cnt; i++)
        {
            if (at_cmd_t.rx_buf[i] != '\r' && at_cmd_t.rx_buf[i] != '\n') //get the line start
            {
                if (i < start_ofs)
                {
                    start_ofs = i;
                }
            }
            else
            {
                if (start_ofs < at_cmd_t.rx_cnt) //get the line end
                {
                    end_ofs = i;
                    break;
                }
            }
        }

        if (end_ofs) //get both line start and end
        {
            uint8_t *p_buf = os_mem_alloc(OS_MEM_TYPE_DATA, end_ofs - start_ofs + 1);
            if (p_buf == NULL)
            {
                APP_PRINT_ERROR0("[wifi] at cmd parser buf malloc fail !");
                return;
            }
            memcpy(p_buf, &at_cmd_t.rx_buf[start_ofs], end_ofs - start_ofs);
            p_buf[end_ofs - start_ofs] = 0;//add string end
            APP_PRINT_INFO1("[wifi] at cmd rsp = %s", TRACE_STRING(p_buf));

            /* Some commands emit data line(s) WITHOUT an "[ATxx]" prefix before
             * the terminating "[ATxx]" status line, so the prefix-match loop below
             * ignores them. Route such lines to the in-flight command's callback:
             *   ATWS  : one "AP : <n>,<ssid>,<chan>,<sec>,<rssi>,<bssid>" line per AP
             *   ATW?  : one "STA,<ssid>,<chan>,<sec>,<mac>,<ip>,<gw>" info line */
            if (p_buf[0] != '[' &&
                ((at_cmd_t.cur_cmd == ATCMD_ATWS &&
                  (end_ofs - start_ofs) >= 4 && !memcmp(p_buf, "AP :", 4)) ||
                 (at_cmd_t.cur_cmd == ATCMD_ATWINFO)))
            {
                T_AT_CMD_QUEUE *data_pkt = uart_atcmd_queue_peek(0);
                if (data_pkt && data_pkt->cb)
                {
                    data_pkt->cb((char *)p_buf);
                }
            }

            uint16_t i = 0;
            for (i = 0; i < ATCMD_NUM; i++)
            {
                if (!memcmp(p_buf, at_cmd_table[i].p_rsp, strlen(at_cmd_table[i].p_rsp)))
                {
                    //cmd wait rsp, flush cmd. need to check the cmd rsp spec, some rsp may have multi lines.
                    if (at_cmd_t.cur_cmd == i)
                    {
                        //trigger cmd sets flow
                        app_stop_timer(&timer_handle_atcmd_resend);

                        if (at_cmd_table[i].rsp_func)
                        {
                            /* peek(0) can momentarily be NULL if a resend-timeout
                             * flush (app_main_task) races this handler (wifi_task);
                             * pass a NULL cb rather than deref a freed/absent pkt. */
                            T_AT_CMD_QUEUE *cmd_queue_pkt = uart_atcmd_queue_peek(0);
                            at_cmd_table[i].rsp_func(p_buf,
                                                     cmd_queue_pkt ? cmd_queue_pkt->cb : NULL);
                        }

                        at_cmd_t.cur_cmd = ATCMD_NUM;
                        at_cmd_t.rx_cnt = 0;
                        at_cmd_t.resend_cnt = 0;
                        T_WIFI_MSG cmd_msg;
                        cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
                        uart_atcmd_queue_flush(1);
                        if (app_send_msg_to_wifitask(&cmd_msg) == false)
                        {
                            APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
                        }
                        break;
                    }
                    else  //The current rsp is not the result we are waiting for.
                    {
                        if (at_cmd_table[i].rsp_func)
                        {
                            at_cmd_table[i].rsp_func(p_buf, NULL);
                        }
                    }
                }
            }
            parser_ofs = end_ofs;
            os_mem_free(p_buf);
        }
        else if (start_ofs < at_cmd_t.rx_cnt) //only get line start
        {
            if (parser_ofs) //data before parser_ofs should be removed
            {
                memmove(at_cmd_t.rx_buf, &at_cmd_t.rx_buf[start_ofs], at_cmd_t.rx_cnt - start_ofs);
                at_cmd_t.rx_cnt -= start_ofs;
                if ((at_cmd_t.rx_cnt + start_ofs) == sizeof(
                        at_cmd_t.rx_buf)) //rx buf is full before remove, maybe more data is in uart console buf, read again.
                {
                    goto cmdbuf_read;
                }
                break;
            }
            if (at_cmd_t.rx_cnt == sizeof(at_cmd_t.rx_buf))
            {
                //one line length is larger than cmd buf length, some errors must happen, delete the data, read again.
                at_cmd_t.rx_cnt = 0;
                goto cmdbuf_read;
            }
            break;
        }
        else //no line start and end
        {
            at_cmd_t.rx_cnt = 0;
            if (at_cmd_t.rx_cnt == sizeof(at_cmd_t.rx_buf))
            {
                //try read again, maybe more data is in uart console buf.
                goto cmdbuf_read;
            }
            break;
        }
    }
}

void uart_atcmd_flow_ctrl_handler(T_WIFI_MSG atcmd_msg)
{
    if (at_cmd_t.cur_cmd != ATCMD_NUM)
    {
        APP_PRINT_INFO1("[wifi] atcmd flow ctrl, cur cmd is sending cur = 0x%x", at_cmd_t.cur_cmd);
        return;
    }

    T_AT_CMD_QUEUE *cmd_queue_pkt = uart_atcmd_queue_peek(0);
    if (cmd_queue_pkt)
    {
        if (cmd_queue_pkt->cmd >= ATCMD_NUM)
        {
            APP_PRINT_ERROR1("[wifi] atcmd invalid cmd = 0x%x", cmd_queue_pkt->cmd);
            return;
        }
        at_cmd_table[cmd_queue_pkt->cmd].cmd_func(cmd_queue_pkt->cmd, cmd_queue_pkt->param);

        //need to check rsp
        if (at_cmd_table[cmd_queue_pkt->cmd].rsp_func)
        {
            at_cmd_t.cur_cmd = cmd_queue_pkt->cmd;
            at_cmd_t.rx_cnt = 0;
            at_cmd_t.resend_cnt = 0;
            //start timer
            app_start_timer(&timer_handle_atcmd_resend, "atcmd timer",
                            atcmd_timer_id, UART_ATCMD_RESEND_TIMER, 0, false, 20000);
        }
        else
        {
            at_cmd_t.cur_cmd = ATCMD_NUM;
            at_cmd_t.rx_cnt = 0;
            at_cmd_t.resend_cnt = 0;
            T_WIFI_MSG cmd_msg;
            cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
            uart_atcmd_queue_flush(1);
            if (app_send_msg_to_wifitask(&cmd_msg) == false)
            {
                APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
            }
        }
    }

}

static bool cmd_send(T_ATCMD_TYPE cmd, char *p_param)
{

    if (at_cmd_t.cur_cmd != ATCMD_NUM)
    {
        APP_PRINT_ERROR2("[wifi] atcmd is sending!  cur = 0x%x, new = %d", at_cmd_t.cur_cmd, cmd);
    }
    uint16_t cmd_len = 0;
    uint16_t param_len = 0;
    cmd_len = strlen(at_cmd_table[cmd].p_cmd);
    if (p_param != NULL)
    {
        param_len = strlen(p_param);
    }
    uint16_t total_len = cmd_len + param_len + 2;

    uint8_t *send_buf = os_mem_alloc(OS_MEM_TYPE_DATA, total_len);
    if (send_buf == NULL)
    {
        return false;
    }

    memcpy(send_buf, at_cmd_table[cmd].p_cmd, cmd_len);
    if (param_len)
    {
        memcpy(send_buf + cmd_len, p_param, param_len);
    }
    send_buf[total_len - 2] = '\r';
    send_buf[total_len - 1] = '\n';
    bool ret = atcmd_send(send_buf, total_len);
    os_mem_free(send_buf);

    return ret;
}


static bool rsp_ATWS(char *p_param, T_AT_CMD_RSP_APP_CB app_cb)
{
    //user process
    if (app_cb)
    {
        app_cb(p_param);
    }
    return true;
}

static bool rsp_ATWINFO(char *p_param, T_AT_CMD_RSP_APP_CB app_cb)
{
    /* Reaches here for the terminating "[ATW?] OK" line; the "STA,...,ip,gw"
     * data line is delivered to the callback earlier via the data-line hook. */
    if (app_cb)
    {
        app_cb(p_param);
    }
    return true;
}

static bool rsp_ATWQ(char *p_param, T_AT_CMD_RSP_APP_CB app_cb)
{
    /* Single reply line "[ATWQ] IP => a.b.c.d" -- forward to the callback, which
     * parses the address after "=>". This line also terminates the command. */
    APP_PRINT_TRACE1("atwq response %s", TRACE_STRING(p_param));
    if (app_cb)
    {
        app_cb(p_param);
    }
    return true;
}

static bool rsp_ATPN(char *p_param, T_AT_CMD_RSP_APP_CB app_cb)
{
    //user process
    APP_PRINT_INFO1("atpn response %s", TRACE_STRING(p_param));

    if (app_cb)
    {
        app_cb(p_param);
    }

    // uart_atcmd_queue_fill(ATCMD_ATWT, "-s", NULL);
    return true;
}

static bool rsp_ATWT(char *p_param, T_AT_CMD_RSP_APP_CB app_cb)
{
    //user process
    APP_PRINT_INFO1("atwt response %s", TRACE_STRING(p_param));

    if (app_cb)
    {
        app_cb(p_param);
    }
    return true;
}

static bool rsp_ATST(char *p_param, T_AT_CMD_RSP_APP_CB app_cb)
{
    //user process
    APP_PRINT_INFO1("atst response %s", TRACE_STRING(p_param));

    if (app_cb)
    {
        app_cb(p_param);
    }
    return true;
}

static bool rsp_ATWD(char *p_param, T_AT_CMD_RSP_APP_CB app_cb)
{
    //user process
    APP_PRINT_INFO1("atwd response %s", TRACE_STRING(p_param));

    if (app_cb)
    {
        app_cb(p_param);
    }
    return true;
}

static bool rsp_ATPS(char *p_param, T_AT_CMD_RSP_APP_CB app_cb)
{
    /* Terminator line is "[ATPS] OK" (success) or "[ATPS] ERROR:n"; the trailing
     * "[ATPS] con_id=n" data line arrives after cur_cmd is cleared and reaches
     * here with app_cb==NULL, so it is a no-op. */
    APP_PRINT_INFO1("atps response %s", TRACE_STRING(p_param));

    if (app_cb)
    {
        app_cb(p_param);
    }
    return true;
}

void uart_atcmd_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("uart_atcmd_timeout_cb: timer_id %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case UART_ATCMD_RESEND_TIMER:
        {
            app_stop_timer(&timer_handle_atcmd_resend);
            if (at_cmd_t.resend_cnt >= ATCMD_RESEND_CNT)
            {
                //clear cur sending cmd status
                at_cmd_t.cur_cmd = ATCMD_NUM;
                at_cmd_t.resend_cnt = 0;
                at_cmd_t.rx_cnt = 0;
                uart_atcmd_queue_flush(1);
            }
            else
            {
                at_cmd_t.resend_cnt++;
            }
            T_WIFI_MSG cmd_msg;
            cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
            if (app_send_msg_to_wifitask(&cmd_msg) == false)
            {
                APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
            }

        }
        break;
    default:
        break;
    }
}

bool uart_atcmd_queue_fill(T_ATCMD_TYPE cmd, char *param, T_AT_CMD_RSP_APP_CB app_cb)
{
    T_AT_CMD_QUEUE *cmd_queue_pkt;
    uint16_t param_len = 1;
    if (param != NULL)
    {
        param_len += strlen(param);
    }

    cmd_queue_pkt = (T_AT_CMD_QUEUE *)os_mem_alloc(OS_MEM_TYPE_DATA,
                                                   sizeof(T_AT_CMD_QUEUE) + param_len);
    if (cmd_queue_pkt == NULL)
    {
        APP_PRINT_ERROR0("[wifi] uart atcmd queue fill get buffer error");
        return false;
    }

    cmd_queue_pkt->cmd = cmd;
    cmd_queue_pkt->cb = app_cb;
    memset(cmd_queue_pkt->param, 0, param_len);
    if (param_len > 1)
    {
        memcpy(cmd_queue_pkt->param, param, param_len - 1);
    }
    os_queue_in(&at_cmd_queue, cmd_queue_pkt);

    return true;
}

void *uart_atcmd_queue_peek(int offset)
{
    void *cmd_queue_pkt = os_queue_peek(&at_cmd_queue, offset);
    return cmd_queue_pkt;
}

void uart_atcmd_queue_flush(uint16_t cnt)
{
    T_AT_CMD_QUEUE *cmd_queue_pkt;
    APP_PRINT_TRACE1("[wifi] uart atcmd queue flush: %d", cnt);
    if (cnt > at_cmd_queue.count)
    {
        cnt = at_cmd_queue.count;
    }
    for (uint16_t i = 0; i < cnt; i++)
    {
        cmd_queue_pkt = os_queue_out(&at_cmd_queue);
        os_mem_free(cmd_queue_pkt);
    }
}

void uart_atcmd_init(void)
{
    if (atcmd_timer_id == 0)
    {
        app_timer_reg_cb(uart_atcmd_timeout_cb, &atcmd_timer_id);
    }
}

void cmd_list_demo(void)
{
    // uart_atcmd_queue_fill(ATCMD_ATW0, "mytest");
    // uart_atcmd_queue_fill(ATCMD_ATW1, "12345678");
    // uart_atcmd_queue_fill(ATCMD_ATWC, NULL);
    // uart_atcmd_queue_fill(ATCMD_ATWT, NULL);
    uart_atcmd_queue_fill(ATCMD_ATPN, "mytest,12345678", NULL);
    // uart_atcmd_queue_fill(ATCMD_ATPN, "zhang-net,admin123");
    //uart_atcmd_queue_fill(ATCMD_ATWS, NULL);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void wifi_atcmd_sleep_mode(void)
{
    uart_atcmd_queue_fill(ATCMD_ATSL, "r[0]", NULL);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void cmd_wifi_scan(T_AT_CMD_RSP_APP_CB cb)
{
    APP_PRINT_INFO0("[wifi]  cmd_wifi_scan !");
    /* "ATWS" with no parameter scans all channels; per-AP results are
     * delivered to cb via the ATWS hook in uart_atcmd_rsp_handler, and the
     * terminating "[ATWS] OK" line is delivered through rsp_ATWS. */
    uart_atcmd_queue_fill(ATCMD_ATWS, NULL, cb);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void cmd_wifi_query_info(T_AT_CMD_RSP_APP_CB cb)
{
    APP_PRINT_INFO0("[wifi]  cmd_wifi_query_info !");
    /* "ATW?" queries current STA info; the "STA,...,<ip>,<gw>" data line is
     * delivered to cb via the data-line hook in uart_atcmd_rsp_handler, and the
     * terminating "[ATW?] OK" line is delivered through rsp_ATWINFO. */
    uart_atcmd_queue_fill(ATCMD_ATWINFO, NULL, cb);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void cmd_wifi_query_ip(T_AT_CMD_RSP_APP_CB cb)
{
    APP_PRINT_INFO0("[wifi]  cmd_wifi_query_ip !");
    /* "ATWQ" -- lightweight local-IP query. The single "[ATWQ] IP => a.b.c.d"
     * reply is delivered to cb via rsp_ATWQ (normal prefix match). */
    uart_atcmd_queue_fill(ATCMD_ATWQ, NULL, cb);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void cmd_wifi_set_server(T_AT_CMD_RSP_APP_CB cb)
{
    APP_PRINT_INFO0("[wifi]  cmd_wifi_set_server !");
    uart_atcmd_queue_fill(ATCMD_ATWT, "-s", cb);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void cmd_wifi_download_file(void)
{
    APP_PRINT_INFO0("[wifi]  cmd_wifi_download_file !");
    uart_atcmd_queue_fill(ATCMD_ATWT, "-s", NULL);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }

}

/**
 * @brief Replace IP address in command string
 *
 * @param cmd_str Original command string (will be modified)
 * @param new_ip New IP address
 * @param output Output buffer
 * @param output_size Output buffer size
 * @return int Returns 0 on success, -1 on failure
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
    uart_atcmd_queue_fill(ATCMD_ATST, "4", NULL);
    uart_atcmd_queue_fill(ATCMD_ATWT, result, (T_AT_CMD_RSP_APP_CB)sdio_tx_test);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}
void cmd_wifi_upload_file_stop(void)
{
    sdcard_test_end();
}

void cmd_wifi_set_client_prepare()
{
    uart_atcmd_queue_fill(ATCMD_ATST, "4", NULL);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void cmd_wifi_set_client(char *str, T_AT_CMD_RSP_APP_CB cb)
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

    uart_atcmd_queue_fill(ATCMD_ATWT, result, cb);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void cmd_wifi_iperf_test_enable(T_AT_CMD_RSP_APP_CB cb)
{
    /* ATST=3 => IPERF_TEST_ENABLE on the WiFi chip (see enum TEST_CMD in the
     * chip's interface_hal.h): iperf_test_enable=1, file_transfer_test_enable=0,
     * i.e. SDIO forwarding is DISABLED so this is pure over-the-air iperf.
     * NOTE: this previously sent "ATWT=3", which the chip's fATWT/cmd_iperf
     * parses as a bogus iperf argument (no-op) -- it never selected iperf mode. */
    uart_atcmd_queue_fill(ATCMD_ATST, "3", cb);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

/* ---- SDIO data-path test (ATST=4 file-transfer mode + ATPS TCP server) ---- *
 * Unlike the iperf path (ATST=3, no SDIO), the data path bridges TCP <-> SDIO
 * on the WiFi chip (ATST=4 => file_transfer_test_enable=1). The device is the
 * TCP SERVER for BOTH directions : the host connects as a
 * TCP client and either sends (downlink: host -> TCP -> SDIO -> SoC) or
 * receives (uplink: SoC -> SDIO -> TCP -> host).                              */

/* Enter file-transfer mode and start a TCP server on <port>. The <cb> is
 * invoked with the actual "[ATPS] OK"/"[ATPS] ERROR:n" reply so the caller can
 * set its "server up" flag from the real result (pass NULL to ignore). */
void cmd_wifi_data_test_enter(uint16_t port, T_AT_CMD_RSP_APP_CB cb)
{
    APP_PRINT_INFO1("[wifi]  cmd_wifi_data_test_enter port=%d !", port);
    char ps_param[16];
    snprintf(ps_param, sizeof(ps_param), "0,%u", (unsigned)port);  /* 0 = TCP */

    // uart_atcmd_queue_fill(ATCMD_ATST, "4", NULL);      /* FILE_TRANSFER_TEST_ENABLE */
    uart_atcmd_queue_fill(ATCMD_ATPS, ps_param, cb); /* ATPS=0,<port> start server */

    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

