/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

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
#include <zephyr/linker/devicetree_regions.h>
#include <zephyr/kernel.h>   /* k_cycle_get_32() for the TP phase profiler */

/* ---- TP uplink phase profiler ---------------------------------------------
 * Splits each master-initiated frame's wall-clock into four phases to localise
 * the per-frame overhead that caps the blast throughput (measured ~7Mbps while
 * the SPI bus itself only clocks ~2.6ms of a ~18ms frame):
 *   hs      m2s asserted -> slave asserts s2m    (slave response latency)
 *   hop     s2m ISR -> transfer submitted        (master ISR->task hop)
 *   xfer    transfer submitted -> completion cb  (actual SPI clocking)
 *   period  m2s(N) -> m2s(N+1)                    (whole per-frame cadence)
 * If hs dominates, the master is waiting on the slave to signal (the ring then
 * stays full purely as back-pressure); if hop/period-minus-the-rest dominate,
 * the cost is master-side software. Averages are printed once per
 * SPI_TP_PROFILE_N frames (NOT per frame) so the profiler stays off the hot
 * path, and only while a counted blast runs (s_tx_pkt_total != 0).
 * k_cycle_get_32() ticks at CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC. Set
 * SPI_TP_PROFILE 0 to compile it all out. */
#define SPI_TP_PROFILE      0
#define SPI_TP_PROFILE_N    128
#if SPI_TP_PROFILE
static uint32_t s_pf_m2s, s_pf_s2m, s_pf_xfer, s_pf_prev_m2s;
static uint32_t s_pf_n, s_pf_hs, s_pf_hop, s_pf_xf, s_pf_period;
static inline uint32_t pf_us(uint32_t cyc)
{
    return (uint32_t)(((uint64_t)cyc * 1000000u) / CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC);
}
#endif

/* Large SPI TX/RX buffers (SPI_XMIT_SIZE each) are placed in a dedicated noinit
 * PSRAM region when the board defines it (gtp record_pen), keeping them out of
 * the 82K DTCM. PSRAM is only initialised in app_system_lower_init() during
 * main(), so this section must NOT be zeroed at boot; the control fields are
 * reset explicitly in app_spi_master_init(). */
#if DT_NODE_EXISTS(DT_NODELABEL(psram0_spi_buf))
#define SPI_BUF_PSRAM __attribute__((__section__(LINKER_DT_NODE_REGION_NAME(DT_NODELABEL(psram0_spi_buf)))))
#else
#define SPI_BUF_PSRAM
#endif

/* The DMA endpoints (tx_dma_buf + the active rx slot) are placed in the on-chip
 * "SRAM" region (0x20020000, 32K) rather than PSRAM. PSRAM's access latency,
 * behind the shallow 32-byte SPI1 FIFO, stalled the SPI clock for ~68% of every
 * frame (profiler: xfer ~8ms vs the ~2.6ms a 16K frame needs at 50MHz); internal
 * SRAM keeps the DMA feeding TX / draining RX at full bus rate. The tx *ring*
 * storage stays in PSRAM - it is a producer/consumer buffer, not a DMA endpoint
 * (the trigger copies one frame out of it into tx_dma_buf). The 32K SRAM budget
 * is exactly tx_dma_buf(16K) + rx_storage[1](16K), so RX runs single-slot here
 * (SPI_RX_NSLOTS); growing it back to a 2-slot ping-pong would need a bigger
 * SRAM region, but heap_sram (DATA_ON) already peaks ~118K/128K so the 160K
 * sram block cannot spare it. */
#if DT_NODE_EXISTS(DT_NODELABEL(sram))
#define SPI_BUF_SRAM __attribute__((__section__(LINKER_DT_NODE_REGION_NAME(DT_NODELABEL(sram)))))
#else
#define SPI_BUF_SRAM SPI_BUF_PSRAM
#endif

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
static uint8_t tx_ring_storage[SPI_TX_RING_SIZE] __aligned(4) SPI_BUF_PSRAM;
/* DMA TX source - in fast on-chip SRAM (see SPI_BUF_SRAM above). */
static uint8_t tx_dma_buf[SPI_XMIT_SIZE] __aligned(32) SPI_BUF_SRAM;

