/*******************************************************************************
**                                                                            **
** File name :  uu_mac_tx.c                                                   **
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
/* $Revision: 1.21 $ */

#include <linux/version.h>

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_limits.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_main.h"
#include "uu_mac_tx.h"
#include "uu_wlan_buf.h"
#include "uu_mac_scheduling.h"
#ifdef UU_WLAN_DFS
#include "uu_wlan_umac_dfs.h"
#endif
#ifdef UU_WLAN_TPC
#include "uu_wlan_umac_tpc.h"
#endif

/* TODO: Move these to a header file */
#define get_max(a, b) (((a) > (b)) ? (a) : (b));
#define get_min(a, b) (((a) > (b)) ? (b) : (a));


/* Beacon sending */

uu_void uu_wlan_send_beacon(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif)
{
    struct sk_buff *skb;
    uu_wlan_umac_context_t *mac_context = hw->priv;

#ifdef UU_WLAN_TPC
    uu_uint8 power_constraint_value;
    uu_int8 bandwidth, i;
    int utpl_max;
    /* For Transmit Power Control */
    int tpc_level_dBm[CBW80_80] = {0};
#endif

    if(hw)
    {
        /* Getting beacon from MAC FW */
        skb = ieee80211_beacon_get(hw, vif);
        if (skb == NULL)
            return;

#ifdef UU_WLAN_TPC
        /**
         * Define power constraint (Uurmi Definition - Its implementation dependent, and not defined in specification.
         *
         * We define power constraint in this way.
         *
         * Power Constraint = max (mitigation requirement,
         *                          (Max transmit power as per the regulatory domain - min (4 times of the base TPC of the Voice for the current
         *                                                                                  operating bandwidth for data/management frames,
         *                                                                                  max TPC Supported)))
         */

        /** for bss operating bandwidth */
        /** TODO: Need to update as per the new framework for 80 and 160/80+80 MHz also.
         *  NOTE : Changes should be made as per the new structure */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
        if(hw->conf.channel_type == NL80211_CHAN_HT40MINUS || hw->conf.channel_type == NL80211_CHAN_HT40PLUS)
            bandwidth = CBW40;
        else
            bandwidth = CBW20;
#else
    struct ieee80211_channel *channel = hw->conf.chandef.chan;
    enum nl80211_channel_type channel_type =
        cfg80211_get_chandef_type(&hw->conf.chandef);

        if(channel_type == NL80211_CHAN_HT40MINUS || channel_type == NL80211_CHAN_HT40PLUS)
            bandwidth = CBW40;
        else
            bandwidth = CBW20;
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0) */

#ifdef UU_WLAN_DFSTPC_DEBUG
        /** Just for testing 80, 160, and 80+80 MHz operation for VHT */
        bandwidth = CBW80_80;
#endif
        /** TODO: We need to add for 80, 160, 80_80 */

        if (bandwidth == CBW80_80)
        {
            bandwidth = CBW160;
        }

        /** For AP we consider from framework */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
        for (i = 0; i <= bandwidth; i++)
            tpc_level_dBm[i] = hw->conf.channel->max_power;
#else
        for (i = 0; i <= bandwidth; i++)
            tpc_level_dBm[i] = hw->conf.chandef.chan->max_power;
#endif

        /** Checks if MAX_TPC has changed for any of the bandwidth */
        if (prev_tpc_dBm[bandwidth] >= 0)
        {
            for (i = 0; i <= bandwidth; i++)
            {
                if (prev_tpc_dBm[i] != tpc_level_dBm[i])
                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("TPC Level gets changed\n");
#endif
                    prev_tpc_dBm[i] = tpc_level_dBm[i];
                    uu_wlan_configure_utpl_threshold (prev_tpc_dBm[i], i);
                }
		    }
	    }
	    /** Configuring TPC for the first time */
	    else
	    {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("When its first time, then configuring UTPL_Threshold\n");
#endif
			prev_tpc_dBm[bandwidth] = tpc_level_dBm[bandwidth];
            uu_wlan_configure_utpl_threshold (prev_tpc_dBm[bandwidth], bandwidth);
        }

        if (uu_wlan_btpl_ac_g[0] < UU_WLAN_TPLevel9)
        {
            utpl_max = uu_wlan_utpl_threshold_11n_g[bandwidth];
        }
        else
        {
            utpl_max = uu_wlan_utpl_threshold_11ac_g[bandwidth];
        }

        /** Now, determine the threshold as per the max_power dBm */

        /**
         * We define the local maximum transmit power to be less or equal to
         * 2x times the value of B_TPL (AC_VO, bandwidth), where x is a natural number.
         *
         * Local maximum transmit power for VHT Transmit Power Envelope element is defined as:
         * Local maximum transmit power [bandwidth], in dBm = 2 × ((x × 3) + B_TPL (AC_VO, bandwidth))
         *
         * But the local maximum transmit power should not exceed maximum
         * regulatory transmit power minus mitigation factor.
         * The Local Power constraint for Power Constraint element is defined as:
         * Local Power Constraint = Maximum (Mitigation factor, Maximum regulatory
         *                              transmit power - ((x × 3) + B_TPL (AC_VO, bandwidth)))
         */

        power_constraint_value
            = get_max (UU_WLAN_TPC_MITIGATION, (tpc_level_dBm[bandwidth]
                        - get_min (((UU_WLAN_BTPL_MUL_FORSTA * 3) + uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(bandwidth * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO]]),
                        uu_wlan_transmit_power_dBm[utpl_max])));
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("power constraint value : %d\n", power_constraint_value);
#endif
        uu_uint8 ie_pwrconst_len = 2 + sizeof (power_constraint_value);
        uu_int8 *pos;
        pos = skb_put(skb, ie_pwrconst_len);
        *pos++ = 32; /* Power constraint element id */
        *pos++ = sizeof (power_constraint_value);
        *pos = power_constraint_value;

        /** Refer to P802.11ac, section 8.4.2.164 */
        /** Sending VHT Transmit Power Envelope at 5 GHz */
        /** For 5 GHz only */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
        if (hw->conf.channel->band)
