/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_WIFI_UART_CMD

#include <string.h>
#include <stdio.h>
#include "trace.h"
#include "app_uart_atcmd.h"
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/pinctrl.h>
#include "os_mem.h"
#include "os_sync.h"
#include "app_wifi_uart.h"
#include "app_io_msg.h"
#include "app_dlps.h"
#include "os_msg.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_uart.h"
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>


LOG_MODULE_REGISTER(app_wifi_uart, LOG_LEVEL_DBG);
#define RX_BUF_LENGTH       100
#define TX_BUF_LENGTH       128

/* Debug: share the wifi-module UART line (uart3 TX = P5_4) with a PC serial
 * adapter. When 1, the TX pin is kept hi-Z while idle so the PC can drive the
 * module; the MCU only takes the pin during its own AT transmit, then releases
 * it again on TX-done. Set 0 for normal operation (uart3 TX owns P5_4 the whole
 * time). NOTE: even with this on, the MCU and PC must not transmit at the same
 * time (two push-pull TX on one wire still contend during the overlap). */
#define WIFI_UART_TX_SHARE_DEBUG    1
#define WIFI_UART3_TX_PIN           P5_4   /* uart3 TX pad on rtl8783gbf */

/* Per-line cap for the raw uart3 RX hex dump. A chunk longer than this is dumped
 * across several lines (segment offset shown) so nothing is truncated. */
#define WIFI_UART_RX_DUMP_SEG       96


typedef struct
{
    uint16_t read_idx;
    uint16_t write_idx;
    uint16_t rx_cnt;
    uint8_t  console_buf[RX_BUF_LENGTH];
    bool     tx_lock;
    uint8_t tx_buf[TX_BUF_LENGTH];
} T_CONSOLE;

typedef struct
{
    uint32_t baud;
    void *mp_sem_handle;
    uint8_t buf_index;
    uint8_t rx_buf[RX_BUF_LENGTH * 2];
} T_WIFI_UART_SET;

static P_WIFI_UART_RX_PARSER rgs_parser_cb = NULL;
static P_WIFI_UART_DEFERRED_CB deferred_connect_cb = NULL;

/* Latched once the module prints its "COMMAND READY" boot banner on UART3.
 * The cold-start bring-up waits on this (via app_wifi_uart_module_is_ready)
 * before issuing the first ATPN, replacing a blind power-on delay. Written
 * from the UART RX callback and read/reset from the WiFi task, hence volatile. */
static volatile bool wifi_module_cmd_ready = false;

void app_wifi_uart_reset_module_ready(void)
{
    wifi_module_cmd_ready = false;
}

bool app_wifi_uart_module_is_ready(void)
{
    return wifi_module_cmd_ready;
}

/* Scan a raw RX chunk for the module's "READY" boot marker (tail of the
 * "COMMAND READY" banner). Matching just the 5-byte tail keeps detection
 * working even if a DMA chunk boundary splits "COMMAND " from "READY". Runs
 * before the wifi_enable_flag gate because the banner arrives while
 * the gate is still closed (wifi_enable_flag == 0). */
static void wifi_uart_scan_cmd_ready(const uint8_t *buf, uint16_t len)
{
    static const char marker[] = "READY";
    const uint16_t mlen = 5; /* strlen("READY") */

    if (wifi_module_cmd_ready || buf == NULL || len < mlen)
    {
        return;
    }
    for (uint16_t i = 0; i + mlen <= len; i++)
    {
        if (memcmp(&buf[i], marker, mlen) == 0)
        {
            wifi_module_cmd_ready = true;
            APP_PRINT_INFO0("[wifi] module COMMAND READY detected");
            return;
        }
    }
}


