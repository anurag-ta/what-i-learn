/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_lmac_tpc.h                                      **
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

/* Contains declarations for calculation of tpc level for the frames to be transmitted. */

#ifndef __UU_WLAN_LMAC_TPC_H__
#define __UU_WLAN_LMAC_TPC_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"


/**
 * Used for calculation of new transmit power level for the frame to be retransmitted.
 * Returns new transmit power level for the frame to be retransmitted, 8 bits in length.
 * @param[in] txpwr_level for the same frame previously transmitted.
 * @param[in] format of the frame
 * @param[in] modulation of the frame if it is non-HT.
 * @param[in] bandwidth by which the frame is transmitted.
 */
extern uu_uint8 uu_calc_tpl_retry_frame (uu_uint8 txpwr_level, uu_uint8 format, uu_uint8 modulation, uu_uint8 bndwdth);

/**
 * Used for calculation of transmit power level for the control frames.
 * Returns transmit power level for the control frames, 8 bits in length.
 * @param[in] bandwidth by which the frame is transmitted.
 * @param[in] format of the frame
 * @param[in] modulation of the frame if it is non-HT.
 */
extern uu_uint16 uu_calc_tpl_ctrl_frames (uu_uint8 ch_bndwdth, uu_uint8 format, uu_uint8 modulation);

#endif /* __UU_WLAN_LMAC_TPC_H__ */

/* EOF */

