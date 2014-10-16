/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_umac_tpc.h                                      **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                            **
** All rights reserved.                                                       **
** http://www.uurmi.com                                                       **
**                                                                            **
** All information contained herein is property of Uurmi Systems              **
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

#ifndef __UU_WLAN_UMAC_TPC_H__
#define __UU_WLAN_UMAC_TPC_H__

#ifdef UU_WLAN_TPC

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_queue.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_rate.h"
#include "uu_wlan_mgmt_frame.h"

/** Function for doing ceiling division */
#define CEIL(x,y)   (((x)/(y)) + ((x)%(y) != 0))

/** Defining the mitigation requirement for calculating the power constraint */
#define UU_WLAN_TPC_MITIGATION      3
#define UU_WLAN_BTPL_MUL_FORSTA     2

/** Defining for supporting VHT Tx Power Envelope */
#define UU_WLAN_LOCAL_MXTPWR_COUNT      0x07
#define UU_WLAN_LOCAL_MXTPWR_UNIT       0x07
#define UU_WLAN_LOCAL_MXTPWR_UNIT_SHIFT 3


/** Maximum value for the RSSI in rxvector provided by the PHY */
#define UU_WLAN_RSSI_MAXIMUM                256

#define UU_WLAN_CONST_RATE_COUNT			5

/**
 * For multiple protection, the level need to
 * be incremented with respect to B_TPL for 11ac */
#define UU_WLAN_MULTIPROT_11AC_LVL_INC      5
/**
 * For multiple protection, the level need to
 * be incremented with respect to B_TPL for ofdm */
#define UU_WLAN_MULTIPROT_OFDM_LVL_INC      1

/**
 * Levels for the path loss obtained by the TPC Report.
 * Refer to uu_wlan_recv.c */
#define Path_Loss_Range_LessThan_20m        0
#define Path_Loss_Range_LessThan_50m        1
#define Path_Loss_Range_LessThan_100m       2
#define Path_Loss_Range_Beyond_100m         3
#define Path_Loss_MaxValue_LessThan_20m     72
#define Path_Loss_MaxValue_LessThan_50m     80
#define Path_Loss_MaxValue_LessThan_100m    86


/**
 * Threshold for link margin obtained by the TPC Report.
 * The levels for incrementing or decrementing the TPL
 * as per the Link margin value
 */
#define Min_Link_Threshold                  50
#define Max_Link_Threshold                  80
#define TPC_Link_Min_11ac_Inc               5
#define TPC_Link_Max_11ac_Dec               3
#define TPC_Link_Min_Ofdm_Inc               1
#define TPC_Link_Max_Ofdm_Dec               1

/**
 * TODO: If the resolution of the uu_wlan_transmit_power_dBm
 * changes, then value of 3 also needs to be changed */
#define UU_WLAN_DBM_DOUBLING_FACTOR         3

/** Threshold for WAC-TFAR Stats */
#define WAC_TFAR_Tmin                       400     /* 4096 * 10/100 */
#define WAC_TFAR_Tmax                       3700    /* 4096 * 90/100 */
#define TPC_WAC_TFAR_Ofdm_Inc               1
#define TPC_WAC_TFAR_Ofdm_Dec               1
#define TPC_WAC_TFAR_11ac_Inc               5
#define TPC_WAC_TFAR_11ac_Dec               3

/** For TPC Request Timer */
#define TPC_Threshold_Ofdm                  7
#define TPC_Threshold_11ac                  114

/**
 * TODO: TEMP FIX: for 250ms
 * Since beacon is 100 ms defined as 800000.
 * After association after this much interval
 * we are trigerring the tpc request
 */
#define UU_REG_LMAC_TPCREQ_INRVL            1800000

/** The maximum TPL implemented for VHT TPL */
extern uu_uint8 dot11TxPowerLevelExtended;

/** Definition used for Link Margin */
extern uu_uint8 TPC_Link_Max_Dec;
extern uu_uint8 TPC_Link_Min_Inc;

/** Definition used for TPC Request timer */
extern uu_int32 TPC_Request_Timer_Interval;


