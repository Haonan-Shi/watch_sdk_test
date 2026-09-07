/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
  *                                  Header Files
  *============================================================================*/
#include "audio_fs.h"
#include "pm.h"
#include "wdg.h"
#include "aon_wdg_ext.h"
#include "rtl876x_wdg.h"

#define     FILE_NUM_OPENED_TOGETHER    1
#define     PLAYLIST_INFO_SIZE          1024
static T_OS_QUEUE     audio_fs_list;
static bool           g_is_all_list_opened    = false;
static uint16_t       g_initStatus            = AUDIO_FS_NO_INIT;
static T_AUDIO_FS_DB  audio_fs_db;

static const TCHAR    *g_header_file      = (const TCHAR *)_T("header.bin");
static const TCHAR    *g_name_file        = (const TCHAR *)_T("name.bin");
static const TCHAR    *g_playlist_file    = (const TCHAR *)_T("playlist.bin");
static const TCHAR    *g_driver_path      = (const TCHAR *)_T("0:");

uint32_t g_offset = 0;
uint16_t g_curHeaderCount = 0;

static uint16_t audio_fs_open_all_list(void);
/**
 * @description: read count from header.bin
 * @param
 *      p_count
 * @return
 *      0:      success
 *      other:  fail
 */
static uint16_t audio_fs_read_header_count(uint16_t *p_count)
{
    uint16_t res = AUDIO_FS_OK;
    UINT len = 0;
    uint16_t count = 0;

    if (g_is_all_list_opened == false)
    {
        res = audio_fs_open_all_list();
    }
    if (g_is_all_list_opened)
    {
        f_lseek(audio_fs_db.header_fil, AUDIO_FS_HEADER_COUNT);
        res = f_read(audio_fs_db.header_fil, (void *)&count, (UINT)AUDIO_FS_HEADER_COUNT_SIZE, &len);
        if ((res == FR_OK) && (len != AUDIO_FS_HEADER_COUNT_SIZE))
        {
            res = AUDIO_FS_ERR_READ;
        }
        else if ((res == FR_OK) && (len == AUDIO_FS_HEADER_COUNT_SIZE))
        {
            *p_count = count;
        }
    }
    return res;
}

/**
 * @description: write count to header.bin
 * @param
 *      count
 * @return
 *      0:      success
 *      other:  fail
 */
static uint16_t audio_fs_write_header_count(uint16_t count)
{
    uint16_t res = FR_OK;
    UINT len = 0;

    if (g_is_all_list_opened == false)
    {
        res = audio_fs_open_all_list();
    }
    if (g_is_all_list_opened)
    {
        f_lseek(audio_fs_db.header_fil, AUDIO_FS_HEADER_COUNT);
        res = f_write(audio_fs_db.header_fil, (void *)&count, (UINT)AUDIO_FS_HEADER_COUNT_SIZE, &len);
        if ((res == FR_OK) && (len != AUDIO_FS_HEADER_COUNT_SIZE))
        {
            res = AUDIO_FS_ERR_WRITE;
        }
    }
    return res;
}

/**
 * @description: read info from header.bin
 * @param
 *      fileIdx
 *      pInfo
 *      num: num of T_SONG_NAME_INFO
 * @return
 *      0:      success
 *      other:  fail
 */
static uint16_t audio_fs_read_header_info(uint16_t fileIdx, T_SONG_NAME_INFO *pInfo, uint16_t num)
{
    UINT len = 0;
    uint16_t res = AUDIO_FS_ERR_PARAM;
    if (pInfo != NULL)
    {
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.header_fil, AUDIO_FS_HEADER_INFO_START + fileIdx * sizeof(T_SONG_NAME_INFO));
            res = f_read(audio_fs_db.header_fil, pInfo, num * sizeof(T_SONG_NAME_INFO), &len);
            if ((res == FR_OK) && (len != (num * sizeof(T_SONG_NAME_INFO))))
            {
                res = AUDIO_FS_ERR_READ;
            }
            else if ((res == FR_OK) && (len == (num * sizeof(T_SONG_NAME_INFO))))
            {
                res = AUDIO_FS_OK;
            }
        }
    }
    return res;
}

/**
 * @description: write info to header.bin
 * @param
 *      fileIdx
 *      pInfo
 *      num: num of T_SONG_NAME_INFO
 * @return
 *      0:      success
 *      other:  fail
 */
static uint16_t audio_fs_write_header_info(uint16_t fileIdx, T_SONG_NAME_INFO *pInfo, uint16_t num)
{
    UINT len = 0;
    uint16_t res = AUDIO_FS_ERR_PARAM;
    if (pInfo != NULL)
    {
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.header_fil, AUDIO_FS_HEADER_INFO_START + fileIdx * sizeof(T_SONG_NAME_INFO));
            res = f_write(audio_fs_db.header_fil, pInfo, num * sizeof(T_SONG_NAME_INFO), &len);
            if ((res == FR_OK) && (len != (num * sizeof(T_SONG_NAME_INFO))))
            {
                res = AUDIO_FS_ERR_WRITE;
            }
            else if ((res == FR_OK) && (len == (num * sizeof(T_SONG_NAME_INFO))))
            {
                res = AUDIO_FS_OK;
            }
        }
    }
    return res;
}

/**
 * @description: read name info from name.bin
 * @param
 *      offset: location of file pointer
 *      length: length of filename
 *      pFileName
 * @return
 *      0:      success
 *      other:  fail
*/
static uint16_t audio_fs_read_name_info(uint32_t offset, uint16_t length, TCHAR *pFileName)
{
    UINT readLen = 0;
    uint16_t res = AUDIO_FS_ERR_PARAM;
    if (pFileName != NULL)
    {
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.name_fil, offset);
            res = f_read(audio_fs_db.name_fil, pFileName, length, &readLen);
            if ((res == FR_OK) && (readLen != length))
            {
                res = AUDIO_FS_ERR_READ;
            }
            else if ((res == FR_OK) && (readLen == length))
            {
                pFileName[readLen / 2] = '0';
                res = AUDIO_FS_OK;
            }
        }
    }
    return res;
}

/**
 * @description: write name info to name.bin
 * @param
 *      offset: location of file pointer
 *      length: length of filename
 *      pFileName
 * @return
 *      0:      success
 *      other:  fail
*/
static uint16_t audio_fs_write_name_info(uint32_t offset, uint16_t length, TCHAR *pFileName)
{
    UINT len = 0;
    uint16_t res = AUDIO_FS_ERR_PARAM;
    if (pFileName != NULL)
    {
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.name_fil, offset);
            res = f_write(audio_fs_db.name_fil, pFileName, length, &len);
            if ((res == FR_OK) && (len != length))
            {
                res = AUDIO_FS_ERR_WRITE;
            }
            else if ((res == FR_OK) && (len == length))
            {
                res = AUDIO_FS_OK;
            }
        }
    }
    return res;
}

static uint16_t audio_fs_read_playlist_info(uint16_t index, T_PLAY_LIST_INFO *pInfo, uint16_t num)
{
    uint16_t res = AUDIO_FS_ERR_PARAM;
    if (audio_fs_db.playlistSize == 0)
    {
        UINT len = 0;
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.playlist.playlist_fil,
                    AUDIO_FS_PLAYLIST_INFO_START + index * sizeof(T_PLAY_LIST_INFO));
            res = f_read(audio_fs_db.playlist.playlist_fil, pInfo, num * sizeof(T_PLAY_LIST_INFO), &len);
            if ((res == FR_OK) && (len != num * sizeof(T_PLAY_LIST_INFO)))
            {
                APP_PRINT_ERROR1("audio_fs_read_playlist_info: res:%d", res);
                res = AUDIO_FS_ERR_READ;
            }
            else if ((res == FR_OK) && (len == num * sizeof(T_PLAY_LIST_INFO)))
            {
                res = AUDIO_FS_OK;
            }
        }
    }
    else
    {
        uint32_t addrOffset = 2 + index * 3;/* plIndex + count + pInfo0 + pInfo1 + ... */
        T_PLAY_LIST_INFO *pTemp = (T_PLAY_LIST_INFO *)(audio_fs_db.playlist.playlistInfo + addrOffset);
        memcpy(pInfo, pTemp, num * sizeof(T_PLAY_LIST_INFO));
        res = AUDIO_FS_OK;
    }
    return res;
}

static uint16_t audio_fs_write_playlist_info(uint16_t index, T_PLAY_LIST_INFO info, uint16_t num)
{
    uint16_t res = AUDIO_FS_ERR_PARAM;
    if (audio_fs_db.playlistSize == 0)
    {
        UINT len = 0;
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.playlist.playlist_fil,
                    AUDIO_FS_PLAYLIST_INFO_START + index * sizeof(T_PLAY_LIST_INFO));
            res = f_write(audio_fs_db.playlist.playlist_fil, &info, num * sizeof(T_PLAY_LIST_INFO), &len);
            if ((res == FR_OK) && (len != num * sizeof(T_PLAY_LIST_INFO)))
            {
                APP_PRINT_ERROR1("audio_fs_write_playlist_info: res:%d", res);
                res = AUDIO_FS_ERR_WRITE;
            }
            else if ((res == FR_OK) && (len == num * sizeof(T_PLAY_LIST_INFO)))
            {
                res = AUDIO_FS_OK;
            }
        }
    }
    else
    {
        uint32_t addrOffset = 2 + index * 3;/* plIndex + count + pInfo0 + pInfo1 + ... */
        T_PLAY_LIST_INFO *pTemp = (T_PLAY_LIST_INFO *)(audio_fs_db.playlist.playlistInfo + addrOffset);
        memcpy(pTemp, &info, num * sizeof(T_PLAY_LIST_INFO));
        res = AUDIO_FS_OK;
    }
    return res;
}

static uint16_t audio_fs_read_playlistIndex(uint16_t *p_playlistIndex)
{
    uint16_t res = AUDIO_FS_ERR_PARAM;
    uint16_t playlistIndex = 1;
    if (audio_fs_db.playlistSize == 0)
    {
        UINT len = 0;
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.playlist.playlist_fil, AUDIO_FS_PLAYLIST_PLINDEX);
            res = f_read(audio_fs_db.playlist.playlist_fil, &playlistIndex, AUDIO_FS_PLAYLIST_PLINDEX_SIZE,
                         &len);
            if ((res == FR_OK) && (len != AUDIO_FS_PLAYLIST_PLINDEX_SIZE))
            {
                APP_PRINT_ERROR1("audio_fs_read_playlistIndex: res:%d", res);
                res = AUDIO_FS_ERR_READ;
            }
            else if ((res == FR_OK) && (len == AUDIO_FS_PLAYLIST_PLINDEX_SIZE) &&
                     (playlistIndex > 0) && (playlistIndex < 16))
            {
                *p_playlistIndex = playlistIndex;
                res = AUDIO_FS_OK;
            }
        }
    }
    else
    {
        playlistIndex = (*(uint16_t *)(audio_fs_db.playlist.playlistInfo));
        if ((playlistIndex > 0) && (playlistIndex < 16))
        {
            *p_playlistIndex = playlistIndex;
            res = AUDIO_FS_OK;
        }
    }
    return res;
}

static uint16_t audio_fs_write_playlistIndex(uint16_t playlistIndex)
{
    uint16_t res = AUDIO_FS_ERR_PARAM;
    if (audio_fs_db.playlistSize == 0)
    {
        UINT len = 0;
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.playlist.playlist_fil, AUDIO_FS_PLAYLIST_PLINDEX);
            res = f_write(audio_fs_db.playlist.playlist_fil, &playlistIndex, AUDIO_FS_PLAYLIST_PLINDEX_SIZE,
                          &len);
            if ((res == FR_OK) && (len != AUDIO_FS_PLAYLIST_PLINDEX_SIZE))
            {
                APP_PRINT_ERROR1("audio_fs_write_playlistIndex: res:%d", res);
                res = AUDIO_FS_ERR_WRITE;
            }
            else if ((res == FR_OK) && (len == AUDIO_FS_PLAYLIST_PLINDEX_SIZE))
            {
                res = AUDIO_FS_OK;
            }
        }
    }
    else
    {
        (*(uint16_t *)(audio_fs_db.playlist.playlistInfo)) = playlistIndex;
        res = AUDIO_FS_OK;
    }
    return res;
}

