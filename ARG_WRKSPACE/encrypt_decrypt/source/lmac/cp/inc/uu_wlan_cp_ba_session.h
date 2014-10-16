/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cp_ba_session.h                                **
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

#ifndef __UU_WLAN_CP_BA_SESSION_H__
#define __UU_WLAN_CP_BA_SESSION_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_cp_ba_sb.h"


extern uu_wlan_ba_ses_context_t* uu_wlan_get_ba_contxt_of_sta(uu_uchar *sta, uu_uchar tid, uu_bool dir);

#endif /* __UU_WLAN_CP_BA_SESSION_H__ */

