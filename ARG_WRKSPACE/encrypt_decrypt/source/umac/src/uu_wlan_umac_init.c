/*******************************************************************************
**                                                                            **
** File name :  uu_wlan_umac_init.c                                           **
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
/* $Revision: 1.23 $ */

#include <linux/err.h>

#include "uu_datatypes.h"
#include "uu_wlan_buf.h"
#include "uu_wlan_fwk_log.h"
#include "uu_wlan_mac_defaults.h"
#include "uu_wlan_reg.h"
#include "uu_wlan_init.h"
#include "uu_wlan_umac_init.h"
#include "uu_wlan_eeprom.h"
#ifdef UU_WLAN_TPC
#include "uu_wlan_umac_tpc.h"
#endif


/* TODO: Fix the todo comment related to 'q_state' */
/* TODO: Compare with txq structure
         Move to header file */  
struct qstate q_state =
{
    .num_pkts = {0},
    .q_stopped = {0},
    .max_pkts = {4,4,4,4}
};


#ifdef UU_WLAN_DFS
/** Supported channel list fo legacy 2GHz */
static const struct ieee80211_channel uu_wlan_chan_2ghz_g[] = 
{
    CHAN2G(2412, 1), /* Channel 1 */
    CHAN2G(2417, 2), /* Channel 2 */
    CHAN2G(2422, 3), /* Channel 3 */
    CHAN2G(2427, 4), /* Channel 4 */
    CHAN2G(2432, 5), /* Channel 5 */
    CHAN2G(2437, 6), /* Channel 6 */
    CHAN2G(2442, 7), /* Channel 7 */
    CHAN2G(2447, 8), /* Channel 8 */
    CHAN2G(2452, 9), /* Channel 9 */
    CHAN2G(2457, 10), /* Channel 10 */
    CHAN2G(2462, 11), /* Channel 11 */
    CHAN2G(2467, 12), /* Channel 12 */
    CHAN2G(2472, 13), /* Channel 13 */
    CHAN2G(2484, 14), /* Channel 14 */
};

/* The hx_value for the channels are kept same as the channel number */
static const struct ieee80211_channel uu_wlan_chan_5ghz_g[] = 
{
    /* _We_ call this UNII 1 */
    CHAN5G(5180, 36), /* Channel 36 */
    CHAN5G(5200, 40), /* Channel 40 */
    CHAN5G(5220, 44), /* Channel 44 */
    CHAN5G(5240, 48), /* Channel 48 */
    /* _We_ call this UNII 2 */
    CHAN5G(5260, 52), /* Channel 52 */
    CHAN5G(5280, 56), /* Channel 56 */
    CHAN5G(5300, 60), /* Channel 60 */
    CHAN5G(5320, 64), /* Channel 64 */
    /* _We_ call this "Middle band" */
    CHAN5G(5500, 100), /* Channel 100 */
    CHAN5G(5520, 104), /* Channel 104 */
    CHAN5G(5540, 108), /* Channel 108 */
    CHAN5G(5560, 112), /* Channel 112 */
    CHAN5G(5580, 116), /* Channel 116 */
    CHAN5G(5600, 120), /* Channel 120 */
    CHAN5G(5620, 124), /* Channel 124 */
    CHAN5G(5640, 128), /* Channel 128 */
    CHAN5G(5660, 132), /* Channel 132 */
    CHAN5G(5680, 136), /* Channel 136 */
    CHAN5G(5700, 140), /* Channel 140 */
    /* _We_ call this UNII 3 */
    CHAN5G(5745, 149), /* Channel 149 */
    CHAN5G(5765, 153), /* Channel 153 */
    CHAN5G(5785, 157), /* Channel 157 */
    CHAN5G(5805, 161), /* Channel 161 */
    CHAN5G(5825, 165), /* Channel 165 */
};
#else /* Not UU_WLAN_DFS */
static const struct ieee80211_channel uu_wlan_chan_2ghz_g[] = 
{
    CHAN2G(2412, 0), /* Channel 1 */
    CHAN2G(2417, 1), /* Channel 2 */
    CHAN2G(2422, 2), /* Channel 3 */
    CHAN2G(2427, 3), /* Channel 4 */
    CHAN2G(2432, 4), /* Channel 5 */
    CHAN2G(2437, 5), /* Channel 6 */
    CHAN2G(2442, 6), /* Channel 7 */
    CHAN2G(2447, 7), /* Channel 8 */
    CHAN2G(2452, 8), /* Channel 9 */
    CHAN2G(2457, 9), /* Channel 10 */
    CHAN2G(2462, 10), /* Channel 11 */
    CHAN2G(2467, 11), /* Channel 12 */
    CHAN2G(2472, 12), /* Channel 13 */
    CHAN2G(2484, 13), /* Channel 14 */
};

