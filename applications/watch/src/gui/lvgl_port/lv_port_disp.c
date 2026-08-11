/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*Copy this file as "lv_port_disp.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include "lv_port_disp.h"
#include <stdbool.h>
#include "os_mem.h"
#include "trace.h"
#include "rtl876x_gdma.h"
#include "section.h"
#include "dma_channel.h"
#include "lcd_sh8601z_410_502_qspi.h"
#include "clk_mgr.h"

/*********************
 *      DEFINES
 *********************/
#define MY_DISP_HOR_RES    410
#define MY_DISP_VER_RES    502
#ifndef DRV_PIXEL_BITS
#define DRV_PIXEL_BITS     LV_COLOR_DEPTH
#endif

#ifndef MY_DISP_HOR_RES
#warning Please define or replace the macro MY_DISP_HOR_RES with the actual screen width, default value 320 is used for now.
#define MY_DISP_HOR_RES    320
#endif

#ifndef MY_DISP_VER_RES
#warning Please define or replace the macro MY_DISP_VER_RES with the actual screen height, default value 240 is used for now.
#define MY_DISP_VER_RES    240
#endif

#if DRV_PIXEL_BITS == 32
#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_ARGB8888)) /*will be 2 for RGB565 */
#elif DRV_PIXEL_BITS == 16
#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */
#elif DRV_PIXEL_BITS == 24
#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888)) /*will be 3 for RGB888 */
#else
#error "Invalid color depth! Valid options: 16, 24, 32"
#endif

/* Display configuration schemes (choose one):
 * [SCHEME_RAM_PARTIAL]        - Dual RAM buffer with partial rendering
 * [SCHEME_RAM_PSRAM_PARTIAL]  - Dual buffers (RAM+PSRAM) with partial rendering. It does't work now.
 * [SCHEME_PSRAM_DIRECT]       - Dual PSRAM buffer with direct rendering
 * [SCHEME_PSRAM_FULL]         - Dual PSRAM buffer with full rendering */
#define SCHEME_RAM_PARTIAL        0
#define SCHEME_RAM_PSRAM_PARTIAL  1
#define SCHEME_PSRAM_DIRECT       2
#define SCHEME_PSRAM_FULL         3

/* Select active scheme by setting this macro */
#define ACTIVE_DISPLAY_SCHEME SCHEME_PSRAM_DIRECT

/* LV_DRAW_TRANSFORM_USE_MATRIX is enabled, so force display scheme to SCHEME_PSRAM_FULL */
#if LV_DRAW_TRANSFORM_USE_MATRIX
#undef ACTIVE_DISPLAY_SCHEME
#define ACTIVE_DISPLAY_SCHEME SCHEME_PSRAM_FULL
#endif

// Map scheme selection to LVGL render mode
#if ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PARTIAL || ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PSRAM_PARTIAL
#define LV_DISPLAY_RENDER_MODE LV_DISPLAY_RENDER_MODE_PARTIAL
#elif ACTIVE_DISPLAY_SCHEME == SCHEME_PSRAM_DIRECT
#define LV_DISPLAY_RENDER_MODE LV_DISPLAY_RENDER_MODE_DIRECT
#elif ACTIVE_DISPLAY_SCHEME == SCHEME_PSRAM_FULL
#define LV_DISPLAY_RENDER_MODE LV_DISPLAY_RENDER_MODE_FULL
#else
#error "Invalid display scheme! Valid options: SCHEME_RAM_PARTIAL(0), SCHEME_RAM_PSRAM_PARTIAL(1), SCHEME_PSRAM_DIRECT(2), SCHEME_PSRAM_FULL(3)"
#endif

#if ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PARTIAL || ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PSRAM_PARTIAL
#define SECTION_HEIGHT 40
#define SINGLE_PSRAM_BUFFER             0x22000000
#define SINGLE_PSRAM_BUFFER_TAIL        (0x22000000 + MY_DISP_HOR_RES * MY_DISP_VER_RES * LV_COLOR_DEPTH / 8)
#elif ACTIVE_DISPLAY_SCHEME == SCHEME_PSRAM_DIRECT || ACTIVE_DISPLAY_SCHEME == SCHEME_PSRAM_FULL
#define LV_PORT_BUF_PSRAM1        (uint32_t)0x22000000
#define LV_PORT_BUF_PSRAM2        (uint32_t)0x22100000
#endif