static uint16_t audio_fs_write_playlist_count(uint16_t count)
{
    uint16_t res = AUDIO_FS_ERR_PARAM;
    if (audio_fs_db.playlistSize == 0)
    {
        UINT len = 0;
        if (g_is_all_list_opened == false)
        {
            res = audio_fs_open_all_list();
        }
        if (g_is_all_list_opened)
        {
            f_lseek(audio_fs_db.playlist.playlist_fil, AUDIO_FS_PLAYLIST_COUNT);
            res = f_write(audio_fs_db.playlist.playlist_fil, &count, AUDIO_FS_PLAYLIST_COUNT_SIZE, &len);
            if ((res == FR_OK) && (len != AUDIO_FS_PLAYLIST_COUNT_SIZE))
            {
                APP_PRINT_ERROR1("audio_fs_write_playlist_count: res:%d", res);
                res = AUDIO_FS_ERR_WRITE;
            }
            else if ((res == FR_OK) && (len == AUDIO_FS_PLAYLIST_COUNT_SIZE))
            {
                res = AUDIO_FS_OK;
            }
        }
    }
    else
    {
        (*(uint16_t *)(audio_fs_db.playlist.playlistInfo + 1)) = count;
        res = AUDIO_FS_OK;
    }
    return res;
}

#define AUDIO_FS_READ_WRITE_LEN_MAX         512
#define AUDIO_FS_HEADER_INFO_NUM_MAX        (AUDIO_FS_READ_WRITE_LEN_MAX / sizeof(T_SONG_NAME_INFO))
/**
 * @description: compare filename from name.bin with p_filename, if find, store the songNameInfo to p_songNameInfo[0]
 * @param
 *      p_songNameInfo: filename from name.bin
 *      num:            the num of filename, smaller than AUDIO_FS_HEADER_INFO_NUM_MAX
 *      p_filename
 *      namelen
 * @return
 *      0:      find
 *      other:  not find
 */
static uint16_t audio_fs_compare_filename(uint16_t *p_fileIdx, T_SONG_NAME_INFO *p_Info,
                                          uint16_t num, TCHAR *p_filename, uint16_t namelen)
{
    uint16_t res = AUDIO_FS_OK;
    uint8_t name[PATH_LEN * sizeof(TCHAR)] = "";
    int idx = 0;

    if ((res = audio_fs_read_header_info(*p_fileIdx, p_Info, num)) == AUDIO_FS_OK)
    {
        for (idx = 0; idx < num; idx++)
        {
            res = audio_fs_read_name_info(p_Info[idx].offset, p_Info[idx].length, (TCHAR *)name);
#if (AUDIO_FS_DECODE_DEBUG == 1)
            APP_PRINT_INFO2("audio_fs_compare_filename: idx:%d, name(%b)", idx, TRACE_BINARY(p_Info[idx].length,
                            name));
#endif
            if (res == AUDIO_FS_OK)
            {
                if (0 != memcmp((TCHAR *)name, p_filename, namelen))
                {
                    res = AUDIO_FS_ERR_COMPARE;
                }
                else
                {
                    *p_fileIdx = (*p_fileIdx) + idx;
                    memcpy(&p_Info[0], &p_Info[idx], sizeof(T_SONG_NAME_INFO));
                    break;
                }
            }
        }
    }
    return res;
}

static uint16_t audio_fs_find_fileInfo(uint16_t *p_fileIdx, uint16_t num,
                                       TCHAR *p_filename, uint16_t namelen,
                                       T_SONG_NAME_INFO *p_songNameInfo)
{
    uint16_t res = AUDIO_FS_OK;
    uint16_t read_times = 0;
    uint16_t remain_num = 0;
    T_SONG_NAME_INFO *p_name_info = NULL;
    uint16_t fileIdx = 0;
    uint16_t p_info_len = 0;

    if ((num == 0) || (p_filename == NULL) || (p_songNameInfo == NULL))
    {
        res = AUDIO_FS_ERR_PARAM;
    }
    else
    {
        if (num < AUDIO_FS_HEADER_INFO_NUM_MAX)
        {
            p_info_len = num * sizeof(T_SONG_NAME_INFO);
        }
        else
        {
            p_info_len = AUDIO_FS_READ_WRITE_LEN_MAX;
        }
        p_name_info = os_mem_alloc(OS_MEM_TYPE_DATA, p_info_len);
        if (p_name_info == NULL)
        {
            res = AUDIO_FS_ERR_MALLOC;
        }
        else
        {
            read_times = num / AUDIO_FS_HEADER_INFO_NUM_MAX;
            remain_num = num % AUDIO_FS_HEADER_INFO_NUM_MAX;
            res = AUDIO_FS_ERR_COMPARE;
            for (int i = 0; i < read_times; i++)
            {
                memset(p_name_info, 0, p_info_len);
                fileIdx = *p_fileIdx + i * AUDIO_FS_HEADER_INFO_NUM_MAX;
                if (AUDIO_FS_OK == audio_fs_compare_filename(&fileIdx, p_name_info, AUDIO_FS_HEADER_INFO_NUM_MAX,
                                                             p_filename, namelen))
                {
                    *p_fileIdx = fileIdx;
                    memcpy(p_songNameInfo, &p_name_info[0], sizeof(T_SONG_NAME_INFO));
                    res = AUDIO_FS_OK;
                    break;
                }
            }
            if ((res == AUDIO_FS_ERR_COMPARE) && (remain_num > 0))
            {
                memset(p_name_info, 0, p_info_len);
                fileIdx = *p_fileIdx + read_times * AUDIO_FS_HEADER_INFO_NUM_MAX;
                if (AUDIO_FS_OK == audio_fs_compare_filename(&fileIdx, p_name_info, remain_num, p_filename,
                                                             namelen))
                {
                    *p_fileIdx = fileIdx;
                    memcpy(p_songNameInfo, &p_name_info[0], sizeof(T_SONG_NAME_INFO));
                    res = AUDIO_FS_OK;
                }
                else
                {
                    res = AUDIO_FS_ERR_COMPARE;
                }
            }
            os_mem_free(p_name_info);
            p_name_info = NULL;
        }

    }
    APP_PRINT_INFO4("audio_fs_find_fileInfo: res:0x%x, fileIndex:0x%x, num:0x%x, p_filename(%b)",
                    res, *p_fileIdx, num, TRACE_BINARY(namelen, p_filename));
    return res;
}

uint16_t audio_fs_get_filecount_from_playlist(void)
{
    uint16_t count = 0;
    UINT len = 0;
    uint16_t res = FR_OK;

    if (audio_fs_db.playlistSize == 0)
    {
        f_lseek(audio_fs_db.playlist.playlist_fil, 2);
        res = f_read(audio_fs_db.playlist.playlist_fil, &count, 2, &len);
        if ((res != FR_OK) || (len != 2))
        {
            count = AUDIO_FS_ERR_ERROR;
        }
    }
    else
    {
        count = (*((uint16_t *)(audio_fs_db.playlist.playlistInfo + 1)));
    }
    return count;
}

uint16_t audio_fs_set_playlist(uint16_t playlistIndex)
{
    uint16_t res = AUDIO_FS_OK, rdRes = 0;
    uint16_t count = 0;
    T_PLAY_LIST_INFO playListInfo;
    uint16_t playlistCount = 0;
    uint16_t fileIdx = 0;
    T_SONG_NAME_INFO *p_info = NULL;
    uint16_t p_info_len = 0;
    uint16_t read_times = 0;
    uint16_t remain_num = 0;

    APP_PRINT_INFO3("audio_fs_set_playlist, playlistSize:%d, playlistIndex:0x%x, sd_init_status:0x%x",
                    audio_fs_db.playlistSize, playlistIndex, g_initStatus);
    if (g_initStatus != AUDIO_FS_OK)
    {
        res = AUDIO_FS_NO_INIT;
    }
    else
    {
        if ((playlistIndex == 0) || (playlistIndex > 17))
        {
            res = AUDIO_FS_ERR_PARAM;
        }
        else
        {
            /* clear playlist info */
            if (audio_fs_db.playlistSize == 0)
            {
                f_lseek(audio_fs_db.playlist.playlist_fil, 0);
                f_truncate(audio_fs_db.playlist.playlist_fil);
            }
            else
            {
                memset(audio_fs_db.playlist.playlistInfo, 0x0, audio_fs_db.playlistSize);
            }
            /* update playlistIndex */
            audio_fs_write_playlistIndex(playlistIndex);
            rdRes = audio_fs_read_header_count(&count);
            if (rdRes != AUDIO_FS_OK)
            {
                res = AUDIO_FS_ERR_SET_PLAYLIST;
            }
            else if ((rdRes == AUDIO_FS_OK) && (count == 0))
            {
                res = AUDIO_FS_OK;
            }
            else
            {
                if (count < AUDIO_FS_HEADER_INFO_NUM_MAX)
                {
                    p_info_len = count * sizeof(T_SONG_NAME_INFO);
                }
                else
                {
                    p_info_len = AUDIO_FS_READ_WRITE_LEN_MAX;
                }
                p_info = os_mem_alloc(OS_MEM_TYPE_DATA, p_info_len);
                if (p_info == NULL)
                {
                    res = AUDIO_FS_ERR_MALLOC;
                }
                else
                {
                    read_times = count / AUDIO_FS_HEADER_INFO_NUM_MAX;
                    remain_num = count % AUDIO_FS_HEADER_INFO_NUM_MAX;
                    for (int i = 0; i < read_times; i++)
                    {
                        fileIdx = i * AUDIO_FS_HEADER_INFO_NUM_MAX;
                        memset(p_info, 0, p_info_len);
                        rdRes = audio_fs_read_header_info(fileIdx, p_info, AUDIO_FS_HEADER_INFO_NUM_MAX);
                        if (rdRes != AUDIO_FS_OK)
                        {
                            res = AUDIO_FS_ERR_SET_PLAYLIST;
                        }
                        else
                        {
                            for (int idx = 0; idx < AUDIO_FS_HEADER_INFO_NUM_MAX; idx++)
                            {
                                if (((p_info[idx].plIndex & (1 << (playlistIndex - 1))) != 0) && (p_info[idx].isDeleted == 0))
                                {
                                    playListInfo.offset = p_info[idx].offset;
                                    playListInfo.length = p_info[idx].length;
                                    audio_fs_write_playlist_info(playlistCount, playListInfo, 1);
                                    playlistCount++;
                                }
                            }
                        }
                    }
                    if ((remain_num > 0) && (res == AUDIO_FS_OK))
                    {
                        fileIdx = read_times * AUDIO_FS_HEADER_INFO_NUM_MAX;
                        memset(p_info, 0, p_info_len);
                        rdRes = audio_fs_read_header_info(fileIdx, p_info, remain_num);
                        if (rdRes != AUDIO_FS_OK)
                        {
                            res = AUDIO_FS_ERR_SET_PLAYLIST;
                        }
                        else
                        {
                            for (int idx = 0; idx < remain_num; idx++)
                            {
                                if (((p_info[idx].plIndex & (1 << (playlistIndex - 1))) != 0) && (p_info[idx].isDeleted == 0))
                                {
                                    playListInfo.offset = p_info[idx].offset;
                                    playListInfo.length = p_info[idx].length;
                                    audio_fs_write_playlist_info(playlistCount, playListInfo, 1);
                                    playlistCount++;
                                }
                            }
                        }
                    }
                    if (res == AUDIO_FS_OK)
                    {
                        audio_fs_write_playlist_count(playlistCount);
                        if (audio_fs_db.playlistSize == 0)
                        {
                            f_sync(audio_fs_db.playlist.playlist_fil);
                        }
                    }
                    os_mem_free(p_info);
                    p_info = NULL;
                }
            }
        }
    }
    APP_PRINT_INFO5("audio_fs_set_playlist: res:0x%x, rdRes:%d, playlistIndex:0x%x, headerCount:0x%x, playlistCount:0x%x",
                    res, rdRes, playlistIndex, count, playlistCount);
    return res;
}

