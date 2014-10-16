/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_tx_handler.c                                   **
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

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_msgq.h"
#include "crc32.h"
#include "uu_wlan_utils.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_cap_context.h" /* TODO: Remove this dependecy. Current, used for uu_wlan_cap_retain_tx_frame_info & uu_wlan_cap_ctxt_g */
#include "uu_wlan_duration.h"
#include "uu_wlan_cap_txop.h"
#include "uu_wlan_tx_aggregation.h"
#include "uu_wlan_tx_handler.h"
#include "uu_wlan_cp_ba_session.h"
#include "uu_wlan_rate.h"
#ifdef UU_WLAN_TPC
#include "uu_wlan_lmac_tpc.h"
#endif


/* TODO: See whether CP is better place for this */
/** buffer for QoS null data frame -----trigger for uapsd powersave */
static uu_uchar  uu_wlan_ps_trigger_frame_info_g[ sizeof(uu_wlan_tx_frame_info_t) + UU_QOS_NULL_FRAME_LEN ];
uu_wlan_tx_frame_info_t  *ps_trigger_frame_info_g = (uu_wlan_tx_frame_info_t *)uu_wlan_ps_trigger_frame_info_g;
/** Indicates Tx hadler has to send a U-APSD trigger frame(QoS Null data frame).
  * Set in Rx handler when an U-APSD STA receives a data frame with both EOSP and more data bit set.
  * Reset in Tx handler after sending trigger frame.
 **/
uu_bool uu_wlan_lmac_trigger_frame_ready_g = UU_FALSE;


/** buffer for pspoll frame -----trigger for legacy powersave */
static uu_uchar  uu_wlan_ps_poll_frame_info_g[ sizeof(uu_wlan_tx_frame_info_t) + UU_PSPOLL_FRAME_LEN ];
uu_wlan_tx_frame_info_t  *ps_poll_frame_info_g = (uu_wlan_tx_frame_info_t *)uu_wlan_ps_poll_frame_info_g;
/** Indicates Tx handler has to send a PS-Poll frame .
  * Set in Rx handler when TIM is set for the STA in beacon and when the more data bit is set in received frame.
  * Reset in Tx handler after sending PS-Poll frame.
 **/
uu_bool uu_wlan_lmac_pspoll_frame_ready_g = UU_FALSE;


/* Reference frame sent info from start_request 0: Data frame, 1: Control frame*/
/** It is initialized to 'UU_FALSE' in "uu_wlan_handle_txop_tx_start_req"
 * if UU_WLAN_RTS_THRESHOLD_R > frame_length, it is set to value 'UU_TRUE'.
 * uu_wlan_frame_is_cntl_frame_g is used in uu_wlan_tx_phy_data_req function, to get the control frame reference.
 * it is reset, after getting the frame_info reference in uu_wlan_tx_phy_data_req function.
 */
static uu_bool uu_wlan_frame_is_cntl_frame_g = UU_FALSE;

uu_wlan_ampdu_status_t uu_wlan_tx_ampdu_status_g[UU_WLAN_MAX_QID];
uu_uint8  uu_wlan_ShortRetryCount[UU_WLAN_MAX_QID];
uu_uint8  uu_wlan_LongRetryCount[UU_WLAN_MAX_QID];


static uu_int32 __txop_limit_check(uu_wlan_tx_frame_info_t* frame_info, uu_uint32 tx_nav)
{
    uu_uint32 duration = 0, resp_frame_length = 0;

    /* Multi Frame transmission in single TXOP */
    if(UU_WLAN_TXOP_LIMIT_R(uu_wlan_cap_ctxt_g.txop_owner_ac) > 0)
    {
        duration = uu_calc_frame_duration (&frame_info->txvec);
        resp_frame_length = uu_wlan_tx_get_expt_response_frame_length (frame_info);
        duration += uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, 0);

        if(tx_nav < duration)
        {
            return UU_FAILURE;/* as frame can not be sent with available TXOP.*/
        }
        else 
        {
            return UU_SUCCESS;
        }
    }
    /* Txop limit 0 case or non-QoS case, with more fragment bit is clear */
    else if(((UU_WLAN_TXOP_LIMIT_R(frame_info->ac) == 0) || (! UU_WLAN_IEEE80211_QOS_MODE_R))
            && (!(uu_wlan_tx_get_frame_fc1(frame_info->ac) & IEEE80211_FC1_MORE_FRAG)))
    {
        /* If the previous frame is not control frame, it is error */
        if ((uu_wlan_cap_ctxt_g.tx_frame_info.mpdu[0] & IEEE80211_FC0_TYPE_MASK) != IEEE80211_FC0_TYPE_CTL)
        {
            UU_WLAN_LOG_DEBUG(("LMAC: tx_send_more_data - Last frame was not control frame\n"));
            return UU_FAILURE;
        }
    }
    return UU_SUCCESS;
}

