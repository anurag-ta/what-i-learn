/*******************************************************************************
**                                                                            **
** File name :        uu_wlan_main.h                                          **
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

#ifndef __UU_WLAN_MAIN_H__
#define __UU_WLAN_MAIN_H__

#include <linux/etherdevice.h>
#include <net/mac80211.h>
#include <linux/interrupt.h>
#include "uu_datatypes.h"
#include "uu_wlan_frame.h"
#include "uu_wlan_lmac_if.h"
#include "uu_wlan_queue.h"


/** These must match mac80211 skb queue mapping numbers */

/* voice */
#define WME_AC_VO   0
/* video */
#define WME_AC_VI   1
/* best effort */
#define WME_AC_BE   2
/* background */
#define WME_AC_BK   3
#define WME_NUM_AC  4
/** Number of tids */
#define WME_NUM_TID 16
/** Black ack window bitmap size.
 * Indicates maximum number frames to be acked */
#define WME_BA_BMP_SIZE         64
/** Maximum Black ack window size */
#define WME_MAX_BA              WME_BA_BMP_SIZE
/** Maximum number of buffers per tid */
#define UU_WLAN_TID_MAX_BUFS        (2 * WME_MAX_BA)

/** Minimum h/w qdepth to be sustained to maximize aggregation */
#define UU_WLAN_AGGR_MIN_QDEPTH    2

/** Maximum number of beacon intervals to wait.
 * Regardless of the number of BSSIDs, if a given beacon does not go out even after
 * waiting this number of beacon intervals, reset the device
 */
#define BSTUCK_THRESH 9

/** Maximum sequence number. 
 * Sequence number is incremented during Black-Ack window updation
 */
#define IEEE80211_SEQ_MAX      4096

/** UMAC supported flags.
 * Add this to mac_context_flags of uu_wlan_umac_context to indicate the TXAGGR support.
 */
#define MAC_CONTEXT_OP_TXAGGR         BIT(0)

#define UU_WLAN_MAC_ADDR_LEN 6

UU_BEGIN_DECLARATIONS

/** Low level driver statistics. 
 * Used to maintain the statistics of the low level driver. 
 * Read and Clear the statistics during recv frame.
 * Update these statistics to the mac framework .*/
typedef struct uu_wlan_stats{

/* MIB counters */
    /** Number of ack failed pkts. */
    uu_uint32 ack_fail;
    /** Number of failed rts pkts. */
    uu_uint32 rts_fail;
    /** Number of successed rts packets */
    uu_uint32 rts_ok;
    /** FCS error frames count */
    uu_uint32 fcs_error;

} uu_wlan_stats_t;


/** Tx DMA structure.
 * Contains information of transmitted frames. Update this structure
 * after getting frame from mac framework.
 */

typedef struct uu_wlan_tx
{
    /** Tx frame sequence number.
     * If framework doesnot set sequence number then,
     * update this value after getting the frame from mac layer.
     */
    uu_uint16 seq_no;

    /** Bitmap to indicates intialized TX Queues.
     * If Queue is enabled, Set that corresponding bit during queue setup.
     * Reset this Queue status during queue cleanup.
     */
    uu_uint32 txqsetup;
    /** Lock before adding to listhead.
     * Unlock after adding to listhead. Intitialized druing init.
     */
    //spinlock_t txbuflock;
    /** Buffers associated with tx.
     * Allocated during tx init 
     */
    struct list_head txbuf; 

    /** Array of AC queue Instances per hardware queue*/
    //uu_wlan_txq_t txq[UU_WLAN_NUM_TX_QUEUES];

    /** Array of AC queue Instances */
    //uu_wlan_txq_t *txq_map[WME_NUM_AC];
} uu_wlan_tx_t;


#if 0
/** Frame info structure in umac.
    Used to store the frame information of the transmitted frame.
 */
typedef struct uu_wlan_umac_frame_info {
    /** Buffer used to store the current frame */
    uu_wlan_buf_t *bf;
    /** Length of the current mpdu */
    uu_int32 framelen;
} uu_wlan_umac_frame_info_t;
#endif


/** Channel information structure which holds the current channel information.
 * Initialized during hardware init and updated when bss is changed.
 */
