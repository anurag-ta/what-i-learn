/*************************************************************************
**                                                                      **
** File name :  uu_wlan_lmac_if.h                                       **
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

#ifndef __UU_WLAN_LMAC_IF_H__
#define __UU_WLAN_LMAC_IF_H__

#include "uu_datatypes.h"
#include "uu_wlan_frame.h"

UU_BEGIN_DECLARATIONS

/** Maximum number of simultaneous Tx Status that LMAC may give to UMAC.
 * This is same as the max number of MPDUs that can be outstanding at LMAC.
 * This limit is on per-AC basis. So, these many can be there for each AC.
 */
#define UU_WLAN_MAX_TX_STATUS_TO_UMAC      32
#define UU_WLAN_MAX_TX_STATUS_TO_UMAC_MASK (UU_WLAN_MAX_TX_STATUS_TO_UMAC-1)

/** Maximum number of simultaneous Rx frame status information that LMAC may give to UMAC.
 */
#define UU_WLAN_MAX_RX_STATUS_TO_UMAC 32
#define UU_WLAN_MAX_RX_STATUS_TO_UMAC_MASK (UU_WLAN_MAX_RX_STATUS_TO_UMAC-1)

#if 0
/** Following Tx Status, Tx Flags and RX status are not used */
/* Tx Status */
#define UU_WLAN_TXERR_XRETRY         0x01
#define UU_WLAN_TXERR_FILT           0x02
#define UU_WLAN_TXERR_FIFO           0x04
#define UU_WLAN_TXERR_XTXOP          0x08
#define UU_WLAN_TXERR_TIMER_EXPIRED  0x10
#define UU_WLAN_TX_ACKED             0x20
#define UU_WLAN_TXERR_MASK                                                \
        (UU_WLAN_TXERR_XRETRY | UU_WLAN_TXERR_FILT | UU_WLAN_TXERR_FIFO |     \
         UU_WLAN_TXERR_XTXOP | UU_WLAN_TXERR_TIMER_EXPIRED)

/* Tx Flags */
#define UU_WLAN_TX_BA                0x01
#define UU_WLAN_TX_PWRMGMT           0x02
#define UU_WLAN_TX_DESC_CFG_ERR      0x04
#define UU_WLAN_TX_DATA_UNDERRUN     0x08
#define UU_WLAN_TX_DELIM_UNDERRUN    0x10
#define UU_WLAN_TX_SW_FILTERED       0x80

/* Rx status info moved to uu_wlan_rx_status.h filr */
/* RX status */
#define UU_WLAN_RXERR_CRC           0x01
#define UU_WLAN_RXERR_PHY           0x02
#define UU_WLAN_RXERR_FIFO          0x04
#define UU_WLAN_RXERR_DECRYPT       0x08
#define UU_WLAN_RXERR_MIC           0x10
#endif


/** Used in filling rx status */
/* RX Flags */
#define UU_WLAN_RX_FLAG_MMIC_ERROR      0x001 
#define UU_WLAN_RX_FLAG_DECRYPTED       0x002 
#define UU_WLAN_RX_FLAG_MMIC_STRIPPED   0x004
#define UU_WLAN_RX_FLAG_IV_STRIPPED     0x008
#define UU_WLAN_RX_FLAG_FAILED_FCS_CRC  0x010
#define UU_WLAN_RX_FLAG_FAILED_PLCP_CRC 0x020
#define UU_WLAN_RX_FLAG_MACTIME_MPDU    0x040
#define UU_WLAN_RX_FLAG_SHORTPRE        0x080
#define UU_WLAN_RX_FLAG_HT              0x100
#define UU_WLAN_RX_FLAG_40MHZ           0x200
#define UU_WLAN_RX_FLAG_SHORT_GI        0x400


#define UU_WLAN_RXKEYIX_INVALID   ((uu_uint8)-1)
#define UU_WLAN_TXKEYIX_INVALID   ((uu_uint32)-1)
#define UU_WLAN_RSSI_BAD          -128


