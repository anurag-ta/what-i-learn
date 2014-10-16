`timescale 1ns/1ns

module uu_acmac_cap_tb;
 reg                                     clk;
 reg                                     rst_n;
 reg                                     cap_in_enable;

 // Trigger Events into the CAP Module
 reg                                     cap_in_ev_mac_reset;
 reg                                     cap_in_ev_txready;
 reg                                     cap_in_ev_timer_tick;
 reg                                     cap_in_ev_txstart_confirm;
 reg                                     cap_in_ev_txdata_confirm;
 reg                                     cap_in_ev_txend_confirm;
 reg                                     cap_in_ev_phyenergy_ind;
 reg                                     cap_in_ev_rxstart_ind;
 reg                                     cap_in_ev_rxdata_ind;
 reg                                     cap_in_ev_rxend_ind;
 reg                                     cap_in_ev_phyrx_error;
 reg                                     cap_in_ev_ccareset_confirm;

 reg                                     cap_in_frame_valid;
 reg [19:0]                              cap_in_rts_threshold_len;
 reg [19:0]                              cap_in_frame_length;
 reg [7:0]                               cap_in_frame;
 reg [7:0]                               cap_in_aggr_count;
 reg [31:0]                              cap_in_txhandler_ret_value;	
 reg                                     cap_in_txhandler_ret_valid;
 
 // regs used during IDLE State
 reg                                     cap_in_qos_mode;
 reg [1:0]                               cap_in_cca_status;
 reg [2:0]                               cap_in_channel_list;
 reg [2:0]                               cap_in_txvec_chbw;

 reg [1:0]                               cap_in_txvec_format;

 reg [3:0]                               cap_in_difs_value;
 reg [3:0]                               cap_in_ac0_aifs_value;
 reg [3:0]                               cap_in_ac1_aifs_value;
 reg [3:0]                               cap_in_ac2_aifs_value;  
 reg [3:0]                               cap_in_ac3_aifs_value; 
 reg [5:0]                               cap_in_eifs_value;


 reg [15:0]                              cap_in_cwmin_value;
 reg [15:0]                              cap_in_ac0_cwmin_value;
 reg [15:0]                              cap_in_ac1_cwmin_value;
 reg [15:0]                              cap_in_ac2_cwmin_value;
 reg [15:0]                              cap_in_ac3_cwmin_value;
 reg [15:0]                              cap_in_cwmax_value;
 reg [15:0]                              cap_in_ac0_cwmax_value;
 reg [15:0]                              cap_in_ac1_cwmax_value;
 reg [15:0]                              cap_in_ac2_cwmax_value;
 reg [15:0]                              cap_in_ac3_cwmax_value;

 reg [15:0]                              cap_in_ac0_txnav_value;
 reg [15:0]                              cap_in_ac1_txnav_value; 
 reg [15:0]                              cap_in_ac2_txnav_value;
 reg [15:0]                              cap_in_ac3_txnav_value;

 // regs used during the RECEIVE State
 reg [3:0]                               cap_in_slot_timer_value;
 reg [4:0]                               cap_in_cts_timer_value;
 reg [4:0]                               cap_in_ack_timer_value;
 reg [31:0]                              cap_in_rxhandler_ret_value;
 reg [15:0]                              cap_in_sifs_timer_value;
 reg [15:0]                              cap_in_phy_rxstart_delay;
 reg [47:0]                              cap_in_mac_addr; 

 reg [7:0]                               cap_in_short_retry_count;
 reg [7:0]                               cap_in_long_retry_count;
 
 // regs used during the TRANSMIT State
 reg                                     cap_in_ac0_txframeinfo_valid;
 reg                                     cap_in_ac1_txframeinfo_valid;
 reg                                     cap_in_ac2_txframeinfo_valid;
 reg                                     cap_in_ac3_txframeinfo_valid;

 `ifdef RTL_TEST_SIM
 reg                                      test_FLAG;
 reg                                      test_AC0_TXREADY; 
 reg                                      test_AC1_TXREADY; 
 reg                                      test_AC2_TXREADY; 
 reg                                      test_AC3_TXREADY; 
 reg                                      test_AC0_TXPROG; 
 reg                                      test_AC1_TXPROG; 
 reg                                      test_AC2_TXPROG; 
 reg                                      test_AC3_TXPROG; 
 reg [1:0]                                test_CCA_STATUS;
 reg [2:0]                                test_TXOP_OWNER_AC;
 reg [3:0]                                test_AC0_AIFS;
 reg [3:0]                                test_AC1_AIFS;
 reg [3:0]                                test_AC2_AIFS;
 reg [3:0]                                test_AC3_AIFS;
 reg [4:0]                                test_ACK_TIMER;
 reg [4:0]                                test_CTS_TIMER;
 reg [5:0]                                test_EIFS_TIMER;
 reg [7:0]                                test_STATE;
 reg [7:0]                                test_AC0_SSRC;
 reg [7:0]                                test_AC1_SSRC;
 reg [7:0]                                test_AC2_SSRC;
 reg [7:0]                                test_AC3_SSRC;
 reg [7:0]                                test_AC0_SLRC;
 reg [7:0]                                test_AC1_SLRC;
 reg [7:0]                                test_AC2_SLRC;
 reg [7:0]                                test_AC3_SLRC;
 reg [15:0]                               test_AC0_CWVAL;
 reg [15:0]                               test_AC1_CWVAL;
 reg [15:0]                               test_AC2_CWVAL;
 reg [15:0]                               test_AC3_CWVAL;
 reg [15:0]                               test_AC0_BACKOFF;
 reg [15:0]                               test_AC1_BACKOFF;
 reg [15:0]                               test_AC2_BACKOFF;
 reg [15:0]                               test_AC3_BACKOFF;
 reg [15:0]                               test_AC0_TXNAV;
 reg [15:0]                               test_AC1_TXNAV;
 reg [15:0]                               test_AC2_TXNAV;
 reg [15:0]                               test_AC3_TXNAV;
 reg [15:0]                               test_NAV_RESET;
 reg [15:0]                               test_NAV_VALUE;
