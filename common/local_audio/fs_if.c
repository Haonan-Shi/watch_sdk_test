/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

// Abbrevations:
// ofs = offset             ver = version           btr = bytes to read
// br = bytes read          btw = bytes to write    bw = bytes written
// hdr = header             idx = index             hdl = handle
// frm = frame              sqn = sequence          curr = current
// pos = position           au = audio


// Prefix:
// g = global variable      j = static variable     k = const
// p = pointer              a = array
// u = unsigned integer     s = signed integer
// f = float                d = double
// c = char                 b = bool
// S/z = struct             E/e = enum              U/n = union
// v = void                 x = unknown/unconcerned type
// h = handle

#include "fs_if.h"
#include "pm.h"
#include "trace.h"
#include "string.h"
#include "wchar.h"
#include "os_mem.h"
#include "diskio.h"
#include "wdg.h"
#if (CONFIG_SOC_SERIES_RTL87X3G == 1)
T_OS_MEM_TYPE    fs_ram_type = OS_MEM_TYPE_DATA;
#else
T_OS_MEM_TYPE    fs_ram_type = OS_MEM_TYPE_SHARING;
#endif
static const TCHAR    *header_fil_pth      = (const TCHAR *)_T("header.bin");
static const TCHAR    *name_fil_pth        = (const TCHAR *)_T("name.bin");

FRESULT seek_read(FIL *fp, FSIZE_t ofs, void *buff, UINT btr, UINT *br)
{
    FRESULT fr;

    fr = f_lseek(fp, ofs);
    if (fr != FR_OK)
    {
        return fr;
    }

    fr = f_read(fp, buff, btr, br);
    if (fr != FR_OK)
    {
        return fr;
    }

    return FR_OK;
}

FRESULT seek_write(FIL *fp, FSIZE_t ofs, void *buff, UINT btw, UINT *bw)
{
    FRESULT fr;

    fr = f_lseek(fp, ofs);
    if (fr != FR_OK)
    {
        return fr;
    }

    fr = f_write(fp, buff, btw, bw);
    if (fr != FR_OK)
    {
        return fr;
    }

    return FR_OK;
}

int fs_open_head_name_bin(T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    FRESULT fs_res = FR_OK;
    int res = 0;
    TCHAR *bin_path = NULL;
    uint16_t path_len = 0;
    uint16_t scan_path_len = 0;
    uint16_t head_path_len = 0;
    uint16_t name_path_len = 0;

    if (fil_scan_hdl->hdr_name_open == true)
    {
        return res;
    }

    if (fil_scan_hdl->header_fil == NULL || fil_scan_hdl->name_fil == NULL)
    {
        res = -1;
        goto Error;
    }

    scan_path_len = wcslen(fil_scan_hdl->file_path);
    head_path_len = wcslen(header_fil_pth);
    name_path_len = wcslen(name_fil_pth);
    path_len = scan_path_len + head_path_len + 2;

    bin_path = os_mem_zalloc(fs_ram_type, path_len * 2);
    if (bin_path == NULL)
    {
        res = -2;
        goto Error;
    }
    memcpy((uint8_t *)bin_path, fil_scan_hdl->file_path, scan_path_len * 2);
    if (bin_path[scan_path_len - 1] != _T(':') && bin_path[scan_path_len - 1] != _T('\\') && \
        bin_path[scan_path_len - 1] != _T('/'))
    {
        bin_path[scan_path_len] = _T('\\');
        scan_path_len++;
    }
    memcpy((uint8_t *)bin_path + scan_path_len * 2, header_fil_pth, head_path_len * 2);

    fs_res = f_open(fil_scan_hdl->header_fil, (const TCHAR *)bin_path,
                    FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    WDG_Kick();
    if ((fs_res != FR_OK) && (fs_res != FR_NO_FILE))
    {
        res = -3;
        goto Error1;
    }
    else if (fs_res == FR_NO_FILE)
    {
        fs_res = f_open(fil_scan_hdl->header_fil, (const TCHAR *)bin_path,
                        FA_CREATE_NEW | FA_READ | FA_WRITE);
        if (fs_res != FR_OK)
        {
            res = -4;
            goto Error1;
        }
    }
    WDG_Kick();
    f_chmod((const TCHAR *)bin_path, AM_HID, AM_HID | AM_MASK);

    WDG_Kick();
    memcpy((uint8_t *)bin_path + scan_path_len * 2, name_fil_pth, name_path_len * 2);
    bin_path[scan_path_len + name_path_len] = 0;
    fs_res = f_open(fil_scan_hdl->name_fil, (const TCHAR *)bin_path,
                    FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    WDG_Kick();
    if ((fs_res != FR_OK) && (fs_res != FR_NO_FILE))
    {
        f_close(fil_scan_hdl->header_fil);
        res = -5;
        goto Error1;
    }
    else if (fs_res == FR_NO_FILE)
    {
        fs_res = f_open(fil_scan_hdl->name_fil, (const TCHAR *)bin_path,
                        FA_CREATE_NEW | FA_READ | FA_WRITE);
        if (fs_res != FR_OK)
        {
            f_close(fil_scan_hdl->header_fil);
            res = -6;
            goto Error1;
        }
    }
    f_chmod((const TCHAR *)bin_path, AM_HID, AM_HID | AM_MASK);

    if (f_size(fil_scan_hdl->header_fil) == 0)
    {
        uint16_t count = 0;
        UINT bw;
        fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_COUNT_OFFSET, &count, FS_HEADER_COUNT_SIZE,
                            &bw);
        if (fs_res != FR_OK || bw != FS_HEADER_COUNT_SIZE)
        {
            res = -7;
            goto Error1;
        }
    }
    f_sync(fil_scan_hdl->header_fil);
    f_sync(fil_scan_hdl->name_fil);

    fil_scan_hdl->hdr_name_open = true;
    os_mem_free(bin_path);
    bin_path = NULL;
    return res;

Error1:
    os_mem_free(bin_path);
    bin_path = NULL;
Error:
    APP_PRINT_ERROR1("ERROR: fs_open_head_name_bin res = %d", res);
    return res;
}

int fs_close_head_name_bin(T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        return res;
    }

    if (fil_scan_hdl->header_fil == NULL || fil_scan_hdl->name_fil == NULL)
    {
        res = -1;
        goto Error;
    }

    if (FR_OK != f_close(fil_scan_hdl->header_fil))
    {
        res = -2;
        goto Error;
    }

    if (FR_OK != f_close(fil_scan_hdl->name_fil))
    {
        res = -3;
        goto Error;
    }

    fil_scan_hdl->hdr_name_open = false;

    return res;
Error:
    APP_PRINT_ERROR1("ERROR: fs_close_head_name_bin res = %d", res);
    return res;
}

