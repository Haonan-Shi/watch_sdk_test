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
#include "mp3_parser.h"
#include "os_mem.h"
#include "trace.h"
#include <string.h>
#include "app_fs_if.h" //for filesystem path


#define ID3V1_BYTES         128
#define ID3V2_HEADER_SIZE   10

#define FRAME_NUM_FLAG_MASK             0x01
#define FILE_LEN_FLAG_MASK              0x02
#define TOC_FLAG_MASK                   0x04

#define EXTENDED_HEADER_FLAG_MASK       (1 << 6)

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define PRINT_ERROR(extra_msg) APP_PRINT_INFO2("Error! " extra_msg " File: mp3_parser.c. Func: %s. Line: %d", TRACE_STRING(__func__), __LINE__)


typedef struct
{
    char acHeader[3];
    uint8_t uVer;
    uint8_t uRevision;
    uint8_t uFlag;
    uint8_t auSize[4];
} __attribute__((packed)) SId3v2Hdr;

typedef struct
{
    uint8_t auExtendedHdrSize[4];
    uint8_t auExtendedFlags[2];
    uint8_t auSizeOfPadding[4];
} __attribute__((packed)) SId3v2ExtendedHdr;

typedef struct
{
    char acFrameId[4];
    uint8_t auSize[4];
    uint8_t auFlags[2];
} __attribute__((packed)) SId3v2FrmHdr;


static EMp3Res InitId3v1Info(Mp3Hdl_t hMp3);
static EMp3Res InitId3v2Info(Mp3Hdl_t hMp3);
static EMp3Res InitAudioFrameInfo(Mp3Hdl_t hMp3);
static inline bool IsFrameHeaderValid(const SMp3FrmHdr *pzHdrToCheck,
                                      const SMp3FrmHdr *pz1stAuFrmHdr);
static inline uint32_t GetSamplingFrequency_Hz(const SMp3FrmHdr *pzFrmHdr);
static inline uint32_t GetBitRate_kbps(const SMp3FrmHdr *pzFrmHdr);
static inline uint32_t GetSamplePerFrm(const SMp3FrmHdr *pzFrmHdr);
static inline uint32_t ComputeFrameBytes(const SMp3FrmHdr *pzFrmHdr);
static inline uint32_t GetTotalFrameBytes(Mp3Hdl_t hMp3);
static inline uint32_t XingInfoOfs(const SMp3FrmHdr *pzFrmHdr);
static inline uint32_t FlagOfs(const SMp3FrmHdr *pzFrmHdr);
static inline uint32_t FrmNumOfs(const SMp3FrmHdr *pzFrmHdr);
static inline uint32_t FileLenOfs(const SMp3FrmHdr *pzFrmHdr);
static inline uint32_t TocOfs(const SMp3FrmHdr *pzFrmHdr);
static inline bool IsCbr(Mp3Hdl_t hMp3);
static inline uint32_t GetBigEndianUint32From(const void *pvAddr);
static EMp3Res SearchFrameFrom(struct fs_file_t *pxFil, uint8_t *puBuf, uint32_t uBufLen,
                               uint32_t uStartOfs,
                               const SMp3FrmHdr *pz1stAuFrmHdr, uint32_t *puFoundOfs, uint32_t *puFrmBytes);
static int32_t SeekRead(struct fs_file_t *fp, size_t ofs, void *buff, uint32_t btr, uint32_t *br);



Mp3Hdl_t Mp3_CreateHandle(const char *pxFileName, EMp3Res *peRes)
{
    if (peRes == NULL)
    {
        ASSERT(peRes != NULL);
        return NULL;
    }

    *peRes = MP3RES_OK;
    int32_t eFr = 0;

    char *full_file_path;
    char *dir_path = (char *)AUDIO_FILE_PATH;
    uint16_t total_len = strlen(pxFileName) + strlen(dir_path) + 2; // for '/' and '\0'

    full_file_path = os_mem_zalloc(RAM_TYPE_DATA_ON, total_len);
    if (full_file_path == NULL)
    {
        *peRes = MP3RES_MALLOC_FAILED;
        goto Err1;
    }

    snprintf(full_file_path, total_len, "%s%s", dir_path, pxFileName);

    Mp3Hdl_t hMp3 = os_mem_alloc(OS_MEM_TYPE_DATA, sizeof(*hMp3));

    if (hMp3 == NULL)
    {
        *peRes = MP3RES_MALLOC_FAILED;
        goto Err1;
    }

    fs_file_t_init(&hMp3->xFil);
    eFr = fs_open(&hMp3->xFil, full_file_path, FS_O_READ);
    if (eFr != 0)
    {
        *peRes = MP3RES_FOPEN_ERROR;
        goto Err2;
    }

    *peRes = InitId3v1Info(hMp3);
    if (*peRes != MP3RES_OK)
    {
        goto Err3;
    }

    *peRes = InitId3v2Info(hMp3);
    if (*peRes != MP3RES_OK)
    {
        goto Err3;
    }

    *peRes = InitAudioFrameInfo(hMp3);
    if (*peRes != MP3RES_OK)
    {
        goto Err3;
    }

    os_mem_free(full_file_path);
    return hMp3;

Err3:
    fs_close(&hMp3->xFil);
Err2:
    os_mem_free(hMp3);
Err1:
    if (full_file_path != NULL)
    {
        os_mem_free(full_file_path);
    }

    APP_PRINT_ERROR3("Mp3_CreateHandle eFr %d peRes %d %s", eFr, *peRes, TRACE_STRING(pxFileName));
    return NULL;
}

