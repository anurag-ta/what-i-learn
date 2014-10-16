/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_rx_handler.c                                   **
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
#include "uu_wlan_utils.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_msgq.h"
#include "crc32.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_rx_handler.h"
#include "uu_wlan_ps.h"

static uu_int32 uu_wlan_fill_frame_bytes(uu_wlan_rx_frame_info_t *frame_info, uu_uchar data);
static uu_int32 uu_wlan_rx_frame_handler(uu_wlan_rx_frame_info_t *frame_info);

static uu_uint8  uu_wlan_delimiter_bytes_count_g;

/** This is used during deaggregation. It contains the remaning length of the A-MPDU.
 * Initialized when rx-start indication is received.
 * Reset when rx-end indication received.
 * Decremented & checked, on every byte of AMPDU from PLCP.
 **/
static uu_uint32 uu_wlan_rx_psdu_len_g; /* 20 bits in length */


/** After receiving the packet, RX module will use this flag to trigger UMAC or not.
 * Whenever a frame is filled in Rx buffer to send to UMAC, this flag is set.
 * On receiving Rx-End, this flag is checked. If set, Rx trigger (interrupt) is given to UMAC.
 * This is reset, while processing Rx-End.
 * The purpose of this is, to give only 1 interrupt to UMAC for an entire received AMPDU.
 */
static uu_bool uu_wlan_rx_need_to_inform_g; /* 1 bit in length */


#if 0 /* Unused */
static uu_void uu_wlan_update_rxstatus( uu_wlan_rx_status_t  *rx_status)
{
    UU_REG_RX_TSTAMP = rx_status->tstamp;
    UU_REG_RX_DATALEN = rx_status->datalen;
    UU_REG_RX_DATALEN = rx_status->datalen;
    UU_REG_RX_RSSI = rx_status->rssi;
    UU_REG_RX_KEYIX = rx_status->keyix;
    UU_REG_RX_RATE = rx_status->rate;
    UU_REG_RX_ANTENNA = rx_status->antenna;
    UU_REG_RX_RSSI_CT10 = rx_status->rssi;
    UU_REG_RX_RSSI_CT11 = rx_status->rssi_ctl1;
    UU_REG_RX_RSSI_CT12 = rx_status->rssi_ctl2;
    UU_REG_RX_RSSI_EXT0 = rx_status->rssi_ext0;
    UU_REG_RX_RX_RSSI_EXT1 = rx_status->rssi_ext1;
    UU_REG_RX_RX_RSSI_EXT2 = rx_status->rssi_ext2;
    UU_REG_RX_ISAGGR = rx_status->isaggr;
    UU_REG_RX_MOREAGGR = rx_status->moreaggr;
    UU_REG_RX_FLAGS = rx_status->flags;
    UU_REG_RX_PHYERR = rx_status->phyerr;
} /* uu_wlan_update_rxstatus */
#endif


/* Returns the intended destination of the frame, as a macro - Broadcast/Multicast/Us/Others/Loopback-of-ours */
static uu_wlan_rx_frame_dest_t uu_wlan_rx_identify_frame_dest(uu_uchar ra[IEEE80211_MAC_ADDR_LEN])
{
    uu_wlan_rx_frames_g++;

    /* Broadcast Frame */
    if (isBroadcastAddr(ra))
    {
        uu_wlan_rx_broadcast_cnt_g++;
        return UU_WLAN_RX_FRAME_DEST_BROADCAST;
    }

    /* Multicast Frame */
    if (isMulticastAddr(ra))
    {
        uu_wlan_rx_multicast_cnt_g ++;
        return UU_WLAN_RX_FRAME_DEST_MULITCAST;
    }

    /* Unicast Frame to us */
    if (isLocalMacAddr(ra))
    {
        //return UU_SUCCESS;
        uu_wlan_rx_frame_for_us_g++;
        return UU_WLAN_RX_FRAME_DEST_US;
    }

    /* received address is not to us */
    return UU_WLAN_RX_FRAME_DEST_OTHERS;
} /* uu_wlan_rx_identify_frame_dest */


static uu_bool uu_wlan_rx_frame_is_fcs_valid(uu_uchar *mpdu, uu_uint16 len)
{
    return uu_wlan_is_crc_valid(0, (const void*)mpdu, (len - UU_WLAN_MPDU_DELIMITER_LEN));
}/* uu_wlan_rx_frame_is_fcs_valid */