int fs_read_header_bin_count(uint16_t *cnt, T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    UINT br;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    fs_res = seek_read(fil_scan_hdl->header_fil, FS_HEADER_COUNT_OFFSET, cnt, FS_HEADER_COUNT_SIZE,
                       &br);
    if (fs_res != FR_OK || br != FS_HEADER_COUNT_SIZE)
    {
        res = -2;
        goto Error;
    }
    return res;
Error:
    APP_PRINT_ERROR1("ERROR: fs_read_header_bin_count res = %d", res);
    return res;
}

static int fs_write_header_bin_count(uint16_t *cnt, T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    UINT bw;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_COUNT_OFFSET, cnt, FS_HEADER_COUNT_SIZE,
                        &bw);
    if (fs_res != FR_OK || bw != FS_HEADER_COUNT_SIZE)
    {
        res = -2;
        goto Error;
    }
    return res;
Error:
    APP_PRINT_ERROR1("ERROR: fs_write_header_bin_count res = %d", res);
    return res;
}

static uint8_t fs_check_file_ext(TCHAR *fname, const T_FILE_EXTENSION *fil_ext)
{
    uint8_t  ext_match = 0xff;

    for (uint8_t i = 0; i < fil_ext->file_ext_num; i++)
    {
        uint16_t f_len = wcslen(fname);
        uint16_t ext_len = wcslen(fil_ext->file_ext[i].small_letters);
        while (f_len && ext_len)
        {
            if (fil_ext->file_ext[i].small_letters[ext_len - 1] == _T('.'))
            {
                ext_match = fil_ext->file_ext[i].file_ext_def;
                break;
            }
            if (fil_ext->file_ext[i].small_letters[ext_len - 1] != fname[f_len - 1] && \
                fil_ext->file_ext[i].big_letters[ext_len - 1] != fname[f_len - 1])
            {
                break;
            }
            f_len--;
            ext_len--;
        }

        if (ext_match != 0xff)
        {
            break;
        }
    }
    return ext_match;
}

static int fs_read_header_name_info(uint16_t idx, T_HEAD_NAME *hdr_name,
                                    T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    UINT br;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    fs_res = seek_read(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + idx * sizeof(T_HEAD_INFO),
                       &(hdr_name->hdr_info), sizeof(T_HEAD_INFO), &br);
    // APP_PRINT_INFO1("test fs_read_header_info %b", TRACE_BINARY(sizeof(T_HEAD_INFO),
    //                                                             &(hdr_name->hdr_info)));
    if (fs_res != FR_OK || br != sizeof(T_HEAD_INFO))
    {
        res = -2;
        goto Error;
    }

    br = 0;
    fs_res = seek_read(fil_scan_hdl->name_fil, hdr_name->hdr_info.offset, hdr_name->name,
                       hdr_name->hdr_info.length, &br);
    // APP_PRINT_INFO1("test fs_read_name_info %b", TRACE_BINARY(hdr_name->hdr_info.length,
    //                                                           hdr_name->name));
    if (fs_res != FR_OK || br != hdr_name->hdr_info.length)
    {
        res = -3;
        goto Error;
    }
    return res;
Error:
    APP_PRINT_ERROR1("ERROR: fs_read_header_name_info res = %d", res);
    return res;
}

