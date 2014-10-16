/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_duration.c                                     **
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
/* Contains functions and definitions for duration calculations. */

#include "uu_datatypes.h"
#include "uu_wlan_reg.h"
/* TODO: Get rid of uu_wlan_msgq.h & uu_wlan_tx_if.h. There shall be a way to know the length of Queued frames, without traversal */
#include "uu_wlan_msgq.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_rate.h"
#include "uu_wlan_duration.h"


/* Returns the duration for broadcast frames */
uu_uint16 uu_calc_bcast_duration(uu_wlan_tx_vector_t *txvec, uu_uint16 next_frag_len)
{
    uu_uint16 duration = 0;

#if 0 /* TODO: Review this & enable for non-HT QoS. Not required for HT/VHT */
    /* Assign next fragment length */
    uu_wlan_tx_vector_t bcast;
    bcast = *txvec;

    /* It will always be in non-HT */
    /* Still we do some checkings for other format */

    if (bcast.format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        bcast.tx_vector_user_params[UU_VHT_PHY_USR].apep_length = next_frag_len;
    }
    else if (bcast.format == UU_WLAN_FRAME_FORMAT_HT_GF || bcast.format == UU_WLAN_FRAME_FORMAT_HT_GF)
    {
        bcast.ht_length = next_frag_len;
    }
    else
    {
        bcast.L_length = next_frag_len;
    }
    /* If we don't implement fragmentation, then broadcast frames should have 0 in Duration */
    if(next_frag_len)
    {
        duration += uu_calc_frame_duration (&bcast);
        duration += dot11_sifs_timer_value;
    }
#endif /* #if 0 */
    return duration;
} /* uu_calc_bcast_duration */


/* TODO: It is being called even for control frame duration calculation. Review all usages */
/* as per the specs, basic rate is taken into account 802.11n REV_mb D12 section 9.7.6.5 */
/* Calculates the duration of control response, to be added to 'duration' in the current Tx frame */
static uu_uint16 uu_calc_ctrl_resp_duration(uu_wlan_tx_vector_t *txvec, int expected_resp_length)
{
    /* This is for the control response frame calculation in transmitting side
     * while calculating duration for RTS/self-CTS or normal PPDU or any case
     * So, taking txvec as reference, we take all the value present in txvec in
     * a local variable and change the local variable as we expect for the control
     * response frame, and then we calculate passing the address of the local variable
     */
    uu_wlan_tx_vector_t  resp_vec = *txvec;

    /* control frame is send in non-HT format for PPDU in HT-MF or VHT or non-HT. So, we will get the response in non-HT only */
    if (txvec->format == UU_WLAN_FRAME_FORMAT_HT_GF)
    {
        resp_vec.format = UU_WLAN_FRAME_FORMAT_HT_GF;
        resp_vec.mcs = uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
    }
    else
    {
        resp_vec.format = UU_WLAN_FRAME_FORMAT_NON_HT;

        if (txvec->format == UU_WLAN_FRAME_FORMAT_NON_HT)
        {
            resp_vec.L_datarate = uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->L_datarate);
        }
        else
        {
            if (txvec->format == UU_WLAN_FRAME_FORMAT_HT_MF)
            {
                resp_vec.L_datarate = uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
            }
            else
            {
                resp_vec.L_datarate = uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->tx_vector_user_params[0].vht_mcs);

            }
        }
    }

    /* Just to check for no-ack policy */
    if (expected_resp_length)
    {
        /* TODO: Length for VHT */

        /* for HT-GF */
        if (resp_vec.format == UU_WLAN_FRAME_FORMAT_HT_GF)
        {
            resp_vec.ht_length = expected_resp_length;
        }
        else /* for non-HT */
        {
            resp_vec.L_length = expected_resp_length;
        }
        return uu_calc_frame_duration (&resp_vec);
    }
    /* For no-ack policy */
    else
    {
        return 0; /* need to return 0 when response is not there */
    }

} /* uu_calc_ctrl_resp_duration */


