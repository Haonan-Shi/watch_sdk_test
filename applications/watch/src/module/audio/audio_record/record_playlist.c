/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <string.h>
#include <stdio.h>
#include "trace.h"
#include "record_playlist.h"
#include "fmc_api.h"
#include "os_sync.h"
#include "app_task.h"
#include "audio_resource.h"
#include "fs_if.h"


/*============================================================================*
 *                           Constants
 *============================================================================*/

#define SECTOR_SIZE             0x1000
#define NAME_BUF_SIZE           (RECORD_MAX_NAME_LEN * sizeof(T_HEAD_INFO))

/*============================================================================*
 *                            Variables
 *============================================================================*/
static int16_t play_index = 0;
static uint8_t name_buf[NAME_BUF_SIZE];
static T_HEAD_INFO *header_info = (T_HEAD_INFO *)(RECORD_HEADER_BIN_ADDR +
                                                  FS_HEADER_INFO_START);
static uint16_t *file_count = (uint16_t *)RECORD_HEADER_BIN_ADDR;

static const T_EXTENSION_DEF record_ext_def_array[1] =
{
    {0x00, ".PCM", ".pcm"},
};

static const T_FILE_EXTENSION record_scan_ext =
{
    .file_ext_num = 1,
    .file_ext = record_ext_def_array,
};
static T_FILE_SCAN_HANDLE record_scan_hdl;

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

static uint32_t record_fs_get_header_bin_size(void)
{
    size_t size = 0;
    int res = fs_size(record_scan_hdl.header_fil, &size);
    if (res != 0)
    {
        APP_PRINT_ERROR1("Failed to get file size: %d", res);
    }
    return size;
}

static int record_fs_read_header_bin(uint32_t offset, uint8_t *readBuf, uint32_t readLen,
                                     uint32_t *len)
{
    int fs_res = 0;

    if (readBuf == NULL)
    {
        return FR_INVALID_PARAMETER;
    }

    fs_res = seek_read(record_scan_hdl.header_fil, offset, readBuf, readLen);
    if (fs_res < 0)
    {
        return fs_res;
    }
    *len = fs_res;

    return 0;
}

static uint32_t record_fs_get_name_bin_size(void)
{
    size_t size = 0;
    int res = fs_size(record_scan_hdl.name_fil, &size);
    if (res != 0)
    {
        APP_PRINT_ERROR1("Failed to get file size: %d", res);
    }
    return size;
}

static int record_fs_read_name_bin(uint32_t offset, uint8_t *readBuf, uint32_t readLen,
                                   uint32_t *len)
{
    int fs_res = 0;

    if (readBuf == NULL)
    {
        return FR_INVALID_PARAMETER;
    }

    fs_res = seek_read(record_scan_hdl.name_fil, offset, readBuf, readLen);
    if (fs_res < 0)
    {
        return fs_res;
    }
    *len = fs_res;

    return 0;
}


