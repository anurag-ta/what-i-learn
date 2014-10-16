`include "../../inc/defines.h" 

module uu_acmac_tx_update_bitmap(
	input                clk                    ,
	input                rst_n                  ,
	input                tx_bmp_en              ,
	//Interface to Memory sta_ba_info 
	output  reg  [31:0]  tx_bmp_out_sta_ba_data ,
	output  reg  [3 :0]  tx_bmp_out_sta_ba_wen  , 
	output  reg          tx_bmp_out_sta_ba_en   ,
	input        [31:0]  tx_bmp_in_sta_ba_data  ,
	output  reg  [14:0]  tx_bmp_out_sta_ba_addr ,
	//Station & BA Management interface 
	input                tx_bmp_in_sta_info_val ,
	input                tx_bmp_in_sta_info     ,
	input        [13:0]  tx_bmp_in_sta_offset   ,
	output               tx_bmp_out_sta_get_info,
	//Inputs and outputs of CP_IF 
	input        [15:0]  tx_bmp_in_mpdu_scf     ,
	output  reg          tx_bmp_out_rx_res_en   ,
	output  reg  [31:0]  tx_bmp_out_rx_res       
	);

	/* Registers internal to the module 
	......................................................*/
	reg  [15:0]  sb_winstart     ;
	reg  [15:0]  sb_winend       ;
	reg  [15:0]  sb_winsize      ;
	reg  [9 :0]  sb_buf_winstart ;
	reg  [15:0]  sb_new_winstart     ;
	reg  [15:0]  sb_new_winend       ;
	reg  [9 :0]  sb_new_buf_winstart ;
	reg          loc_winsize_rd  ;
	reg          loc_winsize_rd_d  ;
	reg          loc_clear_buf   ;
	reg  [5 :0]  clear_start     ;
	reg  [5 :0]  clear_end       ;
	reg  [5 :0]  loc_count       ;
	wire [87:0]  loc_sb_update   ;
	reg  [13:0]  loc_sta_offset  ;
	reg          loc_sta_info_avl;
	reg          loc_sta_info_val;
	reg          loc_upd_done    ;
	reg          loc_sta_get_info;
	reg          loc_sta_get_info_dly;

	/*----------------------------------------------------*/

	/* Continuous assignments 
	.................................................................................................*/
	assign loc_sb_update = sb_update(sb_buf_winstart,sb_winstart,sb_winend,sb_winsize,tx_bmp_in_mpdu_scf >> 4);
	assign tx_bmp_out_sta_get_info = loc_sta_get_info & ~loc_sta_get_info_dly;
	/*-----------------------------------------------------------------------------------------------*/
	
	/* Function for updating sta_ba_info
	.................................................................................................*/	
	function [87:0] sb_update;
		//(ba_sb_t *sb_p, uu_uint16 sn, uu_uint16 fn)
		input [9 :0] sb_buf_winstart;	  
		input [15:0] sb_winstart    ;
		input [15:0] sb_winend      ;
		input [15:0] sb_winsize     ;
		input [15:0] sn             ;
		reg   [15:0] sn_new         ;
		reg   [15:0] clear_start    ;
		reg   [15:0] clear_end      ;
		reg   [9 :0] sb_new_buf_winstart;
		reg   [15:0] sb_new_winstart;
		reg   [15:0] sb_new_winend  ;
		reg   [5 :0] index          ;
	begin
		if ((sb_winstart <= sn) && (sn <= sb_winend))begin
			index     = (sb_buf_winstart+sn-sb_winstart) & `BA_BASIC_BITMAP_BUF_MASK;
			sb_update = {2'b00,index,16'h0000,16'h0000,16'h0000,16'h0000,16'h0000};
		end //|= (1 << fn);
		else if ((sb_winend < sn) && (sn < sb_winstart + `BA_SEQN_RANGE_BY2))begin
			clear_start         = sb_buf_winstart + (sb_winend-sb_winstart) + 1;
			clear_end           = sb_buf_winstart + (sn - sb_winstart);
			sb_new_buf_winstart = (sb_buf_winstart + sn - sb_winend) & `BA_BASIC_BITMAP_BUF_MASK;
			sb_new_winstart     = sn - sb_winsize + 1;
			sb_new_winend       = sn;
			index = (sb_new_buf_winstart+sn-sb_new_winstart) & `BA_BASIC_BITMAP_BUF_MASK; //= (1 << fn);
			if (sb_new_winstart > `BA_MPDU_SEQN_MAX)begin
				sb_winstart = sb_new_winstart & `MPDU_SEQN_MASK;
				sb_winend   = sb_new_winend & `MPDU_SEQN_MASK;
			end
			sb_update = {2'b01,index,clear_start,clear_end,6'h0,sb_new_buf_winstart,sb_new_winstart,sb_new_winend};	       
		end
		else begin 
			sn_new = sn | 16'h1000;
			if ((sb_winstart <= sn_new) && (sn_new <= sb_winend))begin
				index     = (sb_buf_winstart+sn_new-sb_winstart) & `BA_BASIC_BITMAP_BUF_MASK;
				sb_update = {2'b00,index,16'h0000,16'h0000,16'h0000,16'h0000,16'h0000};
			end //|= (1 << fn);
			else if ((sb_winend < sn_new) && (sn_new < sb_winstart + `BA_SEQN_RANGE_BY2))begin
				clear_start         = sb_buf_winstart + (sb_winend-sb_winstart) + 1;
				clear_end           = sb_buf_winstart + (sn_new - sb_winstart);
				sb_new_buf_winstart = (sb_buf_winstart + sn_new - sb_winend) & `BA_BASIC_BITMAP_BUF_MASK;
				sb_new_winstart     = sn_new - sb_winsize + 1;
				sb_new_winend       = sn_new;
				index = (sb_new_buf_winstart+sn_new-sb_new_winstart) & `BA_BASIC_BITMAP_BUF_MASK;//= (1 << fn);
				if (sb_winstart > `BA_MPDU_SEQN_MAX)begin
					sb_winstart = sb_new_winstart & `MPDU_SEQN_MASK;
					sb_winend   = sb_new_winend & `MPDU_SEQN_MASK;
				end
				sb_update = {2'b01,index,clear_start,clear_end,6'h0,sb_new_buf_winstart,sb_new_winstart,sb_new_winend};		    
			end
			else
			sb_update = {2'b11,clear_start,clear_end,index,6'h0,sb_new_buf_winstart,sb_new_winstart,sb_new_winend};
		end
	end
	endfunction
	/*----------------------------------------------------------------------*/

	/* Concurrent assignments 
	............................................................................................*/
	always @(posedge clk or `EDGE_OF_RESET)begin
		if (`POLARITY_OF_RESET)begin
			tx_bmp_out_rx_res_en <= 0;
			tx_bmp_out_rx_res    <= `UU_FAILURE;
		end
		else if(!tx_bmp_en)begin
			tx_bmp_out_rx_res_en <= 0;
			tx_bmp_out_rx_res    <= `UU_FAILURE;
		end
		else if(loc_sta_info_avl && !loc_sta_info_val)begin
			tx_bmp_out_rx_res_en <= 1;
			tx_bmp_out_rx_res    <= `UU_BA_SESSION_INVALID;
		end
		else if(loc_winsize_rd && loc_sb_update[87])begin
			tx_bmp_out_rx_res_en <= 1;
			tx_bmp_out_rx_res    <= `UU_FAILURE;
		end
		else if(loc_upd_done)begin
			tx_bmp_out_rx_res_en <= 1;
			tx_bmp_out_rx_res    <= `UU_SUCCESS;		
		end
		else begin
			tx_bmp_out_rx_res_en <= 0;
			tx_bmp_out_rx_res    <= `UU_FAILURE;		
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if (`POLARITY_OF_RESET)begin
			loc_upd_done <= 0;
		end
		else if(!tx_bmp_en)begin
			loc_upd_done <= 0;
		end
		else if(loc_winsize_rd && !loc_sb_update[87])begin
			if(loc_sb_update[86])
				if(clear_start>(clear_end+2))
					loc_upd_done <= 1;
				else 
					loc_upd_done <= 0;
			else 
				if(loc_count == 10)
					loc_upd_done <= 1;
				else 
					loc_upd_done <= 0;
		end
		else begin
			loc_upd_done <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_sta_get_info <= 0;
		end
		else if(!tx_bmp_en)begin
			loc_sta_get_info <= 0;
		end
		else if(!loc_sta_info_avl && (loc_count == 0))begin
			loc_sta_get_info <= 1;
		end
		else begin
			loc_sta_get_info <= 0;
		end
	end
	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_sta_get_info_dly <= 0;
		end
		else if(!tx_bmp_en)begin
			loc_sta_get_info_dly <= 0;
		end
		else begin
			loc_sta_get_info_dly <= loc_sta_get_info;
		end
	end


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_sta_info_val <= 0;
		end
		else if(!tx_bmp_en)begin
			loc_sta_info_val <= 0;
		end
		else if(tx_bmp_in_sta_info_val)begin
		     if(tx_bmp_in_sta_info)
				loc_sta_info_val <= 1;
				else 
				loc_sta_info_val <= 0;
			end 		 
	 else if(loc_upd_done)begin
			loc_sta_info_val <= 0;
		end
	  
	end	
	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_sta_info_avl <= 0;
		end
		else if(!tx_bmp_en)begin
			loc_sta_info_avl <= 0;
		end
		else if(tx_bmp_in_sta_info_val)begin
			loc_sta_info_avl <= 1;
		end
		else if(loc_upd_done)begin
			loc_sta_info_avl <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			tx_bmp_out_sta_ba_wen  <= 0;
		end
		else if (!tx_bmp_en)begin
			tx_bmp_out_sta_ba_wen  <= 0;
		end
		else if(loc_upd_done)begin
			tx_bmp_out_sta_ba_wen  <= 0;
		end
		else if(loc_winsize_rd && !loc_sb_update[87])begin
			if(loc_sb_update[86])begin
				if(clear_start<=clear_end)begin
					if(clear_start%2)begin
						tx_bmp_out_sta_ba_wen  <= 4'h3;
					end
					else begin
						tx_bmp_out_sta_ba_wen  <= 4'hC;
					end
				end
				else if(clear_start==(clear_end+1))begin
					if(loc_sb_update[85:80]%2)
						tx_bmp_out_sta_ba_wen <=  4'h3;
					else 
						tx_bmp_out_sta_ba_wen  <= 4'hC;
				end
				else if(clear_start==(clear_end+2))begin
					tx_bmp_out_sta_ba_wen  <= 4'hC;
				end
				else if(clear_start==(clear_end+3))begin
					tx_bmp_out_sta_ba_wen  <= 4'h3;
				end
			end
			//no_clear
			else if(loc_count == 9)begin 
				if(loc_sb_update[85:80]%2)
					tx_bmp_out_sta_ba_wen <=  4'h3;
				else 
					tx_bmp_out_sta_ba_wen  <= 4'hC;
			end
			else begin
				tx_bmp_out_sta_ba_wen <= 0;
			end
		end
		else begin
			tx_bmp_out_sta_ba_wen  <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			tx_bmp_out_sta_ba_addr  <= 0;
		end
		else if(!tx_bmp_en)begin
			tx_bmp_out_sta_ba_addr  <= 0;
		end
		else if(loc_count == 0)begin
			tx_bmp_out_sta_ba_addr  <= loc_sta_offset+33;
		end
		else if(loc_count == 2)begin
			tx_bmp_out_sta_ba_addr  <= loc_sta_offset+66;
		end
		else if(loc_count == 4)begin
			tx_bmp_out_sta_ba_addr  <= loc_sta_offset+67;
		end
		else if(loc_winsize_rd && !loc_sb_update[87])begin
			if(loc_sb_update[86])begin
				if(clear_start<=clear_end)begin
					tx_bmp_out_sta_ba_addr  <= loc_sta_offset+1+(clear_start[5:0]>>1)+(clear_start%2);
				end
				else if(clear_start==(clear_end+1))begin
					tx_bmp_out_sta_ba_addr  <= ((loc_sta_offset+1)+(loc_sb_update[85:80]>>1))+(loc_sb_update[85:80]%2);
				end
				else if(clear_start==(clear_end+2))begin
					tx_bmp_out_sta_ba_addr  <= loc_sta_offset+66;
				end
				else if(clear_start==(clear_end+3))begin
					tx_bmp_out_sta_ba_addr  <= loc_sta_offset+67;
				end
			end
			else if(loc_count <= 9)begin
				tx_bmp_out_sta_ba_addr <= ((loc_sta_offset+1)+(loc_sb_update[85:80]>>1))+(loc_sb_update[85:80]%2);
			end
			else begin
				tx_bmp_out_sta_ba_addr <= 0;
			end
		end
		else begin
			tx_bmp_out_sta_ba_addr  <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			tx_bmp_out_sta_ba_data  <= 0;
		end
		else if(!tx_bmp_en)begin
			tx_bmp_out_sta_ba_data  <= 0;
		end
		//else if(loc_winsize_rd && !loc_sb_update[87])begin
		//	tx_bmp_out_sta_ba_data  <= loc_sb_update[86];
		//end
		else if(loc_clear_buf)begin
			if(clear_start<=clear_end ) begin
				tx_bmp_out_sta_ba_data <= 0;
			end
			else if(clear_start==(clear_end+1))begin
				tx_bmp_out_sta_ba_data <= (1 << tx_bmp_in_mpdu_scf[3:0]);
			end
			else if(clear_start==(clear_end+2))begin
				tx_bmp_out_sta_ba_data <= {sb_new_winstart,16'd0};
			end
			else if(clear_start==(clear_end+3))begin
				tx_bmp_out_sta_ba_data <= {16'd0,sb_new_winend};
			end
		end
		else if(loc_count == 9)begin
			if(loc_sb_update[85:80]%2)
				tx_bmp_out_sta_ba_data <= {16'd0,(tx_bmp_out_sta_ba_data[15:0]|(1 << tx_bmp_in_mpdu_scf[3:0]))};
			else 
				tx_bmp_out_sta_ba_data <= {(tx_bmp_out_sta_ba_data[31:16]|(1 << tx_bmp_in_mpdu_scf[3:0])),16'd0};
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			tx_bmp_out_sta_ba_en   <= 0;
		end
		else if (!tx_bmp_en)begin
			tx_bmp_out_sta_ba_en   <= 0;
		end
		else begin
			tx_bmp_out_sta_ba_en   <= 1;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_count  <= 0;
		end
		else if(!tx_bmp_en)begin
			loc_count  <= 0;
		end
		else if(loc_sta_info_val && loc_sta_info_avl) begin
			if(!loc_upd_done)	
				loc_count  <= loc_count+1;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_sta_offset <= 0;
		end
		else if(!tx_bmp_en)begin
			loc_sta_offset <= 0;
		end
		else if(tx_bmp_in_sta_info_val && tx_bmp_in_sta_info)begin
			loc_sta_offset <= tx_bmp_in_sta_offset;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_buf_winstart <= 0;
		end
		else if (!tx_bmp_en)begin
			sb_buf_winstart <= 0;
		end
		else if(loc_count == 2)begin
			sb_buf_winstart <= tx_bmp_in_sta_ba_data[25:16];//10 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_new_buf_winstart <= 0;
		end
		else if (!tx_bmp_en)begin
			sb_new_buf_winstart <= 0;
		end
		else if((loc_count == 7) && !loc_sb_update[87])begin
			if(loc_sb_update[86])
				sb_new_buf_winstart <= loc_sb_update[41:32];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_winstart  <= 0;
		end
		else if (!tx_bmp_en)begin
			sb_winstart  <= 0;
		end
		else if(loc_count == 4)begin
			sb_winstart  <= tx_bmp_in_sta_ba_data[31:16];//16 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_new_winstart  <= 0;
		end
		else if (!tx_bmp_en)begin
			sb_new_winstart  <= 0;
		end
		else if((loc_count == 7) && !loc_sb_update[87])begin
			if(loc_sb_update[86])
			sb_new_winstart  <= loc_sb_update[31:16];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_winend  <= 0;
		end
		else if (!tx_bmp_en)begin
			sb_winend  <= 0;
		end
		else if(loc_count == 6)begin
			sb_winend  <= tx_bmp_in_sta_ba_data[15:0];//16 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_new_winend  <= 0;
		end
		else if (!tx_bmp_en)begin
			sb_new_winend  <= 0;
		end
		else if ((loc_count == 7) && !loc_sb_update[87]) begin
			if(loc_sb_update[86])
			sb_new_winend	 <= loc_sb_update[15:0];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_winsize  <= 0;
		end
		else if (!tx_bmp_en)begin
			sb_winsize  <= 0;
		end
		else if (loc_count == 6)begin
			sb_winsize  <= tx_bmp_in_sta_ba_data[31:16];//16 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_winsize_rd  <= 0;
		end
		else if (!tx_bmp_en)begin
			loc_winsize_rd  <= 0;
		end
		else if(loc_upd_done)
			loc_winsize_rd <= 0;
		else if(loc_count == 6)begin
			loc_winsize_rd <= 1;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_winsize_rd_d <= 0;
		else if(!tx_bmp_en)
			loc_winsize_rd_d <= 0;
		else if(loc_upd_done)
			loc_winsize_rd_d <= 0;
		else 
			loc_winsize_rd_d <= loc_winsize_rd;
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			clear_start  <= 0;
		end
		else if(!tx_bmp_en)begin
			clear_start  <= 0;
		end
		else if(loc_upd_done)
			clear_start  <= 0;
		else if((loc_winsize_rd&&!loc_winsize_rd_d) && !loc_sb_update[87])
			clear_start <= loc_sb_update[69:64];//take useful 6 bits
		else if(loc_winsize_rd)
			clear_start <= clear_start + 'b1;
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			clear_end  <= 0;
		end
		else if(!tx_bmp_en)begin
			clear_end  <= 0;
		end
		else if(loc_winsize_rd && !loc_sb_update[87])begin
			clear_end <= loc_sb_update[53:48];//take useful 6 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_clear_buf  <= 0;
		end
		else if(!tx_bmp_en)begin
			loc_clear_buf  <= 0;
		end
		else if(loc_winsize_rd && !loc_sb_update[87])begin
			loc_clear_buf <= loc_sb_update[86];
		end
	end

	/*---------------------------------------------------------------------------------------------------------*/
endmodule
//EOF


