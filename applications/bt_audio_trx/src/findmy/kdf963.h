/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "mbedtls/platform.h"
#include "stdint.h"

#include "string.h"

#define word32            uint32_t
#define byte              uint8_t
#define XMEMCPY(d,s,l)    memcpy((d),(s),(l))
#define XMEMSET(b,c,l)    memset((b),(c),(l))
#define XMEMCMP(s1,s2,n)  memcmp((s1),(s2),(n))


typedef enum
{
    MBED_KDF963_SHA128,
    MBED_KDF963_SHA256,
} MBED_KDF963_SHA_TYPE;

int mbed_KDF963(MBED_KDF963_SHA_TYPE type, uint8_t *secret, uint32_t secretSz, uint8_t *sinfo,
                uint32_t sinfoSz, uint8_t *out, uint32_t out_size);