#define PINCTRL_STATE_MP    (PINCTRL_STATE_PRIV_START+1)
PINCTRL_DT_STATE_PINS_DEFINE(DT_PATH(zephyr_user), uart3_mp_pin);
PINCTRL_DT_DEV_CONFIG_DECLARE(DT_ALIAS(uart3));
const struct pinctrl_dev_config *pincfg = PINCTRL_DT_DEV_CONFIG_GET(DT_ALIAS(uart3));
static const struct pinctrl_state uart3_mp_state = PINCTRL_DT_STATE_INIT(uart3_mp_pin,
                                                                         PINCTRL_STATE_MP);
const struct device *const uart3_dev = DEVICE_DT_GET(DT_ALIAS(uart3));

T_WIFI_UART_SET uart_set = {.baud = 115200};
T_CONSOLE wifi_console =
{
    .read_idx = 0,
    .write_idx = 0,
    .rx_cnt = 0,
    .tx_lock = false,
};

#if WIFI_UART_TX_SHARE_DEBUG
/* P5_4 is shared with a PC serial adapter: hand the pad to the PC while idle,
 * grab it back only for our own AT transmit. The trick is that flipping the pad
 * MODE alone is NOT enough -- a pin routed to a peripheral needs BOTH the pad in
 * PINMUX mode AND the pinmux function map (UART3_TX) AND the AON high-speed mux
 * re-programmed, exactly as the vendor pinctrl driver does it
 * (zephyr/drivers/pinctrl/pinctrl_rtl87x3g.c). The earlier "mode-only flip"
 * reconnected the pad to *no function*, so TX went dead. We now replay the full
 * vendor sequence for just this one pin, in the same order:
 *   Pinmux_Deinit -> Pad_Config(mode) -> [Pinmux_Config] -> Pad_HighSpeedMuxSel.
 *
 * NOTE: when acquired the pad drives push-pull, so the PC must stay quiet while
 * the MCU transmits (and vice-versa). If both may drive, add a series R on the
 * PC TX -- this is sharing, not true open-drain (the SoC has no OD on UART pads).
 */
static void wifi_uart_tx_pin_release(void)
{
    /* Never tri-state the line while our own TX is still in flight: uart_tx() is
     * async, so rx_enable / DLPS-exit can call release() mid-transmit and chop
     * the AT command in half (module then never replies -> 20 s timeout). Defer:
     * UART_TX_DONE clears tx_lock first, then calls release() to actually let go. */
    if (wifi_console.tx_lock)
    {
        APP_PRINT_INFO0("[wifi][dbg] tx pin release SKIPPED (tx in flight)");   /* TEMP probe */
        return;
    }
    APP_PRINT_INFO0("[wifi][dbg] tx pin release: P5_4 -> hi-Z");   /* TEMP probe */
    /* Detach UART3_TX and tri-state the pad (SW_MODE + output DISABLE -> hi-Z
     * input) so the PC adapter can drive the module RX. Weak pull-up holds the
     * line idle-high when neither side drives. */
    Pinmux_Deinit(WIFI_UART3_TX_PIN);
    Pad_Config(WIFI_UART3_TX_PIN, PAD_SW_MODE, PAD_IS_PWRON, PAD_PULL_UP,
               PAD_OUT_DISABLE, PAD_OUT_LOW);
    Pad_HighSpeedMuxSel(WIFI_UART3_TX_PIN, FROM_AON_DOMAIN);
}

/* Acquire: re-route P5_4 back to UART3_TX so the peripheral drives the line.
 * Must replay the full pinmux sequence -- Pad_Config(PINMUX) alone leaves the
 * pin connected to no function. */
static void wifi_uart_tx_pin_acquire(void)
{
    Pinmux_Deinit(WIFI_UART3_TX_PIN);
    Pad_Config(WIFI_UART3_TX_PIN, PAD_PINMUX_MODE, PAD_IS_PWRON, PAD_PULL_UP,
               PAD_OUT_ENABLE, PAD_OUT_HIGH);
    Pinmux_Config(WIFI_UART3_TX_PIN, UART3_TX);
    Pad_HighSpeedMuxSel(WIFI_UART3_TX_PIN, FROM_AON_DOMAIN);
    APP_PRINT_INFO0("[wifi][dbg] tx pin acquire: P5_4 -> UART3_TX");   /* TEMP probe */
}

