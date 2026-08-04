/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
*                        Header Files
*============================================================================*/
#include <string.h>
#include <pm.h>
#include <clock_manager.h>
#include <os_mem.h>
#include <zephyr/kernel.h>
#include <trace.h>
#include <section.h>
#include <fmc_api_ext.h>
#include "clk_mgr.h"

/*============================================================================*
 *                           Types
 *============================================================================*/

/**
* @defgroup    CLK_MGR  Clock Manager Module
* @brief       A module for managing high/normal clock performance of different clock domains for multiple users.
* @{
*/

typedef enum
{
    T_CLK_MGR_STATE_IDLE,
    T_CLK_MGR_STATE_SETTING_HIGH,
    T_CLK_MGR_STATE_SETTING_NORMAL,
    T_CLK_MGR_STATE_MAX,
} T_CLK_MGR_STATE;

typedef int32_t (*P_CLK_SET)(uint32_t id, uint32_t required_mhz, uint32_t *actual_mhz);

typedef struct
{
    uint32_t clk_mhz;
    P_CLK_SET set_clk_imp;
} T_CLK_MODULE;

typedef struct
{
    sys_snode_t node;
    const char *user_name;
    U_CLK_BITMAP bitmap_requested;      //bit that set to high or low
    U_CLK_BITMAP bitmap_allowed;        //which bit could be set to high or low
} T_CLK_USER;

typedef struct
{
    T_CLK_MGR_STATE state;
    struct k_queue user_queue;
    T_CLK_MODULE modules[T_CLK_TYPE_MAX];
} T_CLK_MGR;

/*============================================================================*
 *                           Constants
 *============================================================================*/


/*============================================================================*
 *                            Macros
 *============================================================================*/


/*============================================================================*
 *                            Variables
 *============================================================================*/
static uint32_t clk_modules_high_freq[T_CLK_TYPE_MAX] =
{
    200, 200, 280, 200, 280,
};

static uint32_t clk_modules_normal_freq[T_CLK_TYPE_MAX] =
{
    40, 40, 40, 40, 40,
};

static uint32_t clk_modules_id[T_CLK_TYPE_MAX] =
{
    0, FMC_SPIC_ID_0, FMC_SPIC_ID_1, FMC_SPIC_ID_2, FMC_SPIC_ID_3,
};

static const char *clk_type_name[T_CLK_TYPE_MAX] =
{
    "CPU", "SPIC0", "SPIC1", "SPIC2", "SPIC3"
};

static T_CLK_MGR clk_mgr;

/*============================================================================*
 *                           Private Functions
 *============================================================================*/

static int32_t local_cpu_clk_set(uint32_t id, uint32_t required_mhz, uint32_t *actual_mhz)
{
    return pm_cpu_freq_set(required_mhz, actual_mhz);
}

static int32_t local_spic0_clk_set(uint32_t id, uint32_t required_mhz, uint32_t *actual_mhz)
{
    bool ret = false;
#if CONFIG_APP_NANDBOOT
    ret = fmc_flash_nand_switch_clock_and_bit_mode(FMC_SPIC_ID_0, (uint8_t)required_mhz,
                                                   FMC_FLASH_NAND_4_BIT_MODE);
#else
    ret = fmc_flash_nor_clock_switch((FMC_SPIC_ID)id, required_mhz, actual_mhz);
#endif
    return (int32_t)!ret;
}

static int32_t local_spic1_clk_set(uint32_t id, uint32_t required_mhz, uint32_t *actual_mhz)
{
    bool ret = fmc_psram_clock_switch((FMC_SPIC_ID)id, required_mhz, actual_mhz);
    return (int32_t)!ret;
}

static int32_t local_spic2_clk_set(uint32_t id, uint32_t required_mhz, uint32_t *actual_mhz)
{
#if CONFIG_APP_NANDBOOT
    bool ret = 0;
#else
    bool ret = fmc_flash_nor_clock_switch((FMC_SPIC_ID)id, required_mhz, actual_mhz);
#endif
    return (int32_t)!ret;
}

static int32_t local_spic3_clk_set(uint32_t id, uint32_t required_mhz, uint32_t *actual_mhz)
{
    bool ret = fmc_psram_clock_switch((FMC_SPIC_ID)id, required_mhz, actual_mhz);
    return (int32_t)!ret;
}

