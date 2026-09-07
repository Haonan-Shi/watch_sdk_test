/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_WIFI_SPI_CMD

#include <string.h>
#include <stdio.h>
#include "trace.h"
#include "app_spi_atcmd.h"
#include "os_mem.h"
#include "os_queue.h"
#include "app_timer.h"
#include "app_util.h"
#include "app_spi_api.h"

#define USE_WIFI_TASK       0
#define MIN_RSP_LEN         2
#define MAX_RSP_LEN         1024
#define ATCMD_RSP_OK        "OK"
#define ATCMD_RSP_ERR       "ERROR"
#define ATCMD_RESEND_CNT    0

typedef struct
{
    uint8_t     magic[2];   // "AT"
    uint16_t    len;        // Payload Length
    uint8_t     payload[];  // Variable length data
} __attribute__((packed)) spi_frame_hdr_t;

/**
 *  @brief format: [AT(2)][Len(2)][Data(N)][CRC32(4)]...[Padding]
 */
#define SPI_PROTO_MAGIC_0   'A'
#define SPI_PROTO_MAGIC_1   'T'
#define SPI_FRAME_HDR_LEN   4  // Magic(2) + Len(2)
#define SPI_FRAME_CRC_LEN   4  // Checksum(4)

static app_spi_atcmd_cb_t g_at_callback = NULL;

typedef enum
{
    UART_ATCMD_RESEND_TIMER           = 0x00,
} T_UART_ATCMD_TIMER;

T_AT_CMD    at_cmd_t = {.cur_cmd = ATCMD_NUM, .resend_cnt = 0, .rx_cnt = 0};
T_OS_QUEUE  at_cmd_queue;
static uint8_t atcmd_timer_id = 0;
static uint8_t timer_handle_atcmd_resend = 0;

static void process_unsolicited_msg(char *p_param, uint16_t len);
static bool cmd_send(T_ATCMD_TYPE cmd, const char *cmd_str);
static void atcmd_parser_process(void);
static void notify_at_evt(T_AT_EVT_TYPE evt, uint8_t *p_data, uint16_t len);
void spi_atcmd_flow_ctrl_handler(T_WIFI_MSG atcmd_msg);

static bool rsp_WLCONN(char *p_param);
static bool rsp_WLDISCONN(char *p_param);
const T_AT_CMD_TABLE_ENTRY at_cmd_table[ATCMD_NUM] =
{
    {"AT+WLCONN=", rsp_WLCONN, 20000},
    {"AT+WLDISCONN", rsp_WLDISCONN, 10000},
};

static void *spi_atcmd_queue_peek(int offset)
{
    void *cmd_queue_pkt = os_queue_peek(&at_cmd_queue, offset);
    return cmd_queue_pkt;
}

static void spi_atcmd_queue_flush(uint16_t cnt)
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

static uint32_t checksum_32_spi(uint32_t start_value, uint8_t *data, int len)
{
    uint32_t checksum32 = start_value;
    uint16_t data16 = 0;
    int i;

    for (i = 0; i < (len / 2 * 2); i += 2)
    {
        data16 = (data[i] << 8) | data[i + 1];
        checksum32 += data16;
    }

    if (len % 2)
    {
        data16 = data[len - 1] << 8;
        checksum32 += data16;
    }

    return checksum32;
}

void app_spi_atcmd_trigger_send_flow(void)
{

    T_WIFI_MSG cmd_msg = {0};
    cmd_msg.event = 1;
#if USE_WIFI_TASK
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
#else
    spi_atcmd_flow_ctrl_handler(cmd_msg);
#endif
}

/**
 * @brief Parse SPI RX data
 *        Format: [AT(2)][Len(2)][Data(N)][CRC32(4)]...[Padding]
 */
