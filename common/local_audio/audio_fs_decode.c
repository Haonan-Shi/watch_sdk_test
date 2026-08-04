/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*============================================================================*
  *                                  Header Files
  *============================================================================*/
#include "audio_fs_decode.h"
#include "audio_fs.h"

FRAME_CONTENT   g_frameContent;
FRAME_INFO      g_frameInfo;

ID3V2_TAG_INFO tagInfo[ALL_TAGS];

#if (AAC_FORMAT_SUPPORT == 1)
extern ID3V2_INFO  aacId3v2Info;
#endif
#if (MP3_FORMAT_SUPPORT == 1)
#define     LARGEST_TAG_INFO_LEN    (100)
extern      ID3V2_INFO  mp3Id3v2Info;
uint16_t    findMp3HeaderTimes = 0;
#endif
#if (MP4_FORMAT_SUPPORT == 1)
MP4_HEADER_INFO g_mp4HeaderInfo;
#endif

const uint8_t g_file_header[10] = {0x49, 0x44, 0x33, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2B};
const uint8_t g_attr0_header[10] = {0x54, 0x46, 0x4C, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
const uint8_t g_attr1_header[10] = {0x54, 0x53, 0x49, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08};
const uint8_t g_latm_fheader[9] = {0x47, 0xFC, 0x00, 0x00, 0xB0, 0x90, 0x80, 0x03, 0x00};
//uint8_t g_sbc_attr0_data[5] = {0x53, 0x42, 0x43, 0x00, 0x00};
//uint8_t g_latm_attr0_data[5] = {0x4C, 0x41, 0x54, 0x4D, 0x00};
//uint32_t g_attr1_data[2] = {0x00000000, 0x00000000};
//uint8_t g_rtk_pkt_header[4] = {0xFF, 0x00, 0x00, 0x00};
//uint8_t g_sbc_fheader[4] = {0x9C, 0x00, 0x00, 0x00};


extern uint32_t aac_decode_id3v2_header(uint8_t *id3v2Header, uint16_t id3v2HeaderLen);
extern uint32_t aac_decode_id3v1_header(uint8_t *id3v1Header, uint16_t id3v1HeaderLen);
extern uint32_t aac_decode_aac_frame(uint8_t *aacHeader, uint16_t aacHeaderLen);
extern uint32_t aac_decode_judge_frame_header(uint8_t *frameHeader, uint16_t frameHeadeLen);
extern uint32_t mp3_decode_id3v2_header(uint8_t *id3v2Header, uint16_t id3v2HeaderLen);
extern uint32_t mp3_decode_id3v1_header(uint8_t *id3v1Header, uint16_t id3v1HeaderLen);
extern uint32_t mp3_decode_xing_header(uint8_t *xingHeader, uint16_t xingHeaderLen);
extern uint32_t mp3_decode_mp3_frame(uint8_t *mp3Header, uint16_t mp3HeaderLen);
extern bool mp3_decode_check_mp3_para(uint8_t *mp3Header, uint16_t mp3HeaderLen);
extern uint32_t mp3_decode_judge_frame_header(uint8_t *frameHeader, uint16_t frameHeadeLen);


void audio_fs_decode_deinit(void)
{
    for (int i = 0; i < ALL_TAGS; i++)
    {
        if (tagInfo[i].info != NULL)
        {
            os_mem_free(tagInfo[i].info);
            tagInfo[i].info = NULL;
        }
    }
    if (g_frameContent.content != NULL)
    {
        os_mem_free(g_frameContent.content);
        g_frameContent.content = NULL;
    }
}

uint16_t audio_fs_decode_init(void)
{
    memset(&g_frameContent, 0, sizeof(FRAME_CONTENT));
    memset(&g_frameInfo, 0, sizeof(FRAME_INFO));
#if (MP3_FORMAT_SUPPORT == 1)
    memset(&mp3Id3v2Info, 0, sizeof(ID3V2_INFO));
#endif
#if (AAC_FORMAT_SUPPORT == 1)
    memset(&aacId3v2Info, 0, sizeof(ID3V2_INFO));
#endif
#if (MP4_FORMAT_SUPPORT == 1)
    memset(&g_mp4HeaderInfo, 0, sizeof(MP4_HEADER_INFO));
#endif
    return 0;
}

void audio_fs_decode_set_frame_format(TCHAR *fname, uint16_t nameLen)
{
    memset(&g_frameInfo, 0, sizeof(FRAME_INFO));
    for (int i = 0; (i + 3) < nameLen; i++)
    {
        if (fname[i] == _T('.') &&
            ((fname[i + 1] == _T('r')) || (fname[i + 1] == _T('R'))) &&
            ((fname[i + 2] == _T('t')) || (fname[i + 2] == _T('T'))) &&
            ((fname[i + 3] == _T('k')) || (fname[i + 3] == _T('K'))))
        {
            g_frameInfo.format = RTK;
            break;
        }
        else if (fname[i] == _T('.') &&
                 ((fname[i + 1] == _T('a')) || (fname[i + 1] == _T('A'))) &&
                 ((fname[i + 2] == _T('a')) || (fname[i + 2] == _T('A'))) &&
                 ((fname[i + 3] == _T('c')) || (fname[i + 3] == _T('C'))))
        {
            g_frameInfo.format = AAC;
            break;
        }
        else if (fname[i] == _T('.') &&
                 ((fname[i + 1] == _T('m')) || (fname[i + 1] == _T('M'))) &&
                 ((fname[i + 2] == _T('p')) || (fname[i + 2] == _T('P'))) &&
                 (fname[i + 3] == _T('3')))
        {
            g_frameInfo.format = MP3;
            break;
        }
        else if (fname[i] == _T('.') &&
                 ((fname[i + 1] == _T('m')) || (fname[i + 1] == _T('M'))) &&
                 ((fname[i + 2] == _T('p')) || (fname[i + 2] == _T('P'))) &&
                 (fname[i + 3] == _T('4')))
        {
            g_frameInfo.format = MP4;
            break;
        }
        else if (fname[i] == _T('.') &&
                 ((fname[i + 1] == _T('f')) || (fname[i + 1] == _T('F'))) &&
                 ((fname[i + 2] == _T('l')) || (fname[i + 2] == _T('L'))) &&
                 ((fname[i + 3] == _T('a')) || (fname[i + 3] == _T('A'))) &&
                 ((fname[i + 3] == _T('c')) || (fname[i + 3] == _T('C'))))
        {
            g_frameInfo.format = FLAC;
            break;
        }
        else
        {
            g_frameInfo.format = UNKNOWN;
        }
    }
}

#if (MP4_FORMAT_SUPPORT == 1)
static uint16_t audio_fs_decode_judge_mp4_header(void *handle, uint32_t *pOffset)
{
    UINT len = 0;
    uint16_t res = 0;
    uint32_t offset = 0;
    MP4_BOX_HEADER mp4BoxHeader;
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    uint32_t type = 0;
    uint32_t typeSize = 0;

    offset = (*pOffset);
    while (1)
    {
        if ((res = f_read(&audio_file->fil, &mp4BoxHeader, sizeof(MP4_BOX_HEADER), &len)) != FR_OK)
        {
            res = AUDIO_FS_ERR_READ;
            goto L_Return;
        }
        typeSize = (((uint32_t)mp4BoxHeader.size[0] & 0xFF) << 24) |
                   (((uint32_t)mp4BoxHeader.size[1] & 0xFF) << 16) |
                   (((uint32_t)mp4BoxHeader.size[2] & 0xFF) << 8) |
                   (((uint32_t)mp4BoxHeader.size[3] & 0xFF));
        type = (((uint32_t)mp4BoxHeader.type[0] & 0xFF) << 24) |
               (((uint32_t)mp4BoxHeader.type[1] & 0xFF) << 16) |
               (((uint32_t)mp4BoxHeader.type[2] & 0xFF) << 8) |
               (((uint32_t)mp4BoxHeader.type[3] & 0xFF));
        APP_PRINT_INFO3("audio_fs_decode_judge_mp4_header: offset:0x%x, size:0x%x, type:0x%x",
                        offset, typeSize, type);
        switch (type)
        {
        case 0x66747970://ftyp
        case 0x6d766864://mvhd/* movieLength = suration/timeScale */
        case 0x696f6473://iods
        case 0x746b6864: //tkhd/* frequencyRate = timeScale */
        case 0x6d646864://mdhd
        case 0x68646c72://hdlr
        case 0x736d6864://smhd
        case 0x64726566://dref
        case 0x73747473://stts /* time to sample, the relationship between sample and time */
        case 0x73747363://stsc /* sample to chunk, the relationship between sample and chunk */
        case 0x63747473://ctts
        case 0x75647461://udta
        case stss:// /* sync sample, key frame in media */
            {
                offset += typeSize;
                f_lseek(&audio_file->fil, offset);
            }
            break;
        case 0x6d6f6f76://moov
        case 0x7472616b://trak
        case 0x6d646961://mdia
        case 0x6d696e66://minf
        case 0x64696e66://dinf
        case 0x7374626c://stbl
            {
                offset += sizeof(MP4_BOX_HEADER);
            }
            break;
        case 0x73747364://stsd  /* sample description, store encodingType/frequencyRate/channel etc */
            {
                MP4_STSD_BOX mp4StsdBox;
                if ((res = f_read(&audio_file->fil, &mp4StsdBox, sizeof(mp4StsdBox), &len)) != FR_OK)
                {
                    res = AUDIO_FS_ERR_READ;
                    goto L_Return;
                }
                g_frameInfo.format_info.mp4.sampling_frequency = mp4StsdBox.mp4AudioDescript.sampleRate;
                g_frameInfo.format_info.mp4.channel_mode = mp4StsdBox.mp4AudioDescript.channelCount;
                g_frameInfo.format_info.mp4.sample_counts = mp4StsdBox.mp4AudioDescript.sampleCount;
                offset += typeSize;
                f_lseek(&audio_file->fil, offset);
                APP_PRINT_INFO4("audio_fs_decode_judge_mp4_header: offset:0x%x, sampling_frequency:0x%x, channel_mode:0x%x, sample_counts: 0x%x",
                                offset, g_frameInfo.format_info.mp4.sampling_frequency,
                                g_frameInfo.format_info.mp4.channel_mode,
                                g_frameInfo.format_info.mp4.sample_counts);
            }
            break;
        case 0x7374737a://stsz/* each sample size */
        case 0x73747a32://stz2 /* each sample size */
            {
                uint8_t temp[12] = "";/* get from file */
                if ((res = f_read(&audio_file->fil, temp, sizeof(temp), &len)) != FR_OK)
                {
                    res = AUDIO_FS_ERR_READ;
                    goto L_Return;
                }
                g_mp4HeaderInfo.totalFrameCnt = (((uint64_t)temp[4] & 0xFF) << 56) |
                                                (((uint64_t)temp[5] & 0xFF) << 48) |
                                                (((uint64_t)temp[6] & 0xFF) << 40) |
                                                (((uint64_t)temp[7] & 0xFF) << 32) |
                                                (((uint64_t)temp[8] & 0xFF) << 24) |
                                                (((uint64_t)temp[9] & 0xFF) << 16) |
                                                (((uint64_t)temp[10] & 0xFF) << 8) |
                                                (((uint64_t)temp[11] & 0xFF));
                g_frameInfo.format_info.mp4.total_frames = g_mp4HeaderInfo.totalFrameCnt;
                g_frameInfo.format_info.mp4.total_file_size = f_size(&audio_file->fil);
                g_mp4HeaderInfo.frameTableStartOffset = offset + (sizeof(MP4_BOX_HEADER) + sizeof(temp));
                g_mp4HeaderInfo.frameTableLength = typeSize - (sizeof(MP4_BOX_HEADER) + sizeof(temp));
                offset += typeSize;
                f_lseek(&audio_file->fil, offset);
                APP_PRINT_INFO4("audio_fs_decode_judge_mp4_header: frameTableEndOffset:0x%x, frameTableStartOffset:0x%x, frameTableLength:0x%x, totalFrameCnt: 0x%x",
                                offset, g_mp4HeaderInfo.frameTableStartOffset, g_mp4HeaderInfo.frameTableLength,
                                g_mp4HeaderInfo.totalFrameCnt);
            }
            break;
        case 0x7374636f://stco /* chunk offset in file */
        case 0x636f3634://co64 /* chunk offset in file */
            {
                g_mp4HeaderInfo.chunkTableOffset = (offset + sizeof(MP4_BOX_HEADER) + 8);
                g_mp4HeaderInfo.chunkTableLength = typeSize - (sizeof(MP4_BOX_HEADER) + 8);
                offset += typeSize;
                f_lseek(&audio_file->fil, offset);
                APP_PRINT_INFO3("audio_fs_decode_judge_mp4_header: offset:0x%x, chunkTableOffset:0x%x, chunkTableLength:0x%x",
                                offset, g_mp4HeaderInfo.chunkTableOffset, g_mp4HeaderInfo.chunkTableLength);
            }
            break;
        case 0x6d646174://mdat
            {
                g_mp4HeaderInfo.mdatOffset = (offset + sizeof(MP4_BOX_HEADER));
                g_mp4HeaderInfo.mdatLength = (offset + sizeof(MP4_BOX_HEADER));
                offset += sizeof(MP4_BOX_HEADER);
                //jump to the location of first frame, maybe need to known the length of first frame
                f_lseek(&audio_file->fil, offset);
                res = AUDIO_FS_OK;
                APP_PRINT_INFO5("audio_fs_decode_judge_mp4_header: res:%d, offset:0x%x, type:0x%x, mdatOffset:0x%x, mdatLength:0x%x",
                                res, offset, type, g_mp4HeaderInfo.mdatOffset,
                                g_mp4HeaderInfo.mdatLength);
                *pOffset = offset;
                return MP4_HEADER;
            }
        default:
            {
                offset += sizeof(MP4_BOX_HEADER);
                f_lseek(&audio_file->fil, offset);
                if (f_eof(&audio_file->fil))
                {
                    res = FS_END_OF_FILE;
                    goto L_Return;
                }
            }
            break;
        }
    }

L_Return:
    {
        APP_PRINT_INFO3("audio_fs_decode_judge_mp4_header: res:%d, offset:0x%x, type:0x%x", res, offset,
                        type);
        *pOffset = offset;
        return res;
    }
}
#endif

#if (MP3_FORMAT_SUPPORT == 1)
/*
return tagInfo size
*/
static uint32_t audio_fs_decode_judge_tagInfoSize(uint16_t tagIdx, uint16_t tagInfoSize)
{
    if (tagInfoSize == 0)
    {
        return 0;
    }
    else if (tagInfoSize > LARGEST_TAG_INFO_LEN)
    {
        tagInfo[tagIdx].length = LARGEST_TAG_INFO_LEN;
    }
    else
    {
        tagInfo[tagIdx].length = tagInfoSize;
    }

    if (tagInfo[tagIdx].info != NULL)
    {
        os_mem_free(tagInfo[tagIdx].info);
        tagInfo[tagIdx].info = NULL;
    }
    tagInfo[tagIdx].info = os_mem_alloc(OS_MEM_TYPE_DATA, tagInfo[tagIdx].length);
    if (tagInfo[tagIdx].info == NULL)
    {
        return 0;
    }
    return tagInfo[tagIdx].length;
}

uint32_t audio_fs_decode_id3v2_info(void *handle, uint16_t tagIdx)
{
    UINT read_len = 0;
    uint32_t tmpLen = 0;
    ID3V2FH frameHeader;
    uint16_t tagInfoSize = 0;
    uint32_t offset = 0;
    uint16_t tagInfoSizeMalloc = 0;
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
#if (AUDIO_FS_DECODE_DEBUG == 1)
    APP_PRINT_INFO2("audio_fs_decode_id3v2_info: startOffset:0x%x, length:0x%x",
                    mp3Id3v2Info.startOffset, mp3Id3v2Info.length);
#endif
    if (mp3Id3v2Info.startOffset == 0)
    {
        APP_PRINT_ERROR0("audio_fs_decode_id3v2_info: No id3v2 header found!");
        return 0;
    }
    offset = mp3Id3v2Info.startOffset;
    f_lseek(&audio_file->fil, offset);
    while ((tmpLen + sizeof(ID3V2H)) < mp3Id3v2Info.length)
    {
        tagInfoSize = 0;
        tagInfoSizeMalloc = 0;
        f_lseek(&audio_file->fil, offset);
#if (AUDIO_FS_DECODE_DEBUG == 1)
        APP_PRINT_INFO2("audio_fs_decode_id3v2_info: tagIdx:0x%x, offset:0x%x", tagIdx, offset);
#endif
        if (f_read(&audio_file->fil, &frameHeader, sizeof(ID3V2H), &read_len) == FR_OK)
        {
            tagInfoSize = ((frameHeader.size)[0] << 24)
                          | ((frameHeader.size)[1] << 16)
                          | ((frameHeader.size)[2] << 8)
                          | (frameHeader.size)[3];
            tmpLen += sizeof(ID3V2FH) + tagInfoSize;
            switch (tagIdx)
            {
            case TIT2:
                if (strncmp(frameHeader.frameID, "TIT2", 4) == 0)
                {
#if (AUDIO_FS_DECODE_DEBUG == 1)
                    APP_PRINT_INFO1("audio_fs_decode_id3v2_info: TIT2 found - size(%b)", TRACE_BINARY(4,
                                    frameHeader.size));
#endif
                    tagInfoSizeMalloc = audio_fs_decode_judge_tagInfoSize(tagIdx, tagInfoSize);
                    if (tagInfoSizeMalloc != 0)
                    {
                        if (f_read(&audio_file->fil, tagInfo[tagIdx].info, tagInfoSizeMalloc, &read_len) != FR_OK)
                        {
                            return AUDIO_FS_ERR_READ;
                        }
                    }
                    offset = audio_fs_get_file_offset(handle);
                    audio_fs_set_file_offset(handle, offset);
                    return tagInfoSizeMalloc;
                }
                break;
            case TALB:
                if (strncmp(frameHeader.frameID, "TALB", 4) == 0)
                {
#if (AUDIO_FS_DECODE_DEBUG == 1)
                    APP_PRINT_INFO1("audio_fs_decode_id3v2_info: TALB found - size(%b)", TRACE_BINARY(4,
                                    frameHeader.size));
#endif
                    tagInfoSizeMalloc = audio_fs_decode_judge_tagInfoSize(tagIdx, tagInfoSize);
                    if (tagInfoSizeMalloc != 0)
                    {
                        if (f_read(&audio_file->fil, tagInfo[tagIdx].info, tagInfoSizeMalloc, &read_len) != FR_OK)
                        {
                            return AUDIO_FS_ERR_READ;
                        }
                    }
                    offset = audio_fs_get_file_offset(handle);
                    audio_fs_set_file_offset(handle, offset);
                    return tagInfoSizeMalloc;
                }
                break;
            case TCOM:  /* Composer */
                if (strncmp(frameHeader.frameID, "TCOM", 4) == 0)
                {
#if (AUDIO_FS_DECODE_DEBUG == 1)
                    APP_PRINT_INFO1("audio_fs_decode_id3v2_info: TCOM found - size(%b)", TRACE_BINARY(4,
                                    frameHeader.size));
#endif
                    tagInfoSizeMalloc = audio_fs_decode_judge_tagInfoSize(tagIdx, tagInfoSize);
                    if (tagInfoSizeMalloc != 0)
                    {
                        if (f_read(&audio_file->fil, tagInfo[tagIdx].info, tagInfoSizeMalloc, &read_len) != FR_OK)
                        {
                            return AUDIO_FS_ERR_READ;
                        }
                    }
                    offset = audio_fs_get_file_offset(handle);
                    audio_fs_set_file_offset(handle, offset);
                    return tagInfoSizeMalloc;
                }
                break;
            case TCON:  /* content type */
                if (strncmp(frameHeader.frameID, "TCON", 4) == 0)
                {
#if (AUDIO_FS_DECODE_DEBUG == 1)
                    APP_PRINT_INFO1("audio_fs_decode_id3v2_info: TCON found - size(%b)", TRACE_BINARY(4,
                                    frameHeader.size));
#endif
                    tagInfoSizeMalloc = audio_fs_decode_judge_tagInfoSize(tagIdx, tagInfoSize);
                    if (tagInfoSizeMalloc != 0)
                    {
                        if (f_read(&audio_file->fil, tagInfo[tagIdx].info, tagInfoSizeMalloc, &read_len) != FR_OK)
                        {
                            return AUDIO_FS_ERR_READ;
                        }
                    }
                    offset = audio_fs_get_file_offset(handle);
                    audio_fs_set_file_offset(handle, offset);
                    return tagInfoSizeMalloc;
                }
                break;
            case TPE1:
                if (strncmp(frameHeader.frameID, "TPE1", 4) == 0)
                {
#if (AUDIO_FS_DECODE_DEBUG == 1)
                    APP_PRINT_INFO1("audio_fs_decode_id3v2_info: TPE1 found - size(%b)", TRACE_BINARY(4,
                                    frameHeader.size));
#endif
                    tagInfoSizeMalloc = audio_fs_decode_judge_tagInfoSize(tagIdx, tagInfoSize);
                    if (tagInfoSizeMalloc != 0)
                    {
                        if (f_read(&audio_file->fil, tagInfo[tagIdx].info, tagInfoSizeMalloc, &read_len) != FR_OK)
                        {
                            return AUDIO_FS_ERR_READ;
                        }
                    }
                    offset = audio_fs_get_file_offset(handle);
                    audio_fs_set_file_offset(handle, offset);
                    return tagInfoSizeMalloc;
                }
                break;
            case TPE2:
                if (strncmp(frameHeader.frameID, "TPE2", 4) == 0)
                {
#if (AUDIO_FS_DECODE_DEBUG == 1)
                    APP_PRINT_INFO1("audio_fs_decode_id3v2_info: TPE2 found - size(%b)", TRACE_BINARY(4,
                                    frameHeader.size));
#endif
                    tagInfoSizeMalloc = audio_fs_decode_judge_tagInfoSize(tagIdx, tagInfoSize);
                    if (tagInfoSizeMalloc != 0)
                    {
                        if (f_read(&audio_file->fil, tagInfo[tagIdx].info, tagInfoSizeMalloc, &read_len) != FR_OK)
                        {
                            return AUDIO_FS_ERR_READ;
                        }
                    }
                    offset = audio_fs_get_file_offset(handle);
                    audio_fs_set_file_offset(handle, offset);
                    return tagInfoSizeMalloc;
                }
                break;
            case TXXX:
                if (strncmp(frameHeader.frameID, "TXXX", 4) == 0)
                {
#if (AUDIO_FS_DECODE_DEBUG == 1)
                    APP_PRINT_INFO1("audio_fs_decode_id3v2_info: TXXX found - size(%b)", TRACE_BINARY(4,
                                    frameHeader.size));
#endif
                    tagInfoSizeMalloc = audio_fs_decode_judge_tagInfoSize(tagIdx, tagInfoSize);
                    if (tagInfoSizeMalloc != 0)
                    {
                        if (f_read(&audio_file->fil, tagInfo[tagIdx].info, tagInfoSizeMalloc, &read_len) != FR_OK)
                        {
                            return AUDIO_FS_ERR_READ;
                        }
                    }
                    offset = audio_fs_get_file_offset(handle);
                    audio_fs_set_file_offset(handle, offset);
                    return tagInfoSizeMalloc;
                }
                break;
            case TOFN:
                if (strncmp(frameHeader.frameID, "TOFN", 4) == 0)
                {
#if (AUDIO_FS_DECODE_DEBUG == 1)
                    APP_PRINT_INFO1("audio_fs_decode_id3v2_info: TOFN found - size(%b)", TRACE_BINARY(4,
                                    frameHeader.size));
#endif
                    tagInfoSizeMalloc = audio_fs_decode_judge_tagInfoSize(tagIdx, tagInfoSize);
                    if (tagInfoSizeMalloc != 0)
                    {
                        if (f_read(&audio_file->fil, tagInfo[tagIdx].info, tagInfoSizeMalloc, &read_len) != FR_OK)
                        {
                            return AUDIO_FS_ERR_READ;
                        }
                    }
                    offset = audio_fs_get_file_offset(handle);
                    audio_fs_set_file_offset(handle, offset);
                    return tagInfoSizeMalloc;
                }
                break;
            // add more case
            default:
                offset += (sizeof(ID3V2H) + tagInfoSize);
                break;
            }
            offset += (sizeof(ID3V2H) + tagInfoSize);
        }
        else
        {
            return AUDIO_FS_ERR_READ;
        }
    }
    offset = audio_fs_get_file_offset(handle);
    audio_fs_set_file_offset(handle, offset);
    return 0;
}

uint16_t audio_fs_decode_get_id3v2_InfoLen(uint16_t tagIdx)
{
    return tagInfo[tagIdx].length;
}

uint8_t *audio_fs_decode_get_id3v2_Info(uint16_t tagIdx)
{
    return tagInfo[tagIdx].info;
}

void audio_fs_decode_get_mp3_playtime(void)
{
    uint32_t playTime = 0;
    if (audio_fs_decode_judge_tagInfoSize(TIME, 4))
    {
        if (g_frameInfo.format_info.mp3.mp3_sub_format == MP3_CBR)
        {
            playTime = g_frameInfo.format_info.mp3.total_file_size * 8 / (g_frameInfo.format_info.mp3.bit_rate *
                                                                          1000);
        }
        else if (g_frameInfo.format_info.mp3.mp3_sub_format == MP3_VBR)
        {
            if (g_frameInfo.format_info.mp3.total_frames)
            {
                /* if can get total_frames from xing header */
                playTime = g_frameInfo.format_info.mp3.total_frames * g_frameInfo.format_info.mp3.sample_counts /
                           g_frameInfo.format_info.mp3.sampling_frequency;
            }
            else
            {
                /* or get playTime same as CBR */
                playTime = g_frameInfo.format_info.mp3.total_file_size * 8 / (g_frameInfo.format_info.mp3.bit_rate *
                                                                              1000);
            }
        }
        memcpy(tagInfo[TIME].info, &playTime, 4);
    }
#if (AUDIO_FS_DECODE_DEBUG == 1)
    APP_PRINT_INFO5("audio_fs_decode_get_mp3_playtime: total_file_size:0x%x, bitrate:%d, total_frames:%d, sample_counts:%d, sampling_freq:%d",
                    g_frameInfo.format_info.mp3.total_file_size,
                    g_frameInfo.format_info.mp3.bit_rate,
                    g_frameInfo.format_info.mp3.total_frames,
                    g_frameInfo.format_info.mp3.sample_counts,
                    g_frameInfo.format_info.mp3.sampling_frequency);
#endif
    APP_PRINT_INFO2("audio_fs_decode_get_mp3_playtime: playTime:%d, mp3_sub_format:%d",
                    playTime, g_frameInfo.format_info.mp3.mp3_sub_format);
}

static uint16_t audio_fs_decode_judge_mp3_header(void *handle, uint32_t *pOffset)
{
    uint32_t offset = 0;
    uint32_t length = 0;
    uint32_t headerSize = 0;
    UINT read_len = 0;
    uint32_t scan_head_size = 0;
    uint8_t header[sizeof(ID3V1H)] = "";
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    uint16_t i = 1;

    offset = (*pOffset);
#if (AUDIO_FS_DECODE_DEBUG == 1)
    APP_PRINT_INFO1("audio_fs_decode_judge_mp3_header: offset:0x%x", offset);
#endif
    while (1)
    {
        if (f_read(&audio_file->fil, header, sizeof(header), &read_len) == FR_OK)
        {
            if (read_len < sizeof(header))
            {
                return FS_END_OF_FILE;
            }

            if (findMp3HeaderTimes == 0)
            {
                headerSize = sizeof(ID3V2H);
                if ((length = mp3_decode_id3v2_header(header, headerSize)) != 0)
                {
                    offset += (headerSize + length);
                    *pOffset = offset; /* jump to the end of id3v2 header */
                    return ID3V2_HEADER;
                }
                // not id3v2
                headerSize = sizeof(header);
                length = mp3_decode_judge_frame_header(header, headerSize);
                if (length == 0)
                {
                    for (i = 1; i <= (sizeof(header) - sizeof(ID3V2H)); i++)
                    {
                        length = mp3_decode_id3v2_header(header + i, sizeof(ID3V2H));
                        if (length != 0)
                        {
                            offset += (i + sizeof(ID3V2H) + length);
                            *pOffset = offset; /* jump to the end of id3v2 header */

                            return ID3V2_HEADER;
                        }
                        length = mp3_decode_judge_frame_header(header + i, sizeof(MP3_FHEADER));
                        if (length != 0)
                        {
                            offset += i;
                            *pOffset = offset;/* jump to the start of mp3 frame header */

                            return XING_OR_MP3_HEADER; /* check if xing or MP3 header */
                        }
                    }
                }
                else if (length == sizeof(MP3_FHEADER))
                {
                    findMp3HeaderTimes = 1;
                    *pOffset = offset; /* jump to the start of mp3 frame header, start to decode mp3 frame */
                    g_frameInfo.format_info.mp3.mp3_sub_format = MP3_CBR;
                    g_frameInfo.format_info.mp3.total_file_size = f_size(&audio_file->fil) - offset;

                    return MP3_HEADER;
                }
                else if ((length != 0) && (length != sizeof(MP3_FHEADER)))
                {
                    /* jump to the start of xing header, to decode frames and fileSize */
                    offset += length;
                    uint8_t xingHeader[16] = "";
                    memcpy(xingHeader, header + length, sizeof(xingHeader));
                    length = mp3_decode_xing_header(xingHeader, sizeof(xingHeader));
                    if (length != 0)/* find Xing header */
                    {
                        *pOffset = offset + length; /* jump to the end of xing header, start to decode mp3 frame */
                        g_frameInfo.format_info.mp3.mp3_sub_format = MP3_VBR;
                        if (g_frameInfo.format_info.mp3.total_file_size > offset)
                        {
                            /* if can get total_file_size from xing header */
                            g_frameInfo.format_info.mp3.total_file_size -= offset;
                        }
                        else
                        {
                            /* or get total_file_size from f_size */
                            g_frameInfo.format_info.mp3.total_file_size = f_size(&audio_file->fil) - offset;
                        }

                        return XING_HEADER;
                    }
                }
            }

            if (findMp3HeaderTimes == 1)
            {
                length = mp3_decode_judge_frame_header(header, sizeof(MP3_FHEADER));
                if (length != 0)
                {
                    *pOffset = offset; /* jump to the start of mp3 frame header, start to decode mp3 frame */

                    return MP3_HEADER;
                }
                else
                {
                    if ((length = mp3_decode_id3v1_header(header, sizeof(ID3V1H))) != 0)
                    {
                        uint32_t cur_offset = audio_fs_get_file_offset(handle);
                        uint32_t file_size = audio_fs_size(handle);
                        if ((file_size - cur_offset) < length)
                        {
                            offset += (file_size - cur_offset);
                        }
                        else
                        {
                            offset += length;
                        }
                        *pOffset = offset; /* jump to the end of id3v1 header */

                        return ID3V1_HEADER;
                    }

                    for (i = 1; i <= (sizeof(header) - sizeof(MP3_FHEADER)); i++)
                    {
                        length = mp3_decode_judge_frame_header(header + i, sizeof(MP3_FHEADER));
                        if (length != 0)
                        {
                            if (mp3_decode_check_mp3_para(header + i, sizeof(MP3_FHEADER)))
                            {
                                offset += i;
                                *pOffset = offset;/* jump to the start of mp3 frame header, start to decode mp3 frame */

                                return MP3_HEADER;
                            }
                            else
                            {
                                return FS_END_OF_FILE;
                            }

                        }
                    }
                }
            }

            if (length == 0)
            {
                scan_head_size += i;
                offset += i;
                *pOffset = offset;
                audio_fs_set_file_offset(handle, offset);
                if (((scan_head_size >= (50 * sizeof(header))) && (length == 0)) ||
                    f_eof(&audio_file->fil))
                {
                    return FS_END_OF_FILE;
                }
            }
        }
        else
        {
            return AUDIO_FS_ERR_READ;
        }
    }
}

#endif

#if (AAC_FORMAT_SUPPORT == 1)
static uint16_t audio_fs_decode_judge_aac_header(void *handle, uint32_t *pOffset)
{
    uint32_t offset = 0;
    uint32_t length = 0;
    uint32_t headerSize = 0;
    UINT read_len = 0;
    uint8_t header[sizeof(ID3V1H)] = "";
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    uint8_t rtkFileType[4] = "";
    HEADER_TYPE headerType = HEADER_TYPE_NONE;

    offset = (*pOffset);
#if (AUDIO_FS_DECODE_DEBUG == 1)
    APP_PRINT_INFO1("audio_fs_decode_judge_aac_header: offset:0x%x", offset);
#endif
    while (1)
    {
        if (f_read(&audio_file->fil, header, sizeof(header), &read_len) == FR_OK)
        {
            headerSize = sizeof(ID3V2H);
            if ((length = aac_decode_id3v2_header(header, headerSize)) != 0)
            {
                offset += length;
                *pOffset = offset; /* jump to the end of id3v2 header */
                if (g_frameInfo.format == RTK)
                {
                    memcpy(rtkFileType, header + sizeof(ID3V2H) + sizeof(ID3V2EH), sizeof(rtkFileType));
                    if ((memcmp(rtkFileType, "LATM", 4) == 0) ||
                        (memcmp(rtkFileType, "latm", 4) == 0))
                    {
                        g_frameInfo.format_info.rtk.rtkTransFormat = RTK_LATM;
                    }
                    else if ((memcmp(rtkFileType, "SBC", 3) == 0) ||
                             (memcmp(rtkFileType, "sbc", 3) == 0))
                    {
                        g_frameInfo.format_info.rtk.rtkTransFormat = RTK_SBC;
                    }
                    else
                    {
                        g_frameInfo.format_info.rtk.rtkTransFormat = RTK_ADTS;
                    }
                }
                return ID3V2_HEADER;
            }

            headerSize = sizeof(header);
            if ((length = aac_decode_id3v1_header(header, headerSize)) != 0)
            {
                offset += headerSize;
                *pOffset = offset; /* jump to the end of id3v1 header */

                return ID3V1_HEADER;
            }

            if (g_frameInfo.format == RTK)/* for RTK */
            {
                if (g_frameInfo.format_info.rtk.rtkTransFormat == RTK_LATM)
                {
                    headerSize = LATM_SYNC_WORD_LEN;
                }
                else if (g_frameInfo.format_info.rtk.rtkTransFormat == RTK_SBC)
                {
                    headerSize = SBC_SYNC_WORD_LEN;
                }
                else if (g_frameInfo.format_info.rtk.rtkTransFormat == RTK_ADTS)
                {
                    headerSize = ADTS_SYNC_WORD_LEN;
                }
            }
            else/* for AAC */
            {
                headerSize = ADTS_SYNC_WORD_LEN;
            }
            if ((length = aac_decode_judge_frame_header(header, headerSize)) != 0)
            {
                *pOffset = offset; /* jump to the start of frame header */
                if (length == LATM_SYNC_WORD_LEN)
                {
                    headerType = RTK_LATM_HEADER;
                }
                else if (length == SBC_SYNC_WORD_LEN)
                {
                    headerType = RTK_SBC_HEADER;
                }
                else if ((length == ADTS_SYNC_WORD_LEN) && (g_frameInfo.format == RTK))
                {
                    headerType = RTK_ADTS_HEADER;
                }
                else if ((length == ADTS_SYNC_WORD_LEN) && (g_frameInfo.format == AAC))
                {
                    headerType = AAC_HEADER;
                }

                return headerType;
            }

            offset++;
            audio_fs_set_file_offset(handle, offset);
            if (f_eof(&audio_file->fil))
            {
                return FS_END_OF_FILE;
            }
        }
        else
        {
            return AUDIO_FS_ERR_READ;
        }
    }
}
#endif

uint16_t audio_fs_decode_before_get_frame(void *handle)
{
    uint32_t offset = 0;
    uint16_t headerType = 0;

    if (g_frameInfo.format == MP3)
    {
        g_frameInfo.format_info.mp3.mp3_sub_format = MP3_CBR;
    }
    else if (g_frameInfo.format == RTK)
    {
        g_frameInfo.format_info.rtk.rtkTransFormat = RTK_ADTS;
    }
    while (1)
    {
        offset = audio_fs_get_file_offset(handle);
        switch (g_frameInfo.format)
        {
#if (AAC_FORMAT_SUPPORT == 1)
        case RTK:
        case AAC:
            headerType = audio_fs_decode_judge_aac_header(handle, &offset);
            break;
#endif
#if (MP3_FORMAT_SUPPORT == 1)
        case MP3:
            findMp3HeaderTimes = 0;
            headerType = audio_fs_decode_judge_mp3_header(handle, &offset);
            break;
#endif
#if (MP4_FORMAT_SUPPORT == 1)
        case MP4:
            headerType = audio_fs_decode_judge_mp4_header(handle, &offset);
            break;
#endif
        default:
            break;
        }
        audio_fs_set_file_offset(handle, offset);
        APP_PRINT_INFO3("audio_fs_decode_before_get_frame: headerType: 0x%x, offset:0x%x, rtkTransFormat:0x%x",
                        headerType,
                        offset, g_frameInfo.format_info.rtk.rtkTransFormat);
        if (headerType == AUDIO_FS_ERR_READ)
        {
            return AUDIO_FS_ERR_READ;
        }
        else if (headerType == FS_END_OF_FILE)
        {
            return FS_END_OF_FILE;
        }
        else if ((headerType == RTK_ADTS_HEADER) ||
                 (headerType == RTK_LATM_HEADER) ||
                 (headerType == RTK_SBC_HEADER) ||
                 (headerType == AAC_HEADER) ||
                 (headerType == MP3_HEADER) ||
                 (headerType == MP4_HEADER))
        {
            return 0;
        }
    }
}

uint16_t audio_fs_decode_get_frame(void *handle, FRAME_CONTENT *p_frameContent)
{
    UINT read_len = 0;
    uint16_t res = 0;
    uint32_t offset = 0;
    uint32_t frameSize = 0;
#if (AAC_FORMAT_SUPPORT == 1)
    uint16_t pktHeaderSize = 0;
    uint8_t rtkFrame[16] = {0};
#endif
#if (MP3_FORMAT_SUPPORT == 1)
    uint8_t mp3Frame[4] = {0};
#endif
#if (MP4_FORMAT_SUPPORT == 1)
    uint8_t tempFrameSize[4] = "";
    uint32_t frameTableCurOffset = 0;
#endif
#if ((AAC_FORMAT_SUPPORT == 1) || (MP3_FORMAT_SUPPORT == 1) || (MP4_FORMAT_SUPPORT == 1))
    uint16_t headerType = 0;
#endif
    T_AUDIO_FILE *audio_file = NULL;
    audio_file = (T_AUDIO_FILE *)handle;
    RTK_PKT_HEADER pktHeader;

    offset = audio_fs_get_file_offset(handle);
    switch (g_frameInfo.format)
    {
#if (AAC_FORMAT_SUPPORT == 1)
    case RTK:
    case AAC:
        if ((res = f_read(&audio_file->fil, rtkFrame, sizeof(rtkFrame), &read_len)) == FR_OK)
        {
            if ((g_frameInfo.format == RTK) && (g_frameInfo.format_info.rtk.rtkTransFormat == RTK_LATM))
            {
                pktHeaderSize = LATM_SYNC_WORD_LEN;
            }
            else if ((g_frameInfo.format == RTK) && (g_frameInfo.format_info.rtk.rtkTransFormat == RTK_SBC))
            {
                pktHeaderSize = SBC_SYNC_WORD_LEN;
            }
            else if ((g_frameInfo.format == RTK) && (g_frameInfo.format_info.rtk.rtkTransFormat == RTK_ADTS))
            {
                pktHeaderSize = ADTS_SYNC_WORD_LEN;
            }
            // TODO    another AAC format decode
            else if (g_frameInfo.format == AAC)
            {
                pktHeaderSize = ADTS_SYNC_WORD_LEN;
            }
            /* for sbc and latm, frameSize is equal to pkt length
               for aadts and aac, frameSize is equal to frame length */
            frameSize = aac_decode_aac_frame(rtkFrame, pktHeaderSize);
            if (frameSize == 0)
            {
                APP_PRINT_INFO4("audio_fs_decode_get_frame: offset:0x%x, format:0x%x, pktHeaderSize:0x%x, rtkFrame(%b)",
                                offset, g_frameInfo.format, pktHeaderSize, TRACE_BINARY(16, rtkFrame));
                audio_fs_set_file_offset(handle, offset);
                headerType = audio_fs_decode_judge_aac_header(handle, &offset);
                audio_fs_set_file_offset(handle, offset);
                if ((headerType == RTK_ADTS_HEADER) || (headerType == AAC_HEADER))
                {
                    frameSize = aac_decode_aac_frame(rtkFrame, pktHeaderSize);
                }
                else
                {
                    return headerType;
                }
            }
        }
        break;
#endif
#if (MP3_FORMAT_SUPPORT == 1)
    case MP3:
        if ((res = f_read(&audio_file->fil, mp3Frame, sizeof(mp3Frame), &read_len)) == FR_OK)
        {
            frameSize = mp3_decode_mp3_frame(mp3Frame, sizeof(mp3Frame));
            if (frameSize == 0)
            {
                APP_PRINT_INFO5("audio_fs_decode_get_frame: offest:0x%x, mp3Frame[0]:0x%x, mp3Frame[1]: 0x%x, mp3Frame[2]: 0x%x, mp3Frame[3]: 0x%x",
                                offset, mp3Frame[0], mp3Frame[1], mp3Frame[2], mp3Frame[3]);
                audio_fs_set_file_offset(handle, offset);
                headerType = audio_fs_decode_judge_mp3_header(handle, &offset);
                audio_fs_set_file_offset(handle, offset);
                if (headerType == MP3_HEADER)
                {
                    if ((res = f_read(&audio_file->fil, mp3Frame, sizeof(mp3Frame), &read_len)) == FR_OK)
                    {
                        frameSize = mp3_decode_mp3_frame(mp3Frame, sizeof(mp3Frame));
                    }
                }
                else
                {
                    return headerType;
                }
            }
        }
        break;
#endif
#if (MP4_FORMAT_SUPPORT == 1)
    case MP4:
        if (g_mp4HeaderInfo.curFrameNum < g_mp4HeaderInfo.totalFrameCnt)
        {
            frameTableCurOffset = g_mp4HeaderInfo.frameTableStartOffset + g_mp4HeaderInfo.curFrameNum * 4 + 4;
            f_lseek(&audio_file->fil, frameTableCurOffset);
            if ((res = f_read(&audio_file->fil, tempFrameSize, sizeof(tempFrameSize), &read_len)) != FR_OK)
            {
                res = AUDIO_FS_ERR_READ;
                return res;
            }
            frameSize = (((uint32_t)tempFrameSize[0] & 0xFF) << 24) |
                        (((uint32_t)tempFrameSize[1] & 0xFF) << 16) |
                        (((uint32_t)tempFrameSize[2] & 0xFF) << 8) |
                        (((uint32_t)tempFrameSize[3] & 0xFF));
            offset = audio_fs_get_file_offset(handle);
            APP_PRINT_INFO5("audio_fs_decode_get_frame: fileOffset:0x%x-0x%x, frameTableCurOffset:0x%x, curFrameNum:0x%x, frameSize:0x%x",
                            offset, (offset + frameSize), frameTableCurOffset,
                            g_mp4HeaderInfo.curFrameNum, frameSize);
        }
        else
        {
            return FS_END_OF_FILE;
        }
        break;
#endif
    default:
        break;
    }
    if (frameSize == 0)
    {
        APP_PRINT_ERROR2("audio_fs_decode_get_frame: frameSize is zero! g_frameInfo.format: 0x%x, rtkTransFormat:0x%x",
                         g_frameInfo.format, g_frameInfo.format_info.rtk.rtkTransFormat);
        return res;
    }
    if ((g_frameInfo.format_info.rtk.rtkTransFormat == RTK_LATM) ||
        (g_frameInfo.format_info.rtk.rtkTransFormat == RTK_SBC))
    {
        audio_fs_set_file_offset(handle, offset);
        if ((res = f_read(&audio_file->fil, &pktHeader, sizeof(RTK_PKT_HEADER), &read_len)) != FR_OK)
        {
            APP_PRINT_ERROR2("audio_fs_decode_get_frame: frameSize:0x%x, res:0x%x", frameSize, res);
            return res;
        }
        offset += sizeof(RTK_PKT_HEADER);
        if (pktHeader.pktIsInValid == 1) /* pkt is Invalid */
        {
            offset += frameSize;
            audio_fs_set_file_offset(handle, offset);
            if (g_frameContent.content != NULL)
            {
                memcpy(p_frameContent, &g_frameContent, sizeof(FRAME_CONTENT));
            }
            return 0;
        }
    }
    if (g_frameInfo.format_info.rtk.rtkTransFormat == RTK_SBC)
    {
        g_frameContent.frameNum = pktHeader.pktFrameNum;
    }
    else
    {
        g_frameContent.frameNum = 1;
    }
    g_frameContent.length = frameSize;
    if (g_frameContent.content != NULL)
    {
        os_mem_free(g_frameContent.content);
        g_frameContent.content = NULL;
    }
    uint32_t ramSize = os_mem_peek(OS_MEM_TYPE_DATA);
    if (ramSize == 0)
    {
        return AUDIO_FS_NO_SPACE;
    }
    if (frameSize > ramSize)
    {
        frameSize = ramSize - 1;
    }
    g_frameContent.content = os_mem_alloc(OS_MEM_TYPE_DATA, frameSize);
    if (g_frameContent.content == NULL)
    {
        return AUDIO_FS_ERR_MALLOC;
    }
    memset(g_frameContent.content, 0, frameSize);
    audio_fs_set_file_offset(handle, offset);
    if ((res = f_read(&audio_file->fil, g_frameContent.content, frameSize, &read_len)) != FR_OK)
    {
        APP_PRINT_ERROR2("audio_fs_decode_get_frame: frameSize:0x%x, res:0x%x", frameSize, res);
        return res;
    }
#if (MP4_FORMAT_SUPPORT == 1)
    g_mp4HeaderInfo.curFrameNum ++;
#endif
    offset += frameSize;
    audio_fs_set_file_offset(handle, offset);
#if (AUDIO_FS_DECODE_DEBUG == 1)
    if (frameSize >= 32)
    {
        APP_PRINT_INFO2("audio_fs_decode_get_frame: offset:0x%x, (%b)",
                        offset, TRACE_BINARY(32, g_frameContent.content));
    }
    else
    {
        APP_PRINT_INFO2("audio_fs_decode_get_frame: offset, 0x%x(%b)",
                        offset, TRACE_BINARY(frameSize, g_frameContent.content));
    }
#endif
    memcpy(p_frameContent, &g_frameContent, sizeof(FRAME_CONTENT));
    return 0;
}

void audio_fs_decode_get_frame_info(FRAME_INFO *p_frameInfo)
{
    switch (g_frameInfo.format)
    {
#if (AAC_FORMAT_SUPPORT == 1)
    case RTK:
    case AAC:
#endif
#if (MP3_FORMAT_SUPPORT == 1)
    case MP3:
#endif
#if (MP4_FORMAT_SUPPORT == 1)
    case MP4:
        g_mp4HeaderInfo.curFrameNum--;
#endif
        memcpy(p_frameInfo, &g_frameInfo, sizeof(FRAME_INFO));
        break;
    default:
        break;
    }
}
