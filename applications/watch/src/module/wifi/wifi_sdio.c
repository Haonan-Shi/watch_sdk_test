/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sd/sd.h>
#include <zephyr/sd/sdio.h>
#include <zephyr/drivers/gpio.h>
#include "trace.h"
#include "string.h"
#include "wifi_sdio.h"
#include "wifi_app.h"
#include "wifi_desc.h"
#include "os_queue.h"
#include "os_mem.h"


const struct device *sdhc_dev = DEVICE_DT_GET(DT_ALIAS(sdhc0));
static const struct gpio_dt_spec wifi_int = GPIO_DT_SPEC_GET(DT_ALIAS(wifi_int), gpios);
struct sdio_func wifi_sdio_func;
static struct gpio_callback wifi_int_cb_data;
struct sd_card card;

uint32_t read_buf[4096 / 4];
uint16_t buf_size = 4096;
uint16_t tx_bd_num = 0;
static uint8_t wifi_tx_seq_num = 0;

T_WIFI_SDIO_HDL  sdio_data_hdls[2];
T_OS_QUEUE  sdio_write_queue;

uint16_t wifi_sdio_tx_bd_update(void);

int wifi_sdio_init(void)
{
    if (!device_is_ready(sdhc_dev))
    {
        APP_PRINT_ERROR0("[wifi] SDIO device not ready");
        return -ENODEV;
    }
    int ret = sd_init(sdhc_dev, &card);
    if (ret)
    {
        APP_PRINT_ERROR1("[wifi] sdio_init ret = %d", ret);
        return ret;
    }

    ret = sdio_init_func(&card, &wifi_sdio_func, 1);
    if (ret)
    {
        APP_PRINT_ERROR1("[wifi] sdio_init_func ret = %d", ret);
        return ret;
    }

    ret = sdio_set_block_size(&wifi_sdio_func, wifi_sdio_func.cis.max_blk_size);
    if (ret)
    {
        APP_PRINT_ERROR0("[wifi]: set block size failed\n");
        return ret;
    }

    uint8_t read_val = 0;
    uint32_t read_reg;
    uint32_t write_data = 0xFFFFFFFF;

    sdio_write_byte(&card.func0, 0x110, 0x00);
    sdio_write_byte(&card.func0, 0x111, 0x02);
    sdio_read_byte(&card.func0, SDIO_REG_STATIS_RECOVERY_TIMOUT, &read_val);
    sdio_write_byte(&card.func0, SDIO_REG_STATIS_RECOVERY_TIMOUT, 0x02);
    sdio_read_byte(&card.func0, 0x03, &read_val);
    sdio_write_byte(&card.func0, 0x110, 0x00);
    sdio_write_byte(&card.func0, 0x111, 0x02);
    sdio_read_byte(&wifi_sdio_func, SDIO_REG_CPU_IND, &read_val);

    sdio_read_addr(&wifi_sdio_func, SDIO_REG_FREE_TXBD_NUM, (uint8_t *)&read_reg, 4);

    sdio_write_byte(&wifi_sdio_func, SDIO_REG_AVAI_BD_NUM_TH_L, 0x16);
    sdio_write_byte(&wifi_sdio_func, 0xD1, 0x00);
    sdio_write_byte(&wifi_sdio_func, SDIO_REG_AVAI_BD_NUM_TH_H, 0x0b);
    sdio_write_byte(&wifi_sdio_func, 0xD5, 0x00);

    sdio_write_addr(&wifi_sdio_func, SDIO_REG_HISR, (uint8_t *)&write_data, 4);
    write_data = 0;
    sdio_write_addr(&wifi_sdio_func, SDIO_REG_HIMR, (uint8_t *)&write_data, 4);

    sdio_read_byte(&wifi_sdio_func, 0x21, &read_val);
    sdio_read_byte(&wifi_sdio_func, SDIO_REG_FREE_TXBD_NUM, &read_val);
    sdio_read_byte(&wifi_sdio_func, SDIO_REG_TXBUF_UNIT_SZ, &read_val);

    sdio_write_addr(&wifi_sdio_func, SDIO_REG_HIMR, (uint8_t *)&write_data, 4);

    sdio_read_byte(&card.func0, SDIO_REG_32K_TRANS_IDLE_TIME, &read_val);
    sdio_write_byte(&card.func0, SDIO_REG_32K_TRANS_IDLE_TIME, 0x03);

    write_data = 0x40001;
    sdio_write_addr(&wifi_sdio_func, SDIO_REG_HIMR, (uint8_t *)&write_data, 4);

    tx_bd_num = wifi_sdio_tx_bd_update();
    APP_PRINT_INFO1("tx_bd_num = %d", tx_bd_num);

    wifi_sdio_int_init();

    return 0;
}