static uu_bool uu_wlan_rx_frame_is_header_valid(uu_uchar *mpdu)
{
    /* Check version number */
    if((UU_WLAN_GET_FC0_VALUE(mpdu) & IEE80211_FC0_VERSION_MASK) == IEEE80211_FC0_VERSION_0)
    {
        /* Check whether it is reserved frame type */
        if((UU_WLAN_GET_FC0_VALUE(mpdu) & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_MASK)
        {
            return UU_FALSE;
        }
        return UU_TRUE;
    }
    else
    {
        return UU_FALSE;
    }
} /* uu_wlan_rx_frame_is_header_valid */


static uu_bool uu_wlan_filter_is_umac_interested(uu_wlan_rx_frame_info_t *frame_info, uu_int32 ret)
{
    if(ret == UU_WLAN_RX_HANDLER_FRAME_ERROR)
    {
        return UU_FALSE;
    }

    if (UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_NO_FILTER)
    {
        return UU_TRUE;
    }

    if(UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_DROP_ALL)
    {
        return UU_FALSE;
    }

    /* received frame ra is multicast */
    if((ret == UU_WLAN_RX_FRAME_DEST_MULITCAST) && (UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_MULTI_CAST))
    {
        return UU_TRUE;
    }

    /* received frame ra is broadcast */
    if((ret == UU_WLAN_RX_FRAME_DEST_BROADCAST) && (UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_BROAD_CAST))
    {
        return UU_TRUE;
    }

    /* Filtering Control frames */
    if((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_CTL)
    {
        /* Filtering all control frames */
        if(UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_CTL_FRAMES)
        {
            return UU_TRUE;
        }

        /* Filtering PSPOLL frames */
        if (((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_PSPOLL)
                && (UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_PSPOLL))
        {
            return UU_TRUE;
        }

        /* Filtering RTS/CTS frames */
        if ((((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_RTS)
                    || ((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_CTS))
                && (UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_ALLOW_RTS_CTS))
        {
            return UU_TRUE;
        }
    }

    /* filetring management frames */
    if((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_TYPE_MASK)  == IEEE80211_FC0_TYPE_MGT)
    {
        /* probe request frame for filtering */
        if(((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_PROBE_REQ)
                &&(UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_PROBE_REQ))
        {
            return UU_TRUE;
        }

        /* beacon frame for filtering */
        else if(((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_BEACON)
                &&(UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_BEACON))
        {
            return UU_TRUE;
        }

        /* probe response frame for filtering */
        else if(((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_SUBTYPE_MASK) == IEEE80211_FC0_SUBTYPE_PROBE_RESP)
                &&(UU_REG_LMAC_FILTER_FLAG & UU_WLAN_FILTR_PROBE_RESP))
        {
            return UU_TRUE;
        }

        else
        {
            return UU_FALSE;
        }
    }

    UU_WLAN_LOG_DEBUG(("Rx filter Not passed frame %x, reg_lmac_filter_flag is %x\n", frame_info->mpdu[0], UU_REG_LMAC_FILTER_FLAG));
    return UU_FALSE;
} /* uu_wlan_filter_is_umac_interested */


/** Trigger the UMAC about received packet(s).
 * After receving the rx-end indication, RX module triggers UMAC.
 */
static uu_void uu_wlan_rx_trigger_umac(uu_void)
{
    if (uu_wlan_rx_need_to_inform_g)
    {
        lmac_ops_gp->umacCBK(UU_LMAC_IND_RX_FRAME);
        /* TODO: Use macro. A flag, ORed with existing value. Same for Tx also */
        uu_wlan_int_status_reg_g |= 0x02;
        uu_wlan_rx_need_to_inform_g = 0;
    }
} /* uu_wlan_rx_trigger_umac */


/* Fills the received packet status for UMAC
 *@param[in] frame_info contains the received packet information
 *@param[in] status contains received packet status
 */
static uu_void uu_wlan_fill_rx_status_to_UMAC(uu_wlan_rx_frame_info_t *frame_info, uu_int32 status)
{
    uu_bool  ret_l;

    /* filter functionality here */
    ret_l = uu_wlan_filter_is_umac_interested(frame_info, status);
    if (ret_l && (status != UU_WLAN_RX_FRAME_DEST_LOOPBACK))
    {
        uu_wlan_update_rx_status();

        /* Giving end of mpdu indication to buffer for wrap around codition
         * In 'C' model, write index moves to 4k boundary
         * NOTE: In RTL model, write index has to move to 4-byte boundary.
         */
        uu_wlan_rx_write_buffer(0, 0, 1);

        /* We have filled the Rx frame & status for UMAC. Trigger UMAC later */
        uu_wlan_rx_need_to_inform_g = 1;
    }
    else
    {
        /* For failed packets, reset the write index to current frame_info write index */
        uu_wlan_reset_curr_frame_wrindx();
    }
    return;
} /* uu_wlan_fill_rx_status_to_UMAC */


static uu_int32 uu_wlan_DeAgg_of_Rx_frame(uu_wlan_rx_frame_info_t *frame_info, uu_uchar data)
{
    static uu_uint8  delimiter[UU_WLAN_MPDU_DELIMITER_LEN];
    static uu_uint16 mpdu_len_from_del;

    if (uu_wlan_delimiter_bytes_count_g < UU_WLAN_MPDU_DELIMITER_LEN)
    {
        delimiter[uu_wlan_delimiter_bytes_count_g++] = data;

        /* The 4-byte delimiter is always aligned to 4-byte boundary in the AMPDU */
        if (uu_wlan_delimiter_bytes_count_g == UU_WLAN_MPDU_DELIMITER_LEN)
        {
            /* Compare the delimiter signature 0x4E */
            if (delimiter[3] == 0x4E)
            {
                /* Extracting Length field (bytes 0 & 1) from Delimeter */
                mpdu_len_from_del = delimiter[0] | (delimiter[1] << 8);

                /* Delimiter CRC verification. On error, drop all 4 bytes and look for delimiter again */
                if(uu_wlan_is_8bitCrc_valid(mpdu_len_from_del, delimiter[2]) == UU_FALSE)
                {
                    uu_wlan_delimiter_bytes_count_g = 0;
                    uu_wlan_rx_psdu_len_g  -= UU_WLAN_MPDU_DELIMITER_LEN;
                    return UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME;
                }

                /* Identify the actual length: 14bits for 11ac, 12bits for 11n */
                if (frame_info->rxvec.format == UU_WLAN_FRAME_FORMAT_VHT)
                {
                    mpdu_len_from_del = mpdu_len_from_del >> 2;
                }
                else /* 11n */
                {
                    mpdu_len_from_del = mpdu_len_from_del >> 4;
                }

                frame_info->frameInfo.framelen += UU_WLAN_MPDU_DELIMITER_LEN;
                uu_wlan_rx_psdu_len_g -=UU_WLAN_MPDU_DELIMITER_LEN;

                /* MPDU minimum spacing case(Delimiter length should be 0) */
                if(!mpdu_len_from_del)
                {
                    frame_info->frameInfo.framelen = 0;

                    uu_wlan_delimiter_bytes_count_g = 0;

                    return UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME;
                }
            }
            else
            {
                uu_wlan_delimiter_bytes_count_g = 0;
                return UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME;
            }
        }
    }
    else
    {
        uu_uint32 pad_len;
        if((frame_info->frameInfo.framelen - UU_WLAN_MPDU_DELIMITER_LEN) < mpdu_len_from_del)
        {
            /* Writing 1 byte data into circular buffer */
            uu_wlan_rx_write_buffer(&data, 1, 0);
        }
        uu_wlan_rx_psdu_len_g --;
        frame_info->frameInfo.framelen++;

        if (frame_info->rxvec.format == UU_WLAN_FRAME_FORMAT_VHT)
        {
            pad_len = UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(mpdu_len_from_del);
        }
        else
        {
            pad_len = (uu_wlan_rx_psdu_len_g) ? (UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(mpdu_len_from_del)):0;
        }

        /* Check for AMPDU subframe completion */
        if((frame_info->frameInfo.framelen - UU_WLAN_MPDU_DELIMITER_LEN - pad_len) == mpdu_len_from_del)
        {
            //uu_wlan_rx_psdu_len_g -= frame_info->frameInfo.framelen;
            frame_info->frameInfo.framelen -= (UU_WLAN_MPDU_DELIMITER_LEN + pad_len);

            uu_wlan_delimiter_bytes_count_g = 0;
            return UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME;
        }
    }
    return UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME;
} /* uu_wlan_DeAgg_of_Rx_frame */


/*
** Accumlates byte by byte from PHY.
** On receiving the 24th byte for Beacon/Probe-Response, TSF is filled in Rx-Frame-Info.
*/
static uu_int32 uu_wlan_fill_frame_bytes(uu_wlan_rx_frame_info_t *frame_info, uu_uchar data)
{
#ifdef UU_WLAN_TSF
    /* Only for the first byte received, we check if it is beacon frame,
     * then we store the present timestamp */
    if (!(frame_info->frameInfo.framelen))
    {
        uu_uchar first_octet[1];
        first_octet[0] = data;

        /* Update the received timestamp with the timestamp when
           the first octet was received in the MAC-PHY i/f.
           This will be updated as the mactime in the rx_status. */
        frame_info->timestamp_msb = (uu_uint32) ((uu_wlan_tsf_r >> 32) & 0xffffffff);
        frame_info->timestamp_lsb = (uu_uint32) (uu_wlan_tsf_r & 0xffffffff);

        // TODO: Rename the flag 'frameInfo.beacon'. Also rename 'becn_timestamp_msb_tsf'
        // Set the flag, to indicate that TSF is set by LMAC for this Rx frame.
        // Applicable for Beacon & Probe-Response frames.
        if ((IEEE80211_IS_BEACON_FRAME(first_octet)) || (IEEE80211_IS_PRBRESP_FRAME(first_octet)))
        {
            frame_info->frameInfo.beacon = 1;
        }
        else /* For all other frames */
        {
            frame_info->frameInfo.beacon = 0;
        }
    }

    /* TSF is filled on getting 24-byte, if that flag is set in frameInfo */
    // TODO: Rename 'IEEE80211_IS_BEACON_TIMESTAMP'. Our own macros must not start with IEEE80211 */
    if ((frame_info->frameInfo.beacon) && (frame_info->frameInfo.framelen == IEEE80211_IS_BEACON_TIMESTAMP))
    {
        frame_info->becn_timestamp_msb_tsf = (uu_uint32) ((uu_wlan_tsf_r >> 32) & 0xffffffff);
        frame_info->becn_timestamp_lsb_tsf = (uu_uint32) (uu_wlan_tsf_r & 0xffffffff);
    }
#endif /* UU_WLAN_TSF */

    /* Deaggregate, if the aggregated packet is received */
    if(frame_info->rxvec.is_aggregated)
    {
        return uu_wlan_DeAgg_of_Rx_frame(frame_info, data);
    }
    else /* Non-Aggregated frame */
    {
        if(frame_info->frameInfo.framelen != UU_WLAN_GET_FRAME_LEN_FROM_RX_VEC(frame_info->rxvec))
        {
            /* Writing 1 byte data into buffer */
            uu_wlan_rx_write_buffer(&data, 1, 0);
            frame_info->frameInfo.framelen++;
        }
        return UU_LMAC_RX_RET_NON_AGG_FRAME;
    }
} /* uu_wlan_fill_frame_bytes */


uu_int32 uu_wlan_rx_handle_phy_rxstart(uu_wlan_rx_vector_t  *rx_vec)
{
    uu_wlan_rx_frame_info_t rx_frame_info;
    uu_wlan_rx_frame_info_t *frame_info = uu_wlan_rx_get_frame_info();

    UU_WLAN_MEM_SET(&rx_frame_info, sizeof(uu_wlan_rx_frame_info_t));

    /* Clearing rx_vec into the Rx-buffer */
    uu_wlan_rx_write_buffer((uu_uchar *)rx_vec, sizeof(uu_wlan_rx_vector_t), 0);
    /* Clearing frame_details into Rx-buffer */
    uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.frameInfo, sizeof(uu_frame_details_t), 0);
#ifdef UU_WLAN_TSF
    uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.timestamp_msb, sizeof(rx_frame_info.timestamp_msb), 0);
    uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.timestamp_lsb, sizeof(rx_frame_info.timestamp_lsb), 0);
    uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.becn_timestamp_msb_tsf, sizeof(rx_frame_info.becn_timestamp_msb_tsf), 0);
    uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.becn_timestamp_lsb_tsf, sizeof(rx_frame_info.becn_timestamp_lsb_tsf), 0);
#endif
    if (frame_info->rxvec.is_aggregated)
    {
        uu_wlan_delimiter_bytes_count_g = 0;
        uu_wlan_rx_ampdu_frames_g ++;
        /* Initializing psdu length */
        uu_wlan_rx_psdu_len_g = frame_info->rxvec.psdu_length;
    }
    return UU_SUCCESS;
}


uu_int32 uu_wlan_rx_handle_phy_rxend(uu_uchar  rxend_stat)
{
    uu_wlan_rx_frame_info_t *frame_info = uu_wlan_rx_get_frame_info();
    uu_uchar *mpdu;
    uu_int32  ret;

    if ((rxend_stat == UU_WLAN_RX_END_STAT_NO_ERROR) &&
        	(frame_info->frameInfo.framelen >= UU_MIN_80211_FRAME_LEN))
    {
        mpdu = frame_info->mpdu;

        /* frame_info filling here */
        frame_info->frameInfo.is_ampdu = frame_info->rxvec.is_aggregated;

        ret = uu_wlan_rx_frame_handler(frame_info);

        if ((ret == UU_WLAN_RX_HANDLER_FRAME_ERROR) && frame_info->rxvec.is_aggregated)
        {
            /* Special case: Rx-End with error. Need to send BA, if required */
            UU_WLAN_LOG_DEBUG(("Rx: Invoking special handling of Rx-End(error) AMPDU case\n"));
            frame_info->mpdu[0] = IEEE80211_FC0_TYPE_RESERVED;
            if((ret = uu_wlan_cp_process_rx_frame(frame_info, uu_wlan_rx_psdu_len_g?0:1)) < 0)
            {
                UU_WLAN_LOG_ERROR(("Rx: Invoking special handling of Rx-End(error) of AMPDU failed. E:%x \n", ret));
            }
        }

        uu_wlan_rx_psdu_len_g = 0;

        /* Receive packet status(return value) updation to cap */
        uu_wlan_cap_set_recv_pkt_status((uu_wlan_rx_handler_return_value_t)ret);

        /* Post rx_end_ind event to cap q */
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_RX_END_IND, &rxend_stat, 1);

        /* Trigger UMAC about received mpdu/ampdu, if umac intrested */
        uu_wlan_rx_trigger_umac();
        return ret;
    }
    else /* RX-END with error */
    {
        uu_int32  ret = UU_SUCCESS;
        uu_wlan_rx_phy_err_pkts_g++;

        if (frame_info->rxvec.is_aggregated)
        {
            /* Special case: Rx-End with error. Need to send BA, if required */
            UU_WLAN_LOG_DEBUG(("Rx: Invoking special handling of Rx-End(error) AMPDU case\n"));
            frame_info->mpdu[0] = IEEE80211_FC0_TYPE_RESERVED;
            if((ret = uu_wlan_cp_process_rx_frame(frame_info, uu_wlan_rx_psdu_len_g?0:1)) < 0)
            {
                UU_WLAN_LOG_ERROR(("Rx: Invoking special handling of Rx-End(error) of AMPDU failed. E:%x \n", ret));
            }
        }

        /* clear the current mpdu */
        uu_wlan_reset_curr_frame_wrindx();

        uu_wlan_rx_psdu_len_g = 0;

        /* Rx status updation to CAP */
        uu_wlan_cap_set_recv_pkt_status((uu_wlan_rx_handler_return_value_t)ret);

        /* Post phy rx error event to cap q */
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_PHY_RX_ERR, &rxend_stat, 1);

        /* Trigger UMAC about received mpdu/ampdu, if umac intrested */
        uu_wlan_rx_trigger_umac();

        return ret;
    }
} /* uu_wlan_rx_handle_phy_rxend */


