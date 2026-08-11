/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_

//Init value of default features are defined here
//----- [Device related] -----
#define F_APP_AUTO_POWER_TEST_LOG           0
#define F_APP_TEST_SUPPORT                  1
#define F_APP_DATA_CAPTURE_SUPPORT          1
#define F_APP_SAIYAN_MODE                   0
#define F_APP_SAIYAN_EQ_FITTING             1
#define F_APP_SUPPORT_CAPTURE_ACOUSTICS_MP  1
#define F_APP_CONSOLE_SUPPORT               1
#define F_APP_DUT_MODE_AUTO_POWER_OFF       0
#define F_APP_VOICE_NREC_SUPPORT            1 //Enable APIs provided in app_nrec.h
#define F_APP_ATTACH_NREC_SUPPORT           1 //Sub-function of F_SOURCE_PLAY_SUPPORT
#define F_APP_VOICE_SPK_EQ_SUPPORT          1
#define F_APP_VOICE_MIC_EQ_SUPPORT          1
#define F_APP_SIDETONE_SUPPORT              1
#define F_APP_SMOOTH_BAT_REPORT             1
#define F_APP_USER_EQ_SUPPORT               1
#define F_APP_AUDIO_VOICE_SPK_EQ_INDEPENDENT_CFG    1
#define F_APP_AUDIO_VOICE_SPK_EQ_COMPENSATION_CFG   0 // do not support EQ compensation
#define F_APP_DISABLE_NOTIFICATION_SUPPORT  0
#define F_APP_MONITOR_MEMORY_AND_TIMER      0
#define F_APP_UART_DFU                      0
#define F_APP_SD_CARD_PLAY                  0
#define F_APP_USB_HID_PC_TOOL               0
#define F_APP_UAC_MEDIA_SILENCE_DETECT      0
#define F_APP_CFU_FEATURE_SUPPORT           0
#define F_APP_USB_HID_SUPPORT               0
#define F_APP_USB_HID_SEC_SUPPORT           0
#define F_APP_MALLEUS_SUPPORT               0
#define F_APP_RECORD_EQ_SUPPORT             0


#define CONFIG_REALTEK_APP_BOND_MGR_SUPPORT              1

#define F_APP_WIFI_PTA_SUPPORT             0

#define F_APP_FLASH_DUMP_SUPPORT           0
#define F_APP_LOG2FLASH_SUPPORT            (0 && F_APP_FLASH_DUMP_SUPPORT)
#define F_APP_CORE_DUMP_SUPPORT            (0 && F_APP_FLASH_DUMP_SUPPORT)

//----- [Dual Mode related] -----
#define F_APP_BT_ANCS_CLIENT_SUPPORT       0



//----- [BT related] -----
#define F_APP_MULTILINK_ENABLE              0
#define F_APP_A2DP_CODEC_LDAC_SUPPORT       0
#define F_APP_BT_PROFILE_PBAP_PCE_SUPPORT   0
#define F_APP_BT_PROFILE_MAP_MCE_SUPPORT    0
#define F_APP_IAP_RTK_SUPPORT               0
#define F_APP_IAP_SUPPORT                   0
#define F_APP_BT_HID_DEVICE_SUPPORT         0
#define F_APP_HID_MOUSE_SUPPORT             0
#define F_APP_HID_KEYBOARD_SUPPORT          0
#define F_APP_BT_HID_HOST_SUPPORT           0
#define F_APP_A2DP_SOURCE_SUPPORT           0
#define F_APP_A2DP_SINK_SUPPORT             0
#define F_APP_HFP_AG_SUPPORT                0
#define F_APP_HFP_HF_SUPPORT                0
#define F_APP_ACL_ROLE_FORCE_MASTER         0
#define F_APP_GATT_OVER_BREDR_SUPPORT       0
#define F_APP_GATT_OVER_BREDR_ADV_USE_RVDIS 0
#define F_APP_BREDR_SC_CTKD_SUPPORT         1
#define F_APP_A2DP_MULTI_SINK_SUPPORT       0
#define F_APP_PAN_SUPPORT                   0

