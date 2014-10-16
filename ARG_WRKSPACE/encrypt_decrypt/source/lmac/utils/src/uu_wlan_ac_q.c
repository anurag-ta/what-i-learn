/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_ac_q.c                                         **
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
#include "uu_wlan_limits.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_tx_handler.h"


#define UU_BUF_SIZE_16K_MASK   0X00003FFF
#define UU_BUF_SIZE_16K        0X00004000

#define UU_BUF_C_MODEL_MIN_SIZE       0X00001000
#define UU_BUF_C_MODEL_MIN_SIZE_MASK  0X00000FFF

/* Tx Qs, on per-AC basis. The Qs contain buffer descriptors */
static uu_uchar uu_ac_q_g[UU_WLAN_MAX_QID][UU_BUF_SIZE_16K];
/* Contains last-read index. Need to do +1, before referencing message to read */
static uu_int32 uu_ac_q_rd_indx_g[UU_WLAN_MAX_QID] = {0};
/* Contains index, at which next message has to be written. Need to do +1, on write */
static uu_int32 uu_ac_q_wr_indx_g[UU_WLAN_MAX_QID] = {0};

/*
 * Gives the Buffer length available in ac circuller buffer
 */
uu_uint32 uu_wlan_tx_get_available_buffer(uu_uchar ac)
{
    return (UU_BUF_SIZE_16K - (uu_ac_q_wr_indx_g[ac] -uu_ac_q_rd_indx_g[ac]));
}
EXPORT_SYMBOL(uu_wlan_tx_get_available_buffer);

/*
 * Informs about the packet ready in the ac circuller buffer
 * After copiying the packet from UMAC to LMAC, DMA core will gives this trigger
 */
uu_void uu_wlan_tx_dma_done(uu_uchar ac)
{
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_READY, &ac, 1);
}

/*
 * Writes the buffer into the LMAC circular buffer
 */
uu_void uu_wlan_tx_write_buffer(uu_uchar ac, uu_uchar *adr[], uu_uint32 len[])
{
    uu_uint32 ii;

    /* making both the indices to 0 as the buffer is empty */
    if (uu_ac_q_rd_indx_g[ac] == uu_ac_q_wr_indx_g[ac])
    {
        uu_ac_q_rd_indx_g[ac] = 0;
        uu_ac_q_wr_indx_g[ac] = 0;
    }

    /* writing into the circular buffer of particular AC */
    /* ASSUMPTION: Whoever is calling this, has already checked the avialable buffer size */
    for (ii = 0; (adr[ii] != UU_NULL) && (len[ii] > 0) && (ii != UU_WLAN_MAX_MPDU_IN_AMPDU); ii++)
    {
        UU_WLAN_COPY_BYTES(&uu_ac_q_g[ac][uu_ac_q_wr_indx_g[ac] & UU_BUF_SIZE_16K_MASK], adr[ii], len[ii]);
        uu_ac_q_wr_indx_g[ac] = (uu_ac_q_wr_indx_g[ac] + len[ii]);
    } /* while */

#if 1 /* C model only. Avoid problems with wrap-around */
    if (uu_ac_q_wr_indx_g[ac] & UU_BUF_C_MODEL_MIN_SIZE_MASK)
    {
        uu_ac_q_wr_indx_g[ac] += UU_BUF_C_MODEL_MIN_SIZE - (uu_ac_q_wr_indx_g[ac] & UU_BUF_C_MODEL_MIN_SIZE_MASK);
    }
#endif

    uu_wlan_tx_dma_done(ac);
} /* uu_wlan_tx_write_buffer */
EXPORT_SYMBOL(uu_wlan_tx_write_buffer);

/*
 * Frees the ac circular buffer with packet length
 */