/* TODO: What is this silly enumeration? */
/** Defining the TPL Levels */
typedef enum uu_wlan_tplevel
{
    /** Value ranges from 0 - 128,
     * 0 will be only used when TPC is not implemented
     * that represents default TPC value
     */

    /**
     * Normally, if TPC is implemented, then in TxVector,
     * the value of Txpwr_level ranges from 1-8 for OFDM,
     * and 1-128 for VHT TPL
     */

    UU_WLAN_TPLevel0,

    UU_WLAN_TPLevel1,
    UU_WLAN_TPLevel2,
    UU_WLAN_TPLevel3,
    UU_WLAN_TPLevel4,
    UU_WLAN_TPLevel5,
    UU_WLAN_TPLevel6,
    UU_WLAN_TPLevel7,
    UU_WLAN_TPLevel8,

    UU_WLAN_TPLevel9,
    UU_WLAN_TPLevel10,
    UU_WLAN_TPLevel11,
    UU_WLAN_TPLevel12,
    UU_WLAN_TPLevel13,
    UU_WLAN_TPLevel14,
    UU_WLAN_TPLevel15,
    UU_WLAN_TPLevel16,
    UU_WLAN_TPLevel17,
    UU_WLAN_TPLevel18,
    UU_WLAN_TPLevel19,
    UU_WLAN_TPLevel20,
    UU_WLAN_TPLevel21,
    UU_WLAN_TPLevel22,
    UU_WLAN_TPLevel23,
    UU_WLAN_TPLevel24,
    UU_WLAN_TPLevel25,
    UU_WLAN_TPLevel26,
    UU_WLAN_TPLevel27,
    UU_WLAN_TPLevel28,
    UU_WLAN_TPLevel29,
    UU_WLAN_TPLevel30,
    UU_WLAN_TPLevel31,
    UU_WLAN_TPLevel32,
    UU_WLAN_TPLevel33,
    UU_WLAN_TPLevel34,
    UU_WLAN_TPLevel35,
    UU_WLAN_TPLevel36,
    UU_WLAN_TPLevel37,
    UU_WLAN_TPLevel38,
    UU_WLAN_TPLevel39,
    UU_WLAN_TPLevel40,
    UU_WLAN_TPLevel41,
    UU_WLAN_TPLevel42,
    UU_WLAN_TPLevel43,
    UU_WLAN_TPLevel44,
    UU_WLAN_TPLevel45,
    UU_WLAN_TPLevel46,
    UU_WLAN_TPLevel47,
    UU_WLAN_TPLevel48,
    UU_WLAN_TPLevel49,
    UU_WLAN_TPLevel50,
    UU_WLAN_TPLevel51,
    UU_WLAN_TPLevel52,
    UU_WLAN_TPLevel53,
    UU_WLAN_TPLevel54,
    UU_WLAN_TPLevel55,
    UU_WLAN_TPLevel56,
    UU_WLAN_TPLevel57,
    UU_WLAN_TPLevel58,
    UU_WLAN_TPLevel59,
    UU_WLAN_TPLevel60,
    UU_WLAN_TPLevel61,
    UU_WLAN_TPLevel62,
    UU_WLAN_TPLevel63,
    UU_WLAN_TPLevel64,
    UU_WLAN_TPLevel65,
    UU_WLAN_TPLevel66,
    UU_WLAN_TPLevel67,
    UU_WLAN_TPLevel68,
    UU_WLAN_TPLevel69,
    UU_WLAN_TPLevel70,
    UU_WLAN_TPLevel71,
    UU_WLAN_TPLevel72,
    UU_WLAN_TPLevel73,
    UU_WLAN_TPLevel74,
    UU_WLAN_TPLevel75,
    UU_WLAN_TPLevel76,
    UU_WLAN_TPLevel77,
    UU_WLAN_TPLevel78,
    UU_WLAN_TPLevel79,
    UU_WLAN_TPLevel80,
    UU_WLAN_TPLevel81,
    UU_WLAN_TPLevel82,
    UU_WLAN_TPLevel83,
    UU_WLAN_TPLevel84,
    UU_WLAN_TPLevel85,
    UU_WLAN_TPLevel86,
    UU_WLAN_TPLevel87,
    UU_WLAN_TPLevel88,
    UU_WLAN_TPLevel89,
    UU_WLAN_TPLevel90,
    UU_WLAN_TPLevel91,
    UU_WLAN_TPLevel92,
    UU_WLAN_TPLevel93,
    UU_WLAN_TPLevel94,
    UU_WLAN_TPLevel95,
    UU_WLAN_TPLevel96,
    UU_WLAN_TPLevel97,
    UU_WLAN_TPLevel98,
    UU_WLAN_TPLevel99,
    UU_WLAN_TPLevel100,
    UU_WLAN_TPLevel101,
    UU_WLAN_TPLevel102,
    UU_WLAN_TPLevel103,
    UU_WLAN_TPLevel104,
    UU_WLAN_TPLevel105,
    UU_WLAN_TPLevel106,
    UU_WLAN_TPLevel107,
    UU_WLAN_TPLevel108,
    UU_WLAN_TPLevel109,
    UU_WLAN_TPLevel110,
    UU_WLAN_TPLevel111,
    UU_WLAN_TPLevel112,
    UU_WLAN_TPLevel113,
    UU_WLAN_TPLevel114,
    UU_WLAN_TPLevel115,
    UU_WLAN_TPLevel116,
    UU_WLAN_TPLevel117,
    UU_WLAN_TPLevel118,
    UU_WLAN_TPLevel119,
    UU_WLAN_TPLevel120,
    UU_WLAN_TPLevel121,
    UU_WLAN_TPLevel122,
    UU_WLAN_TPLevel123,
    UU_WLAN_TPLevel124,
    UU_WLAN_TPLevel125,
    UU_WLAN_TPLevel126,
    UU_WLAN_TPLevel127,
    UU_WLAN_TPLevel128,
    UU_WLAN_TPLevelMax,
} uu_wlan_tplevel_t;

