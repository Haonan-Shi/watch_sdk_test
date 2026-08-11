/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <string.h>
#include <trace.h>
#include <stddef.h>
#include "ftl.h"
#include "fmc_api.h"
#include "rtl876x.h"
#include "test_mode.h"
#include "eq_utils.h"
#include "gap_le.h"
#include "app_audio_cfg.h"
#include "app_bt_policy_cfg.h"
#include "app_cfg.h"
#include "app_cfg_nv.h"
#include "app_main.h"
#include "wdg.h"
#include "app_mmi.h"
#include "app_charger.h"
#include "btm.h"
#include "bt_hfp.h"
#include "bt_hfp_ag.h"
#include "app_reset_reason.h"

#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
#include "rtl876x_rcc.h"
#include "rtl876x_pinmux.h"
#include "hal_gpio.h"
#include "platform_utils.h"
#endif

#if F_APP_SD_CARD_SUPPORT
#include "sd.h"
#endif

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
#include "app_gfps.h"
#endif

#if F_APP_SD_CARD_SUPPORT
#ifdef CONFIG_SOC_SERIES_RTL87X3G
#define SD_CARD_PLAY_SDHC_ID         SDHC_ID0
#else
#define SD_CARD_PLAY_SDHC_ID         SDHC_ID0
#endif

#if (CONFIG_SOC_SERIES_RTL8773D == 1 || TARGET_RTL8773DFL == 1)
T_SD_CONFIG sd_card_play_cfg =
{
#if F_APP_AI_RECORD_PEN_DEMO_SUPPORT
    .sd_if_type = SD_IF_MMC,
#else
    .sd_if_type = SD_IF_SD_CARD,
#endif

    .sdh_group = GROUP_1,
    .sdh_bus_width = SD_BUS_WIDTH_4B,
    .sd_bus_clk_sel = SD_BUS_CLK_20M,
};
#elif (CONFIG_SOC_SERIES_RTL87X3G == 1)
T_SD_CONFIG sd_card_play_cfg =
{
    .sd_if_type = SD_IF_SD_CARD,
    .sdh_group = GROUP_0,
    .sd_bus_clk_sel = SD_BUS_CLK_25M,
    .sdh_bus_width = SD_BUS_WIDTH_1B,
    .sd_mutex_en = 1,
    .sd_power_en = 1,
    .sd_power_high_active = 1,
    .sd_power_pin = P2_3
};
#else
T_SD_CONFIG sd_card_play_cfg =
{
    .sd_if_type = SD_IF_MMC,
    .sdh_group = GROUP_0,
#if (TARGET_RTL8773EWE == 1 || TARGET_RTL8773EWE_VP == 1)
    .sdh_bus_width = SD_BUS_WIDTH_1B,
#else
    .sdh_bus_width = SD_BUS_WIDTH_4B,
#endif
    .sd_bus_clk_sel = SD_BUS_CLK_20M,
    .sd_debug_en = 0,
    .sd_power_en = 1,
    .sd_mutex_en = 1,
    .sd_power_high_active = 1,
#if(TARGET_RTL8773EWE == 1 || TARGET_RTL8773EWE_VP == 1 || TARGET_RTL8773EWP == 1)
    .sd_power_pin = P2_3
#else
    .sd_power_pin = P5_6
#endif
};
#endif

#endif

T_APP_CFG_CONST app_cfg_const;

void app_mcu_cfg_const_load(void)
{
    uint32_t sync_word = 0;

    fmc_flash_nor_read((flash_cur_bank_img_payload_addr_get(FLASH_IMG_MCUCONFIG) +
                        APP_CONFIG_OFFSET),
                       (uint8_t *)&sync_word, APP_MCU_CFG_SYNC_WORD_LEN);

    if (sync_word == APP_MCU_CFG_SYNC_WORD)
    {
        fmc_flash_nor_read((flash_cur_bank_img_payload_addr_get(FLASH_IMG_MCUCONFIG) +
                            APP_CONFIG_OFFSET),
                           (uint8_t *)&app_cfg_const, sizeof(T_APP_CFG_CONST));
    }
    else
    {
        wdg_reset_with_reason(RESET_ALL, APP_RESET_REASON_MCU_CFG_LOAD_FAIL);
    }
}

