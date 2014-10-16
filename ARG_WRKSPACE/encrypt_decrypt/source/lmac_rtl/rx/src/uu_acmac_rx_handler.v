//This module implements the functionality of RX Handler in LMAC.

`include "../../inc/defines.h"

module uu_acmac_rx_handler 
                     #(
                        parameter WIDTH_WORD = 32,
                        parameter WIDTH_HALF_WORD = 16,
                        parameter WIDTH_BYTE = 8
                      )
                      (
                        input                             rx_clk,                         // Clock Input to module
                        input                             rst_n,                          // Reset Input to module
                        input                             rx_enable,                      // Enable Input to module 
                        input                             rx_in_ev_rxstart,               // RxStart Input Event
                        input                             rx_in_ev_rxdata,                // RxData Input Event
                        input                             rx_in_ev_rxend,                 // RxEnd Input Event
                        input      [WIDTH_BYTE-1:0]       rx_in_frame,                    // Frame Byte Input 
                        input                             rx_in_frame_valid,              // Frame Byte Valid Signal
                        input      [1:0]                  rx_in_rxend_status,             // Rx handler Frame End Status
                        input      [WIDTH_BYTE+1:0]       rx_in_lmac_filter_flag,         // LMAC Filter Flag Signal
                        input      [WIDTH_WORD-1:0]       rx_in_cp_return_value,          // Signal from CP return value
                        input                             rx_in_cp_return_valid,          // Signal from CP return value valid 
                        input                             rx_in_cap_frame_info_retry,     // added for reseting the rx base ptr in reaggregation case
            
                        output reg                        rx_out_frame_decrypt_enable,    // Enable Signal for Decrypt module
                        output reg                        rx_out_frame_post_to_umac,      // Frame post signal to UMAC
                        output reg                        rx_out_rxend_ind,               // End of frame Indication
                        output reg                        rx_out_phyrx_error_ind,         // Frame Error Indication from the PHY
                        output reg                        rx_out_cp_process_rx_frame,     // Pulse to Process of RX frame for CP
                        output reg                        rx_out_mem_en,                  // Output to Enable the Memory
                        output reg [15:0]                 rx_out_mem_addr,                // Address for writing to Memory
                        output reg [WIDTH_BYTE-1:0]       rx_out_frame,                   // Output Frame Byte 
                        output reg                        rx_out_frame_valid,             // Output Frame Byte Valid Signal
                        output reg [WIDTH_WORD-1:0]       rx_out_cap_return_value,        // Rx Handler return value to CAP
                        output reg                        rx_out_rx_ampdu_frames_cnt,     // Pusle to count the AMPDU Frames
                        output reg                        rx_out_rx_ampdu_subframes_cnt,  // Pusle to count the AMPDU SubFrames
                        output reg                        rx_out_rx_phyerr_pkts_cnt,      // Pulse to count Error Packets from PHY				
                        output reg                        rx_out_rx_frames_cnt,           // Pulse to count Rx Frames
                        output reg                        rx_out_rx_multicast_cnt,        // Pulse to count Multicast Packets
                        output reg                        rx_out_rx_broadcast_cnt,        // Pulse to count Broadcast Packets
                        output reg                        rx_out_rx_frame_forus_cnt,      // Pulse to count frames for us
                        output reg                        rx_out_dot11_fcs_error_cnt,     // Pulse to count FCS Error                        
                        output reg [WIDTH_HALF_WORD-1:0]  rx_out_frame_length,	//vibha
			//added newly for de_agggr pkt info
			output reg [WIDTH_HALF_WORD-1:0]  rx_out_cp_seq_no,  //to CP
			output reg                        rx_out_cp_is_aggr,  //to CP
				output reg                        rx_out_cap_ev_rxend_ind
                      );
reg [15:0]rx_addr_ptr_buff;//vibha
reg [15:0] rx_addr_ptr;//vibha
// Register Declarations
   reg                                                    rx_frame_vector_read_r;  // Register to toggle based on Rx Start to receive Rx Vector
   reg                                                    rx_frame_data_read_r;    // Register to toggle based on Rx Data to receive Data frame 
   reg                                                    rx_frame_end_read_r;     // Register to toggle based on Rx End to indicate End of frame
   reg                                                    rx_ampdu_sign_delay_r;   // Register to store the AMPDU Signature valid pulse
   reg                                                    rx_aggr_enable_r;        // Register to indicate Aggregated Frame from Rx Vector
   reg                                                    rx_umac_ready_r;         // Register to indicate UMAC ready signal
   reg                                                    rx_post_to_umac_r;       // Register to indicate posting to UMAC as valid     
   reg  [1:0]                                             rx_frame_ac_r;           // Frame AC category Indication
   reg  [1:0]                                             rx_frame_format_r;       // Register to Store Frame Format from Rx Vector
   reg  [2:0]                                             rx_fill_frame_return_r;  // Frame Return Type while filling Data Values
   reg  [2:0]                                             rx_frame_destination_r;  // Register to Store the Destination of Packet
   reg  [4:0]                                             rx_frame_info_count_r;   // Register to Store the number of Rx Vector Bytes Received
   reg  [WIDTH_HALF_WORD-1:0]                             rx_frame_control_r;      // Register to Store Frame Control Field from the Frame
   reg  [WIDTH_HALF_WORD-1:0]                             rx_l_length_r;           // Register to Store L_length from Rx Vector
   reg  [WIDTH_HALF_WORD-1:0]                             rx_frame_qos_r;          // Register to Store Qos Field from the Frame
   reg  [19:0]                                            rx_frame_len_r;          // Register to decrement the length for byte received 
   reg  [19:0]                                            rx_psdu_len_r;           // Register to Store PSDU Length from Rx Vector
   reg  [19:0]                                            rx_aggr_psdu_len_r;      // Register to Count PSDU Length during Aggregation
   reg  [19:0]                                            rx_frameinfo_len_r;      // Register Frame Length for Valid Frame
   reg  [WIDTH_WORD-1:0]                                  rx_frame_crc_r;          // Register to Store the CRC of the Frame Received 
   reg  [WIDTH_WORD+WIDTH_HALF_WORD-1:0]                  rx_mac_addr_r;           // Register to Store the MAC Address of the Frame
   reg  [WIDTH_WORD+WIDTH_HALF_WORD-1:0]                  rx_bssid_addr_r;         // Register to Store the BSSID of the Frame
   reg                                                    loc_mem_addr_inc;  
   reg [19:0]                                             loc_sub_ampdu_address;
   reg                                                    rx_deagg_data_valid_d;
   reg                                                    rx_deagg_data_valid_dly;
   reg                                                    rx_deagg_data_valid_dly1;
   reg                                                    loc_mem_addr_dly;
   
   // Newly added for Deaggregation Rx vectors for multiiple subampdu
   reg [15:0]                                             loc_frame_info_addr;
   reg [15:0]                                             loc_frame_addr;
   reg [15:0]                                             loc_frame1_addr;
   reg [15:0]                                             loc_frame2_addr;
   reg [15:0]                                             loc_frame3_addr;
   reg [15:0]                                             loc_frame4_addr;
   reg [15:0]                                             loc_frame5_addr;
   reg [15:0]                                             loc_frame6_addr;
   reg [15:0]                                             loc_frame7_addr;
   reg                                                    loc_subampdu_frame1;
   reg                                                    loc_subampdu_frame2;
   reg                                                    loc_subampdu_frame3;
   reg                                                    loc_subampdu_frame4;
   reg                                                    loc_subampdu_frame5;
   reg                                                    loc_subampdu_frame6;
   reg                                                    loc_subampdu_frame7;
   reg                                                    loc_subampdu_frame8;
   reg [2:0]                                              loc_subampdu_count;
   reg                                                    loc_vector_transmit;
   reg                                                    loc_vector_transmit_in_process;
   reg                                                    loc_vector_transmit_frame2;
   reg                                                    loc_vector_transmit_frame3;
   reg                                                    loc_vector_transmit_frame4;
   reg                                                    loc_vector_transmit_frame5;
   reg                                                    loc_vector_transmit_frame6;
   reg                                                    loc_vector_transmit_frame7;
   reg                                                    loc_vector_transmit_frame8;
   reg                                                    loc_vector_transmit_frame2_end;
   reg                                                    loc_vector_transmit_frame3_end;
   reg                                                    loc_vector_transmit_frame4_end;
   reg                                                    loc_vector_transmit_frame5_end;
   reg                                                    loc_vector_transmit_frame6_end;
   reg                                                    loc_vector_transmit_frame7_end;
   reg                                                    loc_vector_transmit_frame8_end;
   reg [5:0]                                              loc_vector_count;
   reg [3:0]                                              loc_vector_transmit_count;
   reg                                                    loc_rxend_indication;
   reg [7:0]                                              loc_rxvector_byte0;
   reg [7:0]                                              loc_rxvector_byte1;
   reg [7:0]                                              loc_rxvector_byte2;
   reg [7:0]                                              loc_rxvector_byte3;
   reg [7:0]                                              loc_rxvector_byte4;
   reg [7:0]                                              loc_rxvector_byte5;
   reg [7:0]                                              loc_rxvector_byte6;
   reg [7:0]                                              loc_rxvector_byte7;
   reg [7:0]                                              loc_rxvector_byte8;
   reg [7:0]                                              loc_rxvector_byte9;
   reg [7:0]                                              loc_rxvector_byte10;
   reg [7:0]                                              loc_rxvector_byte11;
   reg [7:0]                                              loc_rxvector_byte12;
   reg [7:0]                                              loc_rxvector_byte13;
   reg [7:0]                                              loc_rxvector_byte14;
   reg [7:0]                                              loc_rxvector_byte15;
   reg [7:0]                                              loc_rxvector_byte16;
   reg [7:0]                                              loc_rxvector_byte17;
   reg [7:0]                                              loc_rxvector_byte18;
   reg [7:0]                                              loc_rxvector_byte19; 
   
// Wire Declarations
   wire                                                   rx_deagg_crc8_invalid_w;
   wire                                                   rx_deagg_invalid_w;
   wire                                                   rx_deagg_sign_valid_w;
   wire                                                   rx_deagg_data_valid_w;
   wire [1:0]                                             rx_deagg_pad_bytes_w;
   wire [2:0]                                             rx_deagg_return_value_w;
   wire [WIDTH_HALF_WORD-1:0]                             rx_deagg_subframe_len_w;
   wire [WIDTH_HALF_WORD-1:0]                             rx_crc_in_len_w;
   wire                                                   fcs_valid_w;
   wire [WIDTH_WORD-1:0]                                  fcs_word_w;


   // Indication to Enable the Deaggregation Module 
   assign rx_deagg_enable_w = ( rx_in_ev_rxdata || rx_frame_data_read_r ) && rx_aggr_enable_r;
 
   // Indicating a Valid MPDU 
   assign rx_mpdu_valid_w = rx_in_frame_valid && !rx_aggr_enable_r && 
                          ( rx_in_ev_rxdata || rx_frame_data_read_r ) &&
                          ( rx_frame_len_r == ( rx_l_length_r - 16'h1 ));

   // Input Length to CRC Module
   assign rx_crc_in_len_w = rx_aggr_enable_r ? (rx_deagg_subframe_len_w - 'h4): ( rx_l_length_r - 16'h4 ) ; 

   // Enable Signal to the CRC Module
   assign rx_crc_enable_w = (( rx_deagg_data_valid_w && rx_aggr_enable_r ) || 
                            (( rx_in_ev_rxdata || rx_frame_data_read_r ) && !rx_aggr_enable_r ));

   // AMPDU Subframe End Indication
   assign rx_ampdu_subframe_end = ( rx_aggr_enable_r &&
       (( rx_frame_len_r - ( 20'h3 + rx_deagg_pad_bytes_w )) == (rx_deagg_subframe_len_w - 'h1) )) ? 1'b1 : 1'b0;
                                                                            
 reg phy_rxend_ind_r;
// Module Instances
// Deaggregation module
uu_acmac_deaggregation  U_DEAGGREGATION ( .deagg_clk                  ( rx_clk ),
                                          .rst_n                      ( rst_n ),
                                          .deagg_enable               ( rx_deagg_enable_w ),
                                          .deagg_in_data              ( rx_in_frame ),
                                          .deagg_in_data_valid        ( rx_in_frame_valid ),
                                          .deagg_in_frame_format      ( rx_frame_format_r ),
                                          .deagg_out_subframe_length  ( rx_deagg_subframe_len_w ),
                                          .deagg_out_crc8_invalid     ( rx_deagg_crc8_invalid_w ),
                                          .deagg_out_invalid_pulse    ( rx_deagg_invalid_w ),
                                          .deagg_out_pad_bytes        ( rx_deagg_pad_bytes_w ),
                                          .deagg_out_sign_valid       ( rx_deagg_sign_valid_w ),
                                          .deagg_out_data_valid       ( rx_deagg_data_valid_w ),
                                          .deagg_out_return_value     ( rx_deagg_return_value_w )
                                        );

// CRC 32 Bit Calculation Module
uu_acmac_crc32_8in U_CRC32 ( .clk                         ( rx_clk ),
                             .rst_n                       ( rst_n ), 
                             .crc_en                      ( rx_crc_enable_w ),
                             .data_val                    ( rx_in_frame_valid ),
                             .data                        ( rx_in_frame ),  
                             .len                         ( rx_crc_in_len_w ),
                             .crc_avl                     ( fcs_valid_w ),
                             .crc                         ( fcs_word_w )
                  );
 
//added for giving end indication to CAP
   always @(posedge rx_clk or `EDGE_OF_RESET )
   begin
	if( `POLARITY_OF_RESET )
		rx_out_cap_ev_rxend_ind <= 'd0;
	 else if( !rx_enable )
           rx_out_cap_ev_rxend_ind <= 'd0;
       else if(rx_in_cp_return_valid && phy_rxend_ind_r)
		rx_out_cap_ev_rxend_ind <= 'b1;
	else 
		rx_out_cap_ev_rxend_ind <= 'd0;
   end

   always @(posedge rx_clk or `EDGE_OF_RESET )
   begin
	if( `POLARITY_OF_RESET )
		phy_rxend_ind_r <= 'd0;
	 else if( !rx_enable )
           phy_rxend_ind_r <= 'd0;
       else if(rx_in_cp_return_valid)
		phy_rxend_ind_r <= 'b0;
	else if(rx_in_ev_rxend)
		phy_rxend_ind_r <= 'd1;
   end


////////////////////////////////////////////////////////////////
// Frame Statistics Pulses to Increment The External Counters //
////////////////////////////////////////////////////////////////
//Added for De-aggr frame info for CP
   always @(posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_cp_seq_no <= 'd0;
       else if( !rx_enable )
           rx_out_cp_seq_no <= 'd0;
       else if(rx_frame_len_r == 'd26)
           rx_out_cp_seq_no <= rx_in_frame ;
       else if(rx_frame_len_r == 'd27)
           rx_out_cp_seq_no <= {rx_in_frame,rx_out_cp_seq_no[7:0]} ;
   end
   
   always @(posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_cp_is_aggr <= 'b0;
       else if( !rx_enable )
           rx_out_cp_is_aggr <= 'b0;
       else if(rx_frame_len_r == 'd6)
           rx_out_cp_is_aggr <= rx_in_frame[2];
   end

// Always Block for the AMPDU Frame Count Increment Pulse.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_rx_ampdu_frames_cnt <= 1'b0;
       else if( !rx_enable || rx_out_rx_ampdu_frames_cnt )
           rx_out_rx_ampdu_frames_cnt <= 1'b0;
       else if( rx_enable && ( rx_frame_info_count_r == 5'h13 ) && 
                rx_aggr_enable_r && rx_frame_vector_read_r && 
                rx_in_frame_valid )
           rx_out_rx_ampdu_frames_cnt <= 1'b1;
   end 

// Always Block for the AMPDU Subframe Count Increment Pulse
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_rx_ampdu_subframes_cnt <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_rx_ampdu_subframes_cnt )
           rx_out_rx_ampdu_subframes_cnt <= 1'b0;
       else if( rx_enable && ( rx_frame_data_read_r || rx_in_ev_rxend ) &&
              ( rx_aggr_psdu_len_r > 20'h0 ) && rx_aggr_enable_r && 
              ( rx_fill_frame_return_r == `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME ))
           rx_out_rx_ampdu_subframes_cnt <= 1'b1;
   end

