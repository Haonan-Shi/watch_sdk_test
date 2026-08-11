/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "app_panel.h"
#if F_GUI_CHARGEBOX_DEMO
#include "trace.h"
#include "resource.h"
#include <gui_img.h>
#include "gui_win.h"
#include "gui_obj.h"
#include <gui_button.h>
#include "gui_scroll_text.h"
#include "font_mem.h"
#include "app_panel_le_msg.h"
#include "app_panel_db_common.h"
#include "app_panel_le_db.h"
#include "app_panel_device_db.h"
#include "app_panel_msg_util.h"

gui_text_t *text_music_title_prefix;
gui_scroll_text_t *text_music_title;
gui_text_t *text_music_artist;
gui_text_t *text_music_album;
static char music_title[MUSIC_TITLE_MAX_LENGTH];
static char music_artist[MUSIC_ARTIST_MAX_LENGTH];
static char music_album[MUSIC_ALBUM_MAX_LENGTH];
char *title_prefix = "Title: ";
char *artist_prefix = "Artist: ";
char *album_prefix = "Album: ";

#define APP_CHARGEBOX_BASIC_INFO_FONT_SIZE 24

void page_tb_music(void *parent)
{
    //gui_img_create_from_mem(parent, "page_music", BACKGROUND_BIN, 0, 0, 0, 0);
    gui_font_mem_init(SIMKAI_SIZE24_BITS1_FONT_BIN);

    text_music_title_prefix = gui_text_create(parent, "title txt prefix", 20, 15,
                                              gui_get_screen_width(), 32);
    gui_text_set(text_music_title_prefix, title_prefix, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX,
                                                                                  UINT8_MAX),
                 strlen(title_prefix),
                 APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_text_type_set(text_music_title_prefix, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);

    text_music_title = gui_scroll_text_create(parent, "title txt", 20, 60, gui_get_screen_width(), 32);
    gui_scroll_text_set(text_music_title, music_title, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX,
                                                                                 UINT8_MAX),
                        strlen(music_title),
                        APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_scroll_text_type_set(text_music_title, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);
    gui_scroll_text_encoding_set(text_music_title, UTF_8);

    snprintf(music_artist, sizeof(music_artist), "%s", artist_prefix);
    text_music_artist = gui_text_create(parent, "artist txt", 20, 105, gui_get_screen_width(), 32);
    gui_text_set(text_music_artist, music_artist, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX,
                                                                            UINT8_MAX),
                 strlen(music_artist),
                 APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_text_type_set(text_music_artist, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);

    snprintf(music_album, sizeof(music_album), "%s", album_prefix);
    text_music_album = gui_text_create(parent, "album txt", 20, 150, gui_get_screen_width(), 32);
    gui_text_set(text_music_album, music_album, GUI_FONT_SRC_BMP, gui_rgb(UINT8_MAX, UINT8_MAX,
                                                                          UINT8_MAX),
                 strlen(music_album),
                 APP_CHARGEBOX_BASIC_INFO_FONT_SIZE);
    gui_text_type_set(text_music_album, SIMKAI_SIZE24_BITS1_FONT_BIN, FONT_SRC_MEMADDR);
}

void app_tb_set_title(uint8_t *title, uint16_t length)
{
    uint8_t i = 0;
    gui_log("app_tb_set_title: %s", title);
    memset(music_title, 0, sizeof(music_title));

    //memcpy(music_title, title, length<MUSIC_TITLE_MAX_LENGTH?length:MUSIC_TITLE_MAX_LENGTH);
    snprintf(music_title, sizeof(music_title), "%s", title);

    if (text_music_title)
    {
        gui_scroll_text_content_set(text_music_title, music_title,
                                    length < MUSIC_TITLE_MAX_LENGTH ? length : MUSIC_TITLE_MAX_LENGTH);
        gui_scroll_text_scroll_set(text_music_title, SCROLL_X, 0, 0, MUSIC_SCROLL_TEXT_DURATION,
                                   0);
    }
}

void app_tb_set_artist(uint8_t *artist, uint16_t length)
{
    gui_log("app_tb_set_artist: %s", artist);
    memset(music_artist, 0, sizeof(music_artist));

    //memcpy(music_artist, artist, length<MUSIC_ARTIST_MAX_LENGTH?length:MUSIC_ARTIST_MAX_LENGTH);
    snprintf(music_artist, sizeof(music_artist), "%s%s", artist_prefix, artist);

    if (text_music_artist)
    {
        gui_text_content_set(text_music_artist, music_artist,
                             (length + strlen(artist_prefix)) < MUSIC_ARTIST_MAX_LENGTH ? (length + strlen(
                                         artist_prefix)) : MUSIC_ARTIST_MAX_LENGTH);
    }
}

void app_tb_set_album(uint8_t *album, uint16_t length)
{
    gui_log("app_tb_set_album: %s", album);
    memset(music_album, 0, sizeof(music_album));

    //memcpy(music_album, album, length<MUSIC_ALBUM_MAX_LENGTH?length:MUSIC_ALBUM_MAX_LENGTH);
    snprintf(music_album, sizeof(music_album), "%s%s", album_prefix, album);

    if (text_music_album)
    {
        gui_text_content_set(text_music_album, music_album,
                             (length + strlen(album_prefix)) < MUSIC_ALBUM_MAX_LENGTH ? (length + strlen(
                                         album_prefix)) : MUSIC_ALBUM_MAX_LENGTH);
    }
}

#endif
