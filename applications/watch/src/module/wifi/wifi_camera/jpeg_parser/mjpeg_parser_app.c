/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */




/////////////////////////////JPEG Parser////////////////////////////////
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/sd/sd.h>
#include <zephyr/sd/sdio.h>
#include <zephyr/drivers/gpio.h>
#include  "os_mem.h"
#include "def_file.h"
#include "draw_img.h"
#include <stddef.h>
#include <string.h>
#include "mjpeg_parser_app.h"
#include "gui_message.h"
#include "gui_img.h"
#include "trace.h"
#include "wifi_app.h"
#include "wifi_atcmd.h"

/* ========================================================================
 * Step 1: Define custom memory and logging functions (before including header files)
 * ======================================================================== */

typedef struct list_node
{
    unsigned char *node_alloc_buffer;
    // unsigned char *node_jpeg_buffer;
    // unsigned char *node_accu_buffer;
    uint32_t node_accu_size  ;

    struct list_node *p_next_node ;
    struct list_node *p_prev_node ;

    uint16_t data_length;
    uint8_t p_data[0];
} sdio_queue_t;


#define JPEG_ADDR_ALIGNED  8

#define  JPEG_BUF_NODE_NUM  4

#define jpeg_parser_malloc(n)  os_mem_aligned_alloc(OS_MEM_TYPE_DATA, n, JPEG_ADDR_ALIGNED)
#define jpeg_parser_free(n)    os_mem_aligned_free(n)



#define  BOCLK_SIZE  (60 * 1024)  // 60KB per queue node
// CRITICAL: DEFAULT_MAX_FRAME_SIZE in minimal_build_config.h
// must be >= 81920 (80KB) to support frames > 32KB
// Mismatch causes "os_mem_alloc failed" when frame > 32KB

#define DDR_BASE_ADDR              0x24000000
#define MJPEG_NODE_BUFFER_SIZE     (JPEG_BUF_NODE_NUM * BOCLK_SIZE)  // 4 * 60KB = 240KB
#define MJPEG_DISPLAY_BUFFER_SIZE  BOCLK_SIZE                        // 60KB display buffer

static uint8_t *const mjpeg_node_buffer = (uint8_t *)DDR_BASE_ADDR;
static uint8_t *const mjpeg_display_buffer = (uint8_t *)(DDR_BASE_ADDR + MJPEG_NODE_BUFFER_SIZE);

RAM_TEXT_SECTION void *jpeg_memcpy(void *dest, const void *src, uint32_t n)
{
    //DBG_DIRECT("jpeg_memcpy");
    extern void __aeabi_memcpy(void *dest, const void *src, uint32_t n);
    __aeabi_memcpy(dest, src, n);
    return dest;
}

void *jpeg_memmove(void *dest, const void *src, size_t n)
{
    extern void __aeabi_memmove(void *dest, const void *src, uint32_t n);
    __aeabi_memmove(dest, src, n);
    return dest;
}


/*
 * To implement a realloc compatible with jpeg_parser_malloc/free, maintain a simple
 * pointer->size tracking table for safe copying of original content during expansion/contraction.
 */
typedef struct _parser_alloc_rec
{
    void *ptr;
    size_t size;
    struct _parser_alloc_rec *next;
} parser_alloc_rec_t;

static parser_alloc_rec_t *g_parser_allocs = NULL;

static void parser_track_add(void *p, size_t sz)
{
    if (!p) { return; }
    int s = os_lock();
    parser_alloc_rec_t *rec = (parser_alloc_rec_t *)jpeg_parser_malloc(sizeof(parser_alloc_rec_t));
    if (rec)
    {
        rec->ptr = p;
        rec->size = sz;
        rec->next = g_parser_allocs;
        g_parser_allocs = rec;
    }
    os_unlock(s);
}

