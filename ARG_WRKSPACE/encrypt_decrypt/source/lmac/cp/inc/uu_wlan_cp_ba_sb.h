/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_ba_sb.h                                     **
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

#ifndef __UU_WLAN_CP_BA_SB_H__
#define __UU_WLAN_CP_BA_SB_H__

#include "uu_datatypes.h"
#include "uu_wlan_lmac_if.h"


/* This is used to identify whether received Seq Num is NEW or OLD */
#define BA_SEQN_RANGE_BY2        2048
#define BA_COMP_BITMAP_SIZE         8

#define BA_BASIC_BITMAP_BUF_SIZE   64
#define BA_BASIC_BITMAP_BUF_MASK 0x3F

#define BA_MPDU_SEQN_MAX         4095
#define BA_MPDU_SEQN_WIDTH         12
#define BA_SB_MPDU_FRAG_MAX        16 /* Max 16 fragments per SN */
#define BA_ELEVATE_SN_FOR_WRAPAROUND(sn) (sn | 0x1000)


#define MPDU_SN_FROM_SCF(scf)  (scf >> 4)
#define MPDU_FN_FROM_SCF(scf)  (scf & 0x0F)

extern uu_int32 uu_wlan_ba_sb_update_rx(uu_wlan_ba_sb_t *sb_p, uu_uint16  scf);
extern uu_int32 uu_wlan_sb_update_tx(uu_wlan_ba_sb_t *sb_p, uu_uint16 scf);


#endif /* __UU_WLAN_CP_BA_SB_H__ */

