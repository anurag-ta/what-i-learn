`include "../../inc/defines.h" 

module uu_acmac_cp_ctrl_pkt_gen(
	   input                clk                     ,
     input                rst_n                   ,
     input                cp_ctrl_pkt_gen_en      ,
     //interface between cp_ctrl_pkt_gen and cp_if 
     input        [3 :0]  cp_ctrl_in_pkt_subtype  ,
     input        [3 :0]  cp_ctrl_in_pkt_gen_type ,
     input        [15:0]  cp_ctrl_in_duration     ,
     input        [47:0]  cp_ctrl_in_mpdu_ra      ,
     input        [47:0]  cp_ctrl_in_mpdu_ta      ,
     input        [1 :0]  cp_ctrl_in_mpdu_bar_type,
     input        [3 :0]  cp_ctrl_in_mpdu_bar_tid ,
     input        [15:0]  cp_ctrl_in_mpdu_bar_scf ,
	   input        [15:0]  cp_ctrl_in_mpdu_bar_fc  ,
	   input        [15:0]  cp_ctrl_in_mpdu_bar_ssn ,
	   input                cp_ctrl_in_implicit     ,
     output  reg  [7 :0]  cp_ctrl_out_tx_frame_len, 
     output  reg          cp_ctrl_out_pkt_data_val,
     output  reg  [7 :0]  cp_ctrl_out_pkt_data    ,
	//Interface to Memory sta_ba_info 
	   input        [31:0]  cp_ctrl_in_sta_ba_data  ,
	   output  reg  [14:0]  cp_ctrl_out_sta_ba_addr ,
     output  reg  [31:0]  cp_ctrl_out_sta_ba_data ,
     output  reg          cp_ctrl_out_sta_ba_en   ,
     output  reg  [3 :0]  cp_ctrl_out_sta_ba_wen  ,
	//BITMAP offset 
    	input        [14:0]  cp_ctrl_in_sta_offset   ,
	// CRC32 interface
     input                cp_ctrl_in_crc_avl      ,
     input        [31:0]  cp_ctrl_in_crc          ,
     output  reg  [15:0]  cp_ctrl_out_crc_len     ,
     output  reg          cp_ctrl_out_crc_en      ,
	   output  reg          cp_ctrl_out_crc_data_val,
     output  reg  [7 :0]  cp_ctrl_out_crc_data

     ); 

	/* Registers internal to the module
	....................................................*/
  reg  [7 :0]  loc_frame_buf_0       ;
	reg  [7 :0]  loc_frame_buf_1       ;
	reg  [7 :0]  loc_frame_buf_2       ;
	reg  [7 :0]  loc_frame_buf_3       ;
	reg  [7 :0]  loc_frame_buf_4       ;
	reg  [7 :0]  loc_frame_buf_5       ;
	reg  [7 :0]  loc_frame_buf_6       ;
	reg  [7 :0]  loc_frame_buf_7       ;
	reg  [7 :0]  loc_frame_buf_8       ;
	reg  [7 :0]  loc_frame_buf_9       ;
	reg  [7 :0]  loc_frame_buf_10      ;
	reg  [7 :0]  loc_frame_buf_11      ;
	reg  [7 :0]  loc_frame_buf_12      ;
	reg  [7 :0]  loc_frame_buf_13      ;
	reg  [7 :0]  loc_frame_buf_14      ;
	reg  [7 :0]  loc_frame_buf_15      ;
	reg  [7 :0]  loc_frame_buf_16      ;
	reg  [7 :0]  loc_frame_buf_17      ;
	reg  [7 :0]  loc_frame_buf_18      ;
	reg  [7 :0]  loc_frame_buf_19      ;
	reg  [7 :0]  loc_frame_buf_20      ;
	reg  [7 :0]  loc_frame_buf_21      ;
	reg  [7 :0]  loc_frame_buf_22      ;
	reg  [7 :0]  loc_frame_buf_23      ;
	reg  [6 :0]  loc_count             ;
  reg  [7 :0]  loc_index             ;
	reg          loc_crc_len_rd        ;
	reg          loc_ba_bmp_wr_val;
	reg          loc_winsize_rd        ;
	reg          loc_ba_compressed     ;
	reg          loc_comp_avl          ;
	reg          loc_seq_no_bitmap_avl ;
	reg          loc_sta_ba_data_val   ;  
	reg          loc_crc_avl           ;
	reg          loc_bitmap_added      ;
	reg  [7 :0]  loc_err               ;
 	wire [96:0]  loc_sb_fill_ba        ;
	reg  [7 :0]  loc_accum_data        ;
	reg  [15:0]  sb_winstart           ;
  reg  [15:0]  sb_winsize            ;
  reg  [9 :0]  sb_buf_winstart       ;
  reg  [15:0]  sb_winend             ;
	reg  [15:0]  sb_new_winstart       ;
  reg  [9 :0]  sb_new_buf_winstart   ;
  reg  [15:0]  sb_new_winend         ;
	reg  [15:0]  clear_end,clear_start ;
	reg  [15:0]  loc_count2            ;
	reg          loc_clear_done        ;
	reg  [2:0]   loc_temp_count        ;
	wire         loc_is_ba;
	wire         loc_is_bar;
	wire         loc_is_implcom;

	/*-----------------------------------------------*/

	/* Continuous assignments 
	.................................................................................................*/
	assign loc_is_ba = (cp_ctrl_in_pkt_gen_type == `BA_PKT);
	assign loc_is_bar = (cp_ctrl_in_pkt_gen_type == `BAR_PKT);
	assign loc_is_implcom = (cp_ctrl_in_implicit&&loc_ba_compressed);
	assign loc_sb_fill_ba = sb_fill_ba(sb_buf_winstart,sb_winstart,sb_winend,sb_winsize,(cp_ctrl_in_mpdu_bar_scf >> 4)); 
	/*-----------------------------------------------------------------------------------------------*/
	
	/* Function for filling sta_ba_info
	.................................................................................................*/	
	//sb_fill_ba function
	function [97:0] sb_fill_ba;
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
		reg   [15:0] seq_no_bitmap  ;
	begin
		if ((sb_winstart <= sn) && (sn <= sb_winend))begin
			clear_start         = sb_buf_winstart + (sb_winend-sb_winstart) + 1;
			clear_end           = sb_buf_winstart + (sn - sb_winstart + sb_winsize - 1);
			sb_new_buf_winstart = (sb_buf_winstart + sn - sb_winstart) & `BA_BASIC_BITMAP_BUF_MASK;
			sb_new_winstart     = sn;
			sb_new_winend       = sn + sb_winsize - 1;
			seq_no_bitmap       = sb_new_winstart & `MPDU_SEQN_MASK;
			if (sb_new_winstart > `BA_MPDU_SEQN_MAX)begin
				sb_new_winstart = sb_new_winstart & `MPDU_SEQN_MASK;
				sb_new_winend   = sb_new_winend & `MPDU_SEQN_MASK;
			end
			sb_fill_ba = {1'b0,6'h0,sb_new_buf_winstart,sb_new_winstart,sb_new_winend,clear_start,clear_end,seq_no_bitmap};
		end
		else if ((sb_winend < sn) && (sn < (sb_winstart + `BA_SEQN_RANGE_BY2)))begin
			sb_new_buf_winstart = (sb_buf_winstart + sn ) & `BA_BASIC_BITMAP_BUF_MASK;
			sb_new_winstart     = sn;
			sb_new_winend       = sn + sb_winsize - 1;
			clear_start         = sb_new_buf_winstart; 
			clear_end           = sb_new_buf_winstart + sb_winsize;
			seq_no_bitmap       = sb_new_winstart & `MPDU_SEQN_MASK;
			if (sb_new_winstart > `BA_MPDU_SEQN_MAX)begin
				sb_new_winstart = sb_new_winstart & `MPDU_SEQN_MASK;
				sb_new_winend   = sb_new_winend & `MPDU_SEQN_MASK;
			end

			sb_fill_ba          = {1'b0,6'h0,sb_new_buf_winstart,sb_new_winstart,sb_new_winend,clear_start,clear_end,seq_no_bitmap};
		end
		else begin//wrap_around_start
			seq_no_bitmap = (sn & `MPDU_SEQN_MASK); 
			sn_new        = sn | 16'h1000;
			if ((sb_winstart <= sn_new) && (sn_new <= sb_winend))begin
				clear_start         = sb_buf_winstart + (sb_winend-sb_winstart) + 1;
				clear_end           = sb_buf_winstart + (sn_new - ((sb_winstart + sb_winsize) - 1));
				sb_new_buf_winstart = (sb_buf_winstart + (sn_new - sb_winstart)) & `BA_BASIC_BITMAP_BUF_MASK;
				sb_new_winstart     = sn_new;
				sb_new_winend       = sn_new + sb_winsize - 1;
				seq_no_bitmap       = sb_new_winstart & `MPDU_SEQN_MASK;
				if (sb_new_winstart > `BA_MPDU_SEQN_MAX)begin
					sb_new_winstart = sb_new_winstart & `MPDU_SEQN_MASK;
					sb_new_winend   = sb_new_winend & `MPDU_SEQN_MASK;
				end
				sb_fill_ba = {1'b0,6'h0,sb_new_buf_winstart,sb_new_winstart,sb_new_winend,clear_start,clear_end,seq_no_bitmap};
			end
			else if ((sb_winend < sn_new) && (sn_new < (sb_winstart + `BA_SEQN_RANGE_BY2)))begin
				sb_new_buf_winstart = (sb_buf_winstart + sn_new ) & `BA_BASIC_BITMAP_BUF_MASK;
				sb_new_winstart     = sn_new;
				sb_new_winend       = sn_new + sb_winsize - 1;
				clear_start         = sb_buf_winstart; 
				clear_end           = sb_buf_winstart + sb_winsize;
				seq_no_bitmap       = sb_new_winstart & `MPDU_SEQN_MASK;
				if (sb_new_winstart > `BA_MPDU_SEQN_MAX)begin
					sb_new_winstart = sb_new_winstart & `MPDU_SEQN_MASK;
					sb_new_winend   = sb_new_winend & `MPDU_SEQN_MASK;
				end
				sb_fill_ba = {1'b0,6'h0,sb_new_buf_winstart,sb_new_winstart,sb_new_winend,clear_start,clear_end,seq_no_bitmap};
			end
			else begin
				seq_no_bitmap = (sn_new & `MPDU_SEQN_MASK);    
				sb_fill_ba    = {81'h1_0000_0000_0000_0000_0000,seq_no_bitmap};
			end
		end
	end
	endfunction
	/*-----------------------------------------------------------------------------------------------*/


	/* Concurrent assignments 
	...........................................................*/
	//always for generating response control frame
	always @(posedge clk or `EDGE_OF_RESET) begin
		if (`POLARITY_OF_RESET) begin
			loc_frame_buf_0          <= 0;
			loc_frame_buf_1          <= 0;
			loc_frame_buf_2          <= 0;
			loc_frame_buf_3          <= 0;
			loc_frame_buf_4          <= 0;
			loc_frame_buf_5          <= 0;
			loc_frame_buf_6          <= 0;
			loc_frame_buf_7          <= 0;
     		loc_frame_buf_8          <= 0;
			loc_frame_buf_9          <= 0;
			loc_frame_buf_10         <= 0;
			loc_frame_buf_11         <= 0;
			loc_frame_buf_12         <= 0;
			loc_frame_buf_13         <= 0;
			loc_frame_buf_14         <= 0;
			loc_frame_buf_15         <= 0;
     		loc_frame_buf_16         <= 0;
			loc_frame_buf_17         <= 0;
			loc_frame_buf_20         <= 0;
			loc_frame_buf_21         <= 0;
			loc_frame_buf_22         <= 0;
			loc_frame_buf_23         <= 0;
			loc_count                <= 0;
     		loc_index                <= 0;
			loc_crc_len_rd           <= 0;
			loc_ba_bmp_wr_val        <= 0;
			loc_ba_compressed        <= 0;
			loc_comp_avl             <= 0;
    			loc_sta_ba_data_val      <= 0;  
			loc_crc_avl              <= 0;
			cp_ctrl_out_tx_frame_len <= 0;
			loc_err                  <= 0;
			loc_accum_data           <= 0;
			cp_ctrl_out_pkt_data_val <= 0;
			cp_ctrl_out_pkt_data     <= 0;
			cp_ctrl_out_crc_len      <= 0;
			cp_ctrl_out_crc_en       <= 0;
			cp_ctrl_out_crc_data_val <= 0;
			cp_ctrl_out_crc_data     <= 0;
			loc_bitmap_added         <= 0;
			loc_temp_count           <= 0;

		end
		else if(!cp_ctrl_pkt_gen_en)begin
			loc_frame_buf_0          <= 0;
			loc_frame_buf_1          <= 0;
			loc_frame_buf_2          <= 0;
			loc_frame_buf_3          <= 0;
			loc_frame_buf_4          <= 0;
			loc_frame_buf_5          <= 0;
			loc_frame_buf_6          <= 0;
			loc_frame_buf_7          <= 0;
     		loc_frame_buf_8          <= 0;
			loc_frame_buf_9          <= 0;
			loc_frame_buf_10         <= 0;
			loc_frame_buf_11         <= 0;
			loc_frame_buf_12         <= 0;
			loc_frame_buf_13         <= 0;
			loc_frame_buf_14         <= 0;
			loc_frame_buf_15         <= 0;
     		loc_frame_buf_16         <= 0;
			loc_frame_buf_17         <= 0;
			loc_frame_buf_20         <= 0;
			loc_frame_buf_21         <= 0;
			loc_frame_buf_22         <= 0;
			loc_frame_buf_23         <= 0;
			loc_count                <= 0;
     		loc_index                <= 0;
			loc_crc_len_rd           <= 0;
			loc_ba_bmp_wr_val        <= 0;
			loc_ba_compressed        <= 0;
			loc_comp_avl             <= 0;
			loc_sta_ba_data_val      <= 0;  
			loc_crc_avl              <= 0;
		      //cp_ctrl_out_tx_frame_len <= 0;
			loc_err                  <= 0;
			loc_accum_data           <= 0;
			cp_ctrl_out_pkt_data_val <= 0;
			cp_ctrl_out_pkt_data     <= 0;
			cp_ctrl_out_crc_len      <= 0;
			cp_ctrl_out_crc_en       <= 0;
			cp_ctrl_out_crc_data_val <= 0;
			cp_ctrl_out_crc_data     <= 0;
			loc_bitmap_added         <= 0;
			loc_temp_count           <= 0;
		end
		else if (cp_ctrl_in_pkt_gen_type == `RTS_PKT) begin//RTS_FRAME
                         cp_ctrl_out_tx_frame_len <= 8'd20;
		
			if(!loc_crc_len_rd)begin
				loc_frame_buf_0 <= (`IEEE80211_FC0_VERSION_0 | `IEEE80211_FC0_TYPE_CTL |`IEEE80211_FC0_SUBTYPE_RTS);
				loc_frame_buf_1 <= 8'h00;
				loc_frame_buf_2 <= cp_ctrl_in_duration[7 :0 ];
				loc_frame_buf_3 <= cp_ctrl_in_duration[15:8 ];
				loc_frame_buf_4 <= cp_ctrl_in_mpdu_ra [7 :0 ];
				loc_frame_buf_5 <= cp_ctrl_in_mpdu_ra [15:8 ];
				loc_frame_buf_6 <= cp_ctrl_in_mpdu_ra [23:16];
				loc_frame_buf_7 <= cp_ctrl_in_mpdu_ra [31:24];
				loc_frame_buf_8 <= cp_ctrl_in_mpdu_ra [39:32];
				loc_frame_buf_9 <= cp_ctrl_in_mpdu_ra [47:40];
				 //UU_WLAN_IEEE80211_STA_MAC_ADDR_R for TA
				loc_frame_buf_10 <= `STA_MAC_ADDR_0;//[7 :0 ];
				loc_frame_buf_11 <= `STA_MAC_ADDR_1;//[15:8 ];
				loc_frame_buf_12 <= `STA_MAC_ADDR_2;//[23:16];
				loc_frame_buf_13 <= `STA_MAC_ADDR_3;//[31:24];
				loc_frame_buf_14 <= `STA_MAC_ADDR_4;//[39:32];
				loc_frame_buf_15 <= `STA_MAC_ADDR_5;//[47:40];
				//calculate CRC
				cp_ctrl_out_crc_en  <= 1'b1 ;
				cp_ctrl_out_crc_len <= 8'd16; //UU_RTS_FRAME_LEN-4
				loc_crc_len_rd      <= 1'b1 ;
			end
			else if (loc_index < 8'd16) begin
				cp_ctrl_out_crc_data_val <= 1'b1 ;
				cp_ctrl_out_pkt_data_val <= 1'b1 ;
				//cp_ctrl_out_tx_frame_len <= 8'd20; //UU_RTS_FRAME_LEN
				case (loc_index)
				8'h00: begin  cp_ctrl_out_crc_data <= loc_frame_buf_0 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_0 ;end
				8'h01: begin  cp_ctrl_out_crc_data <= loc_frame_buf_1 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_1 ;end
				8'h02: begin  cp_ctrl_out_crc_data <= loc_frame_buf_2 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_2 ;end
				8'h03: begin  cp_ctrl_out_crc_data <= loc_frame_buf_3 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_3 ;end
				8'h04: begin  cp_ctrl_out_crc_data <= loc_frame_buf_4 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_4 ;end
				8'h05: begin  cp_ctrl_out_crc_data <= loc_frame_buf_5 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_5 ;end
				8'h06: begin  cp_ctrl_out_crc_data <= loc_frame_buf_6 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_6 ;end
				8'h07: begin  cp_ctrl_out_crc_data <= loc_frame_buf_7 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_7 ;end 
				8'h08: begin  cp_ctrl_out_crc_data <= loc_frame_buf_8 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_8 ;end 
				8'h09: begin  cp_ctrl_out_crc_data <= loc_frame_buf_9 ;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_9 ;end
				8'h0A: begin  cp_ctrl_out_crc_data <= loc_frame_buf_10;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_10;end
				8'h0B: begin  cp_ctrl_out_crc_data <= loc_frame_buf_11;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_11;end
				8'h0C: begin  cp_ctrl_out_crc_data <= loc_frame_buf_12;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_12;end
				8'h0D: begin  cp_ctrl_out_crc_data <= loc_frame_buf_13;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_13;end
				8'h0E: begin  cp_ctrl_out_crc_data <= loc_frame_buf_14;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_14;end
				8'h0F: begin  cp_ctrl_out_crc_data <= loc_frame_buf_15;
						    cp_ctrl_out_pkt_data <= loc_frame_buf_15;end
				endcase
				loc_index <= loc_index+1;
			end  	
			else	begin
				if (cp_ctrl_in_crc_avl) begin
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
					loc_crc_avl        <= 1'b1;
					cp_ctrl_out_crc_en <= 1'b0;
		/*			loc_frame_buf_16   <= cp_ctrl_in_crc[7 :0 ];
					loc_frame_buf_17   <= cp_ctrl_in_crc[15:8 ];
					loc_frame_buf_20   <= cp_ctrl_in_crc[23:16];
					loc_frame_buf_21   <= cp_ctrl_in_crc[31:24]; */
				end
				else if(loc_crc_avl)begin
					if(loc_index<8'd20)begin
						//cp_ctrl_out_pkt_data_val <= 1'b1;	
                                                cp_ctrl_out_pkt_data_val <= 1'b0;
						case(loc_index)
						8'd16: cp_ctrl_out_pkt_data <= loc_frame_buf_16;
						8'd17: cp_ctrl_out_pkt_data <= loc_frame_buf_17;
						8'd18: cp_ctrl_out_pkt_data <= loc_frame_buf_20;
						8'd19: cp_ctrl_out_pkt_data <= loc_frame_buf_21;
						endcase
						loc_index <= loc_index+1;
					end
					else 
						cp_ctrl_out_pkt_data_val <= 1'b0;	
				end
				else begin
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
				end
			end
		end
		else if (cp_ctrl_in_pkt_gen_type == `CTS_PKT) begin //CTS_FRAME
			if(!loc_crc_len_rd) begin
				loc_frame_buf_0 <= `IEEE80211_FC0_VERSION_0 | `IEEE80211_FC0_TYPE_CTL | `IEEE80211_FC0_SUBTYPE_CTS; 
				loc_frame_buf_1 <= 8'h00;
				loc_frame_buf_2 <= cp_ctrl_in_duration[7 :0 ];
				loc_frame_buf_3 <= cp_ctrl_in_duration[15:8 ];
				loc_frame_buf_4 <= cp_ctrl_in_mpdu_ta [7 :0 ];
				loc_frame_buf_5 <= cp_ctrl_in_mpdu_ta [15:8 ];
				loc_frame_buf_6 <= cp_ctrl_in_mpdu_ta [23:16];
				loc_frame_buf_7 <= cp_ctrl_in_mpdu_ta [31:24];
				loc_frame_buf_8 <= cp_ctrl_in_mpdu_ta [39:32];
				loc_frame_buf_9 <= cp_ctrl_in_mpdu_ta [47:40];
				//calculate CRC
				cp_ctrl_out_crc_en  <= 1'b1 ;
				cp_ctrl_out_crc_len <= 8'd10; //UU_CTS_FRAME_LEN-4
				loc_crc_len_rd      <= 1'b1 ;
			end
			else if (loc_index < 8'd10) begin
				cp_ctrl_out_crc_data_val <= 1'b1 ;
				cp_ctrl_out_pkt_data_val <= 1'b1 ;
				cp_ctrl_out_tx_frame_len <= 8'd14; //UU_CTS_FRAME_LEN				   
				case (loc_index)
				8'h00: begin  cp_ctrl_out_crc_data <= loc_frame_buf_0;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_0;end
				8'h01: begin  cp_ctrl_out_crc_data <= loc_frame_buf_1;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_1;end
				8'h02: begin  cp_ctrl_out_crc_data <= loc_frame_buf_2;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_2;end
				8'h03: begin  cp_ctrl_out_crc_data <= loc_frame_buf_3;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_3;end
				8'h04: begin  cp_ctrl_out_crc_data <= loc_frame_buf_4;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_4;end
				8'h05: begin  cp_ctrl_out_crc_data <= loc_frame_buf_5;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_5;end
				8'h06: begin  cp_ctrl_out_crc_data <= loc_frame_buf_6;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_6;end
				8'h07: begin  cp_ctrl_out_crc_data <= loc_frame_buf_7;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_7;end 
				8'h08: begin  cp_ctrl_out_crc_data <= loc_frame_buf_8;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_8;end 
				8'h09: begin  cp_ctrl_out_crc_data <= loc_frame_buf_9;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_9;end 
				endcase
				loc_index <= loc_index+1;
			end
			else begin
				if (cp_ctrl_in_crc_avl) begin
					loc_crc_avl        <= 1'b1;
					cp_ctrl_out_crc_en <= 1'b0;				
					loc_frame_buf_10 <= cp_ctrl_in_crc[7 :0 ];
					loc_frame_buf_11 <= cp_ctrl_in_crc[15:8 ];
					loc_frame_buf_12 <= cp_ctrl_in_crc[23:16];
					loc_frame_buf_13 <= cp_ctrl_in_crc[31:24];
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
				end
				else if(loc_crc_avl)begin
					if(loc_index<8'd14)begin
						cp_ctrl_out_pkt_data_val <= 1'b1;	
						case(loc_index)
						8'd10: cp_ctrl_out_pkt_data <= loc_frame_buf_10;
						8'd11: cp_ctrl_out_pkt_data <= loc_frame_buf_11;
						8'd12: cp_ctrl_out_pkt_data <= loc_frame_buf_12;
						8'd13: cp_ctrl_out_pkt_data <= loc_frame_buf_13;
						endcase
						loc_index <= loc_index+1;			   
					end
					else 
					cp_ctrl_out_pkt_data_val <= 1'b0;	
				end
				else begin
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
				end
			end
		end
		else if (cp_ctrl_in_pkt_gen_type == `ACK_PKT) begin //ACK_FRAME
		      cp_ctrl_out_tx_frame_len <= 8'd14;
			if(!loc_crc_len_rd)begin
				loc_frame_buf_0 <= `IEEE80211_FC0_VERSION_0 | `IEEE80211_FC0_TYPE_CTL | `IEEE80211_FC0_SUBTYPE_ACK;
				loc_frame_buf_1 <= 8'h00;
				loc_frame_buf_2 <= cp_ctrl_in_duration[7 :0 ];
				loc_frame_buf_3 <= cp_ctrl_in_duration[15:8 ];
				loc_frame_buf_4 <= cp_ctrl_in_mpdu_ta [7 :0 ];
				loc_frame_buf_5 <= cp_ctrl_in_mpdu_ta [15:8 ];
				loc_frame_buf_6 <= cp_ctrl_in_mpdu_ta [23:16];
				loc_frame_buf_7 <= cp_ctrl_in_mpdu_ta [31:24];
				loc_frame_buf_8 <= cp_ctrl_in_mpdu_ta [39:32];
				loc_frame_buf_9 <= cp_ctrl_in_mpdu_ta [47:40];
				//calculate CRC
				cp_ctrl_out_crc_en  <= 1'b1 ;
				cp_ctrl_out_crc_len <= 8'd10; //UU_ACK_FRAME_LEN-4
				loc_crc_len_rd      <= 1'b1 ;
				end
			else	if(loc_index < 8'd10) begin
				cp_ctrl_out_crc_data_val <= 1'b1 ;
				cp_ctrl_out_pkt_data_val <= 1'b1 ;	
			//cp_ctrl_out_tx_frame_len <= 8'd14; //UU_ACK_FRAME_LEN
				case (loc_index)
				8'h00: begin  cp_ctrl_out_crc_data <= loc_frame_buf_0;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_0;end
				8'h01: begin  cp_ctrl_out_crc_data <= loc_frame_buf_1;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_1;end
				8'h02: begin  cp_ctrl_out_crc_data <= loc_frame_buf_2;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_2;end
				8'h03: begin  cp_ctrl_out_crc_data <= loc_frame_buf_3;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_3;end
				8'h04: begin  cp_ctrl_out_crc_data <= loc_frame_buf_4;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_4;end
				8'h05: begin  cp_ctrl_out_crc_data <= loc_frame_buf_5;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_5;end
				8'h06: begin  cp_ctrl_out_crc_data <= loc_frame_buf_6;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_6;end
				8'h07: begin  cp_ctrl_out_crc_data <= loc_frame_buf_7;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_7;end 
				8'h08: begin  cp_ctrl_out_crc_data <= loc_frame_buf_8;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_8;end 
				8'h09: begin  cp_ctrl_out_crc_data <= loc_frame_buf_9;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_9;end 
				endcase
				loc_index <= loc_index+1;	
			end			      
			else	begin
				if(cp_ctrl_in_crc_avl) begin
					loc_crc_avl        <= 1'b1;
					cp_ctrl_out_crc_en <= 1'b0;
					loc_frame_buf_10 <= cp_ctrl_in_crc[7:0];
					loc_frame_buf_11 <= cp_ctrl_in_crc[15:8];
					loc_frame_buf_12 <= cp_ctrl_in_crc[23:16];
					loc_frame_buf_13 <= cp_ctrl_in_crc[31:24];
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;				
				end
				else if(loc_crc_avl) begin
					if(loc_index<8'd14)begin
						cp_ctrl_out_pkt_data_val <= 1'b1;	
						case(loc_index)
						8'd10: cp_ctrl_out_pkt_data <= loc_frame_buf_10;
						8'd11: cp_ctrl_out_pkt_data <= loc_frame_buf_11;
						8'd12: cp_ctrl_out_pkt_data <= loc_frame_buf_12;
						8'd13: cp_ctrl_out_pkt_data <= loc_frame_buf_13;
						endcase
						loc_index <= loc_index+1;
					end
					else 
						cp_ctrl_out_pkt_data_val <= 1'b0;				      
				end
				else begin
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
				end
			end
		end
		else if (cp_ctrl_in_pkt_gen_type ==  `BAR_PKT) begin //BAR_FRAME
			if(!loc_crc_len_rd) begin
				loc_frame_buf_0 <= `IEEE80211_FC0_VERSION_0 | `IEEE80211_FC0_TYPE_CTL | `IEEE80211_FC0_SUBTYPE_BAR;
				loc_frame_buf_1 <= 8'h00;
				loc_frame_buf_2 <= cp_ctrl_in_duration[7 :0 ];
				loc_frame_buf_3 <= cp_ctrl_in_duration[15:8 ];
				loc_frame_buf_4 <= cp_ctrl_in_mpdu_ra [7 :0 ];
				loc_frame_buf_5 <= cp_ctrl_in_mpdu_ra [15:8 ];
				loc_frame_buf_6 <= cp_ctrl_in_mpdu_ra [23:16];
				loc_frame_buf_7 <= cp_ctrl_in_mpdu_ra [31:24];
				loc_frame_buf_8 <= cp_ctrl_in_mpdu_ra [39:32];
				loc_frame_buf_9 <= cp_ctrl_in_mpdu_ra [47:40];				
				//UU_WLAN_IEEE80211_STA_MAC_ADDR_R for TA
				loc_frame_buf_10 <= `STA_MAC_ADDR_0;//[7 :0 ];
				loc_frame_buf_11 <= `STA_MAC_ADDR_1;//[15:8 ];
				loc_frame_buf_12 <= `STA_MAC_ADDR_2;//[23:16];
				loc_frame_buf_13 <= `STA_MAC_ADDR_3;//[31:24];
				loc_frame_buf_14 <= `STA_MAC_ADDR_4;//[39:32];
				loc_frame_buf_15 <= `STA_MAC_ADDR_5;//[47:40];
				loc_frame_buf_16 <= {6'h00,cp_ctrl_in_mpdu_bar_type};
				loc_frame_buf_17 <= {cp_ctrl_in_mpdu_bar_tid,4'h0};

				//calculate CRC
				cp_ctrl_out_crc_en  <= 1'b1;
				cp_ctrl_out_crc_len <= 8'd20; //UU_BAR_FRAME_LEN-4
				loc_crc_len_rd      <= 1'b1;
			end
			else if(loc_index < 8'd20) begin
				cp_ctrl_out_crc_data_val <= 1'b1 ;
				cp_ctrl_out_pkt_data_val <= 1'b1 ;
				cp_ctrl_out_tx_frame_len <= 8'd24; //UU_BAR_FRAME_LEN
				case (loc_index)
				8'h00: begin  cp_ctrl_out_crc_data <= loc_frame_buf_0 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_0 ;end
				8'h01: begin  cp_ctrl_out_crc_data <= loc_frame_buf_1 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_1 ;end
				8'h02: begin  cp_ctrl_out_crc_data <= loc_frame_buf_2 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_2 ;end
				8'h03: begin  cp_ctrl_out_crc_data <= loc_frame_buf_3 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_3 ;end
				8'h04: begin  cp_ctrl_out_crc_data <= loc_frame_buf_4 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_4 ;end
				8'h05: begin  cp_ctrl_out_crc_data <= loc_frame_buf_5 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_5 ;end
				8'h06: begin  cp_ctrl_out_crc_data <= loc_frame_buf_6 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_6 ;end
				8'h07: begin  cp_ctrl_out_crc_data <= loc_frame_buf_7 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_7 ;end 
				8'h08: begin  cp_ctrl_out_crc_data <= loc_frame_buf_8 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_8 ;end 
				8'h09: begin  cp_ctrl_out_crc_data <= loc_frame_buf_9 ;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_9 ;end 
				8'h0A: begin  cp_ctrl_out_crc_data <= loc_frame_buf_10;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_10;end
				8'h0B: begin  cp_ctrl_out_crc_data <= loc_frame_buf_11;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_11;end
				8'h0C: begin  cp_ctrl_out_crc_data <= loc_frame_buf_12;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_12;end
				8'h0D: begin  cp_ctrl_out_crc_data <= loc_frame_buf_13;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_13;end
				8'h0E: begin  cp_ctrl_out_crc_data <= loc_frame_buf_14;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_14;end
				8'h0F: begin  cp_ctrl_out_crc_data <= loc_frame_buf_15;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_15;end
				8'h10: begin  cp_ctrl_out_crc_data <= loc_frame_buf_16;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_16;end
				8'h11: begin  cp_ctrl_out_crc_data <= loc_frame_buf_17;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_17;end 
				8'h12: begin  cp_ctrl_out_crc_data <= loc_frame_buf_18;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_18;end 
				8'h13: begin  cp_ctrl_out_crc_data <= loc_frame_buf_19;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_19;end
				endcase
				loc_index <= loc_index+1;	
			end
			else	begin
				if (cp_ctrl_in_crc_avl) begin
					loc_crc_avl        <= 1'b1;
					cp_ctrl_out_crc_en <= 1'b0;
					loc_frame_buf_20 <= cp_ctrl_in_crc[7:0];
					loc_frame_buf_21 <= cp_ctrl_in_crc[15:8];
					loc_frame_buf_22 <= cp_ctrl_in_crc[23:16];
					loc_frame_buf_23 <= cp_ctrl_in_crc[31:24];
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
				end
				else if(loc_crc_avl)begin
					if(loc_index<8'd24)begin
						cp_ctrl_out_pkt_data_val <= 1'b1;	
						case(loc_index)
						8'd20: cp_ctrl_out_pkt_data<=loc_frame_buf_20;
						8'd21: cp_ctrl_out_pkt_data<=loc_frame_buf_21;
						8'd22: cp_ctrl_out_pkt_data<=loc_frame_buf_22;
						8'd23: cp_ctrl_out_pkt_data<=loc_frame_buf_23;
						endcase
						loc_index <= loc_index+1;
					end
					else 
						cp_ctrl_out_pkt_data_val <= 1'b0;					      
				end
				else begin
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
				end
			end
		end
		else if (cp_ctrl_in_pkt_gen_type == `SELF_CTS_PKT) begin //CTS_FRAME
			if(!loc_crc_len_rd)begin
				loc_frame_buf_0 <= `IEEE80211_FC0_VERSION_0 | `IEEE80211_FC0_TYPE_CTL | `IEEE80211_FC0_SUBTYPE_CTS; 
				loc_frame_buf_1 <= 8'h00;
				loc_frame_buf_2 <= cp_ctrl_in_duration[7:0];
				loc_frame_buf_3 <= cp_ctrl_in_duration[15:8];
				loc_frame_buf_4 <= cp_ctrl_in_mpdu_ra[7:0];
				loc_frame_buf_5 <= cp_ctrl_in_mpdu_ra[15:8];
				loc_frame_buf_6 <= cp_ctrl_in_mpdu_ra[23:16];
				loc_frame_buf_7 <= cp_ctrl_in_mpdu_ra[31:24];
				loc_frame_buf_8 <= cp_ctrl_in_mpdu_ra[39:32];
				loc_frame_buf_9 <= cp_ctrl_in_mpdu_ra[47:40];
				//calculate CRC
				cp_ctrl_out_crc_en  <= 1'b1 ;
				cp_ctrl_out_crc_len <= 8'd10; //UU_CTS_FRAME_LEN-4
				loc_crc_len_rd      <= 1'b1 ;
			end
			else if(loc_index < 8'd10) begin
				cp_ctrl_out_crc_data_val <= 1'b1;
				cp_ctrl_out_pkt_data_val <= 1'b1;
				cp_ctrl_out_tx_frame_len <= 8'd14; //UU_CTS_FRAME_LEN				   
				case (loc_index)
				8'h00: begin  cp_ctrl_out_crc_data <= loc_frame_buf_0;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_0;end
				8'h01: begin  cp_ctrl_out_crc_data <= loc_frame_buf_1;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_1;end
				8'h02: begin  cp_ctrl_out_crc_data <= loc_frame_buf_2;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_2;end
				8'h03: begin  cp_ctrl_out_crc_data <= loc_frame_buf_3;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_3;end
				8'h04: begin  cp_ctrl_out_crc_data <= loc_frame_buf_4;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_4;end
				8'h05: begin  cp_ctrl_out_crc_data <= loc_frame_buf_5;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_5;end
				8'h06: begin  cp_ctrl_out_crc_data <= loc_frame_buf_6;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_6;end
				8'h07: begin  cp_ctrl_out_crc_data <= loc_frame_buf_7;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_7;end 
				8'h08: begin  cp_ctrl_out_crc_data <= loc_frame_buf_8;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_8;end 
				8'h09: begin  cp_ctrl_out_crc_data <= loc_frame_buf_9;
				              cp_ctrl_out_pkt_data <= loc_frame_buf_9;end 
				endcase
				loc_index <= loc_index+1;
			end
			else	begin
				if(cp_ctrl_in_crc_avl) begin
					loc_crc_avl        <= 1'b1;
					cp_ctrl_out_crc_en <= 1'b0;
					loc_frame_buf_10 <= cp_ctrl_in_crc[7 :0 ];					
					loc_frame_buf_11 <= cp_ctrl_in_crc[15:8 ];
					loc_frame_buf_12 <= cp_ctrl_in_crc[23:16];
					loc_frame_buf_13 <= cp_ctrl_in_crc[31:24];
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
				end
				else	if(loc_crc_avl)begin
					if(loc_index<8'd14)begin
						cp_ctrl_out_pkt_data_val <= 1'b1;	
						case(loc_index)
						8'd10: cp_ctrl_out_pkt_data <= loc_frame_buf_10;
						8'd11: cp_ctrl_out_pkt_data <= loc_frame_buf_11;
						8'd12: cp_ctrl_out_pkt_data <= loc_frame_buf_12;
						8'd13: cp_ctrl_out_pkt_data <= loc_frame_buf_13;
						endcase
						loc_index <= loc_index+1;			   
					end
					else 
					cp_ctrl_out_pkt_data_val <= 1'b0;	
				end
				else begin
					cp_ctrl_out_crc_data_val <= 1'b0 ;
					cp_ctrl_out_pkt_data_val <= 1'b0 ;
				end				
			end
		end
		else if (cp_ctrl_in_pkt_gen_type == `BA_PKT) begin //BA_FRAME
			if(!loc_comp_avl)begin
				loc_frame_buf_0  <= `IEEE80211_FC0_VERSION_0 | `IEEE80211_FC0_TYPE_CTL | `IEEE80211_FC0_SUBTYPE_BA;
				loc_frame_buf_1  <= 8'h00;
				loc_frame_buf_2  <= cp_ctrl_in_duration[7 :0 ];
				loc_frame_buf_3  <= cp_ctrl_in_duration[15:8 ];
				loc_frame_buf_4  <= cp_ctrl_in_mpdu_ta [7 :0 ];
				loc_frame_buf_5  <= cp_ctrl_in_mpdu_ta [15:8 ];
				loc_frame_buf_6  <= cp_ctrl_in_mpdu_ta [23:16];
				loc_frame_buf_7  <= cp_ctrl_in_mpdu_ta [31:24];
				loc_frame_buf_8  <= cp_ctrl_in_mpdu_ta [39:32];
				loc_frame_buf_9  <= cp_ctrl_in_mpdu_ta [47:40];
				loc_frame_buf_10 <= `STA_MAC_ADDR_0; //[7 :0 ];
				loc_frame_buf_11 <= `STA_MAC_ADDR_1; //[15:8 ];
				loc_frame_buf_12 <= `STA_MAC_ADDR_2; //[23:16];
				loc_frame_buf_13 <= `STA_MAC_ADDR_3; //[31:24];
				loc_frame_buf_14 <= `STA_MAC_ADDR_4; //[39:32];
				loc_frame_buf_15 <= `STA_MAC_ADDR_5; //[47:40];
				//received is compressed or not
				if (cp_ctrl_in_pkt_subtype == `BAR_PKT) begin
					if(cp_ctrl_in_mpdu_bar_type == `UU_WLAN_BAR_TYPE_COMP_BA)begin
						cp_ctrl_out_tx_frame_len <= `UU_BA_COMPRESSED_FRAME_LEN;
						loc_ba_compressed <= 1'b1;
					end
					else begin
						cp_ctrl_out_tx_frame_len <= `UU_BA_BASIC_FRAME_LEN;
						loc_ba_compressed <= 1'b0;
					end
					loc_frame_buf_16 <= cp_ctrl_in_mpdu_bar_fc[7 :0];
					loc_frame_buf_17 <= cp_ctrl_in_mpdu_bar_fc[15:8];
				end
				else begin
					loc_frame_buf_16  <= 8'h05; //OR found
					loc_frame_buf_17  <= {cp_ctrl_in_mpdu_bar_tid,4'h0}; //OR found
					cp_ctrl_out_tx_frame_len <= `UU_BA_COMPRESSED_FRAME_LEN;
					loc_ba_compressed <= 1'b1;
				end
				loc_comp_avl <= 1'b1;
			end 
			else begin
				//calculate CRC and send frame
				if(!loc_crc_len_rd)begin
					cp_ctrl_out_crc_en  <= 1'b1;
					cp_ctrl_out_crc_len <= cp_ctrl_out_tx_frame_len-4; //UU_BA_FRAME_LEN-4 => (either 152 or 32)-4
					loc_crc_len_rd      <= 1'b1;
				end 
				else if (loc_index < 8'd18) begin
					cp_ctrl_out_crc_data_val <= 1'b1;
					cp_ctrl_out_pkt_data_val <= 1'b1;
					case (loc_index)
					8'h00: begin  cp_ctrl_out_crc_data <= loc_frame_buf_0 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_0 ;end
					8'h01: begin  cp_ctrl_out_crc_data <= loc_frame_buf_1 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_1 ;end
					8'h02: begin  cp_ctrl_out_crc_data <= loc_frame_buf_2 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_2 ;end
					8'h03: begin  cp_ctrl_out_crc_data <= loc_frame_buf_3 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_3 ;end
					8'h04: begin  cp_ctrl_out_crc_data <= loc_frame_buf_4 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_4 ;end
					8'h05: begin  cp_ctrl_out_crc_data <= loc_frame_buf_5 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_5 ;end
					8'h06: begin  cp_ctrl_out_crc_data <= loc_frame_buf_6 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_6 ;end
					8'h07: begin  cp_ctrl_out_crc_data <= loc_frame_buf_7 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_7 ;end 
					8'h08: begin  cp_ctrl_out_crc_data <= loc_frame_buf_8 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_8 ;end 
					8'h09: begin  cp_ctrl_out_crc_data <= loc_frame_buf_9 ;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_9 ;end 
					8'h0A: begin  cp_ctrl_out_crc_data <= loc_frame_buf_10;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_10;end
					8'h0B: begin  cp_ctrl_out_crc_data <= loc_frame_buf_11;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_11;end
					8'h0C: begin  cp_ctrl_out_crc_data <= loc_frame_buf_12;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_12;end
					8'h0D: begin  cp_ctrl_out_crc_data <= loc_frame_buf_13;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_13;end
					8'h0E: begin  cp_ctrl_out_crc_data <= loc_frame_buf_14;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_14;end
					8'h0F: begin  cp_ctrl_out_crc_data <= loc_frame_buf_15;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_15;end
					8'h10: begin  cp_ctrl_out_crc_data <= loc_frame_buf_16;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_16;end
					8'h11: begin  cp_ctrl_out_crc_data <= loc_frame_buf_17;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_17;end 		
					endcase
					loc_index <= loc_index+1;	  
				end
				else if((loc_seq_no_bitmap_avl)&&(loc_index<8'd20)) begin
					cp_ctrl_out_crc_data_val <= 1'b1;
					cp_ctrl_out_pkt_data_val <= 1'b1;
					case (loc_index)
					8'h12: begin  cp_ctrl_out_crc_data <= loc_frame_buf_18;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_18;end
					8'h13: begin  cp_ctrl_out_crc_data <= loc_frame_buf_19;
							    cp_ctrl_out_pkt_data <= loc_frame_buf_19;
							    loc_ba_bmp_wr_val <= 1'b1; end
					endcase
					loc_index <= loc_index+1;	 					     					     
				end
				else if (cp_ctrl_in_crc_avl) begin
					loc_crc_avl        <= 1'b1;
					cp_ctrl_out_crc_en <= 1'b0;
					loc_frame_buf_20 <= cp_ctrl_in_crc[7:0];
					loc_frame_buf_21 <= cp_ctrl_in_crc[15:8];
					loc_frame_buf_22 <= cp_ctrl_in_crc[23:16];
					loc_frame_buf_23 <= cp_ctrl_in_crc[31:24];
				end
				else if(loc_crc_avl)begin
					if(loc_index<cp_ctrl_out_tx_frame_len)begin
						cp_ctrl_out_pkt_data_val <= 1'b1;	
						if(loc_index ==cp_ctrl_out_tx_frame_len-4)begin
							cp_ctrl_out_pkt_data<=loc_frame_buf_20;
							loc_index<=loc_index+1;
						end
						if(loc_index==cp_ctrl_out_tx_frame_len-3)begin
							cp_ctrl_out_pkt_data<=loc_frame_buf_21;
							loc_index<=loc_index+1;
						end
						if(loc_index==cp_ctrl_out_tx_frame_len-2)begin
							cp_ctrl_out_pkt_data<=loc_frame_buf_22;
							loc_index<=loc_index+1;
						end
						if(loc_index==cp_ctrl_out_tx_frame_len-1)begin
							cp_ctrl_out_pkt_data<=loc_frame_buf_23;
							loc_index<=loc_index+1;
						end
					end
					else
						cp_ctrl_out_pkt_data_val <= 1'b0;	
				end
				else if(loc_bitmap_added) begin
					cp_ctrl_out_crc_data_val <= 1'b0;
					cp_ctrl_out_pkt_data_val <= 1'b0;
				end
				if(loc_ba_bmp_wr_val&&loc_clear_done)begin //fill BA or compressed BA
					if(loc_ba_compressed)begin 
						if(cp_ctrl_in_implicit)begin //fill_implicit_comp_ba
							if(loc_count<`BA_BASIC_BITMAP_BUF_SIZE+4)begin
								if(!loc_sta_ba_data_val)begin
									loc_sta_ba_data_val      <= 1'b1;
									cp_ctrl_out_pkt_data_val <= 1'b0;
									cp_ctrl_out_crc_data_val <= 1'b0;	
								end
								else if((sb_new_buf_winstart&`BA_BASIC_BITMAP_BUF_MASK)%2 == 0) begin
									loc_count<=loc_count +2;
									if((loc_count==7'd10 )||(loc_count==7'd18)||(loc_count==7'd26)||
									   (loc_count==7'd34)||(loc_count==7'd42)||(loc_count==7'd50)||
									   (loc_count==7'd58)||(loc_count==7'd66))begin
										loc_accum_data <= 0;
										cp_ctrl_out_pkt_data_val <= 1'b1;
										cp_ctrl_out_pkt_data     <= loc_accum_data | {cp_ctrl_in_sta_ba_data[16],cp_ctrl_in_sta_ba_data[0],6'h0};
										cp_ctrl_out_crc_data_val <= 1'b1;
										cp_ctrl_out_crc_data     <= loc_accum_data | {cp_ctrl_in_sta_ba_data[16],cp_ctrl_in_sta_ba_data[0],6'h0};
										loc_index <= loc_index + 1;
										loc_temp_count <= 0;
										if(loc_count ==7'd66)begin
											loc_bitmap_added <= 1'b1;
										end
									end
									else begin
										if(loc_count>7'd2)begin 
											loc_accum_data <= loc_accum_data|({cp_ctrl_in_sta_ba_data[16],cp_ctrl_in_sta_ba_data[0]} << loc_temp_count);
											loc_temp_count <= loc_temp_count + 2;
										end
										cp_ctrl_out_pkt_data_val <= 1'b0;
										cp_ctrl_out_crc_data_val <= 1'b0;									
									end
								end
								else begin
									if((loc_count==7'd10 )||(loc_count==7'd18)||(loc_count==7'd26)||
									   (loc_count==7'd34)||(loc_count==7'd42)||(loc_count==7'd50)||
									   (loc_count==7'd58)||(loc_count==7'd66))begin
										loc_accum_data <= 0;
										cp_ctrl_out_pkt_data_val <= 1'b1;
										cp_ctrl_out_pkt_data     <= loc_accum_data|{cp_ctrl_in_sta_ba_data[0],7'h0};
										cp_ctrl_out_crc_data_val <= 1'b1;
										cp_ctrl_out_crc_data     <= loc_accum_data|{cp_ctrl_in_sta_ba_data[0],7'h0};
										loc_index <= loc_index + 1;
										loc_count<=loc_count +2;
										loc_temp_count <= 0;
										if(loc_count ==7'd66)begin
											loc_bitmap_added <= 1'b1;
										end
									end
									else begin
										if(loc_temp_count != 3)
											loc_count<=loc_count +2;
										if(loc_count>7'd2)begin
											if(loc_temp_count==0)begin
												loc_accum_data <= loc_accum_data|({7'h0,cp_ctrl_in_sta_ba_data[16]});
												loc_temp_count <= loc_temp_count + 1;
											end
											else begin
												loc_accum_data <= loc_accum_data|({cp_ctrl_in_sta_ba_data[16],cp_ctrl_in_sta_ba_data[0]} << loc_temp_count);
												loc_temp_count <= loc_temp_count + 2;
											end
										end
										cp_ctrl_out_pkt_data_val <= 1'b0;
										cp_ctrl_out_crc_data_val <= 1'b0;									
									end
								end
							end
						end
						else if(loc_count<`BA_BASIC_BITMAP_BUF_SIZE+4)begin//not implicit  
							if(!loc_sta_ba_data_val)begin
								loc_sta_ba_data_val      <= 1'b1;
								cp_ctrl_out_pkt_data_val <= 1'b0;
								cp_ctrl_out_crc_data_val <= 1'b0;	
							end
							else if((sb_new_buf_winstart&`BA_BASIC_BITMAP_BUF_MASK)%2 == 0) begin
								loc_count<=loc_count +2;
								if((loc_count==7'd10 )||(loc_count==7'd18)||(loc_count==7'd26)||
								   (loc_count==7'd34)||(loc_count==7'd42)||(loc_count==7'd50)||
								   (loc_count==7'd58)||(loc_count==7'd66))begin
									loc_accum_data <= 0;
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= loc_accum_data | {cp_ctrl_in_sta_ba_data[16],cp_ctrl_in_sta_ba_data[0],6'h0};
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= loc_accum_data | {cp_ctrl_in_sta_ba_data[16],cp_ctrl_in_sta_ba_data[0],6'h0};
									loc_index <= loc_index + 1;
									loc_temp_count <= 0;
									if(loc_count ==7'd66)begin
										loc_bitmap_added <= 1'b1;
									end
								end
								else begin
									if(loc_count>7'd2)begin 
										loc_accum_data <= loc_accum_data|({cp_ctrl_in_sta_ba_data[16],cp_ctrl_in_sta_ba_data[0]} << loc_temp_count);
										loc_temp_count <= loc_temp_count + 2;
									end
									cp_ctrl_out_pkt_data_val <= 1'b0;
									cp_ctrl_out_crc_data_val <= 1'b0;									
								end
							end
							else begin
								if((loc_count==7'd10 )||(loc_count==7'd18)||(loc_count==7'd26)||
								   (loc_count==7'd34)||(loc_count==7'd42)||(loc_count==7'd50)||
								   (loc_count==7'd58)||(loc_count==7'd66))begin
									loc_accum_data <= 0;
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= loc_accum_data|{cp_ctrl_in_sta_ba_data[0],7'h0};
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= loc_accum_data|{cp_ctrl_in_sta_ba_data[0],7'h0};
									loc_index <= loc_index + 1;
									loc_count<=loc_count +2;
									loc_temp_count <= 0;
									if(loc_count ==7'd66)begin
										loc_bitmap_added <= 1'b1;
									end
								end
								else begin
									if(loc_temp_count != 3)
										loc_count<=loc_count +2;
									if(loc_count>7'd2)begin
										if(loc_temp_count==0)begin
											loc_accum_data <= loc_accum_data|({7'h0,cp_ctrl_in_sta_ba_data[16]});
											loc_temp_count <= loc_temp_count + 1;
										end
										else begin
											loc_accum_data <= loc_accum_data|({cp_ctrl_in_sta_ba_data[16],cp_ctrl_in_sta_ba_data[0]} << loc_temp_count);
											loc_temp_count <= loc_temp_count + 2;
										end
									end
									cp_ctrl_out_pkt_data_val <= 1'b0;
									cp_ctrl_out_crc_data_val <= 1'b0;									
								end
							end
						end
					end
					else	if(loc_count<`BA_BASIC_BITMAP_BUF_SIZE)begin//not compressed
						if(!loc_sta_ba_data_val)begin
							loc_sta_ba_data_val      <= 1'b1;
							cp_ctrl_out_pkt_data_val <= 1'b0;
							cp_ctrl_out_crc_data_val <= 1'b0;	
						end
						else if((sb_new_buf_winstart&`BA_BASIC_BITMAP_BUF_MASK)%2==0)begin
							case (loc_temp_count)
								0:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[7:0];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[7:0];
									loc_temp_count           <= loc_temp_count +1;
									loc_index                <= loc_index +1;
								end
								1:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[15:8];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[15:8];
									loc_temp_count           <= loc_temp_count +1;
									loc_index                <= loc_index +1;
									loc_count                <= loc_count +1;
								end
								2:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[23:16];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[23:16];
									loc_temp_count           <= loc_temp_count +1;
									loc_index                <= loc_index +1;
								end
								3:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[31:24];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[31:24];
									loc_temp_count           <= 0;
									loc_index                <= loc_index +1;
									loc_count                <= loc_count +1;
									if(loc_count ==7'd63)
										loc_bitmap_added <= 1'b1;
								end
							endcase
						end
						else if(loc_count == 0)begin
							case (loc_temp_count)
								0:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[23:16];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[23:16];
									loc_temp_count           <= loc_temp_count +1;
									loc_index                <= loc_index +1;
								end
								1:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[31:24];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[31:24];
									loc_temp_count           <= 0;
									loc_index                <= loc_index +1;
									loc_count                <= loc_count +1;
								end
							endcase
						end
						else if(loc_count == 63)begin
							case(loc_temp_count)
								0:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[7:0];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[7:0];
									loc_temp_count           <= loc_temp_count +1;
									loc_index                <= loc_index +1;
								end
								1:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[15:8];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[15:8];
									loc_temp_count           <= 0;
									loc_index                <= loc_index +1;
									loc_count                <= loc_count +1;	
									if(loc_count ==7'd63)
										loc_bitmap_added <= 1'b1;
								end
							endcase
						end
						else begin
							case (loc_temp_count)
								0:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[7:0];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[7:0];
									loc_temp_count           <= loc_temp_count +1;
									loc_index                <= loc_index +1;
								end
								1:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[15:8];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[15:8];
									loc_temp_count           <= loc_temp_count +1;
									loc_index                <= loc_index +1;
									loc_count                <= loc_count +1;
								end
								2:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[23:16];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[23:16];
									loc_temp_count           <= loc_temp_count +1;
									loc_index                <= loc_index +1;
								end
								3:begin
									cp_ctrl_out_pkt_data_val <= 1'b1;
									cp_ctrl_out_pkt_data     <= cp_ctrl_in_sta_ba_data[31:24];
									cp_ctrl_out_crc_data_val <= 1'b1;
									cp_ctrl_out_crc_data     <= cp_ctrl_in_sta_ba_data[31:24];
									loc_temp_count           <= 0;
									loc_index                <= loc_index +1;
									loc_count                <= loc_count +1;
								end
							endcase
						end
					end
				end
				else if(loc_sb_fill_ba[96]&&loc_ba_bmp_wr_val)begin
					if(loc_ba_compressed)begin
						if(loc_index<28)begin
							cp_ctrl_out_pkt_data_val <= 1'b1;
							cp_ctrl_out_pkt_data     <= 0;
							cp_ctrl_out_crc_data_val <= 1'b1;
							cp_ctrl_out_crc_data     <= 0;
							loc_index                <= loc_index +1;

						end
						else if(!cp_ctrl_in_crc_avl&&!loc_crc_avl)begin
							cp_ctrl_out_pkt_data_val <= 1'b0;
							cp_ctrl_out_pkt_data     <= 0;
							cp_ctrl_out_crc_data_val <= 1'b0;
							cp_ctrl_out_crc_data     <= 0;
						end
					end
					else begin
						if(loc_index<148)begin
							cp_ctrl_out_pkt_data_val <= 1'b1;
							cp_ctrl_out_pkt_data     <= 0;
							cp_ctrl_out_crc_data_val <= 1'b1;
							cp_ctrl_out_crc_data     <= 0;
							loc_index                <= loc_index +1;

						end
						else if(!cp_ctrl_in_crc_avl&&!loc_crc_avl)begin
							cp_ctrl_out_pkt_data_val <= 1'b0;
							cp_ctrl_out_pkt_data     <= 0;
							cp_ctrl_out_crc_data_val <= 1'b0;
							cp_ctrl_out_crc_data     <= 0;
						end
					end
				end
			end
		end
	end


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_winsize_rd  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			loc_winsize_rd  <= 0;
		end
		else if(loc_count2 == 6)begin
			loc_winsize_rd <= 1;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_count2 <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			loc_count2 <= 0;
		end
		else if(loc_is_ba)begin
			if(loc_count2 < 12)
				loc_count2 <= loc_count2 +1;
		end
		else begin
			loc_count2 <= 0;
		end
	end


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_buf_winstart  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			sb_buf_winstart  <= 0;
		end
		else if(loc_count2 == 2)begin
			sb_buf_winstart  <= cp_ctrl_in_sta_ba_data[25:16];//10 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_new_buf_winstart  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			sb_new_buf_winstart  <= 0;
		end
		else if(loc_count2 == 7)begin
			sb_new_buf_winstart <= loc_sb_fill_ba[95:80];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_winstart  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			sb_winstart  <= 0;
		end
		else if(loc_count2 == 4)begin
			sb_winstart  <= cp_ctrl_in_sta_ba_data[31:16];//16 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_new_winstart  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			sb_new_winstart  <= 0;
		end
		else if(loc_count2 == 7)begin
			sb_new_winstart  <= loc_sb_fill_ba[79:64];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_winend  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			sb_winend  <= 0;
		end
		else if(loc_count2 == 6)begin
			sb_winend  <= cp_ctrl_in_sta_ba_data[15:0];//16 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_new_winend  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			sb_new_winend  <= 0;
		end
		else if(loc_count2 == 7)begin
			sb_new_winend  <= loc_sb_fill_ba[63:48];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			sb_winsize  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			sb_winsize  <= 0;
		end
		else if(loc_count2 == 6)begin
			sb_winsize  <= cp_ctrl_in_sta_ba_data[31:16];//16 bits
		end
	end


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_ctrl_out_sta_ba_en <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			cp_ctrl_out_sta_ba_en <= 0;
		end
		else if(loc_is_ba)begin
			cp_ctrl_out_sta_ba_en <= 1;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_frame_buf_18 <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			loc_frame_buf_18 <= 0;
		end
		else if(loc_count2 == 7)begin
			if(loc_is_implcom)begin
				loc_frame_buf_18 <= sb_winstart[7:0];
			end
			else begin
				loc_frame_buf_18 <= {loc_sb_fill_ba[3:0],4'h0};
			end
		end
		else if(loc_is_bar)begin
			loc_frame_buf_18 <= cp_ctrl_in_mpdu_bar_ssn [7 :0];
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_frame_buf_19 <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			loc_frame_buf_19 <= 0;
		end
		else if(loc_count2 == 7)begin
			if(loc_is_implcom)begin 
				loc_frame_buf_19 <= {4'h0,sb_winstart[11:8]};
			end
			else begin
				loc_frame_buf_19 <= loc_sb_fill_ba[11:4];
			end
		end
		else if(loc_is_bar)begin
			loc_frame_buf_19 <= cp_ctrl_in_mpdu_bar_ssn [15:0];
		end
	end
	
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_seq_no_bitmap_avl <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			loc_seq_no_bitmap_avl <= 0;
		end
		else if(loc_count2 == 7) begin
			loc_seq_no_bitmap_avl <= 1;
		end
	end
	
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_err <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			loc_err <= 0;
		end
		else if(loc_sb_fill_ba[96]) begin
			loc_err <= `ERROR;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			clear_start  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			clear_start  <= 0;
		end
		else if(!loc_sb_fill_ba[87])begin
			if(loc_count2 == 7)begin
				clear_start <= loc_sb_fill_ba[37:32];//take useful 6 bits
			end
			else if(loc_count2>10)begin
				if(clear_start<=clear_end)
					clear_start <= clear_start + 1;
			end
		end
		else begin
			clear_start  <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			clear_end  <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			clear_end  <= 0;
		end
		else if((loc_count2 == 7)&& !loc_sb_fill_ba[87])begin
			clear_end <= loc_sb_fill_ba[21:16];//take useful 6 bits
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_ctrl_out_sta_ba_wen <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			cp_ctrl_out_sta_ba_wen <= 0;
		end
		else if(loc_is_ba)begin
			if(loc_winsize_rd)begin
				if(loc_is_implcom)
					cp_ctrl_out_sta_ba_wen  <= 0;
				else if(!loc_sb_fill_ba[96])begin
					if((loc_count2==8)||(loc_count2==9))
						cp_ctrl_out_sta_ba_wen  <= 4'hc;
					else if(loc_count2==10)
						cp_ctrl_out_sta_ba_wen  <= 4'h3;
					else if(loc_count2>10)
						if(clear_start <= clear_end)begin
							if(clear_start%2)
								cp_ctrl_out_sta_ba_wen <= 4'h3;
							else
								cp_ctrl_out_sta_ba_wen <= 4'hC;
						end
						else 
							cp_ctrl_out_sta_ba_wen <= 0;
					else 
						cp_ctrl_out_sta_ba_wen <= 0;
				end
				else 
					cp_ctrl_out_sta_ba_wen <= 0;
			end
			else 
				cp_ctrl_out_sta_ba_wen <= 0;
		end
		else 
			cp_ctrl_out_sta_ba_wen <= 0;
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_clear_done <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			loc_clear_done <= 0;
		end
		else if(loc_is_ba)begin
			if(loc_winsize_rd)begin
				if(loc_is_implcom)
					loc_clear_done <= 1;
				else	if(!loc_sb_fill_ba[96])
					if(cp_ctrl_in_crc_avl)
						loc_clear_done <= 0;
					else if(clear_start>clear_end)
						loc_clear_done <= 1;
				else 
					loc_clear_done <= 0;
			end
			else 
				loc_clear_done <= 0;					
		end
		else begin
			loc_clear_done <= 0;
		end
	end


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_ctrl_out_sta_ba_addr <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			cp_ctrl_out_sta_ba_addr <= 0;
		end
		else if(loc_is_ba)begin
			if(!loc_winsize_rd)begin
				if(loc_count2 == 0)
					cp_ctrl_out_sta_ba_addr <= cp_ctrl_in_sta_offset+33;
				else if(loc_count2 == 2)
					cp_ctrl_out_sta_ba_addr <= cp_ctrl_in_sta_offset+66;
				else if(loc_count2 == 4)
					cp_ctrl_out_sta_ba_addr <= cp_ctrl_in_sta_offset+67;
			end
			else if(loc_is_implcom)begin
				if(loc_count<`BA_BASIC_BITMAP_BUF_SIZE+4)
 					cp_ctrl_out_sta_ba_addr <= cp_ctrl_in_sta_offset + 1 + (((loc_count+sb_buf_winstart)&`BA_BASIC_BITMAP_BUF_MASK)>>1) 				                                                                                         + (((loc_count+sb_buf_winstart)&`BA_BASIC_BITMAP_BUF_MASK)%2);
  			end
			else if(!loc_sb_fill_ba[96])begin
				if(loc_count2==8)
					cp_ctrl_out_sta_ba_addr  <= cp_ctrl_in_sta_offset+33;
				else if(loc_count2==9)
					cp_ctrl_out_sta_ba_addr  <= cp_ctrl_in_sta_offset+66;
				else if(loc_count2==10)
					cp_ctrl_out_sta_ba_addr  <= cp_ctrl_in_sta_offset+67;
				else if(loc_count2>10)begin
					if(clear_start <= clear_end)
						cp_ctrl_out_sta_ba_addr  <= cp_ctrl_in_sta_offset+1+(clear_start[5:0]>>1)+(clear_start%2);
					else if(loc_ba_bmp_wr_val&&loc_clear_done)begin 
						if(loc_ba_compressed)begin
							if(loc_count<`BA_BASIC_BITMAP_BUF_SIZE+4)
								cp_ctrl_out_sta_ba_addr <= cp_ctrl_in_sta_offset + 1 + (((loc_count+sb_new_buf_winstart)&`BA_BASIC_BITMAP_BUF_MASK)>>1)                                                                                                         + (((loc_count+sb_new_buf_winstart)&`BA_BASIC_BITMAP_BUF_MASK)%2);
						end
						else 
							if(loc_count<`BA_BASIC_BITMAP_BUF_SIZE)
								cp_ctrl_out_sta_ba_addr <= cp_ctrl_in_sta_offset + 1 + (((loc_count+sb_new_buf_winstart)&`BA_BASIC_BITMAP_BUF_MASK)>>1) 
						                                                     + (((loc_count+sb_new_buf_winstart)&`BA_BASIC_BITMAP_BUF_MASK)%2);
					end
					else begin
						cp_ctrl_out_sta_ba_addr <= cp_ctrl_in_sta_offset + 1 + (((loc_count+sb_new_buf_winstart)&`BA_BASIC_BITMAP_BUF_MASK)>>1) 
						                                                     + (((loc_count+sb_new_buf_winstart)&`BA_BASIC_BITMAP_BUF_MASK)%2);
					end
				end
			end
		end
		else begin
			cp_ctrl_out_sta_ba_addr <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_ctrl_out_sta_ba_data <= 0;
		end
		else if(!cp_ctrl_pkt_gen_en)begin
			cp_ctrl_out_sta_ba_data <= 0;
		end
		else if(loc_is_ba)begin
			if(loc_winsize_rd)begin
				if(loc_is_implcom)
					cp_ctrl_out_sta_ba_data <= 0;
				else if(!loc_sb_fill_ba[96])begin
					if(loc_count2==8)
						cp_ctrl_out_sta_ba_data <= {6'h0,sb_new_buf_winstart,16'h0};
					else if(loc_count2==9)
						cp_ctrl_out_sta_ba_data <= {sb_new_winstart,16'h0};
					else if(loc_count2==10)
						cp_ctrl_out_sta_ba_data <= {16'h0,sb_new_winend};
					else if(loc_count2>10)
						cp_ctrl_out_sta_ba_data <= 0;
				end
				else 
					cp_ctrl_out_sta_ba_data <= 0;
			end
			else 
				cp_ctrl_out_sta_ba_data <= 0;
		end
		else 
			cp_ctrl_out_sta_ba_data <= 0;
	end
	
	/*---------------------------------------------------------------------------------------------------------*/
endmodule
//EOF