static uu_void uu_wlan_tx_update_frame_duration(uu_uchar *mpdu, uu_char ac, uu_uint16 duration)
{
    mpdu[2] = (uu_uchar)(duration & 0x00ff);
    mpdu[3] = (uu_uchar)((duration >> 8) & 0x00ff);
}


/** This request given by CAP, sets the retry indication
 * Uses aggeration module to re aggregate the packets.
 * param[in] ac contains the access category
 */
uu_void uu_wlan_tx_set_retry_bit(uu_uint8 ac)
{
    uu_wlan_tx_frame_info_t *frame_info = uu_wlan_tx_get_frame_info(ac);

    /* WARNING: RTL model, use the sperate internal register to store this value */
    frame_info->frameInfo.retry = 1;
}


/** update rtry bit in the packet
 * param[in] ac contains the access category
 */
uu_void uu_wlan_tx_set_retry(uu_uint8 ac)
{
    uu_wlan_tx_frame_info_t *frame_info = uu_wlan_tx_get_frame_info(ac);

    frame_info->mpdu[1] |= IEEE80211_FC1_RETRY;
}


/** This request given by CAP, updates the fall back rate/mcs
 * param[in] ac contains the access category
 */
uu_void uu_wlan_tx_update_with_fb_rate(uu_uint8 ac)
{
    uu_wlan_tx_frame_info_t *frame_info = uu_wlan_tx_get_frame_info(ac);

    /* WARNING: RTL model, use the sperate internal registers to store fall back values */
    /* This condition is for RTS frame rate which is common for legacy/HT/VHT */
    frame_info->rtscts_rate = frame_info->rtscts_rate_fb;

    /* Note This fallbacks are for PPDU and not for RTS */
    if (frame_info->txvec.format == UU_WLAN_FRAME_FORMAT_NON_HT) /* for non-HT case */
    {
        frame_info->txvec.L_datarate = frame_info->fallback_rate;
    }
    else /* for HT and VHT case */
    {
        frame_info->txvec.mcs = frame_info->mcs_fb;
        frame_info->txvec.stbc = frame_info->stbc_fb;
        frame_info->txvec.tx_vector_user_params[0].num_sts = frame_info->num_sts_fb[0];
        frame_info->txvec.n_tx = frame_info->n_tx_fb;
    }
}


#if 0 /* No longer using Tx Q & thread. Directly writing to AC Q */
uu_int32 uu_wlan_tx_handler(uu_uchar *temp_umac_data)
{
    uu_wlan_tx_frame_info_t *tx_frame_info;
    uu_int16 desc_no;
    uu_uint8 ac;

    //UU_WLAN_LOG_DEBUG(("LMAC: lmac_tx_handler\n"));
    desc_no = uu_wlan_get_msg_from_TX_Q();
    if (desc_no < 0)
    {
        //UU_WLAN_LOG_ERROR(("LMAC TX: Error uu_wlan_get_msg_from_TX_Q failed\n"));
        return UU_FAILURE;
    }

    //desc_no = UU_REG_LMAC_TX_DESC_ID;
    //UU_REG_LMAC_TX_DESC_READ_PENDING = 0;
    tx_frame_info = (uu_wlan_tx_frame_info_t *)uu_wlan_desc_get_tx_data_ptr(desc_no);

    /* put tx_frame_info in queue*/
    UU_WLAN_LOG_DEBUG(("LMAC ----- before AC Q ac is %d\n", tx_frame_info->ac));
    uu_wlan_put_msg_in_AC_Q(tx_frame_info->ac, tx_frame_info->desc_id);

    /* Post TX Ready event to CAP */
    ac = tx_frame_info->ac;
    if(!uu_wlan_cap_ctxt_g.ac_cntxt[tx_frame_info->ac].tx_ready)
    {
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_READY, &ac, 1);
    }
    return UU_SUCCESS;
} /* uu_wlan_tx_handler */
#endif

