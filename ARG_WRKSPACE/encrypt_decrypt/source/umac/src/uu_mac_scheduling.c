/*******************************************************************************
**                                                                            **
** File name :  uu_mac_scheduling.c                                           **
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

#include "uu_mac_scheduling.h"

/* TODO: Lot of cleanup is required in this file */

uu_uint8* macaddr_array[UU_WLAN_MAX_ASSOCIATIONS] = {NULL};
uu_sch_stati_t *uu_umac_sch_stati_AID[UU_WLAN_MAX_ASSOCIATIONS];


/** To search for a free postion in the station info table.
 * If a slot in associated station info table is free and we can add a new STA info.
 * This function returns the index of the free slot whichever comes first while searching.
 * If not free slot, it returns 'UU_WLAN_MAX_ASSOCIATIONS'.
 */
static uu_uint16 search_for_avail_slot(uu_void)
{
    uu_int16 j;
    for(j=0; j<UU_WLAN_MAX_ASSOCIATIONS; j++)
    {
        if(!macaddr_array[j])
            break;
    }
    return j;
}


/* To get index from MAC address */
uu_uint16 uu_mac_sch_get_index_from_macaddr(uu_uint8* macaddr)
{  
    uu_uint16  j;
    for(j=0; j<UU_WLAN_MAX_ASSOCIATIONS; j++)
    {
        if(macaddr_array[j] != NULL)
        {
            if(UU_IS_SAME_MAC_ADDR(macaddr, macaddr_array[j]))
                return j;
        }
    }
    UU_WLAN_LOG_ERROR(("\n\tINVALID MAC_ADDR\n"));
    return (UU_WLAN_MAX_ASSOCIATIONS);
}


/**
 * call this function in uu_wlan_add_station, in uu_mac_fwk_cb.c.
 * adds the station aid and mac address to the table.
 * initializes the statistics required for scheduling.
 */

uu_int16 uu_mac_sch_add_to_sta_info_table(struct ieee80211_sta *sta)
{
    uu_uint16 index;
    uu_uint8 i, j;

    index = search_for_avail_slot();
    if(index >= UU_WLAN_MAX_ASSOCIATIONS)
    {
        UU_WLAN_LOG_ERROR(("\nError: No empty space in table\n"));
        return -1;
    }

    /** Allocating memory to store mac address
     * Deallocate after dis association
     */
    macaddr_array[index] = (uu_uint8 *) kmalloc(IEEE80211_MAC_ADDR_LEN*sizeof(uu_uint8), GFP_ATOMIC);

    UU_COPY_MAC_ADDR(macaddr_array[index], sta->addr);

    /** Allocate memory for scheduling statistics
     * Deallocate after station dis association
     */
    uu_umac_sch_stati_AID[index] = (uu_sch_stati_t *) kmalloc(sizeof(uu_sch_stati_t), GFP_ATOMIC);

    /** Initialise statistics structure */
    for(j=0; j<UU_WLAN_AC_MAX_AC; j++)
    {
        uu_umac_sch_stati_AID[index]->uu_stati_queue_t[j].no_of_frames_in_queue = 0;
        uu_umac_sch_stati_AID[index]->uu_stati_queue_t[j].HOL_packet_delay = 0;
        uu_umac_sch_stati_AID[index]->uu_stati_queue_t[j].PF_metric = 0;
        uu_umac_sch_stati_AID[index]->uu_stati_queue_t[j].max_weight_metric = 0;
        uu_umac_sch_stati_AID[index]->uu_stati_queue_t[j].avg_alloc_rate = 1000;
    }
    uu_umac_sch_stati_AID[index]->current_supported_rate = 0;
    printk("station added at index %d\n", index);    
    return 0;
} /* uu_mac_sch_add_to_sta_info_table */


/** Call this function in uu_wlan_del_station, in uu_mac_fwk_cb.c.
 * removes the station details from table.
 * replaces the corresponding aid with 0.
 * replaces the curresponding mac address pointer with NULL.
 */
uu_int16 uu_mac_sch_remove_from_sta_info_table(struct ieee80211_sta *sta)
{
    uu_uint16 index = uu_mac_sch_get_index_from_macaddr(sta->addr);

    if (index >= UU_WLAN_MAX_ASSOCIATIONS)
        return -1;

    kfree(uu_umac_sch_stati_AID[index]);
    kfree(macaddr_array[index]);
    macaddr_array[index] = NULL;
    return 0;
}


/** Get the current supported rate from skb.
 * Rate control algorithm calculates the current rate.
 * update the corresponding current(primary is used) rate.
 */
uu_int16 uu_mac_sch_fill_current_rate_per_sta(struct ieee80211_hw *hw, struct sk_buff *skb, uu_uint16 index)
{
    uu_uint32 current_rate;
    struct ieee80211_tx_info *tx_info;
    UU_WLAN_LOG_DEBUG(("\n\n\n Filling current rate: index is %d \n\n\n", index));
    tx_info = IEEE80211_SKB_CB(skb);
    current_rate = ieee80211_get_tx_rate(hw, tx_info)->bitrate; /* in 100Kbps */
    uu_umac_sch_stati_AID[index]->current_supported_rate = current_rate;
    return 0;
}


