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
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_utils.h"
#include "uu_wlan_reg.h"
#include "crc32.h"
#include "uu_wlan_duration.h"
#include "uu_wlan_msgq.h"

#include "uu_wlan_cp_ba_sb.h"
#include "uu_wlan_cp_ba_session.h"
#include "uu_wlan_tx_handler.h"
#include "uu_wlan_tx_aggregation.h"


/* WARNING: For testing aggregation in C maintaing ampdu_pkt, 
   RTL coding no need of seperate buffer for ampdu packet, does the aggregation packet by packet submits same thing to PHY */
static uu_uint8  uu_wlan_ampdu_pkt_g[10*1500]; 
static uu_uint32 uu_wlan_ampdu_len_g;


static uu_uint32 uu_wlan_tx_agg_delimiter_prep(uu_uint16 len, uu_wlan_frame_format_t format)
{
    uu_uint32  delimiter = UU_WLAN_ZERO_LEN_DELIMITER;
    uu_uint8   crc;

    /* Inserting the MPDU length into the delimiter */
    if (format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        INSERT_BITS(delimiter,15,2,len);
    }
    else /* 11n aggregation, where MPDU length is 12 bits */
    {
        INSERT_BITS(delimiter,15,4,len);
    }

    /* Calculate the CRC of the first 16bits of the delimiter */
    crc = uu_wlan_get_crc_8bit(*(uu_uint16*)&delimiter); /* ???? need to take care about alignment issues */

    /* Inserting the CRC into the delimiter */
    INSERT_BITS(delimiter, 23, 16, crc);

    return delimiter;
}


uu_void uu_wlan_tx_fill_ampdu(uu_char* data, uu_uint32 len)
{
    UU_WLAN_COPY_BYTES(&uu_wlan_ampdu_pkt_g[uu_wlan_ampdu_len_g], data, len); 

    uu_wlan_ampdu_len_g += len;
}


uu_int32 uu_wlan_tx_aggregation(uu_wlan_tx_frame_info_t* frame_info, uu_uint16 duration, uu_uint8 min_insertion, uu_uint8 mpdu_num)
{
    uu_uint32  delimiter, zero = 0;
    uu_uint32  zero_delimiter = UU_WLAN_ZERO_LEN_DELIMITER;
    uu_uint8   i; 
    uu_wlan_ba_ses_context_t *ba;
    uu_uint16  scf;
    uu_uint8   tid;
    uu_uchar*  frame;
    uu_uint16  len = frame_info->ampdu_info.mpdu_info[mpdu_num].mpdu_len; 

    frame = uu_wlan_tx_get_next_mpdu_in_ampdu(frame_info->ac, mpdu_num);

    /* Calculate the delimiter for the MPDU */
    delimiter = uu_wlan_tx_agg_delimiter_prep(len, frame_info->txvec.format);
    uu_wlan_tx_fill_ampdu((uu_uchar*)&delimiter, sizeof(delimiter));

    /* Update the duration in each MPDU */
    *((uu_uint16 *)&frame[2]) = duration;

    /* Calculate the FCS of the packet */
    uu_wlan_fill_crc(0, frame, len - 4);

    uu_wlan_tx_fill_ampdu(frame, len);

    tid = uu_wlan_tx_get_qos_frame_tid(frame_info->ac);
    scf = *(uu_uint16*)(frame + UU_SCF_OFFSET);

    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(&frame[UU_RA_OFFSET], tid, 1);

    if(ba == NULL)
    {
        UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
        return UU_FAILURE;
    }

    uu_wlan_sb_update_tx(&ba->ses.sb, scf);
    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);

    /* All mpdu's in A-MPDU except last MPDU */
    if (frame_info->aggr_count != (mpdu_num+1))
    {
        /* Add padding */
        uu_wlan_tx_fill_ampdu((uu_uchar*)&zero, frame_info->ampdu_info.mpdu_info[mpdu_num].pad_len);

        /* Insert the min spacing delimiters at the end of the MPDU */
        for (i = 0; i < min_insertion; i++)
        {
            uu_wlan_tx_fill_ampdu((uu_uchar*)&zero_delimiter, sizeof(zero_delimiter));
        }
    }
    else if (frame_info->txvec.format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        /* VHT case, add padding for last frame */
        uu_wlan_tx_fill_ampdu((uu_uchar*)&zero, frame_info->ampdu_info.mpdu_info[mpdu_num].pad_len);
    }

    if (mpdu_num == 0)
    {
        uu_wlan_tx_ampdu_status_g[frame_info->ac].ssn = scf >> 4;
    }

    return UU_SUCCESS;
} /* uu_wlan_tx_aggregation */


