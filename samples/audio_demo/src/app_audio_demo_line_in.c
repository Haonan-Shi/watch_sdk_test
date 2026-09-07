/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdbool.h>
#include "trace.h"
#include "audio.h"
#include "audio_type.h"
#include "audio_line.h"
#include "app_audio_demo_line_in.h"

static T_AUDIO_LINE_HANDLE line_handle;

bool app_audio_line_start(uint32_t device)
{
    line_handle = audio_line_create(device,
                                    48000,
                                    48000);

    if (line_handle != NULL)
    {
        return audio_line_start(line_handle);
    }

    return false;
}

bool app_audio_line_stop(void)
{
    if (line_handle != NULL)
    {
        return audio_line_release(line_handle);
    }

    return false;
}
