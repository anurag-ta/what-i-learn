/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_cm.c                                       **
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
#include <linux/random.h>
#include <linux/version.h>
#else
#include <stdlib.h>
#endif

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_msgq.h"
#include "uu_wlan_utils.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_cp_if.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_rx_if.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_cap_sm.h"
#include "uu_wlan_cap_init.h"
#include "uu_wlan_cap_cm.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_cap_handler.h"
#include "uu_wlan_cap_slot_timer.h"
#include "uu_wlan_cap_txop.h"
#include "uu_wlan_rate.h"
#include "uu_wlan_ps.h"

uu_wlan_rx_handler_return_value_t uu_wlan_cap_rx_status_value_g;

static uu_int32 idle_phy_energy_ind(uu_uchar* sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 idle_tx_ready_pkt(uu_uchar* sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_phy_error(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_phy_energy_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_tx_ready_pkt(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_rx_start_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_rx_data_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_rx_end_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_cca_reset_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_tx_start_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_tx_data_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 receiving_tx_end_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 contend_tx_ready_pkt(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 contend_phy_energy_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 contend_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);

static uu_int32 eifs_tx_ready_pkt(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 eifs_phy_energy_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 eifs_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
static uu_int32 eifs_cca_reset_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);


static action_t cm_idle_action_g[] = {        //UU_NULL,/* UU_WLAN_CAP_EV_INVALID  */
                        UU_NULL, /* UU_WLAN_CAP_EV_MAC_RESET */
                        UU_NULL, /* UU_WLAN_CAP_EV_TIMER_TICK */
                        idle_tx_ready_pkt, /* UU_WLAN_CAP_EV_TX_READY */
                        UU_NULL, /* UU_WLAN_CAP_EV_TX_START_CONFIRM */
                        UU_NULL, /*  UU_WLAN_CAP_EV_TX_DATA_CONFIRM */
                        UU_NULL, /* UU_WLAN_CAP_EV_TX_END_CONFIRM */
                        idle_phy_energy_ind, /* UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL */ 
                        UU_NULL, /* UU_WLAN_CAP_EV_RX_START_IND */
                        UU_NULL, /* UU_WLAN_CAP_EV_DATA_INDICATION */
                        UU_NULL, /* UU_WLAN_CAP_EV_RX_END_IND */
                        UU_NULL ,/* UU_WLAN_CAP_EV_PHY_RX_ERR */
                        UU_NULL, /* UU_WLAN_CAP_EV_CCA_RESET_CONFIRM */
                        UU_NULL, /* UU_WLAN_CAP_TXOP_TX_START */
                    };

static action_t cm_receiving_action_g[] = {     //UU_NULL,/* UU_WLAN_CAP_EV_INVALID  */
                        UU_NULL, /* UU_WLAN_CAP_EV_MAC_RESET */
                        receiving_timer_tick, /* UU_WLAN_CAP_EV_TIMER_TICK */
                        receiving_tx_ready_pkt, /* UU_WLAN_CAP_EV_TX_READY */
                        receiving_tx_start_confirm, /* UU_WLAN_CAP_EV_TX_START_CONFIRM */
                        receiving_tx_data_confirm, /*  UU_WLAN_CAP_EV_TX_DATA_CONFIRM */
                        receiving_tx_end_confirm, /* UU_WLAN_CAP_EV_TX_END_CONFIRM */
                        receiving_phy_energy_ind, /* UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL */ 
                        receiving_rx_start_ind, /* UU_WLAN_CAP_EV_RX_START_IND */
                        receiving_rx_data_ind, /* UU_WLAN_CAP_EV_DATA_INDICATION */
                        receiving_rx_end_ind, /* UU_WLAN_CAP_EV_RX_END_IND */
                        receiving_phy_error,/* UU_WLAN_CAP_EV_PHY_RX_ERR */
                        receiving_cca_reset_confirm, /* UU_WLAN_CAP_EV_CCA_RESET_CONFIRM */
                        UU_NULL, /* UU_WLAN_CAP_TXOP_TX_START */
                    };

static action_t cm_contend_action_g[] = {    //UU_NULL,/* UU_WLAN_CAP_EV_INVALID  */
                        UU_NULL, /* UU_WLAN_CAP_EV_MAC_RESET */
                        contend_timer_tick, /* UU_WLAN_CAP_EV_TIMER_TICK */
                        contend_tx_ready_pkt, /* UU_WLAN_CAP_EV_TX_READY */
                        UU_NULL, /* UU_WLAN_CAP_EV_TX_START_CONFIRM */
                        UU_NULL, /*  UU_WLAN_CAP_EV_TX_DATA_CONFIRM */
                        UU_NULL, /* UU_WLAN_CAP_EV_TX_END_CONFIRM */
                        contend_phy_energy_ind, /* UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL */ 
                        UU_NULL, /* UU_WLAN_CAP_EV_RX_START_IND */
                        UU_NULL, /* UU_WLAN_CAP_EV_DATA_INDICATION */
                        UU_NULL, /* UU_WLAN_CAP_EV_RX_END_IND */
                        UU_NULL ,/* UU_WLAN_CAP_EV_PHY_RX_ERR */
                        UU_NULL, /* UU_WLAN_CAP_EV_CCA_RESET_CONFIRM */
                        UU_NULL, /* UU_WLAN_CAP_TXOP_TX_START */
                    };

/* Action for 'TXOP': Invocation of TXOP sub-state machine & handling its return value */


static action_t cm_eifs_action_g[] = {       //UU_NULL, /* UU_WLAN_CAP_EV_INVALID  */
                                UU_NULL, /* UU_WLAN_CAP_EV_MAC_RESET */
                                eifs_timer_tick, /* UU_WLAN_CAP_EV_TIMER_TICK */
                                eifs_tx_ready_pkt, /* UU_WLAN_CAP_EV_TX_READY */
                                UU_NULL, /* UU_WLAN_CAP_EV_TX_START_CONFIRM */
                                UU_NULL, /*  UU_WLAN_CAP_EV_TX_DATA_CONFIRM */
                                UU_NULL, /* UU_WLAN_CAP_EV_TX_END_CONFIRM */
                                eifs_phy_energy_ind, /* UU_WLAN_CAP_EV_PHY_ENERGY_LEVEL */ 
                                UU_NULL, /* UU_WLAN_CAP_EV_RX_START_IND */
                                UU_NULL, /* UU_WLAN_CAP_EV_DATA_INDICATION */
                                UU_NULL, /* UU_WLAN_CAP_EV_RX_END_IND */
                                UU_NULL ,/* UU_WLAN_CAP_EV_PHY_RX_ERR */
                                eifs_cca_reset_confirm, /* UU_WLAN_CAP_EV_CCA_RESET_CONFIRM */
                                UU_NULL, /* UU_WLAN_CAP_TXOP_TX_START */
                            };


static uu_uint32 _random_num(uu_uint16 limit)
{
    uu_uint32  num;
    do {
#ifdef __KERNEL__
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,8,0)
	num = limit & random32();
	#else
	num = limit & prandom_u32(); /* random32() is changed to prandom_u32 in latest linux kernels */
	#endif
#else
        num = limit & random();
#endif
    } while (num == 0);
    return num;
}


/* IDLE STATE */

uu_int32 idle_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    uu_int32 i;

    if(UU_WLAN_IEEE80211_QOS_MODE_R)
    {
        for(i = 0; i < UU_WLAN_MAX_QID; i++)
        {
            uu_wlan_cap_ctxt_g.ac_cntxt[i].cw_val = UU_WLAN_CW_MIN_VALUE_R(i);
        }
    }
    else /* Non-QoS */
    {
        uu_wlan_cap_ctxt_g.ac_cntxt[0].cw_val = UU_WLAN_CWMIN_VALUE_R;
#ifdef UU_WLAN_BQID /* TODO: Test this functionality (Separate beacon Q in non-QoS) */
        uu_wlan_cap_ctxt_g.ac_cntxt[UU_WLAN_BEACON_QID].cw_val = UU_WLAN_CW_MIN_VALUE_R(UU_WLAN_BEACON_QID);
#endif
    }

    uu_wlan_ps_handle_idle_entry();
    return UU_SUCCESS;
}

uu_int32 idle_exit(uu_uchar* sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

static uu_int32 idle_phy_energy_ind(uu_uchar* sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;
    uu_wlan_cap_ctxt_g.cca_status = ev_info->u.cca_status.status;

    /* power save related */
    /*
     * Check is added to take care of race-condition in C model.
     * In reality, there is no such case like receiving data in sleep!!!
     */
    if(uu_wlan_is_sta_in_sleep())
    {
        return UU_WLAN_CAP_STATE_IDLE;
    }

    /* energy level indication is high */
    if(ev_info->u.cca_status.status)
    {
        uu_wlan_cap_ctxt_g.updated_channel_list = (uu_wlan_busy_channel_list_t)ev_info->u.cca_status.channel_list;

#ifdef UU_WLAN_DFS
        if (msrment_stats_g == UU_WLAN_CHANNEL_LOAD_MSR)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Channel Load value in idle_phy_energy_ind: %d\n", chan_busy_dur_g);
#endif
            chan_busy_dur_g ++;
        }
#endif
    }
    else
    {
        uu_wlan_cap_ctxt_g.updated_channel_list = UU_WLAN_CH_IDLE;
    }

    /* energy level indication is high and primary is free */
    if(uu_wlan_cap_ctxt_g.updated_channel_list == UU_WLAN_BUSY_CH_PRIMARY)
    {
        return UU_WLAN_CAP_STATE_RECEIVING;
    }

    return UU_WLAN_CAP_STATE_IDLE;
}

static uu_int32 idle_tx_ready_pkt(uu_uchar* sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = UU_WLAN_CAP_STATE_CONTEND;
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;

    /* power save related */
    uu_wlan_ps_idle_exit();

    uu_wlan_cap_ctxt_g.ac_cntxt[ev_info->u.tx_ready.ac].tx_ready = 1;
    return ret;
}


uu_int32 idle_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = UU_WLAN_CAP_STATE_IDLE;

    if (cm_idle_action_g[event] != UU_NULL)
    {
        ret = (*cm_idle_action_g[event])(sm_cntx, event, ev_data);
    }
    else
    {
        ret = UU_WLAN_CAP_STATE_IDLE;
    }
    return ret;
}


/* RECEIVING state */
uu_int32 receiving_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

uu_int32 receiving_exit(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

static uu_int32 receiving_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = UU_WLAN_CAP_STATE_RECEIVING;
    if(uu_wlan_cap_ctxt_g.nav_val > 0)
    {
        uu_wlan_cap_ctxt_g.nav_val--;

#ifdef UU_WLAN_DFS
        if (msrment_stats_g == UU_WLAN_CHANNEL_LOAD_MSR)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Channel Load value in receiving_timer_tick: %d\n",chan_busy_dur_g);
#endif
            chan_busy_dur_g ++;
        }
#endif

        /* nav turns to 0 and cca_status is low */
        if(!uu_wlan_cap_ctxt_g.nav_val)
        {
            /* SEND CCA_RESET REQUEST TO PHY */
            uu_wlan_phy_ccareset_req();
            return UU_WLAN_CAP_STATE_RECEIVING;
        }
    }

    if((UU_WLAN_GET_FC0_VALUE(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu) != (IEEE80211_FC0_SUBTYPE_CTS | IEEE80211_FC0_TYPE_CTL)) && 
            !(isLocalMacAddr(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu + UU_RA_OFFSET)) && (uu_wlan_cap_ctxt_g.nav_reset_timer != 0))
    {
        if(uu_wlan_cap_ctxt_g.nav_reset_timer > 0)
        {
            uu_wlan_cap_ctxt_g.nav_reset_timer--;
            if(!uu_wlan_cap_ctxt_g.nav_reset_timer)
            {
                /* resetting nav value */
                uu_wlan_cap_ctxt_g.nav_val = 0;
                uu_wlan_cap_ctxt_g.nav_reset_timer = 0;
                /* SEND CCA_RESET REQUEST TO PHY */
                uu_wlan_phy_ccareset_req();
            }
        }
    }

    return ret;
}