/*RX Buffer: control in internal RAM, storage in fast on-chip SRAM.
 * SPI_RX_NSLOTS slots form a ping-pong pool; it is 1 here to fit the 32K SRAM
 * region alongside tx_dma_buf (see SPI_BUF_SRAM). The slot-selection and init
 * loops below are written against SPI_RX_NSLOTS so this can grow back to 2 if a
 * larger SRAM region ever becomes available. */
#define SPI_RX_NSLOTS 1
struct rx_buf_slot
{
    atomic_t in_use;    // 0: idle, 1: using
};
static struct rx_buf_slot rx_slots[SPI_RX_NSLOTS];
static uint8_t rx_storage[SPI_RX_NSLOTS][SPI_XMIT_SIZE] __aligned(32) SPI_BUF_SRAM;
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
static struct rx_msg     s_rx_msgs[SPI_RX_NSLOTS];

/* Zero-copy "prefilled" TX path (throughput blast). The producer builds the
 * frame directly into tx_dma_buf (SRAM) and sets s_prefill_len; the trigger then
 * clocks it out with NO ring pop and NO PSRAM copy (the old path copied every
 * frame PSRAM->PSRAM into the ring and then PSRAM->SRAM in the trigger). Only one
 * frame fits (a single tx_dma_buf), so s_tx_buf_free (a 1-count semaphore) gates
 * the producer: it is taken before the buffer is refilled and given back by
 * spi_done_cb once the frame has finished on the wire. s_prefill_len!=0 also marks
 * the in-flight frame as a prefill so spi_done_cb knows to release the buffer
 * instead of auto-continuing from the (empty) ring. */
static volatile uint16_t s_prefill_len;
static struct k_sem      s_tx_buf_free;

/*ASync API*/
static int m2s_gpio_set_level(SPI_GPIO_STATE state);
static void trigger_next_tx(void);

static inline void rx_buf_return(uint8_t index)
{
    if (index < SPI_RX_NSLOTS) { atomic_set(&rx_slots[index].in_use, 0); }
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
     * no-op). A prefilled blast frame lives in tx_dma_buf rather than the ring, so
     * s_prefill_len re-arms it too - otherwise a deferred prefill would never be
     * retried and the producer would block on s_tx_buf_free forever. */
    if ((s_prefill_len || ring_buf_size_get(&tx_ring) >= SPI_TX_RING_REC_HDR) &&
        atomic_cas(&master_idle, 1, 0))
    {
        trigger_next_tx();
    }
}

static void trigger_next_tx(void)
{
    /* Hold off DLPS while a transfer is in flight: take the per-transfer XFER
     * lock here and release it only when the pump goes idle (spi_done_cb with an
     * empty ring, or the transceive-error path).
     *
     * The lock is refcounted through app_spi_disable_dlps/_enable_dlps (see
     * app_spi_common.c), so during a TP blast the blast-session lock keeps the
     * hardware APP_DLPS_ENTER_CHECK_SPI bit held continuously and this per-frame
     * XFER churn never actually re-allows DLPS mid-stream. That closes the old
     * hole where parsing frame N's RX cleared a single-bit lock that frame N+1's
     * trigger had just set, letting a DLPS wake reconfigure the SPI1 master clock
     * under the in-flight DMA and lose the completion callback. */
    app_spi_disable_dlps(APP_SPI_XFER_BIT);
    APP_PRINT_TRACE1("trigger_next_tx: queued %u bytes", ring_buf_size_get(&tx_ring));
    m2s_gpio_set_level(SPI_GPIO_STATE_ACTIVE);
#if SPI_TP_PROFILE
    {
        uint32_t now = k_cycle_get_32();
        if (s_pf_prev_m2s) { s_pf_period += (now - s_pf_prev_m2s); }
        s_pf_prev_m2s = now;
        s_pf_m2s = now;
    }
#endif
}

