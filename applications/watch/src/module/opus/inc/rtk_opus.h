#ifndef RTK_OPUS_H
#define RTK_OPUS_H

/**
 * @file rtk_opus.h
 * @author YuYin
 * @brief
 * @version 0.1
 * @date 2025-07-02
 *
 * @copyright Copyright (c) 2025
 *
 * 说明：rtk_celt_encode、rtk_celt_decode、rtk_silk_encode、rtk_silk_decode 这四个接口都是针对每一帧 OPUS 音频数据进行编解码处理的。
 * 每调用一次这些函数，通常就是对一帧 PCM 数据进行编码，或对一帧 OPUS 比特流进行解码。
 * 帧长由参数 frame_size 或 frame_size_type 决定。
 *
 */

#include "opus.h"
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Sample rate options
typedef enum
{
    RTK_OPUS_SR_8000   = 8000,
    RTK_OPUS_SR_12000  = 12000,
    RTK_OPUS_SR_16000  = 16000,
    RTK_OPUS_SR_24000  = 24000,
    RTK_OPUS_SR_48000  = 48000
} rtk_opus_samplerate_t;

// Channel options
typedef enum
{
    RTK_OPUS_CHANNEL_MONO   = 1,
    RTK_OPUS_CHANNEL_STEREO = 2
} rtk_opus_channels_t;

// Application type options
typedef enum
{
    RTK_OPUS_APP_VOIP       = OPUS_APPLICATION_VOIP,
    RTK_OPUS_APP_AUDIO      = OPUS_APPLICATION_AUDIO,
    RTK_OPUS_APP_RESTRICTED = OPUS_APPLICATION_RESTRICTED_LOWDELAY
} rtk_opus_application_t;

// Bandwidth options
typedef enum
{
    RTK_OPUS_BW_AUTO        = OPUS_AUTO,
    RTK_OPUS_BW_NARROWBAND  = OPUS_BANDWIDTH_NARROWBAND,
    RTK_OPUS_BW_MEDIUMBAND  = OPUS_BANDWIDTH_MEDIUMBAND,
    RTK_OPUS_BW_WIDEBAND    = OPUS_BANDWIDTH_WIDEBAND,
    RTK_OPUS_BW_SUPERWIDEBAND = OPUS_BANDWIDTH_SUPERWIDEBAND,
    RTK_OPUS_BW_FULLBAND    = OPUS_BANDWIDTH_FULLBAND
} rtk_opus_bandwidth_t;

// Frame size options (unit: samples, need to calculate by sample rate)
typedef enum
{
    RTK_OPUS_FS_2_5MS  = 0, // sample_rate/400
    RTK_OPUS_FS_5MS    = 1, // sample_rate/200
    RTK_OPUS_FS_10MS   = 2, // sample_rate/100
    RTK_OPUS_FS_20MS   = 3, // sample_rate/50
    RTK_OPUS_FS_40MS   = 4, // sample_rate/25
    RTK_OPUS_FS_60MS   = 5, // 3*sample_rate/50
    RTK_OPUS_FS_80MS   = 6, // 4*sample_rate/50
    RTK_OPUS_FS_100MS  = 7, // 5*sample_rate/50
    RTK_OPUS_FS_120MS  = 8  // 6*sample_rate/50
} rtk_opus_framesize_t;

// Final range mode options
typedef enum
{
    RTK_OPUS_FINAL_RANGE_NONE = 0,   // Do not get final range
    RTK_OPUS_FINAL_RANGE_GET  = 1    // Get final range (Opus official API only supports get)
} rtk_opus_final_range_mode_t;

// VBR mode options
typedef enum
{
    RTK_OPUS_CBR = 0,   // Constant Bitrate: Encoder produces packets of constant size/bitrate.
    RTK_OPUS_VBR = 1    // Variable Bitrate: Encoder dynamically adjusts bitrate for better quality.
} rtk_opus_vbr_mode_t;