static uu_int32 receiving_cca_reset_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
	/* Resetting the Channel list as IDLE */
    uu_wlan_cap_ctxt_g.updated_channel_list = UU_WLAN_CH_IDLE;
    if (uu_wlan_is_tx_ready_for_any_AC_Q())
    {
        return UU_WLAN_CAP_STATE_CONTEND;
    }
    else
    {
        return UU_WLAN_CAP_STATE_IDLE;
    }
}


static uu_int32 receiving_phy_error(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;

    /* handover rx_end indication to Rx module */
    uu_wlan_rx_handle_phy_rxend(ev_info->u.rx_end_ind.ind);
    return UU_WLAN_CAP_STATE_EIFS;
}

static uu_int32 receiving_phy_energy_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;
    uu_wlan_cap_ctxt_g.cca_status = ev_info->u.cca_status.status;
    if(ev_info->u.cca_status.status)
    {
        uu_wlan_cap_ctxt_g.updated_channel_list = (uu_wlan_busy_channel_list_t)ev_info->u.cca_status.channel_list;

#ifdef UU_WLAN_DFS
        if (msrment_stats_g == UU_WLAN_CHANNEL_LOAD_MSR)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Channel Load value in receiving_phy_energy_ind: %d\n", chan_busy_dur_g);
#endif
            chan_busy_dur_g ++;
        }