uint16_t audio_fs_get_fileinfo(uint16_t fileIdx, uint8_t *pFileName, uint16_t *pNameLen)
{
    uint16_t res = AUDIO_FS_ERR_PARAM, rdRes = 0;
    T_PLAY_LIST_INFO playlistInfo;
    uint16_t playlistCount = audio_fs_get_filecount_from_playlist();
    uint16_t playlistIndex = 0;

    audio_fs_read_playlistIndex(&playlistIndex);

    if ((playlistCount != AUDIO_FS_ERR_ERROR) && (fileIdx < playlistCount))
    {
        memset(&playlistInfo, 0, sizeof(T_PLAY_LIST_INFO));
        audio_fs_read_playlist_info(fileIdx, &playlistInfo, 1);
        if (playlistInfo.length == 0)
        {
            res = AUDIO_FS_ERR_READ;
        }
        else
        {
            rdRes = audio_fs_read_name_info(playlistInfo.offset, playlistInfo.length, (TCHAR *)pFileName);
            if (rdRes != 0)
            {
                res = AUDIO_FS_ERR_READ;
            }
            else
            {
                *pNameLen = playlistInfo.length;
                T_AUDIO_FILE *audio_file = NULL;
                res = AUDIO_FS_ERR_ERROR;
                for (int i = 0; i < audio_fs_list.count; i++)
                {
                    audio_file = os_queue_peek(&audio_fs_list, i);
                    if (audio_file == NULL)
                    {
                        continue;
                    }
                    if ((audio_file->flag & 0x3) == 0)
                    {
                        audio_file->flag = 1;
                        audio_file->fileIndex = fileIdx;
                        audio_file->fileOffset = 0;
                        audio_file->namelen = *pNameLen;
                        memcpy((uint8_t *)audio_file->filename, pFileName, audio_file->namelen);
                        res = AUDIO_FS_OK;
                        APP_PRINT_INFO2("audio_fs_get_fileinfo: audio_file:%p, filename(%b)", audio_file,
                                        TRACE_BINARY(audio_file->namelen, audio_file->filename));
                        break;
                    }
                }
            }
        }
    }
    APP_PRINT_INFO5("audio_fs_get_fileinfo: res:0x%x, rdRes:%d, playlistIndex:0x%x, fileIndex:%d, playlistCount:%d",
                    res, rdRes, playlistIndex, fileIdx, playlistCount);
    return res;
}

static uint16_t audio_fs_before_get_frame(T_AUDIO_FS_HANDLE handle)
{
    return audio_fs_decode_before_get_frame(handle);
}

T_AUDIO_FS_HANDLE audio_fs_open(uint8_t *pFilename, uint16_t namelen, uint8_t open_mode)
{
    uint16_t res = AUDIO_FS_ERR_PARAM, rdRes = 0;
    FIL tempFil = {0};
    T_AUDIO_FS_HANDLE handle = NULL;

    memset(&tempFil, 0, sizeof(FIL));

    rdRes = f_open(&tempFil, (const TCHAR *)pFilename, open_mode);
    if (rdRes != FR_OK)
    {
        res = AUDIO_FS_ERR_OPEN;
        f_close(&tempFil);
    }
    else
    {
        T_AUDIO_FILE *audio_file = NULL;
        if (open_mode == (FA_CREATE_ALWAYS | FA_WRITE))
        {
            for (int i = 0; i < audio_fs_list.count; i++)
            {
                audio_file = os_queue_peek(&audio_fs_list, i);
                if ((audio_file != NULL) && ((audio_file->flag & 0x3) == 0))
                {
                    audio_file->flag = 2;
                    audio_file->fileOffset = 0;
                    audio_file->fil = tempFil;
                    audio_file->namelen = namelen;
                    memcpy((uint8_t *)audio_file->filename, (uint8_t *)pFilename, audio_file->namelen);
                    handle = (T_AUDIO_FS_HANDLE)audio_file;
                    res = AUDIO_FS_OK;
                    break;
                }
                else
                {
                    handle = NULL;
                    res = AUDIO_FS_ERR_ERROR;
                }
            }
        }
        else
        {
            for (int i = 0; i < audio_fs_list.count; i++)
            {
                audio_file = os_queue_peek(&audio_fs_list, i);
                if ((audio_file != NULL) && ((audio_file->flag & 0x3) == 1) &&
                    (memcmp((TCHAR *)pFilename, audio_file->filename, namelen) == 0))
                {
                    audio_file->fileOffset = 0;
                    audio_file->fil = tempFil;
                    handle = (T_AUDIO_FS_HANDLE)audio_file;
                    res = AUDIO_FS_OK;
                    break;
                }
                else
                {
                    handle = NULL;
                    res = AUDIO_FS_ERR_COMPARE;
                }
            }
            if (res == AUDIO_FS_ERR_COMPARE)
            {
                for (int i = 0; i < audio_fs_list.count; i++)
                {
                    audio_file = os_queue_peek(&audio_fs_list, i);
                    if ((audio_file != NULL) && ((audio_file->flag & 0x3) == 0))
                    {
                        audio_file->flag = 1;
                        audio_file->fileOffset = 0;
                        audio_file->fil = tempFil;
                        audio_file->namelen = namelen;
                        memcpy((uint8_t *)audio_file->filename, (uint8_t *)pFilename, audio_file->namelen);
                        handle = (T_AUDIO_FS_HANDLE)audio_file;
                        res = AUDIO_FS_OK;
                        break;
                    }
                    else
                    {
                        handle = NULL;
                        res = AUDIO_FS_ERR_ERROR;
                    }
                }
            }
            if ((res == AUDIO_FS_OK) && (handle != NULL))
            {
                APP_PRINT_INFO5("audio_fs_open: audio_file(%p), open_mode:0x%x, file_size:0x%x, namelen:0x%x, pFilename(%b)",
                                audio_file, open_mode, f_size(&tempFil), namelen, TRACE_BINARY(namelen, pFilename));
                if (audio_fs_decode_init() != 0)
                {
                    res = AUDIO_FS_ERR_PARAM;
                }
                else
                {
                    audio_fs_decode_set_frame_format((TCHAR *)pFilename, namelen);
                    if ((rdRes = audio_fs_before_get_frame(handle)) != 0)
                    {
                        res = AUDIO_FS_ERR_PARAM;
                    }
                }
            }
        }
    }
    APP_PRINT_INFO6("audio_fs_open: handle:0x%x, res:0x%x, rdRes:%d, open_mode:0x%x, namelen:0x%x, pFilename(%b)",
                    handle, res, rdRes, open_mode, namelen, TRACE_BINARY(namelen, pFilename));
    return handle;
}

uint16_t audio_fs_close(T_AUDIO_FS_HANDLE handle)
{
    uint16_t res = AUDIO_FS_ERR_ERROR, rdRes = 0;
    T_AUDIO_FILE *audio_file = NULL;

    for (int i = 0; i < audio_fs_list.count; i++)
    {
        audio_file = os_queue_peek(&audio_fs_list, i);
        if ((audio_file != NULL)  && (audio_file == (T_AUDIO_FILE *)handle) &&
            ((audio_file->flag & 0x3) != 0))
        {
            APP_PRINT_INFO3("audio_fs_close: handle:0x%x, audio_file->flag:%d, filename(%b)",
                            handle, (audio_file->flag & 0x3),
                            TRACE_BINARY(audio_file->namelen, audio_file->filename));
            if ((rdRes = f_close(&audio_file->fil)) != FR_OK)
            {
                res = AUDIO_FS_ERR_CLOSE;
            }
            else
            {
                if ((audio_file->flag & 0x3) == 1)
                {
                    /* close the reading file */
                    audio_fs_decode_deinit();
                }
                else if ((audio_file->flag & 0x3) == 2)
                {
                    //TODO  /* close the writing file */
                }
                /* clear the handle, can not memset the handle(p_next can not be clear) */
                memset(&audio_file->fil, 0, sizeof(FIL));
                memset(audio_file->filename, 0, sizeof(audio_file->filename));
                audio_file->fileOffset = 0;
                audio_file->namelen = 0;
                audio_file->fileIndex = 0;
                audio_file->flag = 0;
                res = AUDIO_FS_OK;
            }
            break;
        }
    }
    APP_PRINT_INFO2("audio_fs_close: res:0x%x, rdRes:%d", res, rdRes);

    extern bool g_is_mp3_info_obtained;
    g_is_mp3_info_obtained = false;

    return res;
}

uint32_t audio_fs_get_file_offset(T_AUDIO_FS_HANDLE handle)
{
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    return audio_file->fileOffset;
}

uint8_t *audio_fs_get_filename(T_AUDIO_FS_HANDLE handle)
{
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    return (uint8_t *)audio_file->filename;
}

uint16_t audio_fs_get_filenameLen(T_AUDIO_FS_HANDLE handle)
{
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    return audio_file->namelen;
}

uint16_t audio_fs_get_fileIndex(T_AUDIO_FS_HANDLE handle)
{
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    return audio_file->fileIndex;
}

void audio_fs_set_file_offset(T_AUDIO_FS_HANDLE handle, uint32_t offset)
{
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    audio_file->fileOffset = offset;
    f_lseek(&audio_file->fil, offset);
}

bool audio_fs_end_of_file(T_AUDIO_FS_HANDLE handle)
{
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    return f_eof(&audio_file->fil);
}

uint16_t audio_fs_get_frame(T_AUDIO_FS_HANDLE handle, FRAME_CONTENT *p_framContent)
{
    uint32_t beforeOffset = 0;
    uint32_t afterOffset = 0;
    uint16_t res = AUDIO_FS_ERR_PARAM;
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;

    if (p_framContent != NULL)
    {
        // check the end of file
        if (f_eof(&audio_file->fil))
        {
            res = FS_END_OF_FILE;
        }
        else
        {
            beforeOffset = audio_fs_get_file_offset(handle);
            res = audio_fs_decode_get_frame(handle, p_framContent);
            afterOffset = audio_fs_get_file_offset(handle);
            if (res != 0)
            {
                afterOffset = f_size(&audio_file->fil);
                audio_fs_set_file_offset(handle, afterOffset);
                APP_PRINT_INFO4("audio_fs_get_frame: res:0x%x, eof: 0x%x, offset(beforeGetFrame):0x%x, offset(afterGetFrame):0x%x",
                                res, f_eof(&audio_file->fil), beforeOffset, afterOffset);
                res = FS_END_OF_FILE;
            }
        }
    }
    return res;
}

uint16_t audio_fs_get_frame_para(T_AUDIO_FS_HANDLE handle, FRAME_INFO *p_frameInfo)
{
    uint32_t offset = 0;
    uint16_t res = AUDIO_FS_ERR_PARAM;
    FRAME_CONTENT frameContent;
    if (p_frameInfo != NULL)
    {
        offset = audio_fs_get_file_offset(handle);/* save the location when read paraInfo */
        res = audio_fs_decode_get_frame(handle, &frameContent);
        if (res == 0)
        {
            audio_fs_decode_get_frame_info(p_frameInfo);
            audio_fs_set_file_offset(handle, offset);
        }
    }
    APP_PRINT_INFO2("audio_fs_get_frame_para: res:%d, offset:0x%x", res, offset);
    return res;
}

