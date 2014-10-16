/*************************************************************************
**                                                                      **
** File name :  uu_wlan_umac_dfs.h                                       **
**                                                                      **
** Copyright © 2013, Uurmi Systems                                      **
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

#ifndef __UU_WLAN_UMAC_DFS_H__
#define __UU_WLAN_UMAC_DFS_H__

#ifdef UU_WLAN_DFS
#include "uu_wlan_lmac_if.h"
#include "uu_datatypes.h"
#include "uu_wlan_mgmt_frame.h"

/** TODO: TEMP FIX: for 250ms */
/**
 * Since beacon is 100 ms defined as 800000
 * After association after this much interval
 * we are trigerring the dfs procedures
 */
#define UU_REG_LMAC_MEASREQ_INRVL   2100000

/* Function for doing integer division */
#define INT(x,y)    ((x)/(y))

/**
 * This function is called to start and process the measurement request in LMAC.
 */
uu_void uu_wlan_start_measurement (uu_uint8 meas_type);
uu_void uu_wlan_process_measurement (uu_void);

/**
 * This function is used for preparing Measurement
 * Report as a response to Measurement Request frame.
 * informing that it is refused.
 * @param[in] msr_request: measurement_request frame for which
 * the STA is refusing the request.
 */
uu_void uu_wlan_refuse_measurement_report (uu_mgmt_action_frame_t *msr_request);

/**
 * This function is used for preparing Measurement
 * Report as a response to Measurement Request frame.
 * informing that it received the request late.
 * @param[in] msr_request: measurement_request frame for which
 * the STA is refusing the request because it received the
 * request frame lately.
 */
uu_void uu_wlan_late_measurement_report (uu_mgmt_action_frame_t *msr_request);

/**
 * This function is used for perform autonomous
 * Basic Measurement and if radar is detected at AP
 * then event is sent to upper layer,
 * and if radar is detected at STA
 * then autonomous Basic Report is sent to AP.
 * @param[in] meas_start_time: time when the measurement is to be started.
 * @param[in] meas_duration: duration for which measurement is to be done.
 */
uu_void uu_wlan_process_auto_measurement_report (uu_uint64 meas_start_time, uu_uint16 meas_duration);

/**
 * This function is used for preparing Channel Switch
 * Announcement frame when the channel switching is decided by AP.
 * @param[in] new_channel: the channel to switch.
 */
uu_void uu_wlan_prepare_and_send_ch_switch_ann_frame (uu_uint8 new_channel);

/**
 * This function is used for preparing Measurement
 * Report as a response to Measurement Request frame.
 * Prepares and sends Measurement Report frame.
 * @param[in] msr_request: measurement_request frame for which
 * the response is needed to be sent.
 */
uu_void uu_wlan_process_and_send_measurement_report (uu_mgmt_action_frame_t *msr_request);

/**
 * This function is used for preparing Measurement
 * Request when the timer triggers.
 * Prepares and sends Measurement Request frame.
 * @param[in] meas_req_type: type of the measurement_request frame.
 */
uu_void uu_wlan_prepare_and_send_measurement_request (uu_uint8 meas_req_type);

/**
 * This function is uesd to stop the timer
 * for triggering Measurement Request to send.
 */
uu_int32 uu_wlan_dfs_timer_stop(uu_void);
uu_int32 uu_wlan_dfs_timer_start(uu_void);

#endif /* UU_WLAN_DFS */

#endif /* __UU_WLAN_UMAC_DFS_H__ */

/* EOF */

