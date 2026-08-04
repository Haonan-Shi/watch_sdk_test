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
 * The first four values mirror IO_SPI_MSG_TYPE (app_spi_common.h) one-to-one,
 * so app_spi_msg_send() can post an SPI subtype straight through as the event
 * id. ATCMD_FLOW_CTRL wakes the AT-over-SPI flow control (folded in from the
 * old self-contained atcmd flow task).
 */
typedef enum
{
    WIFI_8711_EVENT_SPI_MASTER_DATA_IN = 0,
    WIFI_8711_EVENT_SPI_SLAVE_DATA_IN  = 1,
    WIFI_8711_EVENT_SPI_SLAVE_TRIGGER  = 2,
    WIFI_8711_EVENT_SPI_MASTER_TRIGGER = 3,
    WIFI_8711_EVENT_ATCMD_FLOW_CTRL    = 4,
} T_WIFI_8711_EVENT;

typedef struct
{
    uint16_t event;    /* T_WIFI_8711_EVENT                                */
    void    *buf;      /* SPI rx_msg pointer for *_DATA_IN events, else NULL */
} T_WIFI_8711_MSG;

/**
 * @brief Post a message to the dedicated wifi_8711 task queue.
 * @return true on success, false if the queue is full / not created yet.
 */
bool app_send_msg_to_wifi_8711_task(T_WIFI_8711_MSG *p_msg);

/**
 * @brief Create the wifi_8711 task + queue and initialise the AT-over-SPI
 *        engine and the SPI master. Registered with APP_MODULE_INIT, so it runs
 *        from app_module_init_all() during boot.
 */
void wifi_8711_init(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _WIFI_8711_APP_H_ */
