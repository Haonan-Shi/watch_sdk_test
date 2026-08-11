/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#include "fmna_gatt_platform.h"
#include "fmna_timer_platform.h"
#include "fmna_connection_platform.h"
#include "fmna_constants_platform.h"
#include "fmna_constants.h"
#include "fmna_util.h"
#include "fmna_state_machine.h"
#include "fmna_adv.h"
#include "findmy_ble_internal.h"
#include "string.h"
#include "os_sched.h"
#include "app_findmy_ble.h"
#include "app_ble_gap.h"
#include "ble_ext_adv.h"
#include "app_adv_stop_cause.h"

// static uint32_t fmna_fast_adv_duration = 0;
static uint16_t fmna_fast_adv_interval = 0;
// static uint32_t fmna_slow_adv_duration = 0;
static uint16_t fmna_slow_adv_interval = 0;


#define SERVICE_DATA_OFFSET             (4)
#define MANU_DATA_OFFSET                (4)

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
static uint8_t pairing_adv_data[29] =
{
//    0x03,
//    GAP_ADTYPE_16BIT_COMPLETE,
//    LO_WORD(FINDMY_UUID_SERVICE),
//    HI_WORD(FINDMY_UUID_SERVICE),

    /* Flags */
    0x18,             /* length */
    GAP_ADTYPE_SERVICE_DATA, /* type="Flags" */
    LO_WORD(FINDMY_UUID_SERVICE),
    HI_WORD(FINDMY_UUID_SERVICE),
};

static uint8_t nearby_adv_data[31] =
{
    0x03,
    0xFF,
    0x4C, 0x00,
};

static uint8_t separate_adv_data[31] =
{
    0x03,
    0xFF,
    0x4C, 0x00,
};

const uint8_t sr_data[] = {};

void fmna_adv_platform_get_default_bt_addr(uint8_t default_bt_addr[FMNA_BLE_MAC_ADDR_BLEN])
{
    gap_get_param(GAP_PARAM_BD_ADDR, default_bt_addr);

    reverse_array(default_bt_addr, 0, FMNA_BLE_MAC_ADDR_BLEN - 1);   //LSB
}

void fmna_adv_platform_set_random_static_bt_addr(uint8_t new_bt_mac[FMNA_BLE_MAC_ADDR_BLEN])
{
    ftl_save_to_storage(new_bt_mac, FTL_SAVE_BT_MAC_ADDR, FTL_SAVE_BT_MAC_SIZE);
    reverse_array(new_bt_mac, 0, FMNA_BLE_MAC_ADDR_BLEN - 1);
    APP_PRINT_INFO1("fmna_adv_platform_set_random_static_bt_addr: BT MAC %s",
                    TRACE_BDADDR(new_bt_mac));//FMNA_LOG_INFO
    uint8_t local_bd_type = GAP_LOCAL_ADDR_LE_RANDOM;
    le_cfg_local_identity_address(new_bt_mac, GAP_IDENT_ADDR_RAND);
    ble_ext_adv_mgr_set_random(app_findmy_adv_get_adv_handle(), new_bt_mac);
    le_adv_set_param(GAP_PARAM_ADV_LOCAL_ADDR_TYPE, sizeof(local_bd_type), &local_bd_type);
}

void fmna_adv_platform_start_fast_adv(void)
{
    ble_ext_adv_mgr_change_adv_interval(app_findmy_adv_get_adv_handle(), fmna_fast_adv_interval);
    app_findmy_adv_start(APP_FINDMY_FAST_ADV_TIMEOUT);
}

void fmna_adv_platform_start_slow_adv(void)
{
    ble_ext_adv_mgr_change_adv_interval(app_findmy_adv_get_adv_handle(), fmna_slow_adv_interval);
    app_findmy_adv_start(APP_FINDMY_SLOW_ADV_TIMEOUT);
}

void fmna_adv_platform_stop_adv(void)
{
    app_findmy_adv_stop(APP_STOP_ADV_CAUSE_FINDMY);
}

void fmna_adv_platform_init_pairing(uint8_t *pairing_adv_service_data,
                                    size_t pairing_adv_service_data_size)
{
    // fmna_fast_adv_duration = fmna_pairing_adv_fast_duration;
    fmna_fast_adv_interval = fmna_pairing_adv_fast_intv;
    // fmna_slow_adv_duration = fmna_pairing_adv_slow_duration;
    fmna_slow_adv_interval = fmna_pairing_adv_slow_intv;
    memcpy(pairing_adv_data + SERVICE_DATA_OFFSET, pairing_adv_service_data,
           pairing_adv_service_data_size);
    ble_ext_adv_mgr_set_adv_data(app_findmy_adv_get_adv_handle(), 25, (uint8_t *)pairing_adv_data);
    FMNA_LOG_INFO("fmna_adv_platform_init_pairing: ADV Pairing len %d", pairing_adv_service_data_size);
}

void fmna_adv_platform_init_nearby(uint8_t *nearby_adv_manuf_data,
                                   size_t nearby_adv_manuf_data_size)
{
    // fmna_fast_adv_duration = fmna_nearby_adv_fast_duration;
    fmna_fast_adv_interval = fmna_nearby_adv_fast_intv;
    // fmna_slow_adv_duration = fmna_nearby_adv_duration;
    fmna_slow_adv_interval = fmna_nearby_adv_intv;
    memcpy(nearby_adv_data + MANU_DATA_OFFSET, nearby_adv_manuf_data, nearby_adv_manuf_data_size);
    nearby_adv_data[0] = 3 + nearby_adv_manuf_data_size;
    ble_ext_adv_mgr_set_adv_data(app_findmy_adv_get_adv_handle(), 3 + nearby_adv_manuf_data_size + 1,
                                 (uint8_t *)nearby_adv_data);
    FMNA_LOG_INFO("fmna_adv_platform_init_nearby: ADV Nearby len %d",
                  3 + nearby_adv_manuf_data_size + 1);
}


void fmna_adv_platform_init_separated(uint8_t *separated_adv_manuf_data,
                                      size_t separated_adv_manuf_data_size)
{
    // fmna_fast_adv_duration = fmna_separated_adv_fast_duration;
    fmna_fast_adv_interval = fmna_separated_adv_fast_intv;
    // fmna_slow_adv_duration = fmna_separated_adv_slow_duration;
    fmna_slow_adv_interval = fmna_separated_adv_slow_intv;
    memcpy(separate_adv_data + MANU_DATA_OFFSET, separated_adv_manuf_data,
           separated_adv_manuf_data_size);
    separate_adv_data[0] = 3 + separated_adv_manuf_data_size;
    ble_ext_adv_mgr_set_adv_data(app_findmy_adv_get_adv_handle(), 3 + separated_adv_manuf_data_size + 1,
                                 (uint8_t *)separate_adv_data);
    FMNA_LOG_INFO("fmna_adv_platform_init_separated: ADV Separate len %d",
                  3 + separated_adv_manuf_data_size + 1);
    FMNA_LOG_HEXDUMP_INFO(separate_adv_data, 31);
}
