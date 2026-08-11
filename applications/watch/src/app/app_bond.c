/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "os_mem.h"
#include "trace.h"
#include "bt_bond.h"
#include "btm.h"
#include "remote.h"
#include "app_main.h"
#include "app_cfg.h"
#include "app_bond.h"
#include "ftl.h"
#include "gap_br.h"
#include "app_linkback.h"
#include <stdlib.h>
#include <section.h>
#include "os_task.h"
#include "app_cmd.h"
#include "bt_bond_le.h"
#include "module_global_data.h"
#include "gap_conn_le.h"
#include "app_link_util.h"
#include "app_bt_policy_api.h"

static T_APP_BOND_DEVICE temp_search_device[TEMP_MAX_SEARCH] = {0};
static T_APP_BOND_DEVICE temp_cache_device[TEMP_MAX_SEARCH] = {0};

static uint16_t temp_priority = 0;

void app_bond_set_priority(uint8_t *bd_addr)
{
    uint8_t temp_addr[6];

    if (bt_bond_addr_get(1, temp_addr) == true)
    {
        if (memcmp(bd_addr, temp_addr, 6))
        {
            bt_bond_priority_set(bd_addr);
        }
    }
}

uint32_t app_bt_bond_load_device_info_from_ftl(T_APP_BOND_DEVICE *bond_info)
{
    T_APP_BOND_DEVICE *temp = NULL;
    uint32_t ret = 1;

    ret = ftl_load_from_storage(bond_info, APP_BOND_DEVICE_INFO_ADDR, APP_BOND_DEVICE_INFO_SIZE);
    temp = bond_info;
    if (ret)
    {
        APP_PRINT_INFO1("app_bt_bond_load_device_info_from_ftl ret %d", ret);
        memset(bond_info, 0x00, APP_BOND_DEVICE_INFO_SIZE);
    }
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        APP_PRINT_INFO6("app_bt_bond_load_device_info_from_ftl: bd_addr %s, device type %d ,used %d ,priority %d, exist_addr_flag %x, device_name_len = %x",
                        TRACE_BDADDR(temp[i].bd_addr), temp[i].device_type, temp[i].used, temp[i].priority,
                        temp[i].exist_addr_flag, temp[i].device_name_len);
    }
    return ret;
}

uint32_t app_bt_bond_save_device_info_to_ftl(void *bond_info)
{
    T_APP_BOND_DEVICE *temp = (T_APP_BOND_DEVICE *)bond_info;
    uint32_t ret = 1;

    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        APP_PRINT_INFO6("app_bt_bond_save_device_info_to_ftl: bd_addr %s, device type %d ,used %d ,priority %d, exist_addr_flag %x, device_name_len = %x",
                        TRACE_BDADDR(temp[i].bd_addr), temp[i].device_type, temp[i].used, temp[i].priority,
                        temp[i].exist_addr_flag, temp[i].device_name_len);
    }
    ret = ftl_save_to_storage(bond_info, APP_BOND_DEVICE_INFO_ADDR, APP_BOND_DEVICE_INFO_SIZE);
    return ret;
}

//return the num has been used(0-8)
uint16_t app_bt_bond_get_num(void)
{
    uint16_t device_num = 0;

    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].exist_addr_flag)
        {
            device_num++;
        }
    }
    APP_PRINT_INFO1("app_bt_bond_get_num num %d ", device_num);
    return device_num;
}

uint8_t app_bt_bond_get_num_ear(void)
{
    uint8_t device_num = 0;

    for (uint8_t i = 1; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].exist_addr_flag &&
            (app_db.bond_device[i].device_type == T_DEVICE_TYPE_EARPHONE))
        {
            device_num++;
        }
    }
    APP_PRINT_INFO1("app_bt_bond_get_num_ear num %d ", device_num);
    return device_num;
}

//free all bond device
void app_bt_bond_free_all_device(void)
{
    APP_PRINT_TRACE0("app_bt_bond_free_temp_search_device start");
    for (int i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        app_db.bond_device[i].used = false;
    }
}

T_APP_BOND_DEVICE *app_bt_bond_get_device_by_addr(uint8_t *bd_addr)
{
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (!memcmp(bd_addr, app_db.bond_device[i].bd_addr, 6))
        {
            return &app_db.bond_device[i];
        }
    }
    return NULL;
}