static bool record_playlist_is_file_name_exist(const char *path)
{
    T_HEAD_INFO *headers;
    uint16_t file_count;
    size_t path_len;

    if (path == NULL)
    {
        return false;
    }

    headers = record_get_header_info_start();
    file_count = record_get_file_count();
    path_len = strlen(path) + 1; // Include null terminator
    if (headers == NULL || path_len == 0)
    {
        return false;
    }

    for (uint16_t index = 0; index < file_count; index++)
    {
        T_HEAD_INFO *header = &headers[index];

        if (header->length == path_len)
        {
            const char *exist_path = (const char *)(RECORD_NAME_BIN_ADDR + header->offset);

            if (memcmp(exist_path, path, path_len) == 0)
            {
                return true;
            }
        }
    }

    return false;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

/**
 * @brief Synchronizes playlist name and header data from the file system.
 *
 * This routine refreshes both cached bins and removes deleted song entries
 * from the header metadata before updating the stored song count.
 *
 * Flow:
 * 1. Erase NAME_BIN flash region (recording names)
 * 2. Read name data from filesystem and write to mapped flash region
 * 3. Invalidate DCache for name region
 * 4. Erase HEADER_BIN flash region (recording metadata)
 * 5. Read header data from filesystem, filter out deleted entries, write to flash
 * 6. Update song count (original count - deleted count)
 * 7. Invalidate DCache for header region
 */
void record_sync_playlist(void)
{
    uint32_t s;
    uint32_t name_offset = 0;
    uint32_t name_size = 0;
    uint32_t read_len = 0;
    uint32_t head_size;
    uint16_t del_num = 0;
    uint32_t write_offset = RECORD_HEADER_BIN_ADDR;
    uint32_t read_offset = 0;
    uint8_t header_cnt[FS_HEADER_INFO_START];
    uint16_t len_out = 0;

    /* Step 1: Erase NAME_BIN flash region */
    s = os_lock();
    for (uint8_t i = 0; i < RECORD_NAME_BIN_SIZE / SECTOR_SIZE; i++)
    {
        clear_header_name_info(RECORD_NAME_BIN_ADDR + i * SECTOR_SIZE,
                               FMC_FLASH_NOR_ERASE_SECTOR);
    }
    os_unlock(s);

    /* Step 2: Read name data from filesystem and write to flash mapping region */
    /* Process NAME_BUF_SIZE chunks sequentially */
    fs_open_head_name_bin(&record_scan_hdl);
    name_size = record_fs_get_name_bin_size();
    APP_PRINT_INFO1("[record playlist] name bin size = %d", name_size);

    for (uint16_t i = 0; i < name_size / NAME_BUF_SIZE; i++)
    {
        record_fs_read_name_bin(i * NAME_BUF_SIZE, name_buf, NAME_BUF_SIZE, &read_len);
        write_header_name_info(RECORD_NAME_BIN_ADDR + i * NAME_BUF_SIZE,
                               name_buf,
                               NAME_BUF_SIZE);
        name_offset += NAME_BUF_SIZE;
    }

    /* Handle remaining incomplete chunk */
    if (name_size % NAME_BUF_SIZE)
    {
        record_fs_read_name_bin(name_offset, name_buf, name_size % NAME_BUF_SIZE, &read_len);
        write_header_name_info(RECORD_NAME_BIN_ADDR + name_offset,
                               name_buf,
                               name_size % NAME_BUF_SIZE);
    }

    /* Step 3: Invalidate DCache to ensure flash writes are visible to CPU */
    s = os_lock();
    SCB_InvalidateDCache_by_Addr((uint32_t *)RECORD_NAME_BIN_ADDR, 0xA000);
    os_unlock(s);

    /* Get header bin size from filesystem */
    head_size = record_fs_get_header_bin_size();

    APP_PRINT_INFO1("[record playlist] header bin size = %d", head_size);

    /* Step 4: Erase HEADER_BIN flash region */
    s = os_lock();
    for (uint8_t i = 0; i < RECORD_HEADER_BIN_SIZE / SECTOR_SIZE; i++)
    {
        clear_header_name_info(RECORD_HEADER_BIN_ADDR + i * SECTOR_SIZE,
                               FMC_FLASH_NOR_ERASE_SECTOR);
    }
    os_unlock(s);

    /* Step 5: Process header data - read, filter deleted, write back */
    if (head_size >= FS_HEADER_INFO_START)
    {
        record_fs_read_header_bin(read_offset, header_cnt, FS_HEADER_INFO_START, &read_len);
        write_offset += FS_HEADER_INFO_START;
        read_offset += FS_HEADER_INFO_START;

        for (uint16_t i = 0; i < (head_size - FS_HEADER_INFO_START) / NAME_BUF_SIZE; i++)
        {
            record_fs_read_header_bin(read_offset, name_buf, NAME_BUF_SIZE, &read_len);
            /* Filter out entries marked as deleted (isDeleted = true) */
            filter_del_header_info(name_buf, NAME_BUF_SIZE, &len_out);
            del_num += (NAME_BUF_SIZE - len_out) / sizeof(T_HEAD_INFO);
            write_header_name_info(write_offset, name_buf, len_out);
            write_offset += len_out;
            read_offset += NAME_BUF_SIZE;
        }
        if ((head_size - FS_HEADER_INFO_START) % NAME_BUF_SIZE)
        {
            uint16_t len_in = (head_size - FS_HEADER_INFO_START) % NAME_BUF_SIZE;

            record_fs_read_header_bin(read_offset, name_buf, len_in, &read_len);
            filter_del_header_info(name_buf, len_in, &len_out);
            del_num += (len_in - len_out) / sizeof(T_HEAD_INFO);
            write_header_name_info(write_offset, name_buf, len_out);
        }
    }
    else
    {
        record_fs_read_header_bin(read_offset, header_cnt, head_size, &read_len);
    }

    /* Step 6: Update song count - subtract deleted entries */
    uint16_t *count = (uint16_t *)header_cnt;

    *count -= del_num;
    write_header_name_info(RECORD_HEADER_BIN_ADDR, header_cnt, FS_HEADER_INFO_START);

    /* Step 7: Invalidate DCache for header region to reflect updates */
    s = os_lock();
    SCB_InvalidateDCache_by_Addr((uint32_t *)RECORD_HEADER_BIN_ADDR, 0x5000);
    os_unlock(s);

    fs_close_head_name_bin(&record_scan_hdl);

    APP_PRINT_INFO2("[record playlist]del_num =%d, Header bin = %b",
                    del_num,
                    TRACE_BINARY(head_size, (uint8_t *)RECORD_HEADER_BIN_ADDR));
}

int32_t record_fs_init(void)
{
    bool playlist_changed;
    uint16_t count;

    int32_t ret = 0;
    ret = fs_create_scan_handle(RECORD_FILE_PATH, &record_scan_ext, &record_scan_hdl);

    if (ret != 0)
    {
        APP_PRINT_ERROR1("[record playlist]fs_create_scan_handle error: %d", ret);
        return ret;
    }

    ret = fs_scan_file_list(&record_scan_hdl, &playlist_changed);
    if (ret != 0)
    {
        APP_PRINT_ERROR1("[record playlist]fs_scan_file_list error: %d", ret);
        return ret;
    }
    ret = fs_read_header_bin_count(&count, &record_scan_hdl);
    if (ret != 0)
    {
        fs_close_head_name_bin(&record_scan_hdl);
        APP_PRINT_ERROR1("[record playlist]fs_read_header_bin_count error !!! %d", ret);
        return ret;
    }

    if (playlist_changed || (count != *(uint16_t *)RECORD_HEADER_BIN_ADDR))
    {
        record_sync_playlist();
    }
    app_fs_disk_power_down_enable(APP_DISK_CHECK_RECORD);
    fs_close_head_name_bin(&record_scan_hdl);
    APP_PRINT_TRACE0("[record playlist] record fs init done");

    return 0;
}

/**
 * @brief Gets the current number of recording files.
 *
 * @return Number of recording files currently cached.
 */
uint16_t record_get_file_count(void)
{
    return *file_count;
}

/**
 * @brief Gets the start address of the cached recording header list.
 *
 * @return Pointer to the first recording header entry.
 */
T_HEAD_INFO *record_get_header_info_start(void)
{
    return header_info;
}

/**
 * @brief Gets the current recording playback index.
 *
 * @return Zero-based index of the current recording item.
 */
uint16_t record_get_cur_play_index(void)
{
    return play_index;
}

/**
 * @brief Resets the current recording playback index to the first entry.
 */
void record_reset_cur_play_index(void)
{
    play_index = 0;
}

/**
 * @brief Gets the header information for the current recording item.
 *
 * @return Pointer to the current recording header entry.
 */
T_HEAD_INFO *record_get_cur_play_header_info(void)
{
    return header_info + play_index;
}

T_HEAD_INFO *record_get_lastest_play_header_info(void)
{
    if (record_get_file_count() == 0)
    {
        return header_info;
    }
    return header_info + record_get_file_count() - 1;
}

int record_playlist_update(void *file, const char *file_name, uint16_t str_len)
{
    T_FILE_HANDLE *fil_hdl = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(T_FILE_HANDLE));

    if (fil_hdl == NULL)
    {
        APP_PRINT_ERROR0("record_playlist_update: failed to allocate memory for file handle");
        return -1;
    }
    fil_hdl->fil.filep = file;
    memcpy(fil_hdl->filename, file_name, str_len);
    fil_hdl->filename[str_len] = '\0';
    fil_hdl->namelen = str_len + 1;

    APP_PRINT_TRACE1("record_playlist_update: file name %s", TRACE_STRING(file_name));

    fs_file_add_to_header_name_bin(fil_hdl, &record_scan_hdl);
    fs_file_update_header_bin(fil_hdl, &record_scan_hdl);
    record_sync_playlist();
    os_mem_free(fil_hdl);
    return 0;
}

