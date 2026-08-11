/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* MJPEG Buffer Management Implementation */

#include "mjpeg_buffer.h"
#include <string.h>
#include <stdint.h>
#include "trace.h"

/* Note: MJPEG_MALLOC, MJPEG_REALLOC, MJPEG_FREE are defined in header */

#if MJPEG_USE_STATIC_BUFFER
/* Static buffer for embedded systems */
static char static_buffer[MJPEG_STATIC_BUFFER_SIZE];
static int static_buffer_used = 0;
#endif

void mjpeg_buffer_init(mjpeg_buffer_t *buf)
{
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
    buf->header_reserve = 0;
}

void mjpeg_buffer_init_with_header(mjpeg_buffer_t *buf, size_t header_size)
{
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
    buf->header_reserve = header_size;
}

void mjpeg_buffer_free(mjpeg_buffer_t *buf)
{
#if MJPEG_USE_STATIC_BUFFER
    if (buf->data == static_buffer)
    {
        static_buffer_used = 0;
    }
#else
    if (buf->data)
    {
        MJPEG_FREE(buf->data);
    }
#endif
    buf->data = NULL;
    buf->len = 0;
    buf->cap = 0;
}

int mjpeg_buffer_append(mjpeg_buffer_t *buf, const char *data, size_t len)
{
    //DBG_DIRECT("mjpeg_buffer_append, len=%d, %d, %d", len, buf->len, buf->cap);
    if (buf->len + len > buf->cap)
    {
#if MJPEG_USE_STATIC_BUFFER
        /* Static buffer mode: allocate once, fail if full */
        if (!buf->data && !static_buffer_used)
        {
            buf->data = static_buffer;
            buf->cap = MJPEG_STATIC_BUFFER_SIZE;
            static_buffer_used = 1;
        }
        if (buf->len + len > buf->cap)
        {
            /* Buffer overflow: clear existing data to avoid partial frame */
            buf->len = 0;
            return -1; /* Buffer full, current frame discarded */
        }
#else
        /* Dynamic memory mode with fixed size limit */
        if (buf->cap)
        {
            /* Buffer already allocated but full - discard partial frame */
            buf->len = 0;
            return -1; /* Frame too large, buffer cleared */
        }

        /* First allocation: fixed size, no expansion */
        size_t new_cap = (50 * 1024); /* Fixed 22KB limit */
        char *new_data;

        /* Check if even initial allocation won't fit */
        if (buf->len + len > new_cap)
        {
            /* Frame is too large even for initial buffer */
            buf->len = 0;
            return -1; /* Frame exceeds buffer capacity */
        }

        /* Allocate with header reserve space */
        new_data = (char *)MJPEG_REALLOC(buf->data, new_cap + buf->header_reserve);
        if (!new_data)
        {
            DBG_DIRECT("Out of memory");
            /* Out of memory: clear any existing data */
            buf->len = 0;
            return -1; /* OOM, buffer cleared */
        }

        DBG_DIRECT("Allocating %d bytes", new_cap);

        buf->data = new_data;
        buf->cap = new_cap;
#endif
    }

    /* Append data after header reserve area */
    void *jpeg_memcpy(void *dest, const void *src, uint32_t n);
    jpeg_memcpy(buf->data + buf->header_reserve + buf->len, data, len);
    buf->len += len;
    return 0;
}

const char *mjpeg_buffer_find(mjpeg_buffer_t *buf, const char *needle, size_t needle_len)
{
    size_t i;
    char *data_start;

    if (buf->len < needle_len)
    {
        return NULL;
    }

    /* Search in data area (skip header reserve) */
    data_start = buf->data + buf->header_reserve;

    for (i = 0; i <= buf->len - needle_len; i++)
    {
        if (memcmp(data_start + i, needle, needle_len) == 0)
        {
            return data_start + i;
        }
    }

    return NULL;
}

void mjpeg_buffer_consume(mjpeg_buffer_t *buf, size_t n)
{
    char *data_start;

    if (n >= buf->len)
    {
        buf->len = 0;
    }
    else
    {
        /* Move remaining data within data area (skip header reserve) */
        data_start = buf->data + buf->header_reserve;
        void *jpeg_memmove(void *dest, const void *src, size_t n);
        jpeg_memmove(data_start, data_start + n, buf->len - n);
        //memmove(data_start, data_start + n, buf->len - n);
        buf->len -= n;
    }
}

char *mjpeg_buffer_header_start(mjpeg_buffer_t *buf)
{
    return buf->data;
}

char *mjpeg_buffer_data_start(mjpeg_buffer_t *buf)
{
    return buf->data ? buf->data + buf->header_reserve : NULL;
}
