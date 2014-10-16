/*******************************************************************************
**                                                                            **
** File name :  uu_mac_fwk_cb.c                                               **
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
/* $Revision: 1.19 $ */

#include <linux/version.h>

#include "uu_datatypes.h"
#include "uu_errno.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_limits.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_rate.h"
#include "uu_wlan_main.h"
#include "uu_wlan_recv.h"
#include "uu_mac_tx.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_reg_init.h"
#include "uu_wlan_agg.h"
#include "uu_mac_scheduling.h"

#ifdef UU_WLAN_DFS
#include "uu_wlan_umac_dfs.h"
#endif
#ifdef UU_WLAN_TPC
#include "uu_wlan_umac_tpc.h"
#endif

/** This flag indicates to check whether BA session for the corresponding 
 * tid is established or not. If the flag for tid is 1, then BA session is 
 * established for that tid. 
 */
uu_uint8 uu_wlan_ba_ready_flag_g[8];

/** This flag is set when mac80211 configured to powersave mode and 
 *reset when it disables powersave mode
 **/
uu_bool uu_wlan_ps_g;

/** This function is registered as start call back of mac80211 framework.
 * This is triggered before the first netdevice is attached to the hardware
 * is enabled.
 * This is manatory call back function.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 */
static uu_int32 uu_wlan_start(struct ieee80211_hw *hw)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
    uu_wlan_umac_context_t   *mac_context = hw->priv;
    struct ieee80211_channel *curchan = hw->conf.channel;

    /* setup initial channel */
    mac_context->chan_id = curchan->hw_value;

    UU_WLAN_LOG_DEBUG(("Recved band is %d and hw_value is %d \n", curchan->band, curchan->hw_value));

#endif
    /* Keep default mode as station. Operation mode will be changed based on the userspace application.  */    
    uu_setReg(UU_WLAN_IEEE80211_OP_MODE_R, UU_WLAN_MODE_MANAGED);

    uu_wlan_context_gp->cur_op_mode = NL80211_IFTYPE_STATION;

    /** Keep default mode as QOS 
     * value 1 indicates QOS mode and value 0 indicates non-QoS mode.
     */
    uu_setReg(UU_WLAN_IEEE80211_QOS_MODE_R,0x1);

    /* Work queue creation for lmac */
    //INIT_WORK(&lmac_work, lmac_work_queue);

    if (uu_wlan_lmac_start_platform() != UU_SUCCESS)
    {
        UU_WLAN_LOG_ERROR(("UURMIDRIVER - uu_wlan_start return values returns uu_flase \n"));
        return -EIO;
    }
    UU_WLAN_LOG_DEBUG(("UURMIDRIVER : Inside uu_start\n"));

    uu_wlan_context_gp->running_staus = 1;
    return 0;
} /* uu_wlan_start */


/** This function is registered as stop call back of mac80211 framework.
 * This is triggered after last netdevice attached to the hardware
 * is disabled.
 * This is manatory call back function.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 */
static uu_void uu_wlan_stop(struct ieee80211_hw *hw)
{
    UU_WLAN_LOG_DEBUG(("UURMIDRIVER : Inside uu_stop \n"));
#ifdef UU_WLAN_TPC
#ifndef UU_TPC_HRTIMER_CONST
    if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
        uu_wlan_tpc_timer_stop();
#endif
#endif

    if(uu_wlan_context_gp->running_staus == 0)
        return;

    uu_wlan_context_gp->running_staus = 0;
    if( uu_wlan_lmac_stop_platform())
    {
        UU_WLAN_LOG_DEBUG(("LMAC is stopped\n"));
    }
} /* uu_wlan_stop */


uu_void uu_wlan_umac_stop(struct ieee80211_hw *hw)
{
    /*
    if(uu_wlan_context_gp->running_staus == 1)
        uu_wlan_umac_stop(hw);
    */
} /* uu_wlan_umac_stop */

