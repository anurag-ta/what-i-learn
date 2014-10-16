/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_rx_handler.h                                   **
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

#ifndef __UU_WLAN_RX_HANDLER_H__
#define __UU_WLAN_RX_HANDLER_H__

#include "uu_datatypes.h"


/* Return values used internally in the deaggregation functionality */
#define UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME    2
#define UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME  3
#define UU_LMAC_RX_RET_NON_AGG_FRAME              4

/* List of possible destinations to the frame - Broadcast, Multicast, Unicast to us, Unicast to others */
typedef enum uu_wlan_rx_frame_dest
{
    UU_WLAN_RX_FRAME_DEST_BROADCAST,
    UU_WLAN_RX_FRAME_DEST_MULITCAST,
    UU_WLAN_RX_FRAME_DEST_US,
    UU_WLAN_RX_FRAME_DEST_OTHERS,
    UU_WLAN_RX_FRAME_DEST_LOOPBACK,  /* We got our sent frame back to us (can happen in monitor mode?) */
    UU_WLAN_RX_FRAME_ERROR
} uu_wlan_rx_frame_dest_t;


#endif /* __UU_WLAN_RX_HANDLER_H__ */


/* EOF */