static void clk_mgr_user_set_bit(T_CLK_USER *user, T_CLK_TYPE type)
{
    user->bitmap_requested.data |= BIT(type);
}

static void clk_mgr_user_clear_bit(T_CLK_USER *user, T_CLK_TYPE type)
{
    user->bitmap_requested.data &= ~BIT(type);
}

static T_CLK_USER *clk_mgr_get_user(T_CLK_USER_HANDLE handle)
{
    T_CLK_USER *user = NULL;
    user = (T_CLK_USER *)k_queue_peek_head(&clk_mgr.user_queue);
    while (user != NULL)
    {
        if (user == handle)
        {
            break;
        }
        user = (T_CLK_USER *)user->node.next;
    }

    return user;
}

static int32_t clk_mgr_normal_clk_valid(T_CLK_USER *user, T_CLK_TYPE type)
{
    T_CLK_USER *local_user = NULL;

    local_user = (T_CLK_USER *)k_queue_peek_head(&clk_mgr.user_queue);
    while (local_user != NULL)
    {
        APP_PRINT_TRACE4("clk_mgr_normal_clk_valid user[%s] type %d allowed %d requested %d",
                         TRACE_STRING(local_user->user_name), type, local_user->bitmap_allowed.data,
                         local_user->bitmap_requested.data);
        if ((BIT(type) & local_user->bitmap_requested.data) != 0)
        {
            APP_PRINT_INFO2("clk_mgr_normal_clk_valid: user [%s] holding the normal clk [%s]",
                            TRACE_STRING(local_user->user_name), TRACE_STRING(clk_type_name[type]));
            return -1;
        }
        local_user = (T_CLK_USER *)local_user->node.next;
    }

    return 0;
}

static int32_t clk_mgr_high_clk_valid(T_CLK_USER *user, T_CLK_TYPE type)
{
    T_CLK_USER *local_user = NULL;

    local_user = (T_CLK_USER *)k_queue_peek_head(&clk_mgr.user_queue);
    while (local_user != NULL)
    {
        if (user == local_user)
        {
            local_user = (T_CLK_USER *)local_user->node.next;
            continue;
        }
        if ((BIT(type) & local_user->bitmap_requested.data) != 0)
        {
            APP_PRINT_INFO2("clk_mgr_high_clk_valid: user [%s] holding the high clk [%s]",
                            TRACE_STRING(local_user->user_name), TRACE_STRING(clk_type_name[type]));
            return -1;
        }
        local_user = (T_CLK_USER *)local_user->node.next;
    }

    return 0;
}

