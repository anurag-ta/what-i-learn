/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_recv.c                                                **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                            **
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
/* $Revision: 1.22 $ */

#include <linux/version.h>
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_main.h"
#include "uu_wlan_recv.h"
#include "uu_wlan_rate.h"
#ifdef UU_WLAN_DFS
#include "uu_wlan_umac_dfs.h"
#endif
#ifdef UU_WLAN_TPC
#include "uu_wlan_umac_tpc.h"
#endif

/** Rx Status filling after getting frame from lmac. */
static uu_void uu_wlan_fill_rx_status(uu_wlan_rx_frame_info_t *recv_fi, uu_wlan_rx_status_t *rs)
{
    /* TODO: What is this empty function? */
    /*
    rs->datalen = UU_REG_RX_DATALEN;
    rs->status = UU_REG_RX_STATUS;
    rs->phyerr = UU_REG_RX_PHYERR;
    rs->rssi = recv_fi->rxvec.rssi;
    rs->keyix = recv_fi->frameInfo.keyix;
    rs->rate = UU_REG_RX_RATE;
    rs->antenna = UU_REG_RX_ANTENNA;
    rs->more = UU_REG_RX_MORE;
    rs->rssi_ctl0 = UU_REG_RX_RSSI_CT10;
    rs->rssi_ctl1 = UU_REG_RX_RSSI_CT11;
    rs->rssi_ctl2 = UU_REG_RX_RSSI_CT12;
    rs->rssi_ext0 = UU_REG_RX_RSSI_EXT0;
    rs->rssi_ext1 = UU_REG_RX_RX_RSSI_EXT1;
    rs->rssi_ext2 = UU_REG_RX_RX_RSSI_EXT2;
    rs->isaggr = UU_REG_RX_ISAGGR;
    rs->moreaggr = UU_REG_RX_MOREAGGR;
    rs->num_delims = UU_REG_RX_DELIMS;
    rs->flags = UU_REG_RX_FLAGS; //TODO fill the status registers from lmac
    UU_WLAN_LOG_DEBUG(("UURMIDRIVER:rx status in UMAC rs->datalen %x,rs->rate %x, rs->isaggr %x\n",rs->datalen, rs->rate, rs->isaggr));
    */
} /* uu_wlan_fill_rx_status */


