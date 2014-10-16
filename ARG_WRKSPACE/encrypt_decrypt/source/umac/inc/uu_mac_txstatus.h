/*******************************************************************************
**                                                                            **
** File name :        uu_mac_txstatus.h                                       **
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

#ifndef __UU_MAC_TXSTATUS_H__
#define __UU_MAC_TXSTATUS_H__

UU_BEGIN_DECLARATIONS

/** This will be called from umac_cbk function, after getting txstatus indication from LMAC.
 * This will be invoked from LMAC for each packet transmission.
 */
extern uu_void uu_mac_drv_get_ac_txstatus(uu_void);

extern uu_void uu_wlan_send_tx_status(int i, int j);

UU_END_DECLARATIONS
#endif /*__UU_MAC_TXSTATUS_H__ */
