/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _WIFI_BASIC_H_
#define _WIFI_BASIC_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Inbound command topics. Module subscribes to "wifi/cmd/ *" async
 * (event_bus_async_send_to_apptask), so the callback runs on the APP TASK. */
#define EVENT_BUS_TOPIC_WIFI_ALL_TOPIC      "wifi/cmd/*"
#define EVENT_BUS_TOPIC_WIFI_CMD_ENTER      "wifi/cmd/enter"
#define EVENT_BUS_TOPIC_WIFI_CMD_EXIT       "wifi/cmd/exit"
#define EVENT_BUS_TOPIC_WIFI_CMD_SCAN       "wifi/cmd/scan"
#define EVENT_BUS_TOPIC_WIFI_CMD_CONNECT    "wifi/cmd/connect"
#define EVENT_BUS_TOPIC_WIFI_CMD_IPERF_ENTER "wifi/cmd/iperf_enter"
#define EVENT_BUS_TOPIC_WIFI_CMD_IPERF_UPLINK     "wifi/cmd/iperf_uplink"
#define EVENT_BUS_TOPIC_WIFI_CMD_IPERF_DOWNLINK   "wifi/cmd/iperf_downlink"
/* Data path is split into "enter" (bring up the TCP server via
 * cmd_wifi_data_test_enter) and "start" (kick the SDIO pump / arm rx-rate),
 * so the host can connect a TCP client in between. */
#define EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_UPLINK_ENTER     "wifi/cmd/socket_uplink_enter"
#define EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_UPLINK_START     "wifi/cmd/socket_uplink_start"
#define EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_DOWNLINK_ENTER   "wifi/cmd/socket_downlink_enter"
#define EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_DOWNLINK_START   "wifi/cmd/socket_downlink_start"
#define EVENT_BUS_TOPIC_WIFI_CMD_DISCONNECT "wifi/cmd/disconnect"
#define EVENT_BUS_TOPIC_WIFI_CMD_STATUS     "wifi/cmd/status"
#define EVENT_BUS_TOPIC_WIFI_CMD_GET_IP     "wifi/cmd/get_ip"

/* Payload for CONNECT. */
typedef struct
{
    char ssid[64];
    char password[64];
} T_WIFI_CONNECT_DATA;

/* Payload for UPLINK (upload / client side). */
typedef struct
{
    char server_ip[32];
} T_WIFI_UPLINK_DATA;

/* Default TCP port for the SDIO data-path test (ATPS server on the device).
 * The host-side test tool connects to <device_ip>:<port>. Keep in sync with
 * the test runner's wifi.data_port (config.yaml). */
#define WIFI_DATA_DEFAULT_PORT   5001

/* Payload for DATA uplink/downlink: the local TCP port the device listens on. */
typedef struct
{
    uint16_t port;
} T_WIFI_DATA_DATA;

#ifdef __cplusplus
}
#endif

#endif /* _WIFI_BASIC_H_ */
