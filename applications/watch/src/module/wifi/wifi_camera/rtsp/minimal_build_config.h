/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef ECA4EA27_3B77_45DD_9C89_2F0FE59D47E6
#define ECA4EA27_3B77_45DD_9C89_2F0FE59D47E6

#ifndef MINIMAL_BUILD_CONFIG_H
#define MINIMAL_BUILD_CONFIG_H

/* ========================================================================
 * MEMORY CONFIGURATION
 * ======================================================================== */

/**
 * @brief Maximum size of a single RTP packet buffer (bytes)
 *
 * Default: 65536 (64KB) - Standard live555 value
 * Recommended: 2524 (1500 + 1024) - For 466x466 resolution
 *              1500 bytes: Maximum RTP packet payload (MTU limit)
 *              1024 bytes: JPEG header reservation space
 *
 * Impact: Each buffered packet consumes this amount of memory
 * Memory per packet = MAX_PACKET_SIZE + ~80 bytes (object overhead)
 */
#ifndef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 2524
#endif

/**
 * @brief Maximum number of packets to buffer in reordering queue
 *
 * Default: Unlimited (in original live555)
 * Recommended values:
 *   - 5:  For good network (sequential packets)
 *   - 10: For typical network (minor reordering)
 *   - 15: For 466x466 resolution (recommended)
 *   - 20: For poor network (severe packet loss/reordering)
 *
 * Impact: Total packet buffer memory = MAX_PACKET_SIZE * MAX_BUFFERED_PACKETS
 * Example: 2524B * 15 = 37KB buffer pool
 *
 * When limit reached:
 *   - PACKET_DROP_POLICY_OLDEST: Drop oldest packet to make room
 *   - PACKET_DROP_POLICY_NEWEST: Reject new packet (drop incoming)
 */
#ifndef MAX_BUFFERED_PACKETS
#define MAX_BUFFERED_PACKETS 15
#endif

/**
 * @brief Default maximum frame buffer size for mb_init()
 *
 * This is the default value if user passes 0 to mb_init()
 *
 * Recommended values by resolution:
 *   - 320x240:   8192   (8KB)
 *   - 466x466:   65536  (64KB) - Current configuration
 *   - 640x480:   32768  (32KB)
 *   - 1280x720:  65536  (64KB)
 *   - 1920x1080: 131072 (128KB)
 *
 * NOTE: On Zephyr/embedded platforms, OptimizedCallbackSink uses a static
 * 64KB buffer to avoid dynamic allocation and memory fragmentation.
 */
#ifndef DEFAULT_MAX_FRAME_SIZE
#define DEFAULT_MAX_FRAME_SIZE 65536
#endif

/**
 * @brief Size of sink receive buffer (bytes)
 *
 * This buffer accumulates RTP payload data before delivering complete frame
 * Should be at least as large as expected frame size
 *
 * NOTE: On Zephyr/embedded platforms, uses static 64KB buffer internally
 */
#ifndef SINK_RECEIVE_BUFFER_SIZE
#define SINK_RECEIVE_BUFFER_SIZE 65536
#endif


/* ========================================================================
 * PACKET DROP POLICY
 * ======================================================================== */

/**
 * @brief Policy when packet buffer is full
 *
 * Options:
 *   0 = Drop oldest packet (make room for new packet)
 *   1 = Drop newest packet (reject incoming packet)
 *
 * Recommended: 0 (drop oldest) - Better for real-time streaming
 */
#ifndef PACKET_DROP_POLICY
#define PACKET_DROP_POLICY 0
#endif

#define PACKET_DROP_POLICY_OLDEST 0
#define PACKET_DROP_POLICY_NEWEST 1


/* ========================================================================
 * TIMEOUT CONFIGURATION
 * ======================================================================== */

/**
 * @brief Maximum age of buffered packet before forced delivery (milliseconds)
 *
 * If a packet sits in the reordering buffer for longer than this time,
 * force delivery even if earlier packets are missing (treat as lost)
 *
 * Default: 1000 (1 second)
 * Recommended: 500-2000 ms depending on network latency tolerance
 *
 * Set to 0 to disable timeout-based delivery
 */
#ifndef PACKET_TIMEOUT_MS
#define PACKET_TIMEOUT_MS 1000
#endif

/**
 * @brief Reordering buffer threshold time (microseconds)
 *
 * Time to wait for out-of-order packets before assuming packet loss
 * Used by live555's internal reordering logic
 *
 * Default: 100000 (100ms)
 */
#ifndef REORDERING_THRESHOLD_TIME_US
#define REORDERING_THRESHOLD_TIME_US 100000
#endif


/* ========================================================================
 * JPEG CONFIGURATION
 * ======================================================================== */

