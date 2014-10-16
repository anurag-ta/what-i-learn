`include "../../inc/defines.h"

module uu_acmac_tx_duration(
	input                clk                        ,
	input                rst_n                      ,
	input                tx_dur_en                  ,
	input        [1 :0]  tx_dur_type                ,//00-> uu_calc_bcast_duration
	                                                 //01-> uu_calc_singlep_frame_duration
										    //10-> uu_calc_rtscts_duration 
										    //11-> uu_calc_impl_BA_duration
	input                tx_dur_in_cts_self         ,
	input        [3 :0]  tx_dur_in_rts_cts_rate     ,
	input        [7 :0]  tx_dur_in_exp_resp_len     ,
	input        [19:0]  tx_dur_in_next_frame_len   ,

	input        [1 :0]  tx_dur_in_txvec_format     ,
	input        [1 :0]  tx_dur_in_txvec_stbc       ,
	input        [6 :0]  tx_dur_in_txvec_mcs        ,
	input        [2 :0]  tx_dur_in_txvec_ch_bndwdth ,
	input        [11:0]  tx_dur_in_txvec_num_ext_ss ,
	input        [11:0]  tx_dur_in_txvec_L_length   ,
	input        [15:0]  tx_dur_in_txvec_ht_length  ,
	input        [3 :0]  tx_dur_in_txvec_L_datarate ,
	input        [2 :0]  tx_dur_in_txvec_modulation ,
	input                tx_dur_in_txvec_is_long_pre,
	input                tx_dur_in_txvec_is_short_GI,

	output       [1 :0]  tx_dur_out_resp_format     ,
	output       [15:0]  tx_dur_out_resp_ht_length  ,
	output       [6 :0]  tx_dur_out_resp_mcs        ,
	output       [3 :0]  tx_dur_out_resp_L_datarate ,
	output       [11:0]  tx_dur_out_resp_L_length   ,
	output       [11:0]  tx_dur_out_resp_upa_length ,
	output       [1 :0]  tx_dur_out_resp_stbc       ,

	//interface with memory vht_mcs_table_g
	output  reg          tx_dur_out_vht_mcs_en      ,
	//output  reg          tx_dur_out_vht_mcs_wen     ,
	output  reg  [9 :0]  tx_dur_out_vht_mcs_addr    ,
	input        [39:0]  tx_dur_in_vht_mcs_data     ,

	//interface with memory vh_mcs_table_g
	output  reg          tx_dur_out_ht_mcs_en       ,
	//output  reg          tx_dur_out_ht_mcs_wen      ,
	output  reg  [7 :0]  tx_dur_out_ht_mcs_addr     ,
	input        [39:0]  tx_dur_in_ht_mcs_data      ,

	//interface with memory lrate_table_g
	output  reg          tx_dur_out_lrate_en        ,
	//output  reg          tx_dur_out_lrate_wen       ,
	output  reg  [5 :0]  tx_dur_out_lrate_addr      ,
	input        [39:0]  tx_dur_in_lrate_data       ,
	
	//input register values 
	input        [15:0]  tx_dur_in_BSSBasicRateSet  ,
	input        [15:0]  tx_dur_in_SIFS_timer_value , 

	//output to Tx handler
	output  reg          tx_dur_out_dur_upd_val     ,
	output  reg  [15:0]  tx_dur_out_dur_upd
	);

	//Register Declarations
	reg  [1 :0]  loc_resp_format     ;
	reg  [15:0]  loc_resp_ht_length  ;
	reg  [6 :0]  loc_resp_mcs        ;
	reg  [3 :0]  loc_resp_L_datarate ;
	reg  [11:0]  loc_resp_L_length   ;
	reg  [11:0]  loc_resp_upa_length ;
	reg  [1 :0]  loc_resp_stbc       ;  
	reg  [3 :0]  loc_lmac_rate       ;
	reg          basic_rate_en       ;
	reg          calc_frame_dur_en   ;
	reg          loc_duration_avl    ;
	reg  [15:0]  loc_duration        ;
	reg  [15:0]  loc_duration2       ;
	reg  [15:0]  loc_duration2_resp  ;
	reg          loc_duration2_avl   ;
	reg          loc_basic_rate_avl  ;
	reg          loc_data_avl        ;
	reg          loc_data_avl2       ;
	reg  [3 :0]  loc_basic_rate      ;
	wire [3 :0]  get_blrate          ;
	wire [3 :0]  loc_lmac_index      ;
	wire [15:0]  ht_duration         ;
	wire [15:0]  vht_duration        ;
	wire [15:0]  cck_duration        ;
	reg          loc_ht_mcs_rate_val ;
	reg          loc_vht_mcs_rate_val;
	reg          loc_mcs_rate_val    ;
	reg          loc_mcs_rate_val_d   ;
	reg          loc_dur_avl         ; 
	reg  [15:0]  loc_dur             ;
	reg  [3 :0]  loc_rate            ;
	reg          loc_rate_avl        ;
	reg  [15:0]  loc_duration_resp   ;
	wire [15:0]  nsyms_vht           ;
	wire [15:0]  nsyms_ht            ;
	wire [15:0]  nsyms_lrate         ;
	reg  [39:0]  loc_mcs_250kbps     ;
	reg  [39:0]  loc_rate_250kbps    ;
	reg  [39:0]  loc_ht_mcs_250kbps  ;
	reg          loc_dur_done        ;
	reg          loc_dur_done2       ;


	assign  tx_dur_out_resp_format     = loc_resp_format    ;
	assign  tx_dur_out_resp_ht_length  = loc_resp_ht_length ;
	assign  tx_dur_out_resp_mcs        = loc_resp_mcs       ;
	assign  tx_dur_out_resp_L_datarate = loc_resp_L_datarate;
	assign  tx_dur_out_resp_L_length   = loc_resp_L_length  ;
	assign  tx_dur_out_resp_upa_length = loc_resp_upa_length;
	assign  tx_dur_out_resp_stbc       = loc_resp_stbc      ;
  assign  loc_lmac_index             = lmac_rate_to_index(loc_rate);
  assign  get_blrate                 = basic_rate(loc_lmac_index);
	assign  ht_duration                = ht_duration_fn(loc_lmac_rate,tx_dur_in_txvec_stbc,tx_dur_in_txvec_num_ext_ss);
	assign  vht_duration               = vht_duration_fn(loc_lmac_rate,tx_dur_in_txvec_stbc);
	assign  nsyms_vht                  = nsyms(loc_resp_upa_length,loc_mcs_250kbps,tx_dur_in_txvec_stbc);	
	assign  nsyms_ht                   = nsyms(loc_resp_ht_length,loc_ht_mcs_250kbps,tx_dur_in_txvec_stbc);	
	assign  nsyms_lrate                = nsyms(loc_resp_L_length,loc_rate_250kbps,0);
	assign  cck_duration               = cck_dur(loc_resp_L_length,tx_dur_in_txvec_is_long_pre,loc_rate_250kbps);

	function [15:0] nsyms;
		input [11:0] mac_fr_len ;
		input [39:0] mcs_250kbps;
		input [1 :0] stbc       ; 
		reg add          ;
		reg [63:0] n_syms;

	begin
		n_syms = mcs_250kbps * (`PHY_SERVICE_NBITS + 8 * mac_fr_len + `PHY_TAIL_NBITS);
		if ((n_syms>>8) & 32'hffffffff)
			add = 1'b1;
		else
			add = 1'b0;
		n_syms = (n_syms >> `UU_ONE_BY_RATE_RESOLUTION);
		
		n_syms = n_syms+add;
		
		if (stbc!=2'h0)
			if (n_syms & 8'h01)
				n_syms = n_syms+1;
		
		nsyms = n_syms[15:0];
	end
	endfunction //END of nsyms	
	
     function [3:0] lmac_rate_to_index;
     	input [3:0] loc_lmac_rate;
     begin
		if (loc_lmac_rate & `UU_CCK_RATE_FLAG)
			if(loc_lmac_rate == `UU_HW_RATE_11M)
				lmac_rate_to_index = 4'd3;
			else if(loc_lmac_rate == `UU_HW_RATE_5M5)
				lmac_rate_to_index = 4'd2;
			else if(loc_lmac_rate == `UU_HW_RATE_2M)
				lmac_rate_to_index = 4'd1;
			else
				lmac_rate_to_index = 4'd0;
	  	else
			lmac_rate_to_index = ofdm_rate_to_index(loc_lmac_rate&`UU_OFDM_HW_MASK_INDEX);

	  	end
	endfunction //END of lmac_rate_to_index

	function [3:0] basic_rate;
		input [3:0] l_rate; 
		reg   [3:0] i;
		reg   [3:0] b_rate;
	begin
		b_rate = `UU_RATE_1M_BIT;
		if (l_rate < `UU_SUPPORTED_CCK_RATE) begin
			case (l_rate)
				4'h0:for (i=`UU_RATE_1M_BIT; i <= 0; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h1:for (i=`UU_RATE_1M_BIT; i <= 1; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h2:for (i=`UU_RATE_1M_BIT; i <= 2; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h3:for (i=`UU_RATE_1M_BIT; i <= 3; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
			endcase
		end
		else begin
			case(l_rate)
				4'h4:for (i=`UU_RATE_6M_BIT; i <= 4; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h5:for (i=`UU_RATE_6M_BIT; i <= 5; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h6:for (i=`UU_RATE_6M_BIT; i <= 6; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h7:for (i=`UU_RATE_6M_BIT; i <= 7; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h8:for (i=`UU_RATE_6M_BIT; i <= 8; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h9:for (i=`UU_RATE_6M_BIT; i <= 9; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'ha:for (i=`UU_RATE_6M_BIT; i <= 10; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'hb:for (i=`UU_RATE_6M_BIT; i <= 11; i=i+1)
						if (tx_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
			endcase
		end
		basic_rate = b_rate;		
	end
	endfunction //END of basic_rate

	function [3:0] ofdm_rate_to_index;
		input [3:0] ofdm_rate;
	begin
		case (ofdm_rate)
		4'h0:ofdm_rate_to_index = 4'd10;	
		4'h1:ofdm_rate_to_index = 4'd8;	
		4'h2:ofdm_rate_to_index = 4'd6;	
		4'h3:ofdm_rate_to_index = 4'd4;	
		4'h4:ofdm_rate_to_index = 4'd11;
		4'h5:ofdm_rate_to_index = 4'd9;
		4'h6:ofdm_rate_to_index = 4'd7;
		4'h7:ofdm_rate_to_index = 4'd5;
		endcase
	end	
	endfunction //END of ofdm_rate_to_index

	function [15:0] vht_duration_fn;
		input	[6:0]     txvec_mcs;
		input	[1:0]     txvec_stbc;
		reg	    	[2:0]    	nss;
		reg		[3:0]	n_sts,vhtltf;
	begin
		nss = (txvec_mcs & `UU_VHT_NSS_FROM_MCS) + 1;
		if (txvec_stbc != 2'b00) begin//if enabled check this
			if (nss > 4)
				vht_duration_fn = 0; //Invalid
			else begin
				n_sts = nss * 2;
				vhtltf = ((n_sts == 16'h0003) || (n_sts == 16'h0005) || (n_sts == 16'h0007)) ? (n_sts + 1) : n_sts;
				vht_duration_fn = `PHY_PREAMBLE_TIME + `PHY_SIGNAL_TIME + `PHY_VHT_SIG_A + `PHY_VHT_STF + (vhtltf * `PHY_PER_LTF) + `PHY_VHT_SIG_B;
			end
		end
		else begin
			n_sts = nss;
			vhtltf = ((n_sts == 16'h0003) || (n_sts == 16'h0005) || (n_sts == 16'h0007)) ? (n_sts + 1) : n_sts;
			vht_duration_fn = `PHY_PREAMBLE_TIME + `PHY_SIGNAL_TIME + `PHY_VHT_SIG_A + `PHY_VHT_STF + (vhtltf * `PHY_PER_LTF) + `PHY_VHT_SIG_B;
		end
     end
     endfunction //END of vht_duration_fn

	function [15:0] ht_duration_fn;
		input	[6:0]	 txvec_mcs;
		input	[1:0]	 txvec_stbc;
		input	[11:0]	 txvec_num_ext_ss;
		reg	     [15:0]	 nss,n_sts,htdltf,htltf,n_sym,hteltf;
	begin
		if(txvec_mcs < `UU_MAX_MCS_N_SUP)
			nss = (((txvec_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_NSS_FROM_MCS) + 1);
		else
			nss = 1; /* 1 spatial stream */
		
		if (txvec_stbc > nss)
			ht_duration_fn = 0;
		else if ((nss + txvec_stbc) > `UU_MAX_STREAM_HT)
			ht_duration_fn = 0;
		else begin
			n_sts = nss + txvec_stbc;
			htdltf = (n_sts == 3) ? 4 : n_sts;
			hteltf = (txvec_num_ext_ss == 3) ? 4 : txvec_num_ext_ss;
			
			htltf = htdltf + hteltf;
			if (htltf > 5)
				ht_duration_fn = 0;
			else
				ht_duration_fn = `PHY_PREAMBLE_TIME + `PHY_HT_SIG + (htltf * `PHY_PER_LTF);
		end
	end
	endfunction //END of ht_duration_fn

	function [15:0] cck_dur;
		input [11:0] mac_fr_len;
		input        is_long_pre;
		input [39:0] rate;
		reg add;
		reg [64:0] dur;
	begin
		dur = rate * (mac_fr_len * 8 * 4);
		if ((dur>>8) & 32'hffffffff)
			add = 1'b1;
		else
			add = 1'b0;
		dur = (dur >> `UU_ONE_BY_RATE_RESOLUTION);
		
		dur = dur+add;
		
		if (is_long_pre) 
			dur = dur + `PHY_PLCP_TIME;
		else
			dur = dur + `PHY_PLCP_SHORT_TIME;

		cck_dur = dur[15:0];
	end
	endfunction//END of cck_duration


	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
			loc_resp_format        <= 0;
			loc_resp_ht_length     <= 0;
			loc_lmac_rate          <= 0;
			basic_rate_en          <= 0;
			loc_resp_mcs           <= 0;
			calc_frame_dur_en      <= 0;
			loc_resp_L_datarate    <= 0;
			loc_duration           <= 0;
			loc_duration_avl       <= 0;
			loc_duration2          <= 0;
			loc_duration2_avl      <= 0;
			loc_duration2_resp     <= 0;
			tx_dur_out_dur_upd_val <= 0;
			tx_dur_out_dur_upd     <= 0;
			loc_resp_L_length      <= 0;
			loc_dur_done           <= 0;
			loc_dur_done2          <= 0;
			loc_resp_upa_length    <= 0;
		end
		else if(!tx_dur_en)begin
			loc_resp_format        <= 0;
			loc_resp_ht_length     <= 0;
			loc_lmac_rate          <= 0;
			basic_rate_en          <= 0;
			loc_resp_mcs           <= 0;
			calc_frame_dur_en      <= 0;
			loc_resp_L_datarate    <= 0;
			loc_duration           <= 0;
			loc_duration_avl       <= 0;
			loc_duration2          <= 0;
			loc_duration2_avl      <= 0;
			loc_duration2_resp     <= 0;
			tx_dur_out_dur_upd_val <= 0;
			tx_dur_out_dur_upd     <= 0;
			loc_resp_L_length      <= 0;
			loc_dur_done           <= 0;
			loc_dur_done2          <= 0;
			loc_resp_upa_length    <= 0;
		end
		else	if(tx_dur_type == 2'b00) begin /*uu_calc_bcast_duration*/ 
			tx_dur_out_dur_upd_val <= 1'b1;
			tx_dur_out_dur_upd     <= 0;
		end
		else if(tx_dur_type == 2'b01)begin /* uu_calc_singlep_frame_duration */
			if (!loc_duration_avl) begin 
				if(tx_dur_in_exp_resp_len) begin
					if(!loc_dur_avl) begin
						if (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF) begin
							//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
							loc_resp_format    <= `UU_WLAN_FRAME_FORMAT_HT_GF;
							loc_resp_ht_length <= tx_dur_in_exp_resp_len;
							loc_lmac_rate      <= tx_dur_in_txvec_mcs;
							basic_rate_en      <= 1'b1;
							if(loc_basic_rate_avl) begin
								loc_resp_mcs      <= loc_basic_rate;
								basic_rate_en     <= 1'b0;
								calc_frame_dur_en <= 1'b1;
							end
						end
						else begin
							loc_resp_L_length    <= tx_dur_in_exp_resp_len;
							if (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT)begin
								//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->L_datarate);
								if(!loc_basic_rate_avl) begin
									loc_resp_format <= tx_dur_in_txvec_format;
									loc_lmac_rate   <= tx_dur_in_txvec_L_datarate;						
									basic_rate_en   <= 1'b1;
								end
								else begin
									loc_resp_format     <= `UU_WLAN_FRAME_FORMAT_NON_HT;
									loc_resp_L_datarate <= loc_basic_rate;
									basic_rate_en       <= 1'b0;
									calc_frame_dur_en   <= 1'b1;
								end
							end
							else begin
								//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
								if(!loc_basic_rate_avl) begin
									loc_resp_format    <= tx_dur_in_txvec_format;
									loc_lmac_rate      <= tx_dur_in_txvec_mcs;						
									basic_rate_en      <= 1'b1;
								end
								else begin
									loc_resp_format     <= `UU_WLAN_FRAME_FORMAT_NON_HT;
									loc_resp_L_datarate <= loc_basic_rate;
									basic_rate_en       <= 1'b0;
									calc_frame_dur_en   <= 1'b1;
								end
							end
						end
					end
					else begin
						//uu_calc_frame_duration (&resp_vec);
						loc_duration      <= tx_dur_in_SIFS_timer_value + loc_dur;
						calc_frame_dur_en <= 1'b0;
						loc_duration_avl  <= 1'b1;
						loc_dur_done      <= 1'b1;
					end
				end //END of !ack
				else begin
					loc_duration_avl <= 1'b1;
					loc_duration     <= 0;
					loc_dur_done     <= 1'b1;
				end // END of ack
			end //END of !loc_duration_avl
			//tx_dur_in_next_frame_len = uu_wlan_get_next_frame_len(ac);
			else begin //loc_duration_avl
				if (tx_dur_in_next_frame_len!=0) begin
					if (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_VHT)
						loc_resp_upa_length <= tx_dur_in_next_frame_len;			   
					else if ((tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF) || (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_MF))
						loc_resp_ht_length <= tx_dur_in_next_frame_len;
					else
						loc_resp_L_length <= tx_dur_in_next_frame_len;

					if(loc_dur_done)begin
						loc_resp_format  <= tx_dur_in_txvec_format;
						loc_resp_L_datarate <= tx_dur_in_txvec_L_datarate;
						loc_dur_done      <= 1'b0;
					end
					else if(!loc_dur_avl)begin
						calc_frame_dur_en <= 1'b1;
					end
					else begin
						tx_dur_out_dur_upd_val <= 1'b1;
						tx_dur_out_dur_upd     <= (loc_duration << 1) + tx_dur_in_SIFS_timer_value + loc_dur; /* Double the current DUR */
						calc_frame_dur_en       <= 1'b0;
					end
				end
				else begin
					tx_dur_out_dur_upd_val <= 1'b1;
					tx_dur_out_dur_upd     <= loc_duration;
				end
			end
			//END of tx_dur_in_next_frame_len
		//END of uu_calc_singlep_frame_duration
		end
		else if(tx_dur_type==2'b10) begin /* uu_calc_rtscts_duration */
			if (!loc_duration_avl) begin
				if(!tx_dur_in_cts_self) begin
					if(!loc_dur_avl) begin
						if (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF) begin
							loc_resp_format    <= `UU_WLAN_FRAME_FORMAT_HT_GF;
							loc_resp_ht_length <= `UU_CTS_FRAME_LEN;
							if(!loc_basic_rate_avl) begin
								loc_lmac_rate      <= tx_dur_in_rts_cts_rate;
								basic_rate_en      <= 1'b1;
							end
							else begin
								loc_resp_mcs      <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
								basic_rate_en     <= 1'b0;
								calc_frame_dur_en <= 1'b1;
							end
						end
						else begin
							loc_resp_L_length    <= `UU_CTS_FRAME_LEN;
							if (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT)begin
								if(!loc_basic_rate_avl)begin
									loc_resp_format <= tx_dur_in_txvec_format;
									loc_lmac_rate   <= tx_dur_in_rts_cts_rate;						
									basic_rate_en   <= 1'b1;
								end
								else begin
									loc_resp_format     <= `UU_WLAN_FRAME_FORMAT_NON_HT;
									loc_resp_L_datarate <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->L_datarate);
									basic_rate_en       <= 1'b0;
									calc_frame_dur_en   <= 1'b1;
								end
							end
							else begin
								if(!loc_basic_rate_avl)begin
									loc_resp_format     <= tx_dur_in_txvec_format;
									loc_lmac_rate       <= tx_dur_in_rts_cts_rate;						
									basic_rate_en       <= 1'b1;
								end
								else begin
									loc_resp_format     <= `UU_WLAN_FRAME_FORMAT_NON_HT;
									loc_resp_L_datarate <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
									basic_rate_en       <= 1'b0;
									calc_frame_dur_en   <= 1'b1;
								end
							end
						end
					end
					else begin
						loc_duration      <= tx_dur_in_SIFS_timer_value + tx_dur_in_SIFS_timer_value + tx_dur_in_SIFS_timer_value +loc_dur;//uu_calc_frame_duration(&resp_vec);
						calc_frame_dur_en <= 1'b0;
						loc_duration_avl  <= 1'b1;
						loc_duration_resp <= tx_dur_in_SIFS_timer_value + tx_dur_in_SIFS_timer_value + loc_dur; //this helps in avoiding subtraction
						loc_dur_done      <= 1'b1;
					end
				end //END of !cts_self
				else begin
					loc_duration_avl  <= 1'b1;
					loc_duration      <= tx_dur_in_SIFS_timer_value + tx_dur_in_SIFS_timer_value; /* sifs after cts and after dur of pending frame */
					loc_duration_resp <= tx_dur_in_SIFS_timer_value; //this helps in avoiding subtraction					
					loc_dur_done      <= 1'b1;
				end
			end //END of !loc_duration_avl
			else begin
				if(!loc_duration2_avl)begin
					if(loc_dur_done)begin
						loc_dur_done        <= 1'b0;
						loc_resp_format     <= tx_dur_in_txvec_format;
						loc_resp_L_datarate <= tx_dur_in_txvec_L_datarate;
						loc_lmac_rate       <= tx_dur_in_txvec_mcs;
						loc_resp_mcs        <= tx_dur_in_txvec_mcs;
						loc_resp_L_length   <= tx_dur_in_txvec_L_length;
						loc_resp_ht_length  <= tx_dur_in_txvec_ht_length;
					end
					else if(!loc_dur_avl)begin
						calc_frame_dur_en   <= 1'b1;
					end
					else begin
						loc_duration2      <= loc_duration + loc_dur;//uu_calc_frame_duration(txvec);
						loc_duration2_resp <= loc_duration_resp + loc_dur;//this helps in avoiding subtraction
						calc_frame_dur_en  <= 1'b0;
						loc_duration2_avl  <= 1'b1;
						loc_dur_done2      <= 1'b1;
					end
				end // END of !loc_duration2_avl
				else begin
					if (tx_dur_in_exp_resp_len!=0)begin
						if(loc_dur_done2)
							loc_dur_done2 <= 1'b0;
						else if(!loc_dur_avl)begin
							if (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF) begin
								if(!loc_basic_rate_avl) begin
									loc_resp_format    <= `UU_WLAN_FRAME_FORMAT_HT_GF;
									loc_resp_ht_length <= tx_dur_in_exp_resp_len;
									loc_lmac_rate      <= tx_dur_in_txvec_mcs;
									basic_rate_en      <= 1'b1;
								end
								else begin
									loc_resp_mcs      <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
									basic_rate_en     <= 1'b0;
									calc_frame_dur_en <= 1'b1;
								end
							end
							else begin
								loc_resp_L_length    <= tx_dur_in_exp_resp_len;
								if (tx_dur_in_txvec_format  == `UU_WLAN_FRAME_FORMAT_NON_HT)begin
									if(!loc_basic_rate_avl) begin
										loc_resp_format <= tx_dur_in_txvec_format;
										loc_lmac_rate   <= loc_resp_L_datarate;					
										basic_rate_en   <= 1'b1;
									end
									else begin
										loc_resp_L_datarate <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->L_datarate);
										loc_resp_format     <= `UU_WLAN_FRAME_FORMAT_NON_HT;
										basic_rate_en       <= 1'b0;
										calc_frame_dur_en   <= 1'b1;
									end
								end
								else begin
									if(!loc_basic_rate_avl) begin
										loc_resp_format     <= tx_dur_in_txvec_format;
										loc_lmac_rate       <= tx_dur_in_txvec_mcs;					
										basic_rate_en       <= 1'b1;
									end
									else begin
										loc_resp_L_datarate <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
										loc_resp_format     <= `UU_WLAN_FRAME_FORMAT_NON_HT;
										basic_rate_en       <= 1'b0;
										calc_frame_dur_en   <= 1'b1;
									end
								end
							end
						end
						else begin
							calc_frame_dur_en      <= 1'b0;
							loc_duration_avl       <= 1'b1;
							tx_dur_out_dur_upd     <= loc_duration2 + loc_dur;
							tx_dur_out_dur_upd_val <= 1'b1;
						end
					end
					else begin
						tx_dur_out_dur_upd_val <= 1'b1;
						tx_dur_out_dur_upd     <= loc_duration2_resp; //subtraction of tx_dur_in_SIFS_timer_value here is avoided using "*_resp"
					end
				end //END of loc_duration2_avl
			end //END of loc_duration_avl
		end
		else begin /*uu_calc_impl_BA_duration*/
			if(!loc_duration_avl)begin
				if (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF) begin
					loc_resp_format    <= `UU_WLAN_FRAME_FORMAT_HT_GF;
					loc_resp_ht_length <= tx_dur_in_exp_resp_len;
					if(loc_basic_rate_avl)begin
						loc_lmac_rate     <= tx_dur_in_txvec_mcs;						
						basic_rate_en     <= 1'b1;
					end
					else begin
						loc_resp_mcs      <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
						basic_rate_en     <= 1'b0;
						calc_frame_dur_en <= 1'b1;
					end
				end
				else begin
					loc_resp_L_length    <= tx_dur_in_exp_resp_len;
					if (tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT)begin
						if(!loc_basic_rate_avl) begin
							loc_resp_format <= tx_dur_in_txvec_format;
							loc_lmac_rate   <= tx_dur_in_txvec_L_datarate;						
							basic_rate_en   <= 1'b1;
						end
						else begin
							loc_resp_L_datarate <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->L_datarate);
							basic_rate_en       <= 1'b0;
							calc_frame_dur_en   <= 1'b1;
						end
					end
					else begin
						if(!loc_basic_rate_avl)begin
							loc_resp_format  <= tx_dur_in_txvec_format;
							loc_lmac_rate    <= tx_dur_in_txvec_mcs;						
							basic_rate_en    <= 1'b1;
						end
						else begin
							loc_resp_L_datarate <= loc_basic_rate;//uu_assign_basic_rate(txvec->modulation, txvec->format, txvec->mcs);
							basic_rate_en       <= 1'b0;
							calc_frame_dur_en   <= 1'b1;
						end
					end
				end
				if(loc_dur_avl) begin
					loc_duration      <= tx_dur_in_SIFS_timer_value +loc_dur;//uu_calc_frame_duration(&resp_vec);
					calc_frame_dur_en <= 1'b0;
					loc_duration_avl  <= 1'b1;
				end
			end
			else begin
				tx_dur_out_dur_upd_val <= 1'b1;
				tx_dur_out_dur_upd     <= loc_duration;
			end
		end
	end
		
	// always block for assign_basic_rate and uu_calc_frame_duration calculation 
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET) begin
			loc_basic_rate          <= 0;
			loc_basic_rate_avl      <= 0;
			loc_rate                <= 0;
			loc_rate_avl            <= 0;
			loc_data_avl            <= 0;
			loc_data_avl2           <= 0;
			loc_vht_mcs_rate_val    <= 0;
			loc_ht_mcs_rate_val     <= 0;
			loc_mcs_rate_val        <= 0;
			tx_dur_out_vht_mcs_en   <= 0;
		//	tx_dur_out_vht_mcs_wen  <= 0;
			tx_dur_out_vht_mcs_addr <= 0;
			tx_dur_out_ht_mcs_en    <= 0;
		//	tx_dur_out_ht_mcs_wen   <= 0;
			tx_dur_out_ht_mcs_addr  <= 0;
			tx_dur_out_lrate_en     <= 0;
		//	tx_dur_out_lrate_wen    <= 0;
			tx_dur_out_lrate_addr   <= 0;		
			loc_mcs_250kbps         <= 0;
			loc_ht_mcs_250kbps      <= 0;
			loc_rate_250kbps        <= 0;
		end
		else if(!tx_dur_en) begin
			loc_basic_rate          <= 0;
			loc_data_avl            <= 0;
			loc_rate                <= 0;
			loc_rate_avl            <= 0;
			loc_data_avl2           <= 0;
			loc_basic_rate_avl      <= 0;			
			loc_vht_mcs_rate_val    <= 0;
			loc_ht_mcs_rate_val     <= 0;
			loc_mcs_rate_val        <= 0;
			tx_dur_out_vht_mcs_en   <= 0;
			//tx_dur_out_vht_mcs_wen  <= 0;
			tx_dur_out_vht_mcs_addr <= 0;	
			tx_dur_out_ht_mcs_en    <= 0;
		//	tx_dur_out_ht_mcs_wen   <= 0;
			tx_dur_out_ht_mcs_addr  <= 0;
			tx_dur_out_lrate_en     <= 0;
		//	tx_dur_out_lrate_wen    <= 0;
			tx_dur_out_lrate_addr   <= 0;	
			loc_mcs_250kbps         <= 0;
			loc_ht_mcs_250kbps      <= 0;
			loc_rate_250kbps        <= 0;
		end
		else if(((!basic_rate_en)&&(!calc_frame_dur_en))||((basic_rate_en)&&(calc_frame_dur_en))) begin
			loc_basic_rate          <= 0;
			loc_basic_rate_avl      <= 0;
			loc_rate                <= 0;
			loc_rate_avl            <= 0;
			loc_data_avl            <= 0;
			loc_data_avl2           <= 0;
			loc_vht_mcs_rate_val    <= 0;
			loc_ht_mcs_rate_val     <= 0;
			loc_mcs_rate_val        <= 0;
			tx_dur_out_vht_mcs_en   <= 0;
		//	tx_dur_out_vht_mcs_wen  <= 0;
			tx_dur_out_vht_mcs_addr <= 0;
			tx_dur_out_ht_mcs_en    <= 0;
		//	tx_dur_out_ht_mcs_wen   <= 0;
			tx_dur_out_ht_mcs_addr  <= 0;
			tx_dur_out_lrate_en     <= 0;
		//	tx_dur_out_lrate_wen    <= 0;
			tx_dur_out_lrate_addr   <= 0;	
			loc_mcs_250kbps         <= 0;
		end
		else if ((basic_rate_en)&&(!calc_frame_dur_en))begin
			if(!loc_rate_avl) begin
				if(loc_resp_format == `UU_WLAN_FRAME_FORMAT_VHT) begin
					//from vht_mcs_table_g <memory>
					tx_dur_out_vht_mcs_en <= 1'b1;
		//			tx_dur_out_vht_mcs_wen <= 1'b0;
					tx_dur_out_vht_mcs_addr <= (((loc_lmac_rate >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS)*`VHT_INDEX)+`VHT_OFDM;
					loc_data_avl <= 1'b1;
					if(loc_data_avl) begin
						loc_rate_avl <= 1'b1;
						loc_rate     <= tx_dur_in_vht_mcs_data;
						loc_data_avl <= 1'b0;
					end
				end
				else if (loc_resp_format	 == `UU_WLAN_FRAME_FORMAT_HT_MF)begin
					//from ht_mcs_table_g <memory>
					tx_dur_out_ht_mcs_en <= 1'b1;
		//			tx_dur_out_ht_mcs_wen <= 1'b0;
					tx_dur_out_ht_mcs_addr <= ((loc_lmac_rate & `UU_HT_MCS_MASK)*`HT_INDEX) +`HT_OFDM;
					loc_data_avl <= 1'b1;
					if(loc_data_avl) begin
						loc_rate_avl <= 1'b1;				
						loc_rate     <= tx_dur_in_ht_mcs_data;
						loc_data_avl <= 1'b0;
					end			
				end
				else if(tx_dur_in_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF) begin
					loc_basic_rate <= (loc_lmac_rate & `UU_VHT_NSS_FROM_MCS);
					loc_basic_rate_avl <= 1'b1;			
				end
				else begin/* tx_dur_in_txvec_format == UU_WLAN_FRAME_FORMAT_NON_HT */ 
					loc_rate_avl <= 1'b1;
					loc_rate     <= loc_lmac_rate;
				end
			end
			else begin
				if (get_blrate!=0) begin
					//from lrate_table_g <memory>
					tx_dur_out_lrate_en <= 1'b1;
		//			tx_dur_out_lrate_wen <= 1'b0;
					tx_dur_out_lrate_addr <= (get_blrate*`NON_HT_INDEX)+`NON_HT_HW;
					loc_data_avl <= 1'b1;
					if(loc_data_avl) begin
						loc_basic_rate <= tx_dur_in_lrate_data;
						loc_basic_rate_avl <= 1'b1;
						loc_data_avl <= 1'b0;
					end					
				end
				else begin
					loc_basic_rate_avl <= 1'b1;			
					if ((tx_dur_in_txvec_modulation == `ERP_DSSS) && (loc_lmac_index <= `UU_RATE_2M_BIT))
						if (loc_rate == `UU_HW_RATE_1M)
							loc_basic_rate <= `UU_HW_RATE_1M; 
						else
							loc_basic_rate <= `UU_HW_RATE_2M;
					else if ((tx_dur_in_txvec_modulation == `ERP_CCK || tx_dur_in_txvec_modulation == `ERP_PBCC) && (loc_lmac_index < `UU_SUPPORTED_CCK_RATE))
						if ((loc_rate == `UU_HW_RATE_1M) || (loc_rate == `UU_HW_RATE_2M) || (loc_rate == `UU_HW_RATE_5M5))
							loc_basic_rate <= `UU_HW_RATE_5M5;
						else
							loc_basic_rate <= `UU_HW_RATE_11M;
					else begin/* for ofdm case, cck case */
						if (!(loc_rate & `UU_CCK_RATE_FLAG))
							if(((loc_rate & `UU_OFDM_MODULATION_TYPE_IND) == `UU_OFDM_BPSK) && (loc_lmac_index <= `UU_RATE_9M_BIT))
								loc_basic_rate <= `UU_HW_RATE_6M;
							else if(((loc_rate & `UU_OFDM_MODULATION_TYPE_IND) == `UU_OFDM_QPSK) && (loc_lmac_index <= `UU_RATE_18M_BIT))
								loc_basic_rate <= `UU_HW_RATE_12M;
							else 
								loc_basic_rate <= `UU_HW_RATE_24M;
						else if (loc_lmac_index < `UU_SUPPORTED_CCK_RATE )
							loc_basic_rate <= loc_rate;
						else
							loc_basic_rate <= `UU_HW_RATE_1M;
					end
				end
			end
		end
		else if((!basic_rate_en)&&(calc_frame_dur_en))begin

			//tx_dur_in_txvec_ch_bndwdth & tx_dur_in_txvec_is_short_GI & loc_res_mcs
			//--------------------uu_mcsac_to_rate------------------------------------
			
			if(loc_resp_format == `UU_WLAN_FRAME_FORMAT_VHT) begin
				tx_dur_out_vht_mcs_en <= 1'b1;
				if (tx_dur_in_txvec_ch_bndwdth == `CBW160 || tx_dur_in_txvec_ch_bndwdth == `CBW80_80)begin			
					if(tx_dur_in_txvec_is_short_GI) begin //mcs_ac_rate_160_sgi
						tx_dur_out_vht_mcs_addr <= ((((loc_resp_mcs & `UU_VHT_NSS_FROM_MCS) * `UU_VHT_MCS_INDEX_MAXIMUM) + 
						                             ((loc_resp_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS))*`VHT_INDEX)+`VHT_160_SGI;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin

							loc_mcs_250kbps    <= tx_dur_in_vht_mcs_data;
							loc_vht_mcs_rate_val   <= 1'b1;
						end
					end
					else begin//mcs_ac_rate_160
						tx_dur_out_vht_mcs_addr <= ((((loc_resp_mcs & `UU_VHT_NSS_FROM_MCS) * `UU_VHT_MCS_INDEX_MAXIMUM) + 
						                             ((loc_resp_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS))*`VHT_INDEX)+`VHT_160;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						if (loc_data_avl2) begin

							loc_mcs_250kbps    <= tx_dur_in_vht_mcs_data;
							loc_vht_mcs_rate_val   <= 1'b1;
						end
					end
				end
				else if (tx_dur_in_txvec_ch_bndwdth == `CBW80)begin
					if(tx_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_80_sgi
						tx_dur_out_vht_mcs_addr <= ((((loc_resp_mcs & `UU_VHT_NSS_FROM_MCS) * `UU_VHT_MCS_INDEX_MAXIMUM) + 
						                             ((loc_resp_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS))*`VHT_INDEX)+`VHT_80_SGI;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin
							loc_mcs_250kbps    <= tx_dur_in_vht_mcs_data;
							loc_vht_mcs_rate_val   <=1'b1;
						end
					end
					else begin//mcs_ac_rate_80
						tx_dur_out_vht_mcs_addr <= ((((loc_resp_mcs & `UU_VHT_NSS_FROM_MCS) * `UU_VHT_MCS_INDEX_MAXIMUM) + 
						                             ((loc_resp_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS))*`VHT_INDEX)+`VHT_80;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin
							loc_mcs_250kbps    <= tx_dur_in_vht_mcs_data;
							loc_vht_mcs_rate_val   <= 1'b1;
						end
					end
				end
				else if (tx_dur_in_txvec_ch_bndwdth == `CBW40)begin
					if(tx_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_40_sgi
						tx_dur_out_vht_mcs_addr <= ((((loc_resp_mcs & `UU_VHT_NSS_FROM_MCS) * `UU_VHT_MCS_INDEX_MAXIMUM) + 
						                             ((loc_resp_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS))*`VHT_INDEX)+`VHT_40_SGI;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin

							loc_mcs_250kbps    <= tx_dur_in_vht_mcs_data;
							loc_vht_mcs_rate_val   <= 1'b1;
						end
					end
					else begin//mcs_ac_rate_40
						tx_dur_out_vht_mcs_addr <= ((((loc_resp_mcs & `UU_VHT_NSS_FROM_MCS) * `UU_VHT_MCS_INDEX_MAXIMUM) + 
						                             ((loc_resp_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS))*`VHT_INDEX)+`VHT_40;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin
							loc_mcs_250kbps    <= tx_dur_in_vht_mcs_data;
							loc_vht_mcs_rate_val   <= 1'b1;
						end
					end
				end
				else begin
					if(tx_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_20_sgi
						tx_dur_out_vht_mcs_addr <= ((((loc_resp_mcs & `UU_VHT_NSS_FROM_MCS) * `UU_VHT_MCS_INDEX_MAXIMUM) + 
						                             ((loc_resp_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS))*`VHT_INDEX)+`VHT_20_SGI;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin
							loc_mcs_250kbps    <= tx_dur_in_vht_mcs_data;
							loc_vht_mcs_rate_val   <= 1'b1;
						end
					end
					else begin//mcs_ac_rate_20
						tx_dur_out_vht_mcs_addr <= ((((loc_resp_mcs & `UU_VHT_NSS_FROM_MCS) * `UU_VHT_MCS_INDEX_MAXIMUM) + 
												((loc_resp_mcs >> `UU_VHT_MCSINDEX_SHIFT_FROM_MCS) & `UU_VHT_MCSINDEX_FROM_MCS))*`VHT_INDEX)+`VHT_20;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin
							loc_mcs_250kbps    <= tx_dur_in_vht_mcs_data;
							loc_vht_mcs_rate_val   <= 1'b1;
						end
					end
				end
			end //END of VHT
			if((loc_resp_format == `UU_WLAN_FRAME_FORMAT_HT_MF)||(loc_resp_format == `UU_WLAN_FRAME_FORMAT_HT_GF)) begin
				tx_dur_out_ht_mcs_en <= 1'b1;
				if (tx_dur_in_txvec_ch_bndwdth == `CBW40)begin
					if(tx_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_40_sgi
						tx_dur_out_ht_mcs_addr <= (loc_resp_mcs * `HT_INDEX)+`HT_40_SGI;
					
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin							
							loc_ht_mcs_250kbps <= tx_dur_in_ht_mcs_data;
							loc_ht_mcs_rate_val<=1'b1;
						end
					end
					else begin//mcs_ac_rate_40
						tx_dur_out_ht_mcs_addr <= (loc_resp_mcs * `HT_INDEX)+`HT_40;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin	
							loc_ht_mcs_250kbps <= tx_dur_in_ht_mcs_data;
							loc_ht_mcs_rate_val<=1'b1;
						end
					end
				end
				else begin
					if(tx_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_20_sgi
						tx_dur_out_ht_mcs_addr <= (loc_resp_mcs * `HT_INDEX)+`HT_20_SGI;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin	
							loc_ht_mcs_250kbps <= tx_dur_in_ht_mcs_data;
							loc_ht_mcs_rate_val<=1'b1;
						end
					end
					else begin//mcs_ac_rate_20
						tx_dur_out_ht_mcs_addr <= (loc_resp_mcs * `HT_INDEX)+`HT_20;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
						if (loc_data_avl2) begin	
							loc_ht_mcs_250kbps <= tx_dur_in_ht_mcs_data;
							loc_ht_mcs_rate_val<=1'b1;
						end
					end
				end	
			end //END of HT_MF | HT_GF
			else begin //Non-HT
				tx_dur_out_lrate_en <= 1'b1;
				if (loc_resp_L_datarate & `UU_CCK_RATE_FLAG) begin
					tx_dur_out_lrate_addr <=  (lmac_rate_to_index(loc_resp_L_datarate) * `NON_HT_INDEX)+`NON_HT_250;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;	
						
					if (loc_data_avl2) begin
						loc_rate_250kbps <= tx_dur_in_lrate_data;
						loc_mcs_rate_val <= 1'b1;
					end
				end
				else begin
					tx_dur_out_lrate_addr <=  (ofdm_rate_to_index(loc_resp_L_datarate & `UU_OFDM_HW_MASK_INDEX) * `NON_HT_INDEX)+`NON_HT_250;
					if(!loc_dur_avl)
						loc_data_avl2 <= 1'b1;
					else 
						loc_data_avl2 <= 1'b0;			
					
					if (loc_data_avl2) begin
						loc_rate_250kbps <= tx_dur_in_lrate_data;
						loc_mcs_rate_val <= 1'b1;
					end
				end
			end//END of Non-HT
		end
	end
	/*------------------------duration_out----------------------*/
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_dur_avl <= 0;
			loc_dur     <= 0; 
		end
		else if(!tx_dur_en)begin
			loc_dur_avl <= 0;
			loc_dur     <= 0;
		end
		else if(!calc_frame_dur_en) begin
			loc_dur_avl <= 0;
			loc_dur     <= 0;
		end 
		else	begin
			if(loc_vht_mcs_rate_val)begin
				loc_dur_avl <= 1'b1;
				loc_dur     <= vht_duration + (`PHY_SYMBOL_TIME * nsyms_vht);
			end
			else if(loc_ht_mcs_rate_val)begin
				if(loc_resp_format == `UU_WLAN_FRAME_FORMAT_HT_MF) begin
					loc_dur_avl <= 1'b1;
					loc_dur     <= ht_duration + (`PHY_SYMBOL_TIME * nsyms_ht) + `DOT11_SIGNAL_EXTENSION + `PHY_SIGNAL_TIME + `PHY_HT_STF;
				end
				else begin
					loc_dur_avl <= 1'b1;
					loc_dur     <= ht_duration + (`PHY_SYMBOL_TIME * nsyms_ht) + `DOT11_SIGNAL_EXTENSION;
				end	
			end
								else if(loc_mcs_rate_val_d)begin
				if(loc_resp_L_datarate & `UU_CCK_RATE_FLAG) begin
					loc_dur <= cck_duration;
					loc_dur_avl     <= 1'b1;
				end
				else begin
					loc_dur <= `PHY_PREAMBLE_TIME + `PHY_SIGNAL_TIME + (`PHY_SYMBOL_TIME * nsyms_lrate) + `DOT11_SIGNAL_EXTENSION;
//																			loc_dur <= `PHY_PREAMBLE_TIME + `PHY_SIGNAL_TIME + (`PHY_SYMBOL_TIME ) + `DOT11_SIGNAL_EXTENSION;
					loc_dur_avl <= 1'b1;
				end
			end
			else begin
				loc_dur     <= 0;
				loc_dur_avl <= 0;
			end
		end
	end//always
	
		always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_mcs_rate_val_d <= 0;
		end
		else if(!tx_dur_en)begin
			loc_mcs_rate_val_d <= 0;
		end
		else begin
			loc_mcs_rate_val_d <= loc_mcs_rate_val;
		end
	end

endmodule

