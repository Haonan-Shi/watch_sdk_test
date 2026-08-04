/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if F_APP_MALLEUS_SUPPORT
#include "audio_vse.h"
#include "system_status_api.h"
#include "fmc_api.h"
#include "audio.h"
#include "ftl.h"
#include "sysm.h"
#include "os_sched.h"
#include "malleus.h"
#include "app_timer.h"

#define EFFECT_UNKNOWN                        (0xFF)
#define INT16_SIGN_NIT                        (1 << 15)

#define MALLEUS_EFFECT_EFFECT_ID              (0)
#define MALLEUS_EFFECT_DATA_TYPE              (0xFF)
#define MALLEUS_EFFECT_VERSION                (0x00)

#define MALLEUS_LTV_VALUE_SECP_OCTETS         (244)

#define MALLEUS_LTV_VER_OCTETS                (1)
#define MALLEUS_LTV_ENTITY_OCTETS             (1)
#define MALLEUS_LTV_CTL_OCTETS                (1)
#define MALLEUS_LTV_CMD_OCTETS                (2)
#define MALLEUS_LTV_VALUE_PRE_OCTETS          (MALLEUS_LTV_CMD_OCTETS + MALLEUS_LTV_VER_OCTETS + MALLEUS_LTV_ENTITY_OCTETS + MALLEUS_LTV_CTL_OCTETS)

#define MALLEUS_LTV_LEN_OCTETS                (1)
#define MALLEUS_LTV_TYPE_OCTETS               (1)

#define MALLEUS_SECP_PACKET_NUM               (17)
#define MALLEUS_SECP_TOTAL_OCTETS             (244 * MALLEUS_SECP_PACKET_NUM)

#define MALLEUS_LTV_MIN_OCTETS                (MALLEUS_LTV_LEN_OCTETS + MALLEUS_LTV_TYPE_OCTETS + MALLEUS_LTV_VALUE_PRE_OCTETS)

#define MALLEUS_LTV_RESP_ERR_OCTETS           (1)
#define MALLEUS_LTV_RESP_PRE_OCTETS           (MALLEUS_LTV_VALUE_PRE_OCTETS + MALLEUS_LTV_RESP_ERR_OCTETS)
#define MALLEUS_LTV_RESP_OCTETS               (MALLEUS_LTV_TYPE_OCTETS + MALLEUS_LTV_RESP_PRE_OCTETS)
#define MALLEUS_LTV_RESP_MIN_OCTETS           (MALLEUS_LTV_LEN_OCTETS + MALLEUS_LTV_RESP_OCTETS)

#define MALLEUS_EFFECT_SAMPLE_RATE            (48000)
#define MALLEUS_EFFECT_SAMPLE_RATE_END        (44100)

#define MALLEUS_EFFECT_KEY_UUID_ADDR          (0x02001100)
#define MALLEUS_EFFECT_KEY_EFUSE_INFO_ADDR    (0xAE)

#define MALLEUS_EFFECT_KEY_UUID_MAGIC         (0x5AA5)

#define MALLEUS_EFFECT_KEY_BT_MAC_LEN         (6)
#define MALLEUS_EFFECT_KEY_UUID_BASE_LEN      (14)
#define MALLEUS_EFFECT_KEY_UUID_MAGIC_LEN     (2)
#define MALLEUS_EFFECT_KEY_UUID_LEN           (MALLEUS_EFFECT_KEY_UUID_BASE_LEN+MALLEUS_EFFECT_KEY_UUID_MAGIC_LEN)

#define MALLEUS_EFFECT_KEY_LICENSE_LEN        (128)
#define MALLEUS_EFFECT_KEY_EFUSE_INFO_LEN     (2)

#define MALLEUS_EFFECT_KEY_TOTAL_LEN          (MALLEUS_EFFECT_KEY_BT_MAC_LEN+MALLEUS_EFFECT_KEY_UUID_LEN+MALLEUS_EFFECT_KEY_LICENSE_LEN+MALLEUS_EFFECT_KEY_EFUSE_INFO_LEN)

enum
{
    MALLEUS_ENTITY_NULL               = 0x00,
    MALLEUS_ENTITY_DSP1               = 0x01,
    MALLEUS_ENTITY_DSP2               = 0x02,
};

enum
{
    MALLEUS_CTL_SINGLE                = 0x00,
    MALLEUS_CTL_START                 = 0x01,
    MALLEUS_CTL_CONTINUE              = 0x02,
    MALLEUS_CTL_END                   = 0x03,
};

typedef enum
{
    MALLEUS_CMD_SECP_START            = 0xA0,
    MALLEUS_CMD_SECP_DAT              = 0xA1,
    MALLEUS_CMD_SECP_CHANNEL          = 0xA2,
    MALLEUS_CMD_SECP_KEY              = 0xA3,
    MALLEUS_CMD_SECP_RSV              = 0xA4,
    MALLEUS_CMD_LOAD_NO_EFFECT        = 0xA5,
    MALLEUS_CMD_LOAD_AUDIO_SPACE      = 0xA6,
    MALLEUS_CMD_LOAD_SECURITY_KEY     = 0xA7,
    MALLEUS_CMD_NOTIFY_VER_INFO       = 0xA8,
    MALLEUS_CMD_SPATIAL_AUDIO         = 0xA9,
    MALLEUS_CMD_SECP_END              = 0xAE,
} T_MALLEUS_CMD;

