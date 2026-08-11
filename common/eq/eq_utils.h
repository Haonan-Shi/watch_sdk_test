/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _EQ_UTILS_H_
#define _EQ_UTILS_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif /*__cplusplus*/

#define EQ_TYPE                     (1)
#define EQ_LEN                      (1)
#define EQ_HEAD                     (4)
#define EQ_COEFF                    (10)
#define STAGE_NUM_MAX               (10)
#define EQ_MAX_SIZE                 (EQ_TYPE + EQ_LEN + EQ_HEAD + EQ_COEFF * STAGE_NUM_MAX)

#define EQ_GROUP_NUM                10

#define SAMPLE_RATE_44K             44100
#define SAMPLE_RATE_48K             48000

#define EQ_MODE_NULL                0xFF
#define EQ_MAX_INDEX                9
/**
 * \brief Audio EQ query type
 */
typedef enum
{
    AUDIO_EQ_QUERY_STATE    = 0x00,
    AUDIO_EQ_QUERY_NUM      = 0x01,
    AUDIO_EQ_QUERY_MAPPING  = 0x02,
    AUDIO_EQ_QUERY_SAMPLE_RATE = 0x03,
} T_AUDIO_EQ_QUERY_TYPE;

/**
 * \brief Audio EQ sample rate
 */
typedef enum
{
    AUDIO_EQ_SAMPLE_RATE_8KHZ       = 0x00,
    AUDIO_EQ_SAMPLE_RATE_16KHZ      = 0x01,
    AUDIO_EQ_SAMPLE_RATE_32KHZ      = 0x02,
    AUDIO_EQ_SAMPLE_RATE_44_1KHZ    = 0x03,
    AUDIO_EQ_SAMPLE_RATE_48KHZ      = 0x04,
    AUDIO_EQ_SAMPLE_RATE_88_2KHZ    = 0x05,
    AUDIO_EQ_SAMPLE_RATE_96KHZ      = 0x06,
    AUDIO_EQ_SAMPLE_RATE_MAX,
} T_AUDIO_EQ_SAMPLE_RATE;

/**  @brief  Audio EQ Frame Type.*/
typedef enum
{
    AUDIO_EQ_FRAME_SINGLE = 0x00,
    AUDIO_EQ_FRAME_START = 0x01,
    AUDIO_EQ_FRAME_CONTINUE = 0x02,
    AUDIO_EQ_FRAME_END = 0x03,
    AUDIO_EQ_FRAME_ABORT = 0x04
} T_AUDIO_EQ_FRAME_TYPE;

typedef enum
{
    EQ_IDX_REUSE_NONE,
    EQ_IDX_REUSE_NORMAL_ANC,
    EQ_IDX_REUSE_GAMING_ANC,
    EQ_IDX_REUSE_NORMAL_GAMING,
    EQ_IDX_REUSE_ALL,
} T_EQ_IDX_REUSE_TYPE;

typedef enum
{
    SPK_SW_EQ = 0,
    MIC_SW_EQ = 1,
    EQ_TYPE_MAX
} T_EQ_TYPE;

typedef enum
{
    EQ_STREAM_TYPE_AUDIO  = 0,
    EQ_STREAM_TYPE_VOICE  = 1,
    EQ_STREAM_TYPE_RECORD = 2,
    EQ_STREAM_TYPE_MAX,
} T_EQ_STREAM_TYPE;

typedef enum
{
    EQ_DATA_TO_DSP = 0,
    EQ_DATA_TO_PHONE = 1,
} T_EQ_DATA_DEST;

typedef enum
{
    NORMAL_MODE     = 0,
    GAMING_MODE     = 1,
    ANC_MODE        = 2,
    LINE_IN_MODE    = 3,
    VOICE_SPK_MODE  = 4,
    SPK_EQ_MODE_MAX = 5,
} T_SPK_EQ_MODE;

typedef enum
{
    APT_MODE        = 0,
    VOICE_MIC_MODE  = 1,
    MIC_EQ_MODE_MAX = 2,
} T_MIC_EQ_MODE;

/**
* \brief   Get the stagenum in EQ group currently.
*
* \param[in] eq_len           The EQ data length of this EQ group.
*
* \return  The stagenum of this EQ group.
*
*/
uint8_t eq_utils_stage_num_get(uint16_t eq_len);

#if F_APP_USER_EQ_SUPPORT
/**
* \brief   save the eq param to flash.
*
* \param[in] offset
* \param[in] data
* \param[in] len
*
* \return  The status of saving the EQ param.
*
*/
uint32_t eq_utils_save_eq_to_ftl(uint32_t offset, uint8_t *data, uint32_t len);

/**
* \brief   load the eq param from flash.
*
* \param[in] offset
* \param[in] data
* \param[in] len
*
* \return  The status of loading the EQ param.
*
*/
uint32_t eq_utils_load_eq_from_ftl(uint32_t offset, uint8_t *data, uint32_t len);

#endif

/**
 *
 * \brief   Get the number of EQ settings.
 *
 * \return  The number of EQ settings.
 *
 */
uint8_t eq_utils_num_get(T_EQ_TYPE eq_type, uint8_t mode);

/**
 *
 * \brief   Get the maximum parameter length of EQ settings.
 *
 * \return  The maximum parameter length of EQ settings.
 *
 */
uint16_t eq_utils_param_len_get(void);

/**
* \brief   Get the EQ index in EQ group.
*
* \param[in] eq_type    eq type
* \param[in] mode       app mode, normal / gaming.
* \param[in] index      The EQ index in EQ group;
* \param[out] index     The EQ index in EQ group;
*
* \return  the EQ index in EQ group.
*
*/
uint8_t eq_utils_original_eq_index_get(T_EQ_TYPE eq_type, uint8_t eq_mode, uint8_t index);

/**
* \brief   Get the capacity in EQ group currently.
*
* \param[in] eq_type   eq type
* \param[in] mode      app mode, normal / gaming.
*
* \return  The capacity of this EQ group.
*
*/
uint32_t eq_utils_get_capacity(T_EQ_TYPE type, uint8_t mode);

/**
 *
 * \brief   init the EQ engine.
 *
 * \return  The status of init the EQ engine.
 * \retval  true    EQ engine was inited successfully.
 * \retval  false   EQ engine was failed to init.
 *
 */
bool eq_utils_init(void);

/**
 *
 * \brief   deinit the EQ engine.
 *
 * \return  The status of init the EQ engine.
 * \retval  true    EQ engine was deinited successfully.
 * \retval  false   EQ engine was failed to deinit.
 *
 */
bool eq_utils_deinit(void);

/**
* \brief   init the capacity of this EQ group.
*
* \param[in] eq_type   eq type
* \param[in] mode      app mode, normal / gaming.
* \param[in] bitmask   The capacity of this EQ group;
*
*/
void eq_utils_bitmask_init(T_EQ_TYPE eq_type, uint8_t mode, uint32_t bitmask);

#ifdef __cplusplus
}
#endif

#endif
