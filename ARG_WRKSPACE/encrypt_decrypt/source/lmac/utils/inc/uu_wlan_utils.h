/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_utils.h                                        **
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

#ifndef __UU_WLAN_UTILS_H__
#define __UU_WLAN_UTILS_H__

#include "uu_datatypes.h"
#include "uu_wlan_reg.h"

#define copyAddr(dst,src) \
      *(uu_uint8*)(dst)     = *(uu_uint8*)(src);\
      *((uu_uint8*)(dst)+1) = *((uu_uint8*)(src)+1);\
      *((uu_uint8*)(dst)+2) = *((uu_uint8*)(src)+2);\
      *((uu_uint8*)(dst)+3) = *((uu_uint8*)(src)+3);\
      *((uu_uint8*)(dst)+4) = *((uu_uint8*)(src)+4);\
      *((uu_uint8*)(dst)+5) = *((uu_uint8*)(src)+5);

#define isaddrsame(x,y) \
      ((*(uu_uint8*)(x)    == *(uu_uint8*)(y))&&\
      (*((uu_uint8*)(x)+1) == *((uu_uint8*)(y)+1))&&\
      (*((uu_uint8*)(x)+2) == *((uu_uint8*)(y)+2))&&\
      (*((uu_uint8*)(x)+3) == *((uu_uint8*)(y)+3))&&\
      (*((uu_uint8*)(x)+4) == *((uu_uint8*)(y)+4))&&\
      (*((uu_uint8*)(x)+5) == *((uu_uint8*)(y)+5)) )


#define isNullAddr(addr) \
      ((*(uu_uint8*)(addr)    == 0x00)&&\
      (*((uu_uint8*)(addr)+1) == 0x00)&&\
      (*((uu_uint8*)(addr)+2) == 0x00)&&\
      (*((uu_uint8*)(addr)+3) == 0x00)&&\
      (*((uu_uint8*)(addr)+4) == 0x00)&&\
      (*((uu_uint8*)(addr)+5) == 0x00 ))

#define isBroadcastAddr(addr) \
      ((*(uu_uint8*)(addr)    == 0xff)&&\
      (*((uu_uint8*)(addr)+1) == 0xff)&&\
      (*((uu_uint8*)(addr)+2) == 0xff)&&\
      (*((uu_uint8*)(addr)+3) == 0xff)&&\
      (*((uu_uint8*)(addr)+4) == 0xff)&&\
      (*((uu_uint8*)(addr)+5) == 0xff ))

#define isMulticastAddr(addr)  ((*(uu_uint8*)(addr) & 0x01) ==   0x01) 

#define isLocalMacAddr(addr)   UU_IS_SAME_MAC_ADDR(addr, UU_WLAN_IEEE80211_STA_MAC_ADDR_R)

#define isBSSIDMacAddr(addr)   UU_IS_SAME_MAC_ADDR(addr, UU_WLAN_IEEE80211_STA_BSSID_R)

#define setNullAddr(addr) \
      ((*(uu_uint8*)(addr)    = 0x00), \
      (*((uu_uint8*)(addr)+1) = 0x00), \
      (*((uu_uint8*)(addr)+2) = 0x00), \
      (*((uu_uint8*)(addr)+3) = 0x00), \
      (*((uu_uint8*)(addr)+4) = 0x00), \
      (*((uu_uint8*)(addr)+5) = 0x00 ))


extern uu_uchar uu_wlan_get_ac_from_tid(uu_uchar tid);


#endif /* __UU_WLAN_UTILS_H__ */

