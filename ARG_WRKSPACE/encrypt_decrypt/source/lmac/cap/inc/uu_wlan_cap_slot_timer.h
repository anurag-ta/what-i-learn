/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_cap_slot_timer.h                                **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                           **
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

#ifndef __UU_WLAN_CAP_SLOT_TIMER_H__
#define __UU_WLAN_CAP_SLOT_TIMER_H__

#include "uu_datatypes.h"


#define UU_WLAN_CAP_SLOT_INTERVEL     (UU_WLAN_CAP_SLOT_INTERVEL_REG << TIME_RESOLUTION_EXPONENT)

UU_BEGIN_DECLARATIONS


extern uu_int32 uu_wlan_cap_slot_timer_start(uu_void);
extern uu_int32 uu_wlan_cap_slot_timer_stop(uu_void);

/** Starts the DTIM Timer.
 * Called by Rx handler after Receiving first DTIM beacon immediately after configuring to PS.
 */
extern uu_int32 uu_wlan_cap_dtim_timer_start(uu_void);

/** Stops the DTIM Timer.
 * Called in DTIM timer callback, i.e after DTIM timer expiry if PS is deconfigured.
 * Also called in LMAC shutdown, for stopping the timer before LMAC is going to shutdown.
 */
extern uu_int32 uu_wlan_cap_dtim_timer_stop(uu_void);

UU_END_DECLARATIONS

#endif /* __UU_WLAN_CAP_SLOT_TIMER_H__ */