#endif
    }
    else
    {
        uu_wlan_cap_ctxt_g.updated_channel_list = UU_WLAN_CH_IDLE;
        if(!uu_wlan_cap_ctxt_g.nav_val)
        {
            if (uu_wlan_cap_rx_status_value_g == UU_WLAN_RX_HANDLER_SEND_RESP)
            {
                return UU_WLAN_CAP_STATE_RECEIVING;
            }
            else if (uu_wlan_is_tx_ready_for_any_AC_Q())
            {
                return UU_WLAN_CAP_STATE_CONTEND;
            }
            else
            {
                return UU_WLAN_CAP_STATE_IDLE;
            }
        }
    }
    return UU_WLAN_CAP_STATE_RECEIVING;
}

static uu_int32 receiving_tx_ready_pkt(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;

    uu_wlan_cap_ctxt_g.ac_cntxt[ev_info->u.tx_ready.ac].tx_ready = 1;
    return UU_WLAN_CAP_STATE_RECEIVING;
}

static uu_int32 receiving_rx_start_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;

    /* Retain Rx vector details, for future reference */
    uu_wlan_cap_retain_rx_vec_info(&ev_info->u.rx_vec.rxvec);

     /* stop the nav reset timer on rx-start indication */
    uu_wlan_cap_ctxt_g.nav_reset_timer = 0;
    return UU_WLAN_CAP_STATE_RECEIVING;
}

static uu_int32 receiving_rx_data_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;
    uu_uint32 receive_frame_len;

    /* Retain Rx frame header details, for future reference */
    receive_frame_len = uu_wlan_cap_retain_rx_frame_hdr(ev_info->u.rx_data_ind.frame_p);

    /* free the memory of ev rx_data indication */
    uu_wlan_free_memory(ev_info->u.rx_data_ind.frame_p);
    return UU_WLAN_CAP_STATE_RECEIVING;
}