static void spi_done_cb(const struct device *dev, int result, void *data)
{
    struct spi_cb_ctx *ctx = (struct spi_cb_ctx *)data;
    int rx_idx = ctx->rx_idx;

#if SPI_TP_PROFILE
    /* Fold this frame's phase timestamps into the running average. Guarded on a
     * counted blast and on all three stamps being set (so slave-initiated
     * downlinks, which never call trigger_next_tx, are skipped). */
    if (s_tx_pkt_total && s_pf_m2s && s_pf_s2m && s_pf_xfer)
    {
        uint32_t d = k_cycle_get_32();
        s_pf_hs  += (s_pf_s2m  - s_pf_m2s);
        s_pf_hop += (s_pf_xfer - s_pf_s2m);
        s_pf_xf  += (d         - s_pf_xfer);
        if (++s_pf_n >= SPI_TP_PROFILE_N)
        {
            APP_PRINT_INFO4("[tp-prof] avg us/frame: hs=%u hop=%u xfer=%u period=%u",
                            pf_us(s_pf_hs / s_pf_n), pf_us(s_pf_hop / s_pf_n),
                            pf_us(s_pf_xf / s_pf_n), pf_us(s_pf_period / s_pf_n));
            s_pf_n = 0; s_pf_hs = 0; s_pf_hop = 0; s_pf_xf = 0; s_pf_period = 0;
        }
        s_pf_m2s = 0; s_pf_s2m = 0; s_pf_xfer = 0;
    }
#endif

    if (rx_idx >= 0)
    {
        uint8_t *rxbuf = rx_storage[rx_idx];
        /* Only hand a frame to the parser task when it actually carries a valid
         * ['A']['T'] frame. During a bulk UPLOAD the master drives the bus and
         * the slave usually has nothing to send, so the full-duplex RX is an
         * empty/dummy frame (no 'AT' magic). Freeing the ping-pong slot right
         * here in the completion context - instead of paying a wifi_8711-task
         * round-trip to the parser - (1) removes one of the two per-frame task
         * hops, and (2) frees the RX slot immediately so the next transfer's
         * trigger no longer has to "defer: no rx buf", which was stretching the
         * inter-frame gap. Real frames (AT responses incl. the terminal "OK",
         * and downlink SKT data - both framed as [AT][len]...) still go through
         * the parser. */
        if (result == 0 && rxbuf[0] == 'A' && rxbuf[1] == 'T')
        {
            /* pooled rx_msg (one per ping-pong slot) - no malloc in this
             * callback context. The slot stays in_use until the parser returns
             * it, so this entry cannot be reused while still queued. */
            struct rx_msg *msg = &s_rx_msgs[rx_idx];
            msg->ptr = rxbuf;
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
            /* empty/dummy uplink RX (no magic) or a transfer error: free the
             * slot here, no task hop. */
            if (result != 0)
            {
                APP_PRINT_INFO1("spi async err %d", result);
            }
            rx_buf_return((uint8_t)rx_idx);
        }
    }
    m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);

    /* A prefilled (zero-copy blast) frame just finished on the wire: hand the
     * single tx_dma_buf back to the producer and go idle. The producer drives the
     * next frame (tx_wait_free -> fill -> send_prefilled), so we do NOT
     * auto-continue from the ring here. Release the per-transfer DLPS lock too
     * (paired with the disable in trigger_next_tx); the blast session still holds
     * APP_SPI_BLAST_BIT so DLPS stays off across frames. */
    if (s_prefill_len)
    {
        s_prefill_len = 0;
        atomic_set(&master_idle, 1);
        app_spi_enable_dlps(APP_SPI_XFER_BIT);
        k_sem_give(&s_tx_buf_free);
        return;
    }

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
        /* pump idle: release the per-transfer lock (paired with the disable in
         * trigger_next_tx). DLPS is only truly re-allowed if no blast session is
         * holding APP_SPI_BLAST_BIT. If the race-recheck below re-arms a
         * transfer, trigger_next_tx re-takes the lock, so the net state stays
         * correct. */
        app_spi_enable_dlps(APP_SPI_XFER_BIT);
        if (ring_buf_size_get(&tx_ring) >= SPI_TX_RING_REC_HDR &&
            atomic_cas(&master_idle, 1, 0))
        {
            trigger_next_tx();
        }
    }
}