#else
        if (hw->conf.chandef.chan->band)
#endif
        {
            uu_int8 ie_vht_txpwr_envlp;
            uu_int8 txpwr_info;
            uu_int8 local_max_txpwr_count;
            uu_int8 local_max_txpwr_units;
            uu_int8 i = 0;

            /** local maximum transmit power constraint values: 0 for 20 MHz,
               1 for 20/40 MHz, 2 for 20/40/80 MHz, 3 for 20/40/80/160 MHz */
            local_max_txpwr_count = bandwidth;
            local_max_txpwr_units = 0; /* 0 indicates EIRP unit */

            /** Default : element id, length, tx_pwr_information, local_max_txpwr_20,
               Optional : local_max_txpwr_40, local_max_txpwr_80, local_max_txpwr_160 */
            ie_vht_txpwr_envlp = 4 + bandwidth;

#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("power constraint value : %d\n", power_constraint_value);
#endif
            txpwr_info = (((local_max_txpwr_units & UU_WLAN_LOCAL_MXTPWR_UNIT) << UU_WLAN_LOCAL_MXTPWR_UNIT_SHIFT)
                                | (local_max_txpwr_count & UU_WLAN_LOCAL_MXTPWR_COUNT));

            pos = skb_put(skb, ie_vht_txpwr_envlp);
            *pos++ = 195; /** VHT Tx Power Envelope element id */
            *pos++ = ie_vht_txpwr_envlp - 2;
            *pos++ = txpwr_info;
            while (local_max_txpwr_count)
            {
                /** NOTE: As per the standard, the local maximum transmit power
                 * value varies from -64 dBm to 63 dBm with a step-size of 0.5.
                 * So, we need to double the actual value to have a resolution of 0.5
                 */

                *pos++ = 2 * (get_min (((UU_WLAN_BTPL_MUL_FORSTA * 3) +
                                                uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO]]),
                                    uu_wlan_transmit_power_dBm[utpl_max]));
                i++;
                local_max_txpwr_count--;
            }
            *pos = 2 * (get_min (((UU_WLAN_BTPL_MUL_FORSTA * 3) +
                                            uu_wlan_transmit_power_dBm[uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + UU_WLAN_AC_VO]]),
                                uu_wlan_transmit_power_dBm[utpl_max]));
        }
#endif /* UU_WLAN_TPC */

#ifdef UU_WLAN_BQID
        if (uu_wlan_tx_start(hw, skb, &mac_context->tx_q[UU_WLAN_BEACON_QID][0]) != 0) 
#else
        if (uu_wlan_tx_start(hw, skb, &mac_context->tx_q[skb_get_queue_mapping(skb)][0]) != 0) 
#endif
        {
            UU_WLAN_LOG_ERROR(("UURMIDRIVER - BEACON Beacon TX failed\n"));
            dev_kfree_skb_any(skb);
        }
    }
} /* uu_wlan_send_beacon  */


/**
 * uu_wlan_beacon_update_trigger_work - Work queue for triggering beacons.
 * Work queue is intialized for AP and is triggered in bss_change_info.
 */
