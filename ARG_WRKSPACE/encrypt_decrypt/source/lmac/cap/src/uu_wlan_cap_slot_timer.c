/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_slot_timer.c                               **
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

#ifdef __KERNEL__
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#endif

#include "uu_datatypes.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_cap_slot_timer.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_ps.h"

static uu_bool uu_wlan_run_dtim_timer_g;
#ifdef UU_KTIMER
static struct timer_list uu_ktimer_g;
#endif

#define MicroS_TO_NS(x) (x * 1000)

#ifdef UU_HR_TIMER
static struct hrtimer uu_wlan_cap_slot_timer_g;
static uu_int8   exit_flag_g = 0;

enum hrtimer_restart uu_wlan_cap_slot_timer_cbk( struct hrtimer *timer )
{
    ktime_t  timeout, cur_time;
    unsigned long misses;

    if (exit_flag_g == 1)
    {
        return HRTIMER_NORESTART;
    }

    /* Set the next HR timer tick, to that we get it in time */
    timeout = ktime_set(0, MicroS_TO_NS(UU_WLAN_CAP_SLOT_INTERVEL));
    cur_time = ktime_get();
    misses = hrtimer_forward(timer, cur_time, timeout);

    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);
    return HRTIMER_RESTART;
}
#endif

#ifdef UU_KTIMER
static uu_void uu_wlan_cap_ktimer_cbk(unsigned long data)
{
    /* Converting timer tick into a DTIM timer, using this counter */
    static uu_uint16 dtim_counter;

#ifdef UU_WLAN_TSF
    uu_wlan_tsf_r ++;
#endif
    /* put timer tick event in cap Q */
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TIMER_TICK, NULL, 0);

    if (uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav > 0)
    {
        uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_nav--;
    }

    /* powersave related functionality */
    if(uu_wlan_run_dtim_timer_g)
    {
        if(dtim_counter == uu_wlan_lmac_dtim_period_g)
        {
            dtim_counter = 0;
            if(uu_wlan_is_sta_in_sleep())
                uu_wlan_set_sta_awake();

            uu_wlan_lmac_dtim_bcn_received_g  = 0;
            
            if (!uu_wlan_ps_mode_r)
            {
                uu_wlan_cap_dtim_timer_stop();
            }
        }
        dtim_counter++;
    }

    uu_ktimer_g.expires = jiffies + msecs_to_jiffies(1);
    add_timer(&uu_ktimer_g);
}
#endif

uu_int32 uu_wlan_cap_slot_timer_start(uu_void)
{
#ifdef UU_HR_TIMER
    ktime_t ktime;

    exit_flag_g = 0;
    ktime = ktime_set(0, MicroS_TO_NS(UU_WLAN_CAP_SLOT_INTERVEL));

    hrtimer_init( &uu_wlan_cap_slot_timer_g, CLOCK_MONOTONIC, HRTIMER_MODE_REL );

    uu_wlan_cap_slot_timer_g.function = &uu_wlan_cap_slot_timer_cbk;

    hrtimer_start( &uu_wlan_cap_slot_timer_g, ktime, HRTIMER_MODE_REL ); 
    return UU_SUCCESS;
#endif

#ifdef UU_KTIMER
    /* ktimer function */
    init_timer(&uu_ktimer_g);
    uu_ktimer_g.expires = jiffies + msecs_to_jiffies(1);
    uu_ktimer_g.data = 0;
    uu_ktimer_g.function = uu_wlan_cap_ktimer_cbk;
    add_timer(&uu_ktimer_g);
    return UU_SUCCESS;
#endif
}

uu_int32 uu_wlan_cap_slot_timer_stop(uu_void)
{
#ifdef UU_HR_TIMER
    int ret;

    exit_flag_g = 1;
    ret = hrtimer_cancel( &uu_wlan_cap_slot_timer_g );
    if (ret) 
    {
        UU_WLAN_LOG_DEBUG(("The Slot timer was still in use...\n"));
    }
    return UU_SUCCESS;
#endif

#ifdef UU_KTIMER
    uu_int32 ret;
    ret = del_timer_sync(&uu_ktimer_g);
    return UU_SUCCESS;
#endif
}


/** Starts the DTIM timer.
 * Called in uu_wlan_ps_trace_beacon function when a PS STA receives first DTIM beacon.
 * Timer sets for DTIM period taken from the DTIM beacon,after timer expiry it calls uu_wlan_dtimtimer_cbk function.
 */
uu_int32 uu_wlan_cap_dtim_timer_start(uu_void)
{
    uu_wlan_run_dtim_timer_g = UU_TRUE;
    return UU_SUCCESS;
}


/** Stops the DTIM timer.
 * Called in uu_wlan_dtimtimer_cbk function when PS is deconfigured for the STA.
 * Also called in uu_wlan_lmac_shutdown function for removing timer before LMAC shutdown.
 */
uu_int32 uu_wlan_cap_dtim_timer_stop(uu_void)
{
    uu_wlan_run_dtim_timer_g = UU_FALSE;
    return UU_SUCCESS;
}

/* EOF */