/* This is for single_protection setting */
/* Since we are not supporting fragmentation, no need for next fragment length */
/* Refer to 8.2.5.2 a.6) */
uu_uint16 uu_calc_singlep_frame_duration(uu_wlan_tx_vector_t *txvec, int resp_frame_length, int next_frame_length)
{
    /* Initializing duration as 0, because for this function duration can be 0 also */
    uu_uint16  duration = 0;
    /* Initializing duration as 0, because for this function duration can be 0 also */

    if (resp_frame_length)
    {
        duration = dot11_sifs_timer_value;
        duration += uu_calc_ctrl_resp_duration(txvec, resp_frame_length);
    }

    /* If next frame (fragment of same frame OR another MPDU) is available, calculate its duration */
    if (next_frame_length) 
    {
        uu_wlan_tx_vector_t  singlep = *txvec;

        /* Assigning next_frame_length as per the frame format of the tx vector */
        if (singlep.format == UU_WLAN_FRAME_FORMAT_VHT)
        {
            singlep.tx_vector_user_params[UU_VHT_PHY_USR].apep_length = next_frame_length;
        }
        else if (singlep.format == UU_WLAN_FRAME_FORMAT_HT_GF || singlep.format == UU_WLAN_FRAME_FORMAT_HT_MF)
        {
            singlep.ht_length = next_frame_length;
        }
        else
        {
            singlep.L_length = next_frame_length;
        }

        duration *= 2; /* Double the current DUR to get 2 SIFS + 2 ACKs */
        duration += dot11_sifs_timer_value; /* add another SIFS and the frag time */
        duration += uu_calc_frame_duration (&singlep);
    }

    return duration;
} /* uu_calc_singlep_frame_duration */


/* RTS duration calculation in case of single protection */
uu_uint16 uu_calc_rtscts_duration (uu_wlan_tx_vector_t *txvec, uu_uint8 rts_cts_rate, bool cts_self, int expected_resp_length)
{
    uu_uint16 duration;

    uu_wlan_tx_vector_t rts_frame;
    rts_frame = *txvec;

    /* filling rtscts frame rate as per the PPDU format */
    if (txvec->format == UU_WLAN_FRAME_FORMAT_NON_HT)
    {
        rts_frame.L_datarate = rts_cts_rate;
    }
    else /* for HT or VHT PPDU */
    {
        if (txvec->format == UU_WLAN_FRAME_FORMAT_VHT)
        {
            rts_frame.tx_vector_user_params[0].vht_mcs = rts_cts_rate;
        }
        else
        {
            rts_frame.mcs = rts_cts_rate;
        }
    }

    if (!cts_self) /* FOLLOWS RTS_CTS */
    {
        duration = 3 * dot11_sifs_timer_value; /* sifs after rts, after cts and after dur of pending frame */
        duration += uu_calc_ctrl_resp_duration(&rts_frame, UU_CTS_FRAME_LEN); /* CTS + 3 (SIFS) + DUR OF PENDING FRAME + ACK */
    }
    else /* CTS TO SELF */
    {
        duration = 2 * dot11_sifs_timer_value; /* sifs after cts and after dur of pending frame */
    }

    /* calculate the duration of pending frame for single protection */
    duration += uu_calc_frame_duration (txvec);
    /* Control response length should come from the calling function
     * This can be for ack, block ack, block ack compressed, multi-tid block ack, no-ack (as 0)
     */
    if (expected_resp_length)
    {
        /* Here txvec is because the response is for the PPDU with txvec */
        duration += uu_calc_ctrl_resp_duration (txvec, expected_resp_length);
    }
    else
    {
        /* for NO-ACK policy, we remove sifs duration we added before */
        duration -= dot11_sifs_timer_value;
    }

    return duration;
} /* uu_calc_rtscts_duration */


/* Calculates the duration for implicit BA */
uu_uint16 uu_calc_impl_BA_duration(uu_wlan_tx_vector_t *txvec, int expected_BA_length)
{
    uu_uint16 duration;

    /* Implicit Block Ack frame or Block Ack request frame should have duration of SIFS plus BLOCK ACK response or ACK response required. */
    /* Adding SIFS */
    duration = dot11_sifs_timer_value;

    /* Adding control response frame duration */
    duration += uu_calc_ctrl_resp_duration(txvec, expected_BA_length);

    return duration;
} /* uu_calc_impl_BA_duration */


