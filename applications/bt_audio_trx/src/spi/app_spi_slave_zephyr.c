/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_SPI_ROLE_SLAVE

#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <string.h>
#include "rtk_errno.h"
#include "trace.h"
// #include "system_status_api.h"
#include "pm.h"
#include "app_spi_api.h"
#include "app_cmd.h"
#include "app_spi_common.h"
#include "app_dlps.h"

#define SPI_DROP_WHEN_NO_RX_BUF     1
#define SPI_TX_SLOTS                3

/*Power Save Device*/
#define SPI_AUTO_SLOW_CLK           4

/*GPIO Device*/
#define RES_NODE  DT_PATH(resources)
static const struct gpio_dt_spec m2s_gpio = GPIO_DT_SPEC_GET(RES_NODE, m2s_gpios);
static const struct gpio_dt_spec s2m_gpio = GPIO_DT_SPEC_GET(RES_NODE, s2m_gpios);
static struct gpio_callback m2s_cb_data;

/*SPI Device*/
#define SPI_NODE DT_NODELABEL(spi0_slave)

static const struct device *slave_dev = DEVICE_DT_GET(SPI_NODE);
static struct spi_config slave_config =
{
    .operation = (SPI_OP_MODE_SLAVE | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE)
};
// static struct spi_dt_spec spi_slave_spec = SPI_DT_SPEC_GET(
//         SPI_NODE,
//         SPI_OP_MODE_SLAVE | SPI_MODE_CPOL | SPI_MODE_CPHA | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
//         0);

/*TX Buffer*/
struct tx_buf_slot
{
    uint8_t storage[SPI_XMIT_SIZE] __aligned(32);
    atomic_t in_use;       // 0: free, 1: ready
    atomic_t scheduled;    // 0: not scheduled to DMA, 1: scheduled
};
static struct tx_buf_slot tx_slots[SPI_TX_SLOTS] =
{
    { .in_use = ATOMIC_INIT(0), .scheduled = ATOMIC_INIT(0) },
    { .in_use = ATOMIC_INIT(0), .scheduled = ATOMIC_INIT(0) },
    { .in_use = ATOMIC_INIT(0), .scheduled = ATOMIC_INIT(0) },
};
uint8_t default_ff_tx[SPI_XMIT_SIZE] __aligned(32) = {0};
static atomic_t tx_wr_idx = ATOMIC_INIT(0);
static atomic_t tx_rd_idx = ATOMIC_INIT(0);

/*RX Buffer: Ping-Pong Buffer*/
struct rx_buf_slot
{
    uint8_t storage[SPI_XMIT_SIZE] __aligned(32);
    atomic_t in_use;    // 0: idle, 1: using
};
static struct rx_buf_slot rx_slots[2] =
{
    { .in_use = ATOMIC_INIT(0) },
    { .in_use = ATOMIC_INIT(0) },
};
static atomic_t cur_rx_idx = ATOMIC_INIT(0);

struct rx_msg
{
    uint8_t *ptr;
    uint16_t len;
    uint8_t index;
};

/*RX Parser*/
static P_SPI_PARSER rgs_parser_cb = NULL;

/*SPI State*/
struct spi_cb_ctx
{
    uint8_t rx_idx;
    int tx_slot_idx;      // -1 indicates default_ff_tx
};
/*SPI Seq*/
static uint8_t spi_tx_seq = 0;

/*Functions*/
static int s2m_gpio_set_level(SPI_GPIO_STATE state);

/*ASync API*/
static inline void rx_buf_return(uint8_t index)
{
    atomic_set(&rx_slots[index].in_use, 0);
}

static int pick_idle_rx_slot(void)
{
    int a = atomic_get(&cur_rx_idx);
    if (!atomic_get(&rx_slots[a].in_use))
    {
        return a;
    }
    int b = a ^ 1;
    if (!atomic_get(&rx_slots[b].in_use))
    {
        return b;
    }
    return -1;
}

void app_spi_slave_module_register(P_SPI_PARSER cb_func, uint16_t mode)
{
    if (rgs_parser_cb == NULL)
    {
        rgs_parser_cb = cb_func;
        slave_config.operation &= ~(SPI_MODE_CPOL | SPI_MODE_CPHA);
        slave_config.operation |= mode;
    }
}

void app_spi_slave_rx_parser(void *rx_ctx)
{
    struct rx_msg *msg = (struct rx_msg *)rx_ctx;
    uint8_t err_code = 0;
    uint8_t *p_data = NULL;

    if (msg == NULL)
    {
        err_code = 1;
        goto ERR;
    }
    uint8_t idx = msg->index;

    p_data = malloc(SPI_XMIT_SIZE);
    if (p_data == NULL)
    {
        err_code = 2;
        goto ERR;
    }
    memcpy(p_data, msg->ptr, SPI_XMIT_SIZE);
    rx_buf_return(idx);

    if (rgs_parser_cb)
    {
        rgs_parser_cb(p_data, SPI_XMIT_SIZE);
    }

    free(p_data);
    free(msg);
    app_dlps_enable(APP_DLPS_ENTER_CHECK_SPI);
    return;
ERR:
    APP_PRINT_ERROR1("app_spi_master_cmd_set_parser: err_code -%d", err_code);
    if (msg != NULL) {free(msg);}
    if (p_data != NULL) {free(p_data);}
    app_dlps_enable(APP_DLPS_ENTER_CHECK_SPI);
}

