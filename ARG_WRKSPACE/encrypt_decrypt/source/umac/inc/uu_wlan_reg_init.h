/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_reg_init.h                                      **
**                                                                            **
** Copyright © 2013, Uurmi Systems                                            **
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

#ifndef __UU_WLAN_REG_INIT_H__
#define __UU_WLAN_REG_INIT_H__

#include "uu_datatypes.h"


/** In current implementation we are supporting only OFDM. 
 * The following rates are as for OFDM PHY - 20MHz channel spacing as per specification.
 * Refer Table 18-17-OFDM PHY Characteristics . */

/** Slot timer default value is : 9 micro sec */
#define UU_WLAN_SLOT_TIMER_DEFAULT_VALUE 9

/** SIFS timer default value is : 16 micro sec */
#define UU_WLAN_SIFS_TIMER_DEFAULT_VALUE 16

/** For OFDM PHY aPHY-RX-START-Delay is 25 micro sec for 20MHz channel spacing */
#define UU_WLAN_PHY_RXSTART_DELAY_DEFAULT_VALUE 25

/** Short retry and long retry counts */
/** Tx retry limit defaults from standard Refer IEEE802.11-2007 - Page 874 - 
 * dot11ShortRetryLimit Integer:= 7
 * dot11LongRetryLimit Integer:= 4,
 */
#define UU_WLAN_SHORT_RETRY_DEFAULT_VALUE 7
#define UU_WLAN_LONG_RETRY_DEFAULT_VALUE 4

/** CWmin value is : 15 
 * CWmax value is : 1023 
 * For OFDM PHY :
 * Following table indicates the Default EDCA parameter values in case of dot11OCBActivated is false 
 * -------------------------------------------------------------------------------------
 * -    AC   -    CWMin            -        CWMax       -     AIFSN    - TXOP Limit    -
 * -------------------------------------------------------------------------------------
 * - AC_BK   -    aCWMin           -      aCWMax        -       7      -     0         -
 * - AC_BE   -    aCWMin           -      aCWMax        -       3      -     0         -
 * - AC_VI   -   (aCWMin+1)/2-1    -      aCWMin        -       2      -  3.008 ms     -
 * - AC_VO   -   (aCWMin+1)/4-1    -    (aCWMin+1)/2-1  -       2      -  1.504 ms     -
 * -------------------------------------------------------------------------------------
 * 
 */

#define UU_WLAN_CWMIN_DEFAULT_VALUE 15
#define UU_WLAN_CWMAX_DEFAULT_VALUE 1023

/** EDCA Parameter set default values for BE */
#define UU_WLAN_AIFSN_BE_DEFAULT_VALUE 7
#define UU_WLAN_TXOP_LIMIT_BE_DEFAULT_VALUE 0

/** EDCA Parameter set default values for BK */
#define UU_WLAN_AIFSN_BK_DEFAULT_VALUE 3
#define UU_WLAN_TXOP_LIMIT_BK_DEFAULT_VALUE 0

/** EDCA Parameter set default values for VI */
#define UU_WLAN_CWMIN_VI_DEFAULT_VALUE 7
#define UU_WLAN_CWMAX_VI_DEFAULT_VALUE 15
#define UU_WLAN_AIFSN_VI_DEFAULT_VALUE 2
#define UU_WLAN_TXOP_LIMIT_VI_DEFAULT_VALUE 0 // 3008 Non-zero for multiple-protection

/** EDCA Parameter set default values for VO */
#define UU_WLAN_CWMIN_VO_DEFAULT_VALUE 3
#define UU_WLAN_CWMAX_VO_DEFAULT_VALUE 7
#define UU_WLAN_AIFSN_VO_DEFAULT_VALUE 2
#define UU_WLAN_TXOP_LIMIT_VO_DEFAULT_VALUE 0 //1504 Non-zero for multiple-protection


#ifdef UU_WLAN_BQID
/** EDCA Parameter set default value for IBSS */
#define UU_WLAN_CW_BEACON_IBSS_VALUE        (2 * UU_WLAN_CWMIN_DEFAULT_VALUE)

/** EDCA Parameter set proprietary values for Beacon */
#define UU_WLAN_CW_BEACON_DEFAULT_VALUE      0
#define UU_WLAN_AIFSN_BEACON_DEFAULT_VALUE      1
#define UU_WLAN_TXOP_LIMIT_BEACON_DEFAULT_VALUE 0
#endif

#define UU_WLAN_BEACON_DEFAULT_VALUE_CMODEL 800000

#define UU_WLAN_RTSTHRESHOLD_DEFAULT_VALUE 1500

/** Register values in C Model */
#define UU_WLAN_SLOT_TIMER_DEFAULT_VALUE_CMODEL 1
#define UU_WLAN_CWMAX_DEFAULT_VALUE_CMODEL 15

UU_BEGIN_DECLARATIONS

extern uu_void uu_wlan_set_default_reg_values(uu_void);

UU_END_DECLARATIONS
#endif /*__UU_WLAN_REG_INIT_H__ */
