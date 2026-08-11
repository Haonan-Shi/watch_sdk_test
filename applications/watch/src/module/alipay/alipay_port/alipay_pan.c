/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include "trace.h"
#include "gap.h"
#include "gap_br.h"
#include "app_gap.h"
#include "app_main.h"
#include "app_cfg.h"
#include "remote.h"
#include "btm.h"
#include "app_linkback.h"
#include "bt_bond.h"
#include "app_bt_policy_int.h"
#include "alipay_common.h"

#if (CONFIG_ALIPAY && CONFIG_ALIPAY_TRANSIT)

#include "alipay_pan.h"
#include "bt_pan.h"
#include "bt_sdp.h"


#include "arch/bnepif.h"
#include "lwip/tcpip.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "app_bond.h"
#include "alipay_config.h"

uint8_t bd_addr_local[6]  = {0};
uint8_t bd_addr_remote[6]  = {0}; //redmi yuyin


//#if F_APP_PAN_PANU_SUPPORT
static const uint8_t pan_panu_sdp_record[] =
{
    //total length
    SDP_DATA_ELEM_SEQ_HDR,
    0x76,//0x59,

    //Attribute SDP_ATTR_SRV_CLASS_ID_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SRV_CLASS_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_SRV_CLASS_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PANU >> 8),
    (uint8_t)(UUID_PANU),

    //attribute SDP_ATTR_PROTO_DESC_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROTO_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROTO_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x1E,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_L2CAP >> 8),
    (uint8_t)(UUID_L2CAP),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(PSM_BNEP >> 8),
    (uint8_t)(PSM_BNEP),
    SDP_DATA_ELEM_SEQ_HDR,
    0x14,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_BNEP >> 8),
    (uint8_t)(UUID_BNEP),
    SDP_UNSIGNED_TWO_BYTE,
    0x01,
    0x00,
    SDP_DATA_ELEM_SEQ_HDR,
    0x0C,
    SDP_UNSIGNED_TWO_BYTE,
    0x08,
    0x00,
    SDP_UNSIGNED_TWO_BYTE,
    0x08,
    0x06,
    SDP_UNSIGNED_TWO_BYTE,
    0x81,
    0x00,
    SDP_UNSIGNED_TWO_BYTE,
    0x86,
    0xdd,

    //attribute SDP_ATTR_BROWSE_GROUP_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_BROWSE_GROUP_LIST >> 8),
    (uint8_t)SDP_ATTR_BROWSE_GROUP_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x03,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP >> 8),
    (uint8_t)(UUID_PUBLIC_BROWSE_GROUP),

    //Attribute SDP_ATTR_LANG_BASE_ATTR_ID_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_LANG_BASE_ATTR_ID_LIST >> 8),
    (uint8_t)SDP_ATTR_LANG_BASE_ATTR_ID_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x09,
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_LANG_ENGLISH >> 8),
    (uint8_t)(SDP_LANG_ENGLISH),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_CHARACTER_UTF8 >> 8),
    (uint8_t)(SDP_CHARACTER_UTF8),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_BASE_LANG_OFFSET >> 8),
    (uint8_t)(SDP_BASE_LANG_OFFSET),

    //attribute SDP_ATTR_PROFILE_DESC_LIST
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_PROFILE_DESC_LIST >> 8),
    (uint8_t)SDP_ATTR_PROFILE_DESC_LIST,
    SDP_DATA_ELEM_SEQ_HDR,
    0x08,
    SDP_DATA_ELEM_SEQ_HDR,
    0x06,
    SDP_UUID16_HDR,
    (uint8_t)(UUID_PANU >> 8),
    (uint8_t)(UUID_PANU),
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(0x0100 >> 8),
    (uint8_t)(0x0100),

    //Attribute SDP_ATTR_SRV_NAME
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)((SDP_ATTR_SRV_NAME + SDP_BASE_LANG_OFFSET) >> 8),
    (uint8_t)(SDP_ATTR_SRV_NAME + SDP_BASE_LANG_OFFSET),
    SDP_STRING_HDR,
    0x0C,
    'R', 'e', 'a', 'l', 't', 'e', 'k', ' ', 'P', 'A', 'N', 'U',

    //Attribute SDP_ATTR_SRV_DESC
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)((SDP_ATTR_SRV_DESC + SDP_BASE_LANG_OFFSET) >> 8),
    (uint8_t)(SDP_ATTR_SRV_DESC + SDP_BASE_LANG_OFFSET),
    SDP_STRING_HDR,
    0x0C,
    'R', 'e', 'a', 'l', 't', 'e', 'k', ' ', 'P', 'A', 'N', 'U',

    //Attribute SDP_ATTR_SECURITY_DESC
    SDP_UNSIGNED_TWO_BYTE,
    (uint8_t)(SDP_ATTR_SECURITY_DESC >> 8),
    (uint8_t)SDP_ATTR_SECURITY_DESC,
    SDP_UNSIGNED_TWO_BYTE,
    0x00,
    0x00
};

