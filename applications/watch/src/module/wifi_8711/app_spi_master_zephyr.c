/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if defined(CONFIG_WIFI_8711_ROLE_MASTER)

#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/ring_buffer.h>
#include <string.h>
#include "rtk_errno.h"
#include "trace.h"
#include "app_spi_api.h"
#include "app_cmd.h"
#include "app_spi_common.h"
#include "app_dlps.h"
#include "psram_section.h"

/* Large SPI TX/RX buffers (SPI_XMIT_SIZE each) live in the shared non-cacheable
 * psram1_nc region (SECTION_PSRAM1_NC), keeping them out of the 82K DTCM. PSRAM
 * is only initialised in app_system_lower_init() during main(), so this section
 * must NOT be zeroed at boot; the control fields are reset explicitly in
 * app_spi_master_init(). */

/*GPIO Device*/
#define RES_NODE  DT_NODELABEL(wifi_8711_resources)
static const struct gpio_dt_spec m2s_gpio = GPIO_DT_SPEC_GET(RES_NODE, m2s_gpios);
static const struct gpio_dt_spec s2m_gpio = GPIO_DT_SPEC_GET(RES_NODE, s2m_gpios);
static struct gpio_callback s2m_cb_data;

/*SPI Device*/
#define SPI_NODE DT_NODELABEL(wifi_8711_device)
static struct spi_dt_spec spi_spec = SPI_DT_SPEC_GET(SPI_NODE,
                                                     SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_LINES_SINGLE,
                                                     0);

/*TX Buffer: deep ring buffer (PSRAM-backed) --------------------------------- *
 * Outgoing frames are cached in a single ring buffer instead of the old 2-slot
 * ping-pong, so producers (the AT engine and the SPI TX-blast task) rarely get
 * SPI_SEND_ERR_BUSY and the SPI bus stays fed back-to-back. This mirrors the
 * master-side RingBuffer pattern of the atcmd_host_spi example.
 *
 * Each enqueued frame is stored as a record: a 2-byte little-endian length
 * prefix followed by the already-framed packet bytes ([AT][len][data][CRC]).
 * The trigger handler pops exactly one record per SPI transaction into
 * tx_dma_buf, so one logical frame still maps to one SPI transfer (the slave
 * protocol is unchanged). The ring storage and the in-flight DMA buffer live in
 * PSRAM; the ring_buf control struct stays in internal RAM (its head/tail are
 * plain reads/writes, no LDREX/STREX on PSRAM). */
#define SPI_TX_RING_FRAMES          4
#define SPI_TX_RING_REC_HDR         2   /* 2-byte LE length prefix per record */
#define SPI_TX_RING_SIZE            ((SPI_XMIT_SIZE + SPI_TX_RING_REC_HDR + 6) * SPI_TX_RING_FRAMES)

static struct ring_buf tx_ring;
static uint8_t tx_ring_storage[SPI_TX_RING_SIZE] __aligned(4) SECTION_PSRAM1_NC;
static uint8_t tx_dma_buf[SPI_XMIT_SIZE] __aligned(32) SECTION_PSRAM1_NC;

/*RX Buffer: Ping-Pong Buffer (control in internal RAM, storage in PSRAM)*/
struct rx_buf_slot
{
    atomic_t in_use;    // 0: idle, 1: using
};
static struct rx_buf_slot rx_slots[2];
static uint8_t rx_storage[2][SPI_XMIT_SIZE] __aligned(32) SECTION_PSRAM1_NC;
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
};

/* Hot-path control state is statically allocated - NO malloc per transaction,
 * mirroring the static MasterTxBuf/MasterRxBuf, alloc-free transfer loop of the
 * atcmd_host_spi reference. Exactly one SPI transfer is ever in flight (it is
 * serialised by master_idle), so a single cb context suffices. RX messages use
 * a small pool indexed by the ping-pong slot: a slot is never re-armed until its
 * frame has been parsed and returned, so its message cannot be overwritten while
 * still queued. This also keeps malloc()/free() out of spi_done_cb, which runs
 * in the SPI completion callback context. */
static struct spi_cb_ctx s_ctx;
static struct rx_msg     s_rx_msgs[2];