uint8_t app_spi_master_send_raw_data_trigger(void)
{
    /* no per-frame log here (hot path) */

    /* 1. Decide the frame FIRST so we know the direction.
     *   - prefill (blast) or a queued ring frame  -> frame_len > 0 : the master
     *     is sending its OWN data (UPLINK). We clock it out FULL-DUPLEX (m2s
     *     asserts, slave acks via s2m, master clocks the frame out) exactly like
     *     the reference host's spi_master_write_read_stream_dma(). The slave has
     *     nothing meaningful to return on this transfer, so the MISO bytes are
     *     discarded (no RX slot reserved, rx_i stays -1). The slave signals any
     *     data of its own with a SEPARATE s2m edge (handled as a downlink below).
     *   - ring empty and no prefill -> frame_len == 0 : this trigger came from a
     *     slave-initiated s2m edge (DOWNLINK). We must RECEIVE, so run it
     *     full-duplex with a dummy sync-invalid TX frame.
     *
     * Popping the ring before securing an RX slot is safe: the downlink path
     * (which needs a slot) only runs when the ring is empty, so nothing is ever
     * popped-then-dropped. */
    uint16_t frame_len = 0;
    if (s_prefill_len)
    {
        /* Zero-copy blast: the producer already built this frame directly into
         * tx_dma_buf and gated itself on s_tx_buf_free, so there is nothing to pop
         * or copy - just clock it out. s_prefill_len is cleared in spi_done_cb
         * (which also returns the buffer to the producer), not here, so the
         * in-flight frame stays marked as a prefill for the completion path. */
        frame_len = s_prefill_len;
        if (frame_len > SPI_XMIT_SIZE) { frame_len = SPI_XMIT_SIZE; }
    }
    else if (ring_buf_size_get(&tx_ring) >= SPI_TX_RING_REC_HDR)
    {
        uint8_t lh[SPI_TX_RING_REC_HDR];
        ring_buf_get(&tx_ring, lh, SPI_TX_RING_REC_HDR);
        frame_len = (uint16_t)(lh[0] | (lh[1] << 8));
        if (frame_len > SPI_XMIT_SIZE) { frame_len = SPI_XMIT_SIZE; }
        ring_buf_get(&tx_ring, tx_dma_buf, frame_len);
    }

    const bool tx_only = (frame_len > 0);   /* uplink: full-duplex, RX discarded */

    int rx_i = -1;      /* -1 => spi_done_cb skips the RX-in path (no DATA_IN) */
    struct spi_buf rx;
    struct spi_buf_set rx_set;
    struct spi_buf_set *rx_set_ptr = NULL;

    if (!tx_only)
    {
        /* Downlink (slave-initiated): secure a free RX slot. The SPI driver
         * k_malloc()s a SPI_XMIT_SIZE bounce buffer whenever a full-duplex
         * rx_buf is NULL; under a downlink flood that allocation would run every
         * transfer and exhaust the heap ("spi rx dma malloc fail", -ENOMEM),
         * live-locking the path. So if no ping-pong slot is free, DEFER instead
         * of transferring: drop this opportunity (the slave re-asserts s2m;
         * rx_buf_return() re-kicks any queued TX once a slot frees). The bus
         * stays throttled by the m2s/s2m handshake, so dropping here just applies
         * back-pressure. (Uplink frames reserve no slot - they discard MISO into
         * rx_storage[0] - so they never hit this path or its RX-slot back-pressure.) */
        rx_i = atomic_get(&cur_rx_idx);
        for (int t = 0; t < SPI_RX_NSLOTS && atomic_get(&rx_slots[rx_i].in_use); t++)
        {
            rx_i = (rx_i + 1) % SPI_RX_NSLOTS;
        }
        if (atomic_get(&rx_slots[rx_i].in_use))
        {
            APP_PRINT_WARN0("app_spi_master_send_raw_data_trigger: no rx buf, defer");
            atomic_set(&master_idle, 1);
            m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
            return SPI_SEND_ERR_BUSY;
        }

        /* RX-only-ish (slave-initiated downlink): no TX payload. Only clobber the
         * magic byte so this dummy frame can't be mistaken for a valid [AT] frame
         * by the slave - the rest of tx_dma_buf is don't-care (the slave bounds
         * its read by the length field, padding never enters the CRC), hence no
         * full-buffer zero-pad. Matches the reference's MasterTxBuf. */
        tx_dma_buf[0] = SPI_CMD_SYNC_INVALID;
        APP_PRINT_TRACE0("Trigger with empty TX ring (downlink RX)");

        rx.buf = rx_storage[rx_i];
        rx.len = SPI_XMIT_SIZE;
        rx_set.buffers = &rx;
        rx_set.count = 1;
        rx_set_ptr = &rx_set;
    }
    else
    {
        /* UPLINK: run full-duplex like the reference host rather than TX-only.
         * The SPI driver k_malloc()s a SPI_XMIT_SIZE RX bounce buffer for every
         * transfer whose rx_buf is NULL; once the TCP server is up heap_sram has
         * < 16K free, so that per-frame alloc fails ("spi rx dma malloc fail",
         * -ENOMEM) and the uplink dies. Point RX at a persistent sink instead so
         * the driver takes the no-malloc full-duplex path. The MISO bytes are
         * thrown away: rx_i stays -1, so spi_done_cb posts no DATA_IN and no slot
         * is reserved. We reuse rx_storage[0] as the sink - only one transfer is
         * ever in flight (master_idle), and during an uplink blast the downlink
         * RX slot is idle, so nothing live is clobbered. (There is no spare 16K
         * for a dedicated buffer: the SRAM region is exactly tx_dma_buf(16K) +
         * rx_storage(16K); see SPI_BUF_SRAM.) */
        rx.buf = rx_storage[0];
        rx.len = SPI_XMIT_SIZE;
        rx_set.buffers = &rx;
        rx_set.count = 1;
        rx_set_ptr = &rx_set;

        if (s_tx_pkt_total)
        {
            /* a real uplink frame during a counted blast. Log "cur/total" only
             * every 64th frame (plus the final one): a per-frame INFO here writes
             * the trace buffer on every SPI transaction and measurably eats blast
             * throughput. */
            if (s_tx_pkt_idx < s_tx_pkt_total) { s_tx_pkt_idx++; }
            if ((s_tx_pkt_idx & 0x3F) == 0 || s_tx_pkt_idx == s_tx_pkt_total)
            {
                APP_PRINT_INFO2("app_spi_master_send_raw_data_trigger %u/%u",
                                s_tx_pkt_idx, s_tx_pkt_total);
            }
        }
    }

    struct spi_buf tx = { .buf = tx_dma_buf, .len = SPI_XMIT_SIZE };
    struct spi_buf_set tx_set = { .buffers = &tx, .count = 1 };

    /* single static context - one transfer is ever in flight (master_idle).
     * rx_idx == -1 on the uplink path makes spi_done_cb skip the RX-in handling
     * (the MISO bytes clocked into the discard sink are dropped). */
    s_ctx.rx_idx = rx_i;

    /* Hold off DLPS for the in-flight transfer on THIS path too. A slave-initiated
     * downlink is kicked straight here from the s2m ISR (IO_SPI_MASTER_TRIGGER),
     * never through trigger_next_tx, so the DLPS lock that the uplink path takes
     * in trigger_next_tx was never asserted during a pure downlink flood. DLPS was
     * therefore free to enter mid-flood; a DLPS wake reconfigures the SPI1 master
     * clock (spi_rtl87x3g PM RESUME -> configure) under the in-flight RX DMA, the
     * completion callback is lost and the RX pump stalls permanently (observed:
     * trigger with no matching spi_done_cb, coincident with "spi src freq"). Take
     * the lock right before submitting; spi_done_cb releases it once the pump goes
     * idle (or the error path below). app_dlps_disable is an idempotent bit-clear,
     * so double-disabling with the uplink trigger_next_tx path is harmless. */
    app_spi_disable_dlps(APP_SPI_XFER_BIT);

#if SPI_TP_PROFILE
    s_pf_xfer = k_cycle_get_32();
#endif
    /* rx_set_ptr is always non-NULL now (uplink points it at a discard sink,
     * downlink at a real RX slot), so the driver always runs full-duplex with a
     * caller-supplied RX buffer and never k_malloc()s a bounce buffer. */
    int ret = spi_transceive_cb(spi_spec.bus, &spi_spec.config, &tx_set, rx_set_ptr,
                                spi_done_cb, &s_ctx);
    if (ret)
    {
        APP_PRINT_ERROR1("app_spi_master_send_raw_data_trigger: ret %d", ret);
        atomic_set(&master_idle, 1);
        /* transceive failed: no completion callback will fire, so release the
         * per-transfer DLPS lock here (paired with trigger_next_tx) or the chip
         * could never sleep again. */
        app_spi_enable_dlps(APP_SPI_XFER_BIT);
        m2s_gpio_set_level(SPI_GPIO_STATE_INACTIVE);
        /* If this was a prefilled frame, spi_done_cb will never run to return the
         * buffer, so release it here (and drop the prefill mark) - otherwise the
         * producer's next tx_wait_free() would block forever. Unlike the "no rx
         * buf, defer" early-out above (which keeps s_prefill_len so the re-kick
         * retries), this is a hard error: the frame is dropped, not retried. */
        if (s_prefill_len)
        {
            s_prefill_len = 0;
            k_sem_give(&s_tx_buf_free);
        }
        return SPI_SEND_ERR_BUSY;
    }

    /* Only the downlink path took an RX slot; mark it in-use so the parser owns it
     * until it returns it. TX-only frames never touched the pool. */
    if (!tx_only)
    {
        atomic_set(&rx_slots[rx_i].in_use, 1);
        atomic_set(&cur_rx_idx, (rx_i + 1) % SPI_RX_NSLOTS);
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
        /* Only trigger when m2s is already active (HIGH->LOW happened first) */
        //  if (gpio_pin_get_dt(&m2s_gpio))
        {
#if SPI_TP_PROFILE
            s_pf_s2m = k_cycle_get_32();
#endif
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
        // (no per-frame log here - hot path)
    }

    return SPI_SEND_SUC;
}

/* ---- Zero-copy prefilled TX path (throughput blast) ----------------------- */

/* The single SRAM DMA buffer the producer fills in place. */
uint8_t *app_spi_master_tx_dma_buf(void)
{
    return tx_dma_buf;
}

/* Block until the previous prefilled frame has finished on the wire, i.e. until
 * tx_dma_buf is free to overwrite. The token is given back by spi_done_cb (or by
 * app_spi_master_send_prefilled on a length error). Must be paired 1:1 with a
 * send_prefilled() (or a send_prefilled(0)/error return) so the token is always
 * returned - otherwise the next wait blocks forever. */
void app_spi_master_tx_wait_free(void)
{
    k_sem_take(&s_tx_buf_free, K_FOREVER);
}

/* Hand the already-filled tx_dma_buf (len payload bytes) off to the SPI pump.
 * The caller must have taken the buffer via app_spi_master_tx_wait_free() first.
 * On a length error the buffer token is returned immediately (so the producer's
 * pairing stays balanced) and no transfer is started. */
uint8_t app_spi_master_send_prefilled(uint16_t len)
{
    if (len == 0 || len > SPI_XMIT_SIZE)
    {
        k_sem_give(&s_tx_buf_free);
        return SPI_SEND_ERR_LEN;
    }

    s_prefill_len = len;

    if (atomic_cas(&master_idle, 1, 0))
    {
        /* bus idle: start the handshake now (trigger clocks tx_dma_buf as-is) */
        trigger_next_tx();
    }
    /* else a transfer is in flight; the rx-parser re-kick / spi_done_cb honours
     * s_prefill_len and will drive this frame when the bus next goes idle. */

    return SPI_SEND_SUC;
}

/* Abort helper: return the buffer token without sending (producer hit EOF/error
 * after already taking the buffer). Safe no-op on flow control - just balances a
 * prior tx_wait_free(). */
void app_spi_master_tx_release(void)
{
    k_sem_give(&s_tx_buf_free);
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
    for (int i = 0; i < SPI_RX_NSLOTS; i++)
    {
        atomic_set(&rx_slots[i].in_use, 0);
    }
    atomic_set(&cur_rx_idx, 0);
    atomic_set(&master_idle, 1);
    /* Prefilled-TX flow control: buffer starts free (count 1, max 1). */
    k_sem_init(&s_tx_buf_free, 1, 1);
    s_prefill_len = 0;
    APP_PRINT_INFO0("app_spi_master_init: OK");
}
