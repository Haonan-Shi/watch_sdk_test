/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include <string.h>
#include "trace.h"
#include "os_timer.h"
#include "rgb_led.h"
#include "audio_led.h"

typedef struct _AUDIO_LED_T
{
    void (*audio_led_handler)(void);                        /*!< Specifies audio led handler. */
    T_AUDIO_LED_COLOR_MODE
    color_mode;                      /*!< Specifies rgb change ligher or darker. */
} AUDIO_LED_T;


#define RGB_DATA_LEN                (24)                            //R,G,B total 24 bits
#define RGB_DATA_NUM                (241)                           //total RGB number
#define DATA_LEN                    (RGB_DATA_LEN * RGB_DATA_NUM)   //total RGB data buf length
#define LAP_NUM                     (9)                             //RGB lap number
#define DIV_NUM                      8                              //RGB div number

#define RGB_MAX_VALUE               0xFF
#define RGB_HALF_VALUE              0x80
#define RGB_MIN_VALUE               0x0
#define RGB_CHANGE_LIGHTER_STEP     0x9
#define RGB_CHANGE_DARKER_STEP      0x1C
#define IS_RGB_OVERFLOW(value)      (value = (value > RGB_MAX_VALUE) ? RGB_MAX_VALUE : value)

#define RGB_FIX_PATTERN_DISPLAY_LAP_NUM     4                       //Display lap number when fix pattern
#define AUDIO_GAIN_WEIGHT_NUMBER            5

#define AUDIO_LED_DEBUG                     1

const static uint8_t display_div_8_addr[8] = {0, 4, 6, 2, 1, 5, 7, 3};           //Display order
const static T_AUDIO_LED_COLOR display_div_8_color[8] = {LED_COLOR_RED, LED_COLOR_GREEN,
                                                         LED_COLOR_BLUE, LED_COLOR_PURPLE,
                                                         LED_COLOR_YELLOW, LED_COLOR_LIGHT_BLUE,
                                                         LED_COLOR_ORANGE, LED_COLOR_YELLOW
                                                        };        //Display color
const static uint32_t display_with_gain_color[LAP_NUM] = {0xff00, 0xdb00, 0xb700, 0x9300, 0x6f00, 0x4b00, 0x2700, 0x300, 0x100};
const static uint8_t lap[LAP_NUM] = {60, 48, 40, 32, 24, 16, 12, 8, 1}; //number of LEDs per circle

static uint32_t *rgb_data_buf = NULL;
static int16_t audio_intensity_min = 0x7FFF;                      //Record audio intensity min
static int16_t audio_intensity_max = 0x8000;                      //Record audio intensity max
static void *audio_led_timer = NULL;
static AUDIO_LED_T audio_led = {0};
uint32_t freq_table[FREQ_NUM] = {1000, 2000, 3000, 4000, 5000};     //Audio sampling frequency

