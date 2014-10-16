/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_ps_uapsd.c                                     **
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
/* Contains functions for UAPSD (and Legacy) power-save */

#include "uu_datatypes.h"
#include "uu_wlan_tx_if.h"
#include "uu_wlan_ps.h"
#include "uu_wlan_reg.h"
#include "uu_errno.h"
#include "uu_wlan_rx_handler.h"
#include "uu_wlan_cap_context.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_lmac_sta_info.h"
#include "uu_wlan_cap_slot_timer.h"

static uu_void uu_wlan_set_sta_sleep(uu_void);


/** This function is used to check whether corresponding AID bit in the TIM is set or not in the DTIM beacon.
 * called when tracing a beacon in a PS STA in Rx handler.
 **/
static uu_bool uu_wlan_check_tim(unsigned char *tim , unsigned short aid);

/** This function is used to gather infornation required for Power Saving in a STA from beacon.
 * called in Rx handler if the STA is configured to be in PS.
 **/
static uu_void uu_wlan_ps_trace_beacon(uu_uchar *mpdu);


/* global variables */

/** This flag is used during power save, It indicates whether the STA is in sleep( RF-inactive ) or not.
 * Set in idle state when the power save is enabled in LMAC and there is no more data to send and receive.
 * Reset when there is data to transmit or when the DTIM timer expires.
 **/
static uu_bool uu_wlan_lmac_in_sleep_g;

/** holds the current value of DTIM period.
 * Used in DTIM timer. It contains the maximum time a STA can sleep before waking up for a beacon.
 * Initialized with information in DTIM beacon ,when a powersaving STA receives a  DTIM beacon.
 * Used in DTIM timer setup and DTIM timer call back functions.
 **/
uu_uint32 uu_wlan_lmac_dtim_period_g;

/** Indicate powersaving is enabled, after configuring the STA to PS.
 * Set in Rx handler after receiving a DTIM beacon in a PS STA.
 * Reset in Rx handler or CAP, which ever gets chance after deconfiguring PS.
 **/
static uu_bool uu_wlan_lmac_ps_enabled_g ;

/** Indicate whether the STA is received DTIM beacon or not after DTIM wakeup.
 * set in Rx handler after receiving a DTIM beacon, if the STA is a PS STA.
 * Reset after DTIM expiry i.e in DTIM timer call back.*/
uu_bool uu_wlan_lmac_dtim_bcn_received_g;

/** Indicate that more data is at AP for receiption, for a PS STA.
 * Set in Rx handler whenever STA receive a Data frame with more data bit set and TIM is set for the STA in DTIM beacon.
 * Reset in Rx handler when the is no more data i.e when more data bit is not set in received data frame.
 **/
static uu_bool uu_wlan_lmac_ps_more_data_g;

/** Indicate the End of Service Period ( EOSP ) in case of U-APSD PS.
 * Set in Rx handler when EOSP bit in received QoS data frame is set.
 * Reset in Rx handler when EOSP bit in received Qos data frame is not set.
 **/
static uu_bool uu_wlan_lmac_eosp_g;



uu_void uu_wlan_ps_handle_idle_entry(uu_void)
{
    if ((!uu_wlan_lmac_in_sleep_g) && uu_wlan_lmac_is_ps_enabled())
    {

        if((!uu_wlan_lmac_is_ps_more_data()) && uu_wlan_lmac_is_dtim_bcn_received())
        {   
            uu_wlan_set_sta_sleep(); 
        }

    }
}


uu_void uu_wlan_ps_idle_exit(uu_void)
{
    if (uu_wlan_lmac_in_sleep_g)
    {
        /* If the STA is deconfigured from PS, disable ps by transmitting a frame with PM bit 0 */
        if (!uu_wlan_ps_mode_r)
        {
            uu_wlan_lmac_disable_ps();
        }

        uu_wlan_set_sta_awake();
    }
}