typedef union
{
    uint8_t  data[MALLEUS_LTV_MIN_OCTETS + MALLEUS_LTV_VALUE_SECP_OCTETS];
    struct
    {
        uint8_t len;
        uint8_t type;
        union
        {
            uint8_t value[MALLEUS_LTV_VALUE_PRE_OCTETS + MALLEUS_LTV_VALUE_SECP_OCTETS];
            struct
            {
                uint8_t value_version;
                uint8_t value_entity;
                uint8_t value_control;
                uint8_t value_cmd;
                uint8_t value_cmd_rsv;
                uint8_t value_dat[MALLEUS_LTV_VALUE_SECP_OCTETS];
            };
        };
    };
} T_MALLEUS_DATA;

typedef struct
{
    uint16_t num_effect;
    uint16_t effect_type;
    uint16_t changed_effect;
    uint8_t err_code;
    uint8_t normal_cycle;
    uint8_t gaming_cycle;
    uint16_t effect_offset_48;
    uint32_t secp_table_sended_bytes;

    bool is_online_tune;
    uint16_t effect_offset_tune;

    uint8_t cmd_path;
    uint8_t app_idx;

    T_MALLEUS_POS prev_pos;
    T_MALLEUS_POS cur_pos;
    uint32_t last_anchor;
    uint8_t report_count;
    bool can_send;
    bool get_data;
} T_MALLEUS_DB;

T_MALLEUS malleus = {0};
static T_MALLEUS_DB malleus_db = {0};
static T_MALLEUS_DATA *malleus_data = NULL;
static uint16_t malleus_key_val[MALLEUS_KEY_VAL_LEN] = {0};
static uint8_t malleus_ver_info[MALLEUS_VER_INFO_LEN] = {0};
static T_MALLEUS_CH malleus_channel = MALLEUS_CH_L_R_DIV2;
static T_AUDIO_EFFECT_INSTANCE malleus_instance = NULL;

static uint8_t malleus_timer_id = 0;
static uint8_t timer_idx_fake_report = 0;
typedef enum
{
    APP_TIMER_FAKE_REPORT,
} T_APP_MALLEUS_TIMER_ID;

static uint8_t malleus_get_role(void)
{
    if (malleus.role != NULL)
    {
        return *malleus.role;
    }
    else
    {
        return REMOTE_SESSION_ROLE_SINGLE;
    }
}

static bool malleus_b2b_is_conn(void)
{
    if (malleus.b2b_conn != NULL)
    {
        return malleus.b2b_conn();
    }

    return false;
}

static void malleus_send_data(T_MALLEUS_CMD cmd, T_MALLEUS_DATA **p_malleus_data, uint16_t len)
{
    static uint16_t seq_num = 0;

    APP_PRINT_INFO2("malleus_send_data: cmd 0x%02x len %d", cmd, len);

    if (*p_malleus_data == NULL)
    {
        return;
    }

    if (malleus_instance != NULL)
    {
        (*p_malleus_data)->len = len + MALLEUS_LTV_TYPE_OCTETS + MALLEUS_LTV_VALUE_PRE_OCTETS -
                                 MALLEUS_LTV_MIN_OCTETS;
        (*p_malleus_data)->type = MALLEUS_EFFECT_DATA_TYPE;
        (*p_malleus_data)->value_version = MALLEUS_EFFECT_VERSION;
        (*p_malleus_data)->value_entity = MALLEUS_ENTITY_DSP1;
        (*p_malleus_data)->value_control = MALLEUS_CTL_SINGLE;
        (*p_malleus_data)->value_cmd = cmd;
        (*p_malleus_data)->value_cmd_rsv = 0;

        audio_vse_apply(malleus_instance, seq_num++, (*p_malleus_data)->data,
                        ((*p_malleus_data)->len + MALLEUS_LTV_LEN_OCTETS));
    }

    free(*p_malleus_data);
    *p_malleus_data = NULL;
}

static void malleus_spatial_atti_data_map(int16_t *x, int16_t *y, int16_t *z)
{
    int16_t   temp_x = *x, temp_y = *y, temp_z = *z;
    switch (malleus.spatial_pos)
    {
    case MALLEUS_POS_Z_UP_X_RIGHT:
        *x = -temp_x;
        *y =  temp_z;
        *z =  temp_y;
        break;

    case MALLEUS_POS_Z_UP_X_DOWN:
        *x = -temp_y;
        *y =  temp_z;
        *z = -temp_x;
        break;

    case MALLEUS_POS_Z_UP_X_LEFT:
        *x =  temp_x;
        *y =  temp_z;
        *z = -temp_y;
        break;

    case MALLEUS_POS_Z_UP_X_UP:
        *x =  temp_y;
        *y =  temp_z;
        *z =  temp_x;
        break;

    case MALLEUS_POS_Z_DOWN_X_RIGHT:
        *x = -temp_x;
        *y = -temp_z;
        *z = -temp_y;
        break;

    case MALLEUS_POS_Z_DOWN_X_DOWN:
        *x =  temp_y;
        *y = -temp_z;
        *z = -temp_x;
        break;

    case MALLEUS_POS_Z_DOWN_X_LEFT:
        *x =  temp_x;
        *y = -temp_z;
        *z =  temp_y;
        break;

    case MALLEUS_POS_Z_DOWN_X_UP:
        *x = -temp_y;
        *y = -temp_z;
        *z =  temp_x;
        break;

    default:
        break;
    }
}

