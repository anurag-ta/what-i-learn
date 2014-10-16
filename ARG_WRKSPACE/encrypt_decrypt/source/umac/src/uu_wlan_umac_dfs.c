/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_umac_dfs.c                                            **
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

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

#include "uu_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_main.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_umac_dfs.h"
#include "uu_wlan_lmac_if.h"

#include "uu_wlan_buf_desc.h"

#ifdef UU_WLAN_DFS

/* --------------------------------- MEASURING REPORTS AT LMAC -----------------------------------*/

/* -------------------------- Start measurement ------------------------------ */

uu_void uu_wlan_start_measurement (uu_uint8 meas_type)
{
    /** As per the measurement type, we will call the corresponding function to fill the corresponding register. */
    switch (meas_type)
    {
        case UU_WLAN_BASIC_MSR:
            {
                phy_radar_detected_g = 0;
                msrment_stats_g = UU_WLAN_BASIC_MSR;
            }
            break;

        case UU_WLAN_CHANNEL_LOAD_MSR:
            {
                chan_busy_dur_g = 0;
                msrment_stats_g = UU_WLAN_CHANNEL_LOAD_MSR;
            }
            break;

        default:
            {
                return;
            }
            /** No operation for not supported measurement request types */
            break;
    }
} /* uu_wlan_start_measurement */


/* -------------------------- Stop/Process measurement ------------------------------ */

/** This need to be called whenever the state changes in the measurement request */
uu_void uu_wlan_process_measurement (uu_void)
{
    msrment_stats_g = NO_STATS_MEASUREMENT;
}

/* --------------------------------- AUTO MEASUREMENT REPORT FRAME -----------------------------------*/

/**
 * This will be done only if the current operating channel is DFS affected.
 *
 * Autonomous Basic Measurement Report
 * - This will be sent only when radar is detected.
 * The input parameters will be given by Algorithm.
 */
uu_void uu_wlan_process_auto_measurement_report (uu_uint64 meas_start_time, uu_uint16 meas_duration)
{
    struct sk_buff *skb;
    uu_mgmt_action_frame_t *msr_report;
    struct ieee80211_tx_info *info;
    int i;

    /** START */
    uu_wlan_start_measurement(UU_WLAN_BASIC_MSR);
    msleep (meas_duration);
    /** STOP */
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Requested measurement type %d\n", UU_WLAN_BASIC_MSR);
#endif
    uu_wlan_process_measurement();

    /* TODO: Delete this, Assume that radar is detected */
    phy_radar_detected_g = 1;

    /** If radar is not detected */
    if (!phy_radar_detected_g)
    {
        return;
    }

    /** If radar is detected in AP */
    if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
    {
        /** TODO: give_event_to_higher_layer (); */
        return;
    }

    skb = dev_alloc_skb(sizeof(uu_mgmt_action_frame_t));
    if (!skb)
        return;

    /** 24 is the number of octets of management frame format excluding HT_Control, Frame_body and FCS */
    msr_report = (uu_mgmt_action_frame_t *)skb_put(skb, 24);
    /** Need to fill: frame_control, duration, da, sa, bssid, seq_ctrl, action (category,
                u (measurement (action_code, dialog_token, element_id, length, ieee80211_msrment_ie msr_elem))) */
    memset(msr_report, 0, 24);
    memcpy(msr_report->sa, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
    memcpy(msr_report->da, uu_wlan_context_gp->bssid, ETH_ALEN);
    memcpy(msr_report->bssid, uu_wlan_context_gp->bssid, ETH_ALEN);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Source Address: %pM\nDestination Address: %pM\nBSSID: %pM\n",
                        msr_report->sa, msr_report->da, msr_report->bssid);
#endif
    msr_report->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);

    /** cate (1), mgmt_axn(1), dialog_token(1), e_id(1), length (1), meas_tken(1), mode(1),
       meas_type (1), size of basic request */
    skb_put(skb, 3 + 5 + sizeof(uu_wlan_basic_report_t));
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len 2: %d\n", skb->len);
#endif

    /** Measurement Report frame */
    msr_report->uu_meas_frame.category = WLAN_CATEGORY_SPECTRUM_MGMT;
    msr_report->uu_meas_frame.mgmt_action = WLAN_ACTION_SPCT_MSR_RPRT;
    /** Refer to section 8.4.2.24.1 General in P802.11_REV_md_D12
       for filling the measurement report */
    /** For autonomous report, it is 0. */
    msr_report->uu_meas_frame.dialog_token = 0;

    /** Measurement Report elements */

    /** This below applicable only for Basic Report */
    /** 5 for element_id, length, meas_token, report_mode, meas_type */
    memset(&msr_report->uu_meas_frame.u.meas_element, 0, sizeof(uu_wlan_basic_report_t) + 5);
    /** 3 because length for element_id and length is not considered */
    msr_report->uu_meas_frame.u.meas_element.length = sizeof(uu_wlan_basic_report_t) + 3;

    msr_report->uu_meas_frame.u.meas_element.element_id = WLAN_EID_MEASURE_REPORT;
    /** for autonomous report, it is 0. */
    msr_report->uu_meas_frame.u.meas_element.meas_token = 0;
    /** For autonomous report, it is 0 */
    msr_report->uu_meas_frame.u.meas_element.mode.meas_report_mode = 0;
    /** Presently just basic_report is supported, so no request_ie->type since its autonomous */
    msr_report->uu_meas_frame.u.meas_element.meas_type = IEEE80211_SPCT_MSR_RPRT_TYPE_BASIC;

    /** Filling the basic report */
    msr_report->uu_meas_frame.u.meas_element.u.report.basic_report.basic_msr.channel_number = Channel_hwvalue;
    msr_report->uu_meas_frame.u.meas_element.u.report.basic_report.basic_msr.meas_start_time = meas_start_time;
    msr_report->uu_meas_frame.u.meas_element.u.report.basic_report.basic_msr.meas_duration = meas_duration;

    msr_report->uu_meas_frame.u.meas_element.u.report.basic_report.map.radar = phy_radar_detected_g;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len 3:%d: ", skb->len);
    for (i = 0; i < skb->len; i++)
    {
        printk("%x ", skb->data[i]);
    }
    printk("\n ");
