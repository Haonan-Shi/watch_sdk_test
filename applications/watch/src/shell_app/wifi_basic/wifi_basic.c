/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */
/*
 * Decoupling model:
 *   - Shell handlers run on the SHELL thread and ONLY event_bus_publish() to
 *     this module's own "wifi/cmd/ *" topic.
 *   - That topic is subscribed ASYNC (event_bus_async_send_to_apptask), so the
 *     command callback runs on the APP TASK.
 *   - "enter" dispatches further to the WIFI TASK via app_send_msg_to_wifitask()
 *     because wifi_enable() contains a 2-second os_delay that must not run on
 *     the app task.
 *   - All other commands call uart_atcmd_queue_fill() + trigger EVENT_UART_CMD_FLOW_CTRL
 *     (these are non-blocking queue operations, safe to call from app task).
 *   - Module init auto-registers at boot via APP_MODULE_INIT (no main.c edits).
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/shell/shell.h>

#include "event_bus.h"
#include "app_task.h"          /* event_bus_async_send_to_apptask */
#include "app_module_init.h"   /* APP_MODULE_INIT */
#include "pm.h"

#include "wifi_app.h"          /* T_WIFI_MSG, app_send_msg_to_wifitask, wifi_enable */
#include "wifi_atcmd.h"        /* uart_atcmd_queue_fill, uart_atcmd_init,
                                  cmd_wifi_set_client, cmd_wifi_set_server */
#include "wifi_sdio.h"         /* wifi_sdio_init */
#include "wifi_uart.h"         /* wifi_uart_init */

#include "wifi_basic.h"
#include "wifi_sdio_mem_tx.h"  /* memory-sourced SDIO TX pump (socket uplink) */
#include "wifi_sdio_rx.h"      /* downlink rx average-rate (via read callback) */

#include "rtl876x_uart.h"      /* HAL: loopback + line-status, for overrun repro */
#include "wdg.h"

static T_EVENT_BUS_SUBSCRIBER_HANDLE s_cmd_h;

static bool s_wifi_entered;
static bool s_wifi_connected;
static char s_connected_ssid[64];
/* SSID of the in-flight "connect" (ATPN). Promoted to s_connected_ssid only
 * when the real "[ATPN] OK" reply arrives (see wifi_connect_result_cb), so the
 * connected state is never set optimistically at command-issue time. */
static char s_pending_ssid[64];

/* SDIO data-path test state. The device is a single TCP server
   shared by both directions, so ATPS is started only once per session;
 * s_socket_uplink_active tracks whether the SDIO TX pump / SD-card file is open. */
static bool s_data_server_up;
static bool s_socket_uplink_active;
static bool s_socket_downlink_active;   /* downlink rx-rate measurement armed */

/* iperf (no-SDIO) test mode: set from the real "[ATST] OK" reply to ATST=3. */
static bool s_iperf_mode;

/* ---------- wifi-task callback for "enter" (runs in WIFI TASK) ---------- */

static void wifi_enable_proc(void *msg)
{
    (void)msg;
    wifi_enable(true);       /* GPIO drive + 2 s hardware delay */
    wifi_sdio_init();
    wifi_uart_init();
    // uart_atcmd_init();   /* enabled in wifi_uart_init() */
    s_wifi_entered = true;
    printk("[wifi] WiFi hardware enabled and initialized\n");
}

static void wifi_disable_proc(void *msg)
{
    (void)msg;
    wifi_enable(false);       /* GPIO drive + 2 s hardware delay */
    s_wifi_entered = false;
    printk("[wifi] WiFi hardware disabled and deinitialized\n");
}


/* ---------- scan result sink (runs via ATWS response callback) ---------- */