uint16_t audio_fs_write(T_AUDIO_FS_HANDLE handle, uint8_t *writeBuf, uint32_t writeLen,
                        uint32_t *len)
{
    uint16_t res = 0;
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;

#if 0
    if ((res = f_write(&audio_file->fil, (void *)writeBuf, (UINT)writeLen, (UINT *)len)) != FR_OK)
    {
        APP_PRINT_ERROR1("audio_fs_write: write file failed! res:0x%x", res);
        return res;
    }
#else
    for (int i = 0; i < (writeLen / 512); i++)
    {
        if ((res = f_write(&audio_file->fil, (void *)writeBuf, (UINT)512, (UINT *)len)) != FR_OK)
        {
            APP_PRINT_ERROR2("audio_fs_write: write file failed! res:0x%x, LINE:%d", res, __LINE__);
            return res;
        }
        writeBuf += 512;
    }
    if ((writeLen % 512) != 0)
    {
        if ((res = f_write(&audio_file->fil, (void *)writeBuf, (UINT)(writeLen % 512),
                           (UINT *)len)) != FR_OK)
        {
            APP_PRINT_ERROR2("audio_fs_write: write file failed! res:0x%x, LINE:%d", res, __LINE__);
            return res;
        }
    }
#endif
    //f_sync(&audio_file->fil);
    return 0;
}
uint16_t audio_fs_read(T_AUDIO_FS_HANDLE handle, uint8_t *readBuf, uint32_t readLen, uint32_t *len)
{
    uint16_t res = 0;
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;

    if ((res = f_read(&audio_file->fil, (void *)readBuf, (UINT)readLen, (UINT *)len)) != FR_OK)
    {
        APP_PRINT_ERROR1("audio_fs_read: read file failed! res:0x%x", res);
        return res;
    }
    return 0;
}

uint32_t audio_fs_size(T_AUDIO_FS_HANDLE handle)
{
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;

    f_sync(&audio_file->fil);

    return f_size(&audio_file->fil);
}

#if (MP3_FORMAT_SUPPORT == 1)
uint16_t audio_fs_get_id3v2_InfoLen(uint16_t tagIdx)
{
    return audio_fs_decode_get_id3v2_InfoLen(tagIdx);
}

uint8_t *audio_fs_get_id3v2_Info(uint16_t tagIdx)
{
    return audio_fs_decode_get_id3v2_Info(tagIdx);
}

uint16_t audio_fs_id3v2Info(T_AUDIO_FS_HANDLE handle, uint16_t tagIdx)
{
    return audio_fs_decode_id3v2_info((void *)handle, tagIdx);
}
#endif

uint16_t audio_fs_add_file(T_AUDIO_FS_HANDLE handle, uint16_t playlistIndex)
{
    uint16_t count = 0;
    uint16_t res = AUDIO_FS_OK, rdRes = 0;
    T_SONG_NAME_INFO songNameInfo;
    uint16_t playlistIdx = 0;
    uint16_t playlistIndexTemp = playlistIndex;
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;

    /* if playlistIndex is invalid, set default playlistIndex as 1 */
    if (playlistIndexTemp > 16 || playlistIndexTemp == 0)
    {
        playlistIndexTemp = 1;
    }
    /* 1. read the last file to get the end of header.bin and name.bin */
    rdRes = audio_fs_read_header_count(&count);
    if (rdRes != AUDIO_FS_OK)
    {
        res = AUDIO_FS_ERR_ERROR;
    }
    else
    {
        memset(&songNameInfo, 0, sizeof(T_SONG_NAME_INFO));
        if (count == 0)
        {
            res = AUDIO_FS_OK;
            songNameInfo.offset = 0;
            songNameInfo.length = audio_file->namelen;
            songNameInfo.plIndex = playlistIndexTemp;
            songNameInfo.isDeleted = 0;
            songNameInfo.needToUnlink = 0;
        }
        else
        {
            // 2. check if filename has exist?
            uint16_t idx = 0;
            rdRes = audio_fs_find_fileInfo(&idx, count, audio_file->filename,
                                           audio_file->namelen, &songNameInfo);
            if (rdRes == AUDIO_FS_OK)/* filename is exist */
            {
                res = AUDIO_FS_ERR_ADD;
                if (songNameInfo.isDeleted == 1)
                {
                    songNameInfo.isDeleted = 0;
                    songNameInfo.needToUnlink = 0;
                    songNameInfo.plIndex = playlistIndexTemp;
                    rdRes = audio_fs_write_header_info(idx, &songNameInfo, 1);
                    if (rdRes != AUDIO_FS_OK)
                    {
                        res = AUDIO_FS_ERR_WRITE;
                    }
                    f_sync(audio_fs_db.header_fil);
                }
                else
                {
                    playlistIndexTemp = songNameInfo.plIndex;
                    APP_PRINT_INFO0("audio_fs_add_file: has a same file in SD card!");
                }
            }
            else/* filename not exist */
            {
                /* 3. get the lastest fileInfo from header.bin and name.bin */
                rdRes = audio_fs_read_header_info(count - 1, &songNameInfo, 1);
                if (rdRes != AUDIO_FS_OK)
                {
                    res = AUDIO_FS_ERR_READ;
                }
                else
                {
                    res = AUDIO_FS_OK;
                    songNameInfo.offset += songNameInfo.length;
                    songNameInfo.length = audio_file->namelen;
                    songNameInfo.plIndex = playlistIndexTemp;
                    songNameInfo.isDeleted = 0;
                    songNameInfo.needToUnlink = 0;
                }
            }
        }
        if (res == AUDIO_FS_OK)
        {
            /* 4. update header.bin and name.bin */
            rdRes = audio_fs_write_header_count(count + 1);
            if (rdRes != AUDIO_FS_OK)
            {
                res = AUDIO_FS_ERR_WRITE;
            }
            rdRes = audio_fs_write_header_info(count, &songNameInfo, 1);
            if (rdRes != AUDIO_FS_OK)
            {
                res = AUDIO_FS_ERR_WRITE;
            }
            rdRes = audio_fs_write_name_info(songNameInfo.offset, songNameInfo.length,
                                             (TCHAR *)audio_file->filename);
            if (rdRes != AUDIO_FS_OK)
            {
                res = AUDIO_FS_ERR_WRITE;
            }
            f_sync(audio_fs_db.header_fil);
            f_sync(audio_fs_db.name_fil);
        }
        if ((res == AUDIO_FS_OK) || (res == AUDIO_FS_ERR_ADD))
        {
            res = AUDIO_FS_OK;
            /* 7. update playlist */
            rdRes = audio_fs_read_playlistIndex(&playlistIdx);
            APP_PRINT_INFO2("audio_fs_add_file: playlistIndexTemp:0x%x, playlistIdx:0x%x",
                            playlistIndexTemp, playlistIdx);
            if ((rdRes == AUDIO_FS_OK) && (playlistIndexTemp == playlistIdx))
            {
                res = audio_fs_set_playlist(playlistIndexTemp);
            }
        }
    }
    APP_PRINT_INFO6("audio_fs_add_file: handle:0x%x, res:0x%x, rdRes:%d, count:0x%x, "
                    "playlistIndexTemp:0x%x, playlistIdx:0x%x",
                    handle, res, rdRes, count, playlistIndexTemp, playlistIdx);
    return res;
}

uint16_t audio_fs_spp_delete_file(uint8_t *pFileName, uint16_t namelen)
{
    uint16_t res = 0;
    res = f_unlink((TCHAR *)pFileName);
    return res;
}

uint16_t audio_fs_unlink_all_files(void)
{
    uint16_t res = 0, rdRes = 0;
    uint16_t filename_len = 0;
    uint16_t deleteCount = 0;
    FILINFO g_file_info;
    DIR dir;

    memset(&dir, 0, sizeof(DIR));
    memset(&g_file_info, 0, sizeof(FILINFO));
    if ((rdRes = f_opendir(&dir, g_driver_path)) != FR_OK)
    {
        res = AUDIO_FS_ERR_ERROR;
    }
    else
    {
        while (1)
        {
            rdRes = f_readdir(&dir, &g_file_info);
            if ((rdRes != FR_OK) || ((!g_file_info.fname[0])))
            {
                res = AUDIO_FS_OK;
                break;
            }
            if (g_file_info.fattrib & (AM_HID | AM_SYS))
            {
                //hidden or system file, no parse
                continue;
            }
            else if (g_file_info.fattrib & AM_ARC)
            {
                filename_len = sizeof(g_file_info.fname);
                rdRes = f_unlink(g_file_info.fname);
                if ((rdRes == 0) || (rdRes == FR_NO_FILE))
                {
                    deleteCount++;
                    res = AUDIO_FS_OK;
                }
                else
                {
                    APP_PRINT_INFO4("audio_fs_unlink_all_files: rdRes:%d, file size:%x, filename_len:%x, filename:%s",
                                    rdRes,
                                    g_file_info.fsize,
                                    filename_len,
                                    TRACE_BINARY(filename_len, g_file_info.fname));
                    res = AUDIO_FS_ERR_DELETE;
                }
            }
        }
        f_closedir(&dir);
    }
    APP_PRINT_INFO4("audio_fs_unlink_all_files, res:0x%x, rdRes:%d, deleteCount:%d, relative_path(%s)",
                    res, rdRes, deleteCount, TRACE_BINARY(4, audio_fs_db.relative_path));
    return res;
}

void audio_fs_unlink_file(void)
{
    uint16_t res = 0;
    uint16_t count = 0;
    T_SONG_NAME_INFO songNameInfo;
    uint8_t tempName[PATH_LEN * sizeof(TCHAR)] = "";
    uint16_t canBePlayCount = 0;

    f_sync(audio_fs_db.header_fil);
    res = audio_fs_read_header_count(&count);
    if (res == AUDIO_FS_ERR_ERROR)
    {
        res = AUDIO_FS_ERR_ERROR;
    }
    else if ((count == 0) && (f_size(audio_fs_db.header_fil) == 6) &&
             (f_size(audio_fs_db.name_fil) == 0))
    {
        /* delete all file */
        if ((res = audio_fs_unlink_all_files()) == AUDIO_FS_OK)
        {
            canBePlayCount = 0;
        }
    }
    else
    {
        canBePlayCount = count;
        for (int idx = 0; idx < count; idx++)
        {
            memset(&songNameInfo, 0, sizeof(T_SONG_NAME_INFO));
            if ((res = audio_fs_read_header_info(idx, &songNameInfo, 1)) != FR_OK)
            {
                res = AUDIO_FS_ERR_READ;
                break;
            }
            if ((songNameInfo.isDeleted == 1) && (songNameInfo.needToUnlink == 0))
            {
                canBePlayCount--;
            }
            else if ((songNameInfo.isDeleted == 1) && (songNameInfo.needToUnlink == 1))
            {
                canBePlayCount--;
                songNameInfo.needToUnlink = 0;
                if ((res = audio_fs_write_header_info(idx, &songNameInfo, 1)) != FR_OK)
                {
                    res = AUDIO_FS_ERR_WRITE;
                    break;
                }
                memset(tempName, 0, sizeof(tempName));
                if ((res = audio_fs_read_name_info(songNameInfo.offset, songNameInfo.length,
                                                   (TCHAR *)tempName)) != FR_OK)
                {
                    res = AUDIO_FS_ERR_READ;
                    break;
                }
                APP_PRINT_INFO2("audio_fs_unlink_file: idx:0x%x, songName(%b)",
                                idx, TRACE_BINARY(songNameInfo.length, tempName));
                res = f_unlink((TCHAR *)tempName);
                if ((res == FR_NO_FILE) || (res == 0))
                {
                    res = AUDIO_FS_OK;
                }
                else
                {
                    res = AUDIO_FS_ERR_DELETE;
                }
            }
        }
        f_sync(audio_fs_db.header_fil);
    }
    APP_PRINT_INFO3("audio_fs_unlink_file: res:0x%x, count:%d, canBePlayCount:%d",
                    res, count, canBePlayCount);
}