typedef struct uu_wlan_channel 
{
    /** Pointer to the ieee80211 channel strucutre */
    struct ieee80211_channel *chan;
    /** Contains the channel id. 
     * Updated with the configured channel id of hostapd configuration file.
     */
    u16 channel;

    u32 channelFlags;
    u32 chanmode;
} uu_wlan_channel_t;


/** Hardware structure that should register with mac framework.
 * Initialize this during device init. Free when device is unloaded 
 */
typedef struct uu_wlan_hw {
    /** Instance of hardware information and state */
    struct ieee80211_hw *hw;
    /** Device operational mode . 
     * During driver intialization set the opmode of the hardware.
     * Refer nl80211.h for availabe nl80211_iftypes.
     */
    enum nl80211_iftype opmode;  /* TODO: can remove */
    /** Array of Tx Queue instances */
    uu_wlan_tx_queue_info_t txq[UU_WLAN_NUM_TX_QUEUES]; /* TODO ?????? */

    uu_wlan_channel_t channels[UU_WLAN_NUM_CHANNELS]; /* TODO: can remove */
    uu_wlan_channel_t *curchan; /* TODO: can remove */

    /** Slot time for LMAC use, based on current PHY.
     * Get the updated slot time whenever BSS info changed.
     * Based on this, calculate timers like SIFS.
     */
    uu_int32 slottime; /* TODO: can remove */

} uu_wlan_hw_t;


/** Beacon structure for maitaining current beacon information.
 * Update this whenever device gets new beacon from the framework.
 */
typedef struct uu_wlan_beacon
{
    enum 
    {
        /** No change needed */
        OK,
        /** Update pending.
         * If a station joins/leaves network, change the state to UPDATE and 
         * wait for one beacon for associated stations and then change the 
         * state to COMMIT 
         */
        UPDATE,
        /** beacon sent, commit change */
        COMMIT
    } updateslot;

    /** Beacon Queue id. 
     * Intialized during uu_wlan_init_queues. Used to identify the 
     * configured beacon Queue.
     */
    uu_uint32 beaconq;

    /** Missed beacon count during TX. 
     * Increment this when the previous beacon still in beacon Q and is not sent out. 
     * If we miss too many consecutive beacons reset the device.
     * Maximum bmisscount should be less than configured BSTUCK_THRESH. BSTUCK_THRESH 
     * value indicates the number of beacon intervals
     */
    uu_uint32 bmisscnt;

    /** List of beacon buffers.
     * Intialized during TX DMA initialization. Used when umac got beacon
     * frame from mac framework. 
     */
    struct list_head beacon_buf;

    /** Current Beacon timestamp. 
     * This value is taken from the beacon frame and is updated for the next beacon
     * based on the calculated TSF offset.
     */
    uu_uint32 bc_tstamp; 

    /** Status of the beacon transmission. 
     * Intialize this to false during startup and reset.
     * Set to true after beacon frame is successfully sent from the beacon Queue 
     */
    uu_bool tx_processed;

    /** Slot time for calculating SIFS value.
     * This value is same as whatever is configured in HW.
     * Broadcast this value, if we are the AP.
     */
    uu_int32 slottime;

} uu_wlan_beacon_t;



/** Beacon configuration structure */
typedef struct uu_wlan_beacon_config {
    /** Configured beacon interval. 
     * This is the time interval between beacon transmissions. 
     * Get this value from the beacon frame. 
     */
    uu_int32 beacon_interval; /* TODO: Use this */

    /** Configured beacon listen interval.
     * Listen interval used by the AP as a guaranteed maximum time before stations listen 
     * to one of their beacons. Initialize to default configuration when interface is added. 
     */ 
    uu_uint16 listen_interval;   /** TODO: Currently not handling */
} uu_wlan_beacon_config_t;

/** structure to hold the driver specific data.
 * Structure contains umac context which contains information about 
 * the device, virtual interface, beacon information etc.
 * Intialize this during device init. Free during umac is unloaded.
 */
