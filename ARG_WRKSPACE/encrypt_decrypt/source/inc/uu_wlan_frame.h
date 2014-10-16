/*******************************************************************************
 **                                                                           **
 ** File name :        uu_wlan_frame.h                                        **
 **                                                                           **
 ** Copyright © 2013, Uurmi Systems                                          **
 ** All rights reserved.                                                      **
 ** http://www.uurmi.com                                                      **
 **                                                                           **
 ** All information contained herein is property of Uurmi Systems             **
 ** unless otherwise explicitly mentioned.                                    **
 **                                                                           **
 ** The intellectual and technical concepts in this file are proprietary      **
 ** to Uurmi Systems and may be covered by granted or in process national     **
 ** and international patents and are protect by trade secrets and            **
 ** copyright law.                                                            **
 **                                                                           **
 ** Redistribution and use in source and binary forms of the content in       **
 ** this file, with or without modification are not permitted unless          **
 ** permission is explicitly granted by Uurmi Systems.                        **
 **                                                                           **
 ******************************************************************************/

#ifndef __UU_WLAN_FRAME_H__
#define __UU_WLAN_FRAME_H__

#include "uu_datatypes.h"
#include "uu_wlan_limits.h"
#include "uu_wlan_phy_if.h"


/** Bit operations */
#ifndef __KERNEL__
#define BIT(x)  (1ULL<<x)
#endif
#define TEST_BIT(arg, bit)      ((arg) & (1 << (bit)))
#define SET_BIT(arg, bit)      ((arg) |= (1 << (bit)))
#define MASK_BITS(msb, lsb)               (((1U << ((msb) - (lsb) + 1)) - 1) << (lsb))
#define INSERT_BITS(arg, msb, lsb, value) ((arg) = ((arg) & ~MASK_BITS(msb, lsb)) | (((value) << (lsb)) & MASK_BITS(msb, lsb)))
#define EXTRACT_BITS(arg, msb, lsb)       ((arg & MASK_BITS(msb, lsb)) >> (lsb))


/* bits count starts from 1(LSB) */
#define UU_WLAN_EXTRACT_BITS(x, bits_start, bits_len) ((x >> (bits_start - 1)) & ((1 << bits_len) - 1))

/** Version mask to get the version field from the frame's 1st byte */
#define IEE80211_FC0_VERSION_MASK        0x03
#define IEEE80211_FC0_VERSION_SHIFT        0
#define IEEE80211_FC0_VERSION_0          0x00

/** Mask to get the type of the frame. 
 * Type is one of the following.
 * Management
 * Control
 * Data 
 */
#define IEEE80211_FC0_TYPE_MASK          0x0c
#define IEEE80211_FC0_TYPE_SHIFT           2

/** To indicate management frame. 
 * Extract type field from the frame and compare with the following macro
 * to handle management frames.
 */
#define IEEE80211_FC0_TYPE_MGT           0x00

/** To indicate control frame. 
 * Extract type field from the frame and compare with the following macro
 * to handle control frames.
 */
#define IEEE80211_FC0_TYPE_CTL           0x04

/** To indicate data frame. 
 * Extract type field from the frame and compare with the following macro
 * to handle data frames.
 */
#define IEEE80211_FC0_TYPE_DATA          0x08

/** To indicate Invalid frame type (reserved frame type, in the standard). 
 * Used for special purpose INTERNALLY (for ex, to signify frame error for last MPDU in AMPDU).
 */
#define IEEE80211_FC0_TYPE_RESERVED      0x0C

/** Mask value to extract the subtype from the frame */
#define IEEE80211_FC0_SUBTYPE_MASK       0xf0
#define IEEE80211_FC0_SUBTYPE_SHIFT        4


/* Following are the sub-types of management frames */
/** Management frame sub-type is Beacon */
#define IEEE80211_FC0_SUBTYPE_BEACON     0x80
/** Management frame sub-type is Probe Request */
#define IEEE80211_FC0_SUBTYPE_PROBE_REQ  0x40
/** Management frame sub-type is Probe Response */
#define IEEE80211_FC0_SUBTYPE_PROBE_RESP 0x50
/** Management frame sub-type is 'Action' */
#define IEEE80211_FC0_SUBTYPE_ACTION     0xD0
/** Management frame sub-type is 'Action no-ack' */
#define IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK 0xE0


