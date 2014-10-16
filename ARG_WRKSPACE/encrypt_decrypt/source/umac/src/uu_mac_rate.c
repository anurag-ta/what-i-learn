/*******************************************************************************
**                                                                            **
** File name :  uu_mac_rate.c                                                   **
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

#include "uu_datatypes.h"
#include "uu_wlan_limits.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_rate.h"
#include "uu_mac_rate.h"
#include "uu_wlan_main.h"
#ifdef UU_WLAN_DFS
#include "uu_wlan_umac_dfs.h"
#endif
#ifdef UU_WLAN_TPC
#include "uu_wlan_umac_tpc.h"
#endif

#ifdef UU_WLAN_TPC

/** This variables are to store the previous followed channel and channel bandwidth */
static int channel_hwval = 0;
static int channel_bdwdth = 0;

/** This variable is to store the previous value of max_power supported by
 * the regulatory domain in that corresponding channel
 */
int prev_tpc_dBm[CBW80_80] = {-1, -1, -1, -1};
static int not_first_tx = 0;

/** This is when the framework changes the mcs (ht/vht), so we need to
 * check if the new rate is for sample data or for normal data.
 *
 * We check only if the mcs has increased, then we increase the TPC.
 * This is because, framework might have got nice throughput with
 * the new rate and the temp_tpc for that rate.
 *
 * But when the mcs or nss decreases, we won't considering changing
 * TPC because framework might have got nice throughput for the new
 * mcs with the present base TPC level.
 *
 */

static int prev_htmcs = -1;
static int prev_vhtmcs = -1;

static int count_htmcs = 0;
static int count_vhtmcs = 0;

static int normal_htmcs = -1;
static int normal_vhtmcs = -1;

/** Parameters to store the tpc value for new mcs attempted */
static uu_uint8 ht_tpc_temp = 0;
static uu_uint8 vht_tpc_temp = 0;

#endif /* UU_WLAN_TPC */

#ifdef UU_WLAN_DFS
/* For DFS Channel switching in AP side */
static int ch_switch_count = 0;
static int temp_count = 0;
static int ch_switch_freq;
static int ch_switch_hw_value;
#endif /* UU_WLAN_DFS */



/* -----------------------------------Receiving Side-------------------------------- */

/**
 * Get rate_index as per the L_datarate registered to the framework for 2.4 GHz.
 */
uu_uint8 uu_get_rx_rate_idx (uu_uint8 L_datarate)
{
    uu_uint8 rate_idx;
    if (L_datarate == UU_HW_RATE_54M)
    {
        rate_idx = 11;
    }
    else if (L_datarate == UU_HW_RATE_48M)
    {
        rate_idx = 10;
    }
    else if (L_datarate == UU_HW_RATE_36M)
    {
        rate_idx = 9;
    }
    else if (L_datarate == UU_HW_RATE_24M)
    {
        rate_idx = 8;
    }
    else if (L_datarate == UU_HW_RATE_18M)
    {
        rate_idx = 7;
    }
    else if (L_datarate == UU_HW_RATE_12M)
    {
        rate_idx = 6;
    }
    else if (L_datarate == UU_HW_RATE_9M)
    {
        rate_idx = 5;
    }
    else if (L_datarate == UU_HW_RATE_6M)
    {
        rate_idx = 4;
    }
    else if (L_datarate == UU_HW_RATE_11M)
    {
        rate_idx = 3;
    }
    else if (L_datarate == UU_HW_RATE_5M5)
    {
        rate_idx = 2;
    }
    else if (L_datarate == UU_HW_RATE_2M)
    {
        rate_idx = 1;
    }
    else /* UU_HW_RATE_1M */
    {
        rate_idx = 0;
    }

    return rate_idx;
} /* uu_get_rx_rate_idx */


/* -----------------------------------Transmitting Side-------------------------------- */

/* For STBC */

/* As per table 8-124, STBC will be 0/1 for Tx and 0/1/2/3 for Rx STBC */

/* Notes : in HT Capabilities, section 8.4.2.58
 * In Rx STBC, it is of 2 bits, which is being updates to the flag in minstrel rate control
 * algorithm in framework.
 * 0 -> no STBC support
 * 1 -> 1 stbc support
 * 2 -> 1 and 2 stbc support
 * 3 -> 1, 2 and 3 stbc support
 *
 * And for VHT in 8.4.2.160
 *  it is of 3 bits
 * 0 -> no STBC support
 * 1 -> 1 STBC support
 * 2 -> 1 and 2 stbc support
 * 3 -> 1, 2 and 3 stbc support
 * 4 -> 1, 2, 3 and 4 stbc support
 * Others are reserved
 */
/* We need to make it 3 bits, for extending support for VHT */

/* get the input frame format (HT/VHT) and number of spatial streams and stbc as per HT/VHT Capabilities RX STBC */
/* For number of spatial streams = 1, n_ss should be 0. stbc as per the specs. */
static uu_uint8 uu_get_stbc(uu_wlan_frame_format_t frame_format, int nss, int stbc)
{
    /* nss ranges from 1 to 4 (HT) or 8(VHT) */
    /* Return value of the stbc for tx_vector */
    uu_uint8 result_stbc;

    if (frame_format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        /* For VHT, stbc is 1 only for Nss <= 4
           For others its 0
         */
        if ((nss <= stbc) && ((nss * 2) <= UU_PHY_MAX_STREAM_VHT))
        {
            result_stbc = 1;
        }
        else
        {
            result_stbc = 0;
        }
    }
    else /* for HT case */
    {
        /* HT cases
           Nss     stbc    result_stbc
           1       >=1     1
           2       =2      1
           2       >2      2
           3       >=3     1
           4       any     0
           other cases     0
         */
        /* for Nss = 2 and stbc should support more than 1 extra streams, i.e. stbc = 3, then we support 2 stbc in this case for Nss = 2 */
        if ((nss == 2) && (stbc > nss))
        {
            result_stbc = 2;
        }
        /* This is for all other cases in HT */
        else if ((nss <= stbc) && (nss < 4))
        {
            result_stbc = 1;
        }
        else
        {
            result_stbc = 0;
        }
        /* Just checking condition that it should not exceed max streams supported in PHY */
        if ((nss + result_stbc) > UU_PHY_MAX_STREAM_HT)
        {
            result_stbc = 0;
        }
    }

    return result_stbc;
} /* uu_get_stbc */


#ifdef UU_WLAN_TPC
#ifdef UU_WLAN_DFSTPC_DEBUG
/** This is only for debugging purpose */
static uu_void uu_print_tpc_values(uu_void)
{
    int i, j;

    printk("dot11NumberSupportedPowerLevelsImplemented_g : %d\n", dot11NumberSupportedPowerLevelsImplemented_g);
    printk("dot11TxPowerLevelExtended : %d\n", dot11TxPowerLevelExtended);

    printk("uu_wlan_tp_level_diff_g : %d\n", uu_wlan_tp_level_diff_g);
    printk("uu_wlan_retry_11ac_tpl_inc_g : %d\n", uu_wlan_retry_11ac_tpl_inc_g);
    printk("uu_wlan_retry_ofdm_tpl_inc_g : %d\n", uu_wlan_retry_ofdm_tpl_inc_g);

    printk("Printing uu_wlan_btpl_ac_g \n");
    for (i = 0; i < 4; i ++)
    {
        for (j = 0; j < 4; j ++)
        {
            printk("%u\t", uu_wlan_btpl_ac_g[(i * UU_WLAN_AC_MAX_AC) + j]);
        }
        printk("\n");
    }

    printk("Printing uu_wlan_firstframe_multiprot \n");
    for (i = 0; i < 4; i ++)
    {
            printk("%u\t", uu_wlan_firstframe_multiprot[i]);
    }
    printk("\n");

    printk("Printing uu_wlan_highpload_datamgmt_ltpl \n");
    for (i = 0; i < 4; i ++)
    {
        for (j = 0; j < 4; j ++)
        {
            printk("%u\t", uu_wlan_highpload_datamgmt_ltpl[(i * UU_WLAN_AC_MAX_AC) + j]);
        }
        printk("\n");
    }

    printk("Printing uu_wlan_lowpload_datamgmt_ltpl \n");
    for (i = 0; i < 4; i ++)
    {
        for (j = 0; j < 4; j ++)
        {
            printk("%u\t", uu_wlan_lowpload_datamgmt_ltpl[(i * UU_WLAN_AC_MAX_AC) + j]);
        }
        printk("\n");
    }

    printk("Printing uu_wlan_transmit_power_dBm \n");
    for (i = 1; i < 129; i++)
    {
        printk("%d\t", uu_wlan_transmit_power_dBm[i]);
    }
        printk("\n");
        printk("\n");
} /* uu_print_tpc_values */
#endif /* UU_WLAN_DFSTPC_DEBUG */


