/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef D8C1B25B_E52A_44FC_8E65_9B7C02A9A480
#define D8C1B25B_E52A_44FC_8E65_9B7C02A9A480


#include "os_mem.h"
#include "section.h"
#include "minimal_build_zephyr.h"

typedef struct sdiod_rx_desc_s
{
    // DW 0
    uint32_t pkt_len : 16; /*!< bit[15:0], the packet size */
    uint32_t offset : 8;   /*!< bit[23:16], the offset from the packet start to the payload start,
                                normally it's the size of a RX Desc */
    uint32_t rsvd0 : 6;    /*!< bit[29:24], reserved */
    uint32_t icv : 1;      // bit[30], ICV error
    uint32_t crc : 1;      // bit[31], CRC error

    // DW 1
    uint32_t type : 8;   // bit[7:0], the type of this packet
    uint32_t rsvd1 : 24; // bit[31:8]

    // DW 2
    uint32_t rsvd2;
    uint32_t rsvd3;
    uint32_t rsvd4;
    uint32_t rsvd5;
} sdiod_rx_desc_t, *psdiod_rx_desc_t;


void rtsp_mjpeg_parser_init(void);
void rtsp_mjpeg_parser_deinit(void);

void wifi_power_on_proc(void *msg);
void wifi_camera_enter_proc(void *msg);
void wifi_camera_exit_proc(void *msg);


#endif /* D8C1B25B_E52A_44FC_8E65_9B7C02A9A480 */
