#
# Copyright (c) 2026, Realtek Semiconductor Corporation
#
# SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
#

#------------------------GATT Profiles Client----------------------------------
if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_AMS_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/ams_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_ANCS_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/ancs_gatt_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_ASP_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/asp_gatt_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_BAS_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/bas_gatt_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_DIS_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/dis_gatt_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_GAPS_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/gaps_gatt_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_HID_OVER_ISO_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/hid_over_iso_gatt_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_HIDS_CFU_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/hids_cfu_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_HIDS_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/hids_gatt_client.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_IAS_CLIENT)
    zephyr_include_directories(inc/bt_gatt_client)
    target_sources(app PRIVATE src/bt_gatt_client/ias_gatt_client.c)
endif()

#------------------------GATT Profiles Server---------------------------------

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_ASP_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/asp_svc.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_BAS_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/bas_gatt_svc.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_DIS_SERVER)    
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/dis_gatt_svc.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_HID_OVER_ISO_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/hid_over_iso_gatt_svc.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_HIDS_CFU_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/hids_cfu.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_HIDS_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/hids_gatt_svc.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_IAS_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/ias_gatt_svc.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_TRANSMIT_SVC_DONGLE_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/transmit_svc_dongle.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_TPS_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/tps_gatt_svc.c)
endif()

if(CONFIG_REALTEK_SUBSYS_GATT_PROFILE_HRS_SERVER)
    zephyr_include_directories(inc/bt_gatt_svc)
    target_sources(app PRIVATE src/bt_gatt_svc/hrs_gatt_svc.c)
endif()