uu_void uu_wlan_tx_buffer_free(uu_uchar ac, uu_uint32 len)
{
    if (uu_wlan_tx_get_available_buffer(ac) == UU_BUF_SIZE_16K )
    {
        return;
    }

    uu_ac_q_rd_indx_g[ac] = (uu_ac_q_rd_indx_g[ac] + len);

#if 1 /* C model only. Avoid problems with wrap-around */
    uu_ac_q_rd_indx_g[ac] += UU_BUF_C_MODEL_MIN_SIZE - (uu_ac_q_rd_indx_g[ac] & UU_BUF_C_MODEL_MIN_SIZE_MASK);
#endif
}


/**
 * Called to clear the MPDU at the top of the Q, on
 *   either successful transmission or frame dropping.
 */
/* TODO: Avoid calling it from CP / CAP. It should only be done in Tx, based on status signals from CP & CAP. To be done in next phase */
uu_void uu_wlan_clear_mpdu(uu_uint8 ac, uu_bool status)
{
    uu_wlan_tx_frame_info_t *frame_info;
    uu_uint8 retries;
    uu_uint16 seqno;

    frame_info = uu_wlan_tx_get_frame_info(ac);

    retries = (frame_info->frameInfo.framelen > UU_WLAN_RTS_THRESHOLD_R) ? uu_wlan_LongRetryCount[ac]:uu_wlan_ShortRetryCount[ac];
    seqno = uu_wlan_tx_get_frame_scf(ac);
    uu_wlan_tx_buffer_free(ac, sizeof(frame_info) + frame_info->frameInfo.framelen);
    /*Updating tx status for UMAC */

#ifdef UU_WLAN_TPC
    uu_wlan_tx_update_status_for_umac(ac, seqno, retries, status, frame_info->txvec.ch_bndwdth);
#else
    uu_wlan_tx_update_status_for_umac(ac, seqno, retries, status);
#endif

    uu_wlan_ShortRetryCount[ac] = 0;
    uu_wlan_LongRetryCount[ac] = 0;

} /* uu_wlan_clear_mpdu */

/* TODO: Avoid calling it from CP / CAP. It should only be done in Tx, based on status signals from CP & CAP. To be done in next phase */
uu_void uu_wlan_clear_ampdu(uu_uint8 ac)
{
    uu_uint8 i;
    uu_wlan_tx_frame_info_t *frame_info;
    uu_bool  status;
    uu_uint8 retries;
    uu_uint8 bitmap = uu_wlan_tx_ampdu_status_g[ac].bitmap;
    uu_uint16 seqno;
    uu_uint8 aggr_count;
    uu_uint8* frame;

    frame_info = uu_wlan_tx_get_frame_info(ac);
    if (frame_info == UU_NULL)
    {
        return;
    }

    retries = (frame_info->frameInfo.framelen > UU_WLAN_RTS_THRESHOLD_R) ? uu_wlan_LongRetryCount[ac]:uu_wlan_ShortRetryCount[ac];
    aggr_count = frame_info->aggr_count;

    for (i = 0; i < aggr_count; i++)
    {
        frame =  uu_wlan_tx_get_next_mpdu_in_ampdu(ac, i);
        seqno = *((uu_uint16*)&frame[UU_SCF_OFFSET]);
        status = (bitmap & (1 << i)) ? 0:1;

#ifdef UU_WLAN_TPC
        uu_wlan_tx_update_status_for_umac(ac, seqno, retries, status, frame_info->txvec.ch_bndwdth);
#else
        uu_wlan_tx_update_status_for_umac(ac, seqno, retries, status);
#endif

    }
    /* Freeing the buffer */
    uu_wlan_tx_buffer_free(ac, sizeof(frame_info) + frame_info->frameInfo.framelen);

    uu_wlan_tx_ampdu_status_g[ac].bitmap = 0;
    uu_wlan_tx_ampdu_status_g[ac].pending_mpdu = 0;
    uu_wlan_ShortRetryCount[ac] = 0;
    uu_wlan_LongRetryCount[ac] = 0;
}

