/*************************************************************************
**                                                                      **
** File name :  uu_wlan_phy_if.h                                        **
**                                                                      **
** Copyright © 2013, Uurmi Systems                                     **
** All rights reserved.                                                 **
** http://www.uurmi.com                                                 **
**                                                                      **
** All information contained herein is property of Uurmi Systems        **
** unless otherwise explicitly mentioned.                               **
**                                                                      **
** The intellectual and technical concepts in this file are proprietary **
** to Uurmi Systems and may be covered by granted or in process national**
** and international patents and are protect by trade secrets and       **
** copyright law.                                                       **
**                                                                      **
** Redistribution and use in source and binary forms of the content in  **
** this file, with or without modification are not permitted unless     **
** permission is explicitly granted by Uurmi Systems.                   **
**                                                                      **
*************************************************************************/

#ifndef __UU_WLAN_PHY_IF_H__
#define __UU_WLAN_PHY_IF_H__

#include "uu_datatypes.h"


/*
** These are the 'Rx-End' status codes given by PHY to MAC in
**  the 'PHY-RXEND.indication' primitive.
*/
/* No Error. This value is used to indicate that no error occurred during the receive process in the PLCP */
#define UU_WLAN_RX_END_STAT_NO_ERROR            0
/* FormatViolation. This value is used to indicate that the format of the received PPDU was in error. */
#define UU_WLAN_RX_END_STAT_FORMAT_ERROR        1
/* CarrierLost. This value is used to indicate that during the reception of the incoming PSDU, the
carrier was lost and no further processing of the PSDU can be accomplished. */
#define UU_WLAN_RX_END_STAT_CARRIER_ERROR       2
/* UnsupportedRate. This value is used to indicate that during the reception of the incoming PPDU, a
nonsupported date rate was detected.*/
#define UU_WLAN_RX_END_STAT_RATE_ERROR          3


typedef enum uu_wlan_frame_format
{
    /* NON_HT indicates Clause 15, Clause 17, Clause 18, or Clause 19
    PPDU formats or non-HT duplicated PPDU format. In this case, the
    modulation is determined by the NON_HT_MODULATION
    parameter. */
    UU_WLAN_FRAME_FORMAT_NON_HT = 0,
    /* HT_MF indicates HT-mixed format. */
    UU_WLAN_FRAME_FORMAT_HT_MF = 1,
    /* HT_GF indicates HT-greenfield format. */
    UU_WLAN_FRAME_FORMAT_HT_GF = 2,

    /* 802.11AC changes */
    /* VHT indicates VHT format. */
    UU_WLAN_FRAME_FORMAT_VHT = 3
} uu_wlan_frame_format_t;

typedef enum uu_wlan_non_ht_modulation
{
    ERP_DSSS = 0,
    ERP_CCK = 1,
    ERP_OFDM = 2,
    ERP_PBCC = 3,
    DSSS_OFDM = 4,
    OFDM = 5,
    NON_HT_DUP_OFDM = 6
} uu_wlan_non_ht_modulation_t;


typedef enum uu_wlan_ch_bndwdth_type
{
    /* As per draft 802.11ac_D3.0 section 22.2.3 */
    CBW20 = 0,
    CBW40 = 1,
    CBW80 = 2,
    CBW160 = 3,
    CBW80_80 = 4
} uu_wlan_ch_bndwdth_type_t;


typedef enum uu_wlan_ch_offset_type
{
    /* indicates the use of a 20 MHz channel (that is not part of a 40 MHz channel). */
    CH_OFF_20 = 0,
    /* indicates the entire 40 MHz channel. */
    CH_OFF_40 = 1,
    /* indicates the upper 20 MHz of the 40 MHz channel */
    CH_OFF_20U = 2,
    /* indicates the lower 20 MHz of the 40 MHz channel. */
    CH_OFF_20L = 3
} uu_wlan_ch_offset_type_t;


