/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include "vendor_file.h"
#include "rtl876x_pinmux.h"
#include "rtl876x_rcc.h"
#include "zephyr/fs/fs.h"
#include "alipay_mem.h"
#include "section.h"
#include "wchar.h"
#include "app_fs_if.h"
#include "alipay_config.h"
#include <stdio.h>

#if CONFIG_ALIPAY

#define SUFFIX_STR   (".ali")
#define SUFFIX_LEN   strlen(SUFFIX_STR)

const char *root_path = (char *)ALIPAY_FILE_PATH;
struct fs_file_t g_alipay_file_handle = {0};

void *alipay_open_rsvd_part(PARAM_IN char filename[128])
{
    size_t root_len = strlen(root_path);
    size_t name_len = strlen(filename);
    size_t suffix_len = SUFFIX_LEN;
    size_t total_len = root_len + name_len + suffix_len;
    struct fs_dir_t dir;

    AliPay_LOG("[Alipay][LOG] open name %s, namelen %u", filename, (unsigned)name_len);

    char *path = (char *)csi_malloc(total_len + 1);
    if (!path)
    {
        AliPay_LOG("alipay_open_rsvd_part: malloc failed");
        return NULL;
    }
    memcpy(path, root_path, strlen(root_path));
    memcpy(path + root_len, filename, name_len);
    memcpy(path + root_len + name_len, SUFFIX_STR, suffix_len);
    path[total_len] = '\0';

    AliPay_LOG("[Alipay][LOG] path %p, path=%s", (void *)path, path);

    fs_dir_t_init(&dir);
    int fs_res = fs_opendir(&dir, root_path);
    if (fs_res == -ENOENT)
    {
        fs_res = fs_mkdir(root_path);
        if (fs_res != 0)
        {
            AliPay_LOG("alipay_open_rsvd_part: mkdir alipay dir error! res=%d", fs_res);
            csi_free(path);
            return NULL;
        }
    }
    else if (fs_res != 0)
    {
        AliPay_LOG("alipay_open_rsvd_part: open alipay dir error! res=%d", fs_res);
        csi_free(path);
        return NULL;
    }
    else
    {
        fs_closedir(&dir);
    }

    fs_file_t_init(&g_alipay_file_handle);
    int res = fs_open(&g_alipay_file_handle,
                      path,
                      FS_O_RDWR | FS_O_CREATE);

    csi_free(path);

    if (res != 0)
    {
        AliPay_LOG("alipay_open_rsvd_part: open alipay file error! res=%d", res);
        return NULL;
    }
    return (void *)&g_alipay_file_handle;
}

int alipay_write_rsvd_part(PARAM_IN void *fd, PARAM_IN void *data, PARAM_IN uint32_t data_len)
{
    AliPay_LOG("[Alipay][LOG] write");
    ssize_t actual_write_len;
    actual_write_len =  fs_write(fd, data, data_len);
    if (actual_write_len != data_len)
    {
        AliPay_LOG("alipay_write_rsvd_part: write file error!");
        return -1;
    }
    return 0;
}

int alipay_read_rsvd_part(PARAM_IN void *fd,
                          PARAM_OUT void *buffer,
                          PARAM_INOUT uint32_t *read_len)
{
    AliPay_LOG("[Alipay][LOG] read");
    if ((buffer == NULL) || (fd == NULL) || (read_len == NULL))
    {
        return -1;
    }

    ssize_t actual_read_len = 0;
    actual_read_len = fs_read(fd, buffer, *read_len);
    if (actual_read_len <= 0)
    {
        AliPay_LOG("alipay_read_rsvd_part: read file error!");
        return -1;
    }
    *read_len = actual_read_len;
    return 0;
}

int alipay_close_rsvd_part(PARAM_IN void *fd)
{
    AliPay_LOG("[Alipay][LOG] close");
    int res = fs_close(fd);
    if (res)
    {
        AliPay_LOG("alipay_close_rsvd_part: close file error!");
        return -1;
    }
    return 0;
}

