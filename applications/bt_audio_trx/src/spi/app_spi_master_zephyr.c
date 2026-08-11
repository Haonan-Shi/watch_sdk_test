/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_SPI_ROLE_MASTER

#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <string.h>
#include "rtk_errno.h"
#include "trace.h"
#include "app_spi_api.h"
#include "app_cmd.h"
#include "app_spi_common.h"
#include "app_dlps.h"

#define SPI_DROP_WHEN_NO_RX_BUF     1

/*GPIO Device*/
#define RES_NODE  DT_PATH(resources)
static const struct gpio_dt_spec m2s_gpio = GPIO_DT_SPEC_GET(RES_NODE, m2s_gpios);
static const struct gpio_dt_spec s2m_gpio = GPIO_DT_SPEC_GET(RES_NODE, s2m_gpios);
static struct gpio_callback s2m_cb_data;

/*SPI Device*/
#define SPI_NODE DT_NODELABEL(spi0_device)
static struct spi_dt_spec spi_spec = SPI_DT_SPEC_GET(SPI_NODE,
                                                     SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
                                                     0);

/*TX Buffer*/
struct tx_buf_slot
{
    uint8_t storage[SPI_XMIT_SIZE] __aligned(32);
    atomic_t in_use;    // 0: idle, 1: data ready/sending
};

static struct tx_buf_slot tx_slots[2] =
{
    { .in_use = ATOMIC_INIT(0) },
    { .in_use = ATOMIC_INIT(0) },
};

static atomic_t cur_tx_fill_idx = ATOMIC_INIT(0);
static atomic_t cur_tx_send_idx = ATOMIC_INIT(0);

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
static atomic_t master_idle = ATOMIC_INIT(1);
struct spi_cb_ctx
{
    int rx_idx;
    int tx_idx;
};

/*ASync API*/
static int m2s_gpio_set_level(SPI_GPIO_STATE state);

static inline void rx_buf_return(uint8_t index)
{
    if (index < 2) { atomic_set(&rx_slots[index].in_use, 0); }
}

static inline void tx_buf_return(uint8_t index)
{
    if (index < 2) { atomic_set(&tx_slots[index].in_use, 0); }
}

void app_spi_master_module_register(P_SPI_PARSER cb_func, uint16_t mode)
{
    if (rgs_parser_cb == NULL)
    {
        rgs_parser_cb = cb_func;
        spi_spec.config.operation &= ~(SPI_MODE_CPOL | SPI_MODE_CPHA);
        spi_spec.config.operation |= mode;
    }
}

void app_spi_master_rx_parser(void *rx_ctx)
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
    APP_PRINT_ERROR1("app_spi_master_rx_parser: err_code -%d", err_code);
    if (msg != NULL) {free(msg);}
    if (p_data != NULL) {free(p_data);}
    app_dlps_enable(APP_DLPS_ENTER_CHECK_SPI);
}

static void trigger_next_tx(int tx_idx)
{
    app_dlps_disable(APP_DLPS_ENTER_CHECK_SPI);
    APP_PRINT_INFO1("trigger_next_tx: tx idx %d", tx_idx);
    atomic_set(&cur_tx_send_idx, tx_idx);
    m2s_gpio_set_level(SPI_GPIO_STATE_ACTIVE);
}

