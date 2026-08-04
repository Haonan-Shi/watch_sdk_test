/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* MJPEG Parser Module
 *
 * Parses multipart/x-mixed-replace MJPEG streams
 * Handles TCP packet fragmentation and boundary detection
 */

#ifndef C81BB5FD_0616_4F92_9A40_451817B8FB17
#define C81BB5FD_0616_4F92_9A40_451817B8FB17

#ifndef MJPEG_PARSER_H
#define MJPEG_PARSER_H

#include "mjpeg_buffer.h"

/* Parser state structure */
typedef struct
{
    char boundary[64];           /* Boundary string from Content-Type */
    char full_boundary[68];      /* \r\n--boundary format */
    unsigned int full_boundary_len;
    mjpeg_buffer_t buffer;       /* Data accumulation buffer */
    int frame_count;             /* Number of frames processed */
    int initialized;             /* Initialization flag */
} mjpeg_parser_t;

/* Frame callback function type
 *
 * Called when a complete JPEG frame is detected
 * Parameters:
 *   frame_num  - Frame sequence number (1-based)
 *   jpeg_data  - Pointer to JPEG data (0xFF 0xD8 ... 0xFF 0xD9)
 *   jpeg_len   - Length of JPEG data in bytes
 *   user_data  - User-provided context pointer
 *
 * Returns: 0 to continue, non-zero to stop parsing
 */
typedef int (*mjpeg_frame_callback_t)(
    int frame_num,
    const char *jpeg_data,
    size_t jpeg_len,
    void *user_data
);

/* Frame callback with header support
 *
 * Called when a complete JPEG frame is detected
 * Parameters:
 *   frame_num     - Frame sequence number (1-based)
 *   header_start  - Pointer to reserved header area (writable)
 *   header_size   - Size of header area in bytes
 *   jpeg_data     - Pointer to JPEG data (0xFF 0xD8 ... 0xFF 0xD9)
 *   jpeg_len      - Length of JPEG data in bytes
 *   user_data     - User-provided context pointer
 *
 * Returns: 0 to continue, non-zero to stop parsing
 *
 * Note: header_start and jpeg_data are in contiguous memory:
 *       [header_start ... header_start+header_size-1][jpeg_data ... jpeg_data+jpeg_len-1]
 *       You can write custom header and send the whole block as one packet (zero-copy)
 */
typedef int (*mjpeg_frame_callback_with_header_t)(
    int frame_num,
    char *header_start,
    size_t header_size,
    const char *jpeg_data,
    size_t jpeg_len,
    void *user_data
);

/* Initialize parser with boundary string
 *
 * Parameters:
 *   parser   - Parser instance
 *   boundary - Boundary string from Content-Type (without -- prefix)
 *
 * Returns: 0 on success, -1 on error
 */
int mjpeg_parser_init(mjpeg_parser_t *parser, const char *boundary);

/* Initialize parser with boundary and header reserve space
 *
 * Parameters:
 *   parser         - Parser instance
 *   boundary       - Boundary string from Content-Type (without -- prefix)
 *   header_reserve - Bytes to reserve before JPEG data for custom header
 *
 * Returns: 0 on success, -1 on error
 *
 * Example: mjpeg_parser_init_with_header(&parser, "boundary", 16);
 *          // Reserves 16 bytes before each JPEG frame for custom header
 */
int mjpeg_parser_init_with_header(
    mjpeg_parser_t *parser,
    const char *boundary,
    size_t header_reserve
);

/* Free parser resources */
void mjpeg_parser_free(mjpeg_parser_t *parser);

/* Feed data to parser
 *
 * This function can be called multiple times with chunks of data
 * from recv() calls. It handles TCP packet fragmentation automatically.
 *
 * Parameters:
 *   parser    - Parser instance
 *   data      - Data chunk from network
 *   len       - Length of data chunk
 *   callback  - Function to call for each complete frame (can be NULL)
 *   user_data - Context pointer passed to callback
 *
 * Returns: 0 on success, -1 on error
 */
int mjpeg_parser_feed(
    mjpeg_parser_t *parser,
    const char *data,
    size_t len,
    mjpeg_frame_callback_t callback,
    void *user_data
);

/* Reset parser state (keep boundary, clear buffer) */
void mjpeg_parser_reset(mjpeg_parser_t *parser);

/* Get current frame count */
int mjpeg_parser_get_frame_count(mjpeg_parser_t *parser);

#endif /* MJPEG_PARSER_H */


#endif /* C81BB5FD_0616_4F92_9A40_451817B8FB17 */