extern uu_int8 uu_wlan_transmit_power_dBm[UU_WLAN_TPLevelMax];

/** Definitions used for lower limit tables for TPC levels */
extern uu_uint8 uu_wlan_lowpload_datamgmt_ltpl[CBW80_80 * UU_WLAN_AC_MAX_AC];
extern uu_uint8 uu_wlan_highpload_datamgmt_ltpl[CBW80_80 * UU_WLAN_AC_MAX_AC];
extern uu_uint8 uu_wlan_firstframe_multiprot [CBW80_80];

/** Definitions used for WAC TFAR Histogram */
extern int wactfar[CBW80_80 * UU_WLAN_AC_MAX_AC];
extern int wtfar[CBW80_80];
extern int tfar;

/**
 * This function is called whenever the channel
 * is started in 2.4 GHz or 5 GHz spectrum.
 * This is updated whenever we change the spectrum.
 * This function initalizes L_TPL tables and
 * the parameters and tpc levels.
 */
uu_void uu_wlan_tpc_levels_init_2GHz (uu_void);
uu_void uu_wlan_tpc_levels_init_5GHz (uu_void);


typedef struct uu_mcs_rec_min_sensitivity_table {

    /** MCS Index with 20 MHz and
        BCC Coding with 800 ns GI */
    uu_int8 mcs_20_bcc_lgi;

    /** MCS Index with 40 MHz and
        BCC Coding with 800 ns GI */
    uu_int8 mcs_40_bcc_lgi;

    /** MCS Index with 80 MHz and
        BCC Coding with 800 ns GI */
    uu_int8 mcs_80_bcc_lgi;

    /** MCS Index with 160 MHz and
        BCC Coding with 800 ns GI */
    uu_int8 mcs_160_bcc_lgi;


    /** MCS Index with 20 MHz and
        LDPC Coding with 800 ns GI */
    uu_int8 mcs_20_ldpc_lgi;

    /** MCS Index with 40 MHz and
        LDPC Coding with 800 ns GI */
    uu_int8 mcs_40_ldpc_lgi;

    /** MCS Index with 80 MHz and
        LDPC Coding with 800 ns GI */
    uu_int8 mcs_80_ldpc_lgi;

    /** MCS Index with 160 MHz and
        LDPC Coding with 800 ns GI */
    uu_int8 mcs_160_ldpc_lgi;


    /** MCS Index with 20 MHz and
        BCC Coding with 400 ns GI */
    uu_int8 mcs_20_bcc_sgi;

    /** MCS Index with 40 MHz and
        BCC Coding with 400 ns GI */
    uu_int8 mcs_40_bcc_sgi;

    /** MCS Index with 80 MHz and
        BCC Coding with 400 ns GI */
    uu_int8 mcs_80_bcc_sgi;

    /** MCS Index with 160 MHz and
        BCC Coding with 400 ns GI */
    uu_int8 mcs_160_bcc_sgi;


    /** MCS Index with 20 MHz and
        LDPC Coding with 400 ns GI */
    uu_int8 mcs_20_ldpc_sgi;

    /** MCS Index with 40 MHz and
        LDPC Coding with 400 ns GI */
    uu_int8 mcs_40_ldpc_sgi;

    /** MCS Index with 80 MHz and
        LDPC Coding with 400 ns GI */
    uu_int8 mcs_80_ldpc_sgi;

    /** MCS Index with 160 MHz and
        LDPC Coding with 400 ns GI */
    uu_int8 mcs_160_ldpc_sgi;

} uu_mcs_rec_min_sensitivity_table_t;

/**
 * This represents the receiver minimum sensitivity table.
 * This is used for preparing TPC Report.
 * This is PHY dependent and needs to be updated later.
 */
extern const uu_mcs_rec_min_sensitivity_table_t
    uu_wlan_recv_min_sensitivity_table_mcs_g[UU_MAX_STREAM_VHT][UU_VHT_MCS_INDEX_MAXIMUM];
extern uu_int8 uu_wlan_recv_min_sensitivity_table_legacy[UU_SUPPORTED_CCK_RATE + UU_SUPPORTED_OFDM_RATE];

/**
 * This table is used to convert rssi level
 * to the corresponding rssi power.
 * This value is also needs to be updated
 * as per the PHY. This is PHY dependent
 */
