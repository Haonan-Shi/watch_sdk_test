/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * app_flags.h - TinyML image-classification sample tunables (precompiled header).
 *
 * Image geometry and the normalize API live in ic_normalize.h; this header
 * carries only the on-device knobs specific to the Zephyr port.
 */
#ifndef _APP_FLAGS_H_
#define _APP_FLAGS_H_

/* Tensor arena for the 96x96 image model. The vendor reference measured
 * arena_used ~= 47 KB after AllocateTensors(); 68 KB adds ~21 KB margin. */
#define IC_ARENA_SIZE   (68u * 1024u)

#endif /* _APP_FLAGS_H_ */
