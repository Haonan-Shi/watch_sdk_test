/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/*
 * heap-4 style memory manager, ported from the vendor SDK tinyml_edge
 * mem_module/psRam_heap.c.
 *
 * Two changes for the Zephyr port:
 *   1. The backing ucHeap[] buffer is placed in the non-cacheable PSRAM1_NC
 *      memory-region (0x24300000) via the board overlay instead of internal
 *      .bss, so the
 *      ~100KB on-chip heap is no longer the ceiling for the tensor arena +
 *      MicroProfiler.  The section is NOLOAD (not zeroed/loaded at boot);
 *      psRamHeapInit() builds the free list at runtime on first allocation,
 *      by which point main() has already brought PSRAM up (edge_psram_init).
 *   2. Mutual exclusion uses the Zephyr scheduler lock (k_sched_lock/unlock)
 *      in place of FreeRTOS vTaskSuspendAll/xTaskResumeAll.  All callers run
 *      in thread context (edge worker thread + system workqueue), never ISR.
 */

#include <stdint.h>
#include <string.h>
#include <zephyr/kernel.h>

#include "trace.h"
#include "psRam_heap.h"
#include "psram_section.h"

/*----------------------------------------------------------------------------*
 *                          PSRAM backing store
 *----------------------------------------------------------------------------*/
/* The managed heap, placed in the non-cacheable PSRAM1_NC memory-region via the
 * shared psram_section.h macro.  NOLOAD section: do NOT rely on zero-init;
 * psRamHeapInit() builds the free list at runtime on first allocation. */
static uint8_t ucHeap[configTOTAL_psRAM_HEAP_SIZE] SECTION_PSRAM1_NC;

/*----------------------------------------------------------------------------*
 *                          heap-4 configuration
 *----------------------------------------------------------------------------*/
#define psRAM_portBYTE_ALIGNMENT        8
#define portBYTE_ALIGNMENT_MASK         (0x0007)

/* Assumes 8bit bytes. */
#define heapBITS_PER_BYTE               ((size_t) 8)

/* Max value that fits in a size_t type. */
#define heapSIZE_MAX                    (~((size_t) 0))

/* Check if adding a and b will result in overflow. */
#define heapADD_WILL_OVERFLOW(a, b)     ((a) > (heapSIZE_MAX - (b)))

/* Check if multiplying a and b will result in overflow. */
#define heapMULTIPLY_WILL_OVERFLOW(a, b) \
    (((a) > 0) && ((b) > (heapSIZE_MAX / (a))))

/* MSB of xBlockSize marks the allocation status of a block. */
#define heapBLOCK_ALLOCATED_BITMASK \
    (((size_t) 1) << ((sizeof(size_t) * heapBITS_PER_BYTE) - 1))
#define heapBLOCK_SIZE_IS_VALID(xBlockSize) \
    (((xBlockSize) & heapBLOCK_ALLOCATED_BITMASK) == 0)
#define heapBLOCK_IS_ALLOCATED(pxBlock) \
    (((pxBlock->xBlockSize) & heapBLOCK_ALLOCATED_BITMASK) != 0)
#define heapALLOCATE_BLOCK(pxBlock) \
    ((pxBlock->xBlockSize) |= heapBLOCK_ALLOCATED_BITMASK)
#define heapFREE_BLOCK(pxBlock) \
    ((pxBlock->xBlockSize) &= ~heapBLOCK_ALLOCATED_BITMASK)

#define PSRAM_ASSERT(e)                                                        \
    do {                                                                       \
        if (!(e)) {                                                            \
            DBG_DIRECT("(" #e ") psram assert failed! Func: %s. Line: %d.",    \
                       __func__, __LINE__);                                    \
            for (;;) { }                                                       \
        }                                                                      \
    } while (0)

/* Define the linked list structure used to link free blocks in order of
 * their memory address. */
typedef struct A_BLOCK_LINK
{
    struct A_BLOCK_LINK *pxNextFreeBlock;  /*<< The next free block in the list. */
    size_t xBlockSize;                     /*<< The size of the free block. */
} psRam_BlockLink_t;

/* Block sizes must not get too small. */
#define heapMINIMUM_BLOCK_SIZE  ((size_t)(xHeapStructSize << 1))

/* The size of the structure placed at the beginning of each allocated memory
 * block must be correctly byte aligned. */
static const size_t xHeapStructSize = (sizeof(psRam_BlockLink_t) +
                                       ((size_t)(psRAM_portBYTE_ALIGNMENT - 1))) &
                                      ~((size_t) portBYTE_ALIGNMENT_MASK);

/* Create a couple of list links to mark the start and end of the list. */
static psRam_BlockLink_t xStart;
static psRam_BlockLink_t *pxEnd = NULL;