/*ASync API*/
static int m2s_gpio_set_level(SPI_GPIO_STATE state);
static void trigger_next_tx(void);

static inline void rx_buf_return(uint8_t index)
{
    if (index < 2) { atomic_set(&rx_slots[index].in_use, 0); }
}

/* TX packet progress (SPI+TCP blast test): when s_tx_pkt_total > 0, each real
 * frame popped from the ring in the trigger logs "cur/total". */
static uint32_t s_tx_pkt_total = 0;
static uint32_t s_tx_pkt_idx   = 0;

void app_spi_master_tx_progress_begin(uint32_t total_pkts)
{
    s_tx_pkt_idx   = 0;
    s_tx_pkt_total = total_pkts;
}

void app_spi_master_tx_progress_end(void)
{
    s_tx_pkt_total = 0;
}

/**
 * @brief Push one already-framed packet into the TX ring as a single record
 *        ([len LE 2B][payload]). The record commits atomically via the ring
 *        claim/finish API, so the consumer never observes a half-written frame.
 *
 * @return true on success, false if the ring has no room for the whole record.
 */
static bool tx_ring_put_record(const uint8_t *data, uint16_t len)
{
    uint32_t total = SPI_TX_RING_REC_HDR + len;

    if (ring_buf_space_get(&tx_ring) < total)
    {
        return false;
    }

    uint8_t  prefix[SPI_TX_RING_REC_HDR] = { (uint8_t)(len & 0xFF), (uint8_t)((len >> 8) & 0xFF) };
    uint32_t off = 0;   /* bytes of the record already copied into the ring */

    while (off < total)
    {
        uint8_t *blk;
        uint32_t claimed = ring_buf_put_claim(&tx_ring, &blk, total - off);
        if (claimed == 0)
        {
            /* space was pre-checked, so this should not happen; undo and bail */
            ring_buf_put_finish(&tx_ring, 0);
            return false;
        }

        uint32_t b = 0;
        while (off < SPI_TX_RING_REC_HDR && b < claimed)
        {
            blk[b++] = prefix[off++];
        }
        if (b < claimed)
        {
            uint32_t n = claimed - b;       /* payload bytes in this block */
            memcpy(&blk[b], &data[off - SPI_TX_RING_REC_HDR], n);
            off += n;
        }
    }

    ring_buf_put_finish(&tx_ring, total);
    return true;
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

    if (msg == NULL)
    {
        APP_PRINT_ERROR0("app_spi_master_rx_parser: null msg");
        return;
    }

    /* Parse straight out of the ping-pong RX storage. The parser only reads the
     * frame and copies out the (small) payload, so there is no need to clone the
     * whole SPI_XMIT_SIZE (16K) buffer onto the heap first - that copy was the
     * single biggest per-frame cost on the downlink flood. The slot is returned
     * to the ping-pong pool once parsing is done. */
    if (rgs_parser_cb)
    {
        rgs_parser_cb(msg->ptr, msg->len);
    }
    rx_buf_return(msg->index);

    /* A slot just freed. If TX is queued and the bus went idle because a trigger
     * had to defer for lack of an rx slot, kick it now so queued TX is not
     * stranded (during a pure downlink flood the ring is empty, so this is a
     * no-op). */
    if (ring_buf_size_get(&tx_ring) >= SPI_TX_RING_REC_HDR &&
        atomic_cas(&master_idle, 1, 0))
    {
        trigger_next_tx();
    }
}

static void trigger_next_tx(void)
{
    /* Hold off DLPS for the entire time the master pump is active: disable here
     * on every transfer start, and re-enable ONLY when the pump goes idle
     * (spi_done_cb with an empty ring, or the transceive-error path).
     *
     * The old code re-enabled per RX-parse, but APP_DLPS_ENTER_CHECK_WIFI_8711 is
     * a single bit (app_dlps.c OR/AND-NOTs it, no refcount), so parsing frame N's
     * RX cleared the lock that frame N+1's trigger had just set - leaving a
     * window where the chip could enter DLPS mid-transfer. A DLPS wake then
     * reconfigured the SPI1 master clock under the in-flight DMA and the
     * completion callback was lost, permanently stalling the blast. */
    app_dlps_disable(APP_DLPS_ENTER_CHECK_WIFI_8711);
    APP_PRINT_TRACE1("trigger_next_tx: queued %u bytes", ring_buf_size_get(&tx_ring));
    m2s_gpio_set_level(SPI_GPIO_STATE_ACTIVE);
}