/* Following are the sub-type of control frame */
/** Control frame sub-type is Block ACK Request (BAR) */
#define IEEE80211_FC0_SUBTYPE_BAR        0x80
/** Control frame sub-type is Block ACK (BA) */
#define IEEE80211_FC0_SUBTYPE_BA         0x90
/** Control frame sub-type is PSPOLL */
#define IEEE80211_FC0_SUBTYPE_PSPOLL     0xa0
/** Control frame sub-type is RTS Frame */
#define IEEE80211_FC0_SUBTYPE_RTS        0xb0
/** Control frame sub-type is CTS Frame */
#define IEEE80211_FC0_SUBTYPE_CTS        0xc0
/** Control frame sub-type is ACK Frame */
#define IEEE80211_FC0_SUBTYPE_ACK        0xd0


/** Data frame sub-type is non-QoS Data */
#define IEEE80211_FC0_SUBTYPE_DATA       0x00
/** Data frame sub-type is non-QoS null (no data) */
#define IEEE80211_FC0_SUBTYPE_DATA_NULL  0x40
/** Data frame sub-type is QoS Data */
#define IEEE80211_FC0_SUBTYPE_QOS        0x80
/** Data frame sub-type is NULL Data */
#define IEEE80211_FC0_SUBTYPE_QOS_NULL   0xC0



/** Checks whether the frame is RTS frame or not */
#define IEEE80211_IS_FC0_RTS(mpdu) (mpdu[0] == (IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_RTS))

/** Checks whether the frame is CTS frame or not */
#define IEEE80211_IS_FC0_CTS(mpdu) (mpdu[0] == (IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_CTS))

/** Checks whether the frame is ACK frame or not */
#define IEEE80211_IS_FC0_ACK(mpdu) (mpdu[0] == (IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_ACK))

/** Checks whether the frame is PSPOLL or not */
#define IEEE80211_IS_FC0_PSPOLL(mpdu)  (mpdu[0] == (IEEE80211_FC0_TYPE_CTL | IEEE80211_FC0_SUBTYPE_PSPOLL))

/** Checks whether the frame contains QoS data or not */
#define IEEE80211_IS_FC0_QOSDATA(mpdu) (mpdu[0] == (IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_QOS))

/** Checks whether the frame is Management frame or not */
#define IEEE80211_IS_MGT_FRAME(mpdu) ((mpdu[0] & IEEE80211_FC0_TYPE_MASK) == IEEE80211_FC0_TYPE_MGT)

/** Checks whether the frame is Beacon frame or not */
#define IEEE80211_IS_BEACON_FRAME(mpdu) (mpdu[0] == (IEEE80211_FC0_TYPE_MGT | IEEE80211_FC0_SUBTYPE_BEACON))

/** Checks whether the frame is Probe response frame or not */
#define IEEE80211_IS_PRBRESP_FRAME(mpdu) (mpdu[0] == (IEEE80211_FC0_TYPE_MGT | IEEE80211_FC0_SUBTYPE_PROBE_RESP))

/* ctrl fields of MAC header of control frame */
/** Mask value to get the sub-fields of Frame Control (FC) - first 2 bytes of MPDU */
#define IEEE80211_FC1_DIR_MASK           0x03
#define IEEE80211_FC1_DIR_NODS           0x00    /* STA->STA */

#define IEEE80211_FC1_DIR_TODS           0x01    /* STA->AP  */
#define IEEE80211_FC1_DIR_FROMDS         0x02    /* AP ->STA */
#define IEEE80211_FC1_DIR_DSTODS         0x03    /* AP ->AP  */

#define IEEE80211_FC1_MORE_FRAG          0x04
#define IEEE80211_FC1_RETRY              0x08
#define IEEE80211_FC1_PWR_MGT            0x10
#define IEEE80211_FC1_MORE_DATA          0x20
#define IEEE80211_FC1_PROTECTED          0x40
#define IEEE80211_FC1_WEP                0x40    /* pre-RSNA compat */
#define IEEE80211_FC1_ORDER              0x80

