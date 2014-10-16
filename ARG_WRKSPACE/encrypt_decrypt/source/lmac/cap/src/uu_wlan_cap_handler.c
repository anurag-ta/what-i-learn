/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_handler.c                                  **
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

#include <linux/kthread.h>
#include <linux/delay.h>

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_cap_slot_timer.h"
#include "uu_wlan_cap_sm.h"
#include "uu_wlan_cap_cm.h"
#include "uu_wlan_cap_init.h"
#include "uu_wlan_cap_handler.h"
#include "uu_wlan_phy_if.h"

#define MODULE_NAME  "UU_WLAN_CAP_THRD"

typedef struct uu_wlan_config_thrd_info
{
    struct task_struct *thread;
    int is_running;
    int stop;
} uu_wlan_cap_thrd_info_t;


/*
** Global variables.
*/
static uu_wlan_cap_thrd_info_t uu_wlan_cap_thrd_g;


static uu_int32 uu_wlan_cap_handler(uu_void)
{
    uu_wlan_cap_event_t ev;

    uu_int32 ret;
    if (uu_wlan_get_msg_from_CAP_Q(&ev) < 0)
    {
        return UU_FAILURE;
    }
    ret = uu_wlan_sm_run_for_event(&uu_wlan_cap_ctxt_g.sm_cntxt, (uu_uint32)ev.ev, (uu_uchar *)&ev);
    return ret;
}

static uu_void uu_wlan_cap_handler_thread(uu_void)
{
    uu_wlan_cap_thrd_g.is_running = 1;
    /* Starting SLOT Timer */
    uu_wlan_cap_slot_timer_start();
    UU_WLAN_LOG_DEBUG(("LMAC: jiffies at uu_wlan_cap_handler_thread %ld\n", jiffies));

    /* main loop */
    while(!uu_wlan_cap_thrd_g.stop)
    {
        if (uu_wlan_cap_handler() < 0)
        {
            //UU_WLAN_LOG_DEBUG(("LMAC: jiffies at uu_wlan_cap_handler if failed case%ld\n", jiffies));
            msleep(1); /*  No thread concept in RTL they will use signal, to run in C model 1 milli second is required */
        }
    }

    uu_wlan_cap_slot_timer_stop();

    //config_thrd_g.thread = NULL;
    uu_wlan_cap_thrd_g.is_running = 0;
}


uu_int32  uu_wlan_cap_handler_start(uu_void)
{
    memset(&uu_wlan_cap_thrd_g, 0, sizeof(uu_wlan_cap_thrd_info_t));
    /* start kernel thread */
    uu_wlan_cap_thrd_g.thread = kthread_run((uu_void *)uu_wlan_cap_handler_thread, NULL, MODULE_NAME);
    if (IS_ERR(uu_wlan_cap_thrd_g.thread))
    {
        UU_WLAN_LOG_ERROR((KERN_INFO MODULE_NAME": unable to start CAP kernel thread\n"));
        return -ENOMEM;
    }

    return 0;
}

uu_void  uu_wlan_cap_handler_stop(uu_void)
{
    //UU_WLAN_LOG_DEBUG(("LMAC: jiffies at uu_wlan_cap_handler_stop %ld\n", jiffies));
    if (uu_wlan_cap_thrd_g.thread!= NULL)
    {
        uu_wlan_cap_thrd_g.stop = 1;
        while (uu_wlan_cap_thrd_g.is_running != 0)
        {
            msleep(100);
        }
    }
}


/* EOF */

