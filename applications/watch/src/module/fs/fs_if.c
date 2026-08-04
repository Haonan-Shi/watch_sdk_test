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

#include <stdio.h>
#include <zephyr/storage/disk_access.h>
#include "fs_if.h"
#include "pm.h"
#include "trace.h"
#include "string.h"
#include "os_mem.h"
#include "diskio.h"
#include "wdg.h"
#include "app_fs_if.h"

T_OS_MEM_TYPE    fs_ram_type = OS_MEM_TYPE_DATA;
static const char header_fil_pth[] = {"header.bin"};
static const char name_fil_pth[] = {"name.bin"};

int seek_read(struct fs_file_t *fp, off_t ofs, void *buff, size_t btr)
{
    int fr = 0;
    size_t size = 0;

    //zephyr fat_fs.c does not allow ofs > file size when called fs_seek
    fr = fs_size(fp, &size);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write get size fail res %d, offset 0x%x, btr 0x%x, fp 0x%x",
                         fr, ofs, btr, fp);
        return fr;
    }

    if (ofs > size)
    {
        fr = fs_truncate(fp, ofs);
    }

    fr = fs_seek(fp, ofs, FS_SEEK_SET);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_read seek fail res %d, offset 0x%x, btr 0x%x, fp 0x%x",
                         fr, ofs, btr, fp);
        return fr;
    }

    fr = fs_read(fp, buff, btr);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_read read fail res %d, offset 0x%x, btr 0x%x, fp 0x%x",
                         fr, ofs, btr, fp);
        return fr;
    }

    return fr;
}

int seek_write(struct fs_file_t *fp, off_t ofs, void *buff, size_t btw)
{
    int fr = 0;
    size_t size = 0;

    //zephyr fat_fs.c does not allow ofs > file size when called fs_seek
    fr = fs_size(fp, &size);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write get size fail res %d, offset 0x%x, btw 0x%x, fp 0x%x",
                         fr, ofs, btw, fp);
        return fr;
    }

    if (ofs > size)
    {
        fr = fs_truncate(fp, ofs);
    }
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write truncate fail res %d, offset 0x%x, btw 0x%x, fp 0x%x",
                         fr, ofs, btw, fp);
    }

    fr = fs_seek(fp, ofs, FS_SEEK_SET);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write seek fail res %d, offset 0x%x, btw 0x%x, fp 0x%x",
                         fr, ofs, btw, fp);
        return fr;
    }

    fr = fs_write(fp, buff, btw);
    if (fr < 0)
    {
        APP_PRINT_ERROR4("seek_write write fail res %d, offset 0x%x, btw 0x%x, fp 0x%x",
                         fr, ofs, btw, fp);
        return fr;
    }

    return fr;
}

size_t uint16_strlen(const uint16_t *str)
{
    const uint16_t *s = str;
    while (*s != 0)
    {
        ++s;
    }
    return s - str;
}