#define IEEE80211_IS_FRAME_TODS(mpdu)       (mpdu[1] & IEEE80211_FC1_DIR_TODS)
#define IEEE80211_IS_FRAME_4ADDR(mpdu)      (mpdu[1] & IEEE80211_FC1_DIR_DSTODS)
#define IEEE80211_IS_FC1_MOREFRAG_SET(mpdu) (mpdu[1] & IEEE80211_FC1_MORE_FRAG)
#define IEEE80211_IS_FC1_MOREDATA_SET(mpdu) (mpdu[1] & IEEE80211_FC1_MORE_DATA)

#define IEEE80211_MAC_ADDR_LEN       6


/** Lengths of various frames */
/** RTS Frame length */
#define UU_RTS_FRAME_LEN                     20
/** CTS Frame length */
#define UU_CTS_FRAME_LEN                     14
/** ACK Frame length */
#define UU_ACK_FRAME_LEN                     14
/** Block ACK request Frame length */
#define UU_BAR_FRAME_LEN                     24
/** BA Frame length */
#define UU_BA_FRAME_LEN                      32
/** BA basic Frame length */
#define UU_BA_BASIC_FRAME_LEN               152
/** BA compressed Frame length */
#define UU_BA_COMPRESSED_FRAME_LEN           32
/** PS POLL frame length */
#define UU_PSPOLL_FRAME_LEN                  20
/** QOS NULL data frame length */
#define UU_QOS_NULL_FRAME_LEN                30
/** Minimum length of the data packet */
#define UU_MIN_DATA_PKT_LEN                  26
/** Minimum length of the management packet */
#define UU_MIN_MGT_PKT_LEN                   28
/** MPDU delimiter length */
#define UU_WLAN_MPDU_DELIMITER_LEN            4
/** Minimum 80211 frame length */
#define UU_MIN_80211_FRAME_LEN               14

/* Gives the minimum value of Tx/Rx frame len or uu_min_data_pkt len */
#define UU_WLAN_CAP_MIN_FRAME_HEADER_LEN(x)      (((x) < UU_MIN_DATA_PKT_LEN) ? (x) : UU_MIN_DATA_PKT_LEN)

/** Get the frame length from Rxed frame rxvector */
#define UU_WLAN_GET_FRAME_LEN_FROM_RX_VEC(rxvec)     ((rxvec.format == UU_WLAN_FRAME_FORMAT_NON_HT) ? rxvec.L_length : rxvec.psdu_length)

/** powersave related */
#define UU_BEACON_HDR_SIZE      24     /*refer section 8.3.3.2 */
#define UU_TIM_IE_ID            5      /*refer section 8.4.2 */
#define UU_TIME_STAMP_SIZE      8      /*refer section 8.4.1.10 */
#define UU_BEACON_INTRVL_SIZE   2      /*refer section 8.4.1.3 */
#define UU_CAPABILITY_INFO_SIZE 2      /*refer section 8.4.1.4 */

/** OFFSETs */
#define  UU_BAR_FRAME_CONTROL_FIELD_OFFSET  16
#define  UU_QOS_OFFSET                      24
#define  UU_4ADDR_QOS_OFFSET                30
#define  UU_TA_OFFSET                       10
#define  UU_RA_OFFSET                        4
#define  UU_BSSID_OFFSET                    16
#define  UU_TID_OFFSET                      24
#define  UU_4ADDR_TID_OFFSET                30
#define  TID_SHIFT                           4
#define  UU_SCF_OFFSET                      22
#define  UU_BAR_FC_FIELD_OFFSET             16
#define  UU_BA_BITMAP_OFFSET                20
#define  UU_SSN_FIELD_OFFSET                18

/** BAR type */
#define UU_WLAN_BAR_TYPE_BASIC_BA          0x00
#define UU_WLAN_BAR_TYPE_RESRVD_BA         0x01 /* Reserved in standard */
#define UU_WLAN_BAR_TYPE_COMP_BA           0x02
#define UU_WLAN_BAR_TYPE_MUL_TID_BA        0x03

