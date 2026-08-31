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
#define SPI_XMIT_SIZE                   (16*1024)
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

void app_spi_master_init(void);
void app_spi_master_rx_parser(void *rx_ctx);
uint8_t app_spi_master_send_raw_data(uint8_t *p_data, uint16_t len);
uint8_t app_spi_master_send_raw_data_trigger(void);
void app_spi_master_module_register(P_SPI_PARSER func, uint16_t mode);

/* Zero-copy "prefilled" TX path (used by the throughput blast). Instead of
 * staging a frame in PSRAM and copying it through the TX ring (3x 16K PSRAM
 * touches per frame), the producer builds the frame straight into the SRAM DMA
 * buffer returned by app_spi_master_tx_dma_buf() and then hands it off with
 * app_spi_master_send_prefilled(). There is a single DMA buffer, so the producer
 * MUST call app_spi_master_tx_wait_free() (blocks until the previous prefilled
 * frame has finished on the wire) before writing the buffer for the next frame.
 *   uint8_t *buf = app_spi_master_tx_dma_buf();
 *   app_spi_master_tx_wait_free();     // buffer now free to overwrite
 *   ... fill buf[0..len) ...
 *   app_spi_master_send_prefilled(len);
 */
uint8_t *app_spi_master_tx_dma_buf(void);
void     app_spi_master_tx_wait_free(void);
uint8_t  app_spi_master_send_prefilled(uint16_t len);
/* Return the buffer token taken by tx_wait_free() without sending (EOF/error
 * after the buffer was already acquired). Keeps the sem pairing balanced. */
void     app_spi_master_tx_release(void);

/* Debug: set the SPI rx-sample-delay at runtime (forwards to the rtl87x3g SPI
 * driver). Used by the rsd auto-sweep to hunt the correct sample point at high
 * SCK without rebuilding per value. */
void app_spi_master_set_rx_sample_dly(uint32_t delay);

/* TX packet progress logging (used by the SPI+TCP blast test): begin() arms the
 * "cur/total" counter printed by app_spi_master_send_raw_data_trigger() for each
 * real frame popped from the ring; end() disarms it. total_pkts==0 disables. */
void app_spi_master_tx_progress_begin(uint32_t total_pkts);
void app_spi_master_tx_progress_end(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _APP_SPI_API_H_ */