int uint16_strcmp(const uint16_t *str1, const uint16_t *str2)
{
    while (*str1 != 0 && *str2 != 0)
    {
        if (*str1 != *str2)
        {
            return *str1 - *str2;
        }
        str1++;
        str2++;
    }
    return *str1 - *str2;
}
int fs_open_head_name_bin(T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int fs_res = 0;
    int res = 0;
    char *bin_path = NULL;
    uint16_t path_len = 0;
    uint16_t scan_path_len = 0;
    uint16_t head_path_len = 0;
    uint16_t name_path_len = 0;
    size_t file_size = 0;

    if (fil_scan_hdl->hdr_name_open == true)
    {
        return res;
    }

    if (fil_scan_hdl->header_fil == NULL || fil_scan_hdl->name_fil == NULL)
    {
        res = -1;
        goto Error;
    }

    scan_path_len = strlen(fil_scan_hdl->file_path);
    head_path_len = strlen(header_fil_pth);
    name_path_len = strlen(name_fil_pth);

    path_len = scan_path_len +
               (head_path_len > name_path_len ? head_path_len : name_path_len)
               + 2;

    bin_path = os_mem_zalloc(fs_ram_type, path_len);
    if (bin_path == NULL)
    {
        res = -2;
        goto Error;
    }

    snprintf(bin_path, path_len, "%s%s", fil_scan_hdl->file_path, header_fil_pth);
    APP_PRINT_TRACE1("fs_open_head_name_bin header bin path %s", TRACE_STRING(bin_path));

    fs_file_t_init(fil_scan_hdl->header_fil);
    fs_res = fs_open(fil_scan_hdl->header_fil, bin_path,
                     FS_O_READ | FS_O_WRITE);

    WDG_Kick();
    if ((fs_res != 0) && (fs_res != -ENFILE))
    {
        res = -3;
        goto Error1;
    }
    else if (fs_res == -ENFILE)
    {
        fs_res = fs_open(fil_scan_hdl->header_fil, bin_path,
                         FS_O_CREATE | FS_O_READ | FS_O_WRITE);
        if (fs_res != 0)
        {
            res = -4;
            goto Error1;
        }
    }
    WDG_Kick();
    fs_chmod(bin_path, FS_AM_HID, FS_AM_HID);

    WDG_Kick();

    memset(bin_path, 0, path_len);
    snprintf(bin_path, path_len, "%s%s", fil_scan_hdl->file_path, name_fil_pth);

    APP_PRINT_TRACE1("fs_open_head_name_bin name bin path %s", TRACE_STRING(bin_path));

    fs_file_t_init(fil_scan_hdl->name_fil);
    fs_res = fs_open(fil_scan_hdl->name_fil, bin_path,
                     FS_O_READ | FS_O_WRITE);
    WDG_Kick();
    if ((fs_res != 0) && (fs_res != -ENFILE))
    {
        fs_close(fil_scan_hdl->header_fil);
        res = -5;
        goto Error1;
    }
    else if (fs_res == -ENFILE)
    {
        fs_res = fs_open(fil_scan_hdl->name_fil, bin_path,
                         FS_O_CREATE | FS_O_READ | FS_O_WRITE);
        if (fs_res != 0)
        {
            fs_close(fil_scan_hdl->header_fil);
            res = -6;
            goto Error1;
        }
    }
    fs_chmod(bin_path, FS_AM_HID, FS_AM_HID);
    fs_res = fs_size(fil_scan_hdl->header_fil, &file_size);
    if ((fs_res == 0) && (file_size == 0))
    {
        uint16_t count = 0;
        fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_COUNT_OFFSET, &count, FS_HEADER_COUNT_SIZE);
        if (fs_res < 0 || fs_res != FS_HEADER_COUNT_SIZE)
        {
            res = -7;
            goto Error1;
        }
    }
    else if (fs_res != 0)
    {
        res = -8;
        goto Error1;
    }
    fs_sync(fil_scan_hdl->header_fil);
    fs_sync(fil_scan_hdl->name_fil);

    fil_scan_hdl->hdr_name_open = true;

    os_mem_free(bin_path);
    bin_path = NULL;
    return res;

Error1:
    os_mem_free(bin_path);
    bin_path = NULL;
Error:
    APP_PRINT_ERROR2("ERROR: fs_open_head_name_bin res = %d, fs_res %d", res, fs_res);
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

    if (0 != fs_close(fil_scan_hdl->header_fil))
    {
        res = -2;
        goto Error;
    }

    if (0 != fs_close(fil_scan_hdl->name_fil))
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
    int fs_res = 0;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    fs_res = seek_read(fil_scan_hdl->header_fil, FS_HEADER_COUNT_OFFSET, cnt, FS_HEADER_COUNT_SIZE);

    if (fs_res < 0)
    {
        res = -2;
        goto Error;
    }
    return res;
Error:
    APP_PRINT_ERROR2("ERROR: fs_read_header_bin_count res = %d, fs_res %d", res, fs_res);
    return res;
}

static int fs_write_header_bin_count(uint16_t *cnt, T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    int fs_res = 0;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }
    APP_PRINT_TRACE1("fs_write_header_bin_count cnt to write %d", *cnt);

    fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_COUNT_OFFSET, cnt, FS_HEADER_COUNT_SIZE);

    if (fs_res < 0)
    {
        res = -2;
        goto Error;
    }
    return res;
Error:
    APP_PRINT_ERROR2("ERROR: fs_write_header_bin_count res = %d, fs_res %d", res, fs_res);
    return res;
}