/**********************
 *      MACROS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PSRAM_PARTIAL
static uint8_t dma_channel_num = 0xA5;
static GDMA_LLIDef *GDMA_LLIStruct = NULL;
void dma_copy_rect(const lv_area_t *area, uint8_t *color_p);
void dma_wait_done(void);
#endif

static void disp_init(void);

static void disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);

/**********************
 *  STATIC VARIABLES
 **********************/
static bool first_block = true;
static uint32_t invalid_y_end = 0;
static uint32_t last_invalid_y_end = 0;
/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_disp_init(void)
{
    /*-------------------------
     * Initialize your display
     * -----------------------*/
    disp_init();

    /*------------------------------------
     * Create a display and set a flush_cb
     * -----------------------------------*/
    lv_display_t *disp = lv_display_create(MY_DISP_HOR_RES, MY_DISP_VER_RES);
    lv_display_set_flush_cb(disp, disp_flush);

#if ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PARTIAL
    lv_draw_buf_t *buf1 = lv_draw_buf_create(MY_DISP_HOR_RES, SECTION_HEIGHT,
                                             lv_display_get_color_format(0), 0);
    lv_draw_buf_t *buf2 = lv_draw_buf_create(MY_DISP_HOR_RES, SECTION_HEIGHT,
                                             lv_display_get_color_format(0), 0);
    lv_display_set_draw_buffers(disp, buf1, buf2);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE);
#elif ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PSRAM_PARTIAL
    lv_draw_buf_t *buf1 = lv_draw_buf_create(MY_DISP_HOR_RES, SECTION_HEIGHT,
                                             lv_display_get_color_format(0), 0);
    lv_draw_buf_t *buf2 = lv_draw_buf_create(MY_DISP_HOR_RES, SECTION_HEIGHT,
                                             lv_display_get_color_format(0), 0);
    lv_display_set_draw_buffers(disp, buf1, buf2);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE);
    lv_display_set_user_data(disp, (void *)SINGLE_PSRAM_BUFFER);
#elif ACTIVE_DISPLAY_SCHEME == SCHEME_PSRAM_DIRECT || ACTIVE_DISPLAY_SCHEME == SCHEME_PSRAM_FULL
    lv_draw_buf_t *buf1 = lv_draw_buf_create(MY_DISP_HOR_RES, MY_DISP_VER_RES,
                                             lv_display_get_color_format(0), 0);
    lv_draw_buf_t *buf2 = lv_draw_buf_create(MY_DISP_HOR_RES, MY_DISP_VER_RES,
                                             lv_display_get_color_format(0), 0);
    lv_display_set_draw_buffers(disp, buf1, buf2);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE);
#endif

}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your display and the required peripherals.*/
static void disp_init(void)
{
    /* we enable CONFIG_DISPLAY, lcd driver init at zephyr display driver:
    *  display\device\zephyr\lcd\8773G\rtl87x3g-display-lcd-qspi.c
    */
    // rtk_lcd_hal_init();
#if ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PSRAM_PARTIAL
    if (!GDMA_channel_request(&dma_channel_num, NULL, true))
    {
        assert_param(*dma_channel_num != 0xA5);
        return;
    }
#endif

    U_CLK_BITMAP bitmap;
    bitmap.data = BIT(T_CLK_TYPE_CPU) | BIT(T_CLK_TYPE_SPIC0) | BIT(T_CLK_TYPE_SPIC1) | BIT(
                      T_CLK_TYPE_SPIC3);
    T_CLK_USER_HANDLE clk_user_gui = clk_mgr_user_create("gui", bitmap);
    clk_mgr_set_high_performance(clk_user_gui);
}

volatile bool disp_flush_enabled = true;

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void)
{
    disp_flush_enabled = true;
}

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void)
{
    disp_flush_enabled = false;
}

/*Flush the content of the internal buffer the specific area on the display.
 *`px_map` contains the rendered image as raw pixel map and it should be copied to `area` on the display.
 *You can use DMA or any hardware acceleration to do this operation in the background but
 *'lv_display_flush_ready()' has to be called when it's finished.*/