typedef struct uu_wlan_tx_vector_per_user_params
{
    /** Byte : 1 */
    /** User position */
    uu_uint8  user_position : 2;

    /** Number of Space-time streams */
    uu_uint8  num_sts : 3;

    /** FEC coding per user. Valid values are: 0 for BCC_CODING, 1  for LDPC_CODING */
    uu_uint8  is_fec_ldpc_coding : 1;

    uu_uint8  reserved : 2;

    /** Bytes : 2, 3 & 4 */

    /** APEP length (applicable for 11ac) per user */
    uu_uint32 apep_length : 20;

    /** VHT MCS */
    uu_uint8  vht_mcs : 4;

} uu_wlan_tx_vector_per_user_params_t;


/** Tx vector, sent by MAC to PHY before sending frame. Also part of TxFrameInfo.
 * WARNING : Uses bitfields whose packing is compiler dependent.
 * RTL conversion should ensure that it matches with C version of test setup
 */
typedef struct uu_wlan_tx_vector
{
    /* Byte : 1 */

    /** Determines the format of the PPDU (non-HT, HT-MF, HT-GF, VHT) */
    uu_wlan_frame_format_t      format : 2;

    /** Indicates whether the packet is transmitted using 40 MHz or 20 MHz
     * channel width.
     */
    uu_wlan_ch_bndwdth_type_t    ch_bndwdth :3;

    /** For TX side only.
     * HT-MF and HT-GF: The N_TX parameter indicates the number of transmit chains.
     * Possible values are 1 to 8.
     */
    uu_uint8                     n_tx : 3;

    /* Byte : 2 */
    /** Transmit Power Level.
     * The allowed values for the TXPWR_LEVEL parameter are in the range
     * from 1 to 8. This parameter is used to indicate which of the available
     * TxPowerLevel attributes defined in the MIB shall be used for the current
     * transmission
     * For VHT: range is from 1 to 128.
     */
#ifdef UU_WLAN_TPC
    uu_uint8                     txpwr_level : 8;
#else
    uu_uint8                     txpwr_level : 7;

    /** Reserved based on the vectors doc shared by PHY */
    uu_uint8                     reserved1  : 1;
#endif
    /* Byte : 3 & 4 */

    /** L_datarate 
     * NON-HT: Indicates the rate used to transmit the PSDU in megabits per second.
     * Allowed values depend on the value of the NON_HT_MODULATION
     *    parameter as follows:
     *    ERP-DSSS: 1 and 2
     *    ERP-CCK: 5.5 and 11
      *    ERP-PBCC: 5.5, 11, 22, and 33
     *    DSSS-OFDM, ERP-OFDM, NON_HT_DUP_OFDM: 6, 9, 12, 18, 24, 36, 48, and 54
     *    OFDM: 6, 9, 12, 18, 24, 36, 48, and 54
     * HT-MF: Indicates the data rate value that is in the L-SIG. This use is defined in 9.13.4.
     * HT-GF: Not present.
     */
    uu_uint16                   L_datarate : 4;

    /** L_Length
     * NON_HT: Indicates the length of the PSDU in octets in the range of 1 to 4095 
     * HT-MF: Indicates the value in the Length field of the L-SIG in the range of 1 to 4095.
     * HT-GF: not present.
     */
    uu_uint16                   L_length : 12;

    /* Byte : 5 & 6 */
    /** Service
     * For TX side only.
     * NON-HT: Scrambler initialization, 7 null bits + 9 reserved null bits
     *    This present, If NON_HT_MODULATION is one of
     *     - DSSS-OFDM
     *     - ERP-OFDM
     *     - OFDM
     * HT-MF & HT-GF:   Scrambler initialization, 7 null bits + 9 reserved null bits
     * Not present if FORMAT is VHT 
     */

    /** Note : Not supporting now. PHY is expecting value 0 from MAC */
    uu_uint16                    service : 16;

    /* Byte : 7 */

    /** Antenna used 
     * For TX side only and Optional.
     * HT-MF and HT-GF: Indicates which antennas of the available antennas are used in the
     * transmission. The length of the field is 8 bits. A 1 in bit position n, relative
     * to the LSB, indicates that antenna n is used.In 11N, At most 4 bits out of 8 may be set to 1.
     * NOTE: This field is present only if ASEL is applied.
     */
    /** Note : In current implementation PHY is expecting 0xFF from MAC */
    uu_uint8                     antenna_set : 8;

    /* Byte : 8 */
    /** Number of extension spatial streams 
     * Indicates the number of extension spatial streams that are sounded during 
     * the extension part of the HT training in the range of 0 to 3. 
     */
    /** Note : Not supporting now. Expecting all zero's from MAC */
    uu_uint8                     num_ext_ss : 2;

    /** No signal Extenstion
     * For TX side only.
     * FORMAT is NON_HT and NON_HT_MODULATION is ERPOFDM/DSSSOFDM/NON_HT_DUPOFDM
     * HT-MF and HT-GF: 
     *  Indicates whether signal extension needs to be applied at the end of transmission.
     *    Value 1 indicates no signal extension is present.
     *    Value 0 indicates signal extension may be present depending on other TXVECTOR parameters.
     */
    /** Note : Not using now. PHY is expecting 0 from MAC */
    uu_uint8                     is_no_sig_extn : 1;

    /** Channel offset
     * For TX side only.
     * Indicates which portion of the channel is used for transmission. Refer to
     * Table 20-2 for valid combinations of CH_OFFSET and CH_BANDWIDTH.
     */
    /** Note : Supporting 20, 40, 20U and 20L */
    uu_wlan_ch_offset_type_t     ch_offset : 2;

    /** NON-HT-MODULATION
     */
    uu_wlan_non_ht_modulation_t modulation : 3;

    /* Byte : 9 */
    /** MCS 
     * HT-MF and HT-GF: Selects the modulation and coding scheme used in the transmission of the
     * packet. The value used in each MCS is the index defined in 20.6.
     * Integer: range 0 to 76. Values of 77 to 127 are reserved.
     * The interpretation of the MCS index is defined in 20.6.
     */
    uu_uint16                    mcs : 7;

    /** PREAMBLE TYPE 
     * Preamable type is applicable, only if
     * FORMAT is NON_HT and NON_HT_MODULATION is one of 
     *   ERP-DSSS 
     *   ERP-CCK 
     *   ERP-PBCC 
     *   DSSS-OFDM
     *
     * Value of this flag is 1, for long-premable. 0 for short-preamble.
     */
    uu_uint16                    is_long_preamble : 1;

    /* Byte : 10 & 11 */
    /** Length
     * Indicates the length of an HT PSDU in the range of 0 to 65 535 octets. A 
     * value of 0 indicates a NDP that contains no data symbols after the HT 
     * preamble (see 20.3.9). 
     */
    uu_uint32 ht_length : 16;

    /* Byte : 12 */

    /** Smoothing
     * HT-MF and HT-GF: Indicates whether frequency-domain smoothing is recommended as part of channel estimation.
     * Value 0 indicates smoothing is not recommended and 1 indicates  smoothing is recommended.
     */
    /** Note : In current implementation PHY expecting 0 from MAC */
    uu_uint8    is_smoothing : 1;

    /** Sounding 
     * HT-MF and HT-GF: Indicates whether this packet is a sounding packet.
     *    Value 0 indicates the packet is not a sounding packet and 1 indicates packet is sounding packet.
     */
    /** Note : In current implementation PHY expecting 0 from MAC */
    uu_uint8     is_sounding : 1;

    /** Reserved based on the vectors doc shared by PHY */
    uu_uint8    reserved : 1;

    /** Aggregation 
      * HT-MF and HT-GF: Indicates whether the PSDU contains an A-MPDU.
     *    Value 0 indicates that the packet is not aggregated and 1 indicates aggregated packet.
     */
    uu_uint8  is_aggregated : 1;

    /** STBC
     * Takes values 0 (disable) or 1 (enable) for VHT.
     * HT-MF and HT-GF: Indicates the difference between the number of space-time streams (Nsts )
     *  and the number of spatial streams (Nss ) indicated by the MCS as follows:
     *    0 indicates no STBC (Nsts = Nss ).
     *    1 indicates Nsts - Nss = 1.
     *    2 indicates Nsts - Nss = 2.
     *    Value of 3 is reserved.
     */
    /** Note : Current implementation takes values: 0 or 1 only */
    uu_uint8                     stbc : 2;

    /** FEC coding per user. Valid values are 0 for BCC_CODING, 1  for LDPC_CODING */
    /** Note : Supporting only BCC_CODING */
    uu_uint8    is_fec_ldpc_coding : 1;

    /** Guard Interval
     * HT-MF and HT-GF: Indicates whether a short guard interval is used in the transmission of the packet.
     * Value 1 inidcates SHORT_GI is used and 0 indicates LONG_GI is used.
     */
    /** Note : Transmitter support both LONG_GI and SHORT_GI. But reveiver supports only LONG_GI. */
    uu_uint8    is_short_GI : 1;


    /* Bytes : 13 & 14 */
    /** Prartial AID
     * Indicates the least significant 9 bits of the AID of the intended
     * recipient or 0 if intended for multiple recipients
     * Rx and Tx side.
     */
    uu_uint16 partial_aid : 9;

    /** BEAMFORMED
     * Condition : Format is VHT and Group_id is 0 to 63.
     * Set to 1 if a beamforming steering matrix is applied to the wave- 
     * form in an SU transmission as described in 20.3.11.11.2 (Spatial mapping). 
     * Set to 0 otherwise. 
     * NOTE—when BEAMFORMED is set to 1, smoothing is not recommended.
     */
     uu_uint8 is_beamformed : 1;

    /* Number of users.
     * Indicates the number of users
     * only Tx side.
     */
    uu_uint8 num_users : 2;

    /** Dynamic Bandwidth in non-HT.
     * When present, indicates whether the transmitter is capable of Dynamic Bandwidth Operation.
      * 0 for Static or 1 for Dynamic.
      */
    /** Note : Not supporting Dynamic now. Value is always 0 */
    uu_uint8 is_dyn_bw : 1;

    /** Channel Bandwidth in non-HT
     * When present, indicates the BW to signal in the scrambler init field.
     * Tx and Rx side,for non NON_HT format only.
      */
    /** Note : Not supporting now. Expecting default value as 0 */
    uu_wlan_ch_bndwdth_type_t indicated_chan_bw : 3;

    /* Byte : 15 */
    /** Group ID
     * Indicates the Group ID
     * TX and Rx side. Condition : Format is VHT.
     * Integer: range 0-63 (see Table 22-12 (Fields in the VHT-SIG-A field)) 
     * A value of 0 or 63 indicates an SU PPDU. A value in the range 1 to 62 indicates an MU PPDU. 
     */
    /** Note : PHY expecting default values as per the specification */
    uu_uint8 group_id : 6;

    /** TXOP_PS_NOT_ALLOWED
     * FORMAT is VHT 
     * Indicates whether or not a VHT AP allows non-AP VHT STAs in TXOP power save mode to enter 
     * Doze state during the TXOP. 
     * value 0 indicates that the VHT AP allows non-AP VHT STAs to enter doze mode during a TXOP.
     * value 1 indicates that the VHT AP does not allow non-AP VHT STAs to enter doze mode during a TXOP.
     */
    /** Note : Not using. Expecting all zero's from MAC */
    uu_uint8                     is_tx_op_ps_allowed : 1;

    /** TIME_OF_DEPARTURE_REQUESTED ( Optional for Tx vector)
     * Boolean value: 
     * value 1 (True) indicates that the MAC entity requests that the PHY PLCP entity measures and reports time of 
     * departure parameters corresponding to the time when the first PPDU energy is sent by the 
     * transmitting port. 
     * value 0 (False) indicates that the MAC entity requests that the PHY 
     * PLCP entity neither measures nor reports time of departure parameters. 
     */
    /** Note : Not using. Expecting all zero's from MAC */
    uu_uint8                     is_time_of_departure_req : 1; 
    /** added bytes for padding, to make size a multiple of 4-bytes*/
    uu_uint8 padding_byte0;

    /** Per user Parameters */
    uu_wlan_tx_vector_per_user_params_t tx_vector_user_params[4];

} /*__attribute__ ((packed))*/ uu_wlan_tx_vector_t;


