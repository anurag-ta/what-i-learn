/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_cap_txop.h                                      **
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

#ifndef __UU_WLAN_CAP_TXOP_H__
#define __UU_WLAN_CAP_TXOP_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"

UU_BEGIN_DECLARATIONS


/** Return values of supplementary state machine.
 * Some of these will be consumed internally, while others are returned to
 *   'txop_action' of main state machine.
 */
typedef enum uu_wlan_cap_supp_sm_return_values
{
    /** For continue current state in supplementary state machine */
    SUPP_SM_STATE_CONTINUE  = 0xF1,

    /** Returned, if Tx has ended (for reason other than TXOP limit).
     * For Main SM, behaviour is is same as SUPP_SM_CONTEND return value.
     */
    SUPP_SM_TX_END,
    /** Returned, if TXOP limit has reached.
     * For Main SM, behaviour is is same as SUPP_SM_CONTEND return value.
     */
    SUPP_SM_TXOP_LIMIT_COMP,

    /** Returned, on tx-end-confirm from PHY, if another MPDU can be sent.
     * On this return value, TXOP supp. SM enters TXOP_STATE_SENDING_DATA.
     * Not returned to Main SM, as it has to continue in same state.
     */
    SUPP_SM_START_SENDING,

    /** Returned, on tx-end-confirm from PHY, if a response is awaited.
     * On this return value, TXOP supp. SM enters TXOP_STATE_WAIT_FOR_RX.
     * Not returned to Main SM, as it has to continue in same state.
     */
    SUPP_SM_WAIT_FOR_RX,

    /** Returned, on response timer expiry (may / may-not retry).
     * If QoS, waste the remaining TX-NAV in the supp SM.
     * If non-QoS, main SM enters contention again.
     */
    SUPP_SM_TX_FAILURE,

    /** Returned, if contention has to be performed again.
     * This will be returned only if 'tx_nav', 'eifs' and 'nav' are 0.
     */ 
    SUPP_SM_CONTEND,

    /** Returned by TXOP supp. SM, when 'tx_nav' has ended.
     * The main SM has to check whether 'eifs' or 'nav' timers are non-zero.
     */
    SUPP_SM_TX_NAV_ENDED,

    /** Returned, if we receive frame with RA that is not ours.
     * Main SM will enter RECEIVING state, on this return value.
     */
    SUPP_SM_TXOP_NAV_UPDATE,

    /** Returned by TXOP supp. SM, on either PHY-Error / Frame-Error.
     * On this return, if QoS enabled, supp. SM state changes to TXNAV_END.
     * On this return, if non-QoS, main SM state changes to EIFS.
     */
    SUPP_SM_TXOP_ERROR_RECOVERY,
    /** Returned by TXOP supp SM, if received a valid frame but what we are not expecting.
     * Main SM enters RECEIVING state. 
     */
    SUPP_SM_RX_VALID_FRAME,
} uu_wlan_cap_supp_sm_return_values_t;


/** List of states for supplementary-state-machine in TXOP owning state */
typedef enum uu_wlan_cap_txop_sm_states
{
    /** Invalid state */
    UU_WLAN_CAP_TXOP_STATE_INVALID = -1,

    /** TXOP supplementary state machine init state.
     * Decide & Send either RTS / Data / SelfCTS+Data, and then move to RX WAIT.
     * Entry: As soon as TXOP is owned by this station (or an AC in it).
     * Exit: On sending either RTS/Data/SelfCTS+Data, if either CTS/ACK is
     *         expected, go to 'UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX'.
     * Exit: On sending either Data/SelfCTS+Data, if no ACK is expected, go to
     *         TXOP_STATE_SENDING_DATA.
     */
    UU_WLAN_CAP_TXOP_STATE_INIT,

    /** Sending data, after getting atleast one response (CTS/Data) in this TXOP.
     * The data can be having any ACK policy.
     * Entry: Received correct response in 'UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX'.
     * Entry: No response is expected for the frame sent in TXOP_STATE_INIT.
     * Entry: Received correct response in 'TXOP_STATE_WAIT_FOR_RX'
     * Exit: Go to 'TXOP_STATE_WAIT_FOR_RX' after sending Data frame needing ACK.
     * Exit: End of TXOP ownership, due to TXOP limit / Tx end.
     */
    UU_WLAN_CAP_TXOP_STATE_SENDING_DATA,

    /** Waiting for ACK, for non-first (other than first) frame.
     * Error recovery is different from that of 'UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX'.
     * Entry: Sent data needing an ACK in TXOP_STATE_SENDING_DATA.
     * Exit: Received the expected ACK frame.
     * Exit: On Timer for CTS/ACK expiry, follow the error recovery procedure
     *         in the primary state-machine.
     */
    UU_WLAN_CAP_TXOP_STATE_WAIT_FOR_RX,

    /** Wait till end of TXNAV, without any transmission.
     * This is the state, where we WASTE our TXOP for whatever reason.
     * In this state, process whatever frames are received and also use 'nav'
     *     as in RECEIVING state.
     * In this state, use 'eifs' timer on Rx error, as if in EIFS state.
     * Entry: The received frame in 'WAIT_FOR_FIRST_RX' / 'WAIT_FOR_RX' is not
     *          what is expected.
     * Entry: Received Rx-Error / FCS error in 'WAIT_FOR_FIRST_RX'/'WAIT_FOR_RX'
     * Exit: When 'tx_nav' turns 0, and 'eifs' is non-zero, go to 'EIFS' state.
     * Exit: When 'tx_nav' turns 0, 'eifs' is 0, and 'nav' for Rx is non-zero,
     *          go to RECEIVING state.
     * Exit: When 'tx_nav' turns 0, and 'eifs' & 'nav' are also 0,
     *          go to CONTEND state.
     */
    UU_WLAN_CAP_TXOP_STATE_WAIT_TXNAV_END
} uu_wlan_cap_txop_sm_states_t;


extern uu_int32 txop_init_entry(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 txop_init_exit(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 txop_init_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);

extern uu_int32 sending_data_entry(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 sending_data_exit(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 sending_data_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
extern uu_int32 wait_for_rx_entry(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 wait_for_rx_exit(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 wait_for_rx_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
extern uu_int32 wait_for_tx_nav_end_entry(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 wait_for_tx_nav_end_exit(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 wait_for_tx_nav_end_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);


extern uu_void uu_wlan_fill_txvector(uu_wlan_tx_frame_info_t *tx_frame_info, uu_wlan_tx_vector_t *vector);


UU_END_DECLARATIONS

#endif /* __UU_WLAN_CAP_TXOP_H__ */

