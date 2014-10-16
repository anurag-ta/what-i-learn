/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_rate.h                                          **
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

/* Contains declarations for rate calculations. */

#ifndef __UU_WLAN_RATE_H__
#define __UU_WLAN_RATE_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_phy_if.h"


UU_BEGIN_DECLARATIONS


/** 5 bits as MCS till index 31 is supported */
#define UU_HT_MCS_MASK           0x1f

/** As MCS in HT for Nss=1 is of 3 bits and till MCS index 31 4th and 5th bit will represent Nss */
#define UU_HT_MANDATORY_MCS_MASK 0x07

/* TODO: Review this work-around */
/** Flag 0x08 is for OFDM Flag, as CCK rates will have 4th bit as 0, proprietary implementation */
#define UU_OFDM_RATE_FLAG        0x08

/* Masking OFDM hardware value to get its bit index from legacy rate table */
#define UU_OFDM_HW_MASK_INDEX    0x07


/** Max streams possible in VHT is 8, HT is 4
 * STBC indicates the difference between space-time streams (Nsts) and
 *  the number of spatial streams (Nss) indicated by the MCS.
 * In VHT, STBC might be 0 (disabled, Nsts=Nss) / 1 (STBC used, Nsts=2Nss).
 * In HT, STBC might be 0 (disabled, Nsts=Nss) / 1 (Nsts-Nss=1) / 2 (Nsts-Nss=2).
 */
#define UU_MAX_STREAM_VHT         8
#define UU_MAX_STREAM_HT          4
#define UU_MAX_MCS_PER_STREAM_HT  8

/** Rates used by hardware/driver
 * TODO : CCK Rates need to map to 4 bits that only mac-phy will understand
 * We can choose value till 3 bits as 4th bit is always 1 for OFDM
 * So for CCK we can have values from 0x01 to 0x07
 * While changing this we need to change the condition in rate.c and duration.c for checking if it is CCK/OFDM.
 * CCK flag can be defined as 0x07 and compare the hw-value for CCK as it should be less than 0x08 ie 8
 */
/* TODO: Review this work-around */
/** 0x0a, as per spec. Added 0x80, to avoid conflict with OFDM rate value */
#define UU_HW_RATE_1M   0x04
/** 0x14 for 2 Mbps */
#define UU_HW_RATE_2M   0x05
/** 0x37 for 5.5 Mbps */
#define UU_HW_RATE_5M5  0x06
/** 0x6e for 11 Mbps */
#define UU_HW_RATE_11M  0x07


/** For ofdm, below are hardware values assigned to
 * ofdm rates as per 18.3.4.2 in 802.11 REV-mb D12
 */
#define UU_HW_RATE_6M   0x0b
#define UU_HW_RATE_9M   0x0f
#define UU_HW_RATE_12M  0x0a
#define UU_HW_RATE_18M  0x0e
#define UU_HW_RATE_24M  0x09
#define UU_HW_RATE_36M  0x0d
#define UU_HW_RATE_48M  0x08
#define UU_HW_RATE_54M  0x0c

/** Supported number of rates for CCK */
#define UU_SUPPORTED_CCK_RATE    4

/** Supported number of rates for OFDM */
#define UU_SUPPORTED_OFDM_RATE   8

/** Represents in BIT in BSSBasicRateSet for the given legacy rates */
#define UU_RATE_1M_BIT          0
#define UU_RATE_2M_BIT          1
#define UU_RATE_5M5_BIT         2
#define UU_RATE_11M_BIT         3
#define UU_RATE_6M_BIT          4
#define UU_RATE_9M_BIT          5
#define UU_RATE_12M_BIT         6
#define UU_RATE_18M_BIT         7
#define UU_RATE_24M_BIT         8
#define UU_RATE_36M_BIT         9
#define UU_RATE_48M_BIT        10
#define UU_RATE_54M_BIT        11
#define UU_LEGACY_RATES_BIT    12

/** This is taken as per the rate identification field defined in specification
 * Nss is defined in lower 3 bits in VHT mcs
 * Mcs index is defined in 4th-7th bit in VHT mcs
 * Maximum mcs index value is 10 in VHT
 */
#define UU_VHT_NSS_FROM_MCS_MASK             0x07
#define UU_VHT_MCSINDEX_FROM_MCS_MASK        0x0f
#define UU_VHT_MCSINDEX_SHIFT_FROM_MCS          3
#define UU_VHT_MCS_INDEX_MAXIMUM               10
#define UU_VHT_MCSINDEX_FIELD_MASK           0x78

/** This is for finding modulation of ofdm using its signal field */
#define UU_OFDM_MODULATION_TYPE_IND    0x03

/** Common bits in the signal value of OFDM for the same modulation */
#define UU_OFDM_BPSK             3
#define UU_OFDM_QPSK             2
#define UU_OFDM_16QAM            1
#define UU_OFDM_64QAM            0


/** As per 802.11n REV_mb D12 section 20.3.2 and 802.11ac D3.0 section
 * 22.3.2 explaining the timings related to PPDU format of HT and VHT respectively.
 * All are in microseconds.
 */
#define PHY_PREAMBLE_TIME        16
#define PHY_SIGNAL_TIME           4
#define PHY_SYMBOL_TIME           4
#define PHY_SERVICE_NBITS        16
#define PHY_TAIL_NBITS            6
#define OFDM_SIGNAL_EXTENSION     6
#define PHY_PLCP_SHORT_TIME      96
#define PHY_PLCP_TIME           192