static bool wifi_scan_result_cb(char *line)
{
    /* Lines delivered here (see ATWS hook in uart_atcmd_rsp_handler):
     *   per-AP : "AP : <n>,<ssid>,<chan>,<sec>,<rssi>,<bssid>"
     *   final  : "[ATWS] OK"  (or "[ATWS] ERROR:n") -- scan complete
     */
    if (line == NULL)
    {
        return true;
    }

    if (strncmp(line, "AP :", 4) != 0)
    {
        printk("[wifi] scan done: %s\n", line);
        return true;
    }

    /* SSID = field after the first comma. */
    char *ssid = strchr(line, ',');
    if (ssid == NULL)
    {
        return true;
    }
    ssid++;  /* skip comma */

    /* RSSI = field just before the last (BSSID) field. Parse from the right so
     * an SSID that itself contains a comma cannot shift the field index
     * (BSSID is always last and uses ':' separators, never ','). */
    int rssi = 0;
    char *bssid_comma = strrchr(line, ',');
    if (bssid_comma != NULL && bssid_comma > ssid)
    {
        char *p = bssid_comma - 1;
        while (p > ssid && *p != ',')
        {
            p--;
        }
        if (*p == ',')
        {
            rssi = atoi(p + 1);
        }
    }

    /* Copy SSID (up to the next comma) into a bounded buffer for printing. */
    char ssid_buf[33];
    char *ssid_end = strchr(ssid, ',');
    size_t len = ssid_end ? (size_t)(ssid_end - ssid) : strlen(ssid);
    if (len > sizeof(ssid_buf) - 1)
    {
        len = sizeof(ssid_buf) - 1;
    }
    memcpy(ssid_buf, ssid, len);
    ssid_buf[len] = '\0';

    printk("[wifi] SSID=%-32s RSSI=%d\n", ssid_buf, rssi);
    return true;
}

/* Parse the "ATW?" reply for the current IP and gateway (= hotspot IP).
 * Lines delivered here (see ATW? hook / rsp_ATWINFO in wifi_atcmd.c):
 *   info  : "STA,<ssid>,<chan>,<sec>,...,<mac>,<ip>,<gw>"
 *   final : "[ATW?] OK"
 * GW is the last comma field and IP the one before it; parse from the right so
 * earlier fields (or an SSID/security string containing a comma) cannot shift
 * the index. */
static bool wifi_info_result_cb(char *line)
{
    if (line == NULL)
    {
        return true;
    }

    if (line[0] == '[')   /* "[ATW?] OK" / "[ATW?] ERROR:n" terminator */
    {
        return true;
    }

    char *gw_comma = strrchr(line, ',');
    if (gw_comma == NULL)
    {
        return true;
    }

    char *ip_comma = gw_comma - 1;
    while (ip_comma > line && *ip_comma != ',')
    {
        ip_comma--;
    }
    if (*ip_comma != ',')
    {
        return true;
    }

    char ip_buf[16];
    char gw_buf[16];
    size_t ip_len = (size_t)(gw_comma - (ip_comma + 1));
    size_t gw_len = strlen(gw_comma + 1);
    if (ip_len > sizeof(ip_buf) - 1)
    {
        ip_len = sizeof(ip_buf) - 1;
    }
    if (gw_len > sizeof(gw_buf) - 1)
    {
        gw_len = sizeof(gw_buf) - 1;
    }
    memcpy(ip_buf, ip_comma + 1, ip_len);
    ip_buf[ip_len] = '\0';
    memcpy(gw_buf, gw_comma + 1, gw_len);
    gw_buf[gw_len] = '\0';

    printk("[wifi] IP=%s  hotspot(GW)=%s\n", ip_buf, gw_buf);
    return true;
}

/* Parse the "ATWQ" reply for just the local IP.
 * The single reply line is "[ATWQ] IP => <ip>"; extract the address after "=>".
 * Prints the same "[wifi] IP=<ip>" line the runner already greps for. */
static bool wifi_ip_result_cb(char *line)
{
    if (line == NULL)
    {
        return true;
    }

    char *p = strstr(line, "=>");
    if (p == NULL)
    {
        return true;   /* not the IP line */
    }
    p += 2;
    while (*p == ' ')
    {
        p++;
    }

    unsigned int a, b, c, d;
    if (sscanf(p, "%u.%u.%u.%u", &a, &b, &c, &d) == 4)
    {
        printk("[wifi] IP=%u.%u.%u.%u\n", a, b, c, d);
    }
    return true;
}