/* Bookkeeping (does not account for fragmentation). */
static size_t xFreeBytesRemaining = 0U;
static size_t xMinimumEverFreeBytesRemaining = 0U;
static size_t xNumberOfSuccessfulAllocations = 0;
static size_t xNumberOfSuccessfulFrees = 0;

static void psRamInsertBlockIntoFreeList(psRam_BlockLink_t *pxBlockToInsert);
static void psRamHeapInit(void);

/*----------------------------------------------------------------------------*/

void *psRamPortMalloc(size_t xWantedSize)
{
    psRam_BlockLink_t *pxBlock;
    psRam_BlockLink_t *pxPreviousBlock;
    psRam_BlockLink_t *pxNewBlockLink;
    void *pvReturn = NULL;
    size_t xAdditionalRequiredSize;

    k_sched_lock();
    {
        /* First call sets up the list of free blocks. */
        if (pxEnd == NULL)
        {
            psRamHeapInit();
        }

        if (xWantedSize > 0)
        {
            /* The wanted size must be increased so it can hold a
             * psRam_BlockLink_t plus alignment padding. */
            xAdditionalRequiredSize = xHeapStructSize + psRAM_portBYTE_ALIGNMENT -
                                      (xWantedSize & portBYTE_ALIGNMENT_MASK);

            if (heapADD_WILL_OVERFLOW(xWantedSize, xAdditionalRequiredSize) == 0)
            {
                xWantedSize += xAdditionalRequiredSize;
            }
            else
            {
                xWantedSize = 0;
            }
        }

        /* The top bit of xBlockSize tracks ownership, so it must be free. */
        if (heapBLOCK_SIZE_IS_VALID(xWantedSize) != 0)
        {
            if ((xWantedSize > 0) && (xWantedSize <= xFreeBytesRemaining))
            {
                /* Traverse from the start (lowest address) until a block of
                 * adequate size is found. */
                pxPreviousBlock = &xStart;
                pxBlock = xStart.pxNextFreeBlock;

                while ((pxBlock->xBlockSize < xWantedSize) &&
                       (pxBlock->pxNextFreeBlock != NULL))
                {
                    pxPreviousBlock = pxBlock;
                    pxBlock = pxBlock->pxNextFreeBlock;
                }

                /* If the end marker was reached no block was large enough. */
                if (pxBlock != pxEnd)
                {
                    /* Return the space, jumping over the block link. */
                    pvReturn = (void *)(((uint8_t *) pxPreviousBlock->pxNextFreeBlock) +
                                        xHeapStructSize);

                    /* Take the block out of the free list. */
                    pxPreviousBlock->pxNextFreeBlock = pxBlock->pxNextFreeBlock;

                    /* Split the block if it is larger than required. */
                    if ((pxBlock->xBlockSize - xWantedSize) > heapMINIMUM_BLOCK_SIZE)
                    {
                        pxNewBlockLink = (void *)(((uint8_t *) pxBlock) + xWantedSize);

                        pxNewBlockLink->xBlockSize = pxBlock->xBlockSize - xWantedSize;
                        pxBlock->xBlockSize = xWantedSize;

                        psRamInsertBlockIntoFreeList(pxNewBlockLink);
                    }

                    xFreeBytesRemaining -= pxBlock->xBlockSize;

                    if (xFreeBytesRemaining < xMinimumEverFreeBytesRemaining)
                    {
                        xMinimumEverFreeBytesRemaining = xFreeBytesRemaining;
                    }

                    /* The block now belongs to the application and has no
                     * "next" block. */
                    heapALLOCATE_BLOCK(pxBlock);
                    pxBlock->pxNextFreeBlock = NULL;
                    xNumberOfSuccessfulAllocations++;
                }
            }
        }
    }
    (void) k_sched_unlock();

    return pvReturn;
}
/*----------------------------------------------------------------------------*/

void psRamFree(void *pv)
{
    uint8_t *puc = (uint8_t *) pv;
    psRam_BlockLink_t *pxLink;

    if (pv != NULL)
    {
        /* The block link sits immediately before the returned memory. */
        puc -= xHeapStructSize;
        pxLink = (void *) puc;

        PSRAM_ASSERT(heapBLOCK_IS_ALLOCATED(pxLink) != 0);
        PSRAM_ASSERT(pxLink->pxNextFreeBlock == NULL);

        if (heapBLOCK_IS_ALLOCATED(pxLink) != 0)
        {
            if (pxLink->pxNextFreeBlock == NULL)
            {
                /* No longer allocated - return it to the heap. */
                heapFREE_BLOCK(pxLink);

                k_sched_lock();
                {
                    xFreeBytesRemaining += pxLink->xBlockSize;
                    psRamInsertBlockIntoFreeList(((psRam_BlockLink_t *) pxLink));
                    xNumberOfSuccessfulFrees++;
                }
                (void) k_sched_unlock();
            }
        }
    }
}
/*----------------------------------------------------------------------------*/

