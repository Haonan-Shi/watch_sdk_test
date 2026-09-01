/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "power_test_link.h"

static T_POWER_TEST_APP_DB app_db;//APP SPP data base.

T_POWER_TEST_LINK *power_test_find_link(uint8_t *bd_addr)
{
    T_POWER_TEST_LINK *p_link = NULL;
    uint8_t        i;

    if (bd_addr != NULL)
    {
        for (i = 0; i < POWER_TEST_MAX_BR_LINK_NUM; i++)
        {
            if (app_db.app_link[i].used == true &&
                !memcmp(app_db.app_link[i].bd_addr, bd_addr, 6))
            {
                p_link = &app_db.app_link[i];
                break;
            }
        }
    }

    return p_link;
}

T_POWER_TEST_LINK *power_test_alloc_link(uint8_t *bd_addr)
{
    T_POWER_TEST_LINK *p_link = NULL;
    uint8_t        i;

    if (bd_addr != NULL)
    {
        for (i = 0; i < POWER_TEST_MAX_BR_LINK_NUM; i++)
        {
            if (app_db.app_link[i].used == false)
            {
                p_link = &app_db.app_link[i];

                p_link->used = true;
                p_link->id   = i;
                memcpy(p_link->bd_addr, bd_addr, 6);
                break;
            }
        }
    }

    return p_link;
}

bool power_test_free_link(T_POWER_TEST_LINK *p_link)
{
    if (p_link != NULL)
    {
        if (p_link->used == true)
        {
            memset(p_link, 0, sizeof(T_POWER_TEST_LINK));
            return true;
        }
    }

    return false;
}
