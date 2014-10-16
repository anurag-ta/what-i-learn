/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_reg.c                                          **
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
#include "uu_wlan_reg.h"


/* Mail box Registers */
uu_uint8  lmac_conf_busy_g;
EXPORT_SYMBOL(lmac_conf_busy_g);

uu_uint8  lmac_conf_update_pending_g;
EXPORT_SYMBOL(lmac_conf_update_pending_g);

uu_wlan_conf_reg_update_flags_t  lmac_conf_rq_g;
EXPORT_SYMBOL(lmac_conf_rq_g);

uu_uint32  lmac_conf_update_32bitv1_g;
EXPORT_SYMBOL(lmac_conf_update_32bitv1_g);

uu_uint32  lmac_conf_update_32bitv2_g;
EXPORT_SYMBOL(lmac_conf_update_32bitv2_g);

uu_uint32  lmac_conf_update_32bitv3_g;
EXPORT_SYMBOL(lmac_conf_update_32bitv3_g);

uu_uint32  lmac_conf_update_32bitv4_g;
EXPORT_SYMBOL(lmac_conf_update_32bitv4_g);

uu_uchar   lmac_conf_update_8bitv1_g;
EXPORT_SYMBOL(lmac_conf_update_8bitv1_g);

uu_uchar   lmac_conf_update_8bitv2_g;
EXPORT_SYMBOL(lmac_conf_update_8bitv2_g);

/* BEACON Info */
uu_uint32  dot11_BeaconInterval;
EXPORT_SYMBOL(dot11_BeaconInterval);
uu_uint16  lmac_beacon_listen_interval_g;
EXPORT_SYMBOL(lmac_beacon_listen_interval_g);
uu_uint16  lmac_beacon_miss_timeout_g;
EXPORT_SYMBOL(lmac_beacon_miss_timeout_g);
uu_uint16  lmac_beacon_dtim_period_g;
EXPORT_SYMBOL(lmac_beacon_dtim_period_g);
uu_uint8   lmac_beacon_dtim_count_g;
EXPORT_SYMBOL(lmac_beacon_dtim_count_g);

/* for rate and wideband support */
uu_uint16 BSSBasicRateSet;
EXPORT_SYMBOL(BSSBasicRateSet);
uu_uint32 Operating_chbndwdth;
EXPORT_SYMBOL(Operating_chbndwdth);
uu_uint16 Channel_hwvalue;
EXPORT_SYMBOL(Channel_hwvalue);
uu_uint16 Center_frequency;
EXPORT_SYMBOL(Center_frequency);

#ifdef UU_WLAN_DFS
/* for dfs statistics to measure */
uu_uint8 msrment_stats_g;
EXPORT_SYMBOL(msrment_stats_g);
uu_uint32 chan_busy_dur_g;
EXPORT_SYMBOL(chan_busy_dur_g);
uu_uint8 phy_radar_detected_g;
EXPORT_SYMBOL(phy_radar_detected_g);
#endif

//uu_int8    lmac_tx_desc_id_g; 
//uu_int8    lmac_tx_desc_id_read_pend_g; 

uu_int8    lmac_rx_desc_id_g; 
EXPORT_SYMBOL(lmac_rx_desc_id_g);

uu_wlan_lmac_filter_flags_t   uu_wlan_lmac_filter_flag_g; 
EXPORT_SYMBOL(uu_wlan_lmac_filter_flag_g);

#if 0
uu_uint32 tx_tstamp_g;
EXPORT_SYMBOL(tx_tstamp_g);
uu_uint32 tx_cookie_g;
EXPORT_SYMBOL(tx_cookie_g);
uu_uint16 tx_seqnum_g;
EXPORT_SYMBOL(tx_seqnum_g);
uu_uchar  tx_status_g;
EXPORT_SYMBOL(tx_status_g);
uu_uchar  tx_rateindex_g;
EXPORT_SYMBOL(tx_rateindex_g);
uu_char   tx_rssi_g;
EXPORT_SYMBOL(tx_rssi_g);
uu_uchar  tx_shortretry_g;
EXPORT_SYMBOL(tx_shortretry_g);
uu_uchar  tx_longretry_g;
EXPORT_SYMBOL(tx_longretry_g);
uu_uchar  tx_virtcol_g;
EXPORT_SYMBOL(tx_virtcol_g);
uu_uchar  tx_flags_g;
EXPORT_SYMBOL(tx_flags_g);
uu_char   tx_rssi_ctl0_g;
EXPORT_SYMBOL(tx_rssi_ctl0_g);
uu_char   tx_rssi_ctl1_g;
EXPORT_SYMBOL(tx_rssi_ctl1_g);
uu_char   tx_rssi_ctl2_g;
EXPORT_SYMBOL(tx_rssi_ctl2_g);
uu_char   tx_rssi_ext0_g;
EXPORT_SYMBOL(tx_rssi_ext0_g);
uu_char   tx_rssi_ext1_g;
EXPORT_SYMBOL(tx_rssi_ext1_g);
uu_char   tx_rssi_ext2_g;
EXPORT_SYMBOL(tx_rssi_ext2_g);
uu_uchar  tx_qid_g;
EXPORT_SYMBOL(tx_qid_g);
uu_uint16 tx_desc_id_g;
EXPORT_SYMBOL(tx_desc_id_g);
uu_uchar  tx_tid_g;
EXPORT_SYMBOL(tx_tid_g);
uu_uint32 tx_ba_low_g;
EXPORT_SYMBOL(tx_ba_low_g);
uu_uint32 tx_ba_high_g;
EXPORT_SYMBOL(tx_ba_high_g);
uu_uint32 tx_evm0_g;
EXPORT_SYMBOL(tx_evm0_g);
uu_uint32 tx_evm1_g;
EXPORT_SYMBOL(tx_evm1_g);



