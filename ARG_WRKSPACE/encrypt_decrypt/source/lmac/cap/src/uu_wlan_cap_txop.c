/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_txop.c                                     **
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
#include "uu_wlan_frame.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_utils.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_rate.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_cap_init.h"
#include "uu_wlan_cap_slot_timer.h"
#include "uu_wlan_cap_cm.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_cap_sm.h"
#include "uu_wlan_cap_txop.h"
#include "uu_wlan_lmac_sta_info.h"

#ifdef UU_WLAN_TPC
#include "uu_wlan_lmac_tpc.h"

/* This variable is defined to take care of the transmit
   power level for the retransmission case */
static uu_uint8 orig_txpwr_level = 0;
#endif

/** Indicates whether we are waiting for ACK of PS-POLL.
 * Set on sending PS-POLL.
 * Reset after receiving ACK.
 */
static uu_bool uu_ps_poll_ack_g = UU_FALSE;
/** Indicates whether we are waiting for ACK of QoS NULL data.
 * Set on sending QoS NULL data.
 * Reset after receiving ACK.
 */
static uu_bool uu_ps_trigger_ack_g = UU_FALSE;


static uu_int32 wait_for_rx_rx_data_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 wait_for_rx_rx_end_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 wait_for_rx_phy_error(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 wait_for_rx_rx_start_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 wait_for_txnav_end_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 wait_for_rx_tx_start_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 wait_for_rx_tx_data_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 wait_for_rx_tx_end_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);


static uu_wlan_ch_bndwdth_type_t calc_bw_for_tx_frame(uu_wlan_ch_bndwdth_type_t  ch_bndwdth)
{
    uu_uint8  ch_busy_value;

    ch_busy_value = (uu_wlan_cap_ctxt_g.prev_channel_list <= uu_wlan_cap_ctxt_g.updated_channel_list) ? uu_wlan_cap_ctxt_g.prev_channel_list : uu_wlan_cap_ctxt_g.updated_channel_list;
    switch (ch_busy_value)
    {
        /* For 40mhz bandwidth */
        case UU_WLAN_BUSY_CH_SECONDARY40:
            {
                ch_bndwdth = (ch_bndwdth == CBW40) ? CBW40: CBW20;
            }
            break;
            /* For 80mhz bandwidth */
        case UU_WLAN_BUSY_CH_SECONDARY80:
            {
                ch_bndwdth = (ch_bndwdth == CBW80) ? CBW80:
                    ((ch_bndwdth == CBW40) ? CBW40 : CBW20);
            }
            break;
            /* For 160mhz bandwidth */
        case UU_WLAN_CH_IDLE:
            {
                ch_bndwdth = (ch_bndwdth >= CBW160) ? ch_bndwdth :
                    ((ch_bndwdth == CBW80) ?
                     CBW80: ((ch_bndwdth == CBW40) ? CBW40 : CBW20));
            }
            break;
        default:
            {
                ch_bndwdth = CBW20;
            }
            break;
    }
    return  ch_bndwdth;
} /* calc_bw_for_tx_frame */


static uu_int32 uu_wlan_handle_txop_tx_start(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_tx_frame_info_t *frame_info;
    uu_int8  ret  = UU_SUCCESS;
    uu_wlan_ch_bndwdth_type_t bw;

    if(uu_wlan_lmac_pspoll_frame_ready_g)
    {
        frame_info = ps_poll_frame_info_g;
        uu_ps_poll_ack_g = UU_TRUE;
    }
    else if(uu_wlan_lmac_trigger_frame_ready_g)
    {
        frame_info = ps_trigger_frame_info_g;
        uu_ps_trigger_ack_g = UU_TRUE;
    }
    else
    {
        frame_info = uu_wlan_get_msg_ref_from_AC_Q(uu_wlan_cap_ctxt_g.txop_owner_ac);
    }

    if(frame_info == UU_NULL)
    {
        /* reset tx_ready flag */
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_ready = 0;
        return SUPP_SM_TX_END;
    }

    bw = calc_bw_for_tx_frame(frame_info->txvec.ch_bndwdth);

    ret =  uu_wlan_handle_txop_tx_start_req(uu_wlan_cap_ctxt_g.txop_owner_ac, uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav, bw);
    if (ret == UU_SUCCESS)
    {
        return SUPP_SM_STATE_CONTINUE;
    }
    return SUPP_SM_TX_END;

} /* uu_wlan_handle_txop_tx_start */


static uu_int32 txop_tx_end_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_uchar ack_policy;

    /* resetting tx_prog flag */
    uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_prog = 0;

#ifdef UU_WLAN_IBSS
    /* Assign the register uu_wlan_last_beacon_tx_r as 1, after sending beacon */
    if (IEEE80211_IS_BEACON_FRAME(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu))
    {
        UU_WLAN_LAST_BEACON_TXED_R = 1;
    }