static uu_void uu_wlan_fill_ieee80211_rx_status(struct ieee80211_hw *hw, struct ieee80211_rx_status *rx_status, uu_wlan_rx_frame_info_t *rx_fi)
{
    uu_wlan_rx_status_t rs;
    /*TODO memset done to remove warnings ,can be removed after initialising rs*/
    //memset(&rs,0,sizeof(uu_wlan_rx_status_t));

    /** Fill rx status structure here */
    uu_wlan_fill_rx_status(rx_fi,&rs);

    /* Filling ieee80211 rx_status structure */
    memset(rx_status, 0, sizeof(struct ieee80211_rx_status));

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
    rx_status->freq = hw->conf.channel->center_freq; //TODO - Fill this from PHY control register
    rx_status->band = hw->conf.channel->band; //TODO - Fill this from PHY control register
#else
    rx_status->freq = hw->conf.chandef.chan->center_freq; //TODO - Fill this from PHY control register
    rx_status->band = hw->conf.chandef.chan->band; //TODO - Fill this from PHY control register
#endif

    /* For non-HT */
    if (rx_fi->rxvec.format == UU_WLAN_FRAME_FORMAT_NON_HT)
    {
        rx_status->rate_idx = uu_get_rx_rate_idx (rx_fi->rxvec.L_datarate);
        /* At the station side enable this for testing tpc and dfs (reason:cck rates are not supported by station while operating at 5GHz)*/ 
        if (uu_dot11_op_mode_r == UU_WLAN_MODE_MANAGED)
        {
            if (rx_status->band)
            {
                rx_status->rate_idx -= 4;
            }
        }
    }
    else /* For HT, VHT */
    {
        rx_status->rate_idx = rx_fi->rxvec.mcs;

        if (rx_fi->rxvec.format == UU_WLAN_FRAME_FORMAT_HT_MF)
            rx_status->flag |= RX_FLAG_HT;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
        if (rx_fi->rxvec.format == UU_WLAN_FRAME_FORMAT_HT_GF)
            rx_status->flag |= RX_FLAG_HT_GF;

        if (rx_fi->rxvec.format == UU_WLAN_FRAME_FORMAT_VHT)
        {
            rx_status->flag |= RX_FLAG_VHT;
            rx_status->vht_nss = rx_fi->rxvec.num_sts;
        }
#else
        if (rx_fi->rxvec.format == UU_WLAN_FRAME_FORMAT_HT_GF)
            rx_status->flag |= RX_FLAG_HT;
#endif
        if (rx_fi->rxvec.is_short_GI)
            rx_status->flag |= RX_FLAG_SHORT_GI;

    }

    rx_status->antenna = rs.antenna; //TODO - Fill this from PHY control register
    /** filling signal by reading rxvector which is passed from PHY to LMAC */
    rx_status->signal = rx_fi->rxvec.rssi;

    /* Filling rates in rx_status flags */
    if(rs.flags & UU_WLAN_RX_FLAG_MMIC_ERROR)
        rx_status->flag |=  RX_FLAG_MMIC_ERROR;

    if(rs.flags & UU_WLAN_RX_FLAG_DECRYPTED)
        rx_status->flag |= RX_FLAG_DECRYPTED;

    if(rs.flags & UU_WLAN_RX_FLAG_MMIC_STRIPPED)
        rx_status->flag |= RX_FLAG_MMIC_STRIPPED;
    if(rs.flags & UU_WLAN_RX_FLAG_IV_STRIPPED)
        rx_status->flag |= RX_FLAG_IV_STRIPPED;

    if(rs.flags & UU_WLAN_RX_FLAG_FAILED_FCS_CRC)
        rx_status->flag |= RX_FLAG_FAILED_FCS_CRC;

    if(rs.flags & UU_WLAN_RX_FLAG_FAILED_PLCP_CRC)
        rx_status->flag |= RX_FLAG_FAILED_PLCP_CRC;

    if(rs.flags & UU_WLAN_RX_FLAG_SHORTPRE)
        rx_status->flag |= RX_FLAG_SHORTPRE;

#ifdef UU_WLAN_TSF
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
    if(rs.flags & UU_WLAN_RX_FLAG_MACTIME_MPDU)
        rx_status->flag |= RX_FLAG_MACTIME_START;
#else
    if(rs.flags & UU_WLAN_RX_FLAG_MACTIME_MPDU)
        rx_status->flag |= RX_FLAG_MACTIME_START;
#endif
    /* Since mactime represents the timestamp when the first octet of the MPDU was received in the hardware.
     * Here we decrement the rx_start_of_frame_offset, this is required because framework has no mechanism to
     * consider the PHY delay for synchronization purpose. */
    rx_status->mactime = rx_fi->timestamp_msb;
    rx_status->mactime = (rx_status->mactime << 32) | (rx_fi->timestamp_lsb);
    rx_status->mactime -= (uu_uint32) UU_WLAN_10NS_TO_US(rx_fi->rxvec.rx_start_of_frame_offset);
#endif

    if(rx_fi->rxvec.ch_bndwdth == CBW40)
        rx_status->flag |= RX_FLAG_40MHZ;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
    if(rx_fi->rxvec.ch_bndwdth == CBW80)
        rx_status->flag |= RX_FLAG_80MHZ;

    if(rx_fi->rxvec.ch_bndwdth == CBW80_80)
        rx_status->flag |= RX_FLAG_80P80MHZ;

    if(rx_fi->rxvec.ch_bndwdth == CBW160)
        rx_status->flag |= RX_FLAG_160MHZ;
#endif
    //align_bytes = (rx_fi->frameInfo.framelen % 4);
} /* uu_wlan_fill_ieee80211_rx_status */


struct sk_buff* uu_wlan_rx_fill_fwk_skb(struct ieee80211_hw *hw,uu_wlan_rx_frame_info_t *rx_fi, uu_uint8 index)
{
    struct sk_buff *skb = NULL;
    uu_uchar* mpdu;
    uu_char* addr;
    uu_int32 framelen,align_bytes = 0;
    uu_wlan_rx_status_info_t rx_status_info;
    uu_uint32 frameinfo_len = sizeof(uu_wlan_rx_vector_t) + sizeof(uu_frame_details_t);

#ifdef UU_WLAN_TSF
    frameinfo_len += sizeof(rx_fi->timestamp_msb) + sizeof(rx_fi->timestamp_lsb);
    frameinfo_len += sizeof(rx_fi->becn_timestamp_msb_tsf) + sizeof(rx_fi->becn_timestamp_lsb_tsf);
#endif