/* Associtated station's capability Flags */
/** Smoothing req, ldpc coding, no sig ext and short GI are part of txvector.
 * Following flags are defined but not using now.
 * TODO: Are these required? Some of them are already in Frame-Info.
 */
typedef enum uu_wlan_asso_sta_flags
{
    UU_WLAN_ASSO_STA_FLAG_SELF_CTS      =  BIT(0),
    UU_WLAN_ASSO_STA_FLAG_HT_SUPPORT    =  BIT(1),
    UU_WLAN_ASSO_STA_FLAG_VHT_SUPPORT   =  BIT(2),

    UU_WLAN_ASSO_STA_FLAG_SMOOTHING_REQ =  BIT(3),
    UU_WLAN_ASSO_STA_FLAG_LDPC_CODING   =  BIT(4),
    UU_WLAN_ASSO_STA_FLAG_NO_SIG_EXT    =  BIT(5)
    //UU_WLAN_ASSO_STA_FLAG_SHORT_GI      =  BIT(6), Not used anywhere
} uu_wlan_asso_sta_flags_t;



typedef struct uu_lmac_config_asso_add
{
    /* Remote STA Address */
    uu_uchar  addr[IEEE80211_MAC_ADDR_LEN];

    /* Association ID */
    uu_uint16  aid;

    /* station supported features and flags HT/VHT supports or not, smoothing and LDPC required or not */
    uu_wlan_asso_sta_flags_t     flags;

    uu_wlan_ch_bndwdth_type_t    ch_bndwdth;

    //uu_wlan_non_ht_modulation_t  modulation; /* non-ht modulation codes */
    /* This info is required here, for control frames only */
    uu_uint16                    basic_mcs;  /* 11n basic rate MCS number */

    /* BSSID. Used in IBSS case */
    uu_uchar  bssid[IEEE80211_MAC_ADDR_LEN];

    //uu_wlan_expansion_mat_type_t expansion_mat_type;
    //uu_wlan_chan_mat_type_t      chan_mat_type;
    //uu_uchar tx_power;

    /* added station_info for ampdu */
    uu_uchar ampdu_exponent;
    uu_uchar ampdu_min_spacing;

    /* Padding for 4-byte alignment */
} uu_lmac_config_asso_add_t;


typedef struct uu_lmac_config_asso_del
{
    /* Remote STA Address */
    uu_uchar   addr[IEEE80211_MAC_ADDR_LEN];

    /* Association ID */
    uu_uint16  aid;
} uu_lmac_config_asso_del_t;


/* ADDBA information */
typedef struct uu_lmac_config_addba_info
{
    /* Remote STA Address - 6bytes */
    uu_uchar  addr[IEEE80211_MAC_ADDR_LEN];

    /* Byte 7 */

    /* Traffic Identifier - 4 bits */
    uu_uint8  tid:4;
    /* Direction of BA sesson. 1: Ours is Tx, 0: Ours is Rx */
    uu_uchar  is_dir_tx:1;
    uu_uchar  reserved1:3;

    /* Byte 8 */

    /* Receiver buffer size (number of buffers) - Upto 64 (6 bits) */
    uu_uint8  bufsz:6;
    uu_uchar  reserved2:2;

    /* Byte 9 & 10 */

    /* Starting sequence number (12-bits) */
    uu_uint16 ssn:12;

    /* Block Ack session - immediate and delayed. TODO: Identify the purpose & rename. Also see 'uu_wlan_ba_ses_type_t' */
    uu_uchar  type:3;
    uu_uchar  reserved3:3;

    /* Byte 11 & 12 - reserved */
    uu_uint16 reserved4;
} uu_lmac_config_addba_info_t;