/**
 * @brief Loads the latest recording file name into a caller-provided buffer.
 *
 * @param[out] path Output buffer for the file name.
 * @param[in] path_len Size of the output buffer in bytes.
 *
 * @return Zero on success, negative value on failure.
 */
int32_t record_playlist_load_latest_file_name(char *path, size_t path_len)
{
    /* excluded "/SD:record/", just "record_xxxx.pcm"*/
    char name_path[32] = {0};

    if (path == NULL)
    {
        APP_PRINT_ERROR0("[record playlist] record_playlist_load_latest_file_name: path is NULL");
        return -1;
    }
    if (record_get_file_count() == 0)
    {
        APP_PRINT_ERROR0("[record playlist] record_playlist_load_latest_file_name: no recording files");
        return -2;
    }

    T_HEAD_INFO *last_header = record_get_lastest_play_header_info();
    if (last_header->length == 0 || last_header->length >= sizeof(name_path))
    {
        APP_PRINT_ERROR1("record_playlist_load_latest_file_name: invalid name length %d",
                         last_header->length);
        return -3;
    }
    snprintf(name_path, sizeof(name_path), "%s",
             (const char *)(RECORD_NAME_BIN_ADDR + last_header->offset));
    name_path[last_header->length] = '\0';

    APP_PRINT_INFO1("[record playlist] record_playlist_load_latest_file_name: %s",
                    TRACE_STRING(path));
    return 0;
}

