/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _AUDIO_LED_
#define _AUDIO_LED_

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup  AUDIO_LED    Audio LED
    * @brief This file introduces the Audio LED
    * @{
    */

/*============================================================================*
 *                         Constants
 *============================================================================*/

/** @defgroup AUDIO_LED_Exported_constants AUDIO_LED Exported Constants
  * @{
  */

typedef enum
{
    LED_COLOR_RED                         = 0x0,
    LED_COLOR_GREEN                       = 0x1,
    LED_COLOR_BLUE                        = 0x2,
    LED_COLOR_PURPLE                      = 0x3,
    LED_COLOR_YELLOW                      = 0x4,
    LED_COLOR_LIGHT_BLUE                  = 0x5,
    LED_COLOR_ORANGE                      = 0x6,
} T_AUDIO_LED_COLOR;

typedef enum
{
    LED_CHANGE_LIGHTER              = 0x0,
    LED_CHANGE_DARKER               = 0x1,
} T_AUDIO_LED_COLOR_MODE;

typedef enum
{
    AUDIO_LED_DISPLAY_FIX_PATTERN               = 0x0,
    AUDIO_LED_DISPLAY_PATTERN_CHANGED_BY_GAIN   = 0x1,
} T_AUDIO_LED_DISPLAY_MODE;

#define FREQ_NUM                            5            //Number of sampling frequencies
#define REFRESH_INTERVAL                    100          //ms, sampling interval

/** End of group AUDIO_LED_Exported_constants
  * @}
  */
extern uint32_t freq_table[FREQ_NUM];                    //Sample frequency
/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup AUDIO_LED_Functions AUDIO_LED Exported Functions
  * @{
  */

/**
    * @brief  Clear the recorded audio gain. It is recommended to call when cutting songs or re-playing music.
    * \xrefitem  Added_API_2_14_0_0 "Added Since 2.14.0.0" "Added API"
    * @return none
    */
void audio_gain_clear(void);

/**
    * @brief  Audio gain transfer to RGB
    * \xrefitem  Added_API_2_14_0_0 "Added Since 2.14.0.0" "Added API"
    * @param  freq_num: Number of samping frequencies
    * @param  left_gain: Left gain
    * @param  right_gain: Right gain
    * @return none
    */
void audio_gain_transfer_to_rgb(uint32_t freq_num, uint16_t *left_gain, uint16_t *right_gain);
/**
    * @brief  Init rgb display
    * \xrefitem  Added_API_2_14_0_0 "Added Since 2.14.0.0" "Added API"
    * @param  display_mode: @ref T_AUDIO_LED_DISPLAY_MODE
    * @param  color_mode: @ref T_AUDIO_LED_COLOR_MODE, this parameter only works in fixed pattern mode
    * @param  fixed_pattern_changed_t_ms: This parameter takes effect only when a fixed pattern mode is used
    * @return none
    */
void audio_led_init(T_AUDIO_LED_DISPLAY_MODE display_mode, T_AUDIO_LED_COLOR_MODE color_mode,
                    uint32_t fixed_pattern_changed_t_ms);

#ifdef __cplusplus
}
#endif

#endif /* _AUDIO_LED_ */

/** @} */ /* End of group AUDIO_LED_Functions */
/** @} */ /* End of group AUDIO_GAIN_RGB */