#endif

    /* broad cast frame handling(with TODS bit is 0) */
    if(UU_WLAN_IS_FRAME_RA_MULTICAST(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu) &&
            (!IEEE80211_IS_FRAME_TODS(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu)))
    {
        uu_wlan_clear_mpdu(uu_wlan_cap_ctxt_g.txop_owner_ac, UU_TRUE);
        if(! UU_WLAN_IEEE80211_QOS_MODE_R) /* non-QoS */
        {
            return SUPP_SM_TX_END;
        }

        /* For IBSS, if we send multicast or groupcast, then we need to do backoff */
        if (UU_WLAN_IEEE80211_OP_MODE_R == UU_WLAN_MODE_ADHOC)
        {
            return SUPP_SM_TX_END;
        }
        UU_WLAN_LOG_DEBUG(("LMAC: txop_tx_end_confirm in multicast_frame\n"));
        return SUPP_SM_START_SENDING;
    }

    /* qos frames with BA(setup) case */
    if ((UU_WLAN_IEEE80211_QOS_MODE_R) &&
        (IEEE80211_IS_FC0_QOSDATA(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu)))
    {
        if(IEEE80211_IS_FRAME_4ADDR(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu))
        {
            ack_policy = UU_WLAN_GET_4ADDR_QOS_ACKPOLICY_VALUE(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu);
        }
        else
        {
            ack_policy = UU_WLAN_GET_QOS_ACKPOLICY_VALUE(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu);
        }

        if(ack_policy != UU_WLAN_ACKPOLICY_NORMAL)
        {
            uu_wlan_clear_mpdu(uu_wlan_cap_ctxt_g.txop_owner_ac, UU_TRUE);
            UU_WLAN_LOG_DEBUG(("LMAC: txop_tx_end_confirm at qos_data frame\n"));
            return SUPP_SM_START_SENDING;
        }
    }
    /* for running rts/cts timer */
    if (IEEE80211_IS_FC0_RTS(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu))
    {
        uu_wlan_cp_set_waiting_for_cts();
        uu_wlan_cap_ctxt_g.cts_timer_val = UU_WLAN_CTS_TIMER_VALUE_R;
        uu_wlan_cap_ctxt_g.ack_timer_val =  0;
    }
    else
    {
        uu_wlan_cp_set_waiting_for_ack();
        uu_wlan_cap_ctxt_g.ack_timer_val = UU_WLAN_ACK_TIMER_VALUE_R;
        uu_wlan_cap_ctxt_g.cts_timer_val = 0;
    }
    return SUPP_SM_WAIT_FOR_RX;
} /* txop_tx_end_confirm */


static uu_int32 uu_wlan_cap_send_more_data(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_tx_frame_info_t *frame_info;
    uu_int8   ret  = UU_SUCCESS;
    uu_wlan_ch_bndwdth_type_t bw;

    frame_info = uu_wlan_get_msg_ref_from_AC_Q(uu_wlan_cap_ctxt_g.txop_owner_ac);
    if(frame_info == UU_NULL)
    {
        /** NOTE: Resetting Tx NAV to truncate the TXOP, as we are
         * not continuing TXOP for upcoming frames in remaining TXOP time.
         * TODO: This is not matching with the intended purpose of 'tx_nav'. Review all usages of 'tx_nav'.
         */
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav = 0;

        /* reset tx_ready flag */
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_ready = 0;
        return SUPP_SM_TX_END;
    }

#if 0 /* enable this funtionality, if the same functionality is disabled in tx_handler */
    /* Txop limit 0 case or non-QoS case, with more fragment bit is clear */
    if(((UU_WLAN_TXOP_LIMIT_R(ac) == 0) || (! UU_WLAN_IEEE80211_QOS_MODE_R))
            && (!IEEE80211_IS_FC1_MOREFRAG_SET(frame_info->mpdu)))
    {
        /* If the previous frame is not control frame, it is error */
        if ((uu_wlan_cap_ctxt_g.tx_frame_info.mpdu[0] & IEEE80211_FC0_TYPE_MASK) != IEEE80211_FC0_TYPE_CTL)
        {
            UU_WLAN_LOG_DEBUG(("LMAC: tx_send_more_data - Last frame was not control frame\n"));
            return SUPP_SM_TX_END;
        }
    }
#endif

    bw = calc_bw_for_tx_frame(frame_info->txvec.ch_bndwdth);

    ret = uu_wlan_tx_send_more_data(uu_wlan_cap_ctxt_g.txop_owner_ac, uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav, bw);
    if (ret == UU_SUCCESS)
    {
        return SUPP_SM_STATE_CONTINUE;
    }

    /* Tx-NAV expired */
    if (uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav == 0)
    {
        UU_WLAN_LOG_DEBUG(("MFT- TXOP limit completed"));
        return SUPP_SM_TXOP_LIMIT_COMP;
    }

    /* TODO: If the remaining TXOP duration is insufficient for next frame, then also the control comes here.
  This is not matching with the comment for the return code */
    return SUPP_SM_TX_END;
} /* uu_wlan_cap_send_more_data */


/* incrementing tx_owner(ac) cw value */
static uu_void uu_wlan_increment_cw_value(uu_void)
{
    if(UU_WLAN_IEEE80211_QOS_MODE_R)
    {
        if(uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val <
                UU_WLAN_CW_MAX_VALUE_R(uu_wlan_cap_ctxt_g.txop_owner_ac))
        {
            uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val =
                ((uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val << 1) | 1);
        }
    }
    else/* non-QoS mode */
    {
        /* Note: For beacons, control will not come here as there is no ACK */
        if(uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val <
                UU_WLAN_CWMAX_VALUE_R)
        {
            uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val =
                ((uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val << 1) | 1);
        }
    }
} /* uu_wlan_increment_cw_value */