static int fs_write_header_name_info(uint16_t idx, T_HEAD_NAME *hdr_name,
                                     T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    UINT bw;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + idx * sizeof(T_HEAD_INFO),
                        &(hdr_name->hdr_info), sizeof(T_HEAD_INFO), &bw);
    if (fs_res != FR_OK || bw != sizeof(T_HEAD_INFO))
    {
        res = -2;
        goto Error;
    }

    bw = 0;
    fs_res = seek_write(fil_scan_hdl->name_fil, hdr_name->hdr_info.offset, hdr_name->name,
                        hdr_name->hdr_info.length, &bw);
    if (fs_res != FR_OK || bw != hdr_name->hdr_info.length)
    {
        res = -3;
        goto Error;
    }
    return res;
Error:
    APP_PRINT_ERROR1("ERROR: fs_write_header_name_info res = %d", res);
    return res;
}

int fs_file_add_to_header_name_bin(T_FILE_HANDLE *fil_hdl, T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    uint16_t count = 0;
    T_HEAD_NAME *hdr_name = NULL;

    if (fs_read_header_bin_count(&count, fil_scan_hdl))
    {
        res = -1;
        goto Error;
    }

    hdr_name = os_mem_alloc(fs_ram_type, sizeof(T_HEAD_NAME));
    if (hdr_name == NULL)
    {
        res = -2;
        goto Error;
    }
    memset(hdr_name, 0, sizeof(T_HEAD_NAME));
    if (count == 0)
    {
        hdr_name->hdr_info.offset = 0;
        hdr_name->hdr_info.length = fil_hdl->namelen;
        hdr_name->hdr_info.isDeleted = 0;
        hdr_name->hdr_info.needToUnlink = 1;
        hdr_name->hdr_info.fil_ext = fs_check_file_ext(fil_hdl->filename, fil_scan_hdl->scan_ext);
        memcpy(hdr_name->name, fil_hdl->filename, fil_hdl->namelen);
    }
    else
    {
        uint16_t i = 0;
        for (i = 0; i < count; i++)
        {
            if (fs_read_header_name_info(i, hdr_name, fil_scan_hdl))
            {
                res = -3;
                goto Error1;
            }
            if (wcscmp(fil_hdl->filename, hdr_name->name) == 0)
            {
                UINT bw = 0;
                hdr_name->hdr_info.isDeleted = 0;
                hdr_name->hdr_info.needToUnlink = 1;
                fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + i * sizeof(T_HEAD_INFO),
                                    &hdr_name->hdr_info, sizeof(T_HEAD_INFO), &bw);
                if (fs_res != FR_OK || bw != sizeof(T_HEAD_INFO))
                {
                    res = -4;
                    goto Error1;
                }
                f_sync(fil_scan_hdl->header_fil);

                os_mem_free(hdr_name);
                hdr_name = NULL;
                return res;
            }
        }

        hdr_name->hdr_info.offset = hdr_name->hdr_info.offset + hdr_name->hdr_info.length;
        hdr_name->hdr_info.length = fil_hdl->namelen;
        hdr_name->hdr_info.isDeleted = 0;
        hdr_name->hdr_info.needToUnlink = 1;
        hdr_name->hdr_info.fil_ext = fs_check_file_ext(fil_hdl->filename, fil_scan_hdl->scan_ext);
        memcpy(hdr_name->name, fil_hdl->filename, fil_hdl->namelen);
    }

    if (fs_write_header_name_info(count, hdr_name, fil_scan_hdl))
    {
        res = -5;
        goto Error1;
    }
    f_sync(fil_scan_hdl->header_fil);
    f_sync(fil_scan_hdl->name_fil);

    os_mem_free(hdr_name);
    hdr_name = NULL;
    return res;

Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
Error:
    APP_PRINT_ERROR1("ERROR: fs_add_header_name_info res = %d", res);
    return res;
}

