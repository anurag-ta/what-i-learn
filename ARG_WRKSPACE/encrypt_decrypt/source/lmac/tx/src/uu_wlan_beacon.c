/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_beacon.c                                       **
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include "uu_datatypes.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_cap_slot_timer.h" /* TODO: Remove this depedency on CAP */
#include "uu_wlan_beacon.h"

#define MicroS_TO_NS(x) (x * 1000)


static struct hrtimer uu_wlan_beacon_hr_timer_g;
/*
** This is used to avoid crashes, when the Linux kernel module is unloaded.
** This flag is NOT required in RTL implementation.
*/
static uu_int8   exit_flag_g = 0;


/*
** NOTE: The beacon interval is taken from register, which might be changed at any time.
** The new value from register will be used, after the expiry of the current interval.
*/
enum hrtimer_restart uu_wlan_beacon_hrtimer_cbk( struct hrtimer *timer )
{
    ktime_t timeout, cur_time;
    unsigned long misses;

    if (exit_flag_g == 1)
    {
        return HRTIMER_NORESTART;
    }
    /* Calling UMAC to generate BEACON */
    lmac_ops_gp->umacCBK(UU_LMAC_IND_GEN_BEACON);
    timeout = ktime_set(0, MicroS_TO_NS(UU_REG_LMAC_BEACON_INRVL));
    cur_time = ktime_get();

    misses = hrtimer_forward(timer, cur_time, timeout);
    return HRTIMER_RESTART;
}


uu_int32 uu_wlan_beacon_timer_start( uu_void)
{
    ktime_t ktime;

    UU_WLAN_LOG_DEBUG(("LMAC: Beacon HR Timer module starting\n"));
    exit_flag_g = 0;
    ktime = ktime_set( 0, MicroS_TO_NS(UU_REG_LMAC_BEACON_INRVL) );

    hrtimer_init( &uu_wlan_beacon_hr_timer_g, CLOCK_MONOTONIC, HRTIMER_MODE_REL );

    uu_wlan_beacon_hr_timer_g.function = & uu_wlan_beacon_hrtimer_cbk;

    UU_WLAN_LOG_INFO(( "Starting Beacon timer to fire in %dus (%ld)\n", UU_REG_LMAC_BEACON_INRVL, jiffies ));

    hrtimer_start( &uu_wlan_beacon_hr_timer_g, ktime, HRTIMER_MODE_REL );

    return UU_SUCCESS;
}

uu_int32 uu_wlan_beacon_timer_stop(uu_void)
{
    int ret;

    exit_flag_g = 1;
    ret = hrtimer_cancel( &uu_wlan_beacon_hr_timer_g );
    if (ret)
    {
        UU_WLAN_LOG_DEBUG(("The Beacon timer was still in use...\n"));
    }
    UU_WLAN_LOG_DEBUG(("Beacon HR Timer module uninstalling\n"));

    return UU_SUCCESS;
}


/* EOF */