#endif

    skb_set_queue_mapping(skb, IEEE80211_AC_VO);
    skb->priority = 7;

    info = IEEE80211_SKB_CB(skb);
    info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;

    __send_frame(skb);
} /* uu_wlan_process_auto_measurement_report */


/* --------------------------------- MEASUREMENT REPORT FRAME -----------------------------------*/


/** Measurement Report for processing Measurement Request */
uu_void uu_wlan_process_and_send_measurement_report (uu_mgmt_action_frame_t *msr_request)
{
    struct sk_buff *skb;
    uu_mgmt_action_frame_t *msr_report;
    struct ieee80211_tx_info *info;
    int i;
    uu_uint8 measr_type;
    uu_uint32 Temp_Operating_chbndwdth;
    uu_uint32 Temp_Channel_hwvalue;
    uu_uint32 Temp_Center_frequency;

    /** Assigning the operating channel to the temporary variables. */
    Temp_Operating_chbndwdth = Operating_chbndwdth;
    Temp_Channel_hwvalue = Channel_hwvalue;
    Temp_Center_frequency = Center_frequency;

    /** For spectrum measurement action request frame */
    if (msr_request->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT)
    {
        measr_type = msr_request->uu_meas_frame.u.meas_element.meas_type;
    }
    /** For radio measurement action request frame */
    else
    {
        uu_uint16 temp_timer, temp_timer2;
        get_random_bytes(&temp_timer, sizeof(temp_timer));
        temp_timer2 = temp_timer % ((msr_request->uu_meas_frame.u.radio_req_element.rreq_element.u.request.chanload_request.clreq_msr.rand_interval) + 1);
        measr_type = msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_type;
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Random interval time for channel load : %d\n", temp_timer2);
#endif
        msleep (temp_timer2);
    }

    do
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
        if (msr_request->uu_meas_frame.category == UU_WLAN_CATEGORY_RADIO_MGMT)
        {
            printk("Repetition: %d\n", msr_request->uu_meas_frame.u.radio_req_element.repetitions);
        }
#endif

        skb = dev_alloc_skb(sizeof(uu_mgmt_action_frame_t));
        if (!skb)
            return;
        /** 24 is the number of octets of management frame format excluding HT_Control, Frame_body and FCS */
        msr_report = (uu_mgmt_action_frame_t *)skb_put(skb, 24);
        /** Need to fill: frame_control, duration, da, sa, bssid, seq_ctrl, action (category,
                    u (measurement (action_code, dialog_token, element_id, length, ieee80211_msrment_ie msr_elem))) */
        memset(msr_report, 0, 24);
        memcpy(msr_report->sa, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
        memcpy(msr_report->da, msr_request->sa, ETH_ALEN);
        memcpy(msr_report->bssid, msr_request->bssid, ETH_ALEN);
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Source Address: %pM\nDestination Address: %pM\nBSSID: %pM\n",
                    msr_report->sa, msr_report->da, msr_report->bssid);
#endif

        msr_report->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);

        /** Measurement Report frame */
        switch (measr_type)
        {
            case UU_WLAN_BASIC_MSR:
                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("Preparing BASIC_MEASUREMENT_REPORT\n");
#endif

                    /** Switching to the channel to be measured */
                    /** 20 MHz currently, but this needs to be assigned properly depending on the operating class and channel number */
                    Operating_chbndwdth = 0;
                    Channel_hwvalue = msr_request->uu_meas_frame.u.meas_element.u.request.spec_req.channel_number;
                    Center_frequency = 5000 + Channel_hwvalue * 5;

                    /* if (meas_start_time)
                       {
                            if (meas_start_time < TSF timer)
                            {
                                uu_wlan_late_measurement_report (uu_mgmt_action_frame_t *msr_request);
                                return;
                            }
                            else if (meas_start_time == TSF timer)
                            {
                                start_measurement;
                            }
                        }
                        else
                        {
                            start_measurement;
                        }
                    */
                    /** START */
                    uu_wlan_start_measurement(measr_type);
                    msleep (msr_request->uu_meas_frame.u.meas_element.u.request.spec_req.meas_duration);
                    /** STOP */
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("Requested measurement type %d\n", measr_type);
#endif
                    uu_wlan_process_measurement();

                    /* TODO: Delete this, Assume that radar is detected */
                    phy_radar_detected_g = 1;

                    /** Switching back to the operating channel */
                    Operating_chbndwdth = Temp_Operating_chbndwdth;
                    Channel_hwvalue = Temp_Channel_hwvalue;
                    Center_frequency = Temp_Center_frequency;

                    /** cate (1), mgmt_axn(1), dialog_token(1), e_id(1), length (1), meas_tken(1), mode(1),
                    meas_type (1), size of basic request */
                    skb_put(skb, 3 + 5 + sizeof(uu_wlan_basic_report_t));
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("Action Frame of len 2: %d\n", skb->len);
#endif

                    msr_report->uu_meas_frame.category = WLAN_CATEGORY_SPECTRUM_MGMT;
                    msr_report->uu_meas_frame.mgmt_action = WLAN_ACTION_SPCT_MSR_RPRT;
                    /** Refer to section 8.4.2.24.1 General in P802.11_REV_md_D12 for filling the measurement report */
                    msr_report->uu_meas_frame.dialog_token = msr_request->uu_meas_frame.dialog_token;

                    /** 5 for element_id, length, meas_token, report_mode, meas_type */
                    memset(&msr_report->uu_meas_frame.u.meas_element, 0, sizeof(uu_wlan_basic_report_t) + 5);
                    /** 3 because we will not consider length for element_id and length */
                    msr_report->uu_meas_frame.u.meas_element.length = sizeof(uu_wlan_basic_report_t) + 3;

                    msr_report->uu_meas_frame.u.meas_element.element_id = WLAN_EID_MEASURE_REPORT;
                    msr_report->uu_meas_frame.u.meas_element.meas_token =
                                                        msr_request->uu_meas_frame.u.meas_element.meas_token;
                    /** For autonomous report or for successful processing, it is 0 */
                    msr_report->uu_meas_frame.u.meas_element.mode.meas_report_mode = 0;
                    /** Presently just basic_report is supported, so no request_ie->type since its autonomous */
                    msr_report->uu_meas_frame.u.meas_element.meas_type = IEEE80211_SPCT_MSR_RPRT_TYPE_BASIC;

                    /**
                     * Filling the basic report
                     * TODO: Instead making it equal to measurement duration as per request,
                     * it should be equal to the actual measured duration as per the STA.
                     * Repeat the same for al type of reports.
                     */
                    msr_report->uu_meas_frame.u.meas_element.u.report.basic_report.basic_msr =
                                    msr_request->uu_meas_frame.u.meas_element.u.request.spec_req;
                    //msr_report->uu_meas_frame.u.meas_element.u.report.basic_report.map = 0;

                    msr_report->uu_meas_frame.u.meas_element.u.report.basic_report.map.radar = phy_radar_detected_g;
                }
                break;
            case UU_WLAN_CHANNEL_LOAD_MSR:
                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("Preparing CHANNEL_MEASUREMENT_REPORT\n");
