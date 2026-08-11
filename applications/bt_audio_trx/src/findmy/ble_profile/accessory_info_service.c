/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#include <string.h>
#include "trace.h"
#include "accessory_info_service.h"
#include "gap.h"
#include "fmna_constants_platform.h"
#include "fmna_constants.h"
#include "fmna_util.h"
#include "fmna_gatt.h"
#include "fmna_version.h"
#include "fmna_battery_platform.h"
#include "gap_conn_le.h"
#include "bt_gatt_svc.h"

T_SERVER_ID ais_id;
static P_FUN_EXT_SERVER_GENERAL_CB pfn_ais_ble_cb = NULL;
const uint8_t GATT_UUID_ACC_INFO_SERVICE[16] = AIS_SERVICE_BASE_UUID;
uint8_t product_data[PRODUCT_DATA_BLEN] = PRODUCT_DATA_VAL;
uint8_t manu_name[MANU_NAME_MAX_LEN] = FMNA_MANUFACTURER_NAME;
uint8_t model_name[MODEL_NAME_MAX_LEN] = FMNA_MODEL_NAME;
uint8_t accessory_category[ACC_CATEGORY_MAX_LEN] = {0};
uint32_t acc_capability = (uint32_t)(ACC_CAPABILITY_SUPT_PLAY_SOUND
                                     & ACC_CAPABILITY_SUPT_SRNM_LOOKUP_BLE);
uint32_t fw_vers;
uint32_t findmy_vers = 0x00010000;
uint8_t bat_type = 0;
uint8_t bat_level = 0;


const T_ATTRIB_APPL ais_attr_tbl[] =
{
    /*--------------------------AIS Service ---------------------------*/
    /* <<Primary Service>>, .. 0*/
    {
        (ATTRIB_FLAG_VOID | ATTRIB_FLAG_LE),  /* wFlags     */
        {
            LO_WORD(GATT_UUID_PRIMARY_SERVICE),
            HI_WORD(GATT_UUID_PRIMARY_SERVICE),     /* bTypeValue */
        },
        UUID_128BIT_SIZE,                     /* bValueLen     */
        (void *)GATT_UUID_ACC_INFO_SERVICE,        /* pValueContext */
        GATT_PERM_READ                              /* wPermissions  */
    },

    /* <<Characteristic1>>, .. 1, Product data*/              //------------------------Product data  1
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 2*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_PROD_DATA
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic2>>, .. 3, Manufacturer Name*/          //------------------------Manufacturer Name  2
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 4*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_MANU_NAME
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic3>>, .. 5, Model Name*/                   //------------------------Model Name  3
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 6*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_MODEL_NAME
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

//    /* <<Characteristic4>>, .. 7, Reserved*/                       //------------------------Reserved  4
//    {
//        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
//        {   /* bTypeValue */
//            LO_WORD(GATT_UUID_CHARACTERISTIC),
//            HI_WORD(GATT_UUID_CHARACTERISTIC),
//            GATT_CHAR_PROP_READ,                    /* characteristic properties */
//            /* characteristic UUID not needed here, is UUID of next attrib. */
//        },
//        1,                                          /* bValueLen */
//        NULL,
//        GATT_PERM_READ                              /* wPermissions */
//    },
//    /*  AIS characteristic value 8*/
//    {
//      ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
//        {                                                           /* type_value */
//            GATT_UUID128_RESERVED
//        },
//        0,                                                 /* bValueLen */
//        (void *)NULL,
//        GATT_PERM_READ            /* wPermissions */
//    },

    /* <<Characteristic5>>, .. 9, Accessory Category*/             //------------------------Accessory Category  5
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 0x0A*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_ACC_CATEGORY
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic6>>, .. 0x0b, Accessory Capabilities*/       //------------------------Accessory Capabilities  6
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 0x0c*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_ACC_CAP
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic7>>, .. 0x0d, Firmware Version*/              //------------------------Firmware Version  7
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 0x0e*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_FW_VERS
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic8>>, .. 0x0f, FindMy Version*/               //------------------------FindMy Version  8
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 0x10 */
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_FINDMY_VERS
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic9>>, .. 0x11, Battery Type*/                //------------------------Battery Type  9
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 0x12*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_BATT_TYPE
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    },

    /* <<Characteristic10>>, .. 0x13, Battery Level*/               //------------------------Battery Level  10
    {
        ATTRIB_FLAG_VALUE_INCL,                     /* wFlags */
        {   /* bTypeValue */
            LO_WORD(GATT_UUID_CHARACTERISTIC),
            HI_WORD(GATT_UUID_CHARACTERISTIC),
            GATT_CHAR_PROP_READ,                    /* characteristic properties */
            /* characteristic UUID not needed here, is UUID of next attrib. */
        },
        1,                                          /* bValueLen */
        NULL,
        GATT_PERM_READ                              /* wPermissions */
    },
    /*  AIS characteristic value 0x14*/
    {
        ATTRIB_FLAG_VALUE_APPL | ATTRIB_FLAG_UUID_128BIT,                              /* flags */
        {                                                           /* type_value */
            GATT_UUID128_BATT_LVL
        },
        0,                                                 /* bValueLen */
        (void *)NULL,
        GATT_PERM_READ            /* wPermissions */
    }

};