#if ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PARTIAL
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    rtk_lcd_hal_transfer_done();
    rtk_lcd_hal_set_window(area->x1, area->y1, area->x2 - area->x1 + 1, area->y2 - area->y1 + 1);
    rtk_lcd_hal_start_transfer((uint8_t *)px_map,
                               (area->x2 - area->x1 + 1) * (area->y2 - area->y1 + 1));

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}
#elif ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PSRAM_PARTIAL
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    if (disp_flush_enabled)
    {
        uint32_t end_addr_invalid_area = (SINGLE_PSRAM_BUFFER + (LV_HOR_RES *
                                                                 (area->y2 + 1) + area->x2) * BYTE_PER_PIXEL);
        uint32_t end_addr_last_frame = (SINGLE_PSRAM_BUFFER + LV_HOR_RES * (last_invalid_y_end + 1) *
                                        BYTE_PER_PIXEL);
        uint32_t end_addr = LV_MIN(end_addr_invalid_area, end_addr_last_frame);
        uint32_t curr_dma_addr = LCDC_DMA_Channel0->LCDC_DMA_CURR_SARx;
        if (first_block)
        {
            first_block = false;
        }
        else
        {
            dma_wait_done();
        }
        while (curr_dma_addr < end_addr &&
               curr_dma_addr >= SINGLE_PSRAM_BUFFER &&
               curr_dma_addr < SINGLE_PSRAM_BUFFER_TAIL)
        {
            curr_dma_addr = LCDC_DMA_Channel0->LCDC_DMA_CURR_SARx;
        }
        dma_copy_rect(area, px_map);
        invalid_y_end = invalid_y_end > area->y2 ? invalid_y_end : area->y2;
//        SCB_InvalidateDCache();
    }

    if (lv_disp_flush_is_last(disp_drv))
    {
        dma_wait_done();
        first_block = true;
        rtk_lcd_hal_transfer_done();
        rtk_lcd_hal_set_window(0, 0, LV_HOR_RES, (invalid_y_end + 1));
        rtk_lcd_hal_start_transfer((uint8_t *)SINGLE_PSRAM_BUFFER, LV_HOR_RES * (invalid_y_end + 1));
        last_invalid_y_end = invalid_y_end;
        invalid_y_end = 0;
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}
#elif ACTIVE_DISPLAY_SCHEME == SCHEME_PSRAM_DIRECT
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    invalid_y_end = invalid_y_end > area->y2 ? invalid_y_end : area->y2;
    if (lv_disp_flush_is_last(disp_drv))
    {
        int32_t flush_y_end = LV_MAX(invalid_y_end, last_invalid_y_end) + 1;
        rtk_lcd_hal_transfer_done();
        rtk_lcd_hal_set_window(0, 0, LV_HOR_RES, flush_y_end);
        rtk_lcd_hal_start_transfer((uint8_t *)px_map, LV_HOR_RES * flush_y_end);
        last_invalid_y_end = invalid_y_end;
        invalid_y_end = 0;
    }

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}
#elif ACTIVE_DISPLAY_SCHEME == SCHEME_PSRAM_FULL
static void disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map)
{
    rtk_lcd_hal_transfer_done();
    rtk_lcd_hal_set_window(0, 0, LV_HOR_RES, LV_VER_RES);
    rtk_lcd_hal_start_transfer((uint8_t *)px_map, LV_HOR_RES * LV_VER_RES);

    /*IMPORTANT!!!
     *Inform the graphics library that you are ready with the flushing*/
    lv_display_flush_ready(disp_drv);
}
#endif