static size_t parser_track_get_size(void *p)
{
    if (!p) { return 0; }
    int s = os_lock();
    parser_alloc_rec_t *it = g_parser_allocs;
    while (it)
    {
        if (it->ptr == p)
        {
            size_t sz = it->size;
            os_unlock(s);
            return sz;
        }
        it = it->next;
    }
    os_unlock(s);
    return 0;
}

static void parser_track_remove(void *p)
{
    if (!p) { return; }
    int s = os_lock();
    parser_alloc_rec_t **pp = &g_parser_allocs;
    while (*pp)
    {
        if ((*pp)->ptr == p)
        {
            parser_alloc_rec_t *victim = *pp;
            *pp = victim->next;
            os_mem_free(victim);
            break;
        }
        pp = &((*pp)->next);
    }
    os_unlock(s);
}

static void *jpeg_parser_realloc(void *ptr, size_t new_size);

void *my_malloc(size_t size)
{
    void *ptr = jpeg_parser_malloc(size);
    if (ptr)
    {
        parser_track_add(ptr, size);
    }
    //DBG_DIRECT("[MEM] Allocated %zu bytes at %p\n", size, ptr);
    return ptr;
}

void *my_realloc(void *ptr, size_t size)
{
    void *new_ptr = jpeg_parser_realloc(ptr, size);
    DBG_DIRECT("[MEM] Reallocated %p to %zu bytes at %p\n", ptr, size, new_ptr);
    return new_ptr;
}

void my_free(void *ptr)
{
    if (ptr)
    {
        parser_track_remove(ptr);
        jpeg_parser_free(ptr);
    }
}


void my_log(const char *level, const char *format, ...)
{
    va_list args;
    DBG_DIRECT("[MJPEG-%s] ", level);
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    DBG_DIRECT("\n");
}

// /* must define before include mjpeg_parser.h */
// #define MJPEG_MALLOC(size)        my_malloc(size)
// #define MJPEG_REALLOC(ptr, size)  my_realloc(ptr, size)
// #define MJPEG_FREE(ptr)           my_free(ptr)
// #define MJPEG_LOG(level, fmt, ...) my_log(level, fmt, ##__VA_ARGS__)

static const char *jpeg_parser_boundary = "123456789000000000000987654321";

/* ========================================================================
 * Step 2: Include MJPEG parser header file
 * ======================================================================== */
#include "mjpeg_parser.h"

/*
 * Implementation of jpeg_parser_realloc
 * Semantics:
 * - When ptr == NULL, equivalent to malloc(new_size)
 * - When new_size == 0, equivalent to free(ptr), returns NULL
 * - Otherwise allocate new block of new_size, copy min(old_size, new_size) bytes, free old block
 * Note: Get old_size through internal tracking table
 */
static void *jpeg_parser_realloc(void *ptr, size_t new_size)
{
    if (ptr == NULL)
    {
        if (new_size == 0)
        {
            return NULL;
        }
        void *np = jpeg_parser_malloc(new_size);
        if (np) { parser_track_add(np, new_size); }
        return np;
    }

    if (new_size == 0)
    {
        parser_track_remove(ptr);
        jpeg_parser_free(ptr);
        return NULL;
    }

    size_t old_size = parser_track_get_size(ptr);
    void *np = jpeg_parser_malloc(new_size);
    if (!np)
    {
        // Keep original pointer unchanged
        return NULL;
    }

    size_t cpsz = old_size < new_size ? old_size : new_size;
    if (cpsz > 0)
    {
        // Use jpeg_memcpy to maintain platform consistency
        jpeg_memcpy(np, ptr, (uint32_t)cpsz);
    }

    // Free old block and update tracking
    parser_track_remove(ptr);
    jpeg_parser_free(ptr);
    parser_track_add(np, new_size);
    return np;
}


/* ========================================================================
 * Step 3: Define JPEG frame processing callback
 * ======================================================================== */

/* User data structure */
typedef struct
{
    int total_frames;
    size_t total_bytes;
} user_context_t;

mjpeg_parser_t parser;
user_context_t ctx = {0};

