/*******************************************************************************
 **                                                                            **
 ** File name :  uu_wlan_platform.c                                            **
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
/* $Revision: 1.3 $ */

/** 
 * @uu_wlan_platform.c
 * Contains all the platform dependent fuctionality.
 */

#include "uu_wlan_main.h"
#include "uu_mac_drv.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_buf_desc.h"
#include "uu_wlan_frame.h"
#include "uu_mac_txstatus.h"
#include "uu_wlan_recv.h"
#include "uu_wlan_init.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_reg_init.h"
#include "uu_wlan_reg_platform.h"


uu_void uu_wlan_tx_write_buffer_platform(uu_uchar ac, uu_uchar *adr[], uu_uint32 len[])
{
    uu_wlan_tx_write_buffer(ac, adr, len);
}


uu_uint32 uu_wlan_tx_get_available_buffer_platform(uu_uint8 ac)
{
    return uu_wlan_tx_get_available_buffer(ac);
}


#if 0
uu_void uu_wlan_fill_rx_status(uu_wlan_rx_frame_info_t *recv_fi, uu_wlan_rx_status_t *rs)
{
    memset(rs, 0, sizeof (uu_wlan_rx_status_t));
#if 0 /* TODO: Get these from LMAC / PHY */
    rs->datalen = UU_REG_RX_DATALEN;
    rs->status = UU_REG_RX_STATUS;
    rs->phyerr = UU_REG_RX_PHYERR;
    rs->rssi = recv_fi->rxvec.rssi;
    rs->keyix = recv_fi->frameInfo.keyix;
    rs->rate = UU_REG_RX_RATE;
    rs->antenna = UU_REG_RX_ANTENNA;
    rs->more = UU_REG_RX_MORE;
    rs->rssi_ctl0 = UU_REG_RX_RSSI_CT10;
    rs->rssi_ctl1 = UU_REG_RX_RSSI_CT11;
    rs->rssi_ctl2 = UU_REG_RX_RSSI_CT12;
    rs->rssi_ext0 = UU_REG_RX_RSSI_EXT0;
    rs->rssi_ext1 = UU_REG_RX_RX_RSSI_EXT1;
    rs->rssi_ext2 = UU_REG_RX_RX_RSSI_EXT2;
    rs->isaggr = UU_REG_RX_ISAGGR;
    rs->moreaggr = UU_REG_RX_MOREAGGR;
    rs->num_delims = UU_REG_RX_DELIMS;
    rs->flags = UU_REG_RX_FLAGS; //TODO fill the status registers from lmac 
    //UU_WLAN_LOG_DEBUG(("UURMIDRIVER:rx status in UMAC rs->datalen %x,rs->rate %x, rs->isaggr %x\n",rs->datalen, rs->rate, rs->isaggr));
#endif
} /* uu_wlan_fill_rx_status */


uu_int32 uu_wlan_rx_setfilters_platform(uu_uint32 total_flags)
{
    uu_int32 rx_filter = 0;
    uu_int32 filters = 0;
    if(total_flags & FIF_CONTROL)
    {
        filters = UU_WLAN_FILTR_CTL_FRAMES ;
        rx_filter |= FIF_CONTROL;
    }
    if (total_flags & FIF_PROMISC_IN_BSS)
    {
        rx_filter |= FIF_PROMISC_IN_BSS;
    }
    if (total_flags & FIF_ALLMULTI)
    {
        rx_filter |= FIF_ALLMULTI;
    }
#if 0  /** Enable this after adding the support in both LMAC and UMAC */
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_BEACON ;
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_PROBE_RESP ;
    if(!(total_flags & FIF_PSPOLL))
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_PSPOLL ;
    if(!(total_flags & FIF_PROBE_REQ))
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_PROBE_REQ ;
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_ALLOW_RTS_CTS ;
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_BROAD_CAST ;
    if(total_flags & FIF_BCN_PRBRESP_PROMISC)
        UU_REG_LMAC_FILTER_FLAG |= UU_WLAN_FILTR_DROP_ALL ;
    UU_REG_LMAC_FILTER_FLAG = UU_WLAN_FILTR_CTL_FRAMES | UU_WLAN_FILTR_PROBE_RESP | UU_WLAN_FILTR_PSPOLL | UU_WLAN_FILTR_PROBE_REQ | UU_WLAN_FILTR_ALLOW_RTS_CTS | UU_WLAN_FILTR_BROAD_CAST;
#endif

    UU_REG_LMAC_FILTER_FLAG = UU_WLAN_FILTR_NO_FILTER;
    return rx_filter;

} /* uu_wlan_rx_setfilters_platform */
#endif /* #if 0 */


uu_void uu_wlan_clear_rx_status_flag_platform(uu_uint8 index)
{
    uu_wlan_rx_status_flags_g &= ~(1 << index);
}