uu_void uu_wlan_setup_frame_tpc (struct sk_buff *skb, uu_wlan_tx_frame_info_t *fi, struct ieee80211_hw *hw)
{
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
    int i;

    int ac = uu_wlan_umac_get_ac (skb->queue_mapping);

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("fi->txvec.format : %d\n", fi->txvec.format);
    printk("fi->txvec.modulation : %d\n", fi->txvec.modulation);
#endif

    if (ieee80211_is_beacon(hdr->frame_control))
    {
        //fi->txvec.txpwr_level = hw->conf.channel->max_power;
        fi->txvec.txpwr_level = 0;
        printk("Sending beacon with TPC: %d with power in dBm: %d\n", fi->txvec.txpwr_level,
                uu_wlan_transmit_power_dBm[fi->txvec.txpwr_level]);
        return;
    }

    int bdwdth;
    uu_uint8 frame_tpl;
    uu_uint8 frame_ltpl;
    uu_uint8 ht_mcs;    /** MCS index and Ntx included */
    uu_uint8 vht_mcs;   /** MCS index and Ntx included */
#ifdef UU_WLAN_DFSTPC_DEBUG
    if (ieee80211_is_data(hdr->frame_control))
    {
        printk("NITESH_AC: %d\n", ac);
    }
#endif
    bdwdth = (fi->txvec.ch_bndwdth == CBW80_80) ? CBW160 : (fi->txvec.ch_bndwdth);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("The channel bandwidth is: %d, and the considered bandwidth for TPC is: %d\n", fi->txvec.ch_bndwdth, bdwdth);
#endif
    frame_tpl = uu_wlan_btpl_ac_g[(bdwdth * UU_WLAN_AC_MAX_AC) + ac];
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("The TPC level for the frame(1) is: %d\n", frame_tpl);
#endif
    /** In this condition, we have frame_tpl value >= 9 for 5 GHz and < 9 for 2.4 GHz */
    if (ieee80211_is_data(hdr->frame_control))
    {
        if (fi->txvec.format == UU_WLAN_FRAME_FORMAT_HT_GF || fi->txvec.format == UU_WLAN_FRAME_FORMAT_HT_MF)
        {
            /* Reason for selecting ht_mcs as mcs index (0-7) and n_tx is to
             * take care of STBC also */
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Data frames (uu_mac_tx.c) for HT. MCS is: %d, N_tx is: %d\n", fi->txvec.mcs, fi->txvec.n_tx);
#endif
            ht_mcs = fi->txvec.n_tx;
            ht_mcs = (ht_mcs << 3) | (fi->txvec.mcs & UU_HT_MANDATORY_MCS_MASK);

#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Present mcs is: %d,  previous mcs was: %d, normal mcs is: %d\n", ht_mcs, prev_htmcs, normal_htmcs);
#endif
            if (prev_htmcs >= 0)
            {
                /** Getting the first normal mcs index */
                if (normal_htmcs < 0)
                {
                    if (ht_mcs == prev_htmcs)
                    {
                        count_htmcs++;
                        /** Found normal mcs */
                        if (count_htmcs == UU_WLAN_CONST_RATE_COUNT)
                        {
#ifdef UU_WLAN_DFSTPC_DEBUG
                            printk("normal mcs is found for first time: %d\n", normal_htmcs);
#endif
                            normal_htmcs = ht_mcs;
                            count_htmcs = 0;
                        }
                    }
                    /** Restart count_mcs when new mcs is found and normal mcs is not yet identified */
                    {
                        count_htmcs = 0;
                    }
                }
                /** when first normal mcs is already found */
                else
                {
                    if (ht_mcs != normal_htmcs)
                    {
                        /** Means, new mcs is repeated, so might be normal mcs has changed */
                        if (ht_mcs == prev_htmcs)
                        {
                            /** Getting new normal mcs */
                            count_htmcs ++;
                            frame_tpl = ht_tpc_temp; /** Assigning ht_tpc_temp, its already calculated in previous frame tx */
                            if (count_htmcs == UU_WLAN_CONST_RATE_COUNT)
                            {
                                normal_htmcs = ht_mcs;
                                count_htmcs = 0;
#ifdef UU_WLAN_DFSTPC_DEBUG
                            printk("normal mcs is found not for the first time: %d\n", normal_htmcs);
#endif
                                /** Increment B_TPL as per ht_tpc_temp */
                                /** TODO */
                                uu_update_btpl_new_htmcs_all(ht_mcs, normal_htmcs);
                                /** Decrementing of TPL will be taken care by WAC-TFAR */
                            }
                        }
                        else
                        {
                            if (ht_mcs > normal_htmcs)
                            {
#ifdef UU_WLAN_DFSTPC_DEBUG
                            printk("new mcs is greater than normal mcs, so trying new temp tpl\n");
#endif
                                /** Assign ht_tpc_temp for the first time */
                                ht_tpc_temp = uu_increase_tpl_new_htmcs (ht_mcs, normal_htmcs, bdwdth,
                                                            uu_wlan_btpl_ac_g[(bdwdth * UU_WLAN_AC_MAX_AC) + ac]);
                                frame_tpl = ht_tpc_temp;
                            }
                            /** If mcs < normal_htmcs, no change in TPC */
                            else
                            {
                                ht_tpc_temp = uu_wlan_btpl_ac_g[(bdwdth * UU_WLAN_AC_MAX_AC) + ac];
                                frame_tpl = ht_tpc_temp;
                            }
                        }
                    }
                    /** If mcs == normal_htmcs, no change in TPC */
                }
            }
            prev_htmcs = ht_mcs;
        }

        /** For VHT */
        if (fi->txvec.format == UU_WLAN_FRAME_FORMAT_VHT)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("Data frames (uu_mac_tx.c) for VHT. MCS is: %d, N_sts is: %d\n",
                        fi->txvec.tx_vector_user_params[0].vht_mcs,
                        fi->txvec.tx_vector_user_params[0].num_sts);
#endif
            /* For vht_mcs, we are following the format of Rate Identification field */
            vht_mcs = fi->txvec.tx_vector_user_params[0].vht_mcs;
            vht_mcs = (vht_mcs << UU_VHT_MCSINDEX_SHIFT_FROM_MCS) | (fi->txvec.tx_vector_user_params[0].num_sts);
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("Present vhtmcs is: %d,  previous vhtmcs was: %d, normal vhtmcs is: %d\n", vht_mcs, prev_vhtmcs, normal_vhtmcs);
#endif
            if (prev_vhtmcs >= 0)
            {
                /** Getting the first normal mcs index */
                if (normal_vhtmcs < 0)
                {
                    if (vht_mcs == prev_vhtmcs)
                    {
                        count_vhtmcs++;
                        /** Found normal vhtmcs */
                        if (count_vhtmcs == UU_WLAN_CONST_RATE_COUNT)
                        {
#ifdef UU_WLAN_DFSTPC_DEBUG
                            printk("normal vht mcs is found for the first time: %d\n", normal_vhtmcs);
#endif
                            normal_vhtmcs = vht_mcs;
                            count_vhtmcs = 0;
                        }
                    }
                    /** Restart count_vhtmcs when new mcs is found and normal mcs is not yet identified */
                    else
                    {
                        count_vhtmcs = 0;
                    }
                }
                /** when first normal mcs is already found */
                else
                {
                    if (vht_mcs != normal_vhtmcs)
                    {
                        /** Means, new mcs is repeated, so might be normal mcs has changed */
                        if (vht_mcs == prev_vhtmcs)
                        {
                            /** Getting new normal mcs */
                            count_vhtmcs ++;
                            frame_tpl = vht_tpc_temp; /** Assigning ht_tpc_temp, its already calculated in previous frame tx */
                            if (count_vhtmcs == UU_WLAN_CONST_RATE_COUNT)
                            {
                                normal_vhtmcs = vht_mcs;
                                count_vhtmcs = 0;
#ifdef UU_WLAN_DFSTPC_DEBUG
                            printk("normal vht mcs is found not for the first time: %d\n", normal_vhtmcs);
#endif
                                /** Increment B_TPL as per vht_tpc_temp */
                                uu_update_btpl_new_vhtmcs_all(vht_mcs, normal_vhtmcs);
                                /** Decrementing of TPL will be taken care by WAC-TFAR */
                            }
                        }
                        else
                        {
                            if (((vht_mcs & UU_VHT_NSS_FROM_MCS_MASK) > (normal_vhtmcs & UU_VHT_NSS_FROM_MCS_MASK)) || ((vht_mcs & UU_VHT_MCSINDEX_FIELD_MASK) > (normal_vhtmcs & UU_VHT_MCSINDEX_FIELD_MASK)))
                            {
#ifdef UU_WLAN_DFSTPC_DEBUG
                                printk("new vht mcs is greater than normal vht mcs, so trying new temp tpl\n");
#endif
                                /** Assign vht_tpc_temp for the first time */
                                vht_tpc_temp = uu_increase_tpl_new_vhtmcs (vht_mcs, normal_vhtmcs, bdwdth,
                                                                uu_wlan_btpl_ac_g[(bdwdth * UU_WLAN_AC_MAX_AC) + ac]);
                                frame_tpl = vht_tpc_temp;
                            }
                            /** If mcs&nss < normal_vhtmcs, no change in TPC */
                            else
                            {
                                vht_tpc_temp = uu_wlan_btpl_ac_g[(bdwdth * UU_WLAN_AC_MAX_AC) + ac];
                                frame_tpl = vht_tpc_temp;
                            }
                        }
                    }
                    /** If mcs&nss == normal_vhtmcs, no change in TPC */
                }
            }
            prev_vhtmcs = vht_mcs;
        }
    }

    /** TODO: For the first frame in Multiple Protection (Reference: Duration calculation)
     *
     * If the frame is the first frame in multiple protection,
     * then we will check the lower limit with respect to
     * uu_wlan_firstframe_multiprot[bandwidth]
     *
     *  if (frame is the first frame in multiple protection)
     *  {
     *      frame_ltpl = uu_wlan_firstframe_multiprot[bdwdth];
     *      if (frame_tpl >= UU_WLAN_TPLevel9)
     *      {
     *          frame_tpl += UU_WLAN_MULTIPROT_11AC_LVL_INC;
     *      }
     *      else
     *      {
     *          frame_tpl += UU_WLAN_MULTIPROT_OFDM_LVL_INC;
     *      }
     *      if (frame_tpl < frame_ltpl)
     *      {
     *          frame_tpl = frame_ltpl;
     *      }
     *      printk("The TPC level for the frame(10) is: %d\n", frame_tpl);
     *  }
     *
     */

    /** Need to check the lower limit for the TPL and higher limit for the TPL for VHT */
    /** NOTE: For VHT, There is no need to convert the level from 9-128 to 1-8 */
    if (fi->txvec.format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        /** Verifying the lower limit for the TPL */
        /** If payload > RTS_Threshold, then uu_wlan_highpload_datamgmt_ltpl[(WME_AC * CBW80_80) + bandwidth]
         * else, uu_wlan_lowpload_datamgmt_ltpl[(WME_AC * CBW80_80) + bandwidth]
         */
        if (fi->frameInfo.framelen < UU_WLAN_RTS_THRESHOLD_R)
        {
            frame_ltpl = uu_wlan_lowpload_datamgmt_ltpl[(bdwdth * UU_WLAN_AC_MAX_AC) + ac];
            if (frame_tpl < frame_ltpl)
            {
                frame_tpl = frame_ltpl;
            }
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("The TPC level for the VHT frame after comparing with lower limit for data<RTS is: %d\n", frame_tpl);
#endif
        }
        else
        {
            frame_ltpl = uu_wlan_highpload_datamgmt_ltpl[(bdwdth * UU_WLAN_AC_MAX_AC) + ac];
            if (frame_tpl < frame_ltpl)
            {
                frame_tpl = frame_ltpl;
            }
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("The TPC level for the VHT frame after comparing with lower limit for data>=RTS is: %d\n", frame_tpl);
#endif
        }
        /** Verifying the upper limit for the TPL */
        if (frame_tpl > uu_wlan_utpl_threshold_11ac_g[bdwdth])
        {
            frame_tpl = uu_wlan_utpl_threshold_11ac_g[bdwdth];
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("The TPC level for the VHT frame after comparing with upper threshold is: %d\n", frame_tpl);
#endif
        }

    }

    /** For HT/OFDM/CCK case */
    else
    {
        /** Common for 2.4 GHz and 5 GHz */

        /** Now check the lower limit for the TPL and higher limit for the TPL for HT and OFDM and CCK */

        /** Verifying the lower limit for the TPL */
        /** If payload > RTS_Threshold, then uu_wlan_highpload_datamgmt_ltpl[(WME_AC * CBW80_80) + bandwidth]
         * else, uu_wlan_lowpload_datamgmt_ltpl[(WME_AC * CBW80_80) + bandwidth]
         */
        if (fi->frameInfo.framelen < UU_WLAN_RTS_THRESHOLD_R)
        {
            frame_ltpl = uu_wlan_lowpload_datamgmt_ltpl[(bdwdth * UU_WLAN_AC_MAX_AC) + ac];
            if (frame_tpl < frame_ltpl)
            {
                frame_tpl = frame_ltpl;
            }
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("The TPC level for the OFDM frame after comparing with lower limit for data<RTS is: %d\n", frame_tpl);
#endif
        }
        else
        {
            frame_ltpl = uu_wlan_highpload_datamgmt_ltpl[(bdwdth * UU_WLAN_AC_MAX_AC) + ac];
            if (frame_tpl < frame_ltpl)
            {
                frame_tpl = frame_ltpl;
            }
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("The TPC level for the OFDM frame after comparing with lower limit for data>=RTS is: %d\n", frame_tpl);
#endif
        }

        /** Operating at 5 GHz, so levels starting from 9-128 */
        if (frame_tpl >= UU_WLAN_TPLevel9)
        {
            frame_tpl = get_ofdm_tpl_from_vht_tpl(frame_tpl);
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("The TPC level for the OFDM frame after converting from VHT TPL to OFDM TPL is: %d\n", frame_tpl);
#endif
        }

        /** Common for both 2.4 and 5 GHz */
        /** Verifying the upper limit for the TPL */
        if (frame_tpl > uu_wlan_utpl_threshold_11n_g[bdwdth])
        {
            frame_tpl = uu_wlan_utpl_threshold_11n_g[bdwdth];
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("The TPC level for the OFDM frame after comparing with upper limit is: %d\n", frame_tpl);
#endif
        }
    }


    /** Condition for CCK */
    if ((fi->txvec.format == UU_WLAN_FRAME_FORMAT_NON_HT)
                && (fi->txvec.modulation != OFDM && fi->txvec.modulation != NON_HT_DUP_OFDM))
    {
        frame_tpl = get_cck_tpl_from_ofdm_tpl(frame_tpl);
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("The TPC level for the CCK frame is: %d\n", frame_tpl);
#endif
    }

    /** Now assigning the TPL to the txvector */
    fi->txvec.txpwr_level = frame_tpl;
