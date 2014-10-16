 /*****************************************************************************
 ******************************************************************************
 *                                                                            *
 * Module        : uu_acmac_top_tb.v                                          *
 * Author        : Raghavendra                                                *
 * Data Created  : 21 August 2013                                             *
 *                                                                            *
 * Description                                                                *
 * ------------                                                               *
 * This module implements the Test Bench for Top module of LMAC.              *
 ******************************************************************************
 *****************************************************************************/
`timescale 1ns/1ps
`include "../../inc/defines.h"

module uu_acmac_top_tb();
        reg                clk                            ; // Input Clock to LMAC      
        reg                rst_n                          ; // Reset Input to LMAC        			
        reg                lmac_en                        ; // Enable for ALL LMAC Modules          
        reg                cap_en                         ; // Enable for CAP Module
        reg                cp_en                          ; // Enable for CP Module
        reg                rx_en                          ; // Enable for RX Module
        reg                tx_en                          ; // Enable for TX Module
        reg                sta_en                         ; // Enable for STA and BA Management Module
        reg        [3 :0]  lmac_in_umac_bea               ; // Byte Enable Input from UMAC
        reg        [31:0]  lmac_in_umac_data              ; // Data Input from UMAC
        reg        [13:0]  lmac_in_umac_addr              ; // Address Input from UMAC
        reg                lmac_in_umac_wrn_rd            ; // WritenRead Input from UMAC
        reg                lmac_in_umac_val               ; // Data Valid from UMAC
        reg                lmac_in_frame_valid            ; // Input Frame Valid from DMA
        reg        [7 :0]  lmac_in_frame                  ; // Input Frame from DMA  
        reg        [15:0]  lmac_in_BSSBasicRateSet        ; // Input BSSBasic Rate Set 
        reg        [15:0]  lmac_in_SIFS_timer_value       ; // Input SIFS timer Value
        reg        [1 :0]  lmac_in_rxend_status           ; // RX End Status Input
        reg        [9 :0]  lmac_in_filter_flag            ; // LMAC Filter Flag Input
        reg                lmac_in_ac0_txframeinfo_valid  ; // AC0 Frame Valid to CAP
        reg                lmac_in_ac1_txframeinfo_valid  ; // AC1 Frame Valid to CAP
        reg                lmac_in_ac2_txframeinfo_valid  ; // AC2 Frame Valid to CAP
        reg                lmac_in_ac3_txframeinfo_valid  ; // AC3 Frame Valid to CAP
        reg                lmac_in_ev_mac_reset           ; // MAC Reset Event Input
        reg                lmac_in_ev_txstart_confirm     ; // Tx Start Confrim Event Input
        reg                lmac_in_ev_txdata_confirm      ; // Tx Data Confirm Event reg
        reg                lmac_in_ev_txready             ; // TX Ready Event Input
        reg                lmac_in_ev_timer_tick          ; // Timer Tick Event Input
        reg                lmac_in_ev_txend_confirm       ; // TX End Confirm Event
        reg                lmac_in_ev_phyenergy_ind       ; // PHY Energy Event Input
        reg                lmac_in_ev_rxstart_ind         ; // RX Start Event Input
        reg                lmac_in_ev_rxdata_ind          ; // Rx Data Event Input
        reg                lmac_in_ev_rxend_ind           ; // Rx End Event Input
        reg                lmac_in_ev_phyrx_error         ; // PHY Rx Error Input
        reg                lmac_in_ev_ccareset_confirm    ; // CCA Reset Confirm
        reg                lmac_in_ev_txop_txstart        ; // TXOP Tx Start Event
        reg                lmac_in_qos_mode               ; // QOS Mode Input  
        reg        [15:0]  lmac_in_RTS_threshold          ; // RTS Threshold Input
        reg        [1 :0]  lmac_in_cca_status             ; // CCA Status Input
        reg        [1 :0]  lmac_in_ac_select              ; // Input AC Select
        reg        [2 :0]  lmac_in_channel_list           ; // Input Channel List
        reg        [2 :0]  lmac_in_txvec_chbw             ; // Input Channel BW
        reg        [1 :0]  lmac_in_txvec_format           ; // Input TX Vector Format
        reg        [7 :0]  lmac_in_aggr_count             ; // Input Aggregation Count      
        reg        [15:0]  lmac_in_ac0_txop_value         ; // AC0 TXOP Input
        reg        [15:0]  lmac_in_ac1_txop_value         ; // AC1 TXOP Input 
        reg        [15:0]  lmac_in_ac2_txop_value         ; // AC2 TXOP Input 
        reg        [15:0]  lmac_in_ac3_txop_value         ; // AC3 TXOP Input
        reg        [15:0]  lmac_in_ac0_txnav_value        ; // AC0 TXNAV Value
        reg        [15:0]  lmac_in_ac1_txnav_value        ; // AC1 TXNAV Value 
        reg        [15:0]  lmac_in_ac2_txnav_value        ; // AC2 TXNAV Value 
        reg        [15:0]  lmac_in_ac3_txnav_value        ; // AC3 TXNAV Value 
        reg        [15:0]  lmac_in_phy_rxstart_delay      ; // Rx Start Delay Input
        reg        [15:0]  lmac_in_nav_value              ; // Input NAV Value
        reg        [15:0]  lmac_in_nav_reset_timer        ; // NAV Reset Timer Input
        reg        [3 :0]  lmac_in_difs_value             ; // Input DIFS Value
        reg        [3 :0]  lmac_in_ac0_aifs_value         ; // AC0 AIFS Value Input
        reg        [3 :0]  lmac_in_ac1_aifs_value         ; // AC1 AIFS Value Input 
        reg        [3 :0]  lmac_in_ac2_aifs_value         ; // AC2 AIFS Value Input 
        reg        [3 :0]  lmac_in_ac3_aifs_value         ; // AC3 AIFS Value Input
        reg        [3 :0]  lmac_in_slot_timer_value       ; // Slot Timer Input
        reg        [4 :0]  lmac_in_cts_timer_value        ; // CTS Timer Input
        reg        [4 :0]  lmac_in_ack_timer_value        ; // ACK Timer Input
        reg        [5 :0]  lmac_in_eifs_value             ; // EIFS Value Input
        reg        [15:0]  lmac_in_cwmin_value            ; // NON QOS CWMIN Value Input
        reg        [15:0]  lmac_in_ac0_cwmin_value        ; // AC0 CWMIN Value Input
        reg        [15:0]  lmac_in_ac1_cwmin_value        ; // AC1 CWMIN Value Input 
        reg        [15:0]  lmac_in_ac2_cwmin_value        ; // AC2 CWMIN Value Input 
        reg        [15:0]  lmac_in_ac3_cwmin_value        ; // AC3 CWMIN Value Input 
        reg        [15:0]  lmac_in_cwmax_value            ; // NON QOS CWMAX Value Input
        reg        [15:0]  lmac_in_ac0_cwmax_value        ; // AC0 CWMAX Value Input
        reg        [15:0]  lmac_in_ac1_cwmax_value        ; // AC1 CWMAX Value Input 
        reg        [15:0]  lmac_in_ac2_cwmax_value        ; // AC2 CWMAX Value Input 
        reg        [15:0]  lmac_in_ac3_cwmax_value        ; // AC3 CWMAX Value Input 
        reg        [47:0]  lmac_in_mac_addr               ; // MAC Address of STA
        reg                lmac_in_txready                ; // TX Ready Input
        reg        [47:0]  lmac_in_bar_address            ; // BAR Address Input
        reg        [15:0]  lmac_in_bar_scf                ; // BAR Sequence Control Input
        reg        [3 :0]  lmac_in_bar_tid                ; // BAR TID Input
        reg        [1 :0]  lmac_in_bar_bartype            ; // BAR Type Input
        reg        [15:0]  lmac_in_bar_duration           ; // BAR Duration Input
        reg                lmac_in_bar_valid              ; // BAR Valid Input
        reg        [15:0]  lmac_in_TXOP_limit_ac0         ; // AC0 TXOP Limit Input
        reg        [15:0]  lmac_in_TXOP_limit_ac1         ; // AC1 TXOP Limit Input
        reg        [15:0]  lmac_in_TXOP_limit_ac2         ; // AC2 TXOP Limit Input
        reg        [15:0]  lmac_in_TXOP_limit_ac3         ; // AC3 TXOP Limit Input
        reg        [15:0]  lmac_in_Long_Retry_Count       ; // Long Retry Count Input to Tx Module
        reg        [1 :0]  lmac_in_dma_ac                 ; // DMA AC Input
        reg                lmac_in_dma_wen                ; // DMA Write Enable Input
        reg        [31:0]  lmac_in_dma_data               ; // Input Data to DMA 
        wire       [12:0]  lmac_out_free_memory_ac0       ; // Memory Clear for AC0
        wire       [12:0]  lmac_out_free_memory_ac1       ; // Memory Clear for AC1
        wire       [12:0]  lmac_out_free_memory_ac2       ; // Memory Clear for AC2
        wire       [12:0]  lmac_out_free_memory_ac3       ; // Memory Clear for AC3
        wire       [31:0]  lmac_out_umac_data             ; // Data Output to UMAC
        wire               lmac_out_umac_ack              ; // ACK Output to UMAC
        wire               lmac_out_ev_ccareset_req       ; // CCA Reset Req Output
        wire               lmac_out_ev_txend_req          ; // Tx End Req Output
        wire               lmac_out_ev_txstart_req        ; // Tx Start Req Output
        wire               lmac_out_ev_rxend_ind          ; // Rx End Indication Output
        wire       [1 :0]  lmac_out_cca_status            ; // CCA Status Output
        wire       [15:0]  lmac_out_ac0_cw_value          ; // AC0 CW Value Output
        wire       [15:0]  lmac_out_ac1_cw_value          ; // AC1 CW Value Output
        wire       [15:0]  lmac_out_ac2_cw_value          ; // AC2 CW Value Output
        wire       [15:0]  lmac_out_ac3_cw_value          ; // AC3 CW Value Output
        wire       [15:0]  lmac_out_nav_value             ; // NAV Value Outupt from CAP
        wire       [15:0]  lmac_out_nav_reset_timer       ; // NAV Reset Timer Output from CAP
        wire               lmac_out_ac0_txready           ; // AC0 Tx Ready Output from CAP
        wire               lmac_out_ac1_txready           ; // AC1 Tx Ready Output from CAP 
        wire               lmac_out_ac2_txready           ; // AC2 Tx Ready Output from CAP 
        wire               lmac_out_ac3_txready           ; // AC3 Tx Ready Output from CAP 
        wire       [3:0]   lmac_out_slot_timer_value      ; // Slot Timer Value Output
        wire       [3:0]   lmac_out_ac0_aifs_value        ; // AC0 AIFS Value Output
        wire       [3:0]   lmac_out_ac1_aifs_value        ; // AC1 AIFS Value Output
        wire       [3:0]   lmac_out_ac2_aifs_value        ; // AC2 AIFS Value Output
        wire       [3:0]   lmac_out_ac3_aifs_value        ; // AC3 AIFS Value Output 
        wire       [4:0]   lmac_out_cts_timer_value       ; // CTS Timer Value Output
        wire       [4:0]   lmac_out_ack_timer_value       ; // ACK Timer Value Output 
        wire       [5:0]   lmac_out_eifs_value            ; // EIFS Output from CAP
        wire               lmac_out_ac0_txprog            ; // AC0 Tx Prog Value Output
        wire               lmac_out_ac1_txprog            ; // AC1 Tx Prog Value Output
        wire               lmac_out_ac2_txprog            ; // AC2 Tx Prog Value Output
        wire               lmac_out_ac3_txprog            ; // AC3 Tx Prog Value Output
        wire       [7:0]   lmac_out_txop_return_value     ; // TXOP Return Value
        wire       [7:0]   lmac_out_ac0_ssrc_value        ; // AC0 SSRC Value Output
        wire       [7:0]   lmac_out_ac1_ssrc_value        ; // AC1 SSRC Value Output
        wire       [7:0]   lmac_out_ac2_ssrc_value        ; // AC2 SSRC Value Output
        wire       [7:0]   lmac_out_ac3_ssrc_value        ; // AC3 SSRC Value Output
        wire       [7:0]   lmac_out_ac0_slrc_value        ; // AC0 SLRC Value Output
        wire       [7:0]   lmac_out_ac1_slrc_value        ; // AC1 SLRC Value Output 
        wire       [7:0]   lmac_out_ac2_slrc_value        ; // AC2 SLRC Value Output 
        wire       [7:0]   lmac_out_ac3_slrc_value        ; // AC3 SLRC Value Output  
        wire       [15:0]  lmac_out_ac0_backoff_value     ; // AC0 Backoff Value Output
        wire       [15:0]  lmac_out_ac1_backoff_value     ; // AC1 Backoff Value Output
        wire       [15:0]  lmac_out_ac2_backoff_value     ; // AC2 Backoff Value Output
        wire       [15:0]  lmac_out_ac3_backoff_value     ; // AC3 Backoff Value Output
        wire       [15:0]  lmac_out_ac0_txop_value        ; // AC0 TXOP Output Value
        wire       [15:0]  lmac_out_ac1_txop_value        ; // AC1 TXOP Output Value 
        wire       [15:0]  lmac_out_ac2_txop_value        ; // AC2 TXOP Output Value 
        wire       [15:0]  lmac_out_ac3_txop_value        ; // AC3 TXOP Output Value 
        wire               lmac_out_rx_ampdu_frames_cnt   ; // AMPDU Frame Count to UMAC
        wire               lmac_out_rx_ampdu_subframe_cnt ; // AMPDU Subframe Count to UMAC 
        wire               lmac_out_rx_phyerr_pkts_cnt    ; // PHY Error Packet Count to UMAC 
        wire               lmac_out_rx_frames_cnt         ; // Rx Frames Count to UMAC 
        wire               lmac_out_rx_multicast_cnt      ; // Multicast Frame count to UMAC 
        wire               lmac_out_rx_broadcast_cnt      ; // Broadcast Frame Count to UMAC
        wire               lmac_out_rx_frame_forus_cnt    ; // Frames for us count to UMAC 
        wire               lmac_out_dot11_fcs_error_cnt   ; // FCS Error count to UMAC 
        wire               lmac_out_frame_decrypt_enable  ; // Frame Decrypt Enable to Crypto Engine
        wire               lmac_out_frame_post_to_umac    ; // Output to post frame to UMAC
        wire               lmac_out_phy_start_req         ; // Output PHY Start Request from Tx
     	wire               lmac_out_phy_data_req          ; // Output PHY Data Request from Tx
        wire               lmac_out_phy_frame_val         ; // Output PHY Frame Valid from Tx
        wire       [7 :0]  lmac_out_phy_frame             ; // Output PHY Frame from Tx

