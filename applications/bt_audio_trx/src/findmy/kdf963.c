/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "kdf963.h"
#include "mbedtls/sha256.h"

#define CHECK_RET_GOTO(_ret_, _label_) if ((_ret_) != 0) goto _label_;

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
    int ret = 0;
    int i;
    int digestSz = 32, copySz;
    int remaining = out_size;
    uint8_t *outIdx;
    uint8_t counter[4] = {0};
    uint8_t mbed_temp_buf[32] = {0};

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
        CHECK_RET_GOTO(ret, cleanup);

        IncrementX963KdfCounter(counter);
        ret = mbedtls_sha256_update(&ctx, secret, secretSz);
        CHECK_RET_GOTO(ret, cleanup);

        ret = mbedtls_sha256_update(&ctx, counter, sizeof(counter));
        CHECK_RET_GOTO(ret, cleanup);

        if (sinfo)
        {
            ret = mbedtls_sha256_update(&ctx, sinfo, sinfoSz);
            CHECK_RET_GOTO(ret, cleanup);
        }

        ret = mbedtls_sha256_finish(&ctx, mbed_temp_buf);
        CHECK_RET_GOTO(ret, cleanup);

        copySz = min(remaining, digestSz);
        XMEMCPY(outIdx, mbed_temp_buf, copySz);

        remaining -= copySz;
        outIdx += copySz;
        mbedtls_sha256_free(&ctx);
    }

    return 0;

cleanup:
    mbedtls_sha256_free(&ctx);
    return ret;
}
