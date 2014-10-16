/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_lmac_tpc.c                                     **
 **                                                                           **
 ** Copyright © 2013, Uurmi Systems                                           **
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
#include "uu_wlan_reg.h"
#include "uu_wlan_lmac_tpc.h"

#ifdef UU_WLAN_TPC
/**
 * Updating new tpc for the retransmission case
 *
 * When retransmission occurs and the frame is transmitted with
 * the previous frame rate then increase TPC for the frame.
 * After the retransmission is over, then we change the base TPC
 * as per the final histogram shown by Wideband AC TFAR Histogram.
 * Here the B_TPL remains the same.
 * Here, we increase the level, from
 *      l2 → l1
 *      l1 → n2
 *      n2 → n1
 *      n1 → m2
 *      m2 → m1
 *      m1 → h2
 *      h2 → h1
 *
 * Thus, for 11ac, it’s just same as adding 15 sub-levels to
 * the current Tx_power_level.
 * If we are on some sub-level, and after that the next 15th
 * sub-level is not supported or defined,
 * then we try to increase our sub-level by 5.
 * If even the 5th sub-level is not supported,
 * then we increase by 3.
 * If even that is not supported, then we increase
 * by 1. Else we keep the same.
 */
uu_uint8 uu_calc_tpl_retry_frame (uu_uint8 txpwr_level, uu_uint8 format,
                                    uu_uint8 modulation, uu_uint8 bndwdth)
{
#ifdef UU_WLAN_DFSTPC_DEBUG
	printk("uu_calc_tpl_retry_frame IN\n");
	printk("TPL RETRY in this function input tpc_level: %d, bandwidth: %d, format: %d, modulation: %d\n",
	        txpwr_level, bndwdth, format, modulation);
#endif
    uu_uint8 new_tpl;

    bndwdth = (bndwdth >= CBW160) ? CBW160 : bndwdth;

    /* First identify the maximum transmit power level */
    /* This will be valid for HT and OFDM */
    if (txpwr_level <= dot11NumberSupportedPowerLevelsImplemented_g)
    {
        new_tpl = ((txpwr_level + uu_wlan_retry_ofdm_tpl_inc_g) < uu_wlan_utpl_threshold_11n_g[bndwdth])
                                ? (txpwr_level + uu_wlan_retry_ofdm_tpl_inc_g) : uu_wlan_utpl_threshold_11n_g[bndwdth];
        /* If format == CCK, then assign the even tpl */
        if ((format == UU_WLAN_FRAME_FORMAT_NON_HT)
                & ((modulation == ERP_DSSS) || (modulation == ERP_CCK) || (modulation == ERP_PBCC)))
        {
            new_tpl = ((new_tpl  + (new_tpl%2)) < uu_wlan_utpl_threshold_11n_g[bndwdth])
                                ? (new_tpl  + (new_tpl%2)) : uu_wlan_utpl_threshold_11n_g[bndwdth];
        }
    }
    /* This will be valid for VHT */
    else
    {
        if ((txpwr_level + uu_wlan_retry_11ac_tpl_inc_g)
                                < uu_wlan_utpl_threshold_11ac_g[bndwdth])
        {
            new_tpl = txpwr_level + uu_wlan_retry_11ac_tpl_inc_g;
        }
        else if ((txpwr_level + (uu_wlan_retry_11ac_tpl_inc_g >> 1))
                                < uu_wlan_utpl_threshold_11ac_g[bndwdth])
        {
            new_tpl = txpwr_level + (uu_wlan_retry_11ac_tpl_inc_g >> 1);
        }
        else if ((txpwr_level + (uu_wlan_retry_11ac_tpl_inc_g >> 2))
                                < uu_wlan_utpl_threshold_11ac_g[bndwdth])
        {
            new_tpl = txpwr_level + (uu_wlan_retry_11ac_tpl_inc_g >> 2);
        }
        else
        {
            new_tpl = uu_wlan_utpl_threshold_11ac_g[bndwdth];
        }
    }

#ifdef UU_WLAN_DFSTPC_DEBUG
	printk("TPL RETRY in this function OUT returns :%d\n", new_tpl);
#endif
    return new_tpl;
} /* uu_calc_tpl_retry_frame */


/** Assigning the transmit power levels for the control frames
 * as per the bandwidth, format and modulation.
 * Sending the control packets with highest TPL since these are of less size */
uu_uint16 uu_calc_tpl_ctrl_frames (uu_uint8 ch_bndwdth, uu_uint8 format, uu_uint8 modulation)
{
    uu_uint8 ctrl_txpwr;
    uu_uint8 bandwidth;
	printk("uu_calc_tpl_ctrl_frames IN\n");
    bandwidth = (ch_bndwdth == CBW80_80) ? CBW160 : ch_bndwdth;
#ifdef UU_WLAN_DFSTPC_DEBUG
	    printk("Bandwidth: %d, Format: %d, Modulation: %d\n", bandwidth, format, modulation);
#endif

    if (uu_wlan_btpl_ac_g[0] <= dot11NumberSupportedPowerLevelsImplemented_g)
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
	    printk("For 2.4 GHz as dot11NumberSupportedPowerLevelsImplemented_g: %d\n", dot11NumberSupportedPowerLevelsImplemented_g);
#endif
        ctrl_txpwr = uu_wlan_utpl_threshold_11n_g[bandwidth];

        /* If format == CCK, then assign the even tpl */
        if ((format == UU_WLAN_FRAME_FORMAT_NON_HT)
                & ((modulation == ERP_DSSS) || (modulation == ERP_CCK) || (modulation == ERP_PBCC)))
        {
            /* Make the tx_power level even */
            ctrl_txpwr = ctrl_txpwr & 0xFE;
        }

    }
    /* For 5 GHz */
    else
    {
        if (format == UU_WLAN_FRAME_FORMAT_VHT)
        {
            ctrl_txpwr = uu_wlan_utpl_threshold_11ac_g[bandwidth];
        }
        else
        {
            ctrl_txpwr = uu_wlan_utpl_threshold_11n_g[bandwidth];
        }
    }

#ifdef UU_WLAN_DFSTPC_DEBUG
	printk("Txpwr_level for control frame is: %d\n", ctrl_txpwr);
#endif
    return ctrl_txpwr;
} /* uu_calc_tpl_ctrl_frames */

#endif /* UU_WLAN_TPC */

/* EOF */

