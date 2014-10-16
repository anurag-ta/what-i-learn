`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:       Shreeharsha.B.V
// 
// Create Date:    15:53:48 11/29/2013 
// Design Name: 
// Module Name:    fifo_logic 
// Project Name:   LMAC
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
`include "D:/UURMI/acmac/macphy_intgr/fifo_logic/inc/defines.h"

module uu_fifo_logic
     #(parameter          prog_full_thresh =  4'h7 )
      (
       input                         wr_clk                   ,
       input                         rd_clk                   ,
       input                         rst_n                    ,
       input                         enable                   ,
       // IO interface between LMAC and FIFO module 
       input                         mac2fifo_txstart_req     , // LMAC to FIFO txstart request input 
       input                         mac2fifo_txend_req       , // LMAC to FIFO txend request input
       input        [7 :0]           mac2fifo_phy_frame       , // LMAC output frame to fifo [7:0](vector or data)
       input                         mac2fifo_frame_val       , // LMAC to FIFO frame valid input
       output                        fifo2mac_txstart_confirm , // FIFO to LMAC txstart confirm output
       output                        fifo2mac_txdata_confirm  , // FIFO to LMAC txdata confirm output
       output                        fifo2mac_txend_confirm	  , // FIFO to LMAC txend confirm output
       // IO interface between PHY and FIFO module
       input                         phy2fifo_txstart_confirm , // PHY to FIFO txstart confirm input
       input                         phy2fifo_txdata_confirm  , // PHY to FIFO txdata confirm input
       input                         phy2fifo_txend_confirm   , // PHY to FIFO txend confirm input
       output                        fifo2phy_txstart_req     , // FIFO to PHY txstart req output
       output       [7 :0]           fifo2phy_phy_frame       , // FIFO to PHY output frame [7:0] (vector or data)
       output                        fifo2phy_frame_val       , // FIFO to PHY frame valid output
       output  reg	             fifo2phy_txend_req         // FIFO to PHY txend request output
      );

// Local Signal Declaration
       reg                 loc_txvector_phase          ;
       reg                 loc_txdata_phase            ;
       reg                 loc_txend_phase             ;
       reg                 loc_fifo2mac_txdata_confirm ;

       wire                reset                       ;
       wire                loc_fifo_full               ;
       wire                loc_fifo2phy_frame_val      ;
       wire                loc_mac2fifo_frame_val      ;
       wire  [7 :0]        loc_fifo2phy_frame          ;
       wire  [7 :0]        loc_mac2phy_frame           ;
       wire                loc_prog_full               ;
       wire  [3 :0]        rd_data_count               ;
       wire  [3 :0]        wr_data_count               ;
       wire                loc_fifo_empty              ;

	
//=======================================================================
//--------------------- Concurrent Assignments---------------------------
//-----------------------------------------------------------------------
	
  assign  reset = ~rst_n;
  assign  fifo2phy_txstart_req = mac2fifo_txstart_req;
  assign  fifo2mac_txstart_confirm = phy2fifo_txstart_confirm; 
  assign  fifo2phy_phy_frame = ( loc_txvector_phase == 1 | mac2fifo_txstart_req) ? mac2fifo_phy_frame : loc_fifo2phy_frame ;
  assign  fifo2phy_frame_val = ( loc_txvector_phase == 1 | mac2fifo_txstart_req) ? mac2fifo_frame_val : loc_fifo2phy_frame_val ;

  assign  fifo2mac_txdata_confirm = ( (loc_txdata_phase == 1) ) ? loc_fifo2mac_txdata_confirm : 1'b0;  
  assign  loc_mac2fifo_frame_val = ( (loc_txdata_phase == 1) ) ?  mac2fifo_frame_val : 1'b0;   
  assign  loc_mac2phy_frame = ( loc_mac2fifo_frame_val == 1 ) ? mac2fifo_phy_frame : 8'h00 ;
  assign  fifo2mac_txend_confirm = phy2fifo_txend_confirm;

	
//=======================================================================

