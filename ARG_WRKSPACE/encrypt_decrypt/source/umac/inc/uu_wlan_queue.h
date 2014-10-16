/*************************************************************************
**                                                                      **
** File name :  uu_wlan_queue.h                                         **
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
/* $Revision: 1.12 $ */

#ifndef __UU_WLAN_QUEUE_H__
#define __UU_WLAN_QUEUE_H__

UU_BEGIN_DECLARATIONS

/* Number of Tx Queues 
    UU_WLAN_TXQ_DATA - For the data Queue. Following are the 
               Subtypes of data queues.
    UU_WLAN_TXQ_AC_BE
    UU_WLAN_TXQ_AC_BK
    UU_WLAN_TXQ_AC_VI
    UU_WLAN_TXQ_AC_VO
    UU_WLAN_TXQ_BEACON 
    UU_WLAN_TXQ_CAB
    UU_WLAN_TXQ_UAPSD
    UU_WLAN_TXQ_PSPOLL
*/
#define UU_WLAN_NUM_TX_QUEUES 8

/** Number of channels */
#define UU_WLAN_NUM_CHANNELS 38 

/* TODO: Rename, and add comments about these macros */
#define UU_WLAN_QUEUE_AMPDU_DEPTH 10
#define UU_WLAN_TX_QUEUE_MAX 20
/** Aggregation delimitars size */
#define UU_AGGR_DELIM_SIZE 4

/** Aggregation minimum packet len in bytes */
#define AGGR_MIN_PKT_LEN 256


#if 0 /** Not using in current implementation */
/**
 * enum buffer_type - Buffer type flags
 *
 * @BUF_AMPDU: This buffer is an ampdu, as part of an aggregate (during TX)
 * @BUF_AGGR: Indicates whether the buffer can be aggregated
 *      (used in aggregation scheduling)
 * @BUF_XRETRY: To denote excessive retries of the buffer
 */
typedef enum buffer_type {
    BUF_AMPDU       = BIT(0),
    BUF_AGGR        = BIT(1),
    BUF_XRETRY      = BIT(2),
} buffer_type_t;
#endif

/** Entry to be added to Queue */
typedef struct uu_wlan_q_entry
{
    struct list_head node;

    /** Skb data */
    struct sk_buff *skb;  

} uu_wlan_q_entry_t;


typedef struct uu_wlan_txq_info
{
    /** Indicated the AC Q list.
     * Intialized during tx queue setup. Add to list after aggregation is scheduled.
     * First entry in the Q is acq_head->next (of type uu_wlan_q_entry_t).
     */
    struct list_head acq_head;
    
    /** List head for status */
    struct list_head acq_status_head;

    /** AC queue id. Initialize this during Tx Queue setup. 
     * Used to identify the corresponding Tx Queue. 
     */
    int qid;

    /** Aggregation current queue depth.
     * Donot queue to hardware if this exceeds low water mark.
     * Incremented when frame added into TX Q for aggregation. 
     */
    int q_ampdu_depth;

    /** Set this when there is no available lmac descriptors or
     * Queue depth reaches to maximum value. Inform mac framework to stop queuing the frames. 
     */
    bool stopped;
    /** Tx Q length. Increment this when ever an entry is added to this queue.
     * Decrement queue length after removing an entry from the list.
     */
    int txq_len;

    /** Maximum length of the Tx queue. Initialize this during queue init.
     * Always compare this with queue length before adding an entry to the queue.
     * If the len is > max value, then call ieee80211_queue_stop on the corresponding 
     * AC. 
     */
    int txq_max;

} uu_wlan_txq_info_t;

/* Default value used for aifs, cwmin, cwmax etc */
#define UU_WLAN_TXQ_USEDEFAULT ((uu_uint32) -1)

/** TX Queue types. */
typedef enum uu_wlan_tx_queue {
    UU_WLAN_TXQ_INACTIVE = 0,
    UU_WLAN_TXQ_DATA,
    UU_WLAN_TXQ_BEACON,
    UU_WLAN_TXQ_CAB,
    UU_WLAN_TXQ_UAPSD,
    UU_WLAN_TXQ_PSPOLL,
    UU_WLAN_TXQ_MAX_Q
} uu_wlan_tx_queue_t;


/** Contains the Tx queue properties.
 * This Q is initialized during H/W init. Each AC have different queue information.
 * Update the hardware as per the configured queue information maintained in hostapd configuration
 * file.
 */
typedef struct uu_wlan_tx_queue_info 
{
    /** Indicates the current queue type. 
     * Read this queue type from the hostapd configuration file and update the hardware 
     * as per the configuration. 
     */
    uu_wlan_tx_queue_t tqi_type;
    /** Indicates the subtype of the data queue */
    uu_int32 tqi_subtype;

    /** AIFS per AC.
     * Set default aifs to hardware during initialization.
     * Update the configuration as per the hostapd configuration file incase of AP.
     */
    uu_uint32 tqi_aifs; 
    /** CWMIN per AC.
     * Set default cwmin to hardware during initialization.
     * Update the configuration as per the hostapd configuration file incase of AP.
     */
    uu_uint32 tqi_cwmin; 
    /** CWMAX per AC.
     * Set default cwmax to hardware during initialization.
     * Update the configuration as per the hostapd configuration file incase of AP.
     */
    uu_uint32 tqi_cwmax;
    /** Short retry AC.
     * Set default short retry value to hardware during initialization.
     * Update the configuration as per the hostapd configuration file incase of AP.
     */
    uu_uint16 tqi_shretry;
    /** Long retry AC.
     * Set default long retry value to hardware during initialization.
     * Update the configuration as per the hostapd configuration file incase of AP.
     */
    uu_uint16 tqi_lgretry;

} uu_wlan_tx_queue_info_t;


/* TODO: Rename, and reorganize. */
/* Earlier, Queues are maintained on per-AC basis.  Now, Qs are created for AC+STA combination.
   Because of this recent change, the new structure is required to have aggregate info per-AC.
   Some fields in the previous queue structure are applicable for entire AC.  Need to reorganize.
 */
struct qstate {
    int num_pkts[4];
    int q_stopped[4];
    int max_pkts[4];
};

extern struct qstate q_state;


UU_END_DECLARATIONS

#endif /* __UU_WLAN_QUEUE_H__*/