//----- [LE related] -----
#define F_APP_GATT_SERVER_EXT_API_SUPPORT   1
#define F_BT_GATT_SERVER_EXT_API            1
#define F_APP_BLE_AMS_CLIENT_SUPPORT        0
#define F_APP_BLE_HID_DEVICE_SUPPORT        0
#define F_APP_BLE_HID_HOST_SUPPORT          0
#define F_APP_LE_AUDIO_INITIATOR_SUPPORT    0
#define F_APP_LE_AUDIO_ACCEPTOR_SUPPORT     0
#define F_APP_SC_KEY_DERIVE_SUPPORT         1
#define CONFIG_REALTEK_BT_GATT_CLIENT_SUPPORT            1

//----- [Peripheral related] -----
#define F_APP_ADC_SUPPORT                   1
#define F_APP_LINEIN_SUPPORT                0
#define F_APP_USB_AUDIO_SUPPORT             0
#define F_APP_USB_MSC_SUPPORT               0
#define F_APP_USB_CDC_SUPPORT               0
#if F_APP_USB_CDC_SUPPORT
#undef IAD_SUPPORT
#define IAD_SUPPORT                         1
#endif
#define F_APP_USB_SUSPEND_SUPPORT           1
#define F_APP_EXT_AUDIO_AMP_SUPPORT         0
#define F_APP_HIFI4_SUPPORT                 0 //RTL87x3G Not Supported
#define F_APP_SPDIF_SUPPORT                 0 // RTL87x3G Not Supported
#define F_APP_SD_CARD_SUPPORT               0 // Notes: always 0 for zephyr, config by device tree
#define F_APP_CAN_SUPPORT                   0

#define F_APP_HFP_CMD_SUPPORT               1
#define F_APP_DEVICE_CMD_SUPPORT            1
#define F_APP_AVRCP_CMD_SUPPORT             1
#define F_APP_PBAP_CMD_SUPPORT              0

#define F_APP_CUSTOMER_VD_SPP_SUPPORT       1
#define F_APP_CUSTOMER_RECORD_SUPPORT       1

#define F_APP_MULTI_CHANNEL_SUPPORT         0

#define F_APP_WIFI_SPI_MAP_SUPPORT          0

#define IC_NAME                         "RTL87X3G"

#define F_APP_EXT_RF_PA_SUPPORT             1


#define F_APP_THROUGHPUT_SERVER_SUPPORT     0

//----- [Sample configuration] -----
/**
 *  NOTE: Only one demo support flags shall be set 1
 */
#define F_APP_BT_AUDIO_TRANSMITTER_DEMO_SUPPORT         1
#define F_APP_BT_AUDIO_RECEIVER_DEMO_SUPPORT            0
#define F_APP_BT_AUDIO_TRANSCEIVER_DEMO_SUPPORT         0
#define F_APP_BT_AUDIO_TRANSMITTER_MP3_DEMO_SUPPORT     0
#define F_APP_AI_RECORD_PEN_DEMO_SUPPORT                0


#if F_APP_BT_AUDIO_TRANSMITTER_DEMO_SUPPORT
#undef F_APP_DISABLE_NOTIFICATION_SUPPORT
#define F_APP_DISABLE_NOTIFICATION_SUPPORT      1

#undef F_APP_A2DP_SOURCE_SUPPORT
#define F_APP_A2DP_SOURCE_SUPPORT               1
#undef F_APP_HFP_AG_SUPPORT
#define F_APP_HFP_AG_SUPPORT                    1

#undef F_APP_LE_AUDIO_INITIATOR_SUPPORT
#define F_APP_LE_AUDIO_INITIATOR_SUPPORT        1

#undef F_SOURCE_PLAY_SUPPORT
#define F_SOURCE_PLAY_SUPPORT                   1
#if F_SOURCE_PLAY_SUPPORT
#undef F_APP_ATTACH_LOCAL_PLAY_SUPPORT
#define F_APP_ATTACH_LOCAL_PLAY_SUPPORT         1
#endif