/** Rx vector, passed from PHY to MAC before the frame reception.
 * This same Rx vector is passed on to UMAC, in RxFrameInfo.
 *
 * WARNING: Uses bitfields whose packing is compiler dependent. RTL team should take care of these during packing.
 */
typedef struct uu_wlan_rx_vector
{
    /* Byte : 1 */
    /** Determines the format of the PPDU */
    uu_wlan_frame_format_t      format : 2;

    /** NON-HT-MODULATION 
     */
    uu_wlan_non_ht_modulation_t modulation : 3;

    /** PREAMBLE TYPE 
     * Preamable type is applicable, only if
     * FORMAT is NON_HT and NON_HT_MODULATION is one of 
     *   ERP-DSSS 
     *   ERP-CCK 
     *   ERP-PBCC 
     *   DSSS-OFDM
     *
     * Value of this flag is 1, for long-premable. 0 for short-preamble.
     */
    uu_uint8                    is_long_preamble : 1;
    uu_uint8             reserved1 : 2;

    /* Byte : 2 */

    /** Indicates whether the packet is transmitted using 40 MHz or 20 MHz
     * channel width.
     */
    uu_wlan_ch_bndwdth_type_t    ch_bndwdth : 3;

    /** L-SIG valid
     * For RX side only.
     * HT-MF:  True if L-SIG Parity is valid.
     * False if L-SIG Parity is not valid.
     */
    uu_uint8                    is_L_sigvalid : 1;

    /** L_datarate 
     * NON-HT: Indicates the rate used to transmit the PSDU in megabits per second.
     * Allowed values depend on the value of the NON_HT_MODULATION
     *    parameter as follows:
     *    ERP-DSSS: 1 and 2
     *    ERP-CCK: 5.5 and 11
      *    ERP-PBCC: 5.5, 11, 22, and 33
     *    DSSS-OFDM, ERP-OFDM, NON_HT_DUP_OFDM: 6, 9, 12, 18, 24, 36, 48, and 54
     *    OFDM: 6, 9, 12, 18, 24, 36, 48, and 54
     * HT-MF: Indicates the data rate value that is in the L-SIG. This use is defined in 9.13.4.
     * HT-GF: Not present.
     */
    uu_uint16                   L_datarate : 4;

    /* Byte : 3 & 4 */
    /** L_Length
     * NON_HT: Indicates the length of the PSDU in octets in the range of 1 to 4095 
     * HT-MF: Indicates the value in the Length field of the L-SIG in the range of 1 to 4095.
     * HT-GF: not present.
     */
    uu_uint16                   L_length : 12;

    /** Dynamic Bandwidth in non-HT.
     * When present, indicates whether the transmitter is capable of Dynamic Bandwidth Operation.
     * 0 for Static or 1 for Dynamic.
     */
    /** Note : Not supporting Dynamic now. Value is always 0 */
    uu_uint8 is_dyn_bw : 1;

    /** Channel Bandwidth in non-HT
     * When present, indicates the BW to signal in the scrambler init field.
     * Tx and Rx side,for non NON_HT format only.
      */
    uu_wlan_ch_bndwdth_type_t indicated_chan_bw : 3;

    /* Byte : 5 */
    /** RSSI 
     * For RX side only.
     * The allowed values for the RSSI parameter are in the range from 0 through RSSI maximum.
     */
    uu_uint32                     rssi : 8;

    /* Byte : 6  */
    /** RCPI 
     * For RX side only.
     * HT-MF and HT-GF: Is a measure of the received RF power averaged over all the receive chains
     * in the data portion of a received frame.
     */
    uu_uint16                     rcpi : 8;

    /* Byte : 7 */
    /** Smoothing
     * HT-MF and HT-GF: Indicates whether frequency-domain smoothing is recommended as part of channel estimation.
     * Value 0 indicates smooting is not recommended and 1 indicates  smooting is recommended.
     */
    /** Note : Not supporting now. Expecting default value as 0 */
    uu_uint8      is_smoothing : 1;

    /** Sounding 
     * HT-MF and HT-GF: Indicates whether this packet is a sounding packet.
     *    Value 0 indicates the packet is not sounding packet and 1 indicates packet is sounding packet.
     */
    /** Note : Not supporting now. Expecting default value as 0 */
    uu_uint8      is_sounding : 1;

    /** Aggregation 
     * HT-MF and HT-GF: Indicates whether the PSDU contains an A-MPDU.
     *    Value 0 indicates that the packet is not aggregated and 1 indicates aggregated packet.
     */
    uu_uint8      is_aggregated : 1;

    /** Guard Interval
     * HT-MF and HT-GF: Indicates whether a short guard interval is used in the transmission of the packet.
     * Value 1 inidcates SHORT_GI is used and 0 indicates LONG_GI is used.
     */
    /** Note : Transmitter support both LONG_GI and SHORT_GI. But reveiver supports only LONG_GI. */
    uu_uint8      is_short_GI : 1;

    /** STBC
     * Takes values 0 (disable) or 1 (enable) for VHT.
     * HT-MF and HT-GF: Indicates the difference between the number of space-time streams (Nsts )
     *  and the number of spatial streams (Nss ) indicated by the MCS as follows:
     *    0 indicates no STBC (Nsts = Nss ).
     *    1 indicates Nsts - Nss = 1.
     *    2 indicates Nsts - Nss = 2.
     *    Value of 3 is reserved.
     */
    /** Note : Current implementation uses values : 0 or 1 only */
    uu_uint8      stbc : 2;

    /** Number of extension spatial streams
     * HT-MF and HT-GF: Indicates the number of extension spatial streams that are sounded during
     * the extension part of the HT training in the range of 0 to 3.
     */
    /** Note : Not supporting now. PHY expecting all zero's from MAC */
    uu_uint8      num_ext_ss : 2;

    /* Byte : 8 */

    /** MCS 
     * HT-MF and HT-GF: Selects the modulation and coding scheme used in the transmission of the
     * packet. The value used in each MCS is the index defined in 20.6.
     * Integer: range 0 to 76. Values of 77 to 127 are reserved.
     * The interpretation of the MCS index is defined in 20.6.
     */
    uu_uint16     mcs : 7;

    /** FEC coding per user. Valid values are: 0 for BCC_CODING, 1  for LDPC_CODING */
    /** Note : Supporting only BCC_CODING */
    uu_uint8      is_fec_ldpc_coding : 1;

    /* Bytes : 9 , 10, 11, 12*/

    /** RX start of frame offset 
     *   0 to 232– 1. An estimate of the offset (in 10 ns units) from the 
     * point in time at which the start of the preamble corresponding to 
     * the incoming frame arrived at the receive antenna port to the 
     * point in time at which this primitive is issued to the MAC. 
     */
     uu_uint32    rx_start_of_frame_offset : 32;

    /* Byte : 13 */
    /** RX side Optional.
     * HT-MF and HT-GF: Indicates the MCS that the STA.s receiver recommends.
     */
    uu_uint16     rec_mcs : 7;
    uu_uint8      reserved2 : 1;

    /* Bytes : 14, 15 & 16 */

    /** Length, for both HT & VHT frames.
     * Indicates the length of an HT PSDU(VHT PSDU)in the range of 0 to 65 535 octets(0 to 1MB). A 
     * value of 0 indicates a NDP that contains no data symbols after the HT(VHT).
     */
    uu_uint32 psdu_length : 20;

    /** Number of space time steams
     * Indicates the number of space-time streams
     * Integer: range 1-8 for SU, 0-4 for MU
     * Tx side(if MU is there) ,Rx side
     */
    uu_uint8 num_sts : 3;

    /** BEAMFORMED
     * Condition : Format is VHT and Group_id is 0 to 63.
     * Set to 1 if a beamforming steering matrix is applied to the wave- 
     * form in an SU transmission as described in 20.3.11.11.2 (Spatial mapping). 
     * Set to 0 otherwise. 
     * NOTE—when BEAMFORMED is set to 1, smoothing is not recommended.
     */
     uu_uint8 is_beamformed : 1;
 
    /* Bytes : 17  & 18 */

    /** Prartial AID
     * Indicates the least significant 9 bits of the AID of the intended
     * recipient or 0 if intended for multiple recipients
     * Rx and Tx side.
     */
    uu_uint16 partial_aid : 9;

    /** Group ID
     * Indicates the Group ID
     * TX and Rx side. Condition : Format is VHT.
     * Integer: range 0-63 (see Table 22-12 (Fields in the VHT-SIG-A field)) 
     * A value of 0 or 63 indicates an SU PPDU. A value in the range 1 to 62 indicates an MU PPDU. 
     */
    /** Note : PHY expecting default values as per the specification */
    uu_uint8 group_id : 6;

    /** TXOP_PS_NOT_ALLOWED
     * FORMAT is VHT 
     * Indicates whether or not a VHT AP allows non-AP VHT STAs in TXOP power save mode to enter 
     * Doze state during the TXOP. 
     * value 0 indicates that the VHT AP allows non-AP VHT STAs to enter doze mode during a TXOP.
     * value 1 indicates that the VHT AP does not allow non-AP VHT STAs to enter doze mode during a TXOP.
     */
    /** Note : Not using. Expecting all zero's from MAC */
    uu_uint8                     is_tx_op_ps_allowed : 1;

    /* Byte : 19 */
    /** SNR
     * For RX side only.
     * HT-MF and HT-GF: 
     * If CHAN_MAT_TYPE is CSI_MATRICES: Is a measure of the received SNR per chain. 
     *    SNR indications of 8 bits are supported.
     * If CHAN_MAT_TYPE is COMPRESSED_SV/NON_COMPRESSED_SV: Is a measure of the received SNR per stream. 
     *    SNR indications of 8 bits are supported.
     */
    uu_uint16                     snr : 8;

    /** added bytes for padding */
    uu_uint8 padding_byte0;

} /*__attribute__ ((packed))*/ uu_wlan_rx_vector_t;


