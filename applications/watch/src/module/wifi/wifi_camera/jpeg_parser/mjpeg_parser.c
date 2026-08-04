/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* MJPEG Parser Implementation */

#include "mjpeg_parser.h"
#include <string.h>
#include <stdio.h>
#include "trace.h"

/* Debug output macro - uses custom MJPEG_LOG if available */
#ifndef MJPEG_DEBUG
#define MJPEG_DEBUG 0
#endif

#if MJPEG_DEBUG
#define DEBUG_PRINT(fmt, ...) MJPEG_LOG("DEBUG", fmt, ##__VA_ARGS__)
#else
#define DEBUG_PRINT(fmt, ...)  //DBG_DIRECT(fmt, ##__VA_ARGS__)//do {} while(0)
#endif

int mjpeg_parser_init(mjpeg_parser_t *parser, const char *boundary)
{
    if (!parser || !boundary)
    {
        return -1;
    }

    memset(parser, 0, sizeof(mjpeg_parser_t));

    /* Copy boundary string */
    strncpy(parser->boundary, boundary, sizeof(parser->boundary) - 1);
    parser->boundary[sizeof(parser->boundary) - 1] = '\0';

    /* Build full boundary: \r\n--boundary */
    snprintf(parser->full_boundary, sizeof(parser->full_boundary),
             "\r\n--%s", boundary);

    parser->full_boundary_len = strlen(parser->full_boundary);


    /* Initialize buffer */
    mjpeg_buffer_init(&parser->buffer);

    parser->frame_count = 0;
    parser->initialized = 1;

    DEBUG_PRINT("Parser initialized, boundary: %s", boundary);
    DEBUG_PRINT("Full boundary: \\r\\n--%s (len=%d)", boundary,
                (int)strlen(parser->full_boundary));

    return 0;
}

int mjpeg_parser_init_with_header(mjpeg_parser_t *parser,
                                  const char *boundary,
                                  size_t header_reserve)
{
    if (!parser || !boundary)
    {
        return -1;
    }

    memset(parser, 0, sizeof(mjpeg_parser_t));

    /* Copy boundary string */
    strncpy(parser->boundary, boundary, sizeof(parser->boundary) - 1);
    parser->boundary[sizeof(parser->boundary) - 1] = '\0';

    /* Build full boundary: \r\n--boundary */
    snprintf(parser->full_boundary, sizeof(parser->full_boundary),
             "\r\n--%s", boundary);
    parser->full_boundary_len = strlen(parser->full_boundary);

    /* Initialize buffer WITH header reserve */
    mjpeg_buffer_init_with_header(&parser->buffer, header_reserve);

    parser->frame_count = 0;
    parser->initialized = 1;

    DEBUG_PRINT("Parser initialized with %u byte header reserve", (unsigned)header_reserve);
    DEBUG_PRINT("Boundary: %s", boundary);

    return 0;
}

void mjpeg_parser_free(mjpeg_parser_t *parser)
{
    if (!parser)
    {
        return;
    }

    mjpeg_buffer_free(&parser->buffer);
    parser->initialized = 0;

    DEBUG_PRINT("Parser freed, processed %d frames", parser->frame_count);
}

void mjpeg_parser_reset(mjpeg_parser_t *parser)
{
    if (!parser)
    {
        return;
    }

    parser->buffer.len = 0;
    parser->frame_count = 0;

    DEBUG_PRINT("Parser reset");
}

int mjpeg_parser_get_frame_count(mjpeg_parser_t *parser)
{
    return parser ? parser->frame_count : 0;
}