EMp3Res Mp3_FreeHandle(Mp3Hdl_t hMp3)
{
    if (hMp3 == NULL)
    {
        return MP3RES_OK;
    }

    fs_close(&hMp3->xFil);
    os_mem_free(hMp3);

    return MP3RES_OK;
}

EMp3Ver Mp3_GetVersion(Mp3Hdl_t hMp3)
{
    return (EMp3Ver)hMp3->z1stAuFrmHdr.version;
}

EMp3Layer Mp3_GetLayer(Mp3Hdl_t hMp3)
{
    return (EMp3Layer)hMp3->z1stAuFrmHdr.layer;
}

uint32_t Mp3_GetSamplingFrequency_Hz(Mp3Hdl_t hMp3)
{
    return hMp3->uSamplingFreq_Hz;
}

uint32_t Mp3_GetBitRate_kbps(Mp3Hdl_t hMp3)
{
    return hMp3->uBitRate_kbps;
}

EMp3ChannelMode Mp3_GetChannelMode(Mp3Hdl_t hMp3)
{
    return (EMp3ChannelMode)hMp3->z1stAuFrmHdr.channel_mode;
}

float Mp3_GetTotalPlayTime_ms(Mp3Hdl_t hMp3)
{
    return hMp3->fTotalPlayTime_ms;
}

uint32_t Mp3_GetSamplePerFrame(Mp3Hdl_t hMp3)
{
    return hMp3->uSamplePerFrm;
}

float Mp3_GetTimePerFrame_ms(Mp3Hdl_t hMp3)
{
    return hMp3->fTimePerFrm_ms;
}

EMp3Res Mp3_ReadNextFrame(Mp3Hdl_t hMp3, uint8_t **ppuBuf, uint32_t *puLen, float *pfPlayPos_ms)
{
    if (hMp3 == NULL || ppuBuf == NULL || puLen == NULL || pfPlayPos_ms == NULL)
    {
        ASSERT(hMp3 != NULL && ppuBuf != NULL && puLen != NULL && pfPlayPos_ms != NULL);
        return MP3RES_OTHER_ERR;
    }

    EMp3Res eRes;
    int32_t eFr;
    uint32_t uBr;

    // Don't invoke SearchFrameFrom() straightly, since it will read too much data causing inefficient.
    eFr = SeekRead(&hMp3->xFil, hMp3->uReadOfs, hMp3->auBuf, hMp3->uMaxPossibleFrmBytes, &uBr);
    if (eFr != FR_OK)
    {
        return MP3RES_FLSEEK_OR_FREAD_ERROR;
    }
    else if (uBr < sizeof(SMp3FrmHdr))
    {
        return MP3RES_FILE_ENDS;
    }

    SMp3FrmHdr *pzFrmHdr = (SMp3FrmHdr *)hMp3->auBuf;
    if (!IsFrameHeaderValid(pzFrmHdr, &hMp3->z1stAuFrmHdr) || ComputeFrameBytes(pzFrmHdr) > uBr)
    {
        eRes = SearchFrameFrom(&hMp3->xFil, hMp3->auBuf, sizeof(hMp3->auBuf), hMp3->uReadOfs,
                               &hMp3->z1stAuFrmHdr, &hMp3->uReadOfs, puLen);
        if (eRes == MP3RES_SEARCH_FRAME_FAILED)
        {
            return MP3RES_FILE_ENDS;
        }
        else if (eRes != MP3RES_OK)
        {
            return eRes;
        }
    }

    *ppuBuf = hMp3->auBuf;
    *puLen = ComputeFrameBytes(pzFrmHdr);
    *pfPlayPos_ms = hMp3->fTimePerFrm_ms * hMp3->uReadSqnNum;

    ++hMp3->uReadSqnNum;
    hMp3->uReadOfs += *puLen;

    return MP3RES_OK;
}