uu_int32 uu_wlan_tx_is_ampdu_status_clear(uu_uint8 ac)
{
    if (uu_wlan_tx_ampdu_status_g[ac].bitmap == 0)
    {
        return UU_SUCCESS;
    }
    return UU_FAILURE;
}


uu_void uu_wlan_tx_update_mpdu_status_for_reagg(uu_uint16 seq_num, uu_uchar ac, uu_bool status)
{
    uu_uchar i;

    if (!status)
    {
        return;
    }
    for (i = 0; i < UU_WLAN_MAX_MPDU_IN_AMPDU; i++)
    {
        if ((uu_wlan_tx_ampdu_status_g[ac].ssn + i)  == seq_num)
        {
            /* Clearing the mpdu status */
            if (uu_wlan_tx_ampdu_status_g[ac].bitmap & BIT(i))
            {
                uu_wlan_tx_ampdu_status_g[ac].pending_mpdu--;
            }
            /* Clearing the mpdu status */
            uu_wlan_tx_ampdu_status_g[ac].bitmap &= ~BIT(i);
        }
    }
    return;
}


uu_void uu_wlan_tx_phy_data_req(uu_uint8 ac)
{
    uu_wlan_tx_frame_info_t  *frame_info;
    uu_uint16 scf;
    uu_uint8  fc0;
    uu_uint8  tid;
    uu_uchar* ra_addr;
    uu_uchar* frame;
    uu_uint8  aggr_count;
    uu_uchar  phy_data_cfm = 0;
    uu_uint32 curr_pos = 0;
#ifdef UU_WLAN_TSF
    uu_uint64 beacon_timestamp = 0;
    uu_int32 i;
#endif

    if(uu_wlan_frame_is_cntl_frame_g == UU_TRUE)
    {
        /* control frame taking from control global buffer */
        frame_info =  &tx_ctl_frame_info_g;
        uu_wlan_frame_is_cntl_frame_g = UU_FALSE;
    }
    else if (uu_wlan_lmac_pspoll_frame_ready_g == UU_TRUE)/* power save related */
    {
        uu_wlan_lmac_pspoll_frame_ready_g = UU_FALSE;
        frame_info = ps_poll_frame_info_g;
        frame_info->txvec.L_length = frame_info->frameInfo.framelen;
        aggr_count = 0;
        frame = frame_info->mpdu;
        do
        {
            uu_wlan_phy_data_req(frame[curr_pos], &phy_data_cfm);
            curr_pos++;
        } while ((phy_data_cfm != 0) && (curr_pos < frame_info->frameInfo.framelen));
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_DATA_CONFIRM, UU_NULL, 0);
        return;
    }
    else if (uu_wlan_lmac_trigger_frame_ready_g == UU_TRUE)
    {
        uu_wlan_lmac_trigger_frame_ready_g = UU_FALSE;
        frame_info = ps_trigger_frame_info_g;
        frame_info->txvec.L_length = frame_info->frameInfo.framelen;
        aggr_count = 0;
        frame = frame_info->mpdu;
        do
        {
            uu_wlan_phy_data_req(frame[curr_pos], &phy_data_cfm);
            curr_pos++;
        } while ((phy_data_cfm != 0) && (curr_pos < frame_info->frameInfo.framelen));
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_DATA_CONFIRM, UU_NULL, 0);
        return;
    }/* power save related */
    else
    {
        frame_info =  uu_wlan_tx_get_frame_info(ac);
    }

    /* TODO: Looks like there is an issue here. If it is control frame, why should we look at aggr_count? */
    /* Get the actual aggregation count. If retranmiting, use 'pending_mpdu' */
    if(uu_wlan_tx_ampdu_status_g[ac].pending_mpdu)
    {
        aggr_count = uu_wlan_tx_ampdu_status_g[ac].pending_mpdu;
    }
    else
    {
        aggr_count = frame_info->aggr_count;
    }

    /* Aggregation case. Need to always send AMPDU, for VHT */
    /* TODO: Uncomment this check, after fixing 'uu_wlan_tx_update_len' */
    if ((aggr_count /*> 1) || (frame_info->txvec.format == UU_WLAN_FRAME_FORMAT_VHT*/))
    {
        if(frame_info->frameInfo.retry)
        {
            uu_wlan_tx_do_reaggregation(frame_info);
        }
        else
        {
            uu_wlan_tx_do_aggregation(frame_info);
        }
    }
    else /* Non-Aggregation case */
    {
        fc0 = uu_wlan_tx_get_frame_fc0(ac);
        if(fc0 == (IEEE80211_FC0_SUBTYPE_QOS | IEEE80211_FC0_TYPE_DATA))
        {
            tid = uu_wlan_tx_get_qos_frame_tid(ac);
            scf = uu_wlan_tx_get_frame_scf(ac);

            ra_addr = uu_wlan_tx_get_ra_address(ac);
            uu_wlan_cp_update_the_tx_sb (ra_addr, tid, scf);
        }
        /* set retry bit */
        if(frame_info->frameInfo.retry)
        {
            uu_wlan_tx_set_retry(ac);
        }

        frame = frame_info->mpdu;

        /* If the frame is beacon to be transmitted, then update the timestamp as per the TXSTATUS and the Local TSF */

        uu_wlan_fill_crc(0, frame, frame_info->frameInfo.framelen-4);

        do
        {
            /* update the beacon timestamp as local TSF + Phy_delay,
               while transmitting the first octet of the beacon timestamp. */
            if ((frame_info->frameInfo.beacon) && (curr_pos == IEEE80211_IS_BEACON_TIMESTAMP))
            {
#ifdef UU_WLAN_TSF /*TODO: Yet to implement in RTL */
                beacon_timestamp = uu_wlan_tsf_r + UU_WLAN_PHY_TX_BECN_DELAY_R;

                for (i = 7; i >= 0; i--)
                {
                    frame[curr_pos + i] = (uu_char) (beacon_timestamp & 0xff);
                    beacon_timestamp >>= 8;
                }
#endif
                /* Since while sending beacon frame, we update the beacon timestamp
                   with the current Local TSF when the first octet of the beacon timestamp
                   was in the MAC-PHY i/f. So, we need to recalculate the CRC for the frame.
                   */
                uu_wlan_fill_crc(0, frame, frame_info->frameInfo.framelen-4);
            }
            uu_wlan_phy_data_req(frame[curr_pos], &phy_data_cfm);
            curr_pos++;
        } while ((phy_data_cfm != 0) && (curr_pos < frame_info->frameInfo.framelen));
    }

    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_DATA_CONFIRM, UU_NULL, 0);

    return;
} /* uu_wlan_tx_phy_data_req */