static int32_t clk_mgr_bitmap_permissions_valid(T_CLK_USER *user, T_CLK_TYPE type)
{
    if ((BIT(type) & user->bitmap_allowed.data) == 0)
    {
        APP_PRINT_TRACE2("clk_mgr_bitmap_permissions_invalid [%s] allowd bitmap: 0x%x",
                         TRACE_STRING(clk_type_name[type]),
                         user->bitmap_allowed.data);
        return -1;
    }
    return 0;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

int32_t clk_mgr_set_high_performance(T_CLK_USER_HANDLE handle)
{
    int32_t ret = 0;
    uint32_t local_actual_mhz = UINT32_MAX;

    if (clk_mgr.state)
    {
        ret = -1;
        goto CLK_MGR_ERROR1;
    }
    clk_mgr.state = T_CLK_MGR_STATE_SETTING_HIGH;

    T_CLK_USER *user = clk_mgr_get_user(handle);
    if (user == NULL)
    {
        ret = -2;
        goto CLK_MGR_ERROR2;
    }

    for (T_CLK_TYPE i = 0; i < T_CLK_TYPE_MAX; i++)
    {
        if (clk_mgr_bitmap_permissions_valid(user, i))
        {
            continue;
        }

        if (clk_mgr.modules[i].set_clk_imp == NULL)
        {
            ret = -4;
            goto CLK_MGR_ERROR2;
        }

        clk_mgr_user_set_bit(user, i);

        /*this user should be the first to set high clk*/
        if (clk_mgr_high_clk_valid(user, i) == 0)
        {
            if (clk_mgr.modules[i].set_clk_imp(clk_modules_id[i], clk_modules_high_freq[i],
                                               &local_actual_mhz))
            {
                ret = -5;
                goto CLK_MGR_ERROR2;
            }
            if (local_actual_mhz != UINT32_MAX)
            {
                clk_mgr.modules[i].clk_mhz = local_actual_mhz;
                APP_PRINT_TRACE4("clk_mgr_set_high_performance handle 0x%x - [%s], [%s], actual_mhz %d", handle,
                                 TRACE_STRING(user->user_name), TRACE_STRING(clk_type_name[i]),
                                 clk_mgr.modules[i].clk_mhz);
            }
        }
    }

CLK_MGR_ERROR2:
    clk_mgr.state = T_CLK_MGR_STATE_IDLE;
CLK_MGR_ERROR1:
    if (ret)
    {
        APP_PRINT_ERROR1("clk_mgr_set_high_performance error!, ret %d", ret);
    }

    return ret;
}

int32_t clk_mgr_set_normal_performance(T_CLK_USER_HANDLE handle)
{
    int32_t ret = 0;
    uint32_t local_actual_mhz = UINT32_MAX;

    if (clk_mgr.state)
    {
        ret = -1;
        goto CLK_MGR_ERROR1;
    }
    clk_mgr.state = T_CLK_MGR_STATE_SETTING_HIGH;

    T_CLK_USER *user = clk_mgr_get_user(handle);
    if (user == NULL)
    {
        ret = -2;
        goto CLK_MGR_ERROR2;
    }

    for (T_CLK_TYPE i = 0; i < T_CLK_TYPE_MAX; i++)
    {
        if (clk_mgr_bitmap_permissions_valid(user, i))
        {
            continue;
        }

        if (clk_mgr.modules[i].set_clk_imp == NULL)
        {
            ret = -4;
            goto CLK_MGR_ERROR2;
        }

        clk_mgr_user_clear_bit(user, i);

        /*include this user, all users have set normal*/
        if (clk_mgr_normal_clk_valid(user, i) == 0)
        {
            if (clk_mgr.modules[i].set_clk_imp(clk_modules_id[i], clk_modules_normal_freq[i],
                                               &local_actual_mhz))
            {
                ret = -5;
                goto CLK_MGR_ERROR2;
            }
            if (local_actual_mhz != UINT32_MAX)
            {
                clk_mgr.modules[i].clk_mhz = local_actual_mhz;
            }
        }
        APP_PRINT_TRACE4("clk_mgr_set_normal_performance handle 0x%x - [%s], [%s], actual_mhz %d", handle,
                         TRACE_STRING(user->user_name), TRACE_STRING(clk_type_name[i]),
                         clk_mgr.modules[i].clk_mhz);
    }

CLK_MGR_ERROR2:
    clk_mgr.state = T_CLK_MGR_STATE_IDLE;
CLK_MGR_ERROR1:
    if (ret)
    {
        APP_PRINT_ERROR1("clk_mgr_set_normal_performance error!, ret %d", ret);
    }

    return ret;
}

T_CLK_USER_HANDLE clk_mgr_user_create(const char *user_name, U_CLK_BITMAP bitmap_allowed)
{
    T_CLK_USER *user = NULL;

    user = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(T_CLK_USER));
    if (user == NULL)
    {
        return 0;
    }
    memset(user, 0x00, sizeof(T_CLK_USER));
    k_queue_append(&clk_mgr.user_queue, user);

    user->user_name = user_name;
    user->bitmap_allowed.data = bitmap_allowed.data;

    return (T_CLK_USER_HANDLE)user;
}

int32_t clk_mgr_user_delete(T_CLK_USER_HANDLE handle)
{
    T_CLK_USER *user = NULL;

    if (handle == NULL)
    {
        return -1;
    }

    user = (T_CLK_USER *)k_queue_peek_head(&clk_mgr.user_queue);
    while (user != NULL)
    {
        if (user == handle)
        {
            break;
        }
        user = (T_CLK_USER *)user->node.next;
    }

    if (user == NULL)
    {
        return -2;
    }

    k_queue_remove(&clk_mgr.user_queue, user);
    os_mem_free(user);

    return 0;
}

