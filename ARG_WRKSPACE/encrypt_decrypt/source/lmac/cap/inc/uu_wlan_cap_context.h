/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_context.h                                  **
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

#ifndef __UU_WLAN_CAP_CONTEXT_H__
#define __UU_WLAN_CAP_CONTEXT_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_cap_if.h"
#include "uu_wlan_cap_sm.h"


UU_BEGIN_DECLARATIONS

/** List of STA state machine states */
typedef enum uu_wlan_cap_sm_states
{
    /** Invalid state */
    UU_WLAN_CAP_STATE_INVALID = -1,

    /** Starting / Idle state.
     * Entry: State machine starts with this state.
     * Entry: Channel is Idle and we are not doing contention for any of the ACs.
     * Exit: On CCA Busy event from PHY, go to RECEIVING state.
     * Exit: On getting data into Tx Q, go to CONTEND state.
     */
    UU_WLAN_CAP_STATE_IDLE = 0,

    /** Error recovery state, when medium is idle.
     * Entry: Rx error from PHY, and then PHY indicates medium is IDLE
     * Entry: MAC FCS validation error, and then PHY indicates medium is IDLE.
     * Continuous to decrement any NAV timer, till it becomes 0. On becoming 0,
     *  send PHYCCA-reset to PHY.
     * Starts EIFS (or EIFS-DIFS for QoS) timer, when entering. Resets, in exit.
     * Exit: When either PHYCCA(Busy) is received, go to RECEIVING state.
     * Exit: If EIFS becomes 0 and NAV is non-zero, go to RECEIVING state.
     * Exit: If EIFS becomes 0, and NAV is also 0, go to CONTEND / IDLE based on
     *       status of Tx-Ready flags for all the ACs.
     */
    UU_WLAN_CAP_STATE_EIFS,

    /** The station (at least 1 AC, if QoS STA) is contending for channel.
     * Every AC with Tx-Ready flag set, will be contentding in parallel.
     * While entering this state, each AC will have AIFS[AC] initialized.
     * If non-QoS, DIFS is used instead of AIFS.
     * In this state, if another AC gets Tx-Ready set, AIFS[AC] initialized for
     *    that AC.
     * When AIFS[AC} / DIFS becomes 0, start backoff timer for that AC.
     * If an AC gets backoff timer to 0, check whether there is still data in Q.
     *    If no data, reset Tx-Ready flag. Otherwise, AC gets the TXOP ownership.
     * If more than one AC gets backoff timer to 0 simultaneously (internal
     *    collition) and have data, higher-priority AC gets ownership. Other ACs
     *    follow the internal-collision recovery procedure.
     * Entry: Tx-Ready for an AC in IDLE state.
     * Entry: In TXOP state, if TXOP ends / Tx ends, corresponding AC enters
     *         into contention and changes station state to CONTEND.
     * Entry: NAV becomes 0 in RECEIVING state with PHYCCA state is Idle and atleast
     *         one AC has Tx-Ready set.
     * Entry: NAV & EIFS are 0 in EIFS state.
     * Exit: Whenever an AC gets TXOP ownership, go to TXOP state.
     * Exit: Check Tx-Ready for all ACs, whenever Tx-Ready is reset for an AC. If
     *        all Tx-Ready flags are 0, go to IDLE state.
     */
    UU_WLAN_CAP_STATE_CONTEND,

    /** The station (or an AC in it, for QoS STA) owns the TXOP on channel.
     * In this state, our station sends data and gets acknowledgements.
     * Separate supplementary state-machine is invoked in this state.
     * Entry: In CONTEND state, an AC had backoff expired.
     * On (re-)transmission failure, follow the retransmission/discard procedure.
     * Exit: If TXOP limit has reached / Tx ended, go to CONTEND state.
     * Exit: If ACK / CTS not received (retransmission case), go to CONTEND.
     * TBD: The action on Rx failure is different for the first & subsequent frames.
     * Exit: On Rx error (from PHY, or MAC FCS) 
     */
    UU_WLAN_CAP_STATE_TXOP,

    /** Station is receiving data, with TXOP owned by others.
     * In this state, NAV will be set from received valid frames.
     * Whenever NAV becomes 0, PHYCCA-Reset will be given to PHY.
     * Entry: PHYCCA (Busy) in IDLE or CONTEND or EIFS states.
     * Exit: To CONTEND state, if PHYCCA turns to Idle, NAV already 0 and atleast
     *         one AC has Tx-Ready set.
     * Exit: To CONTEND state, if NAV turns 0 with PHYCCA(Idle) and atleast one AC
     *         has Tx-Ready set.
     * Exit: To IDLE state, if NAV turns 0 with PHYCCA(Idle) and none of the ACs
     *         have Tx-Ready set.
     * Exit: To IDLE state, if PHYCCA turns to Idle, NAV already 0 and none of
     *         the ACs have Tx-Ready set.
     * Exit: To EIFS state, on PHY / MAC-FCS error with PHYCCA(Idle).
     */
    UU_WLAN_CAP_STATE_RECEIVING,

    /** Max state, used only for comparison */
    UU_WLAN_CAP_MAX_STATES /* keep it last */
} uu_wlan_cap_sm_states_t;