static void spi_done_cb(const struct device *dev, int result, void *data)
{

    struct spi_cb_ctx *ctx = (struct spi_cb_ctx *)data;
    int rx_idx = ctx->rx_idx;
    int tx_idx = ctx->tx_idx;
    free(ctx);
    if (tx_idx >= 0)
    {
        tx_buf_return((uint8_t)tx_idx);
    }

    if (rx_idx >= 0)
    {
        if (result == 0)
        {
            struct rx_msg *msg = malloc(sizeof(struct rx_msg));
            if (!msg)
            {
                rx_buf_return((uint8_t)rx_idx);
                APP_PRINT_ERROR0("spi_done_cb: malloc rx_msg failed");
            }
            else
            {
                msg->ptr = rx_slots[rx_idx].storage;
                msg->len = SPI_XMIT_SIZE;
                msg->index = (uint8_t)rx_idx;
                if (!app_spi_msg_send(IO_SPI_MASTER_DATA_IN, msg))
                {
                    free(msg);
                    rx_buf_return((uint8_t)rx_idx);
                    APP_PRINT_ERROR0("spi_done_cb: app_spi_msg_send failed");
                }
            }
        }
        else
        {
            rx_buf_return((uint8_t)rx_idx);
            APP_PRINT_INFO1("spi async err %d", result);
        }
    }
    m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);

    int next_tx_idx = tx_idx ^ 1; // 0->1, 1->0
    if (atomic_get(&tx_slots[next_tx_idx].in_use))
    {
        trigger_next_tx(next_tx_idx);
    }
    else
    {
        // no more data
        atomic_set(&master_idle, 1);
    }
}

uint8_t app_spi_master_send_raw_data_trigger(void)
{
    APP_PRINT_INFO0("app_spi_master_send_raw_data_trigger");

    int tx_i = atomic_get(&cur_tx_send_idx);
    uint8_t *tx_src = tx_slots[tx_i].storage;
    if (!atomic_get(&tx_slots[tx_i].in_use))
    {
        // TODO: Use dummy data?
        atomic_set(&tx_slots[tx_i].in_use, 1);
        atomic_set(&cur_tx_fill_idx, tx_i ^ 1);
        memset(tx_src, 0, SPI_XMIT_SIZE);
        tx_src[0] = SPI_CMD_SYNC_INVALID;
        APP_PRINT_WARN0("Trigger with empty TX buf!");
    }

    int rx_i = atomic_get(&cur_rx_idx);
    uint8_t *rx_dst = rx_slots[rx_i].storage;

#if SPI_DROP_WHEN_NO_RX_BUF
    if (atomic_get(&rx_slots[rx_i].in_use))
    {
        rx_dst = NULL; rx_i = -1;
        APP_PRINT_WARN0("app_spi_master_send_raw_data_trigger: no rx buf");
    }
#else
    k_timeout_t wait = K_MSEC(1);
    int spins = 0;
    while (atomic_get(&rx_slots[rx_i].in_use))
    {
        if (spins++ > 5)   //wait 5ms
        {
            rx_dst = NULL;
            rx_i = -1;
            break;
        }
        k_sleep(wait);
    }
#endif

    struct spi_buf tx = { .buf = tx_src,                .len = SPI_XMIT_SIZE };
    struct spi_buf rx = { .buf = rx_dst,                .len = SPI_XMIT_SIZE };
    struct spi_buf_set tx_set = { .buffers = &tx, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = &rx, .count = 1 };

    if (rx_dst == NULL) { rx_i = -1;};

    struct spi_cb_ctx *ctx = malloc(sizeof(struct spi_cb_ctx));
    if (!ctx)
    {
        APP_PRINT_ERROR0("app_spi_slave_listen_async: ctx fail");
        if (rx_i >= 0) { rx_buf_return(rx_i); }
        tx_buf_return(tx_i);
        atomic_set(&master_idle, 1);
        m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
        return SPI_SEND_ERR_BUSY;
    }
    ctx->rx_idx = rx_i;
    ctx->tx_idx = tx_i;

    int ret = spi_transceive_cb(spi_spec.bus, &spi_spec.config, &tx_set, &rx_set,
                                spi_done_cb, ctx);
    if (ret)
    {
        APP_PRINT_ERROR1("app_spi_master_send_raw_data_trigger: ret %d", ret);
        free(ctx);
        if (rx_i >= 0) { rx_buf_return(rx_i); }
        tx_buf_return(tx_i);
        atomic_set(&master_idle, 1);
        m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
        return SPI_SEND_ERR_BUSY;
    }

    if (rx_dst != NULL)
    {
        atomic_set(&rx_slots[rx_i].in_use, 1);
        atomic_set(&cur_rx_idx, rx_i ^ 1);
    }

    return SPI_SEND_SUC;
}

