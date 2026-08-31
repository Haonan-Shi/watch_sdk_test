/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * @file    spi_tcp_tp_test.h
 * @brief   SPI + TCP throughput test (master side): uplink + downlink, with the
 *          slave acting as either a TCP server or a TCP client.
 *
 * Test topology:
 *
 *   Master(this MCU) <--SPI--> Slave(RTL AT device) <--TCP--> PC
 *
 * The master drives the slave purely through AT commands carried over SPI
 * (see app_spi_atcmd.c).
 *
 * Uplink (master -> PC):
 *   Master issues the two-phase "AT+SKTSENDRAW=<id>,<n>" transparent send and
 *   pushes <n> raw bytes --(SPI)--> Slave --(TCP)--> PC. Goodput is the number
 *   of payload bytes the slave accepted (each "OK") over the elapsed time.
 *
 * Downlink (PC -> master):
 *   The slave is set up with auto-recv on, so any TCP data it receives is
 *   pushed --(SPI)--> Master unsolicited. The test simply counts the payload
 *   bytes of every valid SPI frame received during the window (see the RX byte
 *   tap in app_spi_atcmd.c) - this is format-independent and includes the tiny
 *   per-line recv header overhead (< ~1.5% at MSS-sized packets).
 *
 * Slave role:
 *   - Server: AT+SKTSERVER=0,1,,<port>,1 ; the PC connects in (uplink to the
 *     accepted "seed" link id, downlink from the PC's flood).
 *   - Client: AT+SKTCLIENT=<id>,1,,<ip>,<port>,,1 ; the slave connects out to a
 *     PC-hosted TCP server. Uplink/downlink then run on that client link id.
 *
 * Either direction reflects the *combined* SPI + TCP path, which is what we
 * want to characterise. The flow can be driven step-by-step, or (server uplink)
 * in one shot via app_spi_tcp_tp_run().
 */

#ifndef _SPI_TCP_TP_TEST_H_
#define _SPI_TCP_TP_TEST_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Default TCP listen port used when caller passes 0. */
#define TP_DEFAULT_PORT         5001
/** Default payload bytes per AT+SKTSEND when caller passes 0 (one TCP MSS). */
#define TP_DEFAULT_CHUNK        1460
/** Default test duration (seconds) when caller passes 0. */
#define TP_DEFAULT_DURATION_S   10
/**
 * Upper bound for one SKTSEND payload.
 *
 * One SPI frame is able to carry up to SPI_XMIT_SIZE bytes (16 KB after 341031) and carries
 * [AT(2)][Len(2)][ "AT+SKTSEND=<id>,<n>," + payload + "\r\n" ][CRC32(4)].
 * 4096 stays well within the 16 KB frame, leaving room for the header,
 * command prefix and trailing CRC. The slave AT inline limit may be lower,
 * so chunk is also a tunable parameter of the test command.
 * the trailing CRC.
 */
#define TP_MAX_CHUNK            4096

/**
 * Link id sentinel: pass this as link_id to make the test use the "seed"
 * link id auto-detected from the client-connected event.
 */
#define TP_LINK_ID_AUTO         0xFF

/* ----------------------------------------------------------------------------
 * Parameter source selection for the three driving commands (01/02/03).
 *
 *   TP_PARAMS_FROM_MACRO == 0  (default)
 *       WIFI_CONNECT / CREATE_SERVER / START_UPLINK take their parameters from
 *       the command frame forwarded by app_cmd (the bytes after the action
 *       code) - i.e. the host supplies them over SPI/UART.
 *
 *   TP_PARAMS_FROM_MACRO == 1
 *       Those frame parameters are ignored; the hard-coded TP_CFG_* values
 *       below are used instead. Edit them to match your bench, then a bare
 *       "AA <seq> <len> 41 84 01|02|03" frame (no params) drives each step.
 *       The action code (01/02/03) still selects which step runs.
 * --------------------------------------------------------------------------*/
#define TP_PARAMS_FROM_MACRO    1

/* User-tunable parameters used when TP_PARAMS_FROM_MACRO == 1 */
#define TP_CFG_WIFI_SSID        "mytest"
#define TP_CFG_WIFI_PWD         "12345678"
#define TP_CFG_SERVER_PORT      5001
#define TP_CFG_LINK_ID          TP_LINK_ID_AUTO   /* 0xFF = auto seed link id */
#define TP_CFG_CHUNK            1460
#define TP_CFG_DURATION_S       10

/* Slave-as-TCP-client target: the PC-hosted TCP server to connect out to. */
#define TP_CFG_CLIENT_IP        "192.168.2.3"
#define TP_CFG_CLIENT_PORT      50002
#define TP_CFG_CLIENT_LINK_ID   0       /* link id assigned to the client link */

/**
 * Default total payload bytes for the SPI+TCP "blast" uplink test (action 9).
 *
 * One handshake "AT+SKTSENDRAW=<id>,<total>" -> one ">>>" -> the dedicated SPI
 * TX task (see SPI_TCP_TP_TX_TASK in app_spi_atcmd.h) blasts this many bytes to
 * the slave as back-to-back SPI_XMIT_SIZE (16 KB) frames carrying a repeating
 * 0..127 pattern, then the slave answers one "OK". Adjust any time; the default
 * is 10 MB of repeated data.
 */
#define TP_CFG_BLAST_TOTAL_BYTES    (10u * 1024u * 1024u)   /* 10 MB */

/**
 * Make TP_ACT_START_UPLINK (action 3) auto-blast instead of running the
 * duration-based AT+SKTSEND chunk pump: one AT+SKTSENDRAW handshake, then the
 * dedicated TX task streams TP_CFG_BLAST_TOTAL_BYTES to the slave as
 * back-to-back SPI frames, each carrying (SPI_XMIT_SIZE - 8) payload bytes of a
 * continuous 0..127 ramp. Set to 0 to keep the original chunk-pump behaviour.
 * Requires SPI_TCP_TP_TX_TASK == 1 (see app_spi_atcmd.h).
 */
#ifndef TP_UPLINK_USE_BLAST
#define TP_UPLINK_USE_BLAST         1
#endif

/**
 * When set, the uplink payload is read from a file on the SD card (sdhc0 ->
 * "/SD:") instead of the synthetic 0..127 blast ramp / 'A' chunk filler. This
 * applies to BOTH uplink paths:
 *   - blast   (TP_UPLINK_USE_BLAST==1): the file size is used as the
 *             AT+SKTSENDRAW total, and the dedicated TX task streams the file.
 *   - chunk   (TP_UPLINK_USE_BLAST==0): each AT+SKTSENDRAW carries the next
 *             file chunk.
 * The file is sent exactly once; on EOF the test stops and reports the bytes
 * actually sent. Set to 0 to restore the synthetic-pattern behaviour.
 * See app_spi_sd_source.[ch].
 */
#ifndef TP_TX_DATA_FROM_SDCARD
#define TP_TX_DATA_FROM_SDCARD      0
#endif

/** SD-card file used as the uplink data source when TP_TX_DATA_FROM_SDCARD==1. */
#ifndef TP_SD_FILE_PATH
#define TP_SD_FILE_PATH             "/SD:/audio/test.bin"
#endif

/**
 * @brief Initialise the throughput test module.
 *        Registers the AT-over-SPI event callback. Safe to call repeatedly.
 *
 * @note  This takes over the single app_spi_atcmd event callback while the
 *        test module is in use.
 */
void app_spi_tcp_tp_init(void);

/**
 * @brief Connect the slave to an AP (sends AT+WLCONN over SPI).
 * @param ssid  AP SSID (NUL terminated)
 * @param pwd   AP password (NUL terminated, may be "" for open AP)
 */
void app_spi_tcp_tp_wifi_connect(const char *ssid, const char *pwd);

/**
 * @brief Create the TCP server on the slave (AT+SKTCFG to disable Nagle,
 *        then AT+SKTSERVER with auto_rcv enabled).
 * @param port  TCP listen port (0 -> TP_DEFAULT_PORT)
 */
void app_spi_tcp_tp_create_server(uint16_t port);

/**
 * @brief Query slave socket state (AT+SKTSTATE). The response, including the
 *        accepted "seed" link id, is printed to the trace log.
 */
void app_spi_tcp_tp_query_state(void);

/**
 * @brief Start the uplink data pump (continuous AT+SKTSEND) for a fixed time.
 * @param link_id     Socket link id to send on. 0xFF -> use the seed link id
 *                    auto-detected from the client-connected event.
 * @param chunk       Payload bytes per send (0 -> TP_DEFAULT_CHUNK,
 *                    clamped to TP_MAX_CHUNK).
 * @param duration_s  Test duration in seconds (0 -> TP_DEFAULT_DURATION_S).
 */
void app_spi_tcp_tp_start_uplink(uint8_t link_id, uint16_t chunk, uint16_t duration_s);

/**
 * @brief Start the SPI+TCP "blast" uplink: a single transparent send of
 *        total_bytes driven by the dedicated SPI TX task. One
 *        "AT+SKTSENDRAW=<id>,<total>" handshake, then the TX task streams the
 *        whole payload (0..127 ramp) as 16 KB SPI frames without blocking the
 *        APP task. Throughput is total_bytes over the time until the slave's
 *        terminal "OK". Requires SPI_TCP_TP_TX_TASK == 1 (app_spi_atcmd.h).
 * @param link_id      Socket link id to send on (0xFF -> current/seed link id).
 * @param total_bytes  Bytes to blast (0 -> TP_CFG_BLAST_TOTAL_BYTES).
 */
void app_spi_tcp_tp_start_uplink_blast(uint8_t link_id, uint32_t total_bytes);

/**
 * @brief Connect the slave out to a PC-hosted TCP server as a TCP *client*
 *        (AT+SKTCFG to disable Nagle, then AT+SKTCLIENT with auto-recv on).
 *        The same link id is then used for uplink (SKTSENDRAW) and downlink.
 * @param ip       PC server IPv4 dotted string (NULL/"" -> TP_CFG_CLIENT_IP)
 * @param port     PC server TCP port (0 -> TP_CFG_CLIENT_PORT)
 * @param link_id  Link id to assign to the client connection (SKTCLIENT arg0).
 */
void app_spi_tcp_tp_create_client(const char *ip, uint16_t port, uint8_t link_id);

/**
 * @brief Start the downlink measurement window: enable the SPI RX byte tap and
 *        count every payload byte the slave pushes (PC flood -> slave -> SPI)
 *        for a fixed time, then print the downlink rate.
 * @param link_id     Link id the downlink is expected on (logging only; the tap
 *                    counts all SPI RX). 0xFF -> keep the current link id.
 * @param duration_s  Test duration in seconds (0 -> TP_DEFAULT_DURATION_S).
 */
void app_spi_tcp_tp_start_downlink(uint8_t link_id, uint16_t duration_s);

/**
 * @brief Stop an in-progress uplink or downlink test early and print the
 *        result so far.
 */
void app_spi_tcp_tp_stop(void);

/**
 * @brief One-shot orchestration: connect AP -> create server -> wait for the
 *        PC to connect -> run the uplink pump -> print throughput.
 *
 * @param ssid        AP SSID
 * @param pwd         AP password ("" for open)
 * @param port        TCP listen port (0 -> default)
 * @param chunk       Payload bytes per send (0 -> default)
 * @param duration_s  Test duration seconds (0 -> default)
 */
void app_spi_tcp_tp_run(const char *ssid, const char *pwd, uint16_t port,
                        uint16_t chunk, uint16_t duration_s);

/**
 * @brief Decode a CMD_SPI_TCP_TP_TEST binary payload and dispatch to the
 *        matching API above. See app_cmd.c for the payload layout.
 * @param cmd_ptr  raw command bytes ([0..1]=cmd id, [2]=sub-action, ...)
 * @param cmd_len  total length of cmd_ptr in bytes
 */
void app_spi_tcp_tp_handle_cmd(uint8_t *cmd_ptr, uint16_t cmd_len);

#ifdef __cplusplus
}
#endif

#endif /* _SPI_TCP_TP_TEST_H_ */