EMp3Res Mp3_SetPlayPos(Mp3Hdl_t hMp3, float fPos_ms)
{
    if (hMp3->fTotalPlayTime_ms == 0.0f)
    {
        return MP3RES_NO_PLAY_TIME_INFO;
    }

    if (fPos_ms > hMp3->fTotalPlayTime_ms || fPos_ms < 0.0f)
    {
        return MP3RES_POS_BEYOND_END;
    }

    // Update hMp3->uReadOfs.
    if (IsCbr(hMp3))
    {
        uint32_t uOfsFrom1stFrame = fPos_ms / hMp3->fTotalPlayTime_ms * GetTotalFrameBytes(hMp3);
        hMp3->uReadOfs = hMp3->u1stAuFrmOfs + uOfsFrom1stFrame;

        uint32_t uFrmBytes;
        // Adjust hMp3->uReadOfs to the beginning of a frame.
        SearchFrameFrom(&hMp3->xFil, hMp3->auBuf, sizeof(hMp3->auBuf), hMp3->uReadOfs,
                        &hMp3->z1stAuFrmHdr, &hMp3->uReadOfs, &uFrmBytes);
    }
    else // VBR
    {
        if (!(hMp3->uFlag & TOC_FLAG_MASK))
        {
            return MP3RES_VBR_WITHOUT_TOC_INFO;
        }

        uint8_t uTocIdx = fPos_ms / hMp3->fTotalPlayTime_ms * TOC_SIZE;
        if (uTocIdx == TOC_SIZE)
        {
            // Set read offset to end to cause MP3RES_FILE_ENDS when Mp3_ReadNextFrame();
            if (fs_size(&hMp3->xFil, &hMp3->uReadOfs) != 0)
            {
                return MP3RES_FLSEEK_OR_FREAD_ERROR;
            }
        }
        else
        {
            uint32_t uOfsFrom1stFrame = hMp3->auTocTable[uTocIdx] / 256.0f * GetTotalFrameBytes(hMp3);
            hMp3->uReadOfs  = hMp3->u1stAuFrmOfs + uOfsFrom1stFrame;

            // Fine tune offset using Mp3_ReadNextFrame();
            float fGap = fPos_ms - uTocIdx / 100.0f * hMp3->fTotalPlayTime_ms;
            uint32_t uFrms = (fGap > 0.0f) ? fGap / hMp3->fTimePerFrm_ms : 0;
            DBG_DIRECT("uPos_ms: %f, uTocIdx: %u, auTocTable[uTocIdx]: %u, fGap: %f, uFrms: %u",
                       fPos_ms, uTocIdx, hMp3->auTocTable[uTocIdx], fGap, uFrms);
            while (uFrms--)
            {
                uint8_t *puBuf;
                uint32_t uLen;
                float fCurrPlayTime_ms;
                Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &fCurrPlayTime_ms);
            }
        }
    }
    hMp3->uReadSqnNum = fPos_ms / hMp3->fTotalPlayTime_ms * hMp3->uTotalFrmNum;

    return MP3RES_OK;
}

EMp3Res Mp3_ReadId3v1(Mp3Hdl_t hMp3, uint8_t **ppuBuf, uint32_t *puLen)
{
    int32_t eFr;
    uint32_t uBr;
    size_t uId3v1Ofs = 0;

    if (!hMp3->bIsId3v1Exist)
    {
        return MP3RES_ID3V1_NOT_EXIST;
    }

    eFr = fs_size(&hMp3->xFil, &uId3v1Ofs);
    if (eFr != 0)
    {
        return MP3RES_FLSEEK_OR_FREAD_ERROR;
    }
    uId3v1Ofs = uId3v1Ofs - ID3V1_BYTES;

    eFr = SeekRead(&hMp3->xFil, uId3v1Ofs, hMp3->auBuf, ID3V1_BYTES, &uBr);
    if (eFr != 0 || uBr != ID3V1_BYTES)
    {
        return MP3RES_FLSEEK_OR_FREAD_ERROR;
    }

    *ppuBuf = hMp3->auBuf;
    *puLen = ID3V1_BYTES;
    return MP3RES_OK;
}

