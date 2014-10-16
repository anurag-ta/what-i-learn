/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_recv.h                                          **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                            **
** All rights reserved.                                                       **
** http://www.uurmi.com                                                       **
**                                                                            **
** All information contained herein is property of Uurmi Systems              **
** unless otherwise explicitly mentioned.                                     **
**                                                                            **
** The intellectual and technical concepts in this file are proprietary       **
** to Uurmi Systems and may be covered by granted or in process national      **
** and international patents and are protect by trade secrets and             **
** copyright law.                                                             **
**                                                                            **
** Redistribution and use in source and binary forms of the content in        **
** this file, with or without modification are not permitted unless           **
** permission is explicitly granted by Uurmi Systems.                         **
**                                                                            **
*******************************************************************************/
/* $Revision: 1.3 $ */

#ifndef __UU_WLAN_RECV_H__
#define __UU_WLAN_RECV_H__

UU_BEGIN_DECLARATIONS

/** Rx filters handling */
extern uu_int32 uu_wlan_rx_setfilters(uu_uint32 total_flags);

/** Handles the received frame from lmac and fills the rx_status.
 * Get the current descriptor, based on the descriptor get the data pointer and copy the
 * frame to sk_buff pinter. Fill the rx status and send back the frame to mac framework using 
 * ieee80211_rx_irqsafe.
 * Free the descriptor after reading from current data pointer.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @return Negative value if the packet is dropped due to alignement else return 0.
 */
extern uu_int32 uu_wlan_rx_tasklet(struct ieee80211_hw *hw);


UU_END_DECLARATIONS

#endif /*__UU_WLAN_RECV_H__ */