/** This function is registered as tx call back of mac80211 framework.
 * This is triggered from the mac80211 framework, for each transmitted frame.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] skb Contains the frame which came from mac80211 framework.
 * Note : New parameter "struct ieee80211_tx_control *control" is added from linux 3.10 kernel onwards.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
static uu_void uu_wlan_tx(struct ieee80211_hw *hw, struct sk_buff *skb)
#else
static uu_void uu_wlan_tx(struct ieee80211_hw *hw, struct ieee80211_tx_control *control, struct sk_buff *skb)
#endif
{
    uu_uint8 macaddr[IEEE80211_MAC_ADDR_LEN];
    uu_int16 index;
    uu_wlan_umac_context_t *mac_context = hw->priv;
    struct ieee80211_hdr *hdr;

    if(NULL == skb)
    {
        UU_WLAN_LOG_ERROR(("*******UURMI ERROR: uu_wlan_tx - skb is NULL\n"));
        return;
    }

    hdr = (struct ieee80211_hdr *)skb->data;    /* TODO Handle Power management  */
    /* Add to list based on STA address and queue mapping */
    index = 0; /* index is 0 for broadcast frames */

    if(ieee80211_is_data(hdr->frame_control) && (!is_multicast_ether_addr(hdr->addr1)) && (!ieee80211_is_mgmt(hdr->frame_control)))
    {
        UU_WLAN_LOG_DEBUG(("------ DATA FRAME from FWK: in fwk_cb ---- \n"));

        /* Get the index for adding in to UMAC queue*/
        UU_COPY_MAC_ADDR(macaddr, &skb->data[UU_RA_OFFSET]);
        index = uu_mac_sch_get_index_from_macaddr(macaddr);
        if(index >= UU_WLAN_MAX_ASSOCIATIONS)
        {
            /* TODO: Not expected here. Take some action */
        }

        /* Fill current rate */
        uu_mac_sch_fill_current_rate_per_sta(hw, skb, index);
    }

    /* Handling power management */
    if (ieee80211_is_data(hdr->frame_control) && uu_wlan_ps_g)
    {
        hdr->frame_control |= cpu_to_le16(IEEE80211_FCTL_PM);
    }

    if (uu_wlan_tx_start(hw, skb, &mac_context->tx_q[uu_wlan_umac_get_ac(skb_get_queue_mapping(skb))][index]) != 0)
    {
        UU_WLAN_LOG_ERROR(("UURMI: TX failed\n"));
        /* TODO: Handle error (may never happen). Send failure status through 'ieee80211_tx_status_irqsafe' */
    }
} /* uu_wlan_tx */


/** This function is registered as add_interface call back of mac80211 framework.
 * This is triggered from the mac80211 framework, for each attached netdevice.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] vif Contains the deatains of virtual interface.
 * Based on this return type mac80211 framework may refuse or accept this interface.
 */
static uu_int32 uu_wlan_add_interface(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif)
{
    uu_wlan_umac_context_t *mac_context = hw->priv;
    struct ieee80211_bss_conf *bss_conf = &vif->bss_conf;
    uu_wlan_beacon_config_t *cur_conf = &mac_context->cur_beacon_conf;

    UU_WLAN_LOG_DEBUG(("UURMIDRIVER : Inside uu_add_interface \n"));

    //mutex_lock(&mac_context->mutex);

    switch (vif->type) 
    {
        case NL80211_IFTYPE_STATION:
            break;
        case NL80211_IFTYPE_AP:
            break;
        case NL80211_IFTYPE_WDS:
        case NL80211_IFTYPE_ADHOC:
        case NL80211_IFTYPE_MESH_POINT:
            break;
        default:
            goto out;
    }

    /* Configure beacon */

    /* Setup the beacon configuration parameters */
    cur_conf->beacon_interval = bss_conf->beacon_int;

    if (cur_conf->beacon_interval == 0)
        cur_conf->beacon_interval = 100;

    /* end of beacon configuration */

    return 0;

out:
    // mutex_unlock(&mac_context->mutex);
    return 0;
} /* uu_wlan_add_interface */


/** This function is registered as remove_interface call back of mac80211 framework.
 * This is triggered from the mac80211 framework, for each interface down event.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] vif Contains the deatains of virtual interface.
 */
static uu_void uu_wlan_remove_interface(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif)
{
    UU_WLAN_LOG_DEBUG(("UURMIDRIVER : Inside uu_remove_interface \n"));
} /* uu_wlan_remove_interface */


/** This function is registered as config call back of mac80211 framework.
 * Handles all configuration requests.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] changed Contains the changed configuration.
 */
static uu_int32 uu_wlan_config(struct ieee80211_hw *hw, uu_uint32 changed)
{
    struct ieee80211_conf *conf = &hw->conf;
    uu_wlan_umac_context_t *mac_context = hw->priv;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
    mac_context->hw->conf.channel = conf->channel;
    mac_context->hw->conf.power_level = conf->power_level;
#else
    mac_context->hw->conf.chandef.chan = conf->chandef.chan;
#endif

    if (changed & IEEE80211_CONF_CHANGE_PS)
    {
        if (conf->flags & IEEE80211_CONF_PS)
        {
            uu_wlan_ps_g = UU_TRUE;
            uu_wlan_ps_mode_r = UU_TRUE;
        }
        else
        {
            uu_wlan_ps_g = UU_FALSE;
            uu_wlan_ps_mode_r = UU_FALSE;
        }
    }

    return 0;
} /* uu_wlan_config */


