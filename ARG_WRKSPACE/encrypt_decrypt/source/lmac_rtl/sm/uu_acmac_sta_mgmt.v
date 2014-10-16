 /*****************************************************************************
 * Description                                                                *
 * ------------                                                               *
 * This module implements the functionality of STATION MANAGEMENT in LMAC.    *
 *****************************************************************************/

`include "../../inc/defines.h"

`define UU_WLAN_MAX_ASSOCIATIONS     32'd20
`define STA_IS_USED_OFFSET           14'h0000
`define BA_IS_USED_OFFSET            14'h0004
`define STA00_MAC_ADDR		     14'h0010

`define STA00_INFO_OFFSET            14'h0200
`define STA01_INFO_OFFSET            14'h0230
`define STA02_INFO_OFFSET            14'h0260
`define STA03_INFO_OFFSET            14'h0290
`define STA04_INFO_OFFSET            14'h02C0
`define STA05_INFO_OFFSET            14'h02F0
`define STA06_INFO_OFFSET            14'h0320
`define STA07_INFO_OFFSET            14'h0350
`define STA08_INFO_OFFSET            14'h0380
`define STA09_INFO_OFFSET            14'h03B0
`define STA10_INFO_OFFSET            14'h03E0
`define STA11_INFO_OFFSET            14'h0410
`define STA12_INFO_OFFSET            14'h0440
`define STA13_INFO_OFFSET            14'h0470
`define STA14_INFO_OFFSET            14'h04A0
`define STA15_INFO_OFFSET            14'h04D0
`define STA16_INFO_OFFSET            14'h0500
`define STA17_INFO_OFFSET            14'h0530
`define STA18_INFO_OFFSET            14'h0560
`define STA19_INFO_OFFSET            14'h0590

`define BA00_INFO_OFFSET             14'h05C0
`define BA01_INFO_OFFSET             14'h06D0
`define BA02_INFO_OFFSET             14'h07E0
`define BA03_INFO_OFFSET             14'h08F0
`define BA04_INFO_OFFSET             14'h0A00
`define BA05_INFO_OFFSET             14'h0B10
`define BA06_INFO_OFFSET             14'h0C20
`define BA07_INFO_OFFSET             14'h0D30
`define BA08_INFO_OFFSET             14'h0E40
`define BA09_INFO_OFFSET             14'h0F50
`define BA10_INFO_OFFSET             14'h1060
`define BA11_INFO_OFFSET             14'h1170
`define BA12_INFO_OFFSET             14'h1280
`define BA13_INFO_OFFSET             14'h1390
`define BA14_INFO_OFFSET             14'h14A0
`define BA15_INFO_OFFSET             14'h15B0
`define BA16_INFO_OFFSET             14'h16C0
`define BA17_INFO_OFFSET             14'h17D0
`define BA18_INFO_OFFSET             14'h18E0
`define BA19_INFO_OFFSET             14'h19F0
`define BA20_INFO_OFFSET             14'h1B00
`define BA21_INFO_OFFSET             14'h1C10
`define BA22_INFO_OFFSET             14'h1D20
`define BA23_INFO_OFFSET             14'h1E30
`define BA24_INFO_OFFSET             14'h1F40
`define BA25_INFO_OFFSET             14'h2050
`define BA26_INFO_OFFSET             14'h2160
`define BA27_INFO_OFFSET             14'h2270
`define BA28_INFO_OFFSET             14'h2380
`define BA29_INFO_OFFSET             14'h2490
`define BA30_INFO_OFFSET             14'h25A0
`define BA31_INFO_OFFSET             14'h26B0
`define BA32_INFO_OFFSET             14'h27C0
`define BA33_INFO_OFFSET             14'h28D0
`define BA34_INFO_OFFSET             14'h29E0
`define BA35_INFO_OFFSET             14'h2AF0
`define BA36_INFO_OFFSET             14'h2C00
`define BA37_INFO_OFFSET             14'h2D10
`define BA38_INFO_OFFSET             14'h2E20
`define BA39_INFO_OFFSET             14'h2F30

`define TX_STATUS_AC0_FLAG           14'h3040
`define TX_STATUS_AC1_FLAG           14'h3044
`define TX_STATUS_AC2_FLAG           14'h3048
`define TX_STATUS_AC3_FLAG           14'h304C

`define TX_STATUS_AC0_OFFSET         15'h4310 
`define TX_STATUS_AC1_OFFSET         15'h4410
`define TX_STATUS_AC2_OFFSET         15'h4510
`define TX_STATUS_AC3_OFFSET         15'h4610

module uu_acmac_sta_mgmt #(
                        parameter  MEM_WIDTH = 32'd15
                      )
                      (
                        input                                         sta_clk,             // STA & Ba Management Clock Input
                        input                                         rst_n,               // STA & Ba Management Reset Input
                        input                                         sta_enable,          // STA & Ba Management Module Enable Input

                        // Input from the DMA to the Block RAM
                        input       [3 :0]                            sta_in_umac_bea,     // Byte Enable Input from DMA
                        input       [31:0]                            sta_in_umac_data,    // Data Input from DMA
                        input       [MEM_WIDTH-1:0]                   sta_in_umac_addr,    // Address Input from DMA
                        input                                         sta_in_umac_wrn_rd,  // Write/Read Input from DMA
                        input                                         sta_in_umac_val,     // Data Input Valid from DMA
                        output      [31:0]                            sta_out_umac_data,   // Data Output from STA & BA Module Memory
                        output reg                                    sta_out_umac_ack,    // Data Output Valid from STA & BA Module Memory

                        // Input from Tx and CAP
                        input       [11:0]                            sta_in_tx_seqno_ac0, // Sequence Number from Tx for AC0
                        input       [11:0]                            sta_in_tx_seqno_ac1, // Sequence Number from Tx for AC1
                        input       [11:0]                            sta_in_tx_seqno_ac2, // Sequence Number from Tx for AC2
                        input       [11:0]                            sta_in_tx_seqno_ac3, // Sequence Number from Tx for AC3
                        input       [7 :0]                            sta_in_ac0_src,      // Short Retry Count for AC0
                        input       [7 :0]                            sta_in_ac1_src,      // Short Retry Count for AC1
                        input       [7 :0]                            sta_in_ac2_src,      // Short Retry Count for AC2
                        input       [7 :0]                            sta_in_ac3_src,      // Short Retry Count for AC3
                        input       [7 :0]                            sta_in_ac0_lrc,      // Long Retry Count for AC0
                        input       [7 :0]                            sta_in_ac1_lrc,      // Long Retry Count for AC1
                        input       [7 :0]                            sta_in_ac2_lrc,      // Long Retry Count for AC2
                        input       [7 :0]                            sta_in_ac3_lrc,      // Long Retry Count for AC3
                        input       [2 :0]                            sta_in_ac_owner,     // AC from the CAP
                        input                                         sta_in_clear_mpdu,   // Clear MPDU Signal from CAP
                        input                                         sta_in_clear_ampdu,  // Clear AMPDU Signal from CAP
                        input       [7 :0]                            sta_in_bitmap_ac0,   // Bitmap for AC0 AMPDU
                        input       [7 :0]                            sta_in_bitmap_ac1,   // Bitmap for AC1 AMPDU
                        input       [7 :0]                            sta_in_bitmap_ac2,   // Bitmap for AC2 AMPDU
                        input       [7 :0]                            sta_in_bitmap_ac3,   // Bitmap for AC3 AMPDU
                        input                                         sta_in_mpdu_status,  // Status Signal for MPDU Clear
                        input       [15:0]                            sta_in_RTS_threshold,// Threshold Input Length for the Packet
                        input       [19:0]                            sta_in_frame_length, // Input frame Length for the Packet
                        input       [7 :0]                            sta_in_aggr_count,   // Aggregated number of Packets

                        // Input from CP
                        input       [3 :0]                            sta_in_bea,          // Byte Enable from the CP and Tx Module 
                        input       [31:0]                            sta_in_data,         // Data Input from CP and Tx Module
                        input       [MEM_WIDTH-1:0]                   sta_in_addr,         // Address Input from the CP and Tx Module
                        input                                         sta_in_val,          // Data Input Valid from CP and Tx Module
                        output      [31:0]                            sta_out_data,        // Data Outupt from the STA and BA Module

                        input       [47:0]                            sta_in_mac_addr,     // Input MAC Address for Address Resolution
                        input       [3 :0]                            sta_in_tid,          // Input TID for Address Resolution
                        input                                         sta_in_dir,          // Input Direction for Address Resolution 
                        input                                         sta_in_get_ba,       // Input Request for retrieving the BA session
                        output reg  [14:0]                            sta_out_ba_offset,   // Output Offset of the BA session
                        output reg                                    sta_out_get_ba_ack,  // Acknowledgement for the BA Session Request
                        output reg                                    sta_out_ba_valid     // Output Valid Signal for Processing the BA Session
                      );

   // STATES for the FSM
   parameter   IDLE         = 3'h1,
               COMPARE_ADDR = 3'h2,
               READ_ADDR    = 3'h4;

   // Internal Registers
   reg   [47:0]                           sta_mac_addr_r0;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r1;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r2;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r3;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r4;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r5;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r6;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r7;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r8;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r9;                                 // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r10;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r11;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r12;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r13;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r14;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r15;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r16;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r17;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r18;                                // Registers to Store Valid MAC Addresses
   reg   [47:0]                           sta_mac_addr_r19;                                // Registers to Store Valid MAC Addresses
   reg   [31:0]                           sta_tx_status_flag_ac0_r;                       // Tx Status Flag for AC0
   reg   [31:0]                           sta_tx_status_flag_ac1_r;                       // Tx Status Flag for AC1
   reg   [31:0]                           sta_tx_status_flag_ac2_r;                       // Tx Status Flag for AC2
   reg   [31:0]                           sta_tx_status_flag_ac3_r;                       // Tx Status Flag for AC3
   reg   [5 :0]                           sta_ac0_count_r;                                // AC0 Packet Count   
   reg   [5 :0]                           sta_ac1_count_r;                                // AC1 Packet Count   
   reg   [5 :0]                           sta_ac2_count_r;                                // AC2 Packet Count   
   reg   [5 :0]                           sta_ac3_count_r;                                // AC3 Packet Count   
   reg   [11:0]                           sta_seqno_ac0_r[31:0];                           // Register to Sequence Numbers for AC0
   reg   [11:0]                           sta_seqno_ac1_r[31:0];                           // Register to Sequence Numbers for AC1
   reg   [11:0]                           sta_seqno_ac2_r[31:0];                           // Register to Sequence Numbers for AC2
   reg   [11:0]                           sta_seqno_ac3_r[31:0];                           // Register to Sequence Numbers for AC3
   reg   [7 :0]                           sta_retry_count_ac0_r[31:0];                     // Register to write retry count for AC0             
   reg   [7 :0]                           sta_retry_count_ac1_r[31:0];                     // Register to write retry count for AC1             
   reg   [7 :0]                           sta_retry_count_ac2_r[31:0];                     // Register to write retry count for AC2             
   reg   [7 :0]                           sta_retry_count_ac3_r[31:0];                     // Register to write retry count for AC3             
   reg                                    sta_status_ac0_r[31:0];                          // Register to write status for AC0
   reg                                    sta_status_ac1_r[31:0];                          // Register to write status for AC1
   reg                                    sta_status_ac2_r[31:0];                          // Register to write status for AC2
   reg                                    sta_status_ac3_r[31:0];                          // Register to write status for AC3
   reg   [31:0]                           sta_is_used_r;                                  // Registers to Store Valid STA Bitmap                                                    
   reg   [63:0]                           ba_is_used_r;                                   // Registers to Store Valid BA Bitmap
   reg   [2 :0]                           state;                                          // Current State register for FSM    
   reg   [2 :0]                           next_state;                                     // Next State register for FSM
   reg                                    mac_valid_r;                                    // MAC Address Valid Register
   reg   [MEM_WIDTH-1:0]                  ba_addr_r;                                      // BA Session Address register
   reg   [31:0]                           reg_data_r;                                     // Register to store the MAC address required 
   reg                                    sta_ack_r;                                      // STA Acknowledgement for the Read operation 
 reg                                    sta_clear_ampdu_r;  // Register to store the Clear AMPDU Pulse

reg [11:0]sta_in_tx_seqno_ac0_buff;
	reg [11:0]sta_in_tx_seqno_ac1_buff;
	reg [11:0]sta_in_tx_seqno_ac2_buff;
	reg [11:0]sta_in_tx_seqno_ac3_buff;
   // Wire Declarations
   wire                                   mem_enable;
   wire  [MEM_WIDTH-1:0]                  mem_addr;
   wire  [3 :0]                           mem_bea;
   wire  [31:0]                           mem_in_data;
   wire                                   access_memory;
   wire                                   retry;
   wire  [31:0]                           mem_out_data; 

   wire  [6 :0]                           rsvd2;
   wire  [3 :0]                           rsvd1;

   // Memory Enable Selection from CP and UMAC
   assign mem_enable = (( sta_in_val || sta_in_umac_val || ( state == COMPARE_ADDR ) || ( state == READ_ADDR )) && access_memory );               

   // Address Location Selection based on Input Enable
   assign mem_addr = ( state == COMPARE_ADDR ) ? (ba_addr_r - `STA00_INFO_OFFSET) : ( sta_in_val ?  sta_in_addr : (( sta_in_umac_addr - `STA00_INFO_OFFSET )>>2));

   // Memory Strobe Selection for Inputs from CP and UMAC
   assign mem_bea = sta_in_val ? sta_in_bea : ( sta_in_umac_bea & { 4{sta_in_umac_wrn_rd} } );

   // Input to the Memory from the External Inputs
   assign mem_in_data = sta_in_val ? sta_in_data : sta_in_umac_data;

    assign access_memory = sta_in_val ?  (((sta_in_addr<<2) >= `STA00_INFO_OFFSET)||( state == COMPARE_ADDR )) : (( sta_in_umac_addr >= `STA00_INFO_OFFSET ) && ( sta_in_umac_addr < `TX_STATUS_AC0_FLAG ));

   // Output Data Assignment to the UMAC
   assign sta_out_umac_data = access_memory ? mem_out_data : reg_data_r;

   // Output Data Assignment to the CP
   assign sta_out_data = sta_ack_r ? mem_out_data : 32'h0;

   // Condition for sending the SRC or LRC into retries.
   assign retry = ( sta_in_frame_length > sta_in_RTS_threshold ) ? 1'b1 : 1'b0; 

   // Assigning the Reserved fields
   assign rsvd1 = 4'h0;
   assign rsvd2 = 7'h0;

   // Single Port Block RAM for the STA and BA Memory
   blk_mem_gen_v6_2 U_MEM(
                          .clka              (sta_clk),
                          .ena               (mem_enable),
                          .wea               (mem_bea),
                          .addra             (mem_addr),
                          .dina              (mem_in_data),
                          .douta             (mem_out_data)
                         );

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Procedural Blocks ///////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

// Always Block to store STA_IS_USED bits for corresponding Stations.
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_is_used_r <= 32'h0;
      else if( !sta_enable )
         sta_is_used_r <= 32'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd &&
             ( sta_in_umac_addr == `STA_IS_USED_OFFSET )) 
      begin
         if( sta_in_umac_bea [0] )
            sta_is_used_r[7:0] <= sta_in_umac_data[7:0]; 
         if( sta_in_umac_bea [1] )
            sta_is_used_r[15:8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea [2] )
            sta_is_used_r[19:16] <= sta_in_umac_data[19:16];
      end
   end

// Always Block to store BA_IS_USED bits for corresponding Stations. 
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         ba_is_used_r <= 64'h0;
      else if( !sta_enable )
         ba_is_used_r <= 64'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd &&
             ( sta_in_umac_addr == `BA_IS_USED_OFFSET )) 
      begin
         if( sta_in_umac_bea [0] )
            ba_is_used_r[7:0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea [1] )
            ba_is_used_r[15:8] <= sta_in_umac_data[15:8];
         if( sta_in_umac_bea [2] )
            ba_is_used_r[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea [3] )
            ba_is_used_r[31:24] <= sta_in_umac_data[31:24];
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd &&
             ( sta_in_umac_addr == ( `BA_IS_USED_OFFSET + 14'h0004 ))) 
      begin
         if( sta_in_umac_bea [0] )
            ba_is_used_r[39:32] <= sta_in_umac_data[7:0];
      end
   end   

// Always Block to store the AMPDU Clear Pulse
   always @(posedge sta_clk or `EDGE_OF_RESET )
   begin	   
      if( `POLARITY_OF_RESET )
         sta_clear_ampdu_r <= 1'b0;
      else if( !sta_enable )
         sta_clear_ampdu_r <= 1'b0;
      else if( sta_clear_ampdu_r )
      begin
	 case( sta_in_ac_owner )
	    3'h0 : sta_clear_ampdu_r <= ( sta_ac0_count_r == sta_in_aggr_count ) ? 1'b0 : 1'b1;	 
            3'h1 : sta_clear_ampdu_r <= ( sta_ac1_count_r == sta_in_aggr_count ) ? 1'b0 : 1'b1;
	    3'h2 : sta_clear_ampdu_r <= ( sta_ac2_count_r == sta_in_aggr_count ) ? 1'b0 : 1'b1;
	    3'h3 : sta_clear_ampdu_r <= ( sta_ac3_count_r == sta_in_aggr_count ) ? 1'b0 : 1'b1;
	    default : sta_clear_ampdu_r <= 1'b0;
	 endcase 
      end	      
      else if( sta_in_clear_ampdu )	      
	 sta_clear_ampdu_r <= 1'b1;     
   end
// Always Block for Counting the AC0 Packets
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )begin
         sta_ac0_count_r <= 6'h0;
sta_in_tx_seqno_ac0_buff<=12'b0;
			sta_in_tx_seqno_ac1_buff<=12'b0;
			sta_in_tx_seqno_ac2_buff<=12'b0;
			sta_in_tx_seqno_ac3_buff<=12'b0;end
      else if( !sta_enable )begin
         sta_ac0_count_r <= 6'h0;   
sta_in_tx_seqno_ac0_buff<=12'b0;
			sta_in_tx_seqno_ac1_buff<=12'b0;
			sta_in_tx_seqno_ac2_buff<=12'b0;
			sta_in_tx_seqno_ac3_buff<=12'b0;end
     else if(( sta_in_clear_mpdu || sta_in_clear_ampdu ) &&
              ( sta_in_ac_owner == 3'h0 ))begin
         sta_ac0_count_r <= sta_ac0_count_r + 6'h1;
			sta_in_tx_seqno_ac0_buff<=sta_in_tx_seqno_ac0;
			sta_in_tx_seqno_ac1_buff<=sta_in_tx_seqno_ac1;
			sta_in_tx_seqno_ac2_buff<=sta_in_tx_seqno_ac2;
			sta_in_tx_seqno_ac3_buff<=sta_in_tx_seqno_ac3;end
   end

// Always Block to Store the Status Register for AC0
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_tx_status_flag_ac0_r <= 32'h0;
      else if( !sta_enable )
         sta_tx_status_flag_ac0_r <= 32'h0;
  else if( sta_in_clear_ampdu && ( sta_in_ac_owner == 3'h0 ))   
sta_tx_status_flag_ac0_r <= 32'h0;     
      else if( sta_clear_ampdu_r && ( sta_in_ac_owner == 3'h0 ) && 
	     ( sta_ac0_count_r < sta_in_aggr_count ))
         sta_tx_status_flag_ac0_r <= ( sta_tx_status_flag_ac0_r | 
		                     ( sta_in_bitmap_ac0 & ( 8'h1 << sta_ac0_count_r )));    
else if( sta_in_clear_mpdu && ( sta_in_ac_owner == 3'h0 ))
         sta_tx_status_flag_ac0_r <= ( sta_tx_status_flag_ac0_r | ( 32'h1 << sta_ac0_count_r ));
   end

// Always Block for Counting the AC1 Packets
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_ac1_count_r <= 5'h0;
      else if( !sta_enable )
         sta_ac1_count_r <= 5'h0;
else if( sta_in_clear_ampdu && ( sta_in_ac_owner == 3'h1 ))
	 sta_ac1_count_r <= 5'h0;
      else if( sta_clear_ampdu_r && ( sta_in_ac_owner == 3'h1 ) && 
	     ( sta_ac1_count_r < sta_in_aggr_count ))
         sta_ac1_count_r <= sta_ac1_count_r + 5'h1;
      else if( sta_in_clear_mpdu && ( sta_in_ac_owner == 3'h1 ))
         sta_ac1_count_r <= sta_ac1_count_r + 5'h1;
   end

// Always Block to Store the Status Register for AC1
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_tx_status_flag_ac1_r <= 32'h0;
      else if( !sta_enable )
         sta_tx_status_flag_ac1_r <= 32'h0;
      else if( sta_in_clear_ampdu && ( sta_in_ac_owner == 3'h1 ))
         sta_tx_status_flag_ac1_r <= 32'h0;
      else if( sta_clear_ampdu_r && ( sta_in_ac_owner == 3'h1 ) && 
	     ( sta_ac1_count_r < sta_in_aggr_count ))
         sta_tx_status_flag_ac1_r <= ( sta_tx_status_flag_ac1_r | 
		                     ( sta_in_bitmap_ac1 & ( 8'h1 << sta_ac1_count_r )));     
      else if( sta_in_clear_mpdu && ( sta_in_ac_owner == 3'h1 ))
         sta_tx_status_flag_ac1_r <= ( sta_tx_status_flag_ac1_r | ( 32'h1 << sta_ac1_count_r ));
   end

// Always Block for Counting the AC2 Packets
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_ac2_count_r <= 5'h0;
      else if( !sta_enable )
         sta_ac2_count_r <= 5'h0;
      else if( sta_in_clear_ampdu && ( sta_in_ac_owner == 3'h2 ))
	 sta_ac2_count_r <= 5'h0;
      else if( sta_clear_ampdu_r && ( sta_in_ac_owner == 3'h2 ) && 
	     ( sta_ac2_count_r < sta_in_aggr_count ))
         sta_ac2_count_r <= sta_ac2_count_r + 5'h1;
      else if( sta_in_clear_mpdu && ( sta_in_ac_owner == 3'h2 ))
         sta_ac2_count_r <= sta_ac2_count_r + 5'h1;
   end

// Always Block to Store the Status Register for AC2
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_tx_status_flag_ac2_r <= 32'h0;
      else if( !sta_enable )
         sta_tx_status_flag_ac2_r <= 32'h0;
      else if( sta_in_clear_ampdu && ( sta_in_ac_owner == 3'h2 ))
         sta_tx_status_flag_ac2_r <= 32'h0;
      else if( sta_clear_ampdu_r && ( sta_in_ac_owner == 3'h2 ) && 
	     ( sta_ac2_count_r < sta_in_aggr_count ))
         sta_tx_status_flag_ac2_r <= ( sta_tx_status_flag_ac2_r | 
		                     ( sta_in_bitmap_ac2 & ( 8'h1 << sta_ac2_count_r )));     
      else if( sta_in_clear_mpdu && ( sta_in_ac_owner == 3'h2 ))
         sta_tx_status_flag_ac2_r <= ( sta_tx_status_flag_ac2_r | ( 32'h1 << sta_ac2_count_r ));
   end

// Always Block for Counting the AC3 Packets
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_ac3_count_r <= 5'h0;
      else if( !sta_enable )
         sta_ac3_count_r <= 5'h0;
     else if( sta_in_clear_ampdu && ( sta_in_ac_owner == 3'h3 ))
	 sta_ac3_count_r <= 5'h0;
      else if( sta_clear_ampdu_r && ( sta_in_ac_owner == 3'h3 ) && 
	     ( sta_ac3_count_r < sta_in_aggr_count ))
         sta_ac3_count_r <= sta_ac3_count_r + 5'h1;
      else if( sta_in_clear_mpdu && ( sta_in_ac_owner == 3'h3 ))
         sta_ac3_count_r <= sta_ac3_count_r + 5'h1;
   end

// Always Block to Store the Status Register for AC3
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_tx_status_flag_ac3_r <= 32'h0;
      else if( !sta_enable )
         sta_tx_status_flag_ac3_r <= 32'h0;
      else if( sta_in_clear_ampdu && ( sta_in_ac_owner == 3'h3 ))
         sta_tx_status_flag_ac3_r <= 32'h0;
      else if( sta_clear_ampdu_r && ( sta_in_ac_owner == 3'h3 ) && 
	     ( sta_ac3_count_r < sta_in_aggr_count ))
         sta_tx_status_flag_ac3_r <= ( sta_tx_status_flag_ac3_r | 
		                     ( sta_in_bitmap_ac3 & ( 8'h1 << sta_ac3_count_r )));     
      else if( sta_in_clear_mpdu && ( sta_in_ac_owner == 3'h3 ))
         sta_tx_status_flag_ac3_r <= ( sta_tx_status_flag_ac3_r | ( 32'h1 << sta_ac3_count_r ));
   end

// Always Block for storing the Sequence Numbers for AC0
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_seqno_ac0_r[0]  <= 12'h0;
         sta_seqno_ac0_r[1]  <= 12'h0;
         sta_seqno_ac0_r[2]  <= 12'h0;
         sta_seqno_ac0_r[3]  <= 12'h0;
         sta_seqno_ac0_r[4]  <= 12'h0;
         sta_seqno_ac0_r[5]  <= 12'h0;
         sta_seqno_ac0_r[6]  <= 12'h0;
         sta_seqno_ac0_r[7]  <= 12'h0;
         sta_seqno_ac0_r[8]  <= 12'h0;
         sta_seqno_ac0_r[9]  <= 12'h0;
         sta_seqno_ac0_r[10] <= 12'h0;
         sta_seqno_ac0_r[11] <= 12'h0;
         sta_seqno_ac0_r[12] <= 12'h0;
         sta_seqno_ac0_r[13] <= 12'h0;
         sta_seqno_ac0_r[14] <= 12'h0;
         sta_seqno_ac0_r[15] <= 12'h0;
         sta_seqno_ac0_r[16] <= 12'h0;
         sta_seqno_ac0_r[17] <= 12'h0;
         sta_seqno_ac0_r[18] <= 12'h0;
         sta_seqno_ac0_r[19] <= 12'h0;
         sta_seqno_ac0_r[20] <= 12'h0;
         sta_seqno_ac0_r[21] <= 12'h0;
         sta_seqno_ac0_r[22] <= 12'h0;
         sta_seqno_ac0_r[23] <= 12'h0;
         sta_seqno_ac0_r[24] <= 12'h0;
         sta_seqno_ac0_r[25] <= 12'h0;
         sta_seqno_ac0_r[26] <= 12'h0;
         sta_seqno_ac0_r[27] <= 12'h0;
         sta_seqno_ac0_r[28] <= 12'h0;
         sta_seqno_ac0_r[29] <= 12'h0;
         sta_seqno_ac0_r[30] <= 12'h0;
         sta_seqno_ac0_r[31] <= 12'h0;
      end
      else if( !sta_enable )
      begin
         sta_seqno_ac0_r[0]  <= 12'h0;
         sta_seqno_ac0_r[1]  <= 12'h0;
         sta_seqno_ac0_r[2]  <= 12'h0;
         sta_seqno_ac0_r[3]  <= 12'h0;
         sta_seqno_ac0_r[4]  <= 12'h0;
         sta_seqno_ac0_r[5]  <= 12'h0;
         sta_seqno_ac0_r[6]  <= 12'h0;
         sta_seqno_ac0_r[7]  <= 12'h0;
         sta_seqno_ac0_r[8]  <= 12'h0;
         sta_seqno_ac0_r[9]  <= 12'h0;
         sta_seqno_ac0_r[10] <= 12'h0;
         sta_seqno_ac0_r[11] <= 12'h0;
         sta_seqno_ac0_r[12] <= 12'h0;
         sta_seqno_ac0_r[13] <= 12'h0;
         sta_seqno_ac0_r[14] <= 12'h0;
         sta_seqno_ac0_r[15] <= 12'h0;
         sta_seqno_ac0_r[16] <= 12'h0;
         sta_seqno_ac0_r[17] <= 12'h0;
         sta_seqno_ac0_r[18] <= 12'h0;
         sta_seqno_ac0_r[19] <= 12'h0;
         sta_seqno_ac0_r[20] <= 12'h0;
         sta_seqno_ac0_r[21] <= 12'h0;
         sta_seqno_ac0_r[22] <= 12'h0;
         sta_seqno_ac0_r[23] <= 12'h0;
         sta_seqno_ac0_r[24] <= 12'h0;
         sta_seqno_ac0_r[25] <= 12'h0;
         sta_seqno_ac0_r[26] <= 12'h0;
         sta_seqno_ac0_r[27] <= 12'h0;
         sta_seqno_ac0_r[28] <= 12'h0;
         sta_seqno_ac0_r[29] <= 12'h0;
         sta_seqno_ac0_r[30] <= 12'h0;
         sta_seqno_ac0_r[31] <= 12'h0;
      end
      else if( sta_in_ac_owner == 3'h0 )
      begin
          case( sta_ac0_count_r )
             5'h1    : sta_seqno_ac0_r[0]  <= sta_in_tx_seqno_ac0_buff;
             5'h2    : sta_seqno_ac0_r[1]  <= sta_in_tx_seqno_ac0_buff;
             5'h3   : sta_seqno_ac0_r[2]  <= sta_in_tx_seqno_ac0_buff;
             5'h4    : sta_seqno_ac0_r[3]  <= sta_in_tx_seqno_ac0_buff;
             5'h5    : sta_seqno_ac0_r[4]  <= sta_in_tx_seqno_ac0_buff;
             5'h6    : sta_seqno_ac0_r[5]  <= sta_in_tx_seqno_ac0_buff;
             5'h7    : sta_seqno_ac0_r[6]  <= sta_in_tx_seqno_ac0_buff;
             5'h8    : sta_seqno_ac0_r[7]  <= sta_in_tx_seqno_ac0_buff;
             5'h9    : sta_seqno_ac0_r[8]  <= sta_in_tx_seqno_ac0_buff;
             5'hA    : sta_seqno_ac0_r[9]  <= sta_in_tx_seqno_ac0_buff;
             5'hB    : sta_seqno_ac0_r[10] <= sta_in_tx_seqno_ac0_buff;
             5'hC    : sta_seqno_ac0_r[11] <= sta_in_tx_seqno_ac0_buff;
             5'hD    : sta_seqno_ac0_r[12] <= sta_in_tx_seqno_ac0_buff;
             5'hE    : sta_seqno_ac0_r[13] <= sta_in_tx_seqno_ac0_buff;
             5'hF    : sta_seqno_ac0_r[14] <= sta_in_tx_seqno_ac0_buff;
             5'h10    : sta_seqno_ac0_r[15] <= sta_in_tx_seqno_ac0_buff;
             5'h11   : sta_seqno_ac0_r[16] <= sta_in_tx_seqno_ac0_buff;
             5'h12   : sta_seqno_ac0_r[17] <= sta_in_tx_seqno_ac0_buff;
             5'h13   : sta_seqno_ac0_r[18] <= sta_in_tx_seqno_ac0_buff;
             5'h14   : sta_seqno_ac0_r[19] <= sta_in_tx_seqno_ac0_buff;
             5'h15   : sta_seqno_ac0_r[20] <= sta_in_tx_seqno_ac0_buff;
             5'h16   : sta_seqno_ac0_r[21] <= sta_in_tx_seqno_ac0_buff;
             5'h17   : sta_seqno_ac0_r[22] <= sta_in_tx_seqno_ac0_buff;
             5'h18   : sta_seqno_ac0_r[23] <= sta_in_tx_seqno_ac0_buff;
             5'h19   : sta_seqno_ac0_r[24] <= sta_in_tx_seqno_ac0_buff;
             5'h1A   : sta_seqno_ac0_r[25] <= sta_in_tx_seqno_ac0_buff;
             5'h1B   : sta_seqno_ac0_r[26] <= sta_in_tx_seqno_ac0_buff;
             5'h1C   : sta_seqno_ac0_r[27] <= sta_in_tx_seqno_ac0_buff;
             5'h1D   : sta_seqno_ac0_r[28] <= sta_in_tx_seqno_ac0_buff;
             5'h1E   : sta_seqno_ac0_r[29] <= sta_in_tx_seqno_ac0_buff;
             5'h1F   : sta_seqno_ac0_r[30] <= sta_in_tx_seqno_ac0_buff;
             5'h20   : sta_seqno_ac0_r[31] <= sta_in_tx_seqno_ac0_buff;

         endcase
      end      
   end

// Always Block for storing the Sequence Numbers for AC1
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_seqno_ac1_r[0]  <= 12'h0;
         sta_seqno_ac1_r[1]  <= 12'h0;
         sta_seqno_ac1_r[2]  <= 12'h0;
         sta_seqno_ac1_r[3]  <= 12'h0;
         sta_seqno_ac1_r[4]  <= 12'h0;
         sta_seqno_ac1_r[5]  <= 12'h0;
         sta_seqno_ac1_r[6]  <= 12'h0;
         sta_seqno_ac1_r[7]  <= 12'h0;
         sta_seqno_ac1_r[8]  <= 12'h0;
         sta_seqno_ac1_r[9]  <= 12'h0;
         sta_seqno_ac1_r[10] <= 12'h0;
         sta_seqno_ac1_r[11] <= 12'h0;
         sta_seqno_ac1_r[12] <= 12'h0;
         sta_seqno_ac1_r[13] <= 12'h0;
         sta_seqno_ac1_r[14] <= 12'h0;
         sta_seqno_ac1_r[15] <= 12'h0;
         sta_seqno_ac1_r[16] <= 12'h0;
         sta_seqno_ac1_r[17] <= 12'h0;
         sta_seqno_ac1_r[18] <= 12'h0;
         sta_seqno_ac1_r[19] <= 12'h0;
         sta_seqno_ac1_r[20] <= 12'h0;
         sta_seqno_ac1_r[21] <= 12'h0;
         sta_seqno_ac1_r[22] <= 12'h0;
         sta_seqno_ac1_r[23] <= 12'h0;
         sta_seqno_ac1_r[24] <= 12'h0;
         sta_seqno_ac1_r[25] <= 12'h0;
         sta_seqno_ac1_r[26] <= 12'h0;
         sta_seqno_ac1_r[27] <= 12'h0;
         sta_seqno_ac1_r[28] <= 12'h0;
         sta_seqno_ac1_r[29] <= 12'h0;
         sta_seqno_ac1_r[30] <= 12'h0;
         sta_seqno_ac1_r[31] <= 12'h0;
      end
      else if( !sta_enable )
      begin
         sta_seqno_ac1_r[0]  <= 12'h0;
         sta_seqno_ac1_r[1]  <= 12'h0;
         sta_seqno_ac1_r[2]  <= 12'h0;
         sta_seqno_ac1_r[3]  <= 12'h0;
         sta_seqno_ac1_r[4]  <= 12'h0;
         sta_seqno_ac1_r[5]  <= 12'h0;
         sta_seqno_ac1_r[6]  <= 12'h0;
         sta_seqno_ac1_r[7]  <= 12'h0;
         sta_seqno_ac1_r[8]  <= 12'h0;
         sta_seqno_ac1_r[9]  <= 12'h0;
         sta_seqno_ac1_r[10] <= 12'h0;
         sta_seqno_ac1_r[11] <= 12'h0;
         sta_seqno_ac1_r[12] <= 12'h0;
         sta_seqno_ac1_r[13] <= 12'h0;
         sta_seqno_ac1_r[14] <= 12'h0;
         sta_seqno_ac1_r[15] <= 12'h0;
         sta_seqno_ac1_r[16] <= 12'h0;
         sta_seqno_ac1_r[17] <= 12'h0;
         sta_seqno_ac1_r[18] <= 12'h0;
         sta_seqno_ac1_r[19] <= 12'h0;
         sta_seqno_ac1_r[20] <= 12'h0;
         sta_seqno_ac1_r[21] <= 12'h0;
         sta_seqno_ac1_r[22] <= 12'h0;
         sta_seqno_ac1_r[23] <= 12'h0;
         sta_seqno_ac1_r[24] <= 12'h0;
         sta_seqno_ac1_r[25] <= 12'h0;
         sta_seqno_ac1_r[26] <= 12'h0;
         sta_seqno_ac1_r[27] <= 12'h0;
         sta_seqno_ac1_r[28] <= 12'h0;
         sta_seqno_ac1_r[29] <= 12'h0;
         sta_seqno_ac1_r[30] <= 12'h0;
         sta_seqno_ac1_r[31] <= 12'h0;
      end
      else if( sta_in_ac_owner == 3'h1 )
      begin
          case( sta_ac1_count_r )
             5'h0    : sta_seqno_ac1_r[0]  <= sta_in_tx_seqno_ac1_buff;
             5'h1    : sta_seqno_ac1_r[1]  <= sta_in_tx_seqno_ac1_buff;
             5'h2    : sta_seqno_ac1_r[2]  <= sta_in_tx_seqno_ac1_buff;
             5'h3    : sta_seqno_ac1_r[3]  <= sta_in_tx_seqno_ac1_buff;
             5'h4    : sta_seqno_ac1_r[4]  <= sta_in_tx_seqno_ac1_buff;
             5'h5    : sta_seqno_ac1_r[5]  <= sta_in_tx_seqno_ac1_buff;
             5'h6    : sta_seqno_ac1_r[6]  <= sta_in_tx_seqno_ac1_buff;
             5'h7    : sta_seqno_ac1_r[7]  <= sta_in_tx_seqno_ac1_buff;
             5'h8    : sta_seqno_ac1_r[8]  <= sta_in_tx_seqno_ac1_buff;
             5'h9    : sta_seqno_ac1_r[9]  <= sta_in_tx_seqno_ac1_buff;
             5'hA    : sta_seqno_ac1_r[10] <= sta_in_tx_seqno_ac1_buff;
             5'hB    : sta_seqno_ac1_r[11] <= sta_in_tx_seqno_ac1_buff;
             5'hC    : sta_seqno_ac1_r[12] <= sta_in_tx_seqno_ac1_buff;
             5'hD    : sta_seqno_ac1_r[13] <= sta_in_tx_seqno_ac1_buff;
             5'hE    : sta_seqno_ac1_r[14] <= sta_in_tx_seqno_ac1_buff;
             5'hF    : sta_seqno_ac1_r[15] <= sta_in_tx_seqno_ac1_buff;
             5'h10   : sta_seqno_ac1_r[16] <= sta_in_tx_seqno_ac1_buff;
             5'h11   : sta_seqno_ac1_r[17] <= sta_in_tx_seqno_ac1_buff;
             5'h12   : sta_seqno_ac1_r[18] <= sta_in_tx_seqno_ac1_buff;
             5'h13   : sta_seqno_ac1_r[19] <= sta_in_tx_seqno_ac1_buff;
             5'h14   : sta_seqno_ac1_r[20] <= sta_in_tx_seqno_ac1_buff;
             5'h15   : sta_seqno_ac1_r[21] <= sta_in_tx_seqno_ac1_buff;
             5'h16   : sta_seqno_ac1_r[22] <= sta_in_tx_seqno_ac1_buff;
             5'h17   : sta_seqno_ac1_r[23] <= sta_in_tx_seqno_ac1_buff;
             5'h18   : sta_seqno_ac1_r[24] <= sta_in_tx_seqno_ac1_buff;
             5'h19   : sta_seqno_ac1_r[25] <= sta_in_tx_seqno_ac1_buff;
             5'h1A   : sta_seqno_ac1_r[26] <= sta_in_tx_seqno_ac1_buff;
             5'h1B   : sta_seqno_ac1_r[27] <= sta_in_tx_seqno_ac1_buff;
             5'h1C   : sta_seqno_ac1_r[28] <= sta_in_tx_seqno_ac1_buff;
             5'h1D   : sta_seqno_ac1_r[29] <= sta_in_tx_seqno_ac1_buff;
             5'h1E   : sta_seqno_ac1_r[30] <= sta_in_tx_seqno_ac1_buff;
             5'h1F   : sta_seqno_ac1_r[31] <= sta_in_tx_seqno_ac1_buff;

         endcase
      end      
   end

// Always Block for storing the Sequence Numbers for AC2
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_seqno_ac2_r[0]  <= 12'h0;
         sta_seqno_ac2_r[1]  <= 12'h0;
         sta_seqno_ac2_r[2]  <= 12'h0;
         sta_seqno_ac2_r[3]  <= 12'h0;
         sta_seqno_ac2_r[4]  <= 12'h0;
         sta_seqno_ac2_r[5]  <= 12'h0;
         sta_seqno_ac2_r[6]  <= 12'h0;
         sta_seqno_ac2_r[7]  <= 12'h0;
         sta_seqno_ac2_r[8]  <= 12'h0;
         sta_seqno_ac2_r[9]  <= 12'h0;
         sta_seqno_ac2_r[10] <= 12'h0;
         sta_seqno_ac2_r[11] <= 12'h0;
         sta_seqno_ac2_r[12] <= 12'h0;
         sta_seqno_ac2_r[13] <= 12'h0;
         sta_seqno_ac2_r[14] <= 12'h0;
         sta_seqno_ac2_r[15] <= 12'h0;
         sta_seqno_ac2_r[16] <= 12'h0;
         sta_seqno_ac2_r[17] <= 12'h0;
         sta_seqno_ac2_r[18] <= 12'h0;
         sta_seqno_ac2_r[19] <= 12'h0;
         sta_seqno_ac2_r[20] <= 12'h0;
         sta_seqno_ac2_r[21] <= 12'h0;
         sta_seqno_ac2_r[22] <= 12'h0;
         sta_seqno_ac2_r[23] <= 12'h0;
         sta_seqno_ac2_r[24] <= 12'h0;
         sta_seqno_ac2_r[25] <= 12'h0;
         sta_seqno_ac2_r[26] <= 12'h0;
         sta_seqno_ac2_r[27] <= 12'h0;
         sta_seqno_ac2_r[28] <= 12'h0;
         sta_seqno_ac2_r[29] <= 12'h0;
         sta_seqno_ac2_r[30] <= 12'h0;
         sta_seqno_ac2_r[31] <= 12'h0;
      end
      else if( !sta_enable )
      begin
         sta_seqno_ac2_r[0]  <= 12'h0;
         sta_seqno_ac2_r[1]  <= 12'h0;
         sta_seqno_ac2_r[2]  <= 12'h0;
         sta_seqno_ac2_r[3]  <= 12'h0;
         sta_seqno_ac2_r[4]  <= 12'h0;
         sta_seqno_ac2_r[5]  <= 12'h0;
         sta_seqno_ac2_r[6]  <= 12'h0;
         sta_seqno_ac2_r[7]  <= 12'h0;
         sta_seqno_ac2_r[8]  <= 12'h0;
         sta_seqno_ac2_r[9]  <= 12'h0;
         sta_seqno_ac2_r[10] <= 12'h0;
         sta_seqno_ac2_r[11] <= 12'h0;
         sta_seqno_ac2_r[12] <= 12'h0;
         sta_seqno_ac2_r[13] <= 12'h0;
         sta_seqno_ac2_r[14] <= 12'h0;
         sta_seqno_ac2_r[15] <= 12'h0;
         sta_seqno_ac2_r[16] <= 12'h0;
         sta_seqno_ac2_r[17] <= 12'h0;
         sta_seqno_ac2_r[18] <= 12'h0;
         sta_seqno_ac2_r[19] <= 12'h0;
         sta_seqno_ac2_r[20] <= 12'h0;
         sta_seqno_ac2_r[21] <= 12'h0;
         sta_seqno_ac2_r[22] <= 12'h0;
         sta_seqno_ac2_r[23] <= 12'h0;
         sta_seqno_ac2_r[24] <= 12'h0;
         sta_seqno_ac2_r[25] <= 12'h0;
         sta_seqno_ac2_r[26] <= 12'h0;
         sta_seqno_ac2_r[27] <= 12'h0;
         sta_seqno_ac2_r[28] <= 12'h0;
         sta_seqno_ac2_r[29] <= 12'h0;
         sta_seqno_ac2_r[30] <= 12'h0;
         sta_seqno_ac2_r[31] <= 12'h0;
      end
      else if( sta_in_ac_owner == 3'h2 )
      begin
          case( sta_ac2_count_r )
             5'h0    : sta_seqno_ac2_r[0]  <= sta_in_tx_seqno_ac2_buff;
             5'h1    : sta_seqno_ac2_r[1]  <= sta_in_tx_seqno_ac2_buff;
             5'h2    : sta_seqno_ac2_r[2]  <= sta_in_tx_seqno_ac2_buff;
             5'h3    : sta_seqno_ac2_r[3]  <= sta_in_tx_seqno_ac2_buff;
             5'h4    : sta_seqno_ac2_r[4]  <= sta_in_tx_seqno_ac2_buff;
             5'h5    : sta_seqno_ac2_r[5]  <= sta_in_tx_seqno_ac2_buff;
             5'h6    : sta_seqno_ac2_r[6]  <= sta_in_tx_seqno_ac2_buff;
             5'h7    : sta_seqno_ac2_r[7]  <= sta_in_tx_seqno_ac2_buff;
             5'h8    : sta_seqno_ac2_r[8]  <= sta_in_tx_seqno_ac2_buff;
             5'h9    : sta_seqno_ac2_r[9]  <= sta_in_tx_seqno_ac2_buff;
             5'hA    : sta_seqno_ac2_r[10] <= sta_in_tx_seqno_ac2_buff;
             5'hB    : sta_seqno_ac2_r[11] <= sta_in_tx_seqno_ac2_buff;
             5'hC    : sta_seqno_ac2_r[12] <= sta_in_tx_seqno_ac2_buff;
             5'hD    : sta_seqno_ac2_r[13] <= sta_in_tx_seqno_ac2_buff;
             5'hE    : sta_seqno_ac2_r[14] <= sta_in_tx_seqno_ac2_buff;
             5'hF    : sta_seqno_ac2_r[15] <= sta_in_tx_seqno_ac2_buff;
             5'h10   : sta_seqno_ac2_r[16] <= sta_in_tx_seqno_ac2_buff;
             5'h11   : sta_seqno_ac2_r[17] <= sta_in_tx_seqno_ac2_buff;
             5'h12   : sta_seqno_ac2_r[18] <= sta_in_tx_seqno_ac2_buff;
             5'h13   : sta_seqno_ac2_r[19] <= sta_in_tx_seqno_ac2_buff;
             5'h14   : sta_seqno_ac2_r[20] <= sta_in_tx_seqno_ac2_buff;
             5'h15   : sta_seqno_ac2_r[21] <= sta_in_tx_seqno_ac2_buff;
             5'h16   : sta_seqno_ac2_r[22] <= sta_in_tx_seqno_ac2_buff;
             5'h17   : sta_seqno_ac2_r[23] <= sta_in_tx_seqno_ac2_buff;
             5'h18   : sta_seqno_ac2_r[24] <= sta_in_tx_seqno_ac2_buff;
             5'h19   : sta_seqno_ac2_r[25] <= sta_in_tx_seqno_ac2_buff;
             5'h1A   : sta_seqno_ac2_r[26] <= sta_in_tx_seqno_ac2_buff;
             5'h1B   : sta_seqno_ac2_r[27] <= sta_in_tx_seqno_ac2_buff;
             5'h1C   : sta_seqno_ac2_r[28] <= sta_in_tx_seqno_ac2_buff;
             5'h1D   : sta_seqno_ac2_r[29] <= sta_in_tx_seqno_ac2_buff;
             5'h1E   : sta_seqno_ac2_r[30] <= sta_in_tx_seqno_ac2_buff;
             5'h1F   : sta_seqno_ac2_r[31] <= sta_in_tx_seqno_ac2_buff;
         endcase
      end      
   end

// Always Block for storing the Sequence Numbers for AC3
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_seqno_ac3_r[0]  <= 12'h0;
         sta_seqno_ac3_r[1]  <= 12'h0;
         sta_seqno_ac3_r[2]  <= 12'h0;
         sta_seqno_ac3_r[3]  <= 12'h0;
         sta_seqno_ac3_r[4]  <= 12'h0;
         sta_seqno_ac3_r[5]  <= 12'h0;
         sta_seqno_ac3_r[6]  <= 12'h0;
         sta_seqno_ac3_r[7]  <= 12'h0;
         sta_seqno_ac3_r[8]  <= 12'h0;
         sta_seqno_ac3_r[9]  <= 12'h0;
         sta_seqno_ac3_r[10] <= 12'h0;
         sta_seqno_ac3_r[11] <= 12'h0;
         sta_seqno_ac3_r[12] <= 12'h0;
         sta_seqno_ac3_r[13] <= 12'h0;
         sta_seqno_ac3_r[14] <= 12'h0;
         sta_seqno_ac3_r[15] <= 12'h0;
         sta_seqno_ac3_r[16] <= 12'h0;
         sta_seqno_ac3_r[17] <= 12'h0;
         sta_seqno_ac3_r[18] <= 12'h0;
         sta_seqno_ac3_r[19] <= 12'h0;
         sta_seqno_ac3_r[20] <= 12'h0;
         sta_seqno_ac3_r[21] <= 12'h0;
         sta_seqno_ac3_r[22] <= 12'h0;
         sta_seqno_ac3_r[23] <= 12'h0;
         sta_seqno_ac3_r[24] <= 12'h0;
         sta_seqno_ac3_r[25] <= 12'h0;
         sta_seqno_ac3_r[26] <= 12'h0;
         sta_seqno_ac3_r[27] <= 12'h0;
         sta_seqno_ac3_r[28] <= 12'h0;
         sta_seqno_ac3_r[29] <= 12'h0;
         sta_seqno_ac3_r[30] <= 12'h0;
         sta_seqno_ac3_r[31] <= 12'h0;
      end
      else if( !sta_enable )
      begin
         sta_seqno_ac3_r[0]  <= 12'h0;
         sta_seqno_ac3_r[1]  <= 12'h0;
         sta_seqno_ac3_r[2]  <= 12'h0;
         sta_seqno_ac3_r[3]  <= 12'h0;
         sta_seqno_ac3_r[4]  <= 12'h0;
         sta_seqno_ac3_r[5]  <= 12'h0;
         sta_seqno_ac3_r[6]  <= 12'h0;
         sta_seqno_ac3_r[7]  <= 12'h0;
         sta_seqno_ac3_r[8]  <= 12'h0;
         sta_seqno_ac3_r[9]  <= 12'h0;
         sta_seqno_ac3_r[10] <= 12'h0;
         sta_seqno_ac3_r[11] <= 12'h0;
         sta_seqno_ac3_r[12] <= 12'h0;
         sta_seqno_ac3_r[13] <= 12'h0;
         sta_seqno_ac3_r[14] <= 12'h0;
         sta_seqno_ac3_r[15] <= 12'h0;
         sta_seqno_ac3_r[16] <= 12'h0;
         sta_seqno_ac3_r[17] <= 12'h0;
         sta_seqno_ac3_r[18] <= 12'h0;
         sta_seqno_ac3_r[19] <= 12'h0;
         sta_seqno_ac3_r[20] <= 12'h0;
         sta_seqno_ac3_r[21] <= 12'h0;
         sta_seqno_ac3_r[22] <= 12'h0;
         sta_seqno_ac3_r[23] <= 12'h0;
         sta_seqno_ac3_r[24] <= 12'h0;
         sta_seqno_ac3_r[25] <= 12'h0;
         sta_seqno_ac3_r[26] <= 12'h0;
         sta_seqno_ac3_r[27] <= 12'h0;
         sta_seqno_ac3_r[28] <= 12'h0;
         sta_seqno_ac3_r[29] <= 12'h0;
         sta_seqno_ac3_r[30] <= 12'h0;
         sta_seqno_ac3_r[31] <= 12'h0;
      end
      else if( sta_in_ac_owner == 3'h3 )
      begin
          case( sta_ac3_count_r )
             5'h0    : sta_seqno_ac3_r[0]  <= sta_in_tx_seqno_ac3_buff;
             5'h1    : sta_seqno_ac3_r[1]  <= sta_in_tx_seqno_ac3_buff;
             5'h2    : sta_seqno_ac3_r[2]  <= sta_in_tx_seqno_ac3_buff;
             5'h3    : sta_seqno_ac3_r[3]  <= sta_in_tx_seqno_ac3_buff;
             5'h4    : sta_seqno_ac3_r[4]  <= sta_in_tx_seqno_ac3_buff;
             5'h5    : sta_seqno_ac3_r[5]  <= sta_in_tx_seqno_ac3_buff;
             5'h6    : sta_seqno_ac3_r[6]  <= sta_in_tx_seqno_ac3_buff;
             5'h7    : sta_seqno_ac3_r[7]  <= sta_in_tx_seqno_ac3_buff;
             5'h8    : sta_seqno_ac3_r[8]  <= sta_in_tx_seqno_ac3_buff;
             5'h9    : sta_seqno_ac3_r[9]  <= sta_in_tx_seqno_ac3_buff;
             5'hA    : sta_seqno_ac3_r[10] <= sta_in_tx_seqno_ac3_buff;
             5'hB    : sta_seqno_ac3_r[11] <= sta_in_tx_seqno_ac3_buff;
             5'hC    : sta_seqno_ac3_r[12] <= sta_in_tx_seqno_ac3_buff;
             5'hD    : sta_seqno_ac3_r[13] <= sta_in_tx_seqno_ac3_buff;
             5'hE    : sta_seqno_ac3_r[14] <= sta_in_tx_seqno_ac3_buff;
             5'hF    : sta_seqno_ac3_r[15] <= sta_in_tx_seqno_ac3_buff;
             5'h10   : sta_seqno_ac3_r[16] <= sta_in_tx_seqno_ac3_buff;
             5'h11   : sta_seqno_ac3_r[17] <= sta_in_tx_seqno_ac3_buff;
             5'h12   : sta_seqno_ac3_r[18] <= sta_in_tx_seqno_ac3_buff;
             5'h13   : sta_seqno_ac3_r[19] <= sta_in_tx_seqno_ac3_buff;
             5'h14   : sta_seqno_ac3_r[20] <= sta_in_tx_seqno_ac3_buff;
             5'h15   : sta_seqno_ac3_r[21] <= sta_in_tx_seqno_ac3_buff;
             5'h16   : sta_seqno_ac3_r[22] <= sta_in_tx_seqno_ac3_buff;
             5'h17   : sta_seqno_ac3_r[23] <= sta_in_tx_seqno_ac3_buff;
             5'h18   : sta_seqno_ac3_r[24] <= sta_in_tx_seqno_ac3_buff;
             5'h19   : sta_seqno_ac3_r[25] <= sta_in_tx_seqno_ac3_buff;
             5'h1A   : sta_seqno_ac3_r[26] <= sta_in_tx_seqno_ac3_buff;
             5'h1B   : sta_seqno_ac3_r[27] <= sta_in_tx_seqno_ac3_buff;
             5'h1C   : sta_seqno_ac3_r[28] <= sta_in_tx_seqno_ac3_buff;
             5'h1D   : sta_seqno_ac3_r[29] <= sta_in_tx_seqno_ac3_buff;
             5'h1E   : sta_seqno_ac3_r[30] <= sta_in_tx_seqno_ac3_buff;
             5'h1F   : sta_seqno_ac3_r[31] <= sta_in_tx_seqno_ac3_buff;
         endcase
      end      
   end

// Always Block for storing the Retry Count for AC0
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_retry_count_ac0_r[0]  <= 8'h0;
         sta_retry_count_ac0_r[1]  <= 8'h0;
         sta_retry_count_ac0_r[2]  <= 8'h0;
         sta_retry_count_ac0_r[3]  <= 8'h0;
         sta_retry_count_ac0_r[4]  <= 8'h0;
         sta_retry_count_ac0_r[5]  <= 8'h0;
         sta_retry_count_ac0_r[6]  <= 8'h0;
         sta_retry_count_ac0_r[7]  <= 8'h0;
         sta_retry_count_ac0_r[8]  <= 8'h0;
         sta_retry_count_ac0_r[9]  <= 8'h0;
         sta_retry_count_ac0_r[10] <= 8'h0;
         sta_retry_count_ac0_r[11] <= 8'h0;
         sta_retry_count_ac0_r[12] <= 8'h0;
         sta_retry_count_ac0_r[13] <= 8'h0;
         sta_retry_count_ac0_r[14] <= 8'h0;
         sta_retry_count_ac0_r[15] <= 8'h0;
         sta_retry_count_ac0_r[16] <= 8'h0;
         sta_retry_count_ac0_r[17] <= 8'h0;
         sta_retry_count_ac0_r[18] <= 8'h0;
         sta_retry_count_ac0_r[19] <= 8'h0;
         sta_retry_count_ac0_r[20] <= 8'h0;
         sta_retry_count_ac0_r[21] <= 8'h0;
         sta_retry_count_ac0_r[22] <= 8'h0;
         sta_retry_count_ac0_r[23] <= 8'h0;
         sta_retry_count_ac0_r[24] <= 8'h0;
         sta_retry_count_ac0_r[25] <= 8'h0;
         sta_retry_count_ac0_r[26] <= 8'h0;
         sta_retry_count_ac0_r[27] <= 8'h0;
         sta_retry_count_ac0_r[28] <= 8'h0;
         sta_retry_count_ac0_r[29] <= 8'h0;
         sta_retry_count_ac0_r[30] <= 8'h0;
         sta_retry_count_ac0_r[31] <= 8'h0;
      end
      else if( !sta_enable )
      begin
         sta_retry_count_ac0_r[0]  <= 8'h0;
         sta_retry_count_ac0_r[1]  <= 8'h0;
         sta_retry_count_ac0_r[2]  <= 8'h0;
         sta_retry_count_ac0_r[3]  <= 8'h0;
         sta_retry_count_ac0_r[4]  <= 8'h0;
         sta_retry_count_ac0_r[5]  <= 8'h0;
         sta_retry_count_ac0_r[6]  <= 8'h0;
         sta_retry_count_ac0_r[7]  <= 8'h0;
         sta_retry_count_ac0_r[8]  <= 8'h0;
         sta_retry_count_ac0_r[9]  <= 8'h0;
         sta_retry_count_ac0_r[10] <= 8'h0;
         sta_retry_count_ac0_r[11] <= 8'h0;
         sta_retry_count_ac0_r[12] <= 8'h0;
         sta_retry_count_ac0_r[13] <= 8'h0;
         sta_retry_count_ac0_r[14] <= 8'h0;
         sta_retry_count_ac0_r[15] <= 8'h0;
         sta_retry_count_ac0_r[16] <= 8'h0;
         sta_retry_count_ac0_r[17] <= 8'h0;
         sta_retry_count_ac0_r[18] <= 8'h0;
         sta_retry_count_ac0_r[19] <= 8'h0;
         sta_retry_count_ac0_r[20] <= 8'h0;
         sta_retry_count_ac0_r[21] <= 8'h0;
         sta_retry_count_ac0_r[22] <= 8'h0;
         sta_retry_count_ac0_r[23] <= 8'h0;
         sta_retry_count_ac0_r[24] <= 8'h0;
         sta_retry_count_ac0_r[25] <= 8'h0;
         sta_retry_count_ac0_r[26] <= 8'h0;
         sta_retry_count_ac0_r[27] <= 8'h0;
         sta_retry_count_ac0_r[28] <= 8'h0;
         sta_retry_count_ac0_r[29] <= 8'h0;
         sta_retry_count_ac0_r[30] <= 8'h0;
         sta_retry_count_ac0_r[31] <= 8'h0;
      end
      else if( sta_in_ac_owner == 3'h0 )
      begin
          if( retry )
          begin
             case( sta_ac0_count_r )
                5'h0    : sta_retry_count_ac0_r[0]  <= sta_in_ac0_lrc;
                5'h1    : sta_retry_count_ac0_r[1]  <= sta_in_ac0_lrc;
                5'h2    : sta_retry_count_ac0_r[2]  <= sta_in_ac0_lrc;
                5'h3    : sta_retry_count_ac0_r[3]  <= sta_in_ac0_lrc;
                5'h4    : sta_retry_count_ac0_r[4]  <= sta_in_ac0_lrc;
                5'h5    : sta_retry_count_ac0_r[5]  <= sta_in_ac0_lrc;
                5'h6    : sta_retry_count_ac0_r[6]  <= sta_in_ac0_lrc;
                5'h7    : sta_retry_count_ac0_r[7]  <= sta_in_ac0_lrc;
                5'h8    : sta_retry_count_ac0_r[8]  <= sta_in_ac0_lrc;
                5'h9    : sta_retry_count_ac0_r[9]  <= sta_in_ac0_lrc;
                5'hA    : sta_retry_count_ac0_r[10] <= sta_in_ac0_lrc;
                5'hB    : sta_retry_count_ac0_r[11] <= sta_in_ac0_lrc;
                5'hC    : sta_retry_count_ac0_r[12] <= sta_in_ac0_lrc;
                5'hD    : sta_retry_count_ac0_r[13] <= sta_in_ac0_lrc;
                5'hE    : sta_retry_count_ac0_r[14] <= sta_in_ac0_lrc;
                5'hF    : sta_retry_count_ac0_r[15] <= sta_in_ac0_lrc;
                5'h10   : sta_retry_count_ac0_r[16] <= sta_in_ac0_lrc;
                5'h11   : sta_retry_count_ac0_r[17] <= sta_in_ac0_lrc;
                5'h12   : sta_retry_count_ac0_r[18] <= sta_in_ac0_lrc;
                5'h13   : sta_retry_count_ac0_r[19] <= sta_in_ac0_lrc;
                5'h14   : sta_retry_count_ac0_r[20] <= sta_in_ac0_lrc;
                5'h15   : sta_retry_count_ac0_r[21] <= sta_in_ac0_lrc;
                5'h16   : sta_retry_count_ac0_r[22] <= sta_in_ac0_lrc;
                5'h17   : sta_retry_count_ac0_r[23] <= sta_in_ac0_lrc;
                5'h18   : sta_retry_count_ac0_r[24] <= sta_in_ac0_lrc;
                5'h19   : sta_retry_count_ac0_r[25] <= sta_in_ac0_lrc;
                5'h1A   : sta_retry_count_ac0_r[26] <= sta_in_ac0_lrc;
                5'h1B   : sta_retry_count_ac0_r[27] <= sta_in_ac0_lrc;
                5'h1C   : sta_retry_count_ac0_r[28] <= sta_in_ac0_lrc;
                5'h1D   : sta_retry_count_ac0_r[29] <= sta_in_ac0_lrc;
                5'h1E   : sta_retry_count_ac0_r[30] <= sta_in_ac0_lrc;
                5'h1F   : sta_retry_count_ac0_r[31] <= sta_in_ac0_lrc;
            endcase
         end   
         else
         begin
             case( sta_ac0_count_r )
                5'h0    : sta_retry_count_ac0_r[0]  <= sta_in_ac0_src;
                5'h1    : sta_retry_count_ac0_r[1]  <= sta_in_ac0_src;
                5'h2    : sta_retry_count_ac0_r[2]  <= sta_in_ac0_src;
                5'h3    : sta_retry_count_ac0_r[3]  <= sta_in_ac0_src;
                5'h4    : sta_retry_count_ac0_r[4]  <= sta_in_ac0_src;
                5'h5    : sta_retry_count_ac0_r[5]  <= sta_in_ac0_src;
                5'h6    : sta_retry_count_ac0_r[6]  <= sta_in_ac0_src;
                5'h7    : sta_retry_count_ac0_r[7]  <= sta_in_ac0_src;
                5'h8    : sta_retry_count_ac0_r[8]  <= sta_in_ac0_src;
                5'h9    : sta_retry_count_ac0_r[9]  <= sta_in_ac0_src;
                5'hA    : sta_retry_count_ac0_r[10] <= sta_in_ac0_src;
                5'hB    : sta_retry_count_ac0_r[11] <= sta_in_ac0_src;
                5'hC    : sta_retry_count_ac0_r[12] <= sta_in_ac0_src;
                5'hD    : sta_retry_count_ac0_r[13] <= sta_in_ac0_src;
                5'hE    : sta_retry_count_ac0_r[14] <= sta_in_ac0_src;
                5'hF    : sta_retry_count_ac0_r[15] <= sta_in_ac0_src;
                5'h10   : sta_retry_count_ac0_r[16] <= sta_in_ac0_src;
                5'h11   : sta_retry_count_ac0_r[17] <= sta_in_ac0_src;
                5'h12   : sta_retry_count_ac0_r[18] <= sta_in_ac0_src;
                5'h13   : sta_retry_count_ac0_r[19] <= sta_in_ac0_src;
                5'h14   : sta_retry_count_ac0_r[20] <= sta_in_ac0_src;
                5'h15   : sta_retry_count_ac0_r[21] <= sta_in_ac0_src;
                5'h16   : sta_retry_count_ac0_r[22] <= sta_in_ac0_src;
                5'h17   : sta_retry_count_ac0_r[23] <= sta_in_ac0_src;
                5'h18   : sta_retry_count_ac0_r[24] <= sta_in_ac0_src;
                5'h19   : sta_retry_count_ac0_r[25] <= sta_in_ac0_src;
                5'h1A   : sta_retry_count_ac0_r[26] <= sta_in_ac0_src;
                5'h1B   : sta_retry_count_ac0_r[27] <= sta_in_ac0_src;
                5'h1C   : sta_retry_count_ac0_r[28] <= sta_in_ac0_src;
                5'h1D   : sta_retry_count_ac0_r[29] <= sta_in_ac0_src;
                5'h1E   : sta_retry_count_ac0_r[30] <= sta_in_ac0_src;
                5'h1F   : sta_retry_count_ac0_r[31] <= sta_in_ac0_src;
            endcase
         end
      end      
   end

// Always Block for storing the Retry Count for AC1
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_retry_count_ac1_r[0]  <= 8'h0;
         sta_retry_count_ac1_r[1]  <= 8'h0;
         sta_retry_count_ac1_r[2]  <= 8'h0;
         sta_retry_count_ac1_r[3]  <= 8'h0;
         sta_retry_count_ac1_r[4]  <= 8'h0;
         sta_retry_count_ac1_r[5]  <= 8'h0;
         sta_retry_count_ac1_r[6]  <= 8'h0;
         sta_retry_count_ac1_r[7]  <= 8'h0;
         sta_retry_count_ac1_r[8]  <= 8'h0;
         sta_retry_count_ac1_r[9]  <= 8'h0;
         sta_retry_count_ac1_r[10] <= 8'h0;
         sta_retry_count_ac1_r[11] <= 8'h0;
         sta_retry_count_ac1_r[12] <= 8'h0;
         sta_retry_count_ac1_r[13] <= 8'h0;
         sta_retry_count_ac1_r[14] <= 8'h0;
         sta_retry_count_ac1_r[15] <= 8'h0;
         sta_retry_count_ac1_r[16] <= 8'h0;
         sta_retry_count_ac1_r[17] <= 8'h0;
         sta_retry_count_ac1_r[18] <= 8'h0;
         sta_retry_count_ac1_r[19] <= 8'h0;
         sta_retry_count_ac1_r[20] <= 8'h0;
         sta_retry_count_ac1_r[21] <= 8'h0;
         sta_retry_count_ac1_r[22] <= 8'h0;
         sta_retry_count_ac1_r[23] <= 8'h0;
         sta_retry_count_ac1_r[24] <= 8'h0;
         sta_retry_count_ac1_r[25] <= 8'h0;
         sta_retry_count_ac1_r[26] <= 8'h0;
         sta_retry_count_ac1_r[27] <= 8'h0;
         sta_retry_count_ac1_r[28] <= 8'h0;
         sta_retry_count_ac1_r[29] <= 8'h0;
         sta_retry_count_ac1_r[30] <= 8'h0;
         sta_retry_count_ac1_r[31] <= 8'h0;
      end
      else if( !sta_enable )
      begin
         sta_retry_count_ac1_r[0]  <= 8'h0;
         sta_retry_count_ac1_r[1]  <= 8'h0;
         sta_retry_count_ac1_r[2]  <= 8'h0;
         sta_retry_count_ac1_r[3]  <= 8'h0;
         sta_retry_count_ac1_r[4]  <= 8'h0;
         sta_retry_count_ac1_r[5]  <= 8'h0;
         sta_retry_count_ac1_r[6]  <= 8'h0;
         sta_retry_count_ac1_r[7]  <= 8'h0;
         sta_retry_count_ac1_r[8]  <= 8'h0;
         sta_retry_count_ac1_r[9]  <= 8'h0;
         sta_retry_count_ac1_r[10] <= 8'h0;
         sta_retry_count_ac1_r[11] <= 8'h0;
         sta_retry_count_ac1_r[12] <= 8'h0;
         sta_retry_count_ac1_r[13] <= 8'h0;
         sta_retry_count_ac1_r[14] <= 8'h0;
         sta_retry_count_ac1_r[15] <= 8'h0;
         sta_retry_count_ac1_r[16] <= 8'h0;
         sta_retry_count_ac1_r[17] <= 8'h0;
         sta_retry_count_ac1_r[18] <= 8'h0;
         sta_retry_count_ac1_r[19] <= 8'h0;
         sta_retry_count_ac1_r[20] <= 8'h0;
         sta_retry_count_ac1_r[21] <= 8'h0;
         sta_retry_count_ac1_r[22] <= 8'h0;
         sta_retry_count_ac1_r[23] <= 8'h0;
         sta_retry_count_ac1_r[24] <= 8'h0;
         sta_retry_count_ac1_r[25] <= 8'h0;
         sta_retry_count_ac1_r[26] <= 8'h0;
         sta_retry_count_ac1_r[27] <= 8'h0;
         sta_retry_count_ac1_r[28] <= 8'h0;
         sta_retry_count_ac1_r[29] <= 8'h0;
         sta_retry_count_ac1_r[30] <= 8'h0;
         sta_retry_count_ac1_r[31] <= 8'h0;
      end
      else if( sta_in_ac_owner == 3'h1 )
      begin
          if( retry )
          begin
             case( sta_ac1_count_r )
                5'h0    : sta_retry_count_ac1_r[0]  <= sta_in_ac1_lrc;
                5'h1    : sta_retry_count_ac1_r[1]  <= sta_in_ac1_lrc;
                5'h2    : sta_retry_count_ac1_r[2]  <= sta_in_ac1_lrc;
                5'h3    : sta_retry_count_ac1_r[3]  <= sta_in_ac1_lrc;
                5'h4    : sta_retry_count_ac1_r[4]  <= sta_in_ac1_lrc;
                5'h5    : sta_retry_count_ac1_r[5]  <= sta_in_ac1_lrc;
                5'h6    : sta_retry_count_ac1_r[6]  <= sta_in_ac1_lrc;
                5'h7    : sta_retry_count_ac1_r[7]  <= sta_in_ac1_lrc;
                5'h8    : sta_retry_count_ac1_r[8]  <= sta_in_ac1_lrc;
                5'h9    : sta_retry_count_ac1_r[9]  <= sta_in_ac1_lrc;
                5'hA    : sta_retry_count_ac1_r[10] <= sta_in_ac1_lrc;
                5'hB    : sta_retry_count_ac1_r[11] <= sta_in_ac1_lrc;
                5'hC    : sta_retry_count_ac1_r[12] <= sta_in_ac1_lrc;
                5'hD    : sta_retry_count_ac1_r[13] <= sta_in_ac1_lrc;
                5'hE    : sta_retry_count_ac1_r[14] <= sta_in_ac1_lrc;
                5'hF    : sta_retry_count_ac1_r[15] <= sta_in_ac1_lrc;
                5'h10   : sta_retry_count_ac1_r[16] <= sta_in_ac1_lrc;
                5'h11   : sta_retry_count_ac1_r[17] <= sta_in_ac1_lrc;
                5'h12   : sta_retry_count_ac1_r[18] <= sta_in_ac1_lrc;
                5'h13   : sta_retry_count_ac1_r[19] <= sta_in_ac1_lrc;
                5'h14   : sta_retry_count_ac1_r[20] <= sta_in_ac1_lrc;
                5'h15   : sta_retry_count_ac1_r[21] <= sta_in_ac1_lrc;
                5'h16   : sta_retry_count_ac1_r[22] <= sta_in_ac1_lrc;
                5'h17   : sta_retry_count_ac1_r[23] <= sta_in_ac1_lrc;
                5'h18   : sta_retry_count_ac1_r[24] <= sta_in_ac1_lrc;
                5'h19   : sta_retry_count_ac1_r[25] <= sta_in_ac1_lrc;
                5'h1A   : sta_retry_count_ac1_r[26] <= sta_in_ac1_lrc;
                5'h1B   : sta_retry_count_ac1_r[27] <= sta_in_ac1_lrc;
                5'h1C   : sta_retry_count_ac1_r[28] <= sta_in_ac1_lrc;
                5'h1D   : sta_retry_count_ac1_r[29] <= sta_in_ac1_lrc;
                5'h1E   : sta_retry_count_ac1_r[30] <= sta_in_ac1_lrc;
                5'h1F   : sta_retry_count_ac1_r[31] <= sta_in_ac1_lrc;
            endcase
         end   
         else
         begin
             case( sta_ac1_count_r )
                5'h0    : sta_retry_count_ac1_r[0]  <= sta_in_ac1_src;
                5'h1    : sta_retry_count_ac1_r[1]  <= sta_in_ac1_src;
                5'h2    : sta_retry_count_ac1_r[2]  <= sta_in_ac1_src;
                5'h3    : sta_retry_count_ac1_r[3]  <= sta_in_ac1_src;
                5'h4    : sta_retry_count_ac1_r[4]  <= sta_in_ac1_src;
                5'h5    : sta_retry_count_ac1_r[5]  <= sta_in_ac1_src;
                5'h6    : sta_retry_count_ac1_r[6]  <= sta_in_ac1_src;
                5'h7    : sta_retry_count_ac1_r[7]  <= sta_in_ac1_src;
                5'h8    : sta_retry_count_ac1_r[8]  <= sta_in_ac1_src;
                5'h9    : sta_retry_count_ac1_r[9]  <= sta_in_ac1_src;
                5'hA    : sta_retry_count_ac1_r[10] <= sta_in_ac1_src;
                5'hB    : sta_retry_count_ac1_r[11] <= sta_in_ac1_src;
                5'hC    : sta_retry_count_ac1_r[12] <= sta_in_ac1_src;
                5'hD    : sta_retry_count_ac1_r[13] <= sta_in_ac1_src;
                5'hE    : sta_retry_count_ac1_r[14] <= sta_in_ac1_src;
                5'hF    : sta_retry_count_ac1_r[15] <= sta_in_ac1_src;
                5'h10   : sta_retry_count_ac1_r[16] <= sta_in_ac1_src;
                5'h11   : sta_retry_count_ac1_r[17] <= sta_in_ac1_src;
                5'h12   : sta_retry_count_ac1_r[18] <= sta_in_ac1_src;
                5'h13   : sta_retry_count_ac1_r[19] <= sta_in_ac1_src;
                5'h14   : sta_retry_count_ac1_r[20] <= sta_in_ac1_src;
                5'h15   : sta_retry_count_ac1_r[21] <= sta_in_ac1_src;
                5'h16   : sta_retry_count_ac1_r[22] <= sta_in_ac1_src;
                5'h17   : sta_retry_count_ac1_r[23] <= sta_in_ac1_src;
                5'h18   : sta_retry_count_ac1_r[24] <= sta_in_ac1_src;
                5'h19   : sta_retry_count_ac1_r[25] <= sta_in_ac1_src;
                5'h1A   : sta_retry_count_ac1_r[26] <= sta_in_ac1_src;
                5'h1B   : sta_retry_count_ac1_r[27] <= sta_in_ac1_src;
                5'h1C   : sta_retry_count_ac1_r[28] <= sta_in_ac1_src;
                5'h1D   : sta_retry_count_ac1_r[29] <= sta_in_ac1_src;
                5'h1E   : sta_retry_count_ac1_r[30] <= sta_in_ac1_src;
                5'h1F   : sta_retry_count_ac1_r[31] <= sta_in_ac1_src;
            endcase
         end
      end      
   end

// Always Block for storing the Retry Count for AC2
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_retry_count_ac2_r[0]  <= 8'h0;
         sta_retry_count_ac2_r[1]  <= 8'h0;
         sta_retry_count_ac2_r[2]  <= 8'h0;
         sta_retry_count_ac2_r[3]  <= 8'h0;
         sta_retry_count_ac2_r[4]  <= 8'h0;
         sta_retry_count_ac2_r[5]  <= 8'h0;
         sta_retry_count_ac2_r[6]  <= 8'h0;
         sta_retry_count_ac2_r[7]  <= 8'h0;
         sta_retry_count_ac2_r[8]  <= 8'h0;
         sta_retry_count_ac2_r[9]  <= 8'h0;
         sta_retry_count_ac2_r[10] <= 8'h0;
         sta_retry_count_ac2_r[11] <= 8'h0;
         sta_retry_count_ac2_r[12] <= 8'h0;
         sta_retry_count_ac2_r[13] <= 8'h0;
         sta_retry_count_ac2_r[14] <= 8'h0;
         sta_retry_count_ac2_r[15] <= 8'h0;
         sta_retry_count_ac2_r[16] <= 8'h0;
         sta_retry_count_ac2_r[17] <= 8'h0;
         sta_retry_count_ac2_r[18] <= 8'h0;
         sta_retry_count_ac2_r[19] <= 8'h0;
         sta_retry_count_ac2_r[20] <= 8'h0;
         sta_retry_count_ac2_r[21] <= 8'h0;
         sta_retry_count_ac2_r[22] <= 8'h0;
         sta_retry_count_ac2_r[23] <= 8'h0;
         sta_retry_count_ac2_r[24] <= 8'h0;
         sta_retry_count_ac2_r[25] <= 8'h0;
         sta_retry_count_ac2_r[26] <= 8'h0;
         sta_retry_count_ac2_r[27] <= 8'h0;
         sta_retry_count_ac2_r[28] <= 8'h0;
         sta_retry_count_ac2_r[29] <= 8'h0;
         sta_retry_count_ac2_r[30] <= 8'h0;
         sta_retry_count_ac2_r[31] <= 8'h0;
      end
      else if( !sta_enable )
      begin
         sta_retry_count_ac2_r[0]  <= 8'h0;
         sta_retry_count_ac2_r[1]  <= 8'h0;
         sta_retry_count_ac2_r[2]  <= 8'h0;
         sta_retry_count_ac2_r[3]  <= 8'h0;
         sta_retry_count_ac2_r[4]  <= 8'h0;
         sta_retry_count_ac2_r[5]  <= 8'h0;
         sta_retry_count_ac2_r[6]  <= 8'h0;
         sta_retry_count_ac2_r[7]  <= 8'h0;
         sta_retry_count_ac2_r[8]  <= 8'h0;
         sta_retry_count_ac2_r[9]  <= 8'h0;
         sta_retry_count_ac2_r[10] <= 8'h0;
         sta_retry_count_ac2_r[11] <= 8'h0;
         sta_retry_count_ac2_r[12] <= 8'h0;
         sta_retry_count_ac2_r[13] <= 8'h0;
         sta_retry_count_ac2_r[14] <= 8'h0;
         sta_retry_count_ac2_r[15] <= 8'h0;
         sta_retry_count_ac2_r[16] <= 8'h0;
         sta_retry_count_ac2_r[17] <= 8'h0;
         sta_retry_count_ac2_r[18] <= 8'h0;
         sta_retry_count_ac2_r[19] <= 8'h0;
         sta_retry_count_ac2_r[20] <= 8'h0;
         sta_retry_count_ac2_r[21] <= 8'h0;
         sta_retry_count_ac2_r[22] <= 8'h0;
         sta_retry_count_ac2_r[23] <= 8'h0;
         sta_retry_count_ac2_r[24] <= 8'h0;
         sta_retry_count_ac2_r[25] <= 8'h0;
         sta_retry_count_ac2_r[26] <= 8'h0;
         sta_retry_count_ac2_r[27] <= 8'h0;
         sta_retry_count_ac2_r[28] <= 8'h0;
         sta_retry_count_ac2_r[29] <= 8'h0;
         sta_retry_count_ac2_r[30] <= 8'h0;
         sta_retry_count_ac2_r[31] <= 8'h0;
      end
      else if( sta_in_ac_owner == 3'h2 )
      begin
          if( retry )
          begin
             case( sta_ac2_count_r )
                5'h0    : sta_retry_count_ac2_r[0]  <= sta_in_ac2_lrc;
                5'h1    : sta_retry_count_ac2_r[1]  <= sta_in_ac2_lrc;
                5'h2    : sta_retry_count_ac2_r[2]  <= sta_in_ac2_lrc;
                5'h3    : sta_retry_count_ac2_r[3]  <= sta_in_ac2_lrc;
                5'h4    : sta_retry_count_ac2_r[4]  <= sta_in_ac2_lrc;
                5'h5    : sta_retry_count_ac2_r[5]  <= sta_in_ac2_lrc;
                5'h6    : sta_retry_count_ac2_r[6]  <= sta_in_ac2_lrc;
                5'h7    : sta_retry_count_ac2_r[7]  <= sta_in_ac2_lrc;
                5'h8    : sta_retry_count_ac2_r[8]  <= sta_in_ac2_lrc;
                5'h9    : sta_retry_count_ac2_r[9]  <= sta_in_ac2_lrc;
                5'hA    : sta_retry_count_ac2_r[10] <= sta_in_ac2_lrc;
                5'hB    : sta_retry_count_ac2_r[11] <= sta_in_ac2_lrc;
                5'hC    : sta_retry_count_ac2_r[12] <= sta_in_ac2_lrc;
                5'hD    : sta_retry_count_ac2_r[13] <= sta_in_ac2_lrc;
                5'hE    : sta_retry_count_ac2_r[14] <= sta_in_ac2_lrc;
                5'hF    : sta_retry_count_ac2_r[15] <= sta_in_ac2_lrc;
                5'h10   : sta_retry_count_ac2_r[16] <= sta_in_ac2_lrc;
                5'h11   : sta_retry_count_ac2_r[17] <= sta_in_ac2_lrc;
                5'h12   : sta_retry_count_ac2_r[18] <= sta_in_ac2_lrc;
                5'h13   : sta_retry_count_ac2_r[19] <= sta_in_ac2_lrc;
                5'h14   : sta_retry_count_ac2_r[20] <= sta_in_ac2_lrc;
                5'h15   : sta_retry_count_ac2_r[21] <= sta_in_ac2_lrc;
                5'h16   : sta_retry_count_ac2_r[22] <= sta_in_ac2_lrc;
                5'h17   : sta_retry_count_ac2_r[23] <= sta_in_ac2_lrc;
                5'h18   : sta_retry_count_ac2_r[24] <= sta_in_ac2_lrc;
                5'h19   : sta_retry_count_ac2_r[25] <= sta_in_ac2_lrc;
                5'h1A   : sta_retry_count_ac2_r[26] <= sta_in_ac2_lrc;
                5'h1B   : sta_retry_count_ac2_r[27] <= sta_in_ac2_lrc;
                5'h1C   : sta_retry_count_ac2_r[28] <= sta_in_ac2_lrc;
                5'h1D   : sta_retry_count_ac2_r[29] <= sta_in_ac2_lrc;
                5'h1E   : sta_retry_count_ac2_r[30] <= sta_in_ac2_lrc;
                5'h1F   : sta_retry_count_ac2_r[31] <= sta_in_ac2_lrc;
            endcase
         end   
         else
         begin
             case( sta_ac2_count_r )
                5'h0    : sta_retry_count_ac2_r[0]  <= sta_in_ac2_src;
                5'h1    : sta_retry_count_ac2_r[1]  <= sta_in_ac2_src;
                5'h2    : sta_retry_count_ac2_r[2]  <= sta_in_ac2_src;
                5'h3    : sta_retry_count_ac2_r[3]  <= sta_in_ac2_src;
                5'h4    : sta_retry_count_ac2_r[4]  <= sta_in_ac2_src;
                5'h5    : sta_retry_count_ac2_r[5]  <= sta_in_ac2_src;
                5'h6    : sta_retry_count_ac2_r[6]  <= sta_in_ac2_src;
                5'h7    : sta_retry_count_ac2_r[7]  <= sta_in_ac2_src;
                5'h8    : sta_retry_count_ac2_r[8]  <= sta_in_ac2_src;
                5'h9    : sta_retry_count_ac2_r[9]  <= sta_in_ac2_src;
                5'hA    : sta_retry_count_ac2_r[10] <= sta_in_ac2_src;
                5'hB    : sta_retry_count_ac2_r[11] <= sta_in_ac2_src;
                5'hC    : sta_retry_count_ac2_r[12] <= sta_in_ac2_src;
                5'hD    : sta_retry_count_ac2_r[13] <= sta_in_ac2_src;
                5'hE    : sta_retry_count_ac2_r[14] <= sta_in_ac2_src;
                5'hF    : sta_retry_count_ac2_r[15] <= sta_in_ac2_src;
                5'h10   : sta_retry_count_ac2_r[16] <= sta_in_ac2_src;
                5'h11   : sta_retry_count_ac2_r[17] <= sta_in_ac2_src;
                5'h12   : sta_retry_count_ac2_r[18] <= sta_in_ac2_src;
                5'h13   : sta_retry_count_ac2_r[19] <= sta_in_ac2_src;
                5'h14   : sta_retry_count_ac2_r[20] <= sta_in_ac2_src;
                5'h15   : sta_retry_count_ac2_r[21] <= sta_in_ac2_src;
                5'h16   : sta_retry_count_ac2_r[22] <= sta_in_ac2_src;
                5'h17   : sta_retry_count_ac2_r[23] <= sta_in_ac2_src;
                5'h18   : sta_retry_count_ac2_r[24] <= sta_in_ac2_src;
                5'h19   : sta_retry_count_ac2_r[25] <= sta_in_ac2_src;
                5'h1A   : sta_retry_count_ac2_r[26] <= sta_in_ac2_src;
                5'h1B   : sta_retry_count_ac2_r[27] <= sta_in_ac2_src;
                5'h1C   : sta_retry_count_ac2_r[28] <= sta_in_ac2_src;
                5'h1D   : sta_retry_count_ac2_r[29] <= sta_in_ac2_src;
                5'h1E   : sta_retry_count_ac2_r[30] <= sta_in_ac2_src;
                5'h1F   : sta_retry_count_ac2_r[31] <= sta_in_ac2_src;
            endcase
         end
      end      
   end

// Always Block for storing the Retry Count for AC3
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_retry_count_ac3_r[0]  <= 8'h0;
         sta_retry_count_ac3_r[1]  <= 8'h0;
         sta_retry_count_ac3_r[2]  <= 8'h0;
         sta_retry_count_ac3_r[3]  <= 8'h0;
         sta_retry_count_ac3_r[4]  <= 8'h0;
         sta_retry_count_ac3_r[5]  <= 8'h0;
         sta_retry_count_ac3_r[6]  <= 8'h0;
         sta_retry_count_ac3_r[7]  <= 8'h0;
         sta_retry_count_ac3_r[8]  <= 8'h0;
         sta_retry_count_ac3_r[9]  <= 8'h0;
         sta_retry_count_ac3_r[10] <= 8'h0;
         sta_retry_count_ac3_r[11] <= 8'h0;
         sta_retry_count_ac3_r[12] <= 8'h0;
         sta_retry_count_ac3_r[13] <= 8'h0;
         sta_retry_count_ac3_r[14] <= 8'h0;
         sta_retry_count_ac3_r[15] <= 8'h0;
         sta_retry_count_ac3_r[16] <= 8'h0;
         sta_retry_count_ac3_r[17] <= 8'h0;
         sta_retry_count_ac3_r[18] <= 8'h0;
         sta_retry_count_ac3_r[19] <= 8'h0;
         sta_retry_count_ac3_r[20] <= 8'h0;
         sta_retry_count_ac3_r[21] <= 8'h0;
         sta_retry_count_ac3_r[22] <= 8'h0;
         sta_retry_count_ac3_r[23] <= 8'h0;
         sta_retry_count_ac3_r[24] <= 8'h0;
         sta_retry_count_ac3_r[25] <= 8'h0;
         sta_retry_count_ac3_r[26] <= 8'h0;
         sta_retry_count_ac3_r[27] <= 8'h0;
         sta_retry_count_ac3_r[28] <= 8'h0;
         sta_retry_count_ac3_r[29] <= 8'h0;
         sta_retry_count_ac3_r[30] <= 8'h0;
         sta_retry_count_ac3_r[31] <= 8'h0;
      end
      else if( !sta_enable )
      begin
         sta_retry_count_ac3_r[0]  <= 8'h0;
         sta_retry_count_ac3_r[1]  <= 8'h0;
         sta_retry_count_ac3_r[2]  <= 8'h0;
         sta_retry_count_ac3_r[3]  <= 8'h0;
         sta_retry_count_ac3_r[4]  <= 8'h0;
         sta_retry_count_ac3_r[5]  <= 8'h0;
         sta_retry_count_ac3_r[6]  <= 8'h0;
         sta_retry_count_ac3_r[7]  <= 8'h0;
         sta_retry_count_ac3_r[8]  <= 8'h0;
         sta_retry_count_ac3_r[9]  <= 8'h0;
         sta_retry_count_ac3_r[10] <= 8'h0;
         sta_retry_count_ac3_r[11] <= 8'h0;
         sta_retry_count_ac3_r[12] <= 8'h0;
         sta_retry_count_ac3_r[13] <= 8'h0;
         sta_retry_count_ac3_r[14] <= 8'h0;
         sta_retry_count_ac3_r[15] <= 8'h0;
         sta_retry_count_ac3_r[16] <= 8'h0;
         sta_retry_count_ac3_r[17] <= 8'h0;
         sta_retry_count_ac3_r[18] <= 8'h0;
         sta_retry_count_ac3_r[19] <= 8'h0;
         sta_retry_count_ac3_r[20] <= 8'h0;
         sta_retry_count_ac3_r[21] <= 8'h0;
         sta_retry_count_ac3_r[22] <= 8'h0;
         sta_retry_count_ac3_r[23] <= 8'h0;
         sta_retry_count_ac3_r[24] <= 8'h0;
         sta_retry_count_ac3_r[25] <= 8'h0;
         sta_retry_count_ac3_r[26] <= 8'h0;
         sta_retry_count_ac3_r[27] <= 8'h0;
         sta_retry_count_ac3_r[28] <= 8'h0;
         sta_retry_count_ac3_r[29] <= 8'h0;
         sta_retry_count_ac3_r[30] <= 8'h0;
         sta_retry_count_ac3_r[31] <= 8'h0;
      end
      else if( sta_in_ac_owner == 3'h3 )
      begin
          if( retry )
          begin
             case( sta_ac3_count_r )
                5'h0    : sta_retry_count_ac3_r[0]  <= sta_in_ac3_lrc;
                5'h1    : sta_retry_count_ac3_r[1]  <= sta_in_ac3_lrc;
                5'h2    : sta_retry_count_ac3_r[2]  <= sta_in_ac3_lrc;
                5'h3    : sta_retry_count_ac3_r[3]  <= sta_in_ac3_lrc;
                5'h4    : sta_retry_count_ac3_r[4]  <= sta_in_ac3_lrc;
                5'h5    : sta_retry_count_ac3_r[5]  <= sta_in_ac3_lrc;
                5'h6    : sta_retry_count_ac3_r[6]  <= sta_in_ac3_lrc;
                5'h7    : sta_retry_count_ac3_r[7]  <= sta_in_ac3_lrc;
                5'h8    : sta_retry_count_ac3_r[8]  <= sta_in_ac3_lrc;
                5'h9    : sta_retry_count_ac3_r[9]  <= sta_in_ac3_lrc;
                5'hA    : sta_retry_count_ac3_r[10] <= sta_in_ac3_lrc;
                5'hB    : sta_retry_count_ac3_r[11] <= sta_in_ac3_lrc;
                5'hC    : sta_retry_count_ac3_r[12] <= sta_in_ac3_lrc;
                5'hD    : sta_retry_count_ac3_r[13] <= sta_in_ac3_lrc;
                5'hE    : sta_retry_count_ac3_r[14] <= sta_in_ac3_lrc;
                5'hF    : sta_retry_count_ac3_r[15] <= sta_in_ac3_lrc;
                5'h10   : sta_retry_count_ac3_r[16] <= sta_in_ac3_lrc;
                5'h11   : sta_retry_count_ac3_r[17] <= sta_in_ac3_lrc;
                5'h12   : sta_retry_count_ac3_r[18] <= sta_in_ac3_lrc;
                5'h13   : sta_retry_count_ac3_r[19] <= sta_in_ac3_lrc;
                5'h14   : sta_retry_count_ac3_r[20] <= sta_in_ac3_lrc;
                5'h15   : sta_retry_count_ac3_r[21] <= sta_in_ac3_lrc;
                5'h16   : sta_retry_count_ac3_r[22] <= sta_in_ac3_lrc;
                5'h17   : sta_retry_count_ac3_r[23] <= sta_in_ac3_lrc;
                5'h18   : sta_retry_count_ac3_r[24] <= sta_in_ac3_lrc;
                5'h19   : sta_retry_count_ac3_r[25] <= sta_in_ac3_lrc;
                5'h1A   : sta_retry_count_ac3_r[26] <= sta_in_ac3_lrc;
                5'h1B   : sta_retry_count_ac3_r[27] <= sta_in_ac3_lrc;
                5'h1C   : sta_retry_count_ac3_r[28] <= sta_in_ac3_lrc;
                5'h1D   : sta_retry_count_ac3_r[29] <= sta_in_ac3_lrc;
                5'h1E   : sta_retry_count_ac3_r[30] <= sta_in_ac3_lrc;
                5'h1F   : sta_retry_count_ac3_r[31] <= sta_in_ac3_lrc;
            endcase
         end   
         else
         begin
             case( sta_ac3_count_r )
                5'h0    : sta_retry_count_ac3_r[0]  <= sta_in_ac3_src;
                5'h1    : sta_retry_count_ac3_r[1]  <= sta_in_ac3_src;
                5'h2    : sta_retry_count_ac3_r[2]  <= sta_in_ac3_src;
                5'h3    : sta_retry_count_ac3_r[3]  <= sta_in_ac3_src;
                5'h4    : sta_retry_count_ac3_r[4]  <= sta_in_ac3_src;
                5'h5    : sta_retry_count_ac3_r[5]  <= sta_in_ac3_src;
                5'h6    : sta_retry_count_ac3_r[6]  <= sta_in_ac3_src;
                5'h7    : sta_retry_count_ac3_r[7]  <= sta_in_ac3_src;
                5'h8    : sta_retry_count_ac3_r[8]  <= sta_in_ac3_src;
                5'h9    : sta_retry_count_ac3_r[9]  <= sta_in_ac3_src;
                5'hA    : sta_retry_count_ac3_r[10] <= sta_in_ac3_src;
                5'hB    : sta_retry_count_ac3_r[11] <= sta_in_ac3_src;
                5'hC    : sta_retry_count_ac3_r[12] <= sta_in_ac3_src;
                5'hD    : sta_retry_count_ac3_r[13] <= sta_in_ac3_src;
                5'hE    : sta_retry_count_ac3_r[14] <= sta_in_ac3_src;
                5'hF    : sta_retry_count_ac3_r[15] <= sta_in_ac3_src;
                5'h10   : sta_retry_count_ac3_r[16] <= sta_in_ac3_src;
                5'h11   : sta_retry_count_ac3_r[17] <= sta_in_ac3_src;
                5'h12   : sta_retry_count_ac3_r[18] <= sta_in_ac3_src;
                5'h13   : sta_retry_count_ac3_r[19] <= sta_in_ac3_src;
                5'h14   : sta_retry_count_ac3_r[20] <= sta_in_ac3_src;
                5'h15   : sta_retry_count_ac3_r[21] <= sta_in_ac3_src;
                5'h16   : sta_retry_count_ac3_r[22] <= sta_in_ac3_src;
                5'h17   : sta_retry_count_ac3_r[23] <= sta_in_ac3_src;
                5'h18   : sta_retry_count_ac3_r[24] <= sta_in_ac3_src;
                5'h19   : sta_retry_count_ac3_r[25] <= sta_in_ac3_src;
                5'h1A   : sta_retry_count_ac3_r[26] <= sta_in_ac3_src;
                5'h1B   : sta_retry_count_ac3_r[27] <= sta_in_ac3_src;
                5'h1C   : sta_retry_count_ac3_r[28] <= sta_in_ac3_src;
                5'h1D   : sta_retry_count_ac3_r[29] <= sta_in_ac3_src;
                5'h1E   : sta_retry_count_ac3_r[30] <= sta_in_ac3_src;
                5'h1F   : sta_retry_count_ac3_r[31] <= sta_in_ac3_src;
            endcase
         end
      end      
   end

// Always Block for storing the Status for AC0
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_status_ac0_r[0]  <= 1'b0;
         sta_status_ac0_r[1]  <= 1'b0;
         sta_status_ac0_r[2]  <= 1'b0;
         sta_status_ac0_r[3]  <= 1'b0;
         sta_status_ac0_r[4]  <= 1'b0;
         sta_status_ac0_r[5]  <= 1'b0;
         sta_status_ac0_r[6]  <= 1'b0;
         sta_status_ac0_r[7]  <= 1'b0;
         sta_status_ac0_r[8]  <= 1'b0;
         sta_status_ac0_r[9]  <= 1'b0;
         sta_status_ac0_r[10] <= 1'b0;
         sta_status_ac0_r[11] <= 1'b0;
         sta_status_ac0_r[12] <= 1'b0;
         sta_status_ac0_r[13] <= 1'b0;
         sta_status_ac0_r[14] <= 1'b0;
         sta_status_ac0_r[15] <= 1'b0;
         sta_status_ac0_r[16] <= 1'b0;
         sta_status_ac0_r[17] <= 1'b0;
         sta_status_ac0_r[18] <= 1'b0;
         sta_status_ac0_r[19] <= 1'b0;
         sta_status_ac0_r[20] <= 1'b0;
         sta_status_ac0_r[21] <= 1'b0;
         sta_status_ac0_r[22] <= 1'b0;
         sta_status_ac0_r[23] <= 1'b0;
         sta_status_ac0_r[24] <= 1'b0;
         sta_status_ac0_r[25] <= 1'b0;
         sta_status_ac0_r[26] <= 1'b0;
         sta_status_ac0_r[27] <= 1'b0;
         sta_status_ac0_r[28] <= 1'b0;
         sta_status_ac0_r[29] <= 1'b0;
         sta_status_ac0_r[30] <= 1'b0;
         sta_status_ac0_r[31] <= 1'b0;
      end
      else if( !sta_enable )
      begin
         sta_status_ac0_r[0]  <= 1'b0;
         sta_status_ac0_r[1]  <= 1'b0;
         sta_status_ac0_r[2]  <= 1'b0;
         sta_status_ac0_r[3]  <= 1'b0;
         sta_status_ac0_r[4]  <= 1'b0;
         sta_status_ac0_r[5]  <= 1'b0;
         sta_status_ac0_r[6]  <= 1'b0;
         sta_status_ac0_r[7]  <= 1'b0;
         sta_status_ac0_r[8]  <= 1'b0;
         sta_status_ac0_r[9]  <= 1'b0;
         sta_status_ac0_r[10] <= 1'b0;
         sta_status_ac0_r[11] <= 1'b0;
         sta_status_ac0_r[12] <= 1'b0;
         sta_status_ac0_r[13] <= 1'b0;
         sta_status_ac0_r[14] <= 1'b0;
         sta_status_ac0_r[15] <= 1'b0;
         sta_status_ac0_r[16] <= 1'b0;
         sta_status_ac0_r[17] <= 1'b0;
         sta_status_ac0_r[18] <= 1'b0;
         sta_status_ac0_r[19] <= 1'b0;
         sta_status_ac0_r[20] <= 1'b0;
         sta_status_ac0_r[21] <= 1'b0;
         sta_status_ac0_r[22] <= 1'b0;
         sta_status_ac0_r[23] <= 1'b0;
         sta_status_ac0_r[24] <= 1'b0;
         sta_status_ac0_r[25] <= 1'b0;
         sta_status_ac0_r[26] <= 1'b0;
         sta_status_ac0_r[27] <= 1'b0;
         sta_status_ac0_r[28] <= 1'b0;
         sta_status_ac0_r[29] <= 1'b0;
         sta_status_ac0_r[30] <= 1'b0;
         sta_status_ac0_r[31] <= 1'b0;
      end
      else if( sta_in_ac_owner == 3'h0 )
      begin
         case( sta_ac0_count_r )
            5'h0    : sta_status_ac0_r[0]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac0[0] : 1'b0 ;
            5'h1    : sta_status_ac0_r[1]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status :
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac0[1] : 1'b0 ;
            5'h2    : sta_status_ac0_r[2]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac0[2] : 1'b0 ;
            5'h3    : sta_status_ac0_r[3]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status :
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac0[3] : 1'b0 ;
            5'h4    : sta_status_ac0_r[4]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status :
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac0[4] : 1'b0 ;
            5'h5    : sta_status_ac0_r[5]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status :
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac0[5] : 1'b0 ;
            5'h6    : sta_status_ac0_r[6]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status :
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac0[6] : 1'b0 ;
            5'h7    : sta_status_ac0_r[7]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status :
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac0[7] : 1'b0 ;
            5'h8    : sta_status_ac0_r[8]  <= sta_in_mpdu_status;
            5'h9    : sta_status_ac0_r[9]  <= sta_in_mpdu_status;
            5'hA    : sta_status_ac0_r[10] <= sta_in_mpdu_status;
            5'hB    : sta_status_ac0_r[11] <= sta_in_mpdu_status;
            5'hC    : sta_status_ac0_r[12] <= sta_in_mpdu_status;
            5'hD    : sta_status_ac0_r[13] <= sta_in_mpdu_status;
            5'hE    : sta_status_ac0_r[14] <= sta_in_mpdu_status;
            5'hF    : sta_status_ac0_r[15] <= sta_in_mpdu_status;
            5'h10   : sta_status_ac0_r[16] <= sta_in_mpdu_status;
            5'h11   : sta_status_ac0_r[17] <= sta_in_mpdu_status;
            5'h12   : sta_status_ac0_r[18] <= sta_in_mpdu_status;
            5'h13   : sta_status_ac0_r[19] <= sta_in_mpdu_status;
            5'h14   : sta_status_ac0_r[20] <= sta_in_mpdu_status;
            5'h15   : sta_status_ac0_r[21] <= sta_in_mpdu_status;
            5'h16   : sta_status_ac0_r[22] <= sta_in_mpdu_status;
            5'h17   : sta_status_ac0_r[23] <= sta_in_mpdu_status;
            5'h18   : sta_status_ac0_r[24] <= sta_in_mpdu_status;
            5'h19   : sta_status_ac0_r[25] <= sta_in_mpdu_status;
            5'h1A   : sta_status_ac0_r[26] <= sta_in_mpdu_status;
            5'h1B   : sta_status_ac0_r[27] <= sta_in_mpdu_status;
            5'h1C   : sta_status_ac0_r[28] <= sta_in_mpdu_status;
            5'h1D   : sta_status_ac0_r[29] <= sta_in_mpdu_status;
            5'h1E   : sta_status_ac0_r[30] <= sta_in_mpdu_status;
            5'h1F   : sta_status_ac0_r[31] <= sta_in_mpdu_status;
         endcase
      end   
   end

// Always Block for storing the Status for AC1
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_status_ac1_r[0]  <= 1'b0;
         sta_status_ac1_r[1]  <= 1'b0;
         sta_status_ac1_r[2]  <= 1'b0;
         sta_status_ac1_r[3]  <= 1'b0;
         sta_status_ac1_r[4]  <= 1'b0;
         sta_status_ac1_r[5]  <= 1'b0;
         sta_status_ac1_r[6]  <= 1'b0;
         sta_status_ac1_r[7]  <= 1'b0;
         sta_status_ac1_r[8]  <= 1'b0;
         sta_status_ac1_r[9]  <= 1'b0;
         sta_status_ac1_r[10] <= 1'b0;
         sta_status_ac1_r[11] <= 1'b0;
         sta_status_ac1_r[12] <= 1'b0;
         sta_status_ac1_r[13] <= 1'b0;
         sta_status_ac1_r[14] <= 1'b0;
         sta_status_ac1_r[15] <= 1'b0;
         sta_status_ac1_r[16] <= 1'b0;
         sta_status_ac1_r[17] <= 1'b0;
         sta_status_ac1_r[18] <= 1'b0;
         sta_status_ac1_r[19] <= 1'b0;
         sta_status_ac1_r[20] <= 1'b0;
         sta_status_ac1_r[21] <= 1'b0;
         sta_status_ac1_r[22] <= 1'b0;
         sta_status_ac1_r[23] <= 1'b0;
         sta_status_ac1_r[24] <= 1'b0;
         sta_status_ac1_r[25] <= 1'b0;
         sta_status_ac1_r[26] <= 1'b0;
         sta_status_ac1_r[27] <= 1'b0;
         sta_status_ac1_r[28] <= 1'b0;
         sta_status_ac1_r[29] <= 1'b0;
         sta_status_ac1_r[30] <= 1'b0;
         sta_status_ac1_r[31] <= 1'b0;
      end
      else if( !sta_enable )
      begin
         sta_status_ac1_r[0]  <= 1'b0;
         sta_status_ac1_r[1]  <= 1'b0;
         sta_status_ac1_r[2]  <= 1'b0;
         sta_status_ac1_r[3]  <= 1'b0;
         sta_status_ac1_r[4]  <= 1'b0;
         sta_status_ac1_r[5]  <= 1'b0;
         sta_status_ac1_r[6]  <= 1'b0;
         sta_status_ac1_r[7]  <= 1'b0;
         sta_status_ac1_r[8]  <= 1'b0;
         sta_status_ac1_r[9]  <= 1'b0;
         sta_status_ac1_r[10] <= 1'b0;
         sta_status_ac1_r[11] <= 1'b0;
         sta_status_ac1_r[12] <= 1'b0;
         sta_status_ac1_r[13] <= 1'b0;
         sta_status_ac1_r[14] <= 1'b0;
         sta_status_ac1_r[15] <= 1'b0;
         sta_status_ac1_r[16] <= 1'b0;
         sta_status_ac1_r[17] <= 1'b0;
         sta_status_ac1_r[18] <= 1'b0;
         sta_status_ac1_r[19] <= 1'b0;
         sta_status_ac1_r[20] <= 1'b0;
         sta_status_ac1_r[21] <= 1'b0;
         sta_status_ac1_r[22] <= 1'b0;
         sta_status_ac1_r[23] <= 1'b0;
         sta_status_ac1_r[24] <= 1'b0;
         sta_status_ac1_r[25] <= 1'b0;
         sta_status_ac1_r[26] <= 1'b0;
         sta_status_ac1_r[27] <= 1'b0;
         sta_status_ac1_r[28] <= 1'b0;
         sta_status_ac1_r[29] <= 1'b0;
         sta_status_ac1_r[30] <= 1'b0;
         sta_status_ac1_r[31] <= 1'b0;
      end
      else if( sta_in_ac_owner == 3'h1 )
      begin
         case( sta_ac1_count_r )
            5'h0    : sta_status_ac1_r[0]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac1[0] : 1'b0 ;
            5'h1    : sta_status_ac1_r[1]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac1[1] : 1'b0 ;
            5'h2    : sta_status_ac1_r[2]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac1[2] : 1'b0 ;
            5'h3    : sta_status_ac1_r[3]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac1[3] : 1'b0 ;
            5'h4    : sta_status_ac1_r[4]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac1[4] : 1'b0 ;
            5'h5    : sta_status_ac1_r[5]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac1[5] : 1'b0 ;
            5'h6    : sta_status_ac1_r[6]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac1[6] : 1'b0 ;
            5'h7    : sta_status_ac1_r[7]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac1[7] : 1'b0 ;
            5'h8    : sta_status_ac1_r[8]  <= sta_in_mpdu_status;
            5'h9    : sta_status_ac1_r[9]  <= sta_in_mpdu_status;
            5'hA    : sta_status_ac1_r[10] <= sta_in_mpdu_status;
            5'hB    : sta_status_ac1_r[11] <= sta_in_mpdu_status;
            5'hC    : sta_status_ac1_r[12] <= sta_in_mpdu_status;
            5'hD    : sta_status_ac1_r[13] <= sta_in_mpdu_status;
            5'hE    : sta_status_ac1_r[14] <= sta_in_mpdu_status;
            5'hF    : sta_status_ac1_r[15] <= sta_in_mpdu_status;
            5'h10   : sta_status_ac1_r[16] <= sta_in_mpdu_status;
            5'h11   : sta_status_ac1_r[17] <= sta_in_mpdu_status;
            5'h12   : sta_status_ac1_r[18] <= sta_in_mpdu_status;
            5'h13   : sta_status_ac1_r[19] <= sta_in_mpdu_status;
            5'h14   : sta_status_ac1_r[20] <= sta_in_mpdu_status;
            5'h15   : sta_status_ac1_r[21] <= sta_in_mpdu_status;
            5'h16   : sta_status_ac1_r[22] <= sta_in_mpdu_status;
            5'h17   : sta_status_ac1_r[23] <= sta_in_mpdu_status;
            5'h18   : sta_status_ac1_r[24] <= sta_in_mpdu_status;
            5'h19   : sta_status_ac1_r[25] <= sta_in_mpdu_status;
            5'h1A   : sta_status_ac1_r[26] <= sta_in_mpdu_status;
            5'h1B   : sta_status_ac1_r[27] <= sta_in_mpdu_status;
            5'h1C   : sta_status_ac1_r[28] <= sta_in_mpdu_status;
            5'h1D   : sta_status_ac1_r[29] <= sta_in_mpdu_status;
            5'h1E   : sta_status_ac1_r[30] <= sta_in_mpdu_status;
            5'h1F   : sta_status_ac1_r[31] <= sta_in_mpdu_status;
         endcase
      end   
   end

// Always Block for storing the Status for AC2
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_status_ac2_r[0]  <= 1'b0;
         sta_status_ac2_r[1]  <= 1'b0;
         sta_status_ac2_r[2]  <= 1'b0;
         sta_status_ac2_r[3]  <= 1'b0;
         sta_status_ac2_r[4]  <= 1'b0;
         sta_status_ac2_r[5]  <= 1'b0;
         sta_status_ac2_r[6]  <= 1'b0;
         sta_status_ac2_r[7]  <= 1'b0;
         sta_status_ac2_r[8]  <= 1'b0;
         sta_status_ac2_r[9]  <= 1'b0;
         sta_status_ac2_r[10] <= 1'b0;
         sta_status_ac2_r[11] <= 1'b0;
         sta_status_ac2_r[12] <= 1'b0;
         sta_status_ac2_r[13] <= 1'b0;
         sta_status_ac2_r[14] <= 1'b0;
         sta_status_ac2_r[15] <= 1'b0;
         sta_status_ac2_r[16] <= 1'b0;
         sta_status_ac2_r[17] <= 1'b0;
         sta_status_ac2_r[18] <= 1'b0;
         sta_status_ac2_r[19] <= 1'b0;
         sta_status_ac2_r[20] <= 1'b0;
         sta_status_ac2_r[21] <= 1'b0;
         sta_status_ac2_r[22] <= 1'b0;
         sta_status_ac2_r[23] <= 1'b0;
         sta_status_ac2_r[24] <= 1'b0;
         sta_status_ac2_r[25] <= 1'b0;
         sta_status_ac2_r[26] <= 1'b0;
         sta_status_ac2_r[27] <= 1'b0;
         sta_status_ac2_r[28] <= 1'b0;
         sta_status_ac2_r[29] <= 1'b0;
         sta_status_ac2_r[30] <= 1'b0;
         sta_status_ac2_r[31] <= 1'b0;
      end
      else if( !sta_enable )
      begin
         sta_status_ac2_r[0]  <= 1'b0;
         sta_status_ac2_r[1]  <= 1'b0;
         sta_status_ac2_r[2]  <= 1'b0;
         sta_status_ac2_r[3]  <= 1'b0;
         sta_status_ac2_r[4]  <= 1'b0;
         sta_status_ac2_r[5]  <= 1'b0;
         sta_status_ac2_r[6]  <= 1'b0;
         sta_status_ac2_r[7]  <= 1'b0;
         sta_status_ac2_r[8]  <= 1'b0;
         sta_status_ac2_r[9]  <= 1'b0;
         sta_status_ac2_r[10] <= 1'b0;
         sta_status_ac2_r[11] <= 1'b0;
         sta_status_ac2_r[12] <= 1'b0;
         sta_status_ac2_r[13] <= 1'b0;
         sta_status_ac2_r[14] <= 1'b0;
         sta_status_ac2_r[15] <= 1'b0;
         sta_status_ac2_r[16] <= 1'b0;
         sta_status_ac2_r[17] <= 1'b0;
         sta_status_ac2_r[18] <= 1'b0;
         sta_status_ac2_r[19] <= 1'b0;
         sta_status_ac2_r[20] <= 1'b0;
         sta_status_ac2_r[21] <= 1'b0;
         sta_status_ac2_r[22] <= 1'b0;
         sta_status_ac2_r[23] <= 1'b0;
         sta_status_ac2_r[24] <= 1'b0;
         sta_status_ac2_r[25] <= 1'b0;
         sta_status_ac2_r[26] <= 1'b0;
         sta_status_ac2_r[27] <= 1'b0;
         sta_status_ac2_r[28] <= 1'b0;
         sta_status_ac2_r[29] <= 1'b0;
         sta_status_ac2_r[30] <= 1'b0;
         sta_status_ac2_r[31] <= 1'b0;
      end
      else if( sta_in_ac_owner == 3'h2 )
      begin
         case( sta_ac2_count_r )
            5'h0    : sta_status_ac2_r[0]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac2[0] : 1'b0 ;
            5'h1    : sta_status_ac2_r[1]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac2[1] : 1'b0 ;
            5'h2    : sta_status_ac2_r[2]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac2[2] : 1'b0 ;
            5'h3    : sta_status_ac2_r[3]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac2[3] : 1'b0 ;
            5'h4    : sta_status_ac2_r[4]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac2[4] : 1'b0 ;
            5'h5    : sta_status_ac2_r[5]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac2[5] : 1'b0 ;
            5'h6    : sta_status_ac2_r[6]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac2[6] : 1'b0 ;
            5'h7    : sta_status_ac2_r[7]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac2[7] : 1'b0 ;
            5'h8    : sta_status_ac2_r[8]  <= sta_in_mpdu_status;
            5'h9    : sta_status_ac2_r[9]  <= sta_in_mpdu_status;
            5'hA    : sta_status_ac2_r[10] <= sta_in_mpdu_status;
            5'hB    : sta_status_ac2_r[11] <= sta_in_mpdu_status;
            5'hC    : sta_status_ac2_r[12] <= sta_in_mpdu_status;
            5'hD    : sta_status_ac2_r[13] <= sta_in_mpdu_status;
            5'hE    : sta_status_ac2_r[14] <= sta_in_mpdu_status;
            5'hF    : sta_status_ac2_r[15] <= sta_in_mpdu_status;
            5'h10   : sta_status_ac2_r[16] <= sta_in_mpdu_status;
            5'h11   : sta_status_ac2_r[17] <= sta_in_mpdu_status;
            5'h12   : sta_status_ac2_r[18] <= sta_in_mpdu_status;
            5'h13   : sta_status_ac2_r[19] <= sta_in_mpdu_status;
            5'h14   : sta_status_ac2_r[20] <= sta_in_mpdu_status;
            5'h15   : sta_status_ac2_r[21] <= sta_in_mpdu_status;
            5'h16   : sta_status_ac2_r[22] <= sta_in_mpdu_status;
            5'h17   : sta_status_ac2_r[23] <= sta_in_mpdu_status;
            5'h18   : sta_status_ac2_r[24] <= sta_in_mpdu_status;
            5'h19   : sta_status_ac2_r[25] <= sta_in_mpdu_status;
            5'h1A   : sta_status_ac2_r[26] <= sta_in_mpdu_status;
            5'h1B   : sta_status_ac2_r[27] <= sta_in_mpdu_status;
            5'h1C   : sta_status_ac2_r[28] <= sta_in_mpdu_status;
            5'h1D   : sta_status_ac2_r[29] <= sta_in_mpdu_status;
            5'h1E   : sta_status_ac2_r[30] <= sta_in_mpdu_status;
            5'h1F   : sta_status_ac2_r[31] <= sta_in_mpdu_status;
         endcase
      end   
   end

// Always Block for storing the Status for AC3
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         sta_status_ac3_r[0]  <= 1'b0;
         sta_status_ac3_r[1]  <= 1'b0;
         sta_status_ac3_r[2]  <= 1'b0;
         sta_status_ac3_r[3]  <= 1'b0;
         sta_status_ac3_r[4]  <= 1'b0;
         sta_status_ac3_r[5]  <= 1'b0;
         sta_status_ac3_r[6]  <= 1'b0;
         sta_status_ac3_r[7]  <= 1'b0;
         sta_status_ac3_r[8]  <= 1'b0;
         sta_status_ac3_r[9]  <= 1'b0;
         sta_status_ac3_r[10] <= 1'b0;
         sta_status_ac3_r[11] <= 1'b0;
         sta_status_ac3_r[12] <= 1'b0;
         sta_status_ac3_r[13] <= 1'b0;
         sta_status_ac3_r[14] <= 1'b0;
         sta_status_ac3_r[15] <= 1'b0;
         sta_status_ac3_r[16] <= 1'b0;
         sta_status_ac3_r[17] <= 1'b0;
         sta_status_ac3_r[18] <= 1'b0;
         sta_status_ac3_r[19] <= 1'b0;
         sta_status_ac3_r[20] <= 1'b0;
         sta_status_ac3_r[21] <= 1'b0;
         sta_status_ac3_r[22] <= 1'b0;
         sta_status_ac3_r[23] <= 1'b0;
         sta_status_ac3_r[24] <= 1'b0;
         sta_status_ac3_r[25] <= 1'b0;
         sta_status_ac3_r[26] <= 1'b0;
         sta_status_ac3_r[27] <= 1'b0;
         sta_status_ac3_r[28] <= 1'b0;
         sta_status_ac3_r[29] <= 1'b0;
         sta_status_ac3_r[30] <= 1'b0;
         sta_status_ac3_r[31] <= 1'b0;
      end
      else if( !sta_enable )
      begin
         sta_status_ac3_r[0]  <= 1'b0;
         sta_status_ac3_r[1]  <= 1'b0;
         sta_status_ac3_r[2]  <= 1'b0;
         sta_status_ac3_r[3]  <= 1'b0;
         sta_status_ac3_r[4]  <= 1'b0;
         sta_status_ac3_r[5]  <= 1'b0;
         sta_status_ac3_r[6]  <= 1'b0;
         sta_status_ac3_r[7]  <= 1'b0;
         sta_status_ac3_r[8]  <= 1'b0;
         sta_status_ac3_r[9]  <= 1'b0;
         sta_status_ac3_r[10] <= 1'b0;
         sta_status_ac3_r[11] <= 1'b0;
         sta_status_ac3_r[12] <= 1'b0;
         sta_status_ac3_r[13] <= 1'b0;
         sta_status_ac3_r[14] <= 1'b0;
         sta_status_ac3_r[15] <= 1'b0;
         sta_status_ac3_r[16] <= 1'b0;
         sta_status_ac3_r[17] <= 1'b0;
         sta_status_ac3_r[18] <= 1'b0;
         sta_status_ac3_r[19] <= 1'b0;
         sta_status_ac3_r[20] <= 1'b0;
         sta_status_ac3_r[21] <= 1'b0;
         sta_status_ac3_r[22] <= 1'b0;
         sta_status_ac3_r[23] <= 1'b0;
         sta_status_ac3_r[24] <= 1'b0;
         sta_status_ac3_r[25] <= 1'b0;
         sta_status_ac3_r[26] <= 1'b0;
         sta_status_ac3_r[27] <= 1'b0;
         sta_status_ac3_r[28] <= 1'b0;
         sta_status_ac3_r[29] <= 1'b0;
         sta_status_ac3_r[30] <= 1'b0;
         sta_status_ac3_r[31] <= 1'b0;
      end
      else if( sta_in_ac_owner == 3'h3 )
      begin
         case( sta_ac3_count_r )
5'h0    : sta_status_ac3_r[0]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac3[0] : 1'b0 ;
            5'h1    : sta_status_ac3_r[1]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac3[1] : 1'b0 ;
            5'h2    : sta_status_ac3_r[2]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac3[2] : 1'b0 ;
            5'h3    : sta_status_ac3_r[3]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac3[3] : 1'b0 ;
            5'h4    : sta_status_ac3_r[4]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac3[4] : 1'b0 ;
            5'h5    : sta_status_ac3_r[5]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac3[5] : 1'b0 ;
            5'h6    : sta_status_ac3_r[6]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac3[6] : 1'b0 ;
            5'h7    : sta_status_ac3_r[7]  <= sta_in_clear_mpdu ?  sta_in_mpdu_status : 
		                              sta_clear_ampdu_r ? sta_in_bitmap_ac3[7] : 1'b0 ;
            5'h8    : sta_status_ac3_r[8]  <= sta_in_mpdu_status;
            5'h9    : sta_status_ac3_r[9]  <= sta_in_mpdu_status;
            5'hA    : sta_status_ac3_r[10] <= sta_in_mpdu_status;
            5'hB    : sta_status_ac3_r[11] <= sta_in_mpdu_status;
            5'hC    : sta_status_ac3_r[12] <= sta_in_mpdu_status;
            5'hD    : sta_status_ac3_r[13] <= sta_in_mpdu_status;
            5'hE    : sta_status_ac3_r[14] <= sta_in_mpdu_status;
            5'hF    : sta_status_ac3_r[15] <= sta_in_mpdu_status;
            5'h10   : sta_status_ac3_r[16] <= sta_in_mpdu_status;
            5'h11   : sta_status_ac3_r[17] <= sta_in_mpdu_status;
            5'h12   : sta_status_ac3_r[18] <= sta_in_mpdu_status;
            5'h13   : sta_status_ac3_r[19] <= sta_in_mpdu_status;
            5'h14   : sta_status_ac3_r[20] <= sta_in_mpdu_status;
            5'h15   : sta_status_ac3_r[21] <= sta_in_mpdu_status;
            5'h16   : sta_status_ac3_r[22] <= sta_in_mpdu_status;
            5'h17   : sta_status_ac3_r[23] <= sta_in_mpdu_status;
            5'h18   : sta_status_ac3_r[24] <= sta_in_mpdu_status;
            5'h19   : sta_status_ac3_r[25] <= sta_in_mpdu_status;
            5'h1A   : sta_status_ac3_r[26] <= sta_in_mpdu_status;
            5'h1B   : sta_status_ac3_r[27] <= sta_in_mpdu_status;
            5'h1C   : sta_status_ac3_r[28] <= sta_in_mpdu_status;
            5'h1D   : sta_status_ac3_r[29] <= sta_in_mpdu_status;
            5'h1E   : sta_status_ac3_r[30] <= sta_in_mpdu_status;
            5'h1F   : sta_status_ac3_r[31] <= sta_in_mpdu_status;
         endcase
      end   
   end



//////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////// Registers for STA MAC ADDRESS ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

// Always Block to store the MAC Address of Station STA00
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r0 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r0 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0000 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r0[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r0[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r0[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r0[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0004 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r0[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r0[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA01
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r1 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r1 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0008 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r1[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r1[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r1[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r1[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h000C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r1[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r1[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA02
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r2 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r2 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0010 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r2[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r2[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r2[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r2[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0014 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r2[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r2[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA03
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r3 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r3 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0018 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r3[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r3[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r3[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r3[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h001C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r3[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r3[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA04
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r4 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r4 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0020 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r4[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r4[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r4[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r4[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0024 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r4[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r4[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA05
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r5 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r5 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0028 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r5[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r5[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r5[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r5[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h002C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r5[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r5[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA06
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r6 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r6 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0030 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r6[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r6[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r6[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r6[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0034 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r6[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r6[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA07
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r7 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r7 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0038 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r7[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r7[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r7[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r7[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h003C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r7[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r7[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA08
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r8 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r8 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0040 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r8[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r8[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r8[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r8[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0044 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r8[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r8[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA09
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r9 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r9 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0048 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r9[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r9[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r9[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r9[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h004C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r9[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r9[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA10
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r10 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r10 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0050 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r10[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r10[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r10[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r10[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0054 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r10[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r10[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA11
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r11 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r11 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0058 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r11[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r11[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r11[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r11[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h005C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r11[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r11[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA12
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r12  <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r12  <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0060 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r12[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r12[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r12[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r12[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0064 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r12[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r12[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA13
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r13 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r13 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0068 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r13[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r13[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r13[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r13[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h006C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r13[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r13[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA14
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r14 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r14 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0070 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r14[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r14[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r14[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r14[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0074 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r14[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r14[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA15
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r15 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r15 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0078 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r15[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r15[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r15[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r15[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h007C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r15[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r15[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA16
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r16 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r16 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0080 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r16[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r16[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r16[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r16[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0084 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r16[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r16[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA17
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r17 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r17 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0088 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r17[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r17[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r17[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r17[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h008C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r17[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r17[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA18
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r18 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r18 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0090 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r18[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r18[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r18[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r18[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0094 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r18[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r18[47:40] <= sta_in_umac_data[15:8];
      end
   end

// Always Block to store the MAC Address of Station STA19
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_mac_addr_r19 <= 48'h0;
      else if( !sta_enable )
         sta_mac_addr_r19 <= 48'h0;
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h0098 ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r19[7 : 0] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r19[15: 8] <= sta_in_umac_data[15:8]; 
         if( sta_in_umac_bea[2] )
            sta_mac_addr_r19[23:16] <= sta_in_umac_data[23:16];
         if( sta_in_umac_bea[3] )
            sta_mac_addr_r19[31:24] <= sta_in_umac_data[31:24]; 
      end
      else if( sta_in_umac_val && sta_in_umac_wrn_rd && 
             ( sta_in_umac_addr == `STA00_MAC_ADDR + 14'h009C ))
      begin
         if( sta_in_umac_bea[0] )
            sta_mac_addr_r19[39:32] <= sta_in_umac_data[7:0];
         if( sta_in_umac_bea[1] )
            sta_mac_addr_r19[47:40] <= sta_in_umac_data[15:8];
      end
   end

///////////////////////////////////////////////////////////////////////////////////////////////

// Always Block to read the Internal Registers to be given as output
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         reg_data_r <= 32'h0;
      else if( !sta_enable )
         reg_data_r <= 32'h0;
      else if( sta_in_umac_val && !sta_in_umac_bea && !access_memory )
      begin
         case( sta_in_umac_addr ) // synopsys full case parallel case
            `STA_IS_USED_OFFSET               : begin reg_data_r <= sta_is_used_r;             end
            `BA_IS_USED_OFFSET + 14'h0000     : begin reg_data_r <= ba_is_used_r[31:0];        end 
            `BA_IS_USED_OFFSET + 14'h0004     : begin reg_data_r <= ba_is_used_r[63:32];       end
            `STA00_MAC_ADDR + 14'h0000        : begin reg_data_r <= sta_mac_addr_r0[31:0];     end
            `STA00_MAC_ADDR + 14'h0004        : begin reg_data_r <= sta_mac_addr_r0[47:32];    end
            `STA00_MAC_ADDR + 14'h0008        : begin reg_data_r <= sta_mac_addr_r1[31:0];     end
            `STA00_MAC_ADDR + 14'h000C        : begin reg_data_r <= sta_mac_addr_r1[47:32];    end
            `STA00_MAC_ADDR + 14'h0010        : begin reg_data_r <= sta_mac_addr_r2[31:0];     end
            `STA00_MAC_ADDR + 14'h0014        : begin reg_data_r <= sta_mac_addr_r2[47:32];    end
            `STA00_MAC_ADDR + 14'h0018        : begin reg_data_r <= sta_mac_addr_r3[31:0];     end
            `STA00_MAC_ADDR + 14'h001C        : begin reg_data_r <= sta_mac_addr_r3[47:32];    end
            `STA00_MAC_ADDR + 14'h0020        : begin reg_data_r <= sta_mac_addr_r4[31:0];     end
            `STA00_MAC_ADDR + 14'h0024        : begin reg_data_r <= sta_mac_addr_r4[47:32];    end
            `STA00_MAC_ADDR + 14'h0028        : begin reg_data_r <= sta_mac_addr_r5[31:0];     end
            `STA00_MAC_ADDR + 14'h002C        : begin reg_data_r <= sta_mac_addr_r5[47:32];    end
            `STA00_MAC_ADDR + 14'h0030        : begin reg_data_r <= sta_mac_addr_r6[31:0];     end
            `STA00_MAC_ADDR + 14'h0034        : begin reg_data_r <= sta_mac_addr_r6[47:32];    end
            `STA00_MAC_ADDR + 14'h0038        : begin reg_data_r <= sta_mac_addr_r7[31:0];     end
            `STA00_MAC_ADDR + 14'h003C        : begin reg_data_r <= sta_mac_addr_r7[47:32];    end
            `STA00_MAC_ADDR + 14'h0040        : begin reg_data_r <= sta_mac_addr_r8[31:0];     end
            `STA00_MAC_ADDR + 14'h0044        : begin reg_data_r <= sta_mac_addr_r8[47:32];    end
            `STA00_MAC_ADDR + 14'h0048        : begin reg_data_r <= sta_mac_addr_r9[31:0];     end
            `STA00_MAC_ADDR + 14'h004C        : begin reg_data_r <= sta_mac_addr_r9[47:32];    end
            `STA00_MAC_ADDR + 14'h0050        : begin reg_data_r <= sta_mac_addr_r10[31:0];    end
            `STA00_MAC_ADDR + 14'h0054        : begin reg_data_r <= sta_mac_addr_r10[47:32];   end
            `STA00_MAC_ADDR + 14'h0058        : begin reg_data_r <= sta_mac_addr_r11[31:0];    end
            `STA00_MAC_ADDR + 14'h005C        : begin reg_data_r <= sta_mac_addr_r11[47:32];   end
            `STA00_MAC_ADDR + 14'h0060        : begin reg_data_r <= sta_mac_addr_r12[31:0];    end
            `STA00_MAC_ADDR + 14'h0064        : begin reg_data_r <= sta_mac_addr_r12[47:32];   end
            `STA00_MAC_ADDR + 14'h0068        : begin reg_data_r <= sta_mac_addr_r13[31:0];    end
            `STA00_MAC_ADDR + 14'h006C        : begin reg_data_r <= sta_mac_addr_r13[47:32];   end
            `STA00_MAC_ADDR + 14'h0070        : begin reg_data_r <= sta_mac_addr_r14[31:0];    end
            `STA00_MAC_ADDR + 14'h0074        : begin reg_data_r <= sta_mac_addr_r14[47:32];   end
            `STA00_MAC_ADDR + 14'h0078        : begin reg_data_r <= sta_mac_addr_r15[31:0];    end
            `STA00_MAC_ADDR + 14'h007C        : begin reg_data_r <= sta_mac_addr_r15[47:32];   end
            `STA00_MAC_ADDR + 14'h0080        : begin reg_data_r <= sta_mac_addr_r16[31:0];    end
            `STA00_MAC_ADDR + 14'h0084        : begin reg_data_r <= sta_mac_addr_r16[47:32];   end
            `STA00_MAC_ADDR + 14'h0088        : begin reg_data_r <= sta_mac_addr_r17[31:0];    end
            `STA00_MAC_ADDR + 14'h008C        : begin reg_data_r <= sta_mac_addr_r17[47:32];   end
            `STA00_MAC_ADDR + 14'h0090        : begin reg_data_r <= sta_mac_addr_r18[31:0];    end
            `STA00_MAC_ADDR + 14'h0094        : begin reg_data_r <= sta_mac_addr_r18[47:32];   end
            `STA00_MAC_ADDR + 14'h0098        : begin reg_data_r <= sta_mac_addr_r19[31:0];    end
            `STA00_MAC_ADDR + 14'h009C        : begin reg_data_r <= sta_mac_addr_r19[47:32];   end
            `TX_STATUS_AC0_FLAG               : begin reg_data_r <= sta_tx_status_flag_ac0_r;  end 
            `TX_STATUS_AC1_FLAG               : begin reg_data_r <= sta_tx_status_flag_ac1_r;  end 
            `TX_STATUS_AC2_FLAG               : begin reg_data_r <= sta_tx_status_flag_ac2_r;  end 
            `TX_STATUS_AC3_FLAG               : begin reg_data_r <= sta_tx_status_flag_ac3_r;  end 
            `TX_STATUS_AC0_OFFSET + 14'h0000  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[0], sta_retry_count_ac0_r[0], rsvd1, sta_seqno_ac0_r[0] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0004  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0008  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[1], sta_retry_count_ac0_r[1], rsvd1, sta_seqno_ac0_r[1] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h000C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0010  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[2], sta_retry_count_ac0_r[2], rsvd1, sta_seqno_ac0_r[2] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0014  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0018  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[3], sta_retry_count_ac0_r[3], rsvd1, sta_seqno_ac0_r[3] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h001C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0020  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[4], sta_retry_count_ac0_r[4], rsvd1, sta_seqno_ac0_r[4] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0024  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0028  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[5], sta_retry_count_ac0_r[5], rsvd1, sta_seqno_ac0_r[5] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h002C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0030  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[6], sta_retry_count_ac0_r[6], rsvd1, sta_seqno_ac0_r[6] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0034  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0038  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[7], sta_retry_count_ac0_r[7], rsvd1, sta_seqno_ac0_r[7] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h003C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0040  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[8], sta_retry_count_ac0_r[8], rsvd1, sta_seqno_ac0_r[8] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0044  : begin reg_data_r <= 32'h0; end  
            `TX_STATUS_AC0_OFFSET + 14'h0048  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[9], sta_retry_count_ac0_r[9], rsvd1, sta_seqno_ac0_r[9] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h004C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0050  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[10], sta_retry_count_ac0_r[10], rsvd1, sta_seqno_ac0_r[10] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0054  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0058  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[11], sta_retry_count_ac0_r[11], rsvd1, sta_seqno_ac0_r[11] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h005C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0060  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[12], sta_retry_count_ac0_r[12], rsvd1, sta_seqno_ac0_r[12] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0064  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0068  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[13], sta_retry_count_ac0_r[13], rsvd1, sta_seqno_ac0_r[13] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h006C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0070  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[14], sta_retry_count_ac0_r[14], rsvd1, sta_seqno_ac0_r[14] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0074  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0078  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[15], sta_retry_count_ac0_r[15], rsvd1, sta_seqno_ac0_r[15] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h007C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0080  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[16], sta_retry_count_ac0_r[16], rsvd1, sta_seqno_ac0_r[16] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0084  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0088  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[17], sta_retry_count_ac0_r[17], rsvd1, sta_seqno_ac0_r[17] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h008C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0090  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[18], sta_retry_count_ac0_r[18], rsvd1, sta_seqno_ac0_r[18] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h0094  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h0098  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[19], sta_retry_count_ac0_r[19], rsvd1, sta_seqno_ac0_r[19] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h009C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00A0  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[20], sta_retry_count_ac0_r[20], rsvd1, sta_seqno_ac0_r[20] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00A4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00A8  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[21], sta_retry_count_ac0_r[21], rsvd1, sta_seqno_ac0_r[21] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00AC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00B0  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[22], sta_retry_count_ac0_r[22], rsvd1, sta_seqno_ac0_r[22] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00B4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00B8  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[23], sta_retry_count_ac0_r[23], rsvd1, sta_seqno_ac0_r[23] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00BC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00C0  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[24], sta_retry_count_ac0_r[24], rsvd1, sta_seqno_ac0_r[24] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00C4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00C8  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[25], sta_retry_count_ac0_r[25], rsvd1, sta_seqno_ac0_r[25] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00CC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00D0  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[26], sta_retry_count_ac0_r[26], rsvd1, sta_seqno_ac0_r[26] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00D4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00D8  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[27], sta_retry_count_ac0_r[27], rsvd1, sta_seqno_ac0_r[27] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00DC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00E0  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[28], sta_retry_count_ac0_r[28], rsvd1, sta_seqno_ac0_r[28] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00E4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00E8  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[29], sta_retry_count_ac0_r[29], rsvd1, sta_seqno_ac0_r[29] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00EC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00F0  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[30], sta_retry_count_ac0_r[30], rsvd1, sta_seqno_ac0_r[30] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00F4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC0_OFFSET + 14'h00F8  : begin reg_data_r <= { rsvd2, sta_status_ac0_r[31], sta_retry_count_ac0_r[31], rsvd1, sta_seqno_ac0_r[31] }; end   
            `TX_STATUS_AC0_OFFSET + 14'h00FC  : begin reg_data_r <= 32'h0; end
            `TX_STATUS_AC1_OFFSET + 14'h0000  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[0], sta_retry_count_ac1_r[0], rsvd1, sta_seqno_ac1_r[0] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0004  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0008  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[1], sta_retry_count_ac1_r[1], rsvd1, sta_seqno_ac1_r[1] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h000C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0010  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[2], sta_retry_count_ac1_r[2], rsvd1, sta_seqno_ac1_r[2] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0014  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0018  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[3], sta_retry_count_ac1_r[3], rsvd1, sta_seqno_ac1_r[3] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h001C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0020  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[4], sta_retry_count_ac1_r[4], rsvd1, sta_seqno_ac1_r[4] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0024  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0028  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[5], sta_retry_count_ac1_r[5], rsvd1, sta_seqno_ac1_r[5] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h002C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0030  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[6], sta_retry_count_ac1_r[6], rsvd1, sta_seqno_ac1_r[6] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0034  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0038  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[7], sta_retry_count_ac1_r[7], rsvd1, sta_seqno_ac1_r[7] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h003C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0040  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[8], sta_retry_count_ac1_r[8], rsvd1, sta_seqno_ac1_r[8] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0044  : begin reg_data_r <= 32'h0; end  
            `TX_STATUS_AC1_OFFSET + 14'h0048  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[9], sta_retry_count_ac1_r[9], rsvd1, sta_seqno_ac1_r[9] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h004C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0050  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[10], sta_retry_count_ac1_r[10], rsvd1, sta_seqno_ac1_r[10] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0054  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0058  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[11], sta_retry_count_ac1_r[11], rsvd1, sta_seqno_ac1_r[11] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h005C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0060  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[12], sta_retry_count_ac1_r[12], rsvd1, sta_seqno_ac1_r[12] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0064  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0068  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[13], sta_retry_count_ac1_r[13], rsvd1, sta_seqno_ac1_r[13] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h006C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0070  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[14], sta_retry_count_ac1_r[14], rsvd1, sta_seqno_ac1_r[14] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0074  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0078  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[15], sta_retry_count_ac1_r[15], rsvd1, sta_seqno_ac1_r[15] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h007C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0080  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[16], sta_retry_count_ac1_r[16], rsvd1, sta_seqno_ac1_r[16] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0084  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0088  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[17], sta_retry_count_ac1_r[17], rsvd1, sta_seqno_ac1_r[17] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h008C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0090  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[18], sta_retry_count_ac1_r[18], rsvd1, sta_seqno_ac1_r[18] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h0094  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h0098  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[19], sta_retry_count_ac1_r[19], rsvd1, sta_seqno_ac1_r[19] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h009C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00A0  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[20], sta_retry_count_ac1_r[20], rsvd1, sta_seqno_ac1_r[20] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00A4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00A8  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[21], sta_retry_count_ac1_r[21], rsvd1, sta_seqno_ac1_r[21] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00AC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00B0  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[22], sta_retry_count_ac1_r[22], rsvd1, sta_seqno_ac1_r[22] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00B4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00B8  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[23], sta_retry_count_ac1_r[23], rsvd1, sta_seqno_ac1_r[23] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00BC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00C0  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[24], sta_retry_count_ac1_r[24], rsvd1, sta_seqno_ac1_r[24] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00C4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00C8  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[25], sta_retry_count_ac1_r[25], rsvd1, sta_seqno_ac1_r[25] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00CC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00D0  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[26], sta_retry_count_ac1_r[26], rsvd1, sta_seqno_ac1_r[26] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00D4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00D8  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[27], sta_retry_count_ac1_r[27], rsvd1, sta_seqno_ac1_r[27] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00DC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00E0  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[28], sta_retry_count_ac1_r[28], rsvd1, sta_seqno_ac1_r[28] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00E4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00E8  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[29], sta_retry_count_ac1_r[29], rsvd1, sta_seqno_ac1_r[29] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00EC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00F0  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[30], sta_retry_count_ac1_r[30], rsvd1, sta_seqno_ac1_r[30] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00F4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC1_OFFSET + 14'h00F8  : begin reg_data_r <= { rsvd2, sta_status_ac1_r[31], sta_retry_count_ac1_r[31], rsvd1, sta_seqno_ac1_r[31] }; end   
            `TX_STATUS_AC1_OFFSET + 14'h00FC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0000  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[0], sta_retry_count_ac2_r[0], rsvd1, sta_seqno_ac2_r[0] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0004  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0008  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[1], sta_retry_count_ac2_r[1], rsvd1, sta_seqno_ac2_r[1] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h000C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0010  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[2], sta_retry_count_ac2_r[2], rsvd1, sta_seqno_ac2_r[2] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0014  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0018  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[3], sta_retry_count_ac2_r[3], rsvd1, sta_seqno_ac2_r[3] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h001C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0020  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[4], sta_retry_count_ac2_r[4], rsvd1, sta_seqno_ac2_r[4] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0024  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0028  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[5], sta_retry_count_ac2_r[5], rsvd1, sta_seqno_ac2_r[5] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h002C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0030  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[6], sta_retry_count_ac2_r[6], rsvd1, sta_seqno_ac2_r[6] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0034  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0038  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[7], sta_retry_count_ac2_r[7], rsvd1, sta_seqno_ac2_r[7] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h003C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0040  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[8], sta_retry_count_ac2_r[8], rsvd1, sta_seqno_ac2_r[8] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0044  : begin reg_data_r <= 32'h0; end  
            `TX_STATUS_AC2_OFFSET + 14'h0048  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[9], sta_retry_count_ac2_r[9], rsvd1, sta_seqno_ac2_r[9] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h004C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0050  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[10], sta_retry_count_ac2_r[10], rsvd1, sta_seqno_ac2_r[10] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0054  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0058  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[11], sta_retry_count_ac2_r[11], rsvd1, sta_seqno_ac2_r[11] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h005C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0060  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[12], sta_retry_count_ac2_r[12], rsvd1, sta_seqno_ac2_r[12] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0064  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0068  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[13], sta_retry_count_ac2_r[13], rsvd1, sta_seqno_ac2_r[13] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h006C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0070  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[14], sta_retry_count_ac2_r[14], rsvd1, sta_seqno_ac2_r[14] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0074  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0078  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[15], sta_retry_count_ac2_r[15], rsvd1, sta_seqno_ac2_r[15] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h007C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0080  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[16], sta_retry_count_ac2_r[16], rsvd1, sta_seqno_ac2_r[16] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0084  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0088  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[17], sta_retry_count_ac2_r[17], rsvd1, sta_seqno_ac2_r[17] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h008C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0090  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[18], sta_retry_count_ac2_r[18], rsvd1, sta_seqno_ac2_r[18] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h0094  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h0098  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[19], sta_retry_count_ac2_r[19], rsvd1, sta_seqno_ac2_r[19] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h009C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00A0  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[20], sta_retry_count_ac2_r[20], rsvd1, sta_seqno_ac2_r[20] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00A4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00A8  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[21], sta_retry_count_ac2_r[21], rsvd1, sta_seqno_ac2_r[21] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00AC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00B0  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[22], sta_retry_count_ac2_r[22], rsvd1, sta_seqno_ac2_r[22] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00B4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00B8  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[23], sta_retry_count_ac2_r[23], rsvd1, sta_seqno_ac2_r[23] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00BC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00C0  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[24], sta_retry_count_ac2_r[24], rsvd1, sta_seqno_ac2_r[24] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00C4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00C8  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[25], sta_retry_count_ac2_r[25], rsvd1, sta_seqno_ac2_r[25] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00CC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00D0  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[26], sta_retry_count_ac2_r[26], rsvd1, sta_seqno_ac2_r[26] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00D4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00D8  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[27], sta_retry_count_ac2_r[27], rsvd1, sta_seqno_ac2_r[27] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00DC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00E0  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[28], sta_retry_count_ac2_r[28], rsvd1, sta_seqno_ac2_r[28] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00E4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00E8  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[29], sta_retry_count_ac2_r[29], rsvd1, sta_seqno_ac2_r[29] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00EC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00F0  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[30], sta_retry_count_ac2_r[30], rsvd1, sta_seqno_ac2_r[30] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00F4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC2_OFFSET + 14'h00F8  : begin reg_data_r <= { rsvd2, sta_status_ac2_r[31], sta_retry_count_ac2_r[31], rsvd1, sta_seqno_ac2_r[31] }; end   
            `TX_STATUS_AC2_OFFSET + 14'h00FC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0000  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[0], sta_retry_count_ac3_r[0], rsvd1, sta_seqno_ac3_r[0] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0004  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0008  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[1], sta_retry_count_ac3_r[1], rsvd1, sta_seqno_ac3_r[1] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h000C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0010  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[2], sta_retry_count_ac3_r[2], rsvd1, sta_seqno_ac3_r[2] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0014  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0018  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[3], sta_retry_count_ac3_r[3], rsvd1, sta_seqno_ac3_r[3] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h001C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0020  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[4], sta_retry_count_ac3_r[4], rsvd1, sta_seqno_ac3_r[4] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0024  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0028  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[5], sta_retry_count_ac3_r[5], rsvd1, sta_seqno_ac3_r[5] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h002C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0030  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[6], sta_retry_count_ac3_r[6], rsvd1, sta_seqno_ac3_r[6] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0034  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0038  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[7], sta_retry_count_ac3_r[7], rsvd1, sta_seqno_ac3_r[7] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h003C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0040  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[8], sta_retry_count_ac3_r[8], rsvd1, sta_seqno_ac3_r[8] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0044  : begin reg_data_r <= 32'h0; end  
            `TX_STATUS_AC3_OFFSET + 14'h0048  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[9], sta_retry_count_ac3_r[9], rsvd1, sta_seqno_ac3_r[9] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h004C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0050  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[10], sta_retry_count_ac3_r[10], rsvd1, sta_seqno_ac3_r[10] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0054  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0058  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[11], sta_retry_count_ac3_r[11], rsvd1, sta_seqno_ac3_r[11] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h005C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0060  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[12], sta_retry_count_ac3_r[12], rsvd1, sta_seqno_ac3_r[12] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0064  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0068  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[13], sta_retry_count_ac3_r[13], rsvd1, sta_seqno_ac3_r[13] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h006C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0070  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[14], sta_retry_count_ac3_r[14], rsvd1, sta_seqno_ac3_r[14] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0074  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0078  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[15], sta_retry_count_ac3_r[15], rsvd1, sta_seqno_ac3_r[15] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h007C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0080  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[16], sta_retry_count_ac3_r[16], rsvd1, sta_seqno_ac3_r[16] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0084  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0088  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[17], sta_retry_count_ac3_r[17], rsvd1, sta_seqno_ac3_r[17] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h008C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0090  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[18], sta_retry_count_ac3_r[18], rsvd1, sta_seqno_ac3_r[18] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h0094  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h0098  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[19], sta_retry_count_ac3_r[19], rsvd1, sta_seqno_ac3_r[19] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h009C  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00A0  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[20], sta_retry_count_ac3_r[20], rsvd1, sta_seqno_ac3_r[20] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00A4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00A8  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[21], sta_retry_count_ac3_r[21], rsvd1, sta_seqno_ac3_r[21] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00AC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00B0  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[22], sta_retry_count_ac3_r[22], rsvd1, sta_seqno_ac3_r[22] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00B4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00B8  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[23], sta_retry_count_ac3_r[23], rsvd1, sta_seqno_ac3_r[23] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00BC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00C0  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[24], sta_retry_count_ac3_r[24], rsvd1, sta_seqno_ac3_r[24] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00C4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00C8  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[25], sta_retry_count_ac3_r[25], rsvd1, sta_seqno_ac3_r[25] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00CC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00D0  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[26], sta_retry_count_ac3_r[26], rsvd1, sta_seqno_ac3_r[26] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00D4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00D8  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[27], sta_retry_count_ac3_r[27], rsvd1, sta_seqno_ac3_r[27] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00DC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00E0  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[28], sta_retry_count_ac3_r[28], rsvd1, sta_seqno_ac3_r[28] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00E4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00E8  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[29], sta_retry_count_ac3_r[29], rsvd1, sta_seqno_ac3_r[29] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00EC  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00F0  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[30], sta_retry_count_ac3_r[30], rsvd1, sta_seqno_ac3_r[30] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00F4  : begin reg_data_r <= 32'h0; end   
            `TX_STATUS_AC3_OFFSET + 14'h00F8  : begin reg_data_r <= { rsvd2, sta_status_ac3_r[31], sta_retry_count_ac3_r[31], rsvd1, sta_seqno_ac3_r[31] }; end   
            `TX_STATUS_AC3_OFFSET + 14'h00FC  : begin reg_data_r <= 32'h0; end    
            default: begin reg_data_r <= 32'h0; end
         endcase
      end
   end   

// Always Block to Compare the MAC Address Validity and 
// Calculate the BA_ADDRESS offset
   always @(posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
      begin
         mac_valid_r <= 1'b0; 
         ba_addr_r <= 14'h0;
      end
      else if( !sta_enable )
      begin
         mac_valid_r <= 1'b0; 
         ba_addr_r <= 14'h0;
      end
      else if(( state == IDLE ) && sta_in_get_ba )
      begin   
         case( sta_in_mac_addr ) // synopsys full case // synopsys parallel case
            sta_mac_addr_r0  : begin 
                                    if( sta_is_used_r[0] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA00_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1],2'b00}); 
                                    end
                                    else
                                    begin 
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end     
            sta_mac_addr_r1  : begin
                                    if( sta_is_used_r[1] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA01_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r2  : begin 
                                    if( sta_is_used_r[2] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA02_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r3  : begin      
                                    if( sta_is_used_r[3] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA03_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r4  : begin
                                    if( sta_is_used_r[4] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA04_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r5  : begin      
                                    if( sta_is_used_r[5] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA05_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r6  : begin     
                                    if( sta_is_used_r[6] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA06_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r7  : begin     
                                    if( sta_is_used_r[7] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA07_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r8  : begin     
                                    if( sta_is_used_r[8] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA08_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r9  : begin     
                                    if( sta_is_used_r[9] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA09_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r10 : begin     
                                    if( sta_is_used_r[10] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA10_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r11 : begin     
                                    if( sta_is_used_r[11] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA11_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r12 : begin     
                                    if( sta_is_used_r[12] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA12_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r13 : begin     
                                    if( sta_is_used_r[13] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA13_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r14 : begin     
                                    if( sta_is_used_r[14] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA14_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r15 : begin     
                                    if( sta_is_used_r[15] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA15_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r16 : begin     
                                    if( sta_is_used_r[16] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA16_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r17 : begin     
                                    if( sta_is_used_r[17] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA17_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r18 : begin     
                                    if( sta_is_used_r[18] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA18_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            sta_mac_addr_r19 : begin     
                                    if( sta_is_used_r[19] )
                                    begin
                                       mac_valid_r <= 1'b1; 
                                       ba_addr_r  <= `STA19_INFO_OFFSET + (sta_in_dir ? {10'd0, sta_in_tid[2:1], 2'b0} : {10'b1, sta_in_tid[2:1], 2'b00}); 
                                    end
                                    else 
                                    begin
                                       mac_valid_r <= 1'b0; 
                                       ba_addr_r <= 14'h0;
                                    end
                                 end
            default: begin mac_valid_r <= 1'b0; 
                           ba_addr_r <= 14'h0;
                     end
         endcase   
      end
   end

// Always Block for BA VALID signal to CP or Tx 
// if deasserted it indicates that the BA Address is not valid.
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_out_ba_valid <= 1'b0;
      else if( !sta_enable || sta_out_ba_valid ) 
         sta_out_ba_valid <= 1'b0;
      else if((state == READ_ADDR))
      begin
         if((sta_in_tid[0]) && (mem_out_data[29:16] != 14'h0))
            sta_out_ba_valid <= 1'b1;
         else if((~sta_in_tid[0]) && (mem_out_data[13:0] != 14'h0))  
            sta_out_ba_valid <= 1'b1;
         else
            sta_out_ba_valid <= 1'b0;       
      end
   end

// Always Block for Generation of BA_OFFSET_VALID signal to CP or Tx
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_out_get_ba_ack <= 1'b0;
      else if( !sta_enable || sta_out_get_ba_ack )
         sta_out_get_ba_ack <= 1'b0;
      else if( state == READ_ADDR )
         sta_out_get_ba_ack <= 1'b1;
   end

// Always Block for sending the BA Offset Address to the CP or Tx
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_out_ba_offset <= 14'h0;
      else if( !sta_enable )
         sta_out_ba_offset <= 14'h0;
      else if( state == READ_ADDR )
      begin
         if( sta_in_tid[0] )
            sta_out_ba_offset <=  (mem_out_data[31:16]-`STA00_INFO_OFFSET)>>2;
         else
            sta_out_ba_offset <=  (mem_out_data[15:0]-`STA00_INFO_OFFSET)>>2 ;  
      end
   end

// Always Block to Generate an Acknowledgement for the UMAC Read
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_out_umac_ack <= 1'b0;
      else if( !sta_enable )
         sta_out_umac_ack <= 1'b0;
      else if( !access_memory ) 
         sta_out_umac_ack <= sta_in_umac_val;
      else
         sta_out_umac_ack <= ( sta_in_umac_val && access_memory && !sta_in_val ) ;
   end

// Always Block to Generate an Acknowledgement for CP or Tx Read
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         sta_ack_r <= 1'b0;
      else if( !sta_enable )
         sta_ack_r <= 1'b0;
      else if( !access_memory ) 
         sta_ack_r <= ( sta_in_val && ~sta_in_get_ba);  
      else
         sta_ack_r <= ( sta_in_val && access_memory );
   end

// Always Block for the Current State Information
   always @ ( posedge sta_clk or `EDGE_OF_RESET )
   begin
      if( `POLARITY_OF_RESET )
         state <= IDLE;
      else if( !sta_enable )
         state <= IDLE;
      else 
         state <= next_state;
   end   

// Always Block for the combinational logic
   always @( * )
   begin
      next_state = state;
      if(sta_enable)
      begin
         case( state )
            IDLE : 
            begin
               if( sta_in_get_ba )  
                  next_state = COMPARE_ADDR;
               else 
                  next_state = IDLE;
            end
            COMPARE_ADDR : 
            begin
               if( mac_valid_r )
                  next_state = READ_ADDR; 
               else 
                  next_state = IDLE;
            end
            READ_ADDR : 
            begin
               next_state = IDLE;
            end
            default: begin next_state = IDLE; end
         endcase
      end                
   end

endmodule