/**
 * @brief Reserved space for synthesized JPEG header (bytes)
 *
 * JPEGBufferedPacket reserves space at buffer start for JPEG headers
 * (SOI, APP0, DQT, SOF0, DHT, SOS markers)
 *
 * Default: 1024 bytes
 * Do not change unless you understand JPEG/RTP RFC2435 header structure
 */
#ifndef MAX_JPEG_HEADER_SIZE
#define MAX_JPEG_HEADER_SIZE 1024
#endif

/**
 * @brief Default JPEG dimensions if not specified in RTP header
 *
 * Width and Height must be multiples of 8 (JPEG block size)
 * Set to 0 to require dimensions in RTP header (reject if missing)
 */
#ifndef DEFAULT_JPEG_WIDTH
#define DEFAULT_JPEG_WIDTH 0
#endif

#ifndef DEFAULT_JPEG_HEIGHT
#define DEFAULT_JPEG_HEIGHT 0
#endif


/* ========================================================================
 * DEBUG CONFIGURATION
 * ======================================================================== */

/**
 * @brief Enable debug logging
 *
 * 0 = Disabled (production)
 * 1 = Basic info (packet counts, frame sizes)
 * 2 = Detailed (packet headers, buffer states)
 * 3 = Verbose (all function calls)
 */
#ifndef MB_DEBUG_LEVEL
#define MB_DEBUG_LEVEL 0
#endif

/**
 * @brief Enable memory usage statistics
 *
 * Track peak memory usage, allocation counts, etc.
 * Slight performance overhead when enabled
 */
#ifndef MB_ENABLE_MEMORY_STATS
#define MB_ENABLE_MEMORY_STATS 0
#endif


/* ========================================================================
 * PLATFORM-SPECIFIC CONFIGURATION
 * ======================================================================== */

/**
 * @brief Enable Zephyr RTOS optimizations
 *
 * Automatically defined if compiling for Zephyr
 */
#ifdef __ZEPHYR__
#ifndef MB_PLATFORM_ZEPHYR
#define MB_PLATFORM_ZEPHYR 1
#endif
#endif

/**
 * @brief Stack size for event loop thread (Zephyr only)
 *
 * Only relevant if using threaded event loop on RTOS
 * INCREASED from 4096 to 8192 to prevent stack overflow with 16KB+ frames
 */
#ifdef MB_PLATFORM_ZEPHYR
#ifndef MB_THREAD_STACK_SIZE
#define MB_THREAD_STACK_SIZE 8192
#endif
#endif


/* ========================================================================
 * VALIDATION
 * ======================================================================== */

/* Validate MAX_PACKET_SIZE */
#if MAX_PACKET_SIZE < 2048
#error "MAX_PACKET_SIZE must be at least 2048 bytes (1500 MTU + 1024 JPEG header reserve)"
#endif

#if MAX_PACKET_SIZE > 131072
#warning "MAX_PACKET_SIZE > 128KB may cause excessive memory usage"
#endif

/* Validate MAX_BUFFERED_PACKETS */
#if MAX_BUFFERED_PACKETS < 2
#error "MAX_BUFFERED_PACKETS must be at least 2 (for frame assembly)"
#endif

#if MAX_BUFFERED_PACKETS > 50
#warning "MAX_BUFFERED_PACKETS > 50 may cause excessive memory usage"
#endif

/* Validate PACKET_DROP_POLICY */
#if PACKET_DROP_POLICY != 0 && PACKET_DROP_POLICY != 1
#error "PACKET_DROP_POLICY must be 0 (oldest) or 1 (newest)"
#endif

/* Validate JPEG header size */
#if MAX_JPEG_HEADER_SIZE > MAX_PACKET_SIZE / 2
#error "MAX_JPEG_HEADER_SIZE must be less than MAX_PACKET_SIZE / 2"
#endif


/* ========================================================================
 * MEMORY ESTIMATION
 * ======================================================================== */

