`include "../../inc/defines.h" 

module uu_acmac_cp_duration(
     input                clk                        ,
     input                rst_n                      ,
     input                cp_dur_en                  ,
     //inputs and ouputs for cp_if module
     input        [15:0]  cp_dur_in_mpdu_dur         ,	  
     input        [7 :0]  cp_dur_in_mpdu_fc          ,
     input        [1 :0]  cp_dur_in_rxvec_format     ,
     input        [2 :0]  cp_dur_in_rxvec_modulation ,
     input        [3 :0]  cp_dur_in_rxvec_L_datarate ,
     input        [6 :0]  cp_dur_in_rxvec_mcs        ,
	input        [7 :0]  cp_dur_in_resp_length      ,
     output  reg  [15:0]  cp_dur_out_dur_updated     ,
     output  reg          cp_dur_out_dur_avl         ,
     // global memory txvec_g inputs and outputs 
     output       [1 :0]  cp_dur_out_txvec_format    ,
	output       [3 :0]  cp_dur_out_txvec_L_datarate,
	output       [11:0]  cp_dur_out_txvec_L_length  ,
	output       [15:0]  cp_dur_out_txvec_ht_length ,
	output       [6 :0]  cp_dur_out_txvec_mcs       ,
	output       [1 :0]  cp_dur_out_txvec_stbc      ,
	input        [1 :0]  cp_dur_in_txvec_num_ext_ss ,
     input                cp_dur_in_txvec_is_long_pre,
	input                cp_dur_in_txvec_is_short_GI,
     input        [2 :0]  cp_dur_in_txvec_ch_bndwdth ,
	//interface with memory vht_mcs_table_g     
	output  reg          cp_dur_out_vht_mcs_en      ,
	output  reg          cp_dur_out_vht_mcs_wen     ,
	output  reg  [9 :0]  cp_dur_out_vht_mcs_addr    ,	
	input        [39:0]  cp_dur_in_vht_mcs_data     ,
	//interface with memory ht_mcs_table_g
	output  reg          cp_dur_out_ht_mcs_en       ,
	output  reg          cp_dur_out_ht_mcs_wen      ,
	output  reg  [7 :0]  cp_dur_out_ht_mcs_addr     , 
	input        [39:0]  cp_dur_in_ht_mcs_data      ,
	//interface with memory lrate_table_g
	output  reg          cp_dur_out_lrate_en        ,
	output  reg          cp_dur_out_lrate_wen       ,
	output  reg  [5 :0]  cp_dur_out_lrate_addr      , 
	input        [39:0]  cp_dur_in_lrate_data       ,
	input        [15:0]  cp_dur_in_BSSBasicRateSet  ,
	input        [15:0]  cp_dur_in_SIFS_timer_value 
     );

	/* Registers internal to the module
	....................................................*/
	reg          loc_lrate_data_val   ;
	reg          loc_ht_data_val      ;
	reg          loc_vht_data_val     ;
	reg          loc_ht_rate_val      ;
	reg          loc_non_ht_rate_val  ;
	reg          loc_dur_avl          ; 
     reg          loc_txvec_updated1   ; 
     reg          loc_txvec_updated2   ; 
	wire [15:0]  nsyms_ht             ;
	wire [15:0]  nsyms_lrate          ;
	reg  [15:0]  loc_dur_updated      ;
	reg  [39:0]  loc_rate_250kbps     ;
	reg  [39:0]  loc_ht_mcs_250kbps   ;
	reg  [39:0]  loc_mcs_250kbps      ;
     reg  [19:0]  length               ;
	reg  [1 :0]  loc_txvec_format     ;
     reg  [3 :0]  loc_txvec_L_datarate ;
     reg  [11:0]  loc_txvec_L_length   ;
     reg  [15:0]  loc_txvec_ht_length  ;
     reg  [1 :0]  loc_txvec_stbc       ;
     reg  [6 :0]  loc_txvec_mcs        ;
	reg  [3 :0]  loc_lmac_rate        ;
	wire [15:0]  ht_duration          ;
	wire [15:0]  cck_duration         ;
	wire [3 :0]  l_index              ;
	wire [3 :0]  get_blrate           ;
	wire signed [15:0] loc_duration   ;
	reg  [63:0]  lrate_dur            ;
	reg  [63:0]  n_syms_lrate         ;
	reg  [19:0]  n_syms_ht_1          ;
	reg  [20:0]  n_syms_ht_2          ;
	reg  [63:0]  n_syms_ht_3          ;
	reg  [63:0]  n_syms_ht_4          ;
	reg  [63:0]  n_syms_ht_5          ;
	reg  [2 :0]  loc_ht_val           ;
	reg  [2 :0]  loc_non_ht_val       ;
	/*-----------------------------------------------*/

	/* Continuous assignments 
	.................................................................................................*/
     assign cp_dur_out_txvec_format     = loc_txvec_format     ;
	assign cp_dur_out_txvec_L_datarate = loc_txvec_L_datarate ;
	assign cp_dur_out_txvec_L_length   = loc_txvec_L_length   ;
	assign cp_dur_out_txvec_ht_length  = loc_txvec_ht_length  ;
	assign cp_dur_out_txvec_mcs        = loc_txvec_mcs        ;
	assign cp_dur_out_txvec_stbc       = loc_txvec_stbc       ;
     assign l_index      = lmac_rate_to_index(loc_lmac_rate);
     assign get_blrate   = basic_rate(l_index);
	assign nsyms_ht     = nsyms(n_syms_ht_3,loc_txvec_stbc);	
	assign nsyms_lrate  = nsyms(n_syms_lrate,loc_txvec_stbc);
	assign cck_duration = cck_dur(lrate_dur,cp_dur_in_txvec_is_long_pre);
   	assign ht_duration  = ht_duration_fn(loc_txvec_mcs,loc_txvec_stbc,cp_dur_in_txvec_num_ext_ss);
	assign loc_duration = cp_dur_in_mpdu_dur - loc_dur_updated - cp_dur_in_SIFS_timer_value;
	/*-----------------------------------------------------------------------------------------------*/
	
	/* Functions 
	.................................................................................................*/	
	//ofdm rate to index maping
	function [3:0] ofdm_rate_to_index;
		input [3:0] ofdm_rate;
	begin
		case (ofdm_rate)
		4'h0:ofdm_rate_to_index = 4'd10;	
		4'h1:ofdm_rate_to_index = 4'd8 ;	
		4'h2:ofdm_rate_to_index = 4'd6 ;	
		4'h3:ofdm_rate_to_index = 4'd4 ;	
		4'h4:ofdm_rate_to_index = 4'd11;
		4'h5:ofdm_rate_to_index = 4'd9 ;
		4'h6:ofdm_rate_to_index = 4'd7 ;
		4'h7:ofdm_rate_to_index = 4'd5 ;
		default:ofdm_rate_to_index = 0;
		endcase
	end	
	endfunction //END of ofdm_rate_to_index

	//lmac rate to index calculation
     function [3:0] lmac_rate_to_index;
     	input [3:0] lmac_rate;
     begin
		if (lmac_rate & `UU_CCK_RATE_FLAG)
			if(lmac_rate == `UU_HW_RATE_11M)
				lmac_rate_to_index = 4'd3;
			else if(lmac_rate == `UU_HW_RATE_5M5)
				lmac_rate_to_index = 4'd2;
			else if(lmac_rate == `UU_HW_RATE_2M)
				lmac_rate_to_index = 4'd1;
			else
				lmac_rate_to_index = 4'd0;
	  	else
			lmac_rate_to_index = ofdm_rate_to_index(lmac_rate&`UU_OFDM_HW_MASK_INDEX);

	  	end
	endfunction //END of lmac_rate_to_index

	//basic rate assignment function
	function [3:0] basic_rate;
		input [3:0] l_rate; 
		reg   [3:0] i;
		reg   [3:0] b_rate;
	begin
		b_rate = `UU_RATE_1M_BIT;
		if (l_rate < `UU_SUPPORTED_CCK_RATE) begin
			case (l_rate)
				4'h0:for (i=`UU_RATE_1M_BIT; i <= 0; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h1:for (i=`UU_RATE_1M_BIT; i <= 1; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h2:for (i=`UU_RATE_1M_BIT; i <= 2; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h3:for (i=`UU_RATE_1M_BIT; i <= 3; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
			endcase
		end
		else begin
			case(l_rate)
				4'h4:for (i=`UU_RATE_6M_BIT; i <= 4; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h5:for (i=`UU_RATE_6M_BIT; i <= 5; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h6:for (i=`UU_RATE_6M_BIT; i <= 6; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h7:for (i=`UU_RATE_6M_BIT; i <= 7; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h8:for (i=`UU_RATE_6M_BIT; i <= 8; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'h9:for (i=`UU_RATE_6M_BIT; i <= 9; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'ha:for (i=`UU_RATE_6M_BIT; i <= 10; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
				4'hb:for (i=`UU_RATE_6M_BIT; i <= 11; i=i+1)
						if (cp_dur_in_BSSBasicRateSet & (1<<i))
							b_rate = i;
						else
							b_rate = b_rate;
			endcase
		end
		basic_rate = b_rate;
	end
	endfunction //END of basic_rate
	
	//ht_duration calculation function
	function [15:0] ht_duration_fn;
		input  [6 :0]  txvec_mcs         ;
		input  [1 :0]  txvec_stbc        ;
		input  [11:0]  txvec_num_ext_ss  ;
		reg    [15:0]  nss,n_sts,htdltf  ;
		reg    [15:0]  htltf,n_sym,hteltf;
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

	//number of symbols
	function [15:0] nsyms;
		input [63:0] n_syms;
		input [1 :0] stbc  ; 
		reg          add   ;
	begin
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
	
	//cck duration calculation
	function [15:0] cck_dur;
		input [63:0] dur        ;
		input        is_long_pre;
		reg          add        ;
	begin
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
	endfunction //END of cck_duration
	/*-----------------------------------------------------------------------------------------------*/


	/* Concurrent assignments 
	...........................................................*/
	//always block for updation of txvec and getting rate
	always@(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
			loc_txvec_format        <= 0;
			loc_txvec_L_length      <= 0;
			loc_txvec_L_datarate    <= 0;
			loc_txvec_ht_length     <= 0;
			loc_txvec_updated1      <= 0;
			loc_txvec_updated2      <= 0;
			loc_txvec_stbc          <= 0;
			loc_txvec_format        <= 0;
			cp_dur_out_vht_mcs_en   <= 0;
			cp_dur_out_vht_mcs_wen  <= 0;
			cp_dur_out_vht_mcs_addr <= 0;
			loc_vht_data_val        <= 0;
			loc_mcs_250kbps         <= 0;
			cp_dur_out_ht_mcs_en    <= 0;
			cp_dur_out_ht_mcs_wen   <= 0;
			cp_dur_out_ht_mcs_addr  <= 0;
			loc_ht_data_val         <= 0;
			loc_ht_mcs_250kbps      <= 0;
			loc_ht_rate_val         <= 0;
			cp_dur_out_lrate_en     <= 0;
			cp_dur_out_lrate_wen    <= 0;
			cp_dur_out_lrate_addr   <= 0;
			loc_lrate_data_val      <= 0;
			loc_rate_250kbps        <= 0;
			loc_non_ht_rate_val     <= 0;
			loc_txvec_mcs           <= 0;
		end
		else if (!cp_dur_en) begin
			//loc_txvec_format        <= 0;
			//loc_txvec_L_length      <= 0;
			//loc_txvec_L_datarate    <= 0;
			//loc_txvec_ht_length     <= 0;
			loc_txvec_updated1      <= 0;
			loc_txvec_updated2      <= 0;
			//loc_txvec_stbc          <= 0;
			//loc_txvec_format        <= 0;
			cp_dur_out_vht_mcs_en   <= 0;
			cp_dur_out_vht_mcs_wen  <= 0;
			cp_dur_out_vht_mcs_addr <= 0;
			loc_vht_data_val        <= 0;
			loc_mcs_250kbps         <= 0;
			cp_dur_out_ht_mcs_en    <= 0;
			cp_dur_out_ht_mcs_wen   <= 0;
			cp_dur_out_ht_mcs_addr  <= 0;
			loc_ht_data_val         <= 0;
			loc_ht_mcs_250kbps      <= 0;
			loc_ht_rate_val         <= 0;
			cp_dur_out_lrate_en     <= 0;
			cp_dur_out_lrate_wen    <= 0;
			cp_dur_out_lrate_addr   <= 0;
			loc_lrate_data_val      <= 0;
			loc_rate_250kbps        <= 0;
			loc_non_ht_rate_val     <= 0;			
			//loc_txvec_mcs           <= 0;
		end
		else if ((cp_dur_in_mpdu_fc & `IEEE80211_FC0_TYPE_MASK) == `IEEE80211_FC0_TYPE_CTL) begin
			//***Changes expected in future
			if(!loc_txvec_updated1)begin
				if(!loc_txvec_updated2)begin
					if (cp_dur_in_rxvec_format == `UU_WLAN_FRAME_FORMAT_VHT)begin
						loc_txvec_format        <= `UU_WLAN_FRAME_FORMAT_NON_HT;
						cp_dur_out_vht_mcs_en   <= 1'b1;
						loc_vht_data_val        <= 1'b1;
						loc_txvec_L_length      <= 0;
						cp_dur_out_vht_mcs_addr <= (((cp_dur_in_rxvec_mcs>>3)&8'h0f)*`VHT_INDEX)+`VHT_OFDM;
						if (loc_vht_data_val) begin
							loc_lmac_rate      <= cp_dur_in_vht_mcs_data[3:0];
							loc_vht_data_val   <= 1'b0;
							loc_txvec_updated2 <= 1'b1;
						end
					end
					else	if (cp_dur_in_rxvec_format == `UU_WLAN_FRAME_FORMAT_HT_MF)begin
						loc_txvec_format       <= `UU_WLAN_FRAME_FORMAT_NON_HT;
						cp_dur_out_ht_mcs_en   <= 1'b1;
						loc_ht_data_val        <= 1'b1;
						loc_txvec_L_length     <= 0;
						cp_dur_out_ht_mcs_addr <= ((cp_dur_in_rxvec_mcs & `UU_HT_MCS_MASK)*`HT_INDEX)+`HT_OFDM;
						if (loc_ht_data_val) begin
							loc_lmac_rate      <= cp_dur_in_ht_mcs_data[3:0];
							loc_ht_data_val    <= 1'b0;
							loc_txvec_updated2 <= 1'b1;
						end
					end
					else	if (cp_dur_in_rxvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF)begin
						loc_txvec_format     <= `UU_WLAN_FRAME_FORMAT_HT_GF;
						loc_txvec_mcs        <= cp_dur_in_rxvec_mcs % 8;
						loc_txvec_ht_length  <= cp_dur_in_resp_length;
						loc_txvec_updated2   <= 1'b1;
					end
					else begin
						loc_txvec_format   <= `UU_WLAN_FRAME_FORMAT_NON_HT;
						loc_txvec_L_length <= 'h0;
						loc_lmac_rate      <= cp_dur_in_rxvec_L_datarate;
						loc_txvec_updated2   <= 1'b1;
					end
				end
				else begin//loc_txvec_updated2
					if(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT)begin
						if (get_blrate!=0) begin
							cp_dur_out_lrate_en   <= 1'b1;
							loc_lrate_data_val    <= 1'b1;
							cp_dur_out_lrate_addr <= (get_blrate*`NON_HT_INDEX)+`NON_HT_HW;
							if (loc_lrate_data_val) begin
								loc_txvec_L_datarate <= cp_dur_in_lrate_data;
								loc_lrate_data_val   <= 1'b0;
								loc_txvec_updated1   <= 1'b1;
							end
						end	   
						else begin
							if ((cp_dur_in_rxvec_modulation == `ERP_DSSS) && (l_index <= `UU_RATE_2M_BIT))begin
								if (loc_lmac_rate == `UU_HW_RATE_1M) 
									loc_txvec_L_datarate <= `UU_HW_RATE_1M;
								else 
									loc_txvec_L_datarate <= `UU_HW_RATE_2M;
							end
							else if ((cp_dur_in_rxvec_modulation == `ERP_CCK || cp_dur_in_rxvec_modulation == `ERP_PBCC) && (l_index < `UU_SUPPORTED_CCK_RATE))begin
								if ((loc_lmac_rate == `UU_HW_RATE_1M) || (loc_lmac_rate == `UU_HW_RATE_2M) 
																		 || (loc_lmac_rate == `UU_HW_RATE_5M5))
									loc_txvec_L_datarate <= `UU_HW_RATE_5M5;
								else 
									loc_txvec_L_datarate <= `UU_HW_RATE_11M;
							end
							else begin       
								if ((loc_lmac_rate & `UU_CCK_RATE_FLAG) == 0)begin
									if(((loc_lmac_rate & `UU_OFDM_MODULATION_TYPE_IND) == `UU_OFDM_BPSK) && (l_index <= `UU_RATE_6M_BIT))
										loc_txvec_L_datarate <= `UU_HW_RATE_6M;
									else if(((loc_lmac_rate & `UU_OFDM_MODULATION_TYPE_IND) == `UU_OFDM_QPSK) && (l_index <= `UU_RATE_12M_BIT))
										loc_txvec_L_datarate <= `UU_HW_RATE_12M; 
									else 
										loc_txvec_L_datarate <= `UU_HW_RATE_24M;
								end
								else if (l_index < `UU_SUPPORTED_CCK_RATE )
									loc_txvec_L_datarate <= loc_lmac_rate;
								else 
									loc_txvec_L_datarate <= `UU_HW_RATE_1M;
							end
							loc_txvec_updated1 <= 1'b1;
						end
					end
					else 
						loc_txvec_updated1 <= 1'b1;
				end
			end//txvec_updated
			else begin
				if (loc_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF) begin
					cp_dur_out_ht_mcs_en <= 1'b1;
					if (cp_dur_in_txvec_ch_bndwdth == `CBW40)begin
						if(cp_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_40_sgi
							cp_dur_out_ht_mcs_addr <= ((cp_dur_out_txvec_mcs*`HT_INDEX)+`HT_40_SGI);
							loc_ht_data_val        <= 1'b1;
							if (loc_ht_data_val) begin
								loc_ht_data_val    <= 1'b0;								
								loc_ht_mcs_250kbps <= cp_dur_in_ht_mcs_data;
								loc_ht_rate_val    <= 1'b1;
							end
						end
						else begin//mcs_ac_rate_40
							cp_dur_out_ht_mcs_addr <= ((cp_dur_out_txvec_mcs*`HT_INDEX)+`HT_40);
							loc_ht_data_val        <= 1'b1;
							if (loc_ht_data_val) begin
								loc_ht_data_val    <= 1'b0;		
								loc_ht_mcs_250kbps <= cp_dur_in_ht_mcs_data;
								loc_ht_rate_val    <= 1'b1;
							end
						end
					end
					else begin
						if(cp_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_20_sgi
							cp_dur_out_ht_mcs_addr <= ((cp_dur_out_txvec_mcs*`HT_INDEX)+`HT_20_SGI);
							loc_ht_data_val        <= 1'b1;
							if (loc_ht_data_val) begin
								loc_ht_data_val    <= 1'b0;		
								loc_ht_mcs_250kbps <= cp_dur_in_ht_mcs_data;
								loc_ht_rate_val    <= 1'b1;
							end
						end
						else begin//mcs_ac_rate_20
							cp_dur_out_ht_mcs_addr <= ((cp_dur_out_txvec_mcs*`HT_INDEX)+`HT_20);
							loc_ht_data_val        <= 1'b1;
							if (loc_ht_data_val) begin
								loc_ht_data_val    <= 1'b0;		
								loc_ht_mcs_250kbps <= cp_dur_in_ht_mcs_data;
								loc_ht_rate_val    <= 1'b1;
							end
						end
					end	
				end
				//Non-HT case
				else begin
					cp_dur_out_lrate_en <= 1'b1;
					if (loc_txvec_L_datarate & `UU_CCK_RATE_FLAG) begin
						cp_dur_out_lrate_addr <=  (lmac_rate_to_index(loc_txvec_L_datarate)*`NON_HT_INDEX)+`NON_HT_250;
						loc_lrate_data_val    <= 1'b1;
						if (loc_lrate_data_val) begin
							loc_rate_250kbps    <= cp_dur_in_lrate_data;
							loc_lrate_data_val  <= 1'b0;
							loc_non_ht_rate_val <= 1'b1;
						end
					end
					else begin
						cp_dur_out_lrate_addr <=  ((ofdm_rate_to_index(loc_txvec_L_datarate & `UU_OFDM_HW_MASK_INDEX))*`NON_HT_INDEX)+`NON_HT_250;
						loc_lrate_data_val    <= 1'b1;
						if (loc_lrate_data_val) begin
							loc_rate_250kbps    <= cp_dur_in_lrate_data;
							loc_lrate_data_val  <= 1'b0;
							loc_non_ht_rate_val <= 1'b1;
						end
					end
				end 			 
			end

		end
		else	begin 
			if(!loc_txvec_updated1)begin
				if(!loc_txvec_updated2)begin
					if (cp_dur_in_rxvec_format == `UU_WLAN_FRAME_FORMAT_VHT)begin
						loc_txvec_format        <= `UU_WLAN_FRAME_FORMAT_NON_HT;
						cp_dur_out_vht_mcs_en   <= 1'b1;
						loc_vht_data_val        <= 1'b1;
						loc_txvec_L_length      <= 0;
						cp_dur_out_vht_mcs_addr <= (((cp_dur_in_rxvec_mcs>>3)&8'h0f)*`VHT_INDEX)+`VHT_OFDM;
						if (loc_vht_data_val) begin
							loc_lmac_rate      <= cp_dur_in_vht_mcs_data[3:0];
							loc_vht_data_val   <= 1'b0;
							loc_txvec_updated2 <= 1'b1;
						end
					end
					else	if (cp_dur_in_rxvec_format == `UU_WLAN_FRAME_FORMAT_HT_MF)begin
						loc_txvec_format       <= `UU_WLAN_FRAME_FORMAT_NON_HT;
						cp_dur_out_ht_mcs_en   <= 1'b1;
						loc_ht_data_val        <= 1'b1;
						loc_txvec_L_length     <= 0;
						cp_dur_out_ht_mcs_addr <= ((cp_dur_in_rxvec_mcs & `UU_HT_MCS_MASK)*`HT_INDEX)+`HT_OFDM;
						if (loc_ht_data_val) begin
							loc_lmac_rate      <= cp_dur_in_ht_mcs_data[3:0];
							loc_ht_data_val    <= 1'b0;
							loc_txvec_updated2 <= 1'b1;
						end
					end
					else	if (cp_dur_in_rxvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF)begin
						loc_txvec_format     <= `UU_WLAN_FRAME_FORMAT_HT_GF;
						loc_txvec_mcs        <= cp_dur_in_rxvec_mcs % 8;
						loc_txvec_ht_length  <= cp_dur_in_resp_length;
						loc_txvec_updated2   <= 1'b1;
					end
					else begin
						loc_txvec_format   <= `UU_WLAN_FRAME_FORMAT_NON_HT;
						loc_txvec_L_length <= 'h0;
						loc_lmac_rate      <= cp_dur_in_rxvec_L_datarate;
						loc_txvec_updated2   <= 1'b1;
					end
				end
				else begin//loc_txvec_updated2
					if(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT)begin
						if (get_blrate!=0) begin
							cp_dur_out_lrate_en   <= 1'b1;
							loc_lrate_data_val    <= 1'b1;
							cp_dur_out_lrate_addr <= (get_blrate*`NON_HT_INDEX)+`NON_HT_HW;
							if (loc_lrate_data_val) begin
								loc_txvec_L_datarate <= cp_dur_in_lrate_data;
								loc_lrate_data_val   <= 1'b0;
								loc_txvec_updated1   <= 1'b1;
							end
						end	   
						else begin
							if ((cp_dur_in_rxvec_modulation == `ERP_DSSS) && (l_index <= `UU_RATE_2M_BIT))begin
								if (loc_lmac_rate == `UU_HW_RATE_1M) 
									loc_txvec_L_datarate <= `UU_HW_RATE_1M;
								else 
									loc_txvec_L_datarate <= `UU_HW_RATE_2M;
							end
							else if ((cp_dur_in_rxvec_modulation == `ERP_CCK || cp_dur_in_rxvec_modulation == `ERP_PBCC) && (l_index < `UU_SUPPORTED_CCK_RATE))begin
								if ((loc_lmac_rate == `UU_HW_RATE_1M) || (loc_lmac_rate == `UU_HW_RATE_2M) 
																		 || (loc_lmac_rate == `UU_HW_RATE_5M5))
									loc_txvec_L_datarate <= `UU_HW_RATE_5M5;
								else 
									loc_txvec_L_datarate <= `UU_HW_RATE_11M;
							end
							else begin       
								if ((loc_lmac_rate & `UU_CCK_RATE_FLAG) == 0)begin
									if(((loc_lmac_rate & `UU_OFDM_MODULATION_TYPE_IND) == `UU_OFDM_BPSK) && (l_index <= `UU_RATE_6M_BIT))
										loc_txvec_L_datarate <= `UU_HW_RATE_6M;
									else if(((loc_lmac_rate & `UU_OFDM_MODULATION_TYPE_IND) == `UU_OFDM_QPSK) && (l_index <= `UU_RATE_12M_BIT))
										loc_txvec_L_datarate <= `UU_HW_RATE_12M; 
									else 
										loc_txvec_L_datarate <= `UU_HW_RATE_24M;
								end
								else if (l_index < `UU_SUPPORTED_CCK_RATE )
									loc_txvec_L_datarate <= loc_lmac_rate;
								else 
									loc_txvec_L_datarate <= `UU_HW_RATE_1M;
							end
							loc_txvec_updated1 <= 1'b1;
						end
					end
					else 
						loc_txvec_updated1 <= 1'b1;
				end
			end//txvec_updated
			else begin
				if (loc_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF) begin
					cp_dur_out_ht_mcs_en <= 1'b1;
					if (cp_dur_in_txvec_ch_bndwdth == `CBW40)begin
						if(cp_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_40_sgi
							cp_dur_out_ht_mcs_addr <= ((cp_dur_out_txvec_mcs*`HT_INDEX)+`HT_40_SGI);
							loc_ht_data_val        <= 1'b1;
							if (loc_ht_data_val) begin
								loc_ht_data_val    <= 1'b0;								
								loc_ht_mcs_250kbps <= cp_dur_in_ht_mcs_data;
								loc_ht_rate_val    <= 1'b1;
							end
						end
						else begin//mcs_ac_rate_40
							cp_dur_out_ht_mcs_addr <= ((cp_dur_out_txvec_mcs*`HT_INDEX)+`HT_40);
							loc_ht_data_val        <= 1'b1;
							if (loc_ht_data_val) begin
								loc_ht_data_val    <= 1'b0;		
								loc_ht_mcs_250kbps <= cp_dur_in_ht_mcs_data;
								loc_ht_rate_val    <= 1'b1;
							end
						end
					end
					else begin
						if(cp_dur_in_txvec_is_short_GI) begin//mcs_ac_rate_20_sgi
							cp_dur_out_ht_mcs_addr <= ((cp_dur_out_txvec_mcs*`HT_INDEX)+`HT_20_SGI);
							loc_ht_data_val        <= 1'b1;
							if (loc_ht_data_val) begin
								loc_ht_data_val    <= 1'b0;		
								loc_ht_mcs_250kbps <= cp_dur_in_ht_mcs_data;
								loc_ht_rate_val    <= 1'b1;
							end
						end
						else begin//mcs_ac_rate_20
							cp_dur_out_ht_mcs_addr <= ((cp_dur_out_txvec_mcs*`HT_INDEX)+`HT_20);
							loc_ht_data_val        <= 1'b1;
							if (loc_ht_data_val) begin
								loc_ht_data_val    <= 1'b0;		
								loc_ht_mcs_250kbps <= cp_dur_in_ht_mcs_data;
								loc_ht_rate_val    <= 1'b1;
							end
						end
					end	
				end
				//Non-HT case
				else begin
					cp_dur_out_lrate_en <= 1'b1;
					if (loc_txvec_L_datarate & `UU_CCK_RATE_FLAG) begin
						cp_dur_out_lrate_addr <=  (lmac_rate_to_index(loc_txvec_L_datarate)*`NON_HT_INDEX)+`NON_HT_250;
						loc_lrate_data_val    <= 1'b1;
						if (loc_lrate_data_val) begin
							loc_rate_250kbps    <= cp_dur_in_lrate_data;
							loc_lrate_data_val  <= 1'b0;
							loc_non_ht_rate_val <= 1'b1;
						end
					end
					else begin
						cp_dur_out_lrate_addr <=  ((ofdm_rate_to_index(loc_txvec_L_datarate & `UU_OFDM_HW_MASK_INDEX))*`NON_HT_INDEX)+`NON_HT_250;
						loc_lrate_data_val    <= 1'b1;
						if (loc_lrate_data_val) begin
							loc_rate_250kbps    <= cp_dur_in_lrate_data;
							loc_lrate_data_val  <= 1'b0;
							loc_non_ht_rate_val <= 1'b1;
						end
					end
				end 			 
			end
		end
	end

	//40 bit and 21 bit multiplication
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			n_syms_ht_1    <= 0;
			n_syms_ht_2    <= 0;
			n_syms_ht_3    <= 0;
			n_syms_ht_4    <= 0;
			n_syms_ht_5    <= 0;
			n_syms_lrate <= 0;
			lrate_dur    <= 0;
			loc_ht_val   <= 0;
			loc_non_ht_val <= 0;
		end
		else if(!cp_dur_en)begin
			n_syms_ht_1    <= 0;
			n_syms_ht_2    <= 0;
			n_syms_ht_3    <= 0;
			n_syms_ht_4    <= 0;
			n_syms_ht_5    <= 0;
			n_syms_lrate <= 0;
			n_syms_lrate <= 0;
			n_syms_lrate <= 0;
			lrate_dur    <= 0;
			loc_ht_val   <= 0;
			loc_non_ht_val <= 0;
		end
		else if(loc_ht_rate_val)begin
			n_syms_ht_1  <=  8 * loc_txvec_ht_length ;
			n_syms_ht_2 <= (`PHY_SERVICE_NBITS + n_syms_ht_1 + `PHY_TAIL_NBITS);
			n_syms_ht_3 <= loc_ht_mcs_250kbps * n_syms_ht_2;
			loc_ht_val <= {loc_ht_val,1'b1};
		end
		else if(loc_non_ht_rate_val)begin
			if(loc_txvec_L_datarate & `UU_CCK_RATE_FLAG) begin
				lrate_dur <= loc_rate_250kbps * (loc_txvec_L_length * 8 * 4);
			end
			else begin
				n_syms_lrate <= loc_rate_250kbps * (`PHY_SERVICE_NBITS + 8 * loc_txvec_L_length + `PHY_TAIL_NBITS);
			end
			loc_non_ht_val <= {loc_non_ht_val,1'b1};
		end
	end

	//calculating final duration 
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_dur_avl     <= 0;
			loc_dur_updated <= 0;	
		end
		else if(!cp_dur_en)begin
			loc_dur_avl     <= 0;
			loc_dur_updated <= 0;
		end
		else if(loc_ht_val==5'h7)begin			
			if(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_MF) begin
				loc_dur_avl     <= 1'b1;
				loc_dur_updated <= ht_duration + (`PHY_SYMBOL_TIME * nsyms_ht) + `DOT11_SIGNAL_EXTENSION + `PHY_SIGNAL_TIME + `PHY_HT_STF;
			end
			else begin
				loc_dur_avl     <= 1'b1;
				loc_dur_updated <= ht_duration + (`PHY_SYMBOL_TIME * nsyms_ht) + `DOT11_SIGNAL_EXTENSION;
			end	
		end
		else if(loc_non_ht_val==3'h7)begin
			if(loc_txvec_L_datarate & `UU_CCK_RATE_FLAG) begin
				loc_dur_updated <= cck_duration;
				loc_dur_avl     <= 1'b1;
			end
			else begin
				loc_dur_updated <= `PHY_PREAMBLE_TIME + `PHY_SIGNAL_TIME + (`PHY_SYMBOL_TIME * nsyms_lrate) + `DOT11_SIGNAL_EXTENSION;
				loc_dur_avl     <= 1'b1;
			end
		end
		else begin
				loc_dur_updated <= 0;
				loc_dur_avl     <= 0;
		end
	end

	//always block for sending updated duration 
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_dur_out_dur_avl     <= 0;
			cp_dur_out_dur_updated <= 0;
		end
		else if(!cp_dur_en)begin
			cp_dur_out_dur_avl     <= 0;
			cp_dur_out_dur_updated <= 0;			
		end 
		else if(loc_dur_avl)begin //***Temp Fix by UU
			if(loc_duration < 0 )begin
				cp_dur_out_dur_updated <= 16'd100;
				cp_dur_out_dur_avl     <= 1'b1;
			end 
			else begin
				cp_dur_out_dur_updated <= cp_dur_in_mpdu_dur - loc_dur_updated - cp_dur_in_SIFS_timer_value;
				cp_dur_out_dur_avl     <= 1'b1;
			end
		end
	end
	/*-----------------------------------------------------------------------------------------------*/
endmodule

//***EOF***

