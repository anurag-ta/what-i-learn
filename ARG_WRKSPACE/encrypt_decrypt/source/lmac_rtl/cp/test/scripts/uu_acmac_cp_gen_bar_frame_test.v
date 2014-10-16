`timescale 1ns/1ns
`include "../../inc/defines.h"
module uu_acmac_cp_gen_bar_frame_test;


int 		sym_len=1;
int 		err;
reg 		temp;
int 		i;
reg [47:0] rcvd_ra = 0;
reg [1:0] rcvd_rate;

reg rate;
reg [47:0] data_frame_input;
reg [1:0]  format;
reg [7:0] postcondition;
reg self_cts_flag;
`include "../../cp/test/vectors/uu_acmac_cp_tx_path_of_cp_include.v" 

initial begin
	repeat(19)@(uu_acmac_cp_tb.clk);
	for(int i=1; i<=sym_len; i++) begin
	#1;
        uu_acmac_cp_tb.sym_no = i;
	drive(i);
	uu_acmac_cp_tb.cp_en <= 1;
	uu_acmac_cp_tb.cp_in_bar_valid <= 1;
	uu_acmac_cp_tb.cp_in_bar_address <= 48'h042010f0f8fc;
	uu_acmac_cp_tb.cp_in_bar_scf <= 16'hb0;
	uu_acmac_cp_tb.cp_in_bar_tid <= 3;
	uu_acmac_cp_tb.cp_in_bar_duration <= 16'hff;
	uu_acmac_cp_tb.cp_in_bar_bartype <= 'd5;
          
        

	@(posedge uu_acmac_cp_tb.clk);
	#1;
	uu_acmac_cp_tb.cp_in_bar_valid<= 0;
	repeat(15)@(posedge uu_acmac_cp_tb.clk);
	@(posedge uu_acmac_cp_tb.clk);
	end


repeat(1000)@(uu_acmac_cp_tb.clk);
$finish(2);
end

//---------------CHECKERS---------------//
 //----compare the cp_out_packet_length with the return_value
reg [15:0] return_value = 24;
initial 
 begin
  for(int i=1; i<=sym_len; i++) 
    begin
     @(posedge uu_acmac_cp_tb.CP_WRAP.CP_IF.cp_out_tx_data_val );
     @(negedge uu_acmac_cp_tb.clk);
      if (uu_acmac_cp_tb.CP_WRAP.CP_IF.cp_out_packet_length == return_value) 
       begin
        $display ("                 RETURN_VALUE PASS ");
       end
      else
       begin
      $display("           ERROR:RETURN VALUE : %d %d %d\n", i,uu_acmac_cp_tb.CP_WRAP.CP_IF.cp_out_packet_length,return_value);
      err++; 
       end

 end
end  
 



endmodule
