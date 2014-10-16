/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_if.c                                        **
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
#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/jiffies.h>
#endif

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_utils.h"

#include "uu_wlan_cp_if.h"
#include "uu_wlan_cp_ba_session.h" 
#include "uu_wlan_cp_ba_sb.h"
#include "uu_wlan_cp_ctl_frame_gen.h"
#include "uu_wlan_lmac_sta_info.h"
#include "uu_wlan_cp_rx_proc.h"
#include "uu_wlan_cp_ba_sb_tx.h"

#ifdef UU_WLAN_TPC
#include "uu_wlan_lmac_tpc.h"
#endif


/*
** Global variables
*/
/* Control frame info, generated for sending out */
uu_char  uu_wlan_tx_ctl_frame_info_g[sizeof(uu_wlan_tx_frame_info_t)+256]; /* Assuming 256 bytes is sufficient for control frame */

uu_wlan_tx_frame_info_t *tx_ctl_frame_info_gp = (uu_wlan_tx_frame_info_t *)uu_wlan_tx_ctl_frame_info_g;

/* sets if expected frame is ack/immediate BA */
uu_bool uu_wlan_cp_waiting_for_ack_g;

/* sets if expected frame is cts */
uu_bool uu_wlan_cp_waiting_for_cts_g;

/*
 ** Function to generate Control Request frame - RTS / Self-CTS.
 ** Input is: The frame information extracted from the Data/Management frame.
 */
static uu_int32 _gen_txop_rsrv_frame(uu_wlan_cp_tx_frame_info_t* info)
{
    uu_int32 ret = UU_SUCCESS;
    uu_int32 len;

     /* control packet generator function call here */
    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));
    //tx_ctl_frame_info = (uu_wlan_frame_info_t *)uu_wlan_desc_get_data_ptr(desc_id);
    //tx_ctl_frame_info->desc_id = desc_id;
    //tx_ctl_frame_info = uu_wlan_alloc_rx_frame_info(UU_WLAN_FRAME_BUFF_TYPE_SMALL);
    //tx_ctl_frame_info->src_sta = frame_info->src_sta;
    //tx_ctl_frame_info->dst_sta = frame_info->dst_sta;

    /* While taking reference from the PPDU, we send control request frame in non-HT for legacy/HT-MF/VHT
     * If PPDU is in HT-GF, we will send in HT-GF format for control request frame too */
    if (info->format == UU_WLAN_FRAME_FORMAT_HT_GF)
    {
        tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_HT_GF; /* control frame format */
    }
    else
    {
        tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
    }

    /* TODO : Delete this comment after review. Previously this one was taken care while filling in Txvector in cap_txop. */
    /* for control request frame if it is non-HT format then we assign the rate to L-datarate */
    if (tx_ctl_frame_info_g.txvec.format == UU_WLAN_FRAME_FORMAT_NON_HT)
    {
        tx_ctl_frame_info_g.txvec.L_datarate = info->rtscts_rate/*UU_CTL_FRAME_RATE*/;

        /* Filling modulation */
        if (info->format == UU_WLAN_FRAME_FORMAT_HT_MF || info->format == UU_WLAN_FRAME_FORMAT_VHT)
        {
            /* If bandwidth is 40MHz, 80MHz or 160MHz*/
            if (info->bandwidth)
            {
                tx_ctl_frame_info_g.txvec.modulation = NON_HT_DUP_OFDM;
            }
            else
            {
                /* If bandwidth is 20MHz */
                tx_ctl_frame_info_g.txvec.modulation = OFDM;
            }
        }
        /* For non-HT PPDU, modulation is same. */
        else
        {
                tx_ctl_frame_info_g.txvec.modulation = info->modulation;
        }
    }
    /* else we assign the mcs index present in rtscts_rate to mcs index */
    else /* This must be HT-GF case as present code */
    {
        tx_ctl_frame_info_g.txvec.mcs = info->rtscts_rate/*UU_CTL_FRAME_RATE*/; 

        /* This is for RTS frame or self-CTS frame. This whole function is for rts-cts frame so no need of conditions */
        tx_ctl_frame_info_g.txvec.is_short_GI = 0; /* As per spec. we should NOT send control frame that initiates TXOP with Short GI or LDPC coding */
        tx_ctl_frame_info_g.txvec.is_fec_ldpc_coding = 0; /* BCC Coding */
    }

    if (UU_WLAN_SELF_CTS_R)
    {
        len = uu_wlan_gen_selfcts_frame(info, 0);
    }
    else 
    {
        len = uu_wlan_gen_rts_frame(info, 0);
    }