uu_uint32 uu_wlan_tx_update_len (uu_wlan_tx_frame_info_t* frame_info)
{
    uu_uint8   i;
    uu_uint32  len = 0;
    uu_uint8   aggr_count;
    uu_bool    is_retrans;

    /* To Get the actual aggregation count */    
    if(uu_wlan_tx_ampdu_status_g[frame_info->ac].pending_mpdu)
    {
        aggr_count =  uu_wlan_tx_ampdu_status_g[frame_info->ac].pending_mpdu;
        is_retrans = UU_TRUE;
    }
    else
    {
        aggr_count = frame_info->aggr_count;
        is_retrans = UU_FALSE;
    }

    /* If No-Aggregation */
    if (!aggr_count)
    {
        len = frame_info->frameInfo.framelen;
    }
    /* Aggregation case. During retransmission, for non-VHT, we may need to send individual frame */
    else
    {
        /*
         ** Calculate total length of frames, to be aggregated.
         ** for reaggregation, consider only the frames to be retransmitted.
         */
        for (i = 0; (i < frame_info->aggr_count) && (aggr_count != 0); i++)
        {
            /* For retransmission case */
            if (is_retrans && !((uu_wlan_tx_ampdu_status_g[frame_info->ac].bitmap) & (1 << i)))
            {
                continue;
            }

#if 0 /* TODO: Not enabling for now, as this needs changing 'frame_info->framelen' and that has side-affects */
      /* TODO: Corresponding change is also required in 'uu_wlan_tx_phy_data_req' */
            /* If only 1 MPDU is remaining for retransmission in non-VHT mode */
            if ((len == 0) && (aggr_count == 1) &&
                (frame_info->txvec.format != UU_WLAN_FRAME_FORMAT_VHT))
            {
                len = frame_info->ampdu_info.mpdu_info[i].mpdu_len;
                /* TODO: Update 'framelen', as individual MPDU will be sent in this case */
                break;
            }
#endif

            /* Delimiter */
            len += UU_WLAN_MPDU_DELIMITER_LEN;
            /* mpdu length */
            len += frame_info->ampdu_info.mpdu_info[i].mpdu_len;

            aggr_count--;
            if (aggr_count) /* Not the last frame of AMPDU */
            {
                len += frame_info->ampdu_info.mpdu_info[i].pad_len;
                len += 8; /* Minimum spacing */
            }
            else /* Last mpdu in A-MPDU */
            {
                if (frame_info->txvec.format == UU_WLAN_FRAME_FORMAT_VHT)
                {
                    len += frame_info->ampdu_info.mpdu_info[i].pad_len;
                }
            }
        } /* for */
    }

    if (len != 0)
    {
        if(frame_info->txvec.format == UU_WLAN_FRAME_FORMAT_NON_HT)
        {
            frame_info->txvec.L_length = len;
        }
        else if(frame_info->txvec.format == UU_WLAN_FRAME_FORMAT_VHT)/* presently considering user id is 0*/
        {
            frame_info->txvec.tx_vector_user_params[0].apep_length = len;
        }
        else
        {
            frame_info->txvec.ht_length = len;
        }
    }
    return len;
} /* uu_wlan_tx_update_len */


uu_void uu_wlan_tx_do_aggregation(uu_wlan_tx_frame_info_t* frame_info)
{
    uu_uint16 duration;
    uu_uint8  i;
    uu_int32  ret;
    uu_int16  aggr_count = frame_info->aggr_count;
    uu_uint8   min_insertion = 2; /* TODO */
    uu_uchar phy_data_cfm = 0;
    uu_uint32 curr_pos = 0;

    /* Initialize the ampdu length with zero */
    uu_wlan_ampdu_len_g = 0;
    uu_wlan_tx_ampdu_status_g[frame_info->ac].bitmap = 0;

    /* Calculate the duration for the whole aggregation packet */
    duration = uu_calc_impl_BA_duration(&frame_info->txvec, UU_BA_COMPRESSED_FRAME_LEN);

    /* Aggregate additional frames, if any, of this AMPDU */
    for (i = 0; i < aggr_count; i++)
    {
        ret = uu_wlan_tx_aggregation(frame_info, duration, min_insertion, i);
        /* If can't aggregate (for ex, no BA context), delete all the frames to be aggregated */
        if (ret != UU_SUCCESS)
        {
            uu_wlan_clear_ampdu(frame_info->ac);
            return;
        }

        /* Setting the corresponding mpdu position in bitmap */
        uu_wlan_tx_ampdu_status_g[frame_info->ac].bitmap |= BIT(i);
    } /* for */

    uu_wlan_tx_ampdu_status_g[frame_info->ac].pending_mpdu = aggr_count;
    /* submit the ampdu packet to phy */
    do
    {
        uu_wlan_phy_data_req(uu_wlan_ampdu_pkt_g[curr_pos], &phy_data_cfm);
        curr_pos++;
    } while ((phy_data_cfm != 0) && (curr_pos < uu_wlan_ampdu_len_g));
    return;
}


