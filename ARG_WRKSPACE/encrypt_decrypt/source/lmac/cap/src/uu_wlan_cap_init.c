/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_init.c                                     **
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
#include "uu_wlan_msgq.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_cap_slot_timer.h"
#include "uu_wlan_cap_sm.h"
#include "uu_wlan_cap_cm.h"
#include "uu_wlan_cap_txop.h"
#include "uu_wlan_cap_init.h"
#include "uu_wlan_cap_handler.h"
#include "uu_wlan_reg.h"


uu_int32 uu_wlan_cap_init(uu_void)
{

    /* Add states for primary state machine */
    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.sm_cntxt, UU_WLAN_CAP_STATE_IDLE, 
            idle_entry, (action_t)idle_action, idle_exit, NULL);
    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.sm_cntxt, UU_WLAN_CAP_STATE_EIFS, 
            eifs_entry, (action_t)eifs_action, eifs_exit, NULL);

    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.sm_cntxt, UU_WLAN_CAP_STATE_CONTEND, 
            contend_entry, (action_t)contend_action, contend_exit, NULL);
    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.sm_cntxt, UU_WLAN_CAP_STATE_TXOP, 
            txop_entry, (action_t)txop_action, txop_exit, NULL);
    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.sm_cntxt, UU_WLAN_CAP_STATE_RECEIVING, 
            receiving_entry, (action_t)receiving_action, receiving_exit, NULL);

    /* Add states for suplimentary state machine */
    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_INIT, 
            txop_init_entry, (action_t)txop_init_action, txop_init_exit, NULL);
    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_SENDING_DATA, 
            sending_data_entry, (action_t)sending_data_action, sending_data_exit, NULL);
    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX, 
            wait_for_rx_entry, (action_t)wait_for_rx_action, wait_for_rx_exit, NULL);
    uu_wlan_sm_add_state(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, UU_WLAN_CAP_TXOP_STATE_WAIT_TXNAV_END, 
            wait_for_tx_nav_end_entry, (action_t)wait_for_tx_nav_end_action, wait_for_tx_nav_end_exit, NULL);

    /* Start State machine for primary sm */
    uu_wlan_sm_start(&uu_wlan_cap_ctxt_g.sm_cntxt, UU_WLAN_CAP_STATE_IDLE, UU_WLAN_CAP_EV_MAX);

    uu_wlan_cap_handler_start();

    return UU_SUCCESS;
} /* uu_wlan_cap_init */


uu_void uu_wlan_cap_shutdown(uu_void)
{
    uu_wlan_cap_handler_stop();
}

/* EOF */

