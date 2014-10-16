/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_lmac_sta_info.h                                **
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

#ifndef __UU_WLAN_LMAC_STA_INFO_H__
#define __UU_WLAN_LMAC_STA_INFO_H__

#include "uu_wlan_lmac_if.h"


uu_wlan_asso_sta_info_t* uu_wlan_lmac_get_sta(const uu_uchar*  addr_p);


#endif /* __UU_WLAN_LMAC_STA_INFO_H__ */

