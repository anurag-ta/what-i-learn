/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_drv.h                                           **
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

#ifndef __UU_MAC_DRV_H__
#define __UU_MAC_DRV_H__

#include <linux/skbuff.h>

UU_BEGIN_DECLARATIONS

extern uu_void uu_wlan_send_to_device(uu_uint8 ac, struct sk_buff *skb);

UU_END_DECLARATIONS

#endif /*__UU_MAC_DRV_H__ */