static void malleus_spatial_update_pos(T_MALLEUS_POS pos, bool real)
{
    T_MALLEUS_POS temp;

    if (!malleus_db.can_send)
    {
        app_stop_timer(&timer_idx_fake_report);
        return;
    }

    if (!malleus_db.get_data)
    {
        malleus_db.get_data = true;
        memcpy(&malleus_db.cur_pos, &pos, sizeof(T_MALLEUS_POS));
        app_stop_timer(&timer_idx_fake_report);
        return;
    }

    if (real)
    {
        if (malleus_get_role() == REMOTE_SESSION_ROLE_SECONDARY)
        {
            pos.horz *= -1;
        }

        memcpy(&malleus_db.prev_pos, &malleus_db.cur_pos, sizeof(T_MALLEUS_POS));
        memcpy(&malleus_db.cur_pos, &pos, sizeof(T_MALLEUS_POS));
        memcpy(&temp, &malleus_db.prev_pos, sizeof(T_MALLEUS_POS));
    }
    else
    {
        memcpy(&temp, &pos, sizeof(T_MALLEUS_POS));
    }

    if (malleus_get_role() != REMOTE_SESSION_ROLE_SECONDARY)
    {
        if (malleus.spatial_status == SPATIAL_AUDIO_CLOSE)
        {
            app_stop_timer(&timer_idx_fake_report);
            return;
        }
    }

    malleus_data = (T_MALLEUS_DATA *)calloc(1, (MALLEUS_LTV_MIN_OCTETS + 6));

    if (malleus_data != NULL)
    {
        malleus_data->value_dat[0] = temp.horz & 0xFF;
        malleus_data->value_dat[1] = (temp.horz >> 8) & 0xFF;
        malleus_data->value_dat[2] = temp.vert & 0xFF;
        malleus_data->value_dat[3] = (temp.vert >> 8) & 0xFF;
        malleus_data->value_dat[4] = temp.factor & 0xFF;
        malleus_data->value_dat[5] = (temp.factor >> 8) & 0xFF;

        malleus_send_data(MALLEUS_CMD_SPATIAL_AUDIO, &malleus_data, (MALLEUS_LTV_MIN_OCTETS + 6));
    }

    if (real)
    {
        malleus_db.report_count = 3;
        malleus_db.last_anchor = os_sys_time_get();
        if (timer_idx_fake_report == 0)
        {
            app_start_timer(&timer_idx_fake_report, "fake_report", malleus_timer_id, APP_TIMER_FAKE_REPORT, 0,
                            true, 10);
        }
    }
}

static uint8_t malleus_get_gaming_mode(void)
{
    if (malleus.gaming_mode != NULL)
    {
        return *malleus.gaming_mode;
    }
    else
    {
        return 0;
    }
}

static void malleus_set_gaming_mode(uint8_t mode)
{
    if (malleus.gaming_mode != NULL)
    {
        *malleus.gaming_mode = mode;
    }
}

static uint8_t malleus_get_gaming_type(void)
{
    if (malleus.gaming_type != NULL)
    {
        return *malleus.gaming_type;
    }
    else
    {
        return 0;
    }
}

static void malleus_set_normal_type(uint8_t type)
{
    if (malleus.normal_type != NULL)
    {
        *malleus.normal_type = type;
    }
}

static uint8_t malleus_get_normal_type(void)
{
    if (malleus.normal_type != NULL)
    {
        return *malleus.normal_type;
    }
    else
    {
        return 0;
    }
}

static void malleus_set_gaming_type(uint8_t type)
{
    if (malleus.gaming_type != NULL)
    {
        *malleus.gaming_type = type;
    }
}

static void malleus_report_event(uint8_t *data, uint16_t len)
{
    if (malleus.report_event != NULL)
    {
        malleus.report_event(malleus_db.cmd_path, malleus_db.app_idx, data, len);
    }
}

static uint8_t malleus_mode_valid_check(uint8_t effect_mode)
{
    return (effect_mode >= malleus_db.num_effect) ? MALLEUS_MUSIC_MODE : effect_mode;
}

static void malleus_send_license(void)
{
    uint8_t *uuid = sys_hall_get_ic_euid();
    uint8_t efuse_info[MALLEUS_EFFECT_KEY_EFUSE_INFO_LEN] = {0};

    if (malleus.factory_addr == NULL)
    {
        return;
    }

    malleus_data = (T_MALLEUS_DATA *)calloc(1, (MALLEUS_LTV_MIN_OCTETS + MALLEUS_EFFECT_KEY_TOTAL_LEN));

    if (malleus_data == NULL)
    {
        return;
    }

    memcpy(&malleus_data->value_dat[0], malleus.factory_addr, MALLEUS_EFFECT_KEY_BT_MAC_LEN);

    memcpy(&malleus_data->value_dat[MALLEUS_EFFECT_KEY_BT_MAC_LEN], uuid,
           MALLEUS_EFFECT_KEY_UUID_BASE_LEN);
    malleus_data->value_dat[MALLEUS_EFFECT_KEY_BT_MAC_LEN + MALLEUS_EFFECT_KEY_UUID_BASE_LEN]
        = MALLEUS_EFFECT_KEY_UUID_MAGIC & 0xFF;
    malleus_data->value_dat[MALLEUS_EFFECT_KEY_BT_MAC_LEN + MALLEUS_EFFECT_KEY_UUID_BASE_LEN + 1]
        = MALLEUS_EFFECT_KEY_UUID_MAGIC >> 8;

    fmc_flash_nor_read(MALLEUS_EFFECT_KEY_UUID_ADDR,
                       &malleus_data->value_dat[MALLEUS_EFFECT_KEY_BT_MAC_LEN + MALLEUS_EFFECT_KEY_UUID_LEN],
                       MALLEUS_EFFECT_KEY_LICENSE_LEN);

    read_efuse_on_ram(MALLEUS_EFFECT_KEY_EFUSE_INFO_ADDR, MALLEUS_EFFECT_KEY_EFUSE_INFO_LEN,
                      efuse_info);
    memcpy(&malleus_data->value_dat[MALLEUS_EFFECT_KEY_BT_MAC_LEN + MALLEUS_EFFECT_KEY_UUID_LEN +
                                                                  MALLEUS_EFFECT_KEY_LICENSE_LEN], efuse_info, MALLEUS_EFFECT_KEY_EFUSE_INFO_LEN);

    malleus_send_data(MALLEUS_CMD_LOAD_SECURITY_KEY, &malleus_data,
                      (MALLEUS_LTV_MIN_OCTETS + MALLEUS_EFFECT_KEY_TOTAL_LEN));
}

