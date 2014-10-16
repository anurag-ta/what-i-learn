/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_ctrl_pkt_gen.c                              **
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

/* headers */
#ifdef USER_SPACE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "crc32.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_lock.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_cp_if.h"

#include "uu_wlan_cp_ctl_frame_gen.h"
#include "uu_wlan_cp_ba_sb_rx.h"
#include "uu_wlan_cp_ba_session.h"


/* TODO: The control frame generation functions need only a few fields.  See whether they can be sent, instead of whole structure */

uu_uint32 uu_wlan_gen_rts_frame(uu_wlan_cp_tx_frame_info_t *info, uu_uint16 duration)
{
    struct uu_80211_rts_frame *rts_frame; 

    /* Alloc memory for the RTS packet to be prepared */
    rts_frame = (uu_80211_rts_frame_t *)tx_ctl_frame_info_g.mpdu;

    /* if (NULL == rts_frame){
        return UU_ERR_NULL_ADDRESS;
    } */

    /* populate the RTS ctrl frame */
    rts_frame->fc[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL |
                                  IEEE80211_FC0_SUBTYPE_RTS;
    rts_frame->fc[1] = 0x00; // all fileds are zero need to take care of 'pwrmgmt bit' if working as STA 
    /*duration filling here */
    rts_frame->dur[0] = (unsigned char)duration;
    rts_frame->dur[1] = (unsigned char)(duration >> 8 );

    /*ra fields of rts frame filling here*/
    UU_COPY_MAC_ADDR(rts_frame->ra, &info->ra);
    UU_COPY_MAC_ADDR(rts_frame->ta, UU_WLAN_IEEE80211_STA_MAC_ADDR_R);

    /* calculate & fill FCS */
    uu_wlan_fill_crc(0, tx_ctl_frame_info_g.mpdu, UU_RTS_FRAME_LEN-4);

    return UU_RTS_FRAME_LEN; 
} /* uu_wlan_gen_rts_frame */

       
uu_uint32 uu_wlan_gen_selfcts_frame(uu_wlan_cp_tx_frame_info_t *info, uu_uint16 duration)
{
    uu_uchar *frame_buff = tx_ctl_frame_info_g.mpdu;

    /* populate the cTS ctrl frame */
    frame_buff[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL |
    IEEE80211_FC0_SUBTYPE_CTS;
    frame_buff[1] = 0x00; // all fileds are zero need to take care of 'pwrmgmt bit' if working as STA 
    /*cts frame duration fields filling here*/
    frame_buff[2] = (unsigned char)duration; 
    frame_buff[3] = (unsigned char)(duration>>8);
    /*ra field of cts frame filling here*/
    UU_COPY_MAC_ADDR((frame_buff + UU_RA_OFFSET), info->ra);

    /* Calculate & fill FCS */
    uu_wlan_fill_crc(0, frame_buff,  UU_CTS_FRAME_LEN -4);

    return UU_CTS_FRAME_LEN;
} /* uu_wlan_gen_selfcts_frame */


uu_uint32 uu_wlan_gen_cts_frame(uu_wlan_cp_rx_frame_info_t* info, uu_uint16 duration)
{
    uu_uchar *frame_buff = tx_ctl_frame_info_g.mpdu;

    /* populate the cTS ctrl frame */
    frame_buff[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL |
    IEEE80211_FC0_SUBTYPE_CTS;
    frame_buff[1] = 0x00; // all fileds are zero need to take care of 'pwrmgmt bit' if working as STA 
    /*cts frame duration fields filling here*/
    frame_buff[2] = (unsigned char)duration;
    frame_buff[3] = (unsigned char)(duration>>8);
    /*ra field of cts frame filling here*/
    UU_COPY_MAC_ADDR((frame_buff + UU_RA_OFFSET), info->ta);
    
    /* Calculate & fill FCS */
    uu_wlan_fill_crc(0, frame_buff,  UU_CTS_FRAME_LEN -4);

    return UU_CTS_FRAME_LEN;
} /* uu_wlan_gen_cts_frame */


uu_uint32 uu_wlan_gen_ack_frame(uu_wlan_cp_rx_frame_info_t* info, uu_uint16 duration)
{
    uu_uchar *frame_buff = tx_ctl_frame_info_g.mpdu;

    /*populate the ACK ctrl frame */
    frame_buff[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL |
    IEEE80211_FC0_SUBTYPE_ACK;
    frame_buff[1] = 0x00; // all fileds are zero need to take care of 'pwrmgmt bit' if working as STA 
    /*duration of ack frame filling here*/
    frame_buff[2] = (unsigned char)duration;
    frame_buff[3] = (unsigned char)(duration>>8);
    /*ra fields of ack frame filling here*/
    UU_COPY_MAC_ADDR((frame_buff + UU_RA_OFFSET), info->ta);

    /* Calculate & fill FCS */
    uu_wlan_fill_crc(0, frame_buff, UU_ACK_FRAME_LEN -4);

    return UU_ACK_FRAME_LEN;
} /* uu_wlan_gen_ack_frame */


/* TODO: Multi-TID support */
/* ba frame generation for implicit case */
uu_uint32 uu_wlan_gen_ba_frame(uu_wlan_cp_rx_frame_info_t* info, uu_uint16 duration, uu_bool implicit)
{
    uu_uchar *frame_buff = tx_ctl_frame_info_g.mpdu;
    uu_wlan_ba_ses_context_t *ba;
    uu_uint16 seq_no_bitmap;
    uu_uint16 seq_no;
    uu_uint8  *bitmap;
    uu_uint8  is_compressed = 0;


    /* populate the ba  ctrl frame */
    frame_buff[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_BA;
    frame_buff[1] = 0x00; // all fileds are zero need to take care of 'pwrmgmt bit' if working as STA 

    /*duration assigning here*/
    frame_buff[2] = (unsigned char)duration;
    frame_buff[3] = (unsigned char)(duration>>8);

    /*ra filed of BA frame filling here*/
    UU_COPY_MAC_ADDR((frame_buff + UU_RA_OFFSET), info->ta);
    UU_COPY_MAC_ADDR((frame_buff + UU_TA_OFFSET), UU_WLAN_IEEE80211_STA_MAC_ADDR_R);

    if(info->fc0 == (IEEE80211_FC0_SUBTYPE_BAR | IEEE80211_FC0_TYPE_CTL))
    {
        seq_no = MPDU_SN_FROM_SCF(info->scf);
        if (info->bar_type == UU_WLAN_BAR_TYPE_COMP_BA)
	    {
            is_compressed = 1;
	    }
    }
    else /* TODO: Does it always represent 'implicit'? Then we don't need that additional arg */
    {
        seq_no = MPDU_SN_FROM_SCF(info->scf);
        is_compressed = 1;
    }
    UU_WLAN_SPIN_LOCK(&uu_wlan_ba_ses_context_lock_g);
    ba = uu_wlan_get_ba_contxt_of_sta(info->ta, info->tid, 0);
    if(ba == NULL)
    {
        UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
        return UU_BA_SESSION_INVALID;
    }

    bitmap = (uu_void *) (tx_ctl_frame_info_g.mpdu + UU_BA_BITMAP_OFFSET);
    if (is_compressed)
    {
        UU_WLAN_MEM_SET(bitmap, UU_COMP_BA_BITMAP_SZ);
        tx_ctl_frame_info_g.frameInfo.framelen = UU_BA_COMPRESSED_FRAME_LEN;
        if (implicit)
        {
            /* TODO: Implicit BA generation is required, only if atleast 1 frame with 'Normal ACK' policy is received in recent AMPDU */
            uu_wlan_sb_fill_implicit_comp_ba(&ba->ses.sb, bitmap, &seq_no_bitmap);
        }
        else
        {
            uu_wlan_sb_fill_comp_ba(&ba->ses.sb, seq_no, bitmap, &seq_no_bitmap);
        }
    }
    else
    {
        UU_WLAN_MEM_SET(bitmap, UU_BA_BITMAP_SZ);
        tx_ctl_frame_info_g.frameInfo.framelen = UU_BA_BASIC_FRAME_LEN;
        uu_wlan_sb_fill_basic_ba(&ba->ses.sb, seq_no, (uu_uint16*)bitmap, &seq_no_bitmap);
    }

    UU_WLAN_SPIN_UNLOCK(&uu_wlan_ba_ses_context_lock_g);
    seq_no_bitmap = seq_no_bitmap << 4;
    UU_WLAN_COPY_BYTES( (uu_void *) (tx_ctl_frame_info_g.mpdu + UU_SSN_FIELD_OFFSET), (uu_void *) (&seq_no_bitmap), 2);
    if(info->fc0 == (IEEE80211_FC0_SUBTYPE_BAR | IEEE80211_FC0_TYPE_CTL))
    {
        UU_WLAN_COPY_BYTES((uu_void *) (tx_ctl_frame_info_g.mpdu + UU_BAR_FC_FIELD_OFFSET), 
                                    (uu_void *)&info->bar_fc_field, 2);
    }
    else
    {
        /* Setting ACK policy to no-ACK & Compressed BA, in the BA frame */
        tx_ctl_frame_info_g.mpdu[UU_BAR_FC_FIELD_OFFSET] |= 0X05;
        tx_ctl_frame_info_g.mpdu[UU_BAR_FC_FIELD_OFFSET + 1] |= (info->tid << 4);//filling tid value.
    }

    /* Calculate & fill FCS */
    uu_wlan_fill_crc(0, frame_buff, tx_ctl_frame_info_g.frameInfo.framelen-4);

    return tx_ctl_frame_info_g.frameInfo.framelen;
} /* uu_wlan_gen_ba_frame */


/*bar frame populating starting here*/
uu_uint32 uu_wlan_gen_bar_frame(uu_uint8 bartype, uu_uint8 tid, uu_uint16 ssn, uu_uint16 duration, uu_uchar *ra)
{
    uu_uchar *frame_buff = tx_ctl_frame_info_g.mpdu;

    /* populate the bar  ctrl frame */
    frame_buff[0] = IEEE80211_FC0_VERSION_0 | IEEE80211_FC0_TYPE_CTL |
        IEEE80211_FC0_SUBTYPE_BAR;
    frame_buff[1] = 0x00; // all fileds are zero need to take care of 'pwrmgmt bit' if working as STA
    /*duration assigning here*/
    frame_buff[2] = (unsigned char)duration;
    frame_buff[3] = (unsigned char)(duration>>8);
    /*ra filed of BA frame filling here*/
    UU_COPY_MAC_ADDR((frame_buff + UU_RA_OFFSET), ra);
    UU_COPY_MAC_ADDR((frame_buff + UU_TA_OFFSET), UU_WLAN_IEEE80211_STA_MAC_ADDR_R);

    frame_buff[16] = bartype; /* First 3 bits only contains the bar ack polocy */
    frame_buff[17] = tid << 4; 
    /*  add ctrl field of ba and status fiels............    */
    frame_buff[18] = (unsigned char) ssn;
    frame_buff[19] = (unsigned char)ssn >> 8;

    /* calculate & fill FCS */
    uu_wlan_fill_crc(0, frame_buff, UU_BAR_FRAME_LEN-4);

    return UU_BAR_FRAME_LEN;
}/* uu_wlan_gen_bar_frame */


/* TODO: CRC filling is spread everywhere. Need to calculate just before sending to PHY */


/* EOF */

