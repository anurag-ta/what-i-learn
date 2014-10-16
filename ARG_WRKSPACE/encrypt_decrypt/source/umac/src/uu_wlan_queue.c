/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_queue.c                                                **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                            **
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
/* $Revision: 1.15 $ */

#include "uu_wlan_main.h"
#include "uu_datatypes.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_mac_defaults.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_agg.h" 
#include "uu_mac_scheduling.h"

uu_void uu_wlan_start_tx_send(uu_wlan_umac_context_t *mac_context,
        uu_wlan_txq_info_t *txq,
        struct sk_buff *skb)
{
    uu_int16 sch_index;
    struct ieee80211_hdr *hdr;
    uu_wlan_q_entry_t *temp_node;
    static int i = 0;

    hdr = (struct ieee80211_hdr *)skb->data;
    if(NULL == hdr)
    {
        UU_WLAN_LOG_ERROR(("*******UNEXPECTED - Null hdr in uu_wlan_start_tx_send\n"));
        return;
    }
    UU_WLAN_LOG_DEBUG(("***** UMAC uu_wlan_start_tx_send - Frame Control0: %x\n", hdr->frame_control & 0x00FF));
    if(NULL == txq)
    {
        UU_WLAN_LOG_ERROR(("*******UNEXPECTED - Null txq in uu_wlan_start_tx_send\n"));
        return;
    }

    temp_node = (uu_wlan_q_entry_t *) kmalloc(sizeof(uu_wlan_q_entry_t), GFP_ATOMIC);
    if (!temp_node)
    {
        UU_WLAN_LOG_ERROR(("*******UNEXPECTED - MEM alloc failed\n"));
        return;
    }

    temp_node->skb = skb;

    //printk("Adding node to Tx Q in UMAC- ATOMIC node: %p, skb: %p, skb len: %d \n", temp_node, skb, skb->len);
    /*if((NULL == &temp_node->node) || (NULL == &txq->acq_head))
    {
        printk("list add tail faied\n");
        return;
    } */
    list_add_tail(&temp_node->node, &txq->acq_head);

    /** Increment the queue length */
    txq->txq_len++;

    /* Scheduling */
#if 1 
    if(ieee80211_is_data(hdr->frame_control) && (!is_multicast_ether_addr(hdr->addr1)) && (!                    ieee80211_is_mgmt(hdr->frame_control)))
    {
        //printk("----------In scheduling----------\n");
        uu_umac_sch_stati_AID[uu_mac_sch_get_index_from_macaddr(&skb->data[UU_RA_OFFSET])]->uu_stati_queue_t[skb_get_queue_mapping(skb)].no_of_frames_in_queue++;
        sch_index = uu_mac_sch_umac_sch(uu_wlan_umac_get_ac(skb_get_queue_mapping(skb)));
        //printk("Scheduled index is %d\n", sch_index);
        //printk("------------------------------\n");
        uu_wlan_tx_send_normal(mac_context, &mac_context->tx_q[uu_wlan_umac_get_ac(skb_get_queue_mapping(skb))][sch_index]);
    }
    else
    {
        uu_wlan_tx_send_normal(mac_context, txq);
    }
#endif
    // uu_wlan_tx_send_normal(mac_context, txq);
    // The above line is commented in multiple receiver case.

    /** Check the frame is part of aggreagated frame or not. Handle aggreagation here */

} /* uu_wlan_start_tx_send */


/* EOF */

