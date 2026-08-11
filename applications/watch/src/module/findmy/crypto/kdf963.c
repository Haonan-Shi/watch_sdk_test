/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "fm-crypto.h"
#include "kdf963.h"
#include "mbedtls/sha256.h"
#include "mbedtls/platform_util.h"
#include "string.h"

static __inline void IncrementX963KdfCounter(uint8_t *inOutCtr)
{
    int i;

    /* in network byte order so start at end and work back */
    for (i = 3; i >= 0; i--)
    {
        if (++inOutCtr[i])  /* we're done unless we overflow */
        {
            return;
        }
    }
}

static __inline word32 min(int32_t a, int32_t b)
{
    return a > b ? b : a;
}

int mbed_KDF963(MBED_KDF963_SHA_TYPE type, uint8_t *secret, uint32_t secretSz, uint8_t *sinfo,
                uint32_t sinfoSz, uint8_t *out, uint32_t out_size)
{
    int ret, i;
    int digestSz = 32, copySz;
    int remaining = out_size;
    uint8_t *outIdx;
    uint8_t  counter[4];
    uint8_t mbed_temp_buf[32];

    if (secret == NULL || secretSz == 0 || out == NULL)
    {
        return -1;
    }

    outIdx = out;
    XMEMSET(counter, 0, sizeof(counter));

    mbedtls_sha256_context ctx;

    for (i = 1; remaining > 0; i++)
    {
        mbedtls_sha256_init(&ctx);
        ret = mbedtls_sha256_starts(&ctx, 0);
        IncrementX963KdfCounter(counter);
        mbedtls_sha256_update(&ctx, secret, secretSz);
        if (ret != 0)
        {
            break;
        }

        ret = mbedtls_sha256_update(&ctx, counter, sizeof(counter));
        if (ret != 0)
        {
            break;
        }

        if (sinfo)
        {
            ret = mbedtls_sha256_update(&ctx, sinfo, sinfoSz);
            if (ret != 0)
            {
                break;
            }
        }

        ret = mbedtls_sha256_finish(&ctx, mbed_temp_buf);
        if (ret != 0)
        {
            break;
        }

        copySz = min(remaining, digestSz);
        XMEMCPY(outIdx, mbed_temp_buf, copySz);

        remaining -= copySz;
        outIdx += copySz;
        mbedtls_sha256_free(&ctx);
    }

    return ret;
}
