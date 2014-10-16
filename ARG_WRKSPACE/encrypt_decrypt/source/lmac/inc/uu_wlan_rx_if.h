/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_rx_if.h                                         **
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

#ifndef __UU_WLAN_RX_IF_H__
#define __UU_WLAN_RX_IF_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_phy_if.h"
#include "uu_wlan_lmac_if.h"

/** List of rx handler return values
 * These events are used by CAP and CAP takes action corresponding to events
 */
typedef enum uu_wlan_rx_handler_return_value
{
    /* Indicates the received packet crc filed case */
    UU_WLAN_RX_HANDLER_FRAME_ERROR   = 0xff0,
    /* Indicates the station has to send response to sender ex: ack, cts and ba */
    UU_WLAN_RX_HANDLER_SEND_RESP,
    /* Indicates the station has to update the NAV with duration of the packets except for those RA is equal to the MAC address of the station */
    UU_WLAN_RX_HANDLER_NAV_UPDATE,
    /* Indicates the ACK frame received */
    UU_WLAN_RX_HANDLER_ACK_RCVD,
    /* Indicates the CTS frame received */
    UU_WLAN_RX_HANDLER_CTS_RCVD,
    /* Indicates the BA frame received */
    UU_WLAN_RX_HANDLER_BA_RCVD,
    /* Indicates the BA frame received, Retry required for few frames */
    UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS,
    /* Indicates the un expected packet received  */
    UU_WLAN_RX_HANDLER_FRAME_INVALID

} uu_wlan_rx_handler_return_value_t;


/** Receive start interface function, prepares the receive buffers 
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 * @param[in] receive vector of the packet
 */
extern uu_int32 uu_wlan_rx_handle_phy_rxstart(uu_wlan_rx_vector_t  *rx_vec);

/** Receive end interface function, prepares the receive buffers 
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 * @param[in] status of the received packet, 2 bits in length
 */
extern uu_int32 uu_wlan_rx_handle_phy_rxend(uu_uchar  rxend_stat);

/** Receive data interface function, prepares the mpdu packet 
 * Return UU_FAILURE if fails or UU_SUCCESS in success, 32 bits in length
 * @param[in] data, 8 bits in length
 */
extern uu_int32 uu_wlan_rx_handle_phy_dataind(uu_uchar  data);

/** Receive packet status updation to UMAC.
 * After receiving frame, RX module handovers frame with starting address and its length to UMAC.
 * Rx-module updates its status in uu_wlan_rx_status_flags_g & uu_wlan_rx_status_info_g.
 * @param[in] offset - Offset of the rx-frame-info from the start of Rx-buffer.
 * @param[in] len - Received frame length for umac usage.
 */
extern uu_void uu_wlan_rx_update_status_for_umac(uu_uint32 offset, uu_uint16 len);


#endif /* __UU_WLAN_RX_IF_H__  */

/* EOF */