/* This function returns the length of the expected response frame as per the input frame */
/* for unicast frames or with ToDS = 1 */
int uu_wlan_tx_get_expt_response_frame_length (uu_wlan_tx_frame_info_t  *frame_info)
{
    int resp_length;

    /* This check covers both multicast and broadcast as well. */
    if(UU_WLAN_IS_FRAME_RA_MULTICAST(frame_info->mpdu))
    {
        return 0;
    }

    /* For QoS */
    if (uu_wlan_tx_is_qos_data(frame_info->ac))
    {
        /* Implicit block ack case */
        if ((frame_info->txvec.is_aggregated) && (uu_wlan_tx_get_qos_ackpolicy(frame_info->ac) == 0))
        {
            resp_length = UU_BA_COMPRESSED_FRAME_LEN;
        }
        /* For normal ACK policy */
        if (uu_wlan_tx_get_qos_ackpolicy(frame_info->ac) == 0)
        {
            /* ACK as a response */
               resp_length = UU_ACK_FRAME_LEN;
        }

        /* 1 for no-ACK policy */
        /* 2 for No explicit acknowledgment or PSMP Ack.*/
        /* 3 for Block-ACK policy */
        else
        {
            /* Considering 0 length for no-ACK as per the implementation based on duration calculation */
            /* Not considering No explicit acknowledgment or PSMP Ack */
            resp_length = 0;
        }
    }
    /* For non-QOS */
    else
    {
        resp_length = UU_ACK_FRAME_LEN;
    }

    return resp_length;
} /* uu_wlan_tx_get_expt_response_frame_length */