static void malleus_send_offset_data(uint16_t offset)
{
    malleus_data = (T_MALLEUS_DATA *)calloc(1, (MALLEUS_LTV_MIN_OCTETS + 2));

    if (malleus_data != NULL)
    {
        malleus_data->value_dat[0] = offset & 0xFF;
        malleus_data->value_dat[1] = (offset >> 8) & 0xFF;
        malleus_send_data(MALLEUS_CMD_SECP_END, &malleus_data, (MALLEUS_LTV_MIN_OCTETS + 2));
    }
}

static void malleus_send_key_val(void)
{
    malleus_data = (T_MALLEUS_DATA *)calloc(1, (MALLEUS_LTV_MIN_OCTETS + MALLEUS_KEY_VAL_OCTETS));

    if (malleus_data != NULL)
    {
        memcpy(&malleus_data->value_dat[0], malleus_key_val, MALLEUS_KEY_VAL_OCTETS);

        malleus_send_data(MALLEUS_CMD_SECP_KEY, &malleus_data,
                          (MALLEUS_LTV_MIN_OCTETS + MALLEUS_KEY_VAL_OCTETS));
    }
}

static void malleus_send_secp_data(uint8_t *data, uint16_t len)
{
    if (len > MALLEUS_LTV_VALUE_SECP_OCTETS)
    {
        return;
    }

    malleus_data = (T_MALLEUS_DATA *)calloc(1, sizeof(T_MALLEUS_DATA));

    if (malleus_data != NULL)
    {
        memcpy(&malleus_data->value_dat[0], data, len);
        malleus_send_data(MALLEUS_CMD_SECP_DAT, &malleus_data, (MALLEUS_LTV_MIN_OCTETS + len));
    }
}

static uint16_t malleus_get_key_val(uint8_t idx)
{
    if (idx >= MALLEUS_KEY_VAL_LEN)
    {
        return 0x7fff;
    }

    return malleus_key_val[idx];
}

static void malleus_cmd_get_key_val(uint8_t *cmd_ptr, uint8_t *ack)
{
    uint8_t idx = cmd_ptr[0] | (cmd_ptr[1] << 8);
    uint8_t data[5] = {0};

    if (idx >= MALLEUS_KEY_VAL_LEN)
    {
        *ack = MALLEUS_CMD_STATUS_DISALLOW;
        return;
    }

    data[0] = MALLEUS_CMD_GET_KEY_VALUE;
    data[1] = cmd_ptr[0];
    data[2] = cmd_ptr[1];
    data[3] = malleus_get_key_val(idx) & 0xFF;
    data[4] = malleus_get_key_val(idx) >> 8;

    malleus_report_event(data, sizeof(data));
}

static void malleus_cmd_restart_ack(void)
{
    uint8_t data[2] = {0};

    data[0] = MALLEUS_CMD_TUNE_DATA_REQ;
    data[1] = MALLEUS_LTV_VALUE_SECP_OCTETS;
    malleus_report_event(data, sizeof(data));
}

static void malleus_cmd_get_version(void)
{
    uint8_t data[7] = {0};

    data[0] = MALLEUS_CMD_GET_INFO;
    memcpy(&data[1], malleus_ver_info, MALLEUS_VER_INFO_LEN);
    malleus_report_event(data, sizeof(data));
}

static void malleus_load_effect(void)
{
    APP_PRINT_INFO2("malleus_load_effect: changed_effect %d effect_type %d", malleus_db.changed_effect,
                    malleus_db.effect_type);

    if (malleus_instance == NULL)
    {
        return;
    }

    if (malleus_db.changed_effect == malleus_db.effect_type)
    {
        return;
    }

    malleus_db.changed_effect = malleus_db.effect_type;

    malleus_send_key_val();
}

void malleus_effect_set(void)
{
    malleus_db.effect_type = malleus_mode_valid_check(malleus_effect_mode_sel);
    malleus_load_effect();
}

static void malleus_effect_next_idx(void)
{
    uint8_t type_temp = 0;

    if (malleus_get_gaming_mode() != 0)
    {
        if (malleus_db.gaming_cycle == 0)
        {
            return;
        }

        do
        {
            type_temp = malleus_get_gaming_type();
            type_temp ++;
            type_temp %= 8;
            malleus_set_gaming_type(type_temp);
        }
        while ((malleus_db.gaming_cycle & (1 << malleus_get_gaming_type())) == 0);
    }
    else
    {
        if (malleus_db.normal_cycle == 0)
        {
            return;
        }

        do
        {
            type_temp = malleus_get_normal_type();
            type_temp ++;
            type_temp %= 8;
            malleus_set_normal_type(type_temp);
        }
        while ((malleus_db.normal_cycle & (1 << malleus_get_normal_type())) == 0);
    }
}

static uint8_t malleus_get_cycle_num(void)
{
    uint8_t count = 0, cycle = 0;

    cycle = (malleus_get_gaming_mode() != 0) ? malleus_db.gaming_cycle : malleus_db.normal_cycle;

    for (uint8_t i = 0; i < 8; i++)
    {
        if ((cycle & (1 << i)))
        {
            count ++;
        }
    }

    return count;
}