/** cap context tx frame information structure.
 * Contains tx_vec of Tx frame and mpdu header information.
 * TODO: Identify when it will be set & used.
 */
typedef struct uu_wlan_cap_tx_frame_info
{
    /* Txvector related parameters */ 
    /** Filled, before sending a tx-start-req to phy with tx-vetor of tx frame.
     * Indicates whether the frame being transmitted is aggregated. 
     * Used by CP for validating the received response frame.
     */
    uu_uchar        txvec_is_aggregated;  /* 1bit */

    /* Maintains first few bytes of header (upto seqno) from last Tx frame.
     * To process tx_start_confirm, frame's FC0, FC1(TODS, FROMDS), RA, and QOS are used.
     * Used for - setting the response timer value on tx_end confirm event from phy
     *   and for Discarding the frame after all the Retransmissions.
     */
    uu_uchar        mpdu[UU_MIN_DATA_PKT_LEN];
} uu_wlan_cap_tx_frame_info_t;


/** CAP context rx frame information structure.
 * Contains rx_vec parameters of rx frame and mpdu header information.
 * TODO: Identify when it will be set & used.
 */
typedef struct uu_wlan_cap_rx_frame_info
{
    /** For validationing the legacy length of received frame.
     * And it is filled with rxvector of received frame on rx-start indication.
     */
    uu_uint16       rxvec_L_length;  /* 12 bits */

    /** For validationing the VHT length of received frame.
     * And it is filled with rxvector of received frame on rx-start indication.
     */
    uu_uint32       rxvec_psdu_length; /* 20 bits */

    /** For validationing the HT length of received frame.
     * And it is filled with rxvector of received frame on rx-start indication.
     */
    /* uu_uint32       rxvec_ht_length; */ /* rxvec_psdu_length is also used for HT */

    /** For calculating PSPOLL frame duration, and it is filled with rxvector of received frame on rx-start indication.
     */
    uu_uint16       rxvec_L_datarate;  /* 4 bits */

    /** It is filled with rxvector of received frame on rx-start indication.
     * To decide the length of received frame.
     */
    uu_uchar        rxvec_frame_format; /* 2 bits */

    /* Maintains first few bytes of header (upto seqno) from last Rx frame.
     * Needed for for nav update, nav reset timer, PSPOLL.
     */
    uu_uchar mpdu[UU_MIN_DATA_PKT_LEN];
} uu_wlan_cap_rx_frame_info_t;


/** AC context structure.
 * Contains per AC related information, as maintained in the CAP state machine.
 * Four such contexts are maintained in the CAP.
 * For non-QoS stations, only the AC 0 (Best-Effort) is used.
 */
