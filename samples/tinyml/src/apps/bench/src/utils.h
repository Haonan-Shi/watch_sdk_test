/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * Empty Zephyr shim: the vendor parser/model_process include utils.h for helper
 * declarations that are not exercised in this port.
 */

#ifndef TINYML_EDGE_UTILS_SHIM_H
#define TINYML_EDGE_UTILS_SHIM_H

/* The vendor parser/model_process rely on utils.h transitively pulling in the
 * standard string-formatting helpers (e.g. snprintf). */
#include <stdio.h>

#endif /* TINYML_EDGE_UTILS_SHIM_H */