/* TODO: Calculation of 'next_frame_length' (multiple-protection) */
uu_int32 uu_wlan_tx_send_more_data(uu_uchar ac, uu_int32 tx_nav, uu_wlan_ch_bndwdth_type_t bw)
{
    uu_uint16 duration;
    uu_wlan_tx_frame_info_t *frame_info;
    uu_uchar* ra_addr;
    /* Length of the next frame */
    int next_frame_length = 0;
    /* Expected response frame length for current frame */
    int resp_frame_length = 0;

    /* sending frame */
    frame_info = uu_wlan_tx_get_frame_info(ac);

    /* getting the length of the next frame followed of the same AC */
    //next_frame_length = uu_wlan_tx_get_next_frame_len(ac);
#if 0
    /* Txop limit 0 case or non-QoS case, with more fragment bit is clear */
    if(((UU_WLAN_TXOP_LIMIT_R(ac) == 0) || (! UU_WLAN_IEEE80211_QOS_MODE_R))
            && (!IEEE80211_IS_FC1_MOREFRAG_SET(frame_info->mpdu)))
    {
        /* If the previous frame is not control frame, it is error */
        if ((uu_wlan_cap_ctxt_g.tx_frame_info.mpdu[0] & IEEE80211_FC0_TYPE_MASK) != IEEE80211_FC0_TYPE_CTL)
        {
            UU_WLAN_LOG_DEBUG(("LMAC: tx_send_more_data - Last frame was not control frame\n"));
            return UU_FAILURE;
        }
    }
#else
    if (__txop_limit_check(frame_info, tx_nav) == UU_FAILURE)
    {   
        return UU_FAILURE;
    }
#endif

    /* broadcast frame with TODS is clear */
    /* Here ack goes false */
    ra_addr = uu_wlan_tx_get_ra_address(ac);
    if (UU_WLAN_IS_FRAME_RA_MULTICAST(frame_info->mpdu) &&
            !(uu_wlan_tx_get_frame_fc1(ac) & IEEE80211_FC1_DIR_TODS))
    {
        /* QoS and tx_nav */
        if(UU_WLAN_TXOP_LIMIT_R(ac) > 0)/* multiple protection case */
            duration = uu_calc_multip_frame_duration(ac, tx_nav);
        else
            duration = uu_calc_bcast_duration(&frame_info->txvec, 0);
    }
    else
    {
        /* Getting expected response frame length for the current PPDU */
        resp_frame_length = uu_wlan_tx_get_expt_response_frame_length (frame_info);

        if(UU_WLAN_TXOP_LIMIT_R(ac) > 0)/* multiple protection case */
            duration = uu_calc_multip_frame_duration(ac, tx_nav);
        else
            duration = uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, next_frame_length);
    }

    uu_wlan_tx_update_frame_duration(frame_info->mpdu, ac, duration);

    /* For multiple frame protection */
    duration = 0;
    duration = uu_calc_frame_duration (&frame_info->txvec);
    resp_frame_length = uu_wlan_tx_get_expt_response_frame_length (frame_info);
    duration += uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, 0);

    //printk(" TX HANDLER send more data @ txnav %d , duration %d\n",uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav, duration);
    if( uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav > 0)
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav -= duration;
    //printk(" TX HANDLER send more data @ txnav %d , duration %d\n",uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav, duration);

    if( uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav < 0)
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav = 0;
    /* For multiple frame protection */

    /* Assinging current txop BW in frame_info */
    frame_info->txvec.ch_bndwdth = bw; /* TODO: Should it be common? */

    /* Filling cap tx_frame_info */
    uu_wlan_cap_retain_tx_frame_info(frame_info);

    uu_wlan_tx_update_len (frame_info);

    /* give tx_start.req to phy */
    uu_wlan_phy_txstart_req((uu_uchar *)&frame_info->txvec, sizeof(uu_wlan_tx_vector_t));
    return UU_SUCCESS;
} /* uu_wlan_tx_send_more_data */