static void spi_done_cb(const struct device *dev, int result, void *data)
{
    struct spi_cb_ctx *ctx = (struct spi_cb_ctx *)data;
    int rx_idx = ctx->rx_idx;

    if (rx_idx >= 0)
    {
        if (result == 0)
        {
            /* pooled rx_msg (one per ping-pong slot) - no malloc in this
             * callback context. The slot stays in_use until the parser returns
             * it, so this entry cannot be reused while still queued. */
            struct rx_msg *msg = &s_rx_msgs[rx_idx];
            msg->ptr = rx_storage[rx_idx];
            msg->len = SPI_XMIT_SIZE;
            msg->index = (uint8_t)rx_idx;
            if (!app_spi_msg_send(IO_SPI_MASTER_DATA_IN, msg))
            {
                rx_buf_return((uint8_t)rx_idx);
                APP_PRINT_ERROR0("spi_done_cb: app_spi_msg_send failed");
            }
        }
        else
        {
            rx_buf_return((uint8_t)rx_idx);
            APP_PRINT_INFO1("spi async err %d", result);
        }
    }
    m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);

    /* Keep the pipe running: if the ring still holds a frame, trigger it.
     * Otherwise go idle, then re-check once to close the race with a producer
     * that enqueued just after we observed the ring empty (it grabs master_idle
     * via atomic_cas only when we leave it set). */
    if (ring_buf_size_get(&tx_ring) >= SPI_TX_RING_REC_HDR)
    {
        trigger_next_tx();
    }
    else
    {
        atomic_set(&master_idle, 1);
        /* pump idle: allow DLPS again (paired with the disable in
         * trigger_next_tx). If the race-recheck below re-arms a transfer,
         * trigger_next_tx re-disables it, so the net state stays correct. */
        app_dlps_enable(APP_DLPS_ENTER_CHECK_WIFI_8711);
        if (ring_buf_size_get(&tx_ring) >= SPI_TX_RING_REC_HDR &&
            atomic_cas(&master_idle, 1, 0))
        {
            trigger_next_tx();
        }
    }
}

