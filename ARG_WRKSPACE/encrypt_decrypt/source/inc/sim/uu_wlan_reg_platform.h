
/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_reg_platform.h                                  **
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

#ifndef __UU_WLAN_REG_PLATFORM_H__
#define __UU_WLAN_REG_PLATFORM_H__
#include "uu_datatypes.h"
#include "uu_wlan_lmac_if.h"


#define UU_REG_LMAC_FILTER_FLAG            uu_wlan_lmac_filter_flag_g
/** lmac-umac registers for statistics */
/** ack failure count for statistics. */
#define UU_REG_DOT11_ACK_FAILURE_COUNT     dot11_ack_failure_count_g
/** rts failure count for statistics. */
#define UU_REG_DOT11_RTS_FAILURE_COUNT     dot11_rts_failure_count_g
/** rts success count for statistics. */
#define UU_REG_DOT11_RTS_SUCCESS_COUNT     dot11_rts_success_count_g
/** fcs error count for statistics. */
#define UU_REG_DOT11_FCS_ERROR_COUNT       dot11_fcs_error_count_g

/** Total frames received from PHY */
#define UU_WLAN_RX_FRAMES   uu_wlan_rx_frames_g;
/** Number of received multicast frames */
#define UU_WLAN_RX_MULTICAST_CNT uu_wlan_rx_multicast_cnt_g;
/** Number of received broadcast frames */
#define UU_WLAN_RX_BROADCAST_CNT uu_wlan_rx_broadcast_cnt_g;
/** Number of frames received and received frames for us*/
#define UU_WLAN_RX_FRAME_FOR_US uu_wlan_rx_frame_for_us_g;
/** Number of AMPDU's received */
#define UU_WLAN_RX_AMPDU_FRAMES uu_wlan_rx_ampdu_frames_g;
/** Number of AMPDU-subframes received */
#define UU_WLAN_RX_AMPDU_SUBFRAMES uu_wlan_rx_ampdu_subframes_g;
/** Number of error frames received */
#define UU_WLAN_RX_ERR_FRAMES uu_wlan_rx_phy_err_pkts_g;

/** ack response timeout value. */
#define UU_WLAN_ACK_TIMER_VALUE_R     dot11_ack_timer_value
/** cts response timeout value. */
#define UU_WLAN_CTS_TIMER_VALUE_R     dot11_cts_timer_value
/** minimum interframe space between frames */
#define UU_WLAN_SIFS_TIMER_VALUE_R    dot11_sifs_timer_value
/** frame receive error timeout */
#define UU_WLAN_EIFS_VALUE_R          dot11_eifs_timer_value
/** short retry count for retransmission */
#define UU_WLAN_DOT11_SHORT_RETRY_COUNT_R dot11_short_retry_count
/** long retry count for retransmission */
#define UU_WLAN_DOT11_LONG_RETRY_COUNT_R dot11_long_retry_count
/** frame protection threshold value */
#define UU_WLAN_RTS_THRESHOLD_R        dot11_rts_threshold_value
/** Self-CTS / RTS */
#define UU_WLAN_SELF_CTS_R             uu_dot11_self_cts_r
/** idle channel timeout value for contention(dcf) */
#define UU_WLAN_DIFS_VALUE_R           dot11_difs_value
/** idle channel timeout value for contention(edca) */
#define UU_WLAN_AIFS_VALUE_R(x)        dot11_aifs_value[x]
/** txop_limit */
#define UU_WLAN_TXOP_LIMIT_R(x)        dot11_txop_limit_value[x]
/** cwmin value per ac */
#define UU_WLAN_CW_MIN_VALUE_R(x)      dot11_cwmin_value_ac[x]
/** cwmax value per ac */
#define UU_WLAN_CW_MAX_VALUE_R(x)      dot11_cwmax_value_ac[x]
/** cwmin value for dcf */
#define UU_WLAN_CWMIN_VALUE_R         dot11_cwmin_value
/** cwmax value for dcf */
#define UU_WLAN_CWMAX_VALUE_R         dot11_cwmax_value

#define UU_SLOT_TIMER_VALUE_R         dot11_slot_timer_value
#define UU_PHY_RX_START_DELAY_R       dot11_aPhyRxStartDelay
#define UU_WLAN_SIGNAL_EXTENSION_R    dot11_signal_extension
#define UU_REG_LMAC_BEACON_INRVL      dot11_BeaconInterval