static void malleus_channel_out(T_MALLEUS_CH channel)
{
    malleus_data = (T_MALLEUS_DATA *)calloc(1, (MALLEUS_LTV_MIN_OCTETS + 1));

    if (malleus_data != NULL)
    {
        malleus_data->value_dat[0] = channel;
        malleus_send_data(MALLEUS_CMD_SECP_CHANNEL, &malleus_data, (MALLEUS_LTV_MIN_OCTETS + 1));
    }
}

static uint32_t malleus_save_key_val(void)
{
    return ftl_save_to_storage(malleus_key_val, malleus.key_flash_offset,
                               malleus.key_flash_len);
}

static void malleus_set_key_val(uint8_t idx, uint16_t val)
{
    if (idx >= MALLEUS_KEY_VAL_LEN)
    {
        return;
    }
    malleus_key_val[idx] = val;
}

static uint32_t malleus_load_key_val(void)
{
    return ftl_load_from_storage(malleus_key_val, malleus.key_flash_offset,
                                 malleus.key_flash_len);
}

static void malleus_relay_async(uint8_t msg, uint8_t *data, uint16_t len)
{
    if (malleus.relay_async != NULL)
    {
        malleus.relay_async(msg, data, len);
    }
}

static void malleus_relay_sync(uint8_t msg, uint8_t *data, uint16_t len)
{
    if (malleus.relay_sync != NULL)
    {
        malleus.relay_sync(msg, data, len);
    }
}

static void malleus_relay_key_val(void)
{
    malleus_relay_async(MALLEUS_SYNC_MSG_KEY_VAL, (uint8_t *)malleus_key_val, MALLEUS_KEY_VAL_OCTETS);
}

static void malleus_relay_effect_type(void)
{
    uint8_t data[4];
    data[0] = malleus_get_normal_type();
    data[1] = malleus_get_gaming_type();
    data[2] = malleus_get_gaming_mode();
    data[3] = malleus.spatial_status;

    malleus_relay_async(MALLEUS_SYNC_MSG_EFFECT_TYPE, data, sizeof(data));
}

static void malleus_audio_vse_parse(uint8_t *event_buff, uint16_t len)
{
    uint8_t ret = 0;
    uint32_t StartAddr = 0, NumDataInByte;

    switch (event_buff[3])
    {
    case MALLEUS_CMD_NOTIFY_VER_INFO:
        {
            if (len == (MALLEUS_VER_INFO_LEN + MALLEUS_LTV_VALUE_PRE_OCTETS))
            {
                memcpy(malleus_ver_info, &event_buff[MALLEUS_LTV_VALUE_PRE_OCTETS], MALLEUS_VER_INFO_LEN);
            }
        }
        break;

    case MALLEUS_CMD_SECP_KEY:
        {
            malleus_send_license();
        }
        break;

    case MALLEUS_CMD_LOAD_SECURITY_KEY:
        {
            malleus_data = (T_MALLEUS_DATA *)calloc(1, MALLEUS_LTV_MIN_OCTETS);

            if (malleus_data != NULL)
            {
                malleus_send_data(MALLEUS_CMD_SECP_START, &malleus_data, MALLEUS_LTV_MIN_OCTETS);
            }
        }
        break;

    case MALLEUS_CMD_SECP_START:
        {
            malleus_db.secp_table_sended_bytes = 0;

            if (malleus_db.is_online_tune)
            {
                uint8_t data[2] = {0};

                data[0] = MALLEUS_CMD_TUNE_DATA_REQ;
                data[1] = MALLEUS_LTV_VALUE_SECP_OCTETS;
                malleus_report_event(data, sizeof(data));
            }
            else
            {
                uint8_t ret = malleus_secp_get_start_addr(MALLEUS_EFFECT_SAMPLE_RATE, &malleus_db.num_effect,
                                                          &malleus_db.effect_type, &StartAddr);

                if ((ret == MALLEUS_SECP_SUC) && (StartAddr > 0))
                {
                    malleus_db.effect_offset_48 = StartAddr;
                    malleus_data = (T_MALLEUS_DATA *)calloc(1, sizeof(T_MALLEUS_DATA));

                    if (malleus_data != NULL)
                    {
                        malleus_secp_start_read(StartAddr, &malleus_data->value_dat[0], 0);
                        NumDataInByte = malleus_secp_continue_read(&malleus_data->value_dat[0],
                                                                   MALLEUS_LTV_VALUE_SECP_OCTETS);
                        malleus_send_data(MALLEUS_CMD_SECP_DAT, &malleus_data, (MALLEUS_LTV_MIN_OCTETS + NumDataInByte));
                    }
                }
            }
        }
        break;

    case MALLEUS_CMD_SECP_DAT:
        {
            malleus_db.secp_table_sended_bytes += MALLEUS_LTV_VALUE_SECP_OCTETS;
            /*check secp no data send*/
            if (malleus_db.secp_table_sended_bytes < MALLEUS_SECP_TOTAL_OCTETS)
            {
                if (malleus_db.is_online_tune)
                {
                    uint8_t data[2] = {0};

                    data[0] = MALLEUS_CMD_TUNE_DATA_REQ;
                    data[1] = MALLEUS_LTV_VALUE_SECP_OCTETS;
                    malleus_report_event(data, sizeof(data));
                }
                else
                {
                    malleus_data = (T_MALLEUS_DATA *)calloc(1, sizeof(T_MALLEUS_DATA));

                    if (malleus_data != NULL)
                    {
                        NumDataInByte = malleus_secp_continue_read(&malleus_data->value_dat[0],
                                                                   MALLEUS_LTV_VALUE_SECP_OCTETS);
                        malleus_send_data(MALLEUS_CMD_SECP_DAT, &malleus_data, (MALLEUS_LTV_MIN_OCTETS + NumDataInByte));
                    }
                }
            }
            else
            {
                malleus_db.secp_table_sended_bytes = 0;

                if (malleus_db.is_online_tune)
                {
                    malleus_db.is_online_tune = false;
                    malleus_send_offset_data(malleus_db.effect_offset_tune);
                }
                else
                {
                    ret = malleus_secp_get_start_addr(MALLEUS_EFFECT_SAMPLE_RATE_END, &malleus_db.num_effect,
                                                      &malleus_db.effect_type,
                                                      &StartAddr);

                    if ((ret == MALLEUS_SECP_SUC) && (StartAddr > 0))
                    {
                        malleus_secp_init();
                        StartAddr -= malleus_db.effect_offset_48;
                        malleus_send_offset_data(StartAddr);
                    }
                }
            }
        }
        break;

    case MALLEUS_CMD_SECP_END:
        {

        }
        break;

    default:
        break;
    }
}