#ifdef UU_WLAN_DFSTPC_DEBUG
    printk("The TPC level for the frame actually assigned at last in tx_vector is: %u\n", fi->txvec.txpwr_level);
#endif
    /** Check the TPL and then decide if the frequency of TPC Request is needed to change */

    /** If TPC > TPC_Threshold considering only data/management frames, then AP sends TPC
     * Request at higher frequency
     *
     *  If TPC > TPC_Threshold, then (in any of the B_TPL, check this whenever B_TPL value changes)
     *      Timer_send_tpcreq = TPC_Request_Interval/2;
     *  else
     *      Timer_send_tpcreq = TPC_Request_Interval;
     *
     */

#if 0
    if ((fi->txvec.txpwr_level > TPC_Threshold_11ac)
            || ((fi->txvec.txpwr_level > TPC_Threshold_Ofdm) && (fi->txvec.txpwr_level < UU_WLAN_TPLevel9)))
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("TPC_Request_Timer_Interval: %d\n", TPC_Request_Timer_Interval);
#endif
        TPC_Request_Timer_Interval = (UU_REG_LMAC_TPCREQ_INRVL / 2);
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("TPC_Request_Timer_Interval after increasing frequency: %d\n", TPC_Request_Timer_Interval);
#endif
    }
#endif
    /** Filling transmit power in TPC Report */
    if (ieee80211_is_action(hdr->frame_control))
    {
        uu_mgmt_action_frame_t *mgmt_frame = (uu_mgmt_action_frame_t *) skb->data;
        if (mgmt_frame->uu_meas_frame.u.tpc_rep_element.element_id == WLAN_EID_TPC_REPORT &&
                mgmt_frame->uu_meas_frame.mgmt_action == WLAN_ACTION_SPCT_TPC_RPRT &&
                mgmt_frame->uu_meas_frame.category == WLAN_CATEGORY_SPECTRUM_MGMT)
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("The TPC level for TPC Report is: %d\n", frame_tpl);
            printk("The TPC dBm for TPC Report is: %d\n", uu_wlan_transmit_power_dBm[frame_tpl]);
#endif
            mgmt_frame->uu_meas_frame.u.tpc_rep_element.transmit_power = uu_wlan_transmit_power_dBm[frame_tpl];
#ifdef UU_WLAN_DFSTPC_DEBUG
            printk("TPC Report frame in uu_mac_tx.c of len:%d: ", skb->len);
            for (i = 0; i < skb->len; i++)
            {
                printk("%x ", skb->data[i]);
            }
            printk("\n ");
#endif
        }
    }
} /* uu_wlan_setup_frame_tpc */
#endif /* UU_WLAN_TPC */