    uu_wlan_get_rx_status_info_platform(&rx_status_info, index);
    addr = uu_wlan_rx_buff_base_addr_reg_g + rx_status_info.offset;
    framelen = (rx_status_info.len - frameinfo_len);

    UU_WLAN_LOG_DEBUG(("Flag %d is set and reading address and length %d \n", index , rx_status_info.len));

    uu_wlan_get_data_platform((uu_char*)rx_fi, addr, frameinfo_len);

    mpdu = addr + frameinfo_len;

    align_bytes = (framelen % 4);

    skb = __dev_alloc_skb((framelen + align_bytes + sizeof(struct ieee80211_rx_status)), GFP_ATOMIC);

    skb_reserve(skb, sizeof(struct ieee80211_rx_status));

    /* Clear the status bit after taking the rx frame */
    /*TODO check whether this is correct?*/
    uu_wlan_clear_rx_status_flag_platform(index);
    //uu_setReg(UU_WLAN_RX_STATUS_REG,(uu_getReg(UU_WLAN_RX_STATUS_REG) & ~(1 << index))); /* TODO */
    uu_wlan_get_data_platform(skb_put(skb, framelen), mpdu,framelen);

    return skb;
} /* uu_wlan_rx_fill_fwk_skb */


static uu_void uu_wlan_rx_send_skb(struct ieee80211_hw *hw, struct sk_buff *skb,uu_wlan_rx_frame_info_t *rx_fi,struct ieee80211_rx_status *rx_status)
{
    /* Filling ieee80211 rx status */
    uu_wlan_fill_ieee80211_rx_status(hw,rx_status,rx_fi);
    memcpy(IEEE80211_SKB_RXCB(skb), rx_status, sizeof(struct ieee80211_rx_status));
    ieee80211_rx_irqsafe(hw,skb);
}


