/*************************************************************************
**                                                                      **
** File name :  uu_wlan_mac_defaults.h                                  **
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

#ifndef __UU_WLAN_MAC_DEFAULTS_H__
#define __UU_WLAN_MAC_DEFAULTS_H__

#include "uu_wlan_main.h"

/**
 * Define AIFS, CWMIN, CWMAX, Short retry , Long retry values as per the 
 * ieee80211 standard
 */
#define INIT_AIFS       2
#define INIT_CWMIN      15
#define INIT_CWMAX      1023
#define INIT_SH_RETRY   10
#define INIT_LG_RETRY   10

/* Used to calculate tx time for non 5/10/40MHz
 * operation */
/* It's preamble time + signal time (16 + 4) */
#define UU_WLAN_INIT_OFDM_PREAMBLE_TIME    20 /* TODO: Review these macros */
#define UU_WLAN_INIT_OFDM_PLCP_BITS        22
#define UU_WLAN_INIT_OFDM_SYMBOL_TIME      4

/* Tx retry limit defaults from standard 
 * Refer IEEE802.11-2007 - Page 874 - 
 * dot11ShortRetryLimit Integer:= 7
 * dot11LongRetryLimit Integer:= 4,
 */
#define UU_WLAN_INIT_RETRY_SHORT           7
#define UU_WLAN_INIT_RETRY_LONG            4

/** Maximum number of subframes in aggregation */
#define UU_WLAN_AMPDU_SUBFRAME_DEFAULT 32


#endif /* __UU_WLAN_MAC_DEFAULTS_H__ */