/** This function is registered as config_tx call back of mac80211 framework.
 * Mac80211 framework triggers this, with the TX configuration parameter (parameters
 * are configured from userspace applications) like Configuring TX queue parameters,
 * EDCA parameters set.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] vif Contains the deatains of virtual interface.
 * @param[in] queue Contains the hardware queue id.
 * @param[in] params Contains the TX queue parameter values which are passed from 
 * userspace applications like hostapd or supplicant.
 */
static uu_int32 uu_wlan_conf_tx(struct ieee80211_hw *hw, 
        struct ieee80211_vif *vif, uu_uint16 queue,
        const struct ieee80211_tx_queue_params *params)
{
    uu_wlan_tx_queue_info_t qinfo;

    if (queue >= WME_NUM_AC)
    {
        UU_WLAN_LOG_ERROR(("Wrong Queue %d \n",queue));
        return 0;
    }

    UU_WLAN_LOG_DEBUG(("Queue id is %d\n", queue));

    if(params == NULL)
    {
        UU_WLAN_LOG_ERROR(("No values returned from hostapd \n"));
        return 0;
    }

    /** Update configuration params to registers
     * 1. Get the existing qinfo
     * 2. Update aifs, cwmin, cwmax parameters to configured values
     * 3. Set the hardware with updated params
     */

    memset(&qinfo, 0, sizeof(uu_wlan_tx_queue_info_t));

    qinfo.tqi_aifs = params->aifs;
    qinfo.tqi_cwmin = params->cw_min;
    qinfo.tqi_cwmax = params->cw_max;
    //qinfo.tqi_burstTime = params->txop;

    return 0;
} /* uu_wlan_conf_tx */


/** This function is registered as configure_filter call back of mac80211 framework.
 * Device rx filters. 
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] changed_flags Contains the changed RX filters.
 */
static uu_void uu_wlan_configure_filter(struct ieee80211_hw *hw,
        uu_uint32 changed_flags,
        uu_uint32 *total_flags,
        u64 multicast)
{
    uu_int32 rxfilters;

    rxfilters = uu_wlan_rx_setfilters(*total_flags);
    *total_flags = rxfilters;
} /* uu_wlan_configure_filter */


/** This function is registered as sta_add call back of mac80211 framework.
 * MAC80211 framework will notify UMAC about addition of an associated station.
 * Triggered for every association.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] vif Contains the deatains of virtual interface.
 * @param[in] sta Contains the associated station information.
 */
static uu_int32 uu_wlan_add_station(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif,
        struct ieee80211_sta *sta)
{
    //printk("MODE AFTER ASSOC IS: %d\n\n", uu_dot11_op_mode_r);
    UU_WLAN_LOG_DEBUG(("In function add station"));
    if(uu_mac_sch_add_to_sta_info_table(sta) < 0)
    {
        UU_WLAN_LOG_DEBUG(("Failed to add station details in table"));
    }

#if 1 /* TODO: Move it to platform dependent layer for C model */
    uu_wlan_lmac_config_t cmd_info;

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_ASSO_ADD;

    /** Filling of station info */
    cmd_info.cmd_data.asso_add.aid = sta->aid;
    cmd_info.cmd_data.asso_add.ch_bndwdth = CBW40;