#undef F_APP_USB_AUDIO_SUPPORT
#define F_APP_USB_AUDIO_SUPPORT                 1
#if F_APP_USB_AUDIO_SUPPORT
#define F_APP_FWK_PIPE_DEMO_SUPPORT             1
#else
#define F_APP_FWK_PIPE_DEMO_SUPPORT             0
#endif

#undef F_APP_SD_CARD_LOCALPLAY
#define F_APP_SD_CARD_LOCALPLAY                 1
#undef F_APP_SD_CARD_PLAY
#define F_APP_SD_CARD_PLAY                      1

#undef F_APP_BT_HID_HOST_SUPPORT
#define F_APP_BT_HID_HOST_SUPPORT               1
#undef BLE_HID_CLIENT_SUPPORT
#define BLE_HID_CLIENT_SUPPORT                  1

#define F_APP_USB_HOST_SUPPORT                  0
#if F_APP_USB_HOST_SUPPORT
#define F_APP_DBG_DUMP_PCM_TO_RINGBUF             1
#else
#define F_APP_DBG_DUMP_PCM_TO_RINGBUF             0
#endif

#define F_APP_USB_MSC_SUPPORT               1
#define F_APP_DBG_DUMP_PCM_TO_FILE          0       /* Save decoded PCM to file during SD playback */

#endif /* end of F_APP_BT_AUDIO_TRANSMITTER_DEMO_SUPPORT */


#if F_APP_BT_AUDIO_RECEIVER_DEMO_SUPPORT
#undef F_APP_MULTILINK_ENABLE
#define F_APP_MULTILINK_ENABLE              1
#undef F_APP_A2DP_SINK_SUPPORT
#define F_APP_A2DP_SINK_SUPPORT             1
#undef F_APP_HFP_HF_SUPPORT
#define F_APP_HFP_HF_SUPPORT                1

#undef F_APP_BT_PROFILE_PBAP_PCE_SUPPORT
#define F_APP_BT_PROFILE_PBAP_PCE_SUPPORT   1
#undef F_APP_BT_PROFILE_MAP_MCE_SUPPORT
#define F_APP_BT_PROFILE_MAP_MCE_SUPPORT    1
#undef F_APP_IAP_RTK_SUPPORT
#define F_APP_IAP_RTK_SUPPORT               0
#undef F_APP_IAP_SUPPORT
#define F_APP_IAP_SUPPORT                   0
#undef F_APP_BLE_HID_DEVICE_SUPPORT
#define F_APP_BLE_HID_DEVICE_SUPPORT        0
#undef F_APP_LE_AUDIO_ACCEPTOR_SUPPORT
#define F_APP_LE_AUDIO_ACCEPTOR_SUPPORT     1

#undef F_APP_PBAP_CMD_SUPPORT
#define F_APP_PBAP_CMD_SUPPORT              1

#undef F_APP_MALLEUS_SUPPORT
#define F_APP_MALLEUS_SUPPORT               0

#undef F_APP_GATT_OVER_BREDR_SUPPORT
#define F_APP_GATT_OVER_BREDR_SUPPORT       0
#undef F_APP_GATT_OVER_BREDR_ADV_USE_RVDIS
#define F_APP_GATT_OVER_BREDR_ADV_USE_RVDIS 0

#undef F_APP_BT_ANCS_CLIENT_SUPPORT
#define F_APP_BT_ANCS_CLIENT_SUPPORT        0

#undef F_APP_BLE_AMS_CLIENT_SUPPORT
#define F_APP_BLE_AMS_CLIENT_SUPPORT        0
#endif /* end of F_APP_BT_AUDIO_RECEIVER_DEMO_SUPPORT */


#if F_APP_BT_AUDIO_TRANSCEIVER_DEMO_SUPPORT
#undef F_APP_DISABLE_NOTIFICATION_SUPPORT
#define F_APP_DISABLE_NOTIFICATION_SUPPORT  1