uint8_t app_bt_bond_get_index_by_addr(uint8_t *bd_addr)
{
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        APP_PRINT_INFO2("app_bt_bond_get_index_by_addr: bd_addr %s, bd_addr1 %s",
                        TRACE_BDADDR(bd_addr), TRACE_BDADDR(app_db.bond_device[i].bd_addr));
        if (!memcmp(bd_addr, app_db.bond_device[i].bd_addr, 6))
        {
            return i;
        }
    }
    return MAX_BOND_INFO_NUM;
}

bool app_bt_bond_check_device_exists(uint8_t index)
{
    if (index >= MAX_BOND_INFO_NUM)
    {
        return false;
    }
    return app_db.bond_device[index].exist_addr_flag != 0;
}

uint8_t app_bt_bond_check_exist_device_info(T_DEVICE_TYPE device_type)
{
    uint8_t device_num = 0xff;
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].exist_addr_flag && app_db.bond_device[i].device_type == device_type)
        {
            return i;
        }
    }
    return device_num;
}

void app_bt_bond_sort_priority(T_APP_BOND_DEVICE *temp, int len)
{
    T_APP_BOND_DEVICE temp_elem;
    for (int i = 1; i < len - 1; i ++)
    {
        for (int j = i + 1; j < len; j ++)
        {
            if (temp[i].priority < temp[j].priority)
            {
                temp_elem = temp[i];
                temp[i] = temp[j];
                temp[j] = temp_elem;
            }
        }
    }
    for (int i = 0; i < len; i ++)
    {
        APP_PRINT_INFO6("app_bt_bond_sort_priority: bd_addr %s, device type %d ,used %d ,priority %d, exist_addr_flag %x, device_name_len = %x",
                        TRACE_BDADDR(app_db.bond_device[i].bd_addr), app_db.bond_device[i].device_type,
                        app_db.bond_device[i].used, app_db.bond_device[i].priority, app_db.bond_device[i].exist_addr_flag,
                        app_db.bond_device[i].device_name_len);

    }
}

//updata priority(from 0-8) if delete node,then update. if active node,then update
void app_bt_bond_update_device_priority(uint8_t *bd_addr, bool delete_device)
{
    APP_PRINT_INFO2("app_bt_bond_update_device_priority: bd_addr %s , delete_device =%d",
                    TRACE_BDADDR(bd_addr), delete_device);
    if (delete_device)
    {
        for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
        {
            if (temp_priority < app_db.bond_device[i].priority)
            {
                app_db.bond_device[i].priority = app_db.bond_device[i].priority - 1;
            }

        }
    }
    else
    {
        uint16_t priority = app_bt_bond_get_num();
        for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
        {
            if (!memcmp(bd_addr, app_db.bond_device[i].bd_addr, 6))
            {
                app_db.bond_device[i].priority = priority;
            }
            else
            {
                if (temp_priority < app_db.bond_device[i].priority && app_db.bond_device[i].priority != 0)
                {
                    app_db.bond_device[i].priority = app_db.bond_device[i].priority - 1;
                }
            }
        }
    }
}

//disconnect and inactive
bool app_bt_bond_inactive_device(uint8_t *bd_addr)
{
    APP_PRINT_INFO1("app_bt_bond_inactive_device: bd_addr %s",
                    TRACE_BDADDR(bd_addr));
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (!memcmp(bd_addr, app_db.bond_device[i].bd_addr, 6))
        {
            app_db.bond_device[i].used = false;
            return true;
        }
    }
    return false;
}

//disconnect and inactive by index
bool app_bt_bond_inactive_device_by_index(uint8_t index)
{
    APP_PRINT_INFO1("app_bt_bond_inactive_device_by_index: index %d", index);
    app_db.bond_device[index].used = false;
    return true;
}

//del node,need use bt_bond_delete after this func
bool app_bt_bond_del_bond_device(uint8_t *bd_addr)
{
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (!memcmp(bd_addr, app_db.bond_device[i].bd_addr, 6))
        {
            APP_PRINT_INFO1("app_bt_bond_del_bond_device: bd_addr %s",
                            TRACE_BDADDR(bd_addr));
            app_db.bond_device[i].used = false;
            temp_priority = app_db.bond_device[i].priority;
            app_db.bond_device[i].priority = 0;
            app_db.bond_device[i].exist_addr_flag = 0;
            app_db.bond_device[i].device_name_len = 0;
            app_bt_bond_update_device_priority(bd_addr, true);
            app_db.bond_device[i].device_type = T_DEVICE_TYPE_PHONE;
            memset(app_db.bond_device[i].bd_addr, 0, 6);
            if (i != 0)
            {
                app_bt_bond_sort_priority(app_db.bond_device, MAX_BOND_INFO_NUM);
            }
            return true;
        }
    }
    return false;
}


