/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_tx_handler.h                                   **
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

#ifndef __UU_WLAN_TX_HANDLER_H__
#define __UU_WLAN_TX_HANDLER_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"


/** Mainstains status of AMPDU of an AC, at the top of Q, for Tx internal use.
 * Set, while sending for the first time (at the time of aggregation).
 * Updated, on getting BA, by the CP module.
 * Checked, at the time of reaggregation by Tx.
 * Cleared, while clearing the AMPDU from Tx Q.
 */
typedef struct uu_wlan_ampdu_status
{
    /* Bitmap for transmitted packets. Update 'pending_mpdu', along with this */
    uu_uint8  bitmap:8;
    /* Number of MPDUs, still set in bitmap */
    uu_uint8  pending_mpdu:3;
    uu_uint8  reserved0:5;

    /* Starting mpdu sequence number in A-MPDU */
    uu_uint16 ssn:12;

    /* Reserved */
    uu_uint16 reserved1:4; 
} uu_wlan_ampdu_status_t;


extern uu_wlan_ampdu_status_t uu_wlan_tx_ampdu_status_g[UU_WLAN_MAX_QID];
#endif/* __UU_WLAN_TX_HANDLER_H__ */

/* EOF */