/** Supported channel list for 5GHz */
static const struct ieee80211_channel uu_wlan_chan_5ghz_g[] = 
{
    /* _We_ call this UNII 1 */
    CHAN5G(5180, 14), /* Channel 36 */
    CHAN5G(5200, 15), /* Channel 40 */
    CHAN5G(5220, 16), /* Channel 44 */
    CHAN5G(5240, 17), /* Channel 48 */
    /* _We_ call this UNII 2 */
    CHAN5G(5260, 18), /* Channel 52 */
    CHAN5G(5280, 19), /* Channel 56 */
    CHAN5G(5300, 20), /* Channel 60 */
    CHAN5G(5320, 21), /* Channel 64 */
    /* _We_ call this "Middle band" */
    CHAN5G(5500, 22), /* Channel 100 */
    CHAN5G(5520, 23), /* Channel 104 */
    CHAN5G(5540, 24), /* Channel 108 */
    CHAN5G(5560, 25), /* Channel 112 */
    CHAN5G(5580, 26), /* Channel 116 */
    CHAN5G(5600, 27), /* Channel 120 */
    CHAN5G(5620, 28), /* Channel 124 */
    CHAN5G(5640, 29), /* Channel 128 */
    CHAN5G(5660, 30), /* Channel 132 */
    CHAN5G(5680, 31), /* Channel 136 */
    CHAN5G(5700, 32), /* Channel 140 */
    /* _We_ call this UNII 3 */
    CHAN5G(5745, 33), /* Channel 149 */
    CHAN5G(5765, 34), /* Channel 153 */
    CHAN5G(5785, 35), /* Channel 157 */
    CHAN5G(5805, 36), /* Channel 161 */
    CHAN5G(5825, 37), /* Channel 165 */
};
#endif /* UU_WLAN_DFS */


/* Flag 0x80 is proprietary implementation. It indicates CCK */
/* MSB in CCK is set to 1 because 1 Mbps and 12 Mbps had same representation */

/* For CCK refer to 17.2.3.4 802.11-REVmbD12*/
/* The data rate shall be equal to the SIGNAL field value multiplied by 100 kbit/s. */
/* a) X’0A’ (MSB to LSB) for 1 Mb/s;
   b) X’14’ (MSB to LSB) for 2 Mb/s;
   c) X’37’ (MSB to LSB) for 5.5 Mb/s;
   d) X’6E’ (MSB to LSB) for 11 Mb/s.
   We won't go this way and we will define it for 4 bits that are not used in OFDM  
 */

/* For OFDM refer to 18.3.4.2 802.11-REVmbD12 */
/* R4-R1 : 1011 for 6 Mbps */
/* R4-R1 : 1111 for 9 Mbps */
/* R4-R1 : 1010 for 12 Mbps */
/* R4-R1 : 1110 for 18 Mbps */
/* R4-R1 : 1001 for 24 Mbps */
/* R4-R1 : 1101 for 36 Mbps */
/* R4-R1 : 1000 for 48 Mbps */
/* R4-R1 : 1100 for 54 Mbps */

/** Since the hw_values of 1M and 12M is same, and L_datarate for CCK can
 * go upto 7 bits, so we are making CCK hw_value ranging between 4-7 for
 * our implementation */