static void malleus_audio_vse_cback(T_AUDIO_EFFECT_INSTANCE  instance,
                                    T_AUDIO_VSE_EVENT        event_type,
                                    uint16_t                seq_num,
                                    void                    *event_buf,
                                    uint16_t                 buf_len)
{
    uint8_t *event_buff = event_buf;
    uint32_t NumDataInByte, StartAddr;
    int32_t ret;

    APP_PRINT_TRACE5("malleus_audio_vse_cback: instance %p event_type %d event_buff %p buf_len %d data %b",
                     instance, event_type, event_buf, buf_len, TRACE_BINARY(buf_len, event_buf));

    malleus_instance = instance;

    switch (event_type)
    {
    case AUDIO_VSE_EVENT_ENABLE:
        {
            malleus_channel_out(malleus_channel);
            malleus_load_effect();
        }
        break;

    case AUDIO_VSE_EVENT_DISABLE:
        {
            malleus_instance = NULL;
            malleus_db.changed_effect = EFFECT_UNKNOWN;
        }
        break;

    case AUDIO_VSE_EVENT_TRANS_NOTIFY:
    case AUDIO_VSE_EVENT_TRANS_RSP:
        {
            uint8_t ltv_len = 0, ltv_type = 0;

            if ((event_buff == NULL) || (buf_len < MALLEUS_LTV_RESP_MIN_OCTETS))
            {
                break;
            }

            for (uint16_t i = 0; i < buf_len;)
            {
                ltv_len = event_buff[i];
                ltv_type = event_buff[i + 1];

                if (ltv_type == MALLEUS_EFFECT_DATA_TYPE)
                {
                    malleus_audio_vse_parse(&event_buff[i + 2], (ltv_len - MALLEUS_LTV_TYPE_OCTETS));
                }

                i = i + ltv_len + MALLEUS_LTV_LEN_OCTETS;
            }
        }
        break;

    default:
        break;
    }
}

static void malleus_tune_start(uint8_t cmd_path, uint8_t app_idx, uint16_t offset)
{
    malleus_db.is_online_tune = true;
    malleus_db.changed_effect = EFFECT_UNKNOWN;
    malleus_db.effect_offset_tune = offset;

    malleus_db.cmd_path = cmd_path;
    malleus_db.app_idx = app_idx;

    malleus_relay_async(MALLEUS_SYNC_MSG_TUNE_START, (uint8_t *)&malleus_db.effect_offset_tune, 2);

    malleus_effect_trig();
}

static void malleus_update_secp_data(uint8_t *data, uint16_t len)
{
    malleus_relay_async(MALLEUS_SYNC_MSG_SECP_DATA, data, len);
    malleus_send_secp_data(data, len);
}

static void malleus_cmd_set_key_val(uint8_t *cmd_ptr, uint8_t *ack)
{
    uint8_t idx = cmd_ptr[0] | (cmd_ptr[1] << 8);

    if (idx >= 16)
    {
        *ack = MALLEUS_CMD_STATUS_DISALLOW;
        return;
    }

    malleus_set_key_val(idx, (cmd_ptr[2] | (cmd_ptr[3] << 8)));
    malleus_send_key_val();

    if (malleus_get_role() == REMOTE_SESSION_ROLE_PRIMARY)
    {
        malleus_relay_async(MALLEUS_SYNC_MSG_KEY_VAL, cmd_ptr, 4);
    }
}

void malleus_spatial_set_flag(bool flag)
{
    malleus_db.get_data = flag;
}

void malleus_spatial_data_map(int16_t accs[3], int16_t gyros[3])
{
    malleus_spatial_atti_data_map(&accs[0], &accs[1], &accs[2]);
    malleus_spatial_atti_data_map(&gyros[0], &gyros[1], &gyros[2]);
}

void malleus_relay_param(void)
{
    malleus_relay_effect_type();
    malleus_relay_key_val();
}