uu_void uu_wlan_beacon_update_trigger_work(struct work_struct *work)
{
    uu_wlan_umac_context_t *mac_context = container_of(work, uu_wlan_umac_context_t,
            beacon_update_trigger);
    if(mac_context)
    {
        //UU_WLAN_LOG_DEBUG(("UURMIDRIVER BEACON WORKQUEUE -- vif is %d\n", mac_context->vif->type));
        uu_wlan_send_beacon(mac_context->hw, mac_context->vif);
    }
    else
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER BEACON WORKQUEUE no data found \n"));
    }

} /* uu_wlan_beacon_update_trigger_work */


uu_int32 uu_wlan_setup_tx_flags(struct sk_buff *skb)
{
    struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
    uu_int32 flags = 0;


    if (tx_info->flags & IEEE80211_TX_CTL_NO_ACK)
        flags |= UU_WLAN_TX_CTL_NO_ACK;

    if (tx_info->flags & IEEE80211_TX_CTL_REQ_TX_STATUS)
        flags |= UU_WLAN_TX_CTL_REQ_TX_STATUS;

    if (tx_info->flags & IEEE80211_TX_CTL_ASSIGN_SEQ)
        flags |= UU_WLAN_TX_CTL_ASSIGN_SEQ;

    if (tx_info->flags & IEEE80211_TX_CTL_INJECTED)
        flags |= UU_WLAN_TX_CTL_INJECTED;

    if (tx_info->flags & IEEE80211_TX_RC_USE_RTS_CTS)
        flags |= UU_WLAN_TX_CTL_USE_RTS;

    if (tx_info->flags & IEEE80211_TX_RC_USE_CTS_PROTECT)
        flags |= UU_WLAN_TX_CTL_USE_CTS;

    return flags;
} /* uu_wlan_setup_tx_flags */


uu_void uu_wlan_setup_tx_frame_info(struct ieee80211_hw *hw,
        struct sk_buff *skb,
        uu_int32 framelen,
        uu_wlan_tx_frame_info_t *fi)
{
    uu_wlan_key_type_t keytype;
    uu_int32 flags;

    /** Filling of rate control inforamtion before transmitting the frame */
    uu_wlan_setup_frame_info_rc(uu_wlan_context_gp->hw, skb , framelen, fi);

    flags = uu_wlan_setup_tx_flags(skb);
    flags |= UU_WLAN_TX_CTL_ASSIGN_SEQ;
    /** ShortRetry count and Long retry count are removed from frame info structure */

    uu_wlan_setup_frame_info_length (fi, framelen);

    keytype = UU_WLAN_KEY_TYPE_CLEAR;

    fi->frameInfo.keytype = keytype;
    fi->frameInfo.framelen = framelen;
    /** Note: Following parameters are not implemented in current design. As per MAC-PHY discussion
     * following fixed values are passed to PHY before passing frame */
    fi->txvec.service = 0;
    fi->txvec.antenna_set = 0xFF;
    fi->txvec.num_ext_ss = 0;
    fi->txvec.is_no_sig_extn = 0;
    fi->txvec.is_smoothing = 0;
    /** As per the specification 0 indicates the PPDU is sounding PPDU and
     * 1 indicates the PPDU is not a sounding PPDU */
    fi->txvec.is_sounding = 1;
    fi->txvec.is_tx_op_ps_allowed = 0;
    fi->txvec.is_time_of_departure_req = 0;
    fi->txvec.is_beamformed = 0;
    fi->txvec.is_dyn_bw = 0;
    fi->txvec.indicated_chan_bw = 0;
    /** Groupd id is only present in VHT. value of 0 or 63 indicates SU PPDU. Otherwise indicates MU PPDU */
    fi->txvec.group_id = 0;

#ifdef UU_WLAN_TPC
    /* Setup transmit power level for the frame */
    uu_wlan_setup_frame_tpc (skb, fi, hw);
#endif
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("FRAME FORMAT is: %d\n", fi->txvec.format);
    printk("L_DATARATE is: %d\n", fi->txvec.L_datarate);
    printk("MCS is: %d\n", fi->txvec.mcs);
    printk("VHT_MCS is: %d\n", fi->txvec.tx_vector_user_params[0].vht_mcs);
    printk("TXPWR_LEVEL is: %d\n", fi->txvec.txpwr_level);
    printk("MODULATION is: %d\n", fi->txvec.modulation);
    printk("N_TX is: %d\n", fi->txvec.n_tx);
    printk("N_STS is: %d\n", fi->txvec.tx_vector_user_params[0].num_sts);
    printk("STBC is: %d\n", fi->txvec.stbc);
#endif

    printk("L_DATARATE is: %d\n", fi->txvec.L_datarate);
    printk("MCS is: %d\n", fi->txvec.mcs);
    /* Filling mpdu from skb */

    memcpy(fi->mpdu, skb->data, 1);
}

