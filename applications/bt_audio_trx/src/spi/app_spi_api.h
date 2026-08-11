/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_SPI_API_H_
#define _APP_SPI_API_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "string.h"
#include "rtl876x.h"
#include "btm.h"
#if F_APP_WIFI_SPI_CMD
#define SPI_XMIT_SIZE                   704
#else
#define SPI_XMIT_SIZE                   704
#endif
#define SPI_CMD_SYNC_INVALID            0xFF

typedef enum
{
    SPI_SEND_SUC,
    SPI_SEND_ERR_LEN,
    SPI_SEND_ERR_BUSY,
    SPI_SEND_ERR_NOMEM,
    SPI_SEND_ERR_UNSUPPORTED,
} T_SPI_SEND_ERR_CODE;

typedef void (*P_SPI_PARSER)(uint8_t *p_data, uint16_t data_len);

void app_spi_cmd_parser_init(void);
uint8_t app_spi_send_cmd(uint16_t cmd_id, uint8_t *p_data, uint16_t len);

void app_spi_master_init(void);
void app_spi_master_rx_parser(void *rx_ctx);
uint8_t app_spi_master_send_raw_data(uint8_t *p_data, uint16_t len);
uint8_t app_spi_master_send_raw_data_trigger(void);
void app_spi_master_module_register(P_SPI_PARSER func, uint16_t mode);

void app_spi_slave_init(void);
void app_spi_slave_rx_parser(void *rx_ctx);
uint8_t app_spi_slave_listen_async(void);
uint8_t app_spi_slave_send_raw_data(uint8_t *p_data, uint16_t len);
void app_spi_slave_module_register(P_SPI_PARSER cb_func, uint16_t mode);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_SPI_API_H_ */
