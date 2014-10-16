`include "/home/uurmi/lmac_loopback_test/inc/defines.h"

module uu_acmac_tx_buf(
	input           clk                       ,
	input           rst_n                     ,
	//DMA interface
	input   [1 :0]  tx_buf_in_dma_ac          ,
	input           tx_buf_in_dma_wen         ,
	input   [31:0]  tx_buf_in_dma_data        ,
	//Tx ready event from UMAC
	input           tx_in_ev_tx_ready         ,
	output          tx_out_ev_tx_ready        ,
	output  reg        tx_out_ac0_frame_info     ,
	output  reg       tx_out_ac1_frame_info     ,
	output  reg      tx_out_ac2_frame_info     ,
	output  reg       tx_out_ac3_frame_info     ,
	//Requests from Modules
	input           tx_buf_in_clear_mpdu      ,
	input           tx_buf_in_clear_ampdu     ,
	//tx_handler interface
	input   [1 :0]  tx_buf_in_ac              ,
	output  [31:0]  tx_buf_out_frame_data     ,
	input   [13:0]  tx_buf_in_frame_addr      ,
	input           tx_buf_in_frame_en        ,
	input    tx_buf_in_data_end_confirm,//vibha
      input    tx_buf_in_frame_info_retry,//vibha
	
	input tx_buf_in_ac0_int_col_r,//vibha
	input tx_buf_in_ac1_int_col_r,//vibha
	input tx_buf_in_ac2_int_col_r,//vibha
	input tx_buf_in_ac3_int_col_r,//vibha
	//output pointers to tx 
	output tx_buf_out_rd_ac0,//vibha
	output tx_buf_out_rd_ac1,//vibha
	output tx_buf_out_rd_ac2,//vibha
	output tx_buf_out_rd_ac3,//vibha
	output  [13:0]  tx_buf_out_base_ptr_ac0       ,//vibha
	output  [13:0]  tx_buf_out_base_ptr_ac1       ,//vibha
	output  [13:0]  tx_buf_out_base_ptr_ac2       ,//vibha
	output  [13:0]  tx_buf_out_base_ptr_ac3       ,//vibha
	input   [19:0]  tx_buf_in_pkt_len         
	);


	/* Internal registers
	.....................................................................................*/
	reg   [13:0]  loc_reg_write_ptr [3:0];
	reg   [13:0]  loc_reg_base_ptr  [3:0];
        reg           loc_tx_ready           ;


	reg   [7:0] count_ac0;
	reg   [7:0] count_ac1;
	reg   [7:0] count_ac2;
	reg   [7:0] count_ac3;
	
	reg [2:0]cnt;
	
	reg en_txbuf0_read;
	reg en_txbuf1_read;
	reg en_txbuf2_read;
	reg en_txbuf3_read;
	
	reg ac0_int_col_r_dly1;
	reg ac1_int_col_r_dly1;
	reg ac2_int_col_r_dly1;
	reg ac3_int_col_r_dly1;
	
	reg ac0_int_col_r_dly2;
	reg ac1_int_col_r_dly2;
	reg ac2_int_col_r_dly2;
	reg ac3_int_col_r_dly2;

      reg loc_frame_info_retry;

	
	wire en_txbuf0;
	wire en_txbuf1;
	wire en_txbuf2;
	wire en_txbuf3;
	
	wire  [31:0]  loc_mem_dout_ac0  ;
	wire  [31:0]  loc_mem_dout_ac1  ;
	wire  [31:0]  loc_mem_dout_ac2  ;
	wire  [31:0]  loc_mem_dout_ac3  ;
	wire          loc_mem_en_ac0    ;
	wire          loc_mem_en_ac1    ;
	wire          loc_mem_en_ac2    ;
	wire          loc_mem_en_ac3    ;
	wire          loc_mem_wen_ac0   ;
	wire          loc_mem_wen_ac1   ;
	wire          loc_mem_wen_ac2   ;
	wire          loc_mem_wen_ac3   ;
	wire  [13:0]  loc_mem_addr_ac0  ;
	wire  [13:0]  loc_mem_addr_ac1  ;
	wire  [13:0]  loc_mem_addr_ac2  ;
	wire  [13:0]  loc_mem_addr_ac3  ;
	wire [13:0] frame_read_addr_ac0;
	wire [13:0] frame_read_addr_ac1;
	wire [13:0] frame_read_addr_ac2;
	wire [13:0] frame_read_addr_ac3;
	wire          loc_pkt_add       ;
	reg decr_count_ac0;
	reg decr_count_ac1;
	reg decr_count_ac2;
	reg decr_count_ac3;
	
	reg data_end_confirm_buf;
	/*-----------------------------------------------------------------------------------*/

	/* Module instantiations
	.....................................................................................*/
	//16K memory per AC

	uu_acmac_tx_mem TX_MEM_AC0 (
		.clka   (clk                  ),
		.wea    (loc_mem_wen_ac0      ),
		.addra  (loc_mem_addr_ac0     ),
		.dina   (tx_buf_in_dma_data   ),
		.clkb   (clk                  ),
		.enb    (en_txbuf0   ),
		.addrb  (frame_read_addr_ac0 ),
		.doutb  (loc_mem_dout_ac0     )
	);

	uu_acmac_tx_mem TX_MEM_AC1 (
		.clka   (clk                  ),
		.wea    (loc_mem_wen_ac1      ),
		.addra  (loc_mem_addr_ac1     ),
		.dina   (tx_buf_in_dma_data   ),
		.clkb   (clk                  ),
		.enb    (en_txbuf1   ),
		.addrb  (frame_read_addr_ac1 ),
		.doutb  (loc_mem_dout_ac1     ) 
	);

	uu_acmac_tx_mem TX_MEM_AC2 (
		.clka   (clk                  ),
		.wea    (loc_mem_wen_ac2      ),
		.addra  (loc_mem_addr_ac2     ),
		.dina   (tx_buf_in_dma_data   ),
		.clkb   (clk                  ),
		.enb    (en_txbuf2  ),
		.addrb  (frame_read_addr_ac2 ),
		.doutb  (loc_mem_dout_ac2     )
	);

	uu_acmac_tx_mem TX_MEM_AC3 (
		.clka   (clk                  ),
		.wea    (loc_mem_wen_ac3      ),
		.addra  (loc_mem_addr_ac3     ),
		.dina   (tx_buf_in_dma_data   ),
		.clkb   (clk                  ),
		.enb    (en_txbuf3   ),
		.addrb  (frame_read_addr_ac3 ),
		.doutb  (loc_mem_dout_ac3     )
	);

	/*-----------------------------------------------------------------------------------*/


	/* Continuous assignments
	.....................................................................................*/
	assign tx_buf_out_rd_ac0=en_txbuf0;
	assign tx_buf_out_rd_ac1=en_txbuf1;
	assign tx_buf_out_rd_ac2=en_txbuf2;
	assign tx_buf_out_rd_ac3=en_txbuf3;
//       assign tx_out_ac0_frame_info = (count_ac0 == 0) ? 0 : 1;
//       assign tx_out_ac1_frame_info = (count_ac1 == 0) ? 0 : 1;
//       assign tx_out_ac2_frame_info = (count_ac2 == 0) ? 0 : 1;
//       assign tx_out_ac3_frame_info = (count_ac3 == 0) ? 0 : 1;
		always @(*)begin
		if (count_ac0>0 && !count_ac0[7])begin
		   tx_out_ac0_frame_info<=1'b1;
		   tx_out_ac1_frame_info<=1'b0;
		   tx_out_ac2_frame_info<=1'b0;
		   tx_out_ac3_frame_info<=1'b0;end		
//      else if (tx_buf_in_data_end_confirm)begin
//		  	tx_out_ac0_frame_info<=1'b0;
//		   tx_out_ac1_frame_info<=1'b0;
//		   tx_out_ac2_frame_info<=1'b0;
//		   tx_out_ac3_frame_info<=1'b0;end		        		
		else if (count_ac1>0 && !count_ac1[7])begin
		   tx_out_ac0_frame_info<=1'b0;
		   tx_out_ac1_frame_info<=1'b1;
		   tx_out_ac2_frame_info<=1'b0;
		   tx_out_ac3_frame_info<=1'b0;end				
		else if (count_ac2>0 && !count_ac2[7])begin
		   tx_out_ac0_frame_info<=1'b0;
		   tx_out_ac1_frame_info<=1'b0;
		   tx_out_ac2_frame_info<=1'b1;
		   tx_out_ac3_frame_info<=1'b0;end			
		else if (count_ac3>0 && !count_ac3[7])begin
		   tx_out_ac0_frame_info<=1'b0;
		   tx_out_ac1_frame_info<=1'b0;
		   tx_out_ac2_frame_info<=1'b0;
		   tx_out_ac3_frame_info<=1'b1;end			
		else begin
			tx_out_ac0_frame_info<=0;
			tx_out_ac1_frame_info<=0;
			tx_out_ac2_frame_info<=0;
			tx_out_ac3_frame_info<=0;end
		
		end
//	assign tx_buf_out_frame_data = (tx_buf_in_ac == 0) ? loc_mem_dout_ac0 :
//	                              ((tx_buf_in_ac == 1) ? loc_mem_dout_ac1 :
//							((tx_buf_in_ac == 2) ? loc_mem_dout_ac2 : loc_mem_dout_ac3));

	assign tx_buf_out_frame_data = (en_txbuf0) ? loc_mem_dout_ac0 ://vibha
	                              ((en_txbuf1) ? loc_mem_dout_ac1 :
							((en_txbuf2) ? loc_mem_dout_ac2 : loc_mem_dout_ac3));
//	assign tx_buf_out_base_ptr   = (tx_buf_in_ac == 0) ? loc_reg_base_ptr [0] ://vibha
//	                              ((tx_buf_in_ac == 1) ? loc_reg_base_ptr [1] :
//				               ((tx_buf_in_ac == 2) ? loc_reg_base_ptr [2] : loc_reg_base_ptr [3]));
assign tx_buf_out_base_ptr_ac0=loc_reg_base_ptr [0];
assign tx_buf_out_base_ptr_ac1=loc_reg_base_ptr [1];
assign tx_buf_out_base_ptr_ac2=loc_reg_base_ptr [2];
assign tx_buf_out_base_ptr_ac3=loc_reg_base_ptr [3];
	assign frame_read_addr_ac0=(en_txbuf0== 1) ? tx_buf_in_frame_addr:0;
	assign frame_read_addr_ac1=(en_txbuf1== 1) ? tx_buf_in_frame_addr:0;
	assign frame_read_addr_ac2=(en_txbuf2 ==1) ? tx_buf_in_frame_addr:0;
	assign frame_read_addr_ac3=(en_txbuf3== 1) ? tx_buf_in_frame_addr:0;
		always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
          data_end_confirm_buf<=0;
		else if (data_end_confirm_buf)
		     data_end_confirm_buf<=0;   
		else if (tx_buf_in_data_end_confirm)
		      data_end_confirm_buf<=1;
		end
		always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			en_txbuf0_read <= 1'b0;
			en_txbuf1_read<=1'b0;
			en_txbuf2_read<=1'b0;
			en_txbuf3_read<=1'b0;
			cnt<=0;end	
		else if( data_end_confirm_buf && cnt==0)begin
			en_txbuf0_read<=1'b0;
			cnt<=1;end	
		else if (data_end_confirm_buf && cnt==1 )begin
		   en_txbuf1_read<=1'b0;
			cnt<=2;end
		else if (data_end_confirm_buf && cnt==2 )begin
		   en_txbuf2_read<=1'b0;
			cnt<=3;end
		else if (data_end_confirm_buf && cnt==3 )begin
		   en_txbuf3_read<=1'b0;
			cnt<=4;end
		else if (tx_buf_in_dma_ac==0 && cnt==0)
			en_txbuf0_read<=1'b1;
		else if (tx_buf_in_dma_ac==1 && cnt==0)
			en_txbuf1_read<=1'b1;
		else if (tx_buf_in_dma_ac==2 && cnt==0)
			en_txbuf2_read<=1'b1;
		else if (tx_buf_in_dma_ac==3 && cnt==0)
			en_txbuf3_read<=1'b1;
		end
		assign en_txbuf0=(tx_buf_in_data_end_confirm)?0:
												((en_txbuf0_read ||loc_frame_info_retry) && tx_buf_in_frame_en)?tx_buf_in_frame_en:0;
		assign en_txbuf1=(tx_buf_in_data_end_confirm)?0:
												((en_txbuf1_read ||loc_frame_info_retry) && tx_buf_in_frame_en && !en_txbuf0)?tx_buf_in_frame_en:0;
		assign en_txbuf2=(tx_buf_in_data_end_confirm)?0:
												((en_txbuf2_read ||loc_frame_info_retry) && tx_buf_in_frame_en  && !en_txbuf1 && !en_txbuf0)? tx_buf_in_frame_en:0;
		assign en_txbuf3=(tx_buf_in_data_end_confirm)?0:
												((en_txbuf3_read ||loc_frame_info_retry) && tx_buf_in_frame_en && !en_txbuf2 && !en_txbuf1 && !en_txbuf0)?tx_buf_in_frame_en:0;
		
      
		always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
				decr_count_ac0<=1'b0;
				decr_count_ac1<=1'b0;
				decr_count_ac2<=1'b0;
				decr_count_ac3<=1'b0;end
		else begin
				if (en_txbuf0)
						decr_count_ac0<=1'b1;
				if (en_txbuf1)
						decr_count_ac1<=1'b1;
				if (en_txbuf2)
						decr_count_ac2<=1'b1;
				if (en_txbuf3)
						decr_count_ac3<=1'b1;
		end
		end
/*	assign tx_buf_free_memory_ac0 = (loc_reg_write_ptr[0] > loc_reg_base_ptr[0]) ? (12'd4095 - (loc_reg_write_ptr[0] - loc_reg_base_ptr[0])) : 
	                                                                                   (loc_reg_base_ptr[0] - loc_reg_write_ptr[0]);
	assign tx_buf_free_memory_ac1 = (loc_reg_write_ptr[1] > loc_reg_base_ptr[1]) ? (12'd4095 - (loc_reg_write_ptr[1] - loc_reg_base_ptr[1])) : 
	                                                                                   (loc_reg_base_ptr[1] - loc_reg_write_ptr[1]);
	assign tx_buf_free_memory_ac2 = (loc_reg_write_ptr[2] > loc_reg_base_ptr[2]) ? (12'd4095 - (loc_reg_write_ptr[2] - loc_reg_base_ptr[2])) : 
	                                                                                   (loc_reg_base_ptr[2] - loc_reg_write_ptr[2]);
	assign tx_buf_free_memory_ac3 = (loc_reg_write_ptr[3] > loc_reg_base_ptr[3]) ? (12'd4095 - (loc_reg_write_ptr[3] - loc_reg_base_ptr[3])) : 
	                                                                                   (loc_reg_base_ptr[3] - loc_reg_write_ptr[3]);
*/	
	assign loc_mem_wen_ac0 = (tx_buf_in_dma_ac == 0) ? tx_buf_in_dma_wen : 0;
	assign loc_mem_wen_ac1 = (tx_buf_in_dma_ac == 1) ? tx_buf_in_dma_wen : 0;
	assign loc_mem_wen_ac2 = (tx_buf_in_dma_ac == 2) ? tx_buf_in_dma_wen : 0;
	assign loc_mem_wen_ac3 = (tx_buf_in_dma_ac == 3) ? tx_buf_in_dma_wen : 0;

	assign loc_mem_addr_ac0 = (tx_buf_in_dma_wen&&(tx_buf_in_dma_ac == 0)) ? loc_reg_write_ptr[0] : 0;
	assign loc_mem_addr_ac1 = (tx_buf_in_dma_wen&&(tx_buf_in_dma_ac == 1)) ? loc_reg_write_ptr[1] : 0;
	assign loc_mem_addr_ac2 = (tx_buf_in_dma_wen&&(tx_buf_in_dma_ac == 2)) ? loc_reg_write_ptr[2] : 0;
	assign loc_mem_addr_ac3 = (tx_buf_in_dma_wen&&(tx_buf_in_dma_ac == 3)) ? loc_reg_write_ptr[3] : 0;

	assign loc_pkt_add = ((tx_buf_in_pkt_len%4)!=0) ? 1 : 0;
	assign tx_out_ev_tx_ready = loc_tx_ready;

	/*-----------------------------------------------------------------------------------*/


	/* Concurrent statements
	.....................................................................................*/
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_tx_ready <= 0;
		end
		else if(tx_in_ev_tx_ready)begin
			loc_tx_ready <= 1'b1;
			end
		else
		  		loc_tx_ready <= 1'b0;
		end
	
	//Maintaining base and write pointers
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_write_ptr[0] <= 0;
		end
		else if(loc_mem_wen_ac0)begin
			loc_reg_write_ptr[0] <= loc_reg_write_ptr[0] + 1'b1;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_write_ptr[1] <= 0;
		end
		else if(loc_mem_wen_ac1)begin
			loc_reg_write_ptr[1] <= loc_reg_write_ptr[1] + 1'b1;
		end
	end
	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_write_ptr[2] <= 0;
		end
		else if(loc_mem_wen_ac2)begin
			loc_reg_write_ptr[2] <= loc_reg_write_ptr[2] + 1'b1;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_write_ptr[3] <= 0;
		end
		else if(loc_mem_wen_ac3)begin
			loc_reg_write_ptr[3] <= loc_reg_write_ptr[3] + 1'b1;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_base_ptr[0] <= 0;
		end
		else if((tx_buf_in_clear_mpdu || tx_buf_in_clear_ampdu)&&(tx_buf_in_ac==0))begin
			loc_reg_base_ptr[0] <= loc_reg_base_ptr[0] + loc_pkt_add + (tx_buf_in_pkt_len>>2) + 8'd22;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_base_ptr[1] <= 0;
		end
		else if((tx_buf_in_clear_mpdu || tx_buf_in_clear_ampdu)&&(tx_buf_in_ac==1))begin
			loc_reg_base_ptr[1] <= loc_reg_base_ptr[1] + loc_pkt_add + (tx_buf_in_pkt_len>>2) + 8'd22;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_base_ptr[2] <= 0;
		end
		else if((tx_buf_in_clear_mpdu || tx_buf_in_clear_ampdu)&&(tx_buf_in_ac==2))begin
			loc_reg_base_ptr[2] <= loc_reg_base_ptr[2] + loc_pkt_add + (tx_buf_in_pkt_len>>2) + 8'd22;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_base_ptr[3] <= 0;
		end
		else if((tx_buf_in_clear_mpdu || tx_buf_in_clear_ampdu)&&(tx_buf_in_ac==3) && !ac0_int_col_r_dly2 && !ac1_int_col_r_dly2 && !ac2_int_col_r_dly2 && !ac3_int_col_r_dly2)begin
			loc_reg_base_ptr[3] <= loc_reg_base_ptr[3] + loc_pkt_add + (tx_buf_in_pkt_len>>2) + 8'd22;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			count_ac0 <= 0;
		else if(tx_in_ev_tx_ready && (en_txbuf0_read))
			count_ac0 <= count_ac0 + 1;
		else if((tx_buf_in_clear_mpdu || tx_buf_in_clear_ampdu) && decr_count_ac0)
			count_ac0 <= count_ac0 - 1;
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			count_ac1 <= 0;
		else if(tx_in_ev_tx_ready && (en_txbuf1_read))
			count_ac1 <= count_ac1 + 1;
		else if((tx_buf_in_clear_mpdu || tx_buf_in_clear_ampdu) && decr_count_ac1)
			count_ac1 <= count_ac1 - 1;
	end
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			count_ac2 <= 0;
		else if(tx_in_ev_tx_ready && (en_txbuf2_read) )
			count_ac2 <= count_ac2 + 1;
		else if((tx_buf_in_clear_mpdu || tx_buf_in_clear_ampdu)&& decr_count_ac2)
			count_ac2 <= count_ac2 - 1;
	end
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			count_ac3 <= 0;
		else if(tx_in_ev_tx_ready && (en_txbuf3_read))
			count_ac3 <= count_ac3 + 1;
		else if((tx_buf_in_clear_mpdu || tx_buf_in_clear_ampdu) && decr_count_ac3)
			count_ac3 <= count_ac3 - 1;
	end
	//Delaying the internal collision signals by 2 clock cycles to synchronize with clear mpdu
	
	always @(posedge clk or `EDGE_OF_RESET)begin//vibha
		if(`POLARITY_OF_RESET) begin
		ac0_int_col_r_dly1<=0;
		ac1_int_col_r_dly1<=0;
		ac2_int_col_r_dly1<=0;
		ac3_int_col_r_dly1<=0;
		
		ac0_int_col_r_dly2<=0;
		ac1_int_col_r_dly2<=0;
		ac2_int_col_r_dly2<=0;
		ac3_int_col_r_dly2<=0;end

		
		else begin
		ac0_int_col_r_dly1<=tx_buf_in_ac0_int_col_r;
		ac0_int_col_r_dly2<=ac0_int_col_r_dly1;
		
		ac1_int_col_r_dly1<=tx_buf_in_ac1_int_col_r;
		ac1_int_col_r_dly2<=ac1_int_col_r_dly1;
		
		ac2_int_col_r_dly1<=tx_buf_in_ac2_int_col_r;
		ac2_int_col_r_dly2<=ac2_int_col_r_dly1;
		
		ac3_int_col_r_dly1<=tx_buf_in_ac3_int_col_r;
		ac3_int_col_r_dly2<=ac3_int_col_r_dly1;
end
		end




      always @(posedge clk or `EDGE_OF_RESET)begin//vibha
		        if(`POLARITY_OF_RESET) 
               loc_frame_info_retry<=0;            
            else if (tx_buf_in_frame_info_retry)
               loc_frame_info_retry<=1;
            else if (tx_buf_in_data_end_confirm)
               loc_frame_info_retry<=0;
               
       end


	/*-----------------------------------------------------------------------------------*/
endmodule
/*-------------------------------------------EOF------------------------------------*/

