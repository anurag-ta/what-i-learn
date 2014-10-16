/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_tx_if.h                                         **
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

#ifndef __UU_WLAN_TX_IF_H__
#define __UU_WLAN_TX_IF_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"

/* TODO: Retry counters will be different, for each MPDU of an AMPDU */
/* NOTE: Temporarily added here, as CAP is updating these */
extern uu_uint8  uu_wlan_ShortRetryCount[UU_WLAN_MAX_QID];
extern uu_uint8  uu_wlan_LongRetryCount[UU_WLAN_MAX_QID];

/* TODO: The PS-POLL generation & handling shall be in CP, instead of Tx. Reorganize */
/** Indicates Tx handler has to send a PS-Poll frame.
 * Set in Rx handler when TIM is set for the STA in beacon and
 *  when the more data bit is set in received frame.
 * Reset in Tx handler after sending PS-Poll frame.
 */
extern uu_bool uu_wlan_lmac_pspoll_frame_ready_g;
extern uu_bool uu_wlan_lmac_trigger_frame_ready_g;

/** Pointer to buffer that holds ps-poll frame.
 * Used in CAP to retrieve ps-poll frame when required.
 */
/* TODO: These shall be maintained in CP, similar to ACK/RTS/CTS */
extern uu_wlan_tx_frame_info_t  *ps_poll_frame_info_g;
extern uu_wlan_tx_frame_info_t  *ps_trigger_frame_info_g;


#if 0
/** This function called from UMAC-LMAC Interface whenever packet received from UMAC 
 * Maps the descriptor to corresponding AC Q, post the event to CAP, Return UU_FAILURE if fails or UU_SUCCESS in success, 2 bits in length
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 * @param[in] temp data 
 */
extern uu_int32 uu_wlan_tx_handler(uu_uchar *temp_umac_data);
#endif

/** This function used to transmit multiple packets in TXOP, calculates duratin and crc of the packet and transmits to phy
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 * @param[in] access category of teh packet, 2 bits in length.
 * @param[in] tx_nav used in calculate the duration, 16 bits in length.
 * @param[in] bandwidth at which packet has to transmitr, 3bits in length.
 */

extern uu_int32 uu_wlan_tx_send_more_data(uu_uchar ac, uu_int32 tx_nav, uu_wlan_ch_bndwdth_type_t bw);

/** This function initiates the TXOP, calculates duratin and crc of the packet and transmits to phy
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 * @param[in] access category of teh packet, 2 bits in length.
 * @param[in] tx_nav used in calculate the duration, 16 bits in length.
 * @param[in] bandwidth at which packet has to transmitr, 3bits in length.
 */
extern uu_int32 uu_wlan_handle_txop_tx_start_req(uu_uchar ac, uu_int32 tx_nav, uu_wlan_ch_bndwdth_type_t bw);

/** This function used to start the transmit the packet
 * @param[in] access category of teh packet, 2 bits in length.
 */
extern uu_void  uu_wlan_tx_phy_data_req(uu_uint8 ac);

extern uu_void uu_wlan_tx_update_mpdu_status_for_reagg(uu_uint16 seq_num, uu_uchar ac,uu_bool status);

extern uu_int32 uu_wlan_tx_is_ampdu_status_clear(uu_uint8 ac);

#ifdef UU_WLAN_TPC
extern uu_void uu_wlan_tx_update_status_for_umac(uu_uint8 ac, uu_uint16 seqno, uu_uint8 retries, uu_bool status, uu_uint8 bandwidth);
#else
extern uu_void uu_wlan_tx_update_status_for_umac(uu_uint8 ac, uu_uint16 seqno, uu_uint8 retries, uu_bool status);
#endif

/** This function used to set the retry bit in th frame
 * @param[in] access category of teh packet, 2 bits in length.
 */
extern uu_void uu_wlan_tx_set_retry_bit(uu_uint8 ac);
 
/** This function used to update the fall back rate
 * @param[in] access category of teh packet, 2 bits in length.
 */
extern uu_void uu_wlan_tx_update_with_fb_rate(uu_uint8 ac);


/* TODO: The control frame generation & handling shall be in CP, instead of Tx. Reorganize */
/** This function is used to send a qos null data frame as a trigger frame in U-APSD 
  * @params[in] receiver address and duration.
  * called in Rx handler if it receives a frame with both more data bit and EOSP bit are set, to extend the service period(SP).
  * also called by DTIM timer call back function to coming out of PS mode by sending a null data frame with PM bit zero.
  **/
extern uu_uint32 uu_wlan_lmac_qos_null_tx_handling( uu_uchar *ra , uu_uint16 duration );

/** This function is used to send pspoll for retrieving data from ap in legacy PS.
  * @params[in] receiver address and AID of the STA.
  * called in Rx handler after receiving a DTIM beacon and AID found to be set in TIM.
  * also called when more data bit is set in received frame in legacy PS.
  **/
extern uu_uint32 uu_wlan_lmac_pspoll_tx_handling( uu_uchar *ra , uu_uint16 aid );

/** This function is used to get the response frame length of the given frame 
 *  @param[in] frame info of the packet to be transmitted.
 */
extern int uu_wlan_tx_get_expt_response_frame_length (uu_wlan_tx_frame_info_t  *frame_info);


#endif /* __UU_WLAN_TX_IF_H__  */

/* EOF */


