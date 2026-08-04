/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <zephyr/shell/shell.h>
#include "os_mem.h"
#include "os_msg.h"
#include "bt_types.h"
#include "app_shell_cmd.h"
#include "trace.h"

static void *shell_evt_queue_handle = NULL;
static void *shell_io_queue_handle = NULL;
const T_USER_CMD_TABLE_ENTRY *p_shell_cmd_table;


static void cmd_send_result(T_USER_CMD_PARSE_RESULT result)
{
    switch (result)
    {
    case RESULT_ERR:
        printk("CMD:Error\r\n");
        break;
    case RESULT_CMD_NOT_FOUND:
        printk("CMD:Command not found\r\n");
        break;
    case RESULT_CMD_ERR_PARAM_NUM:
        printk("CMD:Wrong number of parameters\r\n");
        break;
    case RESULT_CMD_ERR_PARAM:
        printk("CMD:Wrong parameter\r\n");
        break;
    case RESULT_CMD_OUT_OF_RANGE:
        printk("CMD:Value out of range\r\n");
        break;
    case RESULT_CMD_NOT_SUPPORT:
        printk("CMD:Not support\r\n");
        break;
    default:
        return;
    }
}

void app_shell_print(char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}

// help cmd, to list all of the user command
static T_USER_CMD_PARSE_RESULT user_cmd_list(const T_USER_CMD_TABLE_ENTRY *p_cmd_table)
{
    int32_t i = 0;
    T_USER_CMD_PARSE_RESULT result = RESULT_CMD_NOT_FOUND;

    /* find command in table */
    while ((p_cmd_table + i)->p_cmd != NULL)
    {
        printk("cmd: %s\r\n", (p_cmd_table + i)->p_option);
        printk("%s\r\n", (p_cmd_table + i)->p_help);

        result = RESULT_SUCCESS;
        i++;
    };

    return result;
}

bool app_shell_cmd_send_msg(void *param_buf)
{
    T_IO_MSG msg = {0};
    T_EVENT_TYPE  event;
    bool ret = false;

    msg.type    = IO_MSG_TYPE_UART;
    msg.subtype = IO_MSG_UART_RX;
    msg.u.buf   = param_buf;
    event = EVENT_IO_TO_APP;

    if (os_msg_send(shell_io_queue_handle, &msg, 0) == true)
    {
        ret = os_msg_send(shell_evt_queue_handle, &event, 0);
    }

    return ret;
}

static void cmd_shell_le(const struct shell *shell, size_t argc, char **argv)
{
    char       *subcmd;
    uint32_t    param_len;
    void       *param_buf;
    uint32_t    param_buf_len = 0;


    param_len = strlen(argv[1]);
    subcmd = (char *)argv[1];
    subcmd[param_len] = '\0';

    param_buf_len = param_len + 1 + sizeof(uint32_t) * 2 + sizeof(uint32_t) * (argc - 2);

    //APP_PRINT_INFO3("cmd_shell_le: subcmd %s, param_num %d, param_buf_len %d", TRACE_STRING(subcmd),
    //                argc, param_buf_len);

    param_buf = os_mem_alloc(OS_MEM_TYPE_DATA, param_buf_len);
    if (param_buf != NULL)
    {
        uint8_t    *p;

        p = param_buf;
        LE_UINT32_TO_STREAM(p, (param_len + 1));
        ARRAY_TO_STREAM(p, subcmd, (param_len + 1));
        LE_UINT32_TO_STREAM(p, (argc - 2));

        for (uint8_t i = 2; i < argc; i++)
        {
            uint32_t temp_data = strtoumax(argv[i], NULL, 0);
            LE_UINT32_TO_STREAM(p, temp_data);
            APP_PRINT_INFO3("console_le: data[%d] %d, 0x%x", i, temp_data, temp_data);
        }

        if (app_shell_cmd_send_msg(param_buf) == false)
        {
            os_mem_free(param_buf);
            goto err;
        }
    }

    return;
err:
    printk("Invalid param %s.\r\n", argv[0]);
    return;
}

void app_shell_cmd_register(void *evt_queue_handle, void *io_queue_handle,
                            const T_USER_CMD_TABLE_ENTRY *p_cmd_table)
{
    shell_evt_queue_handle = evt_queue_handle;
    shell_io_queue_handle = io_queue_handle;
    //APP_PRINT_INFO2("app_lea_cap_acc_cmd_init_msg_queue: cmd_evt_queue_handle 0x%x, cmd_io_queue_handle 0x%x",
    //                shell_evt_queue_handle, shell_io_queue_handle);
    p_shell_cmd_table = p_cmd_table;
}

static T_USER_CMD_PARSE_RESULT user_cmd_execute(T_USER_CMD_PARSED_VALUE *p_parse_value)
{
    int32_t i = 0;
    T_USER_CMD_PARSE_RESULT result = RESULT_CMD_NOT_FOUND;

    if (strcmp((const char *)p_parse_value->p_cmd, (const char *)"help") == 0)
    {
        user_cmd_list(p_shell_cmd_table);
        return RESULT_SUCCESS;
    }

    /* find command in table */
    while ((p_shell_cmd_table + i)->p_cmd != NULL)
    {
        if (strcmp((const char *)(p_shell_cmd_table + i)->p_cmd, (const char *)p_parse_value->p_cmd) == 0)
        {
            /* execute command function */
            result = (p_shell_cmd_table + i)->func(p_parse_value);
            /* exit while */
            break;
        }
        i++;
    };

    return result;
}

void app_shell_cmd_handle_msg(T_IO_MSG *p_shell_msg)
{
    uint16_t  subtype;
    uint8_t  *p;

    p       = p_shell_msg->u.buf;
    subtype = p_shell_msg->subtype;

    if (subtype == IO_MSG_UART_RX)
    {
        char       *subcmd;
        uint32_t    subcmd_len;
        uint32_t    param_num;
        uint32_t    param_value;
        T_USER_CMD_PARSED_VALUE parsed_value = {0};
        T_USER_CMD_PARSE_RESULT result;

        LE_STREAM_TO_UINT32(subcmd_len, p);
        subcmd = (char *)p;
        STREAM_SKIP_LEN(p, subcmd_len);
        LE_STREAM_TO_UINT32(param_num, p);

        //APP_PRINT_INFO2("user_cmd: subcmd %s, param_num %d", TRACE_STRING(subcmd), param_num);

        parsed_value.p_cmd = subcmd;
        parsed_value.param_count = param_num;


        for (uint8_t i = 0; i < param_num; i++)
        {
            LE_STREAM_TO_UINT32(param_value, p);
            parsed_value.dw_param[i] = param_value;
            //APP_PRINT_INFO3("user_cmd: data[%d] %d, 0x%x", i, param_value, param_value);
        }

        result = user_cmd_execute(&parsed_value);
        if (result != RESULT_SUCCESS)
        {
            cmd_send_result(result);
        }
    }

    free(p_shell_msg->u.buf);
}

/* register cmd to shell */
SHELL_CMD_REGISTER(le, NULL, "List battery service handle cache", cmd_shell_le);