int fs_file_update_header_bin(T_FILE_HANDLE *fil_hdl, T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    uint16_t count = 0;
    uint16_t i = 0;
    FRESULT fs_res = FR_OK;
    T_HEAD_NAME *hdr_name = NULL;

    if (fs_read_header_bin_count(&count, fil_scan_hdl))
    {
        res = -1;
        goto Error;
    }

    hdr_name = os_mem_alloc(fs_ram_type, sizeof(T_HEAD_NAME));
    if (hdr_name == NULL)
    {
        res = -2;
        goto Error;
    }

    for (i = 0; i < count; i++)
    {
        if (fs_read_header_name_info(i, hdr_name, fil_scan_hdl))
        {
            res = -3;
            goto Error1;
        }
        if (hdr_name->hdr_info.needToUnlink)
        {
            UINT bw = 0;
            hdr_name->hdr_info.needToUnlink = 0;
            fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + i * sizeof(T_HEAD_INFO),
                                &hdr_name->hdr_info, sizeof(T_HEAD_INFO), &bw);
            if (fs_res != FR_OK || bw != sizeof(T_HEAD_INFO))
            {
                res = -4;
                goto Error1;
            }
            f_sync(fil_scan_hdl->header_fil);
            os_mem_free(hdr_name);
            hdr_name = NULL;

            return res;
        }
    }
    count++;
    if (fs_write_header_bin_count(&count, fil_scan_hdl))
    {
        res = -5;
        goto Error1;
    }

    if (fs_read_header_name_info(count - 1, hdr_name, fil_scan_hdl))
    {
        res = -6;
        goto Error2;
    }

    hdr_name->hdr_info.needToUnlink = 0;
    if (fs_write_header_name_info(count - 1, hdr_name, fil_scan_hdl))
    {
        res = -7;
        goto Error2;
    }
    f_sync(fil_scan_hdl->header_fil);
    f_sync(fil_scan_hdl->name_fil);
    os_mem_free(hdr_name);
    hdr_name = NULL;
    return res;

Error2:
    f_sync(fil_scan_hdl->header_fil);
Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
Error:
    APP_PRINT_ERROR1("ERROR: fs_file_update_header_bin res = %d", res);
    return res;
}

int fs_mark_delete_file_to_head_bin(const TCHAR *file_name, T_FILE_SCAN_HANDLE *fil_scan_hdl,
                                    uint16_t file_idx)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    uint16_t count = 0;
    T_HEAD_NAME *hdr_name = NULL;

    if (fs_read_header_bin_count(&count, fil_scan_hdl))
    {
        res = -1;
        goto Error;
    }

    hdr_name = os_mem_alloc(fs_ram_type, sizeof(T_HEAD_NAME));
    if (hdr_name == NULL)
    {
        res = -2;
        goto Error;
    }
    memset(hdr_name, 0, sizeof(T_HEAD_NAME));

    res = fs_read_header_name_info(file_idx, hdr_name, fil_scan_hdl);
    if (res || wcscmp(file_name, hdr_name->name) != 0)
    {
        res = 0;
        uint16_t i = 0;
        for (i = 0; i < count; i++)
        {
            if (fs_read_header_name_info(i, hdr_name, fil_scan_hdl))
            {
                res = -3;
                goto Error1;
            }
            if (wcscmp(file_name, hdr_name->name) == 0)
            {
                file_idx = i;
                break;
            }
        }
        if (i >= count)
        {
            res = -4;
            goto Error1;
        }
    }

    if (hdr_name->hdr_info.isDeleted == 0)
    {
        UINT bw = 0;
        hdr_name->hdr_info.isDeleted = 1;
        fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + file_idx * sizeof(T_HEAD_INFO),
                            &hdr_name->hdr_info, sizeof(T_HEAD_INFO), &bw);
        if (fs_res != FR_OK || bw != sizeof(T_HEAD_INFO))
        {
            res = -5;
            goto Error1;
        }
        f_sync(fil_scan_hdl->header_fil);

        os_mem_free(hdr_name);
        hdr_name = NULL;
    }

    APP_PRINT_INFO4("fs_mark_delete_file_to_head_bin: res:0x%x, fs_res:%d, delete file_idx:%d, count:0x%x",
                    res, fs_res, file_idx, count);
    return res;
Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
Error:
    APP_PRINT_ERROR1("ERROR: fs_mark_delete_file_to_head_bin res = %d", res);
    return res;
}

int fs_set_playlist_info_to_head_bin(const TCHAR *file_name, T_FILE_SCAN_HANDLE *fil_scan_hdl,
                                     uint16_t playlist_idx, uint16_t file_idx)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    uint16_t count = 0;
    T_HEAD_NAME *hdr_name = NULL;
    UINT bw = 0;

    if (fs_read_header_bin_count(&count, fil_scan_hdl))
    {
        res = -1;
        goto Error;
    }

    hdr_name = os_mem_alloc(fs_ram_type, sizeof(T_HEAD_NAME));
    if (hdr_name == NULL)
    {
        res = -2;
        goto Error;
    }
    memset(hdr_name, 0, sizeof(T_HEAD_NAME));

    res = fs_read_header_name_info(file_idx, hdr_name, fil_scan_hdl);
    if (res || wcscmp(file_name, hdr_name->name) != 0)
    {
        res = 0;
        uint16_t i = 0;
        for (i = 0; i < count; i++)
        {
            if (fs_read_header_name_info(i, hdr_name, fil_scan_hdl))
            {
                res = -3;
                goto Error1;
            }
            if (wcscmp(file_name, hdr_name->name) == 0)
            {
                file_idx = i;
                break;
            }
        }
        if (i >= count)
        {
            res = -4;
            goto Error1;
        }
    }

    hdr_name->hdr_info.plIndex = playlist_idx;
    fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + file_idx * sizeof(T_HEAD_INFO),
                        &hdr_name->hdr_info, sizeof(T_HEAD_INFO), &bw);
    if (fs_res != FR_OK || bw != sizeof(T_HEAD_INFO))
    {
        res = -5;
        goto Error1;
    }
    f_sync(fil_scan_hdl->header_fil);

    os_mem_free(hdr_name);
    hdr_name = NULL;

    APP_PRINT_INFO4("fs_set_playlist_info_to_head_bin: res:0x%x, fs_res:%d, delete file_idx:%d, count:0x%x",
                    res, fs_res, file_idx, count);
    return res;
Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
Error:
    APP_PRINT_ERROR1("ERROR: fs_set_playlist_info_to_head_bin res = %d", res);
    return res;
}

