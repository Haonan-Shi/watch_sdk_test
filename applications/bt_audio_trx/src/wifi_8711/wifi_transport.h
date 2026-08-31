/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_TRANSPORT_H_
#define _WIFI_TRANSPORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* Max number of AT-event listeners in the broadcast dispatch array.
 * May be pre-defined by app_spi_atcmd.h (value 4); default to 8 here. */
#ifndef WIFI_TRANSPORT_MAX_CBS
#define WIFI_TRANSPORT_MAX_CBS          8
#endif

/* AT event types (mirrors T_AT_EVT_TYPE values; shared across transports) */
typedef enum
{
    WIFI_AT_EVT_CMD_RESPONSE,        /* Command response (OK/ERROR) */
    WIFI_AT_EVT_WIFI_CONNECTED,      /* WiFi connected */
    WIFI_AT_EVT_WIFI_DISCONNECTED,   /* WiFi disconnected */
    WIFI_AT_EVT_WIFI_GOT_IP,         /* Got IP address */
    WIFI_AT_EVT_UNKNOWN_DATA,        /* Unknown / unsolicited data */
} wifi_at_evt_type_t;

/* AT command response state */
typedef enum
{
    WIFI_AT_RSP_OK    = 0,
    WIFI_AT_RSP_ERROR = 1,
} wifi_at_rsp_state_t;

/* AT event callback: all registered listeners receive every event */
typedef void (*wifi_at_evt_cb_t)(wifi_at_evt_type_t evt, void *p_data, uint16_t len);

/* Wi-Fi transport vtable (one instance per transport backend) */
typedef struct
{
    const char *name;

    /** @brief Initialise the transport hardware */
    bool (*init)(void);

    /** @brief Register an AT-event callback (broadcast dispatch) */
    bool (*register_callback)(wifi_at_evt_cb_t cb);

    /** @brief Queue an AT command for sending */
    bool (*queue_fill)(uint8_t cmd_type, const char *param);

    /** @brief Trigger the send flow (kick the AT engine) */
    void (*trigger_send)(void);

    /** @brief Two-phase transparent send (e.g. AT+SKTSENDRAW) */
    bool (*sendraw)(const char *cmd_line, const uint8_t *raw, uint16_t raw_len);

    /** @brief Initialise the file-upload sub-module */
    int (*file_upload_init)(void *cfg);

    /** @brief Is a file upload currently in progress? */
    bool (*file_upload_is_busy)(void);

    /** @brief Restore clocks after a file upload completes */
    void (*file_upload_restore_clk)(void);

    /** @brief Power on the external Wi-Fi chip */
    void (*power_on)(void);

    /** @brief Power down the external Wi-Fi chip */
    void (*power_down)(bool disable_pin);
} wifi_transport_ops_t;

/* Return the transport ops for the currently compiled platform */
const wifi_transport_ops_t *wifi_transport_get(void);

/* Convenience: register a callback on whatever transport is active */
bool wifi_transport_register_callback(wifi_at_evt_cb_t cb);

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_TRANSPORT_H_ */
