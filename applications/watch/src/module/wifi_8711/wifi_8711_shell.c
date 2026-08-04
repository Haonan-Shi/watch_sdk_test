/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * wifi_8711 engineering shell commands (CONFIG_WIFI_8711_TEST).
 *
 * Replaces the host-command triggers that used to live in app/app_cmd.c:
 *   CMD_SPI_INIT        -> `wifi8711 spiinit [master|slave]`
 *   CMD_AT_CMD_WLCONN   -> `wifi8711 wlconn` / `wifi8711 wldisconn`
 *   CMD_SPI_TCP_TP_TEST -> `wifi8711 tp <subcmd>`
 *
 * The `tp` subcommands build the same binary payload the host used to send and
 * feed it to app_spi_tcp_tp_handle_cmd(), so behaviour is identical to the old
 * CMD_SPI_TCP_TP_TEST path. Parameters come from the TP_CFG_* macros
 * (TP_PARAMS_FROM_MACRO=1 in spi_tcp_tp_test.h).
 *
 * Usage
 * -----
 * Type the commands over the console UART. They exist only in a build made
 * with `-S wifi_8711`, which turns on CONFIG_WIFI_8711 + CONFIG_SHELL +
 * CONFIG_WIFI_8711_TEST (see snippets/wifi_8711/wifi_8711.conf).
 *
 *   wifi8711 spiinit [master|slave]  bring up the SPI role (default master)
 *   wifi8711 wlconn                  AT+WLCONN to the built-in test AP
 *   wifi8711 wldisconn               AT+WLDISCONN
 *   wifi8711 tp init                 init the throughput test module
 *   wifi8711 tp connect              slave connects to the test AP (macro cfg)
 *   wifi8711 tp server               create a TCP server on the slave
 *   wifi8711 tp client               slave connects out to a PC TCP server
 *   wifi8711 tp uplink               start the uplink pump
 *   wifi8711 tp blast                start the uplink blast (dedicated TX task)
 *   wifi8711 tp downlink             start a downlink measurement window
 *   wifi8711 tp state                query the slave socket state
 *   wifi8711 tp autorun              one-shot connect -> server -> pump
 *   wifi8711 tp stop                 stop the running uplink/downlink test
 *
 * Typical throughput run (this chip is the SPI master, the 8711 is the AT
 * slave running its AT-over-SPI firmware):
 *   wifi8711 spiinit master
 *   wifi8711 tp init
 *   wifi8711 tp connect
 *   wifi8711 tp server
 *   wifi8711 tp blast
 *   wifi8711 tp stop
 *
 * Notes:
 *   - The shell only reports that the action was dispatched; the real result
 *     (link state, measured rate) is printed asynchronously on the UART log.
 *   - All test parameters (AP ssid/pw, server ip/port, packet size) are the
 *     compile-time TP_CFG_* macros in spi_tcp_tp_test.h; the shell takes no
 *     parameters, so changing a value means editing the macro and rebuilding.
 *   - `tp blast` uses the dedicated TX task for max uplink rate; `tp uplink`
 *     is the plain pump.
 *   - Tab-completion works, and `wifi8711 help` / `wifi8711 tp help` list the
 *     command tree at runtime.
 */

#if defined(CONFIG_WIFI_8711_TEST)

#include <string.h>
#include <zephyr/shell/shell.h>

#include "app_spi_api.h"
#include "app_spi_atcmd.h"
#include "spi_tcp_tp_test.h"

/* CMD_SPI_TCP_TP_TEST == 0x8441 (app_cmd.h). The decoder ignores cmd_ptr[0..1]
 * and reads cmd_ptr[2] as the sub-action; the two id bytes are kept only to
 * mirror the original host frame exactly. */
#define TP_CMD_ID_LO        0x41
#define TP_CMD_ID_HI        0x84

/* Sub-actions (T_TP_ACTION in spi_tcp_tp_test.c). */
#define TP_ACT_INIT              0
#define TP_ACT_WIFI_CONNECT      1
#define TP_ACT_CREATE_SERVER     2
#define TP_ACT_START_UPLINK      3
#define TP_ACT_STOP              4
#define TP_ACT_QUERY_STATE       5
#define TP_ACT_AUTO_RUN          6
#define TP_ACT_START_DOWNLINK    7
#define TP_ACT_CREATE_CLIENT     8
#define TP_ACT_START_UPLINK_BLAST 9

static void tp_dispatch(const struct shell *sh, uint8_t action)
{
    /* Same layout app_cmd.c forwarded to app_spi_tcp_tp_handle_cmd():
     * [0..1] = CMD_SPI_TCP_TP_TEST, [2] = sub-action. No parameter bytes
     * (TP_PARAMS_FROM_MACRO uses the TP_CFG_* macros). */
    uint8_t buf[3] = { TP_CMD_ID_LO, TP_CMD_ID_HI, action };
    app_spi_tcp_tp_handle_cmd(buf, sizeof(buf));
    shell_print(sh, "wifi8711 tp: action %u dispatched (see UART log)", action);
}