static int fs_check_file_change_happen(T_FILE_SCAN_HANDLE *fil_scan_hdl, bool *file_changed)
{
    int res = 0;
    uint16_t fil_cnt = 0;
    uint16_t hdr_cnt = 0;
    DIR dir;
    *file_changed = false;
    FILINFO fil_info;
    T_HEAD_NAME *hdr_name = NULL;
    FRESULT fs_res = FR_OK;
    if (fs_read_header_bin_count(&hdr_cnt, fil_scan_hdl))
    {
        res = -1;
        goto Error;
    }

    fs_res = f_opendir(&dir, fil_scan_hdl->file_path);
    if (FR_OK != fs_res)
    {
        res = -2;
        goto Error;
    }

    while (1)
    {
        if (FR_OK != f_readdir(&dir, &fil_info) || fil_info.fname[0] == NULL)
        {
            break;
        }
        if (fil_info.fattrib & AM_HID)
        {
            continue;
        }
        if (fil_info.fattrib & AM_ARC)
        {
            if (fs_check_file_ext(fil_info.fname, fil_scan_hdl->scan_ext) != 0xff)
            {
                fil_cnt++;
                if (fil_cnt % 200 == 0)
                {
                    WDG_Kick();
                }
            }
        }
    }

    f_closedir(&dir);

    if (fil_cnt != hdr_cnt)
    {
        /* sd card is modified */
        *file_changed = true;
    }
    else if ((fil_cnt == hdr_cnt) && (fil_cnt == 0))
    {
        /* sd card is empty and modify */
        *file_changed = true;
    }
    else
    {
        uint16_t fil_idx = 0;
        if (FR_OK != f_opendir(&dir, fil_scan_hdl->file_path))
        {
            res = -3;
            goto Error;
        }

        hdr_name = os_mem_alloc(fs_ram_type, sizeof(T_HEAD_NAME));
        if (hdr_name == NULL)
        {
            f_closedir(&dir);
            res = -4;
            goto Error;
        }
        memset(hdr_name, 0x00, sizeof(T_HEAD_NAME));

        while (1)
        {
            if (FR_OK != f_readdir(&dir, &fil_info) || fil_info.fname[0] == NULL)
            {
                break;
            }
            if (fil_info.fattrib & AM_HID)
            {
                continue;
            }
            if (fil_info.fattrib & AM_ARC)
            {
                if (fs_check_file_ext(fil_info.fname, fil_scan_hdl->scan_ext) != 0xff)
                {
                    if (fs_read_header_name_info(fil_idx, hdr_name, fil_scan_hdl))
                    {
                        res = -5;
                        goto Error1;
                    }
                    if (wcscmp(fil_info.fname, hdr_name->name) != 0 || hdr_name->hdr_info.needToUnlink)
                    {
                        *file_changed = true;
                        break;
                    }
                    fil_idx++;
                    if (fil_idx % 200 == 0)
                    {
                        WDG_Kick();
                    }
                }
            }
        }
        os_mem_free(hdr_name);
        hdr_name = NULL;
        f_closedir(&dir);
    }

    return res;
Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
    f_closedir(&dir);
Error:
    APP_PRINT_ERROR1("ERROR: fs_check_file_change_happen res = %d", res);
    return res;
}