/* TODO: Name & usage should be reviewed */
static struct ieee80211_rate uu_wlan_rates_g[] = 
{
    RATE(10, 0x04),     /* CCK 1 Mbps */
    RATE(20, 0x05),     /* CCK 2 Mbps */
    RATE(55, 0x06),     /* CCK 5.5 Mbps */
    RATE(110, 0x07),    /* CCK 11 Mbps */
    RATE(60, 0x0b),     /* OFDM 6 Mbps */
    RATE(90, 0x0f),     /* OFDM 9 Mbps */
    RATE(120, 0x0a),    /* OFDM 12 Mbps */
    RATE(180, 0x0e),    /* OFDM 18 Mbps */
    RATE(240, 0x09),    /* OFDM 24 Mbps */
    RATE(360, 0x0d),    /* OFDM 36 Mbps */
    RATE(480, 0x08),    /* OFDM 48 Mbps */
    RATE(540, 0x0c),    /* OFDM 54 Mbps */
};

static struct device_driver uu_wlan_driver_g = 
{
    .name = "uurmi-wlan-driver"
};

uu_wlan_umac_context_t *uu_wlan_context_gp;

static struct class *uu_wlan_class_g;
static spinlock_t uu_wlan_radio_lock_g;
static struct list_head uu_wlan_radios_g;
static struct net_device *uu_wlan_net_dev_gp;

static uu_int32 uu_set_hw_cap(uu_wlan_umac_context_t *mac_context, struct ieee80211_hw *hw);
static uu_void uu_wlan_free(uu_void);


static netdev_tx_t uurmi_dev_xmit(struct sk_buff *skb,
        struct net_device *dev)
{
    dev_kfree_skb(skb);
    return NETDEV_TX_OK;
}


static const struct net_device_ops uurmi_netdev_ops = {
    .ndo_start_xmit     = uurmi_dev_xmit,
    .ndo_change_mtu     = eth_change_mtu,
    .ndo_set_mac_address    = eth_mac_addr,
    .ndo_validate_addr      = eth_validate_addr,
};


static uu_void uurmi_wlan_dev_setup(struct net_device *dev)
{
    dev->netdev_ops = &uurmi_netdev_ops;
    dev->destructor = free_netdev;
    ether_setup(dev);
    dev->tx_queue_len = 0;
    dev->type = IEEE80211_RADIOTAP;
    memset(dev->dev_addr, 0, ETH_ALEN);
    dev->dev_addr[0] = 0x12;
} /* uurmi_wlan_dev_setup */

/** Intialization of Queues.
 */
static void uu_wlan_acq_init(uu_wlan_umac_context_t *mac_context, 
        uu_int32 qtype, 
        uu_int32 ac_type)
{
    uu_uint8 i;
    for(i=0; i<UU_WLAN_MAX_ASSOCIATIONS; i++)
    {
        mac_context->tx_q[ac_type][i].qid = ac_type;
        mac_context->tx_q[ac_type][i].q_ampdu_depth = UU_WLAN_QUEUE_AMPDU_DEPTH;
        mac_context->tx_q[ac_type][i].stopped = 0;
        mac_context->tx_q[ac_type][i].txq_len = 0;
        mac_context->tx_q[ac_type][i].txq_max = UU_WLAN_TX_QUEUE_MAX;
        INIT_LIST_HEAD(&mac_context->tx_q[ac_type][i].acq_head);   
        INIT_LIST_HEAD(&uu_wlan_context_gp->tx_q[ac_type][i].acq_status_head);   
    }
}


static uu_int32 uu_wlan_init_queues(uu_wlan_umac_context_t *mac_context)
{
    uu_int32 i;

    if(mac_context == NULL)
        return -1;

    if(uu_wlan_context_gp->hw->queues == 4)
    {
        for (i = 0; i < WME_NUM_AC; i++)
        {
            uu_wlan_acq_init(mac_context, UU_WLAN_TXQ_DATA, i);
        }
    }
    else if(uu_wlan_context_gp->hw->queues == 1)
    {
        uu_wlan_acq_init(mac_context, UU_WLAN_TXQ_DATA, 0);
    }
    else
    {
        UU_WLAN_LOG_ERROR(("************NOT EXPECTED HERE. Invalid num of Qs: %d*********\n", uu_wlan_context_gp->hw->queues));
    }

#ifdef UU_WLAN_BQID
    uu_wlan_acq_init (mac_context, UU_WLAN_TXQ_DATA, UU_WLAN_BEACON_QID);
#endif

    return 0;
} /* uu_wlan_init_queues */


