/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_tx_aggregation.h                               **
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

#ifndef __UU_WLAN_TX_AGGREGATION_H__
#define __UU_WLAN_TX_AGGREGATION_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"

#define UU_WLAN_ZERO_LEN_DELIMITER      0x4E000000

extern uu_uint32 uu_wlan_tx_update_len(uu_wlan_tx_frame_info_t* frame_info);
extern uu_void uu_wlan_tx_do_aggregation(uu_wlan_tx_frame_info_t* frame_info);
extern uu_void uu_wlan_tx_do_reaggregation(uu_wlan_tx_frame_info_t* frame_info);

#endif/* __UU_WLAN_TX_AGGREGATION_H__ */

/* EOF */