/** Macros to get frame header fields */
/** Get the first byte of the frame control */
#define UU_WLAN_GET_FC0_VALUE(x)              (x[0])
/** Get the second byte of the frame control */
#define UU_WLAN_GET_FC1_VALUE(x)              (x[1])
#define UU_WLAN_GET_BAR_CF_VALUE(x)           (x[16])
/** Get the type of BAR */
#define UU_WLAN_GET_BAR_TYPE_VALUE(x)         ((x[16]& 0x06) >>1)
#define UU_WLAN_GET_BAR_TID_VALUE(x)          (x[17] >> 4)
#define UU_WLAN_GET_BA_TID_VALUE(x)           UU_WLAN_GET_BAR_TID_VALUE(x)
#define UU_WLAN_GET_QOS_TID_VALUE(x)          (x[24] & 0x0f)
#define UU_WLAN_GET_4ADDR_QOS_TID_VALUE(x)    (x[30] & 0x0f)
#define UU_WLAN_GET_QOS_EOSP_VALUE(x)         (x[24] & 0x10)
#define UU_WLAN_GET_QOS_ACKPOLICY_VALUE(x)    ((x[24] << 1) >> 6)
#define UU_WLAN_GET_4ADDR_QOS_ACKPOLICY_VALUE(x)  ((x[30] << 1) >> 6)
#define UU_WLAN_GET_BAR_ACKPOLICY_VALUE(x)    (x[UU_BAR_FRAME_CONTROL_FIELD_OFFSET] & 0x01)



/** This is division operation to be done for duration calculation */
#define DIV_CEIL(x, y)           (((x) + ((y)-1)) / (y))
#define DIV_CEIL_EVEN(x, y)      (2*(((x) + ((y)*2-1)) / ((y)*2)))


#define UU_WLAN_GET_FRAME_DURATION(x)          (*(uu_uint16*)(&x[2]))
#define UU_WLAN_GET_PSPOLL_FRAME_DURATION(x)        DIV_CEIL(UU_PSPOLL_FRAME_LEN, x)
#define UU_WLAN_GET_PSPOLL_FRAME_DURATION_REM(x)    (UU_PSPOLL_FRAME_LEN % x)

/** Timestamp bit for the Beacon frame */
#define IEEE80211_IS_BEACON_TIMESTAMP               24
#define IEEE80211_IS_BEACON_CAPABILITY_IBSS(x)      ((x[34] & 0x20))

/** Convert 10 ns to us. Same procedure is followed for the duration calculation.
 * This number represents 1/100.
 * Process of getting this:
 * uint32 time_10ns, time_us;
 * uint64_t mul = 0xFFFFFFFFFF;
 * uint64_t one_hundredth = mul/100;
 * time_us = (one_hundredth * time_10ns) >> 40; This is the result expected.
 * Note: mul/100 = 10995116277ULL
 */

#define UU_WLAN_10NS_TO_US_MUL          10995116277ULL

#define UU_WLAN_10NS_TO_US(x)           ((UU_WLAN_10NS_TO_US_MUL * (x)) >> 40)

/** Normal Ack or Implicit Block Ack Request.
 * Applicable for all non-QoS Data & Management frames addressed to us.
 * May be used for QoS Data, BAR-delayed, and BA frames - in QoS header.
 */
#define UU_WLAN_ACKPOLICY_NORMAL        0
/** Ack Policy is - No ACK.
 * The addressed recipient takes no action upon receipt of the frame.
 * Set in individually addressed frames in which the sender does not require acknowledgment.
 * Also used for group addressed frames that use the QoS frame format.
 */
#define UU_WLAN_ACKPOLICY_NO_ACK        1
/** Ack Policy is - No explicit acknowledgment or PSMP Ack. */
#define UU_WLAN_ACKPOLICY_PSMP_ACK      2
/** Ack Policy is - Block Ack.
 * Recipient takes no action upon the receipt of the frame except for recording the state.
 * Recipient can expect a BlockAckReq frame in the future to which it responds.
 */
#define UU_WLAN_ACKPOLICY_BA            3


#define UU_WLAN_BA_BASIC_BITMAP_SZ    128
#define UU_WLAN_MAX_TID                 8
#define UU_COMP_BA_BITMAP_SZ            8
#define UU_BA_BITMAP_SZ               128