uint8_t app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE device_type)
{
    uint8_t device_num = 0xff;

    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].used && (app_db.bond_device[i].device_type == device_type))
        {
            return i;
        }
    }
    return device_num;
}

void app_bt_bond_remove_by_index(uint8_t index)
{
    APP_PRINT_INFO1("app_bt_bond_remove_by_index: index=%d", index);
    if (index >= MAX_BOND_INFO_NUM || !app_db.bond_device[index].exist_addr_flag)
    {
        return;
    }

    uint8_t *bd_addr = app_db.bond_device[index].bd_addr;

    if (app_find_br_link(bd_addr) != NULL)
    {
        APP_PRINT_INFO0("app_bt_bond_remove_by_index: device connected, disconnect first");
        app_bt_policy_disconnect(bd_addr,
                                 A2DP_PROFILE_MASK | HFP_PROFILE_MASK | AVRCP_PROFILE_MASK);
    }

    if (!bt_bond_delete(bd_addr))
    {
        APP_PRINT_INFO0("app_bt_bond_remove_by_index: bt_bond_delete fail");
    }
    app_bt_bond_del_bond_device(bd_addr);
    app_bt_bond_save_device_info_to_ftl(app_db.bond_device);
}

T_APP_BOND_DEVICE *app_bt_bond_get_active_device_by_type(T_DEVICE_TYPE device_type)
{
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].used && (app_db.bond_device[i].device_type == device_type))
        {
            return &app_db.bond_device[i];
        }
    }
    return NULL;
}

uint8_t app_bt_bond_get_num_by_type(T_DEVICE_TYPE device_type)
{
    uint8_t device_num = 0;

    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if ((app_db.bond_device[i].exist_addr_flag) && (app_db.bond_device[i].device_type == device_type))
        {
            device_num++;
        }
    }
    return device_num;
}

//the first position is for phone
uint8_t *app_bt_bond_find_least_priority_device_new(void)
{
    uint8_t flag_count = 1;
    for (uint8_t i = 2; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].priority < app_db.bond_device[flag_count].priority)
        {
            flag_count = i;
        }
    }
    return app_db.bond_device[flag_count].bd_addr;
}



uint8_t app_bt_bond_check_bond_device_full(void)
{
    APP_PRINT_TRACE0("app_bt_bond_check_bond_device_full start");
    uint8_t ret = 0;
    uint8_t *temp_addr = NULL;
    uint16_t temp_bond_num = app_bt_bond_get_num_ear();
    if (temp_bond_num < (MAX_BOND_INFO_NUM - 1))
    {
        return ret;
    }
    else
    {
        temp_addr = app_bt_bond_find_least_priority_device_new();
        bt_bond_delete(temp_addr);
        app_bt_bond_del_bond_device(temp_addr);
        APP_PRINT_TRACE1("app_bt_bond_check_bond_device_full bond device reach max, delete device(%s)",
                         TRACE_BDADDR(temp_addr));
        return ret;
    }
}