static uu_int32 receiving_rx_end_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_rx_handler_return_value_t ret = uu_wlan_cap_rx_status_value_g;
    uu_int16 duration;

    UU_WLAN_LOG_DEBUG((" LMAC: receiving rx end ind and Rx_event handler return value %x\n", ret));

    /* for resetting nav */
    if((UU_WLAN_GET_FC0_VALUE(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu) == (IEEE80211_FC0_SUBTYPE_RTS | IEEE80211_FC0_TYPE_CTL)) && 
                !(isLocalMacAddr(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu + UU_RA_OFFSET)))
    {
#if 0
        uu_wlan_cap_ctxt_g.nav_reset_timer = (UU_SLOT_TIMER_VALUE_R << 1) + UU_PHY_RX_START_DELAY_R + (UU_WLAN_SIFS_TIMER_VALUE_R << 1) + 
                        ((UU_CTS_FRAME_LEN)/uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate);
#else
        uu_wlan_cap_ctxt_g.nav_reset_timer = (UU_SLOT_TIMER_VALUE_R << 1) + UU_PHY_RX_START_DELAY_R + (UU_WLAN_SIFS_TIMER_VALUE_R << 1);
        if (uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate == UU_HW_RATE_6M)
        {
            /* cts frame length is 14, so 14/6 ceiling to 3 */
            uu_wlan_cap_ctxt_g.nav_reset_timer += 3;
        }
        else if (uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate <= UU_HW_RATE_12M)
        {
            /* cts frame length is 14, so 14/(9 or 12) ceiling to 2 */
            uu_wlan_cap_ctxt_g.nav_reset_timer += 2;
        }
        else
        {
            /* cts frame length is 14, so 14/(>12) ceiling to 2 */
            uu_wlan_cap_ctxt_g.nav_reset_timer += 1;
        }
#endif
    }

    if(ret == UU_WLAN_RX_HANDLER_FRAME_ERROR)
    {
        return UU_WLAN_CAP_STATE_EIFS;
    }
    else if(ret == UU_WLAN_RX_HANDLER_NAV_UPDATE)
    {
        /* Duration calculation for PS-poll frame */
        if(IEEE80211_IS_FC0_PSPOLL(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu) &&
             uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate)
        {
            duration = UU_WLAN_GET_PSPOLL_FRAME_DURATION(uu_wlan_cap_ctxt_g.rx_frame_info.rxvec_L_datarate);
            uu_wlan_cap_ctxt_g.rx_frame_info.mpdu[2] = (uu_uchar)(duration & 0x00ff);
            uu_wlan_cap_ctxt_g.rx_frame_info.mpdu[3] = (uu_uchar)(duration >> 8);
        }
        if(UU_WLAN_GET_FRAME_DURATION(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu) > uu_wlan_cap_ctxt_g.nav_val)
        {
            uu_wlan_cap_ctxt_g.nav_val = UU_WLAN_GET_FRAME_DURATION(uu_wlan_cap_ctxt_g.rx_frame_info.mpdu);
        }
        return UU_WLAN_CAP_STATE_RECEIVING;
    }
    else if(ret == UU_WLAN_RX_HANDLER_SEND_RESP)
    {
        uu_wlan_tx_frame_info_t *frame_info;
        frame_info = &tx_ctl_frame_info_g;
        /*if(frame_info == UU_NULL)
        {
            return UU_WLAN_CAP_STATE_RECEIVING;
        }*/

        /** CTS will be sent only when the NAV is 0.
         * TODO: And the medium was idle for PIFS period prior to receiving RTS.
         * Refer section: 9.3.26 (IEEE P802.11acTM/D6.0)
         */
        if (IEEE80211_IS_FC0_CTS(frame_info->mpdu))
        {
           if (uu_wlan_cap_ctxt_g.nav_val)
           {
                return UU_WLAN_CAP_STATE_RECEIVING;
           }
        }

        uu_wlan_cap_retain_tx_frame_info(frame_info);
        /* send tx_start.req to phy */
        uu_wlan_phy_txstart_req((uu_uchar *)&frame_info->txvec, sizeof(uu_wlan_tx_vector_t));
        return UU_WLAN_CAP_STATE_RECEIVING;
    }
    else
    {
        if (uu_wlan_is_tx_ready_for_any_AC_Q())
        {
            return UU_WLAN_CAP_STATE_CONTEND;
        }
        else
        {
            return UU_WLAN_CAP_STATE_IDLE;
        }
    }
}

