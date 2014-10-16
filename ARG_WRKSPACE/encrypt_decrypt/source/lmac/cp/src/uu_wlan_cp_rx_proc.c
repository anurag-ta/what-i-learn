/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_cp_rx_proc.c                                          **
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

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_fwk_lock.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_rate.h"
#include "uu_wlan_utils.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_tx_if.h"

#include "uu_wlan_cp_ba_sb.h"
#include "uu_wlan_cp_ba_session.h"
#include "uu_wlan_cp_ba_sb_rx.h"
#include "uu_wlan_cp_ba_sb_tx.h"
#include "uu_wlan_lmac_sta_info.h"
#include "uu_wlan_cp_ctl_frame_gen.h"
#include "uu_wlan_cp_rx_proc.h"

#ifdef UU_WLAN_TPC
#include "uu_wlan_lmac_tpc.h"
#endif

/** Flag to indicate that 'implicit BA frame' has to be generated after RX-End */ 
static uu_bool uu_wlan_implicit_ba_pending_g;

static uu_int32 uu_wlan_rx_proc_data_with_ba(uu_wlan_cp_rx_frame_info_t* info);


static uu_int32 uu_wlan_cp_fill_resp_rate_dur(uu_wlan_cp_rx_frame_info_t* info, uu_uchar ctrl_resp_type, uu_uint16 ctrl_resp_len)
{
    uu_int32  duration;

    tx_ctl_frame_info_g.txvec.ch_bndwdth = info->bandwidth;

    /* for control request frames received, we wont change anything, just will update the duration */
    /* This is because we will get the control request frames only in basic rate or mcs */
    if ((info->fc0 & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL)
    {
        /* Sending control response frame in the same format as the control frame */
        tx_ctl_frame_info_g.txvec.format = info->format;

        /* For HT-GF control frames */
        if (info->format == UU_WLAN_FRAME_FORMAT_HT_GF)
        {
            /* When Control frame with some mcs is received, control
               response frame will be responded with the same MCS, as
               the mcs will be of Basic MCS set. */
            /* Since the MCS is supported by us, so we are able to receive the data,
             thus the response frame will be sent in the same mcs as of the control frame. */
            tx_ctl_frame_info_g.txvec.mcs = info->mcs;
            tx_ctl_frame_info_g.txvec.stbc = 0;
            tx_ctl_frame_info_g.txvec.is_short_GI = 0;
        }
        /* Assuming only non-HT control frames */
        /* We won't be receiving control frames with HT-MF and VHT format. */
        else
        {
            tx_ctl_frame_info_g.txvec.L_datarate = info->L_datarate;
            tx_ctl_frame_info_g.txvec.modulation = info->modulation;
        }

        /*
         * TODO : For later. This will be used in VHT case
         * This is for Dynamic RTS-CTS procedure in VHT case
         * This case if secondary channel is busy we can send cts in lower channel
        if (IEEE80211_IS_FC0_RTS (rx_frame_info->mpdu[0]))
         */

        /* For all other control request frames the control response frames will be same */
    }
    else /* for data and management frame, we will send in non-HT or HT-GF format for control response frames */
    {
        if (info->format == UU_WLAN_FRAME_FORMAT_HT_GF)
        {
            tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_HT_GF;

            /** As per section 9.7.6.5.5(3) control response frame
             * can't be transmitted with format = HT_GF, but we are sending the control response
             * frame in HT-GF format as no reason is mentioned in specification for not sending
             * in HT-GF, so we decided to send in HT-GF. 
             */
            /* TODO Since we don't have information about Basic MCS Set. So, we are sending with Mandatory MCS set */
            tx_ctl_frame_info_g.txvec.mcs = (info->mcs) % 8;

            /* Sending response frame with STBC = 0 and Short GI = 0. */
            tx_ctl_frame_info_g.txvec.stbc = 0;
            tx_ctl_frame_info_g.txvec.is_short_GI = 0;
        }
        else /* Non-HT */
        {
            tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
            tx_ctl_frame_info_g.txvec.L_datarate = uu_assign_basic_rate(info->modulation, info->format, info->format ? info->mcs:info->L_datarate);
            /* The modulation needs to be changed to OFDM for the control response frames which is sent
               in non-HT format for the HT or VHT frames */
            if (info->format == UU_WLAN_FRAME_FORMAT_VHT || info->format == UU_WLAN_FRAME_FORMAT_HT_MF)
            {
                /* For bandwidth (40MHz, 80MHz or 160  > 20 MHz */
                if (tx_ctl_frame_info_g.txvec.ch_bndwdth)
                {
                    tx_ctl_frame_info_g.txvec.modulation = NON_HT_DUP_OFDM;
                }
                else
                {
                    tx_ctl_frame_info_g.txvec.modulation = OFDM;
                }
            }
            else
            {
                tx_ctl_frame_info_g.txvec.modulation = info->modulation;
            }
        }
    }

    /* for HT case, we define length in ht_length */
    if (tx_ctl_frame_info_g.txvec.format == UU_WLAN_FRAME_FORMAT_HT_GF)
    {
        tx_ctl_frame_info_g.txvec.ht_length = ctrl_resp_len;
    }
    else /* for non-HT case */
    {
        tx_ctl_frame_info_g.txvec.L_length = ctrl_resp_len;
    }

    duration = info->duration - uu_calc_frame_duration(&tx_ctl_frame_info_g.txvec) - UU_WLAN_SIFS_TIMER_VALUE_R;

    return duration;
} /* uu_wlan_get_duration */


/* TODO: Every frame received from remote party MUST be validated for length */
static uu_int32 uu_wlan_rx_proc_cts_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32  ret = UU_WLAN_RX_HANDLER_CTS_RCVD;

    if (uu_wlan_cp_waiting_for_cts_g == 0)
    {
        ret = UU_WLAN_RX_HANDLER_FRAME_INVALID;
    }

    /*if (frameLen != UU_CTS_FRAME_LEN)
    {
        ret = UU_FRAME_ERROR;
    }*/

    return ret;
}/* uu_wlan_rx_proc_cts_pkt */