/* ---------- flow-flag callbacks: set state from the REAL atcmd reply ---------- *
 * These run on the WIFI TASK (uart_atcmd_rsp_handler -> rsp_xxx -> cb), the same
 * context as the scan/info/ip parsers above. They flip the shell's flow flags
 * based on the actual "[ATxx] OK/ERROR" reply line instead of optimistically at
 * command-issue time. Each also prints the raw reply so it is visible on the
 * shell console (APP_PRINT_* goes to the trace UART, not uart2). */

static bool wifi_connect_result_cb(char *line)
{
    if (line == NULL)
    {
        return true;
    }
    printk("[wifi] connect reply: %s\n", line);

    /* ATPN terminator: "[ATPN] OK" on success, "[ATPN] ERROR:n" on failure. */
    if (strstr(line, "ERROR") != NULL)
    {
        s_wifi_connected = false;
        s_connected_ssid[0] = '\0';
        printk("[wifi] connect failed (per ATPN reply)\n");
    }
    else if (strstr(line, "OK") != NULL)
    {
        strncpy(s_connected_ssid, s_pending_ssid, sizeof(s_connected_ssid) - 1);
        s_connected_ssid[sizeof(s_connected_ssid) - 1] = '\0';
        s_wifi_connected = true;
        printk("[wifi] connected to %s (per ATPN reply)\n", s_connected_ssid);
    }
    return true;
}

static bool wifi_disconnect_result_cb(char *line)
{
    if (line == NULL)
    {
        return true;
    }
    printk("[wifi] disconnect reply: %s\n", line);

    /* Any "[ATWD]" terminator means "no longer associated". */
    s_wifi_connected = false;
    s_connected_ssid[0] = '\0';
    return true;
}

static bool wifi_data_server_result_cb(char *line)
{
    if (line == NULL)
    {
        return true;
    }
    printk("[wifi] data server reply: %s\n", line);

    /* ATPS terminator: "[ATPS] OK" (listening) or "[ATPS] ERROR:n". Only mark the
     * shared TCP server up on a real OK. */
    if (strstr(line, "ERROR") != NULL)
    {
        s_data_server_up = false;
        printk("[wifi] data server start failed (per ATPS reply)\n");
    }
    else if (strstr(line, "OK") != NULL)
    {
        s_data_server_up = true;
        printk("[wifi] data server up (per ATPS reply)\n");
    }
    return true;
}

static bool wifi_iperf_mode_result_cb(char *line)
{
    if (line == NULL)
    {
        return true;
    }
    printk("[wifi] iperf mode reply: %s\n", line);

    /* ATST=3 terminator: "[ATST] OK <mode>" on success, "[ATST] ERROR:n". */
    if (strstr(line, "ERROR") != NULL)
    {
        s_iperf_mode = false;
        printk("[wifi] iperf mode enable failed (per ATST reply)\n");
    }
    else if (strstr(line, "OK") != NULL)
    {
        s_iperf_mode = true;
        printk("[wifi] iperf mode enabled (per ATST reply)\n");
    }
    return true;
}

static bool wifi_iperf_run_result_cb(char *line)
{
    if (line == NULL)
    {
        return true;
    }
    printk("[wifi] iperf run reply: %s\n", line);

    /* ATWT terminator: "[ATWT] OK" when the iperf client/server command is
     * accepted / the run completes, "[ATWT] ERROR:n" on failure. This is a
     * transient test action, so it only confirms the reply (no persistent flag). */
    if (strstr(line, "ERROR") != NULL)
    {
        printk("[wifi] iperf run failed (per ATWT reply)\n");
    }
    else if (strstr(line, "OK") != NULL)
    {
        printk("[wifi] iperf run ok (per ATWT reply)\n");
    }
    return true;
}

/* ---------- app-task command callback ---------- */