#define MPDU_SEQN_MASK             0x0FFF

#define UU_WLAN_MAX_MPDU_IN_AMPDU       8

/** Checks */
#define UU_WLAN_IS_FRAME_RA_MULTICAST(frame)  isMulticastAddr(frame+UU_RA_OFFSET)


//#define UU_WLAN_MAX_SUPPORTED_RATES       2


/* Access Category types */
#define UU_WLAN_AC_BE                    0x00
#define UU_WLAN_AC_BK                    0x01
#define UU_WLAN_AC_VI                    0x02
#define UU_WLAN_AC_VO                    0x03
#ifdef UU_WLAN_BQID
#define UU_WLAN_BEACON_QID               0x04  /* Make QID of AC as 4 */
#else
#define UU_WLAN_BEACON_QID               UU_WLAN_AC_VO
#endif

typedef struct uu_80211_rts_frame {
    unsigned char    fc[2];
    unsigned char    dur[2];
    unsigned char    ra[IEEE80211_MAC_ADDR_LEN];
    unsigned char    ta[IEEE80211_MAC_ADDR_LEN];
    unsigned char   fcs[4];  /* FCS */
} uu_80211_rts_frame_t;

/** contains the fields of a  3-address QoS null data frame of size 30 bytes */
typedef struct uu_80211_qos_null_frame {
    unsigned char    fc[2];
    unsigned char    dur[2];
    unsigned char    add1[IEEE80211_MAC_ADDR_LEN];
    unsigned char    add2[IEEE80211_MAC_ADDR_LEN];
    unsigned char    add3[IEEE80211_MAC_ADDR_LEN];
    unsigned char    seq_ctl[2];
    unsigned char    qos_ctl[2];
    unsigned char    fcs[4];  /* FCS */
} uu_80211_qos_null_frame_t;

/** contains the fields of a PS-Poll frame of size 20 bytes */
typedef struct uu_80211_pspoll_frame {
    unsigned char    fc[2];
    unsigned char    aid[2];
    unsigned char    ra[IEEE80211_MAC_ADDR_LEN];
    unsigned char    ta[IEEE80211_MAC_ADDR_LEN];
    unsigned char    fcs[4];  /* FCS */
} uu_80211_pspoll_frame_t;

#if 0
typedef enum uu_wlan_pkt_type
{
    UU_WLAN_PKT_TYPE_NORMAL = 0,
    UU_WLAN_PKT_TYPE_BEACON,
    UU_WLAN_PKT_TYPE_PROBE_RESP,
    /*
       UU_WLAN_PKT_TYPE_ATIM,
       UU_WLAN_PKT_TYPE_PSPOLL,
       UU_WLAN_PKT_TYPE_CHIRP,
       UU_WLAN_PKT_TYPE_GRP_POLL,
     */
} uu_wlan_pkt_type_t;
#endif


/** Encryption key type, 2 bits in length */
typedef enum uu_wlan_key_type
{
    UU_WLAN_KEY_TYPE_CLEAR,
    UU_WLAN_KEY_TYPE_WEP,
    UU_WLAN_KEY_TYPE_AES,
    UU_WLAN_KEY_TYPE_TKIP,
} uu_wlan_key_type_t;



/*
 * Common Frame info structure, with common details of Tx & Rx sides.
 * Structure uses bit-fields, and aligned to 4-byte boundary.
 * Total structure size is: 12 bytes
 * TODO: Most of these are relavent for only Tx-Frame-Info.  Reorganization is required.
 */
