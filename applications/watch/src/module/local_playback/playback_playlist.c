/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <string.h>
#include "trace.h"
#include "playback_playlist.h"
#include "section.h"
#include "fmc_api.h"
#include "os_sync.h"
#include "app_fs_if.h"
#include "app_task.h"
#include "audio_resource.h"

/*============================================================================*
 *                           Constants
 *============================================================================*/
#define SECTOR_SIZE             0x1000
#define NAME_BUF_SIZE           (100 * sizeof(T_HEAD_INFO))

/*============================================================================*
 *                            Variables
 *============================================================================*/
static int16_t play_index = 0;
static uint8_t name_buf[NAME_BUF_SIZE];
static T_HEAD_INFO *header_info = (T_HEAD_INFO *)(MUSIC_HEADER_BIN_ADDR +
                                                  FS_HEADER_INFO_START);
/* Songs deleted through BLE/SPP transfer do not update the header bin count. */
static uint16_t *song_count = (uint16_t *)MUSIC_HEADER_BIN_ADDR;


/*============================================================================*
 *                           Private Functions
 *============================================================================*/

/**
 * @brief Erases the specified header or name storage region.
 *
 * @param[in] addr Flash base address to erase.
 * @param[in] mode NOR flash erase mode.
 */
static void clear_header_name_info(uint32_t addr, FMC_FLASH_NOR_ERASE_MODE mode)
{
#if (CONFIG_APP_NANDBOOT == 0)
    fmc_flash_nor_erase(addr, mode);
#endif
}

/**
 * @brief Writes header or name data into the mapped storage region.
 *
 * @param[in] addr Destination address.
 * @param[in] data Source buffer.
 * @param[in] len Data length in bytes.
 */
static void write_header_name_info(uint32_t addr, void *data, uint32_t len)
{
#if (CONFIG_APP_NANDBOOT == 0)
    fmc_flash_nor_write(addr, data, len);
#else
    memcpy((uint8_t *)addr, data, len);
#endif
}

/**
 * @brief Filters deleted song headers from a header buffer in place.
 *
 * @param[in,out] header_in Header buffer to compact.
 * @param[in] len_in Input buffer length.
 * @param[out] len_out Output buffer length after deleted entries are removed.
 */