parameter CLK_PERIOD = 3'h6;

// Script File Instantiation
uu_acmac_test U_LMAC_TEST();

// Top Module Instantiation
uu_acmac_top U_LMAC_TOP(
                        .clk                             (clk                              ), // Clock for LMAC
                        .rst_n                           (rst_n                            ), // Resetn for LMAC        			
                        .lmac_en                         (lmac_en                          ), // Enable for ALL LMAC Modules          
                        .cap_en                          (cap_en                           ), // Enable for CAP Module
                        .cp_en                           (cp_en                            ), // Enable for CP Module
                        .rx_en                           (rx_en                            ), // Enable for RX Module
                        .tx_en                           (tx_en                            ), // Enable for TX Module
                        .sta_en                          (sta_en                           ), // Enable for STA and BA Management Module
                        .lmac_in_umac_bea                (lmac_in_umac_bea                 ), // Byte Enable Input from UMAC
                        .lmac_in_umac_data               (lmac_in_umac_data                ), // Data Input from UMAC
                        .lmac_in_umac_addr               (lmac_in_umac_addr                ), // Address Input from UMAC
                        .lmac_in_umac_wrn_rd             (lmac_in_umac_wrn_rd              ), // WritenRead Input from UMAC
                        .lmac_in_umac_val                (lmac_in_umac_val                 ), // Data Valid from UMAC
                        .lmac_in_frame_valid             (lmac_in_frame_valid              ), // Input Frame Valid from DMA
                        .lmac_in_frame                   (lmac_in_frame                    ), // Input Frame from DMA  
                        .lmac_in_BSSBasicRateSet         (lmac_in_BSSBasicRateSet          ), // Input BSSBasic Rate Set 
                        .lmac_in_SIFS_timer_value        (lmac_in_SIFS_timer_value         ), // Input SIFS timer Value
                        .lmac_in_rxend_status            (lmac_in_rxend_status             ), // RX End Status Input
                        .lmac_in_filter_flag             (lmac_in_filter_flag              ), // LMAC Filter Flag Input
                        .lmac_in_ac0_txframeinfo_valid   (lmac_in_ac0_txframeinfo_valid    ), // AC0 Frame Valid to CAP
                        .lmac_in_ac1_txframeinfo_valid   (lmac_in_ac1_txframeinfo_valid    ), // AC1 Frame Valid to CAP
                        .lmac_in_ac2_txframeinfo_valid   (lmac_in_ac2_txframeinfo_valid    ), // AC2 Frame Valid to CAP
                        .lmac_in_ac3_txframeinfo_valid   (lmac_in_ac3_txframeinfo_valid    ), // AC3 Frame Valid to CAP
                        .lmac_in_ev_mac_reset            (lmac_in_ev_mac_reset             ), // MAC Reset Event Input
                        .lmac_in_ev_txstart_confirm      (lmac_in_ev_txstart_confirm       ), // Tx Start Confrim Event Input
                        .lmac_in_ev_txdata_confirm       (lmac_in_ev_txdata_confirm        ), // Tx Data Confirm Event reg
                        .lmac_in_ev_txready              (lmac_in_ev_txready               ), // TX Ready Event Input
                        .lmac_in_ev_timer_tick           (lmac_in_ev_timer_tick            ), // Timer Tick Event Input
                        .lmac_in_ev_txend_confirm        (lmac_in_ev_txend_confirm         ), // TX End Confirm Event
                        .lmac_in_ev_phyenergy_ind        (lmac_in_ev_phyenergy_ind         ), // PHY Energy Event Input
                        .lmac_in_ev_rxstart_ind          (lmac_in_ev_rxstart_ind           ), // RX Start Event Input
                        .lmac_in_ev_rxdata_ind           (lmac_in_ev_rxdata_ind            ), // Rx Data Event Input
                        .lmac_in_ev_rxend_ind            (lmac_in_ev_rxend_ind             ), // Rx End Event Input
                        .lmac_in_ev_phyrx_error          (lmac_in_ev_phyrx_error           ), // PHY Rx Error Input
                        .lmac_in_ev_ccareset_confirm     (lmac_in_ev_ccareset_confirm      ), // CCA Reset Confirm
                        .lmac_in_ev_txop_txstart         (lmac_in_ev_txop_txstart          ), // TXOP Tx Start Event
                        .lmac_in_qos_mode                (lmac_in_qos_mode                 ), // QOS Mode Input  
                        .lmac_in_RTS_threshold           (lmac_in_RTS_threshold            ), // RTS Threshold Input
                        .lmac_in_cca_status              (lmac_in_cca_status               ), // CCA Status Input
                        .lmac_in_ac_select               (lmac_in_ac_select                ), // Input AC Select
                        .lmac_in_channel_list            (lmac_in_channel_list             ), // Input Channel List
                        .lmac_in_txvec_chbw              (lmac_in_txvec_chbw               ), // Input Channel BW
                        .lmac_in_txvec_format            (lmac_in_txvec_format             ), // Input TX Vector Format
                        .lmac_in_aggr_count              (lmac_in_aggr_count               ), // Input Aggregation Count      
                        .lmac_in_ac0_txop_value          (lmac_in_ac0_txop_value           ), // AC0 TXOP Input
                        .lmac_in_ac1_txop_value          (lmac_in_ac1_txop_value           ), // AC1 TXOP Input 
                        .lmac_in_ac2_txop_value          (lmac_in_ac2_txop_value           ), // AC2 TXOP Input 
                        .lmac_in_ac3_txop_value          (lmac_in_ac3_txop_value           ), // AC3 TXOP Input
                        .lmac_in_ac0_txnav_value         (lmac_in_ac0_txnav_value          ), // AC0 TXNAV Value
                        .lmac_in_ac1_txnav_value         (lmac_in_ac1_txnav_value          ), // AC1 TXNAV Value 
                        .lmac_in_ac2_txnav_value         (lmac_in_ac2_txnav_value          ), // AC2 TXNAV Value 
                        .lmac_in_ac3_txnav_value         (lmac_in_ac3_txnav_value          ), // AC3 TXNAV Value 
                        .lmac_in_phy_rxstart_delay       (lmac_in_phy_rxstart_delay        ), // Rx Start Delay Input
                        .lmac_in_nav_value               (lmac_in_nav_value                ), // Input NAV Value
                        .lmac_in_nav_reset_timer         (lmac_in_nav_reset_timer          ), // NAV Reset Timer Input
                        .lmac_in_difs_value              (lmac_in_difs_value               ), // Input DIFS Value
                        .lmac_in_ac0_aifs_value          (lmac_in_ac0_aifs_value           ), // AC0 AIFS Value Input
                        .lmac_in_ac1_aifs_value          (lmac_in_ac1_aifs_value           ), // AC1 AIFS Value Input 
                        .lmac_in_ac2_aifs_value          (lmac_in_ac2_aifs_value           ), // AC2 AIFS Value Input 
                        .lmac_in_ac3_aifs_value          (lmac_in_ac3_aifs_value           ), // AC3 AIFS Value Input
                        .lmac_in_slot_timer_value        (lmac_in_slot_timer_value         ), // Slot Timer Input
                        .lmac_in_cts_timer_value         (lmac_in_cts_timer_value          ), // CTS Timer Input
                        .lmac_in_ack_timer_value         (lmac_in_ack_timer_value          ), // ACK Timer Input
                        .lmac_in_eifs_value              (lmac_in_eifs_value               ), // EIFS Value Input
                        .lmac_in_cwmin_value             (lmac_in_cwmin_value              ), // NON QOS CWMIN Value Input
                        .lmac_in_ac0_cwmin_value         (lmac_in_ac0_cwmin_value          ), // AC0 CWMIN Value Input
                        .lmac_in_ac1_cwmin_value         (lmac_in_ac1_cwmin_value          ), // AC1 CWMIN Value Input 
                        .lmac_in_ac2_cwmin_value         (lmac_in_ac2_cwmin_value          ), // AC2 CWMIN Value Input 
                        .lmac_in_ac3_cwmin_value         (lmac_in_ac3_cwmin_value          ), // AC3 CWMIN Value Input 
                        .lmac_in_cwmax_value             (lmac_in_cwmax_value              ), // NON QOS CWMAX Value Input
                        .lmac_in_ac0_cwmax_value         (lmac_in_ac0_cwmax_value          ), // AC0 CWMAX Value Input
                        .lmac_in_ac1_cwmax_value         (lmac_in_ac1_cwmax_value          ), // AC1 CWMAX Value Input 
                        .lmac_in_ac2_cwmax_value         (lmac_in_ac2_cwmax_value          ), // AC2 CWMAX Value Input 
                        .lmac_in_ac3_cwmax_value         (lmac_in_ac3_cwmax_value          ), // AC3 CWMAX Value Input 
                        .lmac_in_mac_addr                (lmac_in_mac_addr                 ), // MAC Address of STA
                        .lmac_in_txready                 (lmac_in_txready                  ), // TX Ready Input
                        .lmac_in_bar_address             (lmac_in_bar_address              ), // BAR Address Input
                        .lmac_in_bar_scf                 (lmac_in_bar_scf                  ), // BAR Sequence Control Input
                        .lmac_in_bar_tid                 (lmac_in_bar_tid                  ), // BAR TID Input
                        .lmac_in_bar_bartype             (lmac_in_bar_bartype              ), // BAR Type Input
                        .lmac_in_bar_duration            (lmac_in_bar_duration             ), // BAR Duration Input
                        .lmac_in_bar_valid               (lmac_in_bar_valid                ), // BAR Valid Input
                        .lmac_in_TXOP_limit_ac0          (lmac_in_TXOP_limit_ac0           ), // AC0 TXOP Limit Input
                        .lmac_in_TXOP_limit_ac1          (lmac_in_TXOP_limit_ac1           ), // AC1 TXOP Limit Input
                        .lmac_in_TXOP_limit_ac2          (lmac_in_TXOP_limit_ac2           ), // AC2 TXOP Limit Input
                        .lmac_in_TXOP_limit_ac3          (lmac_in_TXOP_limit_ac3           ), // AC3 TXOP Limit Input
                        .lmac_in_Long_Retry_Count        (lmac_in_Long_Retry_Count         ), // Long Retry Count Input to Tx Module
                        .lmac_in_dma_ac                  (lmac_in_dma_ac                   ), // DMA AC Input
                        .lmac_in_dma_wen                 (lmac_in_dma_wen                  ), // DMA Write Enable Input
                        .lmac_in_dma_data                (lmac_in_dma_data                 ), // Input Data to DMA 
                        .lmac_out_free_memory_ac0        (lmac_out_free_memory_ac0         ), // Memory Clear for AC0
                        .lmac_out_free_memory_ac1        (lmac_out_free_memory_ac1         ), // Memory Clear for AC1
                        .lmac_out_free_memory_ac2        (lmac_out_free_memory_ac2         ), // Memory Clear for AC2
                        .lmac_out_free_memory_ac3        (lmac_out_free_memory_ac3         ), // Memory Clear for AC3
                        .lmac_out_umac_data              (lmac_out_umac_data               ), // Data Output to UMAC
                        .lmac_out_umac_ack               (lmac_out_umac_ack                ), // ACK Output to UMAC
                        .lmac_out_ev_ccareset_req        (lmac_out_ev_ccareset_req         ), // CCA Reset Req Output
                        .lmac_out_ev_txend_req           (lmac_out_ev_txend_req            ), // Tx End Req Output
                        .lmac_out_ev_txstart_req         (lmac_out_ev_txstart_req          ), // Tx Start Req Output
                        .lmac_out_ev_rxend_ind           (lmac_out_ev_rxend_ind            ), // Rx End Indication Output
                        .lmac_out_cca_status             (lmac_out_cca_status              ), // CCA Status Output
                        .lmac_out_ac0_cw_value           (lmac_out_ac0_cw_value            ), // AC0 CW Value Output
                        .lmac_out_ac1_cw_value           (lmac_out_ac1_cw_value            ), // AC1 CW Value Output
                        .lmac_out_ac2_cw_value           (lmac_out_ac2_cw_value            ), // AC2 CW Value Output
                        .lmac_out_ac3_cw_value           (lmac_out_ac3_cw_value            ), // AC3 CW Value Output
                        .lmac_out_nav_value              (lmac_out_nav_value               ), // NAV Value Outupt from CAP
                        .lmac_out_nav_reset_timer        (lmac_out_nav_reset_timer         ), // NAV Reset Timer Output from CAP
                        .lmac_out_ac0_txready            (lmac_out_ac0_txready             ), // AC0 Tx Ready Output from CAP
                        .lmac_out_ac1_txready            (lmac_out_ac1_txready             ), // AC1 Tx Ready Output from CAP 
                        .lmac_out_ac2_txready            (lmac_out_ac2_txready             ), // AC2 Tx Ready Output from CAP 
                        .lmac_out_ac3_txready            (lmac_out_ac3_txready             ), // AC3 Tx Ready Output from CAP 
                        .lmac_out_slot_timer_value       (lmac_out_slot_timer_value        ), // Slot Timer Value Output
                        .lmac_out_ac0_aifs_value         (lmac_out_ac0_aifs_value          ), // AC0 AIFS Value Output
                        .lmac_out_ac1_aifs_value         (lmac_out_ac1_aifs_value          ), // AC1 AIFS Value Output
                        .lmac_out_ac2_aifs_value         (lmac_out_ac2_aifs_value          ), // AC2 AIFS Value Output
                        .lmac_out_ac3_aifs_value         (lmac_out_ac3_aifs_value          ), // AC3 AIFS Value Output 
                        .lmac_out_cts_timer_value        (lmac_out_cts_timer_value         ), // CTS Timer Value Output
                        .lmac_out_ack_timer_value        (lmac_out_ack_timer_value         ), // ACK Timer Value Output 
                        .lmac_out_eifs_value             (lmac_out_eifs_value              ), // EIFS Output from CAP
                        .lmac_out_ac0_txprog             (lmac_out_ac0_txprog              ), // AC0 Tx Prog Value Output
                        .lmac_out_ac1_txprog             (lmac_out_ac1_txprog              ), // AC1 Tx Prog Value Output
                        .lmac_out_ac2_txprog             (lmac_out_ac2_txprog              ), // AC2 Tx Prog Value Output
                        .lmac_out_ac3_txprog             (lmac_out_ac3_txprog              ), // AC3 Tx Prog Value Output
                        .lmac_out_txop_return_value      (lmac_out_txop_return_value       ), // TXOP Return Value
                        .lmac_out_ac0_ssrc_value         (lmac_out_ac0_ssrc_value          ), // AC0 SSRC Value Output
                        .lmac_out_ac1_ssrc_value         (lmac_out_ac1_ssrc_value          ), // AC1 SSRC Value Output
                        .lmac_out_ac2_ssrc_value         (lmac_out_ac2_ssrc_value          ), // AC2 SSRC Value Output
                        .lmac_out_ac3_ssrc_value         (lmac_out_ac3_ssrc_value          ), // AC3 SSRC Value Output
                        .lmac_out_ac0_slrc_value         (lmac_out_ac0_slrc_value          ), // AC0 SLRC Value Output
                        .lmac_out_ac1_slrc_value         (lmac_out_ac1_slrc_value          ), // AC1 SLRC Value Output 
                        .lmac_out_ac2_slrc_value         (lmac_out_ac2_slrc_value          ), // AC2 SLRC Value Output 
                        .lmac_out_ac3_slrc_value         (lmac_out_ac3_slrc_value          ), // AC3 SLRC Value Output  
                        .lmac_out_ac0_backoff_value      (lmac_out_ac0_backoff_value       ), // AC0 Backoff Value Output
                        .lmac_out_ac1_backoff_value      (lmac_out_ac1_backoff_value       ), // AC1 Backoff Value Output
                        .lmac_out_ac2_backoff_value      (lmac_out_ac2_backoff_value       ), // AC2 Backoff Value Output
                        .lmac_out_ac3_backoff_value      (lmac_out_ac3_backoff_value       ), // AC3 Backoff Value Output
                        .lmac_out_ac0_txop_value         (lmac_out_ac0_txop_value          ), // AC0 TXOP Output Value
                        .lmac_out_ac1_txop_value         (lmac_out_ac1_txop_value          ), // AC1 TXOP Output Value 
                        .lmac_out_ac2_txop_value         (lmac_out_ac2_txop_value          ), // AC2 TXOP Output Value 
                        .lmac_out_ac3_txop_value         (lmac_out_ac3_txop_value          ), // AC3 TXOP Output Value 
                        .lmac_out_rx_ampdu_frames_cnt    (lmac_out_rx_ampdu_frames_cnt     ), // AMPDU Frame Count to UMAC
                        .lmac_out_rx_ampdu_subframe_cnt  (lmac_out_rx_ampdu_subframe_cnt   ), // AMPDU Subframe Count to UMAC 
                        .lmac_out_rx_phyerr_pkts_cnt     (lmac_out_rx_phyerr_pkts_cnt      ), // PHY Error Packet Count to UMAC 
                        .lmac_out_rx_frames_cnt          (lmac_out_rx_frames_cnt           ), // Rx Frames Count to UMAC 
                        .lmac_out_rx_multicast_cnt       (lmac_out_rx_multicast_cnt        ), // Multicast Frame count to UMAC 
                        .lmac_out_rx_broadcast_cnt       (lmac_out_rx_broadcast_cnt        ), // Broadcast Frame Count to UMAC
                        .lmac_out_rx_frame_forus_cnt     (lmac_out_rx_frame_forus_cnt      ), // Frames for us count to UMAC 
                        .lmac_out_dot11_fcs_error_cnt    (lmac_out_dot11_fcs_error_cnt     ), // FCS Error count to UMAC 
                        .lmac_out_frame_decrypt_enable   (lmac_out_frame_decrypt_enable    ), // Frame Decrypt Enable to Crypto Engine
                        .lmac_out_frame_post_to_umac     (lmac_out_frame_post_to_umac      ), // Output to post frame to UMAC
                        .lmac_out_phy_start_req          (lmac_out_phy_start_req           ), // Output PHY Start Request from Tx
                        .lmac_out_phy_data_req           (lmac_out_phy_data_req            ), // Output PHY Data Request from Tx
                        .lmac_out_phy_frame_val          (lmac_out_phy_frame_val           ), // Output PHY Frame Valid from Tx
                        .lmac_out_phy_frame              (lmac_out_phy_frame               )  // Output PHY Frame from Tx
                       );

