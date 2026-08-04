/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*Copy this file as "lv_port_fs.c" and set this value to "1" to enable content*/
#if 1

/*********************
 *      INCLUDES
 *********************/
#include <lvgl.h>
#include <zephyr/fs/fs.h>
#include <stdio.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct
{
    struct fs_file_t file;
} lvgl_fs_zephyr_file_t;

typedef struct
{
    struct fs_dir_t dir;
    struct fs_dirent entry;
} lvgl_fs_zephyr_dir_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);

static void *lvgl_fs_zephyr_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t lvgl_fs_zephyr_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t lvgl_fs_zephyr_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr,
                                       uint32_t *br);
static lv_fs_res_t lvgl_fs_zephyr_write(lv_fs_drv_t *drv, void *file_p, const void *buf,
                                        uint32_t btw,
                                        uint32_t  *bw);
static lv_fs_res_t lvgl_fs_zephyr_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos,
                                       lv_fs_whence_t whence);
static lv_fs_res_t lvgl_fs_zephyr_size(lv_fs_drv_t *drv, void *file_p, uint32_t *size_p);
static lv_fs_res_t lvgl_fs_zephyr_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);

static void *lvgl_fs_zephyr_dir_open(lv_fs_drv_t *drv, const char *path);
static lv_fs_res_t lvgl_fs_zephyr_dir_read(lv_fs_drv_t *drv, void *dir_p, char *fn,
                                           uint32_t fn_len);
static lv_fs_res_t lvgl_fs_zephyr_dir_close(lv_fs_drv_t *drv, void *rddir_p);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_fs_init(void)
{
    /*----------------------------------------------------
     * Initialize your storage device and File System
     * -------------------------------------------------*/
    // fs_init();

    /*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

    static lv_fs_drv_t drv;
    lv_fs_drv_init(&drv);
    drv.letter = '/';
    drv.open_cb = lvgl_fs_zephyr_open;
    drv.close_cb = lvgl_fs_zephyr_close;
    drv.read_cb = lvgl_fs_zephyr_read;
    drv.write_cb = lvgl_fs_zephyr_write;
    drv.seek_cb = lvgl_fs_zephyr_seek;
    drv.tell_cb = lvgl_fs_zephyr_tell;
    drv.dir_open_cb = lvgl_fs_zephyr_dir_open;
    drv.dir_read_cb = lvgl_fs_zephyr_dir_read;
    drv.dir_close_cb = lvgl_fs_zephyr_dir_close;


    lv_fs_drv_register(&drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/*Initialize your Storage device and File system.*/
static void fs_init(void)
{
    /*E.g. for FatFS initialize the SD card and FatFS itself*/

    /*You code here*/
}

/**
 * Open a file
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode      read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return          a file descriptor or NULL on error
 */
static void *lvgl_fs_zephyr_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    lv_fs_res_t res = LV_FS_RES_NOT_IMP;

    char full_path[128];
    snprintf(full_path, sizeof(full_path), "/%s", path);

    // LV_LOG_WARN("lvgl_fs_zephyr_open full_path %s", full_path);

    lvgl_fs_zephyr_file_t *file = lv_malloc(sizeof(lvgl_fs_zephyr_file_t));
    if (!file)
    {
        return NULL;
    }

    fs_file_t_init(&file->file);

    int flags = 0;
    if (mode & LV_FS_MODE_WR)
    {
        flags = FS_O_CREATE | FS_O_WRITE;
    }
    if (mode & LV_FS_MODE_RD)
    {
        flags |= FS_O_READ;
    }
    int rc = fs_open(&file->file, full_path, flags);
    if (rc < 0)
    {
        lv_free(file);
        LV_LOG_ERROR("lvgl_fs_zephyr_open fail rc %d path: %s", rc, full_path);
        return NULL;
    }
    return file;
}

