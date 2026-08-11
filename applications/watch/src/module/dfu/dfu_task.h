/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef _DFU_TASK_H_
#define _DFU_TASK_H_


/** @defgroup PERIPH_DFU_TASK Peripheral App Task
  * @brief Peripheral DFU Task
  * @{
  */
extern void *dfu_evt_queue_handle;
void dfu_main_task_queue_create(void);

/**
 * @brief  Initialize DFU task
 * @return void
 */
void dfu_task_init(void);


/** End of PERIPH_DFU_TASK
* @}
*/


#endif
