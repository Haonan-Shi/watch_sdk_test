/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef FF5E4277_A378_4FD2_A631_F2D595F5AA47
#define FF5E4277_A378_4FD2_A631_F2D595F5AA47


#include "os_mem.h"
#include "section.h"
#include "mjpeg_parser.h"


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


int jpeg_parser_init(void);
int mjpeg_parser(const char *data, size_t len, mjpeg_frame_callback_t callback);
int on_jpeg_frame(int num,
                  char *header_start, size_t header_size,  // Reserved area
                  const char *jpeg, size_t jpeg_len,       // JPEG data
                  void *ctx);
void jpeg_parser_deinit(void);

#endif /* FF5E4277_A378_4FD2_A631_F2D595F5AA47 */