/* DELBA information */
typedef struct uu_lmac_config_delba_info
{
    /* Remote STA Address - 6bytes */
    uu_uchar  addr[IEEE80211_MAC_ADDR_LEN];

    /* Traffic Identifier - 4 bits */
    uu_uint8  tid:4;

    /* Direction of BA sesson. 1: Ours is Tx, 0: Ours is Rx */
    uu_uchar  is_dir_tx:1;
    uu_uchar  reserved1:3;

    /* Byte 8 - reserved */
    uu_uchar  reserved2;
} uu_lmac_config_delba_info_t;


#define UU_WLAN_LMAC_CONFIG_ASSO_ADD         1
#define UU_WLAN_LMAC_CONFIG_ASSO_DEL         2
#define UU_WLAN_LMAC_CONFIG_BA_SESS_ADD      3
#define UU_WLAN_LMAC_CONFIG_BA_SESS_DEL      4

typedef struct uu_wlan_lmac_config
{
    /*  Config command type
       1: Association Addition
       2. Association deletion
       3: BA session add
       4. BA session deletion
     */
    uu_uint8   cmd_type;

    union 
    {
        /** Association information, to be added */
        uu_lmac_config_asso_add_t  asso_add;

        /** Assotiotion to be deleted */
        uu_lmac_config_asso_del_t  asso_del;

        /** ADDBA info to be added */
        uu_lmac_config_addba_info_t  addba;
    
        /** DELBA info to be deleted */
        uu_lmac_config_delba_info_t  delba;

    } cmd_data;
} uu_wlan_lmac_config_t;


typedef enum uu_wlan_tx_control_flags
{
    /* Tx status required for this frame */
    UU_WLAN_TX_CTL_REQ_TX_STATUS    = BIT(0),
    /* Asigns the seq number for this frame */
    UU_WLAN_TX_CTL_ASSIGN_SEQ       = BIT(1),
    /* ACK not required for this frame */
    UU_WLAN_TX_CTL_NO_ACK           = BIT(2),
    /* This frame is injected */
    UU_WLAN_TX_CTL_INJECTED         = BIT(3),

    /* Below two are mutual exclusive */
    UU_WLAN_TX_CTL_USE_RTS          = BIT(4),
    UU_WLAN_TX_CTL_USE_CTS          = BIT(5), /* If CTS-to-self supported */

    //UU_WLAN_TX_CTL_AMPDU       = BIT(6),

} uu_wlan_tx_control_flags_t;


/** Indication type from LMAC to UMAC. Will map to interrupt, in RTL */
typedef enum uu_wlan_lmac_ind
{
    /** Indication for rx frame ready */
    UU_LMAC_IND_RX_FRAME,
    /** Indication to generate and send the beacon frame */
    UU_LMAC_IND_GEN_BEACON,
    /** Indication for tx status */
    UU_LMAC_IND_TX_STATUS
} uu_wlan_lmac_ind_t;


typedef struct uu_wlan_tx_status {
    /* Tx timestamp  */
    uu_uint32 tstamp;
    /* Sequence number  */
    uu_uint16 seqnum;
    /* Tx Status  */
    uu_uint8 status;
    /* Rate index used for this frame to send */
    uu_uint8 rateindex;
    /* RSSI   */
    uu_uint8 rssi;
    /* cookie value for confirmation */
    uu_uint32 cookie;
    /*   */
    uu_uint8 shortretry;
    /*   */
    uu_uint8 longretry;
    /*   */
    uu_uint8 virtcol;
    /*   */
    uu_uint8 flags;
    /* RSSI of each antenna  */
    uu_uint8 rssi_ctl0;
    uu_uint8 rssi_ctl1;
    uu_uint8 rssi_ctl2;
    /*   */
    uu_uint8 rssi_ext0;
    uu_uint8 rssi_ext1;
    uu_uint8 rssi_ext2;
    /* Queue ID of frame  */
    uu_uint8 qid;
    /* Description ID of frame  */
    uu_uint16 desc_id;
    /* TID of frame  */
    uu_uint8 tid;
    /* Lower byte of BlockACK  */
    uu_uint32 ba_low;
    /* Higher byte of BlockACK  */
    uu_uint32 ba_high;
    /*   */
    uu_uint32 evm0;
    uu_uint32 evm1;
    uu_uint32 evm2;
} uu_wlan_tx_status_t;