void wifi_sdio_int_init(void)
{
    if (!device_is_ready(wifi_int.port))
    {
        APP_PRINT_ERROR0("Error: wifi int pin is not ready!\n");
        return;
    }

    if (gpio_pin_configure_dt(&wifi_int,
                              GPIO_INPUT | GPIO_PULL_UP))//DT_GPIO_PIN(DT_ALIAS(wifi_int), gpios)
    {
        APP_PRINT_ERROR0("Error: Failed to configure wifi int pin\n");
        return;
    }

    if (gpio_pin_interrupt_configure_dt(&wifi_int, GPIO_INT_EDGE_TO_ACTIVE))
    {
        APP_PRINT_ERROR0("Error: Failed to configure wifi interrupt\n");
        return;
    }

    gpio_init_callback(&wifi_int_cb_data, wifi_sdio_int_handler, BIT(wifi_int.pin));
    gpio_add_callback(wifi_int.port, &wifi_int_cb_data);

}

void wifi_sdio_int_handler(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    T_WIFI_MSG sdio_msg;
    sdio_msg.event = EVENT_SDIO_INT;
    if (app_send_msg_to_wifitask(&sdio_msg) == false)
    {
        APP_PRINT_ERROR0("SDIO int msg send failed!");
    }
}

bool wifi_sdio_read_data(uint8_t *read_data, uint16_t *size)
{
    bool ret = false;
    static uint32_t fifo_cnt = 0;
    uint32_t sdio_hisr = 0;
    uint32_t sdio_himr = 0;
    uint32_t rx_len = 0;

    sdio_read_addr(&wifi_sdio_func, SDIO_REG_HISR, (uint8_t *)&sdio_hisr, 4);

    // APP_PRINT_INFO1("wifi_sdio_read_data sdio_hisr 0x%x!", sdio_hisr);
    // sdio_read_addr(&wifi_sdio_func, SDIO_REG_HIMR, (uint8_t *)&sdio_himr, 4);
    // APP_PRINT_INFO1("wifi_sdio_read_data sdio_himr 0x%x!", sdio_himr);
    // sdio_himr = 0;
    // uint8_t rx_ready = 0;
    // sdio_read_byte(&wifi_sdio_func, SDIO_REG_RX0_REQ_LEN+3, &rx_ready);
    // if (rx_ready & BIT7)
    {
        sdio_read_addr(&wifi_sdio_func, SDIO_REG_RX0_REQ_LEN, (uint8_t *)&rx_len, 4);
    }

    rx_len &= 0xFFFF;
    rx_len = ((rx_len >> 2) + ((rx_len & 3) ? 1 : 0)) << 2; //align 4 bytes
    if (rx_len > *size)
    {
        return ret;
    }

    sdio_himr = SDIO_HIMR_RX_REQUEST_MSK | SDIO_HIMR_AVAL_MSK | SDIO_HIMR_CPWM1_MSK;
    if (sdio_hisr & sdio_himr)
    {
        sdio_hisr &= sdio_himr;
        uint32_t v32 = sdio_hisr & MASK_SDIO_HISR_CLEAR;
        if (v32)
        {
            sdio_write_addr(&wifi_sdio_func, SDIO_REG_HISR, (uint8_t *)&v32, 4);
        }
    }

    if (sdio_hisr & SDIO_HISR_AVAL_INT)
    {
        uint32_t freepage;
        sdio_read_addr(&wifi_sdio_func, SDIO_REG_FREE_TXBD_NUM, (uint8_t *)&freepage, 4);
    }

    if (sdio_hisr & SDIO_HISR_RX_REQUEST)
    {
        sdio_hisr ^= SDIO_HISR_RX_REQUEST;

        do
        {
            if (rx_len == 0)
            {
                // uint8_t rx_ready = 0;
                // sdio_read_byte(&wifi_sdio_func, SDIO_REG_RX0_REQ_LEN+3, &rx_ready);
                // if (rx_ready & BIT7)
                {
                    sdio_read_addr(&wifi_sdio_func, SDIO_REG_RX0_REQ_LEN, (uint8_t *)&rx_len, 4);
                    rx_len &= 0xFFFF;
                }
            }


            if (rx_len)
            {
                uint32_t reg = (WLAN_RX_FIFO_DEVICE_ID << 13) | (fifo_cnt & 0x3);
                fifo_cnt++;
                sdio_read_addr(&wifi_sdio_func, reg, read_data, rx_len);
                *size = rx_len;
                ret = true;
                break;
            }

        }
        while (1);
    }

    if (sdio_hisr & SDIO_HISR_CPWM1)
    {
        //Register HCPWM
        uint8_t cpwm;
        sdio_read_byte(&wifi_sdio_func, SDIO_REG_HCPWM, &cpwm);
        //DBG_DIRECT("WIFI_OFF!HCPWM@(0x%02x)\n", cpwm);
    }

    return ret;
}