#endif

                    /** Switching to the channel to be measured */
                    /* 20 MHz currently, but this needs to be assigned properly depending on the operating class and channel number */
                    Operating_chbndwdth = 0;
                    Channel_hwvalue = msr_request->uu_meas_frame.u.meas_element.u.request.spec_req.channel_number;
                    Center_frequency = 5000 + Channel_hwvalue * 5;

                    /** START */
                    uu_wlan_start_measurement(measr_type);
                    msleep (msr_request->uu_meas_frame.u.radio_req_element.rreq_element.u.request.chanload_request.clreq_msr.meas_duration);
                    /** STOP */
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("Requested measurement type %d\n", measr_type);
#endif
                    uu_wlan_process_measurement();

                    /* TODO: Delete this, Assume that channel busy duration is 101 us */
                    chan_busy_dur_g = 101;

                    /** Switching back to the operating channel */
                    Operating_chbndwdth = Temp_Operating_chbndwdth;
                    Channel_hwvalue = Temp_Channel_hwvalue;
                    Center_frequency = Temp_Center_frequency;

                    /** TODO: Presently not supporting no_of_repititions > 0 */
                    /** For no_of_repititions, size of (uu_wlan_measurement_elements) is added that much of times */
                    skb_put(skb, 3 + 5 + sizeof(uu_wlan_channel_load_report_t));
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("Action Frame of len 2: %d\n", skb->len);
#endif

                    msr_report->uu_meas_frame.category = UU_WLAN_CATEGORY_RADIO_MGMT;
                    msr_report->uu_meas_frame.mgmt_action = UU_WLAN_ACTION_RADIO_MSR_RPRT;
                    msr_report->uu_meas_frame.dialog_token = msr_request->uu_meas_frame.dialog_token;
                    memset(&msr_report->uu_meas_frame.u.meas_element, 0, sizeof(uu_wlan_channel_load_report_t) + 5);
                    msr_report->uu_meas_frame.u.meas_element.length = sizeof(uu_wlan_channel_load_report_t) + 3;
                    msr_report->uu_meas_frame.u.meas_element.element_id = WLAN_EID_MEASURE_REPORT;
                    msr_report->uu_meas_frame.u.meas_element.meas_token =
                                    msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_token;
                    msr_report->uu_meas_frame.u.meas_element.mode.meas_report_mode = 0;
                    msr_report->uu_meas_frame.u.meas_element.meas_type = UU_WLAN_RADIO_MSR_RPRT_TYPE_CHAN_LOAD;

                    msr_report->uu_meas_frame.u.meas_element.u.report.chanload_report.clrep_msr.operating_class =
                                    msr_request->uu_meas_frame.u.meas_element.u.request.chanload_request.clreq_msr.operating_class;
                    msr_report->uu_meas_frame.u.meas_element.u.report.chanload_report.clrep_msr.channel_number =
                                    msr_request->uu_meas_frame.u.meas_element.u.request.chanload_request.clreq_msr.channel_number;
                    /** TODO: Actual Measurement Start Time is set to the value of the measuring STA’s TSF timer at the time the
                       measurement started. Presently it is wrongly assigned. */
                    msr_report->uu_meas_frame.u.meas_element.u.report.chanload_report.clrep_msr.act_meas_start_time =
                                    msr_request->uu_meas_frame.u.meas_element.u.request.chanload_request.clreq_msr.rand_interval;
                    msr_report->uu_meas_frame.u.meas_element.u.report.chanload_report.clrep_msr.meas_duration =
                                    msr_request->uu_meas_frame.u.meas_element.u.request.chanload_request.clreq_msr.meas_duration;

                    msr_report->uu_meas_frame.u.meas_element.u.report.chanload_report.channel_load =
                                    CHAN_LOAD(chan_busy_dur_g,
                                        msr_report->uu_meas_frame.u.meas_element.u.report.chanload_report.clrep_msr.meas_duration);

                }
                break;
            default:
                {
#if 0
                    /** Incapable bit (bit 1) indicates whether this STA is incapable of generating a report of the type
                     * specified in the Measurement Type field that was previously requested by the destination STA of
                     * this Measurement Report element. The Incapable bit is set to 1 to indicate the STA is incapable. The
                     * Incapable bit is set to 0 to indicate the STA is capable or the report is autonomous.
                     */
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("Preparing INCAPABLE_MEASUREMENT_REPORT\n");
#endif
                    skb_put(skb, 3 + 5);
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("Action Frame of len 2: %d\n", skb->len);
#endif
                    msr_report->uu_meas_frame.category = UU_WLAN_CATEGORY_RADIO_MGMT;
                    msr_report->uu_meas_frame.mgmt_action = UU_WLAN_ACTION_RADIO_MSR_RPRT;
                    msr_report->uu_meas_frame.dialog_token = msr_request->uu_meas_frame.dialog_token;
                    memset(&msr_report->uu_meas_frame.u.meas_element, 0, 5);
                    msr_report->uu_meas_frame.u.meas_element.element_id = WLAN_EID_MEASURE_REPORT;
                    msr_report->uu_meas_frame.u.meas_element.length = 3;
                    msr_report->uu_meas_frame.u.meas_element.meas_token =
                                                        msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_token;
                    msr_report->uu_meas_frame.u.meas_element.mode.meas_report_mode = IEEE80211_SPCT_MSR_RPRT_MODE_INCAPABLE;
                    msr_report->uu_meas_frame.u.meas_element.meas_type =
                                                        msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_type;
#endif
                    return;
                }
                break;
        }