/* Frame receive callback - called when complete JPEG data is received */
int on_jpeg_frame(int num,
                  char *header_start, size_t header_size,  // Reserved area
                  const char *jpeg, size_t jpeg_len,       // JPEG data
                  void *ctx)
{

    DBG_DIRECT("\n=== Rev JPEG Frame Success,header_sz %d, jpeg_len %d\n", header_size, jpeg_len);

    extern uint8_t sdio_list_write_full(void);
    extern uint8_t sdio_list_write_node_update(uint8_t *data, uint32_t data_length);
    if (!sdio_list_write_full())
    {
        if (sdio_list_write_node_update((uint8_t *)jpeg, jpeg_len))
        {
            DBG_DIRECT("[Yuyin] Queue write success, frame enqueued\\n");
            void mjpeg_gui_msg_handler(void);
            mjpeg_gui_msg_handler();
        }
        else
        {
            DBG_DIRECT("[Yuyin] Queue write failed, dropping frame\\n");
            // Do NOT trigger GUI update on write failure
        }
    }
    else
    {
        DBG_DIRECT("[Yuyin] Queue full, dropping frame\\n");
        // Do NOT trigger GUI update when queue is full
    }


    return 0;
}

/**
 * @brief must be called by wifi task start
 *
 * @return int
 */
int jpeg_parser_init(void)
{
    int ret;

    /* init jpeg parser */
    //ret = mjpeg_parser_init(&parser, "123456789000000000000987654321");
    ret = mjpeg_parser_init_with_header(&parser, jpeg_parser_boundary, 16);
    if (ret < 0)
    {
        DBG_DIRECT("ERR: JPEG Parser Initial failed\n");
        return 1;
    }

    DBG_DIRECT("[Yuyin] JPEG Parser Success\n\n");

    return 0;
}

int mjpeg_parser(const char *data, size_t len, mjpeg_frame_callback_t callback)
{
    mjpeg_parser_feed(&parser, data, len,
                      (mjpeg_frame_callback_t)callback, &ctx);
}

void jpeg_parser_deinit(void)
{
    DBG_DIRECT("[Yuyin] jpeg_parser_deinit\n");
    mjpeg_parser_free(&parser);
}


////////////////////////////////////////////// Queue ////////////////////////////////////////

#include "trace.h"
#include "section.h"
//#include "sdio_queue.h"
#include "string.h"


sdio_queue_t *p_head_list = NULL;
sdio_queue_t *p_list_write = NULL;
sdio_queue_t *p_list_read = NULL;

#define list_free(ptr) jpeg_parser_free(ptr)

sdio_queue_t *sdio_queue_add_node(sdio_queue_t *p_list_head, sdio_queue_t *p_node);

uint8_t sdio_list_read_empty(void)
{
    int s = os_lock();
    uint8_t is_empty;
    if (p_list_write == p_list_read)
    {
        is_empty = 1;
    }
    else
    {
        is_empty = 0;
    }
    os_unlock(s);
    return is_empty;
}

uint8_t sdio_list_write_full(void)
{
    if (p_list_write && p_list_read)
    {
        if (p_list_write->p_next_node == p_list_read)
        {
            return 1;
        }
    }

    return 0;
}

