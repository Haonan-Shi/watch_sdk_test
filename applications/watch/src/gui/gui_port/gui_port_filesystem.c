/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#include <zephyr/fs/fs.h>
#include "gui_vfs.h"
#include "os_mem.h"
#include "gui_api_os.h"
#include "trace.h"
#include "wdg.h"
#if (CONFIG_APP_NANDBOOT == 1)
#include "fmc_api_ext.h"
#include "flash_map.h"
#endif
// if adapt, please refer to honeygui\realgui\misc\vfs\README.md

#define PORT_FS_DEBUG       0
#define PACK_NAME_MAX_LEN  64
#define PACK_MAGIC         0x5041434B  // 'PACK', keep in sync with pack_bin.py

#if (CONFIG_APP_NANDBOOT == 1)
#define RES_BIN_PATH       "/NAND:/res.bin"
#else
#define RES_BIN_PATH       "/SD:/res.bin"
#endif

typedef struct
{
    uint32_t magic;       // 魔数
    uint16_t version;      // 版本号
    uint16_t entry_count;  // 索引项个数 N
    uint32_t index_offset;  // 索引表起始偏移（通常 = sizeof(PACK_HEADER)）
    uint32_t data_offset;   // 资源数据区起始偏移
    uint32_t reserved[4];   // 预留，将来用得上（CRC、时间戳等）
} PACK_HEADER;

typedef struct
{
    char     name[PACK_NAME_MAX_LEN];  // 资源名称/路径字符串
    // 例如 "ui/home/bg.bin"，必须以 '\0' 结束
    uint32_t offset;        // 资源在 pack 文件中的偏移（从文件头起算）
    uint32_t size;          // 资源数据长度（字节）
    uint16_t type;          // 资源类型：如 1=图片, 2=字体, 3=通用bin...
    uint16_t flags;         // 标志位：bit0:压缩, bit1-bit15:颜色/格式等
    uint32_t reserved1;     // 预留
    uint32_t reserved2;     // 预留
} PACK_ENTRY;


PACK_HEADER res_pack_hdr;
PACK_ENTRY *res_entry;
struct fs_file_t  *res_file;

extern void gui_port_fs_init(void);

/* VFS operations */

static void *zephyr_vfs_open(const char *path, gui_vfs_mode_t mode, void *user_data)
{
#if PORT_FS_DEBUG
    uint32_t start_time = sys_timestamp_get_us();
#endif
    (void)user_data;

    GUI_ASSERT(mode == GUI_VFS_READ);
    if (mode != GUI_VFS_READ) { return NULL; }

    int low = 0;
    int high = (int)res_pack_hdr.entry_count - 1;
    while (low <= high)
    {
        int mid = (low + high) / 2;

        // entries[mid].name 是以 '\0' 结尾的 C 字符串
        int cmp = strcmp(path, res_entry[mid].name);

        if (cmp == 0)
        {
            // 找到了
            fs_seek(res_file, res_entry[mid].offset, FS_SEEK_SET);
#if PORT_FS_DEBUG
            uint32_t end_time = sys_timestamp_get_us();
            gui_log("zephyr_vfs_open string %s, time = %d", path, end_time - start_time);
#endif
            return &res_entry[mid];
        }
        else if (cmp < 0)
        {
            // target_name 比 mid 小，去左半边
            high = mid - 1;
        }
        else
        {
            // target_name 比 mid 大，去右半边
            low = mid + 1;
        }
    }
    return NULL;
}

static int zephyr_vfs_close(void *file)
{
    return 0;
}

static int zephyr_vfs_read(void *file, void *buf, size_t size)
{

    PACK_ENTRY *entry = (PACK_ENTRY *)file;
#if PORT_FS_DEBUG
    uint32_t start_time = sys_timestamp_get_us();
#endif
    ssize_t cnt = fs_read(res_file, buf, size);
#if PORT_FS_DEBUG
    uint32_t end_time = sys_timestamp_get_us();
    gui_log("zephyr_vfs_read ofs 0x%x, buf 0x%x, size %d, time %d", entry->offset, buf, size,
            end_time - start_time);
#endif
    return cnt;
}

static int zephyr_vfs_write(void *file, const void *buf, size_t size)
{
    (void)file; (void)buf; (void)size;
    return -1;  /* Read-only */
}