int mjpeg_parser_feed(
    mjpeg_parser_t *parser,
    const char *data,
    size_t len,
    mjpeg_frame_callback_t callback,
    void *user_data)
{
    size_t boundary_len;

    //DBG_DIRECT("mjpeg_parser_feed(%p, %p, %u, %p, %p)", parser, data, (unsigned)len, callback, user_data);

    if (!parser || !parser->initialized || !data || len == 0)
    {
        DEBUG_PRINT("mjpeg_parser_feed failed");
        return -1;
    }


    // boundary_len = strlen(parser->full_boundary);
    boundary_len = parser->full_boundary_len;

    /* Append data to accumulation buffer */
    if (mjpeg_buffer_append(&parser->buffer, data, len) < 0)
    {
        DEBUG_PRINT("Buffer append failed (frame too large), buffer cleared and waiting for next frame");
        /* Note: buffer is already cleared by mjpeg_buffer_append() */
        /* Continue to receive next frame data */
        return 0; /* Not a fatal error, just skip this oversized frame */
    }

    DEBUG_PRINT("Fed %u bytes, buffer now %u bytes",
                (unsigned)len, (unsigned)parser->buffer.len);

    /* Process all complete frames in buffer */
    while (parser->buffer.len > boundary_len)
    {
        const char *headers_start;
        const char *body_start;
        const char *next_boundary;
        const char *data_start;
        size_t search_start;
        size_t body_len;
        size_t write_len;
        size_t i;
        mjpeg_buffer_t tmp;

        /* Get data area start (skip header reserve) */
        data_start = mjpeg_buffer_data_start(&parser->buffer);

        /* Skip current boundary line (e.g., "--123456\r\n") */
        headers_start = strstr(data_start, "\r\n");
        if (!headers_start)
        {
            DEBUG_PRINT("Waiting for boundary end");
            break; /* Need more data */
        }
        headers_start += 2;

        /* Find headers end (\r\n\r\n) */
        body_start = strstr(headers_start, "\r\n\r\n");
        if (!body_start)
        {
            DEBUG_PRINT("Waiting for headers end");
            break; /* Need more data */
        }
        body_start += 4;

        if (body_start >= data_start + parser->buffer.len)
        {
            DEBUG_PRINT("Body start beyond buffer");
            break;
        }

        /* Find next boundary */
        search_start = body_start - data_start;
        next_boundary = NULL;

        if (search_start < parser->buffer.len)
        {
            tmp.data = (char *)data_start + search_start;
            tmp.len = parser->buffer.len - search_start;
            tmp.cap = 0;
            tmp.header_reserve = 0;

            next_boundary = mjpeg_buffer_find(&tmp, parser->full_boundary, boundary_len);
            if (next_boundary)
            {
                next_boundary = data_start + search_start + (next_boundary - tmp.data);
            }
        }

        if (!next_boundary)
        {
            DEBUG_PRINT("Waiting for next boundary");
            break; /* Need more data */
        }

        /* Calculate body length */
        body_len = next_boundary - body_start;
        DEBUG_PRINT("Found frame: body_len=%u", (unsigned)body_len);

        /* Validate JPEG SOI (Start of Image: 0xFF 0xD8) */
        if (body_len >= 2 &&
            (unsigned char)body_start[0] == 0xFF &&
            (unsigned char)body_start[1] == 0xD8)
        {

            /* Find JPEG EOI (End of Image: 0xFF 0xD9) */
            write_len = body_len;
            for (i = 0; i < body_len - 1; i++)
            {
                if ((unsigned char)body_start[i] == 0xFF &&
                    (unsigned char)body_start[i + 1] == 0xD9)
                {
                    write_len = i + 2;
                    DEBUG_PRINT("Found JPEG EOI at offset %u", (unsigned)i);
                    break;
                }
            }

            /* Increment frame counter */
            parser->frame_count++;

            DEBUG_PRINT("Frame %d: %u bytes", parser->frame_count, (unsigned)write_len);

            /* Call user callback */
            if (callback)
            {
                int ret;

                /* If header reserve is enabled, use extended callback signature */
                if (parser->buffer.header_reserve > 0)
                {
                    /* Calculate header start pointer (before JPEG data) */
                    char *header_start = (char *)body_start - parser->buffer.header_reserve;

                    /* Cast to extended callback and call */
                    mjpeg_frame_callback_with_header_t cb_with_header =
                        (mjpeg_frame_callback_with_header_t)callback;

                    ret = cb_with_header(parser->frame_count,
                                         header_start,
                                         parser->buffer.header_reserve,
                                         body_start,
                                         write_len,
                                         user_data);
                }
                else
                {
                    /* Use standard callback */
                    ret = callback(parser->frame_count, body_start, write_len, user_data);
                }

                if (ret != 0)
                {
                    DEBUG_PRINT("Callback returned %d, stopping", ret);
                    return ret;
                }
            }
        }
        else
        {
            DEBUG_PRINT("Skipping non-JPEG part (first bytes: %02X %02X)",
                        body_len > 0 ? (unsigned char)body_start[0] : 0,
                        body_len > 1 ? (unsigned char)body_start[1] : 0);
        }

        /* Consume processed data (up to next boundary) */
        mjpeg_buffer_consume(&parser->buffer, next_boundary - data_start);
        DEBUG_PRINT("Consumed data, buffer now %u bytes", (unsigned)parser->buffer.len);
    }

    return 0;
}