uint8_t app_spi_master_send_raw_data_trigger(void)
{
    APP_PRINT_TRACE0("app_spi_master_send_raw_data_trigger");

    /* 1. Secure a free RX slot FIRST. A full-duplex transfer always clocks
     * SPI_XMIT_SIZE in both directions, and the SPI driver k_malloc()s a
     * SPI_XMIT_SIZE bounce buffer whenever rx_buf is NULL. Under a downlink
     * flood that allocation runs every transfer and exhausts the heap
     * ("spi rx dma malloc fail", -ENOMEM), live-locking the whole path. So if
     * neither ping-pong slot is free, DEFER instead of transferring with a NULL
     * rx_buf: drop this opportunity (the slave re-asserts s2m; rx_buf_return()
     * re-kicks any queued TX once a slot frees). The bus stays throttled by the
     * m2s/s2m handshake, so dropping here just applies back-pressure. */
    int rx_i = atomic_get(&cur_rx_idx);
    if (atomic_get(&rx_slots[rx_i].in_use))
    {
        rx_i ^= 1;
    }
    if (atomic_get(&rx_slots[rx_i].in_use))
    {
        APP_PRINT_WARN0("app_spi_master_send_raw_data_trigger: no rx buf, defer");
        atomic_set(&master_idle, 1);
        m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
        return SPI_SEND_ERR_BUSY;
    }
    uint8_t *rx_dst = rx_storage[rx_i];

    /* 2. Pop the next queued frame into the DMA buffer. The transfer length is
     * always SPI_XMIT_SIZE; the bytes past the frame are left as-is (the slave
     * bounds its read by the length field, so they are never inspected). When the
     * ring is empty the transaction was started by the slave (downlink): send a
     * sync-invalid dummy frame and just clock the RX in. */
    uint16_t frame_len = 0;
    if (ring_buf_size_get(&tx_ring) >= SPI_TX_RING_REC_HDR)
    {
        uint8_t lh[SPI_TX_RING_REC_HDR];
        ring_buf_get(&tx_ring, lh, SPI_TX_RING_REC_HDR);
        frame_len = (uint16_t)(lh[0] | (lh[1] << 8));
        if (frame_len > SPI_XMIT_SIZE) { frame_len = SPI_XMIT_SIZE; }
        ring_buf_get(&tx_ring, tx_dma_buf, frame_len);
    }

    if (frame_len == 0)
    {
        /* RX-only (slave-initiated downlink): no TX payload. Only clobber the
         * magic byte so this dummy frame can't be mistaken for a valid [AT]
         * frame by the slave - the rest of tx_dma_buf is don't-care (the slave
         * bounds its read by the length field, padding never enters the CRC),
         * hence no full-buffer zero-pad. Matches the reference's MasterTxBuf. */
        tx_dma_buf[0] = SPI_CMD_SYNC_INVALID;
        APP_PRINT_TRACE0("Trigger with empty TX ring (RX-only)");
    }
    else if (s_tx_pkt_total)
    {
        /* a real frame popped during a counted blast: log "cur/total" */
        if (s_tx_pkt_idx < s_tx_pkt_total) { s_tx_pkt_idx++; }
        APP_PRINT_INFO2("app_spi_master_send_raw_data_trigger %u/%u",
                        s_tx_pkt_idx, s_tx_pkt_total);
    }

    struct spi_buf tx = { .buf = tx_dma_buf,            .len = SPI_XMIT_SIZE };
    struct spi_buf rx = { .buf = rx_dst,                .len = SPI_XMIT_SIZE };
    struct spi_buf_set tx_set = { .buffers = &tx, .count = 1 };
    struct spi_buf_set rx_set = { .buffers = &rx, .count = 1 };

    /* single static context - one transfer is ever in flight (master_idle) */
    s_ctx.rx_idx = rx_i;

    int ret = spi_transceive_cb(spi_spec.bus, &spi_spec.config, &tx_set, &rx_set,
                                spi_done_cb, &s_ctx);
    if (ret)
    {
        APP_PRINT_ERROR1("app_spi_master_send_raw_data_trigger: ret %d", ret);
        atomic_set(&master_idle, 1);
        /* transceive failed: no completion callback will fire, so release the
         * DLPS lock here (paired with trigger_next_tx) or the chip could never
         * sleep again. */
        app_dlps_enable(APP_DLPS_ENTER_CHECK_WIFI_8711);
        m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
        return SPI_SEND_ERR_BUSY;
    }

    atomic_set(&rx_slots[rx_i].in_use, 1);
    atomic_set(&cur_rx_idx, rx_i ^ 1);

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
        /* Only trigger when m2s is already active (HIGH->LOW happened first) */
        //  if (gpio_pin_get_dt(&m2s_gpio))
        {
            app_spi_msg_send(IO_SPI_MASTER_TRIGGER, NULL);
        }
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
    if (p_data == NULL || len == 0 || len > SPI_XMIT_SIZE)
    {
        return SPI_SEND_ERR_LEN;
    }

    if (!tx_ring_put_record(p_data, len))
    {
        // ring full: caller retries (the AT engine / TX-blast task yield + resend)
        return SPI_SEND_ERR_BUSY;
    }

    if (atomic_cas(&master_idle, 1, 0))
    {
        // bus idle: start the handshake now
        trigger_next_tx();
    }
    else
    {
        // a transfer is in flight; spi_done_cb drains the ring next
        APP_PRINT_TRACE0("app_spi_master_send_data: Packet queued");
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
    ring_buf_init(&tx_ring, sizeof(tx_ring_storage), tx_ring_storage);
    atomic_set(&rx_slots[0].in_use, 0);
    atomic_set(&rx_slots[1].in_use, 0);
    atomic_set(&cur_rx_idx, 0);
    atomic_set(&master_idle, 1);
    /* The SPI RX parser is registered by the AT engine in app_spi_atcmd_init(),
     * which wifi_8711_init() always runs before this. */
    APP_PRINT_INFO0("app_spi_master_init: OK");
}

#endif