uint16_t audio_fs_delete_all_files(void)
{
    uint16_t rdRes = 0;
    rdRes = audio_fs_write_header_count(0);
    rdRes = f_lseek(audio_fs_db.header_fil, AUDIO_FS_HEADER_INFO_START);
    rdRes = f_truncate(audio_fs_db.header_fil);
    rdRes = f_lseek(audio_fs_db.name_fil, 0);
    rdRes = f_truncate(audio_fs_db.name_fil);
    rdRes = f_sync(audio_fs_db.header_fil);
    rdRes = f_sync(audio_fs_db.name_fil);
    APP_PRINT_INFO1("audio_fs_delete_all_files, rdRes:%d", rdRes);
    return rdRes;
}

uint16_t audio_fs_delete_file(uint8_t *pFileName, uint16_t namelen, uint16_t fileIndex)
{
    uint16_t res = AUDIO_FS_OK, rdRes = 0;
    T_SONG_NAME_INFO songNameInfo;
    uint16_t fileIdx = fileIndex;
    uint16_t headerCount = 0;

    memset(&songNameInfo, 0, sizeof(T_SONG_NAME_INFO));
    audio_fs_read_header_count(&headerCount);
    if (headerCount == 0)
    {
        res = AUDIO_FS_ERR_PARAM;
    }
    else
    {
#if (AUDIO_FS_DECODE_DEBUG == 1)
        APP_PRINT_INFO2("audio_fs_delete_file: fileIdx:%d, pFileName(%b)", fileIdx, TRACE_BINARY(namelen,
                        pFileName));
#endif
        res = audio_fs_compare_filename(&fileIdx, &songNameInfo, 1, (TCHAR *)pFileName, namelen);
        if (res == AUDIO_FS_ERR_COMPARE)
        {
            fileIdx = 0;
            memset(&songNameInfo, 0, sizeof(T_SONG_NAME_INFO));
            res = audio_fs_find_fileInfo(&fileIdx, headerCount, (TCHAR *)pFileName, namelen, &songNameInfo);
        }
        if (res == AUDIO_FS_OK)
        {
            /* check if the delete file has been opened */
            T_AUDIO_FILE *audio_file = NULL;
            for (int i = 0; i < audio_fs_list.count; i++)
            {
                audio_file = os_queue_peek(&audio_fs_list, i);
                if (audio_file != NULL)
                {
                    if ((audio_file->flag & 0x3) == 1)
                    {
                        if (memcmp((TCHAR *)pFileName, audio_file->filename, namelen) == 0)
                        {
                            APP_PRINT_INFO0("audio_fs_delete_file: the delete file has been opened");
                            res = AUDIO_FS_ERR_FILE_OPENED;
                        }
                    }
                }
            }
        }
        if (res == AUDIO_FS_OK)
        {
            /* update header.bin */
            if (songNameInfo.isDeleted == 0)
            {
                songNameInfo.isDeleted = 1;
                songNameInfo.needToUnlink = 1;
            }
            audio_fs_write_header_info(fileIdx, &songNameInfo, 1);
            f_sync(audio_fs_db.header_fil);
        }
        if (res == AUDIO_FS_OK)
        {
            /* update playlist.bin */
            uint16_t playlistIndexTemp = 0;
            rdRes = audio_fs_read_playlistIndex(&playlistIndexTemp);
            APP_PRINT_INFO2("audio_fs_delete_file: playlistIndexTemp:0x%x, plIndex:0x%x", playlistIndexTemp,
                            songNameInfo.plIndex);
            if ((rdRes == AUDIO_FS_OK) && (playlistIndexTemp & songNameInfo.plIndex) != 0)
            {
                rdRes = audio_fs_set_playlist(playlistIndexTemp);
                if (rdRes != AUDIO_FS_OK)
                {
                    res = AUDIO_FS_ERR_SET_PLAYLIST;
                }
            }
        }
    }
    APP_PRINT_INFO5("audio_fs_delete_file: res:0x%x, rdRes:%d, delete fileIdx:%d, headerCount:0x%x, isDeleted:%d",
                    res, rdRes, fileIdx, headerCount, songNameInfo.isDeleted);
    return res;
}

uint16_t audio_fs_update_playlist_index(uint8_t *pFileName, uint16_t namelen,
                                        uint16_t playlistIndex,
                                        uint16_t fileIndex)
{
    uint16_t res = AUDIO_FS_OK, rdRes = 0;
    T_SONG_NAME_INFO songNameInfo;
    uint8_t needToResetPlaylist = 0;
    uint16_t count = 0;
    uint16_t fileIdx = fileIndex;

    //find filename base on the fileIndex from Andriod APP
    memset(&songNameInfo, 0, sizeof(T_SONG_NAME_INFO));
    res = audio_fs_compare_filename(&fileIdx, &songNameInfo, 1, (TCHAR *)pFileName, namelen);
    if (res == AUDIO_FS_ERR_COMPARE)
    {
        if (AUDIO_FS_OK == audio_fs_read_header_count(&count))
        {
            fileIdx = 0;
            memset(&songNameInfo, 0, sizeof(T_SONG_NAME_INFO));
            res = audio_fs_find_fileInfo(&fileIdx, count, (TCHAR *)pFileName, namelen, &songNameInfo);
        }
    }
    if (res == AUDIO_FS_OK)
    {
        uint16_t playlistIndexTemp = 1;
        rdRes = audio_fs_read_playlistIndex(&playlistIndexTemp);
        if ((rdRes == AUDIO_FS_OK) &&
            (((playlistIndexTemp & songNameInfo.plIndex) != 0) || ((playlistIndexTemp & playlistIndex) != 0)))
        {
            needToResetPlaylist = 1;
        }
        /* update header bin */
        songNameInfo.plIndex = playlistIndex;
        audio_fs_write_header_info(fileIdx, &songNameInfo, 1);
        f_sync(audio_fs_db.header_fil);
        /* update playlist */
        if (needToResetPlaylist == 1)
        {
            if ((rdRes = audio_fs_set_playlist(playlistIndexTemp)) != FR_OK)
            {
                res = AUDIO_FS_ERR_SET_PLAYLIST;
            }
            else
            {
                res = AUDIO_FS_OK;
            }
        }
    }
    APP_PRINT_INFO4("audio_fs_update_playlist_index: res:0x%x, rdRes:%d, needToResetPlaylist:%d, pFileName(%b)",
                    res, rdRes, needToResetPlaylist, TRACE_BINARY(namelen, pFileName));
    return res;
}

uint32_t audio_fs_get_header_list_size(void)
{
    return f_size(audio_fs_db.header_fil);
}

uint32_t audio_fs_get_name_list_size(void)
{
    return f_size(audio_fs_db.name_fil);
}

uint16_t audio_fs_read_header_list(uint32_t offset, uint8_t *readBuf, uint32_t readLen,
                                   uint32_t *len)
{
    uint16_t res = 0;
    f_lseek(audio_fs_db.header_fil, offset);
    if ((f_eof(audio_fs_db.header_fil) == 1) && (f_tell(audio_fs_db.header_fil) != 0))
    {
        res = FS_END_OF_FILE;
    }
    else
    {
        res = f_read(audio_fs_db.header_fil, (void *)readBuf, (UINT)readLen, (UINT *)len);
    }
    return res;
}

uint16_t audio_fs_read_name_list(uint32_t offset, uint8_t *readBuf, uint32_t readLen, uint32_t *len)
{
    uint16_t res = 0;
    f_lseek(audio_fs_db.name_fil, offset);
    if ((f_eof(audio_fs_db.name_fil) == 1) && (f_tell(audio_fs_db.name_fil) != 0))
    {
        res = FS_END_OF_FILE;
    }
    else
    {
        res = f_read(audio_fs_db.name_fil, (void *)readBuf, (UINT)readLen, (UINT *)len);
    }
    return res;
}

/* return : status */
static uint16_t get_one_unicode_bytes(uint8_t *pInput, uint8_t *pUtfBytes, uint8_t *pUnicodeBytes)
{
    uint16_t res = AUDIO_FS_OK;
    if ((pInput == NULL) || (pUtfBytes == NULL) || (pUnicodeBytes == NULL))
    {
        res = AUDIO_FS_ERR_PARAM;
    }
    else
    {
        *pUtfBytes = 0x0;
        *pUnicodeBytes = 0x0;
        for (int idx = 7; idx >= 0; idx--)
        {
            if ((((*pInput) >> idx) & 0x1) == 0x1)
            {
                (*pUtfBytes)++;
            }
            else
            {
                break;
            }
        }
        switch (*pUtfBytes)
        {
        case 0:
            {
                *pUtfBytes = 1;
                *pUnicodeBytes = 2;
            }
            break;
        case 2:
            {
                if (((*(pInput + 1)) & 0xC0) != 0x80)
                {
                    res = AUDIO_FS_ERR_ERROR;
                }
                else
                {
                    *pUnicodeBytes = 2;
                }
            }
            break;
        case 3:
            {
                if ((((*(pInput + 1)) & 0xC0) != 0x80) || (((*(pInput + 2)) & 0xC0) != 0x80))
                {
                    res = AUDIO_FS_ERR_ERROR;
                }
                else
                {
                    *pUnicodeBytes = 2;
                }
            }
            break;
        case 4:
            {
                if ((((*(pInput + 1)) & 0xC0) != 0x80) || (((*(pInput + 2)) & 0xC0) != 0x80) ||
                    (((*(pInput + 3)) & 0xC0) != 0x80))
                {
                    res = AUDIO_FS_ERR_ERROR;
                }
                else
                {
                    *pUnicodeBytes = 3;
                }
            }
            break;
        case 5:
            {
                if ((((*(pInput + 1)) & 0xC0) != 0x80) || (((*(pInput + 2)) & 0xC0) != 0x80) ||
                    (((*(pInput + 3)) & 0xC0) != 0x80) || (((*(pInput + 4)) & 0xC0) != 0x80))
                {
                    res = AUDIO_FS_ERR_ERROR;
                }
                else
                {
                    *pUnicodeBytes = 4;
                }
            }
            break;
        case 6:
            {
                if ((((*(pInput + 1)) & 0xC0) != 0x80) || (((*(pInput + 2)) & 0xC0) != 0x80) ||
                    (((*(pInput + 3)) & 0xC0) != 0x80) || (((*(pInput + 4)) & 0xC0) != 0x80) ||
                    (((*(pInput + 5)) & 0xC0) != 0x80))
                {
                    res = AUDIO_FS_ERR_ERROR;
                }
                else
                {
                    *pUnicodeBytes = 4;
                }
            }
            break;
        default:
            {
                res = AUDIO_FS_ERR_ERROR;
            }
            break;
        }
    }
    return res;
}

uint16_t get_unicode_bytes(uint8_t *pInput, uint16_t inputBytes)
{
    uint16_t res = AUDIO_FS_OK;
    uint8_t oneUtfBytes = 0, oneUnicodeBytes = 0;
    uint16_t totalBytes = 0, unicodeBytes = 0;
    uint8_t *pInputTemp = (uint8_t *)pInput;
    while (totalBytes <= inputBytes)
    {
        if ((res = get_one_unicode_bytes(pInputTemp, &oneUtfBytes, &oneUnicodeBytes)) != AUDIO_FS_OK)
        {
            unicodeBytes = 0;
            APP_PRINT_ERROR1("get_unicode_bytes: res: 0x%x", res);
            break;
        }
        totalBytes += oneUtfBytes;
        unicodeBytes += oneUnicodeBytes;
        pInputTemp += oneUtfBytes;
        //APP_PRINT_INFO4("get_unicode_bytes: res: 0x%x, totalBytes: 0x%x, oneUtfBytes: %d, oneUnicodeBytes: %d",
        //                res, totalBytes, oneUtfBytes, oneUnicodeBytes);
    }
    return unicodeBytes;
}

