/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * @file    app_spi_sd_source.h
 * @brief   SD-card data source for the SPI(+TCP) uplink throughput test.
 *
 * Replaces the synthetic uplink payload (the 0..127 blast ramp / the 'A' chunk
 * filler) with real bytes read from a file on the SD card. On this board the
 * card is sdhc0 -> sdcard_disk0 (zephyr,sdmmc-disk), mounted as "/SD:".
 *
 * Only one file is open at a time (the uplink test is strictly serial). open()
 * disables SD idle power-down for the whole read session and close() re-enables
 * it, mirroring wifi_sdio_tx_test.c.
 *
 * Decoupling note: the blast TX task in app_spi_atcmd.c chooses its payload
 * source at runtime by calling app_spi_sd_source_is_open() - if the higher-level
 * test opened a file, the task streams it; otherwise it falls back to the ramp.
 * That keeps the generic AT engine free of any dependency on the test header.
 */

#ifndef _APP_SPI_SD_SOURCE_H_
#define _APP_SPI_SD_SOURCE_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open @p path on the SD card for reading and (optionally) report its
 *        size. Disables SD idle power-down until app_spi_sd_source_close().
 *        If a file is already open it is closed first.
 *
 * @param path      NUL-terminated file path, e.g. "/SD:/audio/test.bin".
 * @param out_size  If non-NULL, receives the file size in bytes on success.
 * @return true on success (file open, size resolved), false otherwise.
 */
bool app_spi_sd_source_open(const char *path, uint32_t *out_size);

/**
 * @brief Read up to @p len bytes of the open file into @p buf.
 * @return >0 : bytes read; 0 : end of file; <0 : error / no file open.
 */
int app_spi_sd_source_read(uint8_t *buf, uint32_t len);

/**
 * @brief Close the open file (if any) and re-enable SD idle power-down.
 *        Idempotent: safe to call when nothing is open.
 */
void app_spi_sd_source_close(void);

/** @brief true while a file opened by app_spi_sd_source_open() is still open. */
bool app_spi_sd_source_is_open(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_SPI_SD_SOURCE_H_ */