extern uu_int8 uu_wlan_rssi_level_to_rssi_power[UU_WLAN_RSSI_MAXIMUM];

/** Represents the maximum bandwidth supported in the band */
extern uu_uint8 TPC_Max_Bandwidth;

/** Variable defined to store the previous TPC for
 * the corresponding bandwidth */
extern int prev_tpc_dBm[CBW80_80];
extern uu_void uu_wlan_configure_utpl_threshold (int tpc_dBm, uu_uint8 bandwidth);

/* Spectrum Measurement Request formation */
/**
 * This function is used for preparing TPC
 * Report as a response to TPC Request frame.
 * Prepares and sends TPC Report frame.
 * @param[in] tpc_request frame
 * @param[in] rx_fi received frame info of TPC Request.
 */
uu_void uu_wlan_process_and_send_tpc_report (uu_mgmt_action_frame_t *tpc_request,
                                                uu_wlan_rx_frame_info_t *rx_fi);

/**
 * This function is used for preparing TPC
 * Request when the timer triggers.
 * Prepares and sends TPC Request frame.
 */
uu_void uu_wlan_prepare_and_send_tpc_request (uu_void);

/**
 * This function is uesd to stop the timer
 * for triggering TPC Request to send.
 */
uu_int32 uu_wlan_tpc_timer_stop (uu_void);
//uu_int32 uu_wlan_tpc_timer_start (uu_void);

/**
 * This function is uesd to get the
 * receiver minimum sensitivity depending
 * on the received frame information as per
 * the PHY.
 * Returns reveiver minimum sensitivity, 8 bits in length.
 * @param[in] rx_fi received frame info of TPC Request.
 */
uu_int8 uu_get_rec_min_sensitivity (uu_wlan_rx_frame_info_t *rx_fi);

/**
 * This function is uesd to get the
 * OFDM TPL corresponding to VHT TPL at 5 GHz.
 * Returns OFDM TPL value, 8 bits in length.
 * @param[in] VHT TPL for 11a/n frame format.
 */
uu_uint8 get_ofdm_tpl_from_vht_tpl (uu_uint8 vht_tpl);

/**
 * This function is uesd to get the
 * CCK TPL corresponding to OFDM TPL at 2.4 GHz.
 * Returns CCK TPL value, 8 bits in length.
 * @param[in] OFDM TPL for 11b frame format.
 */
uu_uint8 get_cck_tpl_from_ofdm_tpl (uu_uint8 ofdm_tpl);

/**
 * This function is uesd to
 * update B_TPL as per the link margin and path
 * loss obtained from the TPC Report received.
 * @param[in] TPC Report
 * @param[in] RSSI power.
 */
uu_void uu_wlan_process_tpc_report (uu_mgmt_action_frame_t *tpc_report, uu_uint8 rssi);

/**
 * This function is used to increase the TPL
 * of the frame without updating B_TPL
 * when the rate control algorithm temporarily
 * changes the mcs/Nss.
 * Returns the new TPL for the frame.
 * @param[in] new mcs index (sample mcs)
 * @param[in] mcs index adapted (normal)
 * @param[in] bandwidth of the frame
 * @param[in] tpl as per the B_TPL.
 */
uu_uint8 uu_increase_tpl_new_htmcs (uu_uint8 mcs,
            int normal_htmcs, uu_uint8 bdwdth, uu_uint8 tpl);
uu_uint8 uu_increase_tpl_new_vhtmcs (uu_uint8 vht_mcs,
            int normal_vhtmcs, uu_uint8 bdwdth, uu_uint8 tpl);

/**
 * This function is used to update B_TPL
 * when the rate control algorithm adapts
 * new mcs/Nss.
 * @param[in] new mcs index (sample mcs)
 * @param[in] mcs index adapted (normal)
 */
uu_void uu_update_btpl_new_htmcs_all(uu_uint8 ht_mcs, int normal_htmcs);
uu_void uu_update_btpl_new_vhtmcs_all(uu_uint8 vht_mcs, int normal_vhtmcs);

/* Updating Cumulative TFAR, Wideband TFAR, Wideband AC TFAR */
/**
 * This function is used to update
 * cumulative TFAR, Wideband TFAR and WAC TFAR
 * statistics after getting Tx status.
 * @param[in] Tx status of the transmitted frame.
 */
uu_void uu_wlan_calc_wtfar_stats (uu_uint8 bandwidth, uu_uint8 ac, uu_uint8 failed, uu_uint8 attempt);

/**
 * This function is used to reset
 * all TFAR statistics.
 * It is called whenever the channel
 * switching is performed.
 */
uu_void uu_wlan_reset_wtfar_stats (uu_void);

#endif /* UU_WLAN_DFS */

#endif  /* __UU_WLAN_TPC_H__ */