uu_uint32 uu_wlan_get_rx_status_platform(uu_uint8 index)
{
    return (uu_wlan_rx_status_flags_g & (1 << index));
}

uu_void uu_wlan_get_rx_status_info_platform(uu_wlan_rx_status_info_t *rsi, uu_uint8 index)
{
    rsi->offset = uu_wlan_rx_status_info_g[index].offset;
    rsi->len  = uu_wlan_rx_status_info_g[index].len;
}

uu_void uu_wlan_get_data_platform(uu_char *dest ,uu_uchar *src ,uu_uint32 len)
{
    memcpy(dest, src, len);
}


uu_void uu_wlan_tx_status_info_platform(uu_uint32 i,uu_uint32 j)
{
    // No action, as the 'uu_wlan_tx_status_info_g' is accessible directly in 'C' model.
    // In RTL model, need to read them from hardware registers into local memory.
}


uu_uint32 uu_wlan_tx_get_status_seq_no_platform(uu_uint32 i, uu_uint32 j)
{
    return uu_wlan_tx_status_info_g[i][j].seqno;
}


uu_uint32 uu_wlan_tx_get_status_platform(uu_uint8 qid, uu_uint8 index)
{
    return  (uu_wlan_tx_status_flags_g[qid] & (1 << index));
}

uu_void uu_wlan_tx_reset_status_flags_platform(uu_uint32 qid, uu_uint32 index)
{
    uu_wlan_tx_status_flags_g[qid] &= ~(1 << index);
}


/** UMAC call back function (simulation of Interrupt Service Routine) */
uu_void uu_wlan_umac_cbk(uu_int8 ind)
{
    uu_int32 err;

    switch(ind)
    {
        case UU_LMAC_IND_RX_FRAME:
            err = uu_wlan_rx_tasklet(uu_wlan_context_gp->hw);
            break;

        case UU_LMAC_IND_GEN_BEACON:
            {
                if(uu_wlan_context_gp->hw)
                    ieee80211_queue_work(uu_wlan_context_gp->hw, &uu_wlan_context_gp->beacon_update_trigger);
            }
            break;

        case UU_LMAC_IND_TX_STATUS:
            {
                uu_mac_drv_get_ac_txstatus();
            }
            break;
        default:
            break;
    }

} /* uu_wlan_umac_cbk */


#if 0 /* Currently unused */
uu_int32 uu_wlan_add_station_platform(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif,
        struct ieee80211_sta *sta)
{
    /* TODO: Properly add the station into LMAC */
    uu_wlan_lmac_config_t cmd_info;

    /* Add to aid mac address table */

    cmd_info.cmd_type = UU_WLAN_LMAC_CONFIG_ASSO_ADD;

    /** Filling of station info */
    cmd_info.cmd_data.asso_add.aid = sta->aid;
    cmd_info.cmd_data.asso_add.ch_bndwdth = CBW40;

    memcpy(cmd_info.cmd_data.asso_add.addr, sta->addr, IEEE80211_MAC_ADDR_LEN);
    /* TODO: Fill the remaining part of the structure */
    //uu_wlan_lmac_config(&cmd_info);
    /** Filling associated station information */

    return 0;
} /* uu_wlan_add_station_platform*/


uu_int32 uu_wlan_del_station_platform(struct ieee80211_hw *hw,
        struct ieee80211_vif *vif,
        struct ieee80211_sta *sta)
{
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
} /* uu_wlan_del_station_platform */
#endif /* Unused */


/* Sends the addba command to LMAC, LMAC adds the BA information to BA table */
uu_void uu_wlan_umac_send_addba_info_platform(uu_uint8 tid, uu_char* addr, uu_uint16 bufsz, uu_uint16 ssn, uu_uint8 type, uu_bool dir)
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

    /*TODO move to sim model*/
} /* uu_wlan_umac_send_addba_info_platform */


uu_void uu_wlan_umac_send_delba_info_platform(uu_uint8 tid, uu_char* addr, uu_uint8 dir)
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

} /* uu_wlan_umac_send_delba_info_platform */


uu_void uu_wlan_change_defaults(uu_void)
{
    dot11_slot_timer_value = UU_WLAN_SLOT_TIMER_DEFAULT_VALUE_CMODEL;
    dot11_difs_value =  2 * dot11_slot_timer_value;
    dot11_cwmax_value = UU_WLAN_CWMAX_DEFAULT_VALUE_CMODEL;
    dot11_cwmax_value_ac[0] = UU_WLAN_CWMAX_DEFAULT_VALUE_CMODEL;
    dot11_cwmax_value_ac[1] = UU_WLAN_CWMAX_DEFAULT_VALUE_CMODEL;
}

