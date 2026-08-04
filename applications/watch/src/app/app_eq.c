/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_eq.h"
#include <string.h>
#include <stdlib.h>
#include "trace.h"
#include "eq_utils.h"
#include "eq.h"
#include "app_link_util.h"
#include "app_main.h"
#include "fmc_api.h"
#include "app_dsp_cfg.h"
#include "app_cfg.h"

uint8_t app_eq_sample_rate_get(void)
{
    uint8_t sample_rate = 0;

    if (app_db.sampling_frequency == SAMPLE_RATE_44K)
    {
        sample_rate = AUDIO_EQ_SAMPLE_RATE_44_1KHZ;
    }
    else if (app_db.sampling_frequency == SAMPLE_RATE_48K)
    {
        sample_rate = AUDIO_EQ_SAMPLE_RATE_48KHZ;
    }

    APP_PRINT_TRACE1("app_eq_sample_rate_get: sample_rate %d", sample_rate);

    return sample_rate;
}

static uint16_t app_eq_dsp_param_get(T_EQ_TYPE eq_type,
                                     uint8_t eq_mode, uint8_t index, void *data, uint16_t len, T_EQ_DATA_DEST eq_data_dest,
                                     uint32_t sample_rate)
{
    uint16_t eq_len = 0;

    APP_PRINT_INFO5("app_eq_dsp_param_get: eq_type %d, dsp cfg index %d, data %p, len %u, sample_rate %d",
                    eq_type, index, data, len, sample_rate);

//20240527    eq_len = eq_utils_param_get(eq_type, eq_mode, index, data, len, eq_data_dest,
//                                sample_rate);

    return eq_len;
}

void app_eq_audio_eq_enable(T_AUDIO_EFFECT_INSTANCE *eq_instance, bool *audio_eq_enabled)
{
    if (eq_instance && audio_eq_enabled)
    {
        if (!(*audio_eq_enabled))
        {
            eq_enable(*eq_instance);
            *audio_eq_enabled = true;
        }
    }
}

T_AUDIO_EFFECT_INSTANCE app_eq_create(T_EQ_CONTENT_TYPE eq_content_type,
                                      T_EQ_TYPE eq_type, uint8_t eq_mode, uint8_t eq_index)
{
    T_AUDIO_EFFECT_INSTANCE eq_instance = NULL;
    uint8_t *dynamic_eq_buf = calloc(1, app_db.max_eq_len);

    if (dynamic_eq_buf != NULL)
    {
        uint16_t eq_len = app_eq_dsp_param_get(eq_type, eq_mode, eq_index, dynamic_eq_buf,
                                               app_db.max_eq_len, EQ_DATA_TO_DSP, app_eq_sample_rate_get());

        eq_instance = eq_create(eq_content_type, dynamic_eq_buf, eq_len);
        free(dynamic_eq_buf);
    }
    else
    {
        APP_PRINT_ERROR0("app_eq_create: fail");
    }

    return eq_instance;
}

bool app_eq_index_set(T_EQ_TYPE eq_type, uint8_t mode, uint8_t index)
{
    uint8_t eq_num;
    uint16_t eq_len;
    uint8_t temp_sample_rate;
    bool ret = false;
    uint8_t *dynamic_eq_buf = calloc(1, app_db.max_eq_len);

    temp_sample_rate = app_eq_sample_rate_get();

    if (dynamic_eq_buf != NULL)
    {
        eq_num = eq_utils_num_get(eq_type, mode);

        APP_PRINT_INFO4("app_eq_index_set: eq_type %d, mode %d, eq_num %d, index 0x%02x",
                        eq_type, mode, eq_num, index);

        if (eq_num == 0)
        {
            //when len < 0x10, has eq off effect
            eq_len = 0x05;
        }
        else
        {
            eq_len = app_eq_dsp_param_get(eq_type, mode, index, dynamic_eq_buf,
                                          app_db.max_eq_len, EQ_DATA_TO_DSP, temp_sample_rate);
        }

        if (eq_type == SPK_SW_EQ)
        {
            eq_set(app_db.eq_instance, dynamic_eq_buf, eq_len);
            app_cfg_nv.eq_idx = index;
            ret = true;
        }

        free(dynamic_eq_buf);
    }
    else
    {
        APP_PRINT_ERROR0("app_eq_index_set: fail");
    }

    return ret;
}

void app_eq_init(void)
{
//    uint16_t len = 0;
    app_db.max_eq_len = 0;

    if (eq_utils_init())
    {
//20240527        len = EQ_GROUP_NUM * (STAGE_NUM_SIZE + EXT_STAGE_NUM_SIZE_VER_2) + MCU_TO_SDK_CMD_HDR_SIZE +
//              PUBLIC_VALUE_SIZE + EXT_PUB_VALUE_SIZE;//262

        app_db.max_eq_len = 262;//len;

    }

    app_db.sampling_frequency = SAMPLE_RATE_44K;
}