#if 0 /* Alternative implementation is used for multiple frame protection */
/* This is to calculate duration required for pending frames */
/* This is for multiple_protection setting */
static uu_uint16 uu_wlan_pending_frame_duration (uu_wlan_tx_vector_t *txvec, int resp_length)
{
    uu_uint16 duration = 0; /* Initializing duration with 0 */
#if 0 /* TODO: to be done later for multiple txop protection */
    uu_bool ack;
    uu_uint16 duration_ack = 0;
    bool sgi;

    sgi = !frame_info->txvec.is_long_guard_intvl;

    /* Tx frame required ack frame or not */
    ack = __is_tx_frame_required_ack_resp(frame_info);
    if (ack)
    {
        duration_ack += dot11_sifs_timer_value;
        duration_ack += uu_calc_ctrl_resp_duration(frame_info, rate, is_long_preamble, UU_CTRL_RSP_ACK_CTS); /* calculating ACK duration */
    }
    /* TODO : Do changes as per aggregation is implemented */
    /* This is to check for the pending frames of the same AC */
    while (frame_info != NULL || )
    {
        duration = 2 * duration_ack; /* Double the current DUR to get 2 SIFS + 2 ACKs */
        duration += dot11_sifs_timer_value; /* add another SIFS and the frag time */
        /* TODO : Do changes as per aggregation is implemented */
        duration += uu_calc_frame_duration (frame_info->next.frame_len, rate, is_long_preamble, frame_info->txvec.ch_bndwdth, sgi, frame_info->frameInfo.frame_format, frame_info->txvec.stbc, frame_info->txvec.num_ext_ss);
        if (frame_info->next != NULL)
            frame_info = frame_info->next;
    }

#endif
    return duration;
} /* uu_wlan_pending_frame_duration */


/* This is for multiple protection setting */
/* It deals only with TXOP holder, and not with TXOP responder */
/* Refer to 8.2.5.2 b. */
/*
 * 1) If TTXOP = 0 and TEND_NAV = 0, then D = TSINGLE-MSDU – TPPDU
 * 2) Else if TTXOP = 0 and TEND_NAV > 0, then D = TEND-NAV – TPPDU
 * 3) Else if TEND-NAV = 0, then min(TPENDING, TTXOP – TPPDU) ≤ D ≤ TTXOP – TPPDU
 * 4) Else TEND-NAV – TPPDU ≤ D ≤ TTXOP-REMAINING – TPPDU
 */

/*
 * TSINGLE-MSDU is the estimated time required for the transmission of the allowed frame exchange sequence (for a TXOP limit value of 0), 
 *      including applicable IFS durations
 * TPPDU is the time required for transmission of the current PPDU

 * TTXOP is the value of dot11EDCATableTXOPLimit (dot11EDCAQAPTableTXOPLimit for the AP) for that AC
 * TTXOP-REMAINING is TTXOP less the time already used time within the TXOP

 * TEND-NAV is the remaining duration of any NAV set by the TXOP holder, or 0 if no NAV has been established
 * TPENDING is the estimated time required for the transmission of
 *  - Pending MPDUs of the same AC
 *  - Any associated immediate response frames
 *  - Any NDP transmissions and explicit feedback response frames
 *  - Applicable IFS durations
 *  - Any RDG
 */
