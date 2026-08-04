/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __ALIPAY_MEM_H__
#define __ALIPAY_MEM_H__

#include "alipay_common.h"
#include "vendor_api.h"
EXTERNC uint16_t read16(const uint8_t *buffer);

EXTERNC uint32_t read32(const uint8_t *buffer);

EXTERNC void write16(uint8_t *buffer, uint16_t value);

EXTERNC void write32(uint8_t *buffer, uint32_t value);

EXTERNC bool is_little_endian(void) ;

void csi_heap_init(void);
extern void *csi_realloc(void *mem, size_t size);
size_t csi_getUsedSize(void *pt);
void *csi_malloc(uint32_t size);
void *csi_calloc(uint32_t nblock, uint32_t size);
void csi_free(void *pt);

#endif