/* NOTE: In RTL implementation, frame validation & CRC calculation will be done while receiving the bytes from PHY. */
uu_int32 uu_wlan_rx_handle_phy_dataind(uu_uchar  data)
{
    uu_wlan_rx_frame_info_t *frame_info = uu_wlan_rx_get_frame_info();
    uu_int32 ret = UU_SUCCESS;

    ret = uu_wlan_fill_frame_bytes(frame_info, data);

    /*
    ** Filling of AMPDU subframe is completed.
    * If PSDU length is non-zero, prepare for next sub-frame.
    */
    if((ret == UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME) && uu_wlan_rx_psdu_len_g)
    {
        uu_wlan_rx_frame_info_t  rx_frame_info;

        UU_WLAN_MEM_SET(&rx_frame_info, sizeof(uu_wlan_rx_frame_info_t));
        uu_wlan_rx_ampdu_subframes_g++;

        uu_wlan_rx_frame_handler(frame_info);
		/* Clearing rx vector into buffer */
        uu_wlan_rx_write_buffer((uu_uchar *)&frame_info->rxvec, sizeof(uu_wlan_rx_vector_t), 0);
        /* Clearing frame_details into Rx-buffer */
        uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.frameInfo, sizeof(uu_frame_details_t), 0);
#ifdef UU_WLAN_TSF
        uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.timestamp_msb, sizeof(rx_frame_info.timestamp_msb), 0);
        uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.timestamp_lsb, sizeof(rx_frame_info.timestamp_lsb), 0);
        uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.becn_timestamp_msb_tsf, sizeof(rx_frame_info.becn_timestamp_msb_tsf), 0);
        uu_wlan_rx_write_buffer((uu_uchar *)&rx_frame_info.becn_timestamp_lsb_tsf, sizeof(rx_frame_info.becn_timestamp_lsb_tsf), 0);