void app_gap_bt_cback_pan_cback(T_BT_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_EVENT_PARAM *param = event_buf;
    switch (event_type)
    {
    case BT_EVENT_SDP_ATTR_INFO:
        {
            T_BT_SDP_ATTR_INFO *sdp_info = &param->sdp_attr_info.info;
            if (sdp_info->srv_class_uuid_data.uuid_16 == UUID_NAP)
            {
                bt_pan_connect_req(bd_addr_local/*app_db.local_addr*/, bd_addr_remote/*app_db.remote_addr*/,
                                   BT_PAN_ROLE_PANU, BT_PAN_ROLE_NAP);
            }
        }
        break;
    default:
        break;
    }
}
void app_gap_bt_cback_pan_handler(T_BT_PAN_EVENT event_type, void *event_buf, uint16_t buf_len)
{
    T_BT_PAN_EVENT_PARAM *param = event_buf;
    T_APP_BR_LINK *p_link;
    DBG_DIRECT("app_gap_bt_cback_pan_handler: event_type:0x%x", event_type);
    switch (event_type)
    {
    // PAN start
    case BT_PAN_EVENT_CONN_IND:
        p_link = app_find_br_link(param->pan_conn_ind.bd_addr);
        if (p_link != NULL)
        {
            bt_pan_connect_cfm(bd_addr_local/*app_db.local_addr*/, p_link->bd_addr, true);
        }
        break;

    case BT_PAN_EVENT_SETUP_CONN_IND:
        p_link = app_find_br_link(param->pan_conn_ind.bd_addr);
        if (p_link != NULL)
        {
            bt_pan_setup_connection_rsp(p_link->bd_addr, 0);
        }
        break;

    case BT_PAN_EVENT_CONN_CMPL:
        {
            //AliPay_LOG(temp_buff);
            p_link = app_find_br_link(param->pan_conn_cmpl.bd_addr);
            if (p_link != NULL)
            {
                memcpy(bd_addr_remote/*app_db.remote_addr*/, param->pan_conn_cmpl.bd_addr, 6);
            }

            //lwip
            bnepif_netif_up(bd_addr_remote/*app_db.remote_addr*/);
            bnepif_dhcp_start();
            DBG_DIRECT("[Alipay] bnepif_dhcp_start");

//          bool alipay_bt_pan_connect(void);
//          if (false == alipay_bt_pan_connect())
//          {
//              AliPay_LOG("[Alipay] alipay_bt_pan_connect, failed!");
//              //return -1;
//          }
        }

        break;

    case BT_PAN_EVENT_DISCONN_CMPL:
        {
            //AliPay_LOG(temp_buff);
            bnepif_netif_down();
        }
        break;

    case BT_PAN_EVENT_ETHERNET_PACKET_IND:
        APP_PRINT_TRACE1("app_gap_bt_cback_pan_handler: BT_EVENT_PAN_ETHERNET_PACKET_IND len 0x%x",
                         param->pan_ethernet_packet_ind.len);
        bnepif_low_level_input(param->pan_ethernet_packet_ind.buf, param->pan_ethernet_packet_ind.len);
        break;
    // PAN End

    default:
        break;
    }
}


bool alipay_bt_pan_connect(void)
{
    T_BT_SDP_UUID_DATA uuid;

    AliPay_LOG("[Alipay] alipay_bt_pan_connect");

    uuid.uuid_16 = UUID_NAP;

    T_APP_BOND_DEVICE *active_device = app_bt_bond_get_active_device_by_type(T_DEVICE_TYPE_PHONE);
    if (active_device)
    {
        APP_PRINT_TRACE1("[Alipay] active phone device addr: %b", TRACE_BINARY(6, active_device->bd_addr));
        memcpy(bd_addr_remote, active_device->bd_addr, 6);
    }
    else
    {
        return false;
    }

    return bt_sdp_discov_start(active_device->bd_addr, BT_SDP_UUID16, uuid);
}

const char *alipay = "iotapi.alipay.com";
bool app_bt_pan_get_host_by_name(void)
{
    struct hostent *host;
    host = gethostbyname(alipay);

    LWIP_PLATFORM_DIAG(("get host by name %s", ipaddr_ntoa((ip_addr_t *)host->h_addr_list[0])));

    return true;
}

void alipay_pan_init(void)
{
    if (bt_sdp_record_add((void *)pan_panu_sdp_record) == false)
    {
        DBG_DIRECT("[Alipay] pan bt_sdp_record_add failed!");
        //goto fail_sdp_add;
    }
    if (bt_pan_init() == false)
    {
        //goto fail_init;
        DBG_DIRECT("[Alipay] pan init failed!");
    }
    else
    {
        DBG_DIRECT("[Alipay] alipay_pan_init!");
    }
    bt_mgr_cback_register(app_gap_bt_cback_pan_cback);
    bt_pan_cback_register(app_gap_bt_cback_pan_handler);


    //lwip
    tcpip_init(NULL, NULL);
    uint8_t *pMac = (uint8_t *)ALIPAY_BT_MAC_RAM_ARRD;
    memcpy(bd_addr_local, pMac, 6);
    APP_PRINT_TRACE1("[Alipay] local device addr: %b", TRACE_BINARY(6, bd_addr_local));
    bnepif_init(bd_addr_local, bt_pan_send);


}

e_pan_status alipay_get_network_status(void)
{
    T_APP_BOND_DEVICE *active_device = app_bt_bond_get_active_device_by_type(T_DEVICE_TYPE_PHONE);
    if (active_device)
    {
        ;
    }
    else
    {
        return BT_STATUS_DISCONNECT;
    }

    //get bt status
    return BT_STATUS_CONNECTED;
}

#endif // #if CONFIG_ALIPAY_TRANSIT



