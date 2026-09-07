/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <stdlib.h>
#include "os_mem.h"
#include "btm.h"
#include "trace.h"
#include "eq.h"
#include "eq_utils.h"

#if F_APP_USER_EQ_SUPPORT
#include "eq_ext_ftl.h"
#include "storage.h"
#endif

#define EQ_MSB_IDX                  (10)

#define EQ_BIT(_n)                  (uint32_t)(1U << (_n))

#define EQ_MIN_INDEX                0

#define EQ_SR_8K                    (0)
#define EQ_SR_16K                   (1)
#define EQ_SR_32K                   (2)
#define EQ_SR_44K                   (3)
#define EQ_SR_48K                   (4)
#define EQ_SR_88K                   (5)
#define EQ_SR_96K                   (6)
#define EQ_SR_192K                  (7)
#define EQ_SR_12K                   (8)
#define EQ_SR_24K                   (9)
#define EQ_SR_11K                   (10)
#define EQ_SR_22K                   (11)
#define EQ_SR_ALL                   (15)

typedef struct t_eq_db
{
    uint8_t normal_eq_map[EQ_MSB_IDX];      /**< index is T_EQ_IDX  spk eq*/
    uint8_t gaming_eq_map[EQ_MSB_IDX];      /**< index is T_EQ_IDX  spk eq*/
    uint8_t anc_eq_map[EQ_MSB_IDX];         /**< index is T_EQ_IDX  spk eq*/
    uint8_t apt_eq_map[EQ_MSB_IDX];         /**< index is T_EQ_IDX  mic eq*/
#if F_APP_LINEIN_SUPPORT
    uint8_t line_in_eq_map[EQ_MSB_IDX];     /**< index is T_EQ_IDX  line in eq*/
#endif
#if F_APP_VOICE_SPK_EQ_SUPPORT
    uint8_t voice_spk_eq_map[EQ_MSB_IDX];  /**< index is T_EQ_IDX  voice spk eq*/
#endif
#if F_APP_VOICE_MIC_EQ_SUPPORT
    uint8_t voice_mic_eq_map[EQ_MSB_IDX];  /**< index is T_EQ_IDX  voice mic eq*/
#endif
    uint8_t normal_eq_num;                  /**< the num of eq group */
    uint8_t gaming_eq_num;                  /**< the num of eq group */
    uint8_t anc_eq_num;                     /**< the num of eq group */
    uint8_t apt_eq_num;                     /**< the num of eq group */
#if F_APP_LINEIN_SUPPORT
    uint8_t line_in_eq_num;                 /**< the num of eq group */
#endif
#if F_APP_VOICE_SPK_EQ_SUPPORT
    uint8_t voice_spk_eq_num;
#endif
#if F_APP_VOICE_MIC_EQ_SUPPORT
    uint8_t voice_mic_eq_num;
#endif
} T_EQ_DB;

static T_EQ_DB *eq_db;
static uint32_t spk_eq_idx_bitmask[SPK_EQ_MODE_MAX];
static uint32_t mic_eq_idx_bitmask[MIC_EQ_MODE_MAX];

uint8_t eq_utils_stage_num_get(uint16_t eq_len)
{
    uint8_t stage_num = 0;

    stage_num = ((eq_len - EQ_HEAD) / EQ_COEFF);

    APP_PRINT_TRACE1("eq_utils_stage_num_get: stage_num %d", stage_num);

    return stage_num;
}

#if F_APP_USER_EQ_SUPPORT
uint32_t eq_utils_save_eq_to_ftl(uint32_t offset, uint8_t *data, uint32_t len)
{
    return storage_write(EQ_EXT_FTL_PARTITION_NAME, offset, len, data, NULL, NULL);
}

uint32_t eq_utils_load_eq_from_ftl(uint32_t offset, uint8_t *data, uint32_t len)
{
    return storage_read(EQ_EXT_FTL_PARTITION_NAME, offset, len, data, NULL, NULL);
}
#endif

uint8_t eq_utils_num_get(T_EQ_TYPE eq_type, uint8_t mode)
{
    if (eq_db != NULL)
    {
        uint8_t map[EQ_TYPE_MAX][SPK_EQ_MODE_MAX] =
        {
            {eq_db->normal_eq_num, eq_db->gaming_eq_num, eq_db->anc_eq_num, 0, 0},
            {eq_db->apt_eq_num, 0, 0, 0, 0}
        };

#if F_APP_LINEIN_SUPPORT
        map[SPK_SW_EQ][LINE_IN_MODE] = eq_db->line_in_eq_num;
#endif

#if F_APP_VOICE_SPK_EQ_SUPPORT
        map[SPK_SW_EQ][VOICE_SPK_MODE] = eq_db->voice_spk_eq_num;
#endif

#if F_APP_VOICE_MIC_EQ_SUPPORT
        map[MIC_SW_EQ][VOICE_MIC_MODE] = eq_db->voice_mic_eq_num;
#endif

        return map[eq_type][mode];
    }

    return 0;
}