static void spi_slave_done_cb(const struct device *dev, int result, void *data)
{
    // sys_hall_auto_sleep_in_idle(true);
    pm_cpu_slow_freq_set(1);
    struct spi_cb_ctx *ctx = (struct spi_cb_ctx *)data;
    int rx_i = ctx->rx_idx;

    if (result == SPI_XMIT_SIZE)
    {
        struct rx_msg *msg = malloc(sizeof(struct rx_msg));
        if (!msg)
        {
            atomic_set(&rx_slots[rx_i].in_use, 0);
            free(ctx);
            s2m_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
            APP_PRINT_ERROR0("spi_slave_done_cb: malloc rx_msg failed");
            return;
        }
        msg->ptr = rx_slots[rx_i].storage;
        msg->len = SPI_XMIT_SIZE;
        msg->index = (uint8_t)rx_i;
        if (!app_spi_msg_send(IO_SPI_SLAVE_DATA_IN, msg))
        {
            atomic_set(&rx_slots[rx_i].in_use, 0);
            free(ctx);
            free(msg);
            s2m_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
            APP_PRINT_ERROR0("spi_slave_done_cb: app_spi_msg_send failed");
            return;
        }
    }
    else
    {
        atomic_set(&rx_slots[rx_i].in_use, 0);
        APP_PRINT_ERROR1("spi_slave_done_cb: err %d", result);
    }

    atomic_set(&cur_rx_idx, rx_i ^ 1);

    if (ctx->tx_slot_idx >= 0)   // Not all FF
    {
        int tx_idx = ctx->tx_slot_idx;
        struct tx_buf_slot *txslot = &tx_slots[tx_idx];
        atomic_set(&txslot->scheduled, 0);
        atomic_set(&txslot->in_use, 0);

        int tx_r = atomic_get(&tx_rd_idx);
        if (tx_idx == tx_r)
        {
            atomic_set(&tx_rd_idx, (tx_r + 1) % SPI_TX_SLOTS);
        }
    }
    free(ctx);
    s2m_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
}

uint8_t app_spi_slave_preset_tx_data(uint8_t *p_data, uint16_t len)
{
    if (len > SPI_XMIT_SIZE) { return SPI_SEND_ERR_LEN; }

    int w = atomic_get(&tx_wr_idx);
    struct tx_buf_slot *slot = &tx_slots[w];
    if (atomic_get(&slot->in_use))
    {
        return SPI_SEND_ERR_BUSY; // full or busy
    }
    memset(slot->storage, 0, SPI_XMIT_SIZE);
    if (p_data && len)
    {
        memcpy(slot->storage, p_data, len);
    }
    atomic_set(&slot->in_use, 1);
    atomic_set(&tx_wr_idx, (w + 1) % SPI_TX_SLOTS);
    return SPI_SEND_SUC;
}

// IO_SPI_SLAVE_TRIGGER
uint8_t app_spi_slave_listen_async(void)
{
    APP_PRINT_TRACE0("app_spi_slave_listen_async");
    app_dlps_disable(APP_DLPS_ENTER_CHECK_SPI);
    int rx_i = pick_idle_rx_slot();

#if SPI_DROP_WHEN_NO_RX_BUF
    if (rx_i < 0)
    {
        APP_PRINT_WARN0("app_spi_slave_listen_async: both rx are using");
        s2m_gpio_set_level(SPI_GPIO_STATE_ACTIVE);
        k_busy_wait(20);
        s2m_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
        return SPI_SEND_ERR_BUSY;
    }
#else
    k_timeout_t wait = K_MSEC(1);
    int spins = 0;
    while (atomic_get(&rx_slots[rx_i].in_use))
    {
        if (spins++ > 5)
        {
            APP_PRINT_WARN1("app_spi_slave_listen_async: rx_i %d is using", rx_i);
            s2m_gpio_set_level(SPI_GPIO_STATE_ACTIVE);
            k_busy_wait(20);
            s2m_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
            return SPI_SEND_ERR_BUSY;
        }
        k_sleep(wait);
    }
#endif

    const uint8_t *tx_src = NULL;
    bool using_default_ff = false;
    int tx_r = atomic_get(&tx_rd_idx);
    struct tx_buf_slot *txslot = &tx_slots[tx_r];
    if (atomic_get(&txslot->in_use))
    {
        tx_src = txslot->storage;
    }
    else
    {
        tx_src = default_ff_tx;
        using_default_ff = true;
    }

    struct spi_buf tx = { .buf = (void *)tx_src,         .len = SPI_XMIT_SIZE };
    struct spi_buf rx = { .buf = rx_slots[rx_i].storage, .len = SPI_XMIT_SIZE };
    struct spi_buf_set tx_set = { .buffers = &tx, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = &rx, .count = 1 };

    struct spi_cb_ctx *ctx = malloc(sizeof(struct spi_cb_ctx));
    if (!ctx)
    {
        APP_PRINT_ERROR0("app_spi_slave_listen_async: ctx fail");
        return SPI_SEND_ERR_BUSY;
    }
    ctx->rx_idx = (uint8_t)rx_i;
    ctx->tx_slot_idx = using_default_ff ? -1 : tx_r;

    int ret = spi_transceive_cb(slave_dev, &slave_config,
                                &tx_set, &rx_set,
                                spi_slave_done_cb, ctx);
    if (ret)
    {
        APP_PRINT_ERROR1("app_spi_slave_listen_async: ret %d", ret);
        free(ctx);
        return SPI_SEND_ERR_BUSY;
    }
    // sys_hall_auto_sleep_in_idle(false);
    pm_cpu_slow_freq_set(SPI_AUTO_SLOW_CLK);

    s2m_gpio_set_level(SPI_GPIO_STATE_ACTIVE);
    atomic_set(&rx_slots[rx_i].in_use, 1);
    if (!using_default_ff)
    {
        atomic_set(&txslot->scheduled, 1);
    }
    APP_PRINT_TRACE1("app_spi_slave_listen_async: dummy %d", using_default_ff);
    return SPI_SEND_SUC;
}