uu_int32 uu_wlan_rx_tasklet(struct ieee80211_hw *hw)
{
    struct sk_buff *skb;
    struct ieee80211_hdr *hdr;
    uu_wlan_rx_frame_info_t rx_fi;
    struct ieee80211_rx_status rx_status;
    uu_uint8 index;
    uu_uint32 status;
    /*TODO check for c model is this correct?*/  
    //uu_uint32 rxstatus =uu_getReg(UU_WLAN_RX_STATUS_REG);

    UU_WLAN_LOG_DEBUG(("In rx_tasklet function \n"));
    for(index=0; index<UU_WLAN_MAX_RX_STATUS_TO_UMAC; index++)
    {
        status = uu_wlan_get_rx_status_platform(index);
        if(status)
        {
            /* Fill skb from LMAC data */
            skb = uu_wlan_rx_fill_fwk_skb(hw, &rx_fi, index);

            hdr = (struct ieee80211_hdr *)skb->data;
            if(hdr)
            {
                if(ieee80211_is_beacon(hdr->frame_control))
                {
                    UU_WLAN_LOG_DEBUG(("UURMIDRIVER - Rcvd frame is beacon frame from %x:%x:%x:%x:%x:%x \n\n",hdr->addr2[0],hdr->addr2[1],hdr->addr2[2],hdr->addr2[3],hdr->addr2[4],hdr->addr2[5]));
                }
            }

#ifdef UU_WLAN_TPC
            /* NOTE: Checking for measurement report */
            if (ieee80211_is_action(hdr->frame_control))
            {
#ifdef UU_WLAN_DFSTPC_DEBUG
                printk("YESSSS !! ACTION FRAME\n");
#endif
                uu_mgmt_action_frame_t *mgmt_frame = (uu_mgmt_action_frame_t *) skb->data;
                if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
                {
                    if (mgmt_frame->uu_meas_frame.u.tpc_rep_element.element_id == WLAN_EID_TPC_REPORT &&
                            mgmt_frame->uu_meas_frame.mgmt_action == WLAN_ACTION_SPCT_TPC_RPRT &&
                            mgmt_frame->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT)
                    {
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("YESSSS !! UURMI TPC REPORT RECEIVED\n");
#endif
                        int i;
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("TPC report frame received in umac/src/uu_wlan_recv.c : \n");

                        for (i = 0; i < skb->len; i++)
                        {
                            printk("%x ", skb->data[i]);
                        }
                        printk("\n");
#endif
                        uu_wlan_process_tpc_report(mgmt_frame, rx_fi.rxvec.rssi);
                    }
                }

                if (uu_dot11_op_mode_r == UU_WLAN_MODE_MANAGED)
                {
                    if (mgmt_frame->uu_meas_frame.u.tpc_req_element.element_id == WLAN_EID_TPC_REQUEST &&
                            mgmt_frame->uu_meas_frame.mgmt_action == WLAN_ACTION_SPCT_TPC_REQ &&
                            mgmt_frame->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT)
                    {
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("YESSSS !! UURMI TPC REQUEST RECEIVED\n");
#endif
                        int i;
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("TPC request frame received in umac/src/uu_wlan_recv.c : \n");
                        for (i = 0; i < skb->len; i++)
                        {
                            printk("%x ", skb->data[i]);
                        }
                        printk("\n");
#endif
                        /* Preparing and sending TPC Report */
                        uu_wlan_process_and_send_tpc_report (mgmt_frame, &rx_fi);
                    }
                }
            }
#endif

#ifdef UU_WLAN_DFS
            /** NOTE: Checking for measurement report */
            if (ieee80211_is_action(hdr->frame_control))
            {
#ifdef UU_WLAN_DFSTPC_DEBUG
                printk("YESSSS !! ACTION FRAME\n");
#endif
                uu_mgmt_action_frame_t *mgmt_frame = (uu_mgmt_action_frame_t *) skb->data;
                struct ieee80211_mgmt *csa_frame = (struct ieee80211_mgmt *) skb->data;

                /* At AP, if measurement report is received. */
                if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
                {

                    if (mgmt_frame->uu_meas_frame.u.meas_element.element_id == WLAN_EID_MEASURE_REPORT)
                    {
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("YESSSS !! UURMI MEASUREMENT REPORT RECEIVED\n");
#endif
                        int i;
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("Measurement report frame received in umac/src/uu_wlan_recv.c : \n");
                        for (i = 0; i < skb->len; i++)
                        {
                            printk("%x ", skb->data[i]);
                        }
                        printk("\n");
#endif
                        /* If measurement report contains some information */
                        if (!(mgmt_frame->uu_meas_frame.u.meas_element.mode.meas_report_mode))
                        {
                            /* AP receives basic report */
                            if (mgmt_frame->uu_meas_frame.u.meas_element.meas_type == IEEE80211_SPCT_MSR_RPRT_TYPE_BASIC)
                            {
#ifdef UU_WLAN_DFSTPC_DEBUG
                                printk("AP received BASIC REPORT\n");
#endif
                                /* This is for channel switching at STA side */
                                if (mgmt_frame->uu_meas_frame.u.meas_element.u.report.basic_report.map.radar)
                                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                                    printk("Radar detected on the channel: %d\n",
                                            mgmt_frame->uu_meas_frame.u.meas_element.u.report.basic_report.basic_msr.channel_number);
#endif
                                    /* TODO: From here we call our DFS algorithm for the next best channel to switch to. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
                                    uu_wlan_prepare_and_send_ch_switch_ann_frame(
                                            (mgmt_frame->uu_meas_frame.u.meas_element.u.report.basic_report.basic_msr.channel_number) + 4);
#endif
                                }
                            }

                            else if (mgmt_frame->uu_meas_frame.u.meas_element.meas_type == UU_WLAN_RADIO_MSR_RPRT_TYPE_CHAN_LOAD)
                            {
#ifdef UU_WLAN_DFSTPC_DEBUG
                                printk("AP received CHANNEL LOAD REPORT\n");
#endif
                            }
                        }
                        /* Else, if the measurement report is refused, late or incapable */
                        else
                        {
#ifdef UU_WLAN_DFSTPC_DEBUG
                            printk("The measurement report is either late, or refused or incapable.");
                            /* TODO: if it is incapable, then we must not send that measurement req type again to that STA */
#endif
                        }
                    }
                }

                /* At STA side */
                if (uu_dot11_op_mode_r == UU_WLAN_MODE_MANAGED)
                {
                    if (((mgmt_frame->uu_meas_frame.u.meas_element.element_id == WLAN_EID_MEASURE_REQUEST)
                                    && (mgmt_frame->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT))
                        || ((mgmt_frame->uu_meas_frame.u.radio_req_element.rreq_element.element_id == WLAN_EID_MEASURE_REQUEST)
                                    && (mgmt_frame->uu_meas_frame.category == UU_WLAN_CATEGORY_RADIO_MGMT)))
                    {
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("YESSSS !! UURMI MEASUREMENT REQUEST RECEIVED\n");
#endif
                        int i;
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("Measurement request frame received in umac/src/uu_wlan_recv.c : \n");
                        for (i = 0; i < skb->len; i++)
                        {
                            printk("%x ", skb->data[i]);
                        }
                        printk("\n");
#endif
                        /* Then process the measurement report */
                        uu_wlan_process_and_send_measurement_report (mgmt_frame);
                    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
                    if (csa_frame->u.action.u.chan_switch.element_id == WLAN_EID_CHANNEL_SWITCH)
                    {
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("YESSSS !! UURMI CHANNEL SWITCH ANNOUNCEMENT FRAME RECEIVED\n");
#endif
                        int i;
#ifdef UU_WLAN_DFSTPC_DEBUG
                        printk("Channel Switch Announcement frame received in umac/src/uu_wlan_recv.c : \n");
                        for (i = 0; i < skb->len; i++)
                        {
                            printk("%x ", skb->data[i]);
                        }
                        printk("\n");
#endif
                    }
#endif
                }
            }
#endif /* UU_WLAN_TPC */

#if 0
            /* Just for debugging. Not valid for release. */
            if ((uu_uint32)skb->data & 1)
            {
                UU_WLAN_LOG_DEBUG(("UURMIDRIVER: Warning ------------- unaligned packet at 0x%p\n", skb->data));
                UU_WLAN_LOG_DEBUG(("*****AVOIDING PREDICTED CRASH*****skb Data pkt recved. Dropped due to alignment \n"));
                dev_kfree_skb_any(skb);
                return -1;
            }
#endif /* #if 0 */

#ifdef UU_WLAN_TPC
            if (uu_dot11_op_mode_r == UU_WLAN_MODE_MANAGED)
            {
                /* For Power Constraint - It is already taken care by Framework,
                   but need to enable the spectrum support in hostapd. Also it is taken care
                   in wrong way, it should take reference of max_power from Country element
                   rather than taking reference from the structure in framework. */

                /* For VHT Tx Power Envelope */
                int tpc_level_dBm[CBW80_80] = {0};
                int i, j, max_cbwdth;

               /* Instead of Prev_TPC, keep for prev_TPC as per VHT Tx Pwr Envelope. */

                if (ieee80211_is_beacon (hdr->frame_control))
                {
                    /* TODO: This is just for testing purpose. It should be handled properly
                       for extracting this element from the beacon frames */
                    /* Get the byte where the element id of VHT Tx Pwr Envelope is present */
                    for (i = 0; i < skb->len; i++)
                    {
                        if (skb->data[i] == 195)
                        {
                            break;
                        }
                    }

                    /* Number of iterations = Value represented by length of VHT Tx Pwr Envelope - 1 */
                    max_cbwdth = (skb->data[i + 1]) - 1;

                    for (j = 0; j < max_cbwdth; j++)
                    {
                        tpc_level_dBm[j] = skb->data[i + 3 + j] / 2;
                        if (prev_tpc_dBm[j] != tpc_level_dBm[j])
                        {
#ifdef UU_WLAN_DFSTPC_DEBUG
                            printk("TPC Level gets changed\n");
#endif
                            prev_tpc_dBm[j] = tpc_level_dBm[j];
                            uu_wlan_configure_utpl_threshold (prev_tpc_dBm[j], j);
                        }
                    }
                }
            }

            /* Not letting the TPC request/report frame to pass to the framework.
               Later if the support is added then we can pass this. */
            if (ieee80211_is_action(hdr->frame_control))
            {
                uu_mgmt_action_frame_t *mgmt_frame = (uu_mgmt_action_frame_t *) skb->data;
                if (mgmt_frame->uu_meas_frame.u.tpc_rep_element.element_id == WLAN_EID_TPC_REPORT &&
                        mgmt_frame->uu_meas_frame.mgmt_action == WLAN_ACTION_SPCT_TPC_RPRT &&
                        mgmt_frame->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT)
                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("TPC REPORT NOT being passed to FRAMEWORK\n");
#endif
                    return 0;
                }
                if (mgmt_frame->uu_meas_frame.u.tpc_req_element.element_id == WLAN_EID_TPC_REQUEST &&
                        mgmt_frame->uu_meas_frame.mgmt_action == WLAN_ACTION_SPCT_TPC_REQ &&
                        mgmt_frame->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT)
                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("TPC REQUEST NOT being passed to FRAMEWORK\n");
#endif
                    return 0;
                }
            }
#endif /* UU_WLAN_TPC */

#ifdef UU_WLAN_DFS
            /* Not letting pass the measurement request frame to the framework.
               Later if the support is added then we can pass this. */
            if (ieee80211_is_action(hdr->frame_control))
            {
                uu_mgmt_action_frame_t *mgmt_frame = (uu_mgmt_action_frame_t *) skb->data;
                if (mgmt_frame->uu_meas_frame.u.meas_element.element_id == WLAN_EID_MEASURE_REPORT
                    && ((mgmt_frame->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT
                    && mgmt_frame->uu_meas_frame.mgmt_action == WLAN_ACTION_SPCT_MSR_RPRT
                    && mgmt_frame->uu_meas_frame.u.meas_element.meas_type == IEEE80211_SPCT_MSR_RPRT_TYPE_BASIC)
                        || (mgmt_frame->uu_meas_frame.category == UU_WLAN_CATEGORY_RADIO_MGMT
                            && mgmt_frame->uu_meas_frame.mgmt_action == UU_WLAN_ACTION_RADIO_MSR_RPRT
                            && mgmt_frame->uu_meas_frame.u.meas_element.meas_type == UU_WLAN_RADIO_MSR_RPRT_TYPE_CHAN_LOAD)))
                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("DFS REPORT NOT being passing to FRAMEWORK\n");
#endif
                    return 0;
                }
                if (((mgmt_frame->uu_meas_frame.u.meas_element.element_id == WLAN_EID_MEASURE_REQUEST)
                                    && (mgmt_frame->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT))
                        || ((mgmt_frame->uu_meas_frame.u.radio_req_element.rreq_element.element_id == WLAN_EID_MEASURE_REQUEST)
                                    && (mgmt_frame->uu_meas_frame.category == UU_WLAN_CATEGORY_RADIO_MGMT)))
                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("DFS REQUEST NOT being passing to FRAMEWORK\n");
#endif
                    return 0;
                }
            }
#endif /* UU_WLAN_DFS */
            /* Sending skb to framework */
            uu_wlan_rx_send_skb(hw, skb, &rx_fi, &rx_status);
        }
    }
    return 0;
} /* uu_wlan_rx_tasklet */


