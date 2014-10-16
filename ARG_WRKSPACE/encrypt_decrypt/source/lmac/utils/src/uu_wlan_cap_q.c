/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_q.c                                        **
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

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_fwk_lock.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_msgq.h"


/* Assuming multiple-writers, and only 1 reader thread */
static uu_wlan_cap_event_t  cap_q_g[UU_WLAN_CAP_Q_SIZE];
static uu_int32 cap_q_rindex_g;
static uu_int32 cap_q_windex_g;

extern uu_wlan_spin_lock_type_t uu_wlan_cap_Q_lock_g;


uu_int32 uu_wlan_get_msg_from_CAP_Q(uu_wlan_cap_event_t *event)
{
    //UU_WLAN_LOG_DEBUG(("LMAC:UU_WLAN_GET_MSG_FROM_CAP_Q\n"));
    /* Queue is empty */
    if (cap_q_rindex_g == cap_q_windex_g)
    {
        //UU_WLAN_LOG_DEBUG(("Queue is empty\n"));
        return UU_ERR_MQ_EMPTY;
    }

    *event = cap_q_g[cap_q_rindex_g];
    /*
     * Adding spinlock even with 1 reader, to cater to Q-full check.
     * Alternative to using lock is, to leave 1 entry in the array empty.
     */
    UU_WLAN_SPIN_LOCK(&uu_wlan_cap_Q_lock_g);
    cap_q_rindex_g++;
    cap_q_rindex_g = cap_q_rindex_g % UU_WLAN_CAP_Q_SIZE;
    UU_WLAN_SPIN_UNLOCK(&uu_wlan_cap_Q_lock_g);
    return UU_SUCCESS;
} /* uu_wlan_get_msg_from_CAP_Q */


uu_int32 uu_wlan_put_msg_in_CAP_Q(uu_wlan_cap_event_type_t ev_type, uu_uchar *data_p, uu_int32 len)
{
    uu_wlan_cap_event_t  event;
    uu_int32  windex;
    //UU_WLAN_LOG_DEBUG((" LMAC: uu_wlan_put_msg_in_CAP_Q, ev_type %d, and len %d\n", ev_type, len));

    switch(ev_type)
    {
        case UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL:
            {
                if ((!data_p) || (len <=0) || (len > sizeof(event.u.cca_status)))
                {
                    UU_WLAN_LOG_ERROR(("ERROR: No/Invalid data for CAP ev %d", ev_type));
                    return UU_FAILURE;
                }
                if(len >= 1)
                {
                    event.u.cca_status.status = data_p[0];
                }
                if(len == 2)
                {
                    event.u.cca_status.channel_list = data_p[1];
                }
                UU_WLAN_COPY_BYTES(&event.u.cca_status.status, data_p, len);
            }
            break;
        case UU_WLAN_CAP_EV_TX_READY:
            {
                if ((!data_p) || (len <=0) || (len != sizeof(event.u.tx_ready.ac)))
                {
                    UU_WLAN_LOG_ERROR(("ERROR: Nodata for CAP ev %d\n", ev_type));
                    return UU_FAILURE;
                }

                UU_WLAN_COPY_BYTES(&event.u.tx_ready.ac, data_p, len);
            }
            break;
        case UU_WLAN_CAP_EV_RX_START_IND:
            {
                if ((!data_p) || (len <=0) || (len != sizeof(event.u.rx_vec.rxvec)))
                {
                    UU_WLAN_LOG_ERROR(("ERROR: Nodata for CAP ev %d and sizeof rx_vec is %u and len is %d\n", ev_type, sizeof(event.u.rx_vec.rxvec), len));
                    return UU_FAILURE;
                }
                UU_WLAN_COPY_BYTES(&event.u.rx_vec.rxvec, data_p, len);
            }
            break;
        case UU_WLAN_CAP_EV_DATA_INDICATION:
            {
                if ((!data_p) || (len <=0) || (len > UU_WLAN_MAX_AMPDU_SZ))
                {
                    UU_WLAN_LOG_ERROR(("ERROR: Nodata for CAP ev %d\n", ev_type));
                    return UU_FAILURE;
                }

                event.u.rx_data_ind.frame_p = (uu_uchar *)uu_wlan_alloc_memory(len);
                if(event.u.rx_data_ind.frame_p == UU_NULL)
                {
                    return UU_FAILURE;
                }
                UU_WLAN_COPY_BYTES(event.u.rx_data_ind.frame_p, data_p, len);
                event.u.rx_data_ind.frame_len = len;
            }
            break;
        /* No data expected for other events */
        case UU_WLAN_CAP_EV_MAC_RESET:
        case UU_WLAN_CAP_EV_TIMER_TICK: 
        case UU_WLAN_CAP_EV_TX_START_CONFIRM:
        case UU_WLAN_CAP_EV_TX_DATA_CONFIRM:
        case UU_WLAN_CAP_EV_TX_END_CONFIRM:
        case UU_WLAN_CAP_EV_RX_END_IND:
        case UU_WLAN_CAP_EV_PHY_RX_ERR:
        case UU_WLAN_CAP_EV_CCA_RESET_CONFIRM:
        case UU_WLAN_CAP_EV_TXOP_TX_START:
        default: /* No data expected for other events */
            {
            }
            break;
    } /* switch */

    event.ev = ev_type;
    if (event.ev != UU_WLAN_CAP_EV_TIMER_TICK) /* Log for other than timer-tick */
    {
       UU_WLAN_LOG_DEBUG(("LMAC:put msg in cap q %x\n", event.ev));
    }

    UU_WLAN_SPIN_LOCK(&uu_wlan_cap_Q_lock_g);
    windex = ((cap_q_windex_g + 1) % UU_WLAN_CAP_Q_SIZE); 
    /* checking for Queue is Full or not */
    if (windex == cap_q_rindex_g)
    {
        UU_WLAN_SPIN_UNLOCK(&uu_wlan_cap_Q_lock_g);
        UU_WLAN_LOG_DEBUG(("Queue is full\n"));
        if (ev_type == UU_WLAN_CAP_EV_DATA_INDICATION)
            uu_wlan_free_memory(event.u.rx_data_ind.frame_p);
        return UU_ERR_MQ_FULL;
    }
    cap_q_g[cap_q_windex_g] = event;
    cap_q_windex_g = windex;
    UU_WLAN_SPIN_UNLOCK(&uu_wlan_cap_Q_lock_g);

    return UU_SUCCESS;
} /* uu_wlan_put_msg_in_CAP_Q */


/* EOF */