uu_void uu_wlan_ps_handle_rx_frame(uu_uchar *mpdu, uu_int32 dest)
{

    /* power save related code starts here */
    if (uu_wlan_ps_mode_r || uu_wlan_lmac_ps_enabled_g)
    {
        if ((dest == UU_WLAN_RX_FRAME_DEST_BROADCAST) &&
                (UU_WLAN_GET_FC0_VALUE(mpdu) == (IEEE80211_FC0_TYPE_MGT | IEEE80211_FC0_SUBTYPE_BEACON)))
        { 
            /* here powersave disabled without packet loss, if deconfigured powersave */ 
            if (!uu_wlan_ps_mode_r)
            {
                uu_wlan_lmac_ps_enabled_g = UU_FALSE;
                uu_wlan_lmac_qos_null_tx_handling((mpdu + UU_TA_OFFSET) , 0 );
            }
            else
            {
                uu_wlan_ps_trace_beacon(mpdu);
                //  uu_wlan_lmac_dtim_bcn_received_g = UU_TRUE;
            }
        }
        else if (dest == UU_WLAN_RX_FRAME_DEST_US)
        {

            if( UU_WLAN_GET_FC0_VALUE(mpdu) == (IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_DATA) )
            {
                uu_wlan_lmac_ps_more_data_g = ( IEEE80211_IS_FC1_MOREDATA_SET(mpdu) ? UU_TRUE : UU_FALSE );

                if( uu_wlan_lmac_ps_more_data_g && uu_wlan_lmac_ps_enabled_g )
                {
                    uu_wlan_lmac_pspoll_tx_handling((mpdu + 10), 1);
                }
            }

            if(UU_WLAN_GET_FC0_VALUE(mpdu) == (IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_QOS))
            {
                uu_wlan_lmac_ps_more_data_g = ( IEEE80211_IS_FC1_MOREDATA_SET(mpdu) ? UU_TRUE : UU_FALSE );

                if(uu_wlan_uapsd_mode_r)
                {
                    uu_wlan_lmac_eosp_g = UU_WLAN_GET_QOS_EOSP_VALUE(mpdu);

                    if(uu_wlan_lmac_eosp_g)

                        // send one more trigger frame if both more data bit and EOSP bit are set 
                        if( uu_wlan_lmac_ps_more_data_g && uu_wlan_lmac_eosp_g )
                        {
                            uu_wlan_lmac_eosp_g = 0;
                            if (!uu_wlan_is_tx_ready_for_any_AC_Q())
                            {
                                uu_wlan_lmac_qos_null_tx_handling(( mpdu + UU_TA_OFFSET ) ,0 );
                            }
                        }
                }
                else if( uu_wlan_lmac_ps_more_data_g && uu_wlan_lmac_ps_enabled_g )
                {
                    uu_wlan_lmac_pspoll_tx_handling(( mpdu + UU_TA_OFFSET ), 1 );
                }
            }
        } 
    }
} /* uu_wlan_ps_handle_rx_frame */



/** Checks whether the bit corresponding to AID of the STA is set or not in TIM of the beacon.
 * Called in uu_wlan_ps_trace_beacon function,if the STA is in PS and the received frame is a DTIM beacon.
 **/
static uu_bool uu_wlan_check_tim(unsigned char *tim , unsigned short aid)
{
    unsigned char *bitmap_ctrl,*tim_len,*virtual_map;
    unsigned short index, n1, n2, mask, oct, bc;

    tim_len = tim+1;
    bitmap_ctrl = tim+4;
    virtual_map = tim+5;
    aid &= 0x3fff;
    oct = (aid & 0xfff8);
    index =( oct >> 3 );
    mask  = 1 << (aid & 7);
    bc = (*bitmap_ctrl) & 0x01;

    if(bc)
    {
        uu_wlan_lmac_ps_more_data_g = UU_TRUE;
    }

    n1 = (*bitmap_ctrl) & 0xfe;
    n2 = (*tim_len) + n1 - 4;

    if (index < n1 || index > n2)
        return 0;

    index -= n1;

    return !!(virtual_map[index] & mask);
}/* uu_wlan_check_tim */