size_t psRamGetFreeHeapSize(void)
{
    return xFreeBytesRemaining;
}
/*----------------------------------------------------------------------------*/

size_t psRamGetMinimumEverFreeHeapSize(void)
{
    return xMinimumEverFreeBytesRemaining;
}
/*----------------------------------------------------------------------------*/

void *psRamPortCalloc(size_t xNum, size_t xSize)
{
    void *pv = NULL;

    if (heapMULTIPLY_WILL_OVERFLOW(xNum, xSize) == 0)
    {
        pv = psRamPortMalloc(xNum * xSize);

        if (pv != NULL)
        {
            (void) memset(pv, 0, xNum * xSize);
        }
    }

    return pv;
}
/*----------------------------------------------------------------------------*/

static void psRamHeapInit(void)
{
    psRam_BlockLink_t *pxFirstFreeBlock;
    uint8_t *pucAlignedHeap;
    uint32_t uxAddress;
    size_t xTotalHeapSize = configTOTAL_psRAM_HEAP_SIZE;

    /* Ensure the heap starts on a correctly aligned boundary. */
    uxAddress = (uint32_t) ucHeap;

    if ((uxAddress & portBYTE_ALIGNMENT_MASK) != 0)
    {
        uxAddress += (psRAM_portBYTE_ALIGNMENT - 1);
        uxAddress &= ~((uint32_t) portBYTE_ALIGNMENT_MASK);
        xTotalHeapSize -= uxAddress - (uint32_t) ucHeap;
    }

    pucAlignedHeap = (uint8_t *) uxAddress;

    /* xStart holds a pointer to the first free block. */
    xStart.pxNextFreeBlock = (void *) pucAlignedHeap;
    xStart.xBlockSize = (size_t) 0;

    /* pxEnd marks the end of the list, placed at the end of the heap. */
    uxAddress = ((uint32_t) pucAlignedHeap) + xTotalHeapSize;
    uxAddress -= xHeapStructSize;
    uxAddress &= ~((uint32_t) portBYTE_ALIGNMENT_MASK);
    pxEnd = (psRam_BlockLink_t *) uxAddress;
    pxEnd->xBlockSize = 0;
    pxEnd->pxNextFreeBlock = NULL;

    /* Start with a single free block covering the whole usable heap. */
    pxFirstFreeBlock = (psRam_BlockLink_t *) pucAlignedHeap;
    pxFirstFreeBlock->xBlockSize = (size_t)(uxAddress - (uint32_t) pxFirstFreeBlock);
    pxFirstFreeBlock->pxNextFreeBlock = pxEnd;

    xMinimumEverFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;
    xFreeBytesRemaining = pxFirstFreeBlock->xBlockSize;

    (void) xNumberOfSuccessfulAllocations;
    (void) xNumberOfSuccessfulFrees;
}
/*----------------------------------------------------------------------------*/

static void psRamInsertBlockIntoFreeList(psRam_BlockLink_t *pxBlockToInsert)
{
    psRam_BlockLink_t *pxIterator;
    uint8_t *puc;

    /* Iterate to the block with a higher address than the one being inserted. */
    for (pxIterator = &xStart; pxIterator->pxNextFreeBlock < pxBlockToInsert;
         pxIterator = pxIterator->pxNextFreeBlock)
    {
        /* Nothing to do - just find the right position. */
    }

    /* Merge with the preceding block if contiguous. */
    puc = (uint8_t *) pxIterator;

    if ((puc + pxIterator->xBlockSize) == (uint8_t *) pxBlockToInsert)
    {
        pxIterator->xBlockSize += pxBlockToInsert->xBlockSize;
        pxBlockToInsert = pxIterator;
    }

    /* Merge with the following block if contiguous. */
    puc = (uint8_t *) pxBlockToInsert;

    if ((puc + pxBlockToInsert->xBlockSize) == (uint8_t *) pxIterator->pxNextFreeBlock)
    {
        if (pxIterator->pxNextFreeBlock != pxEnd)
        {
            pxBlockToInsert->xBlockSize += pxIterator->pxNextFreeBlock->xBlockSize;
            pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock->pxNextFreeBlock;
        }
        else
        {
            pxBlockToInsert->pxNextFreeBlock = pxEnd;
        }
    }
    else
    {
        pxBlockToInsert->pxNextFreeBlock = pxIterator->pxNextFreeBlock;
    }

    /* If the inserted block plugged a gap it was merged on both sides, so its
     * pxNextFreeBlock is already set; avoid pointing it at itself. */
    if (pxIterator != pxBlockToInsert)
    {
        pxIterator->pxNextFreeBlock = pxBlockToInsert;
    }
}
/*----------------------------------------------------------------------------*/