EMp3Res Mp3_RetrieveId3v2Frame(Mp3Hdl_t hMp3, const char acFrmId[4], uint8_t **ppuBuf,
                               uint32_t *puLen)
{
    int32_t eFr;
    uint32_t uBr;

    if (hMp3->uId3v2Count == 0)
    {
        return MP3RES_ID3V2_NOT_EXIST;
    }

    uint32_t uId3v2Bytes;
    for (uint32_t uId3v2Ofs = 0;
         uId3v2Ofs < hMp3->u1stAuFrmOfs - sizeof(SId3v2Hdr);  // Can read an id3v2 header at least.
         uId3v2Ofs += uId3v2Bytes)
    {
        SId3v2Hdr zId3v2Hdr;
        eFr = SeekRead(&hMp3->xFil, uId3v2Ofs, &zId3v2Hdr, sizeof(SId3v2Hdr), &uBr);
        if (eFr != 0)
        {
            return MP3RES_FLSEEK_OR_FREAD_ERROR;
        }
        else if (uBr != sizeof(zId3v2Hdr))
        {
            return MP3RES_ID3V2_NOT_EXIST;
        }

        uId3v2Bytes = sizeof(SId3v2Hdr) +
                      (((zId3v2Hdr.auSize[0] & 0x7fUL) << 21) | ((zId3v2Hdr.auSize[1] & 0x7fUL) << 14) |
                       ((zId3v2Hdr.auSize[2] & 0x7fUL) << 7) | (zId3v2Hdr.auSize[3] & 0x7fUL));

        // Only search id3v2.3 and id3v2.4.
        if (zId3v2Hdr.uVer != 3 && zId3v2Hdr.uVer != 4)
        {
            continue;
        }

        uint32_t uExtendedHdrBytes;
        if (!(zId3v2Hdr.uFlag & EXTENDED_HEADER_FLAG_MASK))
        {
            uExtendedHdrBytes = 0;
        }
        else
        {
            SId3v2ExtendedHdr zExtHdr;
            eFr = SeekRead(&hMp3->xFil, uId3v2Ofs + sizeof(SId3v2Hdr), &zExtHdr, sizeof(zExtHdr), &uBr);
            if (eFr != 0 || uBr != sizeof(zExtHdr))
            {
                return MP3RES_FLSEEK_OR_FREAD_ERROR;
            }

            uExtendedHdrBytes = GetBigEndianUint32From(zExtHdr.auExtendedHdrSize)
                                + sizeof(zExtHdr.auExtendedHdrSize);
        }

        uint32_t uFrmBytes;
        for (uint32_t uFrmOfs = sizeof(SId3v2Hdr) + uExtendedHdrBytes;
             uFrmOfs < uId3v2Bytes - sizeof(SId3v2FrmHdr); // Can read a frame header at least.
             uFrmOfs += uFrmBytes)
        {
            uint32_t uFrmOfsFromBegin = uId3v2Ofs + uFrmOfs;

            SId3v2FrmHdr zFrmHdr;
            eFr = SeekRead(&hMp3->xFil, uFrmOfsFromBegin, &zFrmHdr, sizeof(SId3v2FrmHdr), &uBr);
            if (eFr != 0)
            {
                return MP3RES_FLSEEK_OR_FREAD_ERROR;
            }
            else if (uBr != sizeof(SId3v2FrmHdr))
            {
                return MP3RES_ID3V2_FRAME_NOT_EXIST;
            }

            uint32_t uSizeExcludingHdr = GetBigEndianUint32From(zFrmHdr.auSize);
            if (uSizeExcludingHdr == 0) // Left bytes are padding "00000000...".
            {
                break;    // Search next id3v2 tag.
            }

            uFrmBytes = uSizeExcludingHdr + sizeof(SId3v2FrmHdr);

            if (memcmp(zFrmHdr.acFrameId, acFrmId, 4) == 0) // Frame is found.
            {
                uint32_t uBtr = MIN(uFrmBytes, sizeof(hMp3->auBuf));
                eFr = SeekRead(&hMp3->xFil, uFrmOfsFromBegin, hMp3->auBuf, uBtr, &uBr);
                if (eFr != 0 || uBr != uBtr)
                {
                    return MP3RES_FLSEEK_OR_FREAD_ERROR;
                }

                *ppuBuf = hMp3->auBuf;
                *puLen = uBr;
                return MP3RES_OK;
            }
        }
    }

    return MP3RES_ID3V2_FRAME_NOT_EXIST;
}

static EMp3Res InitId3v1Info(Mp3Hdl_t hMp3)
{
    int32_t eFr;
    uint32_t uBr;
    size_t uSize;

    eFr = fs_size(&hMp3->xFil, &uSize);
    if (eFr != 0)
    {
        return MP3RES_FLSEEK_OR_FREAD_ERROR;
    }

    if (uSize < ID3V1_BYTES)
    {
        hMp3->bIsId3v1Exist = false;
        return MP3RES_OK;
    }

    char acHdr[3];
    uint8_t uId3v1Ofs = uSize - ID3V1_BYTES;
    eFr = SeekRead(&hMp3->xFil, uId3v1Ofs, acHdr, sizeof(acHdr), &uBr);
    if (eFr != 0 || uBr != sizeof(acHdr))
    {
        return MP3RES_FLSEEK_OR_FREAD_ERROR;
    }

    hMp3->bIsId3v1Exist = (memcmp(acHdr, "TAG", 3) == 0) ? true : false;
    return MP3RES_OK;
}