static int fs_update_head_name_bin(T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    DIR dir;
    FILINFO fil_info;
    uint32_t name_ofs = 0;
    uint16_t fil_idx = 0;
    FRESULT fs_res = FR_OK;
    UINT bw;
    T_HEAD_NAME *hdr_name = NULL;
    TCHAR *bin_path = NULL;
    uint16_t hdr_cnt = 0;
    uint16_t path_len = 0;
    uint16_t scan_path_len = 0;
    uint16_t delete_file_path_len = 0;
    uint16_t i = 0;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    hdr_name = os_mem_alloc(fs_ram_type, sizeof(T_HEAD_NAME));
    if (hdr_name == NULL)
    {
        res = -2;
        goto Error;
    }
    if (fs_read_header_bin_count(&hdr_cnt, fil_scan_hdl))
    {
        res = -2;
        goto Error1;
    }
    for (i = 0; i <= hdr_cnt; i++)
    {
        if (fs_read_header_name_info(i, hdr_name, fil_scan_hdl))
        {
            APP_PRINT_ERROR0("maybe there are no files to delete");
        }
        else
        {
            if (hdr_name->hdr_info.needToUnlink)
            {
                scan_path_len = wcslen(fil_scan_hdl->file_path);
                delete_file_path_len = hdr_name->hdr_info.length;
                path_len = scan_path_len + delete_file_path_len + 2;
                bin_path = os_mem_zalloc(fs_ram_type, path_len * 2);
                if (bin_path == NULL)
                {
                    res = -3;
                    goto Error1;
                }
                memcpy((uint8_t *)bin_path, fil_scan_hdl->file_path, scan_path_len * 2);
                if (bin_path[scan_path_len - 1] != _T(':') && bin_path[scan_path_len - 1] != _T('\\') && \
                    bin_path[scan_path_len - 1] != _T('/'))
                {
                    bin_path[scan_path_len] = _T('\\');
                    scan_path_len++;
                }
                memcpy((uint8_t *)bin_path + scan_path_len * 2, hdr_name->name, delete_file_path_len);
                APP_PRINT_INFO1("f_unlink file %b", TRACE_BINARY(50, bin_path));
                WDG_Kick();
                if (f_unlink((const TCHAR *)bin_path))
                {
                    res = -4;
                    goto Error2;
                }
            }
        }
    }

    if (f_size(fil_scan_hdl->name_fil) != 0)
    {
        f_lseek(fil_scan_hdl->header_fil, 0);
        f_truncate(fil_scan_hdl->header_fil);
        f_lseek(fil_scan_hdl->name_fil, 0);
        f_truncate(fil_scan_hdl->name_fil);
    }

    if (FR_OK != f_opendir(&dir, fil_scan_hdl->file_path))
    {
        res = -5;
        goto Error2;
    }

    while (1)
    {
        if (FR_OK != f_readdir(&dir, &fil_info) || fil_info.fname[0] == NULL)
        {
            break;
        }
        if (fil_info.fattrib & AM_HID)
        {
            continue;
        }
        if (fil_info.fattrib & AM_ARC)
        {
            uint8_t ext_type = fs_check_file_ext(fil_info.fname, fil_scan_hdl->scan_ext);
            if (ext_type != 0xff)
            {
                T_HEAD_INFO hdr_info;
                memset(&hdr_info, 0, sizeof(T_HEAD_INFO));
                hdr_info.length = (wcslen(fil_info.fname) + 1) * 2;
                hdr_info.offset = name_ofs;
                hdr_info.plIndex = 0x1;
                hdr_info.fil_ext = ext_type;
                hdr_info.rsv = 0x0;

                bw = 0;
                fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + fil_idx * sizeof(T_HEAD_INFO),
                                    &hdr_info, sizeof(T_HEAD_INFO), &bw);
                if (fs_res != FR_OK || bw != sizeof(T_HEAD_INFO))
                {
                    res = -6;
                    goto Error3;
                }
                bw = 0;
                fs_res = seek_write(fil_scan_hdl->name_fil, name_ofs, fil_info.fname, hdr_info.length, &bw);
                if (fs_res != FR_OK || bw != hdr_info.length)
                {
                    res = -7;
                    goto Error4;
                }
                name_ofs += hdr_info.length;
                fil_idx++;
                if (fil_idx % 200 == 0)
                {
                    WDG_Kick();
                }
            }
        }
    }

    if (fs_write_header_bin_count(&fil_idx, fil_scan_hdl))
    {
        res = -8;
        goto Error5;
    }
    f_sync(fil_scan_hdl->header_fil);
    f_sync(fil_scan_hdl->name_fil);

    os_mem_free(bin_path);
    bin_path = NULL;
    os_mem_free(hdr_name);
    hdr_name = NULL;
    return res;

Error5:
    f_sync(fil_scan_hdl->name_fil);
Error4:
    f_sync(fil_scan_hdl->header_fil);
Error3:
    f_closedir(&dir);
Error2:
    os_mem_free(bin_path);
    bin_path = NULL;
Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
Error:
    APP_PRINT_ERROR1("ERROR: fs_update_head_name_bin res = %d", res);
    return res;
}

