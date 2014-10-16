/*******************************************************************************
**                                                                            **
** File name :        uu_mac_tx.h                                             **
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

#ifndef __UU_MAC_TX_H__
#define __UU_MAC_TX_H__

/** Implementation defaults for tx vector filling */
/* TODO: What is this condition for? */
#ifdef UU_WLAN_TPC
#ifdef UU_WLAN_DATA_160
#define UU_WLAN_CH_BW_DEAFAULT 3
#elif defined UU_WLAN_DATA_80
#define UU_WLAN_CH_BW_DEAFAULT 2
#elif defined UU_WLAN_DATA_40
#define UU_WLAN_CH_BW_DEAFAULT 1
#else
#define UU_WLAN_CH_BW_DEAFAULT 0
#endif
#else /* Not TPC */
#define UU_WLAN_CH_BW_DEAFAULT 2
#endif


UU_BEGIN_DECLARATIONS

/** Work queue function to trigger work queue for beacon generation.
 * This function will be triggered after getting beacon generation indication from LMAC.
 * @param[in] Work structure.
 */
extern uu_void uu_wlan_beacon_update_trigger_work(struct work_struct *work);

/** Mac80211 callback which is called before first netdevice is attached to 
 * the hardware.
 * Start the lmac process here. Return -EIO if failed to start the lmac.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw().
 * @param[in] skb Contains the beacon frame generated from userspace application.
 * @param[in] txctl Contains txq mapping corresponding to ac queue.
 */
extern uu_int32 uu_wlan_tx_start(struct ieee80211_hw *hw,
                                struct sk_buff *skb,
                                uu_wlan_txq_info_t *txq);

/** Sets the supported tx flags.
 * Select the supported flags from "mac80211_tx_control_flags".
 * Refer mac80211.h file for mac framework supported tx control flags.
 * @param[in] skb the frame to which tx flags to be setup.
 * @return supported tx flag bitmap.
 */
extern uu_int32 uu_wlan_setup_tx_flags(struct sk_buff *skb);

/** Calls corresponding tx function based on aggregation support flag.
 * Send normal frame or aggregated frame to lmac based on the aggregation support flag.
 * @param[in] mac_context umac context structure.
 * @param[in] txctl Tx control pointer which contains the corresponding queue support.
 * @param[in] skb sk_buff pointer which contains the frame.
 */
extern uu_void uu_wlan_start_tx_send(uu_wlan_umac_context_t *mac_context,
                                    uu_wlan_txq_info_t *txq,
                                    struct sk_buff *skb);

UU_END_DECLARATIONS
#endif /*__UU_MAC_TX_H__ */