sdio_queue_t *sdio_list_init(uint8_t node_num)
{

    if (node_num > JPEG_BUF_NODE_NUM)
    {
        DBG_DIRECT("[Yuyin] Warning: node_num %d exceed max 4, clamped to 4\n", node_num);
        node_num = JPEG_BUF_NODE_NUM;
    }

    if (p_head_list == NULL)
    {
        for (uint8_t index = 0; index < node_num; index++)
        {
            sdio_queue_t *p_node = (sdio_queue_t *)jpeg_parser_malloc(sizeof(sdio_queue_t));
            if (!p_node)
            {
                DBG_DIRECT("[Yuyin] jpeg_parser_malloc node failed idx=%d\n", index);
                break;
            }
            memset(p_node, 0, sizeof(sdio_queue_t));

            p_node->node_alloc_buffer = &mjpeg_node_buffer[index * BOCLK_SIZE];

            p_node->node_accu_size = BOCLK_SIZE;
            p_node->data_length = 0;

            DBG_DIRECT("[Yuyin] Node %d: buffer @ 0x%x, size %d\n",
                       index, (uint32_t)p_node->node_alloc_buffer, p_node->node_accu_size);

            p_head_list = sdio_queue_add_node(p_head_list, p_node);
        }
    }
    else
    {
        sdio_queue_t *p_node = p_head_list;
        uint8_t index = 0;
        while (p_node)
        {
            sdio_queue_t *p_next = p_node->p_next_node;
            p_node->node_alloc_buffer = &mjpeg_node_buffer[index++ * BOCLK_SIZE];
            p_node->node_accu_size = BOCLK_SIZE;
            p_node->data_length = 0;

            p_node = p_next;

            if (p_node == p_head_list)
            {
                break;
            }
        }
    }


    p_list_write = p_head_list;
    p_list_read = p_head_list;

    return p_head_list;
}

void sdio_list_clear(void)
{
    int s = os_lock();
    sdio_queue_t *p_node = p_head_list;
    while (p_node)
    {
        sdio_queue_t *p_next = p_node->p_next_node;
        if (p_node->node_alloc_buffer)
        {
            //jpeg_parser_free(p_node->node_alloc_buffer);
            p_node->node_alloc_buffer = NULL;
            p_node->node_accu_size = 0;
        }
        //jpeg_parser_free(p_node);
        p_node = p_next;

        if (p_node == p_head_list)
        {
            break;
        }
    }

    p_list_write = p_head_list;
    p_list_read  = p_head_list;

    os_unlock(s);

}



uint8_t sdio_list_write_node_update(uint8_t *data,
                                    uint32_t data_length)
{
    int s = os_lock();  // Lock to protect concurrent access
    uint32_t copy_len = data_length;

    if (data_length > p_list_write->node_accu_size)
    {
        DBG_DIRECT("[Yuyin] Warning: data_length %d exceed node_accu_size %d, node_alloc_buffer 0x%x\n",
                   data_length, p_list_write->node_accu_size, p_list_write->node_alloc_buffer);
        return 0;
    }

    if (!sdio_list_write_full())
    {
        if (p_list_write->node_alloc_buffer == NULL)
        {
            os_unlock(s);
            return 0;
        }

        gui_jpeg_file_head_t *head = (gui_jpeg_file_head_t *)p_list_write->node_alloc_buffer;
        memset(head, 0, 16);
        head->img_header.type     = JPEG;
        head->img_header.compress = false;
        head->img_header.jpeg     = true;
        head->img_header.w        = 466;   // TODO: parse real width/height from SOF
        head->img_header.h        = 466;
        head->size                = copy_len;

        DBG_DIRECT("[Yuyin] Write node: 0x%x, copy_len %d bytes (buffer size=%d)\n",
                   p_list_write->node_alloc_buffer, copy_len, p_list_write->node_accu_size);
        jpeg_memcpy(p_list_write->node_alloc_buffer + 16, data, copy_len);

        /* CRITICAL: Store total length (16-byte header + JPEG data) for read operations */
        /* This ensures we copy the entire structure: metadata + image data */
        p_list_write->data_length = copy_len + 16;

        p_list_write = p_list_write->p_next_node;

        os_unlock(s);
        return 1;
    }
    else
    {
        DBG_DIRECT("[Yuyin] Queue full, override!!\n");
        if (p_list_read->node_alloc_buffer == NULL)
        {
            //jpeg_parser_free(p_list_read->node_alloc_buffer);
        }

        p_list_read = p_list_read->p_next_node;
        //write
        if (!sdio_list_write_full())
        {
            if (p_list_write->node_alloc_buffer == NULL)
            {
                os_unlock(s);
                return 0;
            }

            gui_jpeg_file_head_t *head = (gui_jpeg_file_head_t *)p_list_write->node_alloc_buffer;
            memset(head, 0, 16);
            head->img_header.type     = JPEG;
            head->img_header.compress = false;
            head->img_header.jpeg     = true;
            head->img_header.w        = 466;
            head->img_header.h        = 466;
            head->size                = copy_len;

            jpeg_memcpy(p_list_write->node_alloc_buffer + 16, data, copy_len);

            /* CRITICAL: Store total length (16-byte header + JPEG data) for read operations */
            p_list_write->data_length = copy_len + 16;

            p_list_write = p_list_write->p_next_node;

            os_unlock(s);
            return 1;
        }
    }

    os_unlock(s);
    return 0;
}