    memcpy(cmd_info.cmd_data.asso_add.addr, sta->addr, IEEE80211_MAC_ADDR_LEN);
    /* TODO: Fill the remaining part of the structure */
    uu_wlan_lmac_config(&cmd_info);
    /** Filling associated station information */
#endif

#ifdef UU_WLAN_TPC
#ifdef UU_WLAN_DFS
    if (vif->type == NL80211_IFTYPE_STATION)
    {
        memcpy(uu_wlan_context_gp->bssid,  sta->addr, IEEE80211_MAC_ADDR_LEN);
    }
    else if (vif->type == NL80211_IFTYPE_AP)
    {
        memcpy(uu_wlan_context_gp->assoc_sta,  sta->addr, IEEE80211_MAC_ADDR_LEN);
    }
#else /* Not UU_WLAN_DFS */
    if (vif->type == NL80211_IFTYPE_STATION)
    {
        memcpy(uu_wlan_context_gp->bssid,  sta->addr, IEEE80211_MAC_ADDR_LEN);
    }
    else if (vif->type == NL80211_IFTYPE_AP)
    {
        memcpy(uu_wlan_context_gp->assoc_sta,  sta->addr, IEEE80211_MAC_ADDR_LEN);
    }
#endif /* UU_WLAN_DFS */
#else /* Not UU_WLAN_TPC */
#ifdef UU_WLAN_DFS
    if (vif->type == NL80211_IFTYPE_STATION)
    {
        memcpy(uu_wlan_context_gp->bssid,  sta->addr, IEEE80211_MAC_ADDR_LEN);
    }
    else if (vif->type == NL80211_IFTYPE_AP)
    {
        memcpy(uu_wlan_context_gp->assoc_sta,  sta->addr, IEEE80211_MAC_ADDR_LEN);
    }
#endif /* UU_WLAN_DFS */
#endif /* UU_WLAN_TPC */


#ifdef UU_WLAN_TPC
    if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
    {
        uu_wlan_tpc_timer_start();
    }
#endif
#ifdef UU_WLAN_DFS
    if (uu_dot11_op_mode_r == UU_WLAN_MODE_MASTER)
    {
        uu_wlan_dfs_timer_start();
    }
#endif

    //uu_wlan_add_station_platform(hw,vif,sta);
    return 0;
} /* uu_wlan_add_station */


/** This function is registered as sta_remove call back of mac80211 framework.
 * MAC80211 framework will notify UMAC about removal of an associated station.
 * Triggered for every removed association.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] vif Contains the deatains of virtual interface.
 * @param[in] sta Contains the removed station information.
 */
static uu_int32 uu_wlan_del_station(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif,
        struct ieee80211_sta *sta)
{
    /* Remove entry from aid-macaddr table */
    uu_mac_sch_remove_from_sta_info_table(sta);

#if 0 /* PLatform specific */
    /* TODO: Properly del the station into LMAC */
    uu_wlan_lmac_config_t cmd_info;

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_ASSO_DEL;

    /** Filling of station info */
    cmd_info.cmd_data.asso_del.aid = sta->aid;

    memcpy(cmd_info.cmd_data.asso_del.addr, sta->addr, IEEE80211_MAC_ADDR_LEN);

    /* TODO: Is this required, for deleting station context? */
    uu_wlan_lmac_config(&cmd_info);
    /** Filling associated station information */
    return 0;
#endif
    //uu_wlan_del_station_platform(hw,vif,sta);

    return 0;
}


/** This function is used change the mode of operation based on the
 * mode which is taken from the userspace application.
 * ie Based on hostapd/ supplicant mode will be decided.
 * This is called from bss_change_info function.
 * LMAC mode switch is triggered from this function.
 */
static uu_void uu_wlan_change_mode(uu_int32 vif_type)
{
    uu_wlan_context_gp->cur_op_mode = vif_type;

    if(vif_type == NL80211_IFTYPE_AP )
    {
        UU_WLAN_LOG_DEBUG(("BSS Changed (AP)\n"));
        uu_setReg(UU_WLAN_IEEE80211_OP_MODE_R,UU_WLAN_MODE_MASTER);
#ifdef UU_WLAN_BQID
        dot11_cwmin_value_ac[4] = UU_WLAN_CW_BEACON_DEFAULT_VALUE;
        dot11_cwmax_value_ac[4] = UU_WLAN_CW_BEACON_DEFAULT_VALUE;
#endif
        INIT_WORK(&uu_wlan_context_gp->beacon_update_trigger, uu_wlan_beacon_update_trigger_work);
    }
    else if(vif_type == NL80211_IFTYPE_STATION)
    {
        UU_WLAN_LOG_DEBUG(("BSS Changed (STATION)\n"));
        uu_setReg(UU_WLAN_IEEE80211_OP_MODE_R,UU_WLAN_MODE_MANAGED);
    }
    else if(vif_type == NL80211_IFTYPE_MONITOR)
    {
        uu_setReg(UU_WLAN_IEEE80211_OP_MODE_R,UU_WLAN_MODE_MONITOR);
    }
    else if(vif_type == NL80211_IFTYPE_ADHOC)
    {
        UU_WLAN_LOG_DEBUG(("BSS Changed (IBSS)\n"));
        uu_setReg(UU_WLAN_IEEE80211_OP_MODE_R,UU_WLAN_MODE_ADHOC);
#ifdef UU_WLAN_BQID
        dot11_cwmin_value_ac[4] = UU_WLAN_CW_BEACON_IBSS_VALUE;
        dot11_cwmax_value_ac[4] = UU_WLAN_CW_BEACON_IBSS_VALUE;
#endif
        INIT_WORK(&uu_wlan_context_gp->beacon_update_trigger, uu_wlan_beacon_update_trigger_work);
    }
    else if(vif_type == NL80211_IFTYPE_MESH_POINT)
    {
        uu_setReg(UU_WLAN_IEEE80211_OP_MODE_R,UU_WLAN_MODE_MESH);
        INIT_WORK(&uu_wlan_context_gp->beacon_update_trigger, uu_wlan_beacon_update_trigger_work);
    }
    /*
    else if(vif->type == NL80211_IFTYPE_MONITOR)
    {
        UU_WLAN_IEEE80211_OP_MODE_R = UU_WLAN_MODE_REPEATER;
    }
    */

    if(uu_wlan_lmac_mode_switch_platform() != UU_SUCCESS)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER - uu_wlan_restart return values returns uu_flase \n"));
    }
} /* uu_wlan_change_mode  */