int fs_scan_file_list(T_FILE_SCAN_HANDLE *fil_scan_hdl, bool *file_list_change)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    DIR dir;
    TCHAR *temp_path = _T("\\");

    if (fil_scan_hdl == NULL || fil_scan_hdl->header_fil == NULL || fil_scan_hdl->name_fil == NULL)
    {
        res = -1;
        goto Error;
    }

    if (FR_OK != f_chdir(temp_path))
    {
        res = -2;
        goto Error;
    }

    fs_res = f_opendir(&dir, fil_scan_hdl->file_path);
    if (fs_res == FR_NO_PATH)
    {
        fs_res = f_mkdir(fil_scan_hdl->file_path);
        if (fs_res != FR_OK)
        {
            res = -3;
            goto Error;
        }
    }
    else if (fs_res != FR_OK)
    {
        res = -4;
        goto Error;
    }
    else
    {
        f_closedir(&dir);
    }

    if (fs_open_head_name_bin(fil_scan_hdl))
    {
        res = -4;
        goto Error;
    }

    if (fs_check_file_change_happen(fil_scan_hdl, file_list_change))
    {
        res = -5;
        goto Error;
    }

    if (*file_list_change == true)
    {
        if (fs_update_head_name_bin(fil_scan_hdl))
        {
            res = -6;
            goto Error;
        }
    }

    f_chdir(fil_scan_hdl->file_path);

    return res;

Error:
    APP_PRINT_ERROR1("ERROR: fs_scan_file_list res = %d", res);
    return res;
}

int fs_create_scan_handle(const TCHAR *fil_path, const T_FILE_EXTENSION *scan_ext,
                          T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;

    if (fil_path == NULL || scan_ext == NULL || fil_scan_hdl == NULL)
    {
        res = -1;
        goto Error;
    }

    fil_scan_hdl->file_path = fil_path;
    fil_scan_hdl->hdr_name_open = false;
    fil_scan_hdl->scan_ext = scan_ext;

    if (fil_scan_hdl->header_fil == NULL)
    {
        fil_scan_hdl->header_fil = (FIL *)os_mem_alloc(fs_ram_type, sizeof(FIL));
        if (fil_scan_hdl->header_fil == NULL)
        {
            res = -2;
            goto header_malloc_fail;
        }
    }

    if (fil_scan_hdl->name_fil == NULL)
    {
        fil_scan_hdl->name_fil = (FIL *)os_mem_alloc(fs_ram_type, sizeof(FIL));
        if (fil_scan_hdl->name_fil == NULL)
        {
            res = -3;
            goto name_malloc_fail;
        }
    }

    return res;

name_malloc_fail:
    os_mem_free(fil_scan_hdl->header_fil);
    fil_scan_hdl->header_fil = NULL;
header_malloc_fail:
Error:
    APP_PRINT_ERROR1("ERROR: fs_create_scan_handle res = %d", res);
    return res;
}

int fs_delete_scan_handle(T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;

    if (fil_scan_hdl == NULL)
    {
        res = -1;
        return res;
    }

    if (fil_scan_hdl->header_fil != NULL)
    {
        os_mem_free(fil_scan_hdl->header_fil);
        fil_scan_hdl->header_fil = NULL;
    }

    if (fil_scan_hdl->name_fil != NULL)
    {
        os_mem_free(fil_scan_hdl->name_fil);
        fil_scan_hdl->name_fil = NULL;
    }

    return res;
}

int fs_unlink_all_files(T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    FRESULT fs_res = FR_OK;
    uint16_t count = 0;
    FILINFO fil_info;
    DIR dir;
    TCHAR *temp_path = _T("\\");

    res = fs_write_header_bin_count(&count, fil_scan_hdl);
    f_lseek(fil_scan_hdl->header_fil, FS_HEADER_INFO_START);
    f_truncate(fil_scan_hdl->header_fil);
    f_lseek(fil_scan_hdl->name_fil, 0);
    f_truncate(fil_scan_hdl->name_fil);
    f_sync(fil_scan_hdl->header_fil);
    f_sync(fil_scan_hdl->name_fil);

    if (FR_OK != f_opendir(&dir, temp_path))
    {
        res = -1;
        goto Error;
    }
    else
    {
        while (1)
        {
            if (FR_OK != f_readdir(&dir, &fil_info) || fil_info.fname[0] == NULL)
            {
                break;
            }
            else if (fil_info.fattrib & (AM_HID | AM_SYS))
            {
                //hidden or system file, no parse
                continue;
            }
            if (fil_info.fattrib & AM_ARC)
            {
                WDG_Kick();
                fs_res = f_unlink(fil_info.fname);
                if ((fs_res == 0) || (fs_res == FR_NO_FILE))
                {
                    count++;
                }
                else
                {
                    APP_PRINT_ERROR3("fs_unlink_all_files: fs_res:0x%x, file size:0x%x, , filename:%b",
                                     fs_res,
                                     fil_info.fsize,
                                     TRACE_BINARY(sizeof(fil_info.fname), fil_info.fname));
                    res = -2;
                    goto Error1;
                }
            }
        }
        f_closedir(&dir);
    }
    APP_PRINT_INFO4("fs_unlink_all_files, res:0x%x, fs_res:%d, count:%d, delete_path(%b)",
                    res, fs_res, count, TRACE_BINARY(wcslen(fil_scan_hdl->file_path), fil_scan_hdl->file_path));
    return res;
Error1:
    f_closedir(&dir);
Error:
    APP_PRINT_ERROR1("ERROR: fs_unlink_all_files res = %d", res);
    return res;
}

