/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef APP_TRANSFER_PATH_MSG_H
#define APP_TRANSFER_PATH_MSG_H

#include <stdbool.h>
#include <stdint.h>
/* app_spi_msg_send() posts SPI events to the dedicated wifi_8711 task
 * (app_send_msg_to_wifi_8711_task(), see app_spi_common.c / wifi_8711_app.c)
 * rather than the shared app task. app_msg.h / os_msg.h are kept for the other
 * sources that pull them in transitively through this header. */
#include "app_msg.h"
#include "os_msg.h"

#define APP_SPI_USE_FIXED_DMA_CHANNEL   1
#define APP_SPI_FIXED_CHANNEL           8

#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
#undef APP_SPI_USE_FIXED_DMA_CHANNEL
#define APP_SPI_USE_FIXED_DMA_CHANNEL   0
#endif

typedef enum
{
    IO_SPI_MASTER_DATA_IN,
    IO_SPI_MASTER_TRIGGER,
} IO_SPI_MSG_TYPE;

typedef enum
{
    SPI_GPIO_STATE_INACTIVE,
    SPI_GPIO_STATE_ACTIVE,
} SPI_GPIO_STATE;

/* Reference bits for the SPI DLPS lock (app_spi_disable_dlps / _enable_dlps).
 * APP_DLPS_ENTER_CHECK_SPI is a single hardware bit, so these form a software
 * refcount over it: DLPS is only actually re-allowed once EVERY holder has
 * released. APP_SPI_XFER_BIT is taken/released per SPI transaction by the master
 * driver; APP_SPI_BLAST_BIT is held for the whole duration of a throughput blast
 * so the per-frame XFER churn never briefly clears the hardware lock while the
 * blast is still streaming (a mid-transfer DLPS entry reconfigures the SPI1
 * clock and loses the in-flight DMA completion). */
#define APP_SPI_TX_BIT                0x00000001
#define APP_SPI_RX_BIT                0x00000002
#define APP_SPI_XFER_BIT              0x00000004   /* one in-flight SPI transfer */
#define APP_SPI_BLAST_BIT             0x00000008   /* a TP uplink blast is active */

void app_spi_disable_dlps(uint8_t bit);
void app_spi_enable_dlps(uint8_t bit);
bool app_spi_is_transmit_done(void);
bool app_spi_msg_send(IO_SPI_MSG_TYPE subtype, void *param_buf);
void app_spi_clear_rx_fifo(void);
/* Dispatch one SPI event on the dedicated wifi_8711 task. @p subtype is an
 * IO_SPI_MSG_TYPE value; @p buf is the SPI rx_msg for the *_DATA_IN subtypes. */
void app_spi_msg_handle(uint8_t subtype, void *buf);

#endif
