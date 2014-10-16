`timescale 1ns/1ns
`include "../../inc/defines.h"
module uu_acmac_cp_tx_path_of_cp_test;


event		start,start2;
int 		sym_len=8;
int 		err;
int 		i,a1,a2;

reg 		temp;
reg [47:0] rcvd_ra = 0;
reg [47:0] rcvd_ta = 0;
reg rate;
reg [47:0] data_frame_input;
reg [1:0]  format;
reg [15:0] postcondition;
reg self_cts_flag;

`include "../../cp/test/vectors/uu_acmac_cp_tx_path_of_cp_include.v" 

initial begin
	repeat(19)@(uu_acmac_cp_tb.clk);
	for(int i=1; i<=sym_len; i++) begin
	#1;
        uu_acmac_cp_tb.sym_no = i;
	drive(i);
	uu_acmac_cp_tb.cp_en <= 1;
	uu_acmac_cp_tb.cp_in_sta_ba_data <= 0;
	uu_acmac_cp_tb.cp_in_tx_start_ind <= 1;
	uu_acmac_cp_tb.cp_in_tx_self_cts  <= self_cts_flag;
	uu_acmac_cp_tb.cp_in_tx_txvec_format <= format;
	uu_acmac_cp_tb.cp_in_tx_rtscts_rate <= rate;
	uu_acmac_cp_tb.cp_in_tx_mpdu_ra <= data_frame_input; 
	uu_acmac_cp_tb.cp_in_is_rxend <= 0;
	uu_acmac_cp_tb.cp_in_waiting_for_cts <= 0;
	uu_acmac_cp_tb.cp_in_waiting_for_ack <= 0;
	@(posedge uu_acmac_cp_tb.clk);
	#1;
	uu_acmac_cp_tb.cp_in_tx_start_ind <= 0;
	repeat(15)@(posedge uu_acmac_cp_tb.clk);
	@(negedge uu_acmac_cp_tb.CP_WRAP.CP_IF.loc_cp_tx_busy);
	@(posedge uu_acmac_cp_tb.clk);
	end
	uu_acmac_cp_tb.cp_en   <= 0;

 repeat(1000)@(uu_acmac_cp_tb.clk);
 $finish(2);
end

//---------------CHECKERS---------------//

//---- compare ra field --------// 

initial begin
  for(int i=1; i<=sym_len; i++) begin
    @(posedge uu_acmac_cp_tb.cp_out_crc_data_val );
     @(negedge uu_acmac_cp_tb.clk);
     #12;
    for (int j=0; j<10; j++) begin
       @(posedge uu_acmac_cp_tb.clk);
        rcvd_ra[47:0] <= {rcvd_ra[47:0],uu_acmac_cp_tb.cp_out_crc_data};
     //$display ($time, "j value : %d", j);
     //$display ($time, "rcvd_ra value : %h", rcvd_ra);
     end
     
      if (rcvd_ra == 48'h042010f0f8fc) 
      begin
       $display ("                    RA PASS SYMB  : ",i      );
      end
    else
     begin
      $display("           ERROR:RA SYMB : %d %h %h\n", i,rcvd_ra,48'h042010f0f8fc);
      err++; 
     end
   @(negedge uu_acmac_cp_tb.CP_WRAP.CP_IF.loc_cp_tx_busy);
  end
end

//--------Compare  ta field--------//
// this field should be compared only if the self_cts_flag is 0
 initial begin
  for(int i=1; i<=sym_len; i++) begin
    @(posedge uu_acmac_cp_tb.cp_out_crc_data_val );
     @(negedge uu_acmac_cp_tb.clk);
     if (self_cts_flag == 0)
      begin
       #119
        for (int j=0; j<6 ; j++) begin
         @(posedge uu_acmac_cp_tb.clk);
          rcvd_ta[47:0] <= {rcvd_ta[47:0],uu_acmac_cp_tb.cp_out_crc_data};
            //$display ($time, "j value : %d", j);
            //$display ($time, "rcvd_ta value : %h", rcvd_ta);
      end
      if (rcvd_ta == 48'h00FF00FF00FF)     
      begin
       $display ("                    TA PASS SYMB  : ",i      );
      end
      else
     begin
      $display("           ERROR:TA SYMB : %d %h %h\n", i,rcvd_ta,48'hFF00FF00FF00);
      err++; 
     end
  end
   @(negedge uu_acmac_cp_tb.CP_WRAP.CP_IF.loc_cp_tx_busy);
  end
  end

//---------Compare format --------//	
//---compare the output format with the postcondition [1:0]

initial 
 begin
  for(int i=1; i<=sym_len; i++) begin
    @(posedge uu_acmac_cp_tb.cp_out_crc_data_val );
     @(negedge uu_acmac_cp_tb.clk);
      if (uu_acmac_cp_tb.CP_WRAP.CP_IF.cp_out_txvec_format  == postcondition[1:0]) 
       begin
        $display ("                 FORMAT PASS SYMB : ", i);
       end
      else
       begin
      $display("           ERROR:FORMAT SYMB : %d %d %d\n", i,uu_acmac_cp_tb.CP_WRAP.CP_IF.cp_out_txvec_format,postcondition[1:0]);
      err++; 
       end
   @(negedge uu_acmac_cp_tb.CP_WRAP.CP_IF.loc_cp_tx_busy);

 end
end

//---------Compare rate -----------//
// -----if format = 0 then rate = cp_out_txvec_L_datarate
//      else rate = cp_out_txvec_mcs
//-----compare the rate with the postcondition [3:2]

initial 
 begin //{
  for(int i=1; i<=sym_len; i++) begin //{
    @(posedge uu_acmac_cp_tb.cp_out_crc_data_val );
     if (uu_acmac_cp_tb.CP_WRAP.CP_IF.cp_out_txvec_format== 2'b00) 
      begin//{
       uu_acmac_cp_tb.rcvd_rate = uu_acmac_cp_tb.CP_WRAP.CP_IF.cp_out_txvec_L_datarate;
       $display ($time,"rcvd_rate_1  : %d" , uu_acmac_cp_tb.rcvd_rate);
       if (uu_acmac_cp_tb.rcvd_rate == postcondition[15:8])
        begin //{
         $display ("                    RATE PASS SYMB  : ",i      );
        end //}
       else
        begin //{
         $display("           ERROR:RATE SYMB : %d %h %h\n", i,uu_acmac_cp_tb.rcvd_rate,postcondition[15:8]);
         err++; 
        end //}
      end //}
    else
      begin //{
     uu_acmac_cp_tb.rcvd_rate = uu_acmac_cp_tb.CP_WRAP.CP_IF.cp_out_txvec_mcs;
      $display ($time,"rcvd_rate_2  : %d" , uu_acmac_cp_tb.rcvd_rate);
      if (uu_acmac_cp_tb.rcvd_rate == postcondition[15:8])
        begin //{
         $display ("                    RATE PASS SYMB  : ",i      );
        end //}
       else
        begin //{
         $display("           ERROR:RATE SYMB : %d %h %h\n", i,uu_acmac_cp_tb.rcvd_rate,postcondition[15:8]);
         err++; 
        end //}
      end //}
   @(negedge uu_acmac_cp_tb.CP_WRAP.CP_IF.loc_cp_tx_busy);
     end //}
 end//}


endmodule