void malleus_relay_parse(uint8_t msg_type, uint8_t *buf, uint16_t len, T_REMOTE_RELAY_STATUS status)
{
    APP_PRINT_TRACE2("malleus_parse_cback: event 0x%04x, status %u", msg_type, status);

    switch (msg_type)
    {
    case MALLEUS_SYNC_MSG_TUNE_START:
        {
            if (malleus_get_role() == REMOTE_SESSION_ROLE_SECONDARY)
            {
                uint8_t *data = (uint8_t *)buf;
                malleus_db.effect_offset_tune = data[0] | (data[1] << 8);
                malleus_db.is_online_tune = true;
            }
        }
        break;

    case MALLEUS_SYNC_MSG_SECP_DATA:
        {
            if (malleus_get_role() == REMOTE_SESSION_ROLE_SECONDARY)
            {
                malleus_send_secp_data(buf, len);
            }
        }
        break;

    case MALLEUS_SYNC_MSG_EFFECT_SET:
        {
            if (status != REMOTE_RELAY_STATUS_SYNC_TOUT)
            {
                break;
            }

            malleus_set_normal_type(buf[0]);
            malleus_set_gaming_type(buf[1]);
            malleus_set_gaming_mode(buf[2]);

            malleus_effect_set();
        }
        break;

    case MALLEUS_SYNC_MSG_EFFECT_TYPE:
        {
            if (malleus_get_role() == REMOTE_SESSION_ROLE_PRIMARY)
            {
                break;
            }

            malleus_set_normal_type(buf[0]);
            malleus_set_gaming_type(buf[1]);
            malleus_set_gaming_mode(buf[2]);

            if (malleus.spatial_status_set != NULL)
            {
                malleus.spatial_status = buf[3];
                malleus.spatial_status_set(buf[3]);
            }
        }
        break;

    case MALLEUS_SYNC_MSG_KEY_VAL:
        {
            if (malleus_get_role() == REMOTE_SESSION_ROLE_SECONDARY)
            {
                uint8_t idx = buf[0] | (buf[1] << 8);

                malleus_set_key_val(idx, (buf[2] | (buf[3] << 8)));
                malleus_send_key_val();
            }
        }
        break;

    case MALLEUS_SYNC_MSG_ALL_KEY_VAL:
        {
            uint8_t *data = (uint8_t *)buf;
            memcpy(malleus.key_val, data, MALLEUS_KEY_VAL_OCTETS);
        }
        break;

    case MALLEUS_SYNC_MSG_POS_DATA:
        {
            uint8_t *data = (uint8_t *)buf;
            T_MALLEUS_POS pos;
            memcpy((uint8_t *)&pos, &data[0], sizeof(T_MALLEUS_POS));
            malleus_spatial_update_pos(pos, true);
        }
        break;

    default:
        break;
    }
}

void malleus_relay_load(uint8_t msg_type, uint8_t **data, uint16_t *len)
{
    switch (msg_type)
    {
    case MALLEUS_SYNC_MSG_EFFECT_SET:
        {
            *data = calloc(1, 3);

            if (*data != NULL)
            {
                *data[0] = malleus_get_normal_type();
                *data[1] = malleus_get_gaming_type();
                *data[2] = malleus_get_gaming_mode();
            }

            *len = 3;
        }
        break;

    default:
        break;
    }
}

void malleus_set_channel(T_MALLEUS_CH channel)
{
    malleus_channel = channel;
}

void malleus_cfg_rst(uint8_t normal_type, uint8_t gaming_type)
{
    malleus_set_normal_type(normal_type);
    malleus_set_gaming_type(gaming_type);

    for (uint8_t i = 0; i < 16; i++)
    {
        malleus_set_key_val(i, 0x7fff);
    }

    malleus_save_key_val();
}

void malleus_set_bypass(bool enable)
{
    malleus_data = (T_MALLEUS_DATA *)calloc(1, (MALLEUS_LTV_MIN_OCTETS + 1));

    if (malleus_data != NULL)
    {
        malleus_data->value_dat[0] = enable ? 1 : 0;
        malleus_send_data(MALLEUS_CMD_LOAD_NO_EFFECT, &malleus_data, (MALLEUS_LTV_MIN_OCTETS + 1));
    }
}

void malleus_effect_trig(void)
{
    if (malleus_get_role() == REMOTE_SESSION_ROLE_SECONDARY)
    {
        return;
    }

    if ((malleus_get_role() == REMOTE_SESSION_ROLE_PRIMARY) && malleus_b2b_is_conn())
    {
        malleus_relay_sync(MALLEUS_SYNC_MSG_EFFECT_SET, NULL, 0);
    }
    else if ((malleus.bsrc_start != NULL) && (malleus.bsrc_start()))
    {
        if (malleus.bis_sync_handle != NULL)
        {
            malleus_db.effect_type = malleus_effect_mode_sel;
            malleus.bis_sync_handle(malleus_db.effect_type);
        }
    }
    else
    {
        malleus_effect_set();
    }
}

void malleus_effect_switch(void)
{
    uint8_t cycle_num = malleus_get_cycle_num();

    if (cycle_num == 0)
    {
        return;
    }

    if (malleus_get_role() == REMOTE_SESSION_ROLE_SECONDARY)
    {
        return;
    }

    if ((malleus_get_role() == REMOTE_SESSION_ROLE_PRIMARY) && !malleus_b2b_is_conn())
    {
        return;
    }

    malleus_effect_next_idx();
    malleus_effect_trig();

    if (malleus.play_tone != NULL)
    {
        malleus.play_tone(malleus_effect_mode_sel);
    }
}

T_AUDIO_EFFECT_INSTANCE malleus_create(T_AUDIO_TRACK_HANDLE handle)
{
    T_AUDIO_EFFECT_INSTANCE instance = NULL;

    if (handle != NULL)
    {
        instance = audio_vse_create(AUDIO_VSE_COMPANY_ID_MALLEUSTEK,
                                    MALLEUS_EFFECT_EFFECT_ID,
                                    malleus_audio_vse_cback);


        audio_track_effect_attach(handle, instance);
    }

    return instance;
}

void malleus_release(T_AUDIO_EFFECT_INSTANCE *instance)
{
    if (*instance != NULL)
    {
        audio_vse_release(*instance);
        *instance = NULL;
        malleus_db.changed_effect = EFFECT_UNKNOWN;
        malleus_db.is_online_tune = false;
    }
}