uint16_t audio_fs_utf8_to_unicode(uint8_t *pInput, uint16_t inputBytes, uint8_t *pUnicode)
{
    uint16_t res = AUDIO_FS_OK;
    char b1, b2, b3, b4, b5, b6;
    uint16_t totalBytes = 0;
    uint8_t oneUtfBytes = 0, oneUnicodeBytes = 0;
    *pUnicode = 0x0;
    uint8_t *pOutput = (uint8_t *)pUnicode;
    if (inputBytes == 0)
    {
        return AUDIO_FS_ERR_PARAM;
    }
    while (totalBytes <= inputBytes)
    {
        if ((res = get_one_unicode_bytes(pInput, &oneUtfBytes, &oneUnicodeBytes)) != AUDIO_FS_OK)
        {
            break;
        }
        switch (oneUtfBytes)
        {
        case 1: /* 1byte */
            {
                *pOutput = (*pInput) & 0x7F;
                *(pOutput + 1)  = 0x00;
            }
            break;

        case 2: /* 2byte */
            {
                b1 = *pInput;
                b2 = *(pInput + 1);
                *pOutput = (b1 << 6) + (b2 & 0x3F);
                *(pOutput + 1) = (b1 & 0x1F);
            }
            break;
        case 3: /* 3byte */
            {
                b1 = *pInput;
                b2 = *(pInput + 1);
                b3 = *(pInput + 2);
                *pOutput = (b2 << 6) + (b3 & 0x3F);
                *(pOutput + 1) = (b1 << 4) + ((b2 >> 2) & 0x0F);
            }
            break;
        case 4: /* 4byte */
            {
                b1 = *pInput;
                b2 = *(pInput + 1);
                b3 = *(pInput + 2);
                b4 = *(pInput + 3);
                *pOutput     = (b3 << 6) + (b4 & 0x3F);
                *(pOutput + 1) = (b2 << 4) + ((b3 >> 2) & 0x0F);
                *(pOutput + 2) = ((b1 << 2) & 0x1C)  + ((b2 >> 4) & 0x03);
            }
            break;
        case 5: /* 5byte */
            {
                b1 = *pInput;
                b2 = *(pInput + 1);
                b3 = *(pInput + 2);
                b4 = *(pInput + 3);
                b5 = *(pInput + 4);
                *pOutput     = (b4 << 6) + (b5 & 0x3F);
                *(pOutput + 1) = (b3 << 4) + ((b4 >> 2) & 0x0F);
                *(pOutput + 2) = (b2 << 2) + ((b3 >> 4) & 0x03);
                *(pOutput + 3) = (b1 << 6);
            }
            break;
        case 6: /* 6byte */
            {
                b1 = *pInput;
                b2 = *(pInput + 1);
                b3 = *(pInput + 2);
                b4 = *(pInput + 3);
                b5 = *(pInput + 4);
                b6 = *(pInput + 5);
                *pOutput     = (b5 << 6) + (b6 & 0x3F);
                *(pOutput + 1) = (b5 << 4) + ((b6 >> 2) & 0x0F);
                *(pOutput + 2) = (b3 << 2) + ((b4 >> 4) & 0x03);
                *(pOutput + 3) = ((b1 << 6) & 0x40) + (b2 & 0x3F);
            }
            break;
        default:
            {
                res = AUDIO_FS_ERR_ERROR;
            }
            return res;
        }
        totalBytes += oneUtfBytes;
        pInput += oneUtfBytes;
        pOutput += oneUnicodeBytes;
        //APP_PRINT_INFO4("audio_fs_utf8_to_unicode: res: 0x%x, totalBytes: 0x%x, oneUtfBytes: %d, oneUnicodeBytes: %d",
        //                res, totalBytes, oneUtfBytes, oneUnicodeBytes);
    }
    return res;
}

#if 0
uint16_t utf16_to_unicode(uint16_t *pInput, uint16_t inputBytes, TCHAR *Unic)
{
    uint16_t b1, b2;
    *Unic = 0x0;
    uint16_t *pOutput = (uint16_t *) Unic;
    if (((*pInput) < 0xD800) || ((*pInput) > 0xDFFF))
    {
        *pOutput = *pInput;
        pOutput++;
        pInput++;
    }
    else if (0xD800 < (*pInput) < 0xDBFF)
    {
        if (((*(pInput + 1)) != NULL) && (0xDC00 < (*(pInput + 1)) < 0xDFFF))
        {
            b1 = *pInput;
            b2 = *(pInput + 1);
            *pOutput     = (b2 << 10) + (b1 & 0x3FFF);
            *(pOutput + 1) = (b2 >> 6) + (b2 & 0x0F);
            pInput  += 2;
            pOutput += 2;
        }
    }
}
#endif

uint16_t audio_fs_rename(T_AUDIO_FS_HANDLE handle, uint8_t *pFileName, uint16_t namelen)
{
    uint16_t res = 0;
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    uint8_t oldPath[PATH_LEN * sizeof(TCHAR)] = "";
    uint8_t newPath[PATH_LEN * sizeof(TCHAR)] = "";
    TCHAR tempCharacter = _T('0');

    f_sync(&audio_file->fil);

    memcpy((uint8_t *)oldPath, (uint8_t *)audio_file->filename, audio_file->namelen);
    memcpy((uint8_t *)newPath, (uint8_t *)pFileName, namelen);
    audio_file->namelen = namelen;
    memcpy((uint8_t *)audio_file->filename, pFileName, audio_file->namelen);
    while (tempCharacter < _T('9'))
    {
        res = f_rename((TCHAR *)oldPath, (TCHAR *)newPath);
        if (res == 0)
        {
            APP_PRINT_INFO1("audio_fs_rename: rename success! filename(%b)",
                            TRACE_BINARY(namelen + 2, audio_file->filename));
            return 0;
        }
        else if (res == FR_EXIST)
        {
            APP_PRINT_INFO0("audio_fs_rename: file is exist, rename it!");
            tempCharacter++;
            int idx = 0;
            for (idx = 0; (idx + 6) < namelen; idx += 2)
            {
                if ((pFileName[idx] == _T('.')) &&
                    ((pFileName[idx + 2] == _T('r')) || (pFileName[idx + 2] == _T('R'))) &&
                    ((pFileName[idx + 4] == _T('t')) || (pFileName[idx + 4] == _T('T'))) &&
                    ((pFileName[idx + 6] == _T('k')) || (pFileName[idx + 6] == _T('K'))) &&
                    ((pFileName[idx + 6] == _T('s')) || (pFileName[idx + 6] == _T('S'))))
                {
                    memcpy((uint8_t *)&audio_file->filename[0], (uint8_t *)&pFileName[0], idx);
                    memcpy((uint8_t *)&audio_file->filename[idx / 2], (uint8_t *)&tempCharacter, sizeof(TCHAR));
                    memcpy((uint8_t *)&audio_file->filename[idx / 2 + 1], (uint8_t *)&pFileName[idx], namelen - idx);
                    audio_file->namelen = namelen + sizeof(TCHAR);
                    memcpy((uint8_t *)newPath, (uint8_t *)audio_file->filename, audio_file->namelen);
                    break;
                }
            }
        }
        else
        {
            tempCharacter++;
        }
    }
    APP_PRINT_ERROR2("audio_fs_rename: res:%d, rename FAIL!pFileName(%b)", res, TRACE_BINARY(namelen,
                     pFileName));
    return res;
}

static uint16_t fs_get_filename_length(TCHAR *fname)
{
    uint16_t res = AUDIO_FS_OK;
    int len = 0;
    while ((fname[len] != 0) && (fname[len + 1] != 0) &&
           (fname[len + 2] != 0) && (fname[len + 3] != 0))
    {
        if (fname[len] == _T('.') &&
            ((fname[len + 1] == _T('r')) || (fname[len + 1] == _T('R'))) &&
            ((fname[len + 2] == _T('t')) || (fname[len + 2] == _T('T'))) &&
            ((fname[len + 3] == _T('k')) || (fname[len + 3] == _T('K'))))
        {
            len = len * sizeof(TCHAR);
            len += sizeof(_T(".rtk"));
            res = AUDIO_FS_OK;
            break;
        }
        else if (fname[len] == _T('.') &&
                 ((fname[len + 1] == _T('a')) || (fname[len + 1] == _T('A'))) &&
                 ((fname[len + 2] == _T('a')) || (fname[len + 2] == _T('A'))) &&
                 ((fname[len + 3] == _T('c')) || (fname[len + 3] == _T('C'))))
        {
            len = len * sizeof(TCHAR);
            len += sizeof(_T(".aac"));
            res = AUDIO_FS_OK;
            break;
        }
        else if (fname[len] == _T('.') &&
                 ((fname[len + 1] == _T('m')) || (fname[len + 1] == _T('M'))) &&
                 ((fname[len + 2] == _T('p')) || (fname[len + 2] == _T('P'))) &&
                 (fname[len + 3] == _T('3')))
        {
            len = len * sizeof(TCHAR);
            len += sizeof(_T(".mp3"));
            res = AUDIO_FS_OK;
            break;
        }
        else if (fname[len] == _T('.') &&
                 ((fname[len + 1] == _T('m')) || (fname[len + 1] == _T('M'))) &&
                 ((fname[len + 2] == _T('p')) || (fname[len + 2] == _T('P'))) &&
                 (fname[len + 3] == _T('4')))
        {
            len = len * sizeof(TCHAR);
            len += sizeof(_T(".mp4"));
            return len;
        }
        else if (fname[len] == _T('.') &&
                 ((fname[len + 1] == _T('f')) || (fname[len + 1] == _T('F'))) &&
                 ((fname[len + 2] == _T('l')) || (fname[len + 2] == _T('L'))) &&
                 ((fname[len + 3] == _T('a')) || (fname[len + 3] == _T('A'))) &&
                 ((fname[len + 3] == _T('c')) || (fname[len + 3] == _T('C'))))
        {
            len = len * sizeof(TCHAR);
            len += sizeof(_T(".flac"));
            res = AUDIO_FS_OK;
            break;
        }
        else
        {
            res = AUDIO_FS_ERR_ERROR;
        }
        len++;
    }
    if (res == AUDIO_FS_ERR_ERROR)
    {
        len = 0;
    }
    return len;
}

static uint16_t audio_fs_filename_write(TCHAR *pFileName, uint16_t namelen)
{
    uint16_t res = 0;
    T_SONG_NAME_INFO f_list_header;

    memset(&f_list_header, 0, sizeof(T_SONG_NAME_INFO));
    f_list_header.length = namelen;
    f_list_header.offset = g_offset;
    f_list_header.plIndex = 0x1;
    f_list_header.rsv = 0x0;
    res = audio_fs_write_header_info(g_curHeaderCount++, &f_list_header, 1);

    res = audio_fs_write_name_info(g_offset, f_list_header.length, pFileName);
    g_offset += f_list_header.length;
    return res;
}

#if 0
static char *substring(char *dst, char *src, int start, int len)
{
    char *p = dst;
    char *q = src;
    int length = strlen(src);
    if (start >= length || start < 0)
    {
        return NULL;
    }
    if (len > length)
    {
        len = length - start;
    }
    q += start;
    while (len--)
    {
        *(p++) = *(q++);
    }
    *(p++) = '\0';
    return dst;
}
#endif

void audio_fs_close_all_list(void)
{
    if (g_is_all_list_opened)
    {
        APP_PRINT_INFO0("audio_fs_close_all_list");
        g_is_all_list_opened = false;
        f_close(audio_fs_db.header_fil);
        f_close(audio_fs_db.name_fil);
        if (audio_fs_db.playlistSize == 0)
        {
            f_close(audio_fs_db.playlist.playlist_fil);
        }
    }
}