typedef struct uu_wlan_umac_context 
{
    /** Pointer to the ieee80211 hardware data structure.
     * Allocated in uu_wlan_umac_init during umac module insertion and 
     * free in uu_wlan_free during remove umac module or free this when 
     * device creation is failed. 
     */
    struct ieee80211_hw *hw;

    /** Pointer to the active wireless device on this chip. 
     * created in uu_wlan_umac_init using device_create function 
     */
    struct device *dev;

    /** Instance of per interface data.
     * We can only have one operating interface (802.11 core) at a time. 
     * Initialized during interface add and updated when operating interface 
     * is changed. */
    struct ieee80211_vif *vif;

    /** Work Q structure for beacon handling. 
     * Initialized during device init and scheduled when beacon is generated 
     * and transmitted to the driver. */
    struct work_struct beacon_update_trigger;

    /** Instance of the hardware structure.
     * Intialized during device init and free when device is unloaded. 
     * Update when hardware configuration changed.*/
    uu_wlan_hw_t *mac_context_hw; /* TODO: not using any where */

    /** Driver supported FLAGS. 
     * Set this flag based on supported MAC_CONTEXT_OP_*. 
     */
    uu_uint32 mac_context_flags;

    /** Instance of beacon structure.
     * Contains beacon information like beacon qid, beacon interval, 
     * missed beacon count, beacon status etc... and beacon q is initialized during 
     * device init (uu_wlan_init_queue) 
     */
    uu_wlan_beacon_t beacon; /* TODO: merge with BQID code beacon seperate queue code */

    /** Instance of Tx structure.
     * Initialize the tx path during device init.
     */
    uu_wlan_tx_t tx;

    uu_wlan_txq_info_t tx_q[UU_WLAN_NUM_TX_QUEUES][UU_WLAN_MAX_ASSOCIATIONS];

    /** Supported bands used for frame transmission. 
     * Initalize with supported bands during driver init. 
     * Fill channels information, bitrates information during initialization.
     * Refer cfg80211.h for available sbands. 
     */
    struct ieee80211_supported_band sbands[IEEE80211_NUM_BANDS];

    /** Spink lock variable for handling multiple associations.
     * Lock before adding new association or attaching node and unlock after station is 
     * de-associated 
     */
    //spinlock_t nodes_lock; 

    /** List of associated nodes. 
     * When new station is associated add node to this list and delete from 
     * this list when the stations is de-associated. 
     */
    struct list_head nodes; 

    /** Instance of Current beacon configuration. 
     * Update this when new beacon is generated using ieee80211_get_beacon. 
     */
    uu_wlan_beacon_config_t cur_beacon_conf;

    /** Umac(device) running status. 
     * Set to 1 after umac and lmac are up and set to 0 before lmac and umac stop. */
    uu_int32 running_staus;

    /** Indicates the current mode (vif->type) ie STA / AP / ADHOC etc . 
     * Default set to station mode. Updated based on the userspace application ie acts 
     * as AP for hostapd app and acts as STA for wpa_supplicant. 
     */
    uu_int32 cur_op_mode;

    /** Lowlevel driver statistics */
    uu_wlan_stats_t drv_stats;

    /** Current channel id in use.
     * Get this from configuration when driver is loaded.
     */
    uu_int32 chan_id;
    struct tasklet_struct rx_task;
    struct tasklet_struct txstatus_task;

    uu_uint8 bssid[ETH_ALEN];
    uu_uint8 assoc_sta[ETH_ALEN];

} uu_wlan_umac_context_t;