static uu_int32 receiving_tx_start_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = UU_WLAN_CAP_STATE_RECEIVING;
    /* send tx_data.req to phy */
    uu_uchar phy_data_cfm = 0;
    uu_uint32 curr_pos = 0;
    do
    {
        uu_wlan_phy_data_req(tx_ctl_frame_info_g.mpdu[curr_pos], &phy_data_cfm);//frame_info->frameInfo.framelen);
        curr_pos++;
    } while ((phy_data_cfm != 0) && (curr_pos < tx_ctl_frame_info_g.frameInfo.framelen));
    uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TX_DATA_CONFIRM, UU_NULL, 0);
    return ret;
}
static uu_int32 receiving_tx_data_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = UU_WLAN_CAP_STATE_RECEIVING;
    /* send tx end req to phy */
    uu_wlan_phy_txend_req();
    return ret;
}
static uu_int32 receiving_tx_end_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    if(!uu_wlan_cap_ctxt_g.nav_val)
    {
        if (uu_wlan_is_tx_ready_for_any_AC_Q())
        {
            return UU_WLAN_CAP_STATE_CONTEND;
        }
        else
        {
            return UU_WLAN_CAP_STATE_IDLE;
        }
    }
    return UU_WLAN_CAP_STATE_RECEIVING;
}

uu_int32 receiving_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret;

    if (cm_receiving_action_g[event] != UU_NULL)
    {
        ret = (*cm_receiving_action_g[event])(sm_cntx, event, ev_data);
    }
    else
    {
        ret = UU_WLAN_CAP_STATE_RECEIVING;
    }
    return ret;
}

/* CONTEND state */
uu_int32 contend_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    uu_int32 i;

    if(UU_WLAN_IEEE80211_QOS_MODE_R)
    {
        for(i = 0; i < UU_WLAN_MAX_QID; i++)
        {
            /* get aifs value from register */
            uu_wlan_cap_ctxt_g.ac_cntxt[i].aifs_val = UU_WLAN_AIFS_VALUE_R(i);
        }
    }
    else /* Non-QoS */
    {
        uu_wlan_cap_ctxt_g.ac_cntxt[0].aifs_val = UU_WLAN_DIFS_VALUE_R;
#ifdef UU_WLAN_BQID /* TODO: Test this functionality (Beacon Q in non-QoS) */
        uu_wlan_cap_ctxt_g.ac_cntxt[UU_WLAN_BEACON_QID].aifs_val = UU_WLAN_AIFS_VALUE_R(UU_WLAN_BEACON_QID);
#endif
    }
    /* intializing slot timer reference value to reg slot timer value */
    uu_wlan_cap_ctxt_g.slot_timer_countdown = (uu_uint8)UU_SLOT_TIMER_VALUE_R;
    return UU_SUCCESS;
}

uu_int32 contend_exit(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

static uu_int32 contend_tx_ready_pkt(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;

    uu_wlan_cap_ctxt_g.ac_cntxt[ev_info->u.tx_ready.ac].tx_ready = 1;
    return UU_WLAN_CAP_STATE_CONTEND;
}

static uu_int32 contend_phy_energy_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;

    uu_wlan_cap_ctxt_g.cca_status = ev_info->u.cca_status.status;
    if(ev_info->u.cca_status.status)
    {
        uu_wlan_cap_ctxt_g.updated_channel_list = (uu_wlan_busy_channel_list_t)ev_info->u.cca_status.channel_list;

#ifdef UU_WLAN_DFS
        if (msrment_stats_g == UU_WLAN_CHANNEL_LOAD_MSR)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Channel Load value in contend_phy_energy_ind: %d\n",chan_busy_dur_g);
#endif
            chan_busy_dur_g ++;
        }
#endif
    }
    else
    {
        uu_wlan_cap_ctxt_g.updated_channel_list = UU_WLAN_CH_IDLE;
    }
    /* energy level indication is high and primary is free */
    if(uu_wlan_cap_ctxt_g.updated_channel_list == UU_WLAN_BUSY_CH_PRIMARY)
    {
        return UU_WLAN_CAP_STATE_RECEIVING;
    }
    return UU_WLAN_CAP_STATE_CONTEND;
}