static EMp3Res InitId3v2Info(Mp3Hdl_t hMp3)
{
    int32_t eFr;
    uint32_t uBr;
    size_t uSize;

    eFr = fs_size(&hMp3->xFil, &uSize);
    if (eFr != 0)
    {
        return MP3RES_FLSEEK_OR_FREAD_ERROR;
    }

    if (uSize < ID3V2_HEADER_SIZE)
    {
        hMp3->uId3v2Count = 0;
        hMp3->u1stAuFrmOfs = 0;
        return MP3RES_OK;
    }

    // There may be more than 1 id3v2 tag.
    hMp3->uId3v2Count = 0;
    uint32_t uId3v2Bytes;
    uint32_t uOfs;
    for (uOfs = 0; uOfs < uSize; uOfs += uId3v2Bytes)
    {
        SId3v2Hdr zHdr;
        eFr = SeekRead(&hMp3->xFil, uOfs, &zHdr, sizeof(SId3v2Hdr), &uBr);
        if (eFr != 0 || uBr != sizeof(SId3v2Hdr))
        {
            return MP3RES_FLSEEK_OR_FREAD_ERROR;
        }

        if (memcmp(zHdr.acHeader, "ID3", 3) == 0)
        {
            ++hMp3->uId3v2Count;
        }
        else
        {
            break;
        }

        uId3v2Bytes = sizeof(SId3v2Hdr) +
                      (((zHdr.auSize[0] & 0x7fUL) << 21) | ((zHdr.auSize[1] & 0x7fUL) << 14) |
                       ((zHdr.auSize[2] & 0x7fUL) << 7) | (zHdr.auSize[3] & 0x7fUL));
    }
    hMp3->u1stAuFrmOfs = uOfs;

    return MP3RES_OK;
}

static EMp3Res InitAudioFrameInfo(Mp3Hdl_t hMp3)
{
    EMp3Res eRes;

    // 1. Get info from first frame and first audio frame.
    uint32_t uFrameBytes;
    // hMp3->u1stAuFrmOfs was initialed in InitId3v2Info().
    eRes = SearchFrameFrom(&hMp3->xFil, hMp3->auBuf, sizeof(hMp3->auBuf), hMp3->u1stAuFrmOfs,
                           NULL, &hMp3->u1stAuFrmOfs, &uFrameBytes);
    if (eRes != MP3RES_OK)
    {
        return eRes;
    }

    SMp3FrmHdr *pzHdr = (SMp3FrmHdr *)hMp3->auBuf;

    if (memcmp(&hMp3->auBuf[XingInfoOfs(pzHdr)], "Xing", 4) == 0)
    {
        hMp3->e1stFrmType = XING_FRAME;
    }
    else if (memcmp(&hMp3->auBuf[XingInfoOfs(pzHdr)], "Info", 4) == 0)
    {
        hMp3->e1stFrmType = INFO_FRAME;
    }
    else
    {
        hMp3->e1stFrmType = AUDIO_FRAME;
    }

    uint32_t uFrmNumInXing = 0;
    if (hMp3->e1stFrmType == XING_FRAME || hMp3->e1stFrmType == INFO_FRAME)
    {
        // Get info from Xing/Info frame.
        hMp3->uFlag = hMp3->auBuf[FlagOfs(pzHdr)];
        if (hMp3->uFlag & FILE_LEN_FLAG_MASK)
        {
            hMp3->uFileLen = GetBigEndianUint32From(&hMp3->auBuf[FileLenOfs(pzHdr)]);
        }

        if (hMp3->uFlag & TOC_FLAG_MASK)
        {
            memcpy(hMp3->auTocTable, &hMp3->auBuf[TocOfs(pzHdr)], TOC_SIZE);
        }

        if (hMp3->uFlag & FRAME_NUM_FLAG_MASK)
        {
            uFrmNumInXing = GetBigEndianUint32From(&hMp3->auBuf[FrmNumOfs(pzHdr)]) - 1;
        }
        else // Failed. If you really want to get it, you need to tranverse the file.
        {
            uFrmNumInXing = (uint32_t) - 1;
        }

        // Search 1st audio frame.
        hMp3->u1stAuFrmOfs += uFrameBytes;
        eRes = SearchFrameFrom(&hMp3->xFil, hMp3->auBuf, sizeof(hMp3->auBuf), hMp3->u1stAuFrmOfs,
                               NULL, &hMp3->u1stAuFrmOfs, &uFrameBytes);
        if (eRes != MP3RES_OK)
        {
            return eRes;
        }
    }

    memcpy(&hMp3->z1stAuFrmHdr, hMp3->auBuf, sizeof(hMp3->z1stAuFrmHdr));
    hMp3->uSamplePerFrm = GetSamplePerFrm(&hMp3->z1stAuFrmHdr);
    hMp3->uSamplingFreq_Hz = GetSamplingFrequency_Hz(&hMp3->z1stAuFrmHdr);
    hMp3->uBitRate_kbps = GetBitRate_kbps(&hMp3->z1stAuFrmHdr);
    hMp3->fTimePerFrm_ms = (float)hMp3->uSamplePerFrm / hMp3->uSamplingFreq_Hz * 1000;
    hMp3->fBytesPerFrm =
        (float)hMp3->uSamplePerFrm / hMp3->uSamplingFreq_Hz * hMp3->uBitRate_kbps / 8 * 1000;

    hMp3->uTotalFrmNum = IsCbr(hMp3) ? (GetTotalFrameBytes(hMp3) / hMp3->fBytesPerFrm) : uFrmNumInXing;

    hMp3->uMaxPossibleFrmBytes = IsCbr(hMp3) ?
                                 (uint32_t)hMp3->fBytesPerFrm + 1 :
                                 (uint32_t)((float)hMp3->uSamplePerFrm / hMp3->uSamplingFreq_Hz * 320 / 8 * 1000) + 1;

    hMp3->uReadOfs = hMp3->u1stAuFrmOfs;
    hMp3->uReadSqnNum = 0;

    hMp3->fTotalPlayTime_ms = (hMp3->uTotalFrmNum == (uint32_t) - 1) ?
                              0.0f :
                              hMp3->fTimePerFrm_ms * hMp3->uTotalFrmNum;

    return MP3RES_OK;
}