static uint32_t calculate_rgb_value(T_AUDIO_LED_COLOR color, uint8_t step,
                                    T_AUDIO_LED_COLOR_MODE mode)
{
    uint32_t data = 0;
    uint32_t i = 0;
    uint32_t r = 0, g = 0, b = 0;

    if (color == LED_COLOR_RED)
    {
        if (mode == LED_CHANGE_LIGHTER)
        {
            r = RGB_MAX_VALUE;
            g = RGB_CHANGE_LIGHTER_STEP * step;
            b = RGB_CHANGE_LIGHTER_STEP * step;
        }
        else if (mode == LED_CHANGE_DARKER)
        {
            if ((RGB_CHANGE_DARKER_STEP * step) > RGB_MAX_VALUE)
            {
                r = RGB_MIN_VALUE;
            }
            else
            {
                r = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
            }
            g = RGB_MIN_VALUE;
            b = RGB_MIN_VALUE;
        }
    }
    else if (color == LED_COLOR_GREEN)
    {
        if (mode == LED_CHANGE_LIGHTER)
        {
            r = RGB_CHANGE_LIGHTER_STEP * step;
            g = RGB_MAX_VALUE;
            b = RGB_CHANGE_LIGHTER_STEP * step;
        }
        else if (mode == LED_CHANGE_DARKER)
        {
            if ((RGB_CHANGE_DARKER_STEP * step) > RGB_MAX_VALUE)
            {
                g = RGB_MIN_VALUE;
            }
            else
            {
                g = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
            }
            r = RGB_MIN_VALUE;
            b = RGB_MIN_VALUE;
        }
    }
    else if (color == LED_COLOR_BLUE)
    {
        if (mode == LED_CHANGE_LIGHTER)
        {
            r = RGB_CHANGE_LIGHTER_STEP * step;
            g = RGB_CHANGE_LIGHTER_STEP * step;
            b = RGB_MAX_VALUE;
        }
        else if (mode == LED_CHANGE_DARKER)
        {
            if ((RGB_CHANGE_DARKER_STEP * step) > RGB_MAX_VALUE)
            {
                b = RGB_MIN_VALUE;
            }
            else
            {
                b = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
            }
            r = RGB_MIN_VALUE;
            g = RGB_MIN_VALUE;
        }
    }
    else if (color == LED_COLOR_PURPLE)
    {
        if (mode == LED_CHANGE_LIGHTER)
        {
            r = RGB_MAX_VALUE;
            g = RGB_CHANGE_LIGHTER_STEP * step;
            b = RGB_MAX_VALUE;
        }
        else if (mode == LED_CHANGE_DARKER)
        {
            if ((RGB_CHANGE_DARKER_STEP * step) > RGB_MAX_VALUE)
            {
                r = RGB_MIN_VALUE;
                b = RGB_MIN_VALUE;
            }
            else
            {
                r = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
                b = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
            }
            g = RGB_MIN_VALUE;
        }

    }
    else if (color == LED_COLOR_YELLOW)
    {
        if (mode == LED_CHANGE_LIGHTER)
        {
            r = RGB_MAX_VALUE;
            g = RGB_MAX_VALUE;
            b = RGB_CHANGE_LIGHTER_STEP * step;
        }
        else if (mode == LED_CHANGE_DARKER)
        {
            if ((RGB_CHANGE_DARKER_STEP * step) > RGB_MAX_VALUE)
            {
                r = RGB_MIN_VALUE;
                g = RGB_MIN_VALUE;
            }
            else
            {
                r = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
                g = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
            }
            b = RGB_MIN_VALUE;
        }

    }
    else if (color == LED_COLOR_LIGHT_BLUE)
    {
        if (mode == LED_CHANGE_LIGHTER)
        {
            r = RGB_CHANGE_LIGHTER_STEP * step;
            g = RGB_MAX_VALUE;
            b = RGB_MAX_VALUE;
        }
        else if (mode == LED_CHANGE_DARKER)
        {
            if ((RGB_CHANGE_DARKER_STEP * step) > RGB_MAX_VALUE)
            {
                b = RGB_MIN_VALUE;
                g = RGB_MIN_VALUE;
            }
            else
            {
                b = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
                g = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
            }
            r = RGB_MIN_VALUE;
        }
    }
    else if (color == LED_COLOR_ORANGE)
    {
        if (mode == LED_CHANGE_LIGHTER)
        {
            r = RGB_MAX_VALUE;
            g = RGB_HALF_VALUE + (RGB_CHANGE_LIGHTER_STEP * step / 2);
            b = RGB_CHANGE_LIGHTER_STEP * step;
        }
        else if (mode == LED_CHANGE_DARKER)
        {
            if ((RGB_CHANGE_DARKER_STEP * step) > RGB_MAX_VALUE)
            {
                r = RGB_MIN_VALUE;
            }
            else
            {
                r = RGB_MAX_VALUE - (RGB_CHANGE_DARKER_STEP * step);
            }
            if ((RGB_CHANGE_DARKER_STEP * step / 2) > RGB_HALF_VALUE)
            {
                g = RGB_MIN_VALUE;
            }
            else
            {
                g = RGB_HALF_VALUE - (14 * step);
            }
            b = RGB_MIN_VALUE;
        }
    }

    IS_RGB_OVERFLOW(r);
    IS_RGB_OVERFLOW(g);
    IS_RGB_OVERFLOW(b);

    data = ((g << 16) | (r << 8) | (b));
#if (AUDIO_LED_DEBUG == 1 )
    IO_PRINT_INFO5("calculate_rgb_value data:0x%x r:0x%x g:0x%x b:0x%x step %d", data, r, g, b, step);
#endif
    return data;
}

