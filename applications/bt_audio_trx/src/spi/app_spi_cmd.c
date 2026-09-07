/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if (F_APP_SPI_ROLE_MASTER || F_APP_SPI_ROLE_SLAVE)
#include <zephyr/drivers/spi.h>
#include "app_spi_cmd.h"
#include "trace.h"
#include "app_spi_api.h"
#include "app_cmd.h"
#include "app_util.h"

/*SPI Seq*/
static uint8_t spi_tx_seq = 0;

void app_spi_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                            uint16_t cmd_len, uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));;

    APP_PRINT_TRACE3("app_spi_handle_cmd_set: cmd_id 0x%04x, cmd_len 0x%04x, cmd_path %u",
                     cmd_id, cmd_len, cmd_path);

    switch (cmd_id)
    {
    case CMD_SPI_INIT:
        {
            uint8_t role = cmd_ptr[2];
            if (role != SPI_ROLE_MASTER && role != SPI_ROLE_SLAVE)
            {
                ack_pkt[2] = CMD_SET_STATUS_PARAMETER_ERROR;
            }

#if F_APP_SPI_ROLE_MASTER
            if (role == SPI_ROLE_MASTER)
            {
                app_spi_master_init();
            }
#endif
#if F_APP_SPI_ROLE_SLAVE
            if (role == SPI_ROLE_SLAVE)
            {
                app_spi_slave_init();
            }
#endif
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;

    default:
        break;
    }
}

void app_spi_cmd_parser_cb(uint8_t *p_data, uint16_t data_len)
{
    // Note: data_len will be the mtu of SPI, not the actual cmd len
    uint8_t check_sum = 0;
    uint8_t rx_seq = 0;
    uint16_t cmd_len = 0;
    uint16_t checksum_data_len;

    if (p_data == NULL)
    {
        return;
    }

    typedef struct
    {
        uint8_t sync_byte;
        uint8_t seq;
        uint16_t pkt_len;
        uint16_t cmd_id;
        uint8_t data[0];
    }   __attribute__((packed)) T_RECV_BUF;
    T_RECV_BUF *cmd = (T_RECV_BUF *)p_data;

    if (cmd->sync_byte != CMD_SYNC_BYTE)
    {
        APP_PRINT_INFO0("app_spi_cmd_parser_cb: invalid pkt");
        return;
    }

    checksum_data_len = sizeof(cmd->seq) + sizeof(cmd->pkt_len) + cmd->pkt_len;
    if (checksum_data_len + 2 > data_len)
    {
        APP_PRINT_INFO0("app_spi_cmd_parser_cb: length error");
        return;
    }
    check_sum = app_util_calc_checksum((uint8_t *)&cmd->seq, checksum_data_len);
    if (p_data[1 + checksum_data_len] != check_sum)
    {
        APP_PRINT_ERROR0("app_spi_cmd_parser_cb: checksum err");
        return;
    }

    app_handle_cmd_set((uint8_t *)&cmd->cmd_id, cmd->pkt_len, CMD_PATH_SPI, cmd->seq, 0);
}

uint8_t app_spi_send_cmd(uint16_t cmd_id, uint8_t *p_data, uint16_t len)
{
    uint16_t total_len;
    uint8_t check_sum;
    uint8_t *p_buf;
    uint16_t checksum_data_len;

    typedef struct
    {
        uint8_t sync_byte;
        uint8_t seq;
        uint16_t pkt_len;
        uint16_t cmd_id;
        uint8_t data[0];
    }   __attribute__((packed)) T_SEND_BUF;

    total_len = sizeof(T_SEND_BUF) + len + sizeof(check_sum);
    T_SEND_BUF *send_buf = malloc(total_len);
    if (send_buf == NULL)
    {
        return 1; // TODO: error code
    }
    p_buf = (uint8_t *)send_buf + sizeof(send_buf->sync_byte);
    checksum_data_len = total_len - sizeof(send_buf->sync_byte) - sizeof(check_sum);

    spi_tx_seq++;
    if (spi_tx_seq == 0)
    {
        spi_tx_seq = 1;
    }

    send_buf->sync_byte = CMD_SYNC_BYTE;
    send_buf->seq = spi_tx_seq;
    send_buf->pkt_len = len + sizeof(send_buf->cmd_id);
    send_buf->cmd_id = cmd_id;
    if (len)
    {
        memcpy(send_buf->data, p_data, len);
    }
    check_sum = app_util_calc_checksum(p_buf, checksum_data_len);
    send_buf->data[len] = check_sum;

#if F_APP_SPI_ROLE_MASTER
    app_spi_master_send_raw_data((uint8_t *)send_buf, total_len);
#endif

#if F_APP_SPI_ROLE_SLAVE
    app_spi_slave_send_raw_data((uint8_t *)send_buf, total_len);
#endif

    free(send_buf);
    return 0;
}

void app_spi_cmd_parser_init(void)
{
#if F_APP_SPI_ROLE_MASTER
    app_spi_master_module_register(app_spi_cmd_parser_cb, SPI_MODE_CPOL | SPI_MODE_CPHA);
#endif

#if F_APP_SPI_ROLE_SLAVE
    app_spi_slave_module_register(app_spi_cmd_parser_cb, SPI_MODE_CPOL | SPI_MODE_CPHA);
#endif
}


#endif