#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("Action Frame of len 3:%d: ", skb->len);
        for (i = 0; i < skb->len; i++)
        {
            printk("%x ", skb->data[i]);
        }
        printk("\n ");
#endif

        skb_set_queue_mapping(skb, IEEE80211_AC_VO);
        skb->priority = 7;

        info = IEEE80211_SKB_CB(skb);
        info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;

        __send_frame(skb);
    } while ((msr_request->uu_meas_frame.category == UU_WLAN_CATEGORY_RADIO_MGMT)
                && (msr_request->uu_meas_frame.u.radio_req_element.repetitions--));
} /* uu_wlan_process_and_send_measurement_report */


/** Refused bit (bit 2) indicates whether this STA is refusing to generate a report of the type specified in
 * the Measurement Type field that was previously requested by the destination STA of this
 * Measurement Report element. The Refused bit is set to 1 to indicate the STA is refusing. The
 * Refused bit is set to 0 to indicate the STA is not refusing or the report is autonomous.
 */
uu_void uu_wlan_refuse_measurement_report (uu_mgmt_action_frame_t *msr_request)
{
    struct sk_buff *skb;
    uu_mgmt_action_frame_t *msr_report;
    struct ieee80211_tx_info *info;
    int i;

    skb = dev_alloc_skb(sizeof(uu_mgmt_action_frame_t));
    if (!skb)
        return;
    /** 24 is the number of octets of management frame format excluding HT_Control, Frame_body and FCS */
    msr_report = (uu_mgmt_action_frame_t *)skb_put(skb, 24);
    /** Need to fill: frame_control, duration, da, sa, bssid, seq_ctrl, action (category,
        u (measurement (action_code, dialog_token, element_id, length, ieee80211_msrment_ie msr_elem))) */
    memset(msr_report, 0, 24);
    memcpy(msr_report->sa, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
    memcpy(msr_report->da, msr_request->sa, ETH_ALEN);
    memcpy(msr_report->bssid, msr_request->bssid, ETH_ALEN);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Source Address: %pM\nDestination Address: %pM\nBSSID: %pM\n",
    msr_report->sa, msr_report->da, msr_report->bssid);
#endif
    msr_report->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Preparing REFUSED_MEASUREMENT_REPORT\n");
#endif
    skb_put(skb, 3 + 5);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len 2: %d\n", skb->len);
#endif
    msr_report->uu_meas_frame.dialog_token = msr_request->uu_meas_frame.dialog_token;
    memset(&msr_report->uu_meas_frame.u.meas_element, 0, 5);
    msr_report->uu_meas_frame.u.meas_element.element_id = WLAN_EID_MEASURE_REPORT;
    msr_report->uu_meas_frame.u.meas_element.length = 3;
    msr_report->uu_meas_frame.u.meas_element.mode.meas_report_mode = IEEE80211_SPCT_MSR_RPRT_MODE_REFUSED;
    /** For spectrum measurement action request frame */
    if (msr_request->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT)
    {
        msr_report->uu_meas_frame.category = WLAN_CATEGORY_SPECTRUM_MGMT;
        msr_report->uu_meas_frame.mgmt_action = WLAN_ACTION_SPCT_MSR_RPRT;
        msr_report->uu_meas_frame.u.meas_element.meas_token =
                                    msr_request->uu_meas_frame.u.meas_element.meas_token;
        msr_report->uu_meas_frame.u.meas_element.meas_type = msr_request->uu_meas_frame.u.meas_element.meas_type;
    }
    /** For radio measurement action request frame */
    else
    {
        msr_report->uu_meas_frame.category = UU_WLAN_CATEGORY_RADIO_MGMT;
        msr_report->uu_meas_frame.mgmt_action = UU_WLAN_ACTION_RADIO_MSR_RPRT;
        msr_report->uu_meas_frame.u.meas_element.meas_token =
                                    msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_token;
        msr_report->uu_meas_frame.u.meas_element.meas_type = msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_type;
    }

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len 3:%d: ", skb->len);
    for (i = 0; i < skb->len; i++)
    {
        printk("%x ", skb->data[i]);
    }
    printk("\n ");
#endif

    skb_set_queue_mapping(skb, IEEE80211_AC_VO);
    skb->priority = 7;

    info = IEEE80211_SKB_CB(skb);
    info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;

    __send_frame(skb);
} /* uu_wlan_refuse_measurement_report */