static uu_void uu_wlan_reset_cw_value(uu_void)
{
    if(UU_WLAN_IEEE80211_QOS_MODE_R)
    {
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val = UU_WLAN_CW_MIN_VALUE_R(uu_wlan_cap_ctxt_g.txop_owner_ac);
    }
    else /* non-QoS */
    {
#ifdef UU_WLAN_BQID
        /* TODO: Normally this should not be kept, because we won't retransmit Beacon frames */
        if (uu_wlan_cap_ctxt_g.txop_owner_ac == UU_WLAN_BEACON_QID)
        {
            uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val = UU_WLAN_CW_MIN_VALUE_R(uu_wlan_cap_ctxt_g.txop_owner_ac);
        }
        else
#endif
        {
            uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val = UU_WLAN_CWMIN_VALUE_R;/* dcf cw min value */
        }
    }
}

static uu_int32 _handle_data_short_retry(uu_wlan_tx_frame_info_t *frame_info)
{
    //uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].SSRC++;
    uu_wlan_ShortRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac]++;

#ifdef UU_WLAN_TPC
    /* We store the original txpwr_level value in orig_txpwr_level
       when we need to do retransmission */
    if (uu_wlan_ShortRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] == 1)
    {
        orig_txpwr_level = frame_info->txvec.txpwr_level;
    }
#endif

    if(uu_wlan_ShortRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] >= UU_WLAN_DOT11_SHORT_RETRY_COUNT_R)
    {
        /* Reached frame retry limit. Discard the packet*/
        if (frame_info->aggr_count == 0)
        {
            uu_wlan_clear_mpdu(uu_wlan_cap_ctxt_g.txop_owner_ac, UU_FALSE);
        }
        else
        {
            uu_wlan_clear_ampdu(uu_wlan_cap_ctxt_g.txop_owner_ac);
        }
        /* reset cw_val value to cw min */
        uu_wlan_reset_cw_value();
    }
    else /* Still some retries are possible */
    {
        uu_wlan_increment_cw_value();

        /* TX module updates the retry bit in the packet */
        uu_wlan_tx_set_retry_bit(uu_wlan_cap_ctxt_g.txop_owner_ac);

        if (uu_wlan_ShortRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] == UU_WLAN_DOT11_SHORT_RETRY_COUNT_R >> 1)
        {
            uu_wlan_tx_update_with_fb_rate(uu_wlan_cap_ctxt_g.txop_owner_ac);
#ifdef UU_WLAN_TPC
            /* Keep the original TPL when the rate changes to fallback rate */
            frame_info->txvec.txpwr_level = orig_txpwr_level;
#endif
        }

#ifdef UU_WLAN_TPC
        else
        {
            /* For TPC Report, DO NOT change TPL, And Change TPL for others for retransmission */
            if (!((frame_info->mpdu[0] == (IEEE80211_FC0_TYPE_MGT | IEEE80211_FC0_SUBTYPE_ACTION))
                        && (frame_info->mpdu[9] == 0 /* Action category = 0 for TPC Report */
                            && frame_info->mpdu[10] == 3 /* Management action = 3 for TPC Report */
                            && frame_info->mpdu[12] == 35 /* Element Id = 35 for TPC Report */)))
            {
                frame_info->txvec.txpwr_level = uu_calc_tpl_retry_frame (frame_info->txvec.txpwr_level,
                        frame_info->txvec.format, frame_info->txvec.modulation,
                        frame_info->txvec.ch_bndwdth);
            }
        }
#endif

    }
    return SUPP_SM_TX_FAILURE;
}

static uu_int32 _handle_data_long_retry(uu_wlan_tx_frame_info_t *frame_info)
{
    //uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].SLRC++;
    uu_wlan_LongRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac]++;

#ifdef UU_WLAN_TPC
    /* We store the original txpwr_level value in orig_txpwr_level
       when we need to do retransmission */
    if (uu_wlan_LongRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] == 1)
    {
        orig_txpwr_level = frame_info->txvec.txpwr_level;
    }
#endif

    if(uu_wlan_LongRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] == UU_WLAN_DOT11_LONG_RETRY_COUNT_R)
    {
        if (frame_info->aggr_count == 0)
        {
            /* Reached frame retry limit. Discard the packet*/
            uu_wlan_clear_mpdu(uu_wlan_cap_ctxt_g.txop_owner_ac, UU_FALSE);
        }
        else
        {
            uu_wlan_clear_ampdu(uu_wlan_cap_ctxt_g.txop_owner_ac);
        }
        /* reset cw_val value to cw min */
        uu_wlan_reset_cw_value();
    }
    else
    {
        uu_wlan_increment_cw_value();

        /* TX module updates the retry bit in the packet */
        uu_wlan_tx_set_retry_bit(uu_wlan_cap_ctxt_g.txop_owner_ac);

        /* Use fallback rate, if we reach half of the Long-Retry-Count-Limit */
        if (uu_wlan_LongRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] == (UU_WLAN_DOT11_LONG_RETRY_COUNT_R>>1))
        {
            uu_wlan_tx_update_with_fb_rate(uu_wlan_cap_ctxt_g.txop_owner_ac);
#ifdef UU_WLAN_TPC
            /* Keep the original TPL when the rate changes to fallback rate */
            frame_info->txvec.txpwr_level = orig_txpwr_level;
#endif

        }