typedef enum uu_wlan_phy_req_type
{
    /*
    This primitive is generated by the MAC sublayer for the local PHY entity at the end of a NAV timer. This
    request can be used by some PHY implementations that may synchronize antenna diversity with slot timings.
    */
    UU_WLAN_PHY_REQ_CCARESET = 0,
    /*
    This primitive will be issued by the MAC sublayer to the PHY entity when the MAC sublayer needs to begin
    the transmission of an PSDU.
    */
    UU_WLAN_PHY_REQ_TXSTART = 1,
    /*
    This primitive will be generated when the MAC sublayer has received the last PHY-DATA.confirm from the
    local PHY entity for the PSDU currently being transferred.
    */
    UU_WLAN_PHY_REQ_TXEND = 2,
    /*
    This primitive is generated by the MAC sublayer to transfer an octet of data to the PHY entity. This
    primitive can only be issued following a transmit initialization response (PHY-TXSTART.confirm) from
    the PHY.
    */
    UU_WLAN_PHY_REQ_DATA = 3,
    /*
    This primitive is generated by the MAC sublayer for the local PHY entity when it desires to change the
    configuration of the PHY.
    */
    UU_WLAN_PHY_REQ_CONFIG = 4

} uu_wlan_phy_req_type_t;


typedef enum uu_wlan_phy_ind_type
{
    /*
    This indication is generated within aCCATime of the occurrence of a change in the status of the channel
    changes from channel idle to channel busy or from channel busy to channel idle.
    */
    UU_WLAN_PHY_IND_CCA = 0,
    /*
    This primitive is issued by the PHY to the MAC entity when the PHY has received a PHY-CCARESET.request.
    */
    UU_WLAN_PHY_IND_CCARESET_CONFIRM = 1,

    /*
    This indication will be issued by the PHY to the MAC entity when the PHY has received a PHYTXSTART.request
    from the MAC entity and is ready to begin accepting outgoing data octets from the MAC.
    */
    UU_WLAN_PHY_IND_TXSTART_CONFIRM = 2,
    /*
    This indication is issued by the PHY to the local MAC entity to confirm the completion of a transmission.
    The PHY issues this indication in response to every PHY-TXEND.request primitive issued by the MAC.
    */
    UU_WLAN_PHY_IND_TXEND_CONFIRM = 3,
    /*
    This indication will be issued by the PHY to the MAC entity when the PLCP has completed the transfer of
    data from the MAC entity to the PHY. The PHY will issue this indication in response to every PHYDATA.
    request primitive issued by the MAC sublayer
    */
    UU_WLAN_PHY_IND_DATA_CONFIRM = 4,

    /*
    This primitive is generated by the local PHY entity to the MAC sublayer when the PHY has successfully
    validated the PLCP header at the start of a new PPDU.
    */
    UU_WLAN_PHY_IND_RXSTART = 5,
    /*
    This indication is generated by the PHY to the local MAC entity that the PSDU currently being received
    is complete.
    */
    UU_WLAN_PHY_IND_RXEND = 6,
    /*
    This indication is generated by a receiving PHY entity to transfer the received octet of data to
    the local MAC entity
    */
    UU_WLAN_PHY_IND_DATA = 7,

    /*
    This indication is issued by the PHY to the MAC entity when the PHY has received and successfully applied
    the parameters in the PHY-CONFIG.request primitive.
    */
    UU_WLAN_PHY_IND_CONFIG_CONFIRM = 8

} uu_wlan_phy_ind_type_t;


