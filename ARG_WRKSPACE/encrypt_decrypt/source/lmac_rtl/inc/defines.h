/*-------------Macro Definitions-------------*/

// Clock Edge
`ifdef POS_RST
  `define EDGE_OF_RESET posedge rst_n
  `define POLARITY_OF_RESET rst_n
`else 
  `define EDGE_OF_RESET negedge rst_n
  `define POLARITY_OF_RESET !rst_n
`endif

// Channel Status
`define  UU_WLAN_CH_IDLE                               3'h0
`define  UU_WLAN_BUSY_CH_PRIMARY                       3'h1
`define  UU_WLAN_BUSY_CH_SECONDARY20                   3'h2
`define  UU_WLAN_BUSY_CH_SECONDARY40                   3'h3
`define  UU_WLAN_BUSY_CH_SECONDARY80                   3'h4

// Fallback Retry Counts
`define  UU_WLAN_SHORT_FB_RETRY_COUNT                  4'h3
`define  UU_WLAN_LONG_FB_RETRY_COUNT                   4'h2  

//loc_defines for CP Module
`define SUBTYPE_ERROR                                 4'h0
`define RTS_PKT                                       4'h1
`define CTS_PKT                                       4'h2
`define ACK_PKT                                       4'h3
`define BAR_PKT                                       4'h4
`define BA_PKT                                        4'h5
`define DATA_ACK_PKT                                  4'h6
`define DATA_BA_PKT                                   4'h7
`define DATA_IMBA_PKT                                 4'h8
`define MGT_PKT                                       4'h9
`define SELF_CTS_PKT                                  4'hA
`define RESERVED                                      4'hF

//zero_byte
`define ZERO_BYTE                                      8'h00

// Self CTS Protection
`define UU_WLAN_SELF_CTS_R                             1'b0

/* short retry count for retransmission */
`define  UU_WLAN_DOT11_SHORT_RETRY_COUNT_R             8'h8
/* long retry count for retransmission */
`define  UU_WLAN_DOT11_LONG_RETRY_COUNT_R              8'h8

// Channel Bandwidth
`define  CBW20                                         3'h0
`define  CBW40                                         3'h1
`define  CBW80                                         3'h2
`define  CBW160                                        3'h3
`define  CBW80_80                                      3'h4

/* NON_HT indicates Clause 15, Clause 17, Clause 18, or Clause 19
   PPDU formats or non-HT duplicated PPDU format. In this case, the
   modulation is determined by the NON_HT_MODULATION
   parameter. */
`define  UU_WLAN_FRAME_FORMAT_NON_HT                   2'h0
/* HT_MF indicates HT-mixed format. */
`define  UU_WLAN_FRAME_FORMAT_HT_MF                    2'h1
/* HT_GF indicates HT-greenfield format. */
`define  UU_WLAN_FRAME_FORMAT_HT_GF                    2'h2
/* 802.11AC changes */
/* VHT indicates VHT format. */
`define  UU_WLAN_FRAME_FORMAT_VHT                      2'h3

// RX Frame Responses from CP and RX
`define UU_WLAN_RX_HANDLER_FRAME_ERROR                 32'h0000_0FF0
`define UU_WLAN_RX_HANDLER_SEND_RESP                   32'h0000_0FF1
`define UU_WLAN_RX_HANDLER_NAV_UPDATE                  32'h0000_0FF2
`define UU_WLAN_RX_HANDLER_ACK_RCVD                    32'h0000_0FF3
`define UU_WLAN_RX_HANDLER_CTS_RCVD                    32'h0000_0FF4
`define UU_WLAN_RX_HANDLER_BA_RCVD                     32'h0000_0FF5
`define UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS   32'H0000_0FF6
`define UU_WLAN_RX_HANDLER_FRAME_INVALID               32'H0000_0FF7

// RX Frame Destination
`define UU_WLAN_RX_FRAME_DEST_BROADCAST                3'h0 
`define UU_WLAN_RX_FRAME_DEST_MULTICAST                3'h1
`define UU_WLAN_RX_FRAME_DEST_US                       3'h2
`define UU_WLAN_RX_FRAME_DEST_OTHERS                   3'h3
`define UU_WLAN_RX_FRAME_DEST_LOOPBACK                 3'h4