typedef enum uu_wlan_phyerr 
{
    UU_WLAN_PHYERR_UNDERRUN             = 0,  /* Transmit underrun */
    UU_WLAN_PHYERR_TIMING               = 1,  /* Timing error */
    UU_WLAN_PHYERR_PARITY               = 2,  /* Illegal parity */
    UU_WLAN_PHYERR_RATE                 = 3,  /* Illegal rate */
    UU_WLAN_PHYERR_LENGTH               = 4,  /* Illegal length */
    UU_WLAN_PHYERR_RADAR                = 5,  /* Radar detect */
    UU_WLAN_PHYERR_SERVICE              = 6,  /* Illegal service */
    UU_WLAN_PHYERR_TOR                  = 7,  /* Transmit override receive */

    UU_WLAN_PHYERR_OFDM_TIMING          = 17,
    UU_WLAN_PHYERR_OFDM_SIGNAL_PARITY   = 18,
    UU_WLAN_PHYERR_OFDM_RATE_ILLEGAL    = 19,
    UU_WLAN_PHYERR_OFDM_LENGTH_ILLEGAL  = 20,
    UU_WLAN_PHYERR_OFDM_POWER_DROP      = 21,
    UU_WLAN_PHYERR_OFDM_SERVICE         = 22,
    UU_WLAN_PHYERR_OFDM_RESTART         = 23,
    UU_WLAN_PHYERR_FALSE_RADAR_EXT      = 24,

    UU_WLAN_PHYERR_CCK_TIMING           = 25,
    UU_WLAN_PHYERR_CCK_HEADER_CRC       = 26,
    UU_WLAN_PHYERR_CCK_RATE_ILLEGAL     = 27,
    UU_WLAN_PHYERR_CCK_SERVICE          = 30,
    UU_WLAN_PHYERR_CCK_RESTART          = 31,
    UU_WLAN_PHYERR_CCK_LENGTH_ILLEGAL   = 32,
    UU_WLAN_PHYERR_CCK_POWER_DROP       = 33,

    UU_WLAN_PHYERR_HT_CRC_ERROR         = 34,
    UU_WLAN_PHYERR_HT_LENGTH_ILLEGAL    = 35,
    UU_WLAN_PHYERR_HT_RATE_ILLEGAL      = 36,

    UU_WLAN_PHYERR_MAX                  = 37
} uu_wlan_phyerr_t;


#if 0 /* TODO: Why is this not used? */
/**
 * enum mac80211_rx_flags - receive flags
 *
 * These flags are used with the @flag member of &struct ieee80211_rx_status.
 * @RX_FLAG_MMIC_ERROR: Michael MIC error was reported on this frame.
 *      Use together with %RX_FLAG_MMIC_STRIPPED.
 * @RX_FLAG_DECRYPTED: This frame was decrypted in hardware.
 * @RX_FLAG_MMIC_STRIPPED: the Michael MIC is stripped off this frame,
 *      verification has been done by the hardware.
 * @RX_FLAG_IV_STRIPPED: The IV/ICV are stripped from this frame.
 *      If this flag is set, the stack cannot do any replay detection
 *      hence the driver or hardware will have to do that.
 * @RX_FLAG_FAILED_FCS_CRC: Set this flag if the FCS check failed on
 *      the frame.
 * @RX_FLAG_FAILED_PLCP_CRC: Set this flag if the PCLP check failed on
 *      the frame.
 * @RX_FLAG_MACTIME_MPDU: The timestamp passed in the RX status (@mactime
 *      field) is valid and contains the time the first symbol of the MPDU
 *      was received. This is useful in monitor mode and for proper IBSS
 *      merging.
 * @RX_FLAG_SHORTPRE: Short preamble was used for this frame
 * @RX_FLAG_HT: HT MCS was used and rate_idx is MCS index
 * @RX_FLAG_40MHZ: HT40 (40 MHz) was used
 * @RX_FLAG_SHORT_GI: Short guard interval was used
 */