/**
  * @brief  Set service related data from application.
  *
  * @param[in] param_type            parameter type to set.
  * @param[in] len                   value length to be set.
  * @param[in] p_value             value to set.
  * @return parameter set result.
  * @retval 0 false
  * @retval 1 true
  */
bool ais_set_parameter(T_AIS_PARAM_TYPE param_type, uint16_t len, void *p_value)
{
    bool ret = true;

    switch (param_type)
    {
    case AIS_PARAM_PROD_DATA:
        {
            if (len > PRODUCT_DATA_BLEN)
            {
                len = PRODUCT_DATA_BLEN;
            }
            memcpy(product_data, p_value, len);
        }
        break;

    case AIS_PARAM_MANU_NAME:
        {
            if (len > MANU_NAME_MAX_LEN)
            {
                len = MANU_NAME_MAX_LEN;
            }
            memcpy(manu_name, p_value, len);
        }
        break;

    case AIS_PARAM_MODEL_NAME:
        {
            if (len > MODEL_NAME_MAX_LEN)
            {
                len = MODEL_NAME_MAX_LEN;
            }
            memcpy(model_name, p_value, len);
        }
        break;

    case AIS_PARAM_ACC_CATEGORY:
        {
            if (len > ACC_CATEGORY_MAX_LEN)
            {
                len = ACC_CATEGORY_MAX_LEN;
            }
            memcpy(accessory_category, p_value, len);
        }
        break;

    case AIS_PARAM_ACC_CAP:
        {

            acc_capability = *(uint32_t *)p_value;
        }
        break;

    case AIS_PARAM_FW_VERS:
        {
            fw_vers = *(uint32_t *)p_value;
        }
        break;

    case AIS_PARAM_FINDMY_VERS:
        {
            //this must be set to 1.0.0 for this version of the specification.
            findmy_vers = *(uint32_t *)p_value;;
        }
        break;

    case AIS_PARAM_BATT_TYPE:
        {
            bat_type = *(uint8_t *)p_value;
        }
        break;

    case AIS_PARAM_BATT_LVL:
        {
            bat_level = *(uint8_t *)p_value;
        }
        break;

    default:
        {
            ret = false;
        }
        break;
    }

    if (!ret)
    {
        PROFILE_PRINT_ERROR1("ais parameter set failed: %d\n", param_type);
    }

    return ret;
}