// RX Packet Status
`define UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME         3'h2
`define UU_LMAC_RX_RET_INCOMPLETE_AMPDU_SUBFRAME       3'h3
`define UU_LMAC_RX_RET_NON_AGG_FRAME                   3'h4

// RX Error Status
`define UU_WLAN_RX_END_STAT_NO_ERROR                   2'h0 
`define UU_WLAN_RX_END_STAT_FORMAT_ERROR               2'h1     
`define UU_WLAN_RX_END_STAT_CARRIER_ERROR              2'h2      
`define UU_WLAN_RX_END_STAT_RATE_ERROR                 2'h3   

// RX Memory Offsets
`define RX_MEM_BASE_OFFSET                             12'h0
`define RX_MEM_MPDU_OFFSET                             12'h23
`define RX_MEM_AC_OFFSET                               12'h22
`define RX_MEM_FRAME_INFO_OFFSET                       12'h14

// Version
`define IEEE80211_FC0_VERSION_0                        8'h00

//Type
`define IEEE80211_FC0_TYPE_MASK                        8'h0C
`define IEEE80211_FC0_TYPE_MGT                         8'h00
`define IEEE80211_FC0_TYPE_CTL                         8'h04
`define IEEE80211_FC0_TYPE_DATA                        8'h08
`define IEEE80211_FC0_TYPE_RESERVED                    8'h0C

//Sub Type MGT
`define IEEE80211_FC0_SUBTYPE_BEACON                   8'h80
`define IEEE80211_FC0_SUBTYPE_PROBE_REQ                8'h40
`define IEEE80211_FC0_SUBTYPE_PROBE_RESP               8'h50
`define IEEE80211_FC0_SUBTYPE_ACTION                   8'hD0
`define IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK            8'hE0

//Sub Type CTRL
`define IEEE80211_FC0_SUBTYPE_BAR                      8'h80
`define IEEE80211_FC0_SUBTYPE_BA                       8'h90
`define IEEE80211_FC0_SUBTYPE_PSPOLL                   8'hA0
`define IEEE80211_FC0_SUBTYPE_RTS                      8'hB0
`define IEEE80211_FC0_SUBTYPE_CTS                      8'hC0
`define IEEE80211_FC0_SUBTYPE_ACK                      8'hD0

//Sub Type Data
`define IEEE80211_FC0_SUBTYPE_DATA                     8'h00
`define IEEE80211_FC0_SUBTYPE_DATA_NULL                8'h40
`define IEEE80211_FC0_SUBTYPE_QOS                      8'h80
`define IEEE80211_FC0_SUBTYPE_QOS_NULL                 8'hC0

//Frame Lengths
`define UU_RTS_FRAME_LEN                               8'd20
`define UU_CTS_FRAME_LEN                               8'd14
`define UU_ACK_FRAME_LEN                               8'd14
`define UU_BAR_FRAME_LEN                               8'd24
`define UU_BA_FRAME_LEN                                8'd32
`define UU_BA_BASIC_FRAME_LEN                          8'd152
`define UU_BA_COMPRESSED_FRAME_LEN                     8'd32
`define UU_PSPOLL_FRAME_LEN                            8'd20
`define UU_MIN_DATA_PKT_LEN                            8'd26
`define UU_MIN_MGT_PKT_LEN                             8'd28
`define UU_WLAN_MPDU_DELIMITER_LEN                     8'd4
`define UU_MIN_80211_FRAME_LEN                         8'd14

//Limit
`define UU_WLAN_MAX_BA_SESSIONS                        6'd40

//ACK_POLICY
`define UU_WLAN_ACKPOLICY_NORMAL                       2'd0
`define UU_WLAN_ACKPOLICY_NO_ACK                       2'd1
`define UU_WLAN_ACKPOLICY_PSMP_ACK                     2'd2
`define UU_WLAN_ACKPOLICY_BA                           2'd3

// BAR type
`define UU_WLAN_BAR_TYPE_BASIC_BA                  2'h0
`define UU_WLAN_BAR_TYPE_RESRVD_BA                 2'h1 /* Reserved in standard */
`define UU_WLAN_BAR_TYPE_COMP_BA                   2'h2
`define UU_WLAN_BAR_TYPE_MUL_TID_BA                2'h3

// BA Session Status
`define UU_BA_SESSION_INVALID                  32'hFF0FEFFF //(negative value)