typedef struct uu_wlan_cap_ac_context
{
    /** Tx ready flag for this AC.
     * Set when a frame is added to the AC Q that is currently empty.
     * Reset, when Q is found to be empty when the TXOP is won for this AC.
     */
    uu_uint16   tx_ready;  /* 1 bit */

    /** AIFS[AC] counter for QoS, and DIFS for non-QoS.
     * This is the continuous channel idle time, before starting backoff timer.
     * Initialized, whenever station state changes to CONTEND.
     * Decremented on slot timer (based on count-down timer), in CONTEND state.
     * When turns 0, backoff timer is checked - Backoff timer is re-initialized,
     *   if it is already 0.
     */
    uu_uint16   aifs_val; /* 4 bits */

    /** Random backoff number for channel access.
     * Initialized, if it is 0 when 'aifs_val' turns to 0.
     * Decremented on timer tick in CONTEND state, when 'aifs_val' is already 0.
     * Last value retained, even if it switches to RECEIVING / EIFS states.
     * When it turns to 0, station state changes to TXOP.
     * If it turns to 0 simultaneously for more than one AC, internal-collision
     *   handling mechanism kicks in.
     */
    uu_uint16   backoff_val; /* 16 bits */

#if 0
    /** TXOP limit announced by AP for given AC.
     * There is no such limit for non-QoS stations.
     * Limit 0 has special meaning - TXOP is limited to 1 MSDU.
     * TBD: Why not use config registers, like other params broadcast by AP?
     */
    uu_int32    txop_limit; /* 8 bits */
#endif

    /** Flag indicates whether Tx operation at PHY is in progress.
     * Set on getting Tx-Start Confirm indication from PHY.
     * Reset on getting Tx-End indication from PHY.
     */
    uu_bool     tx_prog; /* 1bit */

    /** TXNAV for the current TXOP owned by this station (or an AC in it).
     * Applicable only for QoS mode.
     * Initialized when the AC wins the TXOP.
     * Updated with Duration set in the most recent successfully transmitted
     *   frame, on getting Tx-End from PHY. (Required???)
     * Decremented on every timer tick, if 'tx_prog' flag is not true. (Related to previous statement. Required???)
     * While filling the QoS MPDU, its Duration is calculated as
     *   ('current tx_nav' - 'Time for Tx of MPDU' - 'Any processing delays').
     * When this value is not sufficient for the next frame (Tx+Rx+IFS), the
     *   TXOP is ended. There are some exceptions: TXOP limit may be exceeded,
     *   when using a lower PHY rate than selected for the initial Tx attempt of
     *   the first MPDU, for a retransmission of an MPDU, for the initial
     *   transmission of an MPDU if any previous MPDU in the current MSDU has
     *   been retransmitted, or for broadcast/multicast MSDUs
     */
    uu_int32    tx_nav; /* 16 bits */ 

    /* TO BE HANDLED: TXNAV. If Tx failed (no response received), till the end of TXNAV, none of the ACs can contend. They have to behave as if NAV is received from third party */

    /** AC's Short-Retry-Count (SRC) / Station LRC for non-QoS.
     * This is different from the MSDU/MPDU's SRC.
     * Initialized to 0, when the Tx-Ready flag is set.
     * Incremented whenever SRC of a Data MPDU is incremented.
     * Reset on CTS for sent RTS, or ACK for sent MPDU / MMPDU, or frame with
     *  group address in Address1 is transmitted.
     * The limit is given by UMAC in config register, and is same for all ACs.
     */
    //uu_uint16   SSRC;  /* 8 bits */

    /** AC's Long-Retry-Count (LRC) / Station LRC for non-QoS.
     * This is different from the MSDU/MPDU's LRC.
     * Initialized to 0, when the Tx-Ready flag is set.
     * Incremented whenever LRC of a Data MPDU is incremented.
     * Reset on ACK for sent MPDU / MMPDU of length > dot11RTSThreshold, or
     *   frame with group address in Address1 is transmitted.
     * The limit is given by UMAC in config register, and is same for all ACs.
     */
    //uu_uint16   SLRC; /* 8 bits */

    /** Current CW value for the AC.
     * If 0, initialized to CWmin for that AC, from config register.
     * Takes next values in the series (2*cw_val+1), whenever either SSRC / SLRC
     *  is incremented, till it reach CWmax. Stays at CWmax, till it is reset.
     * Reset to CWmin after successful Tx of MPDU / MMPDU, or SLRC reaches its
     *  limit, or SSRC reaches its limit.
     * CWmin & CWmax are set by UMAC, on per-AC basis, with SME / AP settings.
     */
    uu_uint16   cw_val; /* 16 bits */
} uu_wlan_cap_ac_context_t;