void wifi_gpio_disable(void)
{
    gpio_pin_interrupt_configure_dt(&wifi_int, GPIO_INT_DISABLE);
}

void wifi_gpio_enable(void)
{
    gpio_pin_interrupt_configure_dt(&wifi_int, GPIO_INT_EDGE_TO_ACTIVE);
}


uint16_t wifi_sdio_tx_bd_update(void)
{
    uint8_t tx_bd[2] = {0};
    uint32_t reg;

    reg = (SDIO_LOCAL_DEVICE_ID << 13) | (SDIO_REG_FREE_TXBD_NUM + 1);
    sdio_read_byte(&wifi_sdio_func, reg, &tx_bd[1]);
    reg = (SDIO_LOCAL_DEVICE_ID << 13) | (SDIO_REG_FREE_TXBD_NUM);
    sdio_read_byte(&wifi_sdio_func, reg, &tx_bd[0]);

    return (tx_bd[1] << 8) | tx_bd[0];
}

// size align 4 bytes
// if size > block_size, write data len should align n * block_size
int wifi_sdio_write_data(T_WIFI_SDIO_WRITE_QUEUE *write_pkt)
{
    tx_bd_num = wifi_sdio_tx_bd_update();
    APP_PRINT_INFO1("tx_bd_num = %d", tx_bd_num);
    if (tx_bd_num == 0)
    {
        return -1;
    }

    uint8_t *p_write = NULL;
    uint16_t size = 0;
    if (write_pkt)
    {
        p_write = (uint8_t *)&write_pkt->tx_desc;
        size = (sizeof(TXDESC) + write_pkt->tx_desc.txpktsize + 511) & (~511);
        uint32_t reg = (WLAN_TX_FIFO_DEVICE_ID << 13) | ((size >> 2) & WLAN_TX_FIFO_MSK);
        sdio_write_addr(&wifi_sdio_func, reg, p_write, size);
        tx_bd_num = wifi_sdio_tx_bd_update();
    }
    else
    {
        APP_PRINT_ERROR0("SDIO write packet is null !");
        return -1;
    }

    return 0;
}

bool wifi_sdio_data_read_cb_reg(uint32_t ip_addr, uint16_t port, P_FUN_SDIO_DATA_CB cb)
{
    bool ret = false;

    if (cb == NULL)
    {
        return ret;
    }

    for (uint8_t i = 0; i < sizeof(sdio_data_hdls) / sizeof(T_WIFI_SDIO_HDL); i++)
    {
        if (sdio_data_hdls[i].used == false)
        {
            sdio_data_hdls[i].ip_addr = ip_addr;
            sdio_data_hdls[i].port = port;
            sdio_data_hdls[i].sdio_data_read_cb = cb;
            sdio_data_hdls[i].used = true;
            ret = true;
            break;
        }
    }

    return ret;
}