// Inband FEC options
typedef enum
{
    RTK_OPUS_FEC_OFF = 0, // Inband FEC disabled: No forward error correction data in packets.
    RTK_OPUS_FEC_ON  = 1  // Inband FEC enabled: Encoder adds FEC data for packet loss robustness.
} rtk_opus_fec_mode_t;

// DTX options
typedef enum
{
    RTK_OPUS_DTX_OFF = 0, // DTX disabled: Encoder always outputs packets, even during silence.
    RTK_OPUS_DTX_ON  = 1  // DTX enabled: Encoder suppresses output during silence to save bandwidth.
} rtk_opus_dtx_mode_t;

// Force channels options
typedef enum
{
    RTK_OPUS_FORCE_AUTO   = OPUS_AUTO, // Output channel count follows input (default).
    RTK_OPUS_FORCE_MONO   = 1,         // Force output to mono, regardless of input.
    RTK_OPUS_FORCE_STEREO = 2          // Force output to stereo, regardless of input.
} rtk_opus_forcechannels_t;

// Codec parameter struct
typedef struct
{
    rtk_opus_samplerate_t sampling_rate;   // Sample rate
    rtk_opus_channels_t channels;          // Channel count (number of channels in input PCM data)
    opus_int32 bitrate_bps;                // Bitrate (bps)
    rtk_opus_application_t application;    // Application type
    rtk_opus_bandwidth_t bandwidth;        // Bandwidth
    rtk_opus_framesize_t frame_size_type;  // Frame size type
    int frame_size;                        // Frame size (samples, calculated by sample rate and frame_size_type)
    int complexity;                        // Complexity (0~10)
    rtk_opus_vbr_mode_t use_vbr;           // VBR mode (CBR or VBR, see rtk_opus_vbr_mode_t)
    rtk_opus_fec_mode_t use_inbandfec;     // Inband FEC mode (enable/disable, see rtk_opus_fec_mode_t)
    rtk_opus_dtx_mode_t use_dtx;           // DTX mode (enable/disable, see rtk_opus_dtx_mode_t)
    rtk_opus_forcechannels_t forcechannels;// Force output channel count (see rtk_opus_forcechannels_t)
    int max_payload_bytes;                 // Max output packet bytes
    rtk_opus_final_range_mode_t final_range_mode; // Final range mode
    opus_uint32 final_range;               // Final range value (input/output, depending on mode)
} rtk_opus_param_t;


/**
 * @brief Set default values for rtk_opus_param_t
 * @param param [out] Pointer to rtk_opus_param_t struct
 *
 * 建议：如仅内部使用，可移到 rtk_opus_celt.c/rtk_opus_silk.c 内部实现，不对外提供。
 * 若需对外提供（如 demo/应用层需用），则保留在头文件。
 */
static inline void rtk_opus_param_set_default(rtk_opus_param_t *param)
{
    if (!param) { return; }
    param->sampling_rate = RTK_OPUS_SR_16000;
    param->channels = RTK_OPUS_CHANNEL_MONO;
    param->bitrate_bps = 16000;
    param->application = RTK_OPUS_APP_AUDIO;
    param->bandwidth = RTK_OPUS_BW_AUTO;
    param->frame_size_type = RTK_OPUS_FS_20MS;
    param->frame_size = 320; // 16000/50, 20ms
    param->complexity = 5;
    param->use_vbr = RTK_OPUS_CBR;
    param->use_inbandfec = RTK_OPUS_FEC_OFF;
    param->use_dtx = RTK_OPUS_DTX_OFF;
    param->forcechannels = RTK_OPUS_FORCE_MONO;
    param->max_payload_bytes = 1275;
    param->final_range_mode = RTK_OPUS_FINAL_RANGE_NONE;
    param->final_range = 0;
}

typedef void *(* pfunc_alloc_cb)(size_t size);
typedef void *(* pfunc_realloc_cb)(void *ptr, size_t size);
typedef void (* pfunc_opus_free_cb)(void *ptr);
typedef void (*opus_log_cb_t)(const char *msg);