enum mac80211_rx_flags {
    RX_FLAG_MMIC_ERROR      = 1<<0,
    RX_FLAG_DECRYPTED       = 1<<1,
    RX_FLAG_MMIC_STRIPPED   = 1<<3,
    RX_FLAG_IV_STRIPPED     = 1<<4,
    RX_FLAG_FAILED_FCS_CRC  = 1<<5,
    RX_FLAG_FAILED_PLCP_CRC = 1<<6,
    RX_FLAG_MACTIME_MPDU    = 1<<7,
    RX_FLAG_SHORTPRE        = 1<<8,
    RX_FLAG_HT              = 1<<9,
    RX_FLAG_40MHZ           = 1<<10,
    RX_FLAG_SHORT_GI        = 1<<11,
};
#endif /* #if 0 */


typedef struct uu_wlan_rx_status 
{
    /* Received time stamp */
    uu_uint32 tstamp;
    /* Data Length */
    uu_uint16 datalen;
    /* Status  */
    uu_uint8 status;
    /* Phy specific error if any  */
    uu_uint8 phyerr;
    /* Received RSSI */
    uu_uint8 rssi;
    /* Key Index used for Decrypt  */
    uu_uint8 keyix;
    /* Data Rate  */
    uu_uint8 rate;
    /* Received antenna's   */
    uu_uint8 antenna;
    /*   */
    uu_uint8 more;
    /* RSSI of each antenna  */
    uu_uint8 rssi_ctl0;
    uu_uint8 rssi_ctl1;
    uu_uint8 rssi_ctl2;
    /* RSSI of each antenna  */
    uu_uint8 rssi_ext0;
    uu_uint8 rssi_ext1;
    uu_uint8 rssi_ext2;
    /* Is this frame aggregated or not  */
    uu_uint8 isaggr;
    /* more aggregated frames are available  */
    uu_uint8 moreaggr;
    /* number of delimiters   */
    uu_uint8 num_delims;
    /* Rx flags  */
    uu_uint8 flags;
    /*   */
    uu_uint32 evm0;
    uu_uint32 evm1;
    uu_uint32 evm2;
    uu_uint32 evm3;
    uu_uint32 evm4;
} uu_wlan_rx_status_t;

typedef struct uu_wlan_tx_status_info
{
    /*byte 1 & 2 */
    uu_uint16   seqno:12;
    uu_uint16   reserved1:4; 
    /* byte 3 */
    uu_uint8    retry_count:4;
    uu_uint8    reserved2:4;
    /* byte 4 */
    uu_uint8    status:1;  
    /* Giving Bandwidth used for transmit this frame */
    uu_uint8    bandwidth: 3;
    uu_uint8    reserved3:4;
    /* byte 5,6,7,8 */
    uu_uint32   cookie;
} uu_wlan_tx_status_info_t;


/** Received frame status information.
 * addr, contains starting address of received frame in the circular buffer
 * len, contains the length of the frame for the UMAC referece. 
 */
typedef struct uu_wlan_rx_status_info
{
    /* Offset from the start of Rx buffer, at which Rx-Frame-Info is kept */
    uu_uint32 offset;

    /* Received frame length */
    uu_uint16 len:14;
    uu_uint16 reserved1:2;
    uu_uint16 reserved2;
} uu_wlan_rx_status_info_t;


typedef enum uu_wlan_asso_sta_mode
{
    UU_WLAN_ASSO_STA_MODE_LEGACY, /* 802.11 a/b/g */
    UU_WLAN_ASSO_STA_MODE_QOS,    /* 802.11e */
    UU_WLAN_ASSO_STA_MODE_HT,     /* 802.11n */
    UU_WLAN_ASSO_STA_MODE_VHT,    /* 802.11ac */
} uu_wlan_asso_sta_mode_t;