bool wifi_sdio_data_read_cb_unreg(uint32_t ip_addr, uint16_t port)
{
    bool ret = false;

    for (uint8_t i = 0; i < sizeof(sdio_data_hdls) / sizeof(T_WIFI_SDIO_HDL); i++)
    {
        if (sdio_data_hdls[i].used)
        {
            if ((sdio_data_hdls[i].ip_addr == ip_addr) && \
                (sdio_data_hdls[i].port == port))
            {
                sdio_data_hdls[i].ip_addr = 0;
                sdio_data_hdls[i].port = 0;
                sdio_data_hdls[i].sdio_data_read_cb = 0;
                sdio_data_hdls[i].used = false;
                ret = true;
                break;
            }
        }
    }

    return ret;
}

P_FUN_SDIO_DATA_CB wifi_sdio_find_data_read_cb(uint32_t ip_addr, uint16_t port)
{
    P_FUN_SDIO_DATA_CB  cb = NULL;

    for (uint8_t i = 0; i < sizeof(sdio_data_hdls) / sizeof(T_WIFI_SDIO_HDL); i++)
    {
        if (sdio_data_hdls[i].used)
        {
            // if ((sdio_data_hdls[i].ip_addr == ip_addr) &&
            //     (sdio_data_hdls[i].port == port))

            {
                cb = sdio_data_hdls[i].sdio_data_read_cb;
                break;
            }
        }
    }

    return cb;
}

bool wifi_sdio_data_write_queue_fill(uint32_t ip_addr, uint16_t port, uint8_t *data, uint16_t len)
{
    T_WIFI_SDIO_WRITE_QUEUE *sdio_write_pkt;

    uint16_t tx_align_size = (sizeof(TXDESC) + len + 511) & (~511) + sizeof(sdio_write_pkt->p_next);

    sdio_write_pkt = (T_WIFI_SDIO_WRITE_QUEUE *)os_mem_aligned_alloc(OS_MEM_TYPE_DATA,
                                                                     tx_align_size, 4);
    if (sdio_write_pkt == NULL)
    {
        APP_PRINT_ERROR1("[wifi] sdio write queue fill get buffer error tx_align_size =%d", tx_align_size);
        return false;
    }

    sdio_write_pkt->tx_desc.offset = SIZE_TX_DESC;
    sdio_write_pkt->tx_desc.bus_agg_num = 1;
    sdio_write_pkt->tx_desc.type = TX_PACKET_USER;
    sdio_write_pkt->tx_desc.txpktsize = len;
    sdio_write_pkt->tx_desc.seq = wifi_tx_seq_num;
    sdio_write_pkt->tx_desc.ext_desc.ip_addr = ip_addr;
    sdio_write_pkt->tx_desc.ext_desc.port = port;
    sdio_write_pkt->tx_desc.ext_desc.seq = wifi_tx_seq_num;
    wifi_tx_seq_num++;
    memcpy(sdio_write_pkt->data, data, len);
    os_queue_in(&sdio_write_queue, sdio_write_pkt);

    return true;
}

void *wifi_sdio_data_write_queue_peek(int offset)
{
    void *sdio_write_pkt = os_queue_peek(&sdio_write_queue, offset);
    return sdio_write_pkt;
}

void wifi_sdio_data_write_queue_flush(uint16_t cnt)
{
    T_WIFI_SDIO_WRITE_QUEUE *sdio_write_pkt;
    APP_PRINT_TRACE1("[wifi] wifi_sdio_data_write_queue_flush: %d", cnt);
    if (cnt > sdio_write_queue.count)
    {
        cnt = sdio_write_queue.count;
    }
    for (uint16_t i = 0; i < cnt; i++)
    {
        sdio_write_pkt = os_queue_out(&sdio_write_queue);
        os_mem_aligned_free(sdio_write_pkt);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////////////


void wifi_sdio_msg_handler(void)
{
    buf_size = 4096;
    uint8_t *p_read = (uint8_t *)read_buf;
    wifi_sdio_read_data(p_read, &buf_size);

    APP_PRINT_INFO1("sdio read data buf_size = %d", buf_size);
    PRXDESC p_rxdesc = (PRXDESC)p_read;
    if (p_rxdesc->pkt_len)
    {
        P_FUN_SDIO_DATA_CB data_read_cb = NULL;
        data_read_cb = wifi_sdio_find_data_read_cb(p_rxdesc->ext_desc.ip_addr, p_rxdesc->ext_desc.port);
        if (data_read_cb)
        {
            data_read_cb(p_read, buf_size);
        }
    }
}