uu_int32 uu_wlan_tx_reaggregation(uu_wlan_tx_frame_info_t* frame_info, uu_uint16 duration, uu_uint8 min_insertion, uu_char* frame, uu_uint16 len, uu_uchar last_mpdu)
{
    uu_uint32  delimiter, zero = 0;
    uu_uint32  zero_delimiter = UU_WLAN_ZERO_LEN_DELIMITER;
    uu_uint8   i;
    uu_wlan_ba_ses_context_t *ba;
    uu_uint16  scf;
    uu_uint8   tid;

    /* Calculate the delimiter for the MPDU */
    delimiter = uu_wlan_tx_agg_delimiter_prep(len, frame_info->txvec.format);
    uu_wlan_tx_fill_ampdu((uu_uchar*)&delimiter, sizeof(delimiter));

    /* Update the duration in each MPDU */
    *((uu_uint16 *)&frame[2]) = duration;

    //frame[1] |= IEEE80211_FC1_RETRY;

    /* Calculate the FCS of the packet */
    uu_wlan_fill_crc(0, frame, len - 4);

    uu_wlan_tx_fill_ampdu(frame, len);

    tid = uu_wlan_tx_get_qos_frame_tid(frame_info->ac);
    scf = *(uu_uint16*)(frame + UU_SCF_OFFSET);

    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(&frame[UU_RA_OFFSET], tid, 1);

    if(ba == NULL)
    {
        UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
        return UU_FAILURE;
    }

    uu_wlan_sb_update_tx(&ba->ses.sb, scf);
    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);

    /* All MPDUs in A-MPDU except last MPDU */
    if (!last_mpdu)
    {
        /* Add padding */
        uu_wlan_tx_fill_ampdu((uu_uchar*)&zero, UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(len));

        /* Insert the min spacing delimiters at the end of the MPDU */
        for (i = 0; i < min_insertion; i++)
        {
            uu_wlan_tx_fill_ampdu((uu_uchar*)&zero_delimiter, sizeof(zero_delimiter));
        }
    }
    else if (frame_info->txvec.format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        /* VHT case, add padding for last frame */
        uu_wlan_tx_fill_ampdu((uu_uchar*)&zero, UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(len));
    }

    return UU_SUCCESS;
} /* uu_wlan_tx_reaggregation */


uu_void uu_wlan_tx_do_reaggregation(uu_wlan_tx_frame_info_t* frame_info)
{
    uu_uint16 duration;
    uu_uint8  i, last = 0;
    uu_int32  ret;
    uu_uint8   min_insertion = 2; /* TODO */
    uu_uchar*  frame;
    uu_uint16  len;
    uu_uchar phy_data_cfm = 0;
    uu_uint32 curr_pos = 0;

    /* Initialize the ampdu length with zero */
    uu_wlan_ampdu_len_g = 0;

    /* Calculate the duration for the whole aggregation packet */
    duration = uu_calc_impl_BA_duration(&frame_info->txvec, UU_BA_COMPRESSED_FRAME_LEN);

    for (i = 0; i < frame_info->aggr_count; i++)
    {
        if (uu_wlan_tx_ampdu_status_g[frame_info->ac].bitmap & BIT(i))
        {
            last = i;
        }
    }

    for (i = 0; i < frame_info->aggr_count; i++)
    {
        if (uu_wlan_tx_ampdu_status_g[frame_info->ac].bitmap & BIT(i))
        {
            frame = uu_wlan_tx_get_next_mpdu_in_ampdu(frame_info->ac, i);
            len = frame_info->ampdu_info.mpdu_info[i].mpdu_len;

            ret = uu_wlan_tx_reaggregation(frame_info, duration, min_insertion, frame, len, (last == i));
            if (ret == UU_FAILURE)
            {
                uu_wlan_clear_ampdu(frame_info->ac);
                return;
            }
        }
    }

    /* submit the ampdu packet to phy */
    do
    {
        uu_wlan_phy_data_req(uu_wlan_ampdu_pkt_g[curr_pos], &phy_data_cfm);
        curr_pos++;
    } while ((phy_data_cfm != 0) && (curr_pos < uu_wlan_ampdu_len_g));

    return;
} /* uu_wlan_tx_do_reaggregation */


/* EOF */