/* uart3 register base, for polling the true "transmitter drained" flag. */
#define WIFI_UART3_REG  ((UART_TypeDef *)DT_REG_ADDR(DT_ALIAS(uart3)))

static void wifi_uart_tx_release_work_fn(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(wifi_uart_tx_release_work, wifi_uart_tx_release_work_fn);
static uint16_t wifi_tx_release_poll;

/* Deferred pad release. The async UART_TX_DONE event fires when the DMA has
 * finished FILLING the TX FIFO -- the shift register is usually still clocking
 * the last bytes onto the wire (DMA-done came ~0.25 ms after acquire for an 8 B
 * cmd that needs ~0.7 ms on the wire). Tri-stating the pad on that event chops
 * the tail of the AT command, so the module never replies (20 s timeout). Here
 * we wait for the HW "fully drained" flag (UART_FLAG_TX_DONE = FIFO empty AND
 * waveform sent) before letting go. Runs in the system workqueue (NOT ISR),
 * re-arming every tick with a safety cap so it can never spin forever. */
static void wifi_uart_tx_release_work_fn(struct k_work *work)
{
    ARG_UNUSED(work);
    if (wifi_console.tx_lock)
    {
        return;   /* a new TX started; its own TX_DONE will re-arm the release */
    }
    if (!UART_GetFlagStatus(WIFI_UART3_REG, UART_FLAG_TX_DONE) && wifi_tx_release_poll < 200)
    {
        wifi_tx_release_poll++;
        k_work_reschedule(&wifi_uart_tx_release_work, K_USEC(200));
        return;
    }
    wifi_uart_tx_pin_release();
}
#endif

void app_wifi_uart_rx_parser_register(P_WIFI_UART_RX_PARSER cb_func)
{
    if (cb_func != NULL)
    {
        rgs_parser_cb = cb_func;
    }
}

void app_wifi_uart_deferred_connect_register(P_WIFI_UART_DEFERRED_CB cb_func)
{
    if (cb_func != NULL)
    {
        deferred_connect_cb = cb_func;
    }
}

bool app_wifi_uart_msg_send(IO_WIFI_UART_MSG_TYPE subtype, void *param_buf)
{
    T_IO_MSG msg;

    msg.type = IO_MSG_TYPE_UART_AT_CMD;
    msg.subtype = subtype;
    msg.u.buf = param_buf;

    return app_io_msg_send(&msg);
}

void app_wifi_uart_msg_handle(T_IO_MSG *io_msg)
{
    IO_WIFI_UART_MSG_TYPE sub_type = (IO_WIFI_UART_MSG_TYPE)io_msg->subtype;
    // APP_PRINT_INFO1("app_wifi_uart_msg_handle: sub_type %d", sub_type);
    switch (sub_type)
    {
    case IO_WIFI_UART_AT_CMD_IND:
        {
            if (rgs_parser_cb)
            {
                rgs_parser_cb();
            }
        }
        break;

    case IO_WIFI_UART_EXIT_DLPS:
        {
            uart_rx_disable(uart3_dev);
            uart_set.buf_index = 0;
            uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
#if WIFI_UART_TX_SHARE_DEBUG
            wifi_uart_tx_pin_release();   /* DLPS resume re-grabs P5_4 as UART3_TX; release it */
#endif
        }
        break;

    case IO_WIFI_UART_SEND_CONNECT:
        {
            /* Runs on the main app task (same context as the proven 0x8440
             * demo). The WiFi task posted this after the blocking bring-up so
             * the ATPN is no longer sent from the WiFi task. */
            if (deferred_connect_cb)
            {
                deferred_connect_cb();
            }
        }
        break;
    default:
        break;
    }
}

void app_wifi_uart_rx_enable(void)
{
    uart_rx_disable(uart3_dev);
    uart_set.buf_index = 0;
    uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
#if WIFI_UART_TX_SHARE_DEBUG
    wifi_uart_tx_pin_release();
#endif
}

extern uint8_t wifi_enable_flag;
static void uart_async_callback(const struct device *dev, struct uart_event *evt, void *user_data)
{
    // APP_PRINT_TRACE1("wifi_uart_async_callback: type %d", evt->type);

    switch (evt->type)
    {
    case UART_TX_DONE:
    case UART_TX_ABORTED:   /* same cleanup: otherwise tx_lock sticks and release() never fires */
        {
            APP_PRINT_INFO2("[wifi][dbg] uart3 TX evt %d (len %d)", evt->type,
                            (evt->type == UART_TX_DONE) ? evt->data.tx.len : 0);   /* TEMP probe */
            // Always release tx_lock after TX completes (done or aborted)
            wifi_console.tx_lock = false;
#if WIFI_UART_TX_SHARE_DEBUG
            /* DON'T release here: UART_TX_DONE means DMA-done, not wire-idle.
             * Defer until the transmitter is physically drained, else the tail
             * of the AT command is chopped and the module never replies. */
            wifi_tx_release_poll = 0;
            k_work_reschedule(&wifi_uart_tx_release_work, K_NO_WAIT);
#endif
        }
        break;
    case UART_RX_RDY:
        {
            /* Dump the whole chunk in <=WIFI_UART_RX_DUMP_SEG-byte segments, so a
             * long chunk is fully logged across multiple lines (offset shown)
             * instead of truncated -- regardless of line terminators. The full
             * chunk is still fed to the parser as-is. */
            uint16_t rx_len = evt->data.rx.len;
            uint8_t *rx_p   = &evt->data.rx.buf[evt->data.rx.offset];
            for (uint16_t off = 0; off < rx_len; off += WIFI_UART_RX_DUMP_SEG)
            {
                uint16_t seg = ((rx_len - off) > WIFI_UART_RX_DUMP_SEG)
                               ? WIFI_UART_RX_DUMP_SEG : (uint16_t)(rx_len - off);
                APP_PRINT_INFO3("[wifi] uart3 RX (%d/%d) data=%b", off, rx_len,
                                TRACE_BINARY(seg, rx_p + off));
            }

            wifi_uart_console_recv(rx_p, rx_len);
            if (app_wifi_uart_msg_send(IO_WIFI_UART_AT_CMD_IND, NULL) == false)
            {
                APP_PRINT_INFO0("wifi msg send fail");
            }
        }
        break;
    case UART_RX_BUF_REQUEST:
        {
            if (uart_set.buf_index == 0)
            {
                uart_set.buf_index = 1;
            }
            else if (uart_set.buf_index == 1)
            {
                uart_set.buf_index = 0;
            }
            uart_rx_buf_rsp(uart3_dev,  &uart_set.rx_buf[uart_set.buf_index * RX_BUF_LENGTH], RX_BUF_LENGTH);
            // APP_PRINT_TRACE1("uart_async_callback: set buf %d", uart_set.buf_index);  // noisy: floods every DLPS cycle
        }
        break;
    default:
        break;
    }
}

void app_wifi_uart_init(void)
{
    if (!device_is_ready(uart3_dev))
    {
        APP_PRINT_ERROR0("[wifi] UART device not found!");
        return;
    }
    if (os_sem_create(&uart_set.mp_sem_handle, "mp_sem", 0, 1) == true)
    {
        APP_PRINT_INFO0("os_sem_create mp_sem success");
    }
    else
    {
        APP_PRINT_ERROR0("os_sem_create mp_sem fail");
    }
    uart_callback_set(uart3_dev, uart_async_callback, NULL);
    uart_set.buf_index = 0;
    uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
#if WIFI_UART_TX_SHARE_DEBUG
    wifi_uart_tx_pin_release();   /* start idle: TX hi-Z so the PC owns the line */
#endif
}

void wifi_uart_console_recv(uint8_t *recv_buf, uint16_t recv_len)
{
    if (recv_buf == NULL || recv_len == 0)
    {
        return;
    }
    if (wifi_console.rx_cnt + recv_len > RX_BUF_LENGTH)
    {
        APP_PRINT_ERROR2("[wifi] uart console buf overflow console_rx_cnt = %d, recv_len = %d", \
                         wifi_console.rx_cnt, recv_len);
        // Reset buffer on overflow to avoid stuck
        wifi_console.rx_cnt = 0;
        wifi_console.read_idx = 0;
        wifi_console.write_idx = 0;
        return;
    }
    if (wifi_console.write_idx + recv_len <= RX_BUF_LENGTH)
    {
        memcpy(&wifi_console.console_buf[wifi_console.write_idx], recv_buf, recv_len);
        wifi_console.rx_cnt += recv_len;
        wifi_console.write_idx += recv_len;
        if (wifi_console.write_idx == RX_BUF_LENGTH)
        {
            wifi_console.write_idx = 0;
        }
    }
    else
    {
        uint32_t temp;
        temp = RX_BUF_LENGTH - wifi_console.write_idx;
        memcpy(&wifi_console.console_buf[wifi_console.write_idx], recv_buf, temp);
        memcpy(&wifi_console.console_buf[0], &recv_buf[temp], recv_len - temp);
        wifi_console.rx_cnt += recv_len;
        wifi_console.write_idx  = recv_len - temp;
    }
}

bool atcmd_send(const uint8_t *tx_data, int size)
{
    if (tx_data == NULL || size <= 0)
    {
        APP_PRINT_ERROR0("[wifi] atcmd_send invalid parameter");
        return false;
    }
    if (wifi_console.tx_lock == false)
    {
        if (size > TX_BUF_LENGTH)
        {
            APP_PRINT_ERROR2("atcmd_send fail, cmd len = %d, buf len = %d", size, TX_BUF_LENGTH);
            return false;
        }
        wifi_console.tx_lock = true;
        memcpy(wifi_console.tx_buf, tx_data, size);
#if WIFI_UART_TX_SHARE_DEBUG
        wifi_uart_tx_pin_acquire();   /* take the shared line for our own TX */
#endif
        {
            int tx_ret = uart_tx(uart3_dev, wifi_console.tx_buf, size, SYS_FOREVER_US);   /* TEMP probe */
            APP_PRINT_INFO2("[wifi][dbg] uart_tx ret %d, len %d", tx_ret, size);
        }
        return true;
    }
    APP_PRINT_WARN0("[wifi] atcmd_send fail, tx is locked");
    return false;
}

uint16_t atcmd_recv(uint8_t *recv_buf, uint16_t recv_len)
{
    if (recv_buf == NULL || recv_len == 0)
    {
        return 0;
    }
    uint16_t read_len = wifi_console.rx_cnt;
    if (wifi_console.rx_cnt)
    {
        if (read_len > recv_len)
        {
            APP_PRINT_ERROR2("[wifi] error atcmd_recv read_len = %d, recv_len = %d", \
                             read_len, recv_len);
            read_len = recv_len;
        }

        if (wifi_console.read_idx + read_len <= RX_BUF_LENGTH)
        {
            memcpy(recv_buf, &wifi_console.console_buf[wifi_console.read_idx], read_len);
            wifi_console.read_idx += read_len;
            if (wifi_console.read_idx == RX_BUF_LENGTH)
            {
                wifi_console.read_idx = 0;
            }
        }
        else
        {
            uint32_t temp;
            temp = RX_BUF_LENGTH - wifi_console.read_idx;
            memcpy(recv_buf, &wifi_console.console_buf[wifi_console.read_idx], temp);
            memcpy(recv_buf + temp, &wifi_console.console_buf[0], read_len - temp);
            wifi_console.read_idx = read_len - temp;
        }
        wifi_console.rx_cnt -= read_len;
        // APP_PRINT_INFO1("recv_buf =%b", TRACE_BINARY(read_len, recv_buf));
    }
    return read_len;
}


int mp_open(void)
{
    int ret = pinctrl_apply_state_direct(pincfg, &uart3_mp_state);
    APP_PRINT_INFO1("mp_open  %d", ret);
    if (ret == 0)
    {
        // Clear console buffer before switching mode
        wifi_console.rx_cnt = 0;
        wifi_console.read_idx = 0;
        wifi_console.write_idx = 0;
        uart_rx_disable(uart3_dev);
        uart_set.buf_index = 0;
        uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    }
    return ret;
}

int mp_close(void)
{
    int ret = pinctrl_apply_state(pincfg, PINCTRL_STATE_DEFAULT);
    APP_PRINT_INFO1("mp_close  %d", ret);
    if (ret == 0)
    {
        // Clear console buffer before switching mode
        wifi_console.rx_cnt = 0;
        wifi_console.read_idx = 0;
        wifi_console.write_idx = 0;
        uart_rx_disable(uart3_dev);
        uart_set.buf_index = 0;
        uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    }
    return ret;
}


void mp_send(uint8_t *cmd_buf, uint16_t cmd_len)
{
    if (cmd_buf == NULL || cmd_len == 0)
    {
        APP_PRINT_ERROR0("[wifi] mp_send invalid parameter");
        return;
    }
    uart_tx(uart3_dev, cmd_buf, cmd_len, SYS_FOREVER_US);
}

uint16_t mp_recv(uint8_t *recv_buf, uint16_t recv_len)
{
    if (recv_buf == NULL || recv_len == 0)
    {
        return 0;
    }
    uint16_t read_len = 0;
    memset(recv_buf, 0, recv_len);
    if (os_sem_take(uart_set.mp_sem_handle, 1200) == true)
    {
        read_len = wifi_console.rx_cnt;
        if (wifi_console.rx_cnt)
        {
            if (read_len > recv_len)
            {
                APP_PRINT_ERROR2("[wifi] error mp_recv read_len = %d, recv_len = %d", \
                                 read_len, recv_len);
                read_len = recv_len;
            }

            if (wifi_console.read_idx + read_len <= RX_BUF_LENGTH)
            {
                memcpy(recv_buf, &wifi_console.console_buf[wifi_console.read_idx], read_len);
                wifi_console.read_idx += read_len;
                if (wifi_console.read_idx == RX_BUF_LENGTH)
                {
                    wifi_console.read_idx = 0;
                }
            }
            else
            {
                uint32_t temp;
                temp = RX_BUF_LENGTH - wifi_console.read_idx;
                memcpy(recv_buf, &wifi_console.console_buf[wifi_console.read_idx], temp);
                memcpy(recv_buf + temp, &wifi_console.console_buf[0], read_len - temp);
                wifi_console.read_idx = read_len - temp;
            }
            wifi_console.rx_cnt -= read_len;
            // APP_PRINT_INFO1("recv_buf =%b", TRACE_BINARY(read_len, recv_buf));
        }
    }
    else
    {
        // APP_PRINT_INFO0("mp_recv os_sem_take failed\n");
    }
    return read_len;
}

int mp_set_baudrate(uint32_t baud)
{
    int ret = 0;
    struct uart_config cfg;
    uart_config_get(uart3_dev, &cfg);
    cfg.baudrate = baud;
    ret = uart_configure(uart3_dev, &cfg);
    if (ret != 0)
    {
        APP_PRINT_ERROR1("[wifi] uart_configure failed, ret = %d", ret);
        return ret;
    }
    uart_set.baud = baud;
    uart_rx_disable(uart3_dev);
    uart_set.buf_index = 0;
    uart_rx_enable(uart3_dev, uart_set.rx_buf, RX_BUF_LENGTH, 0);
    return ret;
}

#endif