/** As per the PPDU format defined for non-HT and HT in 802.11 REV-mb D12, section 20.3.2 
 * and VHT in 802.11ac D3.0 section 22.3.2.
 * These macros are being used for duration calcualtion considering PPDU format.
 */
#define PHY_HT_SIG            8
#define PHY_PER_LTF           4
#define PHY_HT_STF            4
#define PHY_VHT_SIG_A         8
#define PHY_VHT_SIG_B         4
#define PHY_VHT_STF           4

#ifdef UU_WLAN_RC_RTL_TABLE
/* Dividing macro for scaler multiplication output with 0xFFFFFFFFFF (40bits resolution) */
#define UU_ONE_BY_RATE_RESOLUTION	40
#endif

/** This structure is for legacy rates containing index,
 * rate in Mbps, rate in multiple of 250Kbps and
 * the hardware value of the rate.
 */
typedef struct uu_legacy_rate_table {

    /** This will be used while choosing basic rate */
    int bit_index;

    /** Rate in Mbps */
    /* float mbps; */

    /** Rate in 250 Kbps */
    //uu_uint64 kbps250;
    uu_uint64 kbps250;

    /** Hardware value for the rate */
    uu_uint8 hw_rate;

} uu_legacy_rate_table_t;


/** This structure is for HT mcs/rates containing mcs index,
 * rate in multiple of 250Kbps for 20/40 MHz with LGI/SGI
 * and the hardware value of the non-HT reference rate for corresponding mcs index.
 */
typedef struct uu_ht_mcs_rate_table {

    /** rate index */
    uu_uint8 rate_index_n;

    /** rate in 250kbps [20MHz] */
    uu_uint64 mcs_rate_20;

    /** rate in 250kbps [40MHz] */
    uu_uint64 mcs_rate_40;

    /** rate in 250kbps [20MHz] SGI */
    uu_uint64 mcs_rate_20_sgi;

    /** rate in 250kbps [40MHz] SGI */
    uu_uint64 mcs_rate_40_sgi;

    /** OFDM hw_rate */
    uu_uint8 ofdm_rate;

} uu_ht_mcs_rate_table_t;

/** The table is maintained as per 802.11n standard REV_mb D12, section 20.6 */
extern const struct uu_ht_mcs_rate_table uu_ht_mcs_table_g[];

/** Total size for the VHT MCS table */
#define UU_MCS_AC_TABLE_SIZE    (UU_VHT_MCS_INDEX_MAXIMUM * UU_MAX_STREAM_VHT)

/** This structure is for VHT mcs/rates containing mcs index,
 * rate in multiple of 250Kbps for 20/40/80/160/80+80 MHz with LGI/SGI
 * and the hardware value of the non-HT reference rate for corresponding mcs index.
 */
typedef struct uu_vht_mcs_rate_table {

    /** rate index */
    uu_uint8 mcs_nss_index_ac;

    /** rate in 250kbps [20MHz] */
    uu_uint64 mcs_ac_rate_20;

    /** rate in 250kbps [40MHz] */
    uu_uint64 mcs_ac_rate_40;

    /** rate in 250kbps [80MHz] */
    uu_uint64 mcs_ac_rate_80;

    /** rate in 250kbps [160MHz] */
    uu_uint64 mcs_ac_rate_160;

    /** rate in 250kbps [20MHz] SGI */
    uu_uint64 mcs_ac_rate_20_sgi;

    /** rate in 250kbps [40MHz] SGI */
    uu_uint64 mcs_ac_rate_40_sgi;

    /** rate in 250kbps [80MHz] SGI */
    uu_uint64 mcs_ac_rate_80_sgi;

    /** rate in 250kbps [160MHz] SGI */
    uu_uint64 mcs_ac_rate_160_sgi;

    /** OFDM hw_rate */
    uu_uint8 ofdm_rate;

} uu_vht_mcs_rate_table_t;


/** The table is maintained as per 802.11ac standard D3.0, section 22.5 */
extern const struct uu_vht_mcs_rate_table  uu_vht_mcs_table_g[];

/** This is to determine the basic rate that is less than equal to the given rate
 * Returns hardware value of the basic rate, 7 bits in length
 * @param[in] modulation_type of the packet, 3 bits in length.
 * @param[in] frame_format of the packet, 2 bits in length.
 * @param[in] lmac_rate of the packet for which basic rate is needed to be determined, 4bits in length.
 */
extern uu_uint8 uu_assign_basic_rate (uu_uint8 modulation_type, uu_uint8 frame_format, uu_uint8 lmac_rate);

#if 0 /* Not required, as it is implemented in PHY */
/** This functions calcualtes L_length for HT-MF
 * Returns L-length in ht mixed mode, 12 bits in length
 * @param[in] txvec of the packet to be transmitted
 */
extern uu_uint16 uu_calc_htmf_l_length (uu_wlan_tx_vector_t *txvec);
#endif


/** This function calculates the rate as per PPDU format and the information present
 * in txvector as described in 802.11n D12 section 20.3.2.
 * Returns duration of the frame, 16 bits in length
 * @param[in] txvec of the packet to be transmitted
 */
extern uu_uint16 uu_calc_frame_duration (uu_wlan_tx_vector_t *txvec);

#endif /* __UU_WLAN_RATE_H__ */

/* EOF */