#ifdef UU_WLAN_TPC
        else
        {
            /* For TPC Report, DO NOT change TPL, And Change TPL for others for retransmission */
            if (!((frame_info->mpdu[0] == (IEEE80211_FC0_TYPE_MGT | IEEE80211_FC0_SUBTYPE_ACTION))
                        && (frame_info->mpdu[9] == 0 /* Action category = 0 for TPC Report */
                            && frame_info->mpdu[10] == 3 /* Management action = 3 for TPC Report */
                            && frame_info->mpdu[12] == 35 /* Element Id = 35 for TPC Report */)))
            {
                frame_info->txvec.txpwr_level = uu_calc_tpl_retry_frame (frame_info->txvec.txpwr_level,
                        frame_info->txvec.format, frame_info->txvec.modulation,
                        frame_info->txvec.ch_bndwdth);
            }
        }
#endif

    }
    return SUPP_SM_TX_FAILURE;
}

static uu_int32 _handle_rts_retry(uu_wlan_tx_frame_info_t *frame_info)
{
    //uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].SSRC++;
    uu_wlan_ShortRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac]++;

#ifdef UU_WLAN_TPC
    /* We store the original txpwr_level value in orig_txpwr_level
       when we need to do retransmission */
    if (uu_wlan_ShortRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] == 1)
    {
        orig_txpwr_level = frame_info->txvec.txpwr_level;
    }
#endif

    /* If retransmission limit reached */
    if(uu_wlan_ShortRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] == UU_WLAN_DOT11_SHORT_RETRY_COUNT_R)
    {
        /*
         * Ref: 'Recovery procedures and retransmit limits' under 'DCF'.
         * If the RTS transmission fails, the SRC for the MSDU or MMPDU
         *  and the SSRC are incremented. This process shall continue
         *  until the number of attempts to transmit that MSDU or MMPDU
         *  reaches dot11ShortRetryLimit.
         * Assumption: 'Dropping data frame' logic applies, even for this.
         */
        /* TODO: Consider AMPDU case */
        uu_wlan_clear_mpdu(uu_wlan_cap_ctxt_g.txop_owner_ac, UU_FALSE);
        uu_wlan_reset_cw_value();
    }
    else /* Do retransmission */
    {
        uu_wlan_increment_cw_value();
        /* TX module updates the retry bit in the packet */
        uu_wlan_tx_set_retry_bit(uu_wlan_cap_ctxt_g.txop_owner_ac);

        if (uu_wlan_ShortRetryCount[uu_wlan_cap_ctxt_g.txop_owner_ac] == UU_WLAN_DOT11_SHORT_RETRY_COUNT_R>>1)
        {
            uu_wlan_tx_update_with_fb_rate(uu_wlan_cap_ctxt_g.txop_owner_ac);
#ifdef UU_WLAN_TPC
            /* Keep the original TPL when the rate changes to fallback rate */
            frame_info->txvec.txpwr_level = orig_txpwr_level;
#endif

        }

#ifdef UU_WLAN_TPC
        else
        {
            frame_info->txvec.txpwr_level = uu_calc_tpl_retry_frame (frame_info->txvec.txpwr_level,
                    frame_info->txvec.format, frame_info->txvec.modulation,
                    frame_info->txvec.ch_bndwdth);
        }
#endif

    }
    return SUPP_SM_TX_FAILURE;
}

static uu_int32 uu_wlan_cap_ev_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_tx_frame_info_t *frame_info;

    if(uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_prog)
    {
        /* no need to decrement timer */
        return SUPP_SM_STATE_CONTINUE;
    }

    /* decrement tx_nav timer */
    /* TODO: Decrementing in cap slot timer so no need of decrementing here */
    if(uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav > 0)
    {
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav--;
    }

    /* Get the last-sent message, for usage on ACK/CTS expiry */
    if(uu_wlan_lmac_pspoll_frame_ready_g)
    {
        frame_info = ps_poll_frame_info_g;
    }
    else if(uu_wlan_lmac_trigger_frame_ready_g)
    {
        frame_info = ps_trigger_frame_info_g;
    }
    else
    {
        frame_info = uu_wlan_get_msg_ref_from_AC_Q(uu_wlan_cap_ctxt_g.txop_owner_ac);
    }

    if(frame_info == UU_NULL)
    {
        /* reset tx_ready flag */
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_ready = 0;
        return SUPP_SM_TX_END;
    }

    if(uu_wlan_cap_ctxt_g.ack_timer_val)
    {
        uu_wlan_cap_ctxt_g.ack_timer_val--;
        /* timer expiration condtion */
        if(!uu_wlan_cap_ctxt_g.ack_timer_val)
        {
            uu_wlan_cp_reset_waiting_for_ack();

            /* If the MPDU is short, handle short-retry */
            if(UU_WLAN_RTS_THRESHOLD_R > frame_info->frameInfo.framelen)
            {
                return _handle_data_short_retry(frame_info);
            } /* Short Retry */
            /* incrementing long retry count of mpdu */
            else
            {
                return _handle_data_long_retry(frame_info);
            }
        }
    }

    if (uu_wlan_cap_ctxt_g.cts_timer_val)
    {
        uu_wlan_cap_ctxt_g.cts_timer_val--;
        if(!uu_wlan_cap_ctxt_g.cts_timer_val)
        {
            uu_wlan_cp_reset_waiting_for_cts();
            return _handle_rts_retry(frame_info);
        }
    }
    return SUPP_SM_STATE_CONTINUE;
} /* uu_wlan_cap_ev_timer_tick */