static uint8_t fs_check_file_ext(const char *fname, const T_FILE_EXTENSION *fil_ext)
{
    uint8_t  ext_match = 0xff;

    for (uint8_t i = 0; i < fil_ext->file_ext_num; i++)
    {
        uint16_t f_len = strlen(fname);
        uint16_t ext_len = strlen(fil_ext->file_ext[i].small_letters);
        while (f_len && ext_len)
        {
            if (fil_ext->file_ext[i].small_letters[ext_len - 1] == '.')
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
    int fs_res = 0;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    fs_res = seek_read(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + idx * sizeof(T_HEAD_INFO),
                       &(hdr_name->hdr_info), sizeof(T_HEAD_INFO));
    // APP_PRINT_INFO1("test fs_read_header_info %b", TRACE_BINARY(sizeof(T_HEAD_INFO),
    //                                                             &(hdr_name->hdr_info)));
    if (fs_res < 0)
    {
        res = -2;
        goto Error;
    }

    fs_res = seek_read(fil_scan_hdl->name_fil, hdr_name->hdr_info.offset, hdr_name->name,
                       hdr_name->hdr_info.length);
    // APP_PRINT_INFO1("test fs_read_name_info %b", TRACE_BINARY(hdr_name->hdr_info.length,
    //                                                           hdr_name->name));
    if (fs_res < 0)
    {
        res = -3;
        goto Error;
    }
    return res;
Error:
    APP_PRINT_ERROR2("ERROR: fs_read_header_name_info res = %d, fs_res %d", res, fs_res);
    return res;
}

static int fs_write_header_name_info(uint16_t idx, T_HEAD_NAME *hdr_name,
                                     T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    int fs_res = 0;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + idx * sizeof(T_HEAD_INFO),
                        &(hdr_name->hdr_info), sizeof(T_HEAD_INFO));
    if (fs_res < 0)
    {
        res = -2;
        goto Error;
    }

    fs_res = seek_write(fil_scan_hdl->name_fil, hdr_name->hdr_info.offset, hdr_name->name,
                        hdr_name->hdr_info.length);
    if (fs_res < 0)
    {
        res = -3;
        goto Error;
    }
    return res;
Error:
    APP_PRINT_ERROR2("ERROR: fs_write_header_name_info res = %d, fs_res %d", res, fs_res);
    return res;
}

int fs_file_add_to_header_name_bin(T_FILE_HANDLE *fil_hdl, T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    int fs_res = 0;
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
            if (strcmp(fil_hdl->filename, hdr_name->name) == 0)
            {
                hdr_name->hdr_info.isDeleted = 0;
                hdr_name->hdr_info.needToUnlink = 1;
                fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + i * sizeof(T_HEAD_INFO),
                                    &hdr_name->hdr_info, sizeof(T_HEAD_INFO));
                if (fs_res < 0 || fs_res != sizeof(T_HEAD_INFO))
                {
                    res = -4;
                    goto Error1;
                }
                fs_sync(fil_scan_hdl->header_fil);

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
    fs_sync(fil_scan_hdl->header_fil);
    fs_sync(fil_scan_hdl->name_fil);

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
    int fs_res = 0;
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

    APP_PRINT_TRACE1("fs_file_update_header_bin count read from header bin: %d", count);
    for (i = 0; i < count; i++)
    {
        if (fs_read_header_name_info(i, hdr_name, fil_scan_hdl))
        {
            res = -3;
            goto Error1;
        }
        if (hdr_name->hdr_info.needToUnlink)
        {
            hdr_name->hdr_info.needToUnlink = 0;
            fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + i * sizeof(T_HEAD_INFO),
                                &hdr_name->hdr_info, sizeof(T_HEAD_INFO));
            if (fs_res < 0 || fs_res != sizeof(T_HEAD_INFO))
            {
                res = -4;
                goto Error1;
            }
            fs_sync(fil_scan_hdl->header_fil);
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
    fs_sync(fil_scan_hdl->header_fil);
    fs_sync(fil_scan_hdl->name_fil);
    os_mem_free(hdr_name);
    hdr_name = NULL;
    return res;

Error2:
    fs_sync(fil_scan_hdl->header_fil);
Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
Error:
    APP_PRINT_ERROR1("ERROR: fs_file_update_header_bin res = %d", res);
    return res;
}

int fs_mark_delete_file_to_head_bin(const char *file_name, T_FILE_SCAN_HANDLE *fil_scan_hdl,
                                    uint16_t file_idx)
{
    int res = 0;
    int fs_res = 0;
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
    if (res || strcmp(file_name, hdr_name->name) != 0)
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
            if (strcmp(file_name, hdr_name->name) == 0)
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
        hdr_name->hdr_info.isDeleted = 1;
        fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + file_idx * sizeof(T_HEAD_INFO),
                            &hdr_name->hdr_info, sizeof(T_HEAD_INFO));
        if (fs_res < 0 || fs_res != sizeof(T_HEAD_INFO))
        {
            res = -5;
            goto Error1;
        }
        fs_sync(fil_scan_hdl->header_fil);

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

int fs_set_playlist_info_to_head_bin(const char *file_name, T_FILE_SCAN_HANDLE *fil_scan_hdl,
                                     uint16_t playlist_idx, uint16_t file_idx)
{
    int res = 0;
    int fs_res = 0;
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
    if (res || strcmp(file_name, hdr_name->name) != 0)
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
            if (strcmp(file_name, hdr_name->name) == 0)
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
                        &hdr_name->hdr_info, sizeof(T_HEAD_INFO));
    if (fs_res < 0 || fs_res != sizeof(T_HEAD_INFO))
    {
        res = -5;
        goto Error1;
    }
    fs_sync(fil_scan_hdl->header_fil);

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
    struct fs_dir_t dir;
    static struct fs_dirent entry;
    *file_changed = false;
    T_HEAD_NAME *hdr_name = NULL;
    int fs_res = 0;
    if (fs_read_header_bin_count(&hdr_cnt, fil_scan_hdl))
    {
        res = -1;
        goto Error;
    }
    fs_dir_t_init(&dir);
    fs_res = fs_opendir(&dir, fil_scan_hdl->file_path);
    if (0 != fs_res)
    {
        res = -2;
        goto Error;
    }

    while (1)
    {
        if (0 != fs_readdir(&dir, &entry) || entry.name[0] == 0)
        {
            break;
        }
        if (entry.attrib & FS_AM_HID)
        {
            continue;
        }
        if (entry.attrib & FS_AM_ARC)
        {
            if (fs_check_file_ext(entry.name, fil_scan_hdl->scan_ext) != 0xff)
            {
                fil_cnt++;
                if (fil_cnt % 200 == 0)
                {
                    WDG_Kick();
                }
            }
        }
    }

    fs_closedir(&dir);
    APP_PRINT_TRACE2("fs_check_file_change_happen fil_cnt %d hdr_cnt%d", fil_cnt, hdr_cnt);

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
        if (0 != fs_opendir(&dir, fil_scan_hdl->file_path))
        {
            res = -3;
            goto Error;
        }

        hdr_name = os_mem_alloc(fs_ram_type, sizeof(T_HEAD_NAME));
        if (hdr_name == NULL)
        {
            fs_closedir(&dir);
            res = -4;
            goto Error;
        }
        memset(hdr_name, 0x00, sizeof(T_HEAD_NAME));

        while (1)
        {
            if (0 != fs_readdir(&dir, &entry) || entry.name[0] == 0)
            {
                break;
            }
            if (entry.attrib & FS_AM_HID)
            {
                continue;
            }
            if (entry.attrib & FS_AM_ARC)
            {
                if (fs_check_file_ext(entry.name, fil_scan_hdl->scan_ext) != 0xff)
                {
                    if (fs_read_header_name_info(fil_idx, hdr_name, fil_scan_hdl))
                    {
                        res = -5;
                        goto Error1;
                    }
                    if (strcmp(entry.name, hdr_name->name) != 0 || hdr_name->hdr_info.needToUnlink)
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
        fs_closedir(&dir);
    }

    return res;
Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
    fs_closedir(&dir);
Error:
    APP_PRINT_ERROR1("ERROR: fs_check_file_change_happen res = %d", res);
    return res;
}

static int fs_update_head_name_bin(T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    struct fs_dir_t dir;
    static struct fs_dirent entry;
    uint32_t name_ofs = 0;
    uint16_t fil_idx = 0;
    int fs_res = 0;
    T_HEAD_NAME *hdr_name = NULL;
    char *bin_path = NULL;
    uint16_t hdr_cnt = 0;
    uint16_t path_len = 0;
    uint16_t scan_path_len = 0;
    uint16_t delete_file_path_len = 0;
    uint16_t i = 0;
    size_t file_size = 0;

    if (fil_scan_hdl->hdr_name_open == false)
    {
        if (fs_open_head_name_bin(fil_scan_hdl))
        {
            res = -1;
            goto Error;
        }
    }

    hdr_name = os_mem_zalloc(fs_ram_type, sizeof(T_HEAD_NAME));
    if (hdr_name == NULL)
    {
        res = -2;
        goto Error;
    }
    if (fs_read_header_bin_count(&hdr_cnt, fil_scan_hdl))
    {
        res = -3;
        goto Error1;
    }
    APP_PRINT_TRACE1("fs_update_head_name_bin hdr_cnt 0x%x", hdr_cnt);

    fs_res = fs_size(fil_scan_hdl->header_fil, &file_size);
    if ((fs_res == 0) && (file_size != 0))
    {
        /*Delete the incomplete files generated by transmission errors.*/
        if (file_size > (hdr_cnt * sizeof(T_HEAD_INFO) + FS_HEADER_INFO_START))
        {
            if (fs_read_header_name_info(hdr_cnt, hdr_name, fil_scan_hdl))
            {
                res = -4;
                goto Error1;
            }
            else
            {
                if (hdr_name->hdr_info.needToUnlink)
                {
                    scan_path_len = strlen(fil_scan_hdl->file_path);
                    delete_file_path_len = hdr_name->hdr_info.length;
                    path_len = scan_path_len + delete_file_path_len + 2;
                    bin_path = os_mem_zalloc(fs_ram_type, path_len);
                    if (bin_path == NULL)
                    {
                        res = -5;
                        goto Error1;
                    }

                    snprintf(bin_path, path_len, "%s%s", fil_scan_hdl->file_path, hdr_name->name);

                    APP_PRINT_INFO1("f_unlink file %s", TRACE_STRING(bin_path));
                    WDG_Kick();
                    if (fs_unlink(bin_path))
                    {
                        res = -6;
                        goto Error2;
                    }
                }
            }
        }
        else
        {
            for (i = 0; i < hdr_cnt; i++)
            {
                if (fs_read_header_name_info(i, hdr_name, fil_scan_hdl))
                {
                    res = -4;
                    goto Error1;
                }
                else
                {
                    if (hdr_name->hdr_info.needToUnlink)
                    {
                        scan_path_len = strlen(fil_scan_hdl->file_path);
                        delete_file_path_len = hdr_name->hdr_info.length;
                        path_len = scan_path_len + delete_file_path_len + 2;
                        bin_path = os_mem_zalloc(fs_ram_type, path_len);
                        if (bin_path == NULL)
                        {
                            res = -5;
                            goto Error1;
                        }

                        snprintf(bin_path, path_len, "%s%s", fil_scan_hdl->file_path, hdr_name->name);

                        APP_PRINT_INFO1("f_unlink file %s", TRACE_STRING(bin_path));
                        WDG_Kick();
                        if (fs_unlink(bin_path))
                        {
                            res = -6;
                            goto Error2;
                        }
                    }
                }
            }
        }
        fs_res = fs_truncate(fil_scan_hdl->header_fil, 0);
        if (fs_res < 0)
        {
            res = -7;
            goto Error2;
        }
        fs_res = fs_truncate(fil_scan_hdl->name_fil, 0);
        if (res < 0)
        {
            res = -8;
            goto Error2;
        }
    }

    fs_dir_t_init(&dir);
    if (0 != fs_opendir(&dir, fil_scan_hdl->file_path))
    {
        res = -9;
        goto Error2;
    }

    while (1)
    {
        if (0 != fs_readdir(&dir, &entry) || entry.name[0] == 0)
        {
            break;
        }
        if (entry.attrib & FS_AM_HID)
        {
            continue;
        }
        if (entry.attrib & FS_AM_ARC)
        {
            uint8_t ext_type = fs_check_file_ext(entry.name, fil_scan_hdl->scan_ext);
            if (ext_type != 0xff)
            {
                T_HEAD_INFO hdr_info;
                memset(&hdr_info, 0, sizeof(T_HEAD_INFO));
                hdr_info.length = (strlen(entry.name) + 1); // +1 for '/'
                hdr_info.offset = name_ofs;
                hdr_info.plIndex = 0x1;
                hdr_info.fil_ext = ext_type;
                hdr_info.rsv = 0x0;

                APP_PRINT_TRACE1("fs_update_head_name_bin entry name %s", TRACE_STRING(entry.name));
                fs_res = seek_write(fil_scan_hdl->header_fil, FS_HEADER_INFO_START + fil_idx * sizeof(T_HEAD_INFO),
                                    &hdr_info, sizeof(T_HEAD_INFO));
                if (fs_res < 0 || fs_res != sizeof(T_HEAD_INFO))
                {
                    res = -10;
                    goto Error3;
                }

                fs_res = seek_write(fil_scan_hdl->name_fil, name_ofs, entry.name, hdr_info.length);
                if (fs_res < 0 || fs_res != hdr_info.length)
                {
                    res = -11;
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
        res = -12;
        goto Error5;
    }
    fs_sync(fil_scan_hdl->header_fil);
    fs_sync(fil_scan_hdl->name_fil);

    os_mem_free(bin_path);
    bin_path = NULL;
    os_mem_free(hdr_name);
    hdr_name = NULL;
    return res;

Error5:
    fs_sync(fil_scan_hdl->name_fil);
Error4:
    fs_sync(fil_scan_hdl->header_fil);
Error3:
    fs_closedir(&dir);
Error2:
    os_mem_free(bin_path);
    bin_path = NULL;
Error1:
    os_mem_free(hdr_name);
    hdr_name = NULL;
Error:
    APP_PRINT_ERROR2("ERROR: fs_update_head_name_bin res = %d fs_res %d", res, fs_res);
    return res;
}

int fs_scan_file_list(T_FILE_SCAN_HANDLE *fil_scan_hdl, bool *file_list_change)
{
    int res = 0;
    int fs_res = 0;
    struct fs_dir_t dir;

    if (fil_scan_hdl == NULL || fil_scan_hdl->header_fil == NULL || fil_scan_hdl->name_fil == NULL)
    {
        res = -1;
        goto Error;
    }

    fs_dir_t_init(&dir);
    fs_res = fs_opendir(&dir, fil_scan_hdl->file_path);
    if (fs_res == -ENOENT)
    {
        fs_res = fs_mkdir(fil_scan_hdl->file_path);
        if (fs_res != 0)
        {
            res = -3;
            goto Error;
        }
    }
    else if (fs_res != 0)
    {
        res = -4;
        goto Error;
    }
    else
    {
        fs_closedir(&dir);
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
    // header bin name bin processing all done, change directory to "/SD:/audio"
    fs_chdir(fil_scan_hdl->file_path);

    return res;

Error:
    APP_PRINT_ERROR1("ERROR: fs_scan_file_list res = %d", res);
    return res;
}

int fs_create_scan_handle(const char *fil_path, const T_FILE_EXTENSION *scan_ext,
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
        fil_scan_hdl->header_fil = (struct fs_file_t *)os_mem_alloc(fs_ram_type, sizeof(struct fs_file_t));
        if (fil_scan_hdl->header_fil == NULL)
        {
            res = -2;
            goto header_malloc_fail;
        }
    }

    if (fil_scan_hdl->name_fil == NULL)
    {
        fil_scan_hdl->name_fil = (struct fs_file_t *)os_mem_alloc(fs_ram_type, sizeof(struct fs_file_t));
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
    int fs_res = 0;
    uint16_t count = 0;
    struct fs_dir_t dir;
    static struct fs_dirent entry;
    char *full_path = NULL;
    uint16_t full_path_len = 0;
    uint16_t scan_path_len = 0;
    uint16_t delete_file_path_len = 0;

    res = fs_write_header_bin_count(&count, fil_scan_hdl);
    fs_truncate(fil_scan_hdl->header_fil, FS_HEADER_INFO_START);
    fs_truncate(fil_scan_hdl->name_fil, 0);
    fs_sync(fil_scan_hdl->header_fil);
    fs_sync(fil_scan_hdl->name_fil);
    fs_dir_t_init(&dir);
    if (0 != fs_opendir(&dir, fil_scan_hdl->file_path))
    {
        res = -1;
        goto Error;
    }
    else
    {
        while (1)
        {
            if (0 != fs_readdir(&dir, &entry) || entry.name[0] == 0)
            {
                break;
            }
            else if (entry.attrib & (FS_AM_HID | FS_AM_SYS))
            {
                //hidden or system file, no parse
                continue;
            }
            if (entry.attrib & FS_AM_ARC)
            {
                scan_path_len = strlen(fil_scan_hdl->file_path);
                delete_file_path_len = strlen(entry.name);
                full_path_len = scan_path_len + delete_file_path_len + 2;
                full_path = os_mem_zalloc(fs_ram_type, full_path_len);
                if (full_path == NULL)
                {
                    res = -2;
                    goto Error1;
                }
                snprintf(full_path, full_path_len, "%s/%s", fil_scan_hdl->file_path, entry.name);
                WDG_Kick();
                fs_res = fs_unlink(full_path);
                if ((fs_res == 0) || (fs_res == -ENFILE))
                {
                    count++;
                }
                else
                {
                    APP_PRINT_ERROR3("fs_unlink_all_files: fs_res:0x%d, file size:0x%x, filename:%s",
                                     fs_res,
                                     entry.size,
                                     TRACE_STRING(entry.name));
                    res = -3;
                    os_mem_free(full_path);
                    goto Error1;
                }
                os_mem_free(full_path);
                full_path = NULL;
            }
        }
        fs_closedir(&dir);
    }
    APP_PRINT_INFO4("fs_unlink_all_files, res:0x%x, fs_res:%d, count:%d, delete_path(%s)",
                    res, fs_res, count, TRACE_STRING(fil_scan_hdl->file_path));
    return res;
Error1:
    fs_closedir(&dir);
Error:
    APP_PRINT_ERROR1("ERROR: fs_unlink_all_files res = %d", res);
    return res;
}

int fs_delete_all_files_by_format(T_FILE_FORMAT format, T_FILE_SCAN_HANDLE *fil_scan_hdl)
{
    int res = 0;
    uint16_t count = 0;
    T_HEAD_NAME *hdr_name = NULL;
    char *full_path = NULL;
    uint16_t full_path_len = 0;
    uint16_t scan_path_len = 0;
    uint16_t delete_file_path_len = 0;

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
            scan_path_len = strlen(fil_scan_hdl->file_path);
            delete_file_path_len = hdr_name->hdr_info.length;
            full_path_len = scan_path_len + delete_file_path_len + 2;
            full_path = os_mem_zalloc(fs_ram_type, full_path_len);
            if (full_path == NULL)
            {
                res = -4;
                goto Error;
            }
            snprintf(full_path, full_path_len, "%s%s", fil_scan_hdl->file_path, hdr_name->name);
            if (fs_unlink(full_path))
            {
                res = -5;
                os_mem_free(full_path);
                goto Error;
            }
            os_mem_free(full_path);
            full_path = NULL;
            hdr_name->hdr_info.isDeleted = 1;
            if (fs_write_header_name_info(i, hdr_name, fil_scan_hdl))
            {
                res = -6;
                goto Error;
            }
            fs_sync(fil_scan_hdl->header_fil);
        }
    }
    fs_sync(fil_scan_hdl->header_fil);
    os_mem_free(hdr_name);
    hdr_name = NULL;
    return res;
Error:
    os_mem_free(hdr_name);
    hdr_name = NULL;
    APP_PRINT_ERROR1("ERROR: fs_delete_all_files_by_format res = %d", res);
    return res;
}

int fs_init(struct fs_mount_t *mp)
{
    int res = 0;

    if (mp->fs_data == NULL)
    {
        mp->fs_data = os_mem_alloc(fs_ram_type, sizeof(FATFS));
        if (mp->fs_data == NULL)
        {
            res = -1;
            goto Error1;
        }
    }

    res = fs_mount(mp);
    if (0 != res)
    {
        res = -2;
        goto mount_fail;
    }

    if (0 != fs_chdrive(mp->mnt_point))
    {
        res = -3;
        goto mount_fail;
    }

    return res;

mount_fail:
    os_mem_free(mp->fs_data);
    mp->fs_data = NULL;
Error1:
    APP_PRINT_ERROR1("ERROR: fs_init res = %d", res);
    return res;
}

int fs_deinit(struct fs_mount_t *mp)
{

    if (0 != fs_unmount(mp))
    {
        return -1;
    }
    if (mp->fs_data)
    {
        os_mem_free(mp->fs_data);
        mp->fs_data = NULL;
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

int32_t disk_power_off(void)
{
    if (disk_access_ioctl(FATFS_DISK_NAME, DISK_IOCTL_CTRL_DEINIT, NULL) != 0)
    {
        return -1;
    }
    return 0;
}

int32_t disk_power_on(void)
{
    if (disk_access_ioctl(FATFS_DISK_NAME, DISK_IOCTL_CTRL_INIT, NULL) != 0)
    {
        return -1;
    }
    return 0;
}