uint8_t sdio_list_read_node(uint8_t *buffer)
{

    int s = os_lock();  // Lock to protect concurrent access
    if (sdio_list_read_empty())
    {
        os_unlock(s);
        return 0;
    }

    if (buffer == NULL)
    {
        DBG_DIRECT("[Yuyin] Error02, this should not happened!\n");
        os_unlock(s);
        return 0;
    }

    /* Safety checks before reading */
    if (p_list_read->node_alloc_buffer == NULL)
    {
        DBG_DIRECT("[Yuyin] Error: node_alloc_buffer is NULL\n");
        os_unlock(s);
        return 0;
    }

    if (p_list_read->data_length == 0)
    {
        DBG_DIRECT("[Yuyin] Error: data_length is 0, skipping read\n");
        p_list_read = p_list_read->p_next_node;
        os_unlock(s);
        return 0;
    }

    /* Sanity check: data_length should not exceed buffer size */
    if (p_list_read->data_length > p_list_read->node_accu_size)
    {
        DBG_DIRECT("[Yuyin] Warning: data_length (%d) > node_accu_size (%d), clamping\n",
                   p_list_read->data_length, p_list_read->node_accu_size);
        p_list_read->data_length = p_list_read->node_accu_size;
    }

    // copy node data to display buffer
    // DBG_DIRECT("===>%x, %x %d", buffer, p_list_read->node_alloc_buffer, p_list_read->data_length);
    jpeg_memcpy(buffer, p_list_read->node_alloc_buffer, p_list_read->data_length);
    // DBG_DIRECT("<===");

    // clear read node
    p_list_read->data_length = 0;

    // update read pointer
    p_list_read = p_list_read->p_next_node;

    os_unlock(s);
    return 1;
}


const uint8_t *gui_read_jpeg_data(void)
{

    uint8_t ret = sdio_list_read_node(mjpeg_display_buffer);
    if (ret)
    {
        // extern const  unsigned char video_logo_image[];
        // memcpy(mjpeg_display_buffer, video_logo_image, 12914);
        return mjpeg_display_buffer;
    }

    return NULL;
}

sdio_queue_t *sdio_queue_add_node(sdio_queue_t *p_list_head, sdio_queue_t *p_node)
{
    int s = os_lock();
    if (p_node == NULL)
    {
        os_unlock(s);
        return p_list_head;
    }

    if (p_list_head == NULL)
    {
        p_list_head = p_node;

        p_list_head->p_next_node = p_list_head;
        p_list_head->p_prev_node = p_list_head;
    }
    else
    {
        p_node->p_prev_node = p_list_head->p_prev_node;
        p_node->p_next_node = p_list_head;

        p_node->p_prev_node->p_next_node = p_node;
        p_node->p_next_node->p_prev_node = p_node;

        p_list_head = p_node;
    }
    os_unlock(s);
    return p_list_head;
}

/**
 * @brief add queue node for list head
 * @param sdio_queue_t *p_list_head: list head pointer
 *        uint8_t *p_data : data need to be add
 *        uint16_t length: data length for p_data
 * @return sdio_queue_t *: new list head
 * @note  new list head need to be handle
 *
 * @example
 * sdio_queue_t *p_sdio_list = NULL;
 * uint8_t data[20] = {};
 * p_sdio_list = sdio_queue_add_data(p_sdio_list, data, sizeof(data));
 */