#define UU_WLAN_IEEE80211_QOS_MODE_R      uu_dot11_qos_mode_r
#define UU_WLAN_IEEE80211_OP_MODE_R       uu_dot11_op_mode_r
#define UU_WLAN_IEEE80211_STA_MAC_ADDR_R  uu_dot11_sta_mac_addr_r
#define UU_WLAN_IEEE80211_STA_BSSID_R     uu_dot11_sta_bssid_r

/** Rx Status flags register, having 1-flag for each Rx frame */
extern uu_uint32 uu_wlan_rx_status_flags_g;
#define UU_WLAN_RX_STATUS_REG             uu_wlan_rx_status_flags_g

/** Rx buffer base address register*/
extern uu_uchar* uu_wlan_rx_buff_base_addr_reg_g;
#define UU_WLAN_RX_BUFF_BASE_ADDR_REG     uu_wlan_rx_buff_base_addr_reg_g

/** Tx Status flags register array, having a set of flags for each AC */
extern uu_uint32 uu_wlan_tx_status_flags_g[UU_WLAN_MAX_QID];
#define UU_WLAN_TX_STATUS_FLAGS_REG(i)    uu_wlan_tx_status_flags_g[i]

/** Interrupt Status Register simulation.
 * Bit 0 is for Tx status change notification,
 * Bit 1 is for Rx frame notification
 */
extern uu_uint8 uu_wlan_int_status_reg_g;
#define UU_WLAN_INT_STATUS_REG_R          uu_wlan_int_status_reg_g


extern uu_wlan_lmac_filter_flags_t   uu_wlan_lmac_filter_flag_g;

#define uu_setReg(reg_name, val)    (reg_name)=(val)
#define uu_getReg(reg_name)         (reg_name)
#define uu_wlan_lmac_start_platform()     uu_wlan_lmac_start()
#define uu_wlan_lmac_mode_switch_platform() uu_wlan_lmac_mode_switch()
#define uu_wlan_lmac_stop_platform()  uu_wlan_lmac_stop()



/*
** TODO: Categorize the registers, accoding to the 802.11 standard.
*/




/**
 * STATISTICS
 */

extern uu_uint32 dot11_ack_failure_count_g;
extern uu_uint32 dot11_rts_failure_count_g;
extern uu_uint32 dot11_rts_success_count_g;
extern uu_uint32 dot11_fcs_error_count_g;

/** Statistics for rx frames */

extern uu_uint32 uu_wlan_rx_frames_g;
extern uu_uint32 uu_wlan_rx_multicast_cnt_g;
extern uu_uint32 uu_wlan_rx_broadcast_cnt_g;
extern uu_uint32 uu_wlan_rx_frame_for_us_g;
extern uu_uint32 uu_wlan_rx_ampdu_frames_g;
extern uu_uint32 uu_wlan_rx_ampdu_subframes_g;
extern uu_uint32 uu_wlan_rx_phy_err_pkts_g;

/**
 * CONFIGURATION REGISTERS
 */
/* TODO: Put a comment, for the max number of bits actually needed for each of these registers */
/** ACK Timeout value. Configured from umac during initialization. */
extern uu_uint32 dot11_ack_timer_value;

/** CTS Timeout value. Configured from umac during initialization. */
extern uu_uint32 dot11_cts_timer_value;

/** SIFS timeout value. Configured from umac during initialization.
 * Highest value as per specification is 64 μs in OFDM PHY and 5 MHz channel spacing.
 * Number of bits  :  7
 */
extern uu_uint32 dot11_sifs_timer_value;

/** EIFS timeout value. */
extern uu_uint32 dot11_eifs_timer_value;

/** RTS Threshold value. Configured from umac during initialization.
 * Also configured from hostapd configuration file in case of AP.
 */
extern uu_uint32 dot11_rts_threshold_value;

/** Self CTS flag. Configured from umac during initialization.
 * Set the flag for self CTS support (instead of RTS-CTS).
 * Number of bits : 1
 */
extern uu_uint32       uu_dot11_self_cts_r;

/** DIFS value. Configured from umac during initialization.*/
extern uu_uint32 dot11_difs_value;

