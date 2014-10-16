/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_buf.h                                          **
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

#ifndef __UU_WLAN_BUF_H__
#define __UU_WLAN_BUF_H__

#ifdef __KERNEL__
#include <linux/slab.h> 
#else
#include <string.h>
#include <unistd.h>
#include <memory.h>
#include <malloc.h>
#include <assert.h>
#endif

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_frame.h"


#define UU_IS_SAME_MAC_ADDR(x,y)            (memcmp((uu_uchar *)(x), (uu_uchar *)(y), 6) == 0)
#define UU_COPY_MAC_ADDR(x,y)               UU_WLAN_COPY_BYTES(x, y, 6)
#define UU_WLAN_MEM_SET(dst, len)           memset(dst, 0, len)
#define UU_WLAN_COPY_BYTES(dst, src, len)   memcpy(dst, src, len);


#ifdef __KERNEL__ /* Kernel space */
#define uu_wlan_alloc_memory(size)          (void*)kmalloc(size, GFP_KERNEL);
#define uu_wlan_free_memory(address)        kfree(address)
#else /* User space */
#define uu_wlan_alloc_memory(size)          (void*)malloc(size)
#define uu_wlan_free_memory(address)        free(address)
#endif


extern uu_int32 uu_wlan_free_rx_frame_buff(uu_wlan_rx_frame_info_t * buff);


extern uu_int32 uu_wlan_sec_frame_encrypt(uu_wlan_tx_frame_info_t *tx_frame);
extern uu_int32 uu_wlan_sec_frame_decrypt(uu_wlan_rx_frame_info_t* frame_info);


#endif /* __UU_WLAN_BUF_H__ */