sdio_queue_t *sdio_queue_add_data(sdio_queue_t *p_list_head, sdio_queue_t *p_node_data,
                                  uint16_t length)
{
    int s = os_lock();
    if (p_node_data == NULL)
    {
        os_unlock(s);
        return p_list_head;
    }

    if (p_list_head == NULL)
    {
        p_list_head = p_node_data;
        if (p_list_head == NULL)
        {
            os_unlock(s);
            return NULL;
        }
        p_list_head->data_length = length;
        p_list_head->p_next_node = p_list_head;
        p_list_head->p_prev_node = p_list_head;
    }
    else
    {
        sdio_queue_t *p_node = p_node_data;
        if (p_node == NULL)
        {
            APP_PRINT_INFO0("[Yuyin] list_malloc failed ====> ");
            os_unlock(s);
            return p_list_head;
        }
        p_node->data_length = length;
        p_node->p_prev_node = p_list_head->p_prev_node;
        p_node->p_next_node = p_list_head;
        p_node->p_prev_node->p_next_node = p_node;
        p_node->p_next_node->p_prev_node = p_node;
        p_list_head = p_node;
    }
    os_unlock(s);
    return p_list_head;
}

/**
 * @brief remove node by node_index from list
 * @param sdio_queue_t *p_list_head: list head pointer
 *        uint16_t node_index: node index to be remove
 * @return sdio_queue_t *: new list head
 *
 * @example sdio_queue_t *p_sdio_list = NULL;
 *          uint8_t data[20] = {};
 *          p_sdio_list = sdio_queue_add_data(p_sdio_list, data, sizeof(data));
 *          p_sdio_list = sdio_queue_remove_node(p_sdio_list, 0);// p_list_head is null
 */
sdio_queue_t *sdio_queue_remove_node(sdio_queue_t *p_list_head, uint16_t node_index)
{
    int s = os_lock();
    if (p_list_head == NULL)
    {
        os_unlock(s);
        return NULL;
    }

    sdio_queue_t *p_target_node = NULL;
    uint16_t index = 0;
    for (sdio_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        if (index == node_index)
        {
            p_target_node = p_iterator;
            break;
        }
        if (p_iterator->p_next_node == p_list_head)
        {
            os_unlock(s);
            return p_list_head;
        }
        index++;
    }

    if (node_index == 0)
    {
        if (p_list_head->p_next_node == p_list_head)
        {
            list_free(p_list_head);
            p_list_head = NULL;
            os_unlock(s);
            return NULL;
        }
        else
        {
            p_list_head = p_list_head->p_next_node;
        }
    }

    if (p_target_node != NULL)
    {
        p_target_node->p_next_node->p_prev_node = p_target_node->p_prev_node;
        p_target_node->p_prev_node->p_next_node = p_target_node->p_next_node;
        list_free(p_target_node);
        p_target_node = NULL;
    }
    os_unlock(s);
    return p_list_head;
}

void sdio_queue_clear(sdio_queue_t **p_list)
{
    int s = os_lock();
    if (NULL == p_list)
    {
        os_unlock(s);
        return;
    }
    sdio_queue_t *p_list_head = *p_list;
    if (p_list_head == NULL)
    {
        os_unlock(s);
        return;
    }

    for (;;)
    {
        if (p_list_head->p_prev_node == p_list_head)
        {
            DBG_DIRECT("[Yuyin] 001 0x%x", p_list_head);
            list_free(p_list_head);
            p_list_head = NULL;
            break;
        }

        sdio_queue_t *p_temp_node = p_list_head;

        p_list_head->p_prev_node->p_next_node = p_list_head->p_next_node;
        p_list_head->p_next_node->p_prev_node = p_list_head->p_prev_node;

        p_list_head->p_prev_node = NULL;
        p_list_head = p_list_head->p_next_node;

        DBG_DIRECT("[Yuyin] 002");
        list_free(p_temp_node);
        p_temp_node = NULL;
    }

    *p_list = NULL;
    os_unlock(s);
}