static uu_void uu_wlan_setup_frame_info_modulation (uu_wlan_tx_frame_info_t *fi, int band)
{
    if (fi->txvec.format == UU_WLAN_FRAME_FORMAT_NON_HT)
    {
        if (fi->txvec.ch_bndwdth > CBW20)
        {
            fi->txvec.modulation = NON_HT_DUP_OFDM;
        }
        /* for 20MHz */
        else
		{
			/* For 5GHz */
			if (band == IEEE80211_BAND_5GHZ)
        	{
        	    fi->txvec.modulation = OFDM;
        	}
        	else /* for 2.4 ghz */
        	{
                if (fi->txvec.L_datarate & UU_OFDM_RATE_FLAG)
                {
                    fi->txvec.modulation = OFDM;
                }
                else
                {
                    fi->txvec.modulation = ERP_CCK;
                }
        	}
		}
    }
    return;
} /* uu_wlan_setup_frame_info_modulation */


/* For non-HT multicast and groupcast frames */
static uu_void uu_wlan_setup_rc_info_multi_nonHT (uu_wlan_tx_frame_info_t *fi, int band, struct sk_buff *skb)
{
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
    struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);

    fi->txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
    /* For data frames */
    if (ieee80211_is_data(hdr->frame_control))
    {
        /* for qos data in 2.4 GHz or any data in 5 GHz case */
        if (ieee80211_is_data_qos(hdr->frame_control) || (band == IEEE80211_BAND_5GHZ))
        {
            /* Highest mandatory rate in OFDM */
            fi->txvec.L_datarate = UU_HW_RATE_24M;
        }
        /* for 2.4 GHz non-qos data */
        else
        {
            /* Highest mandatory rate common in all modulation type in CCK */
            fi->txvec.L_datarate = UU_HW_RATE_2M; /* TODO: Change the macros. They do not fit in 4-bits */
        }
    }
    /* for management frames */
    else
    {
        /* for 2 GHz operation and CCK is supported */
        if ((band == IEEE80211_BAND_2GHZ) && !(tx_info->flags & IEEE80211_TX_CTL_NO_CCK_RATE))
        {
            fi->txvec.L_datarate = UU_HW_RATE_1M; /* TODO: Change the macros. They do not fit in 4-bits */
        }
        /* all other conditions, ie. for 5 GHz or for 2 GHz with no cck */
        else
        {
            fi->txvec.L_datarate = UU_HW_RATE_6M;
        }
    }

    return;
} /* uu_wlan_setup_rc_info_multi_nonHT */


#ifdef UU_WLAN_UMAC_RCAC
static int uu_wlan_vht_check_invalid_case (uu_uint8 vht_mcs, uu_uint8 vht_nss, uu_uint32 flags, struct ieee80211_sta *sta)
{
    /* Returns 1 for Invalid case */

    /* This is a restricted case, and for the below condition MCSIndex is not
       defined or not supported in the Specification */

    /* MCS index 6 is not defined in Nss = 3 and Nss = 7  in CBW80 */
    if ((vht_mcs == 6) && ((vht_nss == 2) || (vht_nss == 6)) && /* CBW80 */(flags & IEEE80211_TX_RC_80_MHZ_WIDTH))
    {
        return 1;
    }

    /* MCS index 7 is not defined in Nss = 6 in CBW80 */
    if ((vht_mcs == 7) && (vht_nss == 5) && /* CBW80 */ (flags & IEEE80211_TX_RC_80_MHZ_WIDTH))
    {
        return 1;
    }

    /* MCS index 9 is defined in Nss = 3 and Nss = 6 and not defined in other Nss in CBW20 */
    if ((vht_mcs == 9) && (((vht_nss != 2) || (vht_nss != 5)) && /* CBW20 */(flags & IEEE80211_TX_RC_20_MHZ_WIDTH)))
    {
        return 1;
    }

    /* MCS index 9 is not defined for Nss = 3 in CBW160 */
    if ((vht_mcs == 9) && (vht_nss == 2) && /* CBW160 */(flags & IEEE80211_TX_RC_160_MHZ_WIDTH))
    {
        return 1;
    }

    /* As per section 9.7.11.3 in 802.11ac D3.0 */
    /* if the channel bandwidth of the PPDU is equal to CBW20 or CBW40, then the STA should not use a (VHT MCS, NSS)
       combination if the VHT MCS is equal to 0, 1, 2 or 3 and the HT MCS with value min(VHT MCS, 7) + 8(NSS – 1) is
       marked as unsupported in the Rx MCS bitmask of the HT capabilities element of the receiver STA.
     */
    /* Number of spatial streams is less than equal to 4 */
    if (vht_nss <= 3)
    {
        if ((flags & IEEE80211_TX_RC_20_MHZ_WIDTH) || (flags & IEEE80211_TX_RC_40_MHZ_WIDTH))
        {
            if (vht_mcs < 4)
            {
                if (!((sta->ht_cap.mcs.rx_mask[(vht_mcs + 8 * (vht_nss)) / 8]
                                >> ((vht_mcs + 8 * (vht_nss)) % 8)) & 0x01))
                {
                    if (vht_nss != 0)
                    {
                        return 1;
                    }
                }
            }
        }

        /* if the channel bandwidth of the PPDU is equal to CBW80, CBW160 or CBW80+80, then the STA should not use a (VHT MCS, NSS)
           combination if the VHT MCS is equal to 0 or 1 and both the HT MCS values min(2(VHT MCS), 7) + 8(NSS – 1) and min(2(VHT MCS) + 1, 7)
           + 8(NSS – 1) are marked as unsupported in the Rx MCS bitmask of the HT capabilities element of the receiver STA. */
        if ((flags & IEEE80211_TX_RC_80_MHZ_WIDTH) || (flags & IEEE80211_TX_RC_160_MHZ_WIDTH)
                || (flags & IEEE80211_TX_RC_80_80_MHZ_WIDTH))
        {
            if (vht_mcs < 2)
            {
                if (!((sta->ht_cap.mcs.rx_mask[((2 * vht_mcs) + 8 * vht_nss) / 8]
                                >> (((2 * vht_mcs) + 8 * vht_nss) % 8)) & 0x01) &&
                        !((sta->ht_cap.mcs.rx_mask[(((2 * vht_mcs) + 1) + 8 * vht_nss) / 8]
                                >> ((((2 * vht_mcs) + 1) + 8 * vht_nss) % 8)) & 0x01))
                {
                    if(vht_nss != 0)
                    {
                        return 1;
                    }
                }
            }
        }
    }

    return 0;
} /* uu_wlan_vht_check_invalid_case */
#endif /* UU_WLAN_UMAC_RCAC */


static uu_uint8 uu_wlan_rc_non_ht_fb (uu_uint8 primary_rate)
{
    uu_uint8 ret_rate;

    if (primary_rate == UU_HW_RATE_54M)
        ret_rate = UU_HW_RATE_48M;
    else if (primary_rate == UU_HW_RATE_48M)
        ret_rate = UU_HW_RATE_36M;
    else if (primary_rate == UU_HW_RATE_36M)
        ret_rate = UU_HW_RATE_24M;
    else if (primary_rate == UU_HW_RATE_24M)
        ret_rate = UU_HW_RATE_18M;
    else if (primary_rate == UU_HW_RATE_18M)
        ret_rate = UU_HW_RATE_12M;
    else
        ret_rate = UU_HW_RATE_6M;

    return ret_rate;
} /* uu_wlan_rc_non_ht_fb */


/* Only for HT-MF and VHT frames */
static uu_uint8 uu_wlan_get_rtscts_rate (uu_uint8 mcs)
{

    /*  MCS      non-HT Reference rate   Bit position in basic rate
        0           6                           4
        1           12                          6
        2           18                          7
        3           24                          8
        4           36                          9
        5           48                          10
        6           54                          11
        7           54                          11
        8           54                          11
        9           54                          11
     */

    uu_uint8 rtscts_rate;
    uu_uint8 ofdm_lrate_ubit;
    uu_uint8 ofdm_lrate_lbit;
    int i;

    /* Get Bit position in basic rate as per MCS index */
    if (!mcs)
    {
        /* For mcs = 0 */
        rtscts_rate = UU_HW_RATE_6M;
        goto ret_rate;
    }
    else if (mcs < 6)
    {
        /* For mcs = 1 to 5 */
        ofdm_lrate_ubit = mcs + 5;
    }
    else
    {
        /* For mcs = 6 to 9 */
        ofdm_lrate_ubit = 11;
    }

    /* For ofdm_rate 24,36,48,54 Mbps, mandatory rate is 24 Mbps */
    /* For ofdm_rate 12,18 Mbps, mandatory rate is 12 Mbps */
    if (ofdm_lrate_ubit >= 8)
        ofdm_lrate_lbit = 8;
    else
        ofdm_lrate_lbit = 6;

    /* Assign the index of the required basic_rate */
    for (i = ofdm_lrate_ubit; i >= ofdm_lrate_lbit; i--)
    {
        if (!(BSSBasicRateSet & BIT(i)))
            continue;

        rtscts_rate = i;
        goto assign_hw_rate;
    }
    rtscts_rate = ofdm_lrate_lbit;

    /* Assign the hw_value of the basic_rate */
assign_hw_rate:
    if (rtscts_rate == 11)
        rtscts_rate = UU_HW_RATE_54M;
    else if (rtscts_rate == 10)
        rtscts_rate = UU_HW_RATE_48M;
    else if (rtscts_rate == 9)
        rtscts_rate = UU_HW_RATE_36M;
    else if (rtscts_rate == 8)
        rtscts_rate = UU_HW_RATE_24M;
    else if (rtscts_rate == 7)
        rtscts_rate = UU_HW_RATE_18M;
    else
        rtscts_rate = UU_HW_RATE_12M;

ret_rate:
    return rtscts_rate;

} /* uu_wlan_get_rtscts_rate */


