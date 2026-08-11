/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_SDIO_MEM_TX_H_
#define _WIFI_SDIO_MEM_TX_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Memory-sourced SDIO TX pump for the wifi data-uplink test.
 *
 * Mirrors the flow of module/wifi/wifi_sdio_tx_test.c (sdio_tx_proc / sdio_tx_buf)
 * -- it streams fixed-size chunks into the WiFi chip's SDIO write queue and
 * re-arms itself on the wifi task -- but the data source is a fixed in-RAM
 * pattern buffer instead of an SD-card file (no fs_open / fs_read). The original
 * file is left untouched; all symbols here are gm-prefixed to avoid clashing
 * with it (both are linked into `app`).
 */
void wifi_sdio_mem_tx_start(void);
void wifi_sdio_mem_tx_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_SDIO_MEM_TX_H_ */
