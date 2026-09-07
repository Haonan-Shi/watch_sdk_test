/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_WIFI_SPI_CMD
#include <stdlib.h>
#include "app_wifi_transfer.h"
#include "app_spi_api.h"
#include "trace.h"
#include "string.h"
#include "app_cmd.h"

static T_WIFI_AT_CMD_STATE at_cmd_state = WIFI_CMD_AT_STATE_IDLE;


static uint32_t checksum_32_wifi(uint32_t start_value, uint8_t *data, int len)
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

bool app_wifi_send_send_data(uint8_t *p_data, uint16_t len)
{
    uint8_t err_code = 0;
    if (p_data == NULL || len == 0)
    {
        err_code = 1;
        goto ERR;
    }
    else
    {
        T_SPI_SEND_ERR_CODE send_result = SPI_SEND_ERR_UNSUPPORTED;
        uint16_t total_len = len + 4; // Header(2) + len(2) + data(len)

        uint8_t *p_pack_data = malloc(total_len + 4);
        if (p_pack_data == NULL)
        {
            err_code = 2;
            goto ERR;
        }
        else
        {
            p_pack_data[0] = 0x54;
            p_pack_data[1] = 0x58;
            p_pack_data[2] = (uint8_t)len;
            p_pack_data[3] = (uint8_t)(len >> 8);
            memcpy(&p_pack_data[4], p_data, len);
            uint32_t checksum = checksum_32_wifi(0, p_pack_data, total_len);
            memcpy(&p_pack_data[total_len], &checksum, sizeof(checksum));

#if F_APP_SPI_ROLE_MASTER
            // Header(2) + len(2) + data(len) + checksum(4)
            send_result = app_spi_master_send_raw_data(p_pack_data, total_len + 4);
#endif
            APP_PRINT_INFO2("app_wifi_send_send_data: result %d, %b", send_result, TRACE_BINARY(total_len + 4,
                            p_pack_data));
            free(p_pack_data);
            if (send_result != SPI_SEND_SUC)
            {
                return false;
            }
            return true;
        }
    }
ERR:
    APP_PRINT_ERROR1("app_wifi_send_send_data error code: %d", -err_code);
    return false;
}

#if 0
void app_wifi_transfer_handle_at_evts(uint8_t *p_at_evt_data, uint16_t at_evt_len)
{
    uint8_t at_ok[] = "\r\nOK\r\n";
    if (memcmp(p_at_evt, at_ok, strlen((const char *)at_ok)) == 0)
    {
        at_cmd_state = WIFI_CMD_AT_STATE_IDLE;
        return;
    }
    else
    {
        app_wifi_transfer_handle_at_evts(p_at_evt, at_evt_len);
        APP_PRINT_INFO1("app_wifi_recv_data_handle: at_evt %b", TRACE_BINARY(at_evt_len, p_at_evt));
    }
    return;
}

void app_wifi_transfer_handle_raw_data(uint8_t *p_packed_data, uint16_t len)
{
    if (p_packed_data == NULL || len == 0)
    {
        APP_PRINT_ERROR0("app_wifi_recv_data_handle: p_data is NULL or len is 0");
        return;
    }

    if (p_packed_data[0] != 0x54 || p_packed_data[1] != 0x58)
    {
        return;
    }
    uint16_t data_len = (p_packed_data[2] | (p_packed_data[3] << 8));
    uint32_t checksum = 0;
    memcpy(&checksum, &p_packed_data[len - 4], sizeof(checksum));
    uint32_t checksum_cal = checksum_32_wifi(0, p_packed_data, len - 4);
    if (checksum != checksum_cal)
    {
        APP_PRINT_ERROR1("app_wifi_recv_data_handle: checksum error, checksum 0x%08x, cal 0x%08x",
                         checksum_cal);
        return;
    }
    // only at evts will be sent from wifi module
    uint8_t *p_at_evt = &p_packed_data[4];
    uint16_t at_evt_len = data_len;
    app_wifi_transfer_handle_at_evts(p_at_evt, at_evt_len);

    APP_PRINT_INFO1("app_wifi_recv_data_handle: data_len %d", data_len);

}
#endif

T_WIFI_AT_CMD_SEND_RESULT app_wifi_send_at_cmd(T_WIFI_AT_CMD at_cmd)
{
    APP_PRINT_INFO1("app_wifi_send_at_cmd: %d", at_cmd);

    if (at_cmd_state != WIFI_CMD_AT_STATE_IDLE)
    {
        return WIFI_AT_CMD_SEND_RESULT_SEQ_ERR;
    }

    bool send_result = false;
    switch (at_cmd)
    {
    case WIFI_CMD_AT_TEST:
        {
            uint8_t at_test[] = "AT+TEST\r\n";
            send_result = app_wifi_send_send_data(at_test, strlen((const char *)at_test));
        }
        break;
    case WIFI_CMD_AT_WLCONN:
        {
            uint8_t at_wlconn[] = "AT+WLCONN=ssid,Remote_AP,pw,87654321\r\n";
            send_result = app_wifi_send_send_data(at_wlconn, strlen((const char *)at_wlconn));
        }
        break;
    case WIFI_CMD_AT_WLDISCONN:
        {
            uint8_t at_wldisconn[] = "AT+WLDISCONN\r\n";
            send_result = app_wifi_send_send_data(at_wldisconn, strlen((const char *)at_wldisconn));
        }
        break;
    case WIFI_CMD_AT_WLSTARTAP:
        {
            uint8_t at_wlstartap[] = "AT+WLSTARTAP=ssid,AI_Record_Pen,pw,87654321\r\n";
            send_result = app_wifi_send_send_data(at_wlstartap, strlen((const char *)at_wlstartap));
        }
        break;
    case WIFI_CMD_AT_WLSTOPAP:
        {
            uint8_t at_wlstopap[] = "AT+WLSTOPAP\r\n";
            send_result = app_wifi_send_send_data(at_wlstopap, strlen((const char *)at_wlstopap));
        }
        break;

    default:
        break;
    }

#if 0
    if (send_result)
    {
        at_cmd_state = WIFI_CMD_AT_STATE_BUSY;
    }
#endif
    return send_result ? WIFI_AT_CMD_SEND_RESULT_SUCCESS : WIFI_AT_CMD_SEND_RESULT_SPI_ERR;
}

void app_wifi_trans_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                                   uint16_t cmd_len, uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    APP_PRINT_TRACE3("app_spi_handle_cmd_set: cmd_id 0x%04x, cmd_len 0x%04x, cmd_path %u",
                     cmd_id, cmd_len, cmd_path);

    switch (cmd_id)
    {
    case CMD_AT_CMD:
        {
            if (at_cmd_state != WIFI_CMD_AT_STATE_IDLE)
            {
                ack_pkt[2] = CMD_SET_STATUS_BUSY;
            }

            if (app_wifi_send_at_cmd((T_WIFI_AT_CMD)cmd_ptr[2]) != WIFI_AT_CMD_SEND_RESULT_SUCCESS)
            {
                ack_pkt[2] = CMD_SET_STATUS_PROCESS_FAIL;
            }

            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;

    default:
        break;
    }
}
#endif