typedef struct uu_frame_details
{
    /* Bytes 1, 2 & 3 */
    /* Length of the MPDU or AMPDU.
    ** If the frame is an MPDU, contains the length of MPDU in bytes.
    ** If AMPDU, it is the total length of AMPDU with - Data, Padding & Delimiters.
    ** Note that on Rx side, MPDUs are given to UMAC individually.
    */
    uu_uint32  framelen:20;       /* 20 bits */
    //uu_wlan_pkt_type_t  fType:3;  /* 3 bits */

    /* This bit is set to 1, to represent whether the frame is beacon or not */
    uu_uint8 beacon:1;
    uu_uint8   reserved1:3; /* 1 bit reserved */

    /* Byte 4 */
    uu_uint8   is_ampdu:1;  /* 1 bit */
    uu_uint8   retry:1;     /* 1 bit */
    uu_uint8   reserved2:6; /* 6 bits reserved */

    /* Byte 5 */
#if 0 /* Disabled. Updating the RTL buffer is difficult. Now maintained as global variables (registers) in Tx and are posted to UMAC along with Tx-status */
    uu_uint8   ShortRetryCount:4; /* 4 bits */
    uu_uint8   LongRetryCount:4;  /* 4 bits */
#else
    uu_uint8   reserved4; /* 8 bits */
#endif

    /* Byte 6 */
    uu_wlan_key_type_t  keytype:2; /* 2 bits */
    uu_uint8   keyix:6; /* 6 bits */

    /* Byte 7 & 8 are reserved */
    uu_uint16  reserved3; /* 16-bits reserved */
    //uu_uint16  cookie; /* to match the Tx status */

} uu_frame_details_t;


/** Aggregation info attached to AMPDU, while sending from UMAC to LMAC.
** This is added after 'tx_frame_info' and before first MPDU, in the AMPDU.
** Contains the per-MPDU details, as 'tx_frame_info' is common for all MPDUs in the AMPDU.
 * Total structure size is: 256  bytes
*/
typedef struct uu_wlan_ampdu_info
{
    struct {
        /** Mpdu length
         * Include FCS. Does NOT include AMPDU fields & padding
         */
        uu_uint16  mpdu_len:14; /* Include FCS. Does NOT include AMPDU fields & padding */

        uu_uint16  pad_len:2; /* Number of padding bytes, after the MPDU */

        uu_uint16  reserved2:16;
    } mpdu_info[UU_WLAN_MAX_MPDU_IN_AMPDU];
} uu_wlan_ampdu_info_t;


/*
 * Frame info structure, on the Tx side.
 * Structure uses bit-fields, and aligned to 4-byte boundary.
 * Total structure size is: 88 bytes excluding the actual frame length.
 * In aggregation case (AMPDU),
 *   - 'aggr_count' > 0
 *   - 'ampdu_info' will be filled with details of MPDUs in the AMPDU.
 *   - Actual MPDU starts after 'ampdu_info'
 *   - This 'tx_frame_info' and 'amdpu_info' are added just once for entire AMPDU.
 */
