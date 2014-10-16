/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_cap_cm.h                                       **
 **                                                                           **
 ** Copyright © 2013, Uurmi Systems                                           **
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

#ifndef __UU_WLAN_CAP_CM_H__
#define __UU_WLAN_CAP_CM_H__

#include "uu_datatypes.h"
#include "uu_wlan_cap_sm.h"
#include "uu_wlan_cap_cm.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_cap_context.h"


UU_BEGIN_DECLARATIONS

/*
 ** Funtions Declarations
 */

/** idle_entry funtion enters idle state whenever state machine starts.
 * this function eneters in idle state and waits in idle_action funtion.
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 */
extern uu_int32 idle_entry(uu_uchar *sm_cntx, uu_int32 event);
/** idle_exit:with this funtion it exit from the idle state.
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 */
extern uu_int32 idle_exit(uu_uchar* sm_cntx, uu_int32 event);
/** idle_action funtion events are handled in this function and changes states if required(depends on event).
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 * @param[in] ev_data carries the event related information.
 */
extern uu_int32 idle_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
extern uu_int32 receiving_entry(uu_uchar *sm_cntx, uu_int32 event);
/** rx_receiving_exit:with this funtion it exit from the rx_receiving state.
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 */
extern uu_int32 receiving_exit(uu_uchar *sm_cntx, uu_int32 event);
/** rx_receiving_action funtion events are handled in this function and changes states if required(depends on event).
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 * @param[in] ev_data carries the event related information.
 */
extern uu_int32 receiving_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
extern uu_int32 contend_entry(uu_uchar *sm_cntx, uu_int32 event);
/** contend_exit:with this funtion it exit from the contend state.
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 */
extern uu_int32 contend_exit(uu_uchar *sm_cntx, uu_int32 event);
/** contend_action funtion events are handled in this function and changes states if required(depends on event).
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 * @param[in] ev_data carries the event related information.
 */
extern uu_int32 contend_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
extern uu_int32 eifs_entry(uu_uchar *sm_cntx, uu_int32 event);
/** _exifst:with this funtion it exit from the eifs state.
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 */
extern uu_int32 eifs_exit(uu_uchar *sm_cntx, uu_int32 event);
/** eifs_action funtion events are handled in this function and changes states if required(depends on event).
 * @param[in] sm_cntx state_info_cntx.
 * @param[in] event event received by statemachine.
 * @param[in] ev_data carries the event related information.
 */
extern uu_int32 eifs_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);
extern uu_int32 txop_entry(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 txop_exit(uu_uchar *sm_cntx, uu_int32 event);
extern uu_int32 txop_action(uu_uchar *sm_cntx, uu_int32 event, uu_uchar *ev_data);

#ifdef UU_WLAN_DFS
extern uu_uint32 chan_busy_dur_g;
#endif

UU_END_DECLARATIONS

#endif /* __UU_WLAN_CAP_CM_H__ */