/*GPIO: M2S*/
static int m2s_gpio_set_level(SPI_GPIO_STATE state)
{
    return gpio_pin_set_dt(&m2s_gpio, state);
}

static int m2s_gpio_init(void)
{
    if (!device_is_ready(m2s_gpio.port))
    {
        APP_PRINT_ERROR0("m2s_gpio port not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&m2s_gpio, GPIO_OUTPUT_INACTIVE);
    if (ret)
    {
        APP_PRINT_ERROR1("m2s_gpio configure failed: %d", ret);
        return ret;
    }
    return ESUCCESS;
}

/*GPIO: S2M*/
static void s2m_gpio_isr(const struct device *port, struct gpio_callback *cb, uint32_t pins)
{
    if (pins & BIT(s2m_gpio.pin))
    {
        // app_spi_master_send_raw_data_trigger();
        app_spi_msg_send(IO_SPI_MASTER_TRIGGER, NULL);
    }
}

static int s2m_gpio_init(void)
{
    if (!device_is_ready(s2m_gpio.port))
    {
        APP_PRINT_ERROR0("s2m_gpio port not ready");
        return -ENODEV;
    }

    int ret = gpio_pin_configure_dt(&s2m_gpio, GPIO_INPUT);
    if (ret)
    {
        APP_PRINT_ERROR1("s2m_gpio configure failed: %d", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&s2m_gpio, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret)
    {
        APP_PRINT_ERROR1("s2m_gpio interrupt config failed: %d", ret);
        return ret;
    }

    gpio_init_callback(&s2m_cb_data, s2m_gpio_isr, BIT(s2m_gpio.pin));
    ret = gpio_add_callback(s2m_gpio.port, &s2m_cb_data);
    if (ret)
    {
        APP_PRINT_ERROR1("s2m_gpio add callback failed: %d", ret);
        return ret;
    }

    return ESUCCESS;
}

uint8_t app_spi_master_send_raw_data(uint8_t *p_data, uint16_t len)
{
    int tx_idx = atomic_get(&cur_tx_fill_idx);
    if (atomic_get(&tx_slots[tx_idx].in_use))
    {
        // Pingpong buffer full
        return SPI_SEND_ERR_BUSY;
    }

    if (p_data != NULL && (len == 0 || len > SPI_XMIT_SIZE))
    {
        return SPI_SEND_ERR_LEN;
    }

    uint8_t *p_buf = tx_slots[tx_idx].storage;
    memset(p_buf, 0, SPI_XMIT_SIZE);

    if (p_data != NULL)
    {
        memcpy(p_buf, p_data, len);
    }

    atomic_set(&tx_slots[tx_idx].in_use, 1);
    atomic_set(&cur_tx_fill_idx, tx_idx ^ 1);

    if (atomic_cas(&master_idle, 1, 0))
    {
        // send directly
        trigger_next_tx(tx_idx);
    }
    else
    {
        // wait spi_done_cb to trigger next
        APP_PRINT_INFO0("app_spi_master_send_data: Packet queued");
    }

    return SPI_SEND_SUC;
}

/*Initiate API*/
void app_spi_master_init(void)
{
    if (!spi_is_ready_dt(&spi_spec))
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
    atomic_set(&tx_slots[0].in_use, 0);
    atomic_set(&tx_slots[1].in_use, 0);
    atomic_set(&cur_tx_fill_idx, 0);
    atomic_set(&cur_tx_send_idx, 0);
    atomic_set(&master_idle, 1);
    if (rgs_parser_cb == NULL)
    {
        // register a default parser if there is no callback registered
        app_spi_cmd_parser_init();
    }
    APP_PRINT_INFO0("app_spi_master_init: OK");
}

#endif