static uint8_t *eq_utils_map_get(T_EQ_TYPE eq_type, uint8_t mode)
{
    if (eq_db != NULL)
    {
        uint8_t *map[EQ_TYPE_MAX][SPK_EQ_MODE_MAX] =
        {
            {eq_db->normal_eq_map, eq_db->gaming_eq_map, eq_db->anc_eq_map, NULL, NULL},
            {eq_db->apt_eq_map, NULL, NULL, NULL, NULL}
        };

#if F_APP_LINEIN_SUPPORT
        map[SPK_SW_EQ][LINE_IN_MODE] = eq_db->line_in_eq_map;
#endif

#if F_APP_VOICE_SPK_EQ_SUPPORT
        map[SPK_SW_EQ][VOICE_SPK_MODE] = eq_db->voice_spk_eq_map;
#endif

#if F_APP_VOICE_MIC_EQ_SUPPORT
        map[MIC_SW_EQ][VOICE_MIC_MODE] = eq_db->voice_mic_eq_map;
#endif

        return map[eq_type][mode];
    }

    return NULL;
}

uint8_t eq_utils_original_eq_index_get(T_EQ_TYPE eq_type, uint8_t eq_mode, uint8_t index)
{
    uint8_t eq_index = index;

    if ((eq_type != EQ_TYPE_MAX) && (eq_mode != EQ_MODE_NULL))
    {
        uint8_t *map = eq_utils_map_get(eq_type, eq_mode);

        if (map)
        {
            eq_index = map[index];
        }
    }

    return eq_index;
}

uint32_t eq_utils_get_capacity(T_EQ_TYPE type, uint8_t mode)
{
    uint32_t bitmask = 0;

    if (type == SPK_SW_EQ)
    {
        bitmask = spk_eq_idx_bitmask[mode];
    }
    else if (type == MIC_SW_EQ)
    {
        bitmask = mic_eq_idx_bitmask[mode];
    }

    return bitmask;
}

static uint8_t eq_utils_map_init(T_EQ_TYPE eq_type, uint8_t mode)
{
    uint32_t eq_capacity = 0;
    uint8_t i;
    uint8_t eq_num = 0;
    uint8_t *map = eq_utils_map_get(eq_type, mode);

    eq_capacity = eq_utils_get_capacity(eq_type, mode);

    if (eq_capacity == 0 || map == NULL)
    {
        APP_PRINT_ERROR4("eq_utils_map_init: init fail, type %d, mode %d, capacity %d, map %p",
                         eq_type, mode, eq_capacity, map);
        return 0;
    }

    for (i = 0; i < EQ_GROUP_NUM; ++i)
    {
        if (eq_capacity & EQ_BIT(i))
        {
            map[eq_num] = i;
            eq_num++;
        }
    }

    return eq_num;
}

bool eq_utils_init(void)
{
    eq_db = calloc(1, sizeof(T_EQ_DB));

    if (eq_db == NULL)
    {
        APP_PRINT_ERROR0("eq_utils_init: eq init fail, calloc fail");
        return false;
    }

#if F_APP_USER_EQ_SUPPORT
    eq_ext_ftl_storage_init();
#endif

    eq_db->normal_eq_num = eq_utils_map_init(SPK_SW_EQ, NORMAL_MODE);
    eq_db->gaming_eq_num = eq_utils_map_init(SPK_SW_EQ, GAMING_MODE);

#if F_APP_ANC_SUPPORT
    eq_db->anc_eq_num = eq_utils_map_init(SPK_SW_EQ, ANC_MODE);
#endif

#if F_APP_APT_SUPPORT
    eq_db->apt_eq_num = eq_utils_map_init(MIC_SW_EQ, APT_MODE);
#endif

#if F_APP_LINEIN_SUPPORT
    eq_db->line_in_eq_num = eq_utils_map_init(SPK_SW_EQ, LINE_IN_MODE);
#endif

#if F_APP_VOICE_SPK_EQ_SUPPORT
    eq_db->voice_spk_eq_num = eq_utils_map_init(SPK_SW_EQ, VOICE_SPK_MODE);
#endif

#if F_APP_VOICE_MIC_EQ_SUPPORT
    eq_db->voice_mic_eq_num = eq_utils_map_init(MIC_SW_EQ, VOICE_MIC_MODE);
#endif

    return true;
}

bool eq_utils_deinit(void)
{
    if (eq_db != NULL)
    {
        free(eq_db);
    }

    return true;
}

void eq_utils_bitmask_init(T_EQ_TYPE eq_type, uint8_t mode, uint32_t bitmask)
{
    if (eq_type == SPK_SW_EQ)
    {
        spk_eq_idx_bitmask[mode] = bitmask;
    }
    else if (eq_type == MIC_SW_EQ)
    {
        mic_eq_idx_bitmask[mode] = bitmask;
    }
}