/** Allocate new hw device and register that with mac80211.
 * Register the callback with mac framework. 
 * Fill the context structure. Read mac address from EEPROM and set the 
 * permanent mac address for 80211 hardware.
 * Set the hardware capabilities. 
 * All intialization should done here. 
 */
uu_int32 uu_wlan_umac_init(uu_void)
{
    uu_wlan_umac_context_t *mac_context;
    struct ieee80211_hw *hw;
    uu_int32 ret = 0, err;
    unsigned char macaddr[UU_WLAN_MAC_ADDR_LEN];

    uu_wlan_hw_t *uhw = NULL;

    spin_lock_init(&uu_wlan_radio_lock_g);
    INIT_LIST_HEAD(&uu_wlan_radios_g);

    uu_wlan_class_g = class_create(THIS_MODULE, "uurmi-wlan-driver");
    if (IS_ERR(uu_wlan_class_g))
    {
        return PTR_ERR(uu_wlan_class_g);
    }

    hw = ieee80211_alloc_hw(sizeof(uu_wlan_umac_context_t), &uu_wlan_ops); 

    if (hw == NULL) 
    {
        UU_WLAN_LOG_ERROR(("No memory for ieee80211_hw\n"));
        ret = -ENOMEM;
    }

    mac_context = hw->priv;
    mac_context->hw = hw;

    mac_context->dev = device_create(uu_wlan_class_g, NULL, 0, hw,
            "uurmi-wlan-driver%d", 1);

    if (IS_ERR(mac_context->dev)) 
    {
        UU_WLAN_LOG_ERROR(("uuwlan: device_create failed (%ld)\n", 
                    PTR_ERR(mac_context->dev)));
        err = -ENOMEM;
        goto failed_drvdata;
    }

    UU_WLAN_LOG_DEBUG(("uuwlan: device_create successfully \n"));
    mac_context->dev->driver = &uu_wlan_driver_g;

    SET_IEEE80211_DEV(hw, mac_context->dev);

    uu_wlan_get_macaddr_from_eeprom(macaddr); /* TODO eeprom */

    /** Setting mac address */
    SET_IEEE80211_PERM_ADDR(hw, macaddr); /* TODO add */ 

    /** Initializing hardware */
    uhw = kzalloc(sizeof(uu_wlan_hw_t), GFP_KERNEL);
    if (!uhw)
        return -ENOMEM;

    uhw->hw = mac_context->hw;
    mac_context->mac_context_hw = uhw;

    uu_wlan_context_gp = mac_context;

    /** To set the hardware capabilities like channels and bands call uu_set_hw_cap function */
    uu_set_hw_cap(mac_context, hw); 

    err = uu_wlan_init_queues(mac_context);
    if(err)
        goto failed;

    err = ieee80211_register_hw(hw);
    if (err)
        goto error_register;

    //spin_lock_init(&mac_context->nodes_lock);
    list_add_tail(&mac_context->nodes, &uu_wlan_radios_g);

    uu_wlan_net_dev_gp = alloc_netdev(0, "uurmi_wlan", uurmi_wlan_dev_setup);
    if (uu_wlan_net_dev_gp == NULL)
        goto failed;

    err = register_netdev(uu_wlan_net_dev_gp);
    UU_WLAN_LOG_ERROR(("UUDRIVER : err value after register_netdev is %d\n",err));

    if (err < 0)
        goto failed_mon;

    return 0;

failed_mon:
    free_netdev(uu_wlan_net_dev_gp);
failed:
    uu_wlan_free();
error_register:
    uu_wlan_free();
    class_destroy(uu_wlan_class_g);
    //err_tx:
failed_drvdata:
    ieee80211_free_hw(hw);
    return err;
    /* Nothing */;
} /* uu_wlan_umac_init */