bool app_bt_bond_add_device(uint8_t *bd_addr, T_DEVICE_TYPE device_type)
{
    uint8_t bond_temp = app_bt_bond_get_index_by_addr(bd_addr);
    if (bond_temp < MAX_BOND_INFO_NUM)//active old device
    {
        if (app_db.bond_device[bond_temp].used)
        {
            return true;
        }
        app_db.bond_device[bond_temp].used = true;
        app_db.bond_device[bond_temp].exist_addr_flag = 1;
        temp_priority = app_db.bond_device[bond_temp].priority;
        if (app_db.bond_device[bond_temp].device_type != T_DEVICE_TYPE_EARPHONE)
        {
            app_db.bond_device[bond_temp].device_type = T_DEVICE_TYPE_PHONE;
        }
        if (app_db.bond_device[bond_temp].device_name_len == 0)
        {
            gap_br_get_remote_name(bd_addr);
            APP_PRINT_INFO0("app_bt_bond_add_device req active name");
        }
        app_bt_bond_update_device_priority(bd_addr, false);
        app_bt_bond_sort_priority(app_db.bond_device, MAX_BOND_INFO_NUM);
        APP_PRINT_INFO2("app_bt_bond_add_device1: bd_addr %s, device_type  %x",
                        TRACE_BDADDR(bd_addr), app_db.bond_device[bond_temp].device_type);
        return true;
    }
    else
    {
        if (device_type == T_DEVICE_TYPE_DEFAULT) //not an active indicate
        {
            APP_PRINT_INFO1("the deleted device cannot linkback,addr: %s", TRACE_BDADDR(bd_addr));
            return false;
        }
        if (app_db.bond_device[0].exist_addr_flag)
        {
            app_bt_bond_del_bond_device(app_db.bond_device[0].bd_addr);
        }
        app_db.bond_device[0].used = true;
        memcpy(app_db.bond_device[0].bd_addr, bd_addr, 6);
        app_db.bond_device[0].device_type = T_DEVICE_TYPE_PHONE;
        app_db.bond_device[0].exist_addr_flag = 1;
        app_db.bond_device[0].priority = app_bt_bond_get_num();
        if (app_db.bond_device[0].device_name_len == 0)
        {
            gap_br_get_remote_name(bd_addr);
            APP_PRINT_INFO0("app_bt_bond_add_device req new name");
        }
        app_bt_bond_sort_priority(app_db.bond_device, MAX_BOND_INFO_NUM);
        return true;
    }
}

uint8_t *app_bt_bond_check_exist_other_active_device(uint8_t *addr, T_DEVICE_TYPE device_type)
{
    if (app_bt_bond_get_active_num_by_type(device_type) == 0xff)
    {
        return NULL;
    }
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].used && memcmp(app_db.bond_device[i].bd_addr, addr, 6) != 0 &&
            app_db.bond_device[i].device_type == device_type)
        {
            return app_db.bond_device[i].bd_addr;
        }
    }
    return NULL;
}

bool app_bt_bond_add_search_device(uint8_t *bd_addr)
{
    APP_PRINT_INFO1("app_bt_bond_add_search_device: bd_addr %s",
                    TRACE_BDADDR(bd_addr));
    T_APP_BOND_DEVICE *temp_bond_device = NULL;

    uint8_t bond_temp = app_bt_bond_get_index_by_addr(bd_addr);
    temp_bond_device = app_bt_bond_get_temp_search_device(bd_addr);
    if (temp_bond_device != NULL)
    {
        if (bond_temp < MAX_BOND_INFO_NUM)//active old device
        {
            app_db.bond_device[bond_temp].used = true;
            app_db.bond_device[bond_temp].device_type = T_DEVICE_TYPE_EARPHONE;

            if (temp_bond_device->device_name_len != 0)
            {
                memcpy((uint8_t *)app_db.bond_device[bond_temp].device_name,
                       (uint8_t *)temp_bond_device->device_name,
                       sizeof(app_db.bond_device[bond_temp].device_name));
                app_db.bond_device[bond_temp].exist_addr_flag = 1;
                app_db.bond_device[bond_temp].device_name_len = temp_bond_device->device_name_len;
            }
            temp_priority = app_db.bond_device[bond_temp].priority;
            app_bt_bond_update_device_priority(bd_addr, false);
            app_bt_bond_sort_priority(app_db.bond_device, MAX_BOND_INFO_NUM);
            app_bt_bond_free_temp_cache_device();
            app_bt_bond_temp_cache_save_to_search();
            return true;
        }
        else
        {
            if (temp_bond_device->device_name_len == 0)
            {
                gap_br_get_remote_name(bd_addr);
                APP_PRINT_INFO0("app_bt_bond_add_search_device error: no remote name info");
                return false;
            }

            app_bt_bond_check_bond_device_full();
            for (uint8_t i = 1; i < MAX_BOND_INFO_NUM; i++)
            {
                if (!(app_db.bond_device[i].exist_addr_flag))
                {
                    app_db.bond_device[i].used = true;
                    memcpy(app_db.bond_device[i].bd_addr, bd_addr, 6);
                    app_db.bond_device[i].device_type = T_DEVICE_TYPE_EARPHONE;

                    memcpy((uint8_t *)app_db.bond_device[i].device_name, (uint8_t *)temp_bond_device->device_name,
                           sizeof(app_db.bond_device[i].device_name));

                    app_db.bond_device[i].exist_addr_flag = 1;
                    app_db.bond_device[i].device_name_len = temp_bond_device->device_name_len;
                    APP_PRINT_INFO2("app_bt_bond_add_search_device exist_addr_flag %x, device_name_len =%x",
                                    app_db.bond_device[i].exist_addr_flag, app_db.bond_device[i].device_name_len);
                    app_db.bond_device[i].priority = app_bt_bond_get_num();
                    app_bt_bond_sort_priority(app_db.bond_device, MAX_BOND_INFO_NUM);
                    app_bt_bond_free_temp_cache_device();
                    app_bt_bond_temp_cache_save_to_search();
                    return true;
                }
            }
        }
    }
    else
    {
        APP_PRINT_INFO1("app_bt_bond_add_search_device: bd_addr %s ,error!!",
                        TRACE_BDADDR(bd_addr));
    }
    return false;
}