/** This function is registered as bss_info_changed call back of mac80211 framework.
 * MAC80211 framework will notify UMAC about every change in the BSS. ie Configuration requests
 * related to BSS parameters that may vary during BSS's lifespan, and may affect low
 * level driver (e.g. assoc/disassoc status, erp parameters).
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] vif Contains the deatains of virtual interface.
 * @param[in] bss_conf Contains the changed configuration details.
 * @param[in] changed Contains the flags for changed configuration details.
 */
static uu_void uu_wlan_bss_change_info(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif,
        struct ieee80211_bss_conf *bss_conf,
        uu_uint32 changed)
{
    uu_wlan_umac_context_t *mac_context = hw->priv;
    uu_wlan_hw_t *uhw = mac_context->mac_context_hw;
    uu_int32 slottime;
    uu_uint16 bssrate;

    UU_WLAN_LOG_DEBUG(("BSS change info \n"));

    if(uu_wlan_context_gp->cur_op_mode != vif->type)
    {
        uu_wlan_change_mode(vif->type);
    }

    if (changed & BSS_CHANGED_ERP_SLOT)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  slot timing changed \n"));

        if (bss_conf->use_short_slot)
            slottime = 9;
        else
            slottime = 20;

        UU_WLAN_LOG_DEBUG(("solttime is %d\n",slottime));
        if (vif->type == NL80211_IFTYPE_AP)
        {
            /*
             * Defer update, so that connected stations can adjust
             * their settings at the same time.
             * TODO handle later
             */
            mac_context->beacon.slottime = slottime;
        }
        else 
        {
            uhw->slottime = slottime;
        }
    }
    if (changed & BSS_CHANGED_BASIC_RATES)
    {
        bssrate = (vif->bss_conf.basic_rates & 0xfff);
        uu_setReg(BSSBasicRateSet,bssrate);
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: - Basic rateset changed. New set: %x\n", BSSBasicRateSet));
    }

#ifdef UU_WLAN_IBSS
    if (changed & BSS_CHANGED_BSSID)
    {
        if (!is_zero_ether_addr(bss_conf->bssid))
        {
            memcpy(uu_dot11_sta_bssid_r, bss_conf->bssid, IEEE80211_MAC_ADDR_LEN);
            UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  BSSID changed \n"));
        }
    }
    if (changed & BSS_CHANGED_IBSS)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  IBSS join status changed \n"));
        if (!is_zero_ether_addr(bss_conf->bssid))
        {
            memcpy(uu_dot11_sta_bssid_r, bss_conf->bssid, IEEE80211_MAC_ADDR_LEN);
        }
    }
    if (changed & BSS_CHANGED_BEACON_INT)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  Beacon interval changed \n"));
    }
    if (changed & BSS_CHANGED_BEACON)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  Beacon data changed, retrieve new beacon\n"));
    }
    if (changed & BSS_CHANGED_BEACON_ENABLED)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  Beacon Enabled\n"));
    }
#endif