uu_void uu_wlan_discard_msdu(uu_uint8 ac, uu_uchar *ra, uu_uint16 scf)
{
    uu_wlan_clear_mpdu(ac, UU_FALSE);
}


/*
 * Gets the reference of the frame info at the top of the AC Q
 */
uu_wlan_tx_frame_info_t* uu_wlan_tx_get_frame_info(uu_uchar ac)
{
    if ( uu_wlan_tx_get_available_buffer(ac) == UU_BUF_SIZE_16K )
    {
        return UU_NULL;
    }
    return ((uu_wlan_tx_frame_info_t *)(&uu_ac_q_g[ac][uu_ac_q_rd_indx_g[ac] & UU_BUF_SIZE_16K_MASK]));
}

/*
 * TODO: Remove this, and change the implementation of multiple-protection.  Traversal is not viable in RTL.
 * Gets the reference of the frame info at the offset from the current read index of the AC Q
 */
uu_wlan_tx_frame_info_t* uu_wlan_tx_get_frame_info_at(uu_uchar ac,uu_int32 rd_index_offset)
{
    rd_index_offset = (rd_index_offset * UU_BUF_C_MODEL_MIN_SIZE);

    if ( uu_wlan_tx_get_available_buffer(ac) == UU_BUF_SIZE_16K )
    {
        return UU_NULL;
    }
    return ((uu_wlan_tx_frame_info_t *)(&uu_ac_q_g[ac][(uu_ac_q_rd_indx_g[ac] + rd_index_offset) & UU_BUF_SIZE_16K_MASK]));
}

/* Gives the number of packets in the AC queue of given AC */
// TODO: Remove this, and change the implementation of multiple-protection.  Traversal is not viable in RTL.
/* Added for Multiple frame protection, removed in future */
uu_uint16 uu_wlan_tx_get_no_of_pkts(uu_char ac)
{
    return 0;
}

/*
 * Gets the frame
 */
uu_char* uu_wlan_tx_get_frame(uu_uchar ac)
{
    uu_wlan_tx_frame_info_t *frame_info = (uu_wlan_tx_frame_info_t *)(&uu_ac_q_g[ac][(uu_ac_q_rd_indx_g[ac]) & UU_BUF_SIZE_16K_MASK]);
    return (frame_info->mpdu);
}


uu_char* uu_wlan_tx_get_next_mpdu_in_ampdu(uu_uchar ac, uu_uchar mpdu_num)
{
    int i;
    uu_uint32 offset = 0;

    uu_wlan_tx_frame_info_t *frame_info = (uu_wlan_tx_frame_info_t *)(&uu_ac_q_g[ac][(uu_ac_q_rd_indx_g[ac]) & UU_BUF_SIZE_16K_MASK]);

    offset += sizeof(uu_wlan_tx_frame_info_t);
    for(i = 0; i < mpdu_num; i++)
    {
        offset += frame_info->ampdu_info.mpdu_info[i].mpdu_len;
        offset += frame_info->ampdu_info.mpdu_info[i].pad_len;
    }

    offset = ((uu_uchar*)frame_info - uu_ac_q_g[ac] + offset) & UU_BUF_SIZE_16K_MASK;
    return (&uu_ac_q_g[ac][offset]);
}


/*
 * Gives the length of the next frame
 */