bool app_bt_bond_check_active_device_info_by_addr(uint8_t *bd_addr)
{
    for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].used && (!memcmp(bd_addr, app_db.bond_device[i].bd_addr, 6)))
        {
            return true;
        }
    }
    return false;
}

T_DEVICE_TYPE app_bt_bond_get_cod_type(uint8_t *bd_addr)
{
    T_APP_BOND_DEVICE *cod_info = NULL;

    cod_info = app_bt_bond_get_device_by_addr(bd_addr);
    if (NULL != cod_info)
    {
        return cod_info->device_type;
    }
    return T_DEVICE_TYPE_DEFAULT;
}

//cod change format
T_DEVICE_TYPE app_bt_bond_get_device_type(uint32_t cod)
{
    if ((cod & 0x1F00) >> 8 == 0x02)
    {
        return T_DEVICE_TYPE_PHONE;
    }
    else if ((cod & 0x1F00) >> 8 == 0x04)
    {
        return T_DEVICE_TYPE_EARPHONE;
    }
    return T_DEVICE_TYPE_DEFAULT;
}

//cod change format
uint32_t app_bt_bond_get_device_cod(T_DEVICE_TYPE type)
{
    uint32_t cod = 0x0000;

    if (T_DEVICE_TYPE_PHONE == type)
    {
        cod = 0x0200;
    }
    if (T_DEVICE_TYPE_EARPHONE == type)
    {
        cod = 0x0400;
    }
    if (T_DEVICE_TYPE_PHONE == type)
    {
        cod = 0xff00;
    }
    return cod;
}

//get access addr
static T_APP_BOND_DEVICE *app_bt_bond_get_free_temp_search_device_info(void)
{
    for (uint8_t i = 0; i < TEMP_MAX_SEARCH; i++)
    {
        if (temp_search_device[i].used == false)
        {
            return &temp_search_device[i];
        }
    }
    return NULL;
}

//add search device
uint8_t app_bt_bond_add_temp_search_device(uint8_t *bd_addr, uint32_t cod, uint16_t *name,
                                           uint8_t len)
{
    APP_PRINT_TRACE0("app_bt_bond_add_temp_search_device start");
    uint8_t ret = 1;
    uint8_t flag_temp = 0;
    T_APP_BOND_DEVICE *temp_search_info = NULL;

    temp_search_info = app_bt_bond_get_temp_search_device(bd_addr);

    if (temp_search_info == NULL)
    {
        temp_search_info = app_bt_bond_get_free_temp_search_device_info();
    }

    if (temp_search_info)
    {
        temp_search_info->used = true;
        memcpy(temp_search_info->bd_addr, bd_addr, 6);
        temp_search_info->device_type = app_bt_bond_get_device_type(cod);
        memcpy((uint8_t *)temp_search_info->device_name, (uint8_t *)name,
               sizeof(temp_search_info->device_name));
        temp_search_info->exist_addr_flag = flag_temp;
        temp_search_info->device_name_len = len;
        ret = 0;
    }

    return ret;
}

