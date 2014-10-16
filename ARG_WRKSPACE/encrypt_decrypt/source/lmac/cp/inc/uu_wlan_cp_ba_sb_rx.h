/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_ba_sb_rx.h                                  **
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

#ifndef __UU_WLAN_CP_BA_SB_RX_H__
#define __UU_WLAN_CP_BA_SB_RX_H__

#include "uu_datatypes.h"
#include "uu_wlan_cp_ba_sb.h"


/*
* Function declarations
*/

extern uu_int32 uu_wlan_sb_fill_basic_ba(uu_wlan_ba_sb_t *ba, uu_uint16 seq_no, uu_uint16 *bitmap, uu_uint16 *seq_no_ba);

extern uu_int32 uu_wlan_sb_fill_comp_ba(uu_wlan_ba_sb_t *sb, uu_uint16 seq_no, uu_uchar *bitmap, uu_uint16 *seq_no_bitmap);

extern uu_int32 uu_wlan_sb_fill_implicit_comp_ba(uu_wlan_ba_sb_t *sb_p, uu_uchar *ba_bitmap, uu_uint16 *seq_no_bitmap);

#endif /* __UU_WLAN_CP_BA_SB_RX_H */