#define F_APP_SPI_ROLE_MASTER               0
#define F_APP_SPI_ROLE_SLAVE                0
#define F_APP_INTEGRATED_TRANSCEIVER        0

#if F_APP_SPI_ROLE_MASTER
#undef F_APP_A2DP_SINK_SUPPORT
#define F_APP_A2DP_SINK_SUPPORT             1
#undef F_APP_HFP_HF_SUPPORT
#define F_APP_HFP_HF_SUPPORT                1

#define F_APP_A2DP_XMIT_SNK_LEA_SUPPORT     1
#define F_APP_A2DP_XMIT_SNK_SUPPORT         1
#define F_APP_SCO_XMIT_HF_SUPPORT           1
#endif

#if F_APP_SPI_ROLE_SLAVE
#undef F_APP_A2DP_SOURCE_SUPPORT
#define F_APP_A2DP_SOURCE_SUPPORT           1
#undef F_APP_HFP_AG_SUPPORT
#define F_APP_HFP_AG_SUPPORT                1

#define F_APP_A2DP_XMIT_SRC_LEA_SUPPORT     1
#define F_APP_A2DP_XMIT_SRC_SUPPORT         1
#define F_APP_SCO_XMIT_AG_SUPPORT           1
#if F_APP_A2DP_XMIT_SRC_LEA_SUPPORT
#undef F_APP_LE_AUDIO_INITIATOR_SUPPORT
#define F_APP_LE_AUDIO_INITIATOR_SUPPORT    1
#endif /* end of F_APP_A2DP_XMIT_SRC_LEA_SUPPORT */
#endif /* end of F_APP_SPI_ROLE_SLAVE */

#if F_APP_INTEGRATED_TRANSCEIVER
#undef F_APP_MULTILINK_ENABLE
#define F_APP_MULTILINK_ENABLE              1
#undef F_APP_ACL_ROLE_FORCE_MASTER
#define F_APP_ACL_ROLE_FORCE_MASTER         1

#undef F_SOURCE_PLAY_SUPPORT
#define F_SOURCE_PLAY_SUPPORT               1
#undef F_APP_FWK_PIPE_DEMO_SUPPORT
#define F_APP_FWK_PIPE_DEMO_SUPPORT         1
#undef F_APP_A2DP_SOURCE_SUPPORT
#define F_APP_A2DP_SOURCE_SUPPORT           1
#undef F_APP_HFP_AG_SUPPORT
#define F_APP_HFP_AG_SUPPORT                1
#undef F_APP_A2DP_SINK_SUPPORT
#define F_APP_A2DP_SINK_SUPPORT             1
#undef F_APP_HFP_HF_SUPPORT
#define F_APP_HFP_HF_SUPPORT                1

#if F_SOURCE_PLAY_SUPPORT
#undef F_APP_ATTACH_LOCAL_PLAY_SUPPORT
#define F_APP_ATTACH_LOCAL_PLAY_SUPPORT     0
#endif
#endif

/* Shall be set 1 when support a2dp multi_sink function */
#undef F_APP_A2DP_MULTI_SINK_SUPPORT
#define F_APP_A2DP_MULTI_SINK_SUPPORT       0

#undef F_APP_MULTI_CHANNEL_SUPPORT
#define F_APP_MULTI_CHANNEL_SUPPORT         0

#undef F_APP_LE_AUDIO_INITIATOR_SUPPORT
#define F_APP_LE_AUDIO_INITIATOR_SUPPORT    1
#undef F_APP_LE_AUDIO_ACCEPTOR_SUPPORT
#define F_APP_LE_AUDIO_ACCEPTOR_SUPPORT     1

#endif /* end of F_APP_BT_AUDIO_TRANSCEIVER_DEMO_SUPPORT */


#if F_APP_BT_AUDIO_TRANSMITTER_MP3_DEMO_SUPPORT
#undef F_APP_A2DP_SOURCE_SUPPORT
#define F_APP_A2DP_SOURCE_SUPPORT           1
#undef F_APP_HFP_AG_SUPPORT
#define F_APP_HFP_AG_SUPPORT                1