/**
 * Used to update the average allocated rate for each station.
 * Call this function after geting tx status from LMAC.
 * Exponential average is used.
 * new_avg = ((1000-beta)*past_avg + (beta)*present_value)/1000
 * Here 99% of the past average rate is considered and only 1% of 
 * the present value is consider.
 */
uu_int16 uu_mac_sch_update_avg_alloc_rate(uu_uint8 AC, uu_uint16 index, uu_uint32 current_alloc_rate)
{
    uu_int16 j;
    uu_uint32 avg_rate_int;
    uu_uint32 current_rate;

    /**
     * values are EXP_AVG times the actual value. Currently 1000 times.
     * avg_rate_int is the integer part of average rate (multiplied by EXP_AVG).
     * avg_rate_deci is the decimal part of average rate (multiplied by EXP_AVG).
     */

    avg_rate_int = (EXP_AVG - BETA)*uu_umac_sch_stati_AID[index]->uu_stati_queue_t[AC].avg_alloc_rate;
    current_rate = BETA*current_alloc_rate;
    avg_rate_int = (avg_rate_int + current_rate)/1000;
    /* Update average rate of scheduled station */
    uu_umac_sch_stati_AID[index]->uu_stati_queue_t[AC].avg_alloc_rate = avg_rate_int;

    /* Update average rate of other stations */
    for(j=0;(j<UU_WLAN_MAX_ASSOCIATIONS);j++)
    {
        /* only for associated station */
        if((j != index) && macaddr_array[j])
        {
            avg_rate_int =( (EXP_AVG - BETA)*(uu_umac_sch_stati_AID[j]->uu_stati_queue_t[AC].avg_alloc_rate))/1000;
            uu_umac_sch_stati_AID[j]->uu_stati_queue_t[AC].avg_alloc_rate = avg_rate_int;
        }
    }

    return 0;
} /* uu_mac_sch_update_avg_alloc_rate */


/**
 * PF scheduler.
 * Compares the PF metric of each station and selects the station with highest metric.
 * AC - Access category, for which frames have to be scheduled
 */
uu_int32 uu_mac_sch_umac_sch(uu_uint8 AC)
{    
    uu_uint8 j;
    uu_uint8 *sch_macaddr;
    uu_uint32 best_metric = 0;
    uu_uint32 avg_AC;
    uu_uint16 sch_index = 0;

    if(AC >= UU_WLAN_AC_MAX_AC)
    {
        UU_WLAN_LOG_ERROR(("\nError: Invalid access category\n"));
        return -1;
    }

    /* For UU_WLAN_UU_WLAN_AC_BE */
    
    /**
     * Proportional fair rate allocation is used for UU_WLAN_UU_WLAN_AC_BE.
     * Algorithm is not finalized. We shall make minor modifications in the algorithm.
     * Algorithm uses the rate information provided by rate control algorithm and
     * the average allocated rate for each STA.
     * Exponential moving average is used.
     */

    for(j=0; j<UU_WLAN_MAX_ASSOCIATIONS; j++)
    {
        if(macaddr_array[j] && uu_umac_sch_stati_AID[j]->uu_stati_queue_t[AC].no_of_frames_in_queue)
        {
            UU_WLAN_LOG_DEBUG(("No. of frames in queue index %d is %d\n", j, uu_umac_sch_stati_AID[j]->uu_stati_queue_t[AC].no_of_frames_in_queue));
            /**
             * Update PF metric
             * Current supported rate is in units of 100Kbps
             * Algorithm begaviour is invarient to linear scaling of rates
             * average allocated rate is in units of Kbps
             */
            avg_AC = uu_umac_sch_stati_AID[j]->uu_stati_queue_t[AC].avg_alloc_rate;
            uu_umac_sch_stati_AID[j]->uu_stati_queue_t[AC].PF_metric = (10000*uu_umac_sch_stati_AID[j]->current_supported_rate) / avg_AC;

            UU_WLAN_LOG_DEBUG(("Current supported rate is %d\n", uu_umac_sch_stati_AID[j]->current_supported_rate));
            /**
             * Get the STA with highest metric
             * We are comparing only the integer part
             */
            if(uu_umac_sch_stati_AID[j]->uu_stati_queue_t[AC].PF_metric >= best_metric)
            {
                best_metric = uu_umac_sch_stati_AID[j]->uu_stati_queue_t[AC].PF_metric;
                UU_WLAN_LOG_DEBUG(("New best metric is %d, index is %d\n", best_metric, j));
                sch_macaddr = macaddr_array[j];
                sch_index = j;
            }
        }
    }

    return sch_index;
} /* uu_umac_sch */


/* EOF */