// Tx start request latching to allow Tx vector to pass through
  always@( posedge wr_clk or `EDGE_OF_RESET)
  begin
     if( `POLARITY_OF_RESET )   
       loc_txvector_phase <= 1'b0;
     else if( !enable )
       loc_txvector_phase <= 1'b0;
     else if( mac2fifo_txstart_req )
       loc_txvector_phase <= 1'b1;
     else if( phy2fifo_txstart_confirm )
       loc_txvector_phase <= 1'b0;
  end

// Local signal to enable FIFO filling
  always@( posedge wr_clk or `EDGE_OF_RESET)
  begin
     if( `POLARITY_OF_RESET ) 
       loc_txdata_phase <= 1'b0;
     else if( !enable )
       loc_txdata_phase <= 1'b0;
     else if( phy2fifo_txstart_confirm )
       loc_txdata_phase <= 1'b1;
     else if( wr_data_count == (prog_full_thresh -1) & !loc_prog_full )
       loc_txdata_phase <= 1'b0;
     else if( phy2fifo_txdata_confirm & !loc_txend_phase )
       loc_txdata_phase <= 1'b1;
     else if( loc_txend_phase |  fifo2mac_txend_confirm )
       loc_txdata_phase <= 1'b0;
  end

// Local signal to latch txdata confirm
  always@( posedge wr_clk or `EDGE_OF_RESET)
  begin
     if( `POLARITY_OF_RESET )
       loc_fifo2mac_txdata_confirm <= 1'b0;
     else if( !enable)
       loc_fifo2mac_txdata_confirm <= 1'b0;
     else if( loc_txend_phase | loc_fifo_full | mac2fifo_txend_req)
       loc_fifo2mac_txdata_confirm <= 1'b0;
     else if( loc_txdata_phase | phy2fifo_txstart_confirm )
       loc_fifo2mac_txdata_confirm <= ~loc_fifo2mac_txdata_confirm;
     else 
       loc_fifo2mac_txdata_confirm <= 1'b0;
  end
		
// Local signal to latch txend request
  always@( posedge wr_clk or `EDGE_OF_RESET)
  begin
     if( `POLARITY_OF_RESET )
       loc_txend_phase <= 1'b0;
     else if( !enable )
       loc_txend_phase <= 1'b0;
     else if( mac2fifo_txend_req )
       loc_txend_phase <= 1'b1;
     else if( loc_fifo_empty )
       loc_txend_phase <= 1'b0;
  end		

// FIFO to PHY txend request
  always@( posedge wr_clk or `EDGE_OF_RESET)
  begin
     if( `POLARITY_OF_RESET )
       fifo2phy_txend_req <= 1'b0;
     else if( !enable )
       fifo2phy_txend_req <= 1'b0;
     else if( loc_txend_phase & loc_fifo_empty)
       fifo2phy_txend_req <= 1'b1;
     else
       fifo2phy_txend_req <= 1'b0;
  end

// FIFO to sync txdata transmission
  data_fifo fifo (
     .rst                      ( reset                  ), // input rst
     .wr_clk                   ( wr_clk                 ), // input wr_clk
     .rd_clk                   ( rd_clk                 ), // input rd_clk
     .din                      ( loc_mac2phy_frame      ), // input [7 : 0] din
     .wr_en                    ( loc_mac2fifo_frame_val ), // input wr_en
     .rd_en                    ( phy2fifo_txdata_confirm), // input rd_en
     .prog_full_thresh         ( prog_full_thresh       ), // input [3 : 0] prog_full_thresh
     .dout                     ( loc_fifo2phy_frame     ), // output [7 : 0] dout
     .full                     ( loc_fifo_full          ), // output full
     .empty                    ( loc_fifo_empty         ), // output empty
     .valid                    ( loc_fifo2phy_frame_val ), // output valid
     .rd_data_count            ( rd_data_count          ), // output [3 : 0] rd_data_count
     .wr_data_count            ( wr_data_count          ), // output [3 : 0] wr_data_count
     .prog_full                ( loc_prog_full          ) // output prog_full
    );

endmodule
