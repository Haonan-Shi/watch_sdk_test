/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_SDIO_RX_H_
#define _WIFI_SDIO_RX_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Downlink (host -> SoC) average-rate measurement for the wifi data test.
 *
 * The host controls the total amount of data. The SoC cannot know that total in
 * advance, so it times the transfer itself: it starts the clock on the first
 * forwarded packet and stops it when the stream goes idle (no packet for
 * WIFI_RX_IDLE_TIMEOUT_MS), then prints one average rate.
 *
 * This deliberately does NOT touch module/wifi/wifi_sdio.c. It hooks the byte
 * stream through the public callback interface (wifi_sdio_data_read_cb_reg),
 * exactly as the camera RTSP path does, keeping the shared SDIO module clean.
 */

/* Register the SDIO read callback and arm measurement for one downlink run. */
void wifi_sdio_rx_start(void);

/* Unregister the callback; flushes any in-progress measurement first. */
void wifi_sdio_rx_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_SDIO_RX_H_ */