bool app_bt_bond_add_temp_search_device_name_info(uint8_t *bd_addr, uint16_t *name, uint8_t len)
{
    APP_PRINT_TRACE0("app_bt_bond_add_temp_search_device_name_info");

    T_APP_BOND_DEVICE *temp_bond_device = NULL;
    temp_bond_device = app_bt_bond_get_temp_search_device(bd_addr);
    if (temp_bond_device != NULL)
    {
        if (len == 0)
        {
            return false;
        }
        memcpy((uint8_t *)temp_bond_device->device_name, (uint8_t *)name,
               sizeof(temp_bond_device->device_name));
        temp_bond_device->device_name_len = len;

        return true;
    }
    else
    {
        APP_PRINT_INFO0("app_bt_bond_atsd_name_info error: no temp device searched");
        return false;
    }
}

//get search device info
T_APP_BOND_DEVICE *app_bt_bond_get_temp_search_device(uint8_t *bd_addr)
{
    for (uint8_t i = 0; i < TEMP_MAX_SEARCH; i++)
    {
        if (temp_search_device[i].used && !memcmp(temp_search_device[i].bd_addr, bd_addr, 6))
        {
            return &temp_search_device[i];
        }
    }
    return NULL;
}

//get cache device info
T_APP_BOND_DEVICE *app_bt_bond_get_temp_cache_device(uint8_t *bd_addr)
{
    for (uint8_t i = 0; i < TEMP_MAX_SEARCH; i++)
    {
        if (temp_cache_device[i].used && !memcmp(temp_cache_device[i].bd_addr, bd_addr, 6))
        {
            return &temp_cache_device[i];
        }
    }
    return NULL;
}

//get cache addr
static T_APP_BOND_DEVICE *app_bt_bond_get_free_temp_cache_device_info(void)
{
    for (uint8_t i = 0; i < TEMP_MAX_SEARCH; i++)
    {
        if (temp_cache_device[i].used == false)
        {
            return &temp_cache_device[i];
        }
    }
    return NULL;
}

//add cache device
uint8_t app_bt_bond_add_temp_cache_device(uint8_t *bd_addr, uint32_t cod, uint16_t *name,
                                          uint8_t len)
{
    APP_PRINT_TRACE0("app_bt_bond_add_temp_cache_device start");
    uint8_t ret = 1;
    uint8_t flag_temp = 0;
    T_APP_BOND_DEVICE *temp_cache_info = NULL;

    temp_cache_info = app_bt_bond_get_temp_cache_device(bd_addr);

    if (temp_cache_info == NULL)
    {
        temp_cache_info = app_bt_bond_get_free_temp_cache_device_info();
    }

    if (temp_cache_info)
    {
        temp_cache_info->used = true;
        memcpy(temp_cache_info->bd_addr, bd_addr, 6);
        temp_cache_info->device_type = app_bt_bond_get_device_type(cod);
        memcpy((uint8_t *)temp_cache_info->device_name, (uint8_t *)name,
               sizeof(temp_cache_info->device_name));
        temp_cache_info->exist_addr_flag = flag_temp;
        temp_cache_info->device_name_len = len;
        ret = 0;
    }

    return ret;
}

//get cache device info
T_APP_BOND_DEVICE *app_bt_bond_get_temp_cache_device_by_index(uint8_t index)
{
    if (temp_cache_device[index].used)
    {
        return &temp_cache_device[index];
    }
    else
    {
        return NULL;
    }
}

//free all cache device
void app_bt_bond_free_temp_cache_device(void)
{
    APP_PRINT_TRACE0("app_bt_bond_free_temp_cache_device start");
    for (int i = 0; i < TEMP_MAX_SEARCH; i++)
    {
        temp_cache_device[i].used = false;
    }
}

void app_bt_bond_temp_cache_save_to_search(void)
{
    memcpy(temp_search_device, temp_cache_device, sizeof(temp_cache_device));
}

int app_bt_bond_get_hightest_device(T_DEVICE_TYPE type)
{
    int temp_priority = -1;
    int temp_index = -1;

    for (int i = 0; i < MAX_BOND_INFO_NUM; i++)
    {
        if (app_db.bond_device[i].device_type == type && app_db.bond_device[i].priority > temp_priority &&
            app_db.bond_device[i].exist_addr_flag)
        {
            temp_index = i;
            temp_priority = app_db.bond_device[i].priority;
        }
    }
    return temp_index;
}

