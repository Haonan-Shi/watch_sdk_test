/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "os_mem.h"
#include "wifi_rtsp_app.h"
#include "minimal_build_zephyr.h"
#include "def_file.h"
#include "draw_img.h"
#include "def_msg.h"
#include "gui_message.h"
#include "gui_img.h"
#include "wifi_app.h"
#include "wifi_atcmd.h"
#include "os_sync.h"
#include "wifi_sdio.h"
#include "wifi_uart.h"
#include "autoconf.h"
#include "section.h"
#include "string.h"
#include "wifi_sdio.h"
#include "wifi_audio.h"

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

#define jpeg_malloc(n)  os_mem_aligned_alloc(OS_MEM_TYPE_DATA, n, JPEG_ADDR_ALIGNED)
#define jpeg_free(n)    os_mem_aligned_free(n)

#define  JPEG_BUF_NODE_NUM  4


#define  BOCLK_SIZE  (60 * 1024)  // 60KB per queue node
// CRITICAL: DEFAULT_MAX_FRAME_SIZE in minimal_build_config.h
// must be >= 81920 (80KB) to support frames > 32KB
// Mismatch causes "os_mem_alloc failed" when frame > 32KB


#define DDR_BASE_ADDR              0x22000000
#define MJPEG_NODE_BUFFER_SIZE     (JPEG_BUF_NODE_NUM * BOCLK_SIZE)  // 4 * 60KB = 240KB
#define MJPEG_DISPLAY_BUFFER_SIZE  BOCLK_SIZE                        // 60KB display buffer


static uint8_t *const mjpeg_node_buffer = (uint8_t *)DDR_BASE_ADDR;
static uint8_t *const mjpeg_display_buffer = (uint8_t *)(DDR_BASE_ADDR + MJPEG_NODE_BUFFER_SIZE);
// DSP_RAM_BSS_SECTION uint8_t  mjpeg_display_buffer[BOCLK_SIZE];

static void *rtsp_malloc_wrapper(size_t sz, void *user_ctx);
static void rtsp_free_wrapper(void *ptr, void *user_ctx);
static void rtsp_frame_cb(const uint8_t *data, uint32_t len, int is_valid, void *user_ctx);


RAM_TEXT_SECTION void *jpeg_memcpy(void *dest, const void *src, uint32_t n)
{
    extern void __aeabi_memcpy(void *dest, const void *src, uint32_t n);
    __aeabi_memcpy(dest, src, n);
    __DSB();
    __DMB();
    return dest;
}

void *jpeg_memmove(void *dest, const void *src, size_t n)
{
    extern void __aeabi_memmove(void *dest, const void *src, uint32_t n);
    __aeabi_memmove(dest, src, n);
    __DSB();
    __DMB();
    return dest;
}

void rtsp_mjpeg_parser_init(void)
{
    sdio_queue_t *sdio_list_init(uint8_t node_num);
    sdio_list_init(JPEG_BUF_NODE_NUM);

    APP_PRINT_TRACE0("[rtsp] mjpeg parser init");
    mb_init_ex(0, rtsp_malloc_wrapper, rtsp_free_wrapper, NULL, 1, jpeg_memcpy);
    mb_set_frame_callback(rtsp_frame_cb, NULL);

#if CONFIG_WIFI_AUDIO_PLAYER
    void wifi_audio_player_init(void);
    wifi_audio_player_init();
#endif //CONFIG_WIFI_AUDIO_PLAYER


}

void rtsp_mjpeg_parser_deinit(void)
{
    APP_PRINT_TRACE0("[rtsp] mjpeg parser deinit");
    mb_deinit();

    void sdio_list_clear(void);
    sdio_list_clear();

#if CONFIG_WIFI_AUDIO_PLAYER
    // audio
    wifi_audio_stop_play();
#endif // #if CONFIG_WIFI_AUDIO_PLAYER
}


// #define jpeg_parser_malloc(n)  os_mem_aligned_alloc(OS_MEM_TYPE_DATA, n, JPEG_ADDR_ALIGNED)
// #define jpeg_parser_free(n)    os_mem_aligned_free(n)

// Memory allocator wrapper functions required for mb_init
static void *rtsp_malloc_wrapper(size_t sz, void *user_ctx)
{
    (void)user_ctx;
    return jpeg_malloc(sz);
}