//////////////////////////////////////////////////////////
uu_uint32 rx_tstamp_g;
EXPORT_SYMBOL(rx_tstamp_g);
uu_uint16 rx_datalen_g;
EXPORT_SYMBOL(rx_datalen_g);
uu_uchar rx_status_g;
EXPORT_SYMBOL(rx_status_g);
uu_uchar rx_phyerr_g;
EXPORT_SYMBOL(rx_phyerr_g);
uu_char rx_rssi_g;
EXPORT_SYMBOL(rx_rssi_g);
uu_uchar rx_keyix_g;
EXPORT_SYMBOL(rx_keyix_g);
uu_uchar rx_rate_g;
EXPORT_SYMBOL(rx_rate_g);
uu_uchar rx_antenna_g;
EXPORT_SYMBOL(rx_antenna_g);
uu_uchar rx_more_g;
EXPORT_SYMBOL(rx_more_g);
uu_char rx_rssi_ctl0_g;
EXPORT_SYMBOL(rx_rssi_ctl0_g);
uu_char rx_rssi_ctl1_g;
EXPORT_SYMBOL(rx_rssi_ctl1_g);
uu_char rx_rssi_ctl2_g;
EXPORT_SYMBOL(rx_rssi_ctl2_g);
uu_char rx_rssi_ext0_g;
EXPORT_SYMBOL(rx_rssi_ext0_g);
uu_char rx_rssi_ext1_g;
EXPORT_SYMBOL(rx_rssi_ext1_g);
uu_char rx_rssi_ext2_g;
EXPORT_SYMBOL(rx_rssi_ext2_g);
uu_uchar rx_isaggr_g;
EXPORT_SYMBOL(rx_isaggr_g);
uu_uchar rx_moreaggr_g;
EXPORT_SYMBOL(rx_moreaggr_g);
uu_uchar rx_num_delims_g;
EXPORT_SYMBOL(rx_num_delims_g);
uu_uchar rx_flags_g;
EXPORT_SYMBOL(rx_flags_g);
uu_uint32 evm0_g;
EXPORT_SYMBOL(evm0_g);
uu_uint32 evm1_g;
EXPORT_SYMBOL(evm1_g);
uu_uint32 evm2_g;
EXPORT_SYMBOL(evm2_g);
uu_uint32 evm3_g;
EXPORT_SYMBOL(evm3_g);
uu_uint32 evm4_g;
EXPORT_SYMBOL(evm4_g);
#endif

/** umac statistics */
uu_uint32 dot11_ack_failure_count_g;
EXPORT_SYMBOL(dot11_ack_failure_count_g);
uu_uint32 dot11_rts_failure_count_g;
EXPORT_SYMBOL(dot11_rts_failure_count_g);
uu_uint32 dot11_rts_success_count_g;
EXPORT_SYMBOL(dot11_rts_success_count_g);
uu_uint32 dot11_fcs_error_count_g;
EXPORT_SYMBOL(dot11_fcs_error_count_g);

uu_uint32 uu_wlan_rx_frames_g;
EXPORT_SYMBOL(uu_wlan_rx_frames_g);
uu_uint32 uu_wlan_rx_multicast_cnt_g;
EXPORT_SYMBOL(uu_wlan_rx_multicast_cnt_g);
uu_uint32 uu_wlan_rx_broadcast_cnt_g;
EXPORT_SYMBOL(uu_wlan_rx_broadcast_cnt_g);
uu_uint32 uu_wlan_rx_frame_for_us_g;
EXPORT_SYMBOL(uu_wlan_rx_frame_for_us_g);
uu_uint32 uu_wlan_rx_ampdu_frames_g;
EXPORT_SYMBOL(uu_wlan_rx_ampdu_frames_g);
uu_uint32 uu_wlan_rx_ampdu_subframes_g;
EXPORT_SYMBOL(uu_wlan_rx_ampdu_subframes_g);
uu_uint32 uu_wlan_rx_phy_err_pkts_g;
EXPORT_SYMBOL(uu_wlan_rx_phy_err_pkts_g);