#define BA_BASIC_BITMAP_BUF_SIZE   64

/* BA scoreboard structure - Both on Tx & Rx side */
typedef struct uu_wlan_ba_sb
{
   /*
   ** scoreboard bitmap.
   ** Bitmap is maintained for Basic BA, and the same can be used for compressed BA also.
   ** Window size of Score board is 64 SN(which is max reorder buffer, from spec).
   ** Each of these SNs need 16-bits, for status of 16 possible fragments.
   ** The score board is maintained in a circular buffer, whose start is 'buf_winstart'.
   ** The current scoreboad 'winstart' will be at position indicated by buf_winstart.
   ** 
   ** In transmission path, same array can hold status of transmitted frames.
   */
   uu_uint16  bitmap[BA_BASIC_BITMAP_BUF_SIZE];

   /* The sequence number of the starting frame that needs acknowledgment */
   uu_uint16  tx_frame_seq_no; /* 16 bits */

   /* Position of 'winstart' in the scoreboard bitmap buffer */
   uu_uint16  buf_winstart:10; /* 10 bits */
   uu_uint16  received:6; 

   /* 
   ** Received Bitmap.(when the STA is initiator of BA session and transmitted bunch 
   ** of MPDU's and received BA frame)
   ** Holds the bitmap received in BA frame and to be utilised by data unit <TBD> 
   ** either for re-transmissions/freeing the buffer
   */
   uu_uint16  ba_frame_bitmap[BA_BASIC_BITMAP_BUF_SIZE];

   /* The sequence number in the received BA frame */
   uu_uint16  ba_frame_seq_no; /* 16 bits */

   /*
   ** Start & End sequence numbers of frames in the reorder buffer (as seen by BA module).
   ** There will be always be a gap of 'winsize' between the winstart & winend.
   ** To handle wrap-around, doing wraparound of 'winend' only when 'winstart' wraps around.
   */
   uu_uint16  winstart; /* 16 bits */
   uu_uint16  winend; /* Always > winstart */ /* 16 bits */
   /* Size of negotiated reorder buffer (as seen by BA module), in terms number-of-frames */
   uu_uint16  winsize; /* 16 bits */
} uu_wlan_ba_sb_t;


typedef enum uu_wlan_ba_ses_type
{
    UU_WLAN_BA_SES_TYPE_IMMIDIATE = BIT(0),
    UU_WLAN_BA_SES_TYPE_IMPLICIT = BIT(1),
    UU_WLAN_BA_SES_TYPE_DELAYED = BIT(2)
} uu_wlan_ba_ses_type_t;

typedef struct uu_wlan_ba_ses_info
{
    uu_wlan_ba_ses_type_t    type;
    uu_wlan_ba_sb_t        sb;
} uu_wlan_ba_ses_info_t;

typedef struct  uu_wlan_ba_ses_context
{
    uu_wlan_ba_ses_info_t  ses;
} uu_wlan_ba_ses_context_t;


extern uu_wlan_ba_ses_context_t     uu_ba_table_g[UU_WLAN_MAX_BA_SESSIONS];