/* Sends first frame, on TXOP. Can be control (if required) or any frame */
/* TODO: Calculation of 'next_frame_length' (multiple-protection) */
uu_int32 uu_wlan_handle_txop_tx_start_req(uu_uchar ac, uu_int32 tx_nav, uu_wlan_ch_bndwdth_type_t bw)
{
    uu_uint16 duration = 0;
    uu_wlan_tx_frame_info_t *frame_info;
    bool cts_self = false; /* TODO: Presently not using self-cts */
    uu_uint32  len;
    uu_uchar *ra_addr;

    /* Expected response frame length for current frame */
    int resp_frame_length = 0;

    /* Length of the next frame */
    int next_frame_length = 0;

    /* power save related */
    if(uu_wlan_lmac_pspoll_frame_ready_g == UU_TRUE) 
    {
        frame_info = ps_poll_frame_info_g;
        /* Assinging current txop BW in frame_info */
        frame_info->txvec.ch_bndwdth = bw; /* TODO: Should it be common? */

        /* Filling cap tx_frame_info */
        uu_wlan_cap_retain_tx_frame_info(frame_info);

        uu_wlan_tx_update_len (frame_info);

        uu_wlan_phy_txstart_req((uu_uchar *)&frame_info->txvec, sizeof(uu_wlan_tx_vector_t));
        return UU_SUCCESS;
    }
    else if (uu_wlan_lmac_trigger_frame_ready_g == UU_TRUE)
    {
        frame_info = ps_trigger_frame_info_g;
        /* Assinging current txop BW in frame_info */
        frame_info->txvec.ch_bndwdth = bw; /* TODO: Should it be common? */

        /* Filling cap tx_frame_info */
        uu_wlan_cap_retain_tx_frame_info(frame_info);

        uu_wlan_tx_update_len (frame_info);

        uu_wlan_phy_txstart_req((uu_uchar *)&frame_info->txvec, sizeof(uu_wlan_tx_vector_t));
        return UU_SUCCESS;
    }/* power save related */

    uu_wlan_frame_is_cntl_frame_g = UU_FALSE;

    frame_info = uu_wlan_tx_get_frame_info(ac);

    len = uu_wlan_tx_update_len(frame_info);

    /* getting the length of the next frame followed of the same AC */
    //next_frame_length = uu_wlan_tx_get_next_frame_len(ac);

    /* For updating ack policy in unicast case for QoS data */

    ra_addr = uu_wlan_tx_get_ra_address(ac);
    if(UU_WLAN_IS_FRAME_RA_MULTICAST(frame_info->mpdu) && (!(IEEE80211_FC1_DIR_TODS & uu_wlan_tx_get_frame_fc1(ac))))
    {
        /* Duration for broadcast frames, with QoS & tx_nav */
        if(UU_WLAN_TXOP_LIMIT_R(ac) > 0)/* multiple protection case */
            duration = uu_calc_multip_first_frame_duration(ac, tx_nav);
        else
            duration = uu_calc_bcast_duration(&frame_info->txvec, 0);
    }
    else if(UU_WLAN_RTS_THRESHOLD_R > len)
    {
        if (frame_info->txvec.is_aggregated == 0)
        {
            /* Getting expected response frame length for the current PPDU */
            resp_frame_length = uu_wlan_tx_get_expt_response_frame_length (frame_info);

            if(UU_WLAN_TXOP_LIMIT_R(ac) > 0)/* multiple protection case */
                duration = uu_calc_multip_first_frame_duration(ac, tx_nav);
            else
                duration = uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, next_frame_length);
        }
    }
    else
    {
        uu_wlan_frame_is_cntl_frame_g = UU_TRUE;
        /* sending rts or self-cts frame */
        if (uu_wlan_cp_generate_rtscts(frame_info->txvec.format, frame_info->rtscts_rate, frame_info->txvec.ch_bndwdth, frame_info->txvec.modulation, &frame_info->mpdu[UU_RA_OFFSET]) == UU_SUCCESS)
        {
            resp_frame_length = uu_wlan_tx_get_expt_response_frame_length (frame_info);
            if(UU_WLAN_TXOP_LIMIT_R(ac) > 0)/* multiple protection case */
                duration = uu_calc_multip_first_frame_duration(ac, tx_nav);
            else
                duration = uu_calc_rtscts_duration (&frame_info->txvec, frame_info->rtscts_rate, cts_self, resp_frame_length);
            frame_info = &tx_ctl_frame_info_g;
            frame_info->txvec.L_length = frame_info->frameInfo.framelen;
        }
        else
        {
            return UU_FAILURE;
        }
    }

    uu_wlan_tx_update_frame_duration(frame_info->mpdu, ac, duration);

    /* For multiple frame protection */
    duration = 0;
    duration = uu_calc_frame_duration (&frame_info->txvec);
    resp_frame_length = uu_wlan_tx_get_expt_response_frame_length (frame_info);
    duration += uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, 0);

    //printk(" TX HANDLER send data @ txnav %d , duration %d\n",uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav, duration);
    if( uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav > 0)
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav -= duration;

    //printk(" TX HANDLER send data @ txnav %d , duration %d\n",uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav, duration);

    if( uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav < 0)
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav = 0;

    /* Updating the Bandwidth */
    frame_info->txvec.ch_bndwdth = bw;

    /* If bandwidth get decreased, then change the NON_HT_DUP_OFDM modulation to OFDM if bandwidth becomes 20 MHz */
    if (frame_info->txvec.modulation == NON_HT_DUP_OFDM && !bw)
    {
        frame_info->txvec.modulation = OFDM;
    }