/** TID strucutre */
typedef struct uu_wlan_tid
{
    /** List of buffers */
    struct list_head list;
    /** Station instance */
    struct uu_sta_nodes *node;

    /** List of buffers per tid */
    struct sk_buff_head buf_q;

    /** Bitmap to indicate Aggregation current state.
     * Choose states from the following list
     * AGGR_CLEANUP, AGGR_ADDBA_COMPLETE, AGGR_ADDBA_PROGRESS. 
     */
    uu_uint8 state;
    /** Indicates whether aggregation per tid scheduled or not. */
    uu_int32 sched;
    /** Current tid number.
     * Intialized during node initialization.
     */
    uu_int32 tidno;
    /** Bit map to indicates which buffers are set in current tid.
     * Clear the bit if the buffer is not in block-ack range.
     * Updated during block-ack window update and add functions */
    uu_uint64 tx_buf[BITS_TO_LONGS(UU_WLAN_TID_MAX_BUFS)];

    /** Starting sequnce number per this tid.
     * Intialized to 0 during node intialization. seq_start is per tid.
     */
    uu_uint16 seq_start;
    /** Next sequence number.
     * Intialized to 0 during node intialization and is per tid basis. 
     */
    uu_uint16 seq_next;
    /** Blackack window size.
     * Intialized to max Black ack window size and is per tid basis
     */
    uu_uint16 baw_size;
    /** first un-acked tx buffer.
     * Intialized to 0 and is per tid basis.
     */ 
    uu_int32 baw_head;
    /** next unused tx buffer slot.
     * Intialized to 0 and is per tid basis. 
     */
    uu_int32 baw_tail;
    /** Indicates current state of BA session.
     * Following are the BA session states
     * AGGR_CLEANUP, AGGR_ADDBA_COMPLETE, AGGR_ADDBA_PROGRESS
     */
    /** Bitmap to indicate BAR.
     */
    uu_int32 bar_index;
    uu_int32 paused;
}uu_wlan_tid_t;

/** Used to indicated the station information. 
 * Associated stations list is maintained here. 
 */
typedef struct uu_sta_nodes
{
    /** List of associated nodes */
    struct list_head list;
    /** Station information is maintained here.
     * Refer mac80211.h for ieee80211_sta structure 
     */
    struct ieee80211_sta *sta;
    /** Array of tid instances */
    uu_wlan_tid_t tid[WME_NUM_TID];
    /** Indicates power state transmission of the associated station.
     * Changed when ever sta_notify call back called from framework.
     * Donot queue frames for aggregation if that associated station is in sleep mode.
     */
    uu_bool sleeping;

} uu_sta_nodes_t;


/* Global variables */

/** Supported Callbacks from mac framework to driver.
 * Used by device during new hardware is allocated with ieee80211_alloc_hw
 */
extern struct ieee80211_ops uu_wlan_ops;
/** BASE ADDRESS */
extern void __iomem *baseaddr;

/** Instance of the umac context.  */
extern uu_wlan_umac_context_t *uu_wlan_context_gp;

/* Functions */

extern uu_void uu_wlan_umac_exit(uu_void);

/** Mac80211 callback which is called before first netdevice is attached to 
 * the hardware.
 * Start the lmac process here. Return -EIO if failed to start the lmac.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw().
 * @param[in] skb Contains the beacon frame generated from userspace application.
 * @param[in] txctl Contains txq mapping corresponding to ac queue.
 */
extern uu_int32 uu_wlan_tx_start(struct ieee80211_hw *hw, 
                                struct sk_buff *skb, 
                                uu_wlan_txq_info_t *txq);

/** Stop umac process.
 * Make sure umac and lmac are stopped before unloading the driver. 
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 */
extern uu_void uu_wlan_umac_stop(struct ieee80211_hw *hw);

/** Calculates the padding possition.
 * During aggregation, every MPDU should be 4-byte aligned in AMPDU except last MPDU.
 * Calculate the padding position and add padding bits to form 4-byte aligned MPDU.
 * @param[in] frame_control Frame control header.
 * @return Padding position to add padding bits.
 */
extern uu_int32 uu_wlan_agg_cal_pad_position(__le16 frame_control);


/** Fill the frame information from the received skb and configuration.
 * Following information is filled
 * Remote sta address, bssid, band, keytype, frametype, framelen, is_ampdu flag,
 * AC, rates, mpdu and flags.
 * For flags refer uu_wlan_setup_tx_flags.
 * @param[in] hw pointer obtained from ieee80211_alloc_hw
 * @param[in] skb sk_buff variable which contains frame information like frame header and frame body
 * @param[in] framelen Frame length
 * @param[out] fi Frame info structure to be filled.
 */
extern uu_void uu_wlan_setup_tx_frame_info(struct ieee80211_hw *hw, 
                                        struct sk_buff *skb,
                                        uu_int32 framelen, 
                                        uu_wlan_tx_frame_info_t *fi);

/** Sets the supported tx flags.
 * Select the supported flags from "mac80211_tx_control_flags".
 * Refer mac80211.h file for mac framework supported tx control flags.
 * @param[in] skb the frame to which tx flags to be setup.
 * @return supported tx flag bitmap.
 */