sdio_queue_t *sdio_queue_indexof_last(sdio_queue_t *p_list_head)
{
    int s = os_lock();
    if (p_list_head == NULL)
    {
        os_unlock(s);
        return NULL;
    }
    sdio_queue_t *ret = p_list_head->p_prev_node;
    os_unlock(s);
    return ret;
}

sdio_queue_t *sdio_queue_remove_last_node(sdio_queue_t *p_list_head)
{
    int s = os_lock();
    if (p_list_head == NULL)
    {
        os_unlock(s);
        return NULL;
    }
    sdio_queue_t *p_target_node = p_list_head->p_prev_node;
    if (p_target_node == p_list_head)
    {
        list_free(p_list_head);
        p_list_head = NULL;
        os_unlock(s);
        return NULL;
    }
    p_target_node->p_next_node->p_prev_node = p_target_node->p_prev_node;
    p_target_node->p_prev_node->p_next_node = p_target_node->p_next_node;
    list_free(p_target_node);
    p_target_node = NULL;
    os_unlock(s);
    return p_list_head;
}

sdio_queue_t *sdio_queue_remove_first_node(sdio_queue_t *p_list_head)
{
    // Reuse remove_node which is already locked
    return sdio_queue_remove_node(p_list_head, 0);
}

sdio_queue_t *sdio_queue_indexof(sdio_queue_t *p_list_head, uint16_t node_index)
{
    int s = os_lock();
    if (p_list_head == NULL)
    {
        os_unlock(s);
        return NULL;
    }
    uint16_t index = 0;
    sdio_queue_t *p_iterator = NULL;
    for (p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        if (index == node_index)
        {
            os_unlock(s);
            return p_iterator;
        }
        if (p_iterator->p_next_node == p_list_head)
        {
            os_unlock(s);
            return NULL;
        }
        index++;
    }
    os_unlock(s);
    return NULL;
}

void sdio_queue_printf(sdio_queue_t *p_list_head)
{
    int s = os_lock();
    if (p_list_head == NULL)
    {
        APP_PRINT_INFO0("[Yuyin] queue is null!");
        os_unlock(s);
        return;
    }
    uint16_t index = 0;
    for (sdio_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        //APP_PRINT_INFO2("[Yuyin] queue index %d, data %b", index, TRACE_BINARY(p_iterator->data_length, p_iterator->p_data));
        if (p_iterator->p_next_node == p_list_head)
        {
            APP_PRINT_INFO1("[Yuyin] queue node num %d", index);
            os_unlock(s);
            return;
        }
        index++;
    }
    os_unlock(s);
}



////////////////////////////GUI Msg Handler/////////////////////////////////