`endif

 // Trigger Events From the CAP Module
 wire                                    cap_out_ev_ccareset_req;
 wire                                    cap_out_ev_txdata_req;
 wire                                    cap_out_ev_txend_req;
 wire                                    cap_out_ev_rxend_ind;
 wire                                    cap_out_ev_txop_txstart_req;
 wire                                    cap_out_ev_txsendmore_data_req;

 // Outputs for the IDLE State 
 wire [2:0]                              cap_out_channel_bw;

 // Outputs for the RECEIVE State 
 wire                                    cap_out_mpdu_status;
 wire                                    cap_out_mpdu_discard;
 wire [2:0]                              cap_out_txop_owner_ac;

 wire     [15:0]                         cap_out_ac0_txnav_value;
 wire     [15:0]                         cap_out_ac1_txnav_value;
 wire     [15:0]                         cap_out_ac2_txnav_value;
 wire     [15:0]                         cap_out_ac3_txnav_value;

 // Outputs for TXOP_RX State
 wire                                    cap_out_frame_info_retry;
 wire                                    cap_out_clear_ampdu;
 wire     [7:0]                          cap_out_ac0_src;
 wire     [7:0]                          cap_out_ac1_src;
 wire     [7:0]                          cap_out_ac2_src;
 wire     [7:0]                          cap_out_ac3_src;
 wire     [7:0]                          cap_out_ac0_lrc;
 wire     [7:0]                          cap_out_ac1_lrc;
 wire     [7:0]                          cap_out_ac2_lrc;
 wire     [7:0]                          cap_out_ac3_lrc;
 wire                                    cap_out_cp_wait_for_ack;
 wire                                    cap_out_cp_wait_for_cts;
 wire                                    cap_out_fb_update;
 wire                                    cap_out_send_ack_frame;

uu_acmac_cap  U_CAP (

  .cap_in_clk(clk),
  .rst_n(rst_n),
  .cap_in_enable(cap_in_enable),

 // Trigger Events into the CAP Module
   .cap_in_ev_mac_reset(cap_in_ev_mac_reset),
   .cap_in_ev_txready(cap_in_ev_txready),
   .cap_in_ev_timer_tick(cap_in_ev_timer_tick),
   .cap_in_ev_txstart_confirm(cap_in_ev_txstart_confirm),
   .cap_in_ev_txdata_confirm(cap_in_ev_txdata_confirm),
   .cap_in_ev_txend_confirm(cap_in_ev_txend_confirm),
   .cap_in_ev_phyenergy_ind(cap_in_ev_phyenergy_ind),
   .cap_in_ev_rxstart_ind(cap_in_ev_rxstart_ind),
   .cap_in_ev_rxdata_ind(cap_in_ev_rxdata_ind),
   .cap_in_ev_rxend_ind(cap_in_ev_rxend_ind),
   .cap_in_ev_phyrx_error(cap_in_ev_phyrx_error),
   .cap_in_ev_ccareset_confirm(cap_in_ev_ccareset_confirm),
  
  .cap_in_frame_valid(cap_in_frame_valid),
  .cap_in_rts_threshold_len(cap_in_rts_threshold_len),
  .cap_in_frame_length(cap_in_frame_length),
  .cap_in_frame(cap_in_frame),
  .cap_in_txhandler_ret_value(cap_in_txhandler_ret_value),
  .cap_in_txhandler_ret_valid(cap_in_txhandler_ret_valid),
  .cap_in_aggr_count(cap_in_aggr_count),
 
  .cap_in_qos_mode(cap_in_qos_mode),
  .cap_in_cca_status(cap_in_cca_status),
  .cap_in_channel_list(cap_in_channel_list),
  .cap_in_txvec_chbw(cap_in_txvec_chbw),

  .cap_in_txvec_format(cap_in_txvec_format),

  .cap_in_difs_value(cap_in_difs_value),
  .cap_in_ac0_aifs_value(cap_in_ac0_aifs_value),
  .cap_in_ac1_aifs_value(cap_in_ac1_aifs_value),
  .cap_in_ac2_aifs_value(cap_in_ac2_aifs_value),  
  .cap_in_ac3_aifs_value(cap_in_ac3_aifs_value),
  .cap_in_eifs_value(cap_in_eifs_value),
  .cap_in_cwmin_value(cap_in_cwmin_value),
  .cap_in_ac0_cwmin_value(cap_in_ac0_cwmin_value),
  .cap_in_ac1_cwmin_value(cap_in_ac1_cwmin_value),
  .cap_in_ac2_cwmin_value(cap_in_ac2_cwmin_value),
  .cap_in_ac3_cwmin_value(cap_in_ac3_cwmin_value),
  .cap_in_cwmax_value(cap_in_cwmax_value),
  .cap_in_ac0_cwmax_value(cap_in_ac0_cwmax_value),
  .cap_in_ac1_cwmax_value(cap_in_ac1_cwmax_value),
  .cap_in_ac2_cwmax_value(cap_in_ac2_cwmax_value),
  .cap_in_ac3_cwmax_value(cap_in_ac3_cwmax_value),

  .cap_in_ac0_txnav_value(cap_in_ac0_txnav_value),
  .cap_in_ac1_txnav_value(cap_in_ac1_txnav_value), 
  .cap_in_ac2_txnav_value(cap_in_ac2_txnav_value),
  .cap_in_ac3_txnav_value(cap_in_ac3_txnav_value),

  .cap_in_slot_timer_value(cap_in_slot_timer_value),
  .cap_in_cts_timer_value(cap_in_cts_timer_value),
  .cap_in_ack_timer_value(cap_in_ack_timer_value),
  .cap_in_rxhandler_ret_value(cap_in_rxhandler_ret_value),
  .cap_in_sifs_timer_value(cap_in_sifs_timer_value),
  .cap_in_phy_rxstart_delay(cap_in_phy_rxstart_delay),
  .cap_in_mac_addr(cap_in_mac_addr), 
  .cap_in_ac0_txframeinfo_valid(cap_in_ac0_txframeinfo_valid),
  .cap_in_ac1_txframeinfo_valid(cap_in_ac1_txframeinfo_valid),
  .cap_in_ac2_txframeinfo_valid(cap_in_ac2_txframeinfo_valid),
  .cap_in_ac3_txframeinfo_valid(cap_in_ac3_txframeinfo_valid),
  .cap_in_short_retry_count(cap_in_short_retry_count),
  .cap_in_long_retry_count(cap_in_long_retry_count),


 `ifdef RTL_TEST_SIM
  .test_FLAG(test_FLAG),
  .test_AC0_TXREADY(test_AC0_TXREADY), 
  .test_AC1_TXREADY(test_AC1_TXREADY), 
  .test_AC2_TXREADY(test_AC2_TXREADY), 
  .test_AC3_TXREADY(test_AC3_TXREADY), 
  .test_AC0_TXPROG(test_AC0_TXPROG), 
  .test_AC1_TXPROG(test_AC1_TXPROG), 
  .test_AC2_TXPROG(test_AC2_TXPROG), 
  .test_AC3_TXPROG(test_AC3_TXPROG), 
  .test_CCA_STATUS(test_CCA_STATUS),
  .test_TXOP_OWNER_AC(test_TXOP_OWNER_AC),
  .test_AC0_AIFS(test_AC0_AIFS),
  .test_AC1_AIFS(test_AC1_AIFS),
  .test_AC2_AIFS(test_AC2_AIFS),
  .test_AC3_AIFS(test_AC3_AIFS),
  .test_ACK_TIMER(test_ACK_TIMER),
  .test_CTS_TIMER(test_CTS_TIMER),
  .test_EIFS_TIMER(test_EIFS_TIMER),
  .test_STATE(test_STATE),
  .test_AC0_SSRC(test_AC0_SSRC),
  .test_AC1_SSRC(test_AC1_SSRC),
  .test_AC2_SSRC(test_AC2_SSRC),
  .test_AC3_SSRC(test_AC3_SSRC),
  .test_AC0_SLRC(test_AC0_SLRC),
  .test_AC1_SLRC(test_AC1_SLRC),
  .test_AC2_SLRC(test_AC2_SLRC),
  .test_AC3_SLRC(test_AC3_SLRC),
  .test_AC0_CWVAL(test_AC0_CWVAL),
  .test_AC1_CWVAL(test_AC1_CWVAL),
  .test_AC2_CWVAL(test_AC2_CWVAL),
  .test_AC3_CWVAL(test_AC3_CWVAL),
  .test_AC0_BACKOFF(test_AC0_BACKOFF),
  .test_AC1_BACKOFF(test_AC1_BACKOFF),
  .test_AC2_BACKOFF(test_AC2_BACKOFF),
  .test_AC3_BACKOFF(test_AC3_BACKOFF),
  .test_AC0_TXNAV(test_AC0_TXNAV),
  .test_AC1_TXNAV(test_AC1_TXNAV),
  .test_AC2_TXNAV(test_AC2_TXNAV),
  .test_AC3_TXNAV(test_AC3_TXNAV),
  .test_NAV_RESET(test_NAV_RESET),
  .test_NAV_VALUE(test_NAV_VALUE),
 `endif

 // Trigger Events From the .cap Module
 .cap_out_ev_ccareset_req(cap_out_ev_ccareset_req),
 .cap_out_ev_txdata_req(cap_out_ev_txdata_req),
 .cap_out_ev_txend_req(cap_out_ev_txend_req),
 .cap_out_ev_rxend_ind(cap_out_ev_rxend_ind),
 .cap_out_ev_txop_txstart_req(cap_out_ev_txop_txstart_req),
 .cap_out_ev_txsendmore_data_req(cap_out_ev_txsendmore_data_req),

 // Outputs for the IDLE State 
 .cap_out_channel_bw(cap_out_channel_bw),

 // Outputs for the RECEIVE State 
 .cap_out_mpdu_status(cap_out_mpdu_status),
 .cap_out_mpdu_discard(cap_out_mpdu_discard),
 .cap_out_txop_owner_ac(cap_out_txop_owner_ac),

 .cap_out_ac0_txnav_value(cap_out_ac0_txnav_value),
 .cap_out_ac1_txnav_value(cap_out_ac1_txnav_value),
 .cap_out_ac2_txnav_value(cap_out_ac2_txnav_value),
 .cap_out_ac3_txnav_value(cap_out_ac3_txnav_value),

 // Outputs for TXOP_RX State
 .cap_out_frame_info_retry(cap_out_frame_info_retry),
 .cap_out_clear_ampdu(cap_out_clear_ampdu),
 .cap_out_ac0_src(cap_out_ac0_src),
 .cap_out_ac1_src(cap_out_ac1_src),
 .cap_out_ac2_src(cap_out_ac2_src),
 .cap_out_ac3_src(cap_out_ac3_src),
 .cap_out_ac0_lrc(cap_out_ac0_lrc),
 .cap_out_ac1_lrc(cap_out_ac1_lrc),
 .cap_out_ac2_lrc(cap_out_ac2_lrc),
 .cap_out_ac3_lrc(cap_out_ac3_lrc),
 .cap_out_cp_wait_for_ack(cap_out_cp_wait_for_ack),
 .cap_out_cp_wait_for_cts(cap_out_cp_wait_for_cts),
 .cap_out_fb_update(cap_out_fb_update),
 .cap_out_send_ack_frame(cap_out_send_ack_frame)

  );

uu_acmac_states_chain_test test();


initial
 begin

clk=0;
rst_n=1;
cap_in_enable=0;


cap_in_ev_mac_reset=0;
cap_in_ev_txready=0;
cap_in_ev_timer_tick=0;
cap_in_ev_txstart_confirm=0;
cap_in_ev_txdata_confirm=0;
cap_in_ev_txend_confirm=0;
cap_in_ev_phyenergy_ind=0;
cap_in_ev_rxstart_ind=0;
cap_in_ev_rxdata_ind=0;
cap_in_ev_rxend_ind=0;
cap_in_ev_phyrx_error=0;
cap_in_ev_ccareset_confirm=0;

cap_in_frame_valid=0;
cap_in_rts_threshold_len=0;
cap_in_frame_length=0;
cap_in_frame=0;
cap_in_txhandler_ret_value = 0;
cap_in_txhandler_ret_valid = 0;
cap_in_aggr_count = 0;

cap_in_qos_mode=0;
cap_in_cca_status=0;
cap_in_channel_list=3'h0;
cap_in_txvec_chbw=0;
cap_in_txvec_format=0;

cap_in_difs_value=0;
cap_in_ac0_aifs_value=0;
cap_in_ac1_aifs_value=0;
cap_in_ac2_aifs_value=0;  
cap_in_ac3_aifs_value=0;
cap_in_eifs_value=0;
cap_in_cwmin_value=0;
cap_in_ac0_cwmin_value=0;
cap_in_ac1_cwmin_value=0;
cap_in_ac2_cwmin_value=0;
cap_in_ac3_cwmin_value=0;
cap_in_cwmax_value=0;
cap_in_ac0_cwmax_value=0;
cap_in_ac1_cwmax_value=0;
cap_in_ac2_cwmax_value=0;
cap_in_ac3_cwmax_value=0;

cap_in_ac0_txnav_value=0;
cap_in_ac1_txnav_value=0; 
cap_in_ac2_txnav_value=0;
cap_in_ac3_txnav_value=0;

cap_in_slot_timer_value=0;
cap_in_cts_timer_value=0;
cap_in_ack_timer_value=0;
cap_in_rxhandler_ret_value=0;
cap_in_sifs_timer_value=0;
cap_in_phy_rxstart_delay=0;
cap_in_mac_addr=0; 
cap_in_short_retry_count = 8'h0;
cap_in_long_retry_count = 8'h0;

cap_in_ac0_txframeinfo_valid=0;
cap_in_ac1_txframeinfo_valid=0;
cap_in_ac2_txframeinfo_valid=0;
cap_in_ac3_txframeinfo_valid=0;

  #10;
  rst_n=0;
  #10; 
  rst_n=1;

end

//--------------20 MHZ CLK ------////40=12.5 
//--------------100MHZ CLK ------////10ns 
always #2.5 clk = ~clk;
//always #25 cap_in_clk = ~cap_in_clk;   // 80 Mhz clock generation 

//initial
//  begin
   // $shm_open(`DUMPFILE);
   // $shm_probe("ASM");
//end

endmodule