// Always Block for the PHY Error Packets Count Increment Pulse
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_rx_phyerr_pkts_cnt <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_rx_phyerr_pkts_cnt )
           rx_out_rx_phyerr_pkts_cnt <= 1'b0;
       else if( rx_enable && ( rx_in_ev_rxend || rx_frame_end_read_r ) && 
              ( rx_in_rxend_status != `UU_WLAN_RX_END_STAT_NO_ERROR ))
           rx_out_rx_phyerr_pkts_cnt <= 1'b1;
   end

// Always Block for the Rx Frames Count Increment Pulse
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_rx_frames_cnt <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_rx_frames_cnt ) 
           rx_out_rx_frames_cnt <= 1'b0;
       else if( rx_enable && ( rx_in_ev_rxend || rx_frame_end_read_r ) &&
              ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ))
           rx_out_rx_frames_cnt <= 1'b1;
   end   
         
// Always Block for Multicast Packet Frame Count Increment Pulse
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_rx_multicast_cnt <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_rx_multicast_cnt )
           rx_out_rx_multicast_cnt <= 1'b0;
       else if( rx_enable && ( rx_mac_addr_r [0] == 1'b1 ) &&
              ( rx_in_ev_rxend || rx_frame_end_read_r ) && 
              ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ))
           rx_out_rx_multicast_cnt <= 1'b1;
   end

// Always Block for Broadcast Packet Frame Count Increment Pulse
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_rx_broadcast_cnt <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_rx_broadcast_cnt )
           rx_out_rx_broadcast_cnt <= 1'b0;
       else if( rx_enable && ( &rx_mac_addr_r ) &&
              ( rx_in_ev_rxend || rx_frame_end_read_r ) &&
              ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ))
           rx_out_rx_broadcast_cnt <= 1'b1;
   end

// Always Block for Frames Intended for Us Count Increment Pulse
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_rx_frame_forus_cnt <= 1'b0; 
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_rx_frame_forus_cnt )
           rx_out_rx_frame_forus_cnt <= 1'b0; 
       else if( rx_enable && ( rx_in_ev_rxend || rx_frame_end_read_r ) &&
              ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ) &&
              ( rx_mac_addr_r == `UU_WLAN_IEEE80211_STA_MAC_ADDR_R ))
           rx_out_rx_frame_forus_cnt <= 1'b1; 
   end

