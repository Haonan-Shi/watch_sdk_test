/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __KDF963_H
#define __KDF963_H
#include "mbedtls/platform.h"
#include "stdint.h"

typedef enum
{
    MBED_KDF963_SHA128,
    MBED_KDF963_SHA256,
} MBED_KDF963_SHA_TYPE;

int mbed_KDF963(MBED_KDF963_SHA_TYPE type, uint8_t *secret, uint32_t secretSz, uint8_t *sinfo,
                uint32_t sinfoSz, uint8_t *out, uint32_t out_size);
#endif
