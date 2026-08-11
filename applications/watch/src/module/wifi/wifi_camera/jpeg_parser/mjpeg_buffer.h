/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* MJPEG Buffer Management Module
 *
 * Provides dynamic or static buffer management for MJPEG stream parsing
 * Configurable for embedded systems with limited RAM
 */

#ifndef B2DF3582_1A2F_4A39_B4DD_7503E2295FDD
#define B2DF3582_1A2F_4A39_B4DD_7503E2295FDD

#ifndef MJPEG_BUFFER_H
#define MJPEG_BUFFER_H

#include <stddef.h>

/* ========================================================================
 * Custom Memory and Log Functions
 * User should define these before including this header, or use defaults
 * ======================================================================== */

/* Memory allocation functions */
#ifndef MJPEG_MALLOC
// #include <stdlib.h>
// #define MJPEG_MALLOC(size)        malloc(size)
// #define MJPEG_REALLOC(ptr, size)  realloc(ptr, size)
// #define MJPEG_FREE(ptr)           free(ptr)
/* must define before include mjpeg_parser.h */
void *my_malloc(size_t size);
void *my_realloc(void *ptr, size_t size);
void my_free(void *ptr);
void my_log(const char *level, const char *fmt, ...);
#define MJPEG_MALLOC(size)        my_malloc(size)
#define MJPEG_REALLOC(ptr, size)  my_realloc(ptr, size)
#define MJPEG_FREE(ptr)           my_free(ptr)
#define MJPEG_LOG(level, fmt, ...) my_log(level, fmt, ##__VA_ARGS__)
#endif

/* Log function */
#ifndef MJPEG_LOG
#include <stdio.h>
#define MJPEG_LOG(level, fmt, ...) printf("[MJPEG-%s] " fmt "\n", level, ##__VA_ARGS__)
#endif

/* Configuration */
#ifndef MJPEG_USE_STATIC_BUFFER
#define MJPEG_USE_STATIC_BUFFER 0  /* 0=dynamic, 1=static */
#endif

#ifndef MJPEG_STATIC_BUFFER_SIZE
#define MJPEG_STATIC_BUFFER_SIZE (16*1024)  /* 32KB static buffer */
#endif

/* Buffer structure */
typedef struct
{
    char *data;      /* Buffer data pointer */
    size_t len;      /* Current data length */
    size_t cap;      /* Buffer capacity */
    size_t header_reserve; /* Reserved space at buffer start for custom header */
} mjpeg_buffer_t;

/* Initialize buffer */
void mjpeg_buffer_init(mjpeg_buffer_t *buf);

/* Initialize buffer with reserved header space */
void mjpeg_buffer_init_with_header(mjpeg_buffer_t *buf, size_t header_size);

/* Free buffer resources */
void mjpeg_buffer_free(mjpeg_buffer_t *buf);

/* Append data to buffer
 * Returns: 0 on success, -1 on error (OOM or buffer full)
 */
int mjpeg_buffer_append(mjpeg_buffer_t *buf, const char *data, size_t len);

/* Find substring in buffer
 * Returns: pointer to first occurrence, or NULL if not found
 */
const char *mjpeg_buffer_find(mjpeg_buffer_t *buf, const char *needle, size_t needle_len);

/* Remove first n bytes from buffer
 * Moves remaining data to beginning
 */
void mjpeg_buffer_consume(mjpeg_buffer_t *buf, size_t n);

/* Get header area start pointer (writable, user can write custom header) */
char *mjpeg_buffer_header_start(mjpeg_buffer_t *buf);

/* Get data area start pointer (after header reserve) */
char *mjpeg_buffer_data_start(mjpeg_buffer_t *buf);

#endif /* MJPEG_BUFFER_H */


#endif /* B2DF3582_1A2F_4A39_B4DD_7503E2295FDD */