static uu_int32 uu_wlan_cap_ev_tx_start_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    /* setting tx_prog flag */
    uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_prog = 1;

    /* send tx_data.req to phy */
    uu_wlan_tx_phy_data_req(uu_wlan_cap_ctxt_g.txop_owner_ac);
    return SUPP_SM_STATE_CONTINUE;
} /* uu_wlan_cap_ev_tx_start_confirm */

static uu_int32 txop_tx_data_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    /* send tx end req to phy */
    uu_wlan_phy_txend_req();
    return SUPP_SM_STATE_CONTINUE;
}

static uu_int32 txop_tx_ready_pkt(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;
    uu_wlan_cap_ctxt_g.ac_cntxt[ev_info->u.tx_ready.ac].tx_ready = 1;
    return SUPP_SM_STATE_CONTINUE;
}

static action_t cm_txop_init_action_g[] = {        //UU_NULL,/* UU_WLAN_CAP_EV_INVALID  */
    UU_NULL, /* UU_WLAN_CAP_EV_MAC_RESET */
    UU_NULL, /* UU_WLAN_CAP_EV_TIMER_TICK */
    txop_tx_ready_pkt, /* UU_WLAN_CAP_EV_TX_READY */
    uu_wlan_cap_ev_tx_start_confirm, /* UU_WLAN_CAP_EV_TX_START_CONFIRM */
    txop_tx_data_confirm, /*  UU_WLAN_CAP_EV_TX_DATA_CONFIRM */
    txop_tx_end_confirm, /* UU_WLAN_CAP_EV_TX_END_CONFIRM */
    UU_NULL, /* UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL */
    UU_NULL, /* UU_WLAN_CAP_EV_RX_START_IND */
    UU_NULL, /* UU_WLAN_CAP_EV_DATA_INDICATION */
    UU_NULL, /* UU_WLAN_CAP_EV_RX_END_IND */
    UU_NULL ,/* UU_WLAN_CAP_EV_PHY_RX_ERR */
    UU_NULL, /* UU_WLAN_CAP_EV_CCA_RESET_CONFIRM */
    uu_wlan_handle_txop_tx_start, /* UU_WLAN_CAP_EV_TXOP_TX_START */
};

static action_t cm_sending_data_action_g[] = {        //UU_NULL,/* UU_WLAN_CAP_EV_INVALID  */
    UU_NULL, /* UU_WLAN_CAP_EV_MAC_RESET */
    UU_NULL, /* UU_WLAN_CAP_EV_TIMER_TICK */
    txop_tx_ready_pkt, /* UU_WLAN_CAP_EV_TX_READY */
    uu_wlan_cap_ev_tx_start_confirm, /* UU_WLAN_CAP_EV_TX_START_CONFIRM */
    txop_tx_data_confirm, /*  UU_WLAN_CAP_EV_TX_DATA_CONFIRM */
    txop_tx_end_confirm, /* UU_WLAN_CAP_EV_TX_END_CONFIRM */
    UU_NULL, /* UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL */
    UU_NULL, /* UU_WLAN_CAP_EV_RX_START_IND */
    UU_NULL, /* UU_WLAN_CAP_EV_DATA_INDICATION */
    UU_NULL, /* UU_WLAN_CAP_EV_RX_END_IND */
    UU_NULL ,/* UU_WLAN_CAP_EV_PHY_RX_ERR */
    UU_NULL, /* UU_WLAN_CAP_EV_CCA_RESET_CONFIRM */
    UU_NULL, /* UU_WLAN_CAP_EV_TXOP_TX_START */
};

static action_t cm_wait_for_rx_action_g[] = {        //UU_NULL,/* UU_WLAN_CAP_EV_INVALID  */
    UU_NULL, /* UU_WLAN_CAP_EV_MAC_RESET */
    uu_wlan_cap_ev_timer_tick, /* UU_WLAN_CAP_EV_TIMER_TICK */
    txop_tx_ready_pkt, /* UU_WLAN_CAP_EV_TX_READY */
    wait_for_rx_tx_start_confirm, /* UU_WLAN_CAP_EV_TX_START_CONFIRM */
    wait_for_rx_tx_data_confirm, /*  UU_WLAN_CAP_EV_TX_DATA_CONFIRM */
    wait_for_rx_tx_end_confirm, /* UU_WLAN_CAP_EV_TX_END_CONFIRM */
    UU_NULL, /* UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL */
    wait_for_rx_rx_start_ind, /* UU_WLAN_CAP_EV_RX_START_IND */
    wait_for_rx_rx_data_ind, /* UU_WLAN_CAP_EV_DATA_INDICATION */
    wait_for_rx_rx_end_ind, /* UU_WLAN_CAP_EV_RX_END_IND */
    wait_for_rx_phy_error ,/* UU_WLAN_CAP_EV_PHY_RX_ERR */
    UU_NULL, /* UU_WLAN_CAP_EV_CCA_RESET_CONFIRM */
    UU_NULL, /* UU_WLAN_CAP_EV_TXOP_TX_START */
};

