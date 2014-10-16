/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_agg.c                                                 **
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
/* $Revision: 1.35 $ */

#include <linux/version.h>

#include "uu_wlan_reg.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_main.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_queue.h"
#include "uu_mac_drv.h"
#include "uu_wlan_agg.h"
#include "uu_mac_tx.h"
#include "uu_wlan_buf.h"
#include "uu_mac_scheduling.h"

extern uu_uint8 uu_wlan_ba_ready_flag_g[8];

#ifdef UU_WLAN_TPC
uu_int32 __send_frame(struct sk_buff *skb);
#elif defined UU_WLAN_DFS
uu_int32 __send_frame(struct sk_buff *skb);
#else
static uu_int32 __send_frame(struct sk_buff *skb);
#endif


/* TODO: Do the changes in priority, according to this table */
/**
 *   ACI      FWK Q   LMAC
 *  -----     -----   ----
 *    0         2      BE
 *    1         3      BK (Lowest priority)
 *    2         1      VI
 *    3         0      VO
 */

uu_char uu_wlan_umac_get_ac(int queue)
{
    switch (queue) {
        case 0:
            return UU_WLAN_AC_VO;
        case 1:
            return UU_WLAN_AC_VI;
        case 2:
            return UU_WLAN_AC_BE;
        case 3:
            return UU_WLAN_AC_BK;
        default:
            return UU_WLAN_AC_BK;
    }
}


uu_char uu_wlan_lmacq_to_fwkq(int queue)
{
    switch (queue) {
        case UU_WLAN_AC_VO:
            return 0;
        case UU_WLAN_AC_VI:
            return 1;
        case UU_WLAN_AC_BE:
            return 2;
        case UU_WLAN_AC_BK:
            return 3;
        default:
            return 3;
    }
}


/** TODO: For 11ac, need to send individual frames also as an AMPDU.
 * Consider this while enhancing this for 11ac.
*/
#ifdef UU_WLAN_TPC
uu_int32 __send_frame(struct sk_buff *skb)
#elif defined UU_WLAN_DFS
uu_int32 __send_frame(struct sk_buff *skb)
#else
static uu_int32 __send_frame(struct sk_buff *skb)
#endif
{
    uu_uint32 avail_buf_len;
    uu_wlan_tx_frame_info_t *frame_lp;
    uu_int32 frmlen;
    uu_uint8 ac;
    struct sk_buff *skb_frame_info;

#ifdef UU_WLAN_BQID
    struct ieee80211_hdr *hdr;
    hdr = (struct ieee80211_hdr *)skb->data;

    if (ieee80211_is_beacon(hdr->frame_control))
    {
        ac = UU_WLAN_BEACON_QID;
    }
    else
    {
        ac = uu_wlan_umac_get_ac(skb_get_queue_mapping(skb));
    }
#else
    ac = uu_wlan_umac_get_ac(skb_get_queue_mapping(skb));
#endif

    frmlen = skb->len + FCS_LEN;

    /** Check the available buffer length. If the available buffer length is greater than
      the frame length and frame info skb len then send the packet. */
    avail_buf_len = uu_wlan_tx_get_available_buffer_platform(ac);
    UU_WLAN_LOG_DEBUG(("avail_buf len in __send_frame %d\n", avail_buf_len));
    if (avail_buf_len > (frmlen + (sizeof(uu_wlan_tx_frame_info_t))))
    {
        UU_WLAN_LOG_DEBUG((" buffer is available \n"));
        /** TODO - Debug and fix this issue */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,30)
        /* Leave space for 4-byte FCS, to be filled by LMAC */
        //skb_put(skb, 4);
#endif
        /** Form the skb_frame_info */
        /** Allocate an skb */
        skb_frame_info = __dev_alloc_skb(sizeof(uu_wlan_tx_frame_info_t), GFP_ATOMIC);
        //skb_reserve(skb_frame_info, (sizeof(uu_wlan_tx_frame_info_t)));

        frame_lp = (uu_wlan_tx_frame_info_t *)skb_frame_info->data;
        memset(frame_lp, 0, sizeof(uu_wlan_tx_frame_info_t));
        frame_lp->ac = ac;
        frame_lp->txvec.ch_bndwdth = UU_WLAN_CH_BW_DEAFAULT;
        /* Fill frame info structure */
        uu_wlan_setup_tx_frame_info(uu_wlan_context_gp->hw, skb , frmlen, (uu_wlan_tx_frame_info_t *)skb_frame_info->data);

        memcpy(skb_put(skb_frame_info, sizeof(uu_wlan_tx_frame_info_t)), frame_lp, sizeof(uu_wlan_tx_frame_info_t));

        /** Add the mpdu next to this skb */
        skb_frame_info->next = skb;

        /** Send frame to device */
        uu_wlan_send_to_device(ac, skb_frame_info);
    }
    else
    {
        UU_WLAN_LOG_ERROR((" Buffer is not available \n"));
        return -1;
    }

    return 0;

} /* __send_frame */