/** Late bit (bit 0) indicates whether this STA is unable to carry out a measurement request because it
 * received the request after the requested measurement time. The Late bit is set to 1 to indicate the
 * request was too late. The Late bit is set to 0 to indicate the request was received in time for the
 * measurement to be executed. The Late bit only applies to spectrum management measurement and is
 * set to 0 in all measurement report elements for radio measurement types (see Table 8-81).
 */
uu_void uu_wlan_late_measurement_report (uu_mgmt_action_frame_t *msr_request)
{
    struct sk_buff *skb;
    uu_mgmt_action_frame_t *msr_report;
    struct ieee80211_tx_info *info;
    int i;

    skb = dev_alloc_skb(sizeof(uu_mgmt_action_frame_t));
    if (!skb)
        return;
    /** 24 is the number of octets of management frame format excluding HT_Control, Frame_body and FCS */
    msr_report = (uu_mgmt_action_frame_t *)skb_put(skb, 24);
    /** Need to fill: frame_control, duration, da, sa, bssid, seq_ctrl, action (category,
        u (measurement (action_code, dialog_token, element_id, length, ieee80211_msrment_ie msr_elem))) */
    memset(msr_report, 0, 24);
    memcpy(msr_report->sa, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
    memcpy(msr_report->da, msr_request->sa, ETH_ALEN);
    memcpy(msr_report->bssid, msr_request->bssid, ETH_ALEN);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Source Address: %pM\nDestination Address: %pM\nBSSID: %pM\n",
    msr_report->sa, msr_report->da, msr_report->bssid);
#endif
    msr_report->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Preparing LATE_MEASUREMENT_REPORT\n");
#endif
    skb_put(skb, 3 + 5);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len 2: %d\n", skb->len);
#endif
    msr_report->uu_meas_frame.dialog_token = msr_request->uu_meas_frame.dialog_token;
    memset(&msr_report->uu_meas_frame.u.meas_element, 0, 5);
    msr_report->uu_meas_frame.u.meas_element.element_id = WLAN_EID_MEASURE_REPORT;
    msr_report->uu_meas_frame.u.meas_element.length = 3;
    msr_report->uu_meas_frame.u.meas_element.mode.meas_report_mode = IEEE80211_SPCT_MSR_RPRT_MODE_LATE;
    /** For spectrum measurement action request frame */
    if (msr_request->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT)
    {
        msr_report->uu_meas_frame.category = WLAN_CATEGORY_SPECTRUM_MGMT;
        msr_report->uu_meas_frame.mgmt_action = WLAN_ACTION_SPCT_MSR_RPRT;
        msr_report->uu_meas_frame.u.meas_element.meas_token =
                                    msr_request->uu_meas_frame.u.meas_element.meas_token;
        msr_report->uu_meas_frame.u.meas_element.meas_type = msr_request->uu_meas_frame.u.meas_element.meas_type;
    }
    /** For radio measurement action request frame */
    else
    {
        msr_report->uu_meas_frame.category = UU_WLAN_CATEGORY_RADIO_MGMT;
        msr_report->uu_meas_frame.mgmt_action = UU_WLAN_ACTION_RADIO_MSR_RPRT;
        msr_report->uu_meas_frame.u.meas_element.meas_token =
                                    msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_token;
        msr_report->uu_meas_frame.u.meas_element.meas_type = msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_type;
    }

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len 3:%d: ", skb->len);
    for (i = 0; i < skb->len; i++)
    {
        printk("%x ", skb->data[i]);
    }
    printk("\n ");
#endif

    skb_set_queue_mapping(skb, IEEE80211_AC_VO);
    skb->priority = 7;

    info = IEEE80211_SKB_CB(skb);
    info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;

    __send_frame(skb);
} /* uu_wlan_late_measurement_report  */