static action_t cm_wait_for_txnav_end_action_g[] = {        //UU_NULL,/* UU_WLAN_CAP_EV_INVALID  */
    UU_NULL, /* UU_WLAN_CAP_EV_MAC_RESET */
    wait_for_txnav_end_timer_tick, /* UU_WLAN_CAP_EV_TIMER_TICK */
    txop_tx_ready_pkt, /* UU_WLAN_CAP_EV_TX_READY */
    UU_NULL, /* UU_WLAN_CAP_EV_TX_START_CONFIRM */
    UU_NULL, /*  UU_WLAN_CAP_EV_TX_DATA_CONFIRM */
    UU_NULL, /* UU_WLAN_CAP_EV_TX_END_CONFIRM */
    UU_NULL, /* UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL */
    UU_NULL, /* UU_WLAN_CAP_EV_RX_START_IND */
    UU_NULL, /* UU_WLAN_CAP_EV_DATA_INDICATION */
    UU_NULL, /* UU_WLAN_CAP_EV_RX_END_IND */
    UU_NULL ,/* UU_WLAN_CAP_EV_PHY_RX_ERR */
    UU_NULL, /* UU_WLAN_CAP_EV_CCA_RESET_CONFIRM */
    UU_NULL, /* UU_WLAN_CAP_EV_TXOP_TX_START */
};


uu_int32 txop_init_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    /* TODO: Initialize with actually required duration, instead of maximum.
     * If tx has to be terminated before TXOP-Limit due to no data, need to
     *  send CF-END.
     * TODO: Multi-frame protection.
     */
    uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav = UU_WLAN_TXOP_LIMIT_R(uu_wlan_cap_ctxt_g.txop_owner_ac);
    return UU_SUCCESS;
}

uu_int32 txop_init_exit(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

uu_int32 txop_init_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = SUPP_SM_STATE_CONTINUE;

    if (cm_txop_init_action_g[event] != UU_NULL)
    {
        ret = (*cm_txop_init_action_g[event])(sm_cntx, event, ev_data);
    }
    else
    {
        //LOG Error
    }
    return ret;
}

uu_int32 sending_data_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    uu_int32 ret;

    /* sending next frame (already sent a control / data frame) */
    ret = uu_wlan_cap_send_more_data((uu_uchar*)&uu_wlan_cap_ctxt_g.txop_sm_cntxt, event, UU_NULL);
    if(ret == SUPP_SM_STATE_CONTINUE)
    {
        ret = UU_SUCCESS;
    }
    else if ((ret == SUPP_SM_TXOP_LIMIT_COMP) ||
            (ret == SUPP_SM_TX_END))
    {
        ret = UU_FAILURE;
    }
    
    return ret;
}

uu_int32 sending_data_exit(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

uu_int32 sending_data_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = SUPP_SM_STATE_CONTINUE;

    if (cm_sending_data_action_g[event] != UU_NULL)
    {
        ret = (*cm_sending_data_action_g[event])(sm_cntx, event, ev_data);
    }
    else
    {
        //LOG Error
    }
    return ret;
}

uu_int32 wait_for_rx_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

uu_int32 wait_for_rx_exit(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}


static uu_int32 wait_for_rx_tx_start_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    /* send tx_data.req to phy */
    uu_uchar phy_data_cfm = 0;
    uu_uint32 curr_pos = 0;
    do
    {
        uu_wlan_phy_data_req(tx_ctl_frame_info_g.mpdu[curr_pos], &phy_data_cfm);
        curr_pos++;
    } while ((phy_data_cfm!= 0) && (curr_pos < tx_ctl_frame_info_g.frameInfo.framelen));
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_DATA_CONFIRM, UU_NULL, 0);
    return SUPP_SM_STATE_CONTINUE;
}

static uu_int32 wait_for_rx_tx_data_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    /* send tx end req to phy */
    uu_wlan_phy_txend_req();
    return SUPP_SM_STATE_CONTINUE;
}

static uu_int32 wait_for_rx_tx_end_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    /* return value to main state sm for entering receive state */
    return SUPP_SM_RX_VALID_FRAME;
}

static uu_int32 wait_for_rx_rx_start_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = SUPP_SM_STATE_CONTINUE;
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;

    /* Resetting timers */
    //uu_wlan_cap_ctxt_g.ack_timer_val = 0;
    //uu_wlan_cap_ctxt_g.cts_timer_val = 0;

    /* Retain Rx vector details, for future reference */
    uu_wlan_cap_retain_rx_vec_info(&ev_info->u.rx_vec.rxvec);

    return ret;
}

static uu_int32 wait_for_rx_rx_data_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = SUPP_SM_STATE_CONTINUE;
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;
    uu_uint32 receive_frame_len;

    /* Retain Rx frame header details, for future reference */
    receive_frame_len = uu_wlan_cap_retain_rx_frame_hdr(ev_info->u.rx_data_ind.frame_p);

    /* free the memory of ev rx_data indication */
    uu_wlan_free_memory(ev_info->u.rx_data_ind.frame_p);

    if((uu_wlan_cap_ctxt_g.rx_frame_info.mpdu[0] == (IEEE80211_FC0_SUBTYPE_CTS | IEEE80211_FC0_TYPE_CTL)) &&
            !(isLocalMacAddr(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu + UU_RA_OFFSET)))
    {
        uu_wlan_cap_ctxt_g.nav_reset_timer = 0;
    }
    return ret;
}