static void spi_atcmd_rcv_cb(uint8_t *p_data, uint16_t data_len)
{
    spi_frame_hdr_t *hdr;
    uint16_t payload_len;
    uint32_t cal_crc, rcv_crc;

    if (p_data == NULL || data_len < (SPI_FRAME_HDR_LEN + SPI_FRAME_CRC_LEN))
    {
        return;
    }

    hdr = (spi_frame_hdr_t *)p_data;

    // 1. Verify Magic Number "AT"
    if (hdr->magic[0] != SPI_PROTO_MAGIC_0  || hdr->magic[1] != SPI_PROTO_MAGIC_1)
    {
        return;
    }

    // 2. Get Payload Len
    payload_len = hdr->len;

    // 3. Boundary Check
    if (SPI_FRAME_HDR_LEN + payload_len + SPI_FRAME_CRC_LEN > data_len)
    {
        APP_PRINT_ERROR2("[wifi] spi_atcmd_rcv_cb: req %d, act %d",
                         SPI_FRAME_HDR_LEN + payload_len + SPI_FRAME_CRC_LEN, data_len);
        return;
    }
    // APP_PRINT_INFO2("[wifi] spi_atcmd_rcv_cb: len %d, %b", payload_len,
    //                 TRACE_BINARY(SPI_FRAME_HDR_LEN + payload_len + SPI_FRAME_CRC_LEN, p_data));
    // 4. Checksum
    cal_crc = checksum_32_spi(0, p_data, SPI_FRAME_HDR_LEN + payload_len);
    memcpy(&rcv_crc, &p_data[SPI_FRAME_HDR_LEN + payload_len], 4);
    if (cal_crc != rcv_crc)
    {
        APP_PRINT_ERROR0("[wifi] spi_atcmd_rcv_cb: crc err");
        return;
    }

    // 5. Copy Payload to rx_buf
    if (payload_len > 0)
    {
        // Check Free Size
        if (at_cmd_t.rx_cnt + payload_len > sizeof(at_cmd_t.rx_buf))
        {
            APP_PRINT_ERROR2("[wifi] at rx buf full! cur:%d, add:%d", at_cmd_t.rx_cnt, payload_len);
            at_cmd_t.rx_cnt = 0; // TODO: Reset?
        }

        // Copy Payload
        memcpy(at_cmd_t.rx_buf + at_cmd_t.rx_cnt, hdr->payload, payload_len);
        at_cmd_t.rx_cnt += payload_len;

        // 6. Trigger Process
        atcmd_parser_process();
    }
}

static void wifi_at_response_handler(uint8_t *p_data, uint16_t len)
{
    APP_PRINT_INFO1("[wifi] wifi_at_response_handler: %s", TRACE_STRING(p_data));

    bool is_command_finished = false;

    if (strstr((char *)p_data, ATCMD_RSP_OK) ||
        strstr((char *)p_data, ATCMD_RSP_ERR))
    {
        is_command_finished = true;
    }

    if (at_cmd_t.cur_cmd < ATCMD_NUM)
    {
        if (at_cmd_table[at_cmd_t.cur_cmd].rsp_func)
        {
            at_cmd_table[at_cmd_t.cur_cmd].rsp_func((char *)p_data);
        }
    }
    else
    {
        process_unsolicited_msg((char *)p_data, len);
    }

    if (is_command_finished && at_cmd_t.cur_cmd < ATCMD_NUM)
    {
        APP_PRINT_INFO0("[wifi] CMD Finished, Trigger Next");

        app_stop_timer(&timer_handle_atcmd_resend);

        spi_atcmd_queue_flush(1);

        at_cmd_t.cur_cmd = ATCMD_NUM;
        at_cmd_t.resend_cnt = 0;

        app_spi_atcmd_trigger_send_flow();
    }
}

/**
 * @brief AT Parser
 */
static void atcmd_parser_process(void)
{
    if (at_cmd_t.rx_cnt == 0) { return; }

    uint16_t processed_len = 0;
    uint16_t start_idx = 0;
    uint16_t i;

    for (i = 0; i < at_cmd_t.rx_cnt; i++)
    {
        // 1. Find tail
        if (at_cmd_t.rx_buf[i] == '\n')
        {
            uint16_t end_idx = i;
            if (end_idx > 0 && at_cmd_t.rx_buf[end_idx - 1] == '\r')
            {
                end_idx--;
            }

            uint16_t line_len = end_idx - start_idx;

            // 2. extract
            if (line_len > 0)
            {
                // +1 for null terminator
                uint8_t *p_line = malloc(line_len + 1);
                if (p_line)
                {
                    memcpy(p_line, &at_cmd_t.rx_buf[start_idx], line_len);
                    p_line[line_len] = '\0';

                    // APP_PRINT_INFO1("[wifi] atcmd_parser_process: %s", TRACE_STRING(p_line));
                    wifi_at_response_handler(p_line, line_len);

                    os_mem_free(p_line);
                }
            }

            // update legnth(including \r\n)
            processed_len = i + 1;
            start_idx = processed_len;
        }
    }

    // 3. Move unhandled data to header
    if (processed_len > 0)
    {
        uint16_t remain_len = at_cmd_t.rx_cnt - processed_len;
        if (remain_len > 0)
        {
            memmove(at_cmd_t.rx_buf, &at_cmd_t.rx_buf[processed_len], remain_len);
        }
        at_cmd_t.rx_cnt = remain_len;
    }

    // 4. Overflow handler
    if (at_cmd_t.rx_cnt >= sizeof(at_cmd_t.rx_buf))
    {
        APP_PRINT_WARN0("[wifi] RX Buf Full without newline, clear!");
        at_cmd_t.rx_cnt = 0;
    }
}