#if 0 /** Enable later based on the support. */
    if (changed & BSS_CHANGED_ASSOC)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  association status changed (associated/disassociated) \n"));
    }
    if (changed & BSS_CHANGED_ERP_CTS_PROT)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  CTS protection changed \n"));
    }
    if (changed & BSS_CHANGED_ERP_PREAMBLE)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  preamble changed \n"));
    }
    if (changed & BSS_CHANGED_HT)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  802.11n parameters changed \n"));
    }
    if (changed & BSS_CHANGED_BASIC_RATES)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  Basic rateset changed \n"));
    }
    if (changed & BSS_CHANGED_CQM)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  Connection quality monitor config changed\n"));
    }
    if (changed & BSS_CHANGED_ARP_FILTER)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  Hardware ARP filter address list or state changed. \n"));
    }
    if (changed & BSS_CHANGED_QOS)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  QoS for this association was enabled/disabled. \n"));
    }
    if (changed & BSS_CHANGED_IDLE)
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER: -  Idle changed for this BSS/interface. \n"));
    }
#endif

    /* reference from mac80211 hw sim */
    if (vif->type != NL80211_IFTYPE_AP &&
            vif->type != NL80211_IFTYPE_MESH_POINT &&
            vif->type != NL80211_IFTYPE_ADHOC)
    {
        return;
    }

    mac_context->vif = vif;

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
    if(mac_context->hw)
        ieee80211_queue_work(mac_context->hw, &mac_context->beacon_update_trigger);
#endif
    return ;
} /* uu_wlan_bss_change_info */


#if 0 /* TODO: Implement as platform dependent */
/* Sends the addba command to LMAC, LMAC adds the BA information to BA table */
uu_void uu_wlan_umac_send_addba_info(uu_uint8 tid, uu_char* addr, uu_uint16 bufsz, uu_uint16 ssn, uu_uint8 type, uu_bool dir)
{
    uu_wlan_lmac_config_t cmd_info;

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_BA_SESS_ADD;

    /** Filling of ADDBA info */
    memcpy(cmd_info.cmd_data.addba.addr, addr, IEEE80211_MAC_ADDR_LEN);
    cmd_info.cmd_data.addba.tid = tid;
    cmd_info.cmd_data.addba.is_dir_tx = dir;
    cmd_info.cmd_data.addba.ssn = ssn;
    cmd_info.cmd_data.addba.type = type;
    cmd_info.cmd_data.addba.bufsz = bufsz;

    UU_WLAN_LOG_DEBUG(("UMAC-ADDBA: STA Addr: %x:%x:%x:%x:%x:%x, TID:%d, Buffer Size:%d, SSN=%d, type=%d, dir:%d\n",
                cmd_info.cmd_data.addba.addr[0], cmd_info.cmd_data.addba.addr[1], cmd_info.cmd_data.addba.addr[2], cmd_info.cmd_data.addba.addr[3],
                cmd_info.cmd_data.addba.addr[4], cmd_info.cmd_data.addba.addr[5], cmd_info.cmd_data.addba.tid, cmd_info.cmd_data.addba.bufsz,
                cmd_info.cmd_data.addba.ssn, cmd_info.cmd_data.addba.type, cmd_info.cmd_data.addba.is_dir_tx));

    uu_wlan_lmac_config(&cmd_info);
} /* uu_wlan_umac_send_addba_info */


/* Sends the delba command to LMAC, LMAC removes the BA information from BA table */
uu_void uu_wlan_umac_send_delba_info(uu_uint8 tid, uu_char* addr, uu_uint8 dir)
{
    uu_wlan_lmac_config_t cmd_info;

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_BA_SESS_DEL;

    /** Filling of DELBA info */
    memcpy(cmd_info.cmd_data.delba.addr, addr, IEEE80211_MAC_ADDR_LEN);
    cmd_info.cmd_data.delba.tid = tid;
    cmd_info.cmd_data.delba.is_dir_tx = dir;

    UU_WLAN_LOG_DEBUG(("UMAC-DELBA:  STA Addr: %x:%x:%x:%x:%x:%x, TID:%d \n",
                cmd_info.cmd_data.delba.addr[0], cmd_info.cmd_data.delba.addr[1], cmd_info.cmd_data.delba.addr[2], cmd_info.cmd_data.delba.addr[3],
                cmd_info.cmd_data.delba.addr[4], cmd_info.cmd_data.delba.addr[5], cmd_info.cmd_data.delba.tid));
    uu_wlan_lmac_config(&cmd_info);
} /* uu_wlan_umac_send_delba_info */
#endif /* #if 0 */