// Always Block for FCS Error Count Increment Pulse
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_dot11_fcs_error_cnt <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_dot11_fcs_error_cnt )
           rx_out_dot11_fcs_error_cnt <= 1'b0;
       else if( rx_enable && ( !rx_post_to_umac_r ) && 
             (( rx_aggr_enable_r && ( rx_aggr_psdu_len_r > 20'h0 ) &&
              ( rx_fill_frame_return_r == `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME )) || 
              ( !rx_aggr_enable_r && 
              ( rx_fill_frame_return_r == `UU_LMAC_RX_RET_NON_AGG_FRAME))) &&
              ( fcs_valid_w && ( rx_frame_crc_r != fcs_word_w )))
           rx_out_dot11_fcs_error_cnt <= 1'b1;
   end

/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

   always @(posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_deagg_data_valid_d <= 'b0;
       else if( !rx_enable )
           rx_deagg_data_valid_d <= 'b0;
       else 
           rx_deagg_data_valid_d <= rx_deagg_data_valid_w;
   end
   
   always @(posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_deagg_data_valid_dly <= 'b0;
       else if( !rx_enable )
           rx_deagg_data_valid_dly <= 'b0;
       else 
           rx_deagg_data_valid_dly <= rx_deagg_data_valid_d;
   end
   
   always @(posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_deagg_data_valid_dly1 <= 'b0;
       else if( !rx_enable )
           rx_deagg_data_valid_dly1 <= 'b0;
       else 
           rx_deagg_data_valid_dly1 <= rx_deagg_data_valid_dly;
   end

// Always Block to Generate Output Data and Data Valid.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET ) 
       begin
           rx_out_frame <= 8'h0;
           rx_out_frame_valid <= 1'b0;
       end
       else if( !rx_enable )
       begin
           rx_out_frame <= 8'h0;
           rx_out_frame_valid <= 1'b0;
       end
       else if( rx_enable && rx_in_frame_valid && !rx_aggr_enable_r &&  
              ( rx_in_ev_rxdata || rx_frame_data_read_r ))  // Non Aggregated Data
       begin
           if( rx_frame_len_r < ( rx_l_length_r - 16'h4 )) // Data from the Frame
           begin
               rx_out_frame <= rx_in_frame;
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else if( rx_frame_len_r == ( rx_l_length_r - 16'h4 )) // Length 1st Byte
           begin
               rx_out_frame <= rx_l_length_r[7:0];
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else if( rx_frame_len_r == ( rx_l_length_r - 16'h3 )) // Length 2nd Byte
           begin
               rx_out_frame <= { 4'h0, rx_l_length_r[11:8] };
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else if( rx_frame_len_r == ( rx_l_length_r - 16'h2 ))  // Null Data in last 3rd Byte
           begin
               rx_out_frame <= 8'h0;
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else if( rx_frame_len_r == ( rx_l_length_r - 16'h1 )) // Null Data in last 2nd  Byte
           begin
               rx_out_frame <= 8'h0;
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else // Access Category Information for last Byte from the Rx Vector
           begin
               rx_out_frame <= { 6'h0, rx_frame_ac_r };
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
       end
       else if( rx_enable && (rx_deagg_data_valid_w || rx_deagg_data_valid_d || rx_deagg_data_valid_dly || rx_deagg_data_valid_dly1) && 
                rx_aggr_enable_r ) // Aggregated Data
       begin
           if( rx_frame_len_r < rx_deagg_subframe_len_w ) // Aggregated Subframe Data
           begin
               rx_out_frame <= rx_in_frame;
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else if( rx_frame_len_r == rx_deagg_subframe_len_w ) // Aggregated Subframe Length 1st byte
           begin
               rx_out_frame <= rx_deagg_subframe_len_w[7:0];
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else if(( rx_frame_len_r - 20'h1 ) == rx_deagg_subframe_len_w ) // Aggregated Subframe Length 2nd Byte
           begin
               rx_out_frame <= rx_deagg_subframe_len_w[15:8];
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else if(( rx_frame_len_r - 20'h2 ) == rx_deagg_subframe_len_w ) // Null Data for 3rd Byte
           begin
               rx_out_frame <= 8'h0;
               rx_out_frame_valid <= rx_in_frame_valid;  
           end
           else if(( rx_frame_len_r - 20'h3 ) == rx_deagg_subframe_len_w ) // Data for 4th Byte Indicating AMPDU
           begin
               rx_out_frame <= 8'h1;
               rx_out_frame_valid <= rx_in_frame_valid;  
           end    
           else if((( rx_in_ev_rxend && ( rx_in_rxend_status != `UU_WLAN_RX_END_STAT_NO_ERROR )) ||
                    ( rx_out_cap_return_value == `UU_WLAN_RX_HANDLER_FRAME_ERROR )) && rx_aggr_enable_r )
           begin // Passing the Error Indication during Rx End
               rx_out_frame <= 8'h0C;
               rx_out_frame_valid <= rx_in_frame_valid;  
           end      
           else 
           begin // Passing the AC from the Rx Vector
               rx_out_frame <= { 6'h0, rx_frame_ac_r };
               rx_out_frame_valid <= 1'b1;  
           end
       end
       else if(( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata ) // Rx Vector to the Output
       begin
           rx_out_frame <= rx_in_frame;
           rx_out_frame_valid <= rx_in_frame_valid;  
       end
       else if((loc_vector_transmit_frame2 || loc_vector_transmit_frame3 || loc_vector_transmit_frame4 || loc_vector_transmit_frame5 || loc_vector_transmit_frame6 || loc_vector_transmit_frame7 || loc_vector_transmit_frame8  ) &&  ((loc_vector_count >= 6'd0)&& (loc_vector_count <= 6'd19))) // Deaggregation Rx Vector to the Output
           begin
            case (loc_vector_count)
           6'd0: begin 
                 rx_out_frame <= loc_rxvector_byte0;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd1: begin 
                 rx_out_frame <= loc_rxvector_byte1;
                 rx_out_frame_valid <= 1'b1;
                end
           6'd2: begin 
                 rx_out_frame <= loc_rxvector_byte2;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd3: begin 
                 rx_out_frame <= loc_rxvector_byte3;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd4: begin 
                 rx_out_frame <= loc_rxvector_byte4;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd5: begin 
                 rx_out_frame <= loc_rxvector_byte5;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd6: begin 
                 rx_out_frame <= loc_rxvector_byte6;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd7: begin 
                 rx_out_frame <= loc_rxvector_byte7;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd8: begin 
                 rx_out_frame <= loc_rxvector_byte8;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd9: begin 
                 rx_out_frame <= loc_rxvector_byte9;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd10: begin 
                 rx_out_frame <= loc_rxvector_byte10;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd11: begin 
                 rx_out_frame <= loc_rxvector_byte11;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd12: begin 
                 rx_out_frame <= loc_rxvector_byte12;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd13: begin 
                 rx_out_frame <= loc_rxvector_byte13;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd14: begin 
                 rx_out_frame <= loc_rxvector_byte14;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd15: begin 
                 rx_out_frame <= loc_rxvector_byte15;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd16: begin 
                 rx_out_frame <= loc_rxvector_byte16;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd17: begin 
                 rx_out_frame <= loc_rxvector_byte17;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd18: begin 
                 rx_out_frame <= loc_rxvector_byte18;
                 rx_out_frame_valid <= 1'b1;
                 end
           6'd19: begin 
                 rx_out_frame <= loc_rxvector_byte19;
                 rx_out_frame_valid <= 1'b1;
                 end  
            endcase 
       end   
       else // NULL Data when no input is available
       begin
           rx_out_frame <= 8'h0;
           rx_out_frame_valid <= 1'b0;
       end   
   end
   
// Always block for Rx vector byte0
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte0 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte0 <= 8'h0;
       else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte0 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd0)
           loc_rxvector_byte0 <= rx_in_frame;
        end
       else
         loc_rxvector_byte0 <= loc_rxvector_byte0;
   end
   
// Always block for Rx vector byte1
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte1 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte1 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte1 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd1)
           loc_rxvector_byte1 <= rx_in_frame;
        end
       else
         loc_rxvector_byte1 <= loc_rxvector_byte1;
   end

// Always block for Rx vector byte2
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte2 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte2 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte2 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd2)
           loc_rxvector_byte2 <= rx_in_frame;
        end
       else
         loc_rxvector_byte2 <= loc_rxvector_byte2;
   end
   
// Always block for Rx vector byte3
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte3 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte3 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte3 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd3)
           loc_rxvector_byte3 <= rx_in_frame;
        end
       else
         loc_rxvector_byte3 <= loc_rxvector_byte3;
   end
   
// Always block for Rx vector byte4
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte4 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte4 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte4 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd4)
           loc_rxvector_byte4 <= rx_in_frame;
        end
       else
         loc_rxvector_byte4 <= loc_rxvector_byte4;
   end
   
// Always block for Rx vector byte5
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte5 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte5 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte5 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd5)
           loc_rxvector_byte5 <= rx_in_frame;
        end
       else
         loc_rxvector_byte5 <= loc_rxvector_byte5;
   end
   
// Always block for Rx vector byte6
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte6 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte6 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte6 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd6)
           loc_rxvector_byte6 <= rx_in_frame;
        end
       else
         loc_rxvector_byte6 <= loc_rxvector_byte6;
   end
   
// Always block for Rx vector byte7
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte7 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte7 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte7 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd7)
           loc_rxvector_byte7 <= rx_in_frame;
        end
       else
         loc_rxvector_byte7 <= loc_rxvector_byte7;
   end
   
// Always block for Rx vector byte8
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte8 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte8 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte8 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd8)
           loc_rxvector_byte8 <= rx_in_frame;
        end
       else
         loc_rxvector_byte8 <= loc_rxvector_byte8;
   end
   
// Always block for Rx vector byte9
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte9 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte9 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte9 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd9)
           loc_rxvector_byte9 <= rx_in_frame;
        end
       else
         loc_rxvector_byte9 <= loc_rxvector_byte9;
   end
   
   // Always block for Rx vector byte10
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte10 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte10 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte10 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd10)
           loc_rxvector_byte10 <= rx_in_frame;
        end
       else
         loc_rxvector_byte10 <= loc_rxvector_byte10;
   end
   
   // Always block for Rx vector byte11
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte11 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte11 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte11 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd11)
           loc_rxvector_byte11 <= rx_in_frame;
        end
       else
         loc_rxvector_byte11 <= loc_rxvector_byte11;
   end
   
   // Always block for Rx vector byte12
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte12 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte12 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte12 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd12)
           loc_rxvector_byte12 <= rx_in_frame;
        end
       else
         loc_rxvector_byte12 <= loc_rxvector_byte12;
   end
   
   // Always block for Rx vector byte13
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte13 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte13 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte13 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd13)
           loc_rxvector_byte13 <= rx_in_frame;
        end
       else
         loc_rxvector_byte13 <= loc_rxvector_byte13;
   end
   
   // Always block for Rx vector byte14
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte14 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte14 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte14 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd14)
           loc_rxvector_byte14 <= rx_in_frame;
        end
       else
         loc_rxvector_byte14 <= loc_rxvector_byte14;
   end
   
   // Always block for Rx vector byte15
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte15 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte15 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte15 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd15)
           loc_rxvector_byte15 <= rx_in_frame;
        end
       else
         loc_rxvector_byte15 <= loc_rxvector_byte15;
   end
   
   // Always block for Rx vector byte16
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte16 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte16 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte16 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd16)
           loc_rxvector_byte16 <= rx_in_frame;
        end
       else
         loc_rxvector_byte16 <= loc_rxvector_byte16;
   end
   
   // Always block for Rx vector byte17
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte17 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte17 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte17 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd17)
           loc_rxvector_byte17 <= rx_in_frame;
        end
       else
         loc_rxvector_byte17 <= loc_rxvector_byte17;
   end
   
   // Always block for Rx vector byte18
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte18 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte18 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte18 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd18)
           loc_rxvector_byte18 <= rx_in_frame;
        end
       else
         loc_rxvector_byte18 <= loc_rxvector_byte18;
   end
   
   // Always block for Rx vector byte19
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           loc_rxvector_byte19 <= 8'h0;
       else if( !rx_enable )
           loc_rxvector_byte19 <= 8'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_rxvector_byte19 <= 8'h0;
       else if ((( rx_in_ev_rxstart || rx_frame_vector_read_r ) && !rx_in_ev_rxdata )) begin
          if(rx_frame_info_count_r == 'd19)
           loc_rxvector_byte19 <= rx_in_frame;
        end
       else
         loc_rxvector_byte19 <= loc_rxvector_byte19;
   end
   
        
// Always Block to Enable the Memory
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_mem_en <= 1'b0;
       else if( !rx_enable )
           rx_out_mem_en <= 1'b0;
       else
           rx_out_mem_en <= 1'b1;
   end
   
// Always Block to Enable the Memory
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_mem_addr_inc <= 1'b0;
       else if( !rx_enable )
            loc_mem_addr_inc <= 1'b0;
       else if (!rx_aggr_enable_r && rx_frame_vector_read_r && rx_in_frame_valid)
          loc_mem_addr_inc  <= 1'b1;
        else 
          loc_mem_addr_inc <= 1'b0;
   end

// Always Block to Enable the Memory
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_sub_ampdu_address <= 20'b0;
       else if( !rx_enable || rx_in_ev_rxend)
            loc_sub_ampdu_address <= 20'b0;
      else if ( rx_out_rxend_ind)
          loc_sub_ampdu_address  <= 'h0;
       else if ( (rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0))
          loc_sub_ampdu_address  <= rx_out_mem_addr + 20'd40;
   end
    

   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_mem_addr_dly <= 1'b0;
       else if( !rx_enable )
            loc_mem_addr_dly <= 1'b0;
       else if (rx_out_rx_ampdu_subframes_cnt)
          loc_mem_addr_dly  <= 1'b1;
      else if (rx_out_mem_addr == loc_sub_ampdu_address)
          loc_mem_addr_dly  <= 1'b0;
    end
    
 // Always block for frame info offset
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame_addr <= 16'h0;
       else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_frame_addr <= 16'h0; 
       else if ((rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0))
          loc_frame_addr  <= rx_out_mem_addr;
      else 
          loc_frame_addr  <= loc_frame_addr;
    end 
    
 // Always block for frame info offset
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame1_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame1_addr <= 16'h0;
        else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_frame1_addr <= 16'h0; 
       else if ((rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0) && loc_subampdu_frame1 && (loc_frame1_addr == 20'h0))
          loc_frame1_addr  <= loc_frame_addr + 'h4;
      else 
          loc_frame1_addr  <= loc_frame1_addr;
    end 
    
 // Always block for frame info offset
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame2_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame2_addr <= 16'h0;
       else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_frame2_addr <= 16'h0; 
       else if ((rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0) && loc_subampdu_frame2 && (loc_frame2_addr == 20'h0))
          loc_frame2_addr  <= loc_frame_addr + 'h4;
      else 
          loc_frame2_addr  <= loc_frame2_addr;
    end 
    
 // Always block for frame info offset
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame3_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame3_addr <= 16'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_frame3_addr <= 16'h0; 
       else if ((rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0) && loc_subampdu_frame3 && (loc_frame3_addr == 20'h0))
          loc_frame3_addr  <= loc_frame_addr + 'h4;
      else 
          loc_frame3_addr  <= loc_frame3_addr;
    end 
    
   // Always block for frame info offset
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame4_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame4_addr <= 16'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_frame4_addr <= 16'h0; 
       else if ((rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0) && loc_subampdu_frame4 && (loc_frame4_addr == 20'h0))
          loc_frame4_addr  <= loc_frame_addr + 'h4;
      else 
          loc_frame4_addr  <= loc_frame4_addr;
    end 
    
     // Always block for frame info offset
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame5_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame5_addr <= 16'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_frame5_addr <= 16'h0; 
       else if ((rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0) && loc_subampdu_frame5 && (loc_frame5_addr == 20'h0))
          loc_frame5_addr  <= loc_frame_addr + 'h4;
      else 
          loc_frame5_addr  <= loc_frame5_addr;
    end   
    
     // Always block for frame info offset
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame6_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame6_addr <= 16'h0;
       else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_frame6_addr <= 16'h0; 
       else if ((rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0) && loc_subampdu_frame6 && (loc_frame6_addr == 20'h0))
          loc_frame6_addr  <= loc_frame_addr + 'h4;
      else 
          loc_frame6_addr  <= loc_frame6_addr;
    end  
    
    // Always block for frame info offset
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame7_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame7_addr <= 16'h0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_frame7_addr <= 16'h0; 
       else if ((rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0) && loc_subampdu_frame7 && (loc_frame7_addr == 20'h0))
          loc_frame7_addr  <= loc_frame_addr + 'h4;
      else 
          loc_frame7_addr  <= loc_frame7_addr;
    end
    
     always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_frame_info_addr <= 16'h0;
       else if( !rx_enable )
            loc_frame_info_addr <= 16'h0;
       else if ((!loc_subampdu_frame1) && (!loc_subampdu_frame2) && (!loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_frame_info_addr  <= 16'h0;
      else if ((loc_subampdu_frame1) && (!loc_subampdu_frame2) && (!loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_frame_info_addr  <= loc_frame_addr + 'd5;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_frame1 <= 1'b0;
       else if( !rx_enable )
            loc_subampdu_frame1 <= 1'b0;
       else if ((rx_out_rx_ampdu_subframes_cnt) && (!loc_subampdu_frame1) && (!loc_subampdu_frame2) && (!loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_frame1  <= 1'b1;
       else if (rx_out_rxend_ind)
          loc_subampdu_frame1  <= 1'b0;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_frame2 <= 1'b0;
       else if( !rx_enable )
            loc_subampdu_frame2 <= 1'b0;
       else if ((rx_out_rx_ampdu_subframes_cnt) && (loc_subampdu_frame1) && (!loc_subampdu_frame2) && (!loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_frame2  <= 1'b1;
      else if (loc_vector_transmit_frame2_end)
          loc_subampdu_frame2  <= 1'b0;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_frame3 <= 1'b0;
       else if( !rx_enable )
            loc_subampdu_frame3 <= 1'b0;
       else if ((rx_out_rx_ampdu_subframes_cnt) && (loc_subampdu_frame1) && (loc_subampdu_frame2) && (!loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_frame3  <= 1'b1;
      else if (loc_vector_transmit_frame3_end)
          loc_subampdu_frame3  <= 1'b0;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_frame4 <= 1'b0;
       else if( !rx_enable )
            loc_subampdu_frame4 <= 1'b0;
       else if ((rx_out_rx_ampdu_subframes_cnt) && (loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_frame4  <= 1'b1;
      else if (loc_vector_transmit_frame4_end)
          loc_subampdu_frame4  <= 1'b0;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_frame5 <= 1'b0;
       else if( !rx_enable )
            loc_subampdu_frame5 <= 1'b0;
       else if ((rx_out_rx_ampdu_subframes_cnt) && (loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_frame5  <= 1'b1;
      else if (loc_vector_transmit_frame5_end)
          loc_subampdu_frame5  <= 1'b0;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_frame6 <= 1'b0;
       else if( !rx_enable )
            loc_subampdu_frame6 <= 1'b0;
       else if ((rx_out_rx_ampdu_subframes_cnt) && (loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_frame6  <= 1'b1;
      else if (loc_vector_transmit_frame6_end)
          loc_subampdu_frame6  <= 1'b0;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_frame7 <= 1'b0;
       else if( !rx_enable )
            loc_subampdu_frame7 <= 1'b0;
       else if ((rx_out_rx_ampdu_subframes_cnt) && (loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (loc_subampdu_frame5) && (loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_frame7  <= 1'b1;
      else if (loc_vector_transmit_frame7_end)
          loc_subampdu_frame7  <= 1'b0;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_frame8 <= 1'b0;
       else if( !rx_enable )
            loc_subampdu_frame8 <= 1'b0;
       else if ((rx_out_rx_ampdu_subframes_cnt) && (loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (loc_subampdu_frame5) && (loc_subampdu_frame6) && (loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_frame8 <= 1'b1;
      else if (loc_vector_transmit_frame8_end)
          loc_subampdu_frame8  <= 1'b0;
    end 
    
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_subampdu_count <= 3'h0;
       else if( !rx_enable )
            loc_subampdu_count <= 3'h0;
       else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
            loc_subampdu_count <= 3'h0; 
       else if ((loc_subampdu_frame1) && (!loc_subampdu_frame2) && (!loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_count <= 3'h1;
      else if ((loc_subampdu_frame1) && (loc_subampdu_frame2) && (!loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_count <= 3'h2;
      else if ((loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (!loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_count <= 3'h3;
      else if ((loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (!loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_count <= 3'h4;
      else if ((loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (loc_subampdu_frame5) && (!loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_count <= 3'h5;
      else if ((loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (loc_subampdu_frame5) && (loc_subampdu_frame6) && (!loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_count <= 3'h6;
      else if ((loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (loc_subampdu_frame5) && (loc_subampdu_frame6) && (loc_subampdu_frame7) && (!loc_subampdu_frame8))
          loc_subampdu_count <= 3'h7;
      else if ((loc_subampdu_frame1) && (loc_subampdu_frame2) && (loc_subampdu_frame3) && (loc_subampdu_frame4) && (loc_subampdu_frame5) && (loc_subampdu_frame6) && (loc_subampdu_frame7) && (loc_subampdu_frame8))
          loc_subampdu_count <= 3'h8;
    end 
    
    always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit <= 1'b0;
      else if (loc_vector_transmit)
          loc_vector_transmit  <= 1'b0;
       else if ((rx_enable) && (loc_subampdu_count >= 3'h2) && (loc_rxend_indication) && (!loc_mem_addr_dly))
          loc_vector_transmit <= 1'b1;
      end 
      
    always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_rxend_indication <= 1'b0;
       else if( !rx_enable )
            loc_rxend_indication <= 1'b0;
      else if (rx_enable && rx_out_rxend_ind && rx_aggr_enable_r)
          loc_rxend_indication  <= 1'b1;
       else if (loc_vector_transmit)
          loc_rxend_indication <= 1'b0;
      end 
    
    always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_in_process <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_in_process <= 1'b0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
            loc_vector_transmit_in_process <= 1'b0;
      else if (loc_vector_transmit)
          loc_vector_transmit_in_process  <= 1'b1;
        
    end  
    
    always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame2 <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame2 <= 1'b0;
      else if (loc_vector_count == 6'd20)
          loc_vector_transmit_frame2  <= 1'b0;
       else if (loc_vector_transmit_in_process && loc_subampdu_frame2 && (!loc_vector_transmit_frame2_end))
          loc_vector_transmit_frame2 <= 1'b1;
      end 
      
      always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame3 <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame3 <= 1'b0;
      else if (loc_vector_count == 6'd20)
          loc_vector_transmit_frame3  <= 1'b0;
       else if (loc_vector_transmit_in_process && loc_subampdu_frame3 && (!loc_vector_transmit_frame3_end) && loc_vector_transmit_frame2_end && loc_subampdu_count > 3'h2)
          loc_vector_transmit_frame3 <= 1'b1;
      end 
      
        always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame4 <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame4 <= 1'b0;
      else if (loc_vector_count == 6'd20)
          loc_vector_transmit_frame4  <= 1'b0;
       else if (loc_vector_transmit_in_process && loc_subampdu_frame4 && (!loc_vector_transmit_frame4_end) && loc_vector_transmit_frame3_end && loc_subampdu_count > 3'h3)
          loc_vector_transmit_frame4 <= 1'b1;
      end 
      
  always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame5 <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame5 <= 1'b0;
      else if (loc_vector_count == 6'd20)
          loc_vector_transmit_frame5  <= 1'b0;
       else if (loc_vector_transmit_in_process && loc_subampdu_frame5 && (!loc_vector_transmit_frame5_end) && loc_vector_transmit_frame4_end && loc_subampdu_count > 3'h4)
          loc_vector_transmit_frame5 <= 1'b1;
      end
      
 always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame6 <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame6 <= 1'b0;
      else if (loc_vector_count == 6'd20)
          loc_vector_transmit_frame6  <= 1'b0;
       else if (loc_vector_transmit_in_process && loc_subampdu_frame6 && (!loc_vector_transmit_frame6_end) && loc_vector_transmit_frame5_end && loc_subampdu_count > 3'h5)
          loc_vector_transmit_frame6 <= 1'b1;
      end
      
 always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame7 <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame7 <= 1'b0;
      else if (loc_vector_count == 6'd20)
          loc_vector_transmit_frame7  <= 1'b0;
       else if (loc_vector_transmit_in_process && loc_subampdu_frame7 && (!loc_vector_transmit_frame7_end) && loc_vector_transmit_frame6_end && loc_subampdu_count > 3'h6)
          loc_vector_transmit_frame7 <= 1'b1;
      end
      
 always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame8 <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame8 <= 1'b0;
      else if (loc_vector_count == 6'd20)
          loc_vector_transmit_frame8  <= 1'b0;
       else if (loc_vector_transmit_in_process && loc_subampdu_frame8 && (!loc_vector_transmit_frame8_end) && loc_vector_transmit_frame7_end && loc_subampdu_count > 3'h7)
          loc_vector_transmit_frame8 <= 1'b1;
      end
      
    always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame2_end <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame2_end <= 1'b0;
      else if (loc_vector_transmit_frame2 && loc_vector_count == 6'd20)
          loc_vector_transmit_frame2_end  <= 1'b1;
      else if (loc_vector_transmit_frame2_end)
          loc_vector_transmit_frame2_end  <= 1'b0;
      end  
      
      always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame3_end <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame3_end <= 1'b0;
      else if (loc_vector_transmit_frame3 && loc_vector_count == 6'd20)
          loc_vector_transmit_frame3_end  <= 1'b1;
      else if (loc_vector_transmit_frame3_end)
          loc_vector_transmit_frame3_end  <= 1'b0;
      end 
      
always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame4_end <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame4_end <= 1'b0;
      else if (loc_vector_transmit_frame4 && loc_vector_count == 6'd20)
          loc_vector_transmit_frame4_end  <= 1'b1;
      else if (loc_vector_transmit_frame4_end)
          loc_vector_transmit_frame4_end  <= 1'b0;
      end  
      
always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame5_end <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame5_end <= 1'b0;
      else if (loc_vector_transmit_frame5 && loc_vector_count == 6'd20)
          loc_vector_transmit_frame5_end  <= 1'b1;
      else if (loc_vector_transmit_frame5_end)
          loc_vector_transmit_frame5_end  <= 1'b0;
      end 
      
always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame6_end <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame6_end <= 1'b0;
      else if (loc_vector_transmit_frame6 && loc_vector_count == 6'd20)
          loc_vector_transmit_frame6_end  <= 1'b1;
      else if (loc_vector_transmit_frame6_end)
          loc_vector_transmit_frame6_end  <= 1'b0;
      end 
      
always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame7_end <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame7_end <= 1'b0;
      else if (loc_vector_transmit_frame7 && loc_vector_count == 6'd20)
          loc_vector_transmit_frame7_end  <= 1'b1;
      else if (loc_vector_transmit_frame7_end)
          loc_vector_transmit_frame7_end  <= 1'b0;
      end
      
always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_frame8_end <= 1'b0;
       else if( !rx_enable )
            loc_vector_transmit_frame8_end <= 1'b0;
      else if (loc_vector_transmit_frame8 && loc_vector_count == 6'd20)
          loc_vector_transmit_frame8_end  <= 1'b1;
      else if (loc_vector_transmit_frame8_end)
          loc_vector_transmit_frame8_end  <= 1'b0;
      end
      
      always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_count <= 6'b0;
       else if( !rx_enable )
            loc_vector_count <= 6'b0;
      else if (loc_vector_count == 6'd20)
          loc_vector_count  <= 6'b0;
       else if ((loc_vector_transmit_frame2) || (loc_vector_transmit_frame3) || (loc_vector_transmit_frame4) || (loc_vector_transmit_frame5) || (loc_vector_transmit_frame6) || (loc_vector_transmit_frame7) || (loc_vector_transmit_frame8))
          loc_vector_count <= loc_vector_count + 1'b1;
      end 
      
always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
            loc_vector_transmit_count <= 4'd0;
       else if( !rx_enable )
            loc_vector_transmit_count <= 4'd0;
      else if ((loc_vector_transmit_count == loc_subampdu_count) && (loc_subampdu_count != 'h0) && (loc_vector_transmit_count != 'h0))
             loc_vector_transmit_count <= 4'd0;
      else if (loc_vector_transmit_frame8_end)
          loc_vector_transmit_count  <= 4'd8;
       else if (loc_vector_transmit_frame7_end)
          loc_vector_transmit_count <= 4'd7;
      else if (loc_vector_transmit_frame6_end)
          loc_vector_transmit_count <= 4'd6;
      else if (loc_vector_transmit_frame5_end)
          loc_vector_transmit_count <= 4'd5;
      else if (loc_vector_transmit_frame4_end)
          loc_vector_transmit_count <= 4'd4;
      else if (loc_vector_transmit_frame3_end)
          loc_vector_transmit_count <= 4'd3;
      else if (loc_vector_transmit_frame2_end)
          loc_vector_transmit_count <= 4'd2;
      end 

// Always Block to generate Address to be given to the Memory.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_mem_addr <= `RX_MEM_BASE_OFFSET;
       else if( !rx_enable || rx_in_ev_rxstart)
           rx_out_mem_addr <= `RX_MEM_BASE_OFFSET + rx_addr_ptr;
       else if( rx_enable && !rx_aggr_enable_r ) // Non Aggregated Packet
       begin  
           if( rx_frame_len_r == ( rx_l_length_r - 16'h4 ))  // Frame Info Offset Address
               rx_out_mem_addr <= ( `RX_MEM_BASE_OFFSET + `RX_MEM_FRAME_INFO_OFFSET + rx_addr_ptr   );
           else if(rx_ampdu_subframe_end)
             rx_out_mem_addr <= ( `RX_MEM_BASE_OFFSET + `RX_MEM_MPDU_OFFSET+ rx_addr_ptr   );
           else if(( rx_frame_len_r == rx_l_length_r ) && rx_in_ev_rxend )
               rx_out_mem_addr <= ( `RX_MEM_BASE_OFFSET + `RX_MEM_AC_OFFSET+ rx_addr_ptr   ); // AC Offset Address
           else if( rx_in_ev_rxdata && ( rx_frame_info_count_r == 5'h14 ))
               rx_out_mem_addr <= ( `RX_MEM_BASE_OFFSET + `RX_MEM_MPDU_OFFSET+ rx_addr_ptr   ); // MPDU Offset Address
           else if( rx_frame_data_read_r && rx_in_frame_valid )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1;  // Address of Frame Data
           else if( rx_frame_vector_read_r && rx_in_frame_valid )begin
               if(!loc_mem_addr_inc) 
                 rx_out_mem_addr <= rx_out_mem_addr; // Address of Frame Vector
               else 
                 rx_out_mem_addr <= rx_out_mem_addr + 16'h1; // Address of Frame Vector
           end
                     
       end
       else if( rx_enable && rx_aggr_enable_r ) // Aggregated Packet
       begin
           if( (rx_frame_len_r == ( rx_deagg_subframe_len_w )) && (rx_frame_len_r != 20'h0)) // Frame Info Offset Address 
               rx_out_mem_addr <= ( `RX_MEM_BASE_OFFSET + `RX_MEM_FRAME_INFO_OFFSET + loc_frame_info_addr );
          else if( (rx_frame_len_r == ( rx_deagg_subframe_len_w + 'h1 )) && (rx_frame_len_r != 20'h0)) // Frame Info Offset Address 
               rx_out_mem_addr <= rx_out_mem_addr + 'h1; //( `RX_MEM_BASE_OFFSET + `RX_MEM_FRAME_INFO_OFFSET + 'h1);
          else if(( rx_in_ev_rxdata && ( rx_frame_info_count_r == 5'h14 )) ||
                   ( rx_in_ev_rxend && ( rx_in_rxend_status != `UU_WLAN_RX_END_STAT_NO_ERROR )) )
               rx_out_mem_addr <= ( `RX_MEM_BASE_OFFSET + `RX_MEM_MPDU_OFFSET ); // MPDU Offset Address
          else if( loc_vector_transmit)
               rx_out_mem_addr <= loc_frame1_addr; 
          else if(loc_vector_transmit_frame2_end )
               rx_out_mem_addr <= loc_frame2_addr; 
          else if(loc_vector_transmit_frame3_end )
               rx_out_mem_addr <= loc_frame3_addr;
          else if(loc_vector_transmit_frame4_end )
               rx_out_mem_addr <= loc_frame4_addr;
          else if(loc_vector_transmit_frame5_end )
               rx_out_mem_addr <= loc_frame5_addr;
          else if(loc_vector_transmit_frame6_end )
               rx_out_mem_addr <= loc_frame6_addr;
          else if(loc_vector_transmit_frame7_end )
               rx_out_mem_addr <= loc_frame7_addr;
           else if(loc_mem_addr_dly)
               rx_out_mem_addr <= loc_sub_ampdu_address; // MPDU Offset Address
           else if( rx_frame_end_read_r && 
                  ( rx_out_cap_return_value == `UU_WLAN_RX_HANDLER_FRAME_ERROR )) 
               rx_out_mem_addr <= ( `RX_MEM_BASE_OFFSET + `RX_MEM_AC_OFFSET ); // AC Offset Address
           else if( (rx_deagg_data_valid_w || rx_deagg_data_valid_d || rx_deagg_data_valid_dly || rx_deagg_data_valid_dly1) && rx_in_frame_valid )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1;    // Address of the Frame Data
           else if( rx_frame_vector_read_r && rx_in_frame_valid )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1;    // Address of the Frame Vector
           else if( loc_vector_transmit_frame2 )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1; 
          else if( loc_vector_transmit_frame3 )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1; 
         else if( loc_vector_transmit_frame4 )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1; 
         else if( loc_vector_transmit_frame5 )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1;
         else if( loc_vector_transmit_frame6 )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1;
         else if( loc_vector_transmit_frame7 )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1;
         else if( loc_vector_transmit_frame8 )
               rx_out_mem_addr <= rx_out_mem_addr + 16'h1;
       end
   end

// Always Block to generate AMPDU Frame End Indication.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_rxend_ind <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_rxend_ind )
           rx_out_rxend_ind <= 1'h0;
       else if( rx_enable && !rx_aggr_enable_r && rx_in_ev_rxend && 
             ( rx_frameinfo_len_r >= 20'hE ) &&
             ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ) && 
             ( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_US ) &&
             ( rx_out_cap_return_value != `UU_WLAN_RX_HANDLER_FRAME_ERROR ))
           rx_out_rxend_ind <= 1'b1;
       else if( rx_enable && rx_aggr_enable_r && 
              ( rx_aggr_psdu_len_r == 20'h1 ) && rx_in_frame_valid )
           rx_out_rxend_ind <= 1'b1;
   end

// Always Block to generate the PHY Rx Error Inidcation.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_out_phyrx_error_ind <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart || rx_out_phyrx_error_ind )
           rx_out_phyrx_error_ind <= 1'b0;
       else if(( rx_in_rxend_status != `UU_WLAN_RX_END_STAT_NO_ERROR ) || 
               ( rx_out_cap_return_value == `UU_WLAN_RX_HANDLER_FRAME_ERROR ))
           rx_out_phyrx_error_ind <= 1'b1;
   end
   
// Always Block for the RX Vector Valid Signal.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_frame_vector_read_r <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxdata )      
           rx_frame_vector_read_r <= 1'b0;
       else if( rx_enable && rx_in_ev_rxstart )
           rx_frame_vector_read_r <= 1'b1;
   end

// Always Block for RX Data Valid Signal.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_frame_data_read_r <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxend )
           rx_frame_data_read_r <= 1'b0;
       else if( rx_enable && rx_in_ev_rxdata )
           rx_frame_data_read_r <= 1'b1;
   end

// Always Block for Rx End Valid Signal
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_frame_end_read_r <= 1'b0;
       else if( !rx_enable || rx_in_ev_rxstart )
           rx_frame_end_read_r <= 1'b0;
       else if( rx_enable && rx_in_ev_rxend )
           rx_frame_end_read_r <= 1'b1;
   end

// Always Block to store the frame type return value
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_fill_frame_return_r <= 3'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_fill_frame_return_r <= 3'h0;
      else if( rx_deagg_enable_w )
         rx_fill_frame_return_r <= rx_deagg_return_value_w;
      else if( rx_mpdu_valid_w )
         rx_fill_frame_return_r <= `UU_LMAC_RX_RET_NON_AGG_FRAME;
   end

// Always Block to generate a Pusle to Trigger the CP module for Processing the RX Frame.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_out_cp_process_rx_frame <= 1'b0;
      else if( !rx_enable || rx_out_cp_process_rx_frame || rx_in_ev_rxstart )
         rx_out_cp_process_rx_frame <= 1'b0;
      else if(rx_enable && rx_aggr_enable_r && rx_ampdu_subframe_end &&
              ( rx_out_cap_return_value != `UU_WLAN_RX_HANDLER_FRAME_ERROR ))
         rx_out_cp_process_rx_frame <= 1'b1;
  /*    else if(( rx_in_ev_rxend || rx_frame_end_read_r ) && 
                rx_enable && rx_aggr_enable_r &&
              ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ) && 
              ( rx_out_cap_return_value != `UU_WLAN_RX_HANDLER_FRAME_ERROR ))
         rx_out_cp_process_rx_frame <= 1'b1;
      else if( rx_enable && rx_aggr_enable_r &&
             ( rx_in_ev_rxend || rx_frame_end_read_r ) &&
             ( rx_in_rxend_status != `UU_WLAN_RX_END_STAT_NO_ERROR ))
         rx_out_cp_process_rx_frame <= 1'b1; */
      else if( rx_enable && !rx_aggr_enable_r && rx_in_ev_rxend && 
             ( rx_frameinfo_len_r >= 20'hE ) &&
             ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ) && 
             ( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_US ) &&
             ( rx_out_cap_return_value != `UU_WLAN_RX_HANDLER_FRAME_ERROR ))
         rx_out_cp_process_rx_frame <= 1'b1;
   end 

// Always Block for Frame Length received from the Frame Details.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_frame_len_r <= 20'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_frame_len_r <= 20'h0;
      else if( rx_enable )
      begin
         if(( rx_in_ev_rxdata || rx_frame_data_read_r ) && 
              !rx_aggr_enable_r && rx_in_frame_valid &&
            ( rx_frame_len_r < rx_l_length_r ))
             rx_frame_len_r <= rx_frame_len_r + 20'h1;
         else if( rx_frame_data_read_r && rx_deagg_sign_valid_w && rx_aggr_enable_r && 
                  !rx_deagg_crc8_invalid_w && rx_in_frame_valid )
             rx_frame_len_r <= rx_frame_len_r + 20'h4; 
         else if( !rx_deagg_subframe_len_w && rx_aggr_enable_r && 
                  rx_frame_data_read_r && rx_ampdu_sign_delay_r )
         begin
            if( rx_in_frame_valid )
                rx_frame_len_r <= 20'h1;
            else
                rx_frame_len_r <= 20'h0;
         end     
         else if( rx_deagg_data_valid_w && rx_in_frame_valid && 
                  rx_aggr_enable_r && rx_ampdu_subframe_end )
             rx_frame_len_r <= 20'h0;
         else if( rx_frame_data_read_r && rx_aggr_enable_r && !rx_deagg_data_valid_w && !rx_deagg_data_valid_d && !rx_deagg_data_valid_dly && !rx_deagg_data_valid_dly1)
             rx_frame_len_r <= 20'h0; 
         else if( (rx_deagg_data_valid_w || rx_deagg_data_valid_d || rx_deagg_data_valid_dly || rx_deagg_data_valid_dly1) && rx_in_frame_valid && rx_aggr_enable_r )
             rx_frame_len_r <= rx_frame_len_r + 20'h1;
      end
   end

// Always Block to Output Frame Info Length.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )	
         rx_frameinfo_len_r <= 20'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_frameinfo_len_r <= 20'h0;
      else if( rx_enable )
      begin
         if( rx_frame_data_read_r && rx_in_frame_valid && !rx_aggr_enable_r &&
           (( rx_frame_len_r + 20'h1 )== { 4'h0, rx_l_length_r } ))
               rx_frameinfo_len_r <= { 4'h0, rx_l_length_r };
         else if( rx_aggr_enable_r && rx_frame_data_read_r && rx_ampdu_sign_delay_r )
         begin
            if( rx_deagg_subframe_len_w > 16'h0 ) 
               rx_frameinfo_len_r <= rx_frameinfo_len_r + 20'h4;
            else
               rx_frameinfo_len_r <= 20'h0;
         end
         else if( rx_deagg_data_valid_w && rx_in_frame_valid && 
                  rx_aggr_enable_r && rx_ampdu_subframe_end )
            rx_frameinfo_len_r <= { 4'h0, rx_deagg_subframe_len_w }; 
      end
   end

// Always Block for delayed pulse of Sign Valid and CRC Valid.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_ampdu_sign_delay_r <= 1'b0;
      else
         rx_ampdu_sign_delay_r <= ( rx_deagg_sign_valid_w && !rx_deagg_crc8_invalid_w );
   end

// Always Block for the Aggregated PSDU length to be stored.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_aggr_psdu_len_r <= 20'h0;
      else if( !rx_enable || rx_in_ev_rxstart || !rx_aggr_enable_r )
         rx_aggr_psdu_len_r <= 20'h0;
      else if( rx_enable )
      begin
         if(( rx_in_ev_rxend || rx_frame_end_read_r ) && 
            ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ))
            rx_aggr_psdu_len_r <= 20'h0;
         else if(( rx_frame_info_count_r == 5'h13 ) && rx_in_frame_valid &&
                   rx_aggr_enable_r && rx_frame_vector_read_r )
            rx_aggr_psdu_len_r <= rx_psdu_len_r;  
         else if( rx_frame_data_read_r && rx_aggr_enable_r && !rx_ampdu_subframe_end)
         begin
            if(( rx_deagg_sign_valid_w || rx_deagg_crc8_invalid_w || 
                 rx_deagg_invalid_w ) && ( rx_aggr_psdu_len_r >= 20'h4 ) && 
                 rx_in_frame_valid )
               rx_aggr_psdu_len_r <= ( rx_aggr_psdu_len_r - 20'h4 );
            else if( !rx_deagg_subframe_len_w && rx_ampdu_sign_delay_r )
               rx_aggr_psdu_len_r <= ( rx_aggr_psdu_len_r - 20'h4 );
         end
         else if( rx_in_frame_valid && 
                  rx_aggr_enable_r && rx_ampdu_subframe_end )begin
            if(rx_aggr_psdu_len_r == rx_deagg_subframe_len_w)
              rx_aggr_psdu_len_r <= ( rx_aggr_psdu_len_r - (rx_deagg_subframe_len_w -'h1) ); //including 2 zero delimiters (+8) 
            else
              rx_aggr_psdu_len_r <= ( rx_aggr_psdu_len_r - (rx_deagg_subframe_len_w + 'h8) ); //including 2 zero delimiters (+8) 
        end
      end
   end

// Always Block for RX Return Value to CAP.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_out_cap_return_value <= 32'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_out_cap_return_value <= 32'h0;
      else if( rx_enable && ( !rx_post_to_umac_r ) &&
            (( rx_aggr_enable_r && ( rx_aggr_psdu_len_r > 20'h0 ) &&
             ( rx_fill_frame_return_r == `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME )) || 
             ( !rx_aggr_enable_r && ( rx_fill_frame_return_r == `UU_LMAC_RX_RET_NON_AGG_FRAME))))
      begin
         if( fcs_valid_w && ( rx_frame_crc_r != fcs_word_w ))
            rx_out_cap_return_value <= `UU_WLAN_RX_HANDLER_FRAME_ERROR;       
         else if(( rx_frame_control_r [1:0] != 2'h0 ) || ( rx_frame_control_r [3:2] == 2'h3 ))
            rx_out_cap_return_value <= `UU_WLAN_RX_HANDLER_FRAME_ERROR;
         else if( rx_frame_destination_r != `UU_WLAN_RX_FRAME_DEST_US )
         begin 
            if((( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_BROADCAST ) ||
                ( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_MULTICAST ) &&
                ( rx_frame_control_r[8] )) && ( rx_frame_control_r [3:2] == 2'h2 ) &&
                ( rx_bssid_addr_r != `UU_WLAN_IEEE80211_STA_MAC_ADDR_R )) 
               rx_out_cap_return_value <= `UU_WLAN_RX_HANDLER_NAV_UPDATE; 
            else if( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_OTHERS )
               rx_out_cap_return_value <= `UU_WLAN_RX_HANDLER_NAV_UPDATE; 
         end
      end
      else if( rx_in_cp_return_valid ) 
         rx_out_cap_return_value <= rx_in_cp_return_value;
   end

// Always Block for Frame Decrypt Enable.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_out_frame_decrypt_enable <= 1'b0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_out_frame_decrypt_enable <= 1'b0;
      else if( rx_enable && ( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_US ) &&
             ( rx_out_cap_return_value != `UU_WLAN_RX_HANDLER_FRAME_ERROR ))
         rx_out_frame_decrypt_enable <= rx_frame_control_r [14];
   end

// Always Block for Output Post to UMAC.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_out_frame_post_to_umac <= 1'b0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_out_frame_post_to_umac <= 1'b0;
      else if( rx_enable && rx_post_to_umac_r && rx_umac_ready_r && 
             ( rx_frame_destination_r != `UU_WLAN_RX_FRAME_DEST_LOOPBACK ))
         rx_out_frame_post_to_umac <= 1'b1;
   end

// Always Block for UMAC READY register.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_umac_ready_r <= 1'b0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_umac_ready_r <= 1'b0;
      else if(( rx_in_cp_return_value == `UU_WLAN_RX_HANDLER_FRAME_ERROR ) && rx_in_cp_return_valid ) 
         rx_umac_ready_r <= 1'b0;
      else if( rx_enable && rx_in_lmac_filter_flag[0] )
         rx_umac_ready_r <= 1'b1;
      else if( rx_enable && rx_in_lmac_filter_flag[9] )
         rx_umac_ready_r <= 1'b0;
      else if( rx_enable && ( rx_aggr_psdu_len_r > 20'h0 ) && 
             ( rx_fill_frame_return_r == `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME ) &&
               rx_aggr_enable_r && (( fcs_valid_w && ( rx_frame_crc_r != fcs_word_w )) || 
            (( rx_frame_control_r [1:0] != 2'h0 ) && ( rx_frame_control_r [3:2] == 2'h3 ))))
         rx_umac_ready_r <= 1'b0;
      else if( rx_enable && ( rx_fill_frame_return_r == `UU_LMAC_RX_RET_NON_AGG_FRAME ) &&
              !rx_aggr_enable_r && (( fcs_valid_w && ( rx_frame_crc_r != fcs_word_w )) || 
            (( rx_frame_control_r [1:0] != 2'h0 ) && ( rx_frame_control_r [3:2] == 2'h3 ))))
         rx_umac_ready_r <= 1'b0;
      else if( rx_enable && rx_in_lmac_filter_flag[4] &&
             ( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_MULTICAST ))
         rx_umac_ready_r <= 1'b1;
      else if( rx_enable && rx_in_lmac_filter_flag[3] &&
             ( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_BROADCAST )) 
         rx_umac_ready_r <= 1'b1;
      else if( rx_enable && ( rx_frame_control_r [3:2] == 2'h1 ) && 
                rx_in_lmac_filter_flag[1] )
         rx_umac_ready_r <= 1'b1; 
      else if( rx_enable && ( rx_frame_control_r [3:2] == 2'h0 ))
      begin
         if(( rx_frame_control_r [7:4] == 4'h4 ) && rx_in_lmac_filter_flag[5] )
            rx_umac_ready_r <= 1'b1;
         else if(( rx_frame_control_r [7:4] == 4'h8 ) && rx_in_lmac_filter_flag[6] )
            rx_umac_ready_r <= 1'b1;
         else if(( rx_frame_control_r [7:4] == 4'h5 ) && rx_in_lmac_filter_flag[7] )
            rx_umac_ready_r <= 1'b1;
         else
            rx_umac_ready_r <= 1'b0;   
      end
      else
         rx_umac_ready_r <= 1'b0;
   end

// Always Block to Register Post to UMAC.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_post_to_umac_r <= 1'b0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_post_to_umac_r <= 1'b0;
      else if( rx_enable && 
           ((( rx_aggr_psdu_len_r > 20'h0 ) && rx_aggr_enable_r &&                 
             ( rx_fill_frame_return_r == `UU_LMAC_RX_RET_COMPLETE_AMPDU_SUBFRAME )) ||
             ( rx_in_ev_rxend && ( rx_frameinfo_len_r >= 20'hE ) &&
             ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ))))
      begin
         if( fcs_valid_w && ( rx_frame_crc_r != fcs_word_w ))
            rx_post_to_umac_r <= 1'b1;
         else if(( rx_frame_control_r [1:0] != 2'h0 ) && ( rx_frame_control_r [3:2] == 2'h3 ))
            rx_post_to_umac_r <= 1'b1;
         else if(( rx_frame_destination_r != `UU_WLAN_RX_FRAME_DEST_US ) &&
                 ( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_OTHERS ))
               rx_post_to_umac_r <= 1'b1;
         else if( rx_frame_destination_r != `UU_WLAN_RX_FRAME_DEST_US )         
         begin
            if((( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_MULTICAST ) || 
                ( rx_frame_destination_r == `UU_WLAN_RX_FRAME_DEST_BROADCAST )) && 
                ( rx_frame_control_r [9:8] == 2'h1 ))
            begin
                if( !(( rx_bssid_addr_r == `UU_WLAN_IEEE80211_STA_MAC_ADDR_R ) && 
                      ( rx_frame_control_r [3:2] == 2'h2 ))) 
                   rx_post_to_umac_r <= 1'b1;
            end
         end
         else
             rx_post_to_umac_r <= 1'b1;
      end
      else if( rx_in_cp_return_valid && |rx_in_cp_return_value[31:24] )
          rx_post_to_umac_r <= 1'b1;
   end

// Always Block for Frame Length in an MPDU.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_l_length_r <= 16'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_l_length_r <= 16'h0;
      else if( rx_enable && rx_in_frame_valid && rx_frame_vector_read_r )
      begin
         if( rx_frame_info_count_r == 5'h2 ) begin
            rx_l_length_r <= { rx_l_length_r[15:8], rx_in_frame };
rx_out_frame_length<={ rx_l_length_r[15:8], rx_in_frame };end
         else if( rx_frame_info_count_r == 5'h3 )
            //rx_l_length_r <= { rx_in_frame, rx_l_length_r[7:0] }; 
          rx_l_length_r <= {  rx_l_length_r[15:8], rx_in_frame }; 
      end
   end 

// Always Block for the PSDU Length Register internal to the RX Handler Module.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_psdu_len_r <= 20'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_psdu_len_r <= 20'h0;
      else if( rx_enable && rx_in_frame_valid && rx_frame_vector_read_r )
      begin
         if( rx_frame_info_count_r == 5'hD )
            rx_psdu_len_r <= { rx_psdu_len_r [19:8], rx_in_frame };
         else if( rx_frame_info_count_r == 5'hE ) 
            rx_psdu_len_r <= { rx_psdu_len_r [19:16], rx_in_frame, rx_psdu_len_r [7:0] };
         else if( rx_frame_info_count_r == 5'hF ) 
            rx_psdu_len_r <= { rx_in_frame [3:0], rx_psdu_len_r [15:0] };
      end
   end

// Always Block for Aggregation Enable register.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_aggr_enable_r <= 1'b0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_aggr_enable_r <= 1'b0;
      else if( rx_enable && ( rx_frame_info_count_r == 5'h6 ) &&
               rx_frame_vector_read_r && rx_in_frame_valid )
         rx_aggr_enable_r <= rx_in_frame[2];
   end

// Always Block for Frame Format.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_frame_format_r <= `UU_WLAN_FRAME_FORMAT_NON_HT;
      else if( !rx_enable )
         rx_frame_format_r <= `UU_WLAN_FRAME_FORMAT_NON_HT;
      else if( rx_enable && (rx_frame_info_count_r == 5'h0 ) && 
             ( rx_in_ev_rxstart || rx_frame_vector_read_r ) && rx_in_frame_valid )
         rx_frame_format_r <= rx_in_frame [1:0];
   end

// Always Block to keep track of the RX Vector received.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_frame_info_count_r <= 5'h0;
      else if( !rx_enable || rx_in_ev_rxdata )
         rx_frame_info_count_r <= 5'h0;
      else if( rx_enable && rx_in_frame_valid &&
             ( rx_in_ev_rxstart || rx_frame_vector_read_r ))
         rx_frame_info_count_r <= rx_frame_info_count_r + 5'h1;
   end

// Always Block to Store the FCS for AMPDU.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_frame_crc_r <= 32'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_frame_crc_r <= 32'h0;
      else if( rx_enable && rx_in_frame_valid && 
            (( rx_deagg_data_valid_w && rx_aggr_enable_r &&
             ( rx_frame_len_r == rx_deagg_subframe_len_w )) || 
             ( rx_frame_data_read_r && !rx_aggr_enable_r && 
             ( rx_frame_len_r == ( rx_l_length_r - 16'h4 ))))) 
         rx_frame_crc_r <= { rx_in_frame, rx_frame_crc_r [23:0] };
      else if( rx_enable && rx_in_frame_valid &&
            (( rx_deagg_data_valid_w && rx_aggr_enable_r &&
            (( rx_frame_len_r - 20'h1 ) == rx_deagg_subframe_len_w )) || 
             ( rx_frame_data_read_r && !rx_aggr_enable_r && 
             ( rx_frame_len_r == ( rx_l_length_r - 16'h3 ))))) 
         rx_frame_crc_r <= { rx_frame_crc_r [31:24], rx_in_frame, rx_frame_crc_r [15:0] };
      else if( rx_enable && rx_in_frame_valid &&
            (( rx_deagg_data_valid_w && rx_aggr_enable_r &&
            (( rx_frame_len_r - 20'h2 ) == rx_deagg_subframe_len_w )) || 
             ( rx_frame_data_read_r && !rx_aggr_enable_r && 
             ( rx_frame_len_r == ( rx_l_length_r - 16'h2 ))))) 
         rx_frame_crc_r <= { rx_frame_crc_r [31:16], rx_in_frame, rx_frame_crc_r [7:0] };
      else if( rx_enable && rx_in_frame_valid &&
            (( rx_deagg_data_valid_w && rx_aggr_enable_r &&
            (( rx_frame_len_r - 20'h3 ) == rx_deagg_subframe_len_w )) || 
             ( rx_frame_data_read_r && !rx_aggr_enable_r && 
             ( rx_frame_len_r == ( rx_l_length_r - 16'h1 ))))) 
         rx_frame_crc_r <= { rx_frame_crc_r [31:8], rx_in_frame };
   end 

// Always Block for Frame Control field Bytes.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_frame_control_r <= 16'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_frame_control_r <= 16'h0;
      else if( rx_enable && rx_in_frame_valid && 
            (( rx_deagg_data_valid_w && ( rx_frame_len_r == 20'h4 ) && rx_aggr_enable_r) ||
            (( rx_in_ev_rxdata || rx_frame_data_read_r ) && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h0 )))) 
         rx_frame_control_r <= { rx_frame_control_r [15:8], rx_in_frame };
      else if( rx_enable && rx_in_frame_valid && 
            (( rx_deagg_data_valid_w && ( rx_frame_len_r == 20'h5 ) && rx_aggr_enable_r) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r && ( rx_frame_len_r == 20'h1 )))) 
         rx_frame_control_r <= { rx_in_frame, rx_frame_control_r [7:0] };
   end

// Always Block to Store the QOS field of the Frame.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_frame_qos_r <= 16'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_frame_qos_r <= 16'h0;
      else if( rx_enable && rx_in_frame_valid && 
            (( rx_deagg_data_valid_w && ( rx_frame_len_r == 20'h1C ) && rx_aggr_enable_r ) ||
            ( rx_frame_data_read_r && !rx_aggr_enable_r && ( rx_frame_len_r == 20'h18 )))) 
         rx_frame_qos_r <= { rx_frame_qos_r [15:8], rx_in_frame };
      else if( rx_enable && rx_in_frame_valid && 
            (( rx_deagg_data_valid_w && ( rx_frame_len_r == 20'h1D ) && rx_aggr_enable_r ) ||
            ( rx_frame_data_read_r && !rx_aggr_enable_r && ( rx_frame_len_r == 20'h19 )))) 
         rx_frame_qos_r <= { rx_in_frame, rx_frame_qos_r [7:0] };
   end

// Always Block for MAC Address of the frame.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_mac_addr_r <= 48'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_mac_addr_r <= 48'h0;
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'h8 ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h4 )))) // MAC Address 1st Byte 
              
         rx_mac_addr_r <= { rx_mac_addr_r [47:8], rx_in_frame };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'h9 ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h5 )))) // MAC Address 2nd Byte
         rx_mac_addr_r <= { rx_mac_addr_r [47:16], rx_in_frame, rx_mac_addr_r [7:0] };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'hA ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h6 )))) // MAC Address 3rd Byte
         rx_mac_addr_r <= { rx_mac_addr_r [47:24], rx_in_frame, rx_mac_addr_r [15:0] };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'hB ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h7 )))) // MAC Address 4th Byte
         rx_mac_addr_r <= { rx_mac_addr_r [47:32], rx_in_frame, rx_mac_addr_r [23:0] };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'hC ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h8 )))) // MAC Address 5th Byte
         rx_mac_addr_r <= { rx_mac_addr_r [47:40], rx_in_frame, rx_mac_addr_r [31:0] };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'hD ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h9 )))) // MAC Address 6th Byte
         rx_mac_addr_r <= { rx_in_frame, rx_mac_addr_r [39:0] }; 
   end

// Always Block for BSSID Address of the Frame.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_bssid_addr_r <= 48'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_bssid_addr_r <= 48'h0;
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'h14 ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h10 )))) // BSSID 1st Byte 
         rx_bssid_addr_r <= { rx_bssid_addr_r [47:8], rx_in_frame };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'h15 ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h11 )))) // BSSID 2nd Byte 
         rx_bssid_addr_r <= { rx_bssid_addr_r [47:16], rx_in_frame, rx_bssid_addr_r [7:0] };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'h16 ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h12 )))) // BSSID 3rd Byte
         rx_bssid_addr_r <= { rx_bssid_addr_r [47:24], rx_in_frame, rx_bssid_addr_r [15:0] };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'h17 ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h13 )))) // BSSID 4th Byte
         rx_bssid_addr_r <= { rx_bssid_addr_r [47:32], rx_in_frame, rx_bssid_addr_r [23:0] };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'h18 ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h14 )))) // BSSID 5th Byte
         rx_bssid_addr_r <= { rx_bssid_addr_r [47:40], rx_in_frame, rx_bssid_addr_r [31:0] };
      else if( rx_enable && rx_in_frame_valid && (( rx_deagg_data_valid_w &&
             ( rx_frame_len_r == 20'h19 ) && rx_aggr_enable_r ) ||
             ( rx_frame_data_read_r && !rx_aggr_enable_r &&
             ( rx_frame_len_r == 20'h15 )))) // BSSID 6th Byte
         rx_bssid_addr_r <= { rx_in_frame, rx_bssid_addr_r [39:0] };
   end

// Always Block for the Rx Frame Destination Address Decoding.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_frame_destination_r <= `UU_WLAN_RX_FRAME_DEST_LOOPBACK; 
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_frame_destination_r <= `UU_WLAN_RX_FRAME_DEST_LOOPBACK; 
      else if( rx_enable && ( &rx_mac_addr_r ))
         rx_frame_destination_r <= `UU_WLAN_RX_FRAME_DEST_BROADCAST;
      else if( rx_enable && ( rx_mac_addr_r [0] == 1'b1 ))
         rx_frame_destination_r <= `UU_WLAN_RX_FRAME_DEST_MULTICAST;
      else if( rx_enable && ( rx_mac_addr_r == `UU_WLAN_IEEE80211_STA_MAC_ADDR_R ))
         rx_frame_destination_r <= `UU_WLAN_RX_FRAME_DEST_US;
      else 
         rx_frame_destination_r <= `UU_WLAN_RX_FRAME_DEST_OTHERS; 
   end

// Always Block to send the Access Category.
   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         rx_frame_ac_r <= 2'h0;
      else if( !rx_enable || rx_in_ev_rxstart )
         rx_frame_ac_r <= 2'h0;
      else if( rx_enable && ( rx_in_rxend_status == `UU_WLAN_RX_END_STAT_NO_ERROR ) &&
               fcs_valid_w )
      begin 
          if( rx_frame_control_r [7:0] >= 8'h88 )  // Qos Frame
          begin // Block for TID calculation
              if(( rx_frame_qos_r [3:0] == 4'h6 ) || ( rx_frame_qos_r [3:0] == 4'h7 ))
                  rx_frame_ac_r <= 2'h3;
              else if(( rx_frame_qos_r [3:0] == 4'h4 ) || ( rx_frame_qos_r [3:0] == 4'h5 ))
                  rx_frame_ac_r <= 2'h2;
              else if(( rx_frame_qos_r [3:0] == 4'h0 ) || ( rx_frame_qos_r [3:0] == 4'h3 ))
                  rx_frame_ac_r <= 2'h0;
              else if(( rx_frame_qos_r [3:0] == 4'h1 ) || ( rx_frame_qos_r [3:0] == 4'h2 ))
                  rx_frame_ac_r <= 2'h1;
              else
                  rx_frame_ac_r <= 2'h0;
          end
          else
              rx_frame_ac_r <= 2'h0;
      end
   end

   always @( posedge rx_clk or `EDGE_OF_RESET )
   begin
       if( `POLARITY_OF_RESET )
           rx_addr_ptr_buff <= 0;
       else if( !rx_enable || rx_in_ev_rxstart )
           rx_addr_ptr_buff <= 0;
       else if (rx_in_cap_frame_info_retry)
           rx_addr_ptr_buff <= 0;
       else if( rx_enable && !rx_aggr_enable_r ) // Non Aggregated Packet
       begin  
           if( rx_frame_len_r == ( rx_l_length_r - 16'h4 ))  // Frame Info Offset Address
					 rx_addr_ptr_buff<=rx_out_mem_addr;
		end
end

always @( posedge rx_clk or `EDGE_OF_RESET )
begin
if( `POLARITY_OF_RESET )
	rx_addr_ptr<=0;
else if (rx_in_cap_frame_info_retry)
     rx_addr_ptr<= 0;
else if (rx_in_ev_rxend)
rx_addr_ptr<=rx_addr_ptr_buff;
end
endmodule

