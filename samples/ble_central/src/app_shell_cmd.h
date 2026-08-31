/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_BLE_CENTRAL_CMD_PARSE_H_
#define _APP_BLE_CENTRAL_CMD_PARSE_H_

//#include "os_mem.h"
#include "app_msg.h"

#ifdef  __cplusplus
extern "C" {
#endif      /* __cplusplus */

#define USER_CMD_MAX_PARAMETERS         20  /**< max. number of parameters that the parser will scan */

typedef enum
{
    RESULT_SUCCESS,                             //!< Operation success.
    RESULT_ERR                          = 0x20,
    RESULT_CMD_EMPTY_LINE               = 0x21,
    RESULT_CMD_NOT_FOUND                = 0x22,
    RESULT_CMD_ERR_PARAM                = 0x23,
    RESULT_CMD_ERR_PARAM_NUM            = 0x24,
    RESULT_CMD_OUT_OF_RANGE             = 0x25,
    RESULT_CMD_NOT_SUPPORT              = 0x26,
} T_USER_CMD_PARSE_RESULT;


/**
 * @brief Data UART command parse value.
 *
 * This is the structure where the command line parser puts its values.
 */
typedef struct
{
    char       *p_cmd;                              /**< pointer to command */
    int32_t     param_count;                        /**< number of found parameters */
    uint32_t    dw_param[USER_CMD_MAX_PARAMETERS];  /**< automatically parsed parameters */
} T_USER_CMD_PARSED_VALUE;

/** @brief Prototype of functions that can be called from command table. */
typedef T_USER_CMD_PARSE_RESULT(*T_USER_CMD_FUNC)(T_USER_CMD_PARSED_VALUE *p_parse_value);

/**
 * @brief Command table entry.
 *
 */
typedef struct
{
    char               *p_cmd;
    char               *p_option;
    char               *p_help;
    T_USER_CMD_FUNC     func;
} T_USER_CMD_TABLE_ENTRY;

void app_shell_print(char *fmt, ...);

/**
 * @brief Init evt_queue and io_queue
 *
 * @param evt_queue event queue
 * @param io_queue io queue
 */
void app_shell_cmd_register(void *evt_queue_handle, void *io_queue_handle,
                            const T_USER_CMD_TABLE_ENTRY *p_cmd_table);

/**
 * @brief Handle message for uart
 *
 * @param msg  T_IO_MSG
 */
void app_shell_cmd_handle_msg(T_IO_MSG *msg);

/** End of _APP_BLE_CENTRAL_CMD_PARSE_H_
* @}
*/

#ifdef  __cplusplus
}
#endif      /*  __cplusplus */
#endif