static void filter_del_header_info(uint8_t *header_in, uint16_t len_in, uint16_t *len_out)
{
    T_HEAD_INFO *hdr_info;
    uint16_t offset = 0;

    *len_out = len_in;

    while (len_in >= sizeof(T_HEAD_INFO))
    {
        hdr_info = (T_HEAD_INFO *)(header_in + offset);
        if (hdr_info->isDeleted)
        {
            if (len_in > sizeof(T_HEAD_INFO))
            {
                memmove(header_in + offset,
                        header_in + offset + sizeof(T_HEAD_INFO),
                        len_in - sizeof(T_HEAD_INFO));
            }
            *len_out -= sizeof(T_HEAD_INFO);
        }
        else
        {
            offset += sizeof(T_HEAD_INFO);
        }

        len_in -= sizeof(T_HEAD_INFO);
    }
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

/**
 * @brief Synchronizes playlist name and header data from the file system.
 *
 * This routine refreshes both cached bins and removes deleted song entries
 * from the header metadata before updating the stored song count.
 */
void playback_sync_playlist(void)
{
    uint32_t s;
    uint32_t name_offset = 0;
    uint32_t name_size = app_fs_get_name_bin_size();
    uint32_t read_len = 0;
    uint32_t head_size;
    uint16_t del_num = 0;
    uint32_t write_offset = MUSIC_HEADER_BIN_ADDR;
    uint32_t read_offset = 0;
    uint8_t header_cnt[FS_HEADER_INFO_START];
    uint16_t len_out = 0;

    s = os_lock();
    for (uint8_t i = 0; i < MUSIC_NAME_BIN_SIZE / SECTOR_SIZE; i++)
    {
        clear_header_name_info(MUSIC_NAME_BIN_ADDR + i * SECTOR_SIZE,
                               FMC_FLASH_NOR_ERASE_SECTOR);
    }
    os_unlock(s);

    APP_PRINT_INFO1("[playlist] name bin size = %d", name_size);

    for (uint16_t i = 0; i < name_size / NAME_BUF_SIZE; i++)
    {
        app_fs_read_name_bin(i * NAME_BUF_SIZE, name_buf, NAME_BUF_SIZE, &read_len);
        write_header_name_info(MUSIC_NAME_BIN_ADDR + i * NAME_BUF_SIZE,
                               name_buf,
                               NAME_BUF_SIZE);
        name_offset += NAME_BUF_SIZE;
    }

    if (name_size % NAME_BUF_SIZE)
    {
        app_fs_read_name_bin(name_offset, name_buf, name_size % NAME_BUF_SIZE, &read_len);
        write_header_name_info(MUSIC_NAME_BIN_ADDR + name_offset,
                               name_buf,
                               name_size % NAME_BUF_SIZE);
    }

    s = os_lock();
    SCB_InvalidateDCache_by_Addr((uint32_t *)MUSIC_NAME_BIN_ADDR, 0xA000);
    os_unlock(s);

    head_size = app_fs_get_header_bin_size();

    APP_PRINT_INFO1("[playlist] header bin size = %d", head_size);

    s = os_lock();
    for (uint8_t i = 0; i < MUSIC_HEADER_BIN_SIZE / SECTOR_SIZE; i++)
    {
        clear_header_name_info(MUSIC_HEADER_BIN_ADDR + i * SECTOR_SIZE,
                               FMC_FLASH_NOR_ERASE_SECTOR);
    }
    os_unlock(s);

    if (head_size >= FS_HEADER_INFO_START)
    {
        app_fs_read_header_bin(read_offset, header_cnt, FS_HEADER_INFO_START, &read_len);
        write_offset += FS_HEADER_INFO_START;
        read_offset += FS_HEADER_INFO_START;

        for (uint16_t i = 0; i < (head_size - FS_HEADER_INFO_START) / NAME_BUF_SIZE; i++)
        {
            app_fs_read_header_bin(read_offset, name_buf, NAME_BUF_SIZE, &read_len);
            filter_del_header_info(name_buf, NAME_BUF_SIZE, &len_out);
            del_num += (NAME_BUF_SIZE - len_out) / sizeof(T_HEAD_INFO);
            write_header_name_info(write_offset, name_buf, len_out);
            write_offset += len_out;
            read_offset += NAME_BUF_SIZE;
        }

        if ((head_size - FS_HEADER_INFO_START) % NAME_BUF_SIZE)
        {
            uint16_t len_in = (head_size - FS_HEADER_INFO_START) % NAME_BUF_SIZE;

            app_fs_read_header_bin(read_offset, name_buf, len_in, &read_len);
            filter_del_header_info(name_buf, len_in, &len_out);
            del_num += (len_in - len_out) / sizeof(T_HEAD_INFO);
            write_header_name_info(write_offset, name_buf, len_out);
        }
    }
    else
    {
        app_fs_read_header_bin(read_offset, header_cnt, head_size, &read_len);
    }

    uint16_t *count = (uint16_t *)header_cnt;

    *count -= del_num;
    write_header_name_info(MUSIC_HEADER_BIN_ADDR, header_cnt, FS_HEADER_INFO_START);

    s = os_lock();
    SCB_InvalidateDCache_by_Addr((uint32_t *)MUSIC_HEADER_BIN_ADDR, 0x5000);
    os_unlock(s);

    APP_PRINT_INFO2("del_num =%d, Header bin = %b",
                    del_num,
                    TRACE_BINARY(head_size, (uint8_t *)MUSIC_HEADER_BIN_ADDR));
}

uint16_t playback_get_song_count(void)
{
    return *song_count;
}

T_HEAD_INFO *playback_get_header_info_start(void)
{
    return header_info;
}

uint16_t playback_get_cur_play_index(void)
{
    return play_index;
}

void playback_reset_cur_play_index(void)
{
    play_index = 0;
}

T_HEAD_INFO *playback_get_cur_play_header_info(void)
{
    return header_info + play_index;
}

void playback_play_next_music(void)
{
    play_index++;

    if (play_index >= *song_count)
    {
        play_index = 0;
    }

    APP_PRINT_INFO2("play next:play_index = %d, song_count = %d",
                    play_index,
                    *song_count);

    T_IO_MSG play_msg;

    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_PLAY_BY_NAME;
    play_msg.u.param = (uint32_t)(header_info + play_index);
    app_send_msg_to_apptask(&play_msg);
}

void playback_play_prev_music(void)
{
    play_index--;

    if (play_index <= 0)
    {
        play_index = 0;
    }

    APP_PRINT_INFO2("play prev:play_num = %d, song_count = %d,",
                    play_index,
                    *song_count);

    T_IO_MSG play_msg;

    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_PLAY_BY_NAME;
    play_msg.u.param = (uint32_t)(header_info + play_index);
    app_send_msg_to_apptask(&play_msg);
}

void playback_play_select_music(uint16_t index)
{
    play_index = index;

    if (play_index <= 0)
    {
        play_index = 0;
    }

    if (play_index >= *song_count)
    {
        play_index = 0;
    }

    APP_PRINT_INFO2("play select:play_num = %d, song_count = %d,",
                    play_index,
                    *song_count);

    T_IO_MSG play_msg;

    play_msg.type = IO_MSG_TYPE_WRISTBNAD;
    play_msg.subtype = IO_MSG_PLAY_BY_NAME;
    play_msg.u.param = (uint32_t)(header_info + play_index);
    app_send_msg_to_apptask(&play_msg);
}
