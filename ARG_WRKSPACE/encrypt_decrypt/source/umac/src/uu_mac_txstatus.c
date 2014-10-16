/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_tx_status.c                                           **
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
/* $Revision: 1.9 $ */

#include "uu_wlan_main.h"
#include "uu_mac_txstatus.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_reg.h"
#include "uu_mac_scheduling.h"
#ifdef UU_WLAN_TPC
#include "uu_wlan_umac_tpc.h"
#endif


uu_void uu_wlan_send_tx_status(int ac, int j)
{
    uu_uint16 k;
    uu_uint8 q;
    uu_wlan_q_entry_t *temp_node, *next_node;
    struct sk_buff *skb;
    struct ieee80211_hdr *hdr;
    struct ieee80211_tx_info *txi;

    /* Number of times retransmission occured */
    uu_uint8 tx_frame_count;
    /* Short retry limit or Long retry limit */
    uu_uint8 fallback_retry;

#ifdef UU_WLAN_TPC
    uu_uint8 failed;
#endif

    /** Read from the ii status queue and send status to mac framework */
    for(k=0; k<UU_WLAN_MAX_ASSOCIATIONS; k++)
    {
        list_for_each_entry_safe(temp_node, next_node, &uu_wlan_context_gp->tx_q[ac][k].acq_status_head, node)
        {
            skb = temp_node->skb;
            hdr = (struct ieee80211_hdr *) skb->data;

            if(!((hdr->seq_ctrl>>4) == uu_wlan_tx_status_info_g[ac][j].seqno))
                continue;

            UU_WLAN_LOG_DEBUG(("LMAC-UMAC : seq number - %x : Time - %ld \n", (hdr->seq_ctrl>>4), jiffies));

            if (ieee80211_is_data(hdr->frame_control) &&
                (!is_multicast_ether_addr(hdr->addr1)) &&
                (!ieee80211_is_mgmt(hdr->frame_control)))
            {
                uu_uint16  index;

                /* Update average allocated rate*/
                index = uu_mac_sch_get_index_from_macaddr(&skb->data[UU_RA_OFFSET]);

                if(index < UU_WLAN_MAX_ASSOCIATIONS)
                {
                    if(uu_mac_sch_update_avg_alloc_rate(ac, index, skb->len) < 0)
                    {
                        UU_WLAN_LOG_DEBUG(("Failed to update average allocated rate\n"));
                        return;
                    }
                    uu_umac_sch_stati_AID[index]->uu_stati_queue_t[ac].no_of_frames_in_queue--;
                    UU_WLAN_LOG_DEBUG(("TX_STAT: SKB length is %d, index is %d, ac is %d, num frames is %d \n",
                        skb->len, index, ac, uu_umac_sch_stati_AID[index]->uu_stati_queue_t[ac].no_of_frames_in_queue));
                }
            }
            txi = IEEE80211_SKB_CB(skb);
            ieee80211_tx_info_clear_status(txi);

            /* Giving tx_status to Minstrel RC */
            tx_frame_count = (uu_wlan_tx_status_info_g[ac][j].retry_count) + 1;
            fallback_retry = (skb->len > UU_WLAN_RTS_THRESHOLD_R) ?
                              (dot11_long_retry_count >> 1) : (dot11_short_retry_count >> 1);
            if ((tx_frame_count > fallback_retry) && (txi->status.rates[1].idx >= 0))
            {
                txi->status.rates[0].count = fallback_retry;
                txi->status.rates[1].count = tx_frame_count - fallback_retry;
            }
            else
            {
                txi->status.rates[0].count = tx_frame_count;
                txi->status.rates[1].idx = -1;
                txi->status.rates[1].count = 0;
            }

            if (uu_wlan_tx_status_info_g[ac][j].status)
                txi->flags |= IEEE80211_TX_STAT_ACK;
            else
                txi->flags &= ~IEEE80211_TX_STAT_ACK;

            ieee80211_tx_status_irqsafe(uu_wlan_context_gp->hw, skb);
            /** Send status to framework */
            list_del(&temp_node->node);
            kfree(temp_node);
#ifdef UU_WLAN_TPC
            failed = uu_wlan_tx_status_info_g[qid][j].retry_count;
            uu_wlan_calc_wtfar_stats (uu_wlan_tx_status_info_g[qid][j].bandwidth, qid, failed, (uu_wlan_tx_status_flags_g[qid] & (1 << j)) ? failed + 1 : failed);
#endif

            uu_wlan_tx_reset_status_flags_platform(ac,j);
            q = uu_wlan_lmacq_to_fwkq(ac);
            q_state.num_pkts[q]--;
            UU_WLAN_LOG_DEBUG(("Num of frames in AC %d is %d \n", q, q_state.num_pkts[q]));
            if((q_state.q_stopped[q]) && (q_state.num_pkts[q] < q_state.max_pkts[q]))
            {
                UU_WLAN_LOG_DEBUG(("STOPQ: wake queue for ac %d \n", q));
                q_state.q_stopped[q] = 0;
                UU_WLAN_LOG_DEBUG(("wake up queue \n"));
                ieee80211_wake_queue(uu_wlan_context_gp->hw,q);
            }
        }
    }
}

/* EOF */
