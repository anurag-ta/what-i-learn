/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_agg.h                                           **
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

#ifndef __UU_WLAN_AGG_H__
#define __UU_WLAN_AGG_H__

#include "uu_wlan_main.h"

#define UU_WLAN_LOWER_WATER_MARK 2

UU_BEGIN_DECLARATIONS

uu_int32 uu_wlan_tx_send_normal(uu_wlan_umac_context_t *mac_context,
                uu_wlan_txq_info_t *txq);

UU_END_DECLARATIONS
#endif /*__UU_WLAN_AGG_H__ */
