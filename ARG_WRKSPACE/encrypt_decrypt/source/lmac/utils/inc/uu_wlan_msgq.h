/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_msgq.h                                         **
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

#ifndef __UU_WLAN_MSGQ_H__
#define __UU_WLAN_MSGQ_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_cap_if.h"


//extern uu_int32 uu_wlan_put_msg_in_AC_Q(uu_uchar ac, uu_int16 desc_id);

/** Gives the reference of the packet of particuler ac
 * Return reference of the frame.
 * @param[in] access category of the packet, 2 bits in length.
 */
//extern uu_wlan_tx_frame_info_t *uu_wlan_get_msg_ref_from_AC_Q(uu_uchar ac);
//extern uu_wlan_tx_frame_info_t *uu_wlan_get_msg_from_AC_Q(uu_uchar ac);
//#define uu_wlan_tx_is_frame_available(x)  (uu_wlan_get_msg_ref_from_AC_Q(x) == UU_NULL)
#define uu_wlan_get_msg_ref_from_AC_Q     uu_wlan_tx_get_frame_info

/** Gives the reference of the packet at given offset, of particuler ac
 * Return reference of the frame.
 * @param[in] access category of the packet, 2 bits in length.
 * @param[in] offset from the read index (0=first message to be read from Q)
 */
extern uu_wlan_tx_frame_info_t* uu_wlan_get_next_msg_ref_from_ac_Q(uu_uint8 ac, uu_int8 offset);

extern uu_int32 uu_wlan_put_msg_in_CAP_Q(uu_wlan_cap_event_type_t ev, uu_uchar *data, uu_int32 len);
extern uu_int32 uu_wlan_get_msg_from_CAP_Q(uu_wlan_cap_event_t *type);

//extern uu_int32 uu_wlan_put_msg_in_TX_Q(uu_int16 desc_id);
//extern uu_int16 uu_wlan_get_msg_from_TX_Q(uu_void);

extern uu_int32 uu_wlan_put_msg_in_RX_Q(uu_int16 desc_id);
extern uu_int16 uu_wlan_get_msg_from_RX_Q(uu_void);

/** Discard all the MPDU fragments of an msdu, based on seq-control-field 
 * @param[in] access category of the packet, 2 bits in length.
 * @param[in] receive address of the frame,  6 bytes in length.
 * @param[in] sequence control field, 2 bytes in length.
 */
extern uu_void uu_wlan_discard_msdu(uu_uchar ac, uu_uchar *ra, uu_uint16 scf);

/** This function frees the descriptor from top of AC Q.
 * @param[in] access category to identify the AC Q, 2 bits in length.
 */
extern uu_void uu_wlan_clear_mpdu(uu_uint8 ac, uu_bool status);

/** This function frees all the descriptors of an AMPDU from top of AC Q.
 * @param[in] access category to identify the AC Q, 2 bits in length.
 */
extern uu_void uu_wlan_clear_ampdu(uu_uint8 ac);


extern uu_void  uu_wlan_tx_buffer_free(uu_uchar ac, uu_uint32 len);
extern uu_wlan_tx_frame_info_t* uu_wlan_tx_get_frame_info(uu_uchar ac);
extern uu_char* uu_wlan_tx_get_frame(uu_uchar ac);
extern uu_uint16 uu_wlan_tx_get_next_frame_len(uu_uchar ac);
extern uu_uchar uu_wlan_tx_get_aggr_count(uu_uchar ac);
extern uu_bool uu_wlan_tx_is_multicast_pkt(uu_uchar ac);
extern uu_bool uu_wlan_tx_is_tods(uu_uchar ac);
extern uu_bool uu_wlan_tx_is_qos_data(uu_uchar ac);
extern uu_bool uu_wlan_tx_is_frame_available(uu_char ac);
extern uu_uchar* uu_wlan_tx_get_ra_address(uu_char ac);
extern uu_uchar uu_wlan_tx_get_frame_fc0(uu_char ac);
extern uu_uchar uu_wlan_tx_get_frame_fc1(uu_char ac);
extern uu_char uu_wlan_tx_get_qos_frame_tid(uu_char ac);
extern uu_uint16 uu_wlan_tx_get_frame_scf(uu_char ac);
extern uu_uchar uu_wlan_tx_get_qos_ackpolicy(uu_char ac);
extern uu_char* uu_wlan_tx_get_next_mpdu_in_ampdu(uu_uchar ac, uu_uchar mpdu_num);


/** Write number of bytes into Rx buffer.
 * @param[in] adr, Reference from where it copies number of bytes to Rx buffer.
 * @param[in] len, Number of bytes to write to Rx buffer.
 * @param[in] rx_end_ind_success , current frame end indication to Rx buffer to avoid the wrap around condition in 'C' model. 
 * LMAC RX calls this function in following cases
 *          Writing rx vector into buffer (address contains the rx vextor address, len contains the size of rx vector)  
 *          Writing frame info into buffer (address contains the frame info adress, len contains the frame info) 
 *          Writing mpdu data tin buffer (address contains the data address, len contains 1)   
 *          Giving end of mpdu indication to buffer for wrap around codition (rx end idication contains the 1)
 */
extern uu_void uu_wlan_rx_write_buffer(uu_uchar *adr, uu_uint16 len, uu_bool rx_end_ind_success);

/** Reset write index to starting position of current frame info write index.
 */
extern uu_void uu_wlan_reset_curr_frame_wrindx(uu_void);

/** Get the current Rx frame info reference */
extern uu_wlan_rx_frame_info_t* uu_wlan_rx_get_frame_info(uu_void);

/** Update Received frame status from Rx module
 * Handovers Received frame starting address location and its total length, to Rx module to update the Rx status to UMAC. 
 */
extern uu_void uu_wlan_update_rx_status(uu_void);

/* TODO: Get better way of calculating the duration for multiple frames, instead of doing traversal of the AC Q */
/** This function gives the frame info of the packet that is nth packet from the first packet in the AC queue.
  * Here n is the offset value which is an integer starting from 1
 **/
extern uu_wlan_tx_frame_info_t* uu_wlan_tx_get_frame_info_at(uu_uchar ac,uu_int32 rd_index_offset);

/** This function returns the number of packets in a ac queue.*/
// TODO: Find out whether this is essential for multiple-protection. Traversal * of Q is not possible in RTL.
extern uu_uint16 uu_wlan_tx_get_no_of_pkts(uu_char ac);

#endif /*__UU_WLAN_MSGQ_H__ */

/* EOF */