/* This is for data frame case */
/* For multiple txop protection */
/* In txvec, we get the frame to be transmitted, either RTS or self-CTS or any frame */
/* uu_uint16 uu_calc_multip_duration(uu_wlan_tx_vector_t *txvec, int T_txop, int T_endnav, int T_txopremaining, uu_uint8 rts_cts_rate, bool cts_self, bool ack, int resp_len) */
/* If TXOP limit is 0 and TX-Endnav is 0, then give the parameter for RTS or self-CTS same as for single protection. ie, txvec should be for MPDU to be transmitted */
/* for cts-self or for no-ack policy, resp_length is 0 */
uu_uint16 uu_calc_multip_duration(uu_wlan_tx_vector_t *txvec, int T_txop, int T_endnav, int T_txopremaining, uu_uint8 rts_cts_rate, int ctrl_req_frame, int resp_len)
{
    uu_uint16 duration;
    uu_uint16 pending_dur;
    bool cts_self = false;

    if (T_txop)
    {
        /* Else TEND-NAV – TPPDU ≤ D ≤ TTXOP-REMAINING – TPPDU */
        if (T_endnav)
        {
            if (T_endnav > T_txopremaining)
            {
                duration = T_txopremaining - uu_calc_frame_duration (txvec);
            }
            else
            {
                duration = T_endnav - uu_calc_frame_duration (txvec);
            }
            printk("duration.c multiple txop, txop > 0 and tendnav > 0, then duration is: %d\n", duration);
        }
        /* Else if TEND-NAV = 0, then min(TPENDING, TTXOP – TPPDU) ≤ D ≤ TTXOP – TPPDU */
        else
        {
            pending_dur = uu_wlan_pending_frame_duration (txvec, resp_len);
            duration = T_txop - uu_calc_frame_duration (txvec);
            if (pending_dur < duration)
            {
                duration = pending_dur;
            }
            printk("duration.c multiple txop, txop > 0 and tendnav = 0, then duration is: %d\n", duration);
        }
    }

    else /* This will behave like single protection */
    {
        /* Else if TTXOP = 0 and TEND_NAV > 0, then D = TEND-NAV – TPPDU */
        /* If it is data frame with RTS protection, then while sending RTS already the Tx STA have established NAV till the pending frames */
        if (T_endnav)
        {
            /* The duration of these ppdu will be having duration til the response required for this */
            duration = T_endnav - uu_calc_frame_duration (txvec);
            /* uu_uint16 uu_calc_singlep_frame_duration(txvec, resp_len, next_frame_length) */
            printk("duration.c multiple txop, txop = 0 and tendnav > 0, then duration is: %d\n", duration);
        }
        /* for rts frame or self-cts frame or frame with no RTS/self-CTS */
        /* If TTXOP = 0 and TEND_NAV = 0, then D = TSINGLE-MSDU – TPPDU */
        else
        {
            /* Here txvec is because the response is for the PPDU with txvec */
            /* This has duration like rts till the followed frame same as single protection */
            /* Will this duration if rts then contains till CTS or till the ACK of the followed frame */

            /* This is for only response frame */
            if (ctrl_req_frame) /* RTS or self-CTS */
            {
                if (ctrl_req_frame == UU_CTS_FRAME_LEN)
                {
                    cts_self = true;
                }
                duration = uu_calc_rtscts_duration (txvec, rts_cts_rate, cts_self, resp_len);
            }
            else
            {
                duration = uu_calc_ctrl_resp_duration(txvec, resp_len);
            }
            /* else, for data frame with no-RTS we wait for ack, and not for the next frame and for RTS we update our ack till ack of the corresponding frame */
            /* duration = uu_calc_rtscts_duration (txvec, rts_cts_rate, cts_self, resp_len); */
            printk("duration.c multiple txop, txop = 0 and tendnav = 0, then duration is: %d\n", duration);
        }
    }

    return duration;
} /* uu_calc_multip_duration */

#else /* The functions in this block are alternative to above for multiple frame transmission */

/*
 * Multiple protection:
 * To transmit multiple frames, we calculate duration for available frames in Tx buffer for an AC.
 * If the first frame length is more than RTS threshold, we are sending RTS frame.
 * For next frames it is  not required, as it is already protected by NAV.
 * Giving duration value for Broadcast/multicast frames similar to that of normal data frames.
 * Any aggregated packet that is unable to send in present TXOP should wait for next TXOP, in this implementation.
 * Aggregated packet Tx duration that crosses the total TXOP limit is not considering in this implementation.
 * All duration calculations below include corresponding response frame Tx and IFS values.
*/

/* This gives a non-zero response frame value, if
 *  - the frame is individual frame or
 *  - first frame in an AMPDU for duration calculation in multiple protection.
 * params@[in] frame_info Frame info of the corresponding frame.
 */
static uu_uint16 uu_get_multip_resp_frame_length(uu_wlan_tx_frame_info_t *frame_info)
{
    uu_uint16 resp_frame_length;

    if((frame_info->txvec.is_aggregated == 1) && (frame_info->aggr_count != 1))
    {
        return 0;
    }

    resp_frame_length = uu_wlan_tx_get_expt_response_frame_length (frame_info);

    return resp_frame_length;
}


