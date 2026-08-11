/*
*      Copyright (C) 2020 Apple Inc. All Rights Reserved.
*
*      Find My Network ADK is licensed under Apple Inc's MFi Sample Code License Agreement,
*      which is contained in the License.txt file distributed with the Find My Network ADK,
*      and only to those who accept that license.
*/

#if CONFIG_REALTEK_FINDMY_FEATURE_SUPPORT
#if CONFIG_REALTEK_FINDMY_SUPPORT_NFC
#include "fmna_nfc_platform.h"
#include "fmna_constants.h"
#include "fmna_util.h"

fmna_ret_code_t fmna_nfc_platform_init(void)
{
    //TODO: Initialize NFC peripheral, and activate.

    return 0;
}
#endif
#endif