static void rtsp_free_wrapper(void *ptr, void *user_ctx)
{
    (void)user_ctx;
    jpeg_free(ptr);
}

void *jpeg_memcpy(void *dest, const void *src, uint32_t n);

static void rtsp_frame_cb(const uint8_t *data, uint32_t len, int is_valid, void *user_ctx)
{
    APP_PRINT_TRACE2("[rtsp] MJPEG Frame received, len: %u, is_valid %d", len, is_valid);

    extern uint8_t sdio_list_write_full(void);
    extern uint8_t sdio_list_write_node_update(uint8_t *data, uint32_t data_length);
    if (!sdio_list_write_full())
    {
        if (sdio_list_write_node_update((uint8_t *)data, len))
        {
            APP_PRINT_TRACE0("[rtsp] Queue write success, frame enqueued\\n");
            void mjpeg_gui_msg_handler(void);
            mjpeg_gui_msg_handler();
        }
        else
        {
            APP_PRINT_TRACE0("[rtsp] Queue write failed, dropping frame\\n");
            // Do NOT trigger GUI update on write failure
        }
    }
    else
    {
        APP_PRINT_TRACE0("[rtsp] Queue full, dropping frame\\n");
        // Do NOT trigger GUI update when queue is full
    }

    /* CRITICAL FIX: DO NOT free 'data' in zero-copy mode!
     * The 'data' pointer is managed by live555 internal buffer pool.
     * Calling mb_free_frame_buffer(data) corrupts the pool and causes:
     * - Memory allocation failures
     * - System crashes
     * - "os_mem_alloc failed" errors
     *
     * Zero-copy mode (mb_init_ex with zero_copy=1) guarantees:
     * - 'data' is valid ONLY during this callback
     * - Internal pool automatically recycles buffers
     * - No manual free needed
     */
    // mb_free_frame_buffer(data); // REMOVED - causes memory corruption in zero-copy mode
}




////////////////////////////////////////////// Queue ////////////////////////////////////////




sdio_queue_t *p_head_list = NULL;
sdio_queue_t *p_list_write = NULL;
sdio_queue_t *p_list_read = NULL;


