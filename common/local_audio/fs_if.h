/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
  *                   Define to prevent recursive inclusion
  *============================================================================*/
#ifndef _FS_IF_H_
#define _FS_IF_H_

/*============================================================================*
  *                               Header Files
  *============================================================================*/
#include "rtl876x.h"
#include "ff.h"
#include "diskio.h"
#include "os_mem.h"
/*============================================================================*
  *                                  Variables
  *============================================================================*/
#define FS_HEADER_COUNT_OFFSET           0
#define FS_HEADER_COUNT_SIZE             sizeof(uint16_t)
#define FS_HEADER_RSV_SIZE               sizeof(uint32_t)
#define FS_HEADER_INFO_START             (FS_HEADER_COUNT_SIZE + FS_HEADER_RSV_SIZE)

typedef struct
{
    uint8_t file_ext_def; //start from 0x00
    TCHAR   *big_letters;
    TCHAR   *small_letters;
} T_EXTENSION_DEF;

typedef struct
{
    uint8_t file_ext_num;
    const T_EXTENSION_DEF *file_ext;
} T_FILE_EXTENSION;

typedef struct
{
    const TCHAR    *file_path;
    FIL      *header_fil;
    FIL      *name_fil;
    const T_FILE_EXTENSION *scan_ext;
    bool     hdr_name_open;
} T_FILE_SCAN_HANDLE;

typedef struct
{
    const TCHAR    *disk_path;
    FATFS    *fs;
} T_FS_HANDLE;

typedef struct
{
    uint32_t    offset;    //Start offset of the song name
    uint16_t    length;       //Length of the song name
    uint16_t    plIndex;    /*Play List Index, indicate which playlist the song belongs to.
                             0x0: belong to no playlist, 0x1: belong to playlist1.*/
    uint8_t     isDeleted : 1;  /* flag of if song is deleted.
                                    1: deleted,
                                    0: not deleted, */
    uint8_t     needToUnlink : 1;  /* flag of if song need to unlink.
                                      1: need to call audio_fs_unlink to delete the file
                                      2: not need to unlink*/
    uint8_t     extension : 6;
    uint8_t     fil_ext;            /* file extension*/
    uint8_t     rsv;                /* Reserve for future usage, should set to "0" */
} __attribute__((packed)) T_HEAD_INFO;

typedef struct
{
    T_HEAD_INFO hdr_info;
    TCHAR    name[FF_LFN_BUF + 1];
} T_HEAD_NAME;

typedef struct
{
    FIL               fil;
    TCHAR             filename[FF_MAX_LFN];
    uint16_t          namelen;
} T_FILE_HANDLE;

enum
{
    FILE_FORMAT_MP3,
    FILE_FORMAT_MP4,
    FILE_FORMAT_RTK,
    FILE_FORMAT_AAC,
    FILE_FORMAT_FLAC,
    FILE_FORMAT_TXT,
    FILE_FORMAT_DAT,
    FILE_FORMAT_BIN,
} __attribute__((packed));
typedef uint8_t T_FILE_FORMAT;

/*============================================================================*
  *                                Functions
  *============================================================================*/
void fs_set_mem_alloc_type(T_OS_MEM_TYPE ram_type);

FRESULT seek_read(FIL *fp, FSIZE_t ofs, void *buff, UINT btr, UINT *br);

FRESULT seek_write(FIL *fp, FSIZE_t ofs, void *buff, UINT btw, UINT *bw);

int fs_read_header_bin_count(uint16_t *cnt, T_FILE_SCAN_HANDLE *fil_scan_hdl);

int fs_file_add_to_header_name_bin(T_FILE_HANDLE *fil_hdl, T_FILE_SCAN_HANDLE *fil_scan_hdl);

int fs_mark_delete_file_to_head_bin(const TCHAR *file_name, T_FILE_SCAN_HANDLE *fil_scan_hdl,
                                    uint16_t file_idx);

int fs_set_playlist_info_to_head_bin(const TCHAR *file_name, T_FILE_SCAN_HANDLE *fil_scan_hdl,
                                     uint16_t playlist_idx, uint16_t file_idx);

int fs_scan_file_list(T_FILE_SCAN_HANDLE *fil_scan_hdl, bool *file_list_change);

int fs_delete_scan_handle(T_FILE_SCAN_HANDLE *fil_scan_hdl);

int fs_create_scan_handle(const TCHAR *fil_path, const T_FILE_EXTENSION *scan_ext,
                          T_FILE_SCAN_HANDLE *fil_scan_hdl);

int fs_unlink_all_files(T_FILE_SCAN_HANDLE *fil_scan_hdl);

int fs_delete_all_files_by_format(T_FILE_FORMAT format, T_FILE_SCAN_HANDLE *fil_scan_hdl);

int fs_file_update_header_bin(T_FILE_HANDLE *fil_hdl, T_FILE_SCAN_HANDLE *fil_scan_hdl);

int fs_deinit(T_FS_HANDLE *fs_hdl);

int fs_init(const TCHAR *disk_path, T_FS_HANDLE *fs_hdl);

void sd_power_off(void);
void sd_power_on(void);

#endif //_FS_IF_H_
