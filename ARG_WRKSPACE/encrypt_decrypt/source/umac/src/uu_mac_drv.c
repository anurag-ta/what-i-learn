/*******************************************************************************
**                                                                            **
** File name :  uu_mac_drv.c                                                  **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                           **
** All rights reserved.                                                       **
** http://www.uurmi.com                                                       **
**                                                                            **
** All information contained herein is property of Uurmi Systems              **
** unless otherwise explicitly mentioned.                                     **
**                                                                            **
** The intellectual and technical concepts in this file are proprietary       **
** to Uurmi Systems and may be covered by granted or in process national      **
** and international patents and are protect by trade secrets and             **
** copyright law.                                                             **
**                                                                            **
** Redistribution and use in source and binary forms of the content in        **
** this file, with or without modification are not permitted unless           **
** permission is explicitly granted by Uurmi Systems.                         **
**                                                                            **
*******************************************************************************/
/* $Revision: 1.10 $ */

#include "uu_wlan_main.h"
#include "uu_mac_drv.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_frame.h"
#include "uu_mac_txstatus.h"
#include "uu_wlan_recv.h"


uu_void uu_wlan_send_to_device(uu_uint8 ac, struct sk_buff *skb_to_device)
{
    int i;
    struct sk_buff *temp_skb;
    uu_uchar  *addrs[UU_WLAN_MAX_MPDU_IN_AMPDU];
    uu_uint32 len[UU_WLAN_MAX_MPDU_IN_AMPDU];
    uu_uint8  pad_len;
    uu_uint8  pad[4] = {0};

    memset(addrs, 0, sizeof(addrs));
    memset(len, 0, sizeof(len));
    for (i=0, temp_skb = skb_to_device; (temp_skb != NULL) && (i != UU_WLAN_MAX_MPDU_IN_AMPDU); i++, temp_skb = temp_skb->next)
    {
        pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(temp_skb->len);

        /* Add padding to skb end, to align to 4-byte boundary */
        /* Note: Padding for last MPDU of the AMPDU -- varies between 11n & 11ac. To be taken care of, in LMAC */
        if (pad_len)
        {
            memcpy(skb_put(temp_skb, pad_len), pad, pad_len);
        }

        len[i] = temp_skb->len;
        addrs[i] = temp_skb->data;
    }

    uu_wlan_tx_write_buffer_platform(ac, addrs, len);
}


/* EOF */

