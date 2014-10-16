/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_context.c                                  **
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
#include "uu_wlan_buf.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_cap_context.h"

uu_wlan_cap_context_t  uu_wlan_cap_ctxt_g;


uu_bool uu_wlan_is_tx_ready_for_any_AC_Q(uu_void)
{
    uu_int32 i;

    /* tx_ready packets case */
    for(i=0; i < UU_WLAN_MAX_QID; i++)
    {
        if(uu_wlan_cap_ctxt_g.ac_cntxt[i].tx_ready)
        {
            return true;
        }
    }
    return false;
}


uu_void uu_wlan_cap_retain_tx_frame_info(uu_wlan_tx_frame_info_t *frame_info)
{
    UU_WLAN_COPY_BYTES(uu_wlan_cap_ctxt_g.tx_frame_info.mpdu, frame_info->mpdu,
        UU_WLAN_CAP_MIN_FRAME_HEADER_LEN(frame_info->frameInfo.framelen));
    uu_wlan_cap_ctxt_g.tx_frame_info.txvec_is_aggregated = frame_info->txvec.is_aggregated;
}

uu_void uu_wlan_cap_retain_rx_vec_info(uu_wlan_rx_vector_t *rxvec)
{
    uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_frame_format = rxvec->format;
    uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_length = rxvec->L_length;
    uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate = rxvec->L_datarate;
    uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_psdu_length = rxvec->psdu_length;
}

uu_uint32 uu_wlan_cap_retain_rx_frame_hdr(uu_uchar *frame_p)
{
    uu_uint32  receive_frame_len;

    receive_frame_len = (uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_frame_format == UU_WLAN_FRAME_FORMAT_NON_HT) ?
                   uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_length : uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_psdu_length;
    UU_WLAN_COPY_BYTES(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu, frame_p, UU_WLAN_CAP_MIN_FRAME_HEADER_LEN(receive_frame_len));
    return receive_frame_len;
}


/* EOF */