uu_int32 uu_wlan_rx_setfilters(uu_uint32 total_flags)
{
    uu_int32 rx_filter = 0;
    uu_int32 filters = 0;
    if(total_flags & FIF_CONTROL)
    {
        filters = UU_WLAN_FILTR_CTL_FRAMES ;
        rx_filter |= FIF_CONTROL;
    }
    if (total_flags & FIF_PROMISC_IN_BSS)
    {
        rx_filter |= FIF_PROMISC_IN_BSS;
    }
    if (total_flags & FIF_ALLMULTI)
    {
        rx_filter |= FIF_ALLMULTI;
    }

#if 0  /** Enable this after adding the support in both LMAC and UMAC */
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_BEACON ;
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_PROBE_RESP ;
    if(!(total_flags & FIF_PSPOLL))
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_PSPOLL ;
    if(!(total_flags & FIF_PROBE_REQ))
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_PROBE_REQ ;
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_ALLOW_RTS_CTS ;
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_BROAD_CAST ;
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_DROP_ALL ;
    UU_REG_LMAC_FILTER_FLAG = UU_WLAN_FILTR_CTL_FRAMES | UU_WLAN_FILTR_PROBE_RESP | UU_WLAN_FILTR_PSPOLL | UU_WLAN_FILTR_PROBE_REQ | UU_WLAN_FILTR_ALLOW_RTS_CTS | UU_WLAN_FILTR_BROAD_CAST;
#endif

    //UU_REG_LMAC_FILTER_FLAG = UU_WLAN_FILTR_NO_FILTER;
    uu_setReg(UU_REG_LMAC_FILTER_FLAG,UU_WLAN_FILTR_NO_FILTER);
    return rx_filter;
} /* uu_wlan_setfilters */


/* EOF */