static void audio_led_fix_pattern(void)
{
    static uint8_t timer_cnt = 0;
    uint8_t rgb_index = 0;
    uint8_t record_start = 0;
    uint8_t lap_index = 0;
    uint8_t display_index = 0;
    uint8_t cnt_diff = 0;
    uint8_t step = 0;

    display_index = timer_cnt % DIV_NUM;
    timer_cnt++;

    memset(rgb_data_buf, 0, RGB_DATA_NUM * 4);

    rgb_index = 0;
    record_start = rgb_index;

    while (rgb_index <= RGB_DATA_NUM)
    {
        if (lap_index < RGB_FIX_PATTERN_DISPLAY_LAP_NUM)
        {
            cnt_diff = rgb_index - record_start + 1;
            if (display_div_8_addr[display_index] == 0)
            {
                if (cnt_diff <= (lap[lap_index] / 2))
                {
                    step = cnt_diff - 1;
                }
                else
                {
                    step = lap[lap_index] - cnt_diff + 1;
                }
            }
            else if (display_div_8_addr[display_index] < 4)
            {
                if (cnt_diff <= (lap[lap_index] * display_div_8_addr[display_index] / 8))
                {
                    step = lap[lap_index] * display_div_8_addr[display_index] / 8 - cnt_diff;
                }
                else if (cnt_diff <= (lap[lap_index] * (display_div_8_addr[display_index] + 4) / 8))
                {
                    step = cnt_diff - (lap[lap_index] * display_div_8_addr[display_index] / 8);
                }
                else
                {
                    step = lap[lap_index] - cnt_diff + (lap[lap_index] * display_div_8_addr[display_index] / 8);
                }
            }
            else if (display_div_8_addr[display_index] == 4)
            {
                if (cnt_diff <= (lap[lap_index] / 2))
                {
                    step = lap[lap_index] / 2 - cnt_diff;
                }
                else
                {
                    step = cnt_diff - (lap[lap_index] / 2);
                }
            }
            else if (display_div_8_addr[display_index] < 8)
            {
                if (cnt_diff <= (lap[lap_index] * (display_div_8_addr[display_index] - 4) / 8))
                {
                    step = (lap[lap_index] * (8 - display_div_8_addr[display_index]) / 8) + cnt_diff;
                }
                else if (cnt_diff <= (lap[lap_index] * display_div_8_addr[display_index] / 8))
                {
                    step = (lap[lap_index] * display_div_8_addr[display_index] / 8) - cnt_diff;
                }
                else
                {
                    step = cnt_diff - (lap[lap_index] * display_div_8_addr[display_index] / 8);
                }
            }
            rgb_data_buf[rgb_index] = calculate_rgb_value(display_div_8_color[display_index], step,
                                                          audio_led.color_mode);
#if (AUDIO_LED_DEBUG == 1 )
            IO_PRINT_INFO2("audio_led_fix_pattern: rgb_index:%d data:0x%x", rgb_index,
                           rgb_data_buf[rgb_index]);
#endif
            if (cnt_diff == lap[lap_index])
            {
                rgb_index = record_start + lap[lap_index];
                record_start = rgb_index;
                lap_index++;
                continue;
            }
        }
        rgb_index++;
    }

    rgb_led_data_send(rgb_data_buf, RGB_DATA_NUM);
}

static void audio_led_timer_handler(void *xTimer)
{
    if (audio_led.audio_led_handler != NULL)
    {
        audio_led.audio_led_handler();
    }
}

/**
    * @brief  Clear the recorded audio gain. It is recommended to call when cutting songs or re-playing music.
    * @return none
    */
void audio_gain_clear(void)
{
    audio_intensity_min = 0x7FFF;
    audio_intensity_max = 0x8000;
}

/**
    * @brief  Audio gain transfer to RGB
    * @param  freq_num: Number of samping frequencies
    * @param  left_gain: Left gain
    * @param  right_gain: Right gain
    * @return none
    */