/* --------------------------------- MEASUREMENT REQUEST FRAME -----------------------------------*/


/** Spectrum Measurement Request formation */
uu_void uu_wlan_prepare_and_send_measurement_request (uu_uint8 meas_req_type)
{
    struct sk_buff *skb;
    uu_mgmt_action_frame_t *msr_request;
    struct ieee80211_tx_info *info;
    int i;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Preparing MEASUREMENT_REQUEST frame \n");
#endif

    skb = dev_alloc_skb(sizeof(uu_mgmt_action_frame_t));
    if (!skb)
        return;

    /** 24 is the number of octets of management frame format excluding HT_Control, Frame_body and FCS */
    msr_request = (uu_mgmt_action_frame_t *)skb_put(skb, 24);
    /** Need to fill: frame_control, duration, da, sa, bssid, seq_ctrl, action (category,
                u (measurement (action_code, dialog_token, element_id, length, ieee80211_msrment_ie msr_elem))) */
    memset(msr_request, 0, 24);

    memcpy(msr_request->da, uu_wlan_context_gp->assoc_sta, ETH_ALEN);
    memcpy(msr_request->sa, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
    memcpy(msr_request->bssid, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Source Address: %pM\nDestination Address: %pM\nBSSID: %pM\n",
                        msr_request->sa, msr_request->da, msr_request->bssid);
#endif

    msr_request->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);


    /** Spectrum Measurement Request frame
     * Category : 1 octet, Value is 0.
     * Spectrum Measurement Action : 1 octet, Value is 0.
     * Dialog Token : 1 octet, Value is non-zero and contains the ID of request/report transaction.
     * Measurement Request element : Variable.
     *
     * Measurement Request element:-
     * Element ID : 1 octet
     * Length : 1 octet
     * Measurement Token : 1 octet
     * Measurement Request Mode : 1 octet
     * Measurement Type : 1 octet
     * Measurement Request : Variable.
     *
     * Measurement Request Mode :-
     * Parallel : 1 bit (LSB)
     * Enable : 1 bit
     * Request : 1 bit
     * Report : 1 bit
     * Duration Mandatory : 1 bit
     * Reserved : 3 bit (MSB)
     *
     */

    /** TODO: Parallel measurement is not yet supported */

    switch (meas_req_type)
    {
        case UU_WLAN_BASIC_MSR:
            {
#ifdef UU_WLAN_DFSTPC_DEBUG
                printk("Preparing BASIC_MEASUREMENT_REQUEST frame \n");
#endif
                /** cate (1), mgmt_axn(1), dialog_token(1), e_id(1), length (1), meas_tken(1), mode(1),
                   meas_type (1), size of basic request */
                skb_put(skb, 3 + 5 + sizeof(uu_wlan_spec_t));
                msr_request->uu_meas_frame.category = WLAN_CATEGORY_SPECTRUM_MGMT;
                /** Presently only for spectrum measurement request */
                msr_request->uu_meas_frame.mgmt_action = WLAN_ACTION_SPCT_MSR_REQ;
                /** Refer to section 8.4.2.23.1 General in P802.11_REV_md_D12 for filling the measurement request */
                msr_request->uu_meas_frame.dialog_token = 89;

                memset(&msr_request->uu_meas_frame.u.meas_element, 0, sizeof(uu_wlan_spec_t) + 5);
                msr_request->uu_meas_frame.u.meas_element.length = sizeof(uu_wlan_spec_t) + 3;
                msr_request->uu_meas_frame.u.meas_element.meas_type = UU_WLAN_BASIC_MSR;
                msr_request->uu_meas_frame.u.meas_element.u.request.spec_req.channel_number = 40;
                /** The Measurement Start Time field is set to the TSF timer at the time (± 32 μs) at which
                 * the requested measurement request starts.
                 * A value of 0 indicates it starts immediately. */
                msr_request->uu_meas_frame.u.meas_element.u.request.spec_req.meas_start_time = 0;
                /** Measurement duration is in TUs or 1024 us */
                msr_request->uu_meas_frame.u.meas_element.u.request.spec_req.meas_duration = 10;
                /** Measurement Report elements */
                msr_request->uu_meas_frame.u.meas_element.element_id = WLAN_EID_MEASURE_REQUEST;
                /** The Measurement Token is set to a nonzero number that is unique among the Measurement Request
                elements in a particular Measurement Request frame. */
                msr_request->uu_meas_frame.u.meas_element.meas_token = 114;
                /** NOTE : meas_req_mode will be in the meas_request_mode format. */
                msr_request->uu_meas_frame.u.meas_element.mode.meas_request_mode = 0;
            }
            break;
        case UU_WLAN_CHANNEL_LOAD_MSR:
            {
                /** cate (1), mgmt_axn(1), dialog_token(1), no_of_repetitions(2), e_id(1), length (1), meas_tken(1),
                   mode(1), meas_type (1), size of channel load request */
#ifdef UU_WLAN_DFSTPC_DEBUG
                printk("Preparing CHANNEL_MEASUREMENT_REQUEST frame \n");
#endif
                skb_put(skb, 10 + sizeof(uu_wlan_channel_load_request_t));
                msr_request->uu_meas_frame.category = UU_WLAN_CATEGORY_RADIO_MGMT;
                msr_request->uu_meas_frame.mgmt_action = UU_WLAN_ACTION_RADIO_MSR_REQ;
                msr_request->uu_meas_frame.dialog_token = 89;
                memset(&msr_request->uu_meas_frame.u.radio_req_element, 0,
                                    sizeof(uu_wlan_channel_load_request_t) + 7);
                msr_request->uu_meas_frame.u.radio_req_element.repetitions = 1;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.element_id = WLAN_EID_MEASURE_REQUEST;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_token = 114;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.mode.meas_request_mode = 0;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.length =
                                                                    sizeof(uu_wlan_channel_load_request_t) + 3;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.meas_type
                                                                    = UU_WLAN_CHANNEL_LOAD_MSR;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.u.request.chanload_request.clreq_msr.operating_class
                                                                    = 117;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.u.request.chanload_request.clreq_msr.channel_number
                                                                    = 40;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.u.request.chanload_request.clreq_msr.rand_interval
                                                                    = 11;
                msr_request->uu_meas_frame.u.radio_req_element.rreq_element.u.request.chanload_request.clreq_msr.meas_duration
                                                                    = 10;
            }
            break;
        default:
            {
                return;
            }
            break;
    }

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Frame of len 3:%d: ", skb->len);
    for (i = 0; i < skb->len; i++)
    {
        printk("%x ", skb->data[i]);
    }
    printk("\n ");
#endif

    skb_set_queue_mapping(skb, IEEE80211_AC_VO);
    skb->priority = 7;

    info = IEEE80211_SKB_CB(skb);
    info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;

    __send_frame(skb);
} /* uu_wlan_prepare_and_send_measurement_request */