initial
begin
    clk = 1'b0;
    rst_n = 1'b0;

    #10
    rst_n = 1'b1;
end // End of Initial Block

// Clock toggling at half the Clock Period
always #(CLK_PERIOD >> 2) clk = ~clk;

initial
begin
        lmac_en                        = 1'b0;  
        cap_en                         = 1'b0; 
        cp_en                          = 1'b0; 
        rx_en                          = 1'b0; 
        tx_en                          = 1'b0; 
        sta_en                         = 1'b0; 
        lmac_in_umac_bea               = 4'h0; 
        lmac_in_umac_data              = 32'h0; 
        lmac_in_umac_addr              = 14'h0; 
        lmac_in_umac_wrn_rd            = 1'b0; 
        lmac_in_umac_val               = 1'b0; 
        lmac_in_frame_valid            = 1'b0; 
        lmac_in_frame                  = 8'h0; 
        lmac_in_BSSBasicRateSet        = 16'h0; 
        lmac_in_SIFS_timer_value       = 16'h0; 
        lmac_in_rxend_status           = 2'h0; 
        lmac_in_filter_flag            = 10'h0; 
        lmac_in_ac0_txframeinfo_valid  = 1'b0; 
        lmac_in_ac1_txframeinfo_valid  = 1'b0; 
        lmac_in_ac2_txframeinfo_valid  = 1'b0; 
        lmac_in_ac3_txframeinfo_valid  = 1'b0; 
        lmac_in_ev_mac_reset           = 1'b0; 
        lmac_in_ev_txstart_confirm     = 1'b0; 
        lmac_in_ev_txdata_confirm      = 1'b0; 
        lmac_in_ev_txready             = 1'b0; 
        lmac_in_ev_timer_tick          = 1'b0; 
        lmac_in_ev_txend_confirm       = 1'b0; 
        lmac_in_ev_phyenergy_ind       = 1'b0; 
        lmac_in_ev_rxstart_ind         = 1'b0; 
        lmac_in_ev_rxdata_ind          = 1'b0; 
        lmac_in_ev_rxend_ind           = 1'b0; 
        lmac_in_ev_phyrx_error         = 1'b0; 
        lmac_in_ev_ccareset_confirm    = 1'b0; 
        lmac_in_ev_txop_txstart        = 1'b0; 
        lmac_in_qos_mode               = 1'b0; 
        lmac_in_RTS_threshold          = 16'h0; 
        lmac_in_cca_status             = 2'h0; 
        lmac_in_ac_select              = 2'h0; 
        lmac_in_channel_list           = 3'h0; 
        lmac_in_txvec_chbw             = 3'h0; 
        lmac_in_txvec_format           = 2'h0; 
        lmac_in_aggr_count             = 8'h0; 
        lmac_in_ac0_txop_value         = 16'h0; 
        lmac_in_ac1_txop_value         = 16'h0; 
        lmac_in_ac2_txop_value         = 16'h0; 
        lmac_in_ac3_txop_value         = 16'h0; 
        lmac_in_ac0_txnav_value        = 16'h0; 
        lmac_in_ac1_txnav_value        = 16'h0; 
        lmac_in_ac2_txnav_value        = 16'h0; 
        lmac_in_ac3_txnav_value        = 16'h0; 
        lmac_in_phy_rxstart_delay      = 16'h0; 
        lmac_in_nav_value              = 16'h0; 
        lmac_in_nav_reset_timer        = 16'h0; 
        lmac_in_difs_value             = 4'h0; 
        lmac_in_ac0_aifs_value         = 4'h0; 
        lmac_in_ac1_aifs_value         = 4'h0; 
        lmac_in_ac2_aifs_value         = 4'h0; 
        lmac_in_ac3_aifs_value         = 4'h0; 
        lmac_in_slot_timer_value       = 4'h0; 
        lmac_in_cts_timer_value        = 5'h0; 
        lmac_in_ack_timer_value        = 5'h0; 
        lmac_in_eifs_value             = 6'h0; 
        lmac_in_cwmin_value            = 16'h0; 
        lmac_in_ac0_cwmin_value        = 16'h0; 
        lmac_in_ac1_cwmin_value        = 16'h0; 
        lmac_in_ac2_cwmin_value        = 16'h0; 
        lmac_in_ac3_cwmin_value        = 16'h0; 
        lmac_in_cwmax_value            = 16'h0; 
        lmac_in_ac0_cwmax_value        = 16'h0; 
        lmac_in_ac1_cwmax_value        = 16'h0; 
        lmac_in_ac2_cwmax_value        = 16'h0; 
        lmac_in_ac3_cwmax_value        = 16'h0; 
        lmac_in_mac_addr              = 48'h0; 
        lmac_in_txready               = 1'b0; 
        lmac_in_bar_address           = 48'h0; 
        lmac_in_bar_scf               = 16'h0; 
        lmac_in_bar_tid               = 4'h0; 
        lmac_in_bar_bartype           = 2'h0; 
        lmac_in_bar_duration          = 16'h0; 
        lmac_in_bar_valid             = 1'b0; 
        lmac_in_TXOP_limit_ac0        = 16'h0; 
        lmac_in_TXOP_limit_ac1        = 16'h0; 
        lmac_in_TXOP_limit_ac2        = 16'h0; 
        lmac_in_TXOP_limit_ac3        = 16'h0; 
        lmac_in_Long_Retry_Count      = 16'h0; 
        lmac_in_dma_ac                = 2'h0; 
        lmac_in_dma_wen               = 1'b0; 
        lmac_in_dma_data              = 32'h0; 
end // End of Initial Block

endmodule