#if 0 /** TODO Handle tx status */
static uu_void  __give_txstatus(struct sk_buff *skb)
{
    struct ieee80211_tx_info *txi;
    txi = IEEE80211_SKB_CB(skb);

    ieee80211_tx_info_clear_status(txi);
    if (!(txi->flags & IEEE80211_TX_CTL_NO_ACK))
        txi->flags |= IEEE80211_TX_STAT_ACK;
    ieee80211_tx_status_irqsafe(uu_wlan_context_gp->hw, skb);
}
#endif

static uu_int32 uu_wlan_tx_send_aggr(uu_wlan_umac_context_t *mac_context,
        uu_wlan_txq_info_t *txq, uu_uint8 count)
{
    uu_wlan_q_entry_t *temp_node, *next_node;
    struct sk_buff *skb;
    struct sk_buff *skb_frame_info = UU_NULL;
    struct sk_buff *tskb = UU_NULL;
    uu_wlan_tx_frame_info_t *frame_lp = UU_NULL;
    uu_int32 frmlen;
    uu_uint8 ac = 0;
    uu_uint8 *macaddr;
    uu_uint16 index;
    uu_bool  first_frame = true;
    uu_uint16 avail_buf_len = 0;
    uu_uint8 pad_len;
    macaddr = (uu_uint8 *)kmalloc(IEEE80211_MAC_ADDR_LEN*sizeof(uu_uint8),GFP_ATOMIC);

    list_for_each_entry_safe(temp_node, next_node, &txq->acq_head, node)
    {
        skb = temp_node->skb;
        pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(skb->len);

        ac = uu_wlan_umac_get_ac(skb_get_queue_mapping(skb));
        UU_COPY_MAC_ADDR(macaddr, &skb->data[UU_RA_OFFSET]);
        index = uu_mac_sch_get_index_from_macaddr(macaddr);
        if(index >= UU_WLAN_MAX_ASSOCIATIONS)
        {
            UU_WLAN_LOG_DEBUG(("Wrong index in send_aggr\n"));
            return -1;
        }

        if (first_frame)
        {
            first_frame = false;
            frmlen = skb->len + FCS_LEN;

            avail_buf_len = uu_wlan_tx_get_available_buffer_platform(ac);
            if (avail_buf_len <= (frmlen + pad_len + sizeof(uu_wlan_tx_frame_info_t)))
            {
                /* Can't send, as enough device buffer is not available */
                return -1;
            }
            //UU_WLAN_LOG_DEBUG(("Available skb headroom: %d,  tailroom: %d\n", skb->data - skb->head, skb->end - skb->data));

            /** Form the skb_frame_info */
            /** Allocate an skb */
            skb_frame_info = __dev_alloc_skb(sizeof(uu_wlan_tx_frame_info_t), GFP_ATOMIC);

            /** Add the mpdu next to this skb */
            skb_frame_info->next = skb;

            frame_lp = (uu_wlan_tx_frame_info_t *)skb_frame_info->data;
            memset(frame_lp, 0, sizeof(uu_wlan_tx_frame_info_t));
            frame_lp->ac = ac;
            frame_lp->txvec.ch_bndwdth = UU_WLAN_CH_BW_DEAFAULT;
            frame_lp->txvec.is_aggregated = 1;

            uu_wlan_setup_tx_frame_info(uu_wlan_context_gp->hw, skb , frmlen, (uu_wlan_tx_frame_info_t *)skb_frame_info->data);  /* Fill frame info structure */
            memcpy(skb_put(skb_frame_info, sizeof(uu_wlan_tx_frame_info_t)), frame_lp ,sizeof(uu_wlan_tx_frame_info_t));

//#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,30)
            /* Leave space for 4-byte FCS at the end of MPDU, to be filled by LMAC */
            skb_put(skb, 4);
//#endif
            frame_lp->ampdu_info.mpdu_info[frame_lp->aggr_count].mpdu_len = frmlen;
            frame_lp->ampdu_info.mpdu_info[frame_lp->aggr_count].pad_len = pad_len;
            frame_lp->aggr_count = 1;
            frame_lp->frameInfo.framelen = frmlen + pad_len;

            avail_buf_len = avail_buf_len - (frmlen + pad_len + sizeof(uu_wlan_tx_frame_info_t));
        }
        else
        {
            frmlen = skb->len + FCS_LEN;

            if (avail_buf_len > (frmlen + pad_len))
            {
                tskb->next = skb;

//#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,30)
                /* Leave space for 4-byte FCS at the end of MPDU, to be filled by LMAC */
                skb_put(skb, 4);
//#endif
                frame_lp->ampdu_info.mpdu_info[frame_lp->aggr_count].mpdu_len = frmlen;
                frame_lp->ampdu_info.mpdu_info[frame_lp->aggr_count].pad_len = pad_len;
                frame_lp->aggr_count++;
                frame_lp->frameInfo.framelen += frmlen + pad_len;


                avail_buf_len = avail_buf_len - (frmlen - pad_len);
            }
            else
            {
                UU_WLAN_LOG_ERROR(("required buffer is not available send till here \n"));
                break;
            }
        }

        tskb = skb;

        list_del(&temp_node->node); 
        list_add_tail(&temp_node->node, &uu_wlan_context_gp->tx_q[ac][index].acq_status_head);

        txq->txq_len--;
        uu_wlan_context_gp->tx_q[ac][index].txq_len++; 
    }

    uu_wlan_send_to_device(ac, skb_frame_info);
    return 0;
} /* uu_wlan_tx_send_aggr */