static int32_t wifi_cmd_cb(T_EVENT_BUS_EVENT_DATA *ev)
{
    const char *t = ev->topic;

    if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_ENTER) == 0)
    {
        uint32_t actual = 0;
        pm_cpu_freq_set(200, &actual);

        /* Dispatch to wifi task because wifi_enable() has a long delay */
        T_WIFI_MSG task_msg;
        memset(&task_msg, 0, sizeof(task_msg));
        task_msg.event   = EVENT_USER_APP_DEFINE;
        task_msg.msg_cb  = wifi_enable_proc;
        if (app_send_msg_to_wifitask(&task_msg) == false)
        {
            printk("[wifi] enter: send to wifi task failed\n");
        }
        else
        {
            printk("[wifi] enter: WiFi init dispatched to wifi task\n");
        }
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_EXIT) == 0)
    {
        /* Stop the SDIO TX pump before the WiFi HW is torn down, and reset the
         * shared-server state. */
        if (s_socket_uplink_active)
        {
            wifi_sdio_mem_tx_stop();
            s_socket_uplink_active = false;
        }
        if (s_socket_downlink_active)
        {
            wifi_sdio_rx_stop();
            s_socket_downlink_active = false;
        }
        s_data_server_up = false;
        s_iperf_mode = false;

        T_WIFI_MSG task_msg;
        memset(&task_msg, 0, sizeof(task_msg));
        task_msg.event   = EVENT_USER_APP_DEFINE;
        task_msg.msg_cb  = wifi_disable_proc;
        if (app_send_msg_to_wifitask(&task_msg) == false)
        {
            printk("[wifi] enter: send to wifi task failed\n");
        }
        else
        {
            printk("[wifi] enter: WiFi init dispatched to wifi task\n");
        }
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_SCAN) == 0)
    {
        /* ATWS -- scan all channels; per-AP SSID/RSSI printed by the callback */
        cmd_wifi_scan(wifi_scan_result_cb);
        printk("[wifi] scan: ATWS issued (results follow)\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_STATUS) == 0)
    {
        /* ATW? -- query current IP / gateway (hotspot IP); printed by callback */
        cmd_wifi_query_info(wifi_info_result_cb);
        printk("[wifi] status: ATW? issued (IP/GW follow)\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_GET_IP) == 0)
    {
        /* ATWQ -- lightweight local-IP query; single "IP=" line via callback.
         * Preferred over status/ATW?, which can return no data line under load. */
        cmd_wifi_query_ip(wifi_ip_result_cb);
        printk("[wifi] ip: ATWQ issued (IP follows)\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_CONNECT) == 0)
    {
        if (ev->data == NULL || ev->data_len < sizeof(T_WIFI_CONNECT_DATA))
        {
            return EVENT_BUS_OK;
        }
        T_WIFI_CONNECT_DATA *p = (T_WIFI_CONNECT_DATA *)ev->data;

        /* Stash the SSID; it is committed to s_connected_ssid only when the
         * real "[ATPN] OK" reply arrives (wifi_connect_result_cb). */
        strncpy(s_pending_ssid, p->ssid, sizeof(s_pending_ssid) - 1);
        s_pending_ssid[sizeof(s_pending_ssid) - 1] = '\0';

        /* ATPN=ssid,password  -- connect to AP. The callback sets the connected
         * flag from the actual reply, so it is NOT set here. */
        char param[130];
        snprintf(param, sizeof(param), "%s,%s", p->ssid, p->password);
        uart_atcmd_queue_fill(ATCMD_ATPN, param, wifi_connect_result_cb);

        T_WIFI_MSG ctrl;
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.event = EVENT_UART_CMD_FLOW_CTRL;
        app_send_msg_to_wifitask(&ctrl);

        printk("[wifi] connect: ATPN=%s,*** (awaiting reply)\n", p->ssid);
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_IPERF_UPLINK) == 0)
    {
        if (ev->data == NULL || ev->data_len < sizeof(T_WIFI_UPLINK_DATA))
        {
            return EVENT_BUS_OK;
        }
        T_WIFI_UPLINK_DATA *p = (T_WIFI_UPLINK_DATA *)ev->data;
        /* ATWT=-c,<ip>,-i,1,-t,15; callback confirms the "[ATWT]" reply. */
        cmd_wifi_set_client(p->server_ip, wifi_iperf_run_result_cb);
        printk("[wifi] iperf uplink: ATWT client -> %s\n", p->server_ip);
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_IPERF_DOWNLINK) == 0)
    {
        /* ATWT=-s (device = iperf server); callback confirms the "[ATWT]" reply. */
        cmd_wifi_set_server(wifi_iperf_run_result_cb);

        printk("[wifi] iperf downlink\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_IPERF_ENTER) == 0)
    {
        /* ATST=3; callback sets s_iperf_mode from the real "[ATST]" reply. */
        cmd_wifi_iperf_test_enable(wifi_iperf_mode_result_cb);

        printk("[wifi] iperf test enter\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_UPLINK_ENTER) == 0)
    {
        /* Uplink (SoC -> host), SDIO path: device is TCP server, host is client
         * and RECEIVES. "enter" only brings the shared TCP server up so the host
         * can connect before the SDIO TX pump starts flooding the chip FIFO. */
        uint16_t port = WIFI_DATA_DEFAULT_PORT;
        if (ev->data != NULL && ev->data_len >= sizeof(T_WIFI_DATA_DATA))
        {
            port = ((T_WIFI_DATA_DATA *)ev->data)->port;
        }
        if (!s_data_server_up)
        {
            /* Server-up flag is set by wifi_data_server_result_cb on "[ATPS] OK". */
            cmd_wifi_data_test_enter(port, wifi_data_server_result_cb);
        }
        printk("[wifi] socket uplink enter: server port=%u (SoC -> host)\n", port);
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_UPLINK_START) == 0)
    {
        /* Memory-sourced SDIO TX pump: fills a PSRAM buffer and re-arms itself
         * on the wifi task (no blocking file I/O), so it is safe to kick from
         * the app task here. Call this only after the host TCP client is
         * connected and draining. The SoC sends a fixed total and stops. */
        wifi_sdio_mem_tx_start();
        s_socket_uplink_active = true;
        printk("[wifi] socket uplink start: SDIO TX started (SoC -> host)\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_DOWNLINK_ENTER) == 0)
    {
        /* Downlink (host -> SoC), SDIO path: device is TCP server, host is client
         * and SENDS. "enter" only brings the shared TCP server up; the chip
         * forwards the received TCP stream to SDIO. */
        uint16_t port = WIFI_DATA_DEFAULT_PORT;
        if (ev->data != NULL && ev->data_len >= sizeof(T_WIFI_DATA_DATA))
        {
            port = ((T_WIFI_DATA_DATA *)ev->data)->port;
        }
        if (!s_data_server_up)
        {
            /* Server-up flag is set by wifi_data_server_result_cb on "[ATPS] OK". */
            cmd_wifi_data_test_enter(port, wifi_data_server_result_cb);
        }
        printk("[wifi] socket downlink enter: server port=%u (host -> SoC)\n", port);
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_DOWNLINK_START) == 0)
    {
        /* Arm SoC-side average-rate measurement: it times first->last forwarded
         * packet and prints the rate once the host's stream goes idle. */
        wifi_sdio_rx_start();
        s_socket_downlink_active = true;
        printk("[wifi] socket downlink start: rx-rate measurement armed (host -> SoC)\n");
    }
    else if (strcmp(t, EVENT_BUS_TOPIC_WIFI_CMD_DISCONNECT) == 0)
    {
        /* ATWD -- deassociate from AP. NOTE: the SoC firmware maps disconnect
         * to "ATWD" (fATWD/_AT_WLAN_DISC_NET_); "ATW1" is _AT_WLAN_SET_PASSPHRASE_
         * and does NOT disconnect. */
        /* The callback clears the connected flag from the actual "[ATWD]" reply.
         * The chip's fATWD terminator "\r\n[ATWD] OK" lacks a trailing newline;
         * the atcmd parser now dispatches such a terminator (see the partial-line
         * handling in uart_atcmd_rsp_handler), so this reply reaches the cb. */
        uart_atcmd_queue_fill(ATCMD_ATWD, NULL, wifi_disconnect_result_cb);

        T_WIFI_MSG ctrl;
        memset(&ctrl, 0, sizeof(ctrl));
        ctrl.event = EVENT_UART_CMD_FLOW_CTRL;
        app_send_msg_to_wifitask(&ctrl);

        printk("[wifi] disconnect: ATWD (awaiting reply)\n");
    }
    return EVENT_BUS_OK;
}

/* ---------- shell handlers (SHELL thread; publish only) ---------- */

static int sh_wifi_enter(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_ENTER, NULL, 0);
    shell_print(sh, "wifi: enter WiFi test scenario (queued to wifi task)");
    return 0;
}

static int sh_wifi_exit(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_EXIT, NULL, 0);
    shell_print(sh, "wifi: exit WiFi test scenario");
    return 0;
}

static int sh_wifi_scan(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_SCAN, NULL, 0);
    shell_print(sh, "wifi: scan (queued to app task)");
    return 0;
}

static int sh_wifi_connect(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 3)
    {
        shell_error(sh, "usage: wifi connect <ssid> <password>");
        return -EINVAL;
    }

    T_WIFI_CONNECT_DATA d;
    memset(&d, 0, sizeof(d));
    strncpy(d.ssid,     argv[1], sizeof(d.ssid)     - 1);
    strncpy(d.password, argv[2], sizeof(d.password) - 1);

    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_CONNECT, &d, sizeof(d));
    shell_print(sh, "wifi: connect %s *** (queued to app task)", d.ssid);
    return 0;
}

static int sh_wifi_iperf_enter(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_IPERF_ENTER, NULL, 0);
    shell_print(sh, "wifi: iperf test enter / ATST=3 (queued to app task)");
    return 0;
}


static int sh_wifi_iperf_uplink(const struct shell *sh, size_t argc, char **argv)
{
    if (argc < 2)
    {
        shell_error(sh, "usage: wifi uplink <server_ip>");
        return -EINVAL;
    }

    T_WIFI_UPLINK_DATA d;
    memset(&d, 0, sizeof(d));
    strncpy(d.server_ip, argv[1], sizeof(d.server_ip) - 1);

    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_IPERF_UPLINK, &d, sizeof(d));
    shell_print(sh, "wifi: uplink test -> %s (queued to app task)", d.server_ip);
    return 0;
}

