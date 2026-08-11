/*
 * Copyright (c) 2026, Realtek Semiconductor Corporation
 *
 * SPDX-License-Identifier: LicenseRef-Realtek-5-Clause
 */

/* Add Includes here */
#include "platform_types.h"
#include "platform_list.h"

void plt_list_push(plt_list_t *plist, void *plist_e)
{
    plt_list_e_t *pelement = (plt_list_e_t *)plist_e;
    pelement->pnext = NULL;
    if (plist->count == 0)
    {
        plist->pfirst = pelement;
    }
    else
    {
        plist->plast->pnext = pelement;
    }
    plist->plast = pelement;
    plist->count++;
}

void *plt_list_pop(plt_list_t *plist)
{
    plt_list_e_t *pelement = NULL;
    if (plist->count > 0)
    {
        if (plist->count == 1)
        {
            plist->plast = NULL;
        }
        plist->count--;
        pelement = plist->pfirst;
        plist->pfirst = pelement->pnext;
    }
    return pelement;
}

void plt_list_insert(plt_list_t *plist, void *plist_e, void *plist_e_new)
{
    plt_list_e_t *pelement = (plt_list_e_t *)plist_e;
    plt_list_e_t *pelement_new = (plt_list_e_t *)plist_e_new;

    if (pelement == NULL)
    {
        pelement_new->pnext = plist->pfirst;
        plist->pfirst = pelement_new;
        if (plist->count == 0)
        {
            plist->plast = pelement_new;
        }
    }
    else
    {
        pelement_new->pnext = pelement->pnext;
        pelement->pnext = pelement_new;
        if (pelement_new->pnext == NULL)
        {
            plist->plast = pelement_new;
        }
    }
    plist->count++;
}

void plt_list_delete(plt_list_t *plist, void *plist_e_previous, void *plist_e)
{
    plt_list_e_t *pelement = (plt_list_e_t *)plist_e;
    if (plist_e_previous == NULL)//(plist->pfirst == pelement)
    {
        plist->pfirst = pelement->pnext;
        if (pelement->pnext == NULL)
        {
            plist->plast = NULL;
        }
    }
    else
    {
        /*
        plt_list_e_t * pprevious = plist->pfirst;
        while(pprevious->pnext != pelement)
            pprevious = pprevious->pnext;
        */
        plt_list_e_t *pprevious = (plt_list_e_t *)plist_e_previous;
        pprevious->pnext = pelement->pnext;
        if (pelement->pnext == NULL)
        {
            plist->plast = pprevious;
        }
    }
    plist->count--;
}