#ifdef UU_WLAN_UMAC_RCAC
static uu_void uu_wlan_rc_vht_frame_info (struct ieee80211_hw *hw, struct sk_buff *skb, uu_wlan_tx_frame_info_t *fi)
{
    struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
    int i;
    struct ieee80211_tx_rate *tx_rate[2];
    int stbc;
    int num_ss;
    /* Initialize for legacy rts rate and rts rate fallback with 6 Mbps, and mcs index with 0 */
    uu_uint8 rts_rate_update[2] = {UU_HW_RATE_6M, UU_HW_RATE_6M};
    uu_uint8 mcsac_nss[2] = {0x00, 0x00};
    uu_uint8 mcsac_update[2] = {0x00, 0x00};

    /* Receiving stbc information from the flags filled in the framework */
    stbc = (tx_info->flags & IEEE80211_TX_CTL_STBC) >> IEEE80211_TX_CTL_STBC_SHIFT;

    /* This is primary rate/mcs */
    tx_rate[0] = tx_info->control.rates;
    /* This is fallback rate/mcs */
    tx_rate[1] = tx_rate[0] + 1;

    /* If framework doesnot give fallback rate, then make the fallback rate equal to primary rate */
    if (tx_rate[1]->idx < 0)
    {
        tx_rate[1] = tx_rate[0];
    }

    /* Assign frame format */
    fi->txvec.format = UU_WLAN_FRAME_FORMAT_VHT;

    /* Loop goes 2 times as max_rates is defined as 2 in uu_wlan_umac_init.c */
    for (i = 0; i < hw->max_rates; i++)
    {
        /* for multicast/groupcast frames */
        if (is_multicast_ether_addr(hdr->addr1) && !ieee80211_has_tods(hdr->frame_control) && !stbc)
        {
            mcsac_update[i] = (((tx_rate[i]->idx) >> UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & UU_VHT_MCSINDEX_FROM_MCS_MASK);
            mcsac_nss[i] = 0;
        }
        /* for unicast frames or group-addressed stbc frames */
        else
        {
            /* As per 11ac standard 8.4.1.32, in Rate identification field, 0-2nd
               bits represents Nss and 3-6th represents MCS index */
            /* mcsac_update will extract mcs index from mcs got from framework */
            mcsac_update[i] = ((tx_rate[i]->idx) >> UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & UU_VHT_MCSINDEX_FROM_MCS_MASK;
            /* mcsac_nss will extract nss information from mcs got from framework */
            mcsac_nss[i] = (tx_rate[i]->idx) & UU_VHT_NSS_FROM_MCS_MASK;

            if (uu_wlan_vht_check_invalid_case (mcsac_update[i], mcsac_nss[i], tx_rate[i]->flags, tx_info->control.sta))
            {
                return -EINVAL;
            }

            /* Update for RTS rate */
            rts_rate_update[i] =  uu_wlan_get_rtscts_rate (mcsac_update[i]);
            //rts_rate_update[i] = uu_vht_mcs_table_g[mcsac_update[i]].ofdm_rate;
            /* This is proprietary implementation,  and decided to send control frames in non-HT PPDU with non-HT reference rate */
            /* if rts_fallbackrate = rts_primary rate, then we decrement the rts_fallbackrate */
            if (i == 1)
            {
                if (rts_rate_update[i] == rts_rate_update[0])
                {
                    rts_rate_update[i] = uu_wlan_rc_non_ht_fb(rts_rate_update[0]);
                }
            }
        }
    }
    tx_rate[0]->count = 0;
    tx_rate[1]->count = 0;

    /* Filling Tx_Vector for VHT */
    fi->txvec.tx_vector_user_params[0].vht_mcs = mcsac_update[0];
    /* num_ss range is 1-8. The same value is passed to get stbc.
       Then it is subtracted by 1 to get back in the range 0-7.
       1st 3 bits case that contains info about Nss. */
    num_ss = mcsac_nss[0] + 1;
    /* for 1 spatial stream, Nss = 1. */
    fi->txvec.stbc = uu_get_stbc (fi->txvec.format, num_ss, stbc);
    /* for 8 streams, value will be 7 */
    fi->txvec.tx_vector_user_params[0].num_sts = ((fi->txvec.stbc + 1) * (num_ss)) - 1;
    /* TODO: Presently this because spatial mapping is not yet implemented. Ntx can be >= Nsts if spatial mapping */
    fi->txvec.n_tx = fi->txvec.tx_vector_user_params[0].num_sts;
    fi->rtscts_rate = rts_rate_update[0];
    /* For Fallback */
    fi->mcs_fb = mcsac_update[1];
    num_ss = mcsac_nss[1] + 1;
    fi->stbc_fb = uu_get_stbc (fi->txvec.format, num_ss, stbc);
    fi->num_sts_fb[0] = ((fi->stbc_fb + 1) * (num_ss)) - 1;
    fi->n_tx_fb = fi->num_sts_fb;
    fi->rtscts_rate_fb = rts_rate_update[1];

    /* UPDATING SHORT GUARD INTERVAL (true / false) */
    fi->txvec.is_short_GI = ((tx_rate[0]->flags & IEEE80211_TX_RC_SHORT_GI) != 0);

    /* UPDATING FEC CODING */
    if (tx_info->flags & IEEE80211_TX_CTL_LDPC)
        fi->txvec.tx_vector_user_params[0].is_fec_ldpc_coding = 1;
    else
        fi->txvec.tx_vector_user_params[0].is_fec_ldpc_coding = 0;

    return;
} /* uu_wlan_rc_vht_frame_info */
#endif /* UU_WLAN_UMAC_RCAC */


static uu_void uu_wlan_rc_ht_frame_info (struct ieee80211_hw *hw, struct sk_buff *skb, uu_wlan_tx_frame_info_t *fi)
{
    struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
    int i;
    struct ieee80211_tx_rate *tx_rate[2];
    int stbc;
    int num_ss;
    /* Initialize for legacy rts rate and rts rate fallback with 1 Mbps, and mcs index with 0 */
    uu_uint8 rts_rate_update[2] = {UU_HW_RATE_1M, UU_HW_RATE_1M};
    uu_uint8 mcs_update[2];

    /* Receiving stbc information from the flags filled in the framework */
    stbc = (tx_info->flags & IEEE80211_TX_CTL_STBC) >> IEEE80211_TX_CTL_STBC_SHIFT;

    /* This is primary rate/mcs */
    tx_rate[0] = tx_info->control.rates;
    /* This is fallback rate/mcs */
    tx_rate[1] = tx_rate[0] + 1;

    /* If framework doesnot give fallback rate, then make the fallback rate equal to primary rate */
    if (tx_rate[1]->idx < 0)
    {
        tx_rate[1] = tx_rate[0];
    }

    /* Loop goes 2 times as max_rates is defined as 2 in uu_wlan_umac_init.c */
    for (i = 0; i < hw->max_rates; i++)
    {
        mcs_update[i] = tx_rate[i]->idx;

        if (tx_rate[i]->idx < UU_MAX_MCS_N_SUP)
            mcs_update[i] = 0;

        /* for multicast/groupcast frames */
        if (is_multicast_ether_addr(hdr->addr1) && !ieee80211_has_tods(hdr->frame_control)&& !stbc)
        {
            /* Sending with 1 spatial stream, so anding with 0x08 */
            mcs_update[i] = (tx_rate[i]->idx) & UU_HT_MANDATORY_MCS_MASK;

            if (tx_info->control.rates[0].flags & IEEE80211_TX_RC_GREEN_FIELD)
            {
                fi->txvec.format = UU_WLAN_FRAME_FORMAT_HT_GF;
            }
            else
            {
                fi->txvec.format = UU_WLAN_FRAME_FORMAT_HT_MF;
            }
        }
        /* for unicast frames */
        else
        {
            /* for HT-greenfield format */
            if (tx_info->control.rates[0].flags & IEEE80211_TX_RC_GREEN_FIELD)
            {
                fi->txvec.format = UU_WLAN_FRAME_FORMAT_HT_GF;
                /* For HT-GF case, mcs for control frames is being assigned to rts_rate_update */
                /* Considering control frames to be send in HT-PPDU in HT-GF case */
                rts_rate_update[i] = (mcs_update[i] & 0x07);
                if (i == 1)
                {
                    /* if fallback mcs index for rts frames is same, then decrement the fallback rate index */
                    if (rts_rate_update[i] == rts_rate_update[0])
                        rts_rate_update[i] = (!rts_rate_update[0]) ? 0 : (rts_rate_update[0] - 1);
                }
            }
            else /* For HT-mixed mode format */
            {
                fi->txvec.format = UU_WLAN_FRAME_FORMAT_HT_MF;
                rts_rate_update[i] =  uu_wlan_get_rtscts_rate (mcs_update[i]);
                //rts_rate_update[i] = uu_ht_mcs_table_g[mcs_update[i]].ofdm_rate;
                if (i == 1)
                {
                    /* if rts_fallbackrate = rts_primaryrate, then we decrement the rts_fallbackrate */
                    if (rts_rate_update[i] == rts_rate_update[0])
                    {
                        rts_rate_update[i] = uu_wlan_rc_non_ht_fb(rts_rate_update[0]);
                    }
                }
            }
        }
    }
    tx_rate[0]->count = 0;
    tx_rate[1]->count = 0;

    /* Filling Tx_Vector */
    fi->txvec.mcs = mcs_update[0];
    printk("MCS         %d", fi->txvec.mcs);
    fi->rtscts_rate = rts_rate_update[0];
    /* Ranges 0 to 3. */
    num_ss = mcs_update[0] / UU_MAX_MCS_PER_STREAM_HT;
    fi->txvec.stbc = uu_get_stbc (fi->txvec.format, num_ss + 1, stbc);
    /* TODO: Presently this because spatial mapping is not yet implemented. Ntx can be >= Nsts if spatial mapping. */
    fi->txvec.n_tx = fi->txvec.stbc + num_ss;
    /* For fallback */
    fi->mcs_fb = mcs_update[1];
    fi->rtscts_rate_fb = rts_rate_update[1];
    num_ss = mcs_update[1] / UU_MAX_MCS_PER_STREAM_HT;
    fi->stbc_fb = uu_get_stbc (fi->txvec.format, num_ss + 1, stbc);
    fi->n_tx_fb = num_ss + fi->stbc_fb;

    /* UPDATING SHORT GUARD INTERVAL (true / false) */
    fi->txvec.is_short_GI = ((tx_rate[0]->flags & IEEE80211_TX_RC_SHORT_GI) != 0);

    /* UPDATING FEC CODING */
    if (tx_info->flags & IEEE80211_TX_CTL_LDPC)
        fi->txvec.is_fec_ldpc_coding = 1;
    else
        fi->txvec.is_fec_ldpc_coding = 0;

    return;
} /* uu_wlan_rc_ht_frame_info */


static uu_void uu_wlan_rc_legacy_frame_info (struct ieee80211_hw *hw, struct sk_buff *skb, uu_wlan_tx_frame_info_t *fi)
{
    struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
    /* Initialize for legacy rts rate and rts rate fallback with 1 Mbps, and mcs index with 0 */
    uu_uint8 rts_rate_update[2] = {UU_HW_RATE_1M, UU_HW_RATE_1M};

    /* Filling frame format */
    fi->txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
    fi->txvec.L_datarate = ieee80211_get_tx_rate(hw, tx_info)->hw_value;
    printk(" >>>>>>>>>>>>>> %d \n",fi->txvec.L_datarate);
    fi->fallback_rate = ieee80211_get_alt_retry_rate(hw, tx_info, 0) ?
        ieee80211_get_alt_retry_rate(hw, tx_info, 0)->hw_value : fi->txvec.L_datarate;

    /* Assigning rts rate hw_value for legacy */
    rts_rate_update[0] = ieee80211_get_rts_cts_rate(hw, tx_info)->hw_value;

    /* Assigning rts_fallbackrate for legacy cases */
    if (rts_rate_update[0] & 0x80)
    {
        if(rts_rate_update[0] == UU_HW_RATE_11M)
            rts_rate_update[1] = UU_HW_RATE_5M5;
        else if(rts_rate_update[0] == UU_HW_RATE_5M5)
            rts_rate_update[1] = UU_HW_RATE_2M;
        else
            rts_rate_update[1] = UU_HW_RATE_1M;
    }
    else
    {
        if(((rts_rate_update[0] & 0x03) == 3) || (rts_rate_update[0] == UU_HW_RATE_12M))
            rts_rate_update[1] = UU_HW_RATE_6M;
        else if(((rts_rate_update[0] & 0x03) == 2) || (rts_rate_update[0] == UU_HW_RATE_24M))
            rts_rate_update[1] = UU_HW_RATE_12M;
        else
            rts_rate_update[1] = UU_HW_RATE_24M;
    }

    fi->rtscts_rate = rts_rate_update[0];
    fi->rtscts_rate_fb = rts_rate_update[1];

    return;
} /* uu_wlan_rc_legacy_frame_info */


static uu_void uu_wlan_fill_frame_info_rc (struct ieee80211_hw *hw, struct sk_buff *skb, uu_wlan_tx_frame_info_t *fi)
{
    struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;

#ifdef UU_WLAN_UMAC_RCAC
    /* If it is VHT frame, so we will get this info in tx_rate flag */
    if (tx_info->control.rates[0].flags & IEEE80211_TX_RC_MCS_VHT)
    {
        /* Fill frame info for VHT frame */
        uu_wlan_rc_vht_frame_info (hw, skb, fi);
    }
    else
#endif /* UU_WLAN_UMAC_RCAC */
    if (tx_info->control.rates[0].flags & IEEE80211_TX_RC_MCS)
    {
        /* Fill frame info for HT frame */
        uu_wlan_rc_ht_frame_info (hw, skb, fi);
    }
    /* For non-HT */
    else
    {
        /* For handling multicast/groupcast for non-HT data/management frames */
        if (is_multicast_ether_addr(hdr->addr1) && !ieee80211_has_tods(hdr->frame_control))
        {
            uu_wlan_setup_rc_info_multi_nonHT (fi, hw->wiphy->bands[tx_info->band]->band, skb);
        }
        /* Fill frame_info for unicast non-HT frames */
        else
        {
            uu_wlan_rc_legacy_frame_info (hw, skb, fi);
        }
        /* Filling modulation */
        uu_wlan_setup_frame_info_modulation (fi, hw->wiphy->bands[tx_info->band]->band);
    }

    return;
} /* uu_wlan_fill_frame_info_rc */


#ifdef UU_WLAN_DFS
static uu_void uu_wlan_fill_frame_info_chnl_bndwdth(struct sk_buff *skb, uu_wlan_tx_frame_info_t *fi, uu_int32 flags, struct ieee80211_hw *hw)
{
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
#elif defined UU_WLAN_TPC
static uu_void uu_wlan_fill_frame_info_chnl_bndwdth(struct sk_buff *skb, uu_wlan_tx_frame_info_t *fi, uu_int32 flags, struct ieee80211_hw *hw)
{
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
#else
static uu_void uu_wlan_fill_frame_info_chnl_bndwdth(uu_wlan_tx_frame_info_t *fi, uu_int32 flags, struct ieee80211_hw *hw)
{
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
    struct ieee80211_conf *conf = &hw->conf;
    int channel_type = hw->conf.channel_type;

#ifdef UU_WLAN_DFS
    /* It gives the hardware value for the channel */
    /* TODO: The below lines are for switching the channel at AP while sending CSA frame to the STAs.
    Later this will be integrated to the Hostapd/Framework, when the supported is provided. */
    struct ieee80211_mgmt *csa_frame = (struct ieee80211_mgmt *) skb->data;
    if (csa_frame->u.action.u.chan_switch.element_id == WLAN_EID_CHANNEL_SWITCH)
    {
        temp_count = csa_frame->u.action.u.chan_switch.sw_elem.count + 1;
        ch_switch_freq = ieee80211_channel_to_frequency(csa_frame->u.action.u.chan_switch.sw_elem.new_ch_num, IEEE80211_BAND_5GHZ);
        ch_switch_hw_value = csa_frame->u.action.u.chan_switch.sw_elem.new_ch_num;
        printk("DFS_CSA_FRAME_INFO: ch_switch_freq: %d, and hw_value: %d\n", ch_switch_freq, ch_switch_hw_value);
    }

    if(temp_count && (ieee80211_is_beacon(hdr->frame_control)))
    {
        printk("DFS_CHANNEL_SWITCH_AT_AP, channel_count: %d", ch_switch_count);
        ch_switch_count++;
        if(temp_count == ch_switch_count)
        {
            conf->channel->hw_value = ch_switch_hw_value;
            conf->channel->center_freq = ch_switch_freq;
            temp_count = 0;
            ch_switch_count = 0;
        }
    }
#endif /* UU_WLAN_DFS */

    /** TODO Fill this information for latest version of kernel also. */
    /* It gives the hardware value for the channel */
    Channel_hwvalue = conf->channel->hw_value;

    /* for bss operating bandwidth */
    if(hw->conf.channel_type == NL80211_CHAN_HT40MINUS || hw->conf.channel_type == NL80211_CHAN_HT40PLUS)
        Operating_chbndwdth = CBW40;
    else
        Operating_chbndwdth = CBW20;

    /* Center frequency of the current operation in MHz */
    Center_frequency = conf->channel->center_freq;
#endif

#ifdef UU_WLAN_UMAC_RCAC
    if (flags & IEEE80211_TX_RC_80_80_MHZ_WIDTH)
    {
        fi->txvec.ch_bndwdth = CBW80_80;
    }
    else if (flags & IEEE80211_TX_RC_160_MHZ_WIDTH)
    {
        fi->txvec.ch_bndwdth = CBW160;
    }
    else if (flags & IEEE80211_TX_RC_80_MHZ_WIDTH)
    {
        fi->txvec.ch_bndwdth = CBW80;
    }
    else
#endif
    if (flags & IEEE80211_TX_RC_40_MHZ_WIDTH)
    {
        fi->txvec.ch_bndwdth = CBW40;
    }
    else
    {
        fi->txvec.ch_bndwdth = CBW20;
    }

#ifdef UU_WLAN_DATA_160
    /* For data .... 160 MHz ..... */
    if (ieee80211_is_data(hdr->frame_control))
    	fi->txvec.ch_bndwdth = CBW160;
#elif defined UU_WLAN_DATA_80
    /* For data .... 80 MHz ..... */
    if (ieee80211_is_data(hdr->frame_control))
    	fi->txvec.ch_bndwdth = CBW80;
#elif defined UU_WLAN_DATA_40
    /* For data .... 40 MHz ..... */
    if (ieee80211_is_data(hdr->frame_control))
    	fi->txvec.ch_bndwdth = CBW40;
#elif defined UU_WLAN_DATA_20
    /* For data .... 20 MHz ..... */
    if (ieee80211_is_data(hdr->frame_control))
    	fi->txvec.ch_bndwdth = CBW20;
#endif

    /** TODO Fill the information for latest version of kernel as the channel parameters got changed in latest version of kernel */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
    /* UPDATING CHANNEL BANDWIDTH OFFSET, As per 802.11_REV_mb D12, section:20.2.3  */
    /* Note : NON_HT_CBW40 is not included as framework doesnot support */
    if (channel_type == NL80211_CHAN_HT40PLUS)
        fi->txvec.ch_offset = CH_OFF_20U;
    else if (channel_type == NL80211_CHAN_HT40MINUS)
        fi->txvec.ch_offset = CH_OFF_20L;
    else
        fi->txvec.ch_offset = 0;
#endif
}


#ifdef UU_WLAN_TSF
static uu_void uu_update_phy_tx_delay (uu_uint8 format, uu_uint8 mcs)
{
    /* PROVIDED BY PHY TEAM

       NON-HT, MCS 0   12.146us
       NON-HT, MCS 1   12.146us
       NON-HT, MCS 2   12.146us
       NON-HT, MCS 3   12.146us
       NON-HT, MCS 4   12.146us
       NON-HT, MCS 5   12.146us
       NON-HT, MCS 6   12.146us
       NON-HT, MCS 7   12.146us
       HT-MM, MCS 0, 20Mhz 11.943us
       HT-MM, MCS 1, 20Mhz 12.136us
       HT-MM, MCS 2, 20Mhz 12.134us
       HT-MM, MCS 3, 20Mhz 12.134us
       HT-MM, MCS 4, 20Mhz 12.134us
       HT-MM, MCS 5, 20Mhz 12.134us
       HT-MM, MCS 6, 20Mhz 12.134us
       HT-MM, MCS 7, 20Mhz 12.134us

       For BEL
       QPSK – 1/2  158.125us
       QPSK – 3/4  158.125us
       QPSK – 5/6  158.125us
       QPSK – 7/8  158.125us

     */
    /* Since Beacon will be transmitted only in 20 MHz */
    if (format == UU_WLAN_FRAME_FORMAT_NON_HT)
        uu_wlan_phy_tx_delay_r = 13;
    else /* For HT and VHT */
    {
        if (mcs == 0)
            uu_wlan_phy_tx_delay_r = 12;
        else
            uu_wlan_phy_tx_delay_r = 13;
    }
}

/* This function calls the function for updating the register
   uu_wlan_phy_tx_delay_r for beacon timestamp synchronization */

static uu_void uu_wlan_update_beacon_phy_tx_delay (uu_wlan_tx_frame_info_t *fi)
{
    if (fi->txvec.format)
        uu_update_phy_tx_delay (fi->txvec.format, fi->txvec.mcs);
    else
        uu_update_phy_tx_delay (fi->txvec.format, fi->txvec.L_datarate);
}
#endif /* UU_WLAN_TSF */

uu_void uu_wlan_setup_frame_info_rc(struct ieee80211_hw *hw,
        struct sk_buff *skb, uu_int32 framelen,
        uu_wlan_tx_frame_info_t *fi)
{
    struct ieee80211_tx_info *tx_info = IEEE80211_SKB_CB(skb);
    struct ieee80211_hdr *hdr = (struct ieee80211_hdr *) skb->data;
    struct ieee80211_conf *conf = &hw->conf;

#ifdef UU_WLAN_TPC
    int i;
    /* For Transmit Power Control */
    int tpc_level_dBm[CBW80_80] = {0};

    if (conf->flags & IEEE80211_CONF_OFFCHANNEL  || channel_hwval != Channel_hwvalue || channel_bdwdth != Operating_chbndwdth)
    {
        channel_hwval = Channel_hwvalue;
        channel_bdwdth = Operating_chbndwdth;
        not_first_tx = 0;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
        if (hw->conf.channel->band)
#else
        if (hw->conf.chandef.chan->band)
#endif
        {
            uu_wlan_tpc_levels_init_5GHz();
            TPC_Max_Bandwidth = CBW160;
        }
        else
        {
            uu_wlan_tpc_levels_init_2GHz();
            TPC_Max_Bandwidth = CBW40;
        }
    }

    if (!not_first_tx)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
        if (hw->conf.channel->band)
#else
        if (hw->conf.chandef.chan->band)
#endif
            uu_wlan_tpc_levels_init_5GHz();
        else
            uu_wlan_tpc_levels_init_2GHz();
        uu_wlan_reset_wtfar_stats ();
    	if (uu_dot11_op_mode_r == UU_WLAN_MODE_MANAGED)
    	{
        	/** For STA, Calculation of local maximum transmit power will be done in framework,
         	 * using this function ieee80211_handle_pwr_constr in net/mac80211/mlme.c
         	 */
#ifdef UU_WLAN_DFSTPC_DEBUG
        	printk("TPC at STA = %d\n", conf->power_level);
#endif
        	/** Whenever the local power level of the channel changes, the upper threshold is changed */
        	for (i = 0; i <= CBW160; i++)
            	tpc_level_dBm[i] = conf->power_level;
#ifdef UU_WLAN_DFSTPC_DEBUG
        	printk("tpc_level_local: %d\n", tpc_level_dBm[channel_bdwdth]);
#endif
    	}
    	if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
    	{
       		/** This should be only for AP */
        	/** Whenever the max_power of the channel changes, the upper threshold is changed */
#ifdef UU_WLAN_DFSTPC_DEBUG
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
        	printk("TPC at AP = %d\n", conf->channel->max_power);
#else
        	printk("TPC at AP = %d\n", conf->chandef.chan->max_power);
#endif
#endif /* UU_WLAN_DFSTPC_DEBUG */
        	for (i = 0; i <= CBW160; i++)
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
            	tpc_level_dBm[i] = conf->channel->max_power;
#else
            	tpc_level_dBm[i] = conf->chandef.chan->max_power;
#endif

#ifdef UU_WLAN_DFSTPC_DEBUG
        	printk("tpc_level_local: %d\n", tpc_level_dBm[channel_bdwdth]);
#endif
		}
        not_first_tx = 1;
#ifdef UU_WLAN_DFSTPC_DEBUG
    	printk("Channel_hwvalue : %d\n", Channel_hwvalue);
    	printk("Center_frequency : %d\n", Center_frequency);
		uu_print_tpc_values();
#endif
    }

	if (prev_tpc_dBm[channel_bdwdth] < 0)
	{
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("When its first time, then configuring UTPL_Threshold\n");
#endif
        prev_tpc_dBm[channel_bdwdth] = tpc_level_dBm[channel_bdwdth];
        uu_wlan_configure_utpl_threshold (prev_tpc_dBm[channel_bdwdth], channel_bdwdth);
    }
    else
    {
        /** Only at STA side, at 2 GHz, since power constraint is taken care by framework */
        /** For AP, it will be taken care while sending beacon frames */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
        if ((uu_dot11_op_mode_r == UU_WLAN_MODE_MANAGED) && (!(hw->conf.channel->band)))
#else
        if ((uu_dot11_op_mode_r == UU_WLAN_MODE_MANAGED) && (!(hw->conf.chandef.chan->band)))
#endif
        {
#ifdef UU_WLAN_DFSTPC_DEBUG
            int bdwdth = CBW160;
            for (i = 0; i <= bdwdth; i++)
#else
            for (i = 0; i <= channel_bdwdth; i++)
#endif
            {
                if (prev_tpc_dBm[i] != tpc_level_dBm[i])
                {
#ifdef UU_WLAN_DFSTPC_DEBUG
                    printk("TPC Level gets changed\n");
#endif
                    prev_tpc_dBm[i] = tpc_level_dBm[i];
                    uu_wlan_configure_utpl_threshold (prev_tpc_dBm[i], i);
                }
            }
        }
    }

#endif /* UU_WLAN_TPC */

    /* For channel, bandwidth and offset in frame_info */
#ifdef UU_WLAN_DFS
    uu_wlan_fill_frame_info_chnl_bndwdth (skb, fi, tx_info->control.rates[0].flags, hw);
#elif defined UU_WLAN_TPC
    uu_wlan_fill_frame_info_chnl_bndwdth (skb, fi, tx_info->control.rates[0].flags, hw);
#else
    uu_wlan_fill_frame_info_chnl_bndwdth (fi, tx_info->control.rates[0].flags, hw);
#endif

    /* For Rate control */
    uu_wlan_fill_frame_info_rc (hw, skb, fi);


#if defined UU_WLAN_DFS || defined UU_WLAN_TPC
    uu_mgmt_action_frame_t *action_frame = (uu_mgmt_action_frame_t *) skb->data;
#endif

#ifdef UU_WLAN_DFS
#if LINUX_VERSION_CODE <KERNEL_VERSION(3,6,0)
        struct ieee80211_mgmt *csamgmt_frame = (struct ieee80211_mgmt *) skb->data;
        if (hw->conf.channel->band)
        {
            /* Taking care of the frames that are created in UMAC */
            if (csamgmt_frame->u.action.u.chan_switch.element_id == WLAN_EID_CHANNEL_SWITCH)
            {
                fi->txvec.L_datarate = UU_HW_RATE_6M;
                fi->fallback_rate = UU_HW_RATE_6M;
                fi->rtscts_rate = UU_HW_RATE_6M;
                fi->rtscts_rate_fb = UU_HW_RATE_6M;
                fi->txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
            }
        }
#endif
        
        if ((action_frame->uu_meas_frame.u.radio_req_element.rreq_element.element_id == WLAN_EID_MEASURE_REQUEST) ||
					(action_frame->uu_meas_frame.u.meas_element.element_id == WLAN_EID_MEASURE_REQUEST) ||
					(action_frame->uu_meas_frame.u.meas_element.element_id == WLAN_EID_MEASURE_REPORT))
            {
                fi->txvec.L_datarate = UU_HW_RATE_6M;
                fi->fallback_rate = UU_HW_RATE_6M;
                fi->rtscts_rate = UU_HW_RATE_6M;
                fi->rtscts_rate_fb = UU_HW_RATE_6M;
                fi->txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
			}
#endif /* UU_WLAN_DFS */
#ifdef UU_WLAN_TPC
            if ((action_frame->uu_meas_frame.u.tpc_req_element.element_id == WLAN_EID_TPC_REQUEST) ||
					(action_frame->uu_meas_frame.u.tpc_rep_element.element_id == WLAN_EID_TPC_REPORT))
            {
                fi->txvec.L_datarate = UU_HW_RATE_6M;
                fi->fallback_rate = UU_HW_RATE_6M;
                fi->rtscts_rate = UU_HW_RATE_6M;
                fi->rtscts_rate_fb = UU_HW_RATE_6M;
                fi->txvec.format = UU_WLAN_FRAME_FORMAT_NON_HT;
			}
#endif /* UU_WLAN_TPC */
#ifdef UU_WLAN_VHT_FRAME
	/* For VHT TPC Testing */
    if (ieee80211_is_data(hdr->frame_control))
	{
        fi->txvec.stbc = 0;
        printk("VHT DATA Frames\n");
        fi->txvec.format = UU_WLAN_FRAME_FORMAT_VHT;
        fi->txvec.tx_vector_user_params[0].vht_mcs = 2;
        fi->mcs_fb = 1;
        fi->txvec.stbc = 0;
        fi->txvec.tx_vector_user_params[0].num_sts = 0;
        fi->txvec.n_tx = fi->txvec.tx_vector_user_params[0].num_sts;
        fi->rtscts_rate = uu_vht_mcs_table_g[fi->txvec.tx_vector_user_params[0].vht_mcs].ofdm_rate;
        fi->stbc_fb = 0;
        fi->num_sts_fb[0] = 0;
        fi->n_tx_fb = fi->num_sts_fb[0];
        fi->rtscts_rate_fb = uu_vht_mcs_table_g[fi->mcs_fb].ofdm_rate;
	}
#endif /* UU_WLAN_VHT_FRAME */

#ifdef UU_WLAN_DFS
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
    /* It gives the hardware value for the channel */
    /* TODO: The below lines are for switching the channel at AP while sending CSA frame to the STAs.
       Later this will be integrated to the Hostapd/Framework, when the supported is provided. */
    struct ieee80211_mgmt *csa_frame = (struct ieee80211_mgmt *) skb->data;
    if (csa_frame->u.action.u.chan_switch.element_id == WLAN_EID_CHANNEL_SWITCH)
    {
        temp_count = csa_frame->u.action.u.chan_switch.sw_elem.count + 1;
        ch_switch_freq = ieee80211_channel_to_frequency(csa_frame->u.action.u.chan_switch.sw_elem.new_ch_num,
                                                                    IEEE80211_BAND_5GHZ);
        ch_switch_hw_value = csa_frame->u.action.u.chan_switch.sw_elem.new_ch_num;
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("DFS_CSA_FRAME_INFO: ch_switch_freq: %d, and hw_value: %d\n", ch_switch_freq, ch_switch_hw_value);
#endif
    }

    if(temp_count && (ieee80211_is_beacon(hdr->frame_control)))
    {
#ifdef UU_WLAN_DFSTPC_DEBUG
        printk("DFS_CHANNEL_SWITCH_AT_AP, channel_count: %d", ch_switch_count);
#endif
        ch_switch_count++;
        if(temp_count == ch_switch_count)
        {
            conf->channel->hw_value = ch_switch_hw_value;
            conf->channel->center_freq = ch_switch_freq;
            temp_count = 0;
            ch_switch_count = 0;
        }
    }
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0) */
#endif /* UU_WLAN_DFS */

    if (tx_info->flags & IEEE80211_TX_CTL_AMPDU)
        fi->frameInfo.is_ampdu = 1;
    else
        fi->frameInfo.is_ampdu = 0;

#ifdef UU_WLAN_TSF
    /* TODO: Why beacon & Probe-Resp are treated as the same? */
    if (ieee80211_is_beacon (hdr->frame_control) || ieee80211_is_probe_resp (hdr->frame_control))
    {
        /* Updating the PHY_TX_DELAY for beacon frame */
        uu_wlan_update_beacon_phy_tx_delay (fi);

        fi->frameInfo.beacon = 1;
    }
#endif

} /* uu_wlan_setup_frame_info */

uu_void uu_wlan_setup_frame_info_length (uu_wlan_tx_frame_info_t *fi, uu_int32 framelen)
{
    if(fi->txvec.format == UU_WLAN_FRAME_FORMAT_NON_HT)
    {
        fi->txvec.L_length = framelen;
    }
    else if ((fi->txvec.format == UU_WLAN_FRAME_FORMAT_HT_MF) || (fi->txvec.format == UU_WLAN_FRAME_FORMAT_HT_GF))
    {
        fi->txvec.ht_length = framelen;
    }
    else if (fi->txvec.format == UU_WLAN_FRAME_FORMAT_VHT)
    {
        //TODO : Need to fill apep length of that user. this is per user parameter
    }

    return;
} /* uu_wlan_setup_frame_info_length */

/* EOF */