static uu_int32 uu_wlan_rx_proc_rts_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32 duration, len;
    /* if (frameLen != UU_RTS_FRAME_LEN)
    {
        return UU_FRAME_ERROR;
    } */

    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));

    duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_SUBTYPE_CTS | IEEE80211_FC0_TYPE_CTL), UU_CTS_FRAME_LEN);
    len = uu_wlan_gen_cts_frame(info, duration);

    /* copying length to tx_ctl_frame_information */
    tx_ctl_frame_info_g.frameInfo.framelen = len;

#ifdef UU_WLAN_TPC
    /* calculating the transmit power level for control frames */
    tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames (tx_ctl_frame_info_g.txvec.ch_bndwdth, tx_ctl_frame_info_g.txvec.format,
                                tx_ctl_frame_info_g.txvec.modulation);
#endif

    return UU_WLAN_RX_HANDLER_SEND_RESP;
}/* uu_wlan_rx_proc_rts_pkt */


/** Sends ACK for the received PS-POLL frame */
static uu_int32 uu_wlan_rx_proc_pspoll_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32 duration, len;

    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));

    duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_SUBTYPE_ACK | IEEE80211_FC0_TYPE_CTL), UU_ACK_FRAME_LEN);
    len = uu_wlan_gen_ack_frame(info, duration);

    /* copying length to tx_ctl_frame_information */
    tx_ctl_frame_info_g.frameInfo.framelen = len;
    return UU_WLAN_RX_HANDLER_SEND_RESP;
} /* uu_wlan_rx_proc_pspoll_pkt */


static uu_int32 uu_wlan_rx_proc_ack_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32  ret = UU_WLAN_RX_HANDLER_ACK_RCVD;

    if (uu_wlan_cp_waiting_for_ack_g == 0)
    {
        ret = UU_WLAN_RX_HANDLER_FRAME_INVALID;
    }

    /*if (frameLen != UU_ACK_FRAME_LEN)
    {
        ret = UU_FRAME_ERROR;
        goto BOTTOM;
    }*/

    return ret;
}/* uu_wlan_rx_proc_ack_pkt */


