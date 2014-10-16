 /*****************************************************************************
 *                                                                            *
 * Module        : lmac_top.v                                                 *
 * This module implements the TOP module of LMAC.                             *
 *****************************************************************************/

`include "../../inc/defines.h"
 /*****************************************************************************
 *                                                                            *
 * Module        : lmac_top.v                                                 *
 * This module implements the TOP module of LMAC.                             *
 *****************************************************************************/

module uu_acmac_top( 
        input                clk                            , // Input Clock to LMAC      
        input                rst_n                          , // Reset Input to LMAC        			
        input                lmac_en                        , // Enable for ALL LMAC Modules          
        input                cap_en                         , // Enable for CAP Module
        input                cp_en                          , // Enable for CP Module
        input                rx_en                          , // Enable for RX Module
        input                tx_en                          , // Enable for TX Module
        input                sta_en                         , // Enable for STA and BA Management Module
        input        [3 :0]  lmac_in_umac_bea               , // Byte Enable Input from UMAC
        input        [31:0]  lmac_in_umac_data              , // Data Input from UMAC
        input        [14:0]  lmac_in_umac_addr              , // Address Input from UMAC
        input                lmac_in_umac_wrn_rd            , // WritenRead Input from UMAC
        input                lmac_in_umac_val               , // Data Valid from UMAC
        input                lmac_in_frame_valid            , // Input Frame Valid from DMA
        input        [7 :0]  lmac_in_frame                  ,   
        input        [15:0]  lmac_in_BSSBasicRateSet        , // Input BSSBasic Rate Set 
        input        [15:0]  lmac_in_SIFS_timer_value       , // Input SIFS timer Value
        input        [1 :0]  lmac_in_rxend_status           , // RX End Status Input
        input        [9 :0]  lmac_in_filter_flag            , // LMAC Filter Flag Input
        input                lmac_in_ev_txstart_confirm     , // Tx Start Confrim Event Input
        input                lmac_in_ev_txdata_confirm      , // Tx Data Confirm Event input
        input                lmac_in_ev_txready             , // TX Ready Event Input
        input                lmac_in_ev_timer_tick          , // Timer Tick Event Input
        input                lmac_in_ev_phyenergy_ind       , // PHY Energy Event Input
        input                lmac_in_ev_rxstart_ind         , // RX Start Event Input
        input                lmac_in_ev_rxdata_ind          , // Rx Data Event Input
        input                lmac_in_ev_rxend_ind           , // Rx End Event Input
        input                lmac_in_ev_phyrx_error         , // PHY Rx Error Input
        input                lmac_in_ev_ccareset_confirm    , // CCA Reset Confirm
        input                lmac_in_qos_mode               , // QOS Mode Input  
        input        [15:0]  lmac_in_RTS_threshold          , // RTS Threshold Input
        input        [1 :0]  lmac_in_cca_status             , // CCA Status Input
        input        [2 :0]  lmac_in_channel_list           , // Input Channel List
        input        [2 :0]  lmac_in_txvec_chbw             , // Input Channel BW
        input        [1 :0]  lmac_in_txvec_format           , // Input TX Vector Format
        input        [7 :0]  lmac_in_aggr_count             , // Input Aggregation Count      
        input        [15:0]  lmac_in_ac0_txnav_value        , // AC0 TXNAV Value
        input        [15:0]  lmac_in_ac1_txnav_value        , // AC1 TXNAV Value 
        input        [15:0]  lmac_in_ac2_txnav_value        , // AC2 TXNAV Value 
        input        [15:0]  lmac_in_ac3_txnav_value        , // AC3 TXNAV Value 
        input        [15:0]  lmac_in_phy_rxstart_delay      , // Rx Start Delay Input
        input        [3 :0]  lmac_in_difs_value             , // Input DIFS Value
        input        [3 :0]  lmac_in_ac0_aifs_value         , // AC0 AIFS Value Input
        input        [3 :0]  lmac_in_ac1_aifs_value         , // AC1 AIFS Value Input 
        input        [3 :0]  lmac_in_ac2_aifs_value         , // AC2 AIFS Value Input 
        input        [3 :0]  lmac_in_ac3_aifs_value         , // AC3 AIFS Value Input
        input        [3 :0]  lmac_in_slot_timer_value       , // Slot Timer Input
        input        [4 :0]  lmac_in_cts_timer_value        , // CTS Timer Input
        input        [4 :0]  lmac_in_ack_timer_value        , // ACK Timer Input
        input        [5 :0]  lmac_in_eifs_value             , // EIFS Value Input
        input        [15:0]  lmac_in_cwmin_value            , // NON QOS CWMIN Value Input
        input        [15:0]  lmac_in_ac0_cwmin_value        , // AC0 CWMIN Value Input
        input        [15:0]  lmac_in_ac1_cwmin_value        , // AC1 CWMIN Value Input 
        input        [15:0]  lmac_in_ac2_cwmin_value        , // AC2 CWMIN Value Input 
        input        [15:0]  lmac_in_ac3_cwmin_value        , // AC3 CWMIN Value Input 
        input        [15:0]  lmac_in_cwmax_value            , // NON QOS CWMAX Value Input
        input        [15:0]  lmac_in_ac0_cwmax_value        , // AC0 CWMAX Value Input
        input        [15:0]  lmac_in_ac1_cwmax_value        , // AC1 CWMAX Value Input 
        input        [15:0]  lmac_in_ac2_cwmax_value        , // AC2 CWMAX Value Input 
        input        [15:0]  lmac_in_ac3_cwmax_value        , // AC3 CWMAX Value Input 
        input        [47:0]  lmac_in_mac_addr               , // MAC Address of STA
        input        [15:0]  lmac_in_TXOP_limit_ac0         , // AC0 TXOP Limit Input
        input        [15:0]  lmac_in_TXOP_limit_ac1         , // AC1 TXOP Limit Input
        input        [15:0]  lmac_in_TXOP_limit_ac2         , // AC2 TXOP Limit Input
        input        [15:0]  lmac_in_TXOP_limit_ac3         , // AC3 TXOP Limit Input
        input        [15:0]   lmac_in_Long_Retry_Count       , // Long Retry Count Input to Tx Module
        input        [7:0]   lmac_in_short_retry_count      ,
        input        [1 :0]  lmac_in_dma_ac                 , // DMA AC Input
        input                lmac_in_dma_wen                , // DMA Write Enable Input
        input        [31:0]  lmac_in_dma_data               , // Input Data to DMA 
        input                lmac_in_cp_self_cts            ,// Self CTS from Tx to CP
	// ports for axi bus
	input  			rx_buff_rd_en                                ,//------------------vibha : rx buffer read enable from umac
	input         [15:0] rx_buff_rd_addr                  ,//-------------------vibha : rx buffer read address from umac
        output       [12:0]  lmac_out_free_memory_ac0       , // Memory Clear for AC0
        output       [12:0]  lmac_out_free_memory_ac1       , // Memory Clear for AC1
        output       [12:0]  lmac_out_free_memory_ac2       , // Memory Clear for AC2
        output       [12:0]  lmac_out_free_memory_ac3       , // Memory Clear for AC3
        output       [31:0]  lmac_out_umac_data             , // Data Output to UMAC
        output               lmac_out_umac_ack              , // ACK Output to UMAC
        output               lmac_out_ev_ccareset_req       , // CCA Reset Req Output
        output               lmac_out_ev_txend_req          , // Tx End Req Output
        output               lmac_out_ev_rxend_ind          , // Rx End Indication Output
        output               lmac_out_rx_ampdu_frames_cnt   , // AMPDU Frame Count to UMAC
        output               lmac_out_rx_ampdu_subframe_cnt , // AMPDU Subframe Count to UMAC 
        output               lmac_out_rx_phyerr_pkts_cnt    , // PHY Error Packet Count to UMAC 
        output               lmac_out_rx_frames_cnt         , // Rx Frames Count to UMAC 
        output               lmac_out_rx_multicast_cnt      , // Multicast Frame count to UMAC 
        output               lmac_out_rx_broadcast_cnt      , // Broadcast Frame Count to UMAC
        output               lmac_out_rx_frame_forus_cnt    , // Frames for us count to UMAC 
        output               lmac_out_dot11_fcs_error_cnt   , // FCS Error count to UMAC 
        output               lmac_out_frame_decrypt_enable  , // Frame Decrypt Enable to Crypto Engine
        output               lmac_out_frame_post_to_umac    , // Output to post frame to UMAC
        output               lmac_out_phy_start_req         , // Output PHY Start Request from Tx
     	  output               lmac_out_phy_data_req          , // Output PHY Data Request from Tx
        output               lmac_out_phy_frame_val         , // Output PHY Frame Valid from Tx
        output       [7 :0]  lmac_out_phy_frame             ,// Output PHY Frame from Tx
	      output reg   [63:0]  rx_status_reg1_out             ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg2_out             ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg3_out             ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg4_out             ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg5_out             ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg6_out             ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg7_out             ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg8_out             ,// vibha : rx status registers to be read by umac
       	output reg   [63:0]  rx_status_reg9_out             ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg10_out            ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg11_out            ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg12_out            ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg13_out            ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg14_out            ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg15_out            ,// vibha : rx status registers to be read by umac
	      output reg   [63:0]  rx_status_reg16_out            , //vibha : rx status registers to be read by umac
        output       [7 :0]  lmac_out_rx_frame              ,
        output               lmac_out_rx_frame_valid,
        input                lmac_in_phy_txdata_confirm
               );

// CAP
   wire          lmac_txbuf_cap_ac0_frame_info;
   wire          lmac_txbuf_cap_ac1_frame_info;
   wire          lmac_txbuf_cap_ac2_frame_info;
   wire          lmac_txbuf_cap_ac3_frame_info;
   reg           cap_in_enable               ;// Register for CAP Enable
   wire  [31:0]  lmac_tx_cap_res             ;// TX Response to CAP
   wire          lmac_tx_cap_res_val         ;// Tx Response Valid to CAP
   wire          lmac_ev_txop_txstart_req    ;// TXOP Tx Start Req from CAP to TX
   wire          lmac_ev_txsendmore_data_req ;// Send More Data req from CAP to TX
   wire          lmac_wait_for_ack           ;// Wait for ACK from CAP to CP
   wire          lmac_wait_for_cts           ;// Wait for CTS from CAP to CP
   wire          lmac_fb_update              ;// Update FB from CAP to Tx 
   wire          lmac_clear_ampdu            ;// Clear Ampdu signal from CAP to Tx
   wire  [7 :0]  lmac_cap_ac0_src            ;// Short retry Count for AC0 from CAP to Tx
   wire  [7 :0]  lmac_cap_ac1_src            ;// Short retry Count for AC1 from CAP to Tx
   wire  [7 :0]  lmac_cap_ac2_src            ;// Short retry Count for AC2 from CAP to Tx
   wire  [7 :0]  lmac_cap_ac3_src            ;// Short retry Count for AC3 from CAP to Tx
   wire  [7 :0]  lmac_cap_ac0_lrc            ;// Long retry Count for AC0 from CAP to Tx
   wire  [7 :0]  lmac_cap_ac1_lrc            ;// Long retry Count for AC1 from CAP to Tx
   wire  [7 :0]  lmac_cap_ac2_lrc            ;// Long retry Count for AC2 from CAP to Tx
   wire  [7 :0]  lmac_cap_ac3_lrc            ;// Long retry Count for AC3 from CAP to Tx
   wire          lmac_cap_tx_frame_info_retry;// Retry bit set in CAP sent to Tx Memory
   wire          lmac_mpdu_status            ;// MPDU Discard Status
   wire          lmac_mpdu_discard           ;// MPDU Discard Output 
   wire  [19:0]  lmac_frame_length           ; // Input Frame Length
   wire          lmac_tx_cap_ev_data_end_confirm; //data end confirm from tx to cap
   wire		 rx_out_cap_rx_error_ind     ; //RX to CAP
   wire  [1:0]        lmac_tx_cap_ack_policy  ;//added for multicast frames
wire  [7:0]   lmac_tx_cap_mpdu_fc0;//added for multicast frames
wire  [7:0]   lmac_tx_cap_mpdu_fc1;//added for multicast frames

  
// CP
   reg           cp_in_enable                ;// Register for CP Enable
   wire  [1 :0]  lmac_cp_tx_ac               ;// AC to TX from CP
   wire          lmac_process_rx_frame       ;// Rx Frame Process from Rx to CP
   wire          lmac_is_rxend               ;// Rx End from Rx to CP
   wire          lmac_tx_cp_start_ind        ;// Tx Start Indication from Tx to CP
   wire  [1 :0]  lmac_tx_cp_txvec_format     ;// Tx Vector Format from Tx to CP
   wire  [3 :0]  lmac_tx_cp_rtscts_rate      ;// RTSCTS Rate from TX to CP
   wire  [47:0]  lmac_tx_cp_mpdu_ra          ;// RA from Tx to CP 
   wire  [31:0]  lmac_rx_return              ;// Rx Return Value from CP to Rx
   wire          lmac_rx_return_val          ;// Rx Return Valid from CP to Rx
   wire          lmac_cp_info_en             ;// Rx Memory Enable from CP
   wire          lmac_rx_info_en             ;// Rx Memory Enable from Rx
   wire          lmac_info_en                ;// Rx Memory Enable Input
   wire          lmac_info_wen               ;// Rx Memory Write Enable Input
   wire  [7 :0]  lmac_info_din               ;// Rx Memory Input Data from Rx
   wire  [7 :0]  lmac_info_dout              ;// Rx Memory Output Data to CP
   wire  [15:0]  lmac_info_addr              ;// Rx Memory Address Input
   wire  [15:0]  lmac_cp_info_addr           ;// Rx Memory Address from CP
   wire  [15:0]  lmac_rx_info_addr           ;// Rx Memory Address from Rx
   wire          lmac_cp_ctrl_en             ;// Tx Control Memory Enable from CP
   wire          lmac_tx_ctrl_en             ;// Tx Control Memory Enable from Tx
   wire          lmac_ctrl_en                ;// Tx Control Memory Enable Input
   wire          lmac_cp_ctrl_wen            ;// Tx Control Memory Write Enable from CP
   wire          lmac_tx_ctrl_wen            ;// Tx Control Memory Write Enable from TX
   wire          lmac_ctrl_wen               ;// Tx Control Memory Write Enable Input
   wire  [7 :0]  lmac_tx_ctrl_addr           ;// Tx Control Memory Address from Tx
   wire  [7 :0]  lmac_cp_ctrl_addr           ;// Tx Control Memory Address from CP
   wire  [7 :0]  lmac_ctrl_addr              ;// Tx Control Memory Address Input
   wire  [7 :0]  lmac_tx_ctrl_din            ;// Tx Control Memory Data Input from Tx
   wire  [7 :0]  lmac_cp_ctrl_din            ;// Tx Control Memory Data Input from CP
   wire  [7 :0]  lmac_ctrl_din               ;// Tx Control Memory Data Input 
   wire  [15:0]  lmac_cp_tx_seqno            ;// Starting Sequence Number from CP to Tx
   wire          lmac_cp_tx_res_val          ;// Resonse Valid from CP to Tx
   wire  [31:0]  lmac_cp_tx_res              ;// Response from CP to Tx
   wire          lmac_tx_cp_res_val          ;// Response from Tx to CP Module
   wire  [31:0]  lmac_tx_cp_res              ;// Response Valid from Tx to CP Module
   wire  [7:0]  lmac_cp_tx_fI_frame_len     ;// Control Frame Length Information from CP to Tx
   wire          lmac_cp_tx_upd_mpdu_status  ;// MPDU Status from CP to Tx 
   wire          lmac_cp_tx_get_ampdu_status ;// AMPDU Status from CP to Tx
   wire   [47:0]  lmac_in_bar_address         ; // BAR Address Input
   wire   [15:0]  lmac_in_bar_scf             ; // BAR Sequence Control Input
   wire   [3 :0]  lmac_in_bar_tid             ; // BAR TID Input
   wire   [1 :0]  lmac_in_bar_bartype         ; // BAR Type Input
   wire   [15:0]  lmac_in_bar_duration        ; // BAR Duration Input
   wire           lmac_in_bar_valid           ; // BAR Valid Input
   wire   [7:0]   lmac_cp_data                ; //data going to Cp
//TX
wire		 lmac_txhandler_ctrl_start   ;
   wire          lmac_txhandler_ctrl_data    ;
   reg           tx_in_enable                ;// Register the Tx Module Enable
   wire          lmac_ev_txdata_req          ;// Tx Data Req from CAP to Tx
   wire  [1 :0]  lmac_frame_tx_ac            ;// AC from the Tx module.
   wire  [2 :0]  lmac_channel_bw             ;// Channel Bw from CAP to Tx
   wire  [2 :0]  lmac_txop_owner_ac          ;// TXOP AC output from CAP to TX
   wire  [15:0]  lmac_ac0_txnav_value        ;// AC0 TXNAV value from CAP to TX
   wire  [15:0]  lmac_ac1_txnav_value        ;// AC1 TXNAV value from CAP to TX
   wire  [15:0]  lmac_ac2_txnav_value        ;// AC2 TXNAV value from CAP to TX
   wire  [15:0]  lmac_ac3_txnav_value        ;// AC3 TXNAV value from CAP to TX
   wire          lmac_tx_frame_en            ;// Tx Memory Enable from the Tx module
   wire          lmac_tx_frame_wen           ;// Tx Memory Write Enable ( Not Used )
   wire  [13:0]  lmac_tx_frame_addr          ;// Frame Address from Tx module
   wire  [31:0]  lmac_tx_frame_dout          ;// Tx Memory Data Output connected to Tx
   wire  [31:0]  lmac_tx_frame_din           ;// Data Output from Tx Module (Not Used)
   wire  [13:0]  lmac_tx_buf_base_ptr_ac0        ;//vibha
   wire  [13:0]  lmac_tx_buf_base_ptr_ac1        ;//vibha
   wire  [13:0]  lmac_tx_buf_base_ptr_ac2        ;//vibha
   wire  [13:0]  lmac_tx_buf_base_ptr_ac3        ;//vibha
 	wire lmac_tx_buf_rd_ac0;//vibha
	wire lmac_tx_buf_rd_ac1;//vibha
	wire lmac_tx_buf_rd_ac2;//vibha
	wire lmac_tx_buf_rd_ac3;//vibha
   wire  [19:0]  lmac_tx_buf_pkt_len         ;
   wire  [7 :0]  lmac_tx_ctrl_dout           ;
   wire          tx_in_cap_send_ack          ;
   wire          tx_out_cap_ev_txready       ;

// STA & BA
   reg           sta_in_enable               ;//enable for sta
   wire  [3 :0]  lmac_sta_bea                ;// Byte Enable from CP and Tx
   wire  [3 :0]  lmac_cp_sta_bea             ;// Byte Enable Between CP and STA
   wire  [3 :0]  lmac_tx_sta_bea             ;// Byte Enable Between Tx and STA
   wire  [31:0]  lmac_sta_data               ;// Data from CP or Tx
   wire  [31:0]  lmac_tx_sta_data            ;// Data from TX to STA
   wire  [31:0]  lmac_cp_sta_data            ;// Data from CP to STA
   wire  [14:0]  lmac_sta_addr               ;// Address from CP and Tx to STA module
   wire  [14:0]  lmac_tx_sta_addr            ;// Address from Tx to STA
   wire  [14:0]  lmac_cp_sta_addr            ;// Address from CP to STA
   wire          lmac_sta_en                 ;// Enable from CP and Tx
   wire          lmac_tx_sta_en              ;// Enable from Tx to STA
   wire          lmac_cp_sta_en              ;// Enable from CP to STA
   wire  [31:0]  lmac_sta_out_data           ;// Data Output from STA Module
   wire          lmac_sta_dir                ;// Direction from CP and Tx
   wire          lmac_tx_sta_dir             ;// Direction from Tx to STA
   wire          lmac_cp_sta_dir             ;// Direction from CP to STA
   wire  [3 :0]  lmac_sta_tid                ;// TID from CP and Tx
   wire  [3 :0]  lmac_tx_sta_tid             ;// TID from Tx to STA
   wire  [3 :0]  lmac_cp_sta_tid             ;// TID from CP to STA
   wire  [47:0]  lmac_sta_mac_addr           ;// MAC ADDR from CP and Tx
   wire  [47:0]  lmac_tx_sta_mac_addr        ;// MAC ADDR from Tx to STA
   wire  [47:0]  lmac_cp_sta_mac_addr        ;// MAC ADDR from CP to STA
   wire          lmac_sta_get_ba_info        ;// BA Info from CP and Tx
   wire          lmac_tx_sta_get_ba_info     ;// BA Info from Tx to STA
   wire          lmac_cp_sta_get_ba_info     ;// BA Info from CP to STA
   wire          lmac_sta_info               ;// Info from STA to CP and TX
   wire          lmac_sta_info_val           ;// Info Valid from STA to CP and TX
   wire  [14:0]  lmac_sta_offset             ;// OFFSET to TX and CP from STA
//****************************
   wire  [11:0]  lmac_tx_sta_seqno_ac0       ; // Sequence Number from Tx for AC0
   wire  [11:0]  lmac_tx_sta_seqno_ac1       ; // Sequence Number from Tx for AC1
   wire  [11:0]  lmac_tx_sta_seqno_ac2       ; // Sequence Number from Tx for AC2
   wire  [11:0]  lmac_tx_sta_seqno_ac3       ; // Sequence Number from Tx for AC3
   wire  [7 :0]  lmac_ac0_src                ; // Short Retry Count for AC0
   wire  [7 :0]  lmac_ac1_src                ; // Short Retry Count for AC1
   wire  [7 :0]  lmac_ac2_src                ; // Short Retry Count for AC2
   wire  [7 :0]  lmac_ac3_src                ; // Short Retry Count for AC3
   wire  [7 :0]  lmac_ac0_lrc                ; // Long Retry Count for AC0
   wire  [7 :0]  lmac_ac1_lrc                ; // Long Retry Count for AC1
   wire  [7 :0]  lmac_ac2_lrc                ; // Long Retry Count for AC2
   wire  [7 :0]  lmac_ac3_lrc                ; // Long Retry Count for AC3
   wire  [7 :0]  lmac_tx_sta_bitmap_ac0      ; // Bitmap for AC0 AMPDU
   wire  [7 :0]  lmac_tx_sta_bitmap_ac1      ; // Bitmap for AC1 AMPDU
   wire  [7 :0]  lmac_tx_sta_bitmap_ac2      ; // Bitmap for AC2 AMPDU
   wire  [7 :0]  lmac_tx_sta_bitmap_ac3      ; // Bitmap for AC3 AMPDU
   wire  [19:0]  lmac_tx_sta_frame_len       ; // Input frame Length for the Packet
//*****************************************
//RX 
   reg           rx_in_enable                ;// Rx Module Enable Input
   wire  [31:0]  lmac_rx_cap_return_value    ;// RX Return Value to CAP
   wire  [15:0]  lmac_rx_cp_seq_no           ;
   wire          lmac_rx_cp_is_aggr          ;
// CRC
   wire          lmac_crc_en                 ;// CRC Module Enable from CP and Tx
   wire          lmac_tx_crc_en              ;// CRC Module Enable from TX
   wire          lmac_cp_crc_en              ;// CRC Module Enable from CP
   wire          lmac_data_val               ;// CRC Module Data Valid Input from CP and Tx
   wire          lmac_tx_crc_data_val        ;// CRC Module Data Valid Input from Tx
   wire          lmac_cp_crc_data_val        ;// CRC Module Data Valid Input from CP
   wire  [7 :0]  lmac_data                   ;// CRC Module Input Data from Tx and CP
   wire  [7 :0]  lmac_tx_crc_data            ;// CRC Module Input Data from Tx
   wire  [7 :0]  lmac_cp_crc_data            ;// CRC Module Input Data from CP
   wire  [15:0]  lmac_len                    ;// CRC Input Length from Tx and CP 
   wire  [15:0]  lmac_tx_crc_len             ;// CRC Length Input to CRC Module from TX
   wire  [15:0]  lmac_cp_crc_len             ;// CRC Length Input to CRC Module from CP
   wire          lmac_crc_avl                ;// CRC Available Output from CRC
   wire  [31:0]  lmac_crc                    ;// CRC Value from CRC Module
// VHT
   wire          lmac_vht_mcs_en             ;// VHT Memory Enable from CP and Tx
   wire          lmac_cp_vht_mcs_en          ;// VHT Memory Enable from CP 
   wire          lmac_tx_vht_mcs_en          ;// VHT Memory Enable from Tx
   wire          lmac_cp_vht_mcs_wen         ;// VHT Memory Write Enable from CP ( Not Used )
   wire          lmac_tx_vht_mcs_wen         ;// VHT Memory Write Enable from TX ( Not Used )
   wire  [9 :0]  lmac_vht_mcs_addr           ;// VHT Memory Address from CP and Tx
   wire  [9 :0]  lmac_cp_vht_mcs_addr        ;// VHT Memory Address from CP 
   wire  [9 :0]  lmac_tx_vht_mcs_addr        ;// VHT Memory Address from Tx
   wire  [39:0]  lmac_vht_mcs_data           ;// VHT Memory Data to CP and Tx Modules
   wire          lmac_ht_mcs_en              ;// HT Memory Enable from CP and Tx
   wire          lmac_cp_ht_mcs_en           ;// HT Memory Enable from CP
   wire          lmac_tx_ht_mcs_en           ;// HT Memory Enable from Tx
   wire          lmac_cp_ht_mcs_wen          ;// HT Memory Write Enable from CP ( Not Used )
   wire          lmac_tx_ht_mcs_wen          ;// HT Memory Write Enable from Tx ( Not Used )
   wire  [7 :0]  lmac_ht_mcs_addr            ;// HT Memory Address Input from CP and Tx
   wire  [7 :0]  lmac_cp_ht_mcs_addr            ;// HT Memory Address Input from CP and Tx
   wire  [7 :0]  lmac_tx_ht_mcs_addr            ;// HT Memory Address Input from CP and Tx
   wire  [39:0]  lmac_ht_mcs_data            ;// HT Memory data Output to CP and Tx
   wire          lmac_lrate_en               ;// Legacy rate ROM Enable from CP and Tx
   wire          lmac_tx_lrate_en            ;// Legacy rate ROM Enable from Tx
   wire          lmac_cp_lrate_en            ;// Legacy rate ROM Enable from CP
   wire          lmac_tx_lrate_wen           ;// Legacy rate ROM Write Enable from Tx ( Not Used )
   wire          lmac_cp_lrate_wen           ;// Legacy rate ROM Write Enable from CP ( Not Used )
   wire  [5 :0]  lmac_lrate_addr             ;// Legacy rate Read Address from CP and Tx
   wire  [5 :0]  lmac_tx_lrate_addr          ;// Legacy rate Read Address from Tx
   wire  [5 :0]  lmac_cp_lrate_addr          ;// Legacy rate Read Address from CP
   wire  [39:0]  lmac_lrate_data             ;// Legacy rate Read data to CP and Tx
   
   // CP_Rx_Memory
   wire          lmac_cp_wen                 ;
   reg en_status_info; //vibha
   wire [15:0]rx_frame_length;//vibha


   reg en1;
   reg en2,en3,en4,en5,en6,en7,en8,en9,en10,en11,en12,en13,en14,en15,en16;
   reg enb1,enb2,enb3,enb4,enb5,enb6,enb7,enb8,enb9,enb10,enb11,enb12,enb13,enb14,enb15,enb16;
   reg [31:0] addr1,addr2,addr3,addr4,addr5,addr6,addr7,addr8,addr9,addr10,addr11,addr12,addr13,addr14,addr15,addr16;


// Continuous Assignments fro the Wires
   assign lmac_cp_wen = (lmac_rx_info_addr <= 'd127) ? lmac_info_wen: 1'b0; 
   assign lmac_info_en = ( lmac_rx_info_en | lmac_cp_info_en );
   assign lmac_info_addr = lmac_info_wen ? lmac_rx_info_addr :lmac_cp_info_addr;

   assign lmac_ctrl_en = ( lmac_tx_ctrl_en | lmac_cp_ctrl_en );
   assign lmac_ctrl_wen = ( lmac_tx_ctrl_wen | lmac_cp_ctrl_wen );
 
   assign lmac_ctrl_addr = ( lmac_tx_ctrl_addr | lmac_cp_ctrl_addr );
   assign lmac_ctrl_din = ( lmac_tx_ctrl_din | lmac_cp_ctrl_din );
   assign lmac_len = ( lmac_tx_crc_len | lmac_cp_crc_len );
   assign lmac_crc_en = ( lmac_tx_crc_en | lmac_cp_crc_en );
   assign lmac_data_val = ( lmac_tx_crc_data_val | lmac_cp_crc_data_val );
   assign lmac_data = ( lmac_tx_crc_data | lmac_cp_crc_data );

   assign lmac_vht_mcs_en = ( lmac_cp_vht_mcs_en | lmac_tx_vht_mcs_en );
   assign lmac_vht_mcs_addr = ( lmac_cp_vht_mcs_addr | lmac_tx_vht_mcs_addr );

   assign lmac_ht_mcs_en = ( lmac_cp_ht_mcs_en | lmac_tx_ht_mcs_en );
   assign lmac_ht_mcs_addr = ( lmac_cp_ht_mcs_addr | lmac_tx_ht_mcs_addr );

   assign lmac_lrate_en = ( lmac_tx_lrate_en | lmac_cp_lrate_en );
   assign lmac_lrate_addr = ( lmac_cp_lrate_addr | lmac_tx_lrate_addr );

   assign lmac_sta_en = ( lmac_cp_sta_en | lmac_tx_sta_en ); 
   assign lmac_sta_bea = ( lmac_cp_sta_bea | lmac_tx_sta_bea ); 
   assign lmac_sta_addr = ( lmac_cp_sta_addr | lmac_tx_sta_addr ); 
   assign lmac_sta_data = ( lmac_cp_sta_data | lmac_tx_sta_data ); 
   
   assign lmac_sta_dir = ( lmac_cp_sta_dir | lmac_tx_sta_dir ); 
   assign lmac_sta_tid = ( lmac_cp_sta_tid | lmac_tx_sta_tid ); 
   assign lmac_sta_mac_addr = ( lmac_cp_sta_mac_addr | lmac_tx_sta_mac_addr ); 
   assign lmac_sta_get_ba_info = ( lmac_cp_sta_get_ba_info | lmac_tx_sta_get_ba_info ); 
 // ports for axi bus  
   assign lmac_out_rx_frame = lmac_info_dout;
   assign lmac_out_rx_frame_valid = lmac_info_wen;


wire lmac_rx_cap_rxend_end;
// Module Instantiation
uu_acmac_cap U_WLAN_CAP_INIT
        (
        .cap_in_clk                     (clk                            ),// Connected to LMAC Clock
        .rst_n                          (rst_n                          ),// Connected to LMAC Reset
        .cap_in_enable                  (cap_in_enable                  ),// Enable to CAP Module
        .cap_in_ev_txready              (tx_out_cap_ev_txready          ),// TX Ready Event Input
        .cap_in_ev_timer_tick           (lmac_in_ev_timer_tick          ),// Timer Tick Event Input
        .cap_in_ev_txstart_confirm      (lmac_in_ev_txstart_confirm     ),// TX Start input Event
        .cap_in_ev_txdata_confirm       (lmac_tx_cap_ev_data_end_confirm),// Tx Data Confirm input Event
        .cap_in_ev_txend_confirm        (lmac_in_ev_txdata_confirm      ),// Tx End Confirm Event 
        .cap_in_ev_phyenergy_ind        (lmac_in_ev_phyenergy_ind       ),// PHY Energy Event Input
        .cap_in_ev_rxstart_ind          (lmac_in_ev_rxstart_ind         ),// Rx Start Event Input
        .cap_in_ev_rxdata_ind           (lmac_in_ev_rxdata_ind          ),// Rx Data Event Input
        .cap_in_ev_rxend_ind            (lmac_rx_cap_rxend_end          ),// Rx End Event Input
        .cap_in_ev_phyrx_error          (rx_out_cap_rx_error_ind          ),// PHY Rx Error Input
        .cap_in_ev_ccareset_confirm     (lmac_in_ev_ccareset_confirm    ),// CCA Reset Confirm
        .cap_in_frame_valid             (lmac_in_frame_valid            ),// Input Frame Valid from DMA
        .cap_in_frame                   (lmac_in_frame                  ),// Input Frame from DMA
        .cap_in_qos_mode                (lmac_in_qos_mode               ),// QOS Mode Input
        .cap_in_cca_status              (lmac_in_cca_status             ),// CCA Status Input
        .cap_in_channel_list            (lmac_in_channel_list           ),// Input Channel List
        .cap_in_txvec_chbw              (lmac_in_txvec_chbw             ),// Input Channel BW
        .cap_in_txvec_format            (lmac_in_txvec_format           ),// Input Tx Vector Format
        .cap_in_aggr_count              (lmac_in_aggr_count             ),// Input Aggregation Count from DMA
        .cap_in_difs_value              (lmac_in_difs_value             ),// Input DIFS Value
        .cap_in_ac0_aifs_value          (lmac_in_ac0_aifs_value         ),// AC0 AIFS Value Input
        .cap_in_ac1_aifs_value          (lmac_in_ac1_aifs_value         ),// AC1 AIFS Value Input
        .cap_in_ac2_aifs_value          (lmac_in_ac2_aifs_value         ),// AC2 AIFS Value Input
        .cap_in_ac3_aifs_value          (lmac_in_ac3_aifs_value         ),// AC3 AIFS Value Input
        .cap_in_slot_timer_value        (lmac_in_slot_timer_value       ),// Slot Timer Input
        .cap_in_cts_timer_value         (lmac_in_cts_timer_value        ),// CTS Timer Input
        .cap_in_ack_timer_value         (lmac_in_ack_timer_value        ),// ACK Timer Input
        .cap_in_eifs_value              (lmac_in_eifs_value             ),// EIFS Value Input
        .cap_in_cwmin_value             (lmac_in_cwmin_value            ),// NON QOS CWMIN Value Input
        .cap_in_ac0_cwmin_value         (lmac_in_ac0_cwmin_value        ),// AC0 CWMIN Value Input
        .cap_in_ac1_cwmin_value         (lmac_in_ac1_cwmin_value        ),// AC1 CWMIN Value Input
        .cap_in_ac2_cwmin_value         (lmac_in_ac2_cwmin_value        ),// AC2 CWMIN Value Input
        .cap_in_ac3_cwmin_value         (lmac_in_ac3_cwmin_value        ),// AC3 CWMIN Value Input
        .cap_in_cwmax_value             (lmac_in_cwmax_value            ),// NON QOS CWMAX Value Input
        .cap_in_ac0_cwmax_value         (lmac_in_ac0_cwmax_value        ),// AC0 CWMAX Value Input
        .cap_in_ac1_cwmax_value         (lmac_in_ac1_cwmax_value        ),// AC1 CWMAX Value Input
        .cap_in_ac2_cwmax_value         (lmac_in_ac2_cwmax_value        ),// AC2 CWMAX Value Input
        .cap_in_ac3_cwmax_value         (lmac_in_ac3_cwmax_value        ),// AC3 CWMAX Value Input
        .cap_in_ac0_txnav_value         (lmac_in_ac0_txnav_value        ),// AC0 TXNAV Value
        .cap_in_ac1_txnav_value         (lmac_in_ac1_txnav_value        ),// AC1 TXNAV Value
        .cap_in_ac2_txnav_value         (lmac_in_ac2_txnav_value        ),// AC2 TXNAV Value
        .cap_in_ac3_txnav_value         (lmac_in_ac3_txnav_value        ),// AC3 TXNAV Value
        .cap_in_sifs_timer_value        (lmac_in_SIFS_timer_value       ),// Input SIFS Timer Value
        .cap_in_phy_rxstart_delay       (lmac_in_phy_rxstart_delay      ),// Rx Start Delay Input
        .cap_in_rts_threshold_len       (lmac_in_RTS_threshold          ),// RTS Threshold Length Input
        .cap_in_frame_length            (lmac_tx_buf_pkt_len              ),// Input Frame Length
        .cap_in_rxhandler_ret_value     (lmac_rx_cap_return_value       ),// RX Return Value to CAP
        .cap_in_txhandler_ret_value     (lmac_tx_cap_res                ),// Tx Response to CAP
        .cap_in_txhandler_ret_valid     (lmac_tx_cap_res_val            ),// Tx Response Valid to CAP
        .cap_in_mac_addr                (lmac_in_mac_addr               ),// MAC Address of STA
	.cap_in_tx_ack_policy           (lmac_tx_cap_ack_policy         ),// added for multicast 
	.cap_in_tx_mpdu_fc0             (lmac_tx_cap_mpdu_fc0           ),// added for multicast 
      .cap_in_tx_mpdu_fc1             (lmac_tx_cap_mpdu_fc1           ),// added for multicast 
      .cap_in_tx_ra_addr              (lmac_tx_sta_mac_addr             ),// added for multicast           
        .cap_in_short_retry_count       (lmac_in_short_retry_count      ),
        .cap_in_long_retry_count        (lmac_in_Long_Retry_Count       ), 
        .cap_in_ac0_txframeinfo_valid   (lmac_txbuf_cap_ac0_frame_info  ),// AC0 Frame Valid to CAP
        .cap_in_ac1_txframeinfo_valid   (lmac_txbuf_cap_ac1_frame_info  ),// AC1 Frame Valid to CAP
        .cap_in_ac2_txframeinfo_valid   (lmac_txbuf_cap_ac2_frame_info  ),// AC2 Frame Valid to CAP
        .cap_in_ac3_txframeinfo_valid   (lmac_txbuf_cap_ac3_frame_info  ),// AC3 Frame Valid to CAP
        .cap_out_ev_ccareset_req        (lmac_out_ev_ccareset_req       ),// CCA Reset Req Output
        .cap_out_ev_txdata_req          (lmac_ev_txdata_req             ),// Tx Data Req from CAP to Tx
        .cap_out_ev_txend_req           (lmac_out_ev_txend_req          ),// Tx End Req Output
        .cap_out_ev_rxend_ind           (lmac_out_ev_rxend_ind          ),// Rx End Indication Output
        .cap_out_ev_txop_txstart_req    (lmac_ev_txop_txstart_req       ),// TXOP Tx Start Req from CAP to Tx
        .cap_out_ev_txsendmore_data_req (lmac_ev_txsendmore_data_req    ),// Send More Data req from CAP to Tx
        .cap_out_channel_bw             (lmac_channel_bw                ),// Channel BW from CAP to Tx
        .cap_out_frame_info_retry       (lmac_cap_tx_frame_info_retry   ),// Retry Bit from CAP to Tx
        .cap_out_mpdu_status            (lmac_mpdu_status               ),// MpDU Discard Status Signal
        .cap_out_mpdu_discard           (lmac_mpdu_discard              ),// MPDU Clear signal to Tx
        .cap_out_txop_owner_ac          (lmac_txop_owner_ac             ),// TXOP Owner AC from CAP to TX
        .cap_out_ac0_src                (lmac_cap_ac0_src               ),// Short Retry Count for AC0
        .cap_out_ac1_src                (lmac_cap_ac1_src               ),// Short Retry Count for AC1
        .cap_out_ac2_src                (lmac_cap_ac2_src               ),// Short Retry Count for AC2
        .cap_out_ac3_src                (lmac_cap_ac3_src               ),// Short Retry Count for AC3
        .cap_out_ac0_lrc                (lmac_cap_ac0_lrc               ),// Long retry Count for AC0 from CAP to Tx
        .cap_out_ac1_lrc                (lmac_cap_ac1_lrc               ),// Long retry Count for AC1 from CAP to Tx
        .cap_out_ac2_lrc                (lmac_cap_ac2_lrc               ),// Long retry Count for AC2 from CAP to Tx
        .cap_out_ac3_lrc                (lmac_cap_ac3_lrc               ),// Long retry Count for AC3 from CAP to Tx
        .cap_out_ac0_txnav_value        (lmac_ac0_txnav_value           ),// AC0 TXNAV Value from CAP to TX
        .cap_out_ac1_txnav_value        (lmac_ac1_txnav_value           ),// AC1 TXNAV Value from CAP to TX
        .cap_out_ac2_txnav_value        (lmac_ac2_txnav_value           ),// AC2 TXNAV Value from CAP to TX
        .cap_out_ac3_txnav_value        (lmac_ac3_txnav_value           ),// AC3 TXNAV Value from CAP to TX
        .cap_out_cp_wait_for_ack        (lmac_wait_for_ack              ),// Wait for ACK from CAP to CP
        .cap_out_cp_wait_for_cts        (lmac_wait_for_cts              ),// Wait for CTS from CAP to CP
        .cap_out_fb_update              (lmac_fb_update                 ),// Update FB from CAP to Tx
        .cap_out_clear_ampdu            (lmac_clear_ampdu               ), // Clear AMPDU Signal to Tx from CAP
        .cap_out_ctrl_start_req		(lmac_txhandler_ctrl_start	),
			.cap_out_ctrl_data_req		(lmac_txhandler_ctrl_data	),
			.cap_out_ac0_int_coll_r    ( lmac_ac0_int_coll_r),//vibha
			.cap_out_ac1_int_coll_r    ( lmac_ac1_int_coll_r),//vibha
			.cap_out_ac2_int_coll_r    ( lmac_ac2_int_coll_r),//vibha
			.cap_out_ac3_int_coll_r    ( lmac_ac3_int_coll_r)//vibha
        );

uu_acmac_sta_mgmt U_WLAN_STA_MGMT
       (
       .sta_clk                        (clk                            ),// Connected to LMAC Clock    
       .rst_n                          (rst_n                          ),// Connected to LMAC Reset
       .sta_enable                     (sta_in_enable                  ),// Enable to STA Module
       .sta_in_umac_bea                (lmac_in_umac_bea               ),// Byte Enable from UMAC 
       .sta_in_umac_data               (lmac_in_umac_data              ),// Data Input from UMAC
       .sta_in_umac_addr               (lmac_in_umac_addr              ),// Address Input from UMAC
       .sta_in_umac_wrn_rd             (lmac_in_umac_wrn_rd            ),// WritenRead Input from UMAC
       .sta_in_umac_val                (lmac_in_umac_val               ),// Data Valid from UMAC
       .sta_out_umac_data              (lmac_out_umac_data             ),// Data Output to UMAC
       .sta_out_umac_ack               (lmac_out_umac_ack              ),// ACK Output to UMAC
       .sta_in_tx_seqno_ac0            (lmac_tx_sta_seqno_ac0          ),// Sequence Number from Tx for AC0
       .sta_in_tx_seqno_ac1            (lmac_tx_sta_seqno_ac1          ),// Sequence Number from Tx for AC1
       .sta_in_tx_seqno_ac2            (lmac_tx_sta_seqno_ac2          ),// Sequence Number from Tx for AC2
       .sta_in_tx_seqno_ac3            (lmac_tx_sta_seqno_ac3          ),// Sequence Number from Tx for AC3
       .sta_in_ac0_src                 (lmac_cap_ac0_src               ),// Short Retry Count for AC0
       .sta_in_ac1_src                 (lmac_cap_ac1_src               ),// Short Retry Count for AC1
       .sta_in_ac2_src                 (lmac_cap_ac2_src               ),// Short Retry Count for AC2
       .sta_in_ac3_src                 (lmac_cap_ac3_src               ),// Short Retry Count for AC3
       .sta_in_ac0_lrc                 (lmac_cap_ac0_lrc               ),// Long Retry Count for AC0
       .sta_in_ac1_lrc                 (lmac_cap_ac1_lrc               ),// Long Retry Count for AC1
       .sta_in_ac2_lrc                 (lmac_cap_ac2_lrc               ),// Long Retry Count for AC2
       .sta_in_ac3_lrc                 (lmac_cap_ac3_lrc               ),// Long Retry Count for AC3
       .sta_in_ac_owner                (lmac_txop_owner_ac             ),// AC from the CAP
       .sta_in_clear_mpdu              (lmac_mpdu_discard              ),// Clear MPDU Signal from CAP
       .sta_in_clear_ampdu             (lmac_clear_ampdu               ),// Clear AMPDU Signal from CAP
       .sta_in_bitmap_ac0              (lmac_tx_sta_bitmap_ac0         ),// Bitmap for AC0 AMPDU
       .sta_in_bitmap_ac1              (lmac_tx_sta_bitmap_ac1         ),// Bitmap for AC1 AMPDU
       .sta_in_bitmap_ac2              (lmac_tx_sta_bitmap_ac2         ),// Bitmap for AC2 AMPDU
       .sta_in_bitmap_ac3              (lmac_tx_sta_bitmap_ac3         ),// Bitmap for AC3 AMPDU
       .sta_in_mpdu_status             (lmac_mpdu_status               ),// Status Signal for MPDU Clear
       .sta_in_RTS_threshold           (lmac_in_RTS_threshold          ),// Threshold Input Length for the Packet
       .sta_in_frame_length            (lmac_tx_sta_frame_len          ),// Input frame Length for the Packet
       .sta_in_aggr_count              (lmac_in_aggr_count             ),
       .sta_in_bea                     (lmac_sta_bea                   ),// Byte Enable from CP and Tx
       .sta_in_data                    (lmac_sta_data                  ),// Data from CP and Tx to STA
       .sta_in_addr                    (lmac_sta_addr                  ),// Address from CP and Tx to STA
       .sta_in_val                     (lmac_sta_en                    ),// Enable from CP and Tx to STA
       .sta_out_data                   (lmac_sta_out_data              ),// Data Output from STA Module
       .sta_in_mac_addr                (lmac_sta_mac_addr              ),// MAC ADDR from CP and Tx
       .sta_in_tid                     (lmac_sta_tid                   ),// TID from CP and Tx
       .sta_in_dir                     (lmac_sta_dir                   ),// Direction from CP and Tx
       .sta_in_get_ba                  (lmac_sta_get_ba_info           ),// BA Info from CP and Tx
       .sta_out_ba_offset              (lmac_sta_offset                ),// Offset to CP from STA  
       .sta_out_get_ba_ack             (lmac_sta_info                  ),// STA Info Output from STA
       .sta_out_ba_valid               (lmac_sta_info_val              ) // STA Info Valid Output from STA
       );

uu_acmac_cp_wrap WLAN_CP_WRAP
        (
	.clk                       (clk                        ),// Connected to LMAC Clock
	.rst_n                     (rst_n                      ),// Connected to LMAC Reset
	.cp_en                     (cp_in_enable               ),// Enable to CP Module
	.cp_in_process_rx_frame    (lmac_process_rx_frame      ),// Rx Frame Process from Rx to CP
	.cp_in_is_rxend            (lmac_is_rxend              ),// Rx End from Rx to CP
	.cp_out_tx_upd_mpdu_status (lmac_cp_tx_upd_mpdu_status ),// MPDU Status from CP to Tx 
	.cp_out_tx_get_ampdu_status(lmac_cp_tx_get_ampdu_status),// AMPDU Status from CP to Tx
	.cp_out_sta_ba_data        (lmac_cp_sta_data           ),// Data From CP to STA
	.cp_out_sta_ba_wen         (lmac_cp_sta_bea            ),// Byte Enable to STA
	.cp_out_sta_ba_en          (lmac_cp_sta_en             ),// Enable from CP to STA
	.cp_in_sta_ba_data         (lmac_sta_out_data          ),// Data Output from STA Module
	.cp_out_sta_ba_addr        (lmac_cp_sta_addr           ),// Address from CP to STA
	.cp_in_sta_info_val        (lmac_sta_info_val          ),// STA Info Valid from STA
	.cp_in_sta_info            (lmac_sta_info              ),// STA Info from STA
	.cp_in_sta_offset          (lmac_sta_offset            ),// Offset from STA
	.cp_out_sta_get_info       (lmac_cp_sta_get_ba_info    ),// BA Info from CP to STA
	.cp_out_sta_addr           (lmac_cp_sta_mac_addr       ),// MAC ADDR from CP to STA
	.cp_out_sta_tid            (lmac_cp_sta_tid            ),// TID from CP to STA
	.cp_out_sta_dir            (lmac_cp_sta_dir            ),// Direction from CP to STA
	.cp_in_tx_start_ind        (lmac_tx_cp_start_ind       ),// Tx Start Indication from Tx to CP
	.cp_in_tx_self_cts         (lmac_in_cp_self_cts        ),// Self CTS from TX to CP
	.cp_in_tx_txvec_format     (lmac_tx_cp_txvec_format    ),// Tx Vector format from Tx to CP
	.cp_in_tx_rtscts_rate      (lmac_tx_cp_rtscts_rate     ),// RTS CTS Rate from Tx to CP
	.cp_in_tx_mpdu_ra          (lmac_tx_cp_mpdu_ra         ),// RA from Tx to CP
	.cp_in_waiting_for_cts     (lmac_wait_for_cts          ),// Wait for ACK from CAP to CP
	.cp_in_waiting_for_ack     (lmac_wait_for_ack          ),// Wait for ACK from CAP to CP
	.cp_in_crc_avl             (lmac_crc_avl               ),// CRC Available Input from CRC
	.cp_in_crc                 (lmac_crc                   ),// CRC Value from CRC Module
	.cp_out_crc_len            (lmac_cp_crc_len            ),// CRC length Input to CRC Module from CP
	.cp_out_crc_en             (lmac_cp_crc_en             ),// CRC Enable Input from CP
	.cp_out_crc_data_val       (lmac_cp_crc_data_val       ),// CRC Module Data Valid Input from CP
	.cp_out_crc_data           (lmac_cp_crc_data           ),// CRC Module Input Data from CP
	.cp_in_bar_address         (lmac_in_bar_address        ),// BAR Address Input
	.cp_in_bar_scf             (lmac_in_bar_scf            ),// BAR Sequence Control Input
	.cp_in_bar_tid             (lmac_in_bar_tid            ),// BAR TID Input
	.cp_in_bar_bartype         (lmac_in_bar_bartype        ),// BAR Type Input
	.cp_in_bar_duration        (lmac_in_bar_duration       ),// BAR Duration Input
	.cp_in_bar_valid           (lmac_in_bar_valid          ),// BAR Valid Input
	.cp_out_vht_mcs_en         (lmac_cp_vht_mcs_en         ),// VHT Memory Enable Input from CP 
	.cp_out_vht_mcs_wen        (lmac_cp_vht_mcs_wen        ),// VHT Memory Wen ( Not used )
	.cp_out_vht_mcs_addr       (lmac_cp_vht_mcs_addr       ),// VHT Memory Address input from CP
	.cp_in_vht_mcs_data        (lmac_vht_mcs_data          ),// VHT Memory Data to the CP Module
	.cp_out_ht_mcs_en          (lmac_cp_ht_mcs_en          ),// HT Memory Enable Input from CP
	.cp_out_ht_mcs_wen         (lmac_cp_ht_mcs_wen         ),// HT Memory Write Enable Input( Not Used )
	.cp_out_ht_mcs_addr        (lmac_cp_ht_mcs_addr        ),// HT Memory Address Input from CP
	.cp_in_ht_mcs_data         (lmac_ht_mcs_data           ),// HT Memory Data Input from CP
	.cp_out_lrate_en           (lmac_cp_lrate_en           ),// LRate Memory Enable from CP
	.cp_out_lrate_wen          (lmac_cp_lrate_wen          ),// LRate Memory Write Enable from CP
	.cp_out_lrate_addr         (lmac_cp_lrate_addr         ),// LRate Memory Address from CP
	.cp_in_lrate_data          (lmac_lrate_data            ),// LRate Memory Data to CP
	.cp_in_BSSBasicRateSet     (lmac_in_BSSBasicRateSet    ),// Input BSS Basic Rate Set
	.cp_in_SIFS_timer_value    (lmac_in_SIFS_timer_value   ),// Input SIFS timer Value
  .cp_out_tx_ac              (lmac_cp_tx_ac              ),// AC to TX for update ampdu status
  .cp_out_tx_ctrl_fr_len     (lmac_cp_tx_fI_frame_len    ),// Control Frame Length from CP to Tx
	.cp_out_tx_seqno           (lmac_cp_tx_seqno           ),// SSN from CP to Tx
	.cp_in_tx_res_val          (lmac_tx_cp_res_val         ),// Response Valid from Tx to CP Module
	.cp_in_tx_res              (lmac_tx_cp_res             ),// Response from Tx to CP Module
	.cp_out_tx_res_val         (lmac_cp_tx_res_val         ),// Response Valid from CP to Tx
	.cp_out_tx_res             (lmac_cp_tx_res             ),// Response from CP to Tx
	.cp_out_rx_res             (lmac_rx_return             ),// Rx Return value from CP to Rx                     
	.cp_out_rx_res_en          (lmac_rx_return_val         ),// Rx Return Valid from CP to Rx
	.cp_out_rx_info_en         (lmac_cp_info_en            ),// Rx Memory Enable from CP
	.cp_out_rx_info_addr       (lmac_cp_info_addr          ),// Rx Memory Read Address from CP
	.cp_in_rx_info_data        (lmac_cp_data               ),// Rx Memory Data Output to CP
	.cp_out_tx_info_en         (lmac_cp_ctrl_en            ),// Tx Control Memory Enable Input
	.cp_out_tx_info_wen        (lmac_cp_ctrl_wen           ),// Tx Control Memory Write Enable Input
	.cp_out_tx_info_addr       (lmac_cp_ctrl_addr          ),// Tx Contorl Memory Address from CP
	.cp_out_tx_info_data       (lmac_cp_ctrl_din           ), // Tx Control Memory Data Input from CP
	.cp_in_rx_seq_no           (lmac_rx_cp_seq_no          ),
	.cp_in_rx_is_aggr          (lmac_rx_cp_is_aggr         ) 
        );                                                   
                                                             

 uu_acmac_tx_handler U_WLAN_TX_HANLDER(
	.clk                        (clk                          ),// Connected to LMAC Clock
 	.rst_n                      (rst_n                        ),// Connected to LMAC Reset
	.tx_en                      (tx_in_enable                 ),// Enable to Tx module
	.tx_in_cap_data_req         (lmac_ev_txdata_req           ),// Tx Data Req from CAP to Tx
	.tx_in_cap_start_req        (lmac_ev_txop_txstart_req     ),// TXOP Tx Start Req from CAP to Tx
	.tx_in_cap_send_ack         (tx_in_cap_send_ack           ),
	.tx_in_cap_more_data_req    (lmac_ev_txsendmore_data_req  ),// Send More Data Req from CAP to Tx
	.tx_in_cap_fb_update        (lmac_fb_update               ),// Update FB from CAP to Tx
	.tx_in_cp_upd_mpdu_status   (lmac_cp_tx_upd_mpdu_status   ),// MPDU Status from CP to Tx
	.tx_in_cp_get_ampdu_status  (lmac_cp_tx_get_ampdu_status  ),// AMPDU Status from CP to Tx
  .tx_in_clear_ampdu          (lmac_clear_ampdu             ),// Clear AMPDU Signal from CAP
  .tx_in_clear_mpdu           (lmac_mpdu_discard            ),// Clear MPDU Signal from CAP
  .tx_in_cap_retry            (lmac_cap_tx_frame_info_retry ),// Frame retry Input from CAP
	.tx_in_cap_ch_bw            (lmac_channel_bw              ),// Channel BW from CAP to TX
	.tx_in_cap_ac               (lmac_txop_owner_ac           ),// TXOP Owner AC from CAP to TX
  .tx_in_cap_ac0_tx_nav       (lmac_ac0_txnav_value         ),// AC0 TXNAV Value from CAP to TX
  .tx_in_cap_ac1_tx_nav       (lmac_ac1_txnav_value         ),// AC1 TXNAV Value from CAP to TX
	.tx_in_cap_ac2_tx_nav       (lmac_ac2_txnav_value         ),// AC2 TXNAV Value from CAP to TX
	.tx_in_cap_ac3_tx_nav       (lmac_ac3_txnav_value         ),// AC3 TXNAV Value from CAP to TX
  .tx_in_cap_ac0_src          (lmac_cap_ac0_src             ),// AC0 Short retry count from CAP
  .tx_in_cap_ac1_src          (lmac_cap_ac1_src             ),// AC1 Short retry count from CAP
  .tx_in_cap_ac2_src          (lmac_cap_ac2_src             ),// AC2 Short retry count from CAP
  .tx_in_cap_ac3_src          (lmac_cap_ac3_src             ),// AC3 Short retry count from CAP
  .tx_in_cap_ac0_lrc          (lmac_cap_ac0_lrc             ),// AC0 Long retry count from CAP
  .tx_in_cap_ac1_lrc          (lmac_cap_ac1_lrc             ),// AC1 Long retry count from CAP 
  .tx_in_cap_ac2_lrc          (lmac_cap_ac2_lrc             ),// AC2 Long retry count from CAP
  .tx_in_cap_ac3_lrc          (lmac_cap_ac3_lrc             ),// AC3 Long retry count from CAP
	.tx_in_BSSBasicRateSet      (lmac_in_BSSBasicRateSet      ),// Input BSS Basic Rate Set
	.tx_in_SIFS_timer_value     (lmac_in_SIFS_timer_value     ),// Input SIFS timer Value
	.tx_in_TXOP_limit_ac0       (lmac_in_TXOP_limit_ac0       ),// AC0 TXOP Limit Input
	.tx_in_TXOP_limit_ac1       (lmac_in_TXOP_limit_ac1       ),// AC1 TXOP Limit Input
	.tx_in_TXOP_limit_ac2       (lmac_in_TXOP_limit_ac2       ),// AC2 TXOP Limit Input
	.tx_in_TXOP_limit_ac3       (lmac_in_TXOP_limit_ac3       ),// AC3 TXOP Limit Input
	.tx_in_QOS_mode             (lmac_in_qos_mode             ),// QOS Mode Input
	.tx_in_RTS_Threshold        (lmac_in_RTS_threshold        ),// RTS Threshold Input
  .tx_in_Long_Retry_Count     (lmac_in_Long_Retry_Count     ),// Long Retry Count Input to the LMAC     
	.tx_in_cp_ac                (lmac_cp_tx_ac                ),// AC for ampdu stats update
	.tx_out_cp_start_ind        (lmac_tx_cp_start_ind         ),// Tx Start Indication from Tx to CP
	.tx_out_cp_txvec_format     (lmac_tx_cp_txvec_format      ),// Tx Vector Format from Tx to CP
	.tx_out_cp_rtscts_rate      (lmac_tx_cp_rtscts_rate       ),// RTS CTS Rate from Tx to CP
	.tx_out_cp_mpdu_ra          (lmac_tx_cp_mpdu_ra           ),// RA from Tx to CP
  .tx_in_cp_ctrl_fr_len       (lmac_cp_tx_fI_frame_len      ),// Control Frame Length from CP to Tx
	.tx_in_cp_seqno             (lmac_cp_tx_seqno             ),// SSN from CP to Tx
	.tx_out_cp_res_val          (lmac_tx_cp_res_val           ),// Response Valid from Tx to CP module
	.tx_out_cp_res              (lmac_tx_cp_res               ),// Response from Tx to CP Module
	.tx_in_cp_res_val           (lmac_cp_tx_res_val           ),// Response Valid from CP Module
	.tx_in_cp_res               (lmac_cp_tx_res               ),// Response from CP Module
	.tx_in_sta_info_val         (lmac_sta_info_val            ),// STA Info Valid from STA
	.tx_in_sta_info             (lmac_sta_info                ),// STA Info from STA
	.tx_in_sta_offset           (lmac_sta_offset              ),// Offset from STA
	.tx_out_sta_get_info        (lmac_tx_sta_get_ba_info      ),// BA Info Output to STA Module
	.tx_out_sta_addr            (lmac_tx_sta_mac_addr         ),// MAC Address Output to STA Module
	.tx_out_sta_tid             (lmac_tx_sta_tid              ),// TID Output to STA Module
	.tx_out_sta_dir             (lmac_tx_sta_dir              ),// Direction Output to STA Module
	.tx_out_sta_seqno_ac0       (lmac_tx_sta_seqno_ac0        ),// seqno of ac0 to sta
	.tx_out_sta_seqno_ac1       (lmac_tx_sta_seqno_ac1        ),// seqno of ac1 to sta
	.tx_out_sta_seqno_ac2       (lmac_tx_sta_seqno_ac2        ),// seqno of ac2 to sta
	.tx_out_sta_seqno_ac3       (lmac_tx_sta_seqno_ac3        ),// seqno of ac3 to sta
	.tx_out_sta_bitmap_ac0      (lmac_tx_sta_bitmap_ac0       ),// bitmap of ac0 to sta
	.tx_out_sta_bitmap_ac1      (lmac_tx_sta_bitmap_ac1       ),// bitmap of ac1 to sta
	.tx_out_sta_bitmap_ac2      (lmac_tx_sta_bitmap_ac2       ),// bitmap of ac2 to sta
	.tx_out_sta_bitmap_ac3      (lmac_tx_sta_bitmap_ac3       ),// bitmap of ac3 to sta
	.tx_out_sta_frame_len       (lmac_tx_sta_frame_len        ),// frame length of recent frame to sta
	.tx_in_sta_ba_data          (lmac_sta_out_data            ),// Data Output from STA Module
	.tx_out_sta_ba_addr         (lmac_tx_sta_addr             ),// Address Output to STA Module
	.tx_out_sta_ba_data         (lmac_tx_sta_data             ),// Input Data to STA Module from TX
	.tx_out_sta_ba_en           (lmac_tx_sta_en               ),// Enable Output to the STA Module
	.tx_out_sta_ba_wen          (lmac_tx_sta_bea              ),// Byte Enable Between Tx and STA 
	.tx_out_frame_ac            (lmac_frame_tx_ac             ),// AC for the Transmitted frame
	.tx_in_frame_data           (lmac_tx_frame_dout           ),// Tx Memory Data Output Connected to Tx
	.tx_out_frame_data          (lmac_tx_frame_din            ),// Frame Data Output (Not Used)
	.tx_out_frame_addr          (lmac_tx_frame_addr           ),// Tx Frame Address Output from tx module
	.tx_out_frame_en            (lmac_tx_frame_en             ),// Tx Memory Enable from the Tx Module
	.tx_out_frame_wen           (lmac_tx_frame_wen            ),// Tx Memory Write Enable ( Not Used )
	.tx_in_ctrl_data            (lmac_tx_ctrl_dout            ),// Tx Control Memory Data Output to Tx
	.tx_out_ctrl_data           (lmac_tx_ctrl_din             ),// Tx Control Memory Data Input from Tx
	.tx_out_ctrl_addr           (lmac_tx_ctrl_addr            ),// Tx Control Memory Address from Tx
	.tx_out_ctrl_en             (lmac_tx_ctrl_en              ),// Tx Control Memory Enable Input
	.tx_out_ctrl_wen            (lmac_tx_ctrl_wen             ),// Tx Control Memory Write Enable Input
	.tx_out_vht_mcs_en          (lmac_tx_vht_mcs_en           ),// VHT Memory Enable Input from Tx
//	.tx_out_vht_mcs_wen         (lmac_tx_vht_mcs_wen          ),// VHT Memory Wen ( Not Used )
	.tx_out_vht_mcs_addr        (lmac_tx_vht_mcs_addr         ),// VHT Memory Address Input from Tx
	.tx_in_vht_mcs_data         (lmac_vht_mcs_data            ),// VHT Memory Data to the Tx Module
	.tx_out_ht_mcs_en           (lmac_tx_ht_mcs_en            ),// HT Memory Enable Input from Tx
//	.tx_out_ht_mcs_wen          (lmac_tx_ht_mcs_wen           ),// HT Memory Write Enable ( Not Used )
	.tx_out_ht_mcs_addr         (lmac_tx_ht_mcs_addr          ),// HT Memory Address Input from Tx
	.tx_in_ht_mcs_data          (lmac_ht_mcs_data             ),// HT Memory Data to the Tx Module
	.tx_out_lrate_en            (lmac_tx_lrate_en             ),// LRate Memory Enable from Tx
//.tx_out_lrate_wen           (lmac_tx_lrate_wen            ),// LRate Memory Write Enable From Tx
	.tx_out_lrate_addr          (lmac_tx_lrate_addr           ),// Lrate Memory Address from Tx
	.tx_in_lrate_data           (lmac_lrate_data              ),// Lrate Memory Data to Tx
  .tx_in_buf_base_ptr_ac0         (lmac_tx_buf_base_ptr_ac0         ),// Base Address of Buffer in Tx Memory//vibha
  .tx_in_buf_base_ptr_ac1         (lmac_tx_buf_base_ptr_ac1         ),// Base Address of Buffer in Tx Memory//vibha
  .tx_in_buf_base_ptr_ac2         (lmac_tx_buf_base_ptr_ac2         ),// Base Address of Buffer in Tx Memory//vibha
  .tx_in_buf_base_ptr_ac3         (lmac_tx_buf_base_ptr_ac3         ),// Base Address of Buffer in Tx Memory//vibha
  .tx_in_buf_rd_ac0                     (lmac_tx_buf_rd_ac0),//vibha
  .tx_in_buf_rd_ac1                     (lmac_tx_buf_rd_ac1),//vibha
  .tx_in_buf_rd_ac2                     (lmac_tx_buf_rd_ac2),//vibha
  .tx_in_buf_rd_ac3                     (lmac_tx_buf_rd_ac3),//vibha  
  .tx_out_buf_pkt_len         (lmac_tx_buf_pkt_len          ),// Length of Packet to be sent to Tx Memory    
	.tx_in_crc_avl              (lmac_crc_avl                 ),// CRC Available from CRC Module
	.tx_in_crc                  (lmac_crc                     ),// CRC value from CRC Module
	.tx_out_crc_len             (lmac_tx_crc_len              ),// CRC Len Input to CRC from TX
	.tx_out_crc_en              (lmac_tx_crc_en               ),// CRC Module Enable From Tx
	.tx_out_crc_data_val        (lmac_tx_crc_data_val         ),// CRC Module Data Valid from Tx
	.tx_out_crc_data            (lmac_tx_crc_data             ),// CRC Module Input Data from Tx
  .tx_out_cap_data_end_confirm(lmac_tx_cap_ev_data_end_confirm), // TX Response Valid to CAP
	.tx_out_cap_ack_policy      (lmac_tx_cap_ack_policy       ),//added for multicast
	.tx_out_cap_mpdu_fc0        (lmac_tx_cap_mpdu_fc0         ),//added for multicast    
      .tx_out_cap_mpdu_fc1        (lmac_tx_cap_mpdu_fc1         ),//added for multicast        
	.tx_out_cap_res_val         (lmac_tx_cap_res_val          ),// TX Response Valid to CAP
	.tx_out_cap_res             (lmac_tx_cap_res              ),// TX Response to CAP
.tx_in_phy_data_req         (lmac_in_phy_txdata_confirm   ),// From PHY to TX, Data confirm.
	.tx_out_phy_start_req       (lmac_out_phy_start_req       ),// Output PHY Start Request from Tx
	.tx_out_phy_data_req        (lmac_out_phy_data_req        ),// Output PHY Data Request from Tx
	.tx_out_phy_frame_val       (lmac_out_phy_frame_val       ),// Output PHY Frame Valid from Tx
	.tx_out_phy_frame           (lmac_out_phy_frame           ),// Output PHY Frame from Tx
        .tx_in_cap_ctrl_start_req   (lmac_txhandler_ctrl_start	  ),
	.tx_in_cap_ctrl_data_req    (lmac_txhandler_ctrl_data	  )
	);


uu_acmac_rx_handler U_WLAN_RX_HANDLER
       (
       .rx_clk                         (clk                            ),// Connected to LMAC Clock
       .rst_n                          (rst_n                          ),// Connected to LMAC Reset
       .rx_enable                      (rx_in_enable                   ),// Enable to Rx Module
       .rx_in_ev_rxstart               (lmac_in_ev_rxstart_ind         ),// Rx Start Indication to Rx
       .rx_in_ev_rxdata                (lmac_in_ev_rxdata_ind          ),// Rx Data Indication to Rx
       .rx_in_ev_rxend                 (lmac_in_ev_rxend_ind           ),// Rx End Indication to Rx
       .rx_in_frame                    (lmac_in_frame                  ),// Input frame from DMA
       .rx_in_frame_valid              (lmac_in_frame_valid            ),// Input frame Valid from DMA
       .rx_in_rxend_status             (lmac_in_rxend_status           ),// RX End Status Input
       .rx_in_lmac_filter_flag         (lmac_in_filter_flag            ),// LMAC Filter Flag Input
       .rx_in_cp_return_value          (lmac_rx_return                 ),// Rx Return Value from CP to Rx
       .rx_in_cp_return_valid          (lmac_rx_return_val             ),// Rx Return Valid from CP to Rx
       .rx_in_cap_frame_info_retry     (lmac_cap_tx_frame_info_retry   ),// added for resetting the rx base ptr in reaggregation case
       .rx_out_frame_decrypt_enable    (lmac_out_frame_decrypt_enable  ),// Frame Decrypt Enable to Engine
       .rx_out_frame_post_to_umac      (lmac_out_frame_post_to_umac    ),// Output to Post Frame to UMAC
       .rx_out_rxend_ind               (lmac_is_rxend                  ),// Rx End Output from Rx to CP
       .rx_out_phyrx_error_ind         (rx_out_cap_rx_error_ind     ), // Output signal from RX to CAP during error condition
       .rx_out_cp_process_rx_frame     (lmac_process_rx_frame          ),// Rx frame Process Output from Rx to CP
       .rx_out_mem_en                  (lmac_rx_info_en                ),// Rx Memory Enable from RX
       .rx_out_mem_addr                (lmac_rx_info_addr              ),// Rx Memory Write Address from RX
       .rx_out_frame                   (lmac_info_din                  ),// Rx Memory Write Data
       .rx_out_frame_valid             (lmac_info_wen                  ),// Write Enable Input to Rx Memory
       .rx_out_cap_return_value        (lmac_rx_cap_return_value       ),// Rx Return Value to CAP
       .rx_out_rx_ampdu_frames_cnt     (lmac_out_rx_ampdu_frames_cnt   ),// AMPDU Frame Count to UMAC
       .rx_out_rx_ampdu_subframes_cnt  (lmac_out_rx_ampdu_subframe_cnt ),// AMPDU Subframe Count to UMAC
       .rx_out_rx_phyerr_pkts_cnt      (lmac_out_rx_phyerr_pkts_cnt    ),// PHY Error Count to UMAC
       .rx_out_rx_frames_cnt           (lmac_out_rx_frames_cnt         ),// Rx frames Count to UMAC
       .rx_out_rx_multicast_cnt        (lmac_out_rx_multicast_cnt      ),// Multicast frames Count to UMAC
       .rx_out_rx_broadcast_cnt        (lmac_out_rx_broadcast_cnt      ),// Broadcast frames Count to UMAC
       .rx_out_rx_frame_forus_cnt      (lmac_out_rx_frame_forus_cnt    ),// Frames for us Count to UMAC
       .rx_out_dot11_fcs_error_cnt     (lmac_out_dot11_fcs_error_cnt   ), // FCS Error Count to UMAC
       .rx_out_frame_length            ( rx_frame_length               ),
       .rx_out_cp_seq_no               (lmac_rx_cp_seq_no              ),
       .rx_out_cp_is_aggr              (lmac_rx_cp_is_aggr             ),
	.rx_out_cap_ev_rxend_ind         (lmac_rx_cap_rxend_end)
       );

uu_acmac_tx_buf TX_BUF(
	.clk                       (clk                     ),// Input Clock
	.rst_n                     (rst_n                   ),// Input Reset
	.tx_buf_in_dma_ac          (lmac_in_dma_ac          ),// Input DMA AC
	.tx_buf_in_dma_wen         (lmac_in_dma_wen         ),// DMA Write Enable Input
	.tx_buf_in_dma_data        (lmac_in_dma_data        ),// DMA Input Data
//Tx ready event from UMAC
	.tx_in_ev_tx_ready         (lmac_in_ev_txready           ),
	.tx_out_ev_tx_ready        (tx_out_cap_ev_txready        ), 
	.tx_out_ac0_frame_info     (lmac_txbuf_cap_ac0_frame_info),
	.tx_out_ac1_frame_info     (lmac_txbuf_cap_ac1_frame_info),
	.tx_out_ac2_frame_info     (lmac_txbuf_cap_ac2_frame_info),
	.tx_out_ac3_frame_info     (lmac_txbuf_cap_ac3_frame_info),
	.tx_buf_in_clear_mpdu      (lmac_mpdu_discard       ),// MPDU Clear Input from CAP
	.tx_buf_in_clear_ampdu     (lmac_clear_ampdu        ),// AMPDU Clear Input from CAP
	.tx_buf_in_ac              (lmac_frame_tx_ac        ),// Frame AC Input from TX
	.tx_buf_out_frame_data     (lmac_tx_frame_dout      ),// Frame Data Output to Tx module
   . tx_buf_in_data_end_confirm(lmac_tx_cap_ev_data_end_confirm),
	.tx_buf_in_frame_addr      (lmac_tx_frame_addr      ),// Frame Address input from Tx Module
	.tx_buf_in_frame_en        (lmac_tx_frame_en        ),// Frame Enable Input from Tx Module
      .tx_buf_in_frame_info_retry(lmac_cap_tx_frame_info_retry ),//vibha added for enabling the tx buff read for reaggregation
	
	.tx_buf_in_ac0_int_col_r(lmac_ac0_int_coll_r),//vibha
	.tx_buf_in_ac1_int_col_r(lmac_ac1_int_coll_r),//vibha
	.tx_buf_in_ac2_int_col_r(lmac_ac2_int_coll_r),//vibha
	.tx_buf_in_ac3_int_col_r(lmac_ac3_int_coll_r),//vibha
  .tx_buf_out_rd_ac0           (lmac_tx_buf_rd_ac0),//vibha
  .tx_buf_out_rd_ac1          (lmac_tx_buf_rd_ac1),//vibha
  .tx_buf_out_rd_ac2           (lmac_tx_buf_rd_ac2),//vibha
  .tx_buf_out_rd_ac3           (lmac_tx_buf_rd_ac3),//vibha	
  
	.tx_buf_out_base_ptr_ac0 (lmac_tx_buf_base_ptr_ac0    ),// Base Pointer for the Buffer//vibha
	.tx_buf_out_base_ptr_ac1 (lmac_tx_buf_base_ptr_ac1),//vibha
	.tx_buf_out_base_ptr_ac2 (lmac_tx_buf_base_ptr_ac2),//vibha
	.tx_buf_out_base_ptr_ac3 (lmac_tx_buf_base_ptr_ac3),//vibha	
	
	.tx_buf_in_pkt_len         (lmac_tx_buf_pkt_len     )// Input Packet Length for the Buffer
	);

uu_acmac_vht_mem VHT_MEM(
 .clka  (clk              ),// Connected to LMAC Clock
 .ena   (lmac_vht_mcs_en  ),// Connected to CP and Tx 
 .addra (lmac_vht_mcs_addr),// Connected to CP and Tx
 .douta (lmac_vht_mcs_data) // Connected to CP and Tx
);

uu_acmac_ht_mem HT_MEM(
 .clka  (clk             ),// Connected to LMAC Clock
 .ena   (lmac_ht_mcs_en  ),// Connected to CP and Tx
 .addra (lmac_ht_mcs_addr),// Connected to CP and Tx
 .douta (lmac_ht_mcs_data) // Connected to CP and Tx
);

uu_acmac_non_ht_mem NON_HT_MEM(
 .clka  (clk            ),// Connected to LMAC Clock 
 .ena   (lmac_lrate_en  ),// Connected to CP and Tx
 .addra (lmac_lrate_addr),// Connected to CP and Tx
 .douta (lmac_lrate_data) // Connected to CP and Tx
);

/*uu_acmac_rx_mem RX_MEM(
 .clka  (clk              ),// Connected to LMAC Clock 
 .ena   (lmac_info_en     ),// Connected to RX and CP
 .wea   (lmac_info_wen    ),// Connected to Rx
 .addra (lmac_info_addr   ),// Connected to Rx and CP
 .dina  (lmac_info_din    ),// Connected to Rx
 .douta (lmac_info_dout   ) // Connected to CP
);*/
rx_buff rx_buffer (
  .clka    (clk            ), // input clka
  .ena     (lmac_info_en   ), // input ena
  .wea     (lmac_info_wen  ), // input [0 : 0] wea
  .addra   (lmac_info_addr ), // input [15 : 0] addra
  .dina    (lmac_info_din  ), // input [7 : 0] dina
  .clkb    (clk            ), // input clkb
  .enb     (rx_buff_rd_en  ), // input enb
  .addrb   (rx_buff_rd_addr), // input [14 : 0] addrb
  .doutb   (lmac_info_dout ) // output [7 : 0] doutb
);

// Memory added for cp - depth 128 
uu_acmac_rx_cp_mem RX_CP_MEM (
  .clka  (clk               ), 
  .ena   (lmac_info_en      ), 
  .wea   (lmac_cp_wen       ), 
  .addra (lmac_rx_info_addr ), 
  .dina  (lmac_info_din     ), 
  .clkb  (clk               ), 
  .enb   (1'b1              ), 
  .addrb (lmac_cp_info_addr ), 
  .doutb (lmac_cp_data      ) 
);

uu_acmac_tx_ctrl_mem TX_CTRL_MEM(
 .clka  (clk              ),// Connected to LMAC Clock 
 .ena   (lmac_ctrl_en     ),// Connected to CP and Tx
 .wea   (lmac_ctrl_wen    ),// Connected to CP and Tx
 .addra (lmac_ctrl_addr   ),// Connected to CP and Tx
 .dina  (lmac_ctrl_din    ),// Connected to CP and Tx
 .douta (lmac_tx_ctrl_dout) // Connected to Tx
);

uu_acmac_crc32_8in CRC32
        (
	.clk      (clk           ),// Connected to LMAC Clock 
	.rst_n    (rst_n         ),// Connected to LMAC Reset
	.crc_en   (lmac_crc_en   ),// Connected to CP and Tx
	.data_val (lmac_data_val ),// Connected to CP and Tx
	.data     (lmac_data     ),// Connected to CP and Tx
	.len      (lmac_len      ),// Connected to CP and Tx
	.crc_avl  (lmac_crc_avl  ),// Connected to CP and Tx
	.crc      (lmac_crc      ) // Connected to CP and Tx   
        );
		  
// Always Block for Enable Signals to Individual Modules.
   always @( posedge clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         cap_in_enable <= 1'b0;
         cp_in_enable  <= 1'b0;
         rx_in_enable  <= 1'b0;
         tx_in_enable  <= 1'b0; 
         sta_in_enable <= 1'b0;
      end
      else if( !lmac_en )
      begin
         cap_in_enable <= 1'b0;
         cp_in_enable  <= 1'b0;
         rx_in_enable  <= 1'b0;
         tx_in_enable  <= 1'b0;
         sta_in_enable <= 1'b0;
      end
      else
      begin
         cap_in_enable <= cap_en;
         cp_in_enable  <= cp_en;
         rx_in_enable  <= rx_en;
         tx_in_enable  <= tx_en; 
         sta_in_enable <= sta_en;
      end
   end
                   
always@(posedge clk )	
		begin
		  if(rst_n==1'b0)
		  en_status_info<=1'b0;
		  else if(lmac_in_ev_rxstart_ind)
		  en_status_info<=1'b0;
		  else if(lmac_in_ev_rxdata_ind)
		  en_status_info<=1'b1;
		  end
always@(posedge clk )	
		begin
		if(rst_n==1'b0)begin
			rx_status_reg1_out<=64'b0;	
			rx_status_reg2_out<=64'b0;	
			rx_status_reg3_out<=64'b0;	
			rx_status_reg4_out<=64'b0;	
			rx_status_reg5_out<=64'b0;	
			rx_status_reg6_out<=64'b0;	
			rx_status_reg7_out<=64'b0;	
			rx_status_reg8_out<=64'b0;	
			rx_status_reg9_out<=64'b0;	
			rx_status_reg10_out<=64'b0;	
			rx_status_reg11_out<=64'b0;	
			rx_status_reg12_out<=64'b0;	
			rx_status_reg13_out<=64'b0;	
			rx_status_reg14_out<=64'b0;	
			rx_status_reg15_out<=64'b0;	
			rx_status_reg16_out<=64'b0;	

addr1<=0;addr2<=0;addr3<=0;addr4<=0;
			addr5<=0;addr6<=0;addr7<=0;addr8<=0;
			addr9<=0;addr10<=0;addr11<=0;addr12<=0;
			addr13<=0;addr14<=0;addr15<=0;addr16<=0;

en1<=1'b1;en2<=1'b0;en3<=1'b0;en4<=1'b0;
			en5<=1'b0;en6<=1'b0;en7<=1'b0;en8<=1'b0;
			en9<=1'b0;en10<=1'b0;en11<=1'b0;en12<=1'b0;
			en13<=1'b0;en14<=1'b0;en15<=1'b0;en16<=1'b0;
enb1<=1'b1;enb2<=1'b0;enb3<=1'b0;enb4<=1'b0;
			enb5<=1'b0;enb6<=1'b0;enb7<=1'b0;enb8<=1'b0;
			enb9<=1'b0;enb10<=1'b0;enb11<=1'b0;enb12<=1'b0;
			enb13<=1'b0;enb14<=1'b0;enb15<=1'b0;enb16<=1'b0;

			end
else if(lmac_in_ev_rxend_ind==1'b1)begin
  if(enb1)begin
  rx_status_reg1_out<={1'b1,15'b0,rx_frame_length,addr1};
  enb1<=1'b0; enb2<=1'b1;en2<=1'b1;end
  else if(enb2)begin
  rx_status_reg2_out<={1'b1,15'b0,rx_frame_length,addr2};
  enb2<=1'b0; enb3<=1'b1;en3<=1'b1;end
  else if(enb3)begin
  rx_status_reg3_out<={1'b1,15'b0,rx_frame_length,addr3};
  enb3<=1'b0; enb4<=1'b1;en4<=1'b1;end
  else if(enb4)begin
  rx_status_reg4_out<={1'b1,15'b0,rx_frame_length,addr4};
  enb4<=1'b0; enb5<=1'b1;en5<=1'b1;end
  else if(enb5)begin
  rx_status_reg5_out<={1'b1,15'b0,rx_frame_length,addr5};
  enb5<=1'b0; enb6<=1'b1;en6<=1'b1;end
  else if(enb6)begin
  rx_status_reg6_out<={1'b1,15'b0,rx_frame_length,addr6};
  enb6<=1'b0; enb7<=1'b1;en7<=1'b1;end
  else if(enb7)begin
  rx_status_reg7_out<={1'b1,15'b0,rx_frame_length,addr7};
  enb7<=1'b0; enb8<=1'b1;en8<=1'b1;end
  else if(enb8)begin
  rx_status_reg8_out<={1'b1,15'b0,rx_frame_length,addr8};
  enb8<=1'b0; enb9<=1'b1;en9<=1'b1;end
  else if(enb9)begin
  rx_status_reg9_out<={1'b1,15'b0,rx_frame_length,addr9};
  enb9<=1'b0; enb10<=1'b1;en10<=1'b1;end
  else if(enb10)begin
  rx_status_reg10_out<={1'b1,15'b0,rx_frame_length,addr10};
  enb10<=1'b0; enb11<=1'b1;en11<=1'b1;end
  else if(enb11)begin
  rx_status_reg11_out<={1'b1,15'b0,rx_frame_length,addr11};
  enb11<=1'b0; enb12<=1'b1;en12<=1'b1;end
  else if(enb12)begin
  rx_status_reg12_out<={1'b1,15'b0,rx_frame_length,addr12};
  enb12<=1'b0; enb13<=1'b1;en13<=1'b1;end
  else if(enb13)begin
  rx_status_reg13_out<={1'b1,15'b0,rx_frame_length,addr13};
  enb13<=1'b0; enb14<=1'b1;en14<=1'b1;end
  else if(enb14)begin
  rx_status_reg14_out<={1'b1,15'b0,rx_frame_length,addr14};
  enb14<=1'b0; enb15<=1'b0;en15<=1'b1;end
  else if(enb15)begin
  rx_status_reg15_out<={1'b1,15'b0,rx_frame_length,addr15};
  enb15<=1'b0;en16<=1'b1;end
  else if(enb16)begin
  rx_status_reg16_out<={1'b1,15'b0,rx_frame_length,addr16};
  enb16<=1'b0;end
end
else if(en_status_info && lmac_info_wen)begin
if(en1==1'b1 & rx_frame_length>0)begin
		 addr1<={16'h7CAA,lmac_info_addr};
		 en1<=1'b0;end 
 else if(en2==1'b1 & rx_frame_length>0)begin
		 addr2<={16'h7CAA,lmac_info_addr};
		 en2<=1'b0;end 
else if(en3==1'b1 & rx_frame_length>0)begin
		 addr3<={16'h7CAA,lmac_info_addr};
		 en3<=1'b0;end 
else if(en4==1'b1 & rx_frame_length>0)begin
		 addr4<={16'h7CAA,lmac_info_addr};
		 en4<=1'b0;end 
else if(en5==1'b1 & rx_frame_length>0)begin
		 addr5<={16'h7CAA,lmac_info_addr};
		 en5<=1'b0;end 
else if(en6==1'b1 & rx_frame_length>0)begin
		 addr6<={16'h7CAA,lmac_info_addr};
		 en6<=1'b0;end 
else if(en7==1'b1 & rx_frame_length>0)begin
		 addr7<={16'h7CAA,lmac_info_addr};
		 en7<=1'b0;end 
else if(en8==1'b1 & rx_frame_length>0)begin
		 addr8<={16'h7CAA,lmac_info_addr};
		 en8<=1'b0;end 
else if(en9==1'b1 & rx_frame_length>0)begin
		 addr9<={16'h7CAA,lmac_info_addr};
		 en9<=1'b0;end 
else if(en10==1'b1 & rx_frame_length>0)begin
		 addr10<={16'h7CAA,lmac_info_addr};
		 en10<=1'b0;end 
else if(en11==1'b1 & rx_frame_length>0)begin
		 addr11<={16'h7CAA,lmac_info_addr};
		 en11<=1'b0;end 
else if(en12==1'b1 & rx_frame_length>0)begin
		 addr12<={16'h7CAA,lmac_info_addr};
		 en12<=1'b0;end 
else if(en13==1'b1 & rx_frame_length>0)begin
		 addr13<={16'h7CAA,lmac_info_addr};
		 en13<=1'b0;end 
else if(en14==1'b1 & rx_frame_length>0)begin
		 addr14<={16'h7CAA,lmac_info_addr};
		 en14<=1'b0;end 
else if(en15==1'b1 & rx_frame_length>0)begin
		 addr15<={16'h7CAA,lmac_info_addr};
		 en15<=1'b0;end 
else if(en16==1'b1 & rx_frame_length>0)begin
		 addr16<={16'h7CAA,lmac_info_addr};
		 en16<=1'b0;end 
  end end
endmodule