static uu_int32 contend_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32  ret = UU_WLAN_CAP_STATE_IDLE;
    uu_int32  i;
    uu_int32  selected_ac = -1;

    /* decrementing the slot timer reference value  */
    uu_wlan_cap_ctxt_g.slot_timer_countdown--;
    if (uu_wlan_cap_ctxt_g.slot_timer_countdown)
    {
        return UU_WLAN_CAP_STATE_CONTEND;
    }

    /* slot_timer_countdown turned to 0 */
    /* Reseting slot timer usec count down to slot timer val */
    uu_wlan_cap_ctxt_g.slot_timer_countdown = (uu_uint8)UU_SLOT_TIMER_VALUE_R;


    /*
     * Give priority to higher AC, in case of internal collision.
     * Non-QOS mode always uses AC 0 context only.
     */
    for(i = UU_WLAN_BEACON_QID; i >= UU_WLAN_AC_BE; i--)
    {
#if 0 /* Now triggering beacon in UMAC */
		/* TODO: This function NEED NOT be implemented in RTL.
		 * This function is to avoid the race-around condition.
		 * After TBTT, after giving trigger to UMAC for getting Beacon frame,
		 * If we receive Beacon from other IBSS Station of the same BSSID before
		 * getting Beacon from UMAC.
		 * So, in this situation, we may get Beacon frame in Beacon QID after calling
		 * the function uu_wlan_cap_stop_beacon_tx from rx_handler.
		 * So, we are calling this function again if we receive Beacon from UMAC
		 * since we should not send Beacon in this TBTT, so we are clearing Beacon frame
		 * from Beacon QID.
		 *
		 * If beacon is present in the Beacon QID and we should not send Beacon
 		 * for the current TBTT, then we will clear Beacon frame from the buffer. */
        if ((UU_WLAN_IEEE80211_OP_MODE_R == UU_WLAN_MODE_ADHOC) &&
            (!uu_wlan_ibss_send_beacon_flag_r) && (i == UU_WLAN_BEACON_QID))
        {
            if (uu_wlan_tx_get_frame_info(i) != NULL)
            {
                uu_wlan_cap_stop_beacon_tx ();
            }
            continue;
        }
#endif /* #if 0 */

        /* If Tx is not ready for this AC, ignore */
        /* TODO: There is a race condition, according to Chandrasekhar.  Need to fix it */
        if (!uu_wlan_cap_ctxt_g.ac_cntxt[i].tx_ready)
        {
            continue;
        }

        UU_WLAN_LOG_DEBUG(("LMAC: Contend timer tick, aifs val %d, backoff val %d, ac %d\n", uu_wlan_cap_ctxt_g.ac_cntxt[i].aifs_val, uu_wlan_cap_ctxt_g.ac_cntxt[i].backoff_val, i));

        if(uu_wlan_cap_ctxt_g.ac_cntxt[i].aifs_val)
        {
            uu_wlan_cap_ctxt_g.ac_cntxt[i].aifs_val--;
            if(!uu_wlan_cap_ctxt_g.ac_cntxt[i].aifs_val)
            {
                /* Secondary channel assessment. The random backoff could be 1, so update it here */
                uu_wlan_cap_ctxt_g.prev_channel_list = uu_wlan_cap_ctxt_g.updated_channel_list;

                if(!uu_wlan_cap_ctxt_g.ac_cntxt[i].backoff_val)
                {
                    /* Generate backoff random number */
                    /* If its AP mode, then backoff for transmitting beacon frame is 0.
                       But for IBSS mode, the backoff is generally between 0 and (2 * aCWmin * aSlotTime). */
                    if ((i == UU_WLAN_BEACON_QID) && (UU_WLAN_IEEE80211_OP_MODE_R == UU_WLAN_MODE_MASTER))
                    {
                        uu_wlan_cap_ctxt_g.ac_cntxt[i].backoff_val = 0;
                    }
                    else
                    {
                        uu_wlan_cap_ctxt_g.ac_cntxt[i].backoff_val = _random_num(uu_wlan_cap_ctxt_g.ac_cntxt[i].cw_val);
                    }
                }
            }

            ret = UU_WLAN_CAP_STATE_CONTEND; /* Atleast 1 AC has tx pending */
        }
        else /* AIFS is already 0 */
        {
            /* Decrement backoff timer, if non-zero */
            if(uu_wlan_cap_ctxt_g.ac_cntxt[i].backoff_val > 0)
            {
                uu_wlan_cap_ctxt_g.ac_cntxt[i].backoff_val--;

                /* Secondary channel assessment. We have to use the current CCA status, for Tx after 1 slot */
                if(uu_wlan_cap_ctxt_g.ac_cntxt[i].backoff_val == 1)
                {
                    uu_wlan_cap_ctxt_g.prev_channel_list = uu_wlan_cap_ctxt_g.updated_channel_list;
                }
            }
            /* If backoff timer is still non-zero */
            if (uu_wlan_cap_ctxt_g.ac_cntxt[i].backoff_val)
            {
                ret = UU_WLAN_CAP_STATE_CONTEND;
            }
            else /* If Backoff also turned to 0 */
            {
                uu_wlan_tx_frame_info_t *frame_info;

                /* TODO: This is a temporary work-around. Consider using separate Q for PS-Poll & NULL data frames */
                if(uu_wlan_lmac_pspoll_frame_ready_g)
                {
                    frame_info = ps_poll_frame_info_g;
                }
                else if(uu_wlan_lmac_trigger_frame_ready_g)
                {
                    frame_info = ps_trigger_frame_info_g;
                }
                else
                {
                    frame_info = uu_wlan_get_msg_ref_from_AC_Q(i);
                }

                if (frame_info == UU_NULL)
                {
                    /* No more contention for this AC */
                    uu_wlan_cap_ctxt_g.ac_cntxt[i].tx_ready = 0;
                    continue;
                }

                ret = UU_WLAN_CAP_STATE_CONTEND; /* Atleast 1 AC has tx pending */

                /* Select this 'AC', if a higher-priority AC is not selected */
                if (selected_ac < 0)
                {
                    /* Won the TXOP */
                    selected_ac = i;
                }
                else /* Internal collision, as an AC is already selected */
                {
                    /*
                     * Ref: 'Retransmission Procedure' under 'EDCA'.
                     * For internal collisions occurring with the EDCA,
                     *  SRC/LRC of frame & QSRC/QLRC of AC are incremented.
                     * STAs shall do retry mechanism, until the limit is reached.
                     */
                    if(UU_WLAN_RTS_THRESHOLD_R > frame_info->frameInfo.framelen)
                    {
                        //uu_wlan_cap_ctxt_g.ac_cntxt[i].SSRC++;
                        uu_wlan_ShortRetryCount[i]++;
                    }
                    else
                    {
                        //uu_wlan_cap_ctxt_g.ac_cntxt[i].SLRC++;
                        uu_wlan_LongRetryCount[i]++;
                    }
                    if(uu_wlan_cap_ctxt_g.ac_cntxt[i].cw_val < UU_WLAN_CW_MAX_VALUE_R(i))
                    {
                        uu_wlan_cap_ctxt_g.ac_cntxt[i].cw_val =
                            ((uu_wlan_cap_ctxt_g.ac_cntxt[i].cw_val << 1) | 1);
                    }
                    /* Retry limit condition */
                    if((uu_wlan_ShortRetryCount[i] >= UU_WLAN_DOT11_SHORT_RETRY_COUNT_R) ||
                                (uu_wlan_LongRetryCount[i] >= UU_WLAN_DOT11_LONG_RETRY_COUNT_R))
                    {
                        /* Reached frame retry limit. Discard the packet*/
                        uu_wlan_clear_mpdu(i, UU_FALSE);

                        /* reset cw_val value to cw min */
                        uu_wlan_cap_ctxt_g.ac_cntxt[i].cw_val = UU_WLAN_CW_MIN_VALUE_R(i);
                    }
                } /* Internal collision */
            } /* If Backoff also turned to 0 */
        } /* AIFS is already 0 */

        /* We will suspend decrementing backoff for the other ACs for IBSS case
         * As per specification, section 10.1.3.3 a)
         */
        if (UU_WLAN_IEEE80211_OP_MODE_R == UU_WLAN_MODE_ADHOC)
        {
            if (i == UU_WLAN_BEACON_QID)
            {
                break;
            }
        }

    } /* for (each AC) */

    /* If an AC is selected to own the TXOP */
    if (selected_ac >= 0)
    {
        UU_WLAN_LOG_DEBUG(("Selected ac in contend state: %d\n", selected_ac));
        uu_wlan_cap_ctxt_g.txop_owner_ac = selected_ac;

        /* give indication to txop_tx_module */
        uu_wlan_put_msg_in_CAP_Q(UU_WLAN_CAP_EV_TXOP_TX_START, UU_NULL, 0);
        ret = UU_WLAN_CAP_STATE_TXOP;
    }
    return ret;
} /* contend_timer_tick */


