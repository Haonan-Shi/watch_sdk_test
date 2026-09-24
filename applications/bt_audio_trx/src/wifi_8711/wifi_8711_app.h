/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_8711_APP_H_
#define _WIFI_8711_APP_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Events handled by the dedicated wifi_8711 task.
 *
 * The first two values mirror IO_SPI_MSG_TYPE (app_spi_common.h) one-to-one,
 * so app_spi_msg_send() can post an SPI subtype straight through as the event
 * id. ATCMD_FLOW_CTRL wakes the AT-over-SPI flow control (folded in from the
 * old self-contained atcmd flow task).
 */
typedef enum
{
    WIFI_8711_EVENT_SPI_MASTER_DATA_IN = 0,
    WIFI_8711_EVENT_SPI_MASTER_TRIGGER = 1,
    WIFI_8711_EVENT_ATCMD_FLOW_CTRL    = 2,
    /* Run msg.msg_cb on the wifi_8711 task. Lets callers defer work that must
     * NOT run in a BLE/GATT callback (e.g. the ~2s 8711 power-on settle before
     * the first AT+WLCONN) onto this task, where a blocking os_delay is safe. */
    WIFI_8711_EVENT_USER_CB            = 3,
} T_WIFI_8711_EVENT;

typedef struct
{
    uint16_t event;             /* T_WIFI_8711_EVENT                            */
    void    *buf;               /* SPI rx_msg ptr for *_DATA_IN, else user arg  */
    void (*msg_cb)(void *);     /* callback for WIFI_8711_EVENT_USER_CB, else 0 */
} T_WIFI_8711_MSG;

/**
 * @brief Post a message to the dedicated wifi_8711 task queue.
 * @return true on success, false if the queue is full / not created yet.
 */
bool app_send_msg_to_wifi_8711_task(T_WIFI_8711_MSG *p_msg);

/**
 * @brief Create the wifi_8711 task + queue and initialise the AT-over-SPI
 *        engine and the SPI master. Called explicitly from app_main()
 *        during app init (bt_audio_trx has no APP_MODULE_INIT registry, unlike
 *        the watch port), replacing the individual init calls that used to live
 *        in app/app_main.c.
 */
void wifi_8711_init(void);

/**
 * @brief Power the external 8711 Wi-Fi IC on by driving WIFI_EN (P0_0) high.
 *
 *  8773GTP record-pen wiring: the 8711's CHIP_EN is tied to 3V3 (asserted with
 *  system power, no MCU control), so WIFI_EN is the only software power gate.
 *  This only toggles the pin (fast, non-blocking); the module needs ~2s to boot
 *  afterwards, so the caller must wait before issuing the first AT command
 *  (see the CMD_WIFI_CONNECT bring-up path). Idempotent.
 */
void wifi_8711_power_on(void);

/** @brief Power the 8711 down by driving WIFI_EN (P0_0) low. */
void wifi_8711_power_down(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIFI_8711_APP_H_ */
