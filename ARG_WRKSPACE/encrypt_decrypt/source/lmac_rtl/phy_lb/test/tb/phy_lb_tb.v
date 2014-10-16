`timescale 1ns / 1ps

////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer:
//
// Create Date:   10:33:56 08/26/2013
// Design Name:   mac_phy
// Module Name:   D:/Zynq7000/UURMI/mac_phy/macphy_tb.v
// Project Name:  mac_phy
// Target Device:  
// Tool versions:  
// Description: 
//
// Verilog Test Fixture created by ISE for module: mac_phy
//
// Dependencies:
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
////////////////////////////////////////////////////////////////////////////////

module macphy_tb;

	// Inputs
	reg clk;
	reg rst_n;
	reg macphy_en;
	reg mac2phy_in_txstart_req;
	reg mac2phy_in_txdata_req;
	reg mac2phy_in_txend_req;
	reg mac2phy_in_txdata_val;
	reg [7:0] mac2phy_in_txdata_frame;
	reg mac2phy_in_ccareset_req;

	// Outputs
	wire phy2mac_out_txstart_confirm;
	wire phy2mac_out_txdata_confirm;
	wire phy2mac_out_txend_confirm;
	wire phy2mac_out_ccareset_confirm;
	wire phy2mac_out_rxstart_ind;
	wire phy2mac_out_rxdata_ind;
	wire phy2mac_out_rxend_ind;
	wire phy2mac_out_rxdata_val;
	wire [7:0] phy2mac_out_rxdata;
	
parameter PERIOD = 10;

	// Instantiate the Unit Under Test (UUT)
	phy_lb uut (
		.clk(clk), 
		.rst_n(rst_n), 
		.macphy_en(macphy_en), 
		.mac2phy_in_txstart_req(mac2phy_in_txstart_req), 
		.mac2phy_in_txdata_req(mac2phy_in_txdata_req), 
		.mac2phy_in_txend_req(mac2phy_in_txend_req), 
		.mac2phy_in_txdata_val(mac2phy_in_txdata_val),
		.mac2phy_in_txdata_frame(mac2phy_in_txdata_frame), 
		.phy2mac_out_txstart_confirm(phy2mac_out_txstart_confirm), 
		.phy2mac_out_txdata_confirm(phy2mac_out_txdata_confirm), 
		.phy2mac_out_txend_confirm(phy2mac_out_txend_confirm), 
		.mac2phy_in_ccareset_req(mac2phy_in_ccareset_req), 
		.phy2mac_out_ccareset_confirm(phy2mac_out_ccareset_confirm),
                .phy2mac_out_rxdata_val(phy2mac_out_rxdata_val),		
		.phy2mac_out_rxstart_ind(phy2mac_out_rxstart_ind), 
		.phy2mac_out_rxdata_ind(phy2mac_out_rxdata_ind), 
		.phy2mac_out_rxend_ind(phy2mac_out_rxend_ind), 
		.phy2mac_out_rxdata(phy2mac_out_rxdata)
	);
 
 // Add stimulus here
 initial 
 begin
     clk = 0;
     forever #(PERIOD/2) clk = ~clk;	
 end
 
 initial 
 begin
     // Initialize Inputs
     rst_n = 0;
     macphy_en = 0;
     mac2phy_in_txstart_req = 0;
     mac2phy_in_txdata_req = 0;
     mac2phy_in_txend_req = 0;
     mac2phy_in_txdata_val = 0;
     mac2phy_in_txdata_frame = 0;
     mac2phy_in_ccareset_req = 0;
 end
	
 initial 
 begin
     @(negedge clk)
     #1;
     rst_n = 1'b1;
     @(negedge clk)
     rst_n = 1'b0;
     macphy_en = 1'b1;

     // Sending the Tx Vector	
     @(negedge clk)
     txvector();
	
     // Sending the Tx Data
     repeat(10) @(negedge clk);
     txdata();

     // Sending the Tx End
     repeat(10) @(negedge clk);
     txend();

     repeat(500) @(negedge clk);
     $stop();
 end

// Task for Tx Vector
 task txvector();
 begin: vector
     integer i;
     i = 0;

     repeat(10) @(negedge clk);
     mac2phy_in_txstart_req = 1'b1;
     @(negedge clk);
     mac2phy_in_txstart_req = 1'b0;
     @(negedge clk);
		
     while(i < 31)
     begin
         mac2phy_in_txdata_val =1'b1;
         mac2phy_in_txdata_frame = i + 8'h4E;
         i = i+1;
         
         @(negedge clk);
     end
		
     mac2phy_in_txdata_val =1'b0;
     mac2phy_in_txdata_frame = 0;
     wait(phy2mac_out_txstart_confirm);
 end		
 endtask

 // Task for Tx Data
 task txdata();
 begin: data
     integer i;
     integer k;

     k = 100 % $random(i);
     i = 0;

     while(i < k)
     begin
         mac2phy_in_txdata_frame = i + 8'h1E;
         mac2phy_in_txdata_val =1'b1;
         mac2phy_in_txdata_req = 1'b1;
         i = i + 1;
				 
         wait(phy2mac_out_txdata_confirm);		 
         @(negedge clk);
     end
	  
     mac2phy_in_txdata_val =1'b0;
     mac2phy_in_txdata_req = 1'b0;
     mac2phy_in_txdata_frame = 0;		
 end
 endtask

 // Task for Tx End
 task txend();
 begin
     @(negedge clk);
     mac2phy_in_txend_req = 1'b1;
     @(negedge clk);
     mac2phy_in_txend_req = 1'b0;
     @(negedge clk);
     mac2phy_in_ccareset_req = 1'b1;
     @(negedge clk);
     mac2phy_in_ccareset_req = 1'b0;
 end
 endtask

endmodule

