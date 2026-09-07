/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __APP_CHARGEBOX_H__
#define __APP_CHARGEBOX_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "app_panel_le_db.h"

#define BASS_SCAN_DEV_NUM_MAX   (100)
#define MUSIC_TITLE_MAX_LENGTH  (512)
#define MUSIC_ARTIST_MAX_LENGTH  (128)
#define MUSIC_ALBUM_MAX_LENGTH  (32)
#define MUSIC_SCROLL_TEXT_DURATION  (3*1000)

typedef enum
{
    AVRCP_TITLE = 1,
    AVRCP_NAME_OF_ARTIST,
    AVRCP_NAME_OF_ALBUM,
    AVRCP_NUMBER_OF_MEDIA,
    AVRCP_TOTAL_NUMBER_OF_MEDIA,
    AVRCP_GENRE,
    AVRCP_PLAYING_TIME_IN_MILLISECOND,
} APP_AVRCP_ATTRIBUTE_TYPE;

extern void *get_app_chargebox(void);
extern void page_tb_conn(void *parent);
extern void page_tb_func(void *parent);
extern void page_tb_music(void *parent);
void app_tb_set_device_name(uint8_t *data, uint8_t length);
void app_tb_set_battery(uint8_t case_battery, uint8_t left_battery, uint8_t right_battery);
void app_tb_set_bt_link_status(uint8_t *bd_addr,  T_APP_GUI_LE_LINK_STATUS link_status);
void app_tb_set_bt_volume_value(uint8_t *bd_addr,  uint8_t volume_value);
void app_tb_set_bt_anc_status(uint8_t *bd_addr,  T_APP_GUI_LISTENING_MODE anc_enable);
void app_tb_add_scan_dev(uint8_t idx, uint8_t *device_name);

void app_tb_set_title(uint8_t *title, uint16_t length);
void app_tb_set_artist(uint8_t *artist, uint16_t length);
void app_tb_set_album(uint8_t *album, uint16_t length);
void app_chargebox_ui_design(void);

#ifdef __cplusplus
}
#endif
#endif