#include "trace.h"
// callback
void mjpeg_gui_msg_callback(void *msg)
{
    void gui_img_set_attribute(gui_img_t  *_this,
                               const char *name,
                               void       *addr,
                               int16_t     x,
                               int16_t     y);

    DBG_DIRECT("update data");
    // gui_img_set_attribute(img, "img_1_test", addr, 0, 0);

    const uint8_t *gui_read_jpeg_data(void);
    uint8_t sdio_list_read_empty(void);

    if (sdio_list_read_empty())
    {
        // Queue is empty - do NOT trigger another callback immediately
        // This breaks the infinite loop when no new frames arrive
        DBG_DIRECT("[WARN] Queue empty, stopping GUI callbacks\n");
        return;
    }

    int s = os_lock();

    const uint8_t *p_next_disbuf = gui_read_jpeg_data();
    if (p_next_disbuf)
    {
        // DBG_DIRECT("JPEG Data: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
        //     p_next_disbuf[0], p_next_disbuf[1], p_next_disbuf[2], p_next_disbuf[3], p_next_disbuf[4], p_next_disbuf[5], p_next_disbuf[6],
        //     p_next_disbuf[7], p_next_disbuf[8], p_next_disbuf[9], p_next_disbuf[10], p_next_disbuf[11], p_next_disbuf[12], p_next_disbuf[13],
        //     p_next_disbuf[14], p_next_disbuf[15], p_next_disbuf[16], p_next_disbuf[17], p_next_disbuf[18], p_next_disbuf[19]);
        void update_video_image(uint8_t *);

        // DBG_DIRECT("JPEG Data Addr %x", p_next_disbuf);
        // for (uint32_t index =0; index < 12914; index++)
        // {
        //     extern const  unsigned char video_logo_image[];
        //     if (p_next_disbuf[index] != video_logo_image[index])
        //     {
        //         DBG_DIRECT("[Yuyin] Error found, index %d\n", index);
        //         break;
        //     }
        // }
        update_video_image(p_next_disbuf);

        // Only trigger next callback if queue is NOT empty
        // This prevents infinite self-triggering when stream stops
        if (!sdio_list_read_empty())
        {
            void mjpeg_gui_msg_handler(void);
            mjpeg_gui_msg_handler();
        }
    }
    else
    {
        // nothing to do
        DBG_DIRECT("[Yuyin] Queue empty, nothing to be done\n");
    }
    os_unlock(s);

}
void mjpeg_gui_msg_handler(void)
{
    gui_msg_t msg = {.event = GUI_EVENT_USER_DEFINE, .cb = mjpeg_gui_msg_callback};
    gui_send_msg_to_server(&msg);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

void wifi_power_on_proc(void)
{
    //return;////////test

    gui_set_keep_active_time(0xFFFFFFFF);
    wifi_enable(true);
    wifi_sdio_init();
    wifi_uart_init();

    // void rtsp_mjpeg_parser_init(void);
    // rtsp_mjpeg_parser_init();

    uart_atcmd_queue_fill(ATCMD_ATPN, "zhang-net,admin123", NULL);
    sdio_queue_t *sdio_list_init(uint8_t node_num);
    sdio_list_init(JPEG_BUF_NODE_NUM);

    int jpeg_parser_init(void);
    jpeg_parser_init();

    uart_atcmd_queue_fill(ATCMD_ATPN, "zhang-net,admin123");


    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}
void wifi_camera_enter_proc(char *ip_addr)
{

    ///////
#if 0
    gui_set_keep_active_time(0xFFFFFFFF);
    wifi_enable(true);
    wifi_sdio_init();
    wifi_uart_init();

    // void rtsp_mjpeg_parser_init(void);
    // rtsp_mjpeg_parser_init();

    sdio_queue_t *sdio_list_init(uint8_t node_num);
    sdio_list_init(JPEG_BUF_NODE_NUM);

    int jpeg_parser_init(void);
    jpeg_parser_init();

    uart_atcmd_queue_fill(ATCMD_ATPN, "zhang-net,admin123");
#endif
    ///////

    uint8_t ip_addr_buf[50] = {0};
    memcpy(ip_addr_buf, "-c,", 3);
    memcpy(ip_addr_buf + 3, ip_addr, strlen(ip_addr));
    memcpy(ip_addr_buf + 3 + strlen(ip_addr), ",-t,10,-p,8082", 14);

    APP_PRINT_INFO1("[wifi] wifi_camera_enter_cb %s", TRACE_STRING(ip_addr_buf));

    uart_atcmd_queue_fill(ATCMD_ATWT, ip_addr_buf, NULL);
    //uart_atcmd_queue_fill(ATCMD_ATWS, NULL);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void wifi_camera_exit_proc(void)
{
    // void rtsp_mjpeg_parser_deinit(void);
    // rtsp_mjpeg_parser_deinit();


    void jpeg_parser_deinit(void);
    jpeg_parser_deinit();

    void sdio_list_clear(void);
    sdio_list_clear();

    void wifi_gpio_disable(void);
    wifi_gpio_disable();
    wifi_enable(false);
}