/* -------- top-level: SPI role init (was CMD_SPI_INIT) ------------------- */
static int cmd_wifi8711_spiinit(const struct shell *sh, size_t argc, char **argv)
{
    bool slave = (argc >= 2 && strcmp(argv[1], "slave") == 0);

    if (slave)
    {
#if defined(CONFIG_WIFI_8711_ROLE_SLAVE)
        app_spi_slave_init();
        shell_print(sh, "spi slave init done");
#else
        shell_warn(sh, "slave role not built (CONFIG_WIFI_8711_ROLE_SLAVE=n)");
#endif
    }
    else
    {
#if defined(CONFIG_WIFI_8711_ROLE_MASTER)
        app_spi_master_init();
        shell_print(sh, "spi master init done");
#else
        shell_warn(sh, "master role not built (CONFIG_WIFI_8711_ROLE_MASTER=n)");
#endif
    }
    return 0;
}

/* -------- top-level: AT WLCONN / WLDISCONN (was CMD_AT_CMD_WLCONN) ------ */
static int cmd_wifi8711_wlconn(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    app_spi_atcmd_demo(ATCMD_WLCONN);
    shell_print(sh, "wifi8711: AT+WLCONN queued (built-in test AP)");
    return 0;
}

static int cmd_wifi8711_wldisconn(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);
    app_spi_atcmd_demo(ATCMD_WLDISCONN);
    shell_print(sh, "wifi8711: AT+WLDISCONN queued");
    return 0;
}

/* -------- tp subcommands (was CMD_SPI_TCP_TP_TEST) --------------------- */
static int cmd_tp_init(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_INIT);
    return 0;
}
static int cmd_tp_connect(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_WIFI_CONNECT);
    return 0;
}
static int cmd_tp_server(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_CREATE_SERVER);
    return 0;
}
static int cmd_tp_client(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_CREATE_CLIENT);
    return 0;
}
static int cmd_tp_uplink(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_START_UPLINK);
    return 0;
}
static int cmd_tp_blast(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_START_UPLINK_BLAST);
    return 0;
}
static int cmd_tp_downlink(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_START_DOWNLINK);
    return 0;
}
static int cmd_tp_state(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_QUERY_STATE);
    return 0;
}
static int cmd_tp_autorun(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_AUTO_RUN);
    return 0;
}
static int cmd_tp_stop(const struct shell *sh, size_t argc, char **argv)
{
    ARG_UNUSED(argc); ARG_UNUSED(argv);
    tp_dispatch(sh, TP_ACT_STOP);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(wifi8711_tp_subcmds,
                               SHELL_CMD(init,     NULL, "Init throughput test module",
                                         cmd_tp_init),
                               SHELL_CMD(connect,  NULL, "Connect slave to test AP (macro cfg)",
                                         cmd_tp_connect),
                               SHELL_CMD(server,   NULL, "Create TCP server on the slave",
                                         cmd_tp_server),
                               SHELL_CMD(client,   NULL, "Connect slave out to a PC TCP server",
                                         cmd_tp_client),
                               SHELL_CMD(uplink,   NULL, "Start uplink pump / blast",
                                         cmd_tp_uplink),
                               SHELL_CMD(blast,    NULL, "Start uplink blast (TX-task stream)",
                                         cmd_tp_blast),
                               SHELL_CMD(downlink, NULL, "Start downlink measurement window",
                                         cmd_tp_downlink),
                               SHELL_CMD(state,    NULL, "Query slave socket state",
                                         cmd_tp_state),
                               SHELL_CMD(autorun,  NULL, "One-shot connect->server->pump",
                                         cmd_tp_autorun),
                               SHELL_CMD(stop,     NULL, "Stop the running uplink/downlink test",
                                         cmd_tp_stop),
                               SHELL_SUBCMD_SET_END
                              );

SHELL_STATIC_SUBCMD_SET_CREATE(wifi8711_subcmds,
                               SHELL_CMD_ARG(spiinit, NULL,
                                             "Init SPI role: spiinit [master|slave] (default master)",
                                             cmd_wifi8711_spiinit, 1, 1),
                               SHELL_CMD(wlconn,    NULL, "AT+WLCONN to the built-in test AP",
                                         cmd_wifi8711_wlconn),
                               SHELL_CMD(wldisconn, NULL, "AT+WLDISCONN",
                                         cmd_wifi8711_wldisconn),
                               SHELL_CMD(tp, &wifi8711_tp_subcmds,
                                         "SPI+TCP throughput test (CMD_SPI_TCP_TP_TEST)", NULL),
                               SHELL_SUBCMD_SET_END
                              );

SHELL_CMD_REGISTER(wifi8711, &wifi8711_subcmds,
                   "WiFi 8711 AT-over-SPI engineering test commands (CONFIG_WIFI_8711_TEST)",
                   NULL);

#endif /* CONFIG_WIFI_8711_TEST */