static int zephyr_vfs_seek(void *file, int offset, gui_vfs_seek_t whence)
{
#if PORT_FS_DEBUG
    uint32_t start_time = sys_timestamp_get_us();
#endif
    PACK_ENTRY *entry = (PACK_ENTRY *)file;
    switch (whence)
    {
    case GUI_VFS_SEEK_SET:
        {
            fs_seek(res_file, entry->offset + offset, FS_SEEK_SET);
        }
        break;
    case GUI_VFS_SEEK_CUR:
        {
            fs_seek(res_file, offset, FS_SEEK_CUR);
        }
        break;
    case GUI_VFS_SEEK_END:
        break;
    }
#if PORT_FS_DEBUG
    uint32_t end_time = sys_timestamp_get_us();
    gui_log("zephyr_vfs_seektime = %d", end_time - start_time);
#endif

    return 0;
}

static int zephyr_vfs_tell(void *file)
{
    PACK_ENTRY *entry = (PACK_ENTRY *)file;
    return entry->size;
}

#if 0
static void *zephyr_vfs_opendir(const char *path, void *user_data)
{
    (void)user_data;

    /* Add leading slash if missing or empty */
    char *full_path = NULL;
    if (!path || path[0] == '\0' || path[0] != '/')
    {
        size_t len = path ? strlen(path) : 0;
        full_path = (char *)gui_malloc(len + 2);
        if (!full_path) { return NULL; }
        full_path[0] = '/';
        if (len > 0)
        {
            strcpy(full_path + 1, path);
        }
        else
        {
            full_path[1] = '\0';
        }
        path = full_path;
    }

    struct fs_dir_t *dir = os_mem_alloc(RAM_TYPE_DATA_ON, sizeof(struct fs_dir_t));
    if (dir == NULL)
    {
        if (full_path) { gui_free(full_path); }
        return NULL;
    }
    fs_dir_t_init(dir);
    fs_opendir(dir, path);
    if (full_path) { gui_free(full_path); }

    return dir;
}

static int zephyr_vfs_readdir(void *dir, gui_vfs_stat_t *stat)
{
    static struct fs_dirent entry;
    int res = fs_readdir(dir, &entry);
    if (res != 0)
    {
        return res;
    }
    strncpy(stat->name, entry.name, sizeof(stat->name) - 1);
    stat->name[sizeof(stat->name) - 1] = '\0';
    stat->type = (entry.type == FS_DIR_ENTRY_DIR) ? GUI_VFS_TYPE_DIR : GUI_VFS_TYPE_FILE;
    stat->size = entry.size;

    return 0;
}

static int zephyr_vfs_closedir(void *dir)
{
    return fs_closedir(dir);
}

static int zephyr_vfs_stat(const char *path, gui_vfs_stat_t *stat, void *user_data)
{
    (void)user_data;

    struct fs_dirent entry;
    int res = fs_stat(path, &entry);
    if (res != 0) { return res; }

    if (res == 0)
    {
        stat->size = entry.size;
        stat->type = entry.type;
    }
    return res;
}
#endif

/* ROMFS VFS operations table */
static const gui_vfs_ops_t sd_vfs_ops =
{
    .open = zephyr_vfs_open,
    .close = zephyr_vfs_close,
    .read = zephyr_vfs_read,
    .write = zephyr_vfs_write,
    .seek = zephyr_vfs_seek,
    .tell = zephyr_vfs_tell,
    .get_addr = NULL,
    .opendir = NULL,
    .readdir = NULL,
    .closedir = NULL,
    .stat = NULL,
};


int gui_vfs_mount_fs(const char *prefix)
{
    int ret = gui_vfs_mount(prefix, &sd_vfs_ops, NULL);
    if (ret == 0)
    {
        gui_log("[VFS] app fs mounted at %s\n", prefix);
    }
    return ret;
}

#if (CONFIG_APP_NANDBOOT == 1)

#define RES_FLASH_COPY_CHUNK    4096

/* res.bin is programmed into the userdata1 flash region. Depending on how it
 * was flashed it may start at the region base (raw pack) or behind an image
 * header (USER_DATA1_WITH_HEADER). Probe both and match on the pack magic. */
static bool res_flash_find_pack_base(uint32_t *pack_base)
{
    static const uint32_t probe_offset[] = { 0, IMG_HDR_SIZE };

    for (uint32_t i = 0; i < sizeof(probe_offset) / sizeof(probe_offset[0]); i++)
    {
        uint32_t addr = USER_DATA1_ADDR + probe_offset[i];
        uint32_t magic = 0;
        if (!fmc_flash_nand_read(addr, &magic, sizeof(magic)))
        {
            continue;
        }
        if (magic == PACK_MAGIC)
        {
            *pack_base = addr;
            return true;
        }
    }
    return false;
}

/* Rebuild /SD:/res.bin from the copy that ships in the userdata1 flash region.
 * Used the first time the filesystem has no res.bin. Returns 0 on success. */