// BA Session Information
`define BA_SEQN_RANGE_BY2                      16'd2048
`define BA_COMP_BITMAP_SIZE                    16'd8
`define BA_BASIC_BITMAP_BUF_SIZE               16'd64
`define BA_BASIC_BITMAP_BUF_MASK               16'h003F
`define BA_MPDU_SEQN_MAX                       16'd4095
`define BA_MPDU_SEQN_WIDTH                     16'd12
`define BA_BITMAP_MEM_SIZE                     16'd32

// Sequence Number Mask
`define MPDU_SEQN_MASK                                 16'h0FFF

// Packet Status
`define UU_FAILURE                                     32'hFFFF_FFFF
`define UU_SUCCESS                                     32'h0000_0000
`define UNKNOWN_FRAME                                  32'h00F0_4003
`define ERROR                                          8'h00

//Data Rates Supported
`define UU_RATE_1M_BIT                                 4'd0
`define UU_RATE_2M_BIT                                 4'd1 
`define UU_RATE_5M5_BIT                                4'd2 
`define UU_RATE_11M_BIT                                4'd3 
`define UU_RATE_6M_BIT                                 4'd4 
`define UU_RATE_9M_BIT                                 4'd5 
`define UU_RATE_12M_BIT                                4'd6 
`define UU_RATE_18M_BIT                                4'd7 
`define UU_RATE_24M_BIT                                4'd8 
`define UU_RATE_36M_BIT                                4'd9 
`define UU_RATE_48M_BIT                                4'd10 
`define UU_RATE_54M_BIT                                4'd11
`define UU_LEGACY_RATES_BIT                            4'd12

// Data Rate Defaults
`define UU_HW_RATE_1M                          8'h8a
`define UU_HW_RATE_2M                          8'h94
`define UU_HW_RATE_5M5                         8'hb7
`define UU_HW_RATE_11M                         8'hee
`define UU_HW_RATE_6M                          8'h0b 
`define UU_HW_RATE_9M                          8'h0f 
`define UU_HW_RATE_12M                         8'h0a 
`define UU_HW_RATE_18M                         8'h0e 
`define UU_HW_RATE_24M                         8'h09 
`define UU_HW_RATE_36M                         8'h0d 
`define UU_HW_RATE_48M                         8'h08 
`define UU_HW_RATE_54M                         8'h0c 

/** Supported number of rates for CCK */ 
`define UU_SUPPORTED_CCK_RATE                          4'h4

/** Supported number of rates for OFDM */
`define UU_SUPPORTED_OFDM_RATE                         4'h8
`define UU_OFDM_MODULATION_TYPE_IND                    8'h03

// Supported MCS
`define UU_MAX_MCS_N_SUP                               16 

// Supportes Streams
`define UU_MAX_STREAM_HT                       4

// self defined Macros
`define SGI_OFFSET                       7'h0F
`define SGI_HT_OFFSET                           7'h0F
`define DOT11_SIGNAL_EXTENSION                           4'd0    //***UMAC statistics--for2.5GHz
`define UU_WLAN_FRAME_FORMAT                        0 //***cosider changes from UU 
`define UU_ONE_BY_RATE_RESOLUTION                      40

// VHT Parameters
`define UU_VHT_NSS_FROM_MCS                            8'h07
`define UU_VHT_MCSINDEX_FROM_MCS                       8'h0f
`define UU_VHT_MCSINDEX_SHIFT_FROM_MCS                 4'd3
`define UU_VHT_MCS_INDEX_MAXIMUM                       4'd10
`define UU_VHT_MCSINDEX_FIELD                          8'h78

// Modulation Schemes Supported
`define UU_OFDM_BPSK                                   3
`define UU_OFDM_QPSK                                   2
`define UU_OFDM_16QAM                                  1
`define UU_OFDM_64QAM                                  0

