/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "trace.h"
#include "string.h"
#include "app_cmd.h"
#include "app_report.h"
#include "app_fs_test.h"
#include "app_fs_if.h"

T_FILE_HANDLE *g_app_fs_test_handle = NULL;

static bool report_files_callback(const char *name, uint32_t size, void *context)
{
    (void)context;

    size_t full_len = strlen(name);

    if (full_len > (FF_MAX_LFN - 1))
    {
        full_len = FF_MAX_LFN - 1;
    }

    uint8_t rpt[FF_MAX_LFN];

    rpt[0] = (uint8_t)full_len;
    memcpy(&rpt[1], name, full_len);

    app_report_event(CMD_PATH_UART, EVENT_FS_FILE_INFO, 0, rpt, full_len + 1);

    return true;
}

void app_fs_test_handle_cmd_set(uint8_t app_idx, uint8_t cmd_path, uint8_t *cmd_ptr,
                                uint16_t cmd_len, uint8_t *ack_pkt)
{
    uint16_t cmd_id = (uint16_t)(cmd_ptr[0] | (cmd_ptr[1] << 8));

    APP_PRINT_TRACE3("app_fs_test_handle_cmd_set: cmd_id 0x%04x, cmd_len 0x%04x, cmd_path %u",
                     cmd_id, cmd_len, cmd_path);


    switch (cmd_id)
    {
    case CMD_FS_LIST_FILES:
        {
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
            app_fs_if_list_files(NULL, report_files_callback, NULL);
        }
        break;
    case CMD_FS_CREATE_FILE:
        {
            g_app_fs_test_handle = app_fs_open_file("fs_test.bin", FS_O_CREATE | FS_O_WRITE);
            APP_PRINT_INFO1("app_fs_test_handle_cmd_set: file opened handle 0x%x", g_app_fs_test_handle);
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;
    case CMD_FS_WRITE_FILE:
        {
            uint8_t data[256] = {0};
            for (size_t i = 0; i < sizeof(data); i++)
            {
                data[i] = i;
            }
            int result = app_fs_write(g_app_fs_test_handle, data, sizeof(data));
            APP_PRINT_INFO2("app_fs_test_handle_cmd_set: file write handle 0x%x with %d", g_app_fs_test_handle,
                            result);
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);

        }
        break;
    case CMD_FS_CLOSE_FILE:
        {
            int result = app_fs_close_file(g_app_fs_test_handle);
            APP_PRINT_INFO2("app_fs_test_handle_cmd_set: file opened handle 0x%x with %d", g_app_fs_test_handle,
                            result);
            app_cmd_set_event_ack(cmd_path, app_idx, ack_pkt);
        }
        break;

    default:
        break;
    }
}