/* TODO: To be reviewed */
uu_uint16 uu_wlan_tx_get_next_frame_len(uu_uchar ac)
{
#if 0
    uu_int32 skip_len;
    uu_wlan_tx_frame_info_t *frame_info = (uu_wlan_tx_frame_info_t *)(&uu_ac_q_g[ac][(uu_ac_q_rd_indx_g[ac] + 1) & UU_BUF_SIZE_16K_MASK]);

    skip_len = sizeof(uu_wlan_tx_frame_info_t) + frame_info->frameInfo.framelen;

#if 1 /* C model only. Avoid problems with wrap-around */
    if (skip_len & UU_BUF_C_MODEL_MIN_SIZE_MASK)
    {
        skip_len += UU_BUF_C_MODEL_MIN_SIZE - (skip_len & UU_BUF_C_MODEL_MIN_SIZE_MASK);
    }
#endif

    /* Next frame reference */
    frame_info = (uu_wlan_tx_frame_info_t *)((uu_uchar*)frame_info + skip_len + 1);
    if (frame_info >= &(uu_ac_q_g[ac][UU_BUF_SIZE_16K_MASK]))
    {
        frame_info = uu_ac_q_g[ac][0];
    }

    /* Next frame length */
    return (frame_info->txvec.format ? (frame_info->txvec.ht_length):(frame_info->txvec.L_length));
#else
    return 0;
#endif
}


/*
 * Gives the number of packets aggregated
 */
uu_uchar uu_wlan_tx_get_aggr_count(uu_uchar ac)
{
    uu_wlan_tx_frame_info_t *frame_info = uu_wlan_tx_get_frame_info(ac);
    return frame_info->aggr_count;
}


uu_bool uu_wlan_tx_is_qos_data(uu_uchar ac)
{
    uu_uchar *mpdu = uu_wlan_tx_get_frame(ac);
    return IEEE80211_IS_FC0_QOSDATA(mpdu);
}


uu_uchar uu_wlan_tx_get_qos_ackpolicy(uu_char ac)
{
    uu_uchar *mpdu = uu_wlan_tx_get_frame(ac);
    if(IEEE80211_IS_FRAME_4ADDR(mpdu))
    {
        return UU_WLAN_GET_4ADDR_QOS_ACKPOLICY_VALUE(mpdu);
    }
    else
    {
        return UU_WLAN_GET_QOS_ACKPOLICY_VALUE(mpdu);
    }
}

uu_uchar uu_wlan_tx_get_bar_or_ba_ack_policy(uu_char ac)
{
    uu_uchar *mpdu = uu_wlan_tx_get_frame(ac);
    return UU_WLAN_GET_BAR_ACKPOLICY_VALUE(mpdu);
}


uu_bool uu_wlan_tx_is_frame_available(uu_char ac)
{
    if (uu_wlan_tx_get_available_buffer(ac) == UU_BUF_SIZE_16K)
    {
        return UU_FALSE;
    }
    else
    {
        return UU_TRUE;
    }
}


uu_uchar* uu_wlan_tx_get_ra_address(uu_char ac)
{
    uu_uchar *mpdu = uu_wlan_tx_get_frame(ac);
    return (&mpdu[UU_RA_OFFSET]);
}

uu_uchar uu_wlan_tx_get_frame_fc0(uu_char ac)
{
    uu_uchar *mpdu = uu_wlan_tx_get_frame(ac);
    return UU_WLAN_GET_FC0_VALUE(mpdu);
}

uu_uchar uu_wlan_tx_get_frame_fc1(uu_char ac)
{
    uu_uchar *mpdu = uu_wlan_tx_get_frame(ac);
    return UU_WLAN_GET_FC1_VALUE(mpdu);
}

uu_char uu_wlan_tx_get_qos_frame_tid(uu_char ac)
{
    uu_uchar *mpdu = uu_wlan_tx_get_frame(ac);
    if(IEEE80211_IS_FRAME_4ADDR(mpdu))
    {
        return UU_WLAN_GET_4ADDR_QOS_TID_VALUE(mpdu);
    }
    else
    {
        return UU_WLAN_GET_QOS_TID_VALUE(mpdu);
    }
}

uu_uint16 uu_wlan_tx_get_frame_scf(uu_char ac)
{
    uu_uchar *mpdu = uu_wlan_tx_get_frame(ac);
    return (*((uu_uint16 *)(&mpdu[UU_SCF_OFFSET])));
}



/* TODO: Some of the functions above are unrelated to AC Q.  Remove them */

/* EOF */

