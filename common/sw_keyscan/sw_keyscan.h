/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef __SW_KEYSCAN_H
#define __SW_KEYSCAN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

/** @addtogroup SW_KEYSCAN SW KEYSCAN
  * @brief SW KEYSCAN driver module.
  * @{
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/


/** @defgroup SW_KEYSCAN_Exported_constants SW KEYSCAN Exported Constants
  * @{
  */

#define KEYSCAN_DEBOUNCE_TIME           (30)       //!< Debounce time, the unit is ms.
#define KEYSCAN_SCAN_INTERVAL_TIME      (50)       //!< Scan interval time, the unit is ms.
#define KEYSCAN_SCAN_RELEASE_TIME       (30)       //!< Release detect time, the unit is ms.

/** End of group SW_KEYSCAN_Exported_constants
  * @}
  */

/*============================================================================*
 *                         Types
 *============================================================================*/


/** @defgroup SW_KEYSCAN_Exported_Types SW KEYSCAN Exported Types
  * @{
  */

/**
 * @brief Keyscan data parameters.
 *
 */
typedef struct
{
    uint8_t  length;        /*!< The number of keys pressed. */
    uint16_t *key_data;     /*!< Key data, the upper 8 bits are row data, and the lower 8 bits are column data. */
} T_KEYSCAN_DATA;

/**
  * @brief  Keyscan scan end callback function.
  */
typedef void (*KEYSCAN_SCAN_END_CB)(T_KEYSCAN_DATA);

/**
  * @brief  Keyscan release callback function.
  */
typedef void (*KEYSCAN_RELEASE_CB)(void);

/**
 * @brief Keyscan initialize parameters.
 *
 */
typedef struct
{
    uint8_t row_size;                           /*!< Specifies keyscan row size. */
    uint8_t column_size;                        /*!< Specifies keyscan column size. */
    uint8_t *keyscan_row_pins;                  /*!< Specify pins of keyscan row. */
    uint8_t *keyscan_column_pins;               /*!< Specify pins of keyscan column. */
    KEYSCAN_SCAN_END_CB keyscan_scan_end_cb;    /*!< Keyscan scan end callback function. */
    KEYSCAN_RELEASE_CB keyscan_release_cb;      /*!< Keyscan release callback function. */
} T_KEYSCAN_INIT_PARA;

/** End of group SW_KEYSCAN_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/


/** @defgroup SW_KEYSCAN_Exported_Functions SW KEYSCAN Exported Functions
  * @{
  */

/**
 * sw_keyscan.h
 *
 * \brief   Initializes the keyscan according to the specified
 *          parameters in the KeyScan_InitStruct.
 *
 * \param[in]  keyscan_init_para: Pointer to a T_KEYSCAN_INIT_PARA structure that
 *             contains the configuration information for keyscan.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void keyscan_demo(void)
 * {
 *     uint8_t keyscan_row_pin[4] = {P2_2, P2_3, P2_4, P2_5};
 *     uint8_t keyscan_column_pin[4] = {P0_0, P0_1, P0_2, P0_3};
 *
 *     T_KEYSCAN_INIT_PARA keyscan_init_para;
 *     keyscan_init_para.row_size = 4;
 *     keyscan_init_para.column_size = 4;
 *     keyscan_init_para.keyscan_row_pins = keyscan_row_pin;
 *     keyscan_init_para.keyscan_column_pins = keyscan_column_pin;
 *     keyscan_init_para.keyscan_scan_end_cb = scan_end_cb;
 *     keyscan_init_para.keyscan_release_cb = release_cb;
 *     sw_keyscan_init(&keyscan_init_para);
 *
 * }
 * \endcode
 */
void sw_keyscan_init(T_KEYSCAN_INIT_PARA *keyscan_init_para);

/**
 * sw_keyscan.h
 *
 * \brief   Config the filter data function.
 *
 * \param[in] filter_data: Config the data to be filtered.
 *            The upper 8 bits are row data, and the lower 8 bits are column data.
 * \param[in] filter_data_enable: Enable or disable keyscan filter data function.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void keyscan_demo(void)
 * {
 *     sw_keyscan_filter_data_config(0x101, true);
 * }
 * \endcode
 */
void sw_keyscan_filter_data_config(uint16_t filter_data, bool filter_data_enable);

/**
 * sw_keyscan.h
 *
 * \brief   Clear all filter data.
 *
 * <b>Example usage</b>
 * \code{.c}
 *
 * void keyscan_demo(void)
 * {
 *     sw_keyscan_clear_all_filter_data();
 * }
 * \endcode
 */
void sw_keyscan_clear_all_filter_data(void);

#ifdef __cplusplus
}
#endif

#endif

/** @} */ /* End of group SW_KEYSCAN_Exported_Functions */
/** @} */ /* End of group SW_KEYSCAN */
