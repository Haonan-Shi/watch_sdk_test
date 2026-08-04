/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _CLK_MGR_H_
#define _CLK_MGR_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

typedef enum
{
    T_CLK_TYPE_CPU,
    T_CLK_TYPE_SPIC0,
    T_CLK_TYPE_SPIC1,
    T_CLK_TYPE_SPIC2,
    T_CLK_TYPE_SPIC3,
    T_CLK_TYPE_MAX,
} T_CLK_TYPE;

typedef union
{
    uint32_t data;
    struct
    {
        uint32_t cpu : 1;
        uint32_t spic0 : 1;
        uint32_t spic1 : 1;
        uint32_t spic2 : 1;
        uint32_t spic3 : 1;
    };
} U_CLK_BITMAP;

typedef void *T_CLK_USER_HANDLE;

/**
 * @brief   Initialize the clock manager module.
 */
void clk_mgr_init();

/**
 * @brief   Deinitialize the clock manager module.
 */
void clk_mgr_deinit();

/**
 * @brief   Create a new clock user node.
 * @param   user_name  Pointer to the user name string.
 * @param   bitmap_permissions  Clk bitmap that user can set.
 * @return  The user handle. Returns 0 if failed.
 */
T_CLK_USER_HANDLE clk_mgr_user_create(const char *user_name, U_CLK_BITMAP bitmap_permissions);

/**
 * @brief   Delete a clock user node.
 * @param   handle User handle.
 * @return  0 for success, negative for error.
 */
int32_t clk_mgr_user_delete(T_CLK_USER_HANDLE handle);

/**
 * @brief   Set a module to high-performance for a specific user.
 * @param   handle     User handle.
 * @return  0 for success, negative value for different errors.
 */
int32_t clk_mgr_set_high_performance(T_CLK_USER_HANDLE handle);

/**
 * @brief   Set a module back to normal-performance for a specific user.
 * @param   handle     User handle.
 * @return  0 for success, negative value for different errors.
 */
int32_t clk_mgr_set_normal_performance(T_CLK_USER_HANDLE handle);

#ifdef __cplusplus
}
#endif

#endif
