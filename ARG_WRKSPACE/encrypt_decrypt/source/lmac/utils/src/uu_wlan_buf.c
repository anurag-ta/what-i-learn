/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_buf.c                                          **
 **                                                                           **
 ** Copyright © 2013, Uurmi Systems                                          **
 ** All rights reserved.                                                      **
 ** http://www.uurmi.com                                                      **
 **                                                                           **
 ** All information contained herein is property of Uurmi Systems             **
 ** unless otherwise explicitly mentioned.                                    **
 **                                                                           **
 ** The intellectual and technical concepts in this file are proprietary      **
 ** to Uurmi Systems and may be covered by granted or in process national     **
 ** and international patents and are protect by trade secrets and            **
 ** copyright law.                                                            **
 **                                                                           **
 ** Redistribution and use in source and binary forms of the content in       **
 ** this file, with or without modification are not permitted unless          **
 ** permission is explicitly granted by Uurmi Systems.                        **
 **                                                                           **
 ******************************************************************************/

#ifdef USER_SPACE
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#else
#include <linux/slab.h>
#endif

#include "uu_datatypes.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_utils.h"


#if 0
uu_void* uu_wlan_alloc_memory(uu_uint32  size)
{
#ifdef USER_SPACE
    return((void*)malloc(size));
#else
    return((void*)kmalloc(size, GFP_KERNEL));
#endif
}

uu_void uu_wlan_free_memory(uu_void *address)
{
#ifdef USER_SPACE
    free(address);
#else
    kfree(address);
#endif
}

uu_void UU_WLAN_COPY_BYTES(uu_void *dst, uu_void *src, uu_uint32 len)
{
#if 1
    memcpy(dst, src, len);
#else
    uu_uint32 *d = dst;
    uu_uint32 *s = src;
    uu_uint8 *d8 = dst;
    uu_uint8 *s8 = src;
    uu_uint32  wc;

    /* If src or dst are aligned to 4-bytes */
    if (!(((uu_uint32)dst & 0x03) || ((uu_uint32)src & 0x03)))
    {
        wc = len >> 2;
        d8 = dst+(wc*4);
        s8 = src+(wc*4);
        len = len & 3;
        while (wc--)
        {
            *d++ = *s++;
        }
    }

    while (len--)
    {
        *d8++ = *s8++;
    }
#endif
}


uu_void UU_WLAN_MEM_SET(uu_void *dst, uu_uint32 len)
{
#if 1
    memset(dst, 0, len);
#else

    uu_uint32 *d = dst;
    uu_uint8 *d8 = dst;
    uu_uint32  wc;

    /* If src or dst are aligned to 4-bytes */
    if (!((uu_uint32)dst & 0x03))
    {
        wc = len >> 2;
        d8 = dst+(wc*4);
        len = len & 3;
        while (wc--)
        {
            *d++ = 0;
        }
    }

    while (len--)
    {
        *d8++ = 0;
    } 
#endif
    return;
}
#endif


uu_int32 uu_wlan_sec_frame_encrypt(uu_wlan_tx_frame_info_t *frame)
{
    return UU_SUCCESS;
} /* uu_wlan_sec_frame_encrypt */

uu_int32 uu_wlan_sec_frame_decrypt(uu_wlan_rx_frame_info_t *frame_info)
{
    return UU_SUCCESS;
}



/* EOF */