void spi_atcmd_flow_ctrl_handler(T_WIFI_MSG atcmd_msg)
{
    if (at_cmd_t.cur_cmd != ATCMD_NUM)
    {
        APP_PRINT_INFO1("[wifi] busy, cur cmd=0x%x", at_cmd_t.cur_cmd);
        return;
    }

    T_AT_CMD_QUEUE *cmd_queue_pkt = (T_AT_CMD_QUEUE *)spi_atcmd_queue_peek(0);
    if (cmd_queue_pkt)
    {
        if (cmd_queue_pkt->cmd >= ATCMD_NUM)
        {
            spi_atcmd_queue_flush(1);
            return;
        }

        APP_PRINT_INFO1("[wifi] sending cmd type: %d", cmd_queue_pkt->cmd);

        cmd_send(cmd_queue_pkt->cmd, cmd_queue_pkt->param);

        at_cmd_t.cur_cmd = cmd_queue_pkt->cmd;
        at_cmd_t.resend_cnt = 0;
        app_start_timer(&timer_handle_atcmd_resend, "atcmd timer",
                        atcmd_timer_id, UART_ATCMD_RESEND_TIMER, 0, false, at_cmd_table[at_cmd_t.cur_cmd].timeout_ms);
    }
}

/**
 * @brief Send AT Cmd
 */
static bool cmd_send(T_ATCMD_TYPE cmd, const char *cmd_str)
{
    if (cmd_str == NULL) { return false; }

    uint16_t raw_data_len = strlen(at_cmd_table[cmd].p_cmd) + strlen(cmd_str);
    uint16_t packet_len = SPI_FRAME_HDR_LEN + raw_data_len + SPI_FRAME_CRC_LEN;

    uint8_t *send_buf = (uint8_t *)malloc(packet_len);
    if (send_buf == NULL)
    {
        APP_PRINT_ERROR0("Send Malloc Fail");
        return false;
    }

    send_buf[0] = SPI_PROTO_MAGIC_0;
    send_buf[1] = SPI_PROTO_MAGIC_1;
    send_buf[2] = raw_data_len & 0xFF;        // Len Low
    send_buf[3] = (raw_data_len >> 8) & 0xFF; // Len High
    memcpy(&send_buf[SPI_FRAME_HDR_LEN], at_cmd_table[cmd].p_cmd, strlen(at_cmd_table[cmd].p_cmd));
    memcpy(&send_buf[SPI_FRAME_HDR_LEN + strlen(at_cmd_table[cmd].p_cmd)], cmd_str, strlen(cmd_str));
    uint32_t crc = checksum_32_spi(0, send_buf, SPI_FRAME_HDR_LEN + raw_data_len);
    memcpy(&send_buf[packet_len - 4], &crc, 4);

    uint8_t ret = app_spi_master_send_raw_data(send_buf, packet_len);

    os_mem_free(send_buf);

    if (ret != 0)
    {
        APP_PRINT_ERROR1("SPI Hardware Send Fail: %d", ret);
        return false;
    }

    return true;
}

//**************************Parse Specific AT EVT***************************//

/**
 * @brief parse ip address from str
 */
static void process_got_ip_event(char *p_str)
{
    T_AT_IP_ADDR ip_data = {0};
    int ip[4] = {0};

    char *p_start = strchr(p_str, '"');

    if (p_start != NULL)
    {
        p_start++;

        if (sscanf(p_start, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) == 4)
        {
            ip_data.octets[0] = (uint8_t)ip[0];
            ip_data.octets[1] = (uint8_t)ip[1];
            ip_data.octets[2] = (uint8_t)ip[2];
            ip_data.octets[3] = (uint8_t)ip[3];

            APP_PRINT_INFO4("[wifi] process_got_ip_event: IP %d.%d.%d.%d",
                            ip_data.octets[0], ip_data.octets[1],
                            ip_data.octets[2], ip_data.octets[3]);

            notify_at_evt(AT_EVT_WIFI_GOT_IP, (uint8_t *)&ip_data, sizeof(T_AT_IP_ADDR));
            return;
        }
    }

    APP_PRINT_WARN1("[wifi] process_got_ip_event: Parse IP Failed: %s", TRACE_STRING(p_str));
}