typedef enum uu_wlan_ch_state
{
    UU_WLAN_CH_STATE_IDLE = 0,
    UU_WLAN_CH_STATE_BUSY = 1,
} uu_wlan_ch_state_t;


/* TODO: Add proper comments, change name */
typedef enum uu_wlan_busy_channel_list
{
    UU_WLAN_CH_IDLE = 0,
    UU_WLAN_BUSY_CH_PRIMARY = 1,
    UU_WLAN_BUSY_CH_SECONDARY20 = 2,
    UU_WLAN_BUSY_CH_SECONDARY40 = 3,
    UU_WLAN_BUSY_CH_SECONDARY80 = 4
} uu_wlan_busy_channel_list_t;

#if 0
typedef struct uu_wlan_cca_ind
{
    uu_wlan_ch_state_t state : 1; /* 1 bit */
    uu_wlan_busy_channel_list_t  channel_list :3; /* 3 bits */
    uu_uchar   reserved1:4;
} uu_wlan_cca_ind_t;
#endif

/* phy req functions */
extern void uu_wlan_phy_data_req(uu_uchar data_byte, uu_uchar *phy_data_cfm);
extern void uu_wlan_phy_txstart_req(unsigned char *data, int len);
extern void uu_wlan_phy_txend_req(void);
extern void uu_wlan_phy_ccareset_req(void);

#endif /* __UU_WLAN_PHY_IF_H__  */

/* EOF */