uu_uint16 uu_calc_multip_first_frame_duration( uu_uint8 ac, uu_uint32 txop_limit)
{
    uu_uint16 resp_frame_length;
    uu_uint16 size;
    uu_uint16 frame_duration; /* 1st (current sending) frame duration */
    uu_uint16 duration = 0; /* Duration required, except for the 1st frame */
    uu_uint32 offset;
    uu_wlan_tx_frame_info_t *frame_info;

    /* calculating duration for the first frame in the AC q i.e the frame to be sent.*/
    frame_info = uu_wlan_tx_get_frame_info(ac);
    resp_frame_length = uu_get_multip_resp_frame_length(frame_info);
    frame_duration = uu_calc_frame_duration (&frame_info->txvec);

    if(UU_WLAN_RTS_THRESHOLD_R <= frame_info->frameInfo.framelen)
    {
        frame_duration = uu_calc_ctrl_resp_duration(&frame_info->txvec, UU_RTS_FRAME_LEN);
        duration += uu_calc_rtscts_duration (&frame_info->txvec, frame_info->rtscts_rate, 0 , resp_frame_length); 
    }
    else
    {
        duration += uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, 0 );
    }

    /* calculating duration for pending frames.*/
    size = uu_wlan_tx_get_no_of_pkts(ac);

    for(offset = 1 ; offset < size ; offset++)
    {
        frame_info = uu_wlan_tx_get_frame_info_at(ac, offset);

        if(frame_info != UU_NULL)
        {
            resp_frame_length = uu_get_multip_resp_frame_length(frame_info);
            duration += dot11_sifs_timer_value;
            duration += uu_calc_frame_duration (&frame_info->txvec);
            duration += uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, 0 );
        }
    }
    //printk("uu_calc_multip_first_frame_duration - TXOP Limit: %d, Total duration for frames in Tx Q: %d\n", txop_limit, duration);

    if(duration > txop_limit)
    {
        /* pending frames duration is more than the remaining TXOP so returning the remaining TXOP minus current frame Tx time.*/
        return (txop_limit - frame_duration);
    }
    else
    {
        return duration;
    }
} /* uu_calc_multip_first_duration */


/* TODO: Verify the spec. The duration can not be increased, in the middle of the TXOP */
/** Calculates duration for the later frames in multiple frame transmission.
 * Duration is estimated for the frames currently available in LMAC Tx buffer of particular AC.
 * If the duration is less than the remaining TXOP, returns the calculated duration.
 *   Otherwise returrs the remaining TXOP-TPPDU
 * TPPDU is the time required to send the current frame transmitting.
 */
uu_uint16 uu_calc_multip_frame_duration( uu_uint8 ac, uu_uint32 tx_nav )
{
    uu_uint16 resp_frame_length, size, frame_duration, duration = 0;
    uu_uint32 offset;
    uu_wlan_tx_frame_info_t *frame_info;
    /* calculating duration for the first frame in the AC q i.e the frame to be sent.*/
    frame_info = uu_wlan_tx_get_frame_info(ac);
    resp_frame_length = uu_get_multip_resp_frame_length(frame_info);
    frame_duration = uu_calc_frame_duration (&frame_info->txvec);
    duration += uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, 0 );
    /* calculating duration for pending frames.*/
    size = uu_wlan_tx_get_no_of_pkts(ac);

    for(offset = 1 ; offset < size ; offset++)
    {
        frame_info = uu_wlan_tx_get_frame_info_at(ac, offset);

        if(frame_info != UU_NULL)
        {
            resp_frame_length = uu_get_multip_resp_frame_length(frame_info);
            duration += dot11_sifs_timer_value;
            duration += uu_calc_frame_duration (&frame_info->txvec);
            duration += uu_calc_singlep_frame_duration (&frame_info->txvec, resp_frame_length, 0 );
        }
    }
    
    //printk("uu_calc_multip_frame_duration - TXOP Limit: %d, Total duration for frames in Tx Q: %d\n", txop_limit, duration);
    if(duration > tx_nav)
    {  
        /* as the pending frames duration is more then  TXOP-remaining(Tx-NAV) we are returning TX-NAV without 
         * the current frame duration as duration to fill in the sending frame
         */
        return (tx_nav - frame_duration);
    }
    else
    {
        return duration;
    }
} /* uu_calc_multip_frame_duration */

#endif /* Multiple frame transmission / protection */


/* EOF */