static inline bool IsFrameHeaderValid(const SMp3FrmHdr *pzHdrToCheck,
                                      const SMp3FrmHdr *pz1stAuFrmHdr)
{
    if (pzHdrToCheck->sync1 == 0xff && pzHdrToCheck->sync2 == 0x7 &&
        pzHdrToCheck->version != VERSION_NONE &&
        pzHdrToCheck->layer == LAYER_3 &&
        pzHdrToCheck->bit_rate_index != 0xf && pzHdrToCheck->bit_rate_index != 0x0 &&
        // Free bitrate is not supported.
        pzHdrToCheck->sample_rate_index != 0x3 &&
        pzHdrToCheck->emphasis != 0x2)
    {
        if (pz1stAuFrmHdr != NULL) // Check consistency.
        {
            return pzHdrToCheck->version == pz1stAuFrmHdr->version &&
                   pzHdrToCheck->sample_rate_index == pz1stAuFrmHdr->sample_rate_index &&
                   pzHdrToCheck->channel_mode == pz1stAuFrmHdr->channel_mode;
        }
        else
        {
            return true;
        }
    }
    else
    {
        return false;
    }
}

static inline uint32_t XingInfoOfs(const SMp3FrmHdr *pzFrmHdr)
{
    uint32_t uOfs;
    if (pzFrmHdr->version == MPEG_1 && pzFrmHdr->channel_mode != CHANNEL_SINGLE)
    {
        uOfs = 36;
    }
    else if (pzFrmHdr->version == MPEG_2 && pzFrmHdr->channel_mode == CHANNEL_SINGLE)
    {
        uOfs = 13;
    }
    else
    {
        uOfs = 21;
    }
    return uOfs;
}

static inline uint32_t FlagOfs(const SMp3FrmHdr *pzFrmHdr)
{
    return XingInfoOfs(pzFrmHdr) + 4 + 3;
}

static inline uint32_t FrmNumOfs(const SMp3FrmHdr *pzFrmHdr)
{
    return XingInfoOfs(pzFrmHdr) + 8;
}

static inline uint32_t FileLenOfs(const SMp3FrmHdr *pzFrmHdr)
{
    return XingInfoOfs(pzFrmHdr) + 12;
}

static inline uint32_t TocOfs(const SMp3FrmHdr *pzFrmHdr)
{
    return XingInfoOfs(pzFrmHdr) + 16;
}

static inline uint32_t GetSamplingFrequency_Hz(const SMp3FrmHdr *pzFrmHdr)
{
#define BAD ((uint32_t)-1)
    const uint32_t auSamplingFreq_Hz[4][4] =
    {
        {11025, 12000, 8000,  BAD},
        {BAD,   BAD,   BAD,   BAD},
        {22050, 24000, 16000, BAD},
        {44100, 48000, 32000, BAD},
    };
#undef BAD
    return auSamplingFreq_Hz[pzFrmHdr->version][pzFrmHdr->sample_rate_index];
}

static inline uint32_t GetBitRate_kbps(const SMp3FrmHdr *pzFrmHdr)
{
    // ASSERT(pzFrmHdr->layer == LAYER_3);
#define BAD ((uint32_t)-1)
    const uint32_t auBitRateV1[16] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, BAD};
    const uint32_t auBitRateV2[16] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160, BAD};

    switch (pzFrmHdr->version)
    {
    case MPEG_1:
        return auBitRateV1[pzFrmHdr->bit_rate_index];
    case MPEG_2:
    case MPEG_2P5:
        return auBitRateV2[pzFrmHdr->bit_rate_index];
    default:
        return BAD;
    }
#undef BAD
}

static inline uint32_t GetSamplePerFrm(const SMp3FrmHdr *pzFrmHdr)
{
    if (pzFrmHdr->layer != LAYER_3)
    {
        ASSERT(pzFrmHdr->layer == LAYER_3);
        return 0;
    }

    const uint32_t auSampleTable[4] = {576, (uint32_t) - 1, 576, 1152};
    return auSampleTable[pzFrmHdr->version];
}