#if ACTIVE_DISPLAY_SCHEME == SCHEME_RAM_PSRAM_PARTIAL
void dma_copy_rect(const lv_area_t *area, uint8_t *color_p)
{
    uint32_t height = area->y2 - area->y1 + 1;
    uint32_t stride = (area->x2 - area->x1 + 1) * BYTE_PER_PIXEL;
    uint32_t buffer_size;
    uint8_t m_size = 0, data_size = 0;
    if (stride % 4 == 0)
    {
        data_size = GDMA_DataSize_Word;
        m_size = GDMA_Msize_16;
        buffer_size = stride / 4;
    }
    else if (stride % 2 == 0)
    {
        data_size = GDMA_DataSize_HalfWord;
        m_size = GDMA_Msize_32;
        buffer_size = stride / 2;
    }
    else
    {
        data_size = GDMA_DataSize_Byte;
        m_size = GDMA_Msize_64;
        buffer_size = stride;
    }


    GDMA_LLIStruct = os_mem_alloc(RAM_TYPE_DATA_ON, height * sizeof(GDMA_LLIDef));
    if (GDMA_LLIStruct == NULL)
    {
        assert_param(GDMA_LLIStruct != NULL);
    }
    else
    {
        memset(GDMA_LLIStruct, 0, height * sizeof(GDMA_LLIDef));
    }
    uint32_t start_address = (uint32_t)color_p;
    uint32_t dest_address = (uint32_t)(SINGLE_PSRAM_BUFFER + (LV_HOR_RES * area->y1 + area->x1) *
                                       BYTE_PER_PIXEL);
    uint32_t dest_stride = LV_HOR_RES * BYTE_PER_PIXEL;
    RCC_PeriphClockCmd(APBPeriph_GDMA, APBPeriph_GDMA_CLOCK, ENABLE);
    GDMA_ChannelTypeDef *dma_channel = DMA_CH_BASE(dma_channel_num);
    GDMA_InitTypeDef DISP_GDMA_InitStruct;
    /*--------------GDMA init-----------------------------*/
    GDMA_StructInit(&DISP_GDMA_InitStruct);
    DISP_GDMA_InitStruct.GDMA_ChannelNum          = dma_channel_num;
    DISP_GDMA_InitStruct.GDMA_BufferSize          = buffer_size;
    DISP_GDMA_InitStruct.GDMA_DIR                 = GDMA_DIR_MemoryToMemory;
    DISP_GDMA_InitStruct.GDMA_SourceInc           = DMA_SourceInc_Inc;
    DISP_GDMA_InitStruct.GDMA_DestinationInc      = DMA_DestinationInc_Inc;
    DISP_GDMA_InitStruct.GDMA_SourceMsize         =
        m_size;                         // 8 msize for source msize
    DISP_GDMA_InitStruct.GDMA_DestinationMsize    =
        m_size;                         // 8 msize for destiantion msize
    DISP_GDMA_InitStruct.GDMA_DestinationDataSize =
        data_size;                   // 32 bit width for destination transaction
    DISP_GDMA_InitStruct.GDMA_SourceDataSize      =
        data_size;                   // 32 bit width for source transaction
    DISP_GDMA_InitStruct.GDMA_SourceAddr          = (uint32_t)start_address;
    DISP_GDMA_InitStruct.GDMA_DestinationAddr     = (uint32_t)dest_address;

    DISP_GDMA_InitStruct.GDMA_Multi_Block_Mode = LLI_TRANSFER;
    DISP_GDMA_InitStruct.GDMA_Multi_Block_En = 1;
    DISP_GDMA_InitStruct.GDMA_Multi_Block_Struct = (uint32_t)GDMA_LLIStruct;

    GDMA_Init(dma_channel, &DISP_GDMA_InitStruct);
    if (GDMA_LLIStruct != NULL)
    {
        for (int i = 0; i < height; i++)
        {
            if (i == height - 1)
            {
                GDMA_LLIStruct[i].SAR = start_address + stride * i;
                GDMA_LLIStruct[i].DAR = (uint32_t)dest_address + dest_stride * i;
                GDMA_LLIStruct[i].LLP = 0;
                /* configure low 32 bit of CTL register */
                GDMA_LLIStruct[i].CTL_LOW = (BIT(0)
                                             | (DISP_GDMA_InitStruct.GDMA_DestinationDataSize << 1)
                                             | (data_size << 4)
                                             | (DISP_GDMA_InitStruct.GDMA_DestinationInc << 7)
                                             | (DISP_GDMA_InitStruct.GDMA_SourceInc << 9)
                                             | (DISP_GDMA_InitStruct.GDMA_DestinationMsize << 11)
                                             | (DISP_GDMA_InitStruct.GDMA_SourceMsize << 14)
                                             | (DISP_GDMA_InitStruct.GDMA_DIR << 20));
                /* configure high 32 bit of CTL register */
                GDMA_LLIStruct[i].CTL_HIGH = buffer_size;
            }
            else
            {
                GDMA_LLIStruct[i].SAR = start_address + stride * i;
                GDMA_LLIStruct[i].DAR = (uint32_t)dest_address + dest_stride * i;
                GDMA_LLIStruct[i].LLP = (uint32_t)&GDMA_LLIStruct[i + 1];
                /* configure low 32 bit of CTL register */
                GDMA_LLIStruct[i].CTL_LOW = dma_channel->CTL_LOW;
                /* configure high 32 bit of CTL register */
                GDMA_LLIStruct[i].CTL_HIGH = buffer_size;
            }
        }
    }
    GDMA_INTConfig(dma_channel_num, GDMA_INT_Transfer, ENABLE);
    GDMA_ClearINTPendingBit(dma_channel_num, GDMA_INT_Transfer);
    GDMA_Cmd(dma_channel_num, ENABLE);
}
void dma_wait_done(void)
{
    while (GDMA_GetTransferINTStatus(dma_channel_num) != SET);
    GDMA_ClearINTPendingBit(dma_channel_num, GDMA_INT_Transfer);
    if (GDMA_LLIStruct != NULL)
    {
        os_mem_free(GDMA_LLIStruct);
    }
}
#endif

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