bool app_bt_bond_linkback(void)
{
    bool ret = false;
    int bond_num;
    uint32_t bond_flag;
    uint32_t plan_profs;
    T_LINKBACK_RETRY_PARAM retry_param =
    {
        .conn_retry_timeout = 0,
        .conn_retry_cnt = 3,
        .prof_retry_timeout = 0,
        .prof_retry_cnt = 0,
        .delay_timeout = 0
    };

    linkback_todo_queue_delete_all_node();

    bond_num = app_bt_bond_get_hightest_device(T_DEVICE_TYPE_PHONE);
    APP_PRINT_INFO1("linkback_load_bond_list phone bond_num = %d", bond_num);
    if (bond_num >= 0)
    {
        bond_flag = 0;
        bt_bond_flag_get(app_db.bond_device[bond_num].bd_addr, &bond_flag);
        if (bond_flag & (BOND_FLAG_HFP | BOND_FLAG_HSP))
        {
            plan_profs = (HFP_PROFILE_MASK);
            linkback_todo_queue_insert_normal_node(app_db.bond_device[bond_num].bd_addr, plan_profs,
                                                   T_DEVICE_TYPE_PHONE, retry_param);
            ret = true;
        }
    }

    bond_num = app_bt_bond_get_hightest_device(T_DEVICE_TYPE_EARPHONE);
    APP_PRINT_INFO1("linkback_load_bond_list ear bond_num = %d", bond_num);
    if (bond_num >= 0)
    {
        bond_flag = 0;
        bt_bond_flag_get(app_db.bond_device[bond_num].bd_addr, &bond_flag);
        if (bond_flag & BOND_FLAG_A2DP)
        {
            plan_profs = (A2DP_PROFILE_MASK);
            linkback_todo_queue_insert_normal_node(app_db.bond_device[bond_num].bd_addr, plan_profs,
                                                   T_DEVICE_TYPE_EARPHONE, retry_param);
            ret = true;
        }
    }

    linkback_todo_queue_all_node();
    return ret;
}

#if (F_APP_AUTO_SUPPORT == 1)
T_APP_BOND_DEVICE *app_find_br_addr(uint8_t *bd_addr)
{
    T_APP_BOND_DEVICE *temp_search = NULL;
    uint8_t        i;
    if (bd_addr != NULL)
    {
        for (i = 0; i < MAX_BOND_INFO_NUM; i++)
        {
            APP_PRINT_INFO1("address = %s", TRACE_BDADDR(bd_addr));
            if (app_db.bond_device[i].used && (memcmp(app_db.bond_device[i].bd_addr, bd_addr, 6)))
            {
                temp_search = &app_db.bond_device[i];
                APP_PRINT_INFO7("app_db.bond_device[i] info: bd_addr %s, i %d, device type %d ,used %d ,priority %d, exist_addr_flag %x, device_name_len = %x",
                                TRACE_BDADDR(app_db.bond_device[i].bd_addr), i, app_db.bond_device[i].device_type,
                                app_db.bond_device[i].used, app_db.bond_device[i].priority, app_db.bond_device[i].exist_addr_flag,
                                app_db.bond_device[i].device_name_len);
                break;
            }
        }
    }
    return temp_search;
}
#endif

void app_bond_bt_bond_clear(void)
{
    app_db.is_bond_clear = false;

    if ((app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_PHONE) != 0xff) ||
        (app_bt_bond_get_active_num_by_type(T_DEVICE_TYPE_EARPHONE) != 0xff))
    {
        app_db.is_bond_clear = true;

        // delete bond info after link disconnected
        app_bt_policy_enter_state(STATE_INIT);
    }
    else
    {
        if (RtkWristbandSys.gap_conn_state == GAP_CONN_STATE_DISCONNECTED)
        {
            /*delete ble bond info*/
            bt_le_clear_all_keys();
        }

        /*delete bt bond info*/
        for (uint8_t i = 0; i < MAX_BOND_INFO_NUM; i++)
        {
            if (app_db.bond_device[i].exist_addr_flag)
            {
                bt_bond_delete(app_db.bond_device[i].bd_addr);
                app_bt_bond_del_bond_device(app_db.bond_device[i].bd_addr);
            }
        }
        app_bt_bond_save_device_info_to_ftl(app_db.bond_device);
    }

    APP_PRINT_INFO1("app_bond_bt_bond_clear is_bond_clear = %d", app_db.is_bond_clear);
}