#ifdef UU_WLAN_TPC
    /* Updating the Transmit Power Level */
    /* This will take care of Secondary channel assessment, VHT RTS procedure */
    if (frame_info->txvec.format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        if (frame_info->txvec.txpwr_level > uu_wlan_utpl_threshold_11ac_g[frame_info->txvec.ch_bndwdth])
        {
            frame_info->txvec.txpwr_level = uu_wlan_utpl_threshold_11ac_g[frame_info->txvec.ch_bndwdth];
        }
    }
    else
    {
        if (frame_info->txvec.txpwr_level > uu_wlan_utpl_threshold_11n_g[frame_info->txvec.ch_bndwdth])
        {
            frame_info->txvec.txpwr_level = uu_wlan_utpl_threshold_11n_g[frame_info->txvec.ch_bndwdth];
        }
    }
#endif /* UU_WLAN_TPC */

    uu_wlan_cap_retain_tx_frame_info(frame_info);

    /* send tx_start.req to phy */
    uu_wlan_phy_txstart_req((uu_uchar *)&frame_info->txvec, sizeof(uu_wlan_tx_vector_t));

    return UU_SUCCESS;
} /* uu_wlan_handle_txop_tx_start */


/* TODO: Move control frame generation & handling to CP */

/** Used to fill the fields in a PS-Poll frame.
 * params[in] Receiver address and association id (AID).
 * called by uu_wlan_pspoll_tx_handling function in Tx handler.
 */
static uu_uint32 uu_wlan_gen_pspoll_frame(uu_uchar *ra,uu_uint16 uu_aid)
{
    struct uu_80211_pspoll_frame *pspoll_frame;

    /*Allocate memory for the PS Poll packet to be prepared*/
    pspoll_frame = (uu_80211_pspoll_frame_t *) ps_poll_frame_info_g->mpdu;

    /* populate the PS Poll ctrl frame */
    pspoll_frame->fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL |IEEE80211_FC0_SUBTYPE_PSPOLL;

    pspoll_frame->fc[1] = 0x10;/* pwrmgt bit set*/

    /* aid fiels */
    pspoll_frame->aid[0] = 0x30;// (unsigned char)(uu_aid);
    /* two MostSignificantBits are always set in AID field */
    pspoll_frame->aid[1] = 0x00; // (unsigned char)(0xc0 |(uu_aid >> 8));

    /* RA field of PS Poll frame */
    UU_COPY_MAC_ADDR(pspoll_frame->ra, ra );
    /* TA field of PS Poll frame */
    UU_COPY_MAC_ADDR(pspoll_frame->ta, UU_WLAN_IEEE80211_STA_MAC_ADDR_R);

    /* calculate and fill CRC */
    uu_wlan_fill_crc(0,ps_poll_frame_info_g->mpdu,UU_PSPOLL_FRAME_LEN-4);
    return UU_PSPOLL_FRAME_LEN;
} /* uu_wlan_gen_ps_poll_frame */