// PHY Parameters
`define PHY_PREAMBLE_TIME                              8'd16
`define PHY_SIGNAL_TIME                                8'd4
`define PHY_SYMBOL_TIME                                8'd4    
`define PHY_SERVICE_NBITS                              8'd16
`define PHY_TAIL_NBITS                                 8'd6
`define OFDM_SIGNAL_EXTENSION                          8'd6
`define PHY_PLCP_SHORT_TIME                            8'd96
`define PHY_PLCP_TIME                                  8'd192
`define PHY_HT_SIG                                     8'd8
`define PHY_PER_LTF                                    8'd4
`define PHY_HT_STF                                     8'd4
`define PHY_VHT_SIG_A                                  8'd8
`define PHY_VHT_SIG_B                                  8'd4
`define PHY_VHT_STF                                    8'd4

//non_ht_modulation
`define ERP_DSSS                               0
`define ERP_CCK                                1
`define ERP_OFDM                               2
`define ERP_PBCC                               3
`define DSSS_OFDM                              4
`define OFDM                                   5
`define NON_HT_DUP_OFDM                        6

// HT Parameters
`define UU_HT_MCS_MASK                                 8'h1f
`define UU_HT_MANDATORY_MCS_MASK                       8'h07   
`define UU_CCK_RATE_FLAG                               8'h80
`define UU_OFDM_HW_MASK_INDEX                          8'h07

/*-------------------------------------------------*/
//module specific macros ***
`define STA_MAC_ADDR_0                                 8'hFF
`define STA_MAC_ADDR_1                                 8'h00
`define STA_MAC_ADDR_2                                 8'hFF
`define STA_MAC_ADDR_3                                 8'h00
`define STA_MAC_ADDR_4                                 8'hFF
`define STA_MAC_ADDR_5                                 8'h00

/* STA MAC ADDRESS */
`define UU_WLAN_IEEE80211_STA_MAC_ADDR_R               48'h022010b0f8fc 

/*-------------Added for Tx---------------------*/
// USER DEFINED
`define NON_HT_INDEX                                   4'h2
`define NON_HT_250                                     4'h0
`define NON_HT_HW                                      4'h1
`define HT_INDEX                                       4'h5
`define HT_20                                          4'h0
`define HT_40                                          4'h1
`define HT_20_SGI                                      4'h2
`define HT_40_SGI                                      4'h3
`define HT_OFDM                                        4'h4
`define VHT_INDEX                                      4'h9
`define VHT_20                                         4'h0
`define VHT_40                                         4'h1
`define VHT_80                                         4'h2
`define VHT_160                                        4'h3
`define VHT_20_SGI                                     4'h4
`define VHT_40_SGI                                     4'h5
`define VHT_80_SGI                                     4'h6
`define VHT_160_SGI                                    4'h7
`define VHT_OFDM                                       4'h8

// Tx Specific 
`define IEEE80211_FC1_DIR_MASK                         8'h03
`define IEEE80211_FC1_DIR_NODS                         8'h00    /* STA->STA */
`define IEEE80211_FC1_DIR_TODS                         8'h01    /* STA->AP  */
`define IEEE80211_FC1_DIR_FROMDS                       8'h02    /* AP ->STA */
`define IEEE80211_FC1_DIR_DSTODS                       8'h03    /* AP ->AP  */
`define IEEE80211_FC1_MORE_FRAG                        8'h04
`define IEEE80211_FC1_RETRY                            8'h08
`define IEEE80211_FC1_PWR_MGT                          8'h10
`define IEEE80211_FC1_MORE_DATA                        8'h20
`define IEEE80211_FC1_PROTECTED                        8'h40
`define IEEE80211_FC1_WEP                              8'h40    /* pre-RSNA compat */
`define IEEE80211_FC1_ORDER                            8'h80

`define MPDU_OFFSET                                    8'd22
`define CTRL_MPDU_OFFSET                               8'd56
`define AGGR_MPDU_LENGTHS                              8'h4
`define BA_FRAME_BITMAP_OFFSET                         8'd34
`define CIR_BUF_DEPTH                                  8'd100

/* Access Category types */
`define  UU_WLAN_AC_BE                                 2'h0
`define  UU_WLAN_AC_BK                                 2'h1
`define  UU_WLAN_AC_VI                                 2'h2
`define  UU_WLAN_AC_VO                                 2'h3