typedef struct uu_wlan_tx_frame_info
{
    /** TxVector - required in MAC-PHY interface */
    uu_wlan_tx_vector_t txvec; /* Size: 32 bytes */

    /* Common info, for both Tx & Rx */
    uu_frame_details_t  frameInfo; /* Size: 12 bytes */

    /* Bytes 1 & 2, after Common frame-info */

    uu_uint16  reserved0; /* all the extra bits in descriptor ID are reserved */

    /* Byte 3, after Common frame-info */

#ifdef UU_WLAN_BQID /* Enhancements, yet to be carried to RTL */
    /** Access Catagory of frame.
     * Actually, this is qid. Need 3-bits, considering Beacon Q and 4 AC Queues
     */
    uu_uint8   ac:3; /* 3 bits */

    /** added for padding */
    uu_uint8  reserved1:5; /* 6 bits */
#else /* This is what is implemented in RTL */
    /** Access Catagory of frame */
    uu_uint8   ac:2; /* 2 bits */

    /** added for padding */
    uu_uint8  reserved1:6; /* 6 bits */
#endif

    /*
    ** For fallback
    */

    /* Byte 4, after Common frame-info */
    uu_uint8   stbc_fb:2; /* 2 bits */
    uu_uint8   n_tx_fb:3; /* 3-bits */
    uu_uint8   reserved2:3; /* 3-bits reserved for alignment & for future use */

    /* Byte 5, after Common frame-info */
    uu_uint8   mcs_fb:7;  /* 7 bits */
    uu_uint8   reserved3:1; /* 3-bits reserved for alignment & for future use */


    /* Byte 6, after Common frame-info */

    /* Fallback data rate of frame to be sent to air. The original rate (b4 fallback) in the 'txvec' will be tried first */
    uu_uint8   fallback_rate:4; /* 4-bits (as only 4-bits used in MAC-PHY i/f) */
    uu_uint8   reserved4:4; /* 4-bits reserved alignment & for future use */

    /* Byte 7, after Common frame-info */

    /*
    ** These are applicable & filled for only the Data/Management frames.
    ** Not filled / applicable for control frames.
    ** Used for RTS/self-CTS, before sending the data/management frame having this.
    ** 'rtscts_rate_fb' is the fallback rate for RTS/self-CTS, for data/management frame having this.
    */
    uu_uint8   rtscts_rate:4; /* 4 bits (as only 4-bits used in MAC-PHY i/f) */
    /* Fallback rate for 'rtscts_rate' */
    uu_uint8   rtscts_rate_fb:4; /* 4 bits */

    /* Byte 8, after Common frame-info */
    /** Number of MPDUs aggregated in this AMPDU. Filled only for 1st frame of AMPDU.
     * Set to 0, for non-AMPDU frames and also for subsequent frames of MPDU.
     * In 11ac, AMPDU is mandatory even to send MPDU. In that case, it is always non-zero.
     * Valid values are 0 to Max-MPDU-in-AMPDU.
     */
    uu_uint8   aggr_count;

    /* Bytes 9, 10, 11 & 12, after Common frame-info */
    /* For VHT maximum users are 4. */
    uu_uint8   num_sts_fb[4]; /* Total 32-bits (8-bits each). 3-bits value, 5-bits reserved */

    /* Bytes 13 onwards. It is 32 bytes, with UU_WLAN_MAX_MPDU_IN_AMPDU of 8 */
    uu_wlan_ampdu_info_t  ampdu_info; /* Filled only if 'aggr_count' is > 0 */
    /* 88 byte (32-bytes txvec, 12-bytes common frameInfo, 12 bytes of tx-frame info, 32-byte of ampdu info), until this */

    /* Actual frame to send to air. Can be upto the available hardware buffer */
    uu_uchar    mpdu[]; /* Length can be anything */
} /* __attribute__ ((packed)) */ uu_wlan_tx_frame_info_t;


/*
 * Frame info structure, on the Rx side.
 * Structure uses bit-fields, and aligned to 4-byte boundary.
 * Total frame info size except MPDU is 32 bytes.
 */
typedef struct uu_wlan_rx_frame_info
{
    /** RxVector - required in MAC-PHY interface */
    uu_wlan_rx_vector_t  rxvec; /* Size: 20 bytes */

    /* Common info, for both Tx & Rx */
    uu_frame_details_t   frameInfo; /* Size: 12 bytes */

#ifdef UU_WLAN_TSF
    /* Byte 33 - 40 */
    /** Stores the Local timestamp when the first symbol
     * of the frame was received in the MAC-PHY i/f.
     * This parameter will be decremented by the rx_start_of_frame_offset
     * received in the RXVECTOR of the received frame, and is passed to the
     * mactime in ieee80211_rx_status structure with the flag as RX_FLAG_MACTIME_START.
     * This timestamp will be used in the framework for synchronizing the parameters needed for IBSS.
     * Here no need of considering the beacon timestamp's offset,
     * because this will be taken care by the framework and will be added appropriately.
     */
    uu_uint32  timestamp_msb; /* 32 bits */
    uu_uint32  timestamp_lsb; /* 32 bits */

    /* Byte 41-48 */
    /** Stores the Local timestamp when the beacon timestamp
     * first octet was received in the MAC-PHY i/f.
     * This is required for comparing the Local timestamp with the adjusted received timestamp
     * for synchronization purpose after receiving the beacon frame in the IBSS mode.
     * The process of synchronizing and updating the Local TSF is explained in
     * the function uu_wlan_rx_frame_handler in RX module. */
    uu_uint32   becn_timestamp_msb_tsf; /* 32 bits */
    uu_uint32   becn_timestamp_lsb_tsf; /* 32 bits */
#endif
    /* Frame received from air */
    uu_uchar    mpdu[]; /* Length can be anything */
}/* __attribute__ ((packed)) */ uu_wlan_rx_frame_info_t;



#endif /* __UU_WLAN_FRAME_H__ */

/* EOF */