/** This function is registered as ampdu_action call back of mac80211 framework.
 * MAC80211 framework will notify UMAC, to perform required ampdu action.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] vif Contains the deatains of virtual interface.
 * @param[in] action  action to be taken
 * @param[in] sta Contains the deatains of sta.
 * @param[in] tid tid value
 * @param[in] ssn sequence number
 * @param[in] buf_size size of the buffer
 * Note: IEEE80211_AMPDU_TX_STOP is divided into following three.
 * @IEEE80211_AMPDU_TX_STOP_CONT: stop TX aggregation but continue transmitting
 *  queued packets, now unaggregated. After all packets are transmitted the
 *  driver has to call ieee80211_stop_tx_ba_cb_irqsafe().
 * @IEEE80211_AMPDU_TX_STOP_FLUSH: stop TX aggregation and flush all packets,
 *  called when the station is removed. There's no need or reason to call
 *  ieee80211_stop_tx_ba_cb_irqsafe() in this case as mac80211 assumes the
 *  session is gone and removes the station.
 * @IEEE80211_AMPDU_TX_STOP_FLUSH_CONT: called when TX aggregation is stopped
 *  but the driver hasn't called ieee80211_stop_tx_ba_cb_irqsafe() yet and
 *  now the connection is dropped and the station will be removed. Drivers
 *  should clean up and drop remaining packets when this is called.
 *  TODO: handle all three during aggregation and scheduling changes.
 */
uu_int32 uu_wlan_ampdu_action(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif,
        enum ieee80211_ampdu_mlme_action action,
        struct ieee80211_sta *sta,
        uu_uint16 tid, uu_uint16 *ssn, uu_uint8 buf_size)
{
    switch (action)
    {
        case IEEE80211_AMPDU_RX_START:
            {
                UU_WLAN_LOG_DEBUG(("UMAC: RX START tid = %d ssn is %d buf_size is %d\n", tid, *ssn,  buf_size));
                uu_wlan_ba_ready_flag_g[tid] = 1;
                uu_wlan_umac_send_addba_info_platform(tid, sta->addr, 8/*TODO*/, *ssn, (UU_WLAN_BA_SES_TYPE_IMMIDIATE | UU_WLAN_BA_SES_TYPE_IMPLICIT) , 0);
            }
            break;
        case IEEE80211_AMPDU_RX_STOP:
            {
                UU_WLAN_LOG_DEBUG(("UMAC: RX STOP tid = %d \n", tid));
                uu_wlan_ba_ready_flag_g[tid] = 0;
                uu_wlan_umac_send_delba_info_platform(tid, sta->addr, 0);
            }
            break;
        case IEEE80211_AMPDU_TX_START:
            {
                UU_WLAN_LOG_DEBUG(("UMAC: TX START tid = %d ssn is %d buf_size is %d\n", tid, *ssn,  buf_size));
                uu_wlan_umac_send_addba_info_platform(tid, sta->addr, 8/*TODO*/, *ssn, (UU_WLAN_BA_SES_TYPE_IMMIDIATE | UU_WLAN_BA_SES_TYPE_IMPLICIT) , 1);
                ieee80211_start_tx_ba_cb_irqsafe(vif, sta->addr, tid);
            }
            break;
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,10,0)
        case IEEE80211_AMPDU_TX_STOP:
#else
        case IEEE80211_AMPDU_TX_STOP_CONT:
        case IEEE80211_AMPDU_TX_STOP_FLUSH:
        case IEEE80211_AMPDU_TX_STOP_FLUSH_CONT:
#endif
            {
                UU_WLAN_LOG_DEBUG(("UMAC: TX STOP tid = %d \n", tid));
                uu_wlan_ba_ready_flag_g[tid] = 0;
                uu_wlan_umac_send_delba_info_platform(tid, sta->addr, 1);
                ieee80211_stop_tx_ba_cb_irqsafe(vif, sta->addr, tid);
            }
            break;
        case IEEE80211_AMPDU_TX_OPERATIONAL:
            {
                uu_wlan_ba_ready_flag_g[tid] = 1;
                UU_WLAN_LOG_DEBUG(("UMAC: TX OPERATIONAL and tid = %d buf_size is %d\n", tid, buf_size));
            }
            break;
        default:
            {
                UU_WLAN_LOG_DEBUG(("UURMIDRIVER ..........Unknown AMPDU action\n"));
            }
            break;
    }
    return 0;
} /* uu_wlan_ampdu_action */


/** This function is registered as set_tim call back of mac80211 framework.
 * This function is called from mac80211, when a TIM bit must be set or cleared for a given STA.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] sta Contains the deatains of sta.
 * @param[in] set is the boolean value which indicated to set or clear the tim bit.
 */