static void process_ok_event(void)
{
    uint8_t state = AT_CMD_RSP_STATE_OK;
    notify_at_evt(AT_EVT_CMD_RESPONSE, (uint8_t *)&state, sizeof(state));
}

static void process_error_event(void)
{
    uint8_t state = AT_CMD_RSP_STATE_ERROR;
    notify_at_evt(AT_EVT_CMD_RESPONSE, (uint8_t *)&state, sizeof(state));
}

static void process_unsolicited_msg(char *p_param, uint16_t len)
{
    if (strstr(p_param, "wifi got ip"))
    {
        process_got_ip_event(p_param);
    }
    else if (strstr((char *)p_param, "wifi connected"))
    {
        notify_at_evt(AT_EVT_WIFI_CONNECTED, NULL, 0);
    }
    else if (strstr((char *)p_param, "wifi disconnected"))
    {
        notify_at_evt(AT_EVT_WIFI_DISCONNECTED, NULL, 0);
    }
    else
    {
        notify_at_evt(AT_EVT_UNKNOWN_DATA, p_param, len);
    }
}

static bool rsp_WLCONN(char *p_param)
{
    APP_PRINT_INFO1("rsp_WLCONN %s", TRACE_STRING(p_param));
    if (strstr(p_param, "OK"))
    {
        process_ok_event();
    }
    else if (strstr(p_param, "ERROR"))
    {
        process_error_event();
    }
    else if (strstr((char *)p_param, "wifi connected"))
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
    APP_PRINT_INFO1("rsp_WLDISCONN %s", TRACE_STRING(p_param));
    if (strstr(p_param, "OK"))
    {
        process_ok_event();
    }
    else if (strstr(p_param, "ERROR"))
    {
        process_error_event();
    }
    else if (strstr((char *)p_param, "wifi disconnected"))
    {
        notify_at_evt(AT_EVT_WIFI_DISCONNECTED, NULL, 0);
    }
    return true;
}

static void spi_atcmd_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    APP_PRINT_TRACE2("spi_atcmd_timeout_cb: timer_id %d, param %d", timer_evt, param);

    switch (timer_evt)
    {
    case UART_ATCMD_RESEND_TIMER:
        {
            app_stop_timer(&timer_handle_atcmd_resend);
            if (at_cmd_t.resend_cnt >= ATCMD_RESEND_CNT)
            {
                at_cmd_t.cur_cmd = ATCMD_NUM;
                at_cmd_t.resend_cnt = 0;
                at_cmd_t.rx_cnt = 0;
                spi_atcmd_queue_flush(1);
            }
            else
            {
                at_cmd_t.resend_cnt++;
                // TODO: Not implemented
            }

            app_spi_atcmd_trigger_send_flow();
        }
        break;
    default:
        break;
    }
}

bool app_spi_atcmd_queue_fill(T_ATCMD_TYPE cmd, char *param)
{
    APP_PRINT_TRACE1("[wifi] app_spi_atcmd_queue_fill: cmd %d", cmd);
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
    memset(cmd_queue_pkt->param, 0, param_len);
    if (param_len > 1)
    {
        memcpy(cmd_queue_pkt->param, param, param_len - 1);
    }
    os_queue_in(&at_cmd_queue, cmd_queue_pkt);

    return true;
}

static void notify_at_evt(T_AT_EVT_TYPE evt, uint8_t *p_data, uint16_t len)
{
    if (g_at_callback != NULL)
    {
        g_at_callback(evt, p_data, len);
    }
}

void app_spi_atcmd_register_callback(app_spi_atcmd_cb_t cb)
{
    g_at_callback = cb;
}

void app_spi_atcmd_init(void)
{
    if (atcmd_timer_id == 0)
    {
        app_timer_reg_cb(spi_atcmd_timeout_cb, &atcmd_timer_id);
    }

    app_spi_master_module_register(spi_atcmd_rcv_cb, 0);
    APP_PRINT_INFO0("[wifi] atcmd module initialized (SPI Mode)");
}

void app_spi_atcmd_demo(uint8_t type)
{
    APP_PRINT_INFO0("app_spi_atcmd_demo");
    switch ((T_ATCMD_TYPE)type)
    {
    case ATCMD_WLCONN:
        {
            app_spi_atcmd_queue_fill(ATCMD_WLCONN, "ssid,ZOTEST,pw,AAAAAAAA\r\n");
            app_spi_atcmd_trigger_send_flow();
        }
        break;
    case ATCMD_WLDISCONN:
        {
            app_spi_atcmd_queue_fill(ATCMD_WLDISCONN, "\r\n");
            app_spi_atcmd_trigger_send_flow();
        }
        break;

    default:
        break;
    }

}
#endif