void audio_gain_transfer_to_rgb(uint32_t freq_num, uint16_t *left_gain, uint16_t *right_gain)
{
    uint8_t rgb_index = 0;
    uint8_t record_start = 0;
    uint8_t lap_index = 0;
    uint8_t cnt_diff = 0;
    uint16_t audio_intensity_diff = 0;
    uint16_t audio_intensity_step = 0;
    int16_t audio_intensity = 0;
    uint16_t audio_intensity_lap_num = 0;

    for (int i = 0; i < freq_num; i++)
    {
#if (AUDIO_LED_DEBUG == 1 )
        IO_PRINT_INFO3("audio_gain_transfer_to_rgb: freq:%d, left_gain 0x%0x, right_gain 0x%0x",
                       freq_table[i], left_gain[i], right_gain[i]);
#endif
        audio_intensity += ((int16_t)left_gain[i]) / AUDIO_GAIN_WEIGHT_NUMBER;
    }

    if (audio_intensity <= audio_intensity_min)
    {
        audio_intensity_min = audio_intensity;
    }

    if (audio_intensity >= audio_intensity_max)
    {
        audio_intensity_max = audio_intensity;
    }

    audio_intensity_diff = (audio_intensity_max - audio_intensity_min) / LAP_NUM;

    audio_intensity_step = (audio_intensity - audio_intensity_min) / audio_intensity_diff;

#if (AUDIO_LED_DEBUG == 1 )
    IO_PRINT_INFO4("audio_gain_transfer_to_rgb: audio_intensity:%d audio_intensity_min:%d audio_intensity_max:%d audio_intensity_step:%d",
                   audio_intensity, audio_intensity_min, audio_intensity_max, audio_intensity_step);
#endif

    memset(rgb_data_buf, 0, RGB_DATA_NUM * 4);

    if ((LAP_NUM - 1) <= audio_intensity_step)
    {
        rgb_index = 0;
    }
    else
    {
        audio_intensity_lap_num = LAP_NUM - audio_intensity_step - 1;
        for (uint8_t i = 0; i < audio_intensity_lap_num; i++)
        {
            rgb_index += lap[i];
        }
        record_start = rgb_index;
        lap_index = audio_intensity_lap_num;
    }

#if (AUDIO_LED_DEBUG == 1 )
    IO_PRINT_INFO2("audio_gain_transfer_to_rgb: rgb_index:%d lap_index:%d", rgb_index, lap_index);
#endif

    while (rgb_index <= RGB_DATA_NUM)
    {
        if (lap_index < LAP_NUM)
        {
            cnt_diff = rgb_index - record_start + 1;
            rgb_data_buf[rgb_index] = display_with_gain_color[lap_index];
#if (AUDIO_LED_DEBUG == 1 )
            //IO_PRINT_INFO2("audio_gain_transfer_to_rgb: rgb_index:%d data:0x%x", rgb_index, rgb_data_buf[rgb_index]);
#endif
            if (cnt_diff == lap[lap_index])
            {
                rgb_index = record_start + lap[lap_index];
                record_start = rgb_index;
                lap_index++;
                continue;
            }
        }
        rgb_index++;
    }

    rgb_led_data_send(rgb_data_buf, RGB_DATA_NUM);
}
/**
    * @brief  Init rgb display
    * @param  display_mode: @ref T_AUDIO_LED_DISPLAY_MODE
    * @param  color_mode: @ref T_AUDIO_LED_COLOR_MODE
    * @param  fixed_pattern_changed_t_ms: This parameter takes effect only when a fixed pattern mode is used
    * @return none
    */
void audio_led_init(T_AUDIO_LED_DISPLAY_MODE display_mode, T_AUDIO_LED_COLOR_MODE color_mode,
                    uint32_t fixed_pattern_changed_t_ms)
{
    IO_PRINT_INFO0("audio_led_init");

    if (rgb_data_buf == NULL)
    {
        rgb_data_buf = (uint32_t *)malloc(RGB_DATA_NUM * 4);
        if (rgb_data_buf == NULL)
        {
            IO_PRINT_ERROR0("audio_led_init: rgb_data_buf malloc fail!");
            return ;
        }
    }

    rgb_led_init(RGB_DATA_NUM, NULL);

    if (display_mode == AUDIO_LED_DISPLAY_FIX_PATTERN)
    {
        audio_led.color_mode = color_mode;
        audio_led.audio_led_handler = audio_led_fix_pattern;

        os_timer_create(&audio_led_timer, "AUDIO_LED_TIMER", 2, fixed_pattern_changed_t_ms, true,
                        audio_led_timer_handler);

        if (!os_timer_start(&audio_led_timer))
        {
            IO_PRINT_ERROR0("audio_led_init: AUDIO_LED_TIMER Start Fail");
        }
    }
}