static int sh_wifi_iperf_downlink(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_IPERF_DOWNLINK, NULL, 0);
    shell_print(sh, "wifi: downlink test / server mode (queued to app task)");
    return 0;
}

static int sh_wifi_socket_uplink_enter(const struct shell *sh, size_t argc, char **argv)
{
    T_WIFI_DATA_DATA d;
    memset(&d, 0, sizeof(d));
    d.port = (argc >= 2) ? (uint16_t)atoi(argv[1]) : WIFI_DATA_DEFAULT_PORT;

    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_UPLINK_ENTER, &d, sizeof(d));
    shell_print(sh, "wifi: socket uplink enter on port %u (queued to app task)", d.port);
    return 0;
}

static int sh_wifi_socket_uplink_start(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_UPLINK_START, NULL, 0);
    shell_print(sh, "wifi: socket uplink start (queued to app task)");
    return 0;
}

static int sh_wifi_socket_downlink_enter(const struct shell *sh, size_t argc, char **argv)
{
    T_WIFI_DATA_DATA d;
    memset(&d, 0, sizeof(d));
    d.port = (argc >= 2) ? (uint16_t)atoi(argv[1]) : WIFI_DATA_DEFAULT_PORT;

    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_DOWNLINK_ENTER, &d, sizeof(d));
    shell_print(sh, "wifi: socket downlink enter on port %u (queued to app task)", d.port);
    return 0;
}

