/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/**
 * @file    app_spi_sd_source.c
 * @brief   SD-card data source for the SPI(+TCP) uplink throughput test.
 *          See app_spi_sd_source.h for the design rationale.
 */

#include <zephyr/fs/fs.h>
#include "trace.h"
#include "app_fs_if.h"
#include "app_spi_sd_source.h"

/* The single open file handle held for the duration of one uplink test run. */
static struct fs_file_t s_sd_file;
static bool             s_sd_open = false;

bool app_spi_sd_source_is_open(void)
{
    return s_sd_open;
}

bool app_spi_sd_source_open(const char *path, uint32_t *out_size)
{
    int   res;
    off_t size;

    if (path == NULL)
    {
        return false;
    }
    if (s_sd_open)
    {
        /* a previous run left a file open - release it before reopening */
        app_spi_sd_source_close();
    }

    fs_file_t_init(&s_sd_file);
    res = fs_open(&s_sd_file, path, FS_O_READ);
    if (res < 0)
    {
        APP_PRINT_ERROR2("[sd_src] open '%s' fail: %d", TRACE_STRING(path), res);
        return false;
    }

    /* keep the SD powered for the whole read session (see wifi_sdio_tx_test.c) */
    app_fs_disk_power_down_disable(APP_DISK_CHECK_WIFI_TEST);
    s_sd_open = true;

    /* resolve the size by seeking to the end (fs_seek returns a status, not the
     * position, so read it back with fs_tell), then rewind to the start. */
    if (fs_seek(&s_sd_file, 0, FS_SEEK_END) < 0)
    {
        APP_PRINT_ERROR0("[sd_src] seek end fail");
        app_spi_sd_source_close();
        return false;
    }
    size = fs_tell(&s_sd_file);
    (void)fs_seek(&s_sd_file, 0, FS_SEEK_SET);

    if (size <= 0)
    {
        APP_PRINT_ERROR1("[sd_src] '%s' is empty", TRACE_STRING(path));
        app_spi_sd_source_close();
        return false;
    }

    if (out_size != NULL)
    {
        *out_size = (uint32_t)size;
    }
    APP_PRINT_INFO2("[sd_src] open '%s' ok, size=%u bytes",
                    TRACE_STRING(path), (uint32_t)size);
    return true;
}

int app_spi_sd_source_read(uint8_t *buf, uint32_t len)
{
    if (!s_sd_open || buf == NULL)
    {
        return -1;
    }
    return (int)fs_read(&s_sd_file, buf, len);
}

void app_spi_sd_source_close(void)
{
    if (!s_sd_open)
    {
        return;
    }
    fs_close(&s_sd_file);
    s_sd_open = false;
    app_fs_disk_power_down_enable(APP_DISK_CHECK_WIFI_TEST);
    APP_PRINT_INFO0("[sd_src] closed");
}