/** Channel access plane context structure */
typedef struct uu_wlan_cap_context
{
    /** Contains state m/c context */
    uu_wlan_sm_context_t     sm_cntxt;

    /** Contains TXOP's supplementary state m/c context */
    uu_wlan_sm_context_t     txop_sm_cntxt;

    /** Access catageory context of all ac's.
     * In Non-QoS mode, only AC 0 will be used.
     */
    uu_wlan_cap_ac_context_t  ac_cntxt[UU_WLAN_MAX_QID];

    /** Timeout value to wait for ACK frame. 
     * When TXOP sub-state is WAIT_FOR_ACK, this will be used.
     * Initialized to config register value, while entering WAIT_FOR_ACK.
     * Reset, on getting Rx-Start indication from PHY.
     * Decremented on each timer tick.
     * On expiry, transmission is treated as failure. The failure actions may
     *   include - incrementing retry counters, using PIFS, going to CONTEND etc.
     */ 
    uu_uint16                 ack_timer_val; /* 5 bits */

    /** Timeout value to wait for CTS frame. 
     * When TXOP sub-state is WAIT_FOR_CTS, this will be used.
     * Initialized to config register value, while entering WAIT_FOR_CTS.
     * Reset, on getting Rx-Start indication from PHY.
     * Decremented on each timer tick.
     * On expiry, transmission is treated as failure. The failure actions may
     *   include - incrementing retry counters, using PIFS, going to CONTEND etc.
     */ 
    uu_uint16                 cts_timer_val; /* 5 bits */

    /** Time for error recovery, if we receive Rx error from PHY / MAC-FCS.
     * Initialized to config register value, while entering EIFS state.
     * Reset, while exiting from EIFS.
     * Decremented on each timer tick.
     * On expriry, exits EIFS state
     */
    uu_uint16                 eifs_timer_val; /* 6 bits */

    /** Nav duration timeout. 
     * Initialized, as soon as a valid frame is received in RECEIVING state.
     * Updated, if a new valid frame is received with NAV > current NAV.
     * Decremented on every timer tick, in RECEIVING / EIFS.
     * On expiry, do PHYCCA-Reset.
     */
    uu_uint16                 nav_val; /* 16 bits */

    /** Timer to reset the initial NAV from receiving 
     * This is a short timer, to confirm the validity of initial NAV.
     * Initialized in RECEIVING state, on receiving RTS not destined for us.
     * Cleared on getting Rx-Start indication from PHY.
     * Decremented on each timer tick.
     * On expiry, 'nav_val' is reset, and state is changed to IDLE/CONTEND.
     */
    uu_int32                  nav_reset_timer;  /* 16 bits */

    /** PHY CCA status - Busy(1) / Idle(0).
     * Updated on receiving PHYCCA indication from PHY - to either Busy / Idle.
     * Reset to Idle(0), whenever PHY-CCAReset.confirm is received from PHY.
     */
    uu_uint8                  cca_status; /* 1 bit */

    /** PHY CCA channel list - 0(all are idle), 1(all are busy), 2(only primary idle), 3(primary and secondary 20 are idle),
     * 4(primary,secondary 20, secondary 40 are idle). 
     * Updated on receiving PHYCCA indication from PHY - to 0, 1, 2, 3, or 4.
     * Used in TXOP state, along with prev_channel_list, to set txvector BW.
     */
    uu_wlan_busy_channel_list_t  updated_channel_list;  /* 3 bits */

    /** PHY CCA channel list, similar to 'updated_channel_list'.
     * Set with 'updated_channel_list', when 'aifs' turns to 0 or backoff turns to 1.
     * Used in TXOP state, along with updated_channel_list, to set txvector BW.
     */
    uu_wlan_busy_channel_list_t  prev_channel_list;  /* 3 bits */

    /** AC owning the TXOP, in the TXOP state. Always 0, in non-QoS mode.
     * Set whenever entering the TXOP state.
     * Used as long as the TXOP is owned, to operated on AC specific context.
     */
    uu_uint8                  txop_owner_ac;  /* 2 bits */

    /** Slot timer generation counter (coverts micro-sec timer into slot timer).
     * Initialized to slottimer value while entering into the contend state.
     * Reset to slottimer value whenever it reaches to '0'.
     * Continuously decremented on timertick event in contend state.
     * When it turns to '0', either aifs(difs) or backoff value will be decremented.
     */
    uu_uint8                  slot_timer_countdown;  /* 4 bits */

    /** Information about the frame being transmitted (TXOP owner).
     * Contains MPDU, and the parts of 'txvec' of frame.
     * It is filled, while sending the 'Tx-Start' to PHY.
     * 
     */
    uu_wlan_cap_tx_frame_info_t     tx_frame_info;

    /** Information about the frame received (Rx).
     * The 'rxvec' part is filled based on PHY Rx-Start indciation.
     * The Rx MPDU header part will be filled, on PHY Rx-Data indication.
     */
    uu_wlan_cap_rx_frame_info_t     rx_frame_info;
} uu_wlan_cap_context_t;


/** Global cap context for contention module */
extern uu_wlan_cap_context_t uu_wlan_cap_ctxt_g;

uu_bool uu_wlan_is_tx_ready_for_any_AC_Q(uu_void);

uu_void uu_wlan_cap_retain_tx_frame_info(uu_wlan_tx_frame_info_t *frame_info);
uu_void uu_wlan_cap_retain_rx_vec_info(uu_wlan_rx_vector_t *rxvec);
uu_uint32 uu_wlan_cap_retain_rx_frame_hdr(uu_uchar *frame_p);

UU_END_DECLARATIONS

#endif /* __UU_WLAN_CAP_CONTEXT_H__ */

