/*******************************************************************************
**                                                                            **
** File name :        uu_mac_scheduling.h                                     **
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

#include "uu_datatypes.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_limits.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_main.h"
#include "uu_mac_tx.h"


/* Maximum number of associated stations per AP is fixed to 20 */
#define IEEE80211_MAC_ADDR_LEN 6
#define EXP_AVG 1000
#define BETA 10

/** Array for keeping MAC address of each associated STA. */
extern uu_uint8 *macaddr_array[UU_WLAN_MAX_ASSOCIATIONS];


/** Structure for keeping statistics ( per STA ) for scheduling */
typedef struct uu_sch_stat
{
    /**
     * Number of frames in each AC queues
     * Index and corresponding AC
     * 0: UU_WLAN_AC_VO
     * 1: UU_WLAN_AC_VI
     * 2: UU_WLAN_AC_BE
     * 3: UU_WLAN_AC_BK
     */
    uu_uint32 current_supported_rate;
    struct uu_stat_queue
    {
        uu_uint32 no_of_frames_in_queue;

        /* Head of line packet delay. Currently not used */
        uu_uint32 HOL_packet_delay;

        /** Metric used for Proportional Fairness (PF) scheduling. 
         * This metric is the ratio of current supported rate to average allocated rate.
         * This metric is used for UU_WLAN_AC_BE and UU_WLAN_AC_BK scheduling
         */
        uu_uint32 PF_metric;

        /**
         * Max-weight scheduling metric.
         * This metric is the product of current quelength and current supported data rate.
         * Not using currently. 
         */
        uu_uint32 max_weight_metric;

        uu_uint32 avg_alloc_rate;
    }uu_stati_queue_t[UU_WLAN_MAX_QID]; /* TODO: Are you defining type / structure member? */

} uu_sch_stati_t;


/* TODO: Do name changes & cleanup */
extern uu_sch_stati_t *uu_umac_sch_stati_AID[UU_WLAN_MAX_ASSOCIATIONS];


/* TODO: Why are these 'uu_int16'? */
extern uu_int16 uu_mac_sch_add_to_sta_info_table(struct ieee80211_sta *sta);
extern uu_int16 uu_mac_sch_remove_from_sta_info_table(struct ieee80211_sta *sta);
extern uu_int16 uu_mac_sch_fill_current_rate_per_sta(struct ieee80211_hw *hw, struct sk_buff *skb, uu_uint16 index);
extern uu_int16 uu_mac_sch_update_avg_alloc_rate(uu_uint8 AC, uu_uint16 index, uu_uint32 current_alloc_rate);
extern uu_int32 uu_mac_sch_umac_sch(uu_uint8 AC);
extern uu_uint16 uu_mac_sch_get_index_from_macaddr(uu_uint8 *macaddr);


/* EOF */

