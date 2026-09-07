/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _APP_AI_RECORD_PROCESS_H_
#define _APP_AI_RECORD_PROCESS_H_


#ifdef __cplusplus
extern "C" {
#endif
/*============================================================================*
  *                           Header Files
  *============================================================================*/

#include <stdint.h>
#include <stdbool.h>
#include "audio.h"


/** @defgroup APP_AI_RECORD_AI App AI AI_RECORD
  * @brief
  * @{
  */

#define BD_ADDR_LENGTH       6

void app_ai_record_process_init(void);

/**
    * @brief        This function can stop the voice capture.
    * @return       void
    */
void app_ai_record_stop_recording(uint8_t bd_addr[6], bool is_ext);

/**
    * @brief        This function can start the voice capture.
    * @return       void
    */
void app_ai_record_start_recording(uint8_t bd_addr[6], bool is_ext);

bool app_ai_record_is_recording(void);


/** @} End of APP_AI_AI_RECORD */

#ifdef __cplusplus
}
#endif

#endif //_APP_AI_RECORD_PROCESS_H_