int alipay_access_rsvd_part(PARAM_IN char filename[128])
{
    AliPay_LOG("[Alipay][LOG] check exist");

    size_t name_len = strlen(filename);
    size_t suffix_len = strlen(SUFFIX_STR);
    size_t total_len = name_len + suffix_len;

    char *path = (char *)csi_malloc(total_len + 1);
    if (!path)
    {
        AliPay_LOG("alipay_access_rsvd_part: malloc failed");
        return 0;
    }

    memcpy(path, filename, name_len);
    memcpy(path + name_len, SUFFIX_STR, suffix_len);
    path[total_len] = '\0';

    struct fs_dirent fileinfo;
    memset(&fileinfo, 0, sizeof(fileinfo));

    int res = fs_stat(path, &fileinfo);
    csi_free(path);

    if (res != 0)
    {
        AliPay_LOG("alipay_access_rsvd_part: file not exist! res=%d", res);
        return 0;
    }
    return 1;
}

int alipay_remove_rsvd_part(PARAM_IN char filename[128])
{
    AliPay_LOG("[Alipay][LOG] delete");

    size_t name_len = strlen(filename);
    size_t suffix_len = strlen(SUFFIX_STR);
    size_t total_len = name_len + suffix_len;

    char *path = (char *)csi_malloc(total_len + 1);
    if (!path)
    {
        AliPay_LOG("alipay_remove_rsvd_part: malloc failed");
        return -1;
    }

    memcpy(path, filename, name_len);
    memcpy(path + name_len, SUFFIX_STR, suffix_len);
    path[total_len] = '\0';

    int res = fs_unlink(path);

    csi_free(path);

    if (res != 0)
    {
        AliPay_LOG("alipay_remove_rsvd_part: remove file failed! res=%d", res);
        return -1;
    }
    return 0;
}

int alipay_clear_rsvd_part(void)
{
    void alipay_fsdir_search_with_suffix(const char *path, const char *suffix);
    alipay_fsdir_search_with_suffix(root_path, SUFFIX_STR);

    return 0;
}

int alipay_has_suffix(const char *filename, const char *suffix)
{
    if (!filename || !suffix)
    {
        return 0;
    }

    size_t fname_len = strlen(filename);
    size_t suffix_len = strlen(suffix);

    if (suffix_len == 0 || suffix_len > fname_len)
    {
        return 0;
    }

    return (strcmp(filename + fname_len - suffix_len, suffix) == 0) ? 1 : 0;
}

void alipay_fsdir_search_with_suffix(const char *path, const char *suffix)
{
    int res;
    struct fs_dir_t dir;
    struct fs_dirent entry;
    char full_file_path[256];
    fs_dir_t_init(&dir);
    res = fs_opendir(&dir, path);
    if (res == 0)
    {
        DBG_DIRECT("alipay dir success,path %s", path);
        for (;;)
        {
            memset(&entry, 0, sizeof(entry));
            res = fs_readdir(&dir, &entry);
            if (res != 0 || entry.name[0] == 0)//TO-DO:end check
            {
                break;
            }
            uint16_t total_len = strlen(entry.name) + strlen(path) + 2; // for '/' and '\0'
            snprintf(full_file_path, total_len, "%s%s", path, entry.name);

            if (alipay_has_suffix(entry.name, suffix))
            {
                res = fs_unlink(full_file_path);
                if (res != 0)
                {
                    AliPay_LOG("alipay_fsdir_search_with_suffix: unlink file failed! res=%d", res);
                }
                else
                {
                    AliPay_LOG("unlink ok: %s", full_file_path);
                }
            }
            if (entry.attrib & FS_DIR_ENTRY_DIR)
            {
                //dir recursion
                char subpath[256];
                strncpy(subpath, path, strlen(path));
                strncpy(subpath + strlen(path), "/", strlen("/"));
                strncpy(subpath + strlen(path) + strlen("/"), entry.name, strlen(entry.name));
                alipay_fsdir_search_with_suffix(subpath, suffix);
            }
        }
        fs_closedir(&dir);
    }
    else
    {
        AliPay_LOG("alipay open dir failed");
    }
}


#endif // CONFIG_ALIPAY