uu_uint32 dot11_ack_timer_value;
EXPORT_SYMBOL(dot11_ack_timer_value);
uu_uint32 dot11_cts_timer_value;
EXPORT_SYMBOL(dot11_cts_timer_value);
uu_uint32 dot11_sifs_timer_value;
EXPORT_SYMBOL(dot11_sifs_timer_value);
uu_uint32 dot11_signal_extension;
EXPORT_SYMBOL(dot11_signal_extension);
uu_uint32 dot11_eifs_timer_value;
EXPORT_SYMBOL(dot11_eifs_timer_value);
uu_uint32 dot11_rts_threshold_value;
EXPORT_SYMBOL(dot11_rts_threshold_value);
uu_uint32 uu_dot11_self_cts_r;
EXPORT_SYMBOL(uu_dot11_self_cts_r);
uu_uint32 dot11_difs_value;
EXPORT_SYMBOL(dot11_difs_value);
uu_uint32 dot11_aifs_value[UU_WLAN_MAX_QID];
EXPORT_SYMBOL(dot11_aifs_value);
uu_uint32 dot11_cwmin_value_ac[UU_WLAN_MAX_QID];
EXPORT_SYMBOL(dot11_cwmin_value_ac);
uu_uint32 dot11_cwmax_value_ac[UU_WLAN_MAX_QID];
EXPORT_SYMBOL(dot11_cwmax_value_ac);
uu_uint32 dot11_cwmin_value;
EXPORT_SYMBOL(dot11_cwmin_value);
uu_uint32 dot11_cwmax_value;
EXPORT_SYMBOL(dot11_cwmax_value);
uu_uint32 dot11_txop_limit_value[UU_WLAN_MAX_QID];
EXPORT_SYMBOL(dot11_txop_limit_value);
uu_uint32 dot11_short_retry_count;
EXPORT_SYMBOL(dot11_short_retry_count);
uu_uint32 dot11_long_retry_count;
EXPORT_SYMBOL(dot11_long_retry_count);
uu_uint32 dot11_slot_timer_value;
EXPORT_SYMBOL(dot11_slot_timer_value);
uu_uint32 dot11_aPhyRxStartDelay;
EXPORT_SYMBOL(dot11_aPhyRxStartDelay);

uu_uint32         uu_dot11_qos_mode_r;
EXPORT_SYMBOL(uu_dot11_qos_mode_r);
uu_wlan_op_mode_t uu_dot11_op_mode_r;
EXPORT_SYMBOL(uu_dot11_op_mode_r);
uu_uchar          uu_dot11_sta_mac_addr_r[6];
EXPORT_SYMBOL(uu_dot11_sta_mac_addr_r);

uu_uchar          uu_dot11_sta_bssid_r[6];
EXPORT_SYMBOL(uu_dot11_sta_bssid_r);
#ifdef UU_WLAN_TSF /*TODO: Yet to implement in RTL */
uu_uint64 uu_wlan_tsf_r;
EXPORT_SYMBOL(uu_wlan_tsf_r);
#endif
uu_uint32 uu_wlan_phy_tx_delay_r;
EXPORT_SYMBOL(uu_wlan_phy_tx_delay_r);
uu_uint8 uu_wlan_last_beacon_tx_r;
EXPORT_SYMBOL(uu_wlan_last_beacon_tx_r);

#if 0 /* No longer required, as beacons are not triggered in LMAC now */
uu_uint8 uu_wlan_ibss_send_beacon_flag_r;
EXPORT_SYMBOL(uu_wlan_ibss_send_beacon_flag_r);
#endif /* #if 0 */

#ifdef UU_WLAN_TPC
uu_uint32 dot11NumberSupportedPowerLevelsImplemented_g;
EXPORT_SYMBOL(dot11NumberSupportedPowerLevelsImplemented_g);
uu_uint8 uu_wlan_utpl_threshold_11ac_g[CBW80_80];
EXPORT_SYMBOL(uu_wlan_utpl_threshold_11ac_g);
uu_uint8 uu_wlan_utpl_threshold_11n_g[CBW80_80];
EXPORT_SYMBOL(uu_wlan_utpl_threshold_11n_g);
uu_uint8 uu_wlan_tp_level_diff_g;
EXPORT_SYMBOL(uu_wlan_tp_level_diff_g);
uu_uint8 uu_wlan_btpl_ac_g[CBW80_80 * UU_WLAN_AC_MAX_AC];
EXPORT_SYMBOL(uu_wlan_btpl_ac_g);
uu_uint8 uu_wlan_retry_11ac_tpl_inc_g;
EXPORT_SYMBOL(uu_wlan_retry_11ac_tpl_inc_g);
uu_uint8 uu_wlan_retry_ofdm_tpl_inc_g;
EXPORT_SYMBOL(uu_wlan_retry_ofdm_tpl_inc_g);
#endif


uu_uint8 uu_wlan_int_status_reg_g;
EXPORT_SYMBOL(uu_wlan_int_status_reg_g);

/*
 * power save registers
 */

/* This register is set when STA is configured as PS-STA. */
uu_bool uu_wlan_ps_mode_r;
EXPORT_SYMBOL(uu_wlan_ps_mode_r);

/* This register is set when STA is configured as U-APSD-STA. */
uu_bool uu_wlan_uapsd_mode_r = UU_TRUE;


/* TODO: Must no hard-code AID.  AID comes only after association, and UMAC has to set it. Fix this */
uu_uint16 uu_wlan_sta_aid_r = 1;


/* EOF */