static uu_int32 wait_for_rx_rx_end_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_rx_handler_return_value_t ret = uu_wlan_cap_rx_status_value_g;
    uu_uint16 duration;
    uu_wlan_tx_frame_info_t *frame_info;

    /* Get the last-sent message */
    if(uu_wlan_lmac_pspoll_frame_ready_g)
    {
        frame_info = ps_poll_frame_info_g;
    }
    else if(uu_wlan_lmac_trigger_frame_ready_g)
    {
        frame_info = ps_trigger_frame_info_g;
    }
    else
    {
        frame_info = uu_wlan_get_msg_ref_from_AC_Q(uu_wlan_cap_ctxt_g.txop_owner_ac);
    }

    if(frame_info == UU_NULL)
    {
        /* reset tx_ready flag */
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_ready = 0;
        return SUPP_SM_TX_END;
    }

    if(!((ret == UU_WLAN_RX_HANDLER_ACK_RCVD) || (ret == UU_WLAN_RX_HANDLER_CTS_RCVD)
                || (ret == UU_WLAN_RX_HANDLER_BA_RCVD)))
    {
        /* If the MPDU is short, handle short-retry */
        if(UU_WLAN_RTS_THRESHOLD_R > frame_info->frameInfo.framelen)
        {
            return _handle_data_short_retry(frame_info);
        } /* Short Retry */
        /* incrementing long retry count of mpdu */
        else
        {
            return _handle_data_long_retry(frame_info);
        }
    }

    /* starting nav reset timer */
    if((uu_wlan_cap_ctxt_g.rx_frame_info.mpdu[0] == (IEEE80211_FC0_SUBTYPE_RTS | IEEE80211_FC0_TYPE_CTL)) &&
            !(isLocalMacAddr(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu + UU_RA_OFFSET)))
    {
#if 0
        uu_wlan_cap_ctxt_g.nav_reset_timer = (UU_SLOT_TIMER_VALUE_R << 1) + UU_PHY_RX_START_DELAY_R + (UU_WLAN_SIFS_TIMER_VALUE_R << 1) +
            ((UU_CTS_FRAME_LEN)/uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate);
#else
        uu_wlan_cap_ctxt_g.nav_reset_timer = (UU_SLOT_TIMER_VALUE_R << 1) + UU_PHY_RX_START_DELAY_R + (UU_WLAN_SIFS_TIMER_VALUE_R << 1);
        if (uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate == UU_HW_RATE_6M)
        {
            /* cts frame length is 14, so 14/6 ceiling to 3 */
            uu_wlan_cap_ctxt_g.nav_reset_timer += 3;
        }
        else if (uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate <= UU_HW_RATE_12M)
        {
            /* cts frame length is 14, so 14/(9 or 12) ceiling to 2 */
            uu_wlan_cap_ctxt_g.nav_reset_timer += 2;
        }
        else
        {
            /* cts frame length is 14, so 14/(>12) ceiling to 2 */
            uu_wlan_cap_ctxt_g.nav_reset_timer += 1;
        }
#endif
    }
    if(ret == UU_WLAN_RX_HANDLER_FRAME_ERROR)
    {
        uu_wlan_cap_ctxt_g.eifs_timer_val = UU_WLAN_EIFS_VALUE_R;
        return SUPP_SM_TXOP_ERROR_RECOVERY;
    }
    else if(ret == UU_WLAN_RX_HANDLER_NAV_UPDATE)
    {
        /* Duration calculation for PS-poll frame.
         * In PS-Poll, if RA is not ours, we have to calculate duration & update our nav.
         * The duration field in the received PS-poll will contain AID, instead of duration.
         */
        if(IEEE80211_IS_FC0_PSPOLL(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu) &&
                uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate)
        {
            duration = UU_WLAN_GET_PSPOLL_FRAME_DURATION(uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate);
            uu_wlan_cap_ctxt_g.rx_frame_info.mpdu[2] = (uu_uchar)(duration & 0x00ff);
            uu_wlan_cap_ctxt_g.rx_frame_info.mpdu[3] = (uu_uchar)(duration >> 8);
        }

        if(UU_WLAN_GET_FRAME_DURATION(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu) > uu_wlan_cap_ctxt_g.nav_val)
        {
            uu_wlan_cap_ctxt_g.nav_val = UU_WLAN_GET_FRAME_DURATION(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu);
        }
        return SUPP_SM_TXOP_NAV_UPDATE;
    }
    else if(ret == UU_WLAN_RX_HANDLER_SEND_RESP)
    {
        uu_wlan_cap_retain_tx_frame_info(&tx_ctl_frame_info_g);
        /* send tx_start.req to phy */
        uu_wlan_phy_txstart_req((uu_uchar *)&tx_ctl_frame_info_g.txvec, sizeof(uu_wlan_tx_vector_t));
        return SUPP_SM_STATE_CONTINUE;
    }
    else if((ret == UU_WLAN_RX_HANDLER_ACK_RCVD) || (ret == UU_WLAN_RX_HANDLER_CTS_RCVD)
            || (ret == UU_WLAN_RX_HANDLER_BA_RCVD))
    {
        if(UU_WLAN_IEEE80211_QOS_MODE_R)
        {
            uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val = UU_WLAN_CW_MIN_VALUE_R(uu_wlan_cap_ctxt_g.txop_owner_ac);
        }
        else /* non-QoS */
        {
            uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].cw_val = UU_WLAN_CWMIN_VALUE_R;/* dcf cw min value */
        }

        /* reset station (short and long) retry counts to 0*/
        //uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].SSRC = 0;
        //uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].SLRC = 0;

        if (ret == UU_WLAN_RX_HANDLER_CTS_RCVD)
        {
            uu_wlan_cp_reset_waiting_for_cts();
            uu_wlan_cap_ctxt_g.cts_timer_val = 0;
        }
        else if (ret == UU_WLAN_RX_HANDLER_BA_RCVD)
        {
            uu_wlan_cp_reset_waiting_for_ack();
            uu_wlan_cap_ctxt_g.ack_timer_val = 0;
            /* Status already updated by CP, to Tx */
            uu_wlan_clear_ampdu(uu_wlan_cap_ctxt_g.txop_owner_ac);
        }
        /* TODO: Here, we don't know whether the frame is a retransmitted MPDU of an AMPDU (OR) just an MPDU. To be fixed along with 'uu_wlan_tx_update_len" (where we always sending retransmissions as AMPDU). If this is part of an AMPDU, the status update path is also different. Fix in next phase */
        /* freeing the frame */
        else /* UU_WLAN_RX_HANDLER_ACK_RCVD */
        {
            uu_wlan_cp_reset_waiting_for_ack();
            uu_wlan_cap_ctxt_g.ack_timer_val = 0;

            if (uu_ps_poll_ack_g || uu_ps_trigger_ack_g)
            {
                uu_ps_poll_ack_g = UU_FALSE;
                uu_ps_trigger_ack_g = UU_FALSE;
            }
            else
            {
                uu_wlan_clear_mpdu(uu_wlan_cap_ctxt_g.txop_owner_ac, UU_TRUE);
            }
        }

        //ret = uu_wlan_cap_send_more_data(sm_cntx, event, ev_data);
        ret = SUPP_SM_START_SENDING;
        return ret;
    }
    else if (ret == UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS)
    {
        uu_wlan_cp_reset_waiting_for_ack();
        uu_wlan_cap_ctxt_g.ack_timer_val = 0;

        /* If the A-MPDU is short, handle short-retry */
        if(UU_WLAN_RTS_THRESHOLD_R > frame_info->frameInfo.framelen)
        {
            _handle_data_short_retry(frame_info);
        } /* Short Retry */
        /* incrementing long retry count */
        else
        {
            _handle_data_long_retry(frame_info);
        }
        return  SUPP_SM_START_SENDING;
    }
    else
    {
        //uu_wlan_cap_ctxt_g.ack_timer_val =  0;
        //uu_wlan_cap_ctxt_g.cts_timer_val =  0;
        //return SUPP_SM_STATE_CONTINUE;
        return SUPP_SM_RX_VALID_FRAME;
    }
}