/* ----------------------------- END OF SPECTRUM MEASUREMENT REQUEST/REPORT FRAME ----------------------------------*/


/* ------------------------------- TIMER FOR SENDING MEASUREMENT REQUEST FOR AP --------------------------------- */

#define MicroS_TO_NS(x) (x * 1000)

static struct hrtimer uu_wlan_dfs_hr_timer_g;

static uu_int8   exit_flag_g = 0;


enum hrtimer_restart uu_wlan_dfs_hrtimer_cbk( struct hrtimer *timer )
{
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("NITESH: Timer Handler \n");
#endif

    /** Call any one of them (either Basic or Channel Load Measurement as per the DFS algorithm */
    uu_wlan_prepare_and_send_measurement_request(UU_WLAN_BASIC_MSR);
    //uu_wlan_prepare_and_send_measurement_request (UU_WLAN_CHANNEL_LOAD_MSR);

    /** TODO Call measurement preparation function */
#if 0
    ktime_t timeout, cur_time;
    unsigned long misses;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("NITESH: Timer Handler \n");
#endif

    if (exit_flag_g == 1)
    {
        return HRTIMER_NORESTART;
    }
    /* Calling UMAC to generate MEASUREMENT REQUEST */
    /* TODO: UU_LMAC_IND_GEN_MEASREQ I have defined in inc/uu_wlan_lmac_if.h alongwith other indications like beacon */

    timeout = ktime_set(0, MicroS_TO_NS(UU_REG_LMAC_MEASREQ_INRVL));
    cur_time = ktime_get();

    //misses = hrtimer_forward(timer, cur_time, timeout);
    return HRTIMER_NORESTART;
#endif
    return HRTIMER_NORESTART;
} /* uu_wlan_dfs_hrtimer_cbk */


uu_int32 uu_wlan_dfs_timer_start(uu_void)
{
    ktime_t ktime;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("NITESH: Timer starting \n");
    UU_WLAN_LOG_DEBUG(("LMAC: Measurement Request HR Timer module starting\n"));
#endif
    exit_flag_g = 0;
    ktime = ktime_set( 0, MicroS_TO_NS(UU_REG_LMAC_MEASREQ_INRVL) );

    hrtimer_init(&uu_wlan_dfs_hr_timer_g, CLOCK_MONOTONIC, HRTIMER_MODE_REL);

    uu_wlan_dfs_hr_timer_g.function = &uu_wlan_dfs_hrtimer_cbk;

#ifdef UU_WLAN_DFSTPC_DEBUG
    UU_WLAN_LOG_INFO(("Starting Measurement Req timer to fire in %dus (%ld)\n", UU_REG_LMAC_MEASREQ_INRVL, jiffies));
#endif

    hrtimer_start( &uu_wlan_dfs_hr_timer_g, ktime, HRTIMER_MODE_REL );

    return UU_SUCCESS;
} /* uu_wlan_dfs_timer_start */

uu_int32 uu_wlan_dfs_timer_stop(uu_void)
{
    int ret;

    exit_flag_g = 1;
    ret = hrtimer_cancel( &uu_wlan_dfs_hr_timer_g );
#ifdef UU_WLAN_DFSTPC_DEBUG
    if (ret)
    {
        UU_WLAN_LOG_DEBUG(("The Measurement Request timer was still in use...\n"));
    }
    UU_WLAN_LOG_DEBUG(("Measurement Request HR Timer module uninstalling\n"));
#endif
    return UU_SUCCESS;
} /* uu_wlan_dfs_timer_stop */