extern uu_int32 uu_wlan_setup_tx_flags(struct sk_buff *skb);

/** Calls corresponding tx function based on aggregation support flag.
 * Send normal frame or aggregated frame to lmac based on the aggregation support flag.
 * @param[in] mac_context umac context structure.
 * @param[in] txctl Tx control pointer which contains the corresponding queue support.
 * @param[in] skb sk_buff pointer which contains the frame.
 */
extern uu_void uu_wlan_start_tx_send(uu_wlan_umac_context_t *mac_context,
                                    uu_wlan_txq_info_t *txq,
                                    struct sk_buff *skb);


/** This function will be called by uu_wlan_setup_tx_frame_info in uu_mac_tx.c
 * for filling the TxVector informations (Format, Rate, Modulation, Bandwidth,
 * Channel offset, Stbc, Nss, Ntx, etc)
 */
extern uu_void uu_wlan_setup_frame_info_rc(struct ieee80211_hw *hw,
              struct sk_buff *skb, uu_int32 framelen,
              uu_wlan_tx_frame_info_t *fi);

#ifdef UU_WLAN_TPC
/** This function will be called to setup the TX_Power for the frame */
extern uu_void uu_wlan_setup_frame_tpc (struct sk_buff *skb,
                uu_wlan_tx_frame_info_t *fi, struct ieee80211_hw *hw);
#endif

/** This function will be called by uu_wlan_setup_tx_frame_info in uu_mac_tx.c
 * for filling the Length information in TxVector as per the format
 */
extern uu_void uu_wlan_setup_frame_info_length (uu_wlan_tx_frame_info_t *fi, uu_int32 framelen);

/* Called by uu_wlan_recv.c while determining the rate_index of the Legacy rates */
extern uu_uint8 uu_get_rx_rate_idx (uu_uint8 L_datarate);

/** Used to setup the Queue and fill the qinfo.
 * @param[in] mac_context umac context pointer.
 * @param[in] qtype Type of the queue.
 * supported qtypes UU_WLAN_TXQ_DATA, UU_WLAN_TXQ_BEACON, UU_WLAN_TXQ_CAB, UU_WLAN_TXQ_UAPSD, UU_WLAN_TXQ_PSPOLL.
 * @param[in] subtype Valid if the type is UU_WLAN_TXQ_DATA.
 * supported subtypes are UU_WLAN_TXQ_AC_BE, UU_WLAN_TXQ_AC_BK, UU_WLAN_TXQ_AC_VI, UU_WLAN_TXQ_AC_VO.
 * @return Tx queue structure.
 */
/*extern uu_wlan_txq_t *uu_wlan_txq_setup(uu_wlan_umac_context_t *mac_context, 
                        uu_int32 qtype, 
                        uu_int32 subtype);*/

/** Hardware Queue setup */
extern uu_int32 uu_wlan_hw_setuptxqueue(uu_wlan_hw_t *uhw, uu_wlan_tx_queue_t type,
                          const uu_wlan_tx_queue_info_t *qinfo);

/** Set hardware tx queue properties.
 * @param[in] uhw driver hardware pointer,
 * @param[in] q queue number for which queue properties should be set
 * @param[out] qinfo Update the queue information structure with the configured values.
 * @return false for the invalid queue number.
 * @return true after setting the queue information .
 */
extern uu_bool uu_wlan_hw_set_txq_props(uu_wlan_hw_t *uhw, uu_int32 q,
                const uu_wlan_tx_queue_info_t *qinfo);



extern uu_void uu_wlan_fill_tx_status(uu_wlan_tx_status_t *tx_status);

extern uu_char uu_wlan_umac_get_ac(int queue);
extern uu_char uu_wlan_lmacq_to_fwkq(int queue);

#ifdef UU_WLAN_TPC
extern uu_int32 uu_wlan_tpc_timer_start(uu_void);
extern uu_int32 __send_frame(struct sk_buff *skb);
#elif defined UU_WLAN_DFS
extern uu_int32 __send_frame(struct sk_buff *skb);
#endif

UU_END_DECLARATIONS

#endif  /* __UU_WLAN_MAIN_H__ */

