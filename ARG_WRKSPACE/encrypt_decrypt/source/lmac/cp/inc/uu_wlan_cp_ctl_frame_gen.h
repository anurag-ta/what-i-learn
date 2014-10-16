/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_ctl_frame_gen.h                             **
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

#ifndef __UU_WLAN_CP_CTL_FRAME_GEN_H__
#define __UU_WLAN_CP_CTL_FRAME_GEN_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"


/* TODO: In the structure comment, put which memebers are filled for which request */
/* On receiving every frame, required members of this structure are filled */
typedef struct uu_wlan_cp_rx_frame_info
{
    /** Byte 1 */
    /** Frame Control field byte 0.
     * Special case: Set to IEEE80211_FC0_TYPE_RESERVED, to trigger check for
     *   BA generation (like 'is_ampdu_end'), when:
     *      - Rx failed while receiving some MPDU of an AMPDU.
     *      - Last MPDU has got validation error.
     */
    uu_uint8 fc0; /* 8 bits */

    /** Byte 2 */
    /** ToDS, FromDS, More Fragments, Retry, Power Management, More Data, Protected Frame, Order */
    uu_uint8 fc1; /* 8 bits */

    /** Byte 3,4 */
    uu_uint16 bar_fc_field; /* 2 bytes */

    /** Bytes 5 */
    /** QoS ack policy - 0(normal ack / implicit-BA), 1(no ack), 2(PSMP ack), 3(MPDU under BA).
     * If used with BAR and BA frames, only 0 & 1 are applicable.
     */
    uu_uint8 qos_ack_policy:2; /* 2 bits */

    uu_uint8 bar_type:2; /* 2 bits */
    uu_uint8 tid:4; /* 4 bits */

    /** Bytes 6,7 */
    /* Duration of the received packet */
    uu_uint16 duration; /* 16 bits */

    /** Bytes 8,9 */
    /* Sequence number of the packet */
    uu_uint16 scf; /* 16 bits */

    /** Bytes 10-15 */
    /* sender address */
    uu_uchar ta[6]; /* 6 bytes */

    /** Bytes 16-21 */
    /* receipent address */
    uu_uchar ra[6]; /* 6 bytes */

    /** Bytes 22 */
    /** Format of the PPDU (non-HT, HT-MF, HT-GF, VHT) */
    uu_uint8 format:2; /* 2 bits */

    /** L_datarate 
     * NON-HT: Indicates the rate used to transmit the PSDU in megabits per second.
     * HT-MF: Indicates the data rate value that is in the L-SIG. This use is defined in 9.13.4.
     * HT-GF: Not present.
     * used in calculating Duration
     */
    uu_uint8 L_datarate:4; /* 4 bits */
    uu_uint8 reserved1:2;

    /** Bytes 23 */
    /** MCS 
     * HT-MF and HT-GF: Selects the modulation and coding scheme used in the transmission of the
     * packet. The value used in each MCS is the index defined in 20.6.
     * Integer: range 0 to 76. Values of 77 to 127 are reserved.
     * The interpretation of the MCS index is defined in 20.6.
     * used in calculating Duration
     */
    uu_uint8 mcs:7; /* 7 bits */
    uu_uint8 reserved2:1;

    /** Bytes 24 */
    /** NON-HT-MODULATION
     * used in calculating Duration
     */
    uu_uint8 modulation:3; /* 3 bits */

    /** Received frame bandwidth that will be assigned to control response frame
     */
    uu_uint8 bandwidth:3; /* 3 bits */

    /** Aggregation
     * HT-MF and HT-GF: Indicates whether the PSDU contains an A-MPDU.
     *    Value 0 indicates that the packet is not aggregated and 1 indicates aggregated packet.
     */
    uu_uint8 is_aggregated:1; /* 1 bit */

    /** Indicates end of AMPDU (Rx-End from PHY, during AMPDU receiving).
     * CP module has to check whether an Implicit-BA has to be generated.
     */
    uu_uint8 is_ampdu_end:1; /* 1 bit */
    //uu_uint32 psdu_length;/* 20 bits */
} uu_wlan_cp_rx_frame_info_t;


/* TODO: Structure comment should indicate - for which requests this will be used */
/* While transmitting data frame, required members of this structure are filled */
typedef struct uu_wlan_cp_tx_frame_info
{
    /** Format of the PPDU (non-HT, HT-MF, HT-GF, VHT) */
    uu_uint8 format:2; /* 2 bits */

    /** Modulation of the PPDU */
    uu_uint8 modulation:3;

    /** Bandwidth of the PPDU (non-HT, HT-MF, HT-GF, VHT) */
    uu_uint8 bandwidth:3; /* 3 bits */

    /** Rate of required control frame (RTS/Self-CTS) */
    uu_uint8 rtscts_rate:4; /* 4 bits */
    uu_uint8 reserved:4;

    /* receipent address */
    uu_uchar ra[6]; /* 6 bytes */
} uu_wlan_cp_tx_frame_info_t;

extern uu_bool uu_wlan_cp_waiting_for_ack_g;
extern uu_bool uu_wlan_cp_waiting_for_cts_g;

extern uu_uint32 uu_wlan_gen_rts_frame(uu_wlan_cp_tx_frame_info_t* info, uu_uint16 duration);
extern uu_uint32 uu_wlan_gen_selfcts_frame(uu_wlan_cp_tx_frame_info_t* info, uu_uint16 duration);
extern uu_uint32 uu_wlan_gen_cts_frame(uu_wlan_cp_rx_frame_info_t* info, uu_uint16 duration);
extern uu_uint32 uu_wlan_gen_ack_frame(uu_wlan_cp_rx_frame_info_t* info, uu_uint16 duration);
extern uu_uint32 uu_wlan_gen_ba_frame(uu_wlan_cp_rx_frame_info_t* info, uu_uint16 duration, uu_bool implicit);
extern uu_uint32 uu_wlan_gen_bar_frame(uu_uint8 bartype, uu_uint8 tid, uu_uint16 ssn, uu_uint16 duration, uu_uchar *ra);


#endif /* __UU_WLAN_CP_CTL_FRAME_GEN_H__ */

/* EOF */
 