uu_int32 uu_wlan_tx_send_normal(uu_wlan_umac_context_t *mac_context,
        uu_wlan_txq_info_t *txq)
{
    uu_wlan_q_entry_t *temp_node, *next_node;
    struct sk_buff *skb;
    uu_int32  ret;
    uu_uint8  count = 0;
    uu_bool   is_tid_aggr = false;
    uu_uint8  tid = 0;
    u8 *qc;
    uu_uint8 ac = 0;
    uu_uint8 macaddr[IEEE80211_MAC_ADDR_LEN];
    uu_uint16 index;
    struct ieee80211_hdr *hdr;
    uu_bool do_aggregation = UU_FALSE;

    /* TODO: Protect the list with spinlock, as list-add & list-delete happen in different tasklet/workQ */
    if(NULL == &txq->acq_head)
    {
        UU_WLAN_LOG_DEBUG(("&txq->acq_head is NULL\n"));
        return -1;
    }

    list_for_each_entry_safe(temp_node, next_node, &txq->acq_head, node)
    {
        skb = temp_node->skb;
        if(NULL == skb)
        {
            UU_WLAN_LOG_DEBUG(("NULL skb in send normal\n"));
            return -1;
        }
        hdr = (struct ieee80211_hdr *)skb->data;
        if(NULL == hdr)
        {
            UU_WLAN_LOG_DEBUG(("NULL hdr in send normal\n"));
            return -1;
        }
        index = 0;
        if (ieee80211_is_data(hdr->frame_control) &&
            (!is_multicast_ether_addr(hdr->addr1)) &&
            (!ieee80211_is_mgmt(hdr->frame_control)))
        {
            UU_WLAN_LOG_DEBUG(("sending a data frame.... \n"));
            UU_COPY_MAC_ADDR(macaddr, &skb->data[UU_RA_OFFSET]);
            index = uu_mac_sch_get_index_from_macaddr(macaddr);
            if(index >= UU_WLAN_MAX_ASSOCIATIONS)
            {
                UU_WLAN_LOG_DEBUG(("Error in finding index from mac addr \n"));
                return -1;
            }
        }

        /* Do not send the frame immediately, if the Queue size is below lower-water mark */
        /* TODO: Also use timestamp, to prevent MSDU timeouts. Check the all 4 Qs, to prevent timeouts. Do Q checks, on getting tx-status also */
        /* TODO: Check whether aggregation is enabled for this STA+TID. Current logic always checks the top of Q. So this logic has to be handled separately. */
        if (do_aggregation)
        {
            is_tid_aggr = true;
        }

        if (ieee80211_is_data_qos(hdr->frame_control))
        {
            qc = ieee80211_get_qos_ctl(hdr);
            tid = *qc & IEEE80211_QOS_CTL_TID_MASK;
        }

        /* If management frame, send it without waiting for aggregation */
        /* TODO: Consider having separate Q for management frames (esp. Beacon frames) */
        if ((uu_wlan_ba_ready_flag_g[tid] == 0) || is_multicast_ether_addr(hdr->addr1) || ieee80211_is_mgmt(hdr->frame_control) || !is_tid_aggr)
        {
            ret = __send_frame(skb);
            if (ret < 0) /* There are no free descriptors */
            {
                /* Keep the frame in the list, as there are no free descriptors in LMAC */
                return 0;
            }

            /** Add node to status queue */

#ifdef UU_WLAN_BQID
            /** For Beacon and ATIM frames, needs to be queued under Beacon Queue */
            if (ieee80211_is_beacon(hdr->frame_control))
            {
                ac = UU_WLAN_BEACON_QID;
            }
            else
            {
                ac = uu_wlan_umac_get_ac(skb_get_queue_mapping(skb));
            }
#else
            ac = uu_wlan_umac_get_ac(skb_get_queue_mapping(skb));
#endif

            list_del(&temp_node->node);
            list_add_tail(&temp_node->node, &uu_wlan_context_gp->tx_q[ac][index].acq_status_head);

            //kfree(temp_node);
            txq->txq_len--;
            uu_wlan_context_gp->tx_q[ac][index].txq_len++;

            continue;
        }

        /* TODO: If there are no more frames, the last frame will be in Q forever? */
        /* TODO: The upper & lower water mark should be on the LMAC Q. Not UMAC Q */
        if (txq->txq_len < UU_WLAN_LOWER_WATER_MARK)
        {
            /* If data-frame and Q currently is under lower-watermark, do NOT send immediately */
            UU_WLAN_LOG_ERROR(("Tx Q under lower water mark\n"));
            return 0;
        }

        /* Came here, because of one of the reasons:
         **  - the top of the Q has timestamp constraint hit
         **  - This AC Q is above lower watermark level
         **  - Aggregation is not enabled for this frame
         */
        /* TODO: Count the number of frames that can be aggreated (data frames with matching STA+TID) */
        if (count == 0)
        {
            //RA_to_match[6]; /* Copy RA here */
            //TID_to_match;   /* Copy TID here */
            count++;
        }
        else /* TODO: Match the data frames with STA+TID, for aggregation */
        {
            if (1 && (count < 3)) //RA_to_match && TID_to_match )
            {
                UU_WLAN_LOG_DEBUG(("Tx Q aggr counting: %d\n", count));
                count++;
                /* TODO: Check the remote capabilities */
            }
            else /* TODO: Consider non-consecutive frames also, for aggregation */
            {
                if ((count > 1) /* or TODO: if 11ac*/) /* Do aggregation, for 'count' frames */
                {
                    UU_WLAN_LOG_DEBUG(("Tx Q aggregating: %d\n", count));
                    ret = uu_wlan_tx_send_aggr(mac_context, txq, count);
                    return ret; /* Can't continue the loop, as more than 1 node is removed */
                }
                else /* This data frame can't be aggregated. Send individually */
                {
                    UU_WLAN_LOG_DEBUG(("Tx Q sending without aggregation\n"));
                    ret = __send_frame(skb);
                    if (ret < 0) /* There are no free descriptors */
                    {
                        /* Keep the frame in the list, as there are no free descriptors in LMAC */
                        return 0;
                    }

                    list_del(&temp_node->node);
                    list_add_tail(&temp_node->node, &uu_wlan_context_gp->tx_q[ac][index].acq_status_head);

                    //kfree(temp_node);
                    txq->txq_len--;
                    uu_umac_sch_stati_AID[uu_mac_sch_get_index_from_macaddr(&skb->data[UU_RA_OFFSET])]->uu_stati_queue_t[skb_get_queue_mapping(skb)].no_of_frames_in_queue--;

                    uu_wlan_context_gp->tx_q[ac][index].txq_len++;
                }
            }
        }
    } /* list_for_each_entry_safe */
    return 0;
} /* uu_wlan_tx_send_normal */


/* EOF */

