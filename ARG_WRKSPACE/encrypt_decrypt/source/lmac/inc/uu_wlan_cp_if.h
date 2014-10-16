/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_cp_if.h                                         **
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

#ifndef __UU_WLAN_CP_IF_H__
#define __UU_WLAN_CP_IF_H__

#include "uu_datatypes.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_fwk_lock.h"
#include "uu_wlan_cp_ba_sb.h"

/** CP Interface */

/** Synchronization lock fo block ack context */
extern uu_wlan_spin_lock_type_t uu_wlan_ba_ses_context_lock_g;


/** global defination for cntl frame buffer */
#define tx_ctl_frame_info_g (*tx_ctl_frame_info_gp)
extern uu_wlan_tx_frame_info_t *tx_ctl_frame_info_gp;
//extern uu_wlan_tx_frame_info_t tx_ctl_frame_info_g;

/** This function generates RTS or self-CTS 
 * @param[in] format contains the frame-format (HT/VHT/Legacy etc.) of the transmitting packet (2 bits in length)
 * @param[in] rate contains rate at which packet needs to be transmitted. (4 bits in length)
 * @param[in] ra contains receipent address of RTS or self-CTS to be generated. (6 bytes in length)
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 */
extern uu_int32 uu_wlan_cp_generate_rtscts(uu_uint8 format, uu_uint8 rtscts_rate, uu_uint8 ch_bndwdth, uu_uint8 modulation, uu_uchar *ra);

/** This function generate the BAR frame
 * @param[in] bar ack policy
 * @param[in] traffic Identifier
 * @param[in] starting sequence number
 * @param[in] duration value
*/
extern uu_uint32 uu_wlan_cp_generate_bar(uu_uint8 bartype, uu_uint8 tid, uu_uint16 ssn, uu_uint16 duration, uu_uchar *ra);

/** This function process the received frame 
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 8 bits in length
 * @param[in] frame reference 
 * @param[in] 1:end of the rx frame, 0:part of MPDU in A-MPDU 
 */
extern uu_int32 uu_wlan_cp_process_rx_frame(uu_wlan_rx_frame_info_t *frame_info, uu_bool is_rxend);

/** This function adds the BA information into BA Table
 * @param[in] traffic Identifier
 * @param[in] address of the station
 * @param[in] buffer size of remote station
 * @param[in] starting sequence number
 * @param[in] block ack type (delayed or immediate)
 * @param[in] direction of BA (Transmission or reciving)
 */
extern uu_int32 uu_wlan_lmac_add_ba(uu_uint8 tid, uu_uchar* addr, uu_uint16 bufsz, uu_uint16 ssn, uu_uint8 type, uu_bool dir);

/** This function deletes the BA information from BA Table
 * @param[in] traffic Identifier
 * @param[in] address of the station
 * @param[in] direction of BA (Transmission or reciving)
 */
extern uu_void uu_wlan_lmac_del_ba(uu_uint8 tid, uu_uchar* addr, uu_bool dir);

/** This function is to update the tx score board
 * While transmitting packet under block ack, TX updates the score board. 
 * @param[in] ra contains receive address
 * @param[in] tid contains the traffic identifier
 * @param[in] scf contains sequence control field
 */
extern uu_void uu_wlan_cp_update_the_tx_sb(uu_uchar *ra, uu_uint8 tid, uu_uint16 scf);


/** This function sets the 'waiting for ack' flag */
extern uu_void uu_wlan_cp_set_waiting_for_ack(uu_void);
/** This function resets the 'waiting for ack' flag */
extern uu_void uu_wlan_cp_reset_waiting_for_ack(uu_void);

/** This function sets the 'waiting for cts' flag */
extern uu_void uu_wlan_cp_set_waiting_for_cts(uu_void);
/** This function sets the 'waiting for cts' flag */
extern uu_void uu_wlan_cp_reset_waiting_for_cts(uu_void);

#if 0 /* Now doing it as part of station management */
typedef enum uu_wlan_cp_rq_type
{
    UU_WLAN_CP_RQ_INVALID = 0,
    /* This request given by config???, when BA session established */
    UU_WLAN_CP_RQ_ADD_BA_SESSION,
    /* This request given by config???, when BA session teardown */
    UU_WLAN_CP_RQ_DEL_BA_SESSION
} uu_wlan_cp_rq_type_t;

typedef struct uu_wlan_cp_rq_ba_est
{
    uu_uchar       sta_addr[IEEE80211_MAC_ADDR_LEN];
    uu_int32       buff_size;
    uu_uchar       tid;
    uu_uint16      start_seq;
    uu_wlan_ba_ses_type_t    ba_ses_type;
} uu_wlan_cp_rq_ba_est_t;

typedef struct uu_wlan_cp_rq
{
    uu_wlan_cp_rq_type_t rq_type;
    union {
        uu_wlan_cp_rq_ba_est_t             ba;
    } rq_info;
} uu_wlan_cp_rq_t;

extern uu_int32 uu_wlan_cp_handler(uu_wlan_cp_rq_t *rq);
#endif /* #if 0 */

#endif /* __UU_WLAN_CP_IF_H__ */