/** AIFS value per AC. Configured from umac during initialization.
 * Max value as per specification is 7.
 * Number of bits : 3
 */
extern uu_uint32 dot11_aifs_value[UU_WLAN_MAX_QID];

/** CWMIN value per AC. Configured from umac during initialization.
 * CWMIN Max value as per specification is 63.
 * Number of bits : 6
 */
extern uu_uint32 dot11_cwmin_value_ac[UU_WLAN_MAX_QID];

/** CWMAX value per AC. Configured from umac during initialization.
 * CWMIN Max value as per specification is 1023.
 * Number of bits : 10
 */
extern uu_uint32 dot11_cwmax_value_ac[UU_WLAN_MAX_QID];

/** CWMIN value for non-QoS mode. Configured from umac during initialization.
 * CWMIN Max value as per specification is 63.
 * Number of bits : 6
 */
extern uu_uint32 dot11_cwmin_value;

/** CWMAX value for non-QoS mode. Configured from umac during initialization.
 * CWMIN Max value as per specification is 1023.
 * Number of bits : 10
 */
extern uu_uint32 dot11_cwmax_value;

/** TXOP limit value per AC. Configured from umac during initialization.
 * Max value as per specification is 6.016 ms (rounded off to 7)
 * Number of bits : 3.
 */
extern uu_uint32 dot11_txop_limit_value[UU_WLAN_MAX_QID];

/** Short retry count value.
 * Configured from umac during initialization.
 */
extern uu_uint32 dot11_short_retry_count;

/** Long retry count value.
 * Configured from umac during initialization.
 */
extern uu_uint32 dot11_long_retry_count;

/** slot timer value. Configured from umac during initialization.
 * Max value is 50 μs for FHPHY. It is 10 us for OFDM.
 * Number of bits : 6
 */
extern uu_uint32 dot11_slot_timer_value;

/** PHY Rx Start delay value. Configured from umac during initialization.
 * Max value is 192 μs in case of DS PHY and ERP-OFDM.
 * Number of bits : 8
 */
extern uu_uint32 dot11_aPhyRxStartDelay;

/** Signal extension value. Configured from umac during initialization. */
extern uu_uint32 dot11_signal_extension;

/** QoS Mode. Configured from UMAC. 0 for non-QoS Mode and 1 for QoS mode
 * Number of bits : 1
 */
extern uu_uint32         uu_dot11_qos_mode_r;

/** Operational mode. Configured from UMAC during initalization and changed based on
 * BSS changed.
 * Number of bits : 3 bits
 */
extern uu_wlan_op_mode_t uu_dot11_op_mode_r;

/* BEACON Info */
/** Beacon Interval - Beacon timer will fire after this configured interval.
 * This will be set during intialization of registers.
 * Value may update if the user space application defines in hostapd config file.
 */
extern uu_uint32  dot11_BeaconInterval;


/* TODO: Document the purpose of each of these registers. Name them properly */
/* For rate and wideband support */
extern uu_uint16 BSSBasicRateSet;
extern uu_uint32 Operating_chbndwdth;
extern uu_uint16 Channel_hwvalue;
extern uu_uint16 Center_frequency;


#ifdef UU_WLAN_DFS /* TODO: Need thorough review. Do not use DFS for the time being */
/* for dfs statistics */
extern uu_uint8 msrment_stats_g;
/** This will be used for CCA_report in units of microseconds. */
/** Since measurement duration is of 2 octets in TUs,
 * so, to have the unit in microseconds, we need 26 bits
 */
extern uu_uint32 chan_busy_dur_g;

/** This parameter is for radar detected, and will be filled by the PHY after detecting radar.
if (phy_radar_detected_g == 1)
{
    phy_radar_detected_g = 1;
}
else
{
    phy_radar_detected_g = indication_from_PHY
}
*/

extern uu_uint8 phy_radar_detected_g;
#endif /* UU_WLAN_DFS */