static int res_recover_from_flash(void)
{
    uint32_t pack_base;
    if (!res_flash_find_pack_base(&pack_base))
    {
        gui_log("[VFS] res.bin not found in userdata1 flash");
        return -1;
    }

    PACK_HEADER hdr;
    if (!fmc_flash_nand_read(pack_base, &hdr, sizeof(hdr)))
    {
        gui_log("[VFS] read flash pack header fail");
        return -1;
    }

    /* Walk the index table to work out the total pack size (max end offset). */
    uint32_t entry_bytes = hdr.data_offset - sizeof(PACK_HEADER);
    PACK_ENTRY *entries = gui_malloc(entry_bytes);
    if (entries == NULL)
    {
        gui_log("[VFS] alloc pack index fail");
        return -1;
    }
    if (!fmc_flash_nand_read(pack_base + sizeof(PACK_HEADER), entries, entry_bytes))
    {
        gui_free(entries);
        gui_log("[VFS] read flash pack index fail");
        return -1;
    }

    uint32_t total_size = hdr.data_offset;
    for (uint32_t i = 0; i < hdr.entry_count; i++)
    {
        uint32_t end = entries[i].offset + entries[i].size;
        if (end > total_size)
        {
            total_size = end;
        }
    }
    gui_free(entries);

    uint8_t *buf = gui_malloc(RES_FLASH_COPY_CHUNK);
    if (buf == NULL)
    {
        gui_log("[VFS] alloc copy buffer fail");
        return -1;
    }

    /* Drop any stale/partial file, then create a fresh one. */
    fs_unlink(RES_BIN_PATH);

    struct fs_file_t dst;
    fs_file_t_init(&dst);
    int res = fs_open(&dst, RES_BIN_PATH, FS_O_CREATE | FS_O_WRITE);
    if (res)
    {
        gui_free(buf);
        gui_log("[VFS] create %s fail %d", RES_BIN_PATH, res);
        return -1;
    }

    int ret = 0;
    uint32_t copied = 0;
    while (copied < total_size)
    {
        uint32_t chunk = total_size - copied;
        if (chunk > RES_FLASH_COPY_CHUNK)
        {
            chunk = RES_FLASH_COPY_CHUNK;
        }
        if (!fmc_flash_nand_read(pack_base + copied, buf, chunk))
        {
            gui_log("[VFS] flash read fail at 0x%x", pack_base + copied);
            ret = -1;
            break;
        }
        ssize_t wr = fs_write(&dst, buf, chunk);
        if (wr != (ssize_t)chunk)
        {
            gui_log("[VFS] fs_write fail %d", (int)wr);
            ret = -1;
            break;
        }
        copied += chunk;
        WDG_Kick();
    }

    fs_close(&dst);
    gui_free(buf);

    if (ret != 0)
    {
        fs_unlink(RES_BIN_PATH);
        return ret;
    }

    gui_log("[VFS] res.bin recovered from userdata1 flash, size %d", total_size);
    return 0;
}
#endif /* CONFIG_APP_NANDBOOT */

void gui_port_fs_init(void)
{
    gui_vfs_mount_fs("/");

    res_file = os_mem_alloc(RAM_TYPE_DATA_ON, sizeof(struct fs_file_t));
    if (res_file == NULL)
    {
        return;
    }
    fs_file_t_init(res_file);
    int res = fs_open(res_file, RES_BIN_PATH, FS_O_READ);
#if (CONFIG_APP_NANDBOOT == 1)
    if (res)
    {
        /* No res.bin in the filesystem yet - restore it from the factory copy
         * programmed into the userdata1 flash region, then retry the open. */
        gui_log("[VFS] %s open fail %d, try recover from flash", RES_BIN_PATH, res);
        if (res_recover_from_flash() == 0)
        {
            fs_file_t_init(res_file);
            res = fs_open(res_file, RES_BIN_PATH, FS_O_READ);
        }
    }
#endif
    if (res)
    {
        fs_close(res_file);
        os_mem_free(res_file);
        return;
    }
    ssize_t read_cnt = fs_read(res_file, &res_pack_hdr, sizeof(PACK_HEADER));
    if (read_cnt != sizeof(PACK_HEADER))
    {
        return;
    }
    res_entry = gui_malloc(res_pack_hdr.data_offset - sizeof(PACK_HEADER));
    if (res_entry == NULL)
    {
        gui_log("entry null");
        return;
    }
    memset(res_entry, 0, res_pack_hdr.data_offset - sizeof(PACK_HEADER));
    fs_read(res_file, res_entry, res_pack_hdr.data_offset - sizeof(PACK_HEADER));

    return;
}
