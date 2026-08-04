/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "module_font.h"

/**
 * @brief  convert utf8 string to unicode
 * @param  utf8: utf8 string buf
 * @param  len: length of utf8 string
 * @param  unicode_array: unicode output buf
 * @param  unicode_buf_len: unicode output buf length
 * @retval converted unicode count
 */
uint32_t watch_utf8_to_unicode(uint8_t *utf8, uint32_t len, uint16_t *unicode_array,
                               uint32_t unicode_buf_len)
{
    int k = 0;

    for (uint32_t i = 0; i < len; i++)
    {
        if (k >= unicode_buf_len)
        {
            return k;
        }
        switch (get_utf8_byte_num((uint8_t)utf8[i]))
        {
        case 0:
            {
                unicode_array[k] = *(utf8 + i);
                k++;
                break;
            }
        case 1:
            {
                //TODO
                //can not enter here
                //k++;
                break;
            }
        case 2:
            {

                //unicodeArray[k] = utf8[i]>>8;
                unicode_array[k] = utf8[i + 1];
                k++;
                i = i + 1;
                break;
            }
        case 3:
            {
                unicode_array[k] = ((utf8[i + 1] & 0x03) << 6) + (utf8[i + 2] & 0x3F);
                unicode_array[k] |= (uint16_t)(((utf8[i] & 0x0F) << 4) | ((utf8[i + 1] >> 2) & 0x0F)) << 8;
                k++;
                i = i + 2;
                break;
            }
        case 4:
            {
                //TODO
                k++;
                i = i + 3;
                break;
            }
        case 5:
            {
                //TODO
                k++;
                i = i + 4;
                break;
            }
        case 6:
            {
                //TODO
                k++;
                i = i + 5;
                break;
            }
        default:
            //DBG_DIRECT("the len is more than 6\n");
            break;
        }
    }
    return k;
}


/**
 * @brief  check the utf8 length for one character
 * @param  firstCh: first byte of utf8 string
 * @retval length
 */
int get_utf8_byte_num(uint8_t firstCh)
{
    uint8_t temp = 0x80;
    int num = 0;

    while (temp & firstCh)
    {
        num++;
        temp = (temp >> 1);
    }
    return num;
}

// Function to convert UTF-16 to UTF-8
uint16_t watch_utf16_to_utf8(const uint16_t *utf16_str, uint16_t utf16_len, uint8_t *utf8_str,
                             uint16_t utf8_len)
{
    uint16_t utf8_index = 0;
    for (uint16_t i = 0; i < utf16_len && utf8_index < utf8_len; i++)
    {
        uint16_t utf16_char = utf16_str[i];

        if (utf16_char <= 0x7F)
        {
            // 1-byte UTF-8
            utf8_str[utf8_index++] = (uint8_t)utf16_char;
        }
        else if (utf16_char <= 0x7FF)
        {
            // 2-byte UTF-8
            if (utf8_index + 1 >= utf8_len) { break; } // prevent overflow
            utf8_str[utf8_index++] = 0xC0 | (utf16_char >> 6);
            utf8_str[utf8_index++] = 0x80 | (utf16_char & 0x3F);
        }
        else
        {
            // 3-byte UTF-8
            if (utf8_index + 2 >= utf8_len) { break; } // prevent overflow
            utf8_str[utf8_index++] = 0xE0 | (utf16_char >> 12);
            utf8_str[utf8_index++] = 0x80 | ((utf16_char >> 6) & 0x3F);
            utf8_str[utf8_index++] = 0x80 | (utf16_char & 0x3F);
        }
    }
    return utf8_index; // Return the length of the UTF-8 string written
}

/**
 * @brief  calculate utf16 length of a utf8 string
 * @param  utf8_str: a utf8 string
 * @retval utf16 length
 */

uint16_t calculate_utf16_length(const char *utf8_str)
{
    uint16_t utf16_byte_length = 0;
    const unsigned char *ptr = (const unsigned char *)utf8_str;

    while (*ptr)
    {
        unsigned char c = *ptr;

        if ((c & 0x80) == 0)
        {
            // Single-byte ASCII character
            ptr += 1;
        }
        else if ((c & 0xE0) == 0xC0)
        {
            // Two-byte UTF-8 character
            ptr += 2;
        }
        else if ((c & 0xF0) == 0xE0)
        {
            // Three-byte UTF-8 character
            ptr += 3;
        }
        else
        {
            // Skip to next byte if an invalid or non-BMP character is encountered (should not occur)
            ptr += 1;
        }

        utf16_byte_length += 2; // Each character corresponds to one UTF-16 unit
    }

    // '\0'
    utf16_byte_length += 2;

    return utf16_byte_length;
}