/* TODO: Somewhere, need to validate whether we are really waiting for Immediate-BA */
static uu_int32 uu_wlan_rx_proc_data_immediate_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32 duration, len;

    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));

    /* Duration is being calculated for ack frames sent by non-QoS STAs if More fragments
       bit was equal to 0 in immediately previous individually addressed data frame. */
    if ((!uu_dot11_qos_mode_r) && (!(info->fc1 & IEEE80211_FC1_MORE_FRAG)))
    {
        /* For non-Qos Mode, and if More Fragment is 0, then duration is 0.
           and Assigning the required Tx-Vector parameters for control response frame. */
        duration = 0;
        tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
        tx_ctl_frame_info_g.txvec.L_datarate = uu_assign_basic_rate(info->modulation, info->format, info->L_datarate);
        tx_ctl_frame_info_g.txvec.L_length = UU_ACK_FRAME_LEN;
        tx_ctl_frame_info_g.txvec.modulation = info->modulation;
        tx_ctl_frame_info_g.txvec.ch_bndwdth = info->bandwidth;
    }
    else
    {
        duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_SUBTYPE_ACK | IEEE80211_FC0_TYPE_CTL), UU_ACK_FRAME_LEN);
    }

    len = uu_wlan_gen_ack_frame(info, duration);

    /* copying length to tx_ctl_frame_information */
    tx_ctl_frame_info_g.frameInfo.framelen = len;

#ifdef UU_WLAN_TPC
    /* calculating the transmit power level for control frames */
    tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames (tx_ctl_frame_info_g.txvec.ch_bndwdth, tx_ctl_frame_info_g.txvec.format,
                                tx_ctl_frame_info_g.txvec.modulation);
#endif

#if 0
    /* Update the score board for every packet if BA session available */
    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(info->ta, info->tid, 0);
    if(ba == NULL)
    {
        ; /* No action.  ret = UU_BA_SESSION_INVALID; */
    }
    else
    {
        /* Update Recepient Ba bitmap */
        uu_wlan_ba_sb_update_rx(&ba->ses.sb, info->scf);
    }

    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
#else
    /* If a BA session exists, update the score-board. Otherwise, ignore */
    uu_wlan_rx_proc_data_with_ba(info);
#endif

    return UU_WLAN_RX_HANDLER_SEND_RESP;
} /* uu_wlan_rx_proc_data_immediate_pkt */


static uu_int32 uu_wlan_rx_proc_data_implicit_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32  duration;
    //uu_wlan_ba_ses_context_t *ba;

#if 0
    /* if (frameLen < UU_MIN_DATA_PKT_LEN)
    {
        return UU_FRAME_ERROR;
    } */

    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(info->ta, info->tid, 0);
    if(ba == NULL)
    {
        UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
        return UU_BA_SESSION_INVALID;
    }

    uu_wlan_ba_sb_update_rx(&ba->ses.sb, info->scf);
    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
#else
    uu_wlan_rx_proc_data_with_ba(info);
#endif

    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));

    duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_SUBTYPE_BA | IEEE80211_FC0_TYPE_CTL), UU_BA_COMPRESSED_FRAME_LEN);
    tx_ctl_frame_info_g.txvec.ch_bndwdth = info->bandwidth;

    uu_wlan_gen_ba_frame(info, duration, 1);

#ifdef UU_WLAN_TPC
    /* calculating the transmit power level for control frames */
    tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames (tx_ctl_frame_info_g.txvec.ch_bndwdth, tx_ctl_frame_info_g.txvec.format,
                                tx_ctl_frame_info_g.txvec.modulation);
#endif

    uu_wlan_implicit_ba_pending_g = 0;
    return UU_WLAN_RX_HANDLER_SEND_RESP;
} /* uu_wlan_rx_proc_data_implicit_pkt */


static uu_int32 uu_wlan_rx_proc_data_with_ba(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32   ret = UU_SUCCESS;
    uu_wlan_ba_ses_context_t *ba;

    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(info->ta, info->tid, 0);
    if(ba == NULL)
    {
        ret = UU_BA_SESSION_INVALID;
        goto UNLOCK;
    }

    uu_wlan_implicit_ba_pending_g = 1;
    
    /* Update Recepient Ba bitmap */
    ret = uu_wlan_ba_sb_update_rx(&ba->ses.sb , info->scf);

UNLOCK:
    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
    return ret;
}/* uu_wlan_rx_proc_data_with_ba */