/**
 * Estimated memory usage (for reference):
 *
 * Static memory (code + data):       ~87 KB
 * Framework objects:                 ~2 KB
 * Packet buffer pool:                MAX_PACKET_SIZE * MAX_BUFFERED_PACKETS
 * Output frame buffer:               DEFAULT_MAX_FRAME_SIZE (peak)
 * Sink receive buffer:               SINK_RECEIVE_BUFFER_SIZE
 * Stack:                             ~3 KB
 *
 * Total RAM estimate:
 *   Minimum: 87 + 2 + (MAX_PACKET_SIZE * 2) + 8 + SINK_RECEIVE_BUFFER_SIZE + 3
 *   Maximum: 87 + 2 + (MAX_PACKET_SIZE * MAX_BUFFERED_PACKETS) +
 *            DEFAULT_MAX_FRAME_SIZE + SINK_RECEIVE_BUFFER_SIZE + 3
 *
 * Example configurations:
 *
 * [Config 1: Minimal - 320x240]
 *   MAX_PACKET_SIZE=4096, MAX_BUFFERED_PACKETS=5, DEFAULT_MAX_FRAME_SIZE=8192
 *   RAM: 87 + 2 + 20 + 8 + 32 + 3 = ~152 KB
 *
 * [Config 2: Balanced - 640x480]
 *   MAX_PACKET_SIZE=4096, MAX_BUFFERED_PACKETS=10, DEFAULT_MAX_FRAME_SIZE=32768
 *   RAM: 87 + 2 + 40 + 32 + 32 + 3 = ~196 KB
 *
 * [Config 3: High quality - 1280x720]
 *   MAX_PACKET_SIZE=4096, MAX_BUFFERED_PACKETS=15, DEFAULT_MAX_FRAME_SIZE=65536
 *   RAM: 87 + 2 + 60 + 64 + 32 + 3 = ~248 KB
 */


/* ========================================================================
 * PRESET CONFIGURATIONS
 * ======================================================================== */

/* Uncomment ONE of the following presets, or define custom values above */

/* Preset 0: Custom 466x466 (recommended for your use case) */
#ifdef MB_PRESET_466x466
#undef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 2524          // 1500 + 1024
#undef MAX_BUFFERED_PACKETS
#define MAX_BUFFERED_PACKETS 15       // Enough for most cases
#undef DEFAULT_MAX_FRAME_SIZE
#define DEFAULT_MAX_FRAME_SIZE 65536  // 64KB for 466x466
#undef SINK_RECEIVE_BUFFER_SIZE
#define SINK_RECEIVE_BUFFER_SIZE 65536
#endif

/* Preset 1: Minimal Memory (320x240, good network) */
#ifdef MB_PRESET_MINIMAL
#undef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 4096
#undef MAX_BUFFERED_PACKETS
#define MAX_BUFFERED_PACKETS 5
#undef DEFAULT_MAX_FRAME_SIZE
#define DEFAULT_MAX_FRAME_SIZE 8192
#undef SINK_RECEIVE_BUFFER_SIZE
#define SINK_RECEIVE_BUFFER_SIZE 16384
#endif

/* Preset 2: Balanced (640x480, typical network) */
#ifdef MB_PRESET_BALANCED
#undef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 4096
#undef MAX_BUFFERED_PACKETS
#define MAX_BUFFERED_PACKETS 10
#undef DEFAULT_MAX_FRAME_SIZE
#define DEFAULT_MAX_FRAME_SIZE 32768
#undef SINK_RECEIVE_BUFFER_SIZE
#define SINK_RECEIVE_BUFFER_SIZE 32768
#endif

/* Preset 3: High Quality (1280x720, poor network) */
#ifdef MB_PRESET_HIGH_QUALITY
#undef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 4096
#undef MAX_BUFFERED_PACKETS
#define MAX_BUFFERED_PACKETS 20
#undef DEFAULT_MAX_FRAME_SIZE
#define DEFAULT_MAX_FRAME_SIZE 65536
#undef SINK_RECEIVE_BUFFER_SIZE
#define SINK_RECEIVE_BUFFER_SIZE 65536
#endif

/* Preset 4: Maximum (1920x1080, very poor network) */
#ifdef MB_PRESET_MAXIMUM
#undef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 4096
#undef MAX_BUFFERED_PACKETS
#define MAX_BUFFERED_PACKETS 30
#undef DEFAULT_MAX_FRAME_SIZE
#define DEFAULT_MAX_FRAME_SIZE 131072
#undef SINK_RECEIVE_BUFFER_SIZE
#define SINK_RECEIVE_BUFFER_SIZE 131072
#endif

/* Preset 5: Legacy (original live555 defaults) */
#ifdef MB_PRESET_LEGACY
#undef MAX_PACKET_SIZE
#define MAX_PACKET_SIZE 65536
#undef MAX_BUFFERED_PACKETS
#define MAX_BUFFERED_PACKETS 50  /* Effectively unlimited in original */
#undef DEFAULT_MAX_FRAME_SIZE
#define DEFAULT_MAX_FRAME_SIZE 65536
#undef SINK_RECEIVE_BUFFER_SIZE
#define SINK_RECEIVE_BUFFER_SIZE 32768
#endif


#endif /* MINIMAL_BUILD_CONFIG_H */


#endif /* ECA4EA27_3B77_45DD_9C89_2F0FE59D47E6 */