/** Handling tx frame which is received from mac framework.
 * Assign the sequence number if not assigned by framework.
 * Add padding bits to form 4-byte aligned MPDU.
 */
uu_int32 uu_wlan_tx_start(struct ieee80211_hw *hw,
        struct sk_buff *skb,
        uu_wlan_txq_info_t *txq)
{
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
    struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
    uu_wlan_umac_context_t *mac_context = hw->priv;
    uu_int32 q=0;

    if(NULL == mac_context)
    {
        UU_WLAN_LOG_DEBUG(("NULL mac_context in uu_wlan_tx_start\n"));
        return -1;
    }
    //UU_WLAN_SPIN_LOCK(&tx_start_lock);

    if(txq == NULL)
    {
        UU_WLAN_LOG_ERROR(("Predicting crash \n"));
        return -1;
    }

    /* TODO: Sequence number filling needs to be reviewed */
    if(hw->queues == 1) /* Our hardware is non-QoS */
    {
        //UU_WLAN_LOG_DEBUG(("Don't assign sequeunce number "));
        /* TODO: Review this. Confusion between BE & BK */
        q = 2; // For 11b mode AC_BE

        /* TODO: Confirm that sequence number filling is required for non-QoS */
        /** Assign sequence number */
        if (info->flags & IEEE80211_TX_CTL_ASSIGN_SEQ)
        {
            if (info->flags & IEEE80211_TX_CTL_FIRST_FRAGMENT)
            {
                mac_context->tx.seq_no += 0x10;
            }
            hdr->seq_ctrl &= cpu_to_le16(IEEE80211_SCTL_FRAG);
            hdr->seq_ctrl |= cpu_to_le16(mac_context->tx.seq_no);
        }
    }
    else if(hw->queues == 4) /* Our hardware supports QoS */
    {
        /* TODO: Confirm that sequence number filling is required for QoS */
        /* TODO: Sequence number may have to be on STA+AC basis for QoS */
        /** Assign sequence number */
        if (info->flags & IEEE80211_TX_CTL_ASSIGN_SEQ)
        {
            if (info->flags & IEEE80211_TX_CTL_FIRST_FRAGMENT)
            {
                mac_context->tx.seq_no += 0x10;
            }
            hdr->seq_ctrl &= cpu_to_le16(IEEE80211_SCTL_FRAG);
            hdr->seq_ctrl |= cpu_to_le16(mac_context->tx.seq_no);
        }

        /* TODO: Align entire frame to 4-byte boundary. Verify the spec, as it has similar requirement */

#if 0 /* We do NOT need 4-byte alignment for header. Data is immediately following the header */
        /** Calculate padding and Add padding to the header */

        pad_position = uu_wlan_agg_cal_pad_position(hdr->frame_control);
        pad_size = pad_position & 3; /* frame size should be multiples of 4 */

        if (pad_size && skb->len > pad_position)
        {
            if (skb_headroom(skb) < pad_size)
            {
                return -ENOMEM;
            }

            skb_push(skb, pad_size);
            memmove(skb->data, skb->data + pad_size, pad_position);
        }
#endif
        /* Enable this after Queue management work */
        q = skb_get_queue_mapping(skb);
    }
    else /* Will not happen, as we register either 1 or 4 queues */
    {
        q = 0;
    }

    UU_WLAN_LOG_DEBUG(("FWK-UMAC : seq number - %x : Time - %ld \n", (hdr->seq_ctrl>>4), jiffies));
    //spin_lock_bh(&uu_txq->uuq_lock);

#if 0 /* TODO: See the todo comment at 'q_state' and resolve it */
    if (!txq->stopped && txq->txq_len >= txq->txq_max )
    {
        UU_WLAN_LOG_ERROR(("UMAC: Giving stop queue msg to fwk"));
        ieee80211_stop_queue(mac_context->hw, q);
        txq->stopped = 1;
    }
#else
#ifdef UU_WLAN_BQID
    if(!ieee80211_is_beacon(hdr->frame_control))
#endif
    {
        q_state.num_pkts[q]++;

        if(!q_state.q_stopped[q] && (q_state.num_pkts[q] >= q_state.max_pkts[q]))
        {
            q_state.q_stopped[q] = 1;
            printk("STOPQ: stop queue messgae to fwk for ac %d\n", q);
            ieee80211_stop_queue(mac_context->hw, q);
        }
    }
#endif /* #if 0 related to 'q_state' */

    //spin_unlock_bh(&uu_txq->uuq_lock);
    uu_wlan_start_tx_send(mac_context, txq, skb);
    return 0;
} /* uu_wlan_tx_start */

/* EOF */

