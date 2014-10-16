/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_limits.h                                       **
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
#ifndef __UU_WLAN_LIMITS_H__
#define __UU_WLAN_LIMITS_H__

/*
** Higher Limits, based on Specification
*/
#define UU_WLAN_MAX_AMPDU_SZ         3074  /* TODO: Review this value */
#define UU_WLAN_AC_MAX_AC               4

#ifdef UU_WLAN_BQID /* This is what is implemented in RTL */
/* 4 ACs and 1 Beacon queue.
 * This is different from the specs,
 * as we have decided to put the Beacon in separate queue.
 * CAP will also contend for this Beacon queue as per the other AC queues.
 * The difference is that in IBSS mode, while contending for Beacon queue,
 * the backoff counter for other AC's is not decremented, i.e., won't go
 * for contending when beacon is present in the Beacon queue.
 * As refer to the section 10.1.3.3 in P802.11REV-mb D12. */
#define UU_WLAN_MAX_QID                 5
#else /* Enhancements, yet to be carried to RTL */
#define UU_WLAN_MAX_QID                 4
#endif

/*
** Higher Limits, based on Implementation
*/
/** Maximum number of associations supported in the current implementation */
#define UU_WLAN_MAX_ASSOCIATIONS     20
/** Maximum number of BA Sessions supported in the current implementation */
#define UU_WLAN_MAX_BA_SESSIONS    (2*UU_WLAN_MAX_ASSOCIATIONS)

#define UU_WLAN_INVALID_BA_SESSION   0xFFFF
/** Maximum number of AC Q messages supported in current implementation */
#define MAX_AC_Q_MSGS                32



/*
** Lower Limits, based on Specification
*/



/*
** Lower Limits, based on Implementation
*/



/*
** Miscellaneous
*/

/* Number of spatial streams currently supported in hardware */
#define UU_PHY_MAX_STREAM_VHT         2
#define UU_PHY_MAX_STREAM_HT          2
/* Presently we are supporting only 2 spatial streams */
#define UU_MAX_MCS_N_SUP     16 
#define UU_VHT_PHY_USR      0 /* For VHT, only 1 user is supported now. Range : 0 to 3 */


#endif /* __UU_WLAN_LIMITS_H__ */

/* EOF */



