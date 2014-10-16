/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_umac_init.h                                     **
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

#ifndef __UU_WLAN_UMAC_INIT_H__
#define __UU_WLAN_UMAC_INIT_H__

#define CHAN2G(_freq, _idx)  { \
    .band = IEEE80211_BAND_2GHZ, \
    .center_freq = (_freq), \
    .hw_value = (_idx), \
    .max_power = 20, \
}

#define CHAN5G(_freq, _idx) { \
    .band = IEEE80211_BAND_5GHZ, \
    .center_freq = (_freq), \
    .hw_value = (_idx), \
    .max_power = 20, \
}

#define IEEE80211_RADIOTAP 803

#define RATE(_bitrate, _hw_rate) {          \
    .bitrate    = (_bitrate),               \
    .hw_value       = (_hw_rate),           \
}

/** Number of queues used in QoS mode */
#define UU_WLAN_QOS_QUEUES 4
/** Number of queues used in Non-QoS mode */
#define UU_WLAN_NON_QOS_QUEUES 1

/** Number of rates received from the mac80211 framework. 
 * 2 rates in the current implementation. primary rate and fallback rate.
 */
#define UU_WLAN_NUM_RATES_RECVED 2

/** Default values in current implementation. */
#define UU_WLAN_CHANNEL_CHANGE_TIME 5000
#define UU_WLAN_LISTEN_INTERVAL 10
#define UU_WLAN_MAX_RATE_TRIES 10
#define UU_WLAN_FRAG_THRESHOLD 1340

#endif /*__UU_WLAN_UMAC_INIT_H__ */