uu_int32 contend_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = UU_WLAN_CAP_STATE_CONTEND;

    if (cm_contend_action_g[event] != UU_NULL)
    {
        ret = (*cm_contend_action_g[event])(sm_cntx, event, ev_data);
    }
    else
    {
        //LOG Error
        ret = UU_WLAN_CAP_STATE_CONTEND;
    }
    return ret;
}


/* TXOP STATE */
uu_int32 txop_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    uu_int32 ret;
    /* reset supplimentary state machine */
    ret = uu_wlan_cap_supp_sm_reset();
    return UU_SUCCESS;
}

uu_int32 txop_exit(uu_uchar *sm_cntx, uu_int32 event)
{
    return UU_SUCCESS;
}

uu_int32 txop_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = UU_WLAN_CAP_STATE_TXOP;

    /* run event on supp state machine */
    ret = uu_wlan_supp_sm_run_for_event(&uu_wlan_cap_ctxt_g.txop_sm_cntxt, event, ev_data);
    if(ret == SUPP_SM_TXOP_ERROR_RECOVERY)
    {
        ret = UU_WLAN_CAP_STATE_EIFS;
    }
    else if(ret == SUPP_SM_TXOP_NAV_UPDATE)
    {
        ret = UU_WLAN_CAP_STATE_RECEIVING;
    }
    else if ((ret == SUPP_SM_CONTEND) || (ret == SUPP_SM_TX_FAILURE) ||
        (ret == SUPP_SM_TXOP_LIMIT_COMP) || (ret == SUPP_SM_TX_END))
    {
        if (uu_wlan_is_tx_ready_for_any_AC_Q())
            ret = UU_WLAN_CAP_STATE_CONTEND;
        else
            ret = UU_WLAN_CAP_STATE_IDLE;
    }
    else if (ret == SUPP_SM_TX_NAV_ENDED)
    {
        if (uu_wlan_cap_ctxt_g.eifs_timer_val)
        {
            ret = UU_WLAN_CAP_STATE_EIFS;
        }
        else if (uu_wlan_cap_ctxt_g.nav_val)
        {
            ret = UU_WLAN_CAP_STATE_RECEIVING;
        }
        else /* Both 'eifs' & 'nav' are 0 */
        {
            ret = UU_WLAN_CAP_STATE_CONTEND;
        }
    }
    else if (ret == SUPP_SM_STATE_CONTINUE)
    {
        ret = UU_WLAN_CAP_STATE_TXOP;
    }
    else if(ret == SUPP_SM_RX_VALID_FRAME)
    {
        //ret = UU_WLAN_CAP_STATE_RECEIVING;
        ret = UU_WLAN_CAP_STATE_CONTEND;
    }
    else
    {
        /**/
    }
    return ret;
} /* txop_action */