static inline uint32_t ComputeFrameBytes(const SMp3FrmHdr *pzFrmHdr)
{
    // Don't adjust the order of calculation, or may cause unexpected round off. Eg: 720 -> 719.999936.
    return (uint32_t)
           ((float)GetSamplePerFrm(pzFrmHdr) * (1000.0f / 8.0f) / GetSamplingFrequency_Hz(
                pzFrmHdr) * GetBitRate_kbps(pzFrmHdr))
           + pzFrmHdr->padding;
}

static inline uint32_t GetBigEndianUint32From(const void *pvAddr)
{
    const uint8_t *pu = pvAddr;
    return (pu[0] << 24UL) | (pu[1] << 16UL) | (pu[2] << 8UL) | pu[3];
}

// If search success, frame data will be in puBuf,
// frame offset will be in *puFoundOfs, and length of frame data will be in *puFrmBytes.
static EMp3Res SearchFrameFrom(struct fs_file_t *pxFil, uint8_t *puBuf, uint32_t uBufLen,
                               uint32_t uStartOfs,
                               const SMp3FrmHdr *pz1stAuFrmHdr, uint32_t *puFoundOfs, uint32_t *puFrmBytes)
{
    int32_t eFr;
    uint32_t uBr;

    eFr = SeekRead(pxFil, uStartOfs, puBuf, uBufLen, &uBr);
    if (eFr != FR_OK)
    {
        return MP3RES_FLSEEK_OR_FREAD_ERROR;
    }
    else if (uBr < sizeof(SMp3FrmHdr))
    {
        return MP3RES_FILE_ENDS;
    }

    uint32_t i, uMaxSearchTime = uBr - sizeof(SMp3FrmHdr) + 1;
    for (i = 0; i < uMaxSearchTime; ++i)
    {
        // ?? : Does unaligned access works well?
        SMp3FrmHdr *pzHdr = (SMp3FrmHdr *)(puBuf + i);
        if (IsFrameHeaderValid(pzHdr, pz1stAuFrmHdr))
        {
            break;
        }
    }
    if (i == uMaxSearchTime)
    {
        return MP3RES_SEARCH_FRAME_FAILED;
    }

    *puFoundOfs = uStartOfs + i;
    *puFrmBytes = ComputeFrameBytes((SMp3FrmHdr *)(puBuf + i));

    if (i != 0 || uBr < *puFrmBytes)
    {
        eFr = SeekRead(pxFil, *puFoundOfs, puBuf, *puFrmBytes, &uBr);
        if (eFr != FR_OK || uBr != *puFrmBytes)
        {
            return MP3RES_FLSEEK_OR_FREAD_ERROR;
        }
    }

    return MP3RES_OK;
}

// Must parse id3v2 and id3v1 in advance.
static inline uint32_t GetTotalFrameBytes(Mp3Hdl_t hMp3)
{
    int32_t eFr;
    size_t uSize;
    eFr = fs_size(&hMp3->xFil, &uSize);
    if (eFr != 0)
    {
        return MP3RES_FLSEEK_OR_FREAD_ERROR;
    }

    return uSize
           - (hMp3->bIsId3v1Exist ? ID3V1_BYTES : 0) // Id3v1 bytes.
           - hMp3->u1stAuFrmOfs; // Id3v2 bytes;
}

static int32_t SeekRead(struct fs_file_t *fp, size_t ofs, void *buff, uint32_t btr, uint32_t *br)
{
    int32_t eFr;
    ssize_t size;

    eFr = fs_seek(fp, ofs, FS_SEEK_SET);
    if (eFr != 0)
    {
        return eFr;
    }

    size = fs_read(fp, buff, btr);
    if (size < 0)
    {
        return size;
    }
    else
    {
        *br = size;
    }

    return 0;
}

static inline bool IsCbr(Mp3Hdl_t hMp3)
{
    return (hMp3->e1stFrmType == XING_FRAME) ? false : true;
}