#define list_free(ptr) jpeg_free(ptr)

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
        APP_PRINT_TRACE1("[rtsp] Warning: node_num %d exceed max 4, clamped to 4\n", node_num);
        node_num = JPEG_BUF_NODE_NUM;
    }

    if (p_head_list == NULL)
    {

        for (uint8_t index = 0; index < node_num; index++)
        {
            sdio_queue_t *p_node = (sdio_queue_t *)jpeg_malloc(sizeof(sdio_queue_t));
            if (!p_node)
            {
                APP_PRINT_TRACE1("[rtsp] jpeg_malloc node failed idx=%d\n", index);
                break;
            }
            memset(p_node, 0, sizeof(sdio_queue_t));


            p_node->node_alloc_buffer = &mjpeg_node_buffer[index * BOCLK_SIZE];

            p_node->node_accu_size = BOCLK_SIZE;
            p_node->data_length = 0;

            APP_PRINT_TRACE3("[rtsp] Node %d: buffer @ 0x%x, size %d\n",
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
            //jpeg_free(p_node->node_alloc_buffer);
            p_node->node_alloc_buffer = NULL;
            p_node->node_accu_size = 0;
        }
        //jpeg_free(p_node);
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
        APP_PRINT_ERROR3("[rtsp] Warning: data_length %d exceed node_accu_size %d, node_alloc_buffer 0x%x\n",
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

        APP_PRINT_TRACE3("[rtsp] Write node: 0x%x, copy_len %d bytes (buffer size=%d)\n",
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
        APP_PRINT_TRACE0("[rtsp] Queue full, override!!\n");
        if (p_list_read->node_alloc_buffer == NULL)
        {
            //jpeg_free(p_list_read->node_alloc_buffer);
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
        APP_PRINT_ERROR0("[rtsp] Error02, this should not happened!\n");
        os_unlock(s);
        return 0;
    }

    /* Safety checks before reading */
    if (p_list_read->node_alloc_buffer == NULL)
    {
        APP_PRINT_ERROR0("[rtsp] Error: node_alloc_buffer is NULL\n");
        os_unlock(s);
        return 0;
    }

    if (p_list_read->data_length == 0)
    {
        APP_PRINT_ERROR0("[rtsp] Error: data_length is 0, skipping read\n");
        p_list_read = p_list_read->p_next_node;
        os_unlock(s);
        return 0;
    }

    /* Sanity check: data_length should not exceed buffer size */
    if (p_list_read->data_length > p_list_read->node_accu_size)
    {
        APP_PRINT_ERROR2("[rtsp] Warning: data_length (%d) > node_accu_size (%d), clamping\n",
                         p_list_read->data_length, p_list_read->node_accu_size);
        p_list_read->data_length = p_list_read->node_accu_size;
    }

    // copy node data to display buffer
    // APP_PRINT_TRACE3("===>%x, %x %d", buffer, p_list_read->node_alloc_buffer, p_list_read->data_length);
    jpeg_memcpy(buffer, p_list_read->node_alloc_buffer, p_list_read->data_length);
    // APP_PRINT_TRACE0("<===");

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
            APP_PRINT_INFO0("[rtsp] list_malloc failed ====> ");
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
            list_free(p_list_head);
            p_list_head = NULL;
            break;
        }

        sdio_queue_t *p_temp_node = p_list_head;

        p_list_head->p_prev_node->p_next_node = p_list_head->p_next_node;
        p_list_head->p_next_node->p_prev_node = p_list_head->p_prev_node;

        p_list_head->p_prev_node = NULL;
        p_list_head = p_list_head->p_next_node;

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
        APP_PRINT_INFO0("[rtsp] queue is null!");
        os_unlock(s);
        return;
    }
    uint16_t index = 0;
    for (sdio_queue_t *p_iterator = p_list_head;; p_iterator = p_iterator->p_next_node)
    {
        //APP_PRINT_INFO2("[Yuyin] queue index %d, data %b", index, TRACE_BINARY(p_iterator->data_length, p_iterator->p_data));
        if (p_iterator->p_next_node == p_list_head)
        {
            APP_PRINT_INFO1("[rtsp] queue node num %d", index);
            os_unlock(s);
            return;
        }
        index++;
    }
    os_unlock(s);
}



////////////////////////////GUI Msg Handler/////////////////////////////////

// callback
void mjpeg_gui_msg_callback(void *msg)
{
    void gui_img_set_attribute(gui_img_t  *_this,
                               const char *name,
                               void       *addr,
                               int16_t     x,
                               int16_t     y);

    APP_PRINT_TRACE0("update_video_image");
    // gui_img_set_attribute(img, "img_1_test", addr, 0, 0);

    const uint8_t *gui_read_jpeg_data(void);
    uint8_t sdio_list_read_empty(void);

    if (sdio_list_read_empty())
    {
        // Queue is empty - do NOT trigger another callback immediately
        // This breaks the infinite loop when no new frames arrive
        APP_PRINT_TRACE0("[WARN] Queue empty, stopping GUI callbacks\n");
        return;
    }

    int s = os_lock();

    const uint8_t *p_next_disbuf = gui_read_jpeg_data();
    // APP_PRINT_TRACE1("update_video_image p_next_disbuf = 0x%x", p_next_disbuf);
    if (p_next_disbuf)
    {
        // DBG_DIRECT("JPEG Data: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x",
        //     p_next_disbuf[0], p_next_disbuf[1], p_next_disbuf[2], p_next_disbuf[3], p_next_disbuf[4], p_next_disbuf[5], p_next_disbuf[6],
        //     p_next_disbuf[7], p_next_disbuf[8], p_next_disbuf[9], p_next_disbuf[10], p_next_disbuf[11], p_next_disbuf[12], p_next_disbuf[13],
        //     p_next_disbuf[14], p_next_disbuf[15], p_next_disbuf[16], p_next_disbuf[17], p_next_disbuf[18], p_next_disbuf[19]);
        // void update_video_image(uint8_t *jpeg_disp_image);
        // update_video_image((uint8_t *)p_next_disbuf);

        char *topic = "video_update";
        void gui_msg_publish(const char *topic, void *data, uint16_t len);
        gui_msg_publish(topic, p_next_disbuf, BOCLK_SIZE);

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
        APP_PRINT_TRACE0("[Yuyin] Queue empty, nothing to be done\n");
    }
    os_unlock(s);

}
void mjpeg_gui_msg_handler(void)
{
    gui_msg_t msg = {.event = GUI_EVENT_USER_DEFINE, .cb = mjpeg_gui_msg_callback};
    gui_send_msg_to_server(&msg);
}


/////////////////////////////////////////////////////////////////////////////////////////////

uint16_t wifi_camera_data_process_cb(void *p_data, uint16_t len)
{
    uint32_t time1 = sys_timestamp_get_us();
    int s = os_lock();
#if ((!CONFIG_WIFI_RTSP) && (!CONFIG_WIFI_JPEG_PARSER))
    //#error "CONFIG_WIFI_JPEG_PARSER and CONFIG_WIFI_RTSP cannot be enabled at the same time"
#elif (CONFIG_WIFI_JPEG_PARSER && (!CONFIG_WIFI_RTSP))
    sdiod_rx_desc_t *rx_desc = (sdiod_rx_desc_t *)p_data;

    int mjpeg_parser(
        const char *data,
        size_t len,
        mjpeg_frame_callback_t callback);
    int on_jpeg_frame(int num,
                      char *header_start, size_t header_size,  // Reserved area
                      const char *jpeg, size_t jpeg_len,       // JPEG data
                      void *ctx);
    mjpeg_parser((uint8_t *)p_data + 24, rx_desc->pkt_len, on_jpeg_frame);
#elif (CONFIG_WIFI_RTSP && (!CONFIG_WIFI_JPEG_PARSER ))
    sdiod_rx_desc_t *rx_desc = (sdiod_rx_desc_t *)p_data;
    uint8_t *rtp_data = (uint8_t *)p_data + 24;
    uint8_t marker = (rtp_data[1] & 0x80) >> 7; // 1 jpeg frame segment, 0 jpeg frame end

    // Extract RTP timestamp (bytes 4-7, big-endian)
    uint32_t rtp_ts = ((uint32_t)rtp_data[4] << 24) |
                      ((uint32_t)rtp_data[5] << 16) |
                      ((uint32_t)rtp_data[6] << 8)  |
                      ((uint32_t)rtp_data[7]);

    if ((rtp_data[1] & 0x7F) == 26/*MJPEG PT 26*/)
    {
        // // Video: store timestamp for sync reference
        // extern void wifi_audio_set_video_timestamp(uint32_t ts);
        // wifi_audio_set_video_timestamp(rtp_ts);

        //APP_PRINT_TRACE2("[Yuyin] mjpeg data %d bytes, ts=%u\n", rx_desc->pkt_len, rtp_ts);
        mb_process_packet((uint8_t *)p_data + 24, (uint32_t)rx_desc->pkt_len);
    }
    else
    {
#if CONFIG_WIFI_AUDIO_PLAYER
        // Audio: timestamp will be extracted inside wifi_audio_rtp_depayload
        APP_PRINT_TRACE2("[Yuyin] audio data %d bytes, ts=%u\n", rx_desc->pkt_len, rtp_ts);
        wifi_audio_rtp_depayload((uint8_t *)rtp_data, (uint32_t)rx_desc->pkt_len);
#endif

    }

#endif
    os_unlock(s);
    uint32_t time2 = sys_timestamp_get_us();

    //APP_PRINT_TRACE2("time: %u, len %d", time2 - time1, rx_desc->pkt_len);

#if (CONFIG_WIFI_AUDIO_RECORDER)
    {
        static uint8_t audio_frame[1024];  // Match WIFI_AUDIO_RECORD_FRAME_SIZE

        // Read all available complete frames (320 bytes each)
        uint32_t available = wifi_audio_record_buffer_get_data_size();
        while (available >= 1024)
        {
            uint32_t len = wifi_audio_record_buffer_read(audio_frame, sizeof(audio_frame));
            if (len == 1024)
            {
                APP_PRINT_TRACE1("[Record] Send audio frame %d bytes\n", len);
                wifi_sdio_data_write_queue_fill(0, 0, audio_frame, len);
                available -= len;
            }
            else
            {
                APP_PRINT_TRACE1("[Record] Read incomplete frame: %d bytes\n", len);
                break;
            }
        }
    }

    T_WIFI_SDIO_WRITE_QUEUE *write_pkt = wifi_sdio_data_write_queue_peek(0);
    if (write_pkt)
    {
        if (!wifi_sdio_write_data(write_pkt))
        {
            wifi_sdio_data_write_queue_flush(1);
        }
    }
#endif //CONFIG_WIFI_AUDIO_RECORD

    return 0;
}

void wifi_power_on_proc(void *msg)
{
    return ;
    gui_set_keep_active_time(0xFFFFFFFF);
    wifi_enable(true);
    wifi_sdio_init();
    wifi_uart_init();

    void rtsp_mjpeg_parser_init(void);
    rtsp_mjpeg_parser_init();

    //audio recorder start
#if CONFIG_WIFI_AUDIO_RECORDER
    wifi_recorder_start();
#endif //CONFIG_WIFI_AUDIO_RECORDER

    uart_atcmd_queue_fill(ATCMD_ATPN, "zhang-net,admin123", NULL);
    // uart_atcmd_queue_fill(ATCMD_ATPN, "IPCAM_3705DA,");

    // uint8_t ip_addr_buf[50] = {0};
    // memcpy(ip_addr_buf, "-c,", 3);
    // memcpy(ip_addr_buf + 3, ip_addr, strlen(ip_addr));
    // memcpy(ip_addr_buf + 3 + strlen(ip_addr), ",-t,10,-p,554", 13);

    // APP_PRINT_INFO1("[wifi] wifi_camera_enter_cb %s", TRACE_STRING(ip_addr_buf));

    // uart_atcmd_queue_fill(ATCMD_ATWT, ip_addr_buf);

    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}
void wifi_camera_enter_proc(void *msg)
{
    gui_set_keep_active_time(0xFFFFFFFF);
    wifi_enable(true);
    wifi_sdio_init();
    wifi_uart_init();

    void rtsp_mjpeg_parser_init(void);
    rtsp_mjpeg_parser_init();

    //audio recorder start
#if CONFIG_WIFI_AUDIO_RECORDER
    wifi_recorder_start();
#endif //CONFIG_WIFI_AUDIO_RECORDER

    uart_atcmd_queue_fill(ATCMD_ATPN, "zhang-net,admin123", NULL);
    // // uart_atcmd_queue_fill(ATCMD_ATPN, "IPCAM_3705DA,");

    APP_PRINT_TRACE0("[wifi] wifi_camera_enter_cb");
    char *ip_addr = ((T_WIFI_MSG *)msg)->u.buf;

    APP_PRINT_TRACE1("[wifi] ip_addr %x", ip_addr);

    uint8_t ip_addr_buf[50] = {0};
    memcpy(ip_addr_buf, "-c,", 3);
    memcpy(ip_addr_buf + 3, ip_addr, strlen(ip_addr));
    memcpy(ip_addr_buf + 3 + strlen(ip_addr), ",-t,10,-p,554", 13);

    APP_PRINT_INFO1("[wifi] wifi_camera_enter_cb %s", TRACE_STRING(ip_addr_buf));

    uart_atcmd_queue_fill(ATCMD_ATWT, ip_addr_buf, NULL);

    wifi_sdio_data_read_cb_reg(0xC0A82701, 554, wifi_camera_data_process_cb);
    //uart_atcmd_queue_fill(ATCMD_ATWS, NULL);
    T_WIFI_MSG cmd_msg;
    cmd_msg.event = EVENT_UART_CMD_FLOW_CTRL;
    if (app_send_msg_to_wifitask(&cmd_msg) == false)
    {
        APP_PRINT_ERROR0("[wifi] atcmd rsp msg send fail !");
    }
}

void wifi_camera_exit_proc(void *msg)
{
    void rtsp_mjpeg_parser_deinit(void);
    rtsp_mjpeg_parser_deinit();

    //audio recorder start
#if CONFIG_WIFI_AUDIO_RECORDER
    wifi_recorder_stop();
#endif //CONFIG_WIFI_AUDIO_RECORDER

    wifi_sdio_data_read_cb_unreg(0xC0A82701, 554);

    void wifi_gpio_disable(void);
    wifi_gpio_disable();
    wifi_enable(false);
}