static uu_int32 uu_wlan_set_tim(struct ieee80211_hw *hw, struct ieee80211_sta *sta,
        uu_bool set)
{
    UU_WLAN_LOG_DEBUG(("UURMIDRIVER set_tim \n"));
    return 0;
}


/** This function is registered as get_stats call back of mac80211 framework.
 * This function is called from mac80211, to get the low level driver statistcs.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] stats Contains the statistics.
 */
static uu_int32 uu_wlan_get_stats(struct ieee80211_hw *hw,
        struct ieee80211_low_level_stats *stats)
{
    /* Force update */
    UU_WLAN_LOG_DEBUG(("Inside uu_wlan_get_stats call back \n"));

    /** TODO Need to update this by reading registers or from LMAC */
    UU_WLAN_LOG_DEBUG(("Stats are ackfailed %d, rtsfailed %d, rtssucc %d, FCSerror %d\n", stats->dot11ACKFailureCount,
                stats->dot11RTSFailureCount, uu_getReg(UU_REG_DOT11_RTS_SUCCESS_COUNT), uu_getReg(UU_REG_DOT11_FCS_ERROR_COUNT)));

    uu_wlan_context_gp->drv_stats.ack_fail += uu_getReg(UU_REG_DOT11_ACK_FAILURE_COUNT);
    uu_wlan_context_gp->drv_stats.rts_fail += uu_getReg(UU_REG_DOT11_RTS_FAILURE_COUNT);
    uu_wlan_context_gp->drv_stats.rts_ok += uu_getReg(UU_REG_DOT11_RTS_SUCCESS_COUNT);
    uu_wlan_context_gp->drv_stats.fcs_error += uu_getReg(UU_REG_DOT11_FCS_ERROR_COUNT);
    stats->dot11ACKFailureCount = uu_wlan_context_gp->drv_stats.ack_fail;
    stats->dot11RTSFailureCount = uu_wlan_context_gp->drv_stats.rts_fail;
    stats->dot11RTSSuccessCount = uu_wlan_context_gp->drv_stats.rts_ok;
    stats->dot11FCSErrorCount = uu_wlan_context_gp->drv_stats.fcs_error;

    return 0;
} /* uu_wlan_get_stats */

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,8,0)
#else
/** Start operation on the AP interface, this is called after all the
 *  information in bss_conf is set and beacon can be retrieved.
 * This callback is added from the linux kernel 10.3 onwards.
 * For the lower versions of kernel driver can start beacon trigger in bss_change_info callback.
 * Trigger beacon from this callback.
 */
static uu_int32 uu_wlan_start_ap(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
    UU_WLAN_LOG_DEBUG(("START AP \n"));

    if(uu_wlan_context_gp->hw)
        ieee80211_queue_work(uu_wlan_context_gp->hw, &uu_wlan_context_gp->beacon_update_trigger);
    return 0;
}
#endif

#ifdef UU_WLAN_IBSS
static uu_int32 uu_wlan_tx_last_beacon (struct ieee80211_hw *hw)
{
    return uu_wlan_last_beacon_tx_r;
}
#endif

#ifdef UU_WLAN_TSF
static void uu_wlan_reset_tsf (struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
    uu_wlan_tsf_r = 0;
}
#endif

/** mac80211 callbacks - list of callbacks from mac80211 to the driver
 *
 * This tructure contains various callbacks that the UMAC is handled.
 */

struct ieee80211_ops uu_wlan_ops = {
    .tx                 = uu_wlan_tx,
    .start              = uu_wlan_start,
    .stop               = uu_wlan_stop,
    .add_interface      = uu_wlan_add_interface,
    .remove_interface   = uu_wlan_remove_interface,
    .config             = uu_wlan_config,
    .configure_filter   = uu_wlan_configure_filter,
    .conf_tx            = uu_wlan_conf_tx,
    .bss_info_changed   = uu_wlan_bss_change_info,
    .sta_add            = uu_wlan_add_station,
    .sta_remove         = uu_wlan_del_station,
    .set_tim            = uu_wlan_set_tim,
    .get_stats          = uu_wlan_get_stats,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0)
    .start_ap           = uu_wlan_start_ap,
#endif
#ifdef UU_WLAN_TSF
    .reset_tsf          = uu_wlan_reset_tsf,
#endif
#ifdef UU_WLAN_IBSS
    .tx_last_beacon     = uu_wlan_tx_last_beacon,
#endif
    .ampdu_action       = uu_wlan_ampdu_action //TODO: For MESH, there is some problem in this.
}; /* uu_wlan_ops  */


/* EOF */