#ifdef UU_WLAN_TPC
    tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames(
        tx_ctl_frame_info_g.txvec.ch_bndwdth,
        tx_ctl_frame_info_g.txvec.format,
        tx_ctl_frame_info_g.txvec.modulation);
#endif

    /* copying length to tx_ctl_frame_information */
    tx_ctl_frame_info_g.frameInfo.framelen = len;

    return ret;
} /* _gen_txop_rsrv_frame */


/** Request to generate a BAR frame 
 */
uu_uint32 uu_wlan_cp_generate_bar(uu_uint8 bartype, uu_uint8 tid, uu_uint16 ssn, uu_uint16 duration, uu_uchar *ra)
{
    return uu_wlan_gen_bar_frame(bartype, tid, ssn, duration, ra);
} /* uu_wlan_cp_generate_bar */


/** This request given by CAP, when TXOP won.
 * Generates RTS or self-CTS, if transmitting packet length crosses RTS threshold. 
 */
uu_int32 uu_wlan_cp_generate_rtscts(uu_uint8 format, uu_uint8 rtscts_rate, uu_uint8 ch_bndwdth, uu_uint8 modulation, uu_uchar *ra)
{
    uu_wlan_cp_tx_frame_info_t info;

    /* Copy the required field from TX frame */
    info.format = format;
    info.rtscts_rate = rtscts_rate;

    info.bandwidth = ch_bndwdth; /* 3 bits */
    info.modulation = modulation; /* 3 bits */

    UU_COPY_MAC_ADDR(&info.ra, ra);

    return _gen_txop_rsrv_frame(&info);
}


/** Called by TX handler to update the score board for transmitting packet 
*/
uu_void uu_wlan_cp_update_the_tx_sb(uu_uchar *ra, uu_uint8 tid, uu_uint16 scf)
{
    uu_wlan_ba_ses_context_t *ba;

    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(ra, tid, 1);
    if(ba != NULL)
    {
        uu_wlan_sb_update_tx(&ba->ses.sb, scf);
    }
    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
} /* uu_wlan_cp_update_the_tx_sb */


/** Entry point into CP module for Rx frames.
 * Called by RX handler, when a Frame is received from PHY
 */