uint32_t app_mcu_cfg_get_cod(void)
{
    uint32_t class_of_device = 0;

#if F_APP_BT_AUDIO_TRANSMITTER_DEMO_SUPPORT || F_APP_BT_AUDIO_TRANSMITTER_MP3_DEMO_SUPPORT
    class_of_device |= SERVICE_CLASS_RENDERING | SERVICE_CLASS_CAPTURING | SERVICE_CLASS_AUDIO;
#if F_APP_HFP_AG_SUPPORT
    class_of_device |= SERVICE_CLASS_TELEPHONY;
#endif
    class_of_device |= MAJOR_DEVICE_CLASS_AUDIO;
    class_of_device |= MINOR_DEVICE_CLASS_PORTABLEAUDIOLE;
#endif

#if F_APP_BT_AUDIO_RECEIVER_DEMO_SUPPORT || F_APP_BT_AUDIO_TRANSCEIVER_DEMO_SUPPORT
    class_of_device |= SERVICE_CLASS_RENDERING | SERVICE_CLASS_AUDIO;
#if F_APP_LE_AUDIO_ACCEPTOR_SUPPORT && F_APP_BREDR_SC_CTKD_SUPPORT
    class_of_device |= SERVICE_CLASS_LE_AUDIO;
#endif
    class_of_device |= MAJOR_DEVICE_CLASS_AUDIO;
    class_of_device |= MINOR_DEVICE_CLASS_HEADPHONES;
#endif

#if F_APP_AI_RECORD_PEN_DEMO_SUPPORT
    class_of_device |= SERVICE_CLASS_RENDERING | SERVICE_CLASS_CAPTURING | SERVICE_CLASS_AUDIO;
#if F_APP_LE_AUDIO_ACCEPTOR_SUPPORT && F_APP_BREDR_SC_CTKD_SUPPORT
    class_of_device |= SERVICE_CLASS_LE_AUDIO;
#endif
    class_of_device |= MAJOR_DEVICE_CLASS_AUDIO;
    class_of_device |= MINOR_DEVICE_CLASS_PORTABLEAUDIOLE;
#endif

    return class_of_device;
}

void app_mcu_cfg_init(void)
{
    app_mcu_cfg_const_load();

    uint32_t offset_start = offsetof(__typeof__(app_cfg_const), timer_hfp_ring_period);
    memset(&app_cfg_const.timer_hfp_ring_period, 0, sizeof(app_cfg_const) - offset_start);

    app_cfg_const.enable_dlps = 1;
    app_cfg_const.bud_role = REMOTE_SESSION_ROLE_SINGLE;
    app_cfg_const.bud_side = 0;
    app_cfg_const.le_name_sync_to_legacy_name = true;

    app_cfg_const.enable_data_uart = 1;
    app_cfg_const.enable_tx_wake_up = 0;
    app_cfg_const.enable_rx_wake_up = 0;

    //app_cfg_const.data_uart_tx_pinmux = DATA_UART_TX_PINMUX;
    //app_cfg_const.data_uart_rx_pinmux = DATA_UART_RX_PINMUX;

    app_cfg_const.line_in_support = 0;

    app_cfg_const.thermistor_power_vpa_support = 1;

#if F_APP_HIFI4_SUPPORT
    app_cfg_const.ext_buck_support = 1;
    app_cfg_const.buck_enable_pinmux = BUCK_PWR_EN;
#endif

#if F_APP_MONITOR_MEMORY_AND_TIMER
    app_cfg_const.timer_monitor_heap_and_timer_timeout = 10;
#endif

    uint32_t class_of_device = app_mcu_cfg_get_cod();
    memcpy(app_cfg_const.class_of_device, &class_of_device, 3);

    app_cfg_const.supported_profile_mask = A2DP_PROFILE_MASK | AVRCP_PROFILE_MASK | HFP_PROFILE_MASK |
                                           HSP_PROFILE_MASK |
                                           SPP_PROFILE_MASK | PBAP_PROFILE_MASK | HID_PROFILE_MASK | GATT_PROFILE_MASK;

#if F_APP_IAP_SUPPORT || F_APP_HIFI4_SUPPORT
    app_cfg_const.supported_profile_mask |= IAP_PROFILE_MASK;
    //app_cfg_const.i2c_0_dat_pinmux = I2C_0_DAT_PINMUX;
    //app_cfg_const.i2c_0_clk_pinmux = I2C_0_CLK_PINMUX;
#endif

//    app_cfg_const.dsp_log_pin = PIN_DSP_LOG_OUTPUT;
    app_cfg_const.dsp_log_output_select = DSP_OUTPUT_LOG_BY_MCU;

    app_cfg_const.enable_power_off_to_dlps_mode = 0;
    app_cfg_const.timer_auto_power_off = 0; // uint is seconds


#if (F_APP_BT_AUDIO_TRANSMITTER_DEMO_SUPPORT || F_APP_BT_AUDIO_RECEIVER_DEMO_SUPPORT || F_APP_BT_AUDIO_TRANSCEIVER_DEMO_SUPPORT)
#if !F_APP_INTEGRATED_TRANSCEIVER
    app_cfg_const.enable_power_on_adv_with_timeout = true;
#endif
#endif

#if F_APP_AI_RECORD_PEN_DEMO_SUPPORT
    app_cfg_const.enable_power_on_adv_with_timeout = true;
#endif

#if CONFIG_REALTEK_GFPS_FEATURE_SUPPORT
    app_gfps_set_profile_mask();
#endif
}

#if F_APP_SD_CARD_SUPPORT
void app_sd_card_init(void)
{
#if CONFIG_SOC_SERIES_RTL87X3G
    *((uint32_t *) 0x400002ac) |= (0xf << 20);
#endif
    sd_config_init(SD_CARD_PLAY_SDHC_ID, (T_SD_CONFIG *)&sd_card_play_cfg);
    sd_board_init(SD_CARD_PLAY_SDHC_ID);
    sd_card_init(SD_CARD_PLAY_SDHC_ID);
}
#endif