static uu_int32 uu_wlan_rx_proc_bar_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32  duration, len;
    uu_wlan_ba_ses_context_t *ba;

    /* presently consider only implicit BA case(so need of bar)*/
    /* 
     ** if cosider delayed or immediate BA.
     ** for immediate BA - need to generate ba frame.
     ** for delayed BA - need to check support ack policy / // TODO 
     ** if ack policy is set generate ack frame and ba frame.
     ** otherwise generate ba frame.
     */
    /* Delayed Block Ack case */ /* TODO: Who will generate the delayed-BA later? */
    /*if(frameLen != UU_BAR_FRAME_LEN)
    {
        return UU_FRAME_ERROR;
    }*/

    if(info->qos_ack_policy ==  UU_WLAN_ACKPOLICY_NORMAL)
    {
        UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));

        duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_SUBTYPE_ACK | IEEE80211_FC0_TYPE_CTL), UU_ACK_FRAME_LEN);
        len = uu_wlan_gen_ack_frame(info, duration);

        /* copying length to tx_ctl_frame_information */
        tx_ctl_frame_info_g.frameInfo.framelen = len;

#ifdef UU_WLAN_TPC
        /* calculating the transmit power level for control frames */
        tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames (tx_ctl_frame_info_g.txvec.ch_bndwdth, tx_ctl_frame_info_g.txvec.format,
                                tx_ctl_frame_info_g.txvec.modulation);
#endif
        return UU_WLAN_RX_HANDLER_SEND_RESP;
    }

    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));

    duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_SUBTYPE_BA | IEEE80211_FC0_TYPE_CTL), UU_BA_BASIC_FRAME_LEN);
    /* Immediate Block Ack case */
    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(info->ta, info->tid, 0);
    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
    if (ba == NULL)
    {
        return UU_BA_SESSION_INVALID;
    }
    else
    {
        /* Immediate Block Ack case */
        uu_wlan_gen_ba_frame(info, duration, 0);

#ifdef UU_WLAN_TPC
    /* calculating the transmit power level for control frames */
    tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames (tx_ctl_frame_info_g.txvec.ch_bndwdth, tx_ctl_frame_info_g.txvec.format,
                                tx_ctl_frame_info_g.txvec.modulation);
#endif
    }

    return UU_WLAN_RX_HANDLER_SEND_RESP;
}/* uu_wlan_rx_proc_bar_pkt */


static uu_int32 uu_wlan_rx_proc_ba_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32 duration, len;
    uu_wlan_ba_ses_context_t *ba;

    /* TODO This check is valid only for Immediate BA and need add extra flag for Delayed BA */
    if (uu_wlan_cp_waiting_for_ack_g == 0) 
    {
        return UU_WLAN_RX_HANDLER_FRAME_INVALID;
    }

    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(info->ta, info->tid, 1);
    if (ba == NULL)
    {
        UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
        return UU_BA_SESSION_INVALID;
    }

    /* Freeing the trasmitted AMPDU pkts, when the bitmap of received ba frame is macthes with what we are maintained */
    /* compressed BA case */
    if(info->bar_type == UU_WLAN_BAR_TYPE_COMP_BA)
    {
        /* status checking for clearing */
        if (uu_wlan_tx_is_ampdu_status_clear(uu_wlan_get_ac_from_tid(info->tid)) == UU_SUCCESS)
        {
            return UU_WLAN_RX_HANDLER_BA_RCVD;
        }
        else
        {
            /* Not clearing the status because one or more frames not received correctly */
            return UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS;
        }
    }
    else /* Basic BA */
    {
        /* TODO: Basic BA not implemented, and will be taken care later. */
        if (uu_wlan_cp_tx_compare_bitmap(&ba->ses.sb) == UU_SUCCESS)
        {
           uu_wlan_clear_ampdu(uu_wlan_get_ac_from_tid(info->tid));
        }
        else
        {
            // uu_wlan_clear_ampdu(uu_wlan_get_ac_from_tid(info->tid));
            /* Not clearing the status because one or more frames not received correctly */
        }
    }

    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
 
    /* Delayed Block Ack case */ 
    if(info->qos_ack_policy == UU_WLAN_ACKPOLICY_NORMAL) 
    {
        UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));

        duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_SUBTYPE_ACK | IEEE80211_FC0_TYPE_CTL), UU_ACK_FRAME_LEN);
        len = uu_wlan_gen_ack_frame(info, duration);

        /* copying length to tx_ctl_frame_information */
        tx_ctl_frame_info_g.frameInfo.framelen = len;