/**
 * Close an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t lvgl_fs_zephyr_close(lv_fs_drv_t *drv, void *file_p)
{
    lvgl_fs_zephyr_file_t *file = file_p;
    int rc = fs_close(&file->file);
    if (rc < 0)
    {
        LV_LOG_ERROR("lvgl_fs_zephyr_close fail rc %d", rc);
        return -rc;
    }
    lv_free(file);
    return LV_FS_RES_OK;
}

/**
 * Read data from an opened file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable.
 * @param buf       pointer to a memory block where to store the read data
 * @param btr       number of Bytes To Read
 * @param br        the real number of read bytes (Byte Read)
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t lvgl_fs_zephyr_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr,
                                       uint32_t *br)
{
    lvgl_fs_zephyr_file_t *file = file_p;
    ssize_t read_bytes = fs_read(&file->file, buf, btr);
    if (read_bytes < 0)
    {
        *br = 0;
        LV_LOG_ERROR("lvgl_fs_zephyr_read fail rc %d", read_bytes);
        return LV_FS_RES_UNKNOWN;
    }
    *br = read_bytes;
    return LV_FS_RES_OK;
}

/**
 * Write into a file
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param buf       pointer to a buffer with the bytes to write
 * @param btw       Bytes To Write
 * @param bw        the number of real written bytes (Bytes Written). NULL if unused.
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t lvgl_fs_zephyr_write(lv_fs_drv_t *drv, void *file_p, const void *buf,
                                        uint32_t btw,
                                        uint32_t  *bw)
{
    lvgl_fs_zephyr_file_t *file = file_p;
    int size = fs_write(&file->file, buf, btw);
    if (size < 0)
    {
        *bw = 0;
        LV_LOG_ERROR("lvgl_fs_zephyr_write fail rc %d btw %d", size, btw);
        return LV_FS_RES_UNKNOWN;
    }
    *bw = size;
    return LV_FS_RES_OK;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable. (opened with fs_open )
 * @param pos       the new position of read write pointer
 * @param whence    tells from where to interpret the `pos`. See @lv_fs_whence_t
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t lvgl_fs_zephyr_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos,
                                       lv_fs_whence_t whence)
{
    lvgl_fs_zephyr_file_t *file = file_p;
    int origin = 0;
    switch (whence)
    {
    case LV_FS_SEEK_SET: origin = FS_SEEK_SET; break;
    case LV_FS_SEEK_CUR: origin = FS_SEEK_CUR; break;
    case LV_FS_SEEK_END: origin = FS_SEEK_END; break;
    default: return LV_FS_RES_INV_PARAM;
    }
    int rc = fs_seek(&file->file, pos, origin);
    if (rc < 0)
    {
        LV_LOG_ERROR("lvgl_fs_zephyr_seek fail rc %d, pos %d, origin %d", rc, pos, origin);
        return LV_FS_RES_UNKNOWN;
    }
    return LV_FS_RES_OK;
}
/**
 * Give the position of the read write pointer
 * @param drv       pointer to a driver where this function belongs
 * @param file_p    pointer to a file_t variable
 * @param pos_p     pointer to store the result
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t lvgl_fs_zephyr_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    lv_fs_res_t res = LV_FS_RES_OK;
    lvgl_fs_zephyr_file_t *file = file_p;
    off_t offset = fs_tell(&file->file);
    if (offset < 0)
    {
        LV_LOG_ERROR("lvgl_fs_zephyr_tell fail rc %ld", offset);
        return LV_FS_RES_UNKNOWN;
    }
    *pos_p = offset;
    return LV_FS_RES_OK;
}

/**
 * Initialize a 'lv_fs_dir_t' variable for directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param path      path to a directory
 * @return          pointer to the directory read descriptor or NULL on error
 */
static void *lvgl_fs_zephyr_dir_open(lv_fs_drv_t *drv, const char *path)
{
    char full_path[128];
    snprintf(full_path, sizeof(full_path), "/%s", path);

    LV_LOG_WARN("lvgl_fs_zephyr_dir_open full_path %s", full_path);

    lvgl_fs_zephyr_dir_t *dir = lv_malloc(sizeof(lvgl_fs_zephyr_dir_t));
    if (!dir) { return NULL; }
    fs_dir_t_init(&dir->dir);

    int rc = fs_opendir(&dir->dir, full_path);
    if (rc < 0)
    {
        lv_free(dir);
        LV_LOG_ERROR("lvgl_fs_zephyr_dir_open fail rc %d, path: %s", rc, path);
        return NULL;
    }
    return dir;
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @param fn        pointer to a buffer to store the filename
 * @param fn_len    length of the buffer to store the filename
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t lvgl_fs_zephyr_dir_read(lv_fs_drv_t *drv, void *dir_p, char *fn, uint32_t fn_len)
{
    lvgl_fs_zephyr_dir_t *dir = dir_p;
    memset(&dir->entry, 0, sizeof(struct fs_dirent));
    int rc = fs_readdir(&dir->dir, &dir->entry);

    if (rc < 0)
    {
        LV_LOG_ERROR("lvgl_fs_zephyr_dir_read fail rc %d", rc);
        return LV_FS_RES_UNKNOWN;
    }
    if (dir->entry.name[0] == 0)
    {
        fn[0] = '\0';
        return LV_FS_RES_OK;
    }

    strcpy(fn, dir->entry.name);

    return LV_FS_RES_OK;
}

/**
 * Close the directory reading
 * @param drv       pointer to a driver where this function belongs
 * @param rddir_p   pointer to an initialized 'lv_fs_dir_t' variable
 * @return          LV_FS_RES_OK: no error or  any error from @lv_fs_res_t enum
 */
static lv_fs_res_t lvgl_fs_zephyr_dir_close(lv_fs_drv_t *drv, void *dir_p)
{
    lvgl_fs_zephyr_dir_t *dir = dir_p;
    int rc = fs_closedir(&dir->dir);
    if (rc < 0)
    {
        LV_LOG_ERROR("lvgl_fs_zephyr_dir_read fail rc %d", rc);
        return -rc;
    }
    lv_free(dir);
    return LV_FS_RES_OK;
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