/*
** Remote station info. Filled during the association.
** On the STA device, this contains AP info.
** On the AP device, this contains info of associated station and separate object for each associated STA.
*/
typedef struct uu_wlan_asso_sta_info
{
    /* Byte 0 to Byte 3 */
    /* Block Ack context information. Contains references to 'ba_table_g' */
    uu_uint16   ba_cnxt[2][UU_WLAN_MAX_TID]; /* MAX_TID are possible in each direction (Tx & Rx) */

    /* Byte 4 to Byte 7 */
    /* BSSID. Used in IBSS case */
    uu_uchar  bssid[IEEE80211_MAC_ADDR_LEN]; /* 48 bits */
    /* Association ID */
    uu_uint16  aid : 16;

    /* Byte 8 to Byte 11  */
    uu_wlan_asso_sta_mode_t      mode : 2;
    /* station supported features and flags HT/VHT supports or not, smoothing and LDPC required or not */
    uu_wlan_asso_sta_flags_t     flags : 6;
    uu_wlan_ch_bndwdth_type_t    ch_bndwdth : 3;
    /* This info is required here, for control frames only */
    uu_uint16                    basic_mcs : 7;  /* 11n basic rate MCS number */
    /* added station_info for ampdu */
    uu_uchar ampdu_exponent : 2;
    uu_uchar ampdu_min_spacing : 3;
    uu_uint16 reserved : 9;

    /* Byte 12 to Byte 15 */
    /* Padding for 4-byte alignment */
    uu_uint32 padding_byte0;
} uu_wlan_asso_sta_info_t;

/* Remote STA Address */
typedef struct uu_sta_addr
{
    /* Block Ack context information. Contains references to 'ba_table_g' */
    uu_uint8 addr[IEEE80211_MAC_ADDR_LEN];
    uu_uint16 reserved;
}uu_wlan_sta_addr_index_t;

typedef struct uu_lmac_ops {
    uu_void (*umacCBK) (uu_int8 ind);
} uu_lmac_ops_t;


/* Station slots only 20 bits are used for station entry*/
extern uu_uint32 uu_wlan_sta_info_is_used_g;

/* BA slots only 40 bits are used for ba entry*/
extern uu_uint64 uu_wlan_ba_info_is_used_g;

/* Station address table, gives the station index(address) in the Station management table */
extern uu_wlan_sta_addr_index_t     uu_sta_addr_index_g[UU_WLAN_MAX_ASSOCIATIONS];

/* Station management table */
extern uu_wlan_asso_sta_info_t      uu_sta_info_table_g[UU_WLAN_MAX_ASSOCIATIONS];

/* BA management table */
extern uu_wlan_ba_ses_context_t     uu_ba_table_g[UU_WLAN_MAX_BA_SESSIONS];

/** Transmit frame status information of (8? build-time config) frames per AC.
 * Status information contains sequence number, status, retry.
 * After completion of transmition procedure of the frame, LMAC sets the corresponding bit.
 * In C model, UMAC has to reset this bit after reading the status information of transmitted frame. 
 */
extern uu_wlan_tx_status_info_t uu_wlan_tx_status_info_g[UU_WLAN_MAX_QID][UU_WLAN_MAX_TX_STATUS_TO_UMAC];
extern uu_uint32 uu_wlan_tx_status_flags_g[UU_WLAN_MAX_QID];

/** Received frame status information of 32 frames.
 * Each bit represents an individual received frame information.
 * After filling the whole frame in Rx buffer for UMAC, LMAC updates the corresponding bit in the status flags.
 * If bit is set, then its corresponding rx status is available for UMAC. 
 * In C model, UMAC has to reset this bit after reading the status information. 
 * If UMAC can't read in time, the LMAC may overwrite the old values after wrap-around.
 */
extern uu_uint32 uu_wlan_rx_status_flags_g;
extern uu_wlan_rx_status_info_t uu_wlan_rx_status_info_g[UU_WLAN_MAX_RX_STATUS_TO_UMAC];



/*
 * Tx Buffer related functions
 */

/* Padding to align to 4-byte boundary */
#define UU_WLAN_AGG_SUBFRAME_PADDED_BYTES(x) ((x&3)?(4-(x&3)):0)

/** This function is to get a available lmac Tx buffer length, from circular buffer of an AC
 * Return number of bytes available in buffer
 * @param[in] access category of the frame, 2 bits in length.
 */
extern uu_uint32 uu_wlan_tx_get_available_buffer(uu_uchar ac);

/** Writes the frame info and mpdu's into lmac memory
 * Return number of bytes available in buffer
 * @param[in] access category of the frame, 2 bits in length.
 * @param[in] array of UMAC addresses
 * @param[in] array of lengths
 */