/*GPIO: S2M*/
static int s2m_gpio_set_level(SPI_GPIO_STATE state)
{
    return gpio_pin_set_dt(&s2m_gpio, state);
}

static int s2m_gpio_init(void)
{
    if (!device_is_ready(s2m_gpio.port))
    {
        APP_PRINT_ERROR0("s2m_gpio port not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&s2m_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret)
    {
        APP_PRINT_ERROR1("s2m_gpio configure failed: %d", ret);
        return ret;
    }

    return ESUCCESS;
}

/*GPIO: M2S*/
static void m2s_gpio_isr(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
    if (pins & BIT(m2s_gpio.pin))
    {
        app_spi_msg_send(IO_SPI_SLAVE_TRIGGER, NULL);
    }
}

static int m2s_gpio_init(void)
{
    if (!device_is_ready(m2s_gpio.port))
    {
        APP_PRINT_ERROR0("m2s_gpio port not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&m2s_gpio, GPIO_INPUT);
    if (ret)
    {
        APP_PRINT_ERROR1("m2s_gpio configure failed: %d", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&m2s_gpio, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret)
    {
        APP_PRINT_ERROR1("m2s_gpio interrupt config failed: %d", ret);
        return ret;
    }

    gpio_init_callback(&m2s_cb_data, m2s_gpio_isr, BIT(m2s_gpio.pin));
    ret = gpio_add_callback(m2s_gpio.port, &m2s_cb_data);
    if (ret)
    {
        APP_PRINT_ERROR1("m2s_gpio add callback failed: %d", ret);
        return ret;
    }

    return ESUCCESS;
}

/*Initiate API*/
void app_spi_slave_init(void)
{
    if (!device_is_ready(slave_dev))
    {
        APP_PRINT_INFO0("SPI dev not ready");
        return;
    }
    if (s2m_gpio_init() != ESUCCESS)
    {
        APP_PRINT_INFO0("S2M not ready");
        return;
    }
    if (m2s_gpio_init() != ESUCCESS)
    {
        APP_PRINT_INFO0("M2S not ready");
        return;
    }

    memset(default_ff_tx, 0xFF, SPI_XMIT_SIZE);
    for (size_t i = 0; i < SPI_XMIT_SIZE - 1; i++)
    {
        default_ff_tx[i + 1] = i + 1; // for test
    }

    if (rgs_parser_cb == NULL)
    {
        // register a default parser if there is no callback registered
        app_spi_cmd_parser_init();
    }
    APP_PRINT_INFO0("app_spi_slave_init: OK");
}

uint8_t app_spi_slave_send_raw_data(uint8_t *p_data, uint16_t len)
{
    uint16_t total_len = len + 6;

    if (p_data != NULL && (len == 0 || len > SPI_XMIT_SIZE))
    {
        return SPI_SEND_ERR_LEN;
    }

    int w = atomic_get(&tx_wr_idx);
    struct tx_buf_slot *slot = &tx_slots[w];
    if (atomic_get(&slot->in_use))
    {
        return SPI_SEND_ERR_BUSY; // full or busy
    }

    uint8_t *p_buf = tx_slots[w].storage;
    memset(p_buf, 0, SPI_XMIT_SIZE);

    if (p_data != NULL)
    {
        memcpy(p_buf, p_data, len);
    }

    atomic_set(&slot->in_use, 1);
    atomic_set(&tx_wr_idx, (w + 1) % SPI_TX_SLOTS);
    return SPI_SEND_SUC;
}
#endif
