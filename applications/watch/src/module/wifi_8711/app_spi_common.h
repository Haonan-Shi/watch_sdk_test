/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_TRANSFER_PATH_MSG_H
#define APP_TRANSFER_PATH_MSG_H

#include <stdbool.h>
#include <stdint.h>

#define APP_SPI_USE_FIXED_DMA_CHANNEL   1
#define APP_SPI_FIXED_CHANNEL           8

#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
#undef APP_SPI_USE_FIXED_DMA_CHANNEL
#define APP_SPI_USE_FIXED_DMA_CHANNEL   0
#endif

typedef enum
{
    IO_SPI_MASTER_DATA_IN,
    IO_SPI_SLAVE_DATA_IN,
    IO_SPI_SLAVE_TRIGGER,
    IO_SPI_MASTER_TRIGGER,
} IO_SPI_MSG_TYPE;

typedef enum
{
    SPI_GPIO_STATE_INACTIVE,
    SPI_GPIO_STATE_ACTIVE,
} SPI_GPIO_STATE;

#define APP_SPI_TX_BIT                0x00000001
#define APP_SPI_RX_BIT                0x00000002

void app_spi_disable_dlps(uint8_t bit);
void app_spi_enable_dlps(uint8_t bit);
bool app_spi_is_transmit_done(void);
bool app_spi_msg_send(IO_SPI_MSG_TYPE subtype, void *param_buf);
void app_spi_clear_rx_fifo(void);
/* Dispatch one SPI event on the dedicated wifi_8711 task. @p subtype is an
 * IO_SPI_MSG_TYPE value; @p buf is the SPI rx_msg for the *_DATA_IN subtypes. */
void app_spi_msg_handle(uint8_t subtype, void *buf);

#endif
