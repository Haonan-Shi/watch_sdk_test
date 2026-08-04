/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Define to prevent recursive inclusion */
#ifndef _MODULE_FONT_H_
#define _MODULE_FONT_H_

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "rtl876x.h"

typedef struct
{
    uint16_t first_char;
    uint16_t last_char;
    uint32_t addr_offset;
} FONT_SECTION_INFO;

typedef struct
{
    uint32_t addr_offset : 26;
    uint32_t width : 6;
} FONT_CHAR_INDEX;


int get_utf8_byte_num(uint8_t firstCh);
uint32_t watch_utf8_to_unicode(uint8_t *utf8, uint32_t len, uint16_t *unicode_array,
                               uint32_t unicode_buf_len);
uint16_t watch_utf16_to_utf8(const uint16_t *utf16_str,
                             uint16_t utf16_len,
                             uint8_t *utf8_str,
                             uint16_t utf8_len);
uint16_t calculate_utf16_length(const char *utf8_str);



#ifdef  __cplusplus
}
#endif      /*  __cplusplus */


#endif