#ifdef UU_WLAN_TPC /* TODO: Need thorough review. Do not use DFS for the time being */
/* Registers for Transmit Power Control Support */
/* Maximum Supported Power Level implemented for 11g/n */
extern uu_uint32 dot11NumberSupportedPowerLevelsImplemented_g;
/* U_TPL_Threshold for 11ac – 1 register (1 octet) */
extern uu_uint8 uu_wlan_utpl_threshold_11ac_g[CBW80_80];
/* U_TPL_Threshold for 11n – 1 register (1 octet) */
extern uu_uint8 uu_wlan_utpl_threshold_11n_g[CBW80_80];
/* TP_Level_Difference – 1 register (1 octet) */
extern uu_uint8 uu_wlan_tp_level_diff_g;
/* B_TPL table for data/management frames – 16 registers (1 octet each)
[Should we keep 32 bits or 8 bits, as in spec, its 32 bits. uu_uint32 dot11CurrentTxPowerLevelExtended] */
/* bk20, be20, vi20, vo20, bk40, be40, vi40, vo40, bk80, be80, vi80, vo80, bk160, be160, vi160, vo160 */
extern uu_uint8 uu_wlan_btpl_ac_g[CBW80_80 * UU_WLAN_AC_MAX_AC];

extern uu_uint8 uu_wlan_retry_11ac_tpl_inc_g;
extern uu_uint8 uu_wlan_retry_ofdm_tpl_inc_g;
#endif /* UU_WLAN_TPC */


/* 6-bytes MAC address
 * Used in LMAC for - control packet generation, rx frame validation etc.
 */
extern uu_uchar          uu_dot11_sta_mac_addr_r[6];
extern uu_uchar          uu_dot11_sta_bssid_r[6];

#ifdef UU_WLAN_TSF
/** Hardware TSF timer (microsecond).
 * This will be incremented in each timer tick, as timer tick
 * will occur in each microsecond
 */
/* TODO: Describe how this is used. When is it set? When is it used?
 TODO: The comments & the explanation of engineers is not matching */
extern uu_uint64 uu_wlan_tsf_r;
#define UU_WLAN_TSF_TIMER_R         uu_wlan_tsf_r

/** The PHY TX DELAY required for transmitting beacon from MAC-PHY i/f to PHY-WM.
 * This will vary as per the mcs/rate. So, UMAC need to provide as per the mcs
 * it is using for transmitting the beacon.
 */
/* TODO: Why do we need this here? When is this set? When is it used? */
extern uu_uint32 uu_wlan_phy_tx_delay_r;
#define UU_WLAN_PHY_TX_BECN_DELAY_R	uu_wlan_phy_tx_delay_r
#endif /* UU_WLAN_TSF */


#ifdef UU_WLAN_IBSS
/** Defining if the beacon was sent by the STA in last TBTT.
 * Only if the beacon is transmitted, then this register is set to 1.
 * Else if the beacon is received, then the register is set to 0.
 * This is required by the UMAC to tell the framework if the STA is IBSS owner or not.
 */
extern uu_uint8 uu_wlan_last_beacon_tx_r;
#define UU_WLAN_LAST_BEACON_TXED_R  uu_wlan_last_beacon_tx_r
#endif


extern uu_void uu_wlan_umac_cbk(uu_int8 ind);

#if 0 /* Beacon transmission trigger is taken care of in UMAC. So not using it now */
/* This register is used in IBSS mode as a flag.
 * This flag will be set as 1 whenever the bss mode changes as IBSS
 * or LMAC gives trigger to UMAC for sending Beacon.
 * This flag will be set to 0 whenever Beacon is received in IBSS mode,
 * thus for not sending the Beacon.
 * Basically this register is NOT required to implement,
 * but currently it is implemented to avoid race around condition caused
 * by the delay between the LMAC trigger to generate beacon and UMAC
 * writing Beacon buffer to LMAC.
 */

extern uu_uint8 uu_wlan_ibss_send_beacon_flag_r;
#endif



/** Indicates whether the Power-Save feature is enabled by UMAC */
extern uu_bool uu_wlan_ps_mode_r;
/** Indicates whether the Power-Save mode is UAPSD */
extern uu_bool uu_wlan_uapsd_mode_r;

/** gives the aid of the station */
extern uu_uint16 uu_wlan_sta_aid_r;


extern uu_int32 uu_wlan_kthrd_init(uu_void);
extern uu_void uu_wlan_kthrd_exit(uu_void);


#endif /* __UU_WLAN_REG_PLATFORM_H__ */