/* -------------------------- END OF TIMER FOR SENDING MEASUREMENT REQUEST FOR AP ------------------------------- */

/* ------------------------------ FORMING CHANNEL SWITCH ANNOUNCEMENT FRAME --------------------------------- */

uu_void uu_wlan_prepare_and_send_ch_switch_ann_frame (uu_uint8 new_channel)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
    struct sk_buff *skb;
    struct ieee80211_mgmt *csa_frame;
    struct ieee80211_tx_info *info;
    int i;
    static a = 0;
    if (a)
        return;
    a++;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Size of ieee80211_mgmt: %lu\n", sizeof(struct ieee80211_mgmt));
#endif

    skb = dev_alloc_skb(sizeof(struct ieee80211_mgmt) + sizeof(struct ieee80211_channel_sw_ie));
    if (!skb)
        return;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Ch_switch Frame of len 1: %d\n", skb->len);
#endif

    /** 24 is the number of octets of management frame format excluding HT_Control, Frame_body and FCS */
    csa_frame = (struct ieee80211_mgmt*)skb_put(skb, 24);
    /** Need to fill: frame_control, duration, da, sa, bssid, seq_ctrl, action (category,
                u (measurement (action_code, dialog_token, element_id, length, ieee80211_msrment_ie msr_elem))) */
    memset(csa_frame, 0, 24);
    csa_frame->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
    memcpy(csa_frame->da, uu_wlan_context_gp->assoc_sta, ETH_ALEN);
    memcpy(csa_frame->sa, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);
    memcpy(csa_frame->bssid, UU_WLAN_IEEE80211_STA_MAC_ADDR_R, ETH_ALEN);

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Source Address: %pM\nDestination Address: %pM\nBSSID: %pM\n",
            csa_frame->sa, csa_frame->da, csa_frame->bssid);
#endif

    /** 1 is added for category, refer to framework structure definition of ch_switch frame */
    skb_put(skb, 1 + sizeof(csa_frame->u.action.u.chan_switch));
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Action Channel Switch Frame of len 2: %d\n", skb->len);
#endif
    /**
     * TODO: Presently channel switch announcement frame is used as defined in the framework, but in framework
     * i) secondary channel offset element (mandatory for switching to 40 MHz),
     * ii) mesh channel switch parameters element (for mesh networks only),
     * iii) wide bandwidth channel switch element (802.11ac only),
     * iv) new VHT Transmit power envelope (optional, for 802.11ac only)
     * is not supported
     * Linux kernel 3.10 have started defining structures for these :)
     * This structure is as per linux-3.3.2
     */

    /** Channel Switch Announcement frame */
    /** Refer to 802.11 REV_mb D12, Section 8.5.2.6 */
    csa_frame->u.action.category = WLAN_CATEGORY_SPECTRUM_MGMT;
    csa_frame->u.action.u.chan_switch.action_code = WLAN_ACTION_SPCT_CHL_SWITCH;

    /** Channel Switch Announcement elements */
    /** Refer to 802.11 REV_mb D12, Section 8.4.2.21 */
    csa_frame->u.action.u.chan_switch.element_id = WLAN_EID_CHANNEL_SWITCH;
    csa_frame->u.action.u.chan_switch.length = 3;
    memset(&csa_frame->u.action.u.chan_switch.sw_elem, 0, sizeof(struct ieee80211_channel_sw_ie));
    csa_frame->u.action.u.chan_switch.sw_elem.mode = 1;
    csa_frame->u.action.u.chan_switch.sw_elem.new_ch_num = new_channel;
    csa_frame->u.action.u.chan_switch.sw_elem.count = 0;

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("UMAC_CHANNEL_SWITCH_FRAME\n\nCategory: %d\nAction Code: %d\nElementID: %d\n\n \
            Length: %d\nMode: %d\nNew Channel Number: %d\nCount: %d\n\n",
                    csa_frame->u.action.category,
                    csa_frame->u.action.u.chan_switch.action_code,
                    csa_frame->u.action.u.chan_switch.element_id,
                    csa_frame->u.action.u.chan_switch.length,
                    csa_frame->u.action.u.chan_switch.sw_elem.mode,
                    csa_frame->u.action.u.chan_switch.sw_elem.new_ch_num,
                    csa_frame->u.action.u.chan_switch.sw_elem.count);

    printk("CSA Action Frame of len 3:%d: ", skb->len);
    for (i = 0; i < skb->len; i++)
    {
        printk("%x ", skb->data[i]);
    }
    printk("\n ");
#endif

    skb_set_queue_mapping(skb, IEEE80211_AC_VO);
    skb->priority = 7;

    info = IEEE80211_SKB_CB(skb);
    info->flags |= IEEE80211_TX_INTFL_DONT_ENCRYPT;

    __send_frame(skb);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0) */
} /* uu_wlan_prepare_and_send_ch_switch_ann_frame */


/* --------------------------- END OF FORMING CHANNEL SWITCH ANNOUNCEMENT FRAME ----------------------------- */

#endif /* UU_WLAN_DFS */

/* EOF */