static int sh_wifi_socket_downlink_start(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_SOCKET_DOWNLINK_START, NULL, 0);
    shell_print(sh, "wifi: socket downlink start (queued to app task)");
    return 0;
}

static int sh_wifi_disconnect(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_DISCONNECT, NULL, 0);
    shell_print(sh, "wifi: disconnect (queued to app task)");
    return 0;
}

static int sh_wifi_status(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    shell_print(sh, "wifi status:");
    shell_print(sh, "  wifi entered : %s", s_wifi_entered  ? "yes" : "no");
    shell_print(sh, "  wifi connected: %s", s_wifi_connected ? "yes" : "no");
    if (s_wifi_connected && s_connected_ssid[0])
    {
        shell_print(sh, "  ssid          : %s", s_connected_ssid);
    }
    shell_print(sh, "  iperf mode    : %s", s_iperf_mode    ? "yes" : "no");
    shell_print(sh, "  data server up: %s", s_data_server_up ? "yes" : "no");

    /* IP / gateway live on the SoC; query them over AT ("ATW?"). The reply is
     * async, so it is handled on the app task and printed by the callback a
     * moment later (same pattern as scan), not via this synchronous shell_print. */
    if (s_wifi_entered)
    {
        event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_STATUS, NULL, 0);
        shell_print(sh, "  (IP/hotspot-IP follow shortly...)");
    }
    return 0;
}

