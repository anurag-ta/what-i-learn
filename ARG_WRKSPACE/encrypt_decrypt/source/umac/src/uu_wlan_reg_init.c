/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_reg_init.c                                            **
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
/* $Revision: 1.11 $ */

#include "uu_wlan_reg.h"
#include "uu_wlan_reg_init.h"


uu_void uu_wlan_set_default_reg_values(uu_void)
{
    /** Slot timer default value is : 9 micro sec */
    uu_setReg(UU_SLOT_TIMER_VALUE_R, UU_WLAN_SLOT_TIMER_DEFAULT_VALUE);

    /** SIFS timer default value is : 16 micro sec */
    uu_setReg(UU_WLAN_SIFS_TIMER_VALUE_R , UU_WLAN_SIFS_TIMER_DEFAULT_VALUE);

    /** DIFS Timeout value */
    /** As per the specification licensed_P802.11_REVmb_D12.pdf - section 9.3.7 
     * DIFS = aSIFSTime + 2 * aSlotTime 
     * PIFS = aSIFSTime + aSlotTime 
     */
    uu_setReg(UU_WLAN_DIFS_VALUE_R, uu_getReg(UU_WLAN_SIFS_TIMER_VALUE_R + (2 * uu_getReg(UU_SLOT_TIMER_VALUE_R))));

    /** EIFS Timeout value */

    /** For OFDM PHY aPHY-RX-START-Delay is 25 micro sec for 20MHz channel spacing */
    uu_setReg(UU_PHY_RX_START_DELAY_R , UU_WLAN_PHY_RXSTART_DELAY_DEFAULT_VALUE);

    /** ACK Timeout value */
    /** As per the specification licensed_P802.11_REVmb_D12.pdf - section 9.3.2.8 ACK Procedure 
     * ACK timeout interval value is aSIFSTime + aSlotTime + aPHY-RX-START-Delay 
     */
    uu_setReg(UU_WLAN_ACK_TIMER_VALUE_R , uu_getReg(UU_WLAN_SIFS_TIMER_VALUE_R) + uu_getReg(UU_SLOT_TIMER_VALUE_R) + uu_getReg(UU_PHY_RX_START_DELAY_R));

    /** CTS Timeout value */
    /** As per the specification licensed_P802.11_REVmb_D12.pdf - section 9.3.2.6 CTS Procedure 
     * CTS timeout interval value is aSIFSTime + aSlotTime + aPHY-RX-START-Delay 
     */
    /** For OFDM PHY aPHY-RX-START-Delay is 25 micro sec for 20MHz channel spacing */
    uu_setReg(UU_WLAN_CTS_TIMER_VALUE_R, uu_getReg(UU_WLAN_SIFS_TIMER_VALUE_R) + uu_getReg(UU_SLOT_TIMER_VALUE_R) + uu_getReg(UU_PHY_RX_START_DELAY_R));

    /** preamble length is 16 micro sec */


    /** Short retry and long retry counts */
    /** Tx retry limit defaults from standard Refer IEEE802.11-2007 - Page 874 - 
     * dot11ShortRetryLimit Integer:= 7
     * dot11LongRetryLimit Integer:= 4,
     */
    uu_setReg(UU_WLAN_DOT11_SHORT_RETRY_COUNT_R , UU_WLAN_SHORT_RETRY_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_DOT11_LONG_RETRY_COUNT_R , UU_WLAN_LONG_RETRY_DEFAULT_VALUE);

    /** CWmin value is : 15 
     * CWmax value is : 1023 
     * For OFDM PHY :
     * Following table indicates the Default EDCA parameter values in case of dot11OCBActivated is false 
     * -------------------------------------------------------------------------------------
     * -    AC   -    CWMin            -        CWMax       -     AIFSN    - TXOP Limit    -
     * -------------------------------------------------------------------------------------
     * - AC_BK   -    aCWMin           -      aCWMax        -       7      -     0         -
     * - AC_BE   -    aCWMin           -      aCWMax        -       3      -     0         -
     * - AC_VI   -   (aCWMin+1)/2-1    -      aCWMin        -       2      -  3.008 ms     -
     * - AC_VO   -   (aCWMin+1)/4-1    -    (aCWMin+1)/2-1  -       2      -  1.504 ms     -
     * -------------------------------------------------------------------------------------
     *
     */
    uu_setReg(UU_WLAN_CWMIN_VALUE_R, UU_WLAN_CWMIN_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_CWMAX_VALUE_R, UU_WLAN_CWMAX_DEFAULT_VALUE);

    /** For AC_BK */
    uu_setReg(UU_WLAN_CW_MIN_VALUE_R(0) , UU_WLAN_CWMIN_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_CW_MAX_VALUE_R(0) , UU_WLAN_CWMAX_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_TXOP_LIMIT_R(0) , UU_WLAN_TXOP_LIMIT_BK_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_AIFS_VALUE_R(0) , UU_WLAN_AIFSN_BK_DEFAULT_VALUE);

    /** For AC_BE */
    uu_setReg(UU_WLAN_CW_MIN_VALUE_R(1) , UU_WLAN_CWMIN_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_CW_MAX_VALUE_R(1) , UU_WLAN_CWMAX_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_TXOP_LIMIT_R(1) , UU_WLAN_TXOP_LIMIT_BE_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_AIFS_VALUE_R(1) , UU_WLAN_AIFSN_BE_DEFAULT_VALUE);

    /** For AC_VI */
    uu_setReg(UU_WLAN_CW_MIN_VALUE_R(2) , UU_WLAN_CWMIN_VI_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_CW_MAX_VALUE_R(2) , UU_WLAN_CWMAX_VI_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_TXOP_LIMIT_R(2) , UU_WLAN_TXOP_LIMIT_VI_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_AIFS_VALUE_R(2) , UU_WLAN_AIFSN_VI_DEFAULT_VALUE);

    /** For AC_VO */
    uu_setReg(UU_WLAN_CW_MIN_VALUE_R(3) , UU_WLAN_CWMIN_VO_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_CW_MAX_VALUE_R(3) , UU_WLAN_CWMAX_VO_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_TXOP_LIMIT_R(3) , UU_WLAN_TXOP_LIMIT_VO_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_AIFS_VALUE_R(3) , UU_WLAN_AIFSN_VO_DEFAULT_VALUE);

#ifdef UU_WLAN_BQID
    /** For Beacon */
    if (UU_WLAN_IEEE80211_OP_MODE_R == UU_WLAN_MODE_ADHOC)
    {
        uu_setReg(UU_WLAN_CW_MIN_VALUE_R(4), UU_WLAN_CW_BEACON_IBSS_VALUE);
        uu_setReg(UU_WLAN_CW_MAX_VALUE_R(4), UU_WLAN_CW_BEACON_IBSS_VALUE);
    }
    else
    {
        uu_setReg(UU_WLAN_CW_MIN_VALUE_R(4), UU_WLAN_CW_BEACON_DEFAULT_VALUE);
        uu_setReg(UU_WLAN_CW_MAX_VALUE_R(4), UU_WLAN_CW_BEACON_DEFAULT_VALUE);
    }
    uu_setReg(UU_WLAN_TXOP_LIMIT_R(4), UU_WLAN_TXOP_LIMIT_BEACON_DEFAULT_VALUE);
    uu_setReg(UU_WLAN_AIFS_VALUE_R(4), UU_WLAN_AIFSN_BEACON_DEFAULT_VALUE);
#endif

#ifdef UU_WLAN_TSF
    /* Initializing with default value for uurmi wlan for 6 Mbps */
    uu_wlan_phy_tx_delay_r = 13;
#endif

    /** Default beacon interval is 100ms (100000usec) */
    uu_setReg(UU_REG_LMAC_BEACON_INRVL, UU_WLAN_BEACON_DEFAULT_VALUE_CMODEL); /* TODO: Temp fix - Multiplied by 8 */

    /** Default value for RTS threshold value */
    uu_setReg(UU_WLAN_RTS_THRESHOLD_R, UU_WLAN_RTSTHRESHOLD_DEFAULT_VALUE);

    /** As per 802.11 REV-mb D12 section 9.3.8, signal extension is present for HT-MF or HT-GF or ERP-OFDM, DSSS-OFDM, and NON_HT_DUPOFDM
     * For RIFS case, NO_SIG_EXTN is true.
     * Table 20-25, aSignalExtension 0 μs when operating in the 5 GHz band and 6 μs when operating in the 2.4 GHz band
     * Pg 1753, 3rd 4th paragraph.
     * Since it is defined in Annex 20. It probably means that HT STA shall support signal extension in non-HT format too. It may not be supported by legacy STAs.
     * This is not present in VHT format.
     */
    uu_setReg(UU_WLAN_SIGNAL_EXTENSION_R , 0);

    /** Default value for QoS register is 1 (enabled) */
    uu_setReg(UU_WLAN_IEEE80211_QOS_MODE_R,0x1);
    /** Default operational model is Station (Managed) */
    uu_setReg(UU_WLAN_IEEE80211_OP_MODE_R,UU_WLAN_MODE_MANAGED);

    /** Default value for self-CTS flag */
    uu_setReg(UU_WLAN_SELF_CTS_R, 0);

#if 0 /* No need to initialize. In C, these are global variables */
    /** Statistics initialization */
    dot11_ack_failure_count_g = 0;
    dot11_rts_failure_count_g = 0;
    dot11_rts_success_count_g = 0;
    dot11_fcs_error_count_g = 0;

    uu_wlan_rx_frames_g = 0;
    uu_wlan_rx_multicast_cnt_g = 0;
    uu_wlan_rx_broadcast_cnt_g = 0;
    uu_wlan_rx_frame_for_us_g = 0;
    uu_wlan_rx_ampdu_frames_g = 0;
    uu_wlan_rx_ampdu_subframes_g = 0;
    uu_wlan_rx_phy_err_pkts_g = 0;
#endif

    return;
} /* uu_wlan_set_default_reg_values */


/* EOF */