uint8_t malleus_cmd_handle(uint8_t *cmd_ptr, uint16_t cmd_len, uint8_t cmd_path, uint8_t app_idx)
{
    uint8_t ack = MALLEUS_CMD_STATUS_COMP;

    malleus_db.app_idx = app_idx;
    malleus_db.cmd_path = cmd_path;
    switch (cmd_ptr[2])
    {
    case MALLEUS_CMD_EFFECT_SET:
        {
            /*ignore single effect*/
            uint8_t actual_effect_num = (malleus_db.num_effect >= 1) ? (malleus_db.num_effect - 1) : 0;
            uint8_t type = 0;

            if (malleus_get_gaming_mode() != 0)
            {
                ack = MALLEUS_CMD_STATUS_DISALLOW;
                break;
            }

            /*give the previous effect,then call mmi skip flow*/
            if (cmd_ptr[3] < actual_effect_num)
            {
                type = (cmd_ptr[3] == 0) ? (actual_effect_num - 1) :
                       (cmd_ptr[3] - 1);
            }
            else
            {
                type =  actual_effect_num - 1;
            }

            malleus_set_normal_type(type);
            malleus_relay_effect_type();
            malleus_effect_trig();
        }
        break;

    case MALLEUS_CMD_TUNE_START:
        {
            uint16_t offset_tune = cmd_ptr[3] | (cmd_ptr[4] << 8);

            malleus_tune_start(cmd_path, app_idx, offset_tune);
        }
        break;

    case MALLEUS_CMD_REC_SECP_DATA:
        {
            malleus_update_secp_data(&cmd_ptr[3], (cmd_len - 3));
        }
        break;

    case MALLEUS_CMD_GET_SECP_NUM:
        {
            uint8_t data[5] = {0};

            data[0] = MALLEUS_CMD_GET_SECP_NUM;
            data[1] = malleus_db.num_effect & 0xFF;
            data[2] = malleus_db.num_effect >> 8;
            data[3] = malleus_get_normal_type();
            data[4] = malleus_get_gaming_type();

            malleus_report_event(data, sizeof(data));
        }
        break;

    case MALLEUS_CMD_SET_KEY_VALUE:
        {
            malleus_cmd_set_key_val(&cmd_ptr[3], &ack);
        }
        break;

    case MALLEUS_CMD_GET_KEY_VALUE:
        {
            malleus_cmd_get_key_val(&cmd_ptr[3], &ack);
        }
        break;

    case MALLEUS_CMD_RESTART_ACK:
        {
            malleus_cmd_restart_ack();
        }
        break;

    case MALLEUS_CMD_GET_INFO:
        {
            malleus_cmd_get_version();
        }
        break;

    default:
        {
            ack = MALLEUS_CMD_STATUS_UNKNOW;
        }
        break;
    }

    return ack;
}

static void malleus_dm_cback(T_SYS_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    bool handle = true;

    switch (event_type)
    {
    case SYS_EVENT_POWER_OFF:
        {
            malleus_save_key_val();
        }
        break;

    default:
        handle = false;
        break;
    }

    if (handle == true)
    {
        APP_PRINT_INFO1("malleus_dm_cback: event_type 0x%04x", event_type);
    }
}

static void malleus_timeout_cb(uint8_t timer_evt, uint16_t param)
{
    switch (timer_evt)
    {
    case APP_TIMER_FAKE_REPORT:
        {
            uint32_t cur = os_sys_time_get();
            if (cur - malleus_db.last_anchor > 38 || malleus_db.report_count == 0 ||
                (malleus_db.prev_pos.horz & INT16_SIGN_NIT) != (malleus_db.cur_pos.horz & INT16_SIGN_NIT))
            {
                break;
            }
            T_MALLEUS_POS temp;
            temp.horz = ((malleus_db.cur_pos.horz - malleus_db.prev_pos.horz) >> 2) *
                        (4 - malleus_db.report_count) + malleus_db.prev_pos.horz;
            temp.vert = ((malleus_db.cur_pos.vert - malleus_db.prev_pos.vert) >> 2) *
                        (4 - malleus_db.report_count) + malleus_db.prev_pos.vert;
            temp.factor = 0xfae0;
            malleus_spatial_update_pos(temp, false);
            malleus_db.report_count --;
        }
        break;
    default:
        break;
    }
}

static void malleus_sys_register(void)
{
    sys_mgr_cback_register(malleus_dm_cback);

    if (malleus.spatial_audio_en)
    {
        app_timer_reg_cb(malleus_timeout_cb, &malleus_timer_id);
    }
}

static void malleus_param_init(uint8_t normal_cycle, uint8_t gaming_cycle)
{
    uint8_t normal_type = 0, gaming_type = 0;

    malleus_load_key_val();

    malleus_db.err_code = malleus_secp_get_effect_num(&malleus_db.num_effect);
    malleus_db.changed_effect = EFFECT_UNKNOWN;

    normal_type = malleus_mode_valid_check(malleus_get_normal_type());
    gaming_type = malleus_mode_valid_check(malleus_get_gaming_type());

    malleus_set_normal_type(normal_type);
    malleus_set_gaming_type(gaming_type);

    malleus_db.effect_type = malleus_effect_mode_sel;

    malleus_db.normal_cycle = normal_cycle;
    malleus_db.gaming_cycle = gaming_cycle;

    /*clear not support effect,include single mode*/
    for (uint8_t i = malleus_db.num_effect; i < 8; i++)
    {
        malleus_db.normal_cycle &= ~(1 << i);
        malleus_db.gaming_cycle &= ~(1 << i);
    }
}

void malleus_init(uint8_t normal_cycle, uint8_t gaming_cycle)
{
    malleus.ver_info = malleus_ver_info;
    malleus.key_val = (uint8_t *)malleus_key_val;

    malleus_param_init(normal_cycle, gaming_cycle);
    malleus_sys_register();
}
#endif