int fs_delete_all_files_by_format(T_FILE_FORMAT format, T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    uint16_t count = 0;
    T_HEAD_NAME *hdr_name = NULL;
    hdr_name = os_mem_alloc(fs_ram_type, sizeof(T_HEAD_NAME));
    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }
    if (fs_read_header_bin_count(&count, fil_scan_hdl))
    {
        res = -2;
        goto Error;
    }
    for (uint16_t i = 0; i < count; i++)
    {
        if (fs_read_header_name_info(i, hdr_name, fil_scan_hdl))
        {
            res = -3;
            goto Error;
        }
        if (hdr_name->hdr_info.fil_ext == format && hdr_name->hdr_info.isDeleted == 0)
        {
            WDG_Kick();
            if (f_unlink(hdr_name->name))
            {
                res = -4;
                goto Error;
            }
            hdr_name->hdr_info.isDeleted = 1;
            if (fs_write_header_name_info(i, hdr_name, fil_scan_hdl))
            {
                res = -5;
                goto Error;
            }
            f_sync(fil_scan_hdl->header_fil);
        }
    }
    f_sync(fil_scan_hdl->header_fil);
    os_mem_free(hdr_name);
    hdr_name = NULL;
    return res;
Error:
    os_mem_free(hdr_name);
    hdr_name = NULL;
    APP_PRINT_ERROR1("ERROR: fs_delete_all_files_by_format res = %d", res);
    return res;
}

int fs_init(const TCHAR *disk_path, T_FS_HANDLE *fs_hdl)
{
    int res = 0;

    if (disk_path == NULL || fs_hdl == NULL)
    {
        res = -1;
        goto Error1;
    }

    if (fs_hdl->fs == NULL)
    {
        fs_hdl->fs = (FATFS *)os_mem_alloc(fs_ram_type, sizeof(FATFS));
        if (fs_hdl->fs == NULL)
        {
            res = -2;
            goto Error1;
        }
    }

    fs_hdl->disk_path = disk_path;

    res = f_mount(fs_hdl->fs, disk_path, 1);

    if (FR_OK != res)
    {
        if (res == FR_NO_FILESYSTEM)
        {
            res = f_mkfs(disk_path, 0, 0, 4096);
            if (res != FR_OK)
            {
                res = -3;
                goto mount_fail;
            }
            else
            {
                f_mount(NULL, disk_path, 0);
                res = f_mount(fs_hdl->fs, disk_path, 1);
                if (res != FR_OK)
                {
                    res = -3;
                    goto mount_fail;
                }
            }
        }
        else
        {
            res = -3;
            goto mount_fail;
        }
    }

    if (FR_OK != f_chdrive(disk_path))
    {
        res = -4;
        goto mount_fail;
    }

    power_stage_cb_register(sd_power_off, POWER_STAGE_STORE);
    return res;

mount_fail:
    os_mem_free(fs_hdl->fs);
    fs_hdl->fs = NULL;
Error1:
    APP_PRINT_ERROR1("ERROR: fs_init res = %d", res);
    power_stage_cb_register(sd_power_off, POWER_STAGE_STORE);
    return res;
}

int fs_deinit(T_FS_HANDLE *fs_hdl)
{

    if (FR_OK != f_mount(0, fs_hdl->disk_path, 0))
    {
        return -1;
    }
    if (fs_hdl->fs)
    {
        os_mem_free(fs_hdl->fs);
        fs_hdl->fs = NULL;
    }
    return 0;
}

#if (CONFIG_SOC_SERIES_RTL8773E == 1)

T_OS_MEM_TYPE    sd_ram_type = OS_MEM_TYPE_SHARING;

static void sd_set_mem_alloc_type(T_OS_MEM_TYPE ram_type)
{
    if (ram_type < OS_MEM_TYPE_NUM)
    {
        sd_ram_type = ram_type;
    }
    else
    {
        APP_PRINT_ERROR1("ERROR: sd_set_mem_alloc_type type = %d", ram_type);
    }
}

void fs_set_mem_alloc_type(T_OS_MEM_TYPE ram_type)
{
    if (ram_type < OS_MEM_TYPE_NUM)
    {
        fs_ram_type = ram_type;
        sd_set_mem_alloc_type(ram_type);
    }
    else
    {
        APP_PRINT_ERROR1("ERROR: fs_set_mem_alloc_type type = %d", ram_type);
    }
}
#endif

void sd_power_off(void)
{
    disk_ioctl(DEV_SD, CTRL_POWER_OFF, 0);
}

void sd_power_on(void)
{
    disk_ioctl(DEV_SD, CTRL_POWER_ON, 0);
}