/** Used to fill the fields in a QoS null data frame.
  * params[in] Receiver address and duration.
  * called by uu_wlan_qos_null_tx_handling function in Tx handler.
 **/
static uu_uint32 uu_wlan_gen_qos_null_frame( uu_uchar *ra,uu_uint16 duration)
{
    struct uu_80211_qos_null_frame *qos_null_frame;
    static uu_uint16 seq=0;

    /*Allocate memory for the QOS null packet to be prepared*/
    qos_null_frame = (uu_80211_qos_null_frame_t *) ps_trigger_frame_info_g->mpdu ;

    /* populate the QOS null data frame */
    qos_null_frame->fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_DATA |IEEE80211_FC0_SUBTYPE_QOS_NULL;

    /* set power management bit only when STA is in PS mode.*/
    if(uu_wlan_ps_mode_r)
    {
        qos_null_frame->fc[1] = 0x11;/* pwrmgt bit set, to DS bit set */
    }
    else
    {
        qos_null_frame->fc[1] = 0x01;/* pwrmgt bit reset , to DS bit set */
    }
    /* duration */
    qos_null_frame->dur[0] = 0x3c;// (unsigned char)(duration);
    qos_null_frame->dur[1] = 0x00;//(unsigned char)(duration >> 8);
    /* RA field of qos_null frame */
    UU_COPY_MAC_ADDR(qos_null_frame->add1,ra);
    /* TA field of qos_null frame */
    UU_COPY_MAC_ADDR(qos_null_frame->add2, UU_WLAN_IEEE80211_STA_MAC_ADDR_R);

    UU_COPY_MAC_ADDR(qos_null_frame->add3,ra);

    seq = ((seq - 4096)>=0) ? (seq-4096) : seq ;/* as the legth of the sequence number is 12 bits its maximum seq no value is 4095 */
    qos_null_frame->seq_ctl[0] = (unsigned char)(seq);
    qos_null_frame->seq_ctl[1] = (unsigned char)(seq >> 8);
    seq++;
    qos_null_frame->qos_ctl[0] = 0x00;
    qos_null_frame->qos_ctl[1] = 0x00;


    /* calculate and fill CRC */
    uu_wlan_fill_crc(0,ps_trigger_frame_info_g->mpdu,UU_QOS_NULL_FRAME_LEN-4);

    return UU_QOS_NULL_FRAME_LEN;
} /* uu_wlan_gen_qos_null_frame */


/* qos null tx handling */
uu_uint32 uu_wlan_lmac_qos_null_tx_handling( uu_uchar *ra , uu_uint16 duration )
{
    uu_uint8 ac = 0;
    memset(ps_trigger_frame_info_g, 0, sizeof(uu_wlan_tx_frame_info_t ));
    ps_trigger_frame_info_g->txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
    ps_trigger_frame_info_g->frameInfo.framelen = uu_wlan_gen_qos_null_frame(ra , duration );
    ps_trigger_frame_info_g->txvec.L_datarate = 0x0b;
    uu_wlan_lmac_trigger_frame_ready_g = UU_TRUE;
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_READY, &ac, 1);

    return UU_SUCCESS ;
} /* uu_wlan_lmac_qos_null_tx_handling */

/* pspoll tx handling */
uu_uint32 uu_wlan_lmac_pspoll_tx_handling( uu_uchar *ra , uu_uint16 aid )
{
    uu_uint8 ac = 0;
    memset(ps_poll_frame_info_g, 0, sizeof(uu_wlan_tx_frame_info_t ));
    ps_poll_frame_info_g->txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
    ps_poll_frame_info_g->frameInfo.framelen = uu_wlan_gen_pspoll_frame( ra , aid );
    ps_poll_frame_info_g->txvec.L_datarate = 0x0b;
    uu_wlan_lmac_pspoll_frame_ready_g = UU_TRUE;
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_READY, &ac, 1);

    return UU_SUCCESS ;
} /* uu_wlan_lmac_pspoll_tx_handling */


/* EOF */