static uint16_t audio_fs_open_all_list(void)
{
    uint16_t res = AUDIO_FS_OK, rdRes = FR_OK;
    if (g_is_all_list_opened)
    {
        g_is_all_list_opened = false;
        audio_fs_close_all_list();
    }
    rdRes = f_open(audio_fs_db.header_fil, g_header_file, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if ((rdRes != FR_OK) && (rdRes != FR_NO_FILE))
    {
        res = AUDIO_FS_ERR_OPEN;
    }
    else if (rdRes == FR_NO_FILE)
    {
        rdRes = f_open(audio_fs_db.header_fil, g_header_file, FA_CREATE_NEW | FA_READ | FA_WRITE);
        if (rdRes != FR_OK)
        {
            res = AUDIO_FS_ERR_CREATE;
        }
    }
    if (res == AUDIO_FS_OK)
    {
        rdRes = f_open(audio_fs_db.name_fil, g_name_file, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
        if ((rdRes != FR_OK) && (rdRes != FR_NO_FILE))
        {
            f_close(audio_fs_db.header_fil);
            res = AUDIO_FS_ERR_OPEN;
        }
        else if (rdRes == FR_NO_FILE)
        {
            rdRes = f_open(audio_fs_db.name_fil, g_name_file, FA_CREATE_NEW | FA_READ | FA_WRITE);
            if (rdRes != FR_OK)
            {
                f_close(audio_fs_db.header_fil);
                res = AUDIO_FS_ERR_CREATE;
            }
        }
    }
    if ((res == AUDIO_FS_OK) && (audio_fs_db.playlistSize == 0))
    {
        rdRes = f_open(audio_fs_db.playlist.playlist_fil, g_playlist_file,
                       FA_OPEN_EXISTING | FA_READ | FA_WRITE);
        if ((rdRes != FR_OK) && (rdRes != FR_NO_FILE))
        {
            f_close(audio_fs_db.header_fil);
            f_close(audio_fs_db.name_fil);
            res = AUDIO_FS_ERR_OPEN;
        }
        else if (rdRes == FR_NO_FILE)
        {
            rdRes = f_open(audio_fs_db.playlist.playlist_fil, g_playlist_file,
                           FA_CREATE_NEW | FA_READ | FA_WRITE);
            if (rdRes != FR_OK)
            {
                f_close(audio_fs_db.header_fil);
                f_close(audio_fs_db.name_fil);
                res = AUDIO_FS_ERR_CREATE;
            }
        }
    }
    if (res == AUDIO_FS_OK)
    {
        g_is_all_list_opened = true;
        f_chmod((const TCHAR *)g_header_file, AM_HID, AM_HID | AM_MASK);
        f_chmod((const TCHAR *)g_name_file, AM_HID, AM_HID | AM_MASK);
        if (audio_fs_db.playlistSize == 0)
        {
            f_chmod((const TCHAR *)g_playlist_file, AM_HID, AM_HID | AM_MASK);
            f_sync(audio_fs_db.playlist.playlist_fil);
        }
        if (f_size(audio_fs_db.header_fil) == 0)
        {
            res = audio_fs_write_header_count(0);
        }
        f_sync(audio_fs_db.header_fil);
        f_sync(audio_fs_db.name_fil);
    }
    APP_PRINT_INFO2("audio_fs_open_all_list: res:0x%x, rdRes:%d", res, rdRes);
    return res;
}

static bool audio_fs_check_filelist(const TCHAR *path)
{
    bool isUpdate = false;
    uint16_t rdRes = FR_OK, res = AUDIO_FS_OK;
    uint16_t fileIdx = 0;
    uint16_t fileCount = 0;
    uint16_t headerCount = 0;
    DIR dir;
    FILINFO fileInfo;

    res = audio_fs_read_header_count(&headerCount);
    if (res == AUDIO_FS_OK)
    {
        memset(&dir, 0, sizeof(DIR));
        memset(&fileInfo, 0, sizeof(FILINFO));
        if ((rdRes = f_opendir(&dir, path)) != FR_OK)
        {
            res = AUDIO_FS_ERR_OPEN_DIR;
        }
        else
        {
            while (1)
            {
                rdRes = f_readdir(&dir, &fileInfo);
                if (rdRes != FR_OK)
                {
                    res = AUDIO_FS_ERR_READ_DIR;
                    break;
                }
                if (fileInfo.fname[0] == NULL)
                {
                    break;
                }
                if (fileInfo.fattrib & AM_ARC)
                {
                    uint16_t length = fs_get_filename_length(fileInfo.fname);
                    if (length > sizeof(_T(".mp3")))
                    {
                        fileCount++;
                    }
                }
            }
            if (fileCount != headerCount)
            {
                /* sd card is modified */
                isUpdate = true;
            }
            else if ((fileCount == headerCount) && (fileCount == 0))
            {
                /* sd card is empty and modify */
                isUpdate = true;
            }
            else
            {
                T_SONG_NAME_INFO songNameInfo;
                f_closedir(&dir);
                memset(&dir, 0, sizeof(DIR));
                if ((rdRes = f_opendir(&dir, path)) != FR_OK)
                {
                    res = AUDIO_FS_ERR_OPEN_DIR;
                }
                else
                {
                    while (1)
                    {
                        rdRes = f_readdir(&dir, &fileInfo);
                        if (rdRes != FR_OK)
                        {
                            res = AUDIO_FS_ERR_READ_DIR;
                            break;
                        }
                        if (fileInfo.fname[0] == NULL)
                        {
                            break;
                        }
                        if (fileInfo.fattrib & AM_ARC)
                        {
                            uint16_t length = fs_get_filename_length(fileInfo.fname);
                            if (length > sizeof(_T(".mp3")))
                            {
                                memset(&songNameInfo, 0, sizeof(T_SONG_NAME_INFO));
                                res = audio_fs_compare_filename(&fileIdx, &songNameInfo, 1, fileInfo.fname, length);
                                if (res == 0)
                                {
                                    fileIdx++;
                                }
                                else
                                {
                                    isUpdate = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            f_closedir(&dir);
        }
    }
    APP_PRINT_INFO5("audio_fs_check_filelist: res:0x%x, rdRes:%d, isUpdate:%d, headerCount:0x%x, fileCount:0x%x",
                    res, rdRes, isUpdate, headerCount, fileCount);
    return isUpdate;
}

static uint16_t audio_fs_scan_files(const TCHAR *path)
{
    uint16_t rdRes = 0, res = AUDIO_FS_OK;
    g_curHeaderCount = 0;
    g_offset = 0;
    DIR dir;
    FILINFO fileInfo;

    memset(&dir, 0, sizeof(DIR));
    memset(&fileInfo, 0, sizeof(FILINFO));
    if ((rdRes = f_opendir(&dir, path)) != FR_OK)
    {
        res = AUDIO_FS_ERR_OPEN_DIR;
    }
    else
    {
        while (1)
        {
            rdRes = f_readdir(&dir, &fileInfo);
            if (rdRes != FR_OK)
            {
                break;
            }
            if (fileInfo.fname[0] == NULL)
            {
                break;
            }
            if (fileInfo.fattrib & AM_ARC)
            {
                uint16_t length = fs_get_filename_length(fileInfo.fname);
                if (length > sizeof(_T(".mp3")))
                {
                    // APP_PRINT_INFO1("audio_fs_scan_files: filename(%b)", TRACE_BINARY(length, fileInfo.fname));
                    audio_fs_filename_write(fileInfo.fname, length);
                }
            }
        }
        res = audio_fs_write_header_count(g_curHeaderCount);
        f_sync(audio_fs_db.header_fil);
        f_sync(audio_fs_db.name_fil);
        f_closedir(&dir);
    }
    APP_PRINT_INFO3("audio_fs_scan_files: res:0x%x, rdRes:%d, headerCount:0x%x", res, rdRes,
                    g_curHeaderCount);
    return res;
}

static uint16_t audio_fs_scan_storage(const TCHAR *path, bool *needUpdatePlaylist)
{
    uint16_t rdRes = FR_OK, res = AUDIO_FS_OK;
    bool isUpdate = false;

    res = audio_fs_open_all_list();
    /* 3. header/name.playlist bin all opened success, scan storage */
    if (res == AUDIO_FS_OK)
    {
        isUpdate = audio_fs_check_filelist(path);

        if (isUpdate == false)
        {
            uint16_t headerCount = 0;
            uint16_t count = audio_fs_get_filecount_from_playlist();
            audio_fs_read_header_count(&headerCount);
            if (headerCount != count)
            {
                isUpdate = true;
            }
        }

        if (needUpdatePlaylist)
        {
            *needUpdatePlaylist = isUpdate;
        }

        if ((isUpdate == true) && (f_size(audio_fs_db.name_fil) != 0))
        {
            f_lseek(audio_fs_db.header_fil, 0);
            f_truncate(audio_fs_db.header_fil);
            f_lseek(audio_fs_db.name_fil, 0);
            f_truncate(audio_fs_db.name_fil);
            if (audio_fs_db.playlistSize == 0)
            {
                f_lseek(audio_fs_db.playlist.playlist_fil, 0);
                f_truncate(audio_fs_db.playlist.playlist_fil);
            }
        }
    }
    if ((res == AUDIO_FS_OK) && (isUpdate == true))
    {
        g_is_all_list_opened = true;
        rdRes = audio_fs_scan_files(path);
        if (rdRes != AUDIO_FS_OK)
        {
            res = AUDIO_FS_ERR_SCAN;
        }
    }
    APP_PRINT_INFO3("audio_fs_scan_storage: res:0x%x, g_initStatus: %d, isUpdate:%d",
                    res, g_initStatus, isUpdate);
    return res;
}

void audio_fs_sd_power_off(void)
{
    disk_ioctl(DEV_SD, CTRL_POWER_OFF, 0);
}

void audio_fs_sd_power_on(void)
{
    disk_ioctl(DEV_SD, CTRL_POWER_ON, 0);
}

uint16_t audio_fs_sd_status_check_and_init(void)
{
    uint16_t res = AUDIO_FS_OK;
    if (disk_ioctl(DEV_SD, MMC_GET_SDSTAT, 0) != RES_OK)
    {
        res = AUDIO_FS_ERR_CHECK_FS;
    }
    return res;
}

#if (FF_USE_MKFS == 1)
uint16_t audio_fs_mkfs(void)
{
    uint16_t res = 0;
    const MKFS_PARM mkfsPara = {FM_FAT32, 2, 0, 0, 0};
    BYTE work[FF_MAX_SS];

    audio_fs_deinit();

    /* format "0:" */
    res = f_mkfs(g_driver_path, &mkfsPara, work, sizeof(work));
    if (res != 0)
    {
        APP_PRINT_ERROR1("audio_fs_mkfs: mkfs fail, res:%d", res);
    }
    return res;
}
#endif

extern uint8_t g_fullFatScan;
uint16_t audio_fs_free_space(uint32_t *pfreeSpace)
{
    uint16_t res = 0;
    uint32_t freeSpace = 0;
    uint32_t freeClust = 0;
    g_fullFatScan = 1;
    f_getfree(g_driver_path, (DWORD *)&freeClust, (FATFS **)&audio_fs_db.fs);
    g_fullFatScan = 0;
    freeSpace = audio_fs_db.fs->free_clst * audio_fs_db.fs->csize * FF_MAX_SS;
    *pfreeSpace = freeSpace;
    APP_PRINT_INFO3("audio_fs_free_space: freeClust:0x%x, csize:0x%x, freeSpace:0x%x", freeClust,
                    audio_fs_db.fs->csize, freeSpace);
    return res;
}

uint16_t audio_fs_get_space_info(uint32_t *ptotalSpace, uint32_t *pfreeSpace)
{
    uint32_t freeClust = 0;
    g_fullFatScan = 1;
    uint16_t res = f_getfree(g_driver_path, (DWORD *)&freeClust, (FATFS **)&audio_fs_db.fs);
    g_fullFatScan = 0;
    if (res == FR_OK)
    {
        *ptotalSpace = (audio_fs_db.fs->n_fatent - 2) * audio_fs_db.fs->csize * FF_MAX_SS;
        *pfreeSpace = audio_fs_db.fs->free_clst * audio_fs_db.fs->csize * FF_MAX_SS;
    }
    return res;
}

static void audio_fs_queue_deinit(void)
{
    T_AUDIO_FILE *audio_file = NULL;
    for (int i = 0; i < FILE_NUM_OPENED_TOGETHER; i++)
    {
        audio_file = os_queue_peek(&audio_fs_list, i);
        if (audio_file != NULL)
        {
            os_mem_free(audio_file);
            audio_file = NULL;
        }
    }
}

static uint16_t audio_fs_queue_init(void)
{
    uint16_t res = AUDIO_FS_OK;

    os_queue_init(&audio_fs_list);

    for (int i = 0; i < FILE_NUM_OPENED_TOGETHER; i++)
    {
        T_AUDIO_FILE *audio_file = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(T_AUDIO_FILE));
        if (audio_file == NULL)
        {
            res = AUDIO_FS_ERR_MALLOC;
            audio_fs_queue_deinit();
            break;
        }
        else
        {
            memset(audio_file, 0, sizeof(T_AUDIO_FILE));
            os_queue_in(&audio_fs_list, audio_file);
        }
    }
    return res;
}

static void audio_fs_fatfs_deinit(void)
{
    /* unmount current workspace */
    f_mount(0, g_driver_path, 0);
    if (audio_fs_db.fs != NULL)
    {
        os_mem_free(audio_fs_db.fs);
        audio_fs_db.fs = NULL;
    }
    if (audio_fs_db.header_fil != NULL)
    {
        os_mem_free(audio_fs_db.header_fil);
        audio_fs_db.header_fil = NULL;
    }
    if (audio_fs_db.name_fil != NULL)
    {
        os_mem_free(audio_fs_db.name_fil);
        audio_fs_db.name_fil = NULL;
    }
    if ((audio_fs_db.playlistSize == 0) && (audio_fs_db.playlist.playlist_fil != NULL))
    {
        os_mem_free(audio_fs_db.playlist.playlist_fil);
        audio_fs_db.playlist.playlist_fil = NULL;
    }
    else if ((audio_fs_db.playlistSize != 0) && (audio_fs_db.playlist.playlistInfo != NULL))
    {
        os_mem_free(audio_fs_db.playlist.playlistInfo);
        audio_fs_db.playlist.playlistInfo = NULL;
    }
}

static uint16_t audio_fs_fatfs_init(void)
{
    uint16_t res = AUDIO_FS_OK, rdRes = FR_OK;
    audio_fs_db.fs = (FATFS *)os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(FATFS));
    audio_fs_db.header_fil = (FIL *)os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(FIL));
    audio_fs_db.name_fil = (FIL *)os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(FIL));
    if ((audio_fs_db.fs == NULL) || (audio_fs_db.header_fil == NULL) || (audio_fs_db.name_fil == NULL))
    {
        res = AUDIO_FS_ERR_MALLOC;
    }
    else
    {
        memset(audio_fs_db.fs, 0, sizeof(FATFS));
        memset(audio_fs_db.header_fil, 0, sizeof(FIL));
        memset(audio_fs_db.name_fil, 0, sizeof(FIL));
        if (audio_fs_db.playlistSize == 0)
        {
            audio_fs_db.playlist.playlist_fil = (FIL *)os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(FIL));
            if (audio_fs_db.playlist.playlist_fil == NULL)
            {
                res = AUDIO_FS_ERR_MALLOC;
            }
            else
            {
                memset(audio_fs_db.playlist.playlist_fil, 0, sizeof(FIL));
            }
        }
        else
        {
            audio_fs_db.playlist.playlistInfo = (uint16_t *)os_mem_alloc(OS_MEM_TYPE_DATA,
                                                                         audio_fs_db.playlistSize);
            if (audio_fs_db.playlist.playlistInfo == NULL)
            {
                res = AUDIO_FS_ERR_MALLOC;
            }
            else
            {
                memset(audio_fs_db.playlist.playlistInfo, 0, audio_fs_db.playlistSize);
            }
        }
    }
    if (res != AUDIO_FS_OK)
    {
        audio_fs_fatfs_deinit();
    }
    else
    {
        rdRes = f_mount(audio_fs_db.fs, g_driver_path, 1);
        if (rdRes != FR_OK)
        {
            if (rdRes == FR_NO_FILESYSTEM)
            {
                const TCHAR *driver_num = (const TCHAR *)_T("0:");
                const MKFS_PARM mkfsPara = {FM_EXFAT, 2, 0, 0, 0};
                bool wdg_en = false;
                bool aon_wdg_en = false;
                T_WATCH_DOG_TIMER_REG wdg_ctrl_value;

                f_mount(NULL, driver_num, 0);
                wdg_ctrl_value.d32 = WDG->WDG_CTL;
                wdg_en = wdg_ctrl_value.b.en_byte == 0xA5;
                aon_wdg_en = aon_wdg_is_enable(AON_WDG2);
                if (wdg_en)
                {
                    wdg_start(F_MKFS_MAX_TIME, (T_WDG_MODE)wdg_ctrl_value.b.wdg_mode);
                }
                if (aon_wdg_en)
                {
                    aon_wdg_disable(AON_WDG2);
                }
                rdRes = f_mkfs(driver_num, &mkfsPara, 0, FF_MAX_SS);
                if (wdg_en)
                {
                    WDG_Config(wdg_ctrl_value.b.div_factor, wdg_ctrl_value.b.cnt_limit,
                               (T_WDG_MODE)wdg_ctrl_value.b.wdg_mode);
                }
                if (aon_wdg_en)
                {
                    aon_wdg_kick(AON_WDG2);
                    aon_wdg_enable(AON_WDG2);
                }
                if (rdRes == FR_OK)
                {
                    rdRes = f_mount(audio_fs_db.fs, g_driver_path, 1);
                    if (rdRes != FR_OK)
                    {
                        APP_PRINT_ERROR1("audio_fs_fatfs_init: f_mount fail, rdRes:%d", rdRes);
                        res = AUDIO_FS_ERR_MOUNT;
                    }
                }
                else
                {
                    res = AUDIO_FS_NO_INIT;
                    APP_PRINT_ERROR1("audio_fs_fatfs_init: f_mkfs fail, rdRes:%d", rdRes);
                }
            }
        }
    }
    APP_PRINT_INFO2("audio_fs_fatfs_init: res:0x%x, rdRes:%d", res, rdRes);
    return res;
}

static uint16_t audio_fs_chdir(TCHAR *path)
{
    uint16_t res = AUDIO_FS_OK, rdRes = FR_OK;
    rdRes = f_chdir(path);
    if ((rdRes != FR_OK) && (rdRes != FR_NO_PATH))
    {
        res = AUDIO_FS_ERR_CHDIR;
    }
    else if (rdRes == FR_NO_PATH)
    {
        /* create directory based on path */
        if ((rdRes = f_mkdir(path)) == FR_OK)
        {
            rdRes = f_chdir(path);
        }
        if (rdRes != FR_OK)
        {
            res = AUDIO_FS_ERR_CHDIR;
        }
    }
    APP_PRINT_INFO2("audio_fs_chdir: res:0x%x, rdRes:%d", res, rdRes);
    return res;
}

void audio_fs_deinit(void)
{
    audio_fs_decode_deinit();
    audio_fs_fatfs_deinit();
    audio_fs_queue_deinit();
    g_initStatus = AUDIO_FS_NO_INIT;
}

uint16_t audio_fs_update(bool *needUpdatePlaylist)
{
    uint16_t res = 0;
    APP_PRINT_INFO4("audio_fs_update: g_initStatus:%d, relative_path(%b), playlistSize:0x%x, initiativeUpdate:0x%x",
                    g_initStatus, TRACE_BINARY(4, audio_fs_db.relative_path),
                    audio_fs_db.playlistSize, audio_fs_db.initiativeUpdate);
    if (g_initStatus == AUDIO_FS_NO_INIT)
    {
        res = audio_fs_init(audio_fs_db.relative_path, audio_fs_db.playlistSize,
                            audio_fs_db.initiativeUpdate, needUpdatePlaylist);
    }
    else if (g_initStatus == 0)
    {
        /* remount fatfs and set relative path */
        f_mount(0, g_driver_path, 0);
        if (audio_fs_db.fs != NULL)
        {
            memset(audio_fs_db.fs, 0, sizeof(FATFS));
            if ((res = f_mount(audio_fs_db.fs, g_driver_path, 1)) == FR_OK)
            {
                res = audio_fs_chdir(audio_fs_db.relative_path);
            }
        }
        if (res == FR_OK)
        {
            if (audio_fs_db.initiativeUpdate == true)
            {
                res = audio_fs_scan_storage(g_driver_path, needUpdatePlaylist);
                if (res == AUDIO_FS_OK)
                {
                    uint16_t playlistIndex = 1;
                    audio_fs_read_playlistIndex(&playlistIndex);
                    res = audio_fs_set_playlist(playlistIndex);
                }
                if (res != AUDIO_FS_OK)
                {
                    audio_fs_deinit();
                    g_initStatus = AUDIO_FS_NO_INIT;
                }
            }
            else
            {
                audio_fs_open_all_list();
            }
        }
    }
    APP_PRINT_INFO2("audio_fs_update: g_initStatus:%d, res:%d", g_initStatus, res);
    return res;
}

/**
 * @description:
 * @param:
*       path: the directory store music file
*       playlistSize:
            0:      store in SD card
            other:  the size store playlist.bin in RAM
        initiativeUpdate: if need to initiative update header/name bin.
            true:   need
            false:  not need
        needUpdatePlaylist: point of check result if playlist need to update
 * @return:
 *      AUDIO_FS_OK: init success
        other:       init fail
 */
uint16_t audio_fs_init(TCHAR *path, uint32_t playlistSize, bool initiativeUpdate,
                       bool *needUpdatePlaylist)
{
    uint16_t res = AUDIO_FS_OK;
    APP_PRINT_INFO4("audio_fs_init: g_initStatus:0x%x, path(%b), playlistSize:0x%x, initiativeUpdate:0x%x, ",
                    g_initStatus, TRACE_BINARY(4, path), playlistSize, initiativeUpdate);
    if (g_initStatus == AUDIO_FS_NO_INIT)
    {
        res = audio_fs_queue_init();
        if (res != AUDIO_FS_OK)
        {
            goto L_return;
        }
        /* malloc memory for FATFS and FIL and init fatfs */
        audio_fs_db.playlistSize = playlistSize;
        res = audio_fs_fatfs_init();
        if (res != AUDIO_FS_OK)
        {
            goto L_return;
        }
        /* switch to the relative path */
        audio_fs_db.relative_path = path;
        res = audio_fs_chdir(path);
        if (res != AUDIO_FS_OK)
        {
            goto L_return;
        }
        /* check if need to scan the relative path */
        audio_fs_db.initiativeUpdate = initiativeUpdate;
        if (initiativeUpdate == true)
        {
            res = audio_fs_scan_storage(g_driver_path, needUpdatePlaylist);
        }
        else
        {
            res = audio_fs_open_all_list();
            if (res != AUDIO_FS_OK)
            {
                goto L_return;
            }
        }
        if (res == AUDIO_FS_OK)
        {
            g_initStatus = AUDIO_FS_OK;

            uint16_t playlistIndex = 1;
            audio_fs_read_playlistIndex(&playlistIndex);
            res = audio_fs_set_playlist(playlistIndex);
        }
        // audio_fs_sd_power_off();
        power_stage_cb_register(audio_fs_sd_power_off, POWER_STAGE_STORE);
        APP_PRINT_INFO2("audio_fs_init: res:0x%x, g_initStatus:0x%x", res, g_initStatus);
        return res;
    }
L_return:
    audio_fs_queue_deinit();
    audio_fs_fatfs_deinit();
    // audio_fs_sd_power_off();
    power_stage_cb_register(audio_fs_sd_power_off, POWER_STAGE_STORE);
    APP_PRINT_ERROR2("audio_fs_init: res:0x%x, g_initStatus:0x%x", res, g_initStatus);
    return res;
}
