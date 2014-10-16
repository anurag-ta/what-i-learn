/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_rx_proc.h                                   **
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

#ifndef __UU_WLAN_CP_RX_PROC_H__ 
#define __UU_WLAN_CP_RX_PROC_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_cp_ctl_frame_gen.h"


#if 0
typedef enum uu_pkt_type
{
    UU_RTS_PKT,       /* Received RTS */
    UU_CTS_PKT,       /* Received CTS */
    UU_ACK_PKT,       /* Received ACK */
    UU_BAR_PKT,       /* Received BAR */
    UU_BA_PKT,        /* Received BA */
    UU_DATA_ACK_PKT,  /* Received Data MPDU, with ACK policy (may be in BA sess) */
    UU_DATA_BA_PKT,   /* Received Data MPDU, with BA policy */
    UU_AMPDU_RX_DONE, /* Received RX-END, for AMPDU */
    UU_MGMT_PKT       /* M-MPDU is received */
} UU_PKT_TYPE_T;



typedef uu_int32 (*uu_wlan_cp_rx_proc_fn_tp)(uu_wlan_cp_rx_frame_info_t*);
#endif


extern uu_int32  uu_wlan_cp_rx_proc(uu_wlan_cp_rx_frame_info_t* info);


#endif /* __UU_WLAN_CP_RX_PROC_H__ */


