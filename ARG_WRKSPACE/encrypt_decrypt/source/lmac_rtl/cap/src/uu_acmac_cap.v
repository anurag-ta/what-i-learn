// This module implements the functionality of CAP in LMAC.
// NOTE: The "Pragma" are used for counters where only all possible values of the counter are not tested.

`include "../../inc/defines.h"

module uu_acmac_cap 
                #(
                  parameter WIDTH_WORD = 32,
                  parameter WIDTH_HALF_WORD = 16,
                  parameter WIDTH_BYTE = 8 
                 )
                 (
                  input                                     cap_in_clk,
                  input                                     rst_n,
                  input                                     cap_in_enable,

                  // Trigger Events inputs to the CAP Module
                 // input                                     cap_in_ev_mac_reset,
                  input                                     cap_in_ev_txready,
                  input                                     cap_in_ev_timer_tick,
                  input                                     cap_in_ev_txstart_confirm,
                  input                                     cap_in_ev_txdata_confirm,
                  input                                     cap_in_ev_txend_confirm,
                  input                                     cap_in_ev_phyenergy_ind,
                  input                                     cap_in_ev_rxstart_ind,
                  input                                     cap_in_ev_rxdata_ind,
                  input                                     cap_in_ev_rxend_ind,
                  input                                     cap_in_ev_phyrx_error,
                  input                                     cap_in_ev_ccareset_confirm,

                  // Frame Related Inputs from the Memory Registers
                  input                                     cap_in_frame_valid,          // Valid signal for frame from Memory Registers
                  input [WIDTH_BYTE-1:0]                    cap_in_frame,                // Frame Byte from the Memory Registers
                  
                  // Inputs used during IDLE State
                  input                                     cap_in_qos_mode,             // Qos Mode Enable Signal
                  input [1:0]                               cap_in_cca_status,           // CCA Status from PHY only 1 bit is used currently
                  input [2:0]                               cap_in_channel_list,         // Input Channel list
                  input [2:0]                               cap_in_txvec_chbw,           // Tx Vector Channel Bandwidth 

                  // Fallback Values
                  input [1:0]                               cap_in_txvec_format,         // TX Vector Format
                  input [WIDTH_BYTE-1:0]                    cap_in_aggr_count,           // Aggregation Count input from Frame Info   
 
//pragma coverage toggle = off
                  // Default Values from the Registers
                  input [3:0]                               cap_in_difs_value,           // DIFS for non Qos from the Registers
                  input [3:0]                               cap_in_ac0_aifs_value,       // AIFS for AC0 from the Registers
                  input [3:0]                               cap_in_ac1_aifs_value,       // AIFS for AC1 from the Registers
                  input [3:0]                               cap_in_ac2_aifs_value,       // AIFS for AC2 from the Registers
                  input [3:0]                               cap_in_ac3_aifs_value,       // AIFS for AC3 from the Registers
                  input [3:0]                               cap_in_slot_timer_value,     // Slot Timer Input from the Regsiters 
                  input [4:0]                               cap_in_cts_timer_value,      // CTS Timer Input from the Registers
                  input [4:0]                               cap_in_ack_timer_value,      // ACK Timer Input from the Registers
                  input [5:0]                               cap_in_eifs_value,           // EIFS Value from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_cwmin_value,          // CW Minimum Value from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac0_cwmin_value,      // CW Minimum for AC0 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac1_cwmin_value,      // CW Minimum for AC1 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac2_cwmin_value,      // CW Minimum for AC2 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac3_cwmin_value,      // CW Minimum for AC3 from the Registers
	          input [WIDTH_HALF_WORD-1:0]               cap_in_cwmax_value,          // CW Maximum for non Qos from the Regsiters
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac0_cwmax_value,      // CW Maximum for AC0 from the Registers 
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac1_cwmax_value,      // CW Maximum for AC1 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac2_cwmax_value,      // CW Maximum for AC2 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac3_cwmax_value,      // CW Maximum for AC3 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac0_txnav_value,      // TX NAV Value for AC0 from the Regsiters
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac1_txnav_value,      // TX NAV Value for AC1 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac2_txnav_value,      // TX NAV Value for AC2 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_ac3_txnav_value,      // TX NAV Value for AC3 from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_sifs_timer_value,     // SIFS Timer Value from the Registers
                  input [WIDTH_HALF_WORD-1:0]               cap_in_phy_rxstart_delay,    // Rx Start Delay Value from the Registers
                  input [15:0]                              cap_in_rts_threshold_len,    // RTS Threshold Length used in the Control
                  input [19:0]                              cap_in_frame_length,         // Frame Length used in the frame
                  input [WIDTH_WORD-1:0]                    cap_in_rxhandler_ret_value,  // Rx Handler Return Value
                  input [WIDTH_WORD-1:0]                    cap_in_txhandler_ret_value,  // Tx handler Return Value
                  input                                     cap_in_txhandler_ret_valid,  // Tx Handler Return Value Valid Signal
                  input [(WIDTH_WORD+WIDTH_HALF_WORD-1):0]  cap_in_mac_addr,             // MAC Address of the STA.
		      input [1:0]                               cap_in_tx_ack_policy, //added for multicast
                  input [7:0]                               cap_in_tx_mpdu_fc0 ,//  added for multicast  
			input [7:0]                               cap_in_tx_mpdu_fc1 ,//  added for multicast  
                  input [47:0]                              cap_in_tx_ra_addr,        
//pragma coverage toggle = on
                 
                  input [WIDTH_BYTE-1:0]                    cap_in_short_retry_count,    // Short Retry Count Input
                  input [WIDTH_BYTE-1:0]                    cap_in_long_retry_count,    // Long Retry Count Input

                  // Frame Valid Signal used during Transmit State
                  input                                     cap_in_ac0_txframeinfo_valid, // Frame Valid for AC0 (frame_info)
                  input                                     cap_in_ac1_txframeinfo_valid, // Frame Valid for AC1
                  input                                     cap_in_ac2_txframeinfo_valid, // Frame Valid for AC2
                  input                                     cap_in_ac3_txframeinfo_valid, // Frame Valid for AC3

                  // Trigger Events From the CAP Module the Tx, RX module.
                  output reg                                cap_out_ev_ccareset_req,        // CCA Reset Request
                  output reg                                cap_out_ev_txdata_req,          // Tx Data Request
                  output reg                                cap_out_ev_txend_req,           // Tx End Request
                  output reg                                cap_out_ev_rxend_ind,           // Rx End Indication
                  output reg                                cap_out_ev_txop_txstart_req,    // TXOP Tx Start Request to Tx
                  output reg                                cap_out_ev_txsendmore_data_req, // Send More Data to Tx

                  // Outputs for the IDLE State 
                  output reg [2:0]                          cap_out_channel_bw,       // Registered Channel BW to Tx

                  // Outputs for CONTEND State
                  output reg                                cap_out_frame_info_retry, // Retry Output for Tx Handler
                  output reg                                cap_out_mpdu_discard,     // MPDU Discard status
                  output reg                                cap_out_mpdu_status,      // Status Output from the CAP for MPDU
                  output reg [2:0]                          cap_out_txop_owner_ac,    // AC which Won the TXOP to Tx
 
//pragma coverage toggle = off
                  output reg [WIDTH_BYTE-1:0]               cap_out_ac0_src,           // Short Retry Count for AC0
                  output reg [WIDTH_BYTE-1:0]               cap_out_ac1_src,           // Short Retry Count for AC1
                  output reg [WIDTH_BYTE-1:0]               cap_out_ac2_src,           // Short Retry Count for AC2
                  output reg [WIDTH_BYTE-1:0]               cap_out_ac3_src,           // Short Retry Count for AC3
                  output reg [WIDTH_BYTE-1:0]               cap_out_ac0_lrc,           // Long Retry Count for AC0
                  output reg [WIDTH_BYTE-1:0]               cap_out_ac1_lrc,           // Long Retry Count for AC1
                  output reg [WIDTH_BYTE-1:0]               cap_out_ac2_lrc,           // Long Retry Count for AC2
                  output reg [WIDTH_BYTE-1:0]               cap_out_ac3_lrc,           // Long Retry Count for AC3
                  output reg [WIDTH_HALF_WORD-1:0]          cap_out_ac0_txnav_value,   // Tx NAV for AC0 to Tx
                  output reg [WIDTH_HALF_WORD-1:0]          cap_out_ac1_txnav_value,   // Tx NAV for AC1 to Tx
                  output reg [WIDTH_HALF_WORD-1:0]          cap_out_ac2_txnav_value,   // Tx NAV for AC2 to Tx
                  output reg [WIDTH_HALF_WORD-1:0]          cap_out_ac3_txnav_value,   // Tx NAV for AC3 to Tx
//pragma coverage toggle = on
                 
                  // Outputs for TXOP States
                  output reg                                cap_out_cp_wait_for_ack,   // Set/Reset ACK Wait Signal to CP
                  output reg                                cap_out_cp_wait_for_cts,   // Set/Reset CTS Wait Signal to CP
                  output reg                                cap_out_fb_update,         // Pulse to Update FB values to Tx
                  output reg                                cap_out_clear_ampdu,        // Pulse to indicate Clearing of AMPDU   
                  output reg				    cap_out_ctrl_start_req,    // Control Response vector gen signal
		  output reg				    cap_out_ctrl_data_req ,
		  output cap_out_ac0_int_coll_r,//vibha
		  output cap_out_ac1_int_coll_r,//vibha
		  output cap_out_ac2_int_coll_r,//vibha
		  output cap_out_ac3_int_coll_r//vibha
                 );


//**************************************************************************//
//-------------------------- Local Parameters ------------------------------//
//**************************************************************************//
    localparam       
                     IDLE_STATE               =  7'b0000000,
                     EIFS_STATE               =  7'b0000001,
                     CONTEND_STATE            =  7'b0000010,
                     TXOP_STATE               =  7'b0000100,
                     RECEIVE_STATE            =  7'b0001000,   
                     TXOP_SEND_STATE          =  7'b0010000,
                     TXOP_WAIT_RX_STATE       =  7'b0100000,
                     TXOP_WAIT_TXNAVEND_STATE =  7'b1000000;

    localparam       SM_STATE_CONTINUE        = 8'hF1,
                     SM_TX_END                = 8'hF2,
                     SM_TXOP_LIMIT_COMP       = 8'hF3,
                     SM_START_SENDING         = 8'hF4,
                     SM_WAIT_FOR_RX           = 8'hF5,
                     SM_TX_FAILURE            = 8'hF6,
                     SM_CONTEND               = 8'hF7,
                     SM_TX_NAV_ENDED          = 8'hF8,
                     SM_TXOP_NAV_UPDATE       = 8'hF9,
                     SM_TXOP_ERROR_RECOVERY   = 8'hFA,
                     SM_RX_VALID_FRAME        = 8'hFB;

//**************************************************************************//
//-------------------------- Register Declaration --------------------------//
//**************************************************************************//
    reg                                      ac0_int_coll_r;            // AC0 Collision Register
    reg                                      ac1_int_coll_r;            // AC1 Collision Register
    reg                                      ac2_int_coll_r;            // AC2 Collision Register
    reg                                      ac3_int_coll_r;            // AC3 Collision Register
    reg                                      cap_ra_no_match_r;         // RA Address not matched
    reg                                      cap_txstart_confirm_r;     // Toggled to Sample Tx Vector
    reg                                      cap_rxstart_ind_r;         // Toggled to Sample Rx Vector
    reg                                      cap_rxdata_ind_r;          // Toggled to Sample Rx Data
    reg                                      cap_txdata_confirm_r;      // Toggled to Sample Tx Data
    reg                                      cap_txop_start_r;          // Signal to generate single pulse of Txop Txstart
    reg [2:0]                                cap_update_channel_list_r; // Channel Update Register
    reg [2:0]                                cap_prev_channel_list_r;   // Previous Channel Register
//pragma coverage toggle = off
    reg [4:0]                                cap_frame_vector_count_r;  // Counter for Vector bytes
    reg [5:0]                                cap_frame_read_count_r;    // Counter for Frame bytes
    reg [WIDTH_BYTE:0]                       cap_frame_fc_r;            // Frame Control bytes 
    reg [WIDTH_HALF_WORD-1:0]                cap_frame_duration_r;      // Frame Duration bytes
    reg [(WIDTH_WORD+WIDTH_HALF_WORD)-1:0]   cap_ra_addr_r;             // Registers the Receiver Address  
//pragma coverage toggle = on
    reg [6:0]                                curr_state;                // Register for Current State
    reg [6:0]                                next_state;                // Register for Next State  
    reg [1:0]                                cap_frame_ack_policy_r;    // Registers the ACK Policy
    reg [3:0]                                cap_ldatarate_r;           // Register for L_DataRate
    reg                                      cap_pkt_error_r;           // Register the Packet Error
    reg                                      cap_qos_mode_r;            // Register the QoS Mode

    reg [WIDTH_BYTE-1:0]                     cap_ac0_ssrc_r;            // SSRC for AC0    
    reg [WIDTH_BYTE-1:0]                     cap_ac1_ssrc_r;            // SSRC for AC1
    reg [WIDTH_BYTE-1:0]                     cap_ac2_ssrc_r;            // SSRC for AC2
    reg [WIDTH_BYTE-1:0]                     cap_ac3_ssrc_r;            // SSRC for AC3
    reg [WIDTH_BYTE-1:0]                     cap_ac0_slrc_r;            // SLRC for AC0
    reg [WIDTH_BYTE-1:0]                     cap_ac1_slrc_r;            // SLRC for AC1
    reg [WIDTH_BYTE-1:0]                     cap_ac2_slrc_r;            // SLRC for AC2
    reg [WIDTH_BYTE-1:0]                     cap_ac3_slrc_r;            // SLRC for AC3

    reg                                      cap_ac0_txready_r;         // TX Ready flag for AC0
    reg                                      cap_ac1_txready_r;         // TX Ready flag for AC1
    reg                                      cap_ac2_txready_r;         // TX Ready flag for AC2
    reg                                      cap_ac3_txready_r;         // TX Ready flag for AC3

    reg [WIDTH_HALF_WORD-1:0]                cap_ac0_backoff_r;         // Backoff Value for AC0
    reg [WIDTH_HALF_WORD-1:0]                cap_ac1_backoff_r;         // Backoff Value for AC1
    reg [WIDTH_HALF_WORD-1:0]                cap_ac2_backoff_r;         // Backoff Value for AC2
    reg [WIDTH_HALF_WORD-1:0]                cap_ac3_backoff_r;         // Backoff Value for AC3

    reg                                      cap_ac0_txprog_r;          // TX Program Flag for AC0
    reg                                      cap_ac1_txprog_r;          // TX Program Flag for AC1
    reg                                      cap_ac2_txprog_r;          // TX Program Flag for AC2
    reg                                      cap_ac3_txprog_r;          // TX Program Flag for AC3
    reg [WIDTH_BYTE-1:0]                     cap_txop_return_r;         // TXOP Return Value used in FSM
    reg [3:0]                                cap_slot_timer_r;          // Output SLOT Timer Value
    reg [3:0]                                cap_ac0_aifs_r;            // Output AIFS for AC0
    reg [3:0]                                cap_ac1_aifs_r;            // Output AIFS for AC1
    reg [3:0]                                cap_ac2_aifs_r;            // Output AIFS for AC2
    reg [3:0]                                cap_ac3_aifs_r;            // Output AIFS for AC3
    reg [4:0]                                cap_cts_timer_r;           // Output CTS Timer Value
    reg [4:0]                                cap_ack_timer_r;           // Output ACK Timer Value
    reg [5:0]                                cap_eifs_r;                // Output EIFS Timer_value

    reg [1:0]                                cap_cca_status_r;          // Registered CCA Status  
    reg [WIDTH_HALF_WORD-1:0]                cap_nav_val_r;             // Registered NAV Value
    reg [WIDTH_HALF_WORD-1:0]                cap_nav_reset_timer_r;     // Registered NAV Reset Timer Value 

    reg [WIDTH_HALF_WORD-1:0]                cap_ac0_cw_r;              // Registered CW Value for AC0   
    reg [WIDTH_HALF_WORD-1:0]                cap_ac1_cw_r;              // Registered CW Value for AC1
    reg [WIDTH_HALF_WORD-1:0]                cap_ac2_cw_r;              // Registered CW Value for AC2
    reg [WIDTH_HALF_WORD-1:0]                cap_ac3_cw_r;              // Registered CW Value for AC3
    reg                                      ctrl_start_count;

//**************************************************************************//
//-------------------------- Continuous Assignments ------------------------//
//**************************************************************************//

    wire [1:0]                               cap_ac_priority;
    wire [2:0]                               cap_channel_busy;
	 
	 
	 // signals taken as output for controlling the updation of base pointers for the 4 ACs in tx buff code
	 assign cap_out_ac0_int_coll_r=ac0_int_coll_r;
	 assign cap_out_ac1_int_coll_r=ac1_int_coll_r;
	 assign cap_out_ac2_int_coll_r=ac2_int_coll_r;
	 assign cap_out_ac3_int_coll_r=ac3_int_coll_r;
  
    // State Continuous Assignments 
    assign  idle          = ( curr_state == IDLE_STATE ) ? 1'b1 : 1'b0;
    assign  receive       = ( curr_state == RECEIVE_STATE ) ? 1'b1 : 1'b0;
    assign  contend       = ( curr_state == CONTEND_STATE ) ? 1'b1 : 1'b0;
    assign  txop          = ( curr_state == TXOP_STATE ) ? 1'b1 : 1'b0;
    assign  txop_send     = ( curr_state == TXOP_SEND_STATE ) ? 1'b1 : 1'b0;
    assign  txop_rx       = ( curr_state == TXOP_WAIT_RX_STATE ) ? 1'b1 : 1'b0;
    assign  txop_txnavend = ( curr_state == TXOP_WAIT_TXNAVEND_STATE ) ? 1'b1 : 1'b0;
    assign  eifs          = ( curr_state == EIFS_STATE ) ? 1'b1 : 1'b0;

    // Assigned when Length is less than RTS Threshold
    assign  len_lt_rts = ( cap_in_rts_threshold_len > cap_in_frame_length );

    // Assigned when Channel Busy
    assign  cap_channel_busy = ( cap_prev_channel_list_r <= cap_update_channel_list_r ) ?
                                 cap_prev_channel_list_r : cap_update_channel_list_r;
    
    // Assigned when any of Frame Info Valid is asserted for any of the ACs
    assign  cap_txframeinfo_valid = ( cap_in_ac0_txframeinfo_valid || 
                                      cap_in_ac1_txframeinfo_valid ||
                                      cap_in_ac2_txframeinfo_valid ||
                                      cap_in_ac3_txframeinfo_valid);

    // Assigned when Frame Ready Flag is set for any of the ACs
     assign  cap_txready = ( cap_ac0_txready_r || cap_ac1_txready_r ||
                             cap_ac2_txready_r || cap_ac3_txready_r ); 

    // Assigned when Tx Program Flag is set for any of the ACs
    assign  cap_txprog = ( cap_ac0_txprog_r || cap_ac1_txprog_r ||
                           cap_ac2_txprog_r || cap_ac3_txprog_r );
 
    // Assigned when Short retry count is greater than Valid Short Retry Count value for AC0
    assign  src_ac0_exceeded = ( cap_out_ac0_src >= cap_in_short_retry_count );

    // Assigned when Short retry count is greater than Valid Short Retry Count value for AC1
    assign  src_ac1_exceeded = ( cap_out_ac1_src >= cap_in_short_retry_count );
    
    // Assigned when Short retry count is greater than Valid Short Retry Count value for AC2
    assign  src_ac2_exceeded = ( cap_out_ac2_src >= cap_in_short_retry_count );
        
    // Assigned when Short retry count is greater than Valid Short Retry Count value for AC3
    assign  src_ac3_exceeded = ( cap_out_ac3_src >= cap_in_short_retry_count );
    
    // Assigned when Long retry count is greater than Valid Long Retry Count Value for AC0
    assign  lrc_ac0_exceeded = ( cap_out_ac0_lrc >= cap_in_long_retry_count );

    // Assigned when Long retry count is greater than Valid Long Retry Count Value for AC1
    assign  lrc_ac1_exceeded = ( cap_out_ac1_lrc >= cap_in_long_retry_count );

    // Assigned when Long retry count is greater than Valid Long Retry Count Value for AC2
    assign  lrc_ac2_exceeded = ( cap_out_ac2_lrc >= cap_in_long_retry_count );

    // Assigned when Long retry count is greater than Valid Long Retry Count Value for AC3
    assign  lrc_ac3_exceeded = ( cap_out_ac3_lrc >= cap_in_long_retry_count );

    // Assigned to indicate PS POLL Frame
    // assign  pspoll_frame = (cap_frame_fc_r == 8'hA4) ? 1'b1 : 1'b0;
 
    // Assigned to indicate RTS Frame
    assign  rts_frame = (cap_frame_fc_r == 8'hB4) ? 1'b1 : 1'b0;

    // Assigned to indicate CTS Frame
    assign  cts_frame = (cap_frame_fc_r == 8'hC4) ? 1'b1 : 1'b0;

    // Assigned to indicate the Priority of the AC Selected
    assign cap_ac_priority = ( cap_in_ac3_txframeinfo_valid ) ? 2'b11 :
                             ( cap_in_ac2_txframeinfo_valid ) ? 2'b10 :
                             ( cap_in_ac1_txframeinfo_valid ) ? 2'b01 : 2'b00;

//**************************************************************************//
//-------------------------- Procedural Assignments ------------------------//
//**************************************************************************//

// Always Block for the current state
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            curr_state <= IDLE_STATE;
        else if( !cap_in_enable )
            curr_state <= IDLE_STATE;
        else
            curr_state <= next_state;
    end

// Always Block for Output CCA Status
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_cca_status_r <= 2'h0;
        else if( !cap_in_enable )
            cap_cca_status_r <= 2'h0;
        else if( cap_in_enable && cap_in_ev_phyenergy_ind )
            cap_cca_status_r <= cap_in_cca_status;
    end

// Always Block for generating single pulse of TXOP Tx Start
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_txop_start_r <= 1'b0;
        else if( !cap_in_enable ||  cap_in_ev_ccareset_confirm|| !txop) // TODO
            cap_txop_start_r <= 1'b0; 
        else if( cap_out_ev_txop_txstart_req )
            cap_txop_start_r <= 1'b1; 
    end

// Always Block for EIFS Timer Value
// Note: The EIFS Input value is stored unlike the 0 value in C Code.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
       if( `POLARITY_OF_RESET )
            cap_eifs_r <= 6'h0;
        else if( !cap_in_enable )
            cap_eifs_r <= 6'h0;
        else if(( eifs || txop_txnavend ) && cap_in_ev_timer_tick && 
               ( cap_eifs_r > 6'h0 ))
            cap_eifs_r <=  cap_eifs_r - 6'h1;
         else if( txop_rx && (( cap_in_ev_rxend_ind &&
                ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_FRAME_ERROR )) ||
                ( cap_in_ev_phyrx_error ))) 
            cap_eifs_r <= cap_in_eifs_value;
        else if( idle || receive || contend ||
                 txop || txop_send || txop_rx )
            cap_eifs_r <= cap_in_eifs_value;
    end
     
// Always Block for Selecting AC to Win TXOP
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_txop_owner_ac <= 3'b111;
        else if( !cap_in_enable )
            cap_out_txop_owner_ac <= 3'b111;
        else if( contend && cap_in_ev_timer_tick && 
              (( !cap_ac3_txready_r && !cap_in_ac3_txframeinfo_valid && 
               ( cap_ac3_aifs_r == cap_in_ac3_aifs_value ) &&
               ( cap_ac3_backoff_r == cap_ac3_cw_r ) &&
               ( cap_out_txop_owner_ac == 3'b011 )) ||
               ( !cap_ac2_txready_r && !cap_in_ac2_txframeinfo_valid && 
               ( cap_ac2_aifs_r == cap_in_ac2_aifs_value ) &&
               ( cap_ac2_backoff_r == cap_ac2_cw_r ) &&
               ( cap_out_txop_owner_ac == 3'b010 )) ||
               ( !cap_ac1_txready_r && !cap_in_ac1_txframeinfo_valid && 
               ( cap_ac1_aifs_r == cap_in_ac1_aifs_value ) &&
               ( cap_ac1_backoff_r == cap_ac1_cw_r ) &&
               ( cap_out_txop_owner_ac == 3'b001 )) ||
               ( !cap_ac0_txready_r && !cap_in_ac0_txframeinfo_valid && 
               ( cap_ac0_aifs_r == cap_in_ac0_aifs_value ) &&
               ( cap_ac0_backoff_r == cap_ac0_cw_r ) &&
               ( cap_out_txop_owner_ac == 3'b0 ))))
            cap_out_txop_owner_ac <= 3'b111;
        else if( contend && cap_ac3_txready_r && cap_qos_mode_r && 
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac3_aifs_r == 4'h0 ) && 
               ( cap_ac3_backoff_r <= 16'h1 ) &&  
               ( cap_out_txop_owner_ac == 3'b111 ))
            cap_out_txop_owner_ac <= 3'b011;
        else if( contend &&  cap_ac2_txready_r && cap_qos_mode_r &&   
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac2_aifs_r == 4'h0 ) && 
               ( cap_ac2_backoff_r <= 16'h1 ) &&                      
               ( cap_out_txop_owner_ac == 3'b111 )) 
            cap_out_txop_owner_ac <= 3'b010;
        else if( contend && cap_ac1_txready_r && cap_qos_mode_r && 
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac1_aifs_r == 4'h0 ) && 
               ( cap_ac1_backoff_r <= 16'h1 ) &&
               ( cap_out_txop_owner_ac == 3'b111 ))
             cap_out_txop_owner_ac <= 3'b001;
        else if( contend && cap_ac0_txready_r && 
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac0_aifs_r == 4'h0 ) && 
               ( cap_ac0_backoff_r <= 16'h1 ) && 
               ( cap_out_txop_owner_ac == 3'b111 ))
            cap_out_txop_owner_ac <= 3'b0;   
    end    

// Always Block for TX READY for AC0
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac0_txready_r <= 1'b0;
        else if( !cap_in_enable )
            cap_ac0_txready_r <= 1'b0;
        else if( txop || txop_send ) 
            cap_ac0_txready_r <= cap_in_ac0_txframeinfo_valid;
        else if( contend && cap_in_ev_timer_tick &&
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac0_aifs_r == 4'h0 ) &&
               ( cap_ac0_backoff_r == 16'h1))
            cap_ac0_txready_r <= cap_in_ac0_txframeinfo_valid;
        else if( txop_rx && ( cap_in_ev_timer_tick ||
                 cap_in_ev_txend_confirm || cap_in_ev_rxend_ind ))
            cap_ac0_txready_r <= cap_in_ac0_txframeinfo_valid;
        else if( txop_txnavend && ( cap_txop_return_r == SM_TX_NAV_ENDED )) 
            cap_ac0_txready_r <= cap_in_ac0_txframeinfo_valid;
        else if( cap_in_ev_ccareset_confirm )
            cap_ac0_txready_r <= cap_in_ac0_txframeinfo_valid;
        else if(( idle || contend || receive || eifs ) && cap_in_ev_txready )
            cap_ac0_txready_r <= cap_in_ac0_txframeinfo_valid;
    end

// Always Block for TX READY for AC1
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac1_txready_r <= 1'b0;
        else if( !cap_in_enable )
            cap_ac1_txready_r <= 1'b0;
        else if( txop || txop_send ) 
            cap_ac1_txready_r <= cap_in_ac1_txframeinfo_valid;
        else if( contend && cap_in_ev_timer_tick && 
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac1_aifs_r == cap_in_ac1_aifs_value ) &&
               ( cap_ac1_backoff_r == cap_ac1_cw_r ))
            cap_ac1_txready_r <= cap_in_ac1_txframeinfo_valid;
        else if( txop_rx && ( cap_in_ev_timer_tick ||
                 cap_in_ev_txend_confirm || cap_in_ev_rxend_ind ))
            cap_ac1_txready_r <= cap_in_ac1_txframeinfo_valid;
        else if( txop_txnavend && ( cap_txop_return_r == SM_TX_NAV_ENDED )) 
            cap_ac1_txready_r <= cap_in_ac1_txframeinfo_valid;
        else if( cap_in_ev_ccareset_confirm )
            cap_ac1_txready_r <= cap_in_ac1_txframeinfo_valid;
        else if(( idle || contend || receive || eifs ) && cap_in_ev_txready )
            cap_ac1_txready_r <= cap_in_ac1_txframeinfo_valid;
    end

// Always Block for TX READY for AC2
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac2_txready_r <= 1'b0;
        else if( !cap_in_enable )
            cap_ac2_txready_r <= 1'b0;
        else if( txop || txop_send ) 
            cap_ac2_txready_r <= cap_in_ac2_txframeinfo_valid;
        else if( contend && cap_in_ev_timer_tick &&
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac2_aifs_r == cap_in_ac2_aifs_value ) && 
               ( cap_ac2_backoff_r == cap_ac2_cw_r ))
            cap_ac2_txready_r <= cap_in_ac2_txframeinfo_valid;
        else if( txop_rx && ( cap_in_ev_timer_tick ||
                 cap_in_ev_txend_confirm || cap_in_ev_rxend_ind ))
            cap_ac2_txready_r <= cap_in_ac2_txframeinfo_valid;
        else if( txop_txnavend && ( cap_txop_return_r == SM_TX_NAV_ENDED )) 
            cap_ac2_txready_r <= cap_in_ac2_txframeinfo_valid;
        else if( cap_in_ev_ccareset_confirm )
            cap_ac2_txready_r <= cap_in_ac2_txframeinfo_valid;
        else if(( idle || contend || receive || eifs ) && cap_in_ev_txready )
            cap_ac2_txready_r <= cap_in_ac2_txframeinfo_valid;
    end

// Always Block for TX READY for AC3
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac3_txready_r <= 1'b0;
        else if( !cap_in_enable )
            cap_ac3_txready_r <= 1'b0;
        else if( txop || txop_send )
            cap_ac3_txready_r <= cap_in_ac3_txframeinfo_valid;
        else if( contend && cap_in_ev_timer_tick &&
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac3_aifs_r == cap_in_ac3_aifs_value ) &&
               ( cap_ac3_backoff_r == cap_ac3_cw_r ))
            cap_ac3_txready_r <= cap_in_ac3_txframeinfo_valid;
        else if( txop_rx && ( cap_in_ev_timer_tick ||
                 cap_in_ev_txend_confirm || cap_in_ev_rxend_ind ))
            cap_ac3_txready_r <= cap_in_ac3_txframeinfo_valid;
        else if( txop_txnavend && ( cap_txop_return_r == SM_TX_NAV_ENDED )) 
            cap_ac3_txready_r <= cap_in_ac3_txframeinfo_valid;
        else if( cap_in_ev_ccareset_confirm )
            cap_ac3_txready_r <= cap_in_ac3_txframeinfo_valid;
        else if(( idle || contend || receive || eifs ) && cap_in_ev_txready )
            cap_ac3_txready_r <= cap_in_ac3_txframeinfo_valid;
    end

// Always Block for AC0 AIFS Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac0_aifs_r <= 4'h0;
        else if( !cap_in_enable )
            cap_ac0_aifs_r <= 4'h0;
        else if( !contend )
        begin
           if( cap_qos_mode_r )
              cap_ac0_aifs_r <= cap_in_ac0_aifs_value;
           else
              cap_ac0_aifs_r <= cap_in_difs_value;
        end
        else if( contend && cap_ac0_txready_r && ( cap_slot_timer_r == 4'h1 ) &&
                 cap_in_ev_timer_tick && ( cap_ac0_aifs_r > 4'h0 ))
            cap_ac0_aifs_r <= cap_ac0_aifs_r - 4'h1;
    end

// Always Block for AC1 AIFS Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac1_aifs_r <= 4'h0;
        else if( !cap_in_enable )
            cap_ac1_aifs_r <= 4'h0;
        else if( !contend && cap_qos_mode_r )
            cap_ac1_aifs_r <= cap_in_ac1_aifs_value;
        else if( contend && cap_ac1_txready_r && ( cap_slot_timer_r == 4'h1 ) &&
                 cap_in_ev_timer_tick && ( cap_ac1_aifs_r > 4'h0 ))
            cap_ac1_aifs_r <= cap_ac1_aifs_r - 4'h1;
    end

// Always Block for AC2 AIFS Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac2_aifs_r <= 4'h0;
        else if( !cap_in_enable )
            cap_ac2_aifs_r <= 4'h0;
        else if( !contend && cap_qos_mode_r )
            cap_ac2_aifs_r <= cap_in_ac2_aifs_value;
        else if( contend && cap_ac2_txready_r && ( cap_slot_timer_r == 4'h1 ) &&
                 cap_in_ev_timer_tick && ( cap_ac2_aifs_r > 4'h0 ))
            cap_ac2_aifs_r <= cap_ac2_aifs_r - 4'h1;
    end

// Always Block for AC3 AIFS Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac3_aifs_r <= 4'h0;
        else if( !cap_in_enable )
            cap_ac3_aifs_r <= 4'h0;
        else if( !contend && cap_qos_mode_r )
            cap_ac3_aifs_r <= cap_in_ac3_aifs_value;
        else if( contend && cap_ac3_txready_r && ( cap_slot_timer_r == 4'h1 ) &&
                 cap_in_ev_timer_tick && ( cap_ac3_aifs_r > 4'h0 ))
            cap_ac3_aifs_r <= cap_ac3_aifs_r - 4'h1;
    end

// Always Block for Backoff0 Counter Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac0_backoff_r <= 16'h0;
        else if( !cap_in_enable )
            cap_ac0_backoff_r <= 16'h0;
        else if( !contend )
            cap_ac0_backoff_r <= cap_ac0_cw_r;
        else if( contend && ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac0_aifs_r == 4'h0 ) && 
               ( cap_ac0_backoff_r == 16'h0 ) &&
               ( cap_ac0_cw_r > 16'h0 ))
            cap_ac0_backoff_r <= cap_ac0_cw_r;
        else if( contend && cap_in_ev_timer_tick && cap_ac0_txready_r &&
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac0_aifs_r == 4'h0 ) && 
               ( cap_ac0_backoff_r > 16'h0 ))
            cap_ac0_backoff_r <= cap_ac0_backoff_r - 16'h1;
    end

// Always Block for Backoff1 Counter Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac1_backoff_r <= 16'h0;
        else if( !cap_in_enable )
            cap_ac1_backoff_r <= 16'h0;
        else if( !contend && cap_qos_mode_r )
            cap_ac1_backoff_r <= cap_ac1_cw_r;
        else if( contend && ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac1_aifs_r == 4'h0 ) && 
               ( cap_ac1_backoff_r == 16'h0 ) &&
               ( cap_ac1_cw_r > 16'h0 ))
            cap_ac1_backoff_r <= cap_ac1_cw_r;
        else if( contend && cap_in_ev_timer_tick && cap_ac1_txready_r &&
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac1_aifs_r == 4'h0 ) && 
               ( cap_ac1_backoff_r > 16'h0 ))
            cap_ac1_backoff_r <= cap_ac1_backoff_r - 16'h1;
    end

// Always Block for Backoff2 Counter Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac2_backoff_r <= 16'h0;
        else if( !cap_in_enable )
            cap_ac2_backoff_r <= 16'h0;
        else if( !contend && cap_qos_mode_r )
            cap_ac2_backoff_r <= cap_ac2_cw_r;
        else if( contend && ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac2_aifs_r == 4'h0 ) && 
               ( cap_ac2_backoff_r == 16'h0 ) &&
               ( cap_ac2_cw_r > 16'h0 ))
            cap_ac2_backoff_r <= cap_ac2_cw_r;
        else if( contend && cap_in_ev_timer_tick && cap_ac2_txready_r && 
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac2_aifs_r == 4'h0 ) && 
               ( cap_ac2_backoff_r > 16'h0 ))
            cap_ac2_backoff_r <= cap_ac2_backoff_r - 16'h1;
    end

// Always Block for Backoff3 Counter Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac3_backoff_r <= 16'h0;
        else if( !cap_in_enable )
            cap_ac3_backoff_r <= 16'h0;
        else if( !contend && cap_qos_mode_r )
            cap_ac3_backoff_r <= cap_ac3_cw_r;
        else if( contend && ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac3_aifs_r == 4'h0 ) && 
               ( cap_ac3_backoff_r == 16'h0 ) &&
               ( cap_ac3_cw_r > 16'h0 ))
                cap_ac3_backoff_r <= cap_ac3_cw_r;
        else if( contend && cap_in_ev_timer_tick && cap_ac3_txready_r &&
               ( cap_slot_timer_r == 4'h1 ) &&
               ( cap_ac3_aifs_r == 4'h0 ) && 
               ( cap_ac3_backoff_r > 16'h0 ))
            cap_ac3_backoff_r <= cap_ac3_backoff_r - 16'h1;
    end

// Always Block for Count Down of Slot Timer.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_slot_timer_r <= 4'h0;
        else if( !cap_in_enable )
            cap_slot_timer_r <= 4'h0;
        else if( !contend )
            cap_slot_timer_r <= cap_in_slot_timer_value;
        else if( contend && ( cap_slot_timer_r == 4'h0 ) && 
               ( cap_in_slot_timer_value > 4'h0 ))
            cap_slot_timer_r <= cap_in_slot_timer_value;
        else if( contend && cap_in_ev_timer_tick && 
               ( cap_slot_timer_r > 4'h0 ))
            cap_slot_timer_r <= cap_slot_timer_r - 4'h1;
    end

// Always Block for AC0 TXNAV Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac0_txnav_value <= 16'h0;
        else if( !cap_in_enable )
            cap_out_ac0_txnav_value <= 16'h0;
        else if( txop && ( cap_out_txop_owner_ac == 3'h0 ))
            cap_out_ac0_txnav_value <= cap_in_ac0_txnav_value;
        else if(( txop_rx || txop_txnavend ) && cap_in_ev_timer_tick && 
               ( cap_out_ac0_txnav_value > 16'h0 ))
            cap_out_ac0_txnav_value <= cap_out_ac0_txnav_value - 16'h1;
    end

// Always Block for AC1 TXNAV Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac1_txnav_value <= 16'h0;
        else if( !cap_in_enable )
            cap_out_ac1_txnav_value <= 16'h0;
        else if( txop && ( cap_out_txop_owner_ac == 3'h1 ))
            cap_out_ac1_txnav_value <= cap_in_ac1_txnav_value;
        else if(( txop_rx || txop_txnavend ) && cap_in_ev_timer_tick &&
               ( cap_out_ac1_txnav_value > 16'h0 ))
            cap_out_ac1_txnav_value <= cap_out_ac1_txnav_value - 16'h1;
    end

// Always Block for AC2 TXNAV Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac2_txnav_value <= 16'h0;
        else if( !cap_in_enable )
            cap_out_ac2_txnav_value <= 16'h0;
        else if( txop && ( cap_out_txop_owner_ac == 3'h2 ))
            cap_out_ac2_txnav_value <= cap_in_ac2_txnav_value;
        else if(( txop_rx || txop_txnavend ) && cap_in_ev_timer_tick && 
               ( cap_out_ac2_txnav_value > 16'h0 ))
            cap_out_ac2_txnav_value <= cap_out_ac2_txnav_value - 16'h1;
    end

// Always Block for AC3 TXNAV Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac3_txnav_value <= 16'h0;
        else if( !cap_in_enable )
            cap_out_ac3_txnav_value <= 16'h0;
        else if( txop && ( cap_out_txop_owner_ac == 3'h3 ))
            cap_out_ac3_txnav_value <= cap_in_ac3_txnav_value;
        else if(( txop_rx || txop_txnavend ) && cap_in_ev_timer_tick && 
               ( cap_out_ac3_txnav_value > 16'h0 ))
            cap_out_ac3_txnav_value <= cap_out_ac3_txnav_value - 16'h1;
    end

// Always Block for CTS Timer Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_cts_timer_r <= 5'h0;
        else if( !cap_in_enable )
            cap_cts_timer_r <= 5'h0;
        else if( txop_rx && cap_txframeinfo_valid && cap_in_ev_rxend_ind &&
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ))
            cap_cts_timer_r <= 5'h0;
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm && !rts_frame )
            cap_cts_timer_r <= 5'h0;
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm && rts_frame &&
               !( cap_ra_addr_r[0] && !cap_frame_fc_r[8] ) && 
               !( cap_qos_mode_r && ( cap_frame_fc_r[7:0] == 8'h88 ) && 
                ( cap_frame_ack_policy_r != 2'h0 )))
            cap_cts_timer_r <= cap_in_cts_timer_value;
        else if( txop_rx && cap_in_ev_timer_tick && !cap_txprog &&
               ( cap_cts_timer_r > 5'h0 ) && cap_txframeinfo_valid )
            cap_cts_timer_r <= cap_cts_timer_r - 5'h1;
    end

// Always Block for ACK Timer Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ack_timer_r <= 5'h0;
        else if( !cap_in_enable )
            cap_ack_timer_r <= 5'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) &&
                   txop_rx && cap_txframeinfo_valid && cap_in_ev_rxend_ind )
            cap_ack_timer_r <= 5'h0;
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm && rts_frame ) 
            cap_ack_timer_r <= 5'h0;
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm && !rts_frame &&
               (!(  ( cap_ra_addr_r[0] && !cap_frame_fc_r[8] ) || 
                                                                 ( cap_qos_mode_r && ( cap_frame_fc_r[7:0] == 8'h88 ) && 
                                                                    ( cap_frame_ack_policy_r != 2'h0 ))  )))
            cap_ack_timer_r <= cap_in_ack_timer_value;
        else if( txop_rx && cap_in_ev_timer_tick && !cap_txprog &&
               ( cap_ack_timer_r > 5'h0 ) && cap_txframeinfo_valid )
            cap_ack_timer_r <= cap_ack_timer_r - 5'h1;
    end

// Always Block to update the NAV Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_nav_val_r <= 16'h0;
        else if( !cap_in_enable )
            cap_nav_val_r <= 16'h0;
        else if( receive && cap_ra_no_match_r && !cts_frame && 
               ( cap_nav_reset_timer_r == 16'h1 ) && cap_in_ev_timer_tick )
            cap_nav_val_r <= 16'h0;
        else if(( receive || eifs || txop_txnavend ) && 
                 cap_in_ev_timer_tick && ( cap_nav_val_r > 16'h0 ))
            cap_nav_val_r <= cap_nav_val_r - 16'h1;
        else if(( receive || txop_rx ) && cap_in_ev_rxend_ind && 
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_NAV_UPDATE ) && 
               ( cap_frame_duration_r > cap_nav_val_r ))
            cap_nav_val_r <= cap_frame_duration_r;
    end

// Always Block to update NAV Reset Timer Register
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_nav_reset_timer_r <= 16'h0;
        else if( !cap_in_enable )
            cap_nav_reset_timer_r <= 16'h0;
        else if( receive && cap_in_ev_rxstart_ind )
            cap_nav_reset_timer_r <= 16'h0;
        else if( txop_rx && cap_in_ev_rxdata_ind && cts_frame && cap_ra_no_match_r )
            cap_nav_reset_timer_r <= 16'h0;
        else if(( receive || txop_rx ) && cap_ra_no_match_r && rts_frame && cap_in_ev_rxend_ind )
        begin
            if( cap_ldatarate_r == `UU_HW_RATE_6M )
                cap_nav_reset_timer_r <= ({ 12'h0, (cap_slot_timer_r << 1) } +
              ( cap_in_sifs_timer_value << 1 ) + cap_in_phy_rxstart_delay + 16'h3 ); 
            else if( cap_ldatarate_r == `UU_HW_RATE_12M )
                cap_nav_reset_timer_r <= ({ 12'h0, (cap_slot_timer_r << 1) } +
              ( cap_in_sifs_timer_value << 1 ) + cap_in_phy_rxstart_delay + 16'h2 ); 
            else
                cap_nav_reset_timer_r <= ({ 12'h0, (cap_slot_timer_r << 1) } +
              ( cap_in_sifs_timer_value << 1 ) + cap_in_phy_rxstart_delay + 16'h1 ); 
        end
        else if( receive && cap_ra_no_match_r && !cts_frame && 
               ( cap_nav_reset_timer_r > 16'h0 ) && cap_in_ev_timer_tick )
            cap_nav_reset_timer_r <= cap_nav_reset_timer_r - 16'h1;  
    end

// Always Block for FB UPDATE
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_fb_update <= 4'h0;
        else if( !cap_in_enable )
            cap_out_fb_update <= 4'h0;
        else if( cap_out_fb_update )
            cap_out_fb_update <= 4'h0;
        else if( txop_rx && cap_in_ev_timer_tick && cap_qos_mode_r && 
               ( cap_in_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT ) && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || ( cap_cts_timer_r == 5'h1 )) && 
             ((( cap_out_ac0_src == `UU_WLAN_SHORT_FB_RETRY_COUNT ) && 
               ( cap_out_txop_owner_ac == 3'h0 ) && !src_ac0_exceeded ) || 
              (( cap_out_ac1_src == `UU_WLAN_SHORT_FB_RETRY_COUNT ) && 
               ( cap_out_txop_owner_ac == 3'h1 ) && !src_ac1_exceeded ) || 
              (( cap_out_ac2_src == `UU_WLAN_SHORT_FB_RETRY_COUNT ) && 
               ( cap_out_txop_owner_ac == 3'h2 ) && !src_ac2_exceeded ) || 
              (( cap_out_ac3_src == `UU_WLAN_SHORT_FB_RETRY_COUNT ) && 
               ( cap_out_txop_owner_ac == 3'h3 ) && !src_ac3_exceeded )))
            cap_out_fb_update <= 4'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts && 
               ( cap_in_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT ) && 
                 cap_qos_mode_r && ( cap_ack_timer_r == 5'h1 ) && 
              (( !lrc_ac0_exceeded && ( cap_out_txop_owner_ac == 3'h0) &&
               ( cap_out_ac0_lrc == ( cap_in_long_retry_count >> 1 ))) ||
               ( !lrc_ac1_exceeded && ( cap_out_txop_owner_ac == 3'h1) &&
               ( cap_out_ac1_lrc == ( cap_in_long_retry_count >> 1 ))) ||
               ( !lrc_ac2_exceeded && ( cap_out_txop_owner_ac == 3'h2) &&
               ( cap_out_ac2_lrc == ( cap_in_long_retry_count >> 1 ))) ||
               ( !lrc_ac3_exceeded && ( cap_out_txop_owner_ac == 3'h3) &&
               ( cap_out_ac3_lrc == ( cap_in_long_retry_count >> 1 )))))
            cap_out_fb_update <= 4'h1;
    end 

// Always Block for Asserting Retry Bit in Frame Info
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_frame_info_retry <= 1'b0;
        else if( !cap_in_enable || cap_out_frame_info_retry || cap_out_mpdu_discard )
            cap_out_frame_info_retry <= 1'b0;
        else if( txop_rx && cap_in_ev_rxend_ind && cap_txframeinfo_valid && 
               ( !src_ac0_exceeded || !src_ac1_exceeded || !src_ac2_exceeded ||
                 !src_ac3_exceeded || !lrc_ac0_exceeded || !lrc_ac1_exceeded ||
                 !lrc_ac1_exceeded || !lrc_ac2_exceeded || !lrc_ac3_exceeded ) &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_frame_info_retry <= 1'b1;
        else if( txop_rx && cap_in_ev_timer_tick && cap_qos_mode_r &&
               ( cap_ack_timer_r == 5'h1 ) && !cap_txprog && cap_txframeinfo_valid )
            cap_out_frame_info_retry <= 1'b1;
    end

// Always Block for CLEAR AMPDU Signal
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_clear_ampdu <= 1'b0;
        else if( !cap_in_enable || cap_out_clear_ampdu )
            cap_out_clear_ampdu <= 1'b0; 
        else if(( src_ac0_exceeded || src_ac1_exceeded || src_ac2_exceeded || 
                  src_ac3_exceeded || lrc_ac0_exceeded || lrc_ac1_exceeded ||
                  lrc_ac2_exceeded || lrc_ac3_exceeded ) && cap_in_aggr_count )
            cap_out_clear_ampdu <= 1'b1;  
    end

// Always Block for generating MPDU Discard Status
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_mpdu_status <= 1'b0;
        else if( !cap_in_enable )
            cap_out_mpdu_status <= 1'b0;
        else if(( src_ac0_exceeded || src_ac1_exceeded || src_ac2_exceeded || 
                  src_ac3_exceeded || lrc_ac0_exceeded || lrc_ac1_exceeded ||
                  lrc_ac2_exceeded || lrc_ac3_exceeded ) && !cap_in_aggr_count )
            cap_out_mpdu_status <= 1'b1;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) &&
                   txop_rx && cap_in_ev_rxend_ind && cap_txframeinfo_valid )
            cap_out_mpdu_status <= 1'b1;
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm && 
               (( cap_ra_addr_r[0] && !cap_frame_fc_r[8] ) || 
                ( cap_qos_mode_r && ( cap_frame_fc_r[7:0] == 8'h88 ) && 
                ( cap_frame_ack_policy_r != 2'h0 ))))
            cap_out_mpdu_status <= 1'b1;
    end

// Always Block for Discard of MPDU
// Note: The Default value should not be any of AC's
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_mpdu_discard <= 1'b0; 
        else if( !cap_in_enable )
            cap_out_mpdu_discard <= 1'b0; 
        else if( cap_out_mpdu_discard )
            cap_out_mpdu_discard <= 1'b0; 
		  else if(( src_ac0_exceeded || src_ac1_exceeded || src_ac2_exceeded || 
               src_ac3_exceeded || lrc_ac0_exceeded || lrc_ac1_exceeded ||
               lrc_ac2_exceeded || lrc_ac3_exceeded ) && !cap_in_aggr_count )
            cap_out_mpdu_discard <= 1'b1;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) &&
                   txop_rx && cap_in_ev_rxend_ind && cap_txframeinfo_valid )
            cap_out_mpdu_discard <= 1'b1; 
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm && 
               (( cap_ra_addr_r[0] && !cap_frame_fc_r[8] ) || 
                ( cap_qos_mode_r && ( cap_frame_fc_r[7:0] == 8'h88 ) && 
                ( cap_frame_ack_policy_r != 2'h0 ))))
            cap_out_mpdu_discard <= 1'b1; 
    end        

// Always Block for Short Retry Count for AC0
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac0_src <= 8'h0;
        else if( !cap_in_enable || cap_out_clear_ampdu || cap_out_mpdu_discard )
            cap_out_ac0_src <= 8'h0;
        else if( ac0_int_coll_r && contend && len_lt_rts && cap_qos_mode_r )
            cap_out_ac0_src <= cap_out_ac0_src + 8'h1;
        else if( txop_rx && cap_in_ev_rxend_ind && len_lt_rts && cap_ac0_txready_r &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_ac0_src <= cap_out_ac0_src + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || 
               ( cap_cts_timer_r == 5'h1 )) && 
               ( cap_out_txop_owner_ac == 3'h0 ) && 
                 cap_qos_mode_r && !cap_txprog && cap_txframeinfo_valid )
            cap_out_ac0_src <= cap_out_ac0_src + 8'h1;
    end

// Always Block for Short Retry Count for AC1
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac1_src <= 8'h0;
        else if( !cap_in_enable || cap_out_clear_ampdu || cap_out_mpdu_discard )
            cap_out_ac1_src <= 8'h0;
        else if( ac1_int_coll_r && contend && len_lt_rts && cap_qos_mode_r )
            cap_out_ac1_src <= cap_out_ac1_src + 8'h1;
        else if( txop_rx && cap_in_ev_rxend_ind && len_lt_rts && cap_ac1_txready_r &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_ac1_src <= cap_out_ac1_src + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || 
               ( cap_cts_timer_r == 5'h1 )) && 
               ( cap_out_txop_owner_ac == 3'h1 ) && 
                 cap_qos_mode_r && !cap_txprog && cap_txframeinfo_valid )
            cap_out_ac1_src <= cap_out_ac1_src + 8'h1;
    end

// Always Block for Short Retry Count for AC2
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac2_src <= 8'h0;
        else if( !cap_in_enable || cap_out_clear_ampdu || cap_out_mpdu_discard )
            cap_out_ac2_src <= 8'h0;
        else if( ac2_int_coll_r && contend && len_lt_rts && cap_qos_mode_r )
            cap_out_ac2_src <= cap_out_ac2_src + 8'h1;
        else if( txop_rx && cap_in_ev_rxend_ind && len_lt_rts && cap_ac2_txready_r &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_ac2_src <= cap_out_ac2_src + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || 
               ( cap_cts_timer_r == 5'h1 )) && 
               ( cap_out_txop_owner_ac == 3'h2 ) && 
                 cap_qos_mode_r && !cap_txprog && cap_txframeinfo_valid )
            cap_out_ac2_src <= cap_out_ac2_src + 8'h1;
    end

// Always Block for Short Retry Count for AC3
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac3_src <= 8'h0;
        else if( !cap_in_enable || cap_out_clear_ampdu || cap_out_mpdu_discard )
            cap_out_ac3_src <= 8'h0;
        else if( ac3_int_coll_r && contend && len_lt_rts && cap_qos_mode_r )
            cap_out_ac3_src <= cap_out_ac3_src + 8'h1;
        else if( txop_rx && cap_in_ev_rxend_ind && len_lt_rts && cap_ac3_txready_r &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_ac3_src <= cap_out_ac3_src + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || 
               ( cap_cts_timer_r == 5'h1 )) && 
               ( cap_out_txop_owner_ac == 3'h3 ) && 
                 cap_qos_mode_r && !cap_txprog && cap_txframeinfo_valid )
            cap_out_ac3_src <= cap_out_ac3_src + 8'h1;
    end

// Always Block for Long Retry Count for AC0
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac0_lrc <= 8'h0;
        else if( !cap_in_enable || cap_out_clear_ampdu || cap_out_mpdu_discard )
            cap_out_ac0_lrc <= 8'h0;
        else if( ac0_int_coll_r && contend && !len_lt_rts && cap_qos_mode_r )
            cap_out_ac0_lrc <= cap_out_ac0_lrc + 8'h1;
        else if( txop_rx && cap_in_ev_rxend_ind && !len_lt_rts && cap_ac0_txready_r &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_ac0_lrc <= cap_out_ac0_lrc + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts &&
               ( cap_out_txop_owner_ac == 3'h0 ) && ( cap_ack_timer_r == 5'h1 ) && 
                 !cap_txprog && cap_txframeinfo_valid )
            cap_out_ac0_lrc <= cap_out_ac0_lrc + 8'h1;
    end

// Always Block for Long Retry Count for AC1
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac1_lrc <= 8'h0;
        else if( !cap_in_enable || cap_out_clear_ampdu || cap_out_mpdu_discard )
            cap_out_ac1_lrc <= 8'h0;
        else if( ac1_int_coll_r && contend && !len_lt_rts && cap_qos_mode_r )
            cap_out_ac1_lrc <= cap_out_ac1_lrc + 8'h1;
        else if( txop_rx && cap_in_ev_rxend_ind && !len_lt_rts && cap_ac1_txready_r &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_ac1_lrc <= cap_out_ac1_lrc + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts && 
               ( cap_out_txop_owner_ac == 3'h1 ) && ( cap_ack_timer_r == 5'h1 ) && 
                 !cap_txprog && cap_txframeinfo_valid )
            cap_out_ac1_lrc <= cap_out_ac1_lrc + 8'h1;
    end

// Always Block for Long Retry Count for AC2
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac2_lrc <= 8'h0;
        else if( !cap_in_enable || cap_out_clear_ampdu || cap_out_mpdu_discard )
            cap_out_ac2_lrc <= 8'h0;
        else if( ac2_int_coll_r && contend && !len_lt_rts && cap_qos_mode_r )
            cap_out_ac2_lrc <= cap_out_ac2_lrc + 8'h1;
        else if( txop_rx && cap_in_ev_rxend_ind && !len_lt_rts && cap_ac2_txready_r &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_ac2_lrc <= cap_out_ac2_lrc + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts && 
               ( cap_out_txop_owner_ac == 3'h1 ) && ( cap_ack_timer_r == 5'h1 ) && 
                 !cap_txprog && cap_txframeinfo_valid )
            cap_out_ac2_lrc <= cap_out_ac2_lrc + 8'h1;
    end

// Always Block for Long Retry Count for AC3
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ac3_lrc <= 8'h0;
        else if( !cap_in_enable || cap_out_clear_ampdu || cap_out_mpdu_discard )
            cap_out_ac3_lrc <= 8'h0;
        else if( ac3_int_coll_r && contend && !len_lt_rts && cap_qos_mode_r )
            cap_out_ac3_lrc <= cap_out_ac3_lrc + 8'h1;
        else if( txop_rx && cap_in_ev_rxend_ind && !len_lt_rts && cap_ac3_txready_r &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )))
            cap_out_ac3_lrc <= cap_out_ac3_lrc + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts && 
               ( cap_out_txop_owner_ac == 3'h1 ) && ( cap_ack_timer_r == 5'h1 ) && 
                 !cap_txprog && cap_txframeinfo_valid )
            cap_out_ac3_lrc <= cap_out_ac3_lrc + 8'h1;
    end

// Always Block for SSRC AC0
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac0_ssrc_r <= 8'h0;
        else if( !cap_in_enable )
            cap_ac0_ssrc_r <= 8'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                   txop_rx && cap_in_ev_rxend_ind && ( cap_out_txop_owner_ac == 3'h0 ))
            cap_ac0_ssrc_r <= 8'h0;
        else if( ac0_int_coll_r && len_lt_rts && cap_qos_mode_r ) 
            cap_ac0_ssrc_r <= cap_ac0_ssrc_r + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || 
               ( cap_cts_timer_r == 5'h1 )) && 
               ( cap_out_txop_owner_ac == 3'h0 ) && 
                 cap_qos_mode_r && !cap_txprog && cap_txframeinfo_valid )
            cap_ac0_ssrc_r <= cap_ac0_ssrc_r + 8'h1;
    end

// Always Block for SSRC AC1
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac1_ssrc_r <= 8'h0;
        else if( !cap_in_enable )
            cap_ac1_ssrc_r <= 8'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                   txop_rx && cap_in_ev_rxend_ind && ( cap_out_txop_owner_ac == 3'h1 ))
            cap_ac1_ssrc_r <= 8'h0;
        else if( ac1_int_coll_r && len_lt_rts && cap_qos_mode_r ) 
            cap_ac1_ssrc_r <= cap_ac1_ssrc_r + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || 
               ( cap_cts_timer_r == 5'h1 )) && 
               ( cap_out_txop_owner_ac == 3'h1 ) && 
                 cap_qos_mode_r && !cap_txprog && cap_txframeinfo_valid )
            cap_ac1_ssrc_r <= cap_ac1_ssrc_r + 8'h1;
    end

// Always Block for SSRC AC2
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac2_ssrc_r <= 8'h0;
        else if( !cap_in_enable )
            cap_ac2_ssrc_r <= 8'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                   txop_rx && cap_in_ev_rxend_ind && ( cap_out_txop_owner_ac == 3'h2 ))
            cap_ac2_ssrc_r <= 8'h0;
        else if( ac2_int_coll_r && len_lt_rts && cap_qos_mode_r ) 
            cap_ac2_ssrc_r <= cap_ac2_ssrc_r + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || 
               ( cap_cts_timer_r == 5'h1 )) && 
               ( cap_out_txop_owner_ac == 3'h2 ) && 
                 cap_qos_mode_r && !cap_txprog && cap_txframeinfo_valid )
            cap_ac2_ssrc_r <= cap_ac2_ssrc_r + 8'h1;
    end

// Always Block for SSRC AC3
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac3_ssrc_r <= 8'h0;
        else if( !cap_in_enable )
            cap_ac3_ssrc_r <= 8'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                   txop_rx && cap_in_ev_rxend_ind && ( cap_out_txop_owner_ac == 3'h3 ))
            cap_ac3_ssrc_r <= 8'h0;
        else if( ac3_int_coll_r && len_lt_rts && cap_qos_mode_r ) 
            cap_ac3_ssrc_r <= cap_ac3_ssrc_r + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && (( len_lt_rts && 
               ( cap_ack_timer_r == 5'h1 )) || 
               ( cap_cts_timer_r == 5'h1 )) && 
               ( cap_out_txop_owner_ac == 3'h3 ) && 
                 cap_qos_mode_r && !cap_txprog && cap_txframeinfo_valid )
            cap_ac3_ssrc_r <= cap_ac3_ssrc_r + 8'h1;
    end

// Always Block for SLRC AC0
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac0_slrc_r <= 8'h0;
        else if( !cap_in_enable )
            cap_ac0_slrc_r <= 8'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                   txop_rx && cap_in_ev_rxend_ind && ( cap_out_txop_owner_ac == 3'h0 ))
            cap_ac0_slrc_r <= 8'h0;
        else if( ac0_int_coll_r && !len_lt_rts && cap_qos_mode_r )
            cap_ac0_slrc_r <= cap_ac0_slrc_r + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts && cap_qos_mode_r &&
               ( cap_ack_timer_r == 5'h1 ) && ( cap_out_txop_owner_ac == 3'h0 ) &&
                 !cap_txprog && cap_txframeinfo_valid )
            cap_ac0_slrc_r <= cap_ac0_slrc_r + 8'h1;
    end

// Always Block for SLRC AC1
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac1_slrc_r <= 8'h0;
        else if( !cap_in_enable )
            cap_ac1_slrc_r <= 8'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                   txop_rx && cap_in_ev_rxend_ind && ( cap_out_txop_owner_ac == 3'h1 ))
            cap_ac1_slrc_r <= 8'h0;
        else if( ac1_int_coll_r && !len_lt_rts && cap_qos_mode_r ) 
            cap_ac1_slrc_r <= cap_ac1_slrc_r + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts && cap_qos_mode_r && 
               ( cap_ack_timer_r == 5'h1 ) && ( cap_out_txop_owner_ac == 3'h1 ) &&
                 !cap_txprog && cap_txframeinfo_valid )
            cap_ac1_slrc_r <= cap_ac1_slrc_r + 8'h1;
    end

// Always Block for SLRC AC2
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac2_slrc_r <= 8'h0;
        else if( !cap_in_enable )
            cap_ac2_slrc_r <= 8'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                   txop_rx && cap_in_ev_rxend_ind && ( cap_out_txop_owner_ac == 3'h2 ))
            cap_ac2_slrc_r <= 8'h0;
        else if( ac2_int_coll_r && !len_lt_rts && cap_qos_mode_r )
            cap_ac2_slrc_r <= cap_ac2_slrc_r + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts && cap_qos_mode_r && 
               ( cap_ack_timer_r == 5'h1 ) && ( cap_out_txop_owner_ac == 3'h2 ) &&
                 !cap_txprog && cap_txframeinfo_valid )
            cap_ac2_slrc_r <= cap_ac2_slrc_r + 8'h1;
    end

// Always Block for SLRC AC3
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac3_slrc_r <= 8'h0;
        else if( !cap_in_enable )
            cap_ac3_slrc_r <= 8'h0;
        else if((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
                 ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                   txop_rx && cap_in_ev_rxend_ind && ( cap_out_txop_owner_ac == 3'h3 ))
            cap_ac3_slrc_r <= 8'h0;
        else if( ac3_int_coll_r && !len_lt_rts && cap_qos_mode_r )
            cap_ac3_slrc_r <= cap_ac3_slrc_r + 8'h1;
        else if( txop_rx && cap_in_ev_timer_tick && !len_lt_rts && cap_qos_mode_r && 
               ( cap_ack_timer_r == 5'h1 ) && ( cap_out_txop_owner_ac == 3'h3 ) &&
                 !cap_txprog && cap_txframeinfo_valid )
            cap_ac3_slrc_r <= cap_ac3_slrc_r + 8'h1;
    end

// Always Block for CAP AC0 CW Value 
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET ) 
            cap_ac0_cw_r <= 16'h0;
        else if( !cap_in_enable )
            cap_ac0_cw_r <= 16'h0;
        else if( cap_qos_mode_r && ( idle || cap_out_mpdu_status || cap_out_clear_ampdu ||
             ((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                 txop_rx && cap_in_ev_rxend_ind ) || txop_txnavend ))
            cap_ac0_cw_r <= cap_in_ac0_cwmin_value;
        else if( !cap_qos_mode_r && ( idle || cap_out_mpdu_status || receive || eifs || txop ))
            cap_ac0_cw_r <= cap_in_cwmin_value;  
        else if( txop_rx && cap_in_ev_timer_tick && (( cap_ack_timer_r == 5'h1 ) || 
               ( cap_cts_timer_r == 5'h1 )) && ( cap_out_txop_owner_ac == 3'h0 ) && 
               ( !src_ac0_exceeded || !lrc_ac0_exceeded ) && cap_qos_mode_r && 
               ( cap_ac0_cw_r < cap_in_ac0_cwmax_value ))
            cap_ac0_cw_r <= ( cap_ac0_cw_r << 1 ) + 16'h1;     
        else if( ac0_int_coll_r && cap_qos_mode_r && contend &&
               ( cap_ac0_cw_r < cap_in_ac0_cwmax_value ))
            cap_ac0_cw_r <= ( cap_ac0_cw_r << 1 ) + 16'h1;     
//        else if( txop_rx && cap_in_ev_timer_tick && (( cap_ack_timer_r == 5'h1 ) ||
//               ( cap_cts_timer_r == 5'h1 )) && ( cap_out_txop_owner_ac == 3'h0 ) && 
//               ( !src_ac0_exceeded || !lrc_ac0_exceeded ) && !cap_qos_mode_r && 
//               ( cap_ac0_cw_r < cap_in_cwmax_value )) 
//            cap_ac0_cw_r <= ( cap_ac0_cw_r << 1 ) + 16'h1;     
    end

// Always Block for CAP AC1 CW Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac1_cw_r <= 16'h0;
        else if( !cap_in_enable )
            cap_ac1_cw_r <= 16'h0;
        else if( cap_qos_mode_r && ( idle || cap_out_mpdu_status || cap_out_clear_ampdu ||
             ((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                 txop_rx && cap_in_ev_rxend_ind ) || txop_txnavend ))
            cap_ac1_cw_r <= cap_in_ac1_cwmin_value;
        else if( txop_rx && cap_in_ev_timer_tick && (( cap_ack_timer_r == 5'h1 ) || 
               ( cap_cts_timer_r == 5'h1 )) && (cap_out_txop_owner_ac == 3'h1 ) && 
               ( !src_ac1_exceeded || !lrc_ac1_exceeded ) && cap_qos_mode_r && 
               ( cap_ac1_cw_r < cap_in_ac1_cwmax_value ))
            cap_ac1_cw_r <= ( cap_ac1_cw_r << 1 ) + 16'h1;     
        else if( ac1_int_coll_r && cap_qos_mode_r && contend &&
               ( cap_ac1_cw_r < cap_in_ac1_cwmax_value ))
            cap_ac1_cw_r <= ( cap_ac1_cw_r << 1 ) + 16'h1;     
    end

// Always Block for CAP AC2 CW Value 
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac2_cw_r <= 16'h0;
        else if( !cap_in_enable )
            cap_ac2_cw_r <= 16'h0;
        else if( cap_qos_mode_r && ( idle || cap_out_mpdu_status || cap_out_clear_ampdu ||
             ((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                 txop_rx && cap_in_ev_rxend_ind ) || txop_txnavend ))
            cap_ac2_cw_r <= cap_in_ac2_cwmin_value;
        else if( txop_rx && cap_in_ev_timer_tick && (( cap_ack_timer_r == 5'h1 ) ||
               ( cap_cts_timer_r == 5'h1 )) && ( cap_out_txop_owner_ac == 3'h2 ) && 
               ( !src_ac2_exceeded || !lrc_ac2_exceeded ) && cap_qos_mode_r && 
               ( cap_ac2_cw_r < cap_in_ac2_cwmax_value ))
            cap_ac2_cw_r <= ( cap_ac2_cw_r << 1 ) + 16'h1;     
        else if( ac2_int_coll_r && cap_qos_mode_r && contend &&
               ( cap_ac2_cw_r < cap_in_ac2_cwmax_value ))
            cap_ac2_cw_r <= ( cap_ac2_cw_r << 1 ) + 16'h1;     
    end

// Always Block for CAP AC3 CW Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ac3_cw_r <= 16'h0;
        else if( !cap_in_enable )
            cap_ac3_cw_r <= 16'h0;
        else if( cap_qos_mode_r && ( idle || cap_out_mpdu_status || cap_out_clear_ampdu ||
             ((( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )) && 
                 txop_rx && cap_in_ev_rxend_ind ) || txop_txnavend ))
            cap_ac3_cw_r <= cap_in_ac3_cwmin_value;
        else if( txop_rx && cap_in_ev_timer_tick && (( cap_ack_timer_r == 5'h1 ) ||
               ( cap_cts_timer_r == 5'h1)) && (cap_out_txop_owner_ac == 3'h3) && 
               ( !src_ac3_exceeded || !lrc_ac3_exceeded ) && cap_qos_mode_r && 
               ( cap_ac3_cw_r < cap_in_ac3_cwmax_value ))
            cap_ac3_cw_r <= ( cap_ac3_cw_r << 1 ) + 16'h1;     
        else if( ac3_int_coll_r && cap_qos_mode_r && contend &&
               ( cap_ac3_cw_r < cap_in_ac3_cwmax_value ))
            cap_ac3_cw_r <= ( cap_ac3_cw_r << 1 ) + 16'h1;     
    end

// Always Block for Channel Busy Register
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_channel_bw <= `CBW20; 
        else if( !cap_in_enable )
            cap_out_channel_bw <= `CBW20;
        else if(( txop || txop_send ) && cap_qos_mode_r ) 
        begin
            if( cap_channel_busy == `UU_WLAN_BUSY_CH_SECONDARY40 )
            begin
                if( cap_in_txvec_chbw == `CBW40 )
                    cap_out_channel_bw <= `CBW40;
                else
                    cap_out_channel_bw <= `CBW20; 
            end
            else if( cap_channel_busy == `UU_WLAN_BUSY_CH_SECONDARY80 )
            begin
                if( cap_in_txvec_chbw == `CBW80 )
                    cap_out_channel_bw <= `CBW80;
                else if( cap_in_txvec_chbw == `CBW40 )
                    cap_out_channel_bw <= `CBW40; 
                else
                    cap_out_channel_bw <= `CBW20;
            end
            else if( cap_channel_busy == `UU_WLAN_CH_IDLE )
            begin
                if( cap_in_txvec_chbw == `CBW160 )
                    cap_out_channel_bw <= cap_in_txvec_chbw;
                else if( cap_in_txvec_chbw == `CBW80 )
                    cap_out_channel_bw <= `CBW80;
                else if( cap_in_txvec_chbw == `CBW40 )
                    cap_out_channel_bw <= `CBW40; 
                else
                    cap_out_channel_bw <= `CBW20; 
            end
            else
                cap_out_channel_bw <= `CBW20; 
        end
        else                    
            cap_out_channel_bw <= cap_out_channel_bw;
    end

// Always Block for Ac0 Internal Collision
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            ac0_int_coll_r <= 1'b0;
        else if( !cap_in_enable )
            ac0_int_coll_r <= 1'b0;
        else if( ac0_int_coll_r )
            ac0_int_coll_r <= 1'b0;
        else if( contend && ( cap_out_txop_owner_ac == 3'h7 ) &&  
               ( cap_ac_priority == 2'h0 ) && cap_in_ev_timer_tick )
            ac0_int_coll_r <= 1'b0;
        else if( contend && ( cap_out_txop_owner_ac != 3'h0 ) && 
               ( cap_ac_priority == 2'h0 ) && cap_in_ev_timer_tick )
            ac0_int_coll_r <= 1'b1;
    end
    
// Always Block for Ac1 Internal Collision
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            ac1_int_coll_r <= 1'b0;
        else if( !cap_in_enable )
            ac1_int_coll_r <= 1'b0;
        else if( ac1_int_coll_r )
            ac1_int_coll_r <= 1'b0;
        else if( contend && ( cap_out_txop_owner_ac == 3'h7 ) && 
               ( cap_ac_priority == 2'h1 ) && cap_in_ev_timer_tick )
            ac1_int_coll_r <= 1'b0;
        else if( contend && ( cap_out_txop_owner_ac != 3'h1 ) && 
               ( cap_ac_priority == 2'h1 ) && cap_in_ev_timer_tick )
            ac1_int_coll_r <= 1'b1;
    end

// Always Block for Ac2 Internal Collision
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            ac2_int_coll_r <= 1'b0;
        else if( !cap_in_enable )
            ac2_int_coll_r <= 1'b0;
        else if( ac2_int_coll_r )
            ac2_int_coll_r <= 1'b0;
        else if( contend && ( cap_out_txop_owner_ac == 3'h7 ) && 
               ( cap_ac_priority == 2'h2 ) && cap_in_ev_timer_tick )
            ac2_int_coll_r <= 1'b0;
        else if( contend && ( cap_out_txop_owner_ac != 3'h2 ) && 
               ( cap_ac_priority == 2'h2 ) && cap_in_ev_timer_tick )
            ac2_int_coll_r <= 1'b1;
    end

// Always Block for Ac3 Internal Collision
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            ac3_int_coll_r <= 1'b0;
        else if( !cap_in_enable )
            ac3_int_coll_r <= 1'b0;
        else if( ac3_int_coll_r )
            ac3_int_coll_r <= 1'b0;
        else if( contend && ( cap_out_txop_owner_ac == 3'h7 ) &&
               ( cap_ac_priority == 2'h3 ) && cap_in_ev_timer_tick )
            ac3_int_coll_r <= 1'b0;
        else if( contend && ( cap_out_txop_owner_ac != 3'h3 ) &&
               ( cap_ac_priority == 2'h3 ) && cap_in_ev_timer_tick )
            ac3_int_coll_r <= 1'b1;
    end

// Always Block to Store the Qos Mode
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_qos_mode_r <= 1'b0;
        else if( !cap_in_enable )
            cap_qos_mode_r <= 1'b0;
        else if( idle )
            cap_qos_mode_r <= cap_in_qos_mode;
    end

// Always Block for Updated Channel List
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_update_channel_list_r <= `UU_WLAN_CH_IDLE;
        else if( !cap_in_enable )
            cap_update_channel_list_r <= `UU_WLAN_CH_IDLE;
        else if(( idle || receive || contend || eifs ) && 
	       (( cap_in_ev_phyenergy_ind && ( cap_in_cca_status == 2'h0 )) || 
                  cap_in_ev_phyrx_error ))
            cap_update_channel_list_r <= `UU_WLAN_CH_IDLE;
        else if(( idle || receive || contend || eifs) &&
                  cap_in_ev_phyenergy_ind && ( cap_in_cca_status != 2'h0 ))
            cap_update_channel_list_r <= cap_in_channel_list;
    end 

// Always Block for Prev Channel List
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_prev_channel_list_r <= 3'h0;
        else if( !cap_in_enable )
            cap_prev_channel_list_r <= 3'h0;
        else if( contend && cap_in_ev_timer_tick && ( cap_ac_priority == 2'b11 ) && 
               ( cap_slot_timer_r == 4'h1 ) && 
              (( cap_ac3_aifs_r == 4'h1 ) || ( cap_ac3_backoff_r == 16'h1 )))
            cap_prev_channel_list_r <= cap_update_channel_list_r;
        else if( contend && cap_in_ev_timer_tick && ( cap_ac_priority == 2'b10 ) &&
               ( cap_slot_timer_r == 4'h1 ) && 
              (( cap_ac2_aifs_r == 4'h1 ) || (cap_ac2_backoff_r == 16'h1 )))
            cap_prev_channel_list_r <= cap_update_channel_list_r;
        else if( contend && cap_in_ev_timer_tick && ( cap_ac_priority == 2'b01 ) &&
               ( cap_slot_timer_r == 4'h1 ) && 
              (( cap_ac1_aifs_r == 4'h1 ) || ( cap_ac1_backoff_r == 16'h1 )))
            cap_prev_channel_list_r <= cap_update_channel_list_r;
        else if( contend && cap_in_ev_timer_tick && ( cap_ac_priority == 2'b0 ) &&
               ( cap_slot_timer_r == 4'h1 ) && 
              (( cap_ac0_aifs_r == 4'h1 ) || ( cap_ac0_backoff_r == 16'h1 )))
            cap_prev_channel_list_r <= cap_update_channel_list_r;
    end

// ALways Block to generate a toggle signal for TX DATA Pulse.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )   
            cap_txdata_confirm_r <= 1'b0;
        else if( !cap_in_enable )  
            cap_txdata_confirm_r <= 1'b0;
        else if( cap_in_ev_txstart_confirm || cap_in_ev_txend_confirm )
            cap_txdata_confirm_r <= 1'b0;
        else if( cap_in_ev_txdata_confirm )    
            cap_txdata_confirm_r <= 1'b1;
    end 

// ALways Block to generate a toggle signal for TX Start Pulse.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )   
            cap_txstart_confirm_r <= 1'b0;
        else if( !cap_in_enable )  
            cap_txstart_confirm_r <= 1'b0;
        else if( cap_in_ev_txdata_confirm || cap_in_ev_txend_confirm ||
                 cap_out_ev_txop_txstart_req )
            cap_txstart_confirm_r <= 1'b0;
        else if( cap_in_ev_txstart_confirm )    
            cap_txstart_confirm_r <= 1'b1;
    end 

// ALways Block to generate a toggle signal for RX Start Pulse.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )   
            cap_rxstart_ind_r <= 1'b0;
        else if( !cap_in_enable )  
            cap_rxstart_ind_r <= 1'b0;
        else if( cap_in_ev_rxdata_ind || cap_in_ev_rxend_ind )
            cap_rxstart_ind_r <= 1'b0;
        else if( cap_in_ev_rxstart_ind )    
            cap_rxstart_ind_r <= 1'b1;
    end 

// ALways Block to generate a toggle signal for RX DATA Pulse.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )   
            cap_rxdata_ind_r <= 1'b0;
        else if( !cap_in_enable )  
            cap_rxdata_ind_r <= 1'b0;
        else if( cap_in_ev_rxstart_ind || cap_in_ev_rxend_ind )
            cap_rxdata_ind_r <= 1'b0;
        else if( cap_in_ev_rxdata_ind )    
            cap_rxdata_ind_r <= 1'b1;
    end 

// Always Block to store L_DataRate from Rx Vector.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )   
            cap_ldatarate_r <= 4'h0;
        else if( !cap_in_enable )  
            cap_ldatarate_r <= 4'h0;
        else if( cap_in_ev_rxstart_ind || cap_in_ev_txstart_confirm )
            cap_ldatarate_r <= 4'h0;
        else if(( cap_rxstart_ind_r || cap_txstart_confirm_r ) && 
                  cap_in_frame_valid && ( cap_frame_vector_count_r == 5'h1 ))
            cap_ldatarate_r <= cap_in_frame[7:4];
    end        

// Always Block to count the Frame Vector byte reads 
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )   
            cap_frame_vector_count_r <= 5'h0;
        else if( !cap_in_enable )
            cap_frame_vector_count_r <= 5'h0;
        else if( cap_in_ev_txdata_confirm || cap_in_ev_rxdata_ind )
            cap_frame_vector_count_r <= 5'h0;
        else if(( cap_in_ev_rxstart_ind || cap_rxstart_ind_r ||
                  cap_in_ev_txstart_confirm || cap_txstart_confirm_r ) &&
                  cap_in_frame_valid )
            cap_frame_vector_count_r <= cap_frame_vector_count_r + 5'h1;
    end
           
// Always Block to count the Header byte reads.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )   
            cap_frame_read_count_r <= 6'h0;
        else if( !cap_in_enable )
            cap_frame_read_count_r <= 6'h0;
        else if( cap_in_ev_rxstart_ind || cap_in_ev_txstart_confirm )
            cap_frame_read_count_r <= 6'h0;
        else if((( cap_in_ev_rxdata_ind || cap_rxdata_ind_r ) ||
                 ( cap_in_ev_txdata_confirm || cap_txdata_confirm_r )) && 
                   cap_in_frame_valid )
            cap_frame_read_count_r <= cap_frame_read_count_r + 6'h1;
    end 

// Always Block to latch the Frame Control Field
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_frame_fc_r <= 9'h0;
        else if( !cap_in_enable )
            cap_frame_fc_r <= 9'h0;
        else if( cap_in_ev_rxstart_ind || cap_in_ev_txstart_confirm )
            cap_frame_fc_r <= 9'h0;
        else if((receive)&&
               (( cap_in_ev_rxdata_ind || cap_rxdata_ind_r )&& cap_in_frame_valid ))
        begin
            if( cap_frame_read_count_r == 6'h0 )
                cap_frame_fc_r <= { 1'b0, cap_in_frame };
            else if( cap_frame_read_count_r == 6'h1 )
            cap_frame_fc_r <= { cap_in_frame[0], cap_frame_fc_r[7:0] };
        end  
        else if(( txop || txop_send || txop_rx))//  &&( cap_in_ev_txdata_confirm || cap_txdata_confirm_r ))
            
                cap_frame_fc_r <= { cap_in_tx_mpdu_fc1[0],cap_in_tx_mpdu_fc0 };       
        
        end
        
        

// Always Block to register the Frame Duration Field
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_frame_duration_r <= 16'h0;
        else if( !cap_in_enable )
            cap_frame_duration_r <= 16'h0;
        else if( cap_in_ev_rxstart_ind || cap_in_ev_txstart_confirm )
            cap_frame_duration_r <= 16'h0;
        else if(( receive || txop_rx || txop || txop_send ) && 
                ( cap_rxdata_ind_r || cap_txdata_confirm_r ) && cap_in_frame_valid )
        begin       
            if( cap_frame_read_count_r == 6'h2 )
                cap_frame_duration_r <= { cap_frame_duration_r[15:8], cap_in_frame };
            else if( cap_frame_read_count_r == 6'h3 )
                cap_frame_duration_r <= { cap_in_frame, cap_frame_duration_r[7:0] };
        end
    end

// Always Block to register the Qos ACK Policy
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_frame_ack_policy_r <= 2'h0;
        else if( !cap_in_enable )
            cap_frame_ack_policy_r <= 2'h0;
        else if( cap_in_ev_rxstart_ind )
            cap_frame_ack_policy_r <= 16'h0;
	 else if(receive &&  cap_in_frame_valid && cap_qos_mode_r && 
                ( cap_rxdata_ind_r ) && 
                ( cap_frame_read_count_r == 6'h18 ))
            cap_frame_ack_policy_r <= cap_in_frame[6:5];

        else if( txop_rx || txop || txop_send )
            cap_frame_ack_policy_r <= cap_in_tx_ack_policy;
    end                   

// Always Block to Store the RA for the Rx Packet
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ra_addr_r <= 48'h0;
        else if( !cap_in_enable )
            cap_ra_addr_r <= 48'h0;
        else if( cap_in_ev_rxstart_ind )
            cap_ra_addr_r <= 48'h0;
        else if(receive && cap_in_frame_valid && cap_rxdata_ind_r )
        begin
            if( cap_frame_read_count_r == 6'h4 )
                cap_ra_addr_r <= { 40'h0, cap_in_frame };
            else if( cap_frame_read_count_r == 6'h5 )
                cap_ra_addr_r <= { 32'h0, cap_in_frame, cap_ra_addr_r[7:0] };
            else if( cap_frame_read_count_r == 6'h6 )
                cap_ra_addr_r <= { 24'h0, cap_in_frame, cap_ra_addr_r[15:0] };
            else if( cap_frame_read_count_r == 6'h7 )
                cap_ra_addr_r <= { 16'h0, cap_in_frame, cap_ra_addr_r[23:0] };
            else if( cap_frame_read_count_r == 6'h8 )
                cap_ra_addr_r <= { 8'h0, cap_in_frame, cap_ra_addr_r[31:0] };
            else if( cap_frame_read_count_r == 6'h9 )
                cap_ra_addr_r <= { cap_in_frame, cap_ra_addr_r[39:0] };
        end  
      //else if((txop_rx || txop || txop_send ))// && cap_txdata_confirm_r)
      else      
         cap_ra_addr_r <= cap_in_tx_ra_addr;      
    end

// Always Block to Check the validity of RA.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_ra_no_match_r <= 1'b0;
        else if( !cap_in_enable )
            cap_ra_no_match_r <= 1'b0;
        else if(( receive || txop_rx ) && cap_in_ev_rxstart_ind )           
            cap_ra_no_match_r <= 1'b0;
        else if(( receive || txop_rx || txop || txop_send ) && 
                  cap_in_frame_valid && ( cap_txdata_confirm_r ||
                  cap_rxdata_ind_r ) && ( cap_frame_read_count_r == 6'hA ))
            cap_ra_no_match_r <= ( cap_ra_addr_r != cap_in_mac_addr );
    end

// Always Block for Packet Status of AC.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_pkt_error_r <= 1'b0;
        else if( !cap_in_enable )
            cap_pkt_error_r <= 1'b0;
        else if( idle || contend )
            cap_pkt_error_r <= 1'b0;
        else if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_FRAME_ERROR )
            cap_pkt_error_r <= 1'b1;
    end

// Always Block for TX Programming
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
        begin
            cap_ac0_txprog_r <= 1'b0;
            cap_ac1_txprog_r <= 1'b0;
            cap_ac2_txprog_r <= 1'b0;
            cap_ac3_txprog_r <= 1'b0;
        end
        `ifdef RTL_TEST_SIM
        else if( cap_in_enable && test_FLAG )
        begin
            cap_ac0_txprog_r <= test_AC0_TXPROG;
            cap_ac1_txprog_r <= test_AC1_TXPROG;
            cap_ac2_txprog_r <= test_AC2_TXPROG;
            cap_ac3_txprog_r <= test_AC3_TXPROG;
        end
        `endif
        else if(( !cap_in_enable ) || ( cap_out_txop_owner_ac == 3'h7 ))
        begin
            cap_ac0_txprog_r <= 1'b0;
            cap_ac1_txprog_r <= 1'b0;
            cap_ac2_txprog_r <= 1'b0;
            cap_ac3_txprog_r <= 1'b0;
        end
        else if(( txop || txop_send ) && 
                  cap_in_ev_txend_confirm && cap_qos_mode_r )
        begin
            cap_ac0_txprog_r <= 1'b0;
            cap_ac1_txprog_r <= 1'b0;
            cap_ac2_txprog_r <= 1'b0;
            cap_ac3_txprog_r <= 1'b0;
        end
        else if(( cap_out_txop_owner_ac == 3'h3 ) && cap_qos_mode_r &&
                ( txop || txop_send) && cap_in_ev_txstart_confirm )
        begin
            cap_ac0_txprog_r <= 1'b0;
            cap_ac1_txprog_r <= 1'b0;
            cap_ac2_txprog_r <= 1'b0;
            cap_ac3_txprog_r <= 1'b1;
        end
        else if(( cap_out_txop_owner_ac == 3'h2 ) && cap_qos_mode_r && 
                ( txop || txop_send ) && cap_in_ev_txstart_confirm )
        begin
            cap_ac0_txprog_r <= 1'b0;
            cap_ac1_txprog_r <= 1'b0;
            cap_ac2_txprog_r <= 1'b1;
            cap_ac3_txprog_r <= 1'b0;
        end
        else if(( cap_out_txop_owner_ac == 3'h1 ) && cap_qos_mode_r && 
                ( txop || txop_send ) && cap_in_ev_txstart_confirm )
        begin
            cap_ac0_txprog_r <= 1'b0;
            cap_ac1_txprog_r <= 1'b1;
            cap_ac2_txprog_r <= 1'b0;
            cap_ac3_txprog_r <= 1'b0;
        end
        else if(( cap_out_txop_owner_ac == 3'h0 ) && cap_qos_mode_r && 
                ( txop || txop_send ) && cap_in_ev_txstart_confirm )
        begin
            cap_ac0_txprog_r <= 1'b1;
            cap_ac1_txprog_r <= 1'b0;
            cap_ac2_txprog_r <= 1'b0;
            cap_ac3_txprog_r <= 1'b0;
        end
    end

// Always Block for TXOP State Return Value
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_txop_return_r <= SM_STATE_CONTINUE;
        else if( !cap_in_enable )
            cap_txop_return_r <= SM_STATE_CONTINUE;
        else if( idle || receive || contend || eifs )
            cap_txop_return_r <= SM_STATE_CONTINUE;
        else if(( txop || txop_send ) && cap_qos_mode_r &&
                ( cap_ac_priority != cap_out_txop_owner_ac )) 
            cap_txop_return_r <= SM_CONTEND;
        else if(( txop || txop_send ) && 
                ( cap_in_ev_txstart_confirm || cap_in_ev_txdata_confirm ))
            cap_txop_return_r <= SM_STATE_CONTINUE;
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm )
        begin
            if( cap_ra_addr_r[0] && !cap_frame_fc_r[8] )
            begin
                if( !cap_qos_mode_r )
                    cap_txop_return_r <= SM_TX_END;
                else 
                    cap_txop_return_r <= SM_START_SENDING;
            end
            else if( cap_qos_mode_r && ( cap_frame_fc_r[7:0] == 8'h88 ) && 
                   ( cap_frame_ack_policy_r != 2'h0 ))
                cap_txop_return_r <= SM_START_SENDING;
            else
                cap_txop_return_r <= SM_WAIT_FOR_RX;
        end
        else if( txop_send )
        begin
            if( !cap_txframeinfo_valid ||
             (( cap_in_txhandler_ret_value == 32'hFFFFFFFF ) &&
                cap_in_txhandler_ret_valid ))
                cap_txop_return_r <= SM_TX_END;
            else 
                cap_txop_return_r <= SM_STATE_CONTINUE;
        end
        else if( txop )
        begin
            if( cap_txframeinfo_valid )   
                cap_txop_return_r <= SM_STATE_CONTINUE;
            else
                cap_txop_return_r <= SM_TX_END;
        end
        else if( txop_rx && cap_in_ev_timer_tick )
        begin
            if( cap_txprog )
                cap_txop_return_r <= SM_STATE_CONTINUE;
            else if( !cap_txframeinfo_valid )
                cap_txop_return_r <= SM_TX_END;
            else if(( cap_ack_timer_r == 5'h1 ) || 
                    ( cap_cts_timer_r == 5'h1 ))
                cap_txop_return_r <= SM_TX_FAILURE;
            else
                cap_txop_return_r <= SM_STATE_CONTINUE;
        end
        else if( txop_rx && ( cap_in_ev_txstart_confirm || cap_in_ev_txdata_confirm || 
                 cap_in_ev_rxstart_ind || cap_in_ev_rxdata_ind ))
            cap_txop_return_r <= SM_STATE_CONTINUE;
        else if( txop_rx && cap_in_ev_txend_confirm )
            cap_txop_return_r <= SM_RX_VALID_FRAME;
        else if( txop_rx && cap_in_ev_rxend_ind && cap_txframeinfo_valid )
        begin
            if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_FRAME_ERROR )
                cap_txop_return_r <= SM_TXOP_ERROR_RECOVERY;
            else if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_NAV_UPDATE )
                cap_txop_return_r <= SM_TXOP_NAV_UPDATE;
            else if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_SEND_RESP )
                cap_txop_return_r <= SM_STATE_CONTINUE;
            else if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD ) 
                cap_txop_return_r <= SM_TX_END;
            else if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD )
                cap_txop_return_r <= SM_START_SENDING;
            else if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD )
                cap_txop_return_r <= SM_START_SENDING;
            else if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS )
                cap_txop_return_r <= SM_START_SENDING;
            else
                cap_txop_return_r <= SM_RX_VALID_FRAME;
        end
        else if( txop_rx && cap_in_ev_rxend_ind && !cap_txframeinfo_valid )
            cap_txop_return_r <= SM_TX_END;
        else if( txop_rx && cap_in_ev_phyrx_error )
            cap_txop_return_r <= SM_TXOP_ERROR_RECOVERY;
        else if( txop_txnavend && cap_in_ev_timer_tick )
        begin
            if((( cap_out_txop_owner_ac == 3'h0 ) && ( cap_out_ac0_txnav_value <= 16'h1 )) ||
               (( cap_out_txop_owner_ac == 3'h1 ) && ( cap_out_ac1_txnav_value <= 16'h1 )) ||
               (( cap_out_txop_owner_ac == 3'h2 ) && ( cap_out_ac2_txnav_value <= 16'h1 )) ||
               (( cap_out_txop_owner_ac == 3'h3 ) && ( cap_out_ac3_txnav_value <= 16'h1 ))) 
                cap_txop_return_r <= SM_TX_NAV_ENDED; 
            else
                cap_txop_return_r <= SM_STATE_CONTINUE;
        end
        else
            cap_txop_return_r <= cap_txop_return_r;
    end

// Always Block for CCARESET REQUEST Output Event Generation.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ev_ccareset_req <= 1'b0;
        else if( !cap_in_enable )
            cap_out_ev_ccareset_req <= 1'b0;
        else if( cap_out_ev_ccareset_req )
            cap_out_ev_ccareset_req <= 1'b0;
        else if( receive && cap_in_ev_txend_confirm && ( cap_nav_val_r == 16'h0 ))
            cap_out_ev_ccareset_req <= 1'b1;
        else if( receive && cap_in_ev_timer_tick && cap_ra_no_match_r && 
               ( cap_nav_reset_timer_r == 16'h1 ) && ( cap_frame_fc_r[7:0] != 8'hC4 ))
            cap_out_ev_ccareset_req <= 1'b1;
        else if( eifs && cap_in_ev_timer_tick && ( cap_nav_val_r == 16'h1 ))
            cap_out_ev_ccareset_req <= 1'b1;
    end

// Always Block for TXDATA REQUEST Output Event Generation.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ev_txdata_req <= 1'b0;
        else if( !cap_in_enable )
            cap_out_ev_txdata_req <= 1'b0;
        else if( cap_out_ev_txdata_req )
            cap_out_ev_txdata_req <= 1'b0;
        else if(( receive || txop || txop_send || txop_rx ) && 
                  cap_in_ev_txstart_confirm && !ctrl_start_count)
            cap_out_ev_txdata_req <= 1'b1;
    end

// Always Block for TXEND REQUEST Output Event Generation.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ev_txend_req <= 1'b0;
        else if( !cap_in_enable )
            cap_out_ev_txend_req <= 1'b0;
        else if( cap_out_ev_txend_req ) 
            cap_out_ev_txend_req <= 1'b0;
        else if(( receive || txop || txop_send || txop_rx ) && 
                  cap_in_ev_txdata_confirm )
            cap_out_ev_txend_req <= 1'b1;
    end

// Always Block for RXEND REQUEST Output Event Generation.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ev_rxend_ind <= 1'b0;
        else if( !cap_in_enable )
            cap_out_ev_rxend_ind <= 1'b0;
        else if( cap_out_ev_rxend_ind )
            cap_out_ev_rxend_ind <= 1'b0;
        else if( cap_in_ev_phyrx_error )
            cap_out_ev_rxend_ind <= 1'b1; 
    end

// Always Block for TXOP TXSTART REQUEST Output Event Generation.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ev_txop_txstart_req <= 1'b0;
        else if( !cap_in_enable || !cap_txframeinfo_valid )
            cap_out_ev_txop_txstart_req <= 1'b0;
        else if( cap_out_ev_txop_txstart_req )
            cap_out_ev_txop_txstart_req <= 1'b0;
        else if( txop && cap_txframeinfo_valid && !cap_txop_start_r )
            cap_out_ev_txop_txstart_req <= 1'b1;
        
	  else if( cap_txframeinfo_valid && ( txop || txop_rx ) && 
               ( cap_txop_return_r == SM_START_SENDING ) && cap_qos_mode_r )
            cap_out_ev_txop_txstart_req <= 1'b1;
        /*else if(( receive || txop_rx ) && cap_txframeinfo_valid && !cap_txop_start_r &&
                ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_SEND_RESP )) 
            cap_out_ev_txop_txstart_req <= 1'b1; */
    end
// Always Block for generating valid signal to generate ack frame.
 /*     always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ctrl_start_req <= 1'b0;
        else if( !cap_in_enable )
           cap_out_ctrl_start_req  <= 1'b0;
        else if( cap_out_ctrl_start_req )
            cap_out_ctrl_start_req <= 1'b0;
        else if( receive && ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_SEND_RESP )) 
            cap_out_ctrl_start_req <= 1'b1;
    end */
 always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )begin
            cap_out_ctrl_start_req <= 1'b0;
            ctrl_start_count <= 0;
          end
        else if( !cap_in_enable )begin
           cap_out_ctrl_start_req  <= 1'b0;
           ctrl_start_count <= 0;
           end
        else if( cap_out_ctrl_start_req )
            cap_out_ctrl_start_req <= 1'b0;
        else if (!receive)
            ctrl_start_count <= 1'b0;
        else if( receive && ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_SEND_RESP ) && ctrl_start_count == 0)begin
            cap_out_ctrl_start_req <= 1'b1;
            ctrl_start_count <= 1'b1; 
          end
    end
   always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ctrl_data_req <= 1'b0;
        else if( !cap_in_enable )
            cap_out_ctrl_data_req <= 1'b0;
        else if( cap_out_ctrl_data_req )
            cap_out_ctrl_data_req <= 1'b0;
        else if( receive && cap_in_ev_txstart_confirm )
            cap_out_ctrl_data_req <= 1'b1;
    end

// Always Block for Send More Data Pulse Generation.
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_ev_txsendmore_data_req <= 1'b0;
        else if( !cap_in_enable || !cap_txframeinfo_valid )
            cap_out_ev_txsendmore_data_req <= 1'b0;
        else if( cap_out_ev_txsendmore_data_req )
            cap_out_ev_txsendmore_data_req <= 1'b0;
        else if( cap_txframeinfo_valid && ( txop || txop_rx ) && 
               ( cap_txop_return_r == SM_START_SENDING ) && cap_qos_mode_r )
            cap_out_ev_txsendmore_data_req <= 1'b1;
    end

// Always Block to Generate CP Wait for CTS Pulse
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_cp_wait_for_cts <= 1'b0;
        else if( !cap_in_enable )
            cap_out_cp_wait_for_cts <= 1'b0;
        else if( txop_rx && cap_txframeinfo_valid && cap_in_ev_rxend_ind &&
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_CTS_RCVD ))
            cap_out_cp_wait_for_cts <= 1'b0;
        else if( txop_rx && cap_in_ev_timer_tick && cap_qos_mode_r &&
               ( cap_ack_timer_r > 5'h1 ) && 
               ( cap_cts_timer_r == 5'h1 ) &&
                 !cap_txprog && cap_txframeinfo_valid )
            cap_out_cp_wait_for_cts <= 1'b0;
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm && 
                  rts_frame && !( cap_ra_addr_r[0] && !cap_frame_fc_r[8] ) && 
               !( cap_qos_mode_r && ( cap_frame_fc_r[7:0] == 8'h88 ) && 
                ( cap_frame_ack_policy_r != 2'h0 )))
            cap_out_cp_wait_for_cts <= 1'b1;
    end

// Always Block to Generate CP Wait for ACK Pulse
    always @( posedge cap_in_clk or `EDGE_OF_RESET )
    begin
        if( `POLARITY_OF_RESET )
            cap_out_cp_wait_for_ack <= 1'b0;
        else if( !cap_in_enable )
            cap_out_cp_wait_for_ack <= 1'b0;
        else if( txop_rx && cap_txframeinfo_valid && cap_in_ev_rxend_ind &&
              (( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD ) ||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_ACK_RCVD )||
               ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS ))) 
            cap_out_cp_wait_for_ack <= 1'b0;
        else if( txop_rx && cap_in_ev_timer_tick && cap_qos_mode_r &&
               ( cap_ack_timer_r == 5'h1 ) && 
                 !cap_txprog && cap_txframeinfo_valid )
            cap_out_cp_wait_for_ack <= 1'b0;
        else if(( txop || txop_send ) && cap_in_ev_txend_confirm && 
                  !rts_frame && !( cap_ra_addr_r[0] && !cap_frame_fc_r[8] ) && 
               !( cap_qos_mode_r && ( cap_frame_fc_r[7:0] == 8'h88 ) && 
                ( cap_frame_ack_policy_r != 2'h0 )))
            cap_out_cp_wait_for_ack <= 1'b1;
    end


//**************************************************************************//
//------------------------- Combinational Logic ----------------------------//
//**************************************************************************//

    always @( * )
    begin
        if( cap_in_enable )
        begin
            next_state = curr_state;
            case( curr_state ) // Start of Case
            IDLE_STATE: 
            begin
                if( cap_in_cca_status && cap_in_ev_phyenergy_ind &&
                  ( cap_in_channel_list == `UU_WLAN_BUSY_CH_PRIMARY ))
                    next_state = RECEIVE_STATE; 
                else if( cap_in_ev_txready )
                    next_state = CONTEND_STATE;
                else
                    next_state = IDLE_STATE;
            end
            RECEIVE_STATE: 
            begin
                if( cap_in_ev_timer_tick || cap_in_ev_txready || cap_in_ev_rxstart_ind ||
                    cap_in_ev_txstart_confirm || cap_in_ev_txdata_confirm ||
                    cap_in_ev_txend_confirm || cap_in_ev_rxdata_ind )
                    next_state = RECEIVE_STATE; 
                else if(( cap_in_cca_status == 2'h0 ) && cap_in_ev_phyenergy_ind &&
                        ( cap_nav_val_r == 16'h0 ))
                begin
                    if( cap_txready )
                        next_state = CONTEND_STATE;
                    else
                        next_state = IDLE_STATE; 
                end 
                else if( cap_in_ev_phyrx_error )
                    next_state = EIFS_STATE;
                else if( cap_in_ev_ccareset_confirm ) 
                begin
                    if( cap_txready )
                        next_state = CONTEND_STATE;
                    else
                        next_state = IDLE_STATE; 
                end
                else if( cap_in_ev_rxend_ind )
                begin
                    if( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_FRAME_ERROR )
                        next_state = EIFS_STATE;
                    else if(( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_NAV_UPDATE ) ||
                            ( cap_in_rxhandler_ret_value == `UU_WLAN_RX_HANDLER_SEND_RESP ))
                        next_state = RECEIVE_STATE;
                    else 
                    begin
                        if( cap_txready )
                            next_state = CONTEND_STATE;
                        else
                            next_state = IDLE_STATE; 
                    end
                end
                else
                    next_state = RECEIVE_STATE; 
            end
            CONTEND_STATE: 
            begin
                if( cap_in_cca_status && cap_in_ev_phyenergy_ind &&
                  ( cap_in_channel_list == `UU_WLAN_BUSY_CH_PRIMARY ))
                    next_state = RECEIVE_STATE; 
                else if( cap_in_ev_txready || 
                 (((( cap_ac0_aifs_r > 0 ) && ( cap_out_txop_owner_ac == 3'h7 )) || 
                   (( cap_ac0_backoff_r > 0 ) && ( cap_out_txop_owner_ac == 3'h0 ))) &&
                      cap_ac0_txready_r ) ||
                 (((( cap_ac1_aifs_r > 0 ) && ( cap_out_txop_owner_ac == 3'h7 )) ||
                   (( cap_ac1_backoff_r > 0 ) && ( cap_out_txop_owner_ac == 3'h1 ))) &&
                      cap_ac1_txready_r && cap_qos_mode_r ) ||
                 (((( cap_ac2_aifs_r > 0 ) && ( cap_out_txop_owner_ac == 3'h7 )) || 
                   (( cap_ac2_backoff_r > 0 ) && ( cap_out_txop_owner_ac == 3'h2 ))) &&
                      cap_ac2_txready_r && cap_qos_mode_r ) ||
                 (((( cap_ac3_aifs_r > 0 ) && ( cap_out_txop_owner_ac == 3'h7 )) ||
                   (( cap_ac3_backoff_r > 0 ) && ( cap_out_txop_owner_ac == 3'h3 ))) &&
                      cap_ac3_txready_r && cap_qos_mode_r ) || ( cap_slot_timer_r > 0 )) 
                    next_state = CONTEND_STATE;
                else if((( cap_out_txop_owner_ac == 3'h0 ) && cap_ac0_txready_r ) ||
                        (( cap_out_txop_owner_ac == 3'h1 ) && cap_ac1_txready_r && cap_qos_mode_r ) ||
                        (( cap_out_txop_owner_ac == 3'h2 ) && cap_ac2_txready_r && cap_qos_mode_r ) ||
                        (( cap_out_txop_owner_ac == 3'h3 ) && cap_ac3_txready_r && cap_qos_mode_r ))
                    next_state = TXOP_STATE;
                else if((( !cap_in_ac0_txframeinfo_valid && !cap_ac0_txready_r )) &&                       
                        (( !cap_in_ac1_txframeinfo_valid && !cap_ac1_txready_r )) &&
                        (( !cap_in_ac2_txframeinfo_valid && !cap_ac2_txready_r )) &&
                        (( !cap_in_ac3_txframeinfo_valid && !cap_ac3_txready_r )))
                    next_state = IDLE_STATE; 
                else
                    next_state = CONTEND_STATE; 
            end   
            TXOP_STATE: 
            begin
                if( cap_txop_return_r == SM_TXOP_ERROR_RECOVERY ) 
                begin
                    if( cap_qos_mode_r )
                        next_state = TXOP_WAIT_TXNAVEND_STATE;
                    else
                        next_state = EIFS_STATE; 
                end
                else if( cap_txop_return_r == SM_TX_FAILURE )
                begin
                    if( cap_qos_mode_r )
                        next_state = TXOP_WAIT_TXNAVEND_STATE;
                    else if( cap_txready )
                        next_state = CONTEND_STATE;
                    else
                        next_state = IDLE_STATE; 
                end
                else if( cap_txop_return_r == SM_TXOP_NAV_UPDATE )
                begin
                    if( cap_qos_mode_r )
                        next_state = TXOP_WAIT_TXNAVEND_STATE;
                    else
                        next_state = RECEIVE_STATE;
                end
                else if(( cap_txop_return_r == SM_CONTEND ) ||
                        ( cap_txop_return_r == SM_TXOP_LIMIT_COMP ) ||
                        ( cap_txop_return_r == SM_TX_END )) 
                begin
                    if( cap_txready )
                        next_state = CONTEND_STATE;
                    else
                        next_state = IDLE_STATE; 
                end
                else if( cap_txop_return_r == SM_TX_NAV_ENDED )
                begin
                    if( cap_eifs_r == cap_in_eifs_value )
                        next_state = EIFS_STATE;
                    else if( cap_nav_val_r > 16'h0 )
                        next_state = RECEIVE_STATE;
                    else
                        next_state = CONTEND_STATE; 
                end
                else if( cap_txop_return_r == SM_RX_VALID_FRAME )
                    next_state = CONTEND_STATE;
                else if( cap_txop_return_r == SM_START_SENDING )
                begin
                    if( cap_qos_mode_r )  
                        next_state = TXOP_SEND_STATE;
                    else
                        next_state = CONTEND_STATE;
                end
                else if( cap_txop_return_r == SM_WAIT_FOR_RX )
                    next_state = TXOP_WAIT_RX_STATE;
                else if( cap_txop_return_r == SM_STATE_CONTINUE )
                begin
                    if(((( cap_out_txop_owner_ac == 3'h0 ) && !cap_ac0_txready_r ) ||
                        (( cap_out_txop_owner_ac == 3'h1 ) && !cap_ac1_txready_r ) ||
                        (( cap_out_txop_owner_ac == 3'h2 ) && !cap_ac2_txready_r ) ||
                        (( cap_out_txop_owner_ac == 3'h3 ) && !cap_ac3_txready_r )) &&
                           cap_in_ev_txend_confirm )
                        next_state = IDLE_STATE;
                    else
                        next_state = TXOP_STATE;
                end
                else
                    next_state = TXOP_STATE; 
            end
            TXOP_SEND_STATE: 
            begin
                if( cap_in_cca_status && cap_in_ev_phyenergy_ind &&
                  ( cap_in_channel_list == `UU_WLAN_BUSY_CH_PRIMARY ))
                    next_state = RECEIVE_STATE; 
                else if( cap_txop_return_r == SM_WAIT_FOR_RX )
                    next_state = TXOP_WAIT_RX_STATE;
                else if(( cap_txop_return_r == SM_STATE_CONTINUE ) ||
                        ( cap_txop_return_r == SM_START_SENDING ))
                    next_state = TXOP_SEND_STATE;
                else if(( cap_txop_return_r == SM_CONTEND ) ||
                        ( cap_txop_return_r == SM_TXOP_LIMIT_COMP ) ||
                        ( cap_txop_return_r == SM_TX_END ) ||
                        ( cap_txop_return_r == SM_RX_VALID_FRAME )) 
                begin
                    if( cap_txready )
                        next_state = CONTEND_STATE;
                    else
                        next_state = IDLE_STATE; 
                end
                else if( cap_txop_return_r == SM_TX_NAV_ENDED )
                begin
                    if( cap_eifs_r == cap_in_eifs_value )
                        next_state = EIFS_STATE;
                    else if( cap_nav_val_r > 16'h0 )
                        next_state = RECEIVE_STATE;
                    else
                        next_state = CONTEND_STATE; 
                end
                else if(( cap_txop_return_r == SM_TXOP_NAV_UPDATE ) ||
                        ( cap_txop_return_r == SM_TXOP_ERROR_RECOVERY ) ||
                        ( cap_txop_return_r == SM_TX_FAILURE)) 
                begin
                    if( cap_out_txop_owner_ac == cap_ac_priority )
                        next_state = TXOP_WAIT_TXNAVEND_STATE;
                    else
                        next_state = EIFS_STATE; 
                end
                else
                    next_state = TXOP_SEND_STATE; 
            end
            TXOP_WAIT_RX_STATE: 
            begin
                if( cap_txop_return_r == SM_STATE_CONTINUE )
                    next_state = TXOP_WAIT_RX_STATE;
                else if( cap_txop_return_r == SM_START_SENDING )
                begin
                    if( cap_qos_mode_r )  
                        next_state = TXOP_SEND_STATE; 
                    else
                        next_state = CONTEND_STATE;
                end
                else if( cap_txop_return_r == SM_TX_NAV_ENDED )
                begin
                    if( cap_eifs_r == cap_in_eifs_value )
                        next_state = EIFS_STATE;
                    else if( cap_nav_val_r > 16'h0 )
                        next_state = RECEIVE_STATE;
                    else
                        next_state = CONTEND_STATE; 
                end
                else if(( cap_txop_return_r == SM_CONTEND ) ||
                        ( cap_txop_return_r == SM_TXOP_LIMIT_COMP ) ||
                        ( cap_txop_return_r == SM_TX_END ))
                begin
                    if( cap_txready )
                        next_state = CONTEND_STATE;
                    else
                        next_state = IDLE_STATE; 
                end
                else if( cap_txop_return_r == SM_RX_VALID_FRAME )
                    next_state = RECEIVE_STATE;     
                else if( cap_txop_return_r == SM_TX_FAILURE )
                begin
                     if(( cap_eifs_r <= cap_in_eifs_value ) && cap_pkt_error_r )
                        next_state = EIFS_STATE;
                    else
                        next_state = TXOP_WAIT_TXNAVEND_STATE;   
                end
                else if( cap_txop_return_r == SM_TXOP_ERROR_RECOVERY )
                begin
                    if( cap_qos_mode_r )
                        next_state = TXOP_WAIT_TXNAVEND_STATE;
                    else
                        next_state = EIFS_STATE; 
                end
                else if( cap_txop_return_r == SM_TXOP_NAV_UPDATE )
                    next_state = RECEIVE_STATE; 
                else  
                    next_state = TXOP_WAIT_RX_STATE; 
            end
            TXOP_WAIT_TXNAVEND_STATE: 
            begin        
                if( cap_txop_return_r == SM_STATE_CONTINUE )
                    next_state = TXOP_WAIT_TXNAVEND_STATE;
                else if( cap_txop_return_r == SM_TX_NAV_ENDED )
                begin
                    if(( cap_eifs_r <= cap_in_eifs_value ) && cap_pkt_error_r )
                        next_state = EIFS_STATE;
                    else if( cap_nav_val_r > 16'h0 )
                        next_state = RECEIVE_STATE;
                    else
                        next_state = CONTEND_STATE; 
                end
                else 
                    next_state = TXOP_WAIT_TXNAVEND_STATE; 
            end
            EIFS_STATE: 
            begin
                if( cap_in_cca_status && cap_in_ev_phyenergy_ind &&
                  ( cap_in_channel_list == `UU_WLAN_BUSY_CH_PRIMARY ))
                    next_state = RECEIVE_STATE; 
                else if(( cap_eifs_r <= 6'h1 ) && cap_in_ev_timer_tick )
                begin
                    if( cap_nav_val_r > 16'h0 )
                        next_state = RECEIVE_STATE;
                    else 
                    begin
                        if( cap_txready )
                            next_state = CONTEND_STATE;
                        else
                            next_state = IDLE_STATE;
                    end
                end
                else if( cap_in_ev_ccareset_confirm || cap_in_ev_txready )
                    next_state = EIFS_STATE;
                else
                    next_state = EIFS_STATE; 
            end
            default: next_state = IDLE_STATE;
            endcase // End of Case Block
        end // End of If
        else 
        begin
            next_state = IDLE_STATE;
        end
    end // End of Combinational Block


endmodule