/** Setting hardware capabilities. 
 * Set the following
 *      Hardware supported flags.
 *      Supported interfaces.
 *      Supported bands and channels.
 *      Maximum thesholds and intervals.
 */
static uu_int32 uu_set_hw_cap(uu_wlan_umac_context_t *mac_context, struct ieee80211_hw *hw)
{
    uu_void *channels_2ghz;
    uu_void *channels_5ghz;
    enum ieee80211_band band;

    UU_WLAN_LOG_DEBUG(("UURMIDRIVER: inside uurmi_set_hw_cap \n"));
    hw->channel_change_time = 1;

    /* flags desctiption
     *
     * IEEE80211_HW_RX_INCLUDES_FCS
     *         Indicates that received frames passed to the stack include the FCS at the end.
     *
     * IEEE80211_HW_SIGNAL_DBM:
     *         Hardware gives signal values in dBm, decibel difference from
     *         one milliwatt. This is the preferred method since it is standardized
     *         between different devices. @max_signal does not need to be set.

     * IEEE80211_HW_SUPPORTS_PS:
     *         Hardware has power save support (i.e. can go to sleep).
     *
     * IEEE80211_HW_PS_NULLFUNC_STACK:
     *        Hardware requires nullfunc frame handling in stack, implies
     *        stack support for dynamic PS.
     *
     * IEEE80211_HW_SPECTRUM_MGMT:
     *        Hardware supports spectrum management defined in 802.11h
     *        Measurement, Channel Switch, Quieting, TPC
     *
     * IEEE80211_HW_REPORTS_TX_ACK_STATUS:
     *        Hardware can provide ack status reports of Tx frames to
     *        the stack.
     *
     * IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING: 
     *      Some wireless LAN chipsets buffer broadcast/multicast frames 
     *      for power saving stations in the hardware/firmware and others
     *          rely on the host system for such buffering. This option is used
     *      to configure the IEEE 802.11 upper layer to buffer broadcast and
     *      multicast frames when there are power saving stations so that
     *      the driver can fetch them with ieee80211_get_buffered_bc().
     */
    hw->flags = IEEE80211_HW_RX_INCLUDES_FCS |
        IEEE80211_HW_HOST_BROADCAST_PS_BUFFERING | 
        IEEE80211_HW_SIGNAL_DBM |
        IEEE80211_HW_SUPPORTS_PS |
        IEEE80211_HW_SUPPORTS_UAPSD |
        IEEE80211_HW_SUPPORTS_DYNAMIC_PS |
       // IEEE80211_HW_PS_NULLFUNC_STACK |
        IEEE80211_HW_AMPDU_AGGREGATION |
        IEEE80211_HW_SPECTRUM_MGMT;// |
        //IEEE80211_HW_REPORTS_TX_ACK_STATUS;

    UU_WLAN_LOG_DEBUG(("UURMIDRIVER: after setting flags \n"));

    hw->wiphy->interface_modes =
        BIT(NL80211_IFTYPE_P2P_GO) |
        BIT(NL80211_IFTYPE_P2P_CLIENT) |
        BIT(NL80211_IFTYPE_AP) |
        BIT(NL80211_IFTYPE_WDS) |
        BIT(NL80211_IFTYPE_STATION) |
        BIT(NL80211_IFTYPE_ADHOC) |
        BIT(NL80211_IFTYPE_MESH_POINT);
    UU_WLAN_LOG_DEBUG(("UURMIDRIVER: after setting wiphy interface mode \n"));

    hw->wiphy->flags = IEEE80211_HW_MFP_CAPABLE |
        IEEE80211_HW_SIGNAL_DBM |
        IEEE80211_HW_SUPPORTS_STATIC_SMPS |
        IEEE80211_HW_SUPPORTS_DYNAMIC_SMPS;

    hw->wiphy->frag_threshold = UU_WLAN_FRAG_THRESHOLD; 
    hw->queues = UU_WLAN_QOS_QUEUES;

    /* Considering 2 rates to be received from the MAC framework */
    /* Primary rate and Fallback rate */
    hw->max_rates = UU_WLAN_NUM_RATES_RECVED;

#ifdef UU_WLAN_UMAC_RCAC
    /* This is not yet present in framework, and the name may change as per the
       framework support for the rate control */
    hw->rate_control_algorithm = "minstrel_vht";
#else
    /* This is present in the framework as the rate control algorithm */
    hw->rate_control_algorithm = "minstrel_ht";
#endif
    hw->channel_change_time = UU_WLAN_CHANNEL_CHANGE_TIME;
    hw->max_listen_interval = UU_WLAN_LISTEN_INTERVAL;
    hw->max_rate_tries = UU_WLAN_MAX_RATE_TRIES;
    UU_WLAN_LOG_DEBUG(("LAXMAN: frag_threshold %d\n", hw->wiphy->frag_threshold));

    UU_WLAN_LOG_DEBUG(("UURMIDRIVER: Before uu_wlan_chan_2ghz_g memcpy \n"));

    /* Setting channels */
    channels_2ghz = kmemdup(uu_wlan_chan_2ghz_g,
            sizeof(uu_wlan_chan_2ghz_g), GFP_KERNEL);
    if (!channels_2ghz)
        return -ENOMEM;

    channels_5ghz = kmemdup(uu_wlan_chan_5ghz_g,
            sizeof(uu_wlan_chan_5ghz_g), GFP_KERNEL);
    if (!channels_5ghz)
        return -ENOMEM;

#ifdef UU_WLAN_TPC
    dot11NumberSupportedPowerLevelsImplemented_g = 8;
    dot11TxPowerLevelExtended = 128;
#endif
#ifdef UU_WLAN_DFS
    msrment_stats_g = 255;
#endif

    /* Setting bands */
    for (band = IEEE80211_BAND_2GHZ; band < IEEE80211_NUM_BANDS; band++) 
    {
        struct ieee80211_supported_band *sband = &mac_context->sbands[band];
        switch (band) 
        {
            case IEEE80211_BAND_2GHZ:
                sband->channels = channels_2ghz;
                sband->n_channels = ARRAY_SIZE(uu_wlan_chan_2ghz_g);
                sband->bitrates = uu_wlan_rates_g;
                sband->n_bitrates = ARRAY_SIZE(uu_wlan_rates_g);
                sband->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | 
                    IEEE80211_HT_CAP_SM_PS | 
                    IEEE80211_HT_CAP_SGI_20 |
                    IEEE80211_HT_CAP_SGI_40 | 
                    IEEE80211_HT_CAP_DSSSCCK40 | 
                    IEEE80211_HT_CAP_TX_STBC |
                    (IEEE80211_HT_CAP_RX_STBC & 0x0100) /* Enabling stbc support for 1 Nss only in ht */ ; 
                break;
            case IEEE80211_BAND_5GHZ:
                sband->channels = channels_5ghz;
                sband->n_channels = ARRAY_SIZE(uu_wlan_chan_5ghz_g);
                sband->bitrates = uu_wlan_rates_g + 4;
                sband->n_bitrates = ARRAY_SIZE(uu_wlan_rates_g) - 4;
                /* IEEE80211_HT_CAP_DSSSCCK40 is not needed in 5 GHz case */
                sband->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40 | 
                    IEEE80211_HT_CAP_SM_PS | 
                    IEEE80211_HT_CAP_SGI_20 |
                    IEEE80211_HT_CAP_SGI_40 | 
                    IEEE80211_HT_CAP_TX_STBC |
                    (IEEE80211_HT_CAP_RX_STBC & 0x0100) /* Enabling stbc support for 1 Nss only in ht */ ;
#if 0 /** Enable this for VHT */
                /** This will be enabled while supporting VHT. 
                 * Enabling this code cause compilation errors as there is no support for VHT in mac80211 framework.
                 * All the macros used for VHT support are not yet defined in mac80211 framework.
                 */

                sband->vht_cap.vht_supported = true;
                sband->vht_cap.cap = IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 |
                    IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ |
                    IEEE80211_VHT_CAP_RXLDPC |
                    IEEE80211_VHT_CAP_SHORT_GI_80 |
                    IEEE80211_VHT_CAP_SHORT_GI_160 |
                    IEEE80211_VHT_CAP_TXSTBC |
                    IEEE80211_VHT_CAP_RXSTBC_1 | /* Presently we are supporting only 1 */
                    IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;
                sband->vht_cap.vht_mcs.rx_mcs_map =
                    cpu_to_le16(IEEE80211_VHT_MCS_SUPPORT_0_8 << 0 |
                            IEEE80211_VHT_MCS_SUPPORT_0_8 << 2 |
                            IEEE80211_VHT_MCS_SUPPORT_0_8 << 4 | /* For demo we are not supporting this */
                            IEEE80211_VHT_MCS_SUPPORT_0_8 << 6); /* For demo we are not supporting this */ /* Since we are supporting till MCS index 0-7 and 4 spatial streams */
                sband->vht_cap.vht_mcs.tx_mcs_map = sband->vht_cap.vht_mcs.rx_mcs_map;
#endif
                break;
            default:
                continue;
        }

        sband->ht_cap.ht_supported = true;
        sband->ht_cap.ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K;
        sband->ht_cap.ampdu_density = IEEE80211_HT_MPDU_DENSITY_8;
        memset(&sband->ht_cap.mcs, 0, sizeof(sband->ht_cap.mcs));
        sband->ht_cap.mcs.rx_mask[0] = 0xff;
        sband->ht_cap.mcs.rx_mask[1] = 0xff;
#if 0
        /* This will be enabled for supporting 3 or 4 streams */
        /* We will support till MCS 31 so till 31 is enabled for rx_mask */
        /*Currently we are supporting only till 2 Nss */
        /* TODO : enable this when we support for 3 Nss or 4 Nss */
        sband->ht_cap.mcs.rx_mask[2] = 0xff; /* for 3 Nss */
        sband->ht_cap.mcs.rx_mask[3] = 0xff; /* for 4 Nss */
#endif
        sband->ht_cap.mcs.tx_params = IEEE80211_HT_MCS_TX_DEFINED;
        hw->wiphy->bands[band] = sband;
    }
    mac_context->hw = hw;
    return 0;
} /* uu_set_hw_cap */


