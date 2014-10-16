/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_txop_sm.c                                  **
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
#include "uu_wlan_frame.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_cap_if.h"

#include "uu_wlan_cap_cm.h"
#include "uu_wlan_cap_init.h"
#include "uu_wlan_cap_sm.h"
#include "uu_wlan_cap_txop.h"
#include "uu_wlan_fwk_log.h"

uu_int32 uu_wlan_cap_supp_sm_reset()
{
    uu_wlan_cap_ctxt_g.ac_cntxt[uu_wlan_cap_ctxt_g.txop_owner_ac].tx_prog = 0;
    uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_INIT, UU_WLAN_CAP_EV_MAX);
    return UU_SUCCESS;
}

/* changing supp s/m, one state to  another state */
static uu_int32 uu_wlan_change_supp_sm_state(uu_wlan_sm_context_t *cntx, uu_int32 state, uu_int32 event)
{
    uu_int32 ret = UU_SUCCESS;
    uu_wlan_sm_state_info_t *current_state, *new_state;

    if ((!cntx) || (cntx->cur_state == UU_NULL))
    {
        UU_WLAN_LOG_ERROR((KERN_CRIT "Error- NULL state while changing state : %d\n", state));
        return UU_FAILURE;
    }

    current_state = cntx->cur_state;
    ret = current_state->exit_action((uu_uchar*)cntx, event);
    if (ret != UU_SUCCESS)
    {
        UU_WLAN_LOG_ERROR((KERN_CRIT "Error exiting state: %d (event: %d)\n", current_state->state, event));
        return UU_FAILURE;
    }
    new_state = (uu_wlan_sm_state_info_t *)uu_wlan_sm_get_state_info(cntx, state);
    ret = new_state->entry_action((uu_uchar*)cntx, event);
    if (ret != UU_SUCCESS)
    {
        /* TODO: What is this special case? */
        if(state == UU_WLAN_CAP_TXOP_STATE_SENDING_DATA)
        {
            cntx->cur_state = new_state;
        }
        UU_WLAN_LOG_DEBUG((KERN_CRIT "Error entry fun in supp sm state: %d\n", new_state->state));
        return UU_FAILURE;
    }
    cntx->cur_state = new_state;
    return UU_SUCCESS;
} /* uu_wlan_change_supp_sm_state */

/* running event on supplementary state machine */
uu_int32 uu_wlan_supp_sm_run_for_event(uu_wlan_sm_context_t *cntx, uu_int32 event ,uu_uchar *ev_data)
{
    uu_int32 ret = UU_SUCCESS;
    uu_wlan_sm_state_info_t *current_state;

    if ((!cntx) || (cntx->cur_state == UU_NULL))
    {
        UU_WLAN_LOG_DEBUG((KERN_CRIT "Error- NULL state in handling of event: %d\n", event));
        return UU_FAILURE;
    }

    current_state = cntx->cur_state;
    UU_WLAN_LOG_DEBUG(("LMAC: supp s/m current state is %d\n", current_state->state));
    /* running supplementary sm current state action function */
    ret = current_state->action((uu_uchar*)cntx, event, ev_data); 
    UU_WLAN_LOG_DEBUG(("LMAC: Supp state action ret value %x\n",ret));
    if((ret == SUPP_SM_TX_FAILURE) || (ret == SUPP_SM_TXOP_ERROR_RECOVERY) ||
        (ret == SUPP_SM_TXOP_NAV_UPDATE))
    {
        if(UU_WLAN_IEEE80211_QOS_MODE_R)
        {
            /* supp sm enters wait for txnav end state */
            //uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_WAIT_TXNAV_END, UU_WLAN_CAP_EV_INVALID);
            uu_wlan_change_supp_sm_state(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_WAIT_TXNAV_END, UU_WLAN_CAP_EV_MAX);
            UU_WLAN_LOG_DEBUG(("LMAC:changing to supp s/m new state UU_WLAN_CAP_TXOP_STATE_WAIT_TXNAV_END\n"));
            ret = SUPP_SM_STATE_CONTINUE;
        }
        else /* non-QoS */
        {
        }
    }
    else if(ret == SUPP_SM_START_SENDING)
    {
        /* supp sm enters sending data state */
        //if(uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_SENDING_DATA, UU_WLAN_CAP_EV_INVALID) == UU_SUCCESS)
        if(uu_wlan_change_supp_sm_state(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_SENDING_DATA, UU_WLAN_CAP_EV_MAX) == UU_SUCCESS)
        {
            UU_WLAN_LOG_DEBUG(("LMAC:changing to supp s/m new state UU_WLAN_CAP_TXOP_STATE_SENDING_DATA\n"));
            ret = SUPP_SM_STATE_CONTINUE;
        }
        else
        {
            ret = SUPP_SM_CONTEND;
        }
    }
    else if(ret == SUPP_SM_WAIT_FOR_RX)
    {
        /* supp sm enters wait for rx state */
        //uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX, UU_WLAN_CAP_EV_INVALID);
        uu_wlan_change_supp_sm_state(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX, UU_WLAN_CAP_EV_MAX);
        UU_WLAN_LOG_DEBUG(("LMAC:changing to supp s/m new state UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX\n"));
        ret = SUPP_SM_STATE_CONTINUE;
    }

    return ret;
} /* uu_wlan_supp_sm_run_for_event */


/* EOF */