static uu_int32 wait_for_rx_phy_error(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret  = SUPP_SM_TXOP_ERROR_RECOVERY;

    uu_wlan_cap_ctxt_g.eifs_timer_val = UU_WLAN_EIFS_VALUE_R;

    return ret;
}

uu_int32 wait_for_rx_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = SUPP_SM_STATE_CONTINUE;

    if (cm_wait_for_rx_action_g[event] != UU_NULL)
    {
        ret = (*cm_wait_for_rx_action_g[event])(sm_cntx, event, ev_data);
    }
    else
    {
        //LOG Error
    }
    return ret;
}

uu_int32 wait_for_tx_nav_end_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

uu_int32 wait_for_tx_nav_end_exit(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

static uu_int32 wait_for_txnav_end_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = SUPP_SM_STATE_CONTINUE;

    /* TODO: Frame is coming in usec, while C model timer tick is in msec.  So, this has no effect. */
    /* No need of decrementing here as we are decrementing in cap slot timer */
    if (uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav > 0)
    {
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav--;
    }

    if(uu_wlan_cap_ctxt_g.eifs_timer_val > 0)
    {
        uu_wlan_cap_ctxt_g.eifs_timer_val--;
    }
    if(uu_wlan_cap_ctxt_g.nav_val > 0)
    {
        uu_wlan_cap_ctxt_g.nav_val--;
    }

    /* If tx_nav turned to 0 */
    if (!uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav)
    {
        ret = SUPP_SM_TX_NAV_ENDED;
    }
    return ret;
}

uu_int32 wait_for_tx_nav_end_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = SUPP_SM_STATE_CONTINUE;

    if (cm_wait_for_txnav_end_action_g[event] != UU_NULL)
    {
        ret = (*cm_wait_for_txnav_end_action_g[event])(sm_cntx, event, ev_data);
    }
    else
    {
        //LOG Error
    }
    return ret;
}


/* EOF */