/**
 * @brief Generates a new unique recording file name.
 *
 * @param[out] path Output buffer for the generated file name.
 * @param[in] path_len Size of the output buffer in bytes.
 *
 * @return Zero on success, negative value on failure.
 */
int32_t record_playlist_generate_new_file_name(char *path, size_t path_len)
{
    uint16_t start_index = 0;
    /* excluded "/SD:record/", just "record_xxxx.pcm"*/
    char name_path[32] = {0};

    if (path == NULL)
    {
        return -1;
    }

    uint16_t file_count = record_get_file_count();
    if (file_count > 0)
    {
        T_HEAD_INFO *last_header = record_get_lastest_play_header_info();
        if (last_header == NULL || last_header->length == 0 || last_header->length > 31)
        {
            APP_PRINT_ERROR0("[record playlist] invalid last header, use default");
            start_index = 0;
        }
        else
        {
            /* Copy to local buffer with null terminator for safe parsing */
            char lastest_name[32] = {0};
            uint16_t copy_len = last_header->length;
            if (copy_len >= sizeof(lastest_name))
            {
                copy_len = sizeof(lastest_name) - 1;
            }
            memcpy(lastest_name, (const char *)(RECORD_NAME_BIN_ADDR + last_header->offset), copy_len);
            lastest_name[copy_len] = '\0';

            APP_PRINT_TRACE2("[record playlist] record_playlist_generate_new_file_name: current file count %d, lastest file name %s",
                             file_count, TRACE_STRING(lastest_name));

            /* Extract numeric part from "record_xxxx.pcm" */
            uint32_t extracted_num = 0;
            if (sscanf(lastest_name, "record_%4u.pcm", &extracted_num) == 1)
            {
                start_index = (uint16_t)extracted_num;
                APP_PRINT_INFO1("[record playlist] extracted number from file name: %u", start_index);
            }
            else
            {
                APP_PRINT_WARN1("[record playlist] failed to parse file name: %s", TRACE_STRING(lastest_name));
            }
        }
    }
    else
    {
        APP_PRINT_TRACE1("[record playlist] record_playlist_generate_new_file_name: no existing files, start from 0",
                         file_count);
    }

    while (1)
    {
        start_index++;
        snprintf(name_path, sizeof(name_path), "record_%04u.pcm", start_index);
        name_path[sizeof(name_path) - 1] = '\0';
        if (!record_playlist_is_file_name_exist(name_path))
        {
            APP_PRINT_INFO1("[record playlist] record_playlist_generate_new_file_name: %s",
                            TRACE_STRING(name_path));
            break;
        }
    }
    snprintf(path, path_len, "%s", name_path);

    return 0;
}

/**
 * @brief Validates that a path is a record file path with a PCM extension.
 *
 * @param[in] path File path to validate.
 *
 * @retval true The path is valid for a recording file.
 * @retval false The path is invalid.
 */
bool record_playlist_ensure_file_path(char *path)
{
    size_t path_len = 0;

    if (path == NULL)
    {
        return false;
    }
    path_len = strlen(path);
    if (path_len > RECORD_MAX_NAME_LEN)
    {
        APP_PRINT_ERROR1("record_playlist_ensure_file_path: path too long %d", path_len);
        return false;
    }
    if (strncmp(path, RECORD_FILE_PATH, strlen(RECORD_FILE_PATH)) != 0)
    {
        APP_PRINT_ERROR1("record_playlist_ensure_file_path: invalid path %s", TRACE_STRING(path));
        return false;
    }
    if (strstr(path, ".pcm") == NULL && strstr(path, ".PCM") == NULL)
    {
        APP_PRINT_ERROR1("record_playlist_ensure_file_path: not a PCM file %s", TRACE_STRING(path));
        return false;
    }
    return true;
}