#endif
    }

    return UU_SUCCESS;
} /* uu_wlan_rx_handle_phy_dataind */

#ifdef UU_WLAN_TSF
/* Returns the timestamp value present in the beacon frame */
static uu_uint64 uu_get_rx_beacon_timestamp (uu_uchar *mpdu)
{
    uu_uint64 timestamp = 0;
    uu_uint8 i;
    for (i = IEEE80211_IS_BEACON_TIMESTAMP; i < IEEE80211_IS_BEACON_TIMESTAMP + 8; i++)
    {
        timestamp = (timestamp << 8) | mpdu[i];
    }

    return timestamp;
}
#endif /* UU_WLAN_TSF */


static uu_int32 uu_wlan_rx_frame_handler(uu_wlan_rx_frame_info_t *frame_info)
{
    uu_int32 ret = UU_WLAN_RX_HANDLER_FRAME_ERROR, status_to_umac;
    uu_uchar frame_direction;
#ifdef UU_WLAN_TSF
    uu_uint64 rx_beacon_timestamp = 0;
    uu_uint64 sta_rx_beacon_tsf = 0;
#endif

    /* crc validation */
    if (!uu_wlan_rx_frame_is_fcs_valid(frame_info->mpdu, frame_info->frameInfo.framelen))
    {
        /* rx reg set to fcs crc failed */
        UU_WLAN_LOG_INFO(("Rx: Not processing the frame. Reason: FCS failed\n"));
        dot11_fcs_error_count_g ++;
        ret = UU_WLAN_RX_HANDLER_FRAME_ERROR;
        status_to_umac = UU_WLAN_RX_HANDLER_FRAME_ERROR;
        goto BOTTOM;
    }

    /* frame_header validation(version,type validation) */
    if (!uu_wlan_rx_frame_is_header_valid(frame_info->mpdu))
    {
        UU_WLAN_LOG_INFO(("Rx: Not processing the frame. Reason: frame header invalid\n"));
        ret = UU_WLAN_RX_HANDLER_FRAME_ERROR;
        status_to_umac = UU_WLAN_RX_HANDLER_FRAME_ERROR;
        goto BOTTOM;
    }

    /* Destination address verification - RA */
    status_to_umac = uu_wlan_rx_identify_frame_dest(frame_info->mpdu+UU_RA_OFFSET);

    /* power save related functionality */
    uu_wlan_ps_handle_rx_frame(frame_info->mpdu, status_to_umac);

#ifdef UU_WLAN_TSF
    if (frame_info->frameInfo.beacon && isBSSIDMacAddr(frame_info->mpdu + UU_BSSID_OFFSET))
    {
        /** Checking and Updating the STA's TSF after receiving the beacon frame in IBSS mode.
         * If Beacon Timestamp > Local TSF (when the Beacon Timestamp first octet was received in MAC-PHY i/f)
         *                                 – UU_WLAN_10NS_TO_US(RX_START_OF_FRAME_OFFSET (RXVECTOR)),
         * then Local TSF = Beacon Timestamp + (present timestamp - when the first byte of timestamp is received)
         *                                 + UU_WLAN_10NS_TO_US(RX_START_OF_FRAME_OFFSET (RXVECTOR))
         * As per the implementation,
         * Suppose, a = received beacon timestamp, b = PHY_RX_DELAY,
         * c = Local TSF when the first octet of the beacon timestamp was received
         * d = Local TSF when this TSF comparison is being done, i.e. after some random MAC delay.
         *
         * Thus MAC delay = (d-c)
		 * For BSS, 
		 * STA will update their TSF as per received.
		 *
		 * For IBSS,
         * While comparing, if (d < a + b + (d-c)), then d = a + b + (d-c).
         * So, we can rewrite the if condition as, ((d - (d-c)) < (a + b))
         *                                     OR, (c < (a + b))
         *                                     OR, ((a + b) > c)
         * where rx_beacon_timestamp = a + b;
         * Then if the condition is met, then d = rx_beacon_timestamp + MAC delay.
         */

        rx_beacon_timestamp = uu_get_rx_beacon_timestamp (frame_info->mpdu);
        rx_beacon_timestamp += (uu_uint32) UU_WLAN_10NS_TO_US(frame_info->rxvec.rx_start_of_frame_offset);
        sta_rx_beacon_tsf = frame_info->becn_timestamp_msb_tsf;
        sta_rx_beacon_tsf = (sta_rx_beacon_tsf << 32) | (frame_info->becn_timestamp_lsb_tsf);

#ifdef UU_WLAN_IBSS
    	/* In IBSS mode, if we received Beacon frame, then stop sending Beacon frame till next TBTT */
        if (UU_WLAN_IEEE80211_OP_MODE_R == UU_WLAN_MODE_ADHOC)
        {
            /* Check if the capability element has IBSS subfield as 1 */
            /* Also need to check that SSID present in Beacon/Probe_response frame matches with the SSID of the STA */
            if (IEEE80211_IS_BEACON_CAPABILITY_IBSS(frame_info->mpdu))
            {
                uu_wlan_cap_stop_beacon_tx ();
                if (rx_beacon_timestamp > sta_rx_beacon_tsf)
                {
                    uu_wlan_tsf_r = rx_beacon_timestamp + (uu_wlan_tsf_r - sta_rx_beacon_tsf);
                }
            }
        }
		else 
#endif /* UU_WLAN_IBSS */
		if (UU_WLAN_IEEE80211_OP_MODE_R == UU_WLAN_MODE_MANAGED) /* For STA mode only */
        {
            if (rx_beacon_timestamp != sta_rx_beacon_tsf)
            {
                uu_wlan_tsf_r = rx_beacon_timestamp + (uu_wlan_tsf_r - sta_rx_beacon_tsf);
            }
        }
    }
#endif /* UU_WLAN_TSF */

    if (status_to_umac != UU_WLAN_RX_FRAME_DEST_US)
    {
        frame_direction = UU_WLAN_GET_FC1_VALUE(frame_info->mpdu) & IEEE80211_FC1_DIR_MASK;
        if (((status_to_umac == UU_WLAN_RX_FRAME_DEST_BROADCAST)
                || (status_to_umac == UU_WLAN_RX_FRAME_DEST_MULITCAST))
                && (frame_direction == IEEE80211_FC1_DIR_TODS))
        {
            /* check the received frame BSSID with STA mac address */
            if (!((isLocalMacAddr(frame_info->mpdu + UU_BSSID_OFFSET))
                        && ((UU_WLAN_GET_FC0_VALUE(frame_info->mpdu) & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_DATA)))
            {
                ret = UU_WLAN_RX_HANDLER_NAV_UPDATE;
                goto BOTTOM;
            }
        }
        else if ((status_to_umac == UU_WLAN_RX_FRAME_DEST_OTHERS))
        {
            ret = UU_WLAN_RX_HANDLER_NAV_UPDATE;
            goto BOTTOM;
        }
        else
        {
            goto BOTTOM;
        }
    }

    if((UU_WLAN_GET_FC1_VALUE(frame_info->mpdu) & IEEE80211_FC1_PROTECTED) == (IEEE80211_FC1_PROTECTED))
    {
        uu_wlan_sec_frame_decrypt(frame_info);
    }

    if((ret = uu_wlan_cp_process_rx_frame(frame_info, uu_wlan_rx_psdu_len_g?0:1)) < 0)
    {
        UU_WLAN_LOG_ERROR(("Rx: Not processing the frame. Reason: CP handling failed. E:%x \n", ret));
        goto BOTTOM;
    }
BOTTOM:
    /* filling rx status to registers */
    //uu_wlan_update_rxstatus( rx_status);
    /* post to UMAC */
    uu_wlan_fill_rx_status_to_UMAC (frame_info, status_to_umac);

    return ret;
} /* uu_wlan_rx_frame_handler */


/* EOF */

