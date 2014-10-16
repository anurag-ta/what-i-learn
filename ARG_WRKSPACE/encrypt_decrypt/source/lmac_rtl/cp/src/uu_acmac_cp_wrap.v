`include "../../inc/defines.h" 

module uu_acmac_cp_wrap(
	input                clk                        ,
	input                rst_n                      ,
	input                cp_en                      ,
	//Call from Rx_handler
	input                cp_in_process_rx_frame     ,
	//indicaiton from rx_handler
	input                cp_in_is_rxend             ,
	output               cp_out_tx_upd_mpdu_status  , //cp_out_tx_upd_mpdu_status
	output               cp_out_tx_get_ampdu_status , //cp_out_tx_get_ampdu_status
	//ba_ses_info memory interface (depth : 138 used * 40)(width : 16 bits)
	output       [31:0]  cp_out_sta_ba_data         , 
	output       [3 :0]  cp_out_sta_ba_wen          ,
	output               cp_out_sta_ba_en           ,
	input        [31:0]  cp_in_sta_ba_data          ,
	output       [14:0]  cp_out_sta_ba_addr         ,
	//Station & BA Management interface 
	input                cp_in_sta_info_val         ,
	input                cp_in_sta_info             ,
	input        [14:0]  cp_in_sta_offset           ,
	output               cp_out_sta_get_info        ,
	output       [47:0]  cp_out_sta_addr            ,
	output       [3 :0]  cp_out_sta_tid             ,
	output               cp_out_sta_dir             ,
	//Tx side inputs from Tx_handler 
	input                cp_in_tx_start_ind         ,
	input                cp_in_tx_self_cts          ,
	input        [1 :0]  cp_in_tx_txvec_format      ,
	input        [3 :0]  cp_in_tx_rtscts_rate       ,
	input        [47:0]  cp_in_tx_mpdu_ra           ,
	//inputs from CAP
	input                cp_in_waiting_for_cts      ,
	input                cp_in_waiting_for_ack      ,
	//CRC32 interface  
	input                cp_in_crc_avl              ,
	input        [31:0]  cp_in_crc                  ,
	output       [15:0]  cp_out_crc_len             ,
	output               cp_out_crc_en              ,
	output               cp_out_crc_data_val        ,
	output       [7 :0]  cp_out_crc_data            ,
	//Inputs for BAR frame generation 
	input        [47:0]  cp_in_bar_address          ,
	input        [15:0]  cp_in_bar_scf              ,
	input        [3 :0]  cp_in_bar_tid              ,
	input        [1 :0]  cp_in_bar_bartype          ,
	input        [15:0]  cp_in_bar_duration         ,
	input                cp_in_bar_valid            ,
	//vht_mcs_table_g memory interface (depth : 720 used)(width : 40 bits)
	output               cp_out_vht_mcs_en          ,
	output               cp_out_vht_mcs_wen         ,
	output       [9 :0]  cp_out_vht_mcs_addr        ,
	input        [39:0]  cp_in_vht_mcs_data         ,
	//ht_mcs_table_g memory interface (depth : 160 used)(width : 40 bits)
	output               cp_out_ht_mcs_en           ,
	output               cp_out_ht_mcs_wen          ,
	output       [7 :0]  cp_out_ht_mcs_addr         ,
	input        [39:0]  cp_in_ht_mcs_data          ,
	//lrate_table_g memory interface (depth : 24 used)(width : 40 bits)
	output               cp_out_lrate_en            ,
	output               cp_out_lrate_wen           ,
	output       [5 :0]  cp_out_lrate_addr          ,
	input        [39:0]  cp_in_lrate_data           ,
	//Register Interface
	input        [15:0]  cp_in_BSSBasicRateSet      ,
	input        [15:0]  cp_in_SIFS_timer_value     ,
	//Tx_Interface	
	output       [1 :0]  cp_out_tx_ac               ,
	output       [7 :0]  cp_out_tx_ctrl_fr_len      ,
	output       [15:0]  cp_out_tx_seqno            ,
	input                cp_in_tx_res_val           ,
	input        [31:0]  cp_in_tx_res               ,
	output               cp_out_tx_res_val          ,
	output       [31:0]  cp_out_tx_res              ,
	//output reponse to rx handler
	output  reg  [31:0]  cp_out_rx_res              , 
	output  reg          cp_out_rx_res_en           ,
	//Memory rx_frame_info (depth : 1536 used)(width : 8 bits)
	output  reg          cp_out_rx_info_en          ,
	output  reg  [15:0]  cp_out_rx_info_addr        ,
	input        [7 :0]  cp_in_rx_info_data         ,
	//Memory tx_ctrl_frame_info (depth : 208 used)(width : 8 bits)
	output  reg          cp_out_tx_info_en          ,
	output  reg          cp_out_tx_info_wen         ,
	output  reg  [7 :0]  cp_out_tx_info_addr        ,
	output  reg  [7 :0]  cp_out_tx_info_data        ,          
	//added for de-aggr case
	input        [15:0]  cp_in_rx_seq_no            ,
	input                cp_in_rx_is_aggr
	);
	
	/* Registers internal to the module 
	.................................................................................*/
	reg           loc_ev_rxstart_ind      ; 
	reg           loc_ev_rxdata_ind       ; 
	reg           loc_ev_rxdata_ind_dly   ; 
	wire  [7 :0]  loc_tx_data             ;
	wire          loc_tx_data_val         ;
	wire  [7 :0]  lco_packet_length       ;
	wire          loc_txvec_is_fec_ldpc   ;
	wire          loc_txvec_is_short_GI   ;
	wire  [1 :0]  loc_txvec_format        ;
	wire  [3 :0]  loc_txvec_L_datarate    ; 
	wire  [11:0]  loc_txvec_L_length      ;
	wire  [15:0]  loc_txvec_ht_length     ;
	wire  [6 :0]  loc_txvec_mcs           ;
	wire  [1 :0]  loc_txvec_stbc          ;
	reg           loc_rx_data_val         ;
	reg   [7 :0]  loc_rx_data             ; 
	reg   [7 :0]  count                   ;
	reg   [7 :0]  count2                  ;
	reg   [7 :0]  count3,count4          ;
	reg           loc_mem_data_val        ;
	reg           loc_rxstart_ind         ;
	reg           loc_rxdata_ind          ;
	reg           loc_is_rxend            ;
	wire   [7 :0]  loc_packet_length       ; 
	wire  [31:0]  loc_rx_res              ; 
	wire          loc_rx_res_en           ;
	reg   [31:0]  loc_rx_res_r            ; 
	reg           loc_rx_res_en_r         ;
	wire          loc_ev_rxdata_ind_cp    ;
wire          loc_tx_res_val          ;
	reg           loc_tx_response_val     ;
	reg   [15:0]  loc_rx_seq_no           ;
	reg           de_aggr_count           ;
	reg           loc_rx_aggr             ;
	/*-------------------------------------------------------------------------------*/
	assign loc_ev_rxdata_ind_cp = loc_ev_rxdata_ind&(~loc_ev_rxdata_ind_dly);
	assign cp_out_tx_ctrl_fr_len = loc_packet_length;
        assign cp_out_tx_res_val = loc_tx_res_val;

	/* Module Instantiations
	.................................................................................*/
	uu_acmac_cp_if CP_IF(
		.clk                        (clk                        ),
		.rst_n                      (rst_n                      ),
		.cp_en                      (cp_en                      ),
		.cp_in_ev_rxstart_ind       (loc_ev_rxstart_ind         ), 
		.cp_in_ev_rxdata_ind        (loc_ev_rxdata_ind_cp       ), 
		.cp_in_is_rxend             (loc_is_rxend               ),
   	.cp_out_tx_upd_mpdu_status  (cp_out_tx_upd_mpdu_status  ),
		.cp_out_tx_get_ampdu_status (cp_out_tx_get_ampdu_status ),       
		.cp_in_rx_data_val          (loc_rx_data_val            ),
		.cp_in_rx_data              (cp_in_rx_info_data         ), 
		.cp_out_sta_ba_data         (cp_out_sta_ba_data         ), 
		.cp_out_sta_ba_wen          (cp_out_sta_ba_wen          ),
		.cp_out_sta_ba_en           (cp_out_sta_ba_en           ),
		.cp_in_sta_ba_data          (cp_in_sta_ba_data          ),
		.cp_out_sta_ba_addr         (cp_out_sta_ba_addr         ),
		.cp_in_sta_info_val         (cp_in_sta_info_val         ),
		.cp_in_sta_info             (cp_in_sta_info             ),
		.cp_in_sta_offset           (cp_in_sta_offset           ),
		.cp_out_sta_get_info        (cp_out_sta_get_info        ),
		.cp_out_sta_addr            (cp_out_sta_addr            ),
		.cp_out_sta_tid             (cp_out_sta_tid             ),
		.cp_out_sta_dir             (cp_out_sta_dir             ),
		.cp_in_tx_start_ind         (cp_in_tx_start_ind         ),
		.cp_in_tx_self_cts          (cp_in_tx_self_cts          ),
		.cp_in_tx_txvec_format      (cp_in_tx_txvec_format      ),
		.cp_in_tx_rtscts_rate       (cp_in_tx_rtscts_rate       ),
		.cp_in_tx_mpdu_ra           (cp_in_tx_mpdu_ra           ),		   
		.cp_out_tx_ac               (cp_out_tx_ac               ),
		.cp_out_tx_seqno            (cp_out_tx_seqno            ),
		.cp_in_tx_res_val           (cp_in_tx_res_val           ),
		.cp_in_tx_res               (cp_in_tx_res               ),
		//.cp_out_tx_res_val          (cp_out_tx_res_val          ),
                .cp_out_tx_res_val          (loc_tx_res_val             ),
		.cp_out_tx_res              (cp_out_tx_res              ),
		.cp_in_waiting_for_cts      (cp_in_waiting_for_cts      ),
		.cp_in_waiting_for_ack      (cp_in_waiting_for_ack      ),
		.cp_in_crc_avl              (cp_in_crc_avl              ),
		.cp_in_crc                  (cp_in_crc                  ),
		.cp_out_crc_len             (cp_out_crc_len             ),
		.cp_out_crc_en              (cp_out_crc_en              ),
		.cp_out_crc_data_val        (cp_out_crc_data_val        ),
		.cp_out_crc_data            (cp_out_crc_data            ),
		.cp_in_bar_address          (cp_in_bar_address          ),
		.cp_in_bar_scf              (cp_in_bar_scf              ),
		.cp_in_bar_tid              (cp_in_bar_tid              ),
		.cp_in_bar_bartype          (cp_in_bar_bartype          ),
		.cp_in_bar_duration         (cp_in_bar_duration         ),
		.cp_in_bar_valid            (cp_in_bar_valid            ),
		.cp_out_vht_mcs_en          (cp_out_vht_mcs_en          ),
		.cp_out_vht_mcs_wen         (cp_out_vht_mcs_wen         ),
		.cp_out_vht_mcs_addr        (cp_out_vht_mcs_addr        ),
		.cp_in_vht_mcs_data         (cp_in_vht_mcs_data         ),
		.cp_out_ht_mcs_en           (cp_out_ht_mcs_en           ),
		.cp_out_ht_mcs_wen          (cp_out_ht_mcs_wen          ),
		.cp_out_ht_mcs_addr         (cp_out_ht_mcs_addr         ),
		.cp_in_ht_mcs_data          (cp_in_ht_mcs_data          ),
		.cp_out_lrate_en            (cp_out_lrate_en            ),
		.cp_out_lrate_wen           (cp_out_lrate_wen           ),
		.cp_out_lrate_addr          (cp_out_lrate_addr          ),
		.cp_in_lrate_data           (cp_in_lrate_data           ),
		.cp_in_BSSBasicRateSet      (cp_in_BSSBasicRateSet      ),
		.cp_in_SIFS_timer_value     (cp_in_SIFS_timer_value     ),
		.cp_out_txvec_is_fec_ldpc   (loc_txvec_is_fec_ldpc      ),
		.cp_out_txvec_is_short_GI   (loc_txvec_is_short_GI      ),
		.cp_out_txvec_format        (loc_txvec_format           ),
		.cp_out_txvec_L_datarate    (loc_txvec_L_datarate       ), 
		.cp_out_txvec_L_length      (loc_txvec_L_length         ),
		.cp_out_txvec_ht_length     (loc_txvec_ht_length        ),
		.cp_out_txvec_mcs           (loc_txvec_mcs              ),
		.cp_out_txvec_stbc          (loc_txvec_stbc             ),
		.cp_out_rx_res              (loc_rx_res                 ), 
		.cp_out_rx_res_en           (loc_rx_res_en              ),
		.cp_out_packet_length       (loc_packet_length          ),
		.cp_out_tx_data             (loc_tx_data                ),
		.cp_out_tx_data_val         (loc_tx_data_val            ),    
		.cp_in_rx_seq_no            (loc_rx_seq_no              ),
		.cp_in_rx_aggr              (loc_rx_aggr                )
		);
	/*-------------------------------------------------------------------------------*/

	/* Concurrent statements
	.................................................................................*/	
	//added for de-aggr case
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_rx_aggr <= 'd0;
		end
		else if(!cp_en)begin
			loc_rx_aggr <= 'd0;
		end
		else if(de_aggr_count)begin
			loc_rx_aggr <= cp_in_process_rx_frame && cp_in_rx_is_aggr;
		end
		else begin
			loc_rx_aggr <= 'd0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			de_aggr_count <= 'd0;
		end
		else if(!cp_en)begin
			de_aggr_count <= 'd0;
		end
		else if(cp_in_process_rx_frame && cp_in_rx_is_aggr)begin
			if(!de_aggr_count)
				de_aggr_count <= de_aggr_count + 'b1;
		end
		else if(loc_is_rxend && loc_rx_res_en)begin
			de_aggr_count <= 'd0;
		end
end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_rx_seq_no <= 'd0;
		end
		else if (!cp_en) begin
			loc_rx_seq_no <= 'd0;
		end
		else if(cp_in_process_rx_frame)begin
			loc_rx_seq_no <= cp_in_rx_seq_no;
		end
	end

	//RX_MEMORY READ --------> BEGIN
	//latching is_rxend
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_is_rxend <= 0;
		end
		else if (!cp_en) begin
			loc_is_rxend <= 0;
		end
		else if(loc_rx_res_en)begin
			loc_is_rxend <= 0;
		end
		else if(cp_in_is_rxend)begin
			loc_is_rxend <= 1;
		end
	end

	//sending rxstart indication to CP_IF
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_ev_rxstart_ind <= 'b0;
		end
		else if (!cp_en) begin
			loc_ev_rxstart_ind <= 'b0;
		end
		else if(cp_in_process_rx_frame)begin
			if(!de_aggr_count)
				loc_ev_rxstart_ind <= 'b1;
			else  
				loc_ev_rxstart_ind <= 'b0;
		end
		else begin
			loc_ev_rxstart_ind <= 'b0;
		end
	end

	//sending rxdata indication to CP_IF
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_ev_rxdata_ind <= 0;
		end
		else if(!cp_en)begin
			loc_ev_rxdata_ind <= 0;
		end
		else if(count == 8'd21)begin
			loc_ev_rxdata_ind <= 1;			
		end
		else begin
			loc_ev_rxdata_ind <= 0;			
		end
	end

	//dealyed rxdata indication
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_ev_rxdata_ind_dly <= 0;
		end
		else if(!cp_en)begin
			loc_ev_rxdata_ind_dly <= 0;
		end
		else begin		
			loc_ev_rxdata_ind_dly <= loc_ev_rxdata_ind;			
		end
	end

	//lactching event rx_ev_data_ind
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_rxdata_ind <= 0;
		end
		else if(!cp_en)begin
			loc_rxdata_ind <= 0;
		end
		else if(loc_ev_rxdata_ind)begin
			loc_rxdata_ind <= 1;			
		end
		else if((count == 8'd61)||(loc_rx_res_en))begin
			loc_rxdata_ind <= 0;			
		end
	end

	//lactching event rx_ev_start_ind
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_rxstart_ind <= 0;
		end
		else if (!cp_en) begin
			loc_rxstart_ind <= 0;
		end
		else if(loc_ev_rxstart_ind)begin
			loc_rxstart_ind <= 1;			
		end
		else if(count == 8'd20)begin
			loc_rxstart_ind <= 0;			
		end
	end      
	
	//send data_val and data signals to CP_IF by reading frame from rx_frame buffer
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_rx_data_val     <= 0;
			loc_rx_data         <= 0;
			loc_mem_data_val    <= 0;
			cp_out_rx_info_en   <= 0;
			cp_out_rx_info_addr <= 0;
			count               <= 0;
		end
		else if (!cp_en) begin
			loc_rx_data_val     <= 0;
			loc_rx_data         <= 0;
			loc_mem_data_val    <= 0;
			cp_out_rx_info_en   <= 0;
			cp_out_rx_info_addr <= 0;
			count               <= 0;
		end
		else if(loc_rx_res_en)begin
			count      <= 0;
			cp_out_rx_info_en <= 0;
		end
		else if(loc_rxstart_ind)begin
			cp_out_rx_info_en <= 1;
			if(count <= 8'd20)begin
				if(!loc_mem_data_val)begin
					loc_rx_data_val <= 0;
					loc_rx_data     <= 0;
					cp_out_rx_info_addr <= count;
					loc_mem_data_val <= 1;
				end
				else begin
					if(count == 8'd20)
						loc_mem_data_val <= 0;
					loc_rx_data_val <= 1;			
					cp_out_rx_info_addr <= count;
					loc_rx_data     <= cp_in_rx_info_data;
				end
				count <= count + 1;
			end
		end
		else if(loc_rxdata_ind)begin
			if(count <= 8'd61)begin
				if(!loc_mem_data_val)begin
					loc_rx_data_val <= 0;
					loc_rx_data     <= 0;
					cp_out_rx_info_addr <= count+8'd15;
					loc_mem_data_val <= 1;
				end
				else begin
					loc_rx_data_val  <= 1;			
					cp_out_rx_info_addr <= count+8'd15;
					loc_rx_data      <= cp_in_rx_info_data;
				end		
				count <= count + 1;
			end
		end

		else begin
			loc_rx_data_val <= 0;			
			loc_rx_data     <= 0;
			cp_out_rx_info_addr <= 0;
			loc_mem_data_val <= 0;

		end
	end
	//RX_MEMORY READ --------> END

	//TX_MEMORY WRITE --------> BEGIN
	//writing MPDU to tx_ctrl_frame_info
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_out_tx_info_en   <= 0;	
			cp_out_tx_info_wen  <= 0;
			cp_out_tx_info_addr <= 0; 
			cp_out_tx_info_data  <= 0;        
		end
		else if(!cp_en)begin
			cp_out_tx_info_en   <= 0;	
			cp_out_tx_info_wen  <= 0;
			cp_out_tx_info_addr <= 0; 
			cp_out_tx_info_data  <= 0;        
		end
		else if(loc_tx_data_val)begin
			cp_out_tx_info_en   <= 1'b1;	
			cp_out_tx_info_wen  <= 1'b1;
			cp_out_tx_info_addr <= count2 + 6'd56;
			cp_out_tx_info_data  <= loc_tx_data ;
		end
		else if(loc_rx_res_en_r)begin
		  cp_out_tx_info_en   <= 1'b1;
			cp_out_tx_info_wen  <= 1'b1;
			cp_out_tx_info_addr <= count3;
			case (count3)
				8'd0: cp_out_tx_info_data <= {6'd0,loc_txvec_format};
				8'd2: cp_out_tx_info_data <= {loc_packet_length[3:0],loc_txvec_L_datarate};
				8'd3: cp_out_tx_info_data <= {4'h0,loc_packet_length[7:4]};
				8'd8: cp_out_tx_info_data <= {1'd0,loc_txvec_mcs};
				8'd9: cp_out_tx_info_data <= {loc_txvec_ht_length[7:0]};
				8'd10: cp_out_tx_info_data <= {loc_txvec_ht_length[15:8]};
				8'd11: cp_out_tx_info_data <= {loc_txvec_is_short_GI,loc_txvec_is_fec_ldpc,loc_txvec_stbc,4'd0};
			endcase
		end
else if(loc_tx_response_val)begin
      cp_out_tx_info_en   <= 1'b1;
			cp_out_tx_info_wen  <= 1'b1;
			cp_out_tx_info_addr <= count4;
			case (count4)
				8'd0: cp_out_tx_info_data <= {6'd0,loc_txvec_format};
				8'd2: cp_out_tx_info_data <= {loc_packet_length[3:0],cp_in_tx_rtscts_rate};
				8'd3: cp_out_tx_info_data <= {4'h0,loc_packet_length[7:4]};
				8'd8: cp_out_tx_info_data <= {1'd0,loc_txvec_mcs};
				8'd9: cp_out_tx_info_data <= {loc_txvec_ht_length[7:0]};
				8'd10: cp_out_tx_info_data <= {loc_txvec_ht_length[15:8]};
				8'd11: cp_out_tx_info_data <= {loc_txvec_is_short_GI,loc_txvec_is_fec_ldpc,loc_txvec_stbc,4'd0};
			endcase
		end
		else begin
			cp_out_tx_info_en   <= 0;	
			cp_out_tx_info_wen  <= 0;
			cp_out_tx_info_addr <= 0;
		end
	end

	//txside mpdu counter increment
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			count2 <= 0;
		end
		else if (!cp_en) begin
			count2 <= 0;
		end
		else if(loc_tx_data_val)begin
			count2 <= count2 + 1;
		end
		//else if(loc_rx_res_en)begin
else if(loc_rx_res_en || loc_tx_res_val)begin
			count2 <= 0;
		end
	end  
	//txside txvec counter increment
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			count3 <= 0;
		end
		else if (!cp_en) begin
			count3 <= 0;
		end
		else if (cp_in_process_rx_frame)begin
			count3 <= 0;
		end
		else if(loc_rx_res_en_r)begin
			count3 <= count3+1; 
		end

	end
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_out_rx_res_en <= 0;
			cp_out_rx_res    <= 0;
		end
		else if (!cp_en) begin
			cp_out_rx_res_en <= 0;
			cp_out_rx_res    <= 0;
		end
		else if(count3 == 8'd12)begin 
			cp_out_rx_res_en <= loc_rx_res_en_r;
			cp_out_rx_res    <= loc_rx_res_r;			
		end
		else begin 
			cp_out_rx_res_en <= 0;
			cp_out_rx_res    <= 0;			
		end
	end
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_rx_res_en_r <= 0;
			loc_rx_res_r    <= 0;
		end
		else if (loc_rx_res_en) begin
			loc_rx_res_en_r <= loc_rx_res_en;
			loc_rx_res_r    <= loc_rx_res;
		end
		else if(cp_in_process_rx_frame || cp_out_rx_res_en)begin
			loc_rx_res_en_r <= 0;
			loc_rx_res_r    <= 0;
		end
	end

// Logic for loc_tx_response_valid
always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
		loc_tx_response_val <= 0;
		end 
	  else if (!cp_en) begin
			loc_tx_response_val <= 0;
		end
		else if (loc_tx_res_val) begin
		  loc_tx_response_val <= 1'b1;
		end
		else if (count4 == 8'd12) begin
		  loc_tx_response_val <= 0;
		  end
	end
always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
		count4 <= 0;
		end 
	  else if (!cp_en) begin
			count4 <= 0;
		end
		else if (loc_tx_response_val) begin
		  count4 <= count4 + 1;
		  end
		else if (!loc_tx_response_val) begin
		  count4 <= 0;
		 end
	end

	//TX_MEMORY WRITE --------> END
	/*-------------------------------------------------------------------------------*/

endmodule
/*----------EOF--------*/