static int sh_wifi_get_ip(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    if (!s_wifi_entered)
    {
        shell_error(sh, "wifi not entered; run 'wifi enter' first");
        return -EINVAL;
    }
    /* Lightweight query (ATWQ). Reply "[wifi] IP=<ip>" is printed by the
     * async callback on the app task, not here. */
    event_bus_publish(EVENT_BUS_TOPIC_WIFI_CMD_GET_IP, NULL, 0);
    shell_print(sh, "wifi: ip query queued (IP follows shortly)");
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(wifi_subcmds,
                               SHELL_CMD(enter,      NULL, "Enable WiFi HW and init (enter test scenario)",      sh_wifi_enter),
                               SHELL_CMD(exit,       NULL, "Exit WiFi test. Deinit HW",                          sh_wifi_exit),
                               SHELL_CMD(scan,       NULL, "Scan APs and print SSID/RSSI",                       sh_wifi_scan),
                               SHELL_CMD(connect,    NULL, "Connect to AP: wifi connect <ssid> <password>",   sh_wifi_connect),
                               SHELL_CMD(iperf_enter,      NULL,
                                         "iperf mode (ATST=3, no SDIO); send once before iperf up/downlink", sh_wifi_iperf_enter),
                               SHELL_CMD(iperf_uplink,     NULL, "iperf uplink (device=client): wifi iperf_uplink <server_ip>",
                                         sh_wifi_iperf_uplink),
                               SHELL_CMD(iperf_downlink,   NULL, "iperf downlink (device=server)",
                                         sh_wifi_iperf_downlink),
                               SHELL_CMD(socket_uplink_enter,    NULL,
                                         "SDIO socket uplink: bring up TCP server (SoC->host): wifi socket_uplink_enter [port]",
                                         sh_wifi_socket_uplink_enter),
                               SHELL_CMD(socket_uplink_start,    NULL,
                                         "SDIO socket uplink: start SDIO TX pump (after host connected)",
                                         sh_wifi_socket_uplink_start),
                               SHELL_CMD(socket_downlink_enter,  NULL,
                                         "SDIO socket downlink: bring up TCP server (host->SoC): wifi socket_downlink_enter [port]",
                                         sh_wifi_socket_downlink_enter),
                               SHELL_CMD(socket_downlink_start,  NULL, "SDIO socket downlink: arm rx-rate measurement",
                                         sh_wifi_socket_downlink_start),
                               SHELL_CMD(disconnect, NULL, "Disconnect from AP",
                                         sh_wifi_disconnect),
                               SHELL_CMD(status,     NULL, "Print WiFi test state",                               sh_wifi_status),
                               SHELL_CMD(ip,         NULL, "Query local IP only (lightweight ATWQ)",              sh_wifi_get_ip),
                               SHELL_SUBCMD_SET_END
                              );
SHELL_CMD_REGISTER(wifi, &wifi_subcmds,
                   "WiFi-basic test (CONFIG_SHELL_APP_WIFI_BASIC)", NULL);

/* ---------- module init (auto-called at boot via APP_MODULE_INIT) ---------- */

static void wifi_basic_module_init(void)
{
    event_bus_topic_register(EVENT_BUS_TOPIC_WIFI_ALL_TOPIC);

    event_bus_subscribe_async(&s_cmd_h, EVENT_BUS_TOPIC_WIFI_ALL_TOPIC,
                              event_bus_async_send_to_apptask, NULL, wifi_cmd_cb);

}
APP_MODULE_INIT(wifi_basic_module_init);
