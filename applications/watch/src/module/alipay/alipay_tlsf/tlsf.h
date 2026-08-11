/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

#ifndef INCLUDED_tlsf
#define INCLUDED_tlsf

/* -------------------------------------------------------------------------
 * Upstream notice for the vendored TLSF v3.1 sources, reproduced verbatim
 * from the original distribution as required by its terms. Do not alter.
 * ------------------------------------------------------------------------- */

/*
** Two Level Segregated Fit memory allocator, version 3.1.
** Written by Matthew Conte
**  http://tlsf.baisoku.org
**
** Based on the original documentation by Miguel Masmano:
**  http://www.gii.upv.es/tlsf/main/docs
**
** This implementation was written to the specification
** of the document, therefore no GPL restrictions apply.
**
** Copyright (c) 2006-2016, Matthew Conte
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the copyright holder nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL MATTHEW CONTE BE LIABLE FOR ANY
** DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stddef.h>

/*
 * Namespace all public TLSF symbols with an "alipay_" prefix so this
 * copy can co-exist with HoneyGUI's own copy of TLSF
 * (modules/honeygui/realgui/misc/tlsf/), which exports the same symbol
 * names. Both copies link into the final image -- alipay uses this one
 * for its csi_heap, HoneyGUI uses its own for the widget heap.
 *
 * Implemented via #define so the upstream tlsf.c / tlsf.h sources stay
 * unmodified (apart from this header block); the rename happens
 * automatically anywhere tlsf.h is included.
 */
#define tlsf_create             alipay_tlsf_create
#define tlsf_create_with_pool   alipay_tlsf_create_with_pool
#define tlsf_destroy            alipay_tlsf_destroy
#define tlsf_get_pool           alipay_tlsf_get_pool
#define tlsf_add_pool           alipay_tlsf_add_pool
#define tlsf_remove_pool        alipay_tlsf_remove_pool
#define tlsf_malloc             alipay_tlsf_malloc
#define tlsf_memalign           alipay_tlsf_memalign
#define tlsf_realloc            alipay_tlsf_realloc
#define tlsf_free               alipay_tlsf_free
#define tlsf_block_size         alipay_tlsf_block_size
#define tlsf_size               alipay_tlsf_size
#define tlsf_align_size         alipay_tlsf_align_size
#define tlsf_block_size_min     alipay_tlsf_block_size_min
#define tlsf_block_size_max     alipay_tlsf_block_size_max
#define tlsf_pool_overhead      alipay_tlsf_pool_overhead
#define tlsf_alloc_overhead     alipay_tlsf_alloc_overhead
#define tlsf_walk_pool          alipay_tlsf_walk_pool
#define tlsf_check              alipay_tlsf_check
#define tlsf_check_pool         alipay_tlsf_check_pool
/* Non-public helper that's accidentally non-static in upstream tlsf.c */
#define test_ffs_fls            alipay_tlsf_test_ffs_fls

#if defined(__cplusplus)
extern "C" {
#endif

/* tlsf_t: a TLSF structure. Can contain 1 to N pools. */
/* pool_t: a block of memory that TLSF can manage. */
typedef void *tlsf_t;
typedef void *pool_t;

/* Create/destroy a memory pool. */
tlsf_t tlsf_create(void *mem);
tlsf_t tlsf_create_with_pool(void *mem, size_t bytes);
void tlsf_destroy(tlsf_t tlsf);
pool_t tlsf_get_pool(tlsf_t tlsf);

/* Add/remove memory pools. */
pool_t tlsf_add_pool(tlsf_t tlsf, void *mem, size_t bytes);
void tlsf_remove_pool(tlsf_t tlsf, pool_t pool);

/* malloc/memalign/realloc/free replacements. */
void *tlsf_malloc(tlsf_t tlsf, size_t bytes);
void *tlsf_memalign(tlsf_t tlsf, size_t align, size_t bytes);
void *tlsf_realloc(tlsf_t tlsf, void *ptr, size_t size);
void tlsf_free(tlsf_t tlsf, void *ptr);

/* Returns internal block size, not original request size */
size_t tlsf_block_size(void *ptr);

/* Overheads/limits of internal structures. */
size_t tlsf_size(void);
size_t tlsf_align_size(void);
size_t tlsf_block_size_min(void);
size_t tlsf_block_size_max(void);
size_t tlsf_pool_overhead(void);
size_t tlsf_alloc_overhead(void);

/* Debugging. */
typedef void (*tlsf_walker)(void *ptr, size_t size, int used, void *user);
void tlsf_walk_pool(pool_t pool, tlsf_walker walker, void *user);
/* Returns nonzero if any internal consistency check fails. */
int tlsf_check(tlsf_t tlsf);
int tlsf_check_pool(pool_t pool);

#if defined(__cplusplus)
};
#endif

#endif