/* EIFS state */
uu_int32 eifs_entry(uu_uchar *sm_cntx, uu_int32 event)
{
    if(!uu_wlan_cap_ctxt_g.eifs_timer_val)
    {
    uu_wlan_cap_ctxt_g.eifs_timer_val = UU_WLAN_EIFS_VALUE_R;
    }

    return UU_SUCCESS;
}

uu_int32 eifs_exit(uu_uchar *sm_cntx, uu_int32 event)
{
    uu_wlan_cap_ctxt_g.eifs_timer_val = 0;
    return UU_SUCCESS;
}

static uu_int32 eifs_tx_ready_pkt(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;

    uu_wlan_cap_ctxt_g.ac_cntxt[ev_info->u.tx_ready.ac].tx_ready = 1;
    return UU_WLAN_CAP_STATE_EIFS;
}

static uu_int32 eifs_phy_energy_ind(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_wlan_cap_event_t *ev_info = (uu_wlan_cap_event_t *)ev_data;
    uu_wlan_cap_ctxt_g.cca_status = ev_info->u.cca_status.status;
    if(ev_info->u.cca_status.status)
    {
        uu_wlan_cap_ctxt_g.updated_channel_list = (uu_wlan_busy_channel_list_t)ev_info->u.cca_status.channel_list;
    }
    else
    {
        uu_wlan_cap_ctxt_g.updated_channel_list = UU_WLAN_CH_IDLE;
    }
    /* energy level indication is high and primary is free */
    if(uu_wlan_cap_ctxt_g.updated_channel_list == UU_WLAN_BUSY_CH_PRIMARY)
    {
        return UU_WLAN_CAP_STATE_RECEIVING;
    }
    return UU_WLAN_CAP_STATE_EIFS;
}

static uu_int32 eifs_cca_reset_confirm(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    return UU_WLAN_CAP_STATE_EIFS;
}

static uu_int32 eifs_timer_tick(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    if(uu_wlan_cap_ctxt_g.eifs_timer_val)
    {
        uu_wlan_cap_ctxt_g.eifs_timer_val--;
    }
    if(uu_wlan_cap_ctxt_g.nav_val)
    {
        uu_wlan_cap_ctxt_g.nav_val--;
        if(!uu_wlan_cap_ctxt_g.nav_val)
        {
            /* send cca-reset request to phy */
            uu_wlan_phy_ccareset_req();
        }
    }

    /* Continue to decrement NAV (if > 0) in RECEIVING state */
    if (!uu_wlan_cap_ctxt_g.eifs_timer_val)
    {
        if(uu_wlan_cap_ctxt_g.nav_val)
        {
            return UU_WLAN_CAP_STATE_RECEIVING;
        }
        else
        {
            if (uu_wlan_is_tx_ready_for_any_AC_Q())
            {
        return UU_WLAN_CAP_STATE_CONTEND;
    }
            else
            {
                return UU_WLAN_CAP_STATE_IDLE;
            }
        }
    }
    return UU_WLAN_CAP_STATE_EIFS;
}

uu_int32 eifs_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data)
{
    uu_int32 ret = UU_WLAN_CAP_STATE_EIFS;

    if (cm_eifs_action_g[event] != UU_NULL)
    {
        ret = (*cm_eifs_action_g[event])(sm_cntx, event, ev_data);
    }
    else
    {
        ret = UU_WLAN_CAP_STATE_EIFS;
    }
    return ret;
}


/** This function is called by RX module, to set the received packet status
 */
uu_void uu_wlan_cap_set_recv_pkt_status(uu_wlan_rx_handler_return_value_t ret)
{
    uu_wlan_cap_rx_status_value_g = ret;
} /* uu_wlan_cap_change_rx_status */


/** This function is called by RX module, to stop transmitting the beacon in IBSS mode
 * and clear the beacon frame from the Beacon queue.
 * This function is also called by CAP module, when we are contending and
 * Beacon is present in the Buffer but we should not send the Beacon. */

#ifdef UU_WLAN_IBSS
uu_void uu_wlan_cap_stop_beacon_tx (uu_void)
{
    /* Assign this register informing that we are not the IBSS owner for this TBTT */
    uu_wlan_last_beacon_tx_r = 0;
    /* uu_wlan_ibss_send_beacon_flag_r = 0; Now triggering beacon in UMAC */
#ifdef UU_WLAN_BQID
    /* Stop contending for the beacon queue */
    if (uu_wlan_tx_get_frame_info(UU_WLAN_BEACON_QID) != UU_NULL)
    {
        uu_wlan_cap_ctxt_g.ac_cntxt[UU_WLAN_BEACON_QID].tx_ready = 0;
        /* Clear the beacon frame from the Beacon queue */
        uu_wlan_clear_mpdu(UU_WLAN_BEACON_QID, UU_FALSE);
    }
#endif
	return;
}
#endif /* UU_WLAN_IBSS */


/* EOF */