/** Free the wlan driver.
 * Unregister hardware with mac framework.
 * De-register the device.
 * Free the hardware.
 * Destroy the device class. 
 */
static uu_void uu_wlan_free(uu_void)
{
    struct list_head tmplist, *i, *tmp;
    uu_wlan_umac_context_t *mac_context, *tmpdata;

    INIT_LIST_HEAD(&tmplist);

    spin_lock_bh(&uu_wlan_radio_lock_g);
    list_for_each_safe(i, tmp, &uu_wlan_radios_g)
        list_move(i, &tmplist);
    spin_unlock_bh(&uu_wlan_radio_lock_g);

    list_for_each_entry_safe(mac_context, tmpdata, &tmplist, nodes) 
    {
        UU_WLAN_LOG_DEBUG(("UURMIDRIVER : Inside uuwlan_free \n"));

        if(mac_context->hw)
            ieee80211_unregister_hw(mac_context->hw);

        if(mac_context->dev)
            device_unregister(mac_context->dev);

        if(mac_context->hw)
            ieee80211_free_hw(mac_context->hw);
    }
    class_destroy(uu_wlan_class_g);
    UU_WLAN_LOG_DEBUG(("UURMIDRIVER : Driver unregistered \n"));
} /* uu_wlan_free */


uu_void uu_wlan_umac_exit(uu_void)
{
    UU_WLAN_LOG_DEBUG(("Inside uu_exit \n"));
    if(uu_wlan_context_gp->running_staus == 1)
        uu_wlan_umac_stop(uu_wlan_context_gp->hw);

    uu_wlan_free();
    unregister_netdev(uu_wlan_net_dev_gp);
    return;
} /* uu_wlan_umac_exit */


/* EOF */