void Mp3_Test(const TCHAR *pxFileName)
{
    EMp3Res eRes;

    DBG_DIRECT("!!!!!! void Mp3_Test(const TCHAR *pxFileName) !!!!!!");


    Mp3Hdl_t hMp3 = Mp3_CreateHandle(pxFileName, &eRes);
    DBG_DIRECT("Mp3Hdl_t hMp3 = Mp3_CreateHandle(pxFileName, &eRes);""%d", eRes);
    DBG_DIRECT("%d, %d, %d, %d", eRes, MP3_BUF_BYTES, hMp3->bIsId3v1Exist, hMp3->uId3v2Count);
    DBG_DIRECT("0x%08x", *(uint32_t *)&hMp3->z1stAuFrmHdr);
    DBG_DIRECT("%d, %d", hMp3->uSamplingFreq_Hz, hMp3->uBitRate_kbps);
    DBG_DIRECT("%f, %f, %u, %f", hMp3->fBytesPerFrm, hMp3->fTimePerFrm_ms, hMp3->uTotalFrmNum,
               hMp3->fTotalPlayTime_ms);
    DBG_DIRECT("%d, 0x%x, %d", hMp3->uMaxPossibleFrmBytes, hMp3->u1stAuFrmOfs, hMp3->e1stFrmType);
    DBG_DIRECT("0x%x, %d", hMp3->uReadOfs, hMp3->uReadSqnNum);
    DBG_DIRECT("0x%x, 0x%x", hMp3->uFlag, hMp3->uFileLen);


    uint8_t *puBuf;
    uint32_t uLen;
    float fCurrPlayTime_ms;
    eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &fCurrPlayTime_ms);
    DBG_DIRECT("eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &uCurrPlayTime_ms);""%d", eRes);
    DBG_DIRECT("%d, %d, %f", eRes, uLen, fCurrPlayTime_ms);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);

    eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &fCurrPlayTime_ms);
    DBG_DIRECT("eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &uCurrPlayTime_ms);""%d", eRes);
    DBG_DIRECT("%d, %d, %f", eRes, uLen, fCurrPlayTime_ms);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);


    eRes = Mp3_SetPlayPos(hMp3, hMp3->fTotalPlayTime_ms / 2.0f);
    DBG_DIRECT("eRes = Mp3_SetPlayPos(hMp3, hMp3->fTotalPlayTime_ms / 2.0f);""%d", eRes);
    eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &fCurrPlayTime_ms);
    DBG_DIRECT("eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &uCurrPlayTime_ms);""%d", eRes);
    DBG_DIRECT("%d, %d, %f", eRes, uLen, fCurrPlayTime_ms);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);


    eRes = Mp3_SetPlayPos(hMp3, hMp3->fTotalPlayTime_ms);
    DBG_DIRECT("eRes = Mp3_SetPlayPos(hMp3, hMp3->fTotalPlayTime_ms);""%d", eRes);
    eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &fCurrPlayTime_ms);
    DBG_DIRECT("eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &uCurrPlayTime_ms);""%d", eRes);
    DBG_DIRECT("%d, %d, %f", eRes, uLen, fCurrPlayTime_ms);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);

    for (uint32_t i = 0; i < 25; ++i)
    {
        eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &fCurrPlayTime_ms);
        DBG_DIRECT("eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &uCurrPlayTime_ms);""%d", eRes);
        DBG_DIRECT("%d, %d, %f", eRes, uLen, fCurrPlayTime_ms);
        DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
                   puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);
    }

    eRes = Mp3_SetPlayPos(hMp3, 20 * 1000);
    DBG_DIRECT("eRes = Mp3_SetPlayPos(hMp3, 20 * 1000);""%d", eRes);
    eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &fCurrPlayTime_ms);
    DBG_DIRECT("eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &uCurrPlayTime_ms);""%d", eRes);
    DBG_DIRECT("%d, %d, %f", eRes, uLen, fCurrPlayTime_ms);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);

    eRes = Mp3_SetPlayPos(hMp3, 0);
    DBG_DIRECT("eRes = Mp3_SetPlayPos(hMp3, 0);""%d", eRes);
    eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &fCurrPlayTime_ms);
    DBG_DIRECT("eRes = Mp3_ReadNextFrame(hMp3, &puBuf, &uLen, &uCurrPlayTime_ms);""%d", eRes);
    DBG_DIRECT("%d, %d, %f", eRes, uLen, fCurrPlayTime_ms);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);


    eRes = Mp3_ReadId3v1(hMp3, &puBuf, &uLen);
    DBG_DIRECT("eRes = Mp3_ReadId3v1(hMp3, &puBuf, &uLen);""%d", eRes);
    DBG_DIRECT("%d, %d", eRes, uLen);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);


    eRes = Mp3_RetrieveId3v2Frame(hMp3, "TIT2", &puBuf, &uLen);
    DBG_DIRECT("eRes = Mp3_RetrieveId3v2Frame(hMp3, \"TIT2\", &puBuf, &uLen);""%d", eRes);
    DBG_DIRECT("%d, %d", eRes, uLen);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);

    eRes = Mp3_RetrieveId3v2Frame(hMp3, "TPE1", &puBuf, &uLen);
    DBG_DIRECT("eRes = Mp3_RetrieveId3v2Frame(hMp3, \"TPE1\", &puBuf, &uLen);""%d", eRes);
    DBG_DIRECT("%d, %d", eRes, uLen);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);

    eRes = Mp3_RetrieveId3v2Frame(hMp3, "TSSE", &puBuf, &uLen);
    DBG_DIRECT("eRes = Mp3_RetrieveId3v2Frame(hMp3, \"TSSE\", &puBuf, &uLen);""%d", eRes);
    DBG_DIRECT("%d, %d", eRes, uLen);
    DBG_DIRECT("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x",
               puBuf[0], puBuf[1], puBuf[2], puBuf[uLen - 4], puBuf[uLen - 3], puBuf[uLen - 2], puBuf[uLen - 1]);

    Mp3_FreeHandle(hMp3);
}