extern uu_void uu_wlan_tx_write_buffer(uu_uchar ac, uu_uchar *adr[], uu_uint32 len[]);

/** Informs the completion of writing frame info and mpdu's into lmac memory
 * @param[in] access category of the frame, 2 bits in length.
 */
extern uu_void uu_wlan_tx_dma_done(uu_uchar ac);


#ifdef UU_WLAN_DFS /* TODO: Need thorough review. Do not use DFS for the time being */
/* ------------------------ SPECTRUM MEASUREMENT -------------------------- */

/**
 * As per the implementation, The measurement type defined below
 * acts as the communication between LMAC-UMAC.
 */

/** Refer to P802.11 REV_mb D12 Table 8.59 Pg 540 */
typedef enum uu_wlan_meas_type
{
    /** Basic, Spectrum Measurement */
    UU_WLAN_BASIC_MSR = 0,

    /** CCA, Spectrum Measurement */
    UU_WLAN_CCA_MSR = 1,

    /** Received Power Indicator Histogram, Spectrum Measurement */
    UU_WLAN_RPI_HISTOGRAM_MSR = 2,

    /** Channel Load, Radio Measurement */
    UU_WLAN_CHANNEL_LOAD_MSR = 3,

    /** Noise Histogram, Radio Measurement */
    UU_WLAN_NOISE_HISTOGRAM_MSR = 4,

    /** Beacon, Radio Measurement */
    UU_WLAN_BEACON_MSR = 5,

    /** Frame, Radio Measurement */
    UU_WLAN_FRAME_MSR = 6,

    /** STA statistics, Radio Measurement and WNM */
    UU_WLAN_STA_STATISTICS_MSR = 7,

    /** LCI, Radio Measurement, Spectrum Measurement and WNM */
    UU_WLAN_LCI_MSR = 8,

    /** Transmit stream/category measurement, Radio Measurement */
    UU_WLAN_TS_TC_MSR = 9,

    /** Multicast Diagnostics, WNM */
    UU_WLAN_MCAST_DIAG_MSR = 10,

    /** Location Civic, Radio Measurement and WNM */
    UU_WLAN_LOC_CIVIC_MSR = 11,

    /** Location Identifier, Radio Measurement and WNM */
    UU_WLAN_LOC_IDEN_MSR = 12,

    /** Reserved 13-254 */

} uu_wlan_meas_type_t;


/* These are defined as per the specification enum for measurement.
 * Since basic report is 0, so for no measurement it is assigned as 255.
 */
#define NO_STATS_MEASUREMENT    255

/* ------------------------- END OF SPECTRUM MEASUREMENT ---------------------------- */
#endif /* UU_WLAN_DFS */


/*
 * LMAC Init & Shutdown.
 */
extern uu_int32 uu_wlan_lmac_init(uu_void);
extern uu_int32 uu_wlan_lmac_shutdown(uu_void);
extern uu_int32 uu_wlan_lmac_start(uu_void);
extern uu_int32 uu_wlan_lmac_stop(uu_void);

extern uu_int32 uu_wlan_lmac_mode_switch(uu_void);
extern uu_int32 uu_wlan_lmac_config(uu_void* buf);


/** Callback of UMAC, called from LMAC to simulate interrupts (Rx frame, Tx status, Beacon trigger)
 * Called from LMAC with the following indications:
 * UU_LMAC_IND_RX_FRAME - Indicates umac to receive frame
 * UU_LMAC_IND_GEN_BEACON - Indicates umac to send beacon frame.
 * UU_LMAC_IND_TX_STATUS - For indicating tx status.
 */
extern uu_lmac_ops_t *lmac_ops_gp;
extern int uu_lmac_reg_ops(uu_lmac_ops_t *ops);
extern int uu_lmac_dereg_ops(void);

UU_END_DECLARATIONS

#endif /* __UU_WLAN_LMAC_IF_H__*/