#ifdef UU_WLAN_TPC
        /* calculating the transmit power level for control frames */
        tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames (tx_ctl_frame_info_g.txvec.ch_bndwdth, tx_ctl_frame_info_g.txvec.format,
                                tx_ctl_frame_info_g.txvec.modulation);
#endif

        return UU_WLAN_RX_HANDLER_SEND_RESP;
    }

    return UU_WLAN_RX_HANDLER_BA_RCVD;
}/* uu_wlan_rx_proc_ba_pkt */


static uu_int32 uu_wlan_rx_proc_mgt_pkt(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_int32 duration, len;

    /* if (frameLen < UU_MIN_MGT_PKT_LEN)
    {
        return UU_FRAME_ERROR;
    } */

    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));

    /* Duration is being calculated for ack frames sent by non-QoS STAs if More fragments
       bit was equal to 0 in immediately previous individually addressed data frame. */
    if ((!uu_dot11_qos_mode_r) && (!(info->fc1 & IEEE80211_FC1_MORE_FRAG)))
    {
        /* For non-Qos Mode, and if More Fragment is 0, then duration is 0.
           and Assigning the required Tx-Vector parameters for control response frame. */
        duration = 0;
        tx_ctl_frame_info_g.txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
        tx_ctl_frame_info_g.txvec.L_datarate = uu_assign_basic_rate(info->modulation, info->format, info->L_datarate);
        tx_ctl_frame_info_g.txvec.L_length = UU_ACK_FRAME_LEN;
        tx_ctl_frame_info_g.txvec.modulation = info->modulation;
        tx_ctl_frame_info_g.txvec.ch_bndwdth = info->bandwidth;
    }
    else
    {
        duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_TYPE_CTL |IEEE80211_FC0_SUBTYPE_ACK), UU_ACK_FRAME_LEN);
    }
    len = uu_wlan_gen_ack_frame(info, duration);

    /* copying length to tx_ctl_frame_information */
    tx_ctl_frame_info_g.frameInfo.framelen = len;

#ifdef UU_WLAN_TPC
    /* calculating the transmit power level for control frames */
    tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames (tx_ctl_frame_info_g.txvec.ch_bndwdth, tx_ctl_frame_info_g.txvec.format,
                                tx_ctl_frame_info_g.txvec.modulation);
#endif

    return UU_WLAN_RX_HANDLER_SEND_RESP;
} /* uu_wlan_rx_proc_mgt_pkt */