void rtk_opus_register_alloc(pfunc_alloc_cb pfunc);
void rtk_opus_register_free(pfunc_opus_free_cb pfunc);
void rtk_opus_register_realloc(pfunc_realloc_cb pfunc);
// void rtk_opus_register_log(void *pfunc);
// void rtk_opus_register_error(void *pfunc);
void rtk_opus_register_log(opus_log_cb_t cb);


// CELT
/**
 * @brief 初始化 CELT 编码器，返回编码器句柄
 * @param param [in] 编码参数
 * @return 编码器句柄指针，失败返回 NULL
 */
void *rtk_celt_encoder_init(const rtk_opus_param_t *param);

/**
 * @brief 注销 CELT 编码器，释放资源
 * @param handle [in] 编码器句柄
 */
void  rtk_celt_encoder_destroy(void *handle);

/**
 * @brief CELT 编码（使用句柄）
 * @param handle    [in]  编码器句柄
 * @param param     [in]  编码参数
 * @param pcm       [in]  输入 PCM
 * @param data      [out] 输出比特流
 * @param out_bytes [out] 实际编码字节数
 * @return 0 成功，负数失败
 */
int   rtk_celt_encode(void *handle, const rtk_opus_param_t *param, const short *pcm,
                      unsigned char *data, int *out_bytes);

/**
 * @brief 初始化 CELT 解码器，返回解码器句柄
 * @param param [in] 解码参数
 * @return 解码器句柄指针，失败返回 NULL
 */
void *rtk_celt_decoder_init(const rtk_opus_param_t *param);

/**
 * @brief 注销 CELT 解码器，释放资源
 * @param handle [in] 解码器句柄
 */
void  rtk_celt_decoder_destroy(void *handle);

/**
 * @brief CELT 解码（使用句柄）
 * @param handle      [in]  解码器句柄
 * @param param       [in]  解码参数
 * @param data        [in]  输入比特流
 * @param len         [in]  比特流长度
 * @param pcm         [out] 输出 PCM
 * @param out_samples [out] 实际解码采样点数
 * @return 0 成功，负数失败
 */
int   rtk_celt_decode(void *handle, const rtk_opus_param_t *param, const unsigned char *data,
                      int len, short *pcm, int *out_samples);

// SILK
/**
 * @brief 初始化 SILK 编码器，返回编码器句柄
 * @param param [in] 编码参数
 * @return 编码器句柄指针，失败返回 NULL
 */
void *rtk_silk_encoder_init(const rtk_opus_param_t *param);

/**
 * @brief 注销 SILK 编码器，释放资源
 * @param handle [in] 编码器句柄
 */
void  rtk_silk_encoder_destroy(void *handle);

/**
 * @brief SILK 编码（使用句柄）
 * @param handle    [in]  编码器句柄
 * @param param     [in]  编码参数
 * @param pcm       [in]  输入 PCM
 * @param data      [out] 输出比特流
 * @param out_bytes [out] 实际编码字节数
 * @return 0 成功，负数失败
 */
int   rtk_silk_encode(void *handle, const rtk_opus_param_t *param, const short *pcm,
                      unsigned char *data, int *out_bytes);

/**
 * @brief 初始化 SILK 解码器，返回解码器句柄
 * @param param [in] 解码参数
 * @return 解码器句柄指针，失败返回 NULL
 */
void *rtk_silk_decoder_init(const rtk_opus_param_t *param);

/**
 * @brief 注销 SILK 解码器，释放资源
 * @param handle [in] 解码器句柄
 */
void  rtk_silk_decoder_destroy(void *handle);

/**
 * @brief SILK 解码（使用句柄）
 * @param handle      [in]  解码器句柄
 * @param param       [in]  解码参数
 * @param data        [in]  输入比特流
 * @param len         [in]  比特流长度
 * @param pcm         [out] 输出 PCM
 * @param out_samples [out] 实际解码采样点数
 * @return 0 成功，负数失败
 */
int   rtk_silk_decode(void *handle, const rtk_opus_param_t *param, const unsigned char *data,
                      int len, short *pcm, int *out_samples);


#ifdef __cplusplus
}
#endif

#endif // RTK_OPUS_H