void clk_mgr_init()
{
    uint32_t actual_mhz;

    memset(&clk_mgr, 0, sizeof(T_CLK_MGR));
    k_queue_init(&clk_mgr.user_queue);

    clk_mgr.modules[T_CLK_TYPE_CPU].set_clk_imp = local_cpu_clk_set;
    clk_mgr.modules[T_CLK_TYPE_SPIC0].set_clk_imp = local_spic0_clk_set;
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram0), okay)
    clk_mgr.modules[T_CLK_TYPE_SPIC1].set_clk_imp = local_spic1_clk_set;
#endif
    clk_mgr.modules[T_CLK_TYPE_SPIC2].set_clk_imp = NULL;
#if DT_NODE_HAS_STATUS(DT_NODELABEL(psram1), okay)
    clk_mgr.modules[T_CLK_TYPE_SPIC3].set_clk_imp = local_spic3_clk_set;
#endif

    clk_mgr.state = T_CLK_MGR_STATE_IDLE;


    clk_mgr.modules[T_CLK_TYPE_CPU].set_clk_imp(0, clk_modules_normal_freq[T_CLK_TYPE_CPU],
                                                &actual_mhz);
    clk_mgr.modules[T_CLK_TYPE_CPU].clk_mhz = actual_mhz;
    clk_mgr.modules[T_CLK_TYPE_SPIC0].set_clk_imp(FMC_SPIC_ID_0,
                                                  clk_modules_high_freq[T_CLK_TYPE_SPIC0], &actual_mhz);
    clk_mgr.modules[T_CLK_TYPE_SPIC0].clk_mhz = actual_mhz;
    clk_mgr.modules[T_CLK_TYPE_SPIC1].set_clk_imp(FMC_SPIC_ID_1,
                                                  clk_modules_high_freq[T_CLK_TYPE_SPIC1], &actual_mhz);
    clk_mgr.modules[T_CLK_TYPE_SPIC1].clk_mhz = actual_mhz;
    // clk_mgr.modules[T_CLK_TYPE_SPIC2].set_clk_imp(FMC_SPIC_ID_2, clk_modules_normal_freq[T_CLK_TYPE_SPIC2], &actual_mhz);
    // clk_mgr.modules[T_CLK_TYPE_SPIC2].clk_mhz = actual_mhz;
    clk_mgr.modules[T_CLK_TYPE_SPIC3].set_clk_imp(FMC_SPIC_ID_3,
                                                  clk_modules_high_freq[T_CLK_TYPE_SPIC3], &actual_mhz);
    clk_mgr.modules[T_CLK_TYPE_SPIC3].clk_mhz = actual_mhz;
    APP_PRINT_TRACE0("clk_mgr_init done");

}

void clk_mgr_deinit()
{
    clk_mgr.modules[T_CLK_TYPE_CPU].set_clk_imp = NULL;
    clk_mgr.modules[T_CLK_TYPE_SPIC0].set_clk_imp = NULL;
    clk_mgr.modules[T_CLK_TYPE_SPIC1].set_clk_imp = NULL;
    clk_mgr.modules[T_CLK_TYPE_SPIC2].set_clk_imp = NULL;
    clk_mgr.modules[T_CLK_TYPE_SPIC3].set_clk_imp = NULL;
}