#define F_APP_MUSIC_LOCAL_PLAY_SUPPORT                  0
#define F_APP_MUSIC_A2DP_SOURCE_SUPPORT                 0
#define F_APP_CUSTOMER_AUDIO_POLICY_SUPPORT             1
#endif /* end of F_APP_BT_AUDIO_TRANSMITTER_MP3_DEMO_SUPPORT */


#if F_APP_AI_RECORD_PEN_DEMO_SUPPORT
#undef F_APP_DISABLE_NOTIFICATION_SUPPORT
#define F_APP_DISABLE_NOTIFICATION_SUPPORT  1

/* WiFi AT Command Path Config. Choose One of F_APP_WIFI_SPI_CMD or F_APP_WIFI_UART_CMD*/
/* NOTE: Must match overlay's spi0 status to avoid resource waste */
#undef F_APP_WIFI_SPI_CMD
#define F_APP_WIFI_SPI_CMD                  0
#if F_APP_WIFI_SPI_CMD
#undef F_APP_SPI_ROLE_MASTER
#define F_APP_SPI_ROLE_MASTER               1
#endif
/* NOTE: Must match overlay's uart3 status to avoid resource waste */
#undef F_APP_WIFI_UART_CMD
#define F_APP_WIFI_UART_CMD                 1


#undef F_APP_WIFI_PTA_SUPPORT
#define F_APP_WIFI_PTA_SUPPORT              0

#define F_APP_INTEGRATED_TRANSCEIVER        1

#if F_APP_INTEGRATED_TRANSCEIVER
#undef F_APP_MULTILINK_ENABLE
#define F_APP_MULTILINK_ENABLE              1
#undef F_APP_ACL_ROLE_FORCE_MASTER
#define F_APP_ACL_ROLE_FORCE_MASTER         1

#undef F_SOURCE_PLAY_SUPPORT
#define F_SOURCE_PLAY_SUPPORT               1
#undef F_APP_A2DP_SOURCE_SUPPORT
#define F_APP_A2DP_SOURCE_SUPPORT           1
#undef F_APP_HFP_AG_SUPPORT
#define F_APP_HFP_AG_SUPPORT                1
#undef F_APP_A2DP_SINK_SUPPORT
#define F_APP_A2DP_SINK_SUPPORT             1
#undef F_APP_HFP_HF_SUPPORT
#define F_APP_HFP_HF_SUPPORT                1

#undef F_APP_PAN_SUPPORT
#define F_APP_PAN_SUPPORT                   0

#if F_SOURCE_PLAY_SUPPORT
#undef F_APP_ATTACH_LOCAL_PLAY_SUPPORT
#define F_APP_ATTACH_LOCAL_PLAY_SUPPORT     0
#endif
#endif

#undef F_APP_USB_MSC_SUPPORT
#define F_APP_USB_MSC_SUPPORT               1
#undef F_APP_USB_CDC_SUPPORT
#define F_APP_USB_CDC_SUPPORT               1
#if F_APP_USB_CDC_SUPPORT
#undef IAD_SUPPORT
#define IAD_SUPPORT                         1
#endif
#undef F_APP_USB_HID_SUPPORT
#define F_APP_USB_HID_SUPPORT               1
#undef F_APP_CFU_FEATURE_SUPPORT
#define F_APP_CFU_FEATURE_SUPPORT           1

#define F_APP_RECORD_SAVE_SUPPORT           1

#undef F_APP_IAP_RTK_SUPPORT
#define F_APP_IAP_RTK_SUPPORT               0
#undef F_APP_IAP_SUPPORT
#define F_APP_IAP_SUPPORT                   0

#define CONFIG_REALTEK_APP_AI_RECORD        1
#define CONFIG_REALTEK_APP_AI_AUTH          1
#define CONFIG_REALTEK_APP_RTC_CALENDAR_SUPPORT      1

#endif /* end of F_APP_AI_RECORD_PEN_DEMO_SUPPORT */

#endif
