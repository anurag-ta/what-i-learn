/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_duration.h                                      **
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

/* Contains declarations for duration calculations. */

#ifndef __UU_WLAN_DURATION_H__
#define __UU_WLAN_DURATION_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"


/** This function is used for duration calculation of broadcast frames.
 * Returns duration for the broadcast frame, 16 bits in length.
 * @param[in] txvec of the packet to be transmitted
 * @param[in] next_frag_len Length of next fragment of this frame.
 */
extern uu_uint16 uu_calc_bcast_duration(uu_wlan_tx_vector_t *txvec, uu_uint16 next_frag_len);

/** Used for duration calculation of implicit/explict BA-request.
 * Returns duration for the block ack request frame, 16 bits length.
 * @param[in] txvec of the packet to be transmitted.
 * @param[in] length of the block ack response expected /basic/compressed/multi-tid, length 12 bits.
 */
extern uu_uint16 uu_calc_impl_BA_duration(uu_wlan_tx_vector_t *txvec, int length);

/** This function is used for duration calculation of data frame in single protection.
 * Returns duration for the frames other than RTS-self-CTS protected and BAR, 16 bits in length.
 * @param[in] txvec of the packet to be transmitted.
 * @param[in] response frame length, 8 bits in length.
 * @param[in] next_frame_length of the same AC, 20 bits in length.
 */
extern uu_uint16 uu_calc_singlep_frame_duration(uu_wlan_tx_vector_t *txvec, int resp_frame_length, int next_frame_length);


/* This function is used for duration calculation for rts/self-cts frame in single protection
 * Returns duration value for rts/self-cts frame, 16 bits in length.
 * @param[in] txvec of the packet to be transmitted.
 * @param[in] rts_cts_rate (legacy rate/mcs) to be used by RTS/self-CTS frame, 7 bits in length.
 * @param[in] cts_self is to check if it is RTS or self-CTS, 1 bit in length.
 * @param[in] length of the expected response frame for data i.e., ack, block ack, block ack compressed, 
 * multi-tid block ack, no-ack (as 0), max 202 bytes (multi-tid ba), 12 bits in length.
 */
extern uu_uint16 uu_calc_rtscts_duration(uu_wlan_tx_vector_t *txvec, uu_uint8 rts_cts_rate, bool cts_self, int length);


#if 0 /* Alternative implementation is used for multiple frame protection */
/** This function is used for duration calculation of frames in multiple txop protection
 * Returns duration value for any frame that is send in multiple txop protection, 16 bits in length.
 * @param[in] txvec of the packet to be transmitted.
 * @param[in] T_txop, 16 bits in length
 * @param[in] T_endnav, 16 bits in length
 * @param[in] T_txopremaining, 16 bits in length
 * @param[in] rts_cts_rate (legacy rate/mcs) to be used by RTS/self-CTS frame, 7 bits in length.
 * @param[in] ctrl_req_frame is the length of RTS/CTS that initiates the TXOP, max 20 bits in length (RTS).
 * @param[in] resp_len is length of the expected response frame for data i.e., ack, block ack, block ack compressed
 * multi-tid block ack, no-ack (as 0), max 202 bytes (multi-tid ba), 12 bits in length
 */
extern uu_uint16 uu_calc_multip_duration(uu_wlan_tx_vector_t *txvec, int T_txop, int T_endnav, int T_txopremaining, uu_uint8 rts_cts_rate, int ctrl_req_frame, int resp_len);
#else

/** Calculates duration for the 1st frame in multiple frame transmission.
 * Duration is estimated for the frames currently available in LMAC Tx buffer of particular AC.
 * If the duration is less than the remaining TXOP, returns the calculated duration.
 *   Otherwise returrs the remaining TXOP-TPPDU
 * TPPDU is the time required to send the current frame transmitting.
 */
/** Calculates duration for the 1st frame in multiple frame transmission.
 * Duration is estimated for the frames currently available in LMAC Tx buffer of particular AC.
 * If the duration is less than the remaining TXOP, returns the calculated duration.
 *   Otherwise returns the remaining TXOP-TPPDU
 * TPPDU is the time required to send the current frame transmitting.
 * @param[in] ac - Access category of the TXOP owner.
 * @param[in] txop_limit - TXOP limit of the AC.
 */
extern uu_uint16 uu_calc_multip_first_frame_duration( uu_uint8 ac, uu_uint32 tx_nav);


/* TODO: Verify the spec. The duration can not be increased, in the middle of the TXOP */
/** Calculates duration for the later frames in multiple frame transmission.
 * Duration is estimated for the frames currently available in LMAC Tx buffer of particular AC.
 * If the duration is less than the remaining TXOP, returns the calculated duration.
 *   Otherwise returrs the remaining TXOP-TPPDU
 * TPPDU is the time required to send the current frame transmitting.
 * @param[in] ac - Access category of the TXOP owner.
 * @param[in] tx_nav - Remaining Tx NAV value
 */
extern uu_uint16 uu_calc_multip_frame_duration( uu_uint8 ac, uu_uint32 tx_nav);
#endif /* Multiple frame protection */


#endif /* __UU_WLAN_DURATION_H__ */

/* EOF */