uu_int32 uu_wlan_cp_process_rx_frame(uu_wlan_rx_frame_info_t *frame_info, uu_bool is_rxend)
{
    /* If receive rx error in A-MPDU frame, we need to send Imlicit BA if 'uu_wlan_implicit_ba_pending_g' flag is set
     * To form BA packet address and BA context information of previous QOS frame information is required
     */
    static uu_wlan_cp_rx_frame_info_t info;
    uu_wlan_ba_ses_context_t   *ba;

    /* Copy the required field from RX frame, copying info will vary with frame type */
    info.fc0 = UU_WLAN_GET_FC0_VALUE(frame_info->mpdu);
    info.fc1 = UU_WLAN_GET_FC1_VALUE(frame_info->mpdu);
    if ((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_RESERVED)
    {
        goto BOTTOM;
    }
    info.format = frame_info->rxvec.format;
    info.L_datarate = frame_info->rxvec.L_datarate;

    if(!((info.fc0 == (IEEE80211_FC0_SUBTYPE_CTS | IEEE80211_FC0_TYPE_CTL))
                || (info.fc0 == (IEEE80211_FC0_SUBTYPE_ACK | IEEE80211_FC0_TYPE_CTL))))
    {
        UU_COPY_MAC_ADDR(&info.ta, &frame_info->mpdu[UU_TA_OFFSET]);
    }
    UU_COPY_MAC_ADDR(&info.ra, &frame_info->mpdu[UU_RA_OFFSET]);
    info.is_aggregated = frame_info->rxvec.is_aggregated;
    info.modulation = frame_info->rxvec.modulation;
    info.mcs = frame_info->rxvec.mcs;
    info.duration = *((uu_uint16 *)(&frame_info->mpdu[2]));

    /** Bandwidth for the control response frame should be same as the frame
     *  to which it is a response except for VHT RTS Procedure for CTS frame
     */
    info.bandwidth = frame_info->rxvec.ch_bndwdth;

    if(info.fc0 == (IEEE80211_FC0_SUBTYPE_QOS | IEEE80211_FC0_TYPE_DATA))
    {
        /* For Mesh */
        if(IEEE80211_IS_FRAME_4ADDR(frame_info->mpdu))
        {
            info.tid = UU_WLAN_GET_4ADDR_QOS_TID_VALUE(frame_info->mpdu);
            info.qos_ack_policy = UU_WLAN_GET_4ADDR_QOS_ACKPOLICY_VALUE(frame_info->mpdu);
        }
        else
        {
            info.tid = UU_WLAN_GET_QOS_TID_VALUE(frame_info->mpdu);
            info.qos_ack_policy = UU_WLAN_GET_QOS_ACKPOLICY_VALUE(frame_info->mpdu);
        }
        info.scf = *(uu_uint16*)(frame_info->mpdu + UU_SCF_OFFSET);
    }
    else if(info.fc0 == (IEEE80211_FC0_SUBTYPE_BAR | IEEE80211_FC0_TYPE_CTL))
    {
        info.scf = *(uu_uint16*)(frame_info->mpdu + UU_SSN_FIELD_OFFSET);
        info.tid = UU_WLAN_GET_BAR_TID_VALUE(frame_info->mpdu);
        info.bar_type = UU_WLAN_GET_BAR_TYPE_VALUE(frame_info->mpdu);
        info.bar_fc_field = *((uu_uint16 *)(frame_info->mpdu + UU_BAR_FC_FIELD_OFFSET));
        info.qos_ack_policy = UU_WLAN_GET_BAR_ACKPOLICY_VALUE(frame_info->mpdu);
    }
    else if(info.fc0 == (IEEE80211_FC0_SUBTYPE_BA | IEEE80211_FC0_TYPE_CTL))
    {
        info.qos_ack_policy = UU_WLAN_GET_BAR_ACKPOLICY_VALUE(frame_info->mpdu);
        info.bar_type = UU_WLAN_GET_BAR_TYPE_VALUE(frame_info->mpdu);
        info.scf = *(uu_uint16*)(frame_info->mpdu + UU_SSN_FIELD_OFFSET);
        info.tid = UU_WLAN_GET_BA_TID_VALUE(frame_info->mpdu);

        UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
        ba = uu_wlan_get_ba_contxt_of_sta(info.ta, info.tid, 1);
        if(ba == NULL)
        {
            UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
            return UU_BA_SESSION_INVALID;
        }

        /* Compressed BA */
        if(info.bar_type == UU_WLAN_BAR_TYPE_COMP_BA)
        {
            uu_wlan_handle_comp_ba(&ba->ses.sb, (info.scf >> 4), &frame_info->mpdu[UU_BA_BITMAP_OFFSET], uu_wlan_get_ac_from_tid(info.tid));
        }
        else /* Basic BA */
        {
            uu_wlan_handle_basic_ba(&ba->ses.sb, (info.scf >> 4), (uu_uint16*)&frame_info->mpdu[UU_BA_BITMAP_OFFSET]);
        }
        UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
    }
BOTTOM:
    info.is_ampdu_end = is_rxend;
    return uu_wlan_cp_rx_proc(&info);
} /* uu_wlan_cp_process_rx_frame */


/* Called from CAP,  when CTS is expected */
uu_void uu_wlan_cp_set_waiting_for_cts(uu_void)
{
    uu_wlan_cp_waiting_for_cts_g = 1;
}

/* Called from CAP,  when CTS is received/CTS-Timer expired */
uu_void uu_wlan_cp_reset_waiting_for_cts(uu_void)
{
    uu_wlan_cp_waiting_for_cts_g = 0;
}

/* Called from CAP,  when ACK/Immediate BA is expected */
uu_void uu_wlan_cp_set_waiting_for_ack(uu_void)
{
    uu_wlan_cp_waiting_for_ack_g = 1;
}

/* Called from CAP,  when ACK/Immediate BA is received/ACK-Timer expired */
uu_void uu_wlan_cp_reset_waiting_for_ack(uu_void)
{
    uu_wlan_cp_waiting_for_ack_g = 0;
}


/* EOF */