uu_int32 uu_wlan_cp_rx_proc(uu_wlan_cp_rx_frame_info_t* info)
{
    uu_uint8  fc = info->fc0;
    uu_int32  ret = UU_SUCCESS;

    /* Only individually-addressed management frames will come here. They are handled in the same way */
    if ((fc & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_MGT)
    {
        if(fc == (IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK | IEEE80211_FC0_TYPE_MGT))
        {
            /* No CP action for individually-addressed Management frames, with 'Action no-ack' */
            return UU_SUCCESS;
        }
        fc = IEEE80211_FC0_TYPE_MGT;
    }

    switch (fc)
    {
        case (IEEE80211_FC0_SUBTYPE_RTS | IEEE80211_FC0_TYPE_CTL): /* RTS */
            ret = uu_wlan_rx_proc_rts_pkt(info);
            break;
        case (IEEE80211_FC0_SUBTYPE_CTS | IEEE80211_FC0_TYPE_CTL): /* CTS */
            ret = uu_wlan_rx_proc_cts_pkt(info);
            break;
        case (IEEE80211_FC0_SUBTYPE_ACK | IEEE80211_FC0_TYPE_CTL): /* ACK */
            ret = uu_wlan_rx_proc_ack_pkt(info);
            break;
        case (IEEE80211_FC0_SUBTYPE_PSPOLL | IEEE80211_FC0_TYPE_CTL): /* PS-POLL */
            ret = uu_wlan_rx_proc_pspoll_pkt(info);
            break;
        case (IEEE80211_FC0_SUBTYPE_BAR | IEEE80211_FC0_TYPE_CTL): /* BAR */
            ret = uu_wlan_rx_proc_bar_pkt(info);
            break;
        case (IEEE80211_FC0_SUBTYPE_BA | IEEE80211_FC0_TYPE_CTL): /* BA */
            ret = uu_wlan_rx_proc_ba_pkt(info);
            break;
        case (IEEE80211_FC0_SUBTYPE_QOS | IEEE80211_FC0_TYPE_DATA): /* QoS Data */
        case (IEEE80211_FC0_SUBTYPE_QOS_NULL | IEEE80211_FC0_TYPE_DATA): /* QoS Data null (no-data) */
            {
                if(info->qos_ack_policy == UU_WLAN_ACKPOLICY_NORMAL)
                {
                    /* If the packet is a part of A-MPDU */
                    if (info->is_aggregated)
                    {
                        if (info->is_ampdu_end)
                            ret = uu_wlan_rx_proc_data_implicit_pkt(info);
                        else /* (!info->is_ampdu_end) */
                            ret = uu_wlan_rx_proc_data_with_ba(info);
                    }
                    else /* Non-aggregated frame (immediate ACK) */
                    {
                        ret = uu_wlan_rx_proc_data_immediate_pkt(info);
                    }
                }
                else if (info->qos_ack_policy == UU_WLAN_ACKPOLICY_NO_ACK)
                {
                    /*
                    ** It is possible to send A-MPDU, without BA session, if all frames have NO-ACK policy.
                    ** Nothing to do in CP, for frame with ACK policy of NO-ACK.
                    */
                    ret = UU_SUCCESS;
                }
                else if (info->qos_ack_policy == UU_WLAN_ACKPOLICY_PSMP_ACK)
                {
                    /* TODO: Support for Power-Save */
                    ret = UU_SUCCESS;
                }
                else /* if(info->qos_ack_policy == UU_WLAN_ACKPOLICY_BA) */
                {
                    ret = uu_wlan_rx_proc_data_with_ba(info);
                }
            }
            break;
        case IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_DATA: /* Non-QoS Data */
        case IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_DATA_NULL: /* Non-QoS data null (no-data) */
            ret = uu_wlan_rx_proc_data_immediate_pkt(info);
            break;
        case IEEE80211_FC0_TYPE_MGT:
            {
                ret = uu_wlan_rx_proc_mgt_pkt(info);
            }
            break;

        /* Special case: Set to this, with 'is_ampdu_end', to trigger check for
            BA generation, when Rx failed for last MPDU of an AMPDU. */
        case IEEE80211_FC0_TYPE_RESERVED:
            {
                uu_int32  duration;

                if (uu_wlan_implicit_ba_pending_g)
                {
                    UU_WLAN_MEM_SET(&tx_ctl_frame_info_g, sizeof(uu_wlan_tx_frame_info_t));
                    duration = uu_wlan_cp_fill_resp_rate_dur(info, (IEEE80211_FC0_SUBTYPE_BA | IEEE80211_FC0_TYPE_CTL), UU_BA_COMPRESSED_FRAME_LEN);
                    uu_wlan_gen_ba_frame(info, duration, 1);

#ifdef UU_WLAN_TPC
                    /* calculating the transmit power level for control frames */
                    tx_ctl_frame_info_g.txvec.txpwr_level = uu_calc_tpl_ctrl_frames (tx_ctl_frame_info_g.txvec.ch_bndwdth, tx_ctl_frame_info_g.txvec.format,
                                tx_ctl_frame_info_g.txvec.modulation);
#endif

                    uu_wlan_implicit_ba_pending_g = 0;
                    ret = UU_WLAN_RX_HANDLER_SEND_RESP;
                }
            }
            break;
        default:
            break;
    }
    return ret;
} /* uu_wlan_cp_rx_proc */


/* EOF */