/** Used to gather required information from beacon,if the STA is in Power Save.
 * Called in Rx handler if the received frame is a beacon, provided the STA is in PS.
 * Sets uu_wlan_lmac_dtim_bcn_received_g flag if the received frame is a DTIM beacon.
 * updates the beacon interval value from the received DTIM beacon.
 * calls the uu_wlan_check_tim function. if it returns true, sends a PS-Poll frame for retrieving BUs from AP.
 **/
static uu_void uu_wlan_ps_trace_beacon(uu_uchar *mpdu)
{
    static bool temp = 0;
    uu_uchar *beacon_ptr, *tim_ptr, *ra;
    uu_uint16 aid, bcn_int, *bcn_int_ptr, len, i=0;

    aid = uu_wlan_sta_aid_r;
    /*escaping beacon header*/
    ra = (mpdu + UU_TA_OFFSET);
    beacon_ptr = ( mpdu + UU_BEACON_HDR_SIZE );

    /* TIM element starts after timestamp,beacon interval,capability info fields and some of the info elements*/

    beacon_ptr = (beacon_ptr+ UU_TIME_STAMP_SIZE);
    /* updating beacon interval value */
    bcn_int_ptr = (uu_uint16 *)beacon_ptr;
    bcn_int = *bcn_int_ptr;

    beacon_ptr = (beacon_ptr + 4);//UU_BEACON_INTRVL_SIZE + UU_CAPABILITY_INFO_SIZE;

    /* escaping other information element fields */
    while (*beacon_ptr != 5)
    {
        // The next byte of the element id gives the length of the element
        beacon_ptr = (beacon_ptr + 1);
        len =(uu_uint16) (*beacon_ptr);
        len++;
        beacon_ptr = (beacon_ptr + len);
        i++;
    }

    tim_ptr = beacon_ptr;

    if(*(tim_ptr+2) == 0)
    {
        uu_wlan_lmac_ps_enabled_g = UU_TRUE;
        uu_wlan_lmac_dtim_period_g = (*(tim_ptr+3))*(bcn_int);
        if(!temp)
        {
            temp = 1;
            uu_wlan_cap_dtim_timer_start();
        }
        uu_wlan_lmac_dtim_bcn_received_g = UU_TRUE;
    }

    if (uu_wlan_check_tim(tim_ptr, aid))
    {
        //getting ready for PS poll/QoS Null Data.
        uu_wlan_lmac_ps_more_data_g = UU_TRUE;
        uu_wlan_lmac_pspoll_tx_handling( ra , 1 );
    }
    else
    {
        uu_wlan_lmac_ps_more_data_g = UU_FALSE;
    }
} /* uu_wlan_ps_trace_beacon */


uu_bool uu_wlan_lmac_is_ps_enabled(uu_void)
{
    return uu_wlan_lmac_ps_enabled_g;
}

uu_bool uu_wlan_lmac_is_dtim_bcn_received(uu_void)
{
    return uu_wlan_lmac_dtim_bcn_received_g;
}

uu_bool uu_wlan_lmac_is_ps_more_data(uu_void)
{
    return uu_wlan_lmac_ps_more_data_g;
}

uu_bool uu_wlan_lmac_is_eosp(uu_void)
{
    return uu_wlan_lmac_eosp_g;
}
uu_void uu_wlan_lmac_disable_ps(uu_void)
{
    uu_wlan_lmac_ps_enabled_g = UU_FALSE;
}


static uu_void uu_wlan_set_sta_sleep(uu_void)
{
    uu_wlan_lmac_in_sleep_g = UU_TRUE;
}

uu_void uu_wlan_set_sta_awake(uu_void)
{
    uu_wlan_lmac_in_sleep_g = UU_FALSE;
}

uu_bool uu_wlan_is_sta_in_sleep(uu_void)
{
    return uu_wlan_lmac_in_sleep_g;
}


/* EOF */