T_APP_RESULT ais_attr_read_cb(uint16_t conn_handle, uint16_t cid, T_SERVER_ID service_id,
                              uint16_t attrib_index, uint16_t offset, uint16_t *p_length, uint8_t **pp_value)
{
    T_APP_RESULT cause = APP_RESULT_SUCCESS;
    T_AIS_CALLBACK_DATA ais_cb_data;
    uint8_t conn_id;
    le_get_conn_id_by_handle(conn_handle, &conn_id);

    ais_cb_data.msg_type = SERVICE_CALLBACK_TYPE_READ_CHAR_VALUE;
    ais_cb_data.conn_id = conn_id;
    ais_cb_data.conn_handle = conn_handle;
    ais_cb_data.cid = cid;

    APP_PRINT_INFO1("ais_attr_read_cb, Attr found, index %d", attrib_index);
    switch (attrib_index)
    {
    default:
        APP_PRINT_INFO1("ais_attr_read_cb, Attr not found, index %d", attrib_index);
        cause = APP_RESULT_ATTR_NOT_FOUND;
        break;
    case FMNA_AIS_PRODUCT_DATA_INDEX:
        {
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_PRODUCT_DATA_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)product_data;
            *p_length = PRODUCT_DATA_BLEN;
        }
        break;
    case FMNA_AIS_MANU_NAME_INDEX:
        {
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_MANU_NAME_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)manu_name;
            *p_length = sizeof(FMNA_MANUFACTURER_NAME);
        }
        break;
    case FMNA_AIS_MODEL_NAME_INDEX:
        {
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_MODEL_NAME_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)model_name;
            *p_length = sizeof(FMNA_MODEL_NAME);
        }
        break;
    case FMNA_AIS_ACCESSORY_CATEGORY_INDEX:
        {
            accessory_category[0] = ACCESSORY_CATEGORY;
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_ACCESSORY_CATEGORY_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)accessory_category;
            *p_length = ACC_CATEGORY_MAX_LEN;
        }
        break;
    case FMNA_AIS_ACCESSORY_CAPABILITY_INDEX:
        {
            // Accessory capabilities bitmask as defined in Find My Network specification.
            // ADK supports play sound (via nrf52 DK LEDs), Firmware update service, and serial number lookup by BLE.
            // No motion detection or NFC capabilities.
//            SET_BIT(acc_capability, ACC_CAPABILITY_PLAY_SOUND_BIT_POS);
//            SET_BIT(acc_capability, ACC_CAPABILITY_SRNM_LOOKUP_BLE_BIT_POS);
//            SET_BIT(acc_capability, ACC_CAPABILITY_FW_UPDATE_SERVICE_BIT_POS);
//            SET_BIT(acc_capability, ACC_CAPABILITY_SRNM_LOOKUP_NFC_BIT_POS);
//            SET_BIT(acc_capability, ACC_CAPABILITY_UT_MOTION_DETECT_BIT_POS);
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_ACCESSORY_CAPABILITY_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)(&acc_capability);
            *p_length = ACC_CAP_MAX_LEN;
        }
        break;
    case FMNA_AIS_FIRMWARE_VERSION_INDEX:
        {
//            fw_vers = fmna_version_get_fw_version();
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_FIRMWARE_VERSION_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)(&fw_vers);
            *p_length = FW_VERS_MAX_LEN;
        }
        break;
    case FMNA_AIS_FINDMY_VERSION_INDEX:
        {
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_FINDMY_VERSION_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)(&findmy_vers);
            *p_length = FINDMY_VERS_MAX_LEN;
        }
        break;
    case FMNA_AIS_BATTERY_TYPE_INDEX:
        {
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_BATTERY_TYPE_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)(&bat_type);
            *p_length = BATT_TYPE_MAX_LEN;
        }
        break;
    case FMNA_AIS_BATTERY_LEVEL_INDEX:
        {
            ais_cb_data.msg_data.read_value_index = FMNA_AIS_BATTERY_LEVEL_INDEX;
            cause = pfn_ais_ble_cb(service_id, (void *)&ais_cb_data);
            *pp_value = (uint8_t *)(&bat_level);
            *p_length = BATT_LVL_MAX_LEN;
        }
        break;
    }

    return (cause);
}

const T_FUN_GATT_EXT_SERVICE_CBS ais_ble_cbs =
{
    ais_attr_read_cb,  // Read callback function pointer
    NULL, // Write callback function pointer
    NULL // CCCD update callback function pointer
};

T_SERVER_ID accessory_info_add_service(void *p_func)
{
    if (false == gatt_svc_add(&ais_id,
                              (uint8_t *)ais_attr_tbl,
                              sizeof(ais_attr_tbl),
                              &ais_ble_cbs, NULL))
    {
        APP_PRINT_ERROR0("accessory_info_add_service: fail");
        ais_id = 0xff;
        return ais_id;
    }

    pfn_ais_ble_cb = (P_FUN_EXT_SERVER_GENERAL_CB)p_func;
    return ais_id;
}
#endif
