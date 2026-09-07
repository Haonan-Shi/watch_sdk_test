/*
 *      Copyright (C) 2020 Apple Inc. All Rights Reserved.
 *
 *      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
 *      which is contained in the License.txt file distributed with the Find My Network ADK,
 *      and only to those who accept that license.
 */

#ifndef fmna_sound_platform_h
#define fmna_sound_platform_h

typedef enum
{
    FMNA_SOUND_STOP    = 0x00,
    FMNA_SOUND_PLAY    = 0x01,
} T_FMNA_SOUND_STATE;


void fmna_sound_platform_init(void);
void fmna_sound_platform_start(void);
void fmna_sound_platform_stop(void);

#endif /* fmna_sound_platform_h */