void clk_mgr_testcase()
{
    uint32_t line = 0;

    fmc_psram_winbond_opi_init(FMC_SPIC_ID_1);
    fmc_psram_ap_memory_opi_init(FMC_SPIC_ID_3);
    fmc_flash_try_high_speed_mode(FMC_SPIC_ID_0, FMC_FLASH_NOR_4_BIT_MODE);
    clk_mgr_init();

    T_CLK_USER_HANDLE clk_user_handle[10];
    const char *clk_user_name[10] =
    {
        "user0", "user1", "user2", "user3", "user4",
        "user5", "user6", "user7", "user8", "user9",
    };

    U_CLK_BITMAP bitmap[5];
    bitmap[0].data = BIT(T_CLK_TYPE_CPU);
    bitmap[1].data = BIT(T_CLK_TYPE_SPIC0);
    bitmap[2].data = BIT(T_CLK_TYPE_SPIC1);
    bitmap[3].data = BIT(T_CLK_TYPE_SPIC3);
    bitmap[4].data = BIT(T_CLK_TYPE_CPU) | BIT(T_CLK_TYPE_SPIC0) | BIT(T_CLK_TYPE_SPIC1) | BIT(
                         T_CLK_TYPE_SPIC2) | BIT(T_CLK_TYPE_SPIC3);

    os_mem_peek_printf();
    for (uint8_t i = 0; i < 10; i++)
    {
        clk_user_handle[i] = clk_mgr_user_create(clk_user_name[i], bitmap[i % 5]);
    }

    os_mem_peek_printf();
    for (uint8_t i = 0; i < 10; i++)
    {
        uint32_t actual_mhz[5] = {0};
        T_CLK_USER *user = (T_CLK_USER *)clk_user_handle[i];
        clk_mgr_set_high_performance(clk_user_handle[i]);

        APP_PRINT_TRACE3("clk_mgr_test high i %d, user %s, bitmap 0x%08x", i, TRACE_STRING(user->user_name),
                         user->bitmap_requested.data);
        APP_PRINT_TRACE5("clk_mgr_test high clk mgr save %d %d %d %d %d", clk_mgr.modules[0].clk_mhz,
                         clk_mgr.modules[1].clk_mhz,
                         clk_mgr.modules[2].clk_mhz, clk_mgr.modules[3].clk_mhz, clk_mgr.modules[4].clk_mhz);
    }

    os_mem_peek_printf();
    for (uint8_t i = 0; i < 10; i++)
    {
        uint32_t actual_mhz[5] = {0};
        T_CLK_USER *user = (T_CLK_USER *)clk_user_handle[i];
        clk_mgr_set_normal_performance(clk_user_handle[i]);

        APP_PRINT_TRACE3("clk_mgr_test low i %d, user %s, bitmap 0x%08x", i, TRACE_STRING(user->user_name),
                         user->bitmap_requested.data);
        APP_PRINT_TRACE5("clk_mgr_test low clk mgr save %d %d %d %d %d", clk_mgr.modules[0].clk_mhz,
                         clk_mgr.modules[1].clk_mhz,
                         clk_mgr.modules[2].clk_mhz, clk_mgr.modules[3].clk_mhz, clk_mgr.modules[4].clk_mhz);

    }

    os_mem_peek_printf();
    for (uint8_t i = 0; i < 10; i++)
    {
        uint32_t actual_mhz[5] = {0};
        T_CLK_USER *user = (T_CLK_USER *)clk_user_handle[i];
        clk_mgr_set_high_performance(clk_user_handle[i]);

        APP_PRINT_TRACE3("clk_mgr_test high i %d, user %s, bitmap 0x%08x", i, TRACE_STRING(user->user_name),
                         user->bitmap_requested.data);
        APP_PRINT_TRACE5("clk_mgr_test high clk mgr save %d %d %d %d %d", clk_mgr.modules[0].clk_mhz,
                         clk_mgr.modules[1].clk_mhz,
                         clk_mgr.modules[2].clk_mhz, clk_mgr.modules[3].clk_mhz, clk_mgr.modules[4].clk_mhz);

    }

    os_mem_peek_printf();
    for (uint8_t i = 0; i < 10; i++)
    {
        uint32_t actual_mhz[5] = {0};
        T_CLK_USER *user = (T_CLK_USER *)clk_user_handle[i];
        clk_mgr_set_normal_performance(clk_user_handle[i]);

        APP_PRINT_TRACE3("clk_mgr_test low i %d, user %s, bitmap 0x%08x", i, TRACE_STRING(user->user_name),
                         user->bitmap_requested.data);
        APP_PRINT_TRACE5("clk_mgr_test low clk mgr save %d %d %d %d %d", clk_mgr.modules[0].clk_mhz,
                         clk_mgr.modules[1].clk_mhz,
                         clk_mgr.modules[2].clk_mhz, clk_mgr.modules[3].clk_mhz, clk_mgr.modules[4].clk_mhz);
    }

    for (uint8_t i = 0; i < 10; i++)
    {
        clk_mgr_user_delete(clk_user_handle[i]);
    }
    os_mem_peek_printf();

    APP_PRINT_TRACE0("clk mgr test done!");
}

/** @} */ /* end of CLK_MGR */