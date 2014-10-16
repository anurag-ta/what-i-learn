`include "../../inc/defines.h" 

module uu_acmac_cp_if (
	input                clk                        ,
	input                rst_n                      ,
	input                cp_en                      ,
	//Events given by cp_wrap
	input                cp_in_ev_rxstart_ind       , //rxvec start indication
	input                cp_in_ev_rxdata_ind        , //mpdu start indication
	//indicaiton from rx_handler
	input                cp_in_is_rxend             , //rx_out_rxend_ind 
	output  reg          cp_out_tx_upd_mpdu_status  , //connects tx_in_cp_upd_mpdu_status
	output  reg          cp_out_tx_get_ampdu_status , //connects tx_in_cp_get_ampdu_status
	//Input frame on data line  
	input                cp_in_rx_data_val          , 
	input        [7 :0]  cp_in_rx_data              , 
	//Tx side inputs from Tx_handler 
	input                cp_in_tx_start_ind         , //generate rts/cts indication
	input                cp_in_tx_self_cts          ,
	input        [1 :0]  cp_in_tx_txvec_format      ,
	input        [3 :0]  cp_in_tx_rtscts_rate       ,
	input        [47:0]  cp_in_tx_mpdu_ra           ,
	//Tx I/Os
	output  reg  [1 :0]  cp_out_tx_ac               ,
	output  reg  [15:0]  cp_out_tx_seqno            ,
	input                cp_in_tx_res_val           ,
	input        [31:0]  cp_in_tx_res               ,
	output  reg          cp_out_tx_res_val          ,
	output  reg  [31:0]  cp_out_tx_res              ,
	//inputs from CAP
	input                cp_in_waiting_for_cts      , //cap_out_cp_wait_for_cts
	input                cp_in_waiting_for_ack      , //cap_out_cp_wait_for_ack
	//Station & BA Management interface 
	input                cp_in_sta_info_val         ,
	input                cp_in_sta_info             ,
	input        [14:0]  cp_in_sta_offset           ,
	output               cp_out_sta_get_info        ,
	output  reg  [47:0]  cp_out_sta_addr            ,
	output  reg  [3 :0]  cp_out_sta_tid             ,
	output  reg          cp_out_sta_dir             ,
	//sta_ba_info memory interface (depth : 138 used * 40)(width : 16 bits)
	output       [31:0]  cp_out_sta_ba_data         , 
	output       [3 :0]  cp_out_sta_ba_wen          ,
	output               cp_out_sta_ba_en           ,
	output       [14:0]  cp_out_sta_ba_addr         ,
	input        [31:0]  cp_in_sta_ba_data          ,
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
	//Updated txvec 
	output  reg          cp_out_txvec_is_fec_ldpc   ,
	output  reg          cp_out_txvec_is_short_GI   ,
	output       [1 :0]  cp_out_txvec_format        ,
	output       [3 :0]  cp_out_txvec_L_datarate    , 
	output       [11:0]  cp_out_txvec_L_length      ,
	output       [15:0]  cp_out_txvec_ht_length     ,
	output       [6 :0]  cp_out_txvec_mcs           ,
	output       [1 :0]  cp_out_txvec_stbc          ,
	//output reponse to rx handler
	output  reg  [31:0]  cp_out_rx_res              , //rx_in_cp_return_value | return response
	output  reg          cp_out_rx_res_en           , //rx_in_cp_return_valid | return response valid
	//output frame interfaced with tx_ctrl_frame_info_g memory
	output    [7 :0]  cp_out_packet_length       ,
	output    [7 :0]  cp_out_tx_data             ,
	output            cp_out_tx_data_val         ,         
	//added for de-aggr case
	input                cp_in_rx_aggr              ,
	input      [15:0]    cp_in_rx_seq_no
	);
	
	/* Registers internal to the module 
	............................................................*/
	// outputs to cp_dur module
	reg           cp_out_dur_en           ;
	reg  [15:0]   cp_out_mpdu_dur         ;
	reg  [7 :0]   cp_out_mpdu_fc          ;	    
	reg  [1 :0]   cp_out_rxvec_format     ;
	reg  [2 :0]   cp_out_rxvec_modulation ;
	reg  [3 :0]   cp_out_rxvec_L_datarate ;
	reg  [6 :0]   cp_out_rxvec_mcs        ;
	reg  [7 :0]   cp_out_resp_length      ;
	// input from cp_dur module
	reg           loc_dur_avl             ;
	wire          cp_in_dur_avl           ;
	wire [15:0]   cp_in_dur_updated       ;
	//output to cp_pkt_gen module
	reg           cp_out_ctrl_pkt_gen_en  ;
	reg  [3 :0]   cp_out_pkt_gen_type     ;
	reg  [3 :0]   cp_out_pkt_subtype      ;
	reg 	[15:0]   cp_out_duration         ;
	reg  [47:0]   cp_out_mpdu_ra          ;
	reg  [47:0]   cp_out_mpdu_ta          ;
	reg  [15:0]   cp_out_mpdu_bar_fc      ;
	reg  [3 :0]   cp_out_mpdu_tid         ;
	reg  [15:0]   cp_out_mpdu_scf         ;
	reg  [1 :0]   cp_out_bar_type         ;
	reg  [15:0]   cp_out_mpdu_bar_ssn     ;
	reg           cp_out_implicit         ;   
	//BA_SES
	wire          loc_ba1_ses_en          ;
	wire [3 :0]   loc_ba1_ses_wen         ;
	wire [14:0]   loc_ba1_ses_addr        ;
	wire [31:0]   loc_ba1_ses_data        ;
	wire [14:0]   loc_ba2_ses_addr        ;
	wire [31:0]   loc_ba2_ses_data        ;
	wire          loc_ba2_ses_en          ;
	wire [3 :0]   loc_ba2_ses_wen         ;
	reg  [14:0]   loc_ba3_ses_addr        ;
	reg  [31:0]   loc_ba3_ses_data        ;
	reg           loc_ba3_ses_en          ;
	reg  [3 :0]   loc_ba3_ses_wen         ;
	//BITMAP
	reg           loc_bitmap_en           ;
	reg           loc_bmp_update_done     ;
	reg  [47:0]   loc_bmp_in_mpdu_ta      ; 
	reg  [3 :0]   loc_bmp_in_mpdu_tid     ; 
	reg  [15:0]   loc_bmp_in_mpdu_scf     ;
	reg           loc_bmp_in_dir          ;
	wire          loc_bmp_res_en          ;
	wire [31:0]   loc_bmp_res             ;
	//counters and recording
	reg  [5 :0]   count                   ;
	reg  [5 :0]   count2                  ;
	reg  [1 :0]   rxvec_format            ;
	reg  [2 :0]   rxvec_modulation        ;
	reg  [3 :0]   rxvec_L_datarate        ;
	reg           rxvec_is_aggr           ;
	reg  [6 :0]   rxvec_mcs               ;
	wire [3 :0]   loc_f_type              ;
	reg  [7 :0]   mpdu_fc0                ;
	reg  [15:0]   mpdu_dur                ;
	reg  [47:0]   mpdu_ta                 ;
	reg  [47:0]   mpdu_ra                 ;
	reg  [15:0]   mpdu_bar_fc             ;
	reg  [1 :0]   mpdu_qos_ack            ;
	reg  [1 :0]   mpdu_bar_type           ;
	reg  [3 :0]   mpdu_tid                ;
	reg  [15:0]   mpdu_scf                ;
	//local additional registers	
	reg           loc_tx_data_avl         ;
	reg           loc_cp_rx_busy          ;
	reg           loc_cp_tx_busy          ;
	reg           loc_cp_tx_rd            ;
	reg           loc_cp_rd               ;
	reg           loc_out_done            ;
	reg           loc_out_data_val        ;
	reg           loc_bar_val             ;
	reg           loc_tx_start_ind        ;
	reg           loc_rx_start_ind        ;
	reg           loc_rx_data_ind         ;	
	reg           loc_out_imdone          ;
	reg  [3 :0]   loc_tx_rtscts_rate      ;
	reg  [47:0]   loc_tx_mpdu_ra          ;
	reg  [1 :0]   loc_tx_txvec_format     ; 
	reg  [7 :0]   loc_out_count           ;
	wire [1 :0]   cp_out_rx_txvec_format  ;
	wire [6 :0]   cp_out_rx_txvec_mcs     ;
	reg  [6 :0]   cp_out_tx_txvec_mcs     ;
	reg  [1 :0]   cp_out_tx_txvec_format  ;
	reg  [3 :0]   cp_out_tx_txvec_L_datarate;
	wire [3 :0]   cp_out_rx_txvec_L_datarate;
	reg  [63:0]   loc_comp_bitmap         ;
//	reg  [15:0]   loc_basic_bitmap  [0:63];
	reg           loc_sta_info_avl        ;
	reg           loc_sta_info_val        ;
	reg           loc_bmp_upd             ;
	reg           loc_bmp_upd_done        ;
	reg  [14:0]   loc_bmp_offset          ;
	reg           loc_sta_get_info1       ;
	reg           loc_sta_get_info1_dly   ;
	wire           loc_sta_get_info2       ;
	reg           loc_reg_imba_pending    ;
	reg  [7 :0]   addr_count              ;


	/*----------------------------------------------------------------*/


	/* Module Instantiations
	.......................................................................*/
	uu_acmac_cp_ctrl_pkt_gen PKT_GEN(
		.clk                      (clk                    ),
		.rst_n                    (rst_n                  ),
		.cp_ctrl_pkt_gen_en       (cp_out_ctrl_pkt_gen_en ),
		.cp_ctrl_in_pkt_subtype   (cp_out_pkt_subtype     ),
		.cp_ctrl_in_pkt_gen_type  (cp_out_pkt_gen_type    ),
		.cp_ctrl_in_duration      (cp_out_duration        ),
		.cp_ctrl_in_mpdu_ra       (cp_out_mpdu_ra         ),
		.cp_ctrl_in_mpdu_ta       (cp_out_mpdu_ta         ),
		.cp_ctrl_in_mpdu_bar_type (cp_out_bar_type        ),
		.cp_ctrl_in_mpdu_bar_tid  (cp_out_mpdu_tid        ),
		.cp_ctrl_in_mpdu_bar_scf  (cp_out_mpdu_scf        ),
		.cp_ctrl_in_mpdu_bar_fc   (cp_out_mpdu_bar_fc     ),
		.cp_ctrl_in_mpdu_bar_ssn  (cp_out_mpdu_bar_ssn    ),
		.cp_ctrl_in_implicit      (cp_out_implicit        ),
		.cp_ctrl_out_tx_frame_len (cp_out_packet_length   ),
		.cp_ctrl_out_pkt_data_val (cp_out_tx_data_val     ),
		.cp_ctrl_out_pkt_data     (cp_out_tx_data         ),
		.cp_ctrl_in_sta_ba_data   (cp_in_sta_ba_data      ),
		.cp_ctrl_out_sta_ba_addr  (loc_ba1_ses_addr       ),
		.cp_ctrl_out_sta_ba_data  (loc_ba1_ses_data       ),
		.cp_ctrl_out_sta_ba_en    (loc_ba1_ses_en         ),
		.cp_ctrl_out_sta_ba_wen   (loc_ba1_ses_wen        ),
		.cp_ctrl_in_sta_offset    (cp_in_sta_offset       ),
		.cp_ctrl_in_crc_avl       (cp_in_crc_avl          ),
		.cp_ctrl_in_crc           (cp_in_crc              ),
		.cp_ctrl_out_crc_len      (cp_out_crc_len         ),
		.cp_ctrl_out_crc_en       (cp_out_crc_en          ),
		.cp_ctrl_out_crc_data_val (cp_out_crc_data_val    ),
		.cp_ctrl_out_crc_data     (cp_out_crc_data        )

	);
	
	uu_acmac_cp_duration DUR(
		.clk                         (clk                       ), 
		.rst_n                       (rst_n                     ), 
		.cp_dur_en                   (cp_out_dur_en             ), 
		.cp_dur_in_mpdu_dur          (cp_out_mpdu_dur           ), 
		.cp_dur_in_mpdu_fc           (cp_out_mpdu_fc            ), 
		.cp_dur_in_rxvec_format      (cp_out_rxvec_format       ), 
		.cp_dur_in_rxvec_modulation  (cp_out_rxvec_modulation   ), 
		.cp_dur_in_rxvec_L_datarate  (cp_out_rxvec_L_datarate   ), 
		.cp_dur_in_rxvec_mcs         (cp_out_rxvec_mcs          ), 
		.cp_dur_in_resp_length       (cp_out_resp_length        ),
		.cp_dur_out_dur_updated      (cp_in_dur_updated         ), 
		.cp_dur_out_dur_avl          (cp_in_dur_avl             ), 
		.cp_dur_out_txvec_format     (cp_out_rx_txvec_format    ),
		.cp_dur_out_txvec_L_datarate (cp_out_rx_txvec_L_datarate),
		.cp_dur_out_txvec_L_length   (cp_out_txvec_L_length     ),
		.cp_dur_out_txvec_ht_length  (cp_out_txvec_ht_length    ),
		.cp_dur_out_txvec_mcs        (cp_out_rx_txvec_mcs       ),
		.cp_dur_out_txvec_stbc       (cp_out_txvec_stbc         ),
		.cp_dur_in_txvec_num_ext_ss  (2'h0                      ),
		.cp_dur_in_txvec_is_long_pre (1'b0                      ),
		.cp_dur_in_txvec_is_short_GI (1'b0                      ),
		.cp_dur_in_txvec_ch_bndwdth  (3'h0                      ),
		.cp_dur_out_vht_mcs_en       (cp_out_vht_mcs_en         ), 
		.cp_dur_out_vht_mcs_wen      (cp_out_vht_mcs_wen        ), 
		.cp_dur_out_vht_mcs_addr     (cp_out_vht_mcs_addr       ), 
		.cp_dur_in_vht_mcs_data      (cp_in_vht_mcs_data        ), 
		.cp_dur_out_ht_mcs_en        (cp_out_ht_mcs_en          ), 
		.cp_dur_out_ht_mcs_wen       (cp_out_ht_mcs_wen         ), 
		.cp_dur_out_ht_mcs_addr      (cp_out_ht_mcs_addr        ), 
		.cp_dur_in_ht_mcs_data       (cp_in_ht_mcs_data         ), 
		.cp_dur_out_lrate_en         (cp_out_lrate_en           ), 
		.cp_dur_out_lrate_wen        (cp_out_lrate_wen          ), 
		.cp_dur_out_lrate_addr       (cp_out_lrate_addr         ), 
		.cp_dur_in_lrate_data        (cp_in_lrate_data          ),
		.cp_dur_in_BSSBasicRateSet   (cp_in_BSSBasicRateSet     ),
		.cp_dur_in_SIFS_timer_value  (cp_in_SIFS_timer_value    )
	);
	
	uu_acmac_cp_update_bitmap BMP_UPD(
		.clk                    (clk                  ), 
		.rst_n                  (rst_n                ), 
		.cp_bmp_en              (loc_bitmap_en        ), 
		.cp_bmp_out_sta_ba_data (loc_ba2_ses_data     ), 
		.cp_bmp_out_sta_ba_wen  (loc_ba2_ses_wen      ), 
		.cp_bmp_out_sta_ba_en   (loc_ba2_ses_en       ), 
		.cp_bmp_in_sta_ba_data  (cp_in_sta_ba_data    ), 
		.cp_bmp_out_sta_ba_addr (loc_ba2_ses_addr     ), 
		.cp_bmp_in_sta_info_val (cp_in_sta_info_val   ),
		.cp_bmp_in_sta_info     (cp_in_sta_info       ),
		.cp_bmp_in_sta_offset   (cp_in_sta_offset     ),
		.cp_bmp_out_sta_get_info(loc_sta_get_info2    ),
		.cp_bmp_in_mpdu_scf     (loc_bmp_in_mpdu_scf  ), 
		.cp_bmp_out_rx_res_en   (loc_bmp_res_en       ), 
		.cp_bmp_out_rx_res      (loc_bmp_res          )
	);
	/*----------------------------------------------------------------------*/

	/* Continuous assignments 
	............................................................................................*/
	assign  cp_out_sta_ba_data = loc_ba1_ses_data | loc_ba2_ses_data| loc_ba3_ses_data;
	assign  cp_out_sta_ba_wen  = loc_ba1_ses_wen  | loc_ba2_ses_wen | loc_ba3_ses_wen ;
	assign  cp_out_sta_ba_en   = loc_ba1_ses_en   | loc_ba2_ses_en  | loc_ba3_ses_en  ;
	assign  cp_out_sta_ba_addr = loc_ba1_ses_addr | loc_ba2_ses_addr| loc_ba3_ses_addr;
	assign  cp_out_txvec_format = cp_out_rx_txvec_format | cp_out_tx_txvec_format     ;
	assign  cp_out_txvec_mcs    = cp_out_rx_txvec_mcs    | cp_out_tx_txvec_mcs        ;
	assign  cp_out_txvec_L_datarate  =  loc_tx_start_ind ? cp_out_tx_txvec_L_datarate : cp_out_rx_txvec_L_datarate ;   
	assign  loc_f_type = frame_type(mpdu_fc0,rxvec_is_aggr,mpdu_qos_ack,cp_in_is_rxend);
	assign  cp_out_sta_get_info = (loc_sta_get_info1&~loc_sta_get_info1_dly) | loc_sta_get_info2;
	/*------------------------------------------------------------------------------------------*/
		
	/* Function for finding out frame_type received from Rx_handler
	............................................................................................*/
	function [3:0] frame_type;
		input [7:0] fc           ;
		input       is_aggregated;
		input [1:0] qos_ack      ;
		input       is_ampdu_end ;
		reg   [7:0] fn_fc        ;
	begin
		if ((fc & `IEEE80211_FC0_TYPE_MASK) == `IEEE80211_FC0_TYPE_MGT)begin
			if (fc == (`IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK|`IEEE80211_FC0_TYPE_MGT))
				fn_fc = fc;
			else
				fn_fc = `IEEE80211_FC0_TYPE_MGT;
		end
		else 
			fn_fc = fc;

		case (fn_fc)
		//Control
		(`IEEE80211_FC0_SUBTYPE_RTS | `IEEE80211_FC0_TYPE_CTL): frame_type = `RTS_PKT;
		(`IEEE80211_FC0_SUBTYPE_CTS | `IEEE80211_FC0_TYPE_CTL): frame_type = `CTS_PKT;
		(`IEEE80211_FC0_SUBTYPE_ACK | `IEEE80211_FC0_TYPE_CTL): frame_type = `ACK_PKT;
		(`IEEE80211_FC0_SUBTYPE_BAR | `IEEE80211_FC0_TYPE_CTL): frame_type = `BAR_PKT;
		(`IEEE80211_FC0_SUBTYPE_BA  | `IEEE80211_FC0_TYPE_CTL): frame_type = `BA_PKT ;
		//Data
		(`IEEE80211_FC0_SUBTYPE_QOS 	    | `IEEE80211_FC0_TYPE_DATA): 
		begin	
			if(qos_ack == `UU_WLAN_ACKPOLICY_NORMAL)
				if(is_aggregated)
					if(is_ampdu_end) 
						frame_type = `DATA_IMBA_PKT;//implicit
					else
						frame_type = `DATA_BA_PKT;
				else
					frame_type = `DATA_ACK_PKT;
			else if(qos_ack == `UU_WLAN_ACKPOLICY_NO_ACK)
				frame_type = `SUBTYPE_ERROR;
			else if(qos_ack == `UU_WLAN_ACKPOLICY_PSMP_ACK)
				frame_type = `SUBTYPE_ERROR;
			else
				frame_type = `DATA_BA_PKT;
		end
		(`IEEE80211_FC0_SUBTYPE_QOS_NULL  | `IEEE80211_FC0_TYPE_DATA): 	
		begin	
			if(qos_ack == `UU_WLAN_ACKPOLICY_NORMAL)
				if(is_aggregated)
					if(is_ampdu_end) 
						frame_type = `DATA_IMBA_PKT;//implicit
					else
						frame_type = `DATA_BA_PKT;
				else
					frame_type = `DATA_ACK_PKT;
			else if(qos_ack == `UU_WLAN_ACKPOLICY_NO_ACK)
				frame_type = `SUBTYPE_ERROR;
			else if(qos_ack == `UU_WLAN_ACKPOLICY_PSMP_ACK)
				frame_type = `SUBTYPE_ERROR;
			else
				frame_type = `DATA_BA_PKT;
		end
		(`IEEE80211_FC0_SUBTYPE_DATA      | `IEEE80211_FC0_TYPE_DATA):  frame_type = `DATA_ACK_PKT ;		
		(`IEEE80211_FC0_SUBTYPE_DATA_NULL | `IEEE80211_FC0_TYPE_DATA):  frame_type = `DATA_ACK_PKT ;
		//Management
		`IEEE80211_FC0_TYPE_MGT:                                        frame_type = `MGT_PKT      ;
		(`IEEE80211_FC0_SUBTYPE_ACTION_NO_ACK|`IEEE80211_FC0_TYPE_MGT): frame_type = `SUBTYPE_ERROR;
		`IEEE80211_FC0_TYPE_RESERVED:                                   
		begin
			if(loc_reg_imba_pending)
				frame_type = `RESERVED;
			else
				frame_type = `SUBTYPE_ERROR;
		end
		default:                                                        frame_type = `SUBTYPE_ERROR;
		endcase
	end
	endfunction

	function [1:0] ac_from_tid;
		input [3:0] tid;
	begin
		if((tid == 6) || (tid == 7))
			ac_from_tid = `UU_WLAN_AC_VO;
		else if((tid == 1) || (tid == 2))
			ac_from_tid = `UU_WLAN_AC_BK;
		else if((tid == 4) || (tid == 5))
			ac_from_tid = `UU_WLAN_AC_VI;
		else if((tid == 0) || (tid == 3))
			ac_from_tid = `UU_WLAN_AC_BE;
		else 
			ac_from_tid = `UU_WLAN_AC_BE;
	end
	endfunction 

	/*------------------------------------------------------------------------------------------------*/

	/* Concurrent statements
	.................................................................................*/
	//Latching indications from Rx_handler and Tx_Handler
	always@(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
			loc_tx_start_ind <= 0;
			loc_rx_start_ind <= 0;
			loc_rx_data_ind  <= 0;
		end
		else if(!cp_en)begin
			loc_tx_start_ind <= 0;
			loc_rx_start_ind <= 0;
			loc_rx_data_ind  <= 0;
		end
		else if(!loc_tx_start_ind && !loc_rx_start_ind && !loc_rx_data_ind) begin
			loc_tx_start_ind <= cp_in_tx_start_ind  ;
			loc_rx_start_ind <= cp_in_ev_rxstart_ind;
			loc_rx_data_ind  <= cp_in_ev_rxdata_ind ;
		end
		else if(loc_tx_data_avl) begin
			loc_tx_start_ind <= 0;
		end
		else if(count == 6'd19)begin
			loc_rx_start_ind <= 0;		
		end
		else if(loc_cp_rd)begin
			loc_rx_data_ind  <= 0;
		end
	end

	//Latching inputs from Tx_handler
	always @(posedge clk or `EDGE_OF_RESET) begin 
		if(`POLARITY_OF_RESET) begin
			loc_cp_tx_busy      <= 0; 
			loc_tx_txvec_format <= 0;
			loc_tx_rtscts_rate  <= 0;
			loc_tx_mpdu_ra      <= 0;
			loc_tx_data_avl     <= 0;
		end
		else if(!cp_en) begin
			loc_cp_tx_busy      <= 0; 
			loc_tx_txvec_format <= 0;
			loc_tx_rtscts_rate  <= 0;
			loc_tx_mpdu_ra      <= 0;
			loc_tx_data_avl     <= 0;
		end
		else if(!loc_cp_rx_busy)begin
			if(loc_tx_start_ind) begin
				loc_cp_tx_busy  <= 1'b1;
				loc_tx_data_avl <= 1'b1;
			end
			else if(loc_out_done)begin
				loc_cp_tx_busy  <= 1'b0;
				loc_tx_data_avl <= 1'b0;
			end
			loc_tx_txvec_format <= cp_in_tx_txvec_format;
			loc_tx_rtscts_rate  <= cp_in_tx_rtscts_rate ;
			loc_tx_mpdu_ra      <= cp_in_tx_mpdu_ra     ;
		end
	end
	
	//Txvec udpdate for Tx_handler request
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET) begin
			cp_out_tx_txvec_format      <= 0; 
			cp_out_tx_txvec_mcs         <= 0;
			cp_out_txvec_is_short_GI    <= 0;		
			cp_out_txvec_is_fec_ldpc    <= 0;
			cp_out_tx_txvec_L_datarate  <= 0;
			loc_cp_tx_rd                <= 0;
		end
		else if(!cp_en)begin
			cp_out_tx_txvec_format      <= 0; 
			cp_out_tx_txvec_mcs         <= 0;
			cp_out_txvec_is_short_GI    <= 0;		
			cp_out_txvec_is_fec_ldpc    <= 0;
			cp_out_tx_txvec_L_datarate  <= 0;
			loc_cp_tx_rd                <= 0;
		end
		else if(loc_tx_data_avl) begin
			if (loc_tx_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF)begin
				cp_out_tx_txvec_format      <= `UU_WLAN_FRAME_FORMAT_HT_GF; 
				cp_out_tx_txvec_mcs         <= loc_tx_rtscts_rate         ;
				cp_out_txvec_is_short_GI    <= 0                          ;		
				cp_out_txvec_is_fec_ldpc    <= 0                          ; 
			end
			else begin
				cp_out_tx_txvec_format     <= `UU_WLAN_FRAME_FORMAT_NON_HT;
				cp_out_tx_txvec_L_datarate <= loc_tx_rtscts_rate          ;
			end
			loc_cp_tx_rd <= 1'b1;
		end
		else 
			loc_cp_tx_rd <= 1'b0; 
	end
	
	// Latching inputs from Rx_handler
	always @(posedge clk or `EDGE_OF_RESET) begin 
		if (`POLARITY_OF_RESET)begin
			rxvec_format     <=0;
			rxvec_modulation <=0;
			rxvec_L_datarate <=0;
			rxvec_is_aggr    <=0;
			rxvec_mcs        <=0;
			mpdu_fc0         <=0;
			mpdu_dur         <=0;
			mpdu_ta          <=0;
			mpdu_ra          <=0;
			mpdu_bar_fc      <=0;
			mpdu_qos_ack     <=0;
			mpdu_bar_type    <=0;
			mpdu_tid         <=0;
			mpdu_scf         <=0;
			count            <=0;
			count2           <=0;
			loc_cp_rd        <=0;
			loc_cp_rx_busy   <=0;
		end
		else if(!cp_en) begin
			rxvec_format     <=0;
			rxvec_modulation <=0;
			rxvec_L_datarate <=0;
			rxvec_is_aggr    <=0;
			rxvec_mcs        <=0;
			mpdu_fc0         <=0;
			mpdu_dur         <=0;
			mpdu_ta          <=0;
			mpdu_ra          <=0;
			mpdu_bar_fc      <=0;
			mpdu_qos_ack     <=0;
			mpdu_bar_type    <=0;
			mpdu_tid         <=0;
			mpdu_scf         <=0;
			count            <=0;
			count2           <=0;
			loc_cp_rd        <=0;
			loc_cp_rx_busy   <=0;
		end
		else if(((cp_in_ev_rxstart_ind)||(loc_rx_start_ind))&&(!loc_cp_tx_busy)) begin	
			loc_cp_rx_busy <= 1'b1;
			if(cp_in_rx_data_val)begin
				case (count)
				6'h00:begin    rxvec_format	 	<= cp_in_rx_data[1:0];
							rxvec_modulation    <= cp_in_rx_data[4:2]; end
				6'h01:         rxvec_L_datarate    <= cp_in_rx_data[7:4];
				6'h06:	     rxvec_is_aggr	 	<= cp_in_rx_data[2];
				6'h07:	     rxvec_mcs		 	<= cp_in_rx_data[6:0];
				endcase
				count <= count + 1;
			end
			if(count == 6'd19)  //after 20 bytes of PLCP header
				count <= 0;
		end
		else if (((cp_in_ev_rxdata_ind)||(loc_rx_data_ind))&&(!loc_cp_tx_busy)&&(!loc_cp_rd)) begin
			if(cp_in_rx_data_val)begin
				if (count2 <=  6'h1B)begin
					case (count2)
					6'h00:  mpdu_fc0         <= cp_in_rx_data; 
					6'h02:  mpdu_dur[7 :0 ]  <= cp_in_rx_data;
					6'h03:  mpdu_dur[15:8 ]  <= cp_in_rx_data;
					6'h04:  mpdu_ra [7 :0 ]  <= cp_in_rx_data;
					6'h05:  mpdu_ra [15:8 ]  <= cp_in_rx_data;
					6'h06:  mpdu_ra [23:16]  <= cp_in_rx_data;
					6'h07:  mpdu_ra [31:24]  <= cp_in_rx_data;			      
					6'h08:  mpdu_ra [39:32]  <= cp_in_rx_data;
					6'h09:  mpdu_ra [47:40]  <= cp_in_rx_data;
					endcase
					if(!((mpdu_fc0 == (`IEEE80211_FC0_SUBTYPE_CTS | `IEEE80211_FC0_TYPE_CTL))||
					     (mpdu_fc0 == (`IEEE80211_FC0_SUBTYPE_ACK | `IEEE80211_FC0_TYPE_CTL))))begin
						case (count2)
						6'h0A:  mpdu_ta[7 :0 ]  <= cp_in_rx_data;
						6'h0B:  mpdu_ta[15:8 ]  <= cp_in_rx_data;
						6'h0C:  mpdu_ta[23:16]  <= cp_in_rx_data;
						6'h0D:  mpdu_ta[31:24]  <= cp_in_rx_data;			      
						6'h0E:  mpdu_ta[39:32]  <= cp_in_rx_data;
						6'h0F:  mpdu_ta[47:40]  <= cp_in_rx_data;
						endcase
						if(mpdu_fc0 == (`IEEE80211_FC0_SUBTYPE_QOS | `IEEE80211_FC0_TYPE_DATA)) begin
						  if(count2 == 'h18)begin 
						    mpdu_tid  <= cp_in_rx_data[3:0];
								mpdu_qos_ack    <= cp_in_rx_data[6:5];
								mpdu_scf <= cp_in_rx_seq_no;
								loc_cp_rd  <= 1'b1;
								count2     <= 0;
							end
							else
								count2<= count2 +1;
						end
						else if(mpdu_fc0 == (`IEEE80211_FC0_SUBTYPE_BAR | `IEEE80211_FC0_TYPE_CTL)) begin
							case (count2)
							6'h10:  begin mpdu_bar_fc [7 :0] <= cp_in_rx_data     ;
									    mpdu_qos_ack       <= cp_in_rx_data[0]  ;
									    mpdu_bar_type      <= cp_in_rx_data[2:1];end
							6'h11:  begin mpdu_bar_fc [15:8] <= cp_in_rx_data;
									    mpdu_tid           <= cp_in_rx_data[7:4];end
							6'h12:        mpdu_scf    [7 :0] <= cp_in_rx_data;
							6'h13:        mpdu_scf    [15:8] <= cp_in_rx_data;
							endcase
							if(count2 == 6'h13)begin
								loc_cp_rd  <= 1'b1;
								count2     <= 0;
							end
							else
								count2 <= count2 +1;
						end
						else if(mpdu_fc0 == (`IEEE80211_FC0_SUBTYPE_BA | `IEEE80211_FC0_TYPE_CTL)) begin
							case (count2)
							6'h10:  begin mpdu_qos_ack   <= cp_in_rx_data[0];
									    mpdu_bar_type  <= cp_in_rx_data[2:1];end
							6'h11:        mpdu_tid       <= cp_in_rx_data[7:4];
							6'h12:        mpdu_scf[7 :0] <= cp_in_rx_data;
							6'h13:        mpdu_scf[15:8] <= cp_in_rx_data;
							endcase
							if(count2 == 6'h1B)begin
								loc_cp_rd <= 1'b1;
								count2    <= 0;
							end
							else
								count2 <= count2 +1;
						end
						else begin
							if(count2 == 6'h0F) begin
								loc_cp_rd <= 1'b1;
								count2    <= 0;
							end
							else
								count2 <= count2+1;
						end
					end
					else begin
						if(count2 == 6'h09)begin
							loc_cp_rd <= 1'b1;
							count2    <= 0;
						end
						else
							count2 <= count2 +1;
					end
				end
			end
	end
		else if(cp_in_rx_aggr)begin
			loc_cp_rd      <= 'b1;
			loc_cp_rx_busy <= 'b1;
			mpdu_scf       <= cp_in_rx_seq_no;
		end
		else begin
			if(loc_out_done||loc_out_imdone)begin
				loc_cp_rd   <= 0;
				loc_cp_rx_busy <= 0;
			end
		end
	end
	
	//Processing for reply packet to be generated for Rx_handler request
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_out_dur_en            <= 0;
			cp_out_mpdu_dur          <= 0;
			cp_out_mpdu_fc           <= 0;
			cp_out_rxvec_format      <= 0;
			cp_out_rxvec_modulation  <= 0; 
			cp_out_rxvec_L_datarate  <= 0;
			cp_out_rxvec_mcs         <= 0;
			cp_out_ctrl_pkt_gen_en   <= 0;
			cp_out_duration          <= 0;
			cp_out_mpdu_ra           <= 0;
			cp_out_mpdu_ta           <= 0;
			cp_out_mpdu_bar_fc       <= 0;
			cp_out_mpdu_tid          <= 0;
			cp_out_mpdu_scf          <= 0;
			cp_out_bar_type          <= 0;
			loc_dur_avl              <= 0;
			loc_out_imdone           <= 0;
			loc_bitmap_en            <= 0;
			loc_bmp_update_done      <= 0;
			cp_out_implicit          <= 0;
			loc_bar_val              <= 0;
			cp_out_mpdu_bar_ssn      <= 0;
			cp_out_resp_length       <= 0;
			loc_bmp_in_mpdu_ta       <= 0;
			loc_bmp_in_mpdu_tid      <= 0;
			loc_bmp_in_mpdu_scf      <= 0;
			loc_bmp_in_dir           <= 0; 
			cp_out_sta_addr          <= 0;
			cp_out_sta_tid           <= 0;
			cp_out_sta_dir           <= 0;
			loc_sta_get_info1        <= 0;
			cp_out_pkt_subtype       <= `SUBTYPE_ERROR;     
			cp_out_pkt_gen_type      <= `SUBTYPE_ERROR;
			loc_bmp_upd              <= 0;
		end	
		else if(!cp_en)begin
			cp_out_dur_en            <= 0;
			cp_out_mpdu_dur          <= 0;
			cp_out_mpdu_fc           <= 0;
			cp_out_rxvec_format      <= 0;
			cp_out_rxvec_modulation  <= 0; 
			cp_out_rxvec_L_datarate  <= 0;
			cp_out_rxvec_mcs         <= 0;
			cp_out_ctrl_pkt_gen_en   <= 0;
			cp_out_duration          <= 0;
			cp_out_mpdu_ra           <= 0;
			cp_out_mpdu_ta           <= 0;
			cp_out_mpdu_bar_fc       <= 0;
			cp_out_mpdu_tid          <= 0;
			cp_out_mpdu_scf          <= 0;
			cp_out_bar_type          <= 0;
			loc_dur_avl              <= 0;
			loc_out_imdone           <= 0;
			loc_bitmap_en            <= 0;
			loc_bmp_update_done      <= 0;
			cp_out_implicit          <= 0;
			loc_bar_val              <= 0;
			cp_out_mpdu_bar_ssn      <= 0;
			cp_out_resp_length       <= 0;
			loc_bmp_in_mpdu_ta       <= 0;
			loc_bmp_in_mpdu_tid      <= 0;
			loc_bmp_in_mpdu_scf      <= 0;
			loc_bmp_in_dir           <= 0; 
			cp_out_sta_addr          <= 0;
			cp_out_sta_tid           <= 0;
			cp_out_sta_dir           <= 0;
			loc_sta_get_info1        <= 0;
			cp_out_pkt_subtype       <= `SUBTYPE_ERROR;     
			cp_out_pkt_gen_type      <= `SUBTYPE_ERROR;
			loc_bmp_upd              <= 0;
		end
		else if(loc_cp_rd) begin
			case(loc_f_type)
			`SUBTYPE_ERROR: begin
				loc_out_imdone   <= 1'b1;
			end
			`RTS_PKT: begin 
				if(!loc_dur_avl)begin
					cp_out_dur_en           <= 1'b1            ;
					cp_out_mpdu_dur         <= mpdu_dur        ;  
					cp_out_mpdu_fc          <= mpdu_fc0        ;
					cp_out_rxvec_format     <= rxvec_format    ;
					cp_out_rxvec_modulation <= rxvec_modulation; 
					cp_out_rxvec_L_datarate <= rxvec_L_datarate;
					cp_out_rxvec_mcs        <= rxvec_mcs       ;
					cp_out_resp_length      <= `UU_CTS_FRAME_LEN;
				end
				if(cp_in_dur_avl) begin
					loc_dur_avl             <= 1'b1            ;
					cp_out_dur_en           <= 1'b0            ;
					cp_out_ctrl_pkt_gen_en  <= 1'b1            ;  
					cp_out_pkt_gen_type     <= `CTS_PKT        ; 
					cp_out_duration         <= cp_in_dur_updated;
					cp_out_mpdu_ta          <= mpdu_ta         ;
				end
			end
			`CTS_PKT: begin
				loc_out_imdone <= 1'b1;
			end
			`ACK_PKT: begin
				loc_out_imdone <= 1'b1;
			end
			`BAR_PKT: begin 
				if(mpdu_qos_ack == `UU_WLAN_ACKPOLICY_NORMAL) begin    
					if(!loc_dur_avl)begin
						cp_out_dur_en           <= 1'b1            ;
						cp_out_mpdu_dur         <= mpdu_dur        ;  
						cp_out_mpdu_fc          <= mpdu_fc0        ;
						cp_out_rxvec_format     <= rxvec_format    ;
						cp_out_rxvec_modulation <= rxvec_modulation; 
						cp_out_rxvec_L_datarate <= rxvec_L_datarate;
						cp_out_rxvec_mcs        <= rxvec_mcs       ;
						cp_out_resp_length      <= `UU_ACK_FRAME_LEN;
					end
					if(cp_in_dur_avl) begin
						loc_dur_avl             <= 1'b1            ;
						cp_out_dur_en           <= 1'b0            ;
						cp_out_ctrl_pkt_gen_en  <= 1'b1            ;
						cp_out_pkt_gen_type     <= `ACK_PKT        ;
						cp_out_duration         <= cp_in_dur_updated;
						cp_out_mpdu_ta          <= mpdu_ta         ;
					end
				end
				else begin
					if(!loc_sta_info_avl)begin
						loc_sta_get_info1   <= 1'b1    ;
						cp_out_sta_addr     <= mpdu_ta ;
						cp_out_sta_tid      <= mpdu_tid;
						cp_out_sta_dir      <= 1'b0    ;
					end
					else if(!loc_sta_info_val)begin
						loc_out_imdone   <= 1'b1;
						loc_sta_get_info1 <= 1'b0;
					end
					else begin
						loc_sta_get_info1 <= 1'b0;
						if(!loc_dur_avl)begin
							cp_out_dur_en           <= 1'b1            ;
							cp_out_mpdu_dur         <= mpdu_dur        ;  
							cp_out_mpdu_fc          <= mpdu_fc0        ;
							cp_out_rxvec_format     <= rxvec_format    ;
							cp_out_rxvec_modulation <= rxvec_modulation; 
							cp_out_rxvec_L_datarate <= rxvec_L_datarate;
							cp_out_rxvec_mcs        <= rxvec_mcs       ;
							cp_out_resp_length      <= `UU_BA_BASIC_FRAME_LEN;
						end
						if(cp_in_dur_avl) begin
							loc_dur_avl             <= 1'b1            ;
							cp_out_dur_en           <= 1'b0            ;
							cp_out_ctrl_pkt_gen_en  <= 1'b1            ;
							cp_out_pkt_gen_type     <= `BA_PKT         ;
							cp_out_duration         <= cp_in_dur_updated;
							cp_out_mpdu_ra          <= mpdu_ra         ;
							cp_out_mpdu_ta          <= mpdu_ta         ;
							cp_out_mpdu_bar_fc      <= mpdu_bar_fc     ;
							cp_out_pkt_subtype      <= `BAR_PKT        ;     
							cp_out_mpdu_tid         <= mpdu_tid        ;
							cp_out_mpdu_scf         <= mpdu_scf        ;
							cp_out_bar_type         <= mpdu_bar_type   ;	
							cp_out_implicit         <= 1'b0            ;
						end
					end
				end
			end
			`BA_PKT: begin
				if(!loc_sta_info_avl)begin
					loc_sta_get_info1   <= 1'b1    ;
					cp_out_sta_addr     <= mpdu_ta ;
					cp_out_sta_tid      <= mpdu_tid;
					cp_out_sta_dir      <= 1'b1    ;
				end
				else if(!loc_sta_info_val)begin
					loc_sta_get_info1 <= 1'b0;
					loc_out_imdone   <= 1'b1;
				end
				else begin //update BITMAP in ba_frame_bitmap[]
					loc_sta_get_info1 <= 1'b0;
					if(!loc_bmp_upd_done)begin
						loc_bmp_upd <= 1'b1;//call for the bitmap update either COMP_BA or BASIC_BA
					end 
					else if(!cp_in_waiting_for_ack)begin 
						loc_bmp_upd <= 1'b0;
						loc_out_imdone   <= 1'b1;
					end
					else begin
						loc_bmp_upd <= 1'b0;
						if(mpdu_bar_type == `UU_WLAN_BAR_TYPE_COMP_BA)begin
							if(cp_in_tx_res_val)begin
								loc_out_imdone   <= 1'b1;
							end
						end
						else begin
							if(mpdu_qos_ack == `UU_WLAN_ACKPOLICY_NORMAL) begin 
								if(!loc_dur_avl)begin
									cp_out_dur_en           <= 1'b1            ;
									cp_out_mpdu_dur         <= mpdu_dur        ;  
									cp_out_mpdu_fc          <= mpdu_fc0        ;
									cp_out_rxvec_format     <= rxvec_format    ;
									cp_out_rxvec_modulation <= rxvec_modulation;
									cp_out_rxvec_L_datarate <= rxvec_L_datarate;
									cp_out_rxvec_mcs        <= rxvec_mcs       ;
									cp_out_resp_length      <= `UU_ACK_FRAME_LEN;
								end
								if(cp_in_dur_avl) begin
									loc_dur_avl             <= 1'b1            ;
									cp_out_dur_en           <= 1'b0            ;
									cp_out_ctrl_pkt_gen_en  <= 1'b1            ;
									cp_out_pkt_gen_type     <= `ACK_PKT        ;
									cp_out_duration         <= cp_in_dur_updated;
									cp_out_mpdu_ra          <= mpdu_ra         ;
								end
							end
							else begin
								loc_out_imdone   <= 1'b1;
							end
						end
					end
				end
			end
			`DATA_ACK_PKT:begin 
				if(!loc_dur_avl)begin
					cp_out_dur_en           <= 1'b1            ;					
					cp_out_mpdu_dur         <= mpdu_dur        ;  
					cp_out_mpdu_fc          <= mpdu_fc0        ;
					cp_out_rxvec_format     <= rxvec_format    ;
					cp_out_rxvec_modulation <= rxvec_modulation; 
					cp_out_rxvec_L_datarate <= rxvec_L_datarate;
					cp_out_rxvec_mcs        <= rxvec_mcs       ;
					cp_out_resp_length      <= `UU_ACK_FRAME_LEN;
				end
				if(cp_in_dur_avl) begin
					loc_dur_avl             <= 1'b1            ;
					cp_out_dur_en           <= 1'b0            ;						  
					cp_out_ctrl_pkt_gen_en  <= 1'b1            ;
					cp_out_pkt_gen_type     <= `ACK_PKT        ;
					cp_out_duration         <= cp_in_dur_updated;
					cp_out_mpdu_ta          <= mpdu_ta         ;
				end
				if(!loc_bmp_update_done) begin
					loc_bitmap_en       <= 1'b1    ;
					cp_out_sta_addr     <= mpdu_ta ;
					cp_out_sta_tid      <= mpdu_tid;
					cp_out_sta_dir      <= 0;
					loc_bmp_in_mpdu_scf <= mpdu_scf;
					//wait for SB update with received bitmap
					if(loc_bmp_res_en)begin
						loc_bitmap_en       <= 1'b0;
						loc_bmp_update_done <= 1'b1;
					end
				end

			end	 
			`DATA_BA_PKT: begin
				if(!loc_bmp_update_done) begin
					loc_bitmap_en       <= 1'b1    ;
					cp_out_sta_addr     <= mpdu_ta ;
					cp_out_sta_tid      <= mpdu_tid;
					cp_out_sta_dir      <= 0;
					loc_bmp_in_mpdu_scf <= mpdu_scf;
					if(loc_bmp_res_en)begin
						loc_bmp_update_done <= 1'b1;
						loc_bitmap_en  <= 1'b0;
						loc_out_imdone <= 1'b1;
					end
				end
			end
			`DATA_IMBA_PKT:begin //ImplicitBA
				if(!loc_bmp_update_done) begin
					loc_bitmap_en       <= 1'b1    ;
					cp_out_sta_addr     <= mpdu_ta ;
					cp_out_sta_tid      <= mpdu_tid;
					cp_out_sta_dir      <= 0;
					loc_bmp_in_mpdu_scf <= mpdu_scf;	
					if(loc_bmp_res_en)begin
						loc_bmp_update_done <= 1'b1;
						loc_bitmap_en <= 1'b0;
					end
				end
				else begin
					if(!loc_dur_avl)begin
						cp_out_dur_en           <= 1'b1            ;
						cp_out_mpdu_dur         <= mpdu_dur        ;  					
						cp_out_mpdu_fc          <= mpdu_fc0        ;
						cp_out_rxvec_format     <= rxvec_format    ;
						cp_out_rxvec_modulation <= rxvec_modulation; 
						cp_out_rxvec_L_datarate <= rxvec_L_datarate;
						cp_out_rxvec_mcs        <= rxvec_mcs       ;
						cp_out_resp_length      <= `UU_BA_BASIC_FRAME_LEN;
					end
					if(cp_in_dur_avl) begin
						loc_dur_avl             <= 1'b1            ;
						cp_out_dur_en           <= 1'b0            ;
						cp_out_ctrl_pkt_gen_en  <= 1'b1            ;
						cp_out_pkt_gen_type     <= `BA_PKT         ;
						cp_out_duration         <= cp_in_dur_updated;
						cp_out_mpdu_ra          <= mpdu_ra         ;
						cp_out_mpdu_ta          <= mpdu_ta         ;
						cp_out_implicit         <= 1'b1            ;
					end
				end
			end
			`MGT_PKT:begin 
				if(!loc_dur_avl)begin
					cp_out_dur_en           <= 1'b1            ;
					cp_out_mpdu_dur         <= mpdu_dur        ;   
					cp_out_mpdu_fc          <= mpdu_fc0        ;
					cp_out_rxvec_format     <= rxvec_format    ;
					cp_out_rxvec_modulation <= rxvec_modulation; 
					cp_out_rxvec_L_datarate <= rxvec_L_datarate;
					cp_out_rxvec_mcs        <= rxvec_mcs       ;
					cp_out_resp_length      <= `UU_ACK_FRAME_LEN;
				end
				if(cp_in_dur_avl) begin
					loc_dur_avl             <= 1'b1            ;
					cp_out_dur_en           <= 1'b0            ;
					cp_out_ctrl_pkt_gen_en  <= 1'b1            ;
					cp_out_pkt_gen_type     <= `ACK_PKT        ;
					cp_out_duration         <= cp_in_dur_updated;
					cp_out_mpdu_ra          <= mpdu_ra         ;
					cp_out_mpdu_ta          <= mpdu_ta         ;
				end
			end
			`RESERVED:begin
				if(!loc_dur_avl)begin
					cp_out_dur_en           <= 1'b1            ;
					cp_out_mpdu_dur         <= mpdu_dur        ;   
					cp_out_mpdu_fc          <= mpdu_fc0        ;
					cp_out_rxvec_format     <= rxvec_format    ;
					cp_out_rxvec_modulation <= rxvec_modulation; 
					cp_out_rxvec_L_datarate <= rxvec_L_datarate;
					cp_out_rxvec_mcs        <= rxvec_mcs       ;
				end
				if(cp_in_dur_avl) begin
					loc_dur_avl             <= 1'b1            ;
					cp_out_dur_en           <= 1'b0            ;
					cp_out_ctrl_pkt_gen_en  <= 1'b1            ;
					cp_out_pkt_gen_type     <= `BA_PKT         ;
					cp_out_duration         <= cp_in_dur_updated;
					cp_out_mpdu_ra          <= mpdu_ra         ;
					cp_out_implicit         <= 1'b1            ;
				end
			end
			endcase
		end
		else if(loc_cp_tx_rd)begin
			if(cp_in_tx_self_cts) begin
				cp_out_ctrl_pkt_gen_en <= 1'b1          ;
				cp_out_pkt_gen_type    <= `SELF_CTS_PKT ;
				cp_out_duration        <= 0             ;
				cp_out_mpdu_ra         <= loc_tx_mpdu_ra;
			end
			else begin
				cp_out_ctrl_pkt_gen_en <= 1'b1    ;
				cp_out_pkt_gen_type    <= `RTS_PKT;
				cp_out_duration        <= 0       ;
				cp_out_mpdu_ra         <= loc_tx_mpdu_ra;
			end
		end
		else if(cp_in_bar_valid||loc_bar_val) begin
			if(loc_out_done) 
				loc_bar_val       <= 1'b1              ;
			else
				loc_bar_val       <= 1'b1              ;
			cp_out_mpdu_tid        <= cp_in_bar_tid     ;
			cp_out_mpdu_bar_ssn    <= cp_in_bar_scf     ;
			cp_out_bar_type        <= cp_in_bar_bartype ;	
			cp_out_ctrl_pkt_gen_en <= 1'b1              ;
			cp_out_pkt_gen_type    <= `BAR_PKT          ;
			cp_out_duration        <= cp_in_bar_duration;
			cp_out_mpdu_ra         <= cp_in_bar_address ;
		end
		else begin
			cp_out_ctrl_pkt_gen_en <= 1'b0;
			loc_dur_avl            <= 1'b0;
			loc_out_imdone         <= 1'b0;
			loc_bmp_update_done    <= 1'b0;
		end
	end 

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_sta_info_val <= 0;
		end
		else if(!cp_en)begin
			loc_sta_info_val <= 0;
		end
		else if(cp_in_sta_info_val)begin
		     if(cp_in_sta_info)
				loc_sta_info_val <= 1;
			else 
				loc_sta_info_val <= 0;
		end
		else if(loc_out_done | loc_out_imdone)begin
			loc_sta_info_val <= 0;
		end
	end	
	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_sta_info_avl <= 0;
		end
		else if(!cp_en)begin
			loc_sta_info_avl <= 0;
		end
		else if(cp_in_sta_info_val)begin
			loc_sta_info_avl <= 1;
		end
		else if(loc_out_done | loc_out_imdone)begin
			loc_sta_info_avl <= 0;
		end
	end
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET) begin
			cp_out_rx_res_en <= 0;
			cp_out_rx_res    <= `UU_FAILURE;
		end
		else if(!cp_en)begin
			cp_out_rx_res_en <= 0;
			cp_out_rx_res    <= `UU_FAILURE;
		end
		else if(loc_cp_rd)begin
			case(loc_f_type)
				`SUBTYPE_ERROR	:begin
					if(loc_out_imdone)begin
						cp_out_rx_res_en <= 1'b1       ;
						cp_out_rx_res    <= `UU_SUCCESS;
					end
				end
				`RTS_PKT :begin
					if (loc_out_done) begin
						cp_out_rx_res_en <= 1'b1;
						cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_SEND_RESP; 
					end
				end
				`CTS_PKT :begin 	
					if(loc_out_imdone)begin
						cp_out_rx_res_en   <= 1'b1;
						if (!cp_in_waiting_for_cts)
							cp_out_rx_res <= `UU_WLAN_RX_HANDLER_FRAME_INVALID;
						else	
							cp_out_rx_res <= `UU_WLAN_RX_HANDLER_CTS_RCVD;
					end
				end
				`ACK_PKT :begin
					if(loc_out_imdone)begin
						cp_out_rx_res_en   <= 1'b1;
						if (!cp_in_waiting_for_ack)
							cp_out_rx_res <= `UU_WLAN_RX_HANDLER_FRAME_INVALID;
						else
							cp_out_rx_res <= `UU_WLAN_RX_HANDLER_ACK_RCVD;
					end
				end	
				`BAR_PKT :begin	
					if(mpdu_qos_ack == `UU_WLAN_ACKPOLICY_NORMAL) begin    
						if (loc_out_done) begin
							cp_out_rx_res_en <= 1'b1;
							cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_SEND_RESP; 
						end
					end
					else begin
						if(loc_sta_info_avl && !loc_sta_info_val)begin
							cp_out_rx_res_en <= 1'b1;
							cp_out_rx_res    <= `UU_BA_SESSION_INVALID;
						end
						else if(loc_out_done)begin
							cp_out_rx_res_en <= 1'b1;
							cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_SEND_RESP;
						end
					end
				end
				`BA_PKT :begin
					if(loc_sta_info_avl && !loc_sta_info_val)begin
						if(loc_out_imdone)begin
							cp_out_rx_res_en <= 1'b1       ;
							cp_out_rx_res    <= `UU_BA_SESSION_INVALID;
						end
					end
					else if(loc_bmp_upd_done)begin
						if(!cp_in_waiting_for_ack)begin
							if(loc_out_imdone)begin
								cp_out_rx_res_en <= 1'b1;
								cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_FRAME_INVALID;
							end
						end
						else if(mpdu_bar_type == `UU_WLAN_BAR_TYPE_COMP_BA)begin
							if(cp_in_tx_res_val)begin
								if(cp_in_tx_res == `UU_SUCCESS)begin
									if(loc_out_imdone)begin
										cp_out_rx_res_en <= 1'b1;
										cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_BA_RCVD;
									end
								end
								else begin
									if(loc_out_imdone)begin
										cp_out_rx_res_en <= 1'b1;
										cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_BA_RCVD_RETRY_FAILED_PKTS;
									end
								end
							end
						end
						else if(mpdu_qos_ack == `UU_WLAN_ACKPOLICY_NORMAL) begin 
							if(loc_out_done) begin
								cp_out_rx_res_en <= 1'b1;
								cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_SEND_RESP;
							end
						end
						else begin
							if(loc_out_imdone)begin
								cp_out_rx_res_en <= 1'b1;
								cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_BA_RCVD;
							end
						end
					end
				end
				`DATA_ACK_PKT :begin  	
					if (loc_out_done) begin
						cp_out_rx_res_en <= 1'b1;
						cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_SEND_RESP; 
					end
				end
				`DATA_BA_PKT :begin 	
					if(loc_bmp_res_en) begin
						cp_out_rx_res_en <= 1'b1;
						if(loc_bmp_res == `UU_FAILURE) 
							cp_out_rx_res  <= `UU_FAILURE; 
						else if(loc_bmp_res == `UU_BA_SESSION_INVALID) 
							cp_out_rx_res  <= `UU_BA_SESSION_INVALID;
						else 
							cp_out_rx_res  <= `UU_SUCCESS;
					end
					else begin
						cp_out_rx_res_en <= 1'b0;
						cp_out_rx_res    <= `UU_FAILURE; 
					end
				end
				`DATA_IMBA_PKT :begin 	
					if(!loc_bmp_update_done) begin
						if(loc_bmp_res_en)begin
							if(loc_bmp_res == `UU_FAILURE) begin
								cp_out_rx_res_en <= 1'b1;
								cp_out_rx_res    <= `UU_FAILURE; 
							end
							else if(loc_bmp_res == `UU_BA_SESSION_INVALID) begin
								cp_out_rx_res_en <= 1'b1;
								cp_out_rx_res    <= `UU_BA_SESSION_INVALID;
							end
						end
					end
					else if (loc_out_done) begin
						cp_out_rx_res_en <= 1'b1;
						cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_SEND_RESP; 
					end
					else begin
						cp_out_rx_res_en <= 0;
						cp_out_rx_res    <= 0;
					end
				end
				`MGT_PKT :begin 	
					if (loc_out_done) begin
						cp_out_rx_res_en <= 1'b1;
						cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_SEND_RESP; 
					end
				end
				`RESERVED :begin
					if (loc_out_done) begin
						cp_out_rx_res_en <= 1'b1;
						cp_out_rx_res    <= `UU_WLAN_RX_HANDLER_SEND_RESP; 
					end
				end
				default :begin
					cp_out_rx_res_en <= 0;
					cp_out_rx_res    <= 0;
				end
			endcase	
		end
		else begin
			cp_out_rx_res_en <= 0;
			cp_out_rx_res    <= `UU_FAILURE;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin	
			cp_out_tx_res_val <= 0;
			cp_out_tx_res     <= `UU_FAILURE;
		end
		else if(!cp_en)begin
			cp_out_tx_res_val <= 0;
			cp_out_tx_res     <= `UU_FAILURE;
		end 
		else	if(loc_out_done)begin
			cp_out_tx_res_val <= 1;
			cp_out_tx_res     <= `UU_SUCCESS;
		end
		else begin 
			cp_out_tx_res_val <= 0;
			cp_out_tx_res     <= `UU_FAILURE;
		end
	end
			

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_reg_imba_pending <= 0;
		end
		else if(!cp_en)begin
			loc_reg_imba_pending <= 0;
		end 
		else if(loc_cp_rd)begin
			case(loc_f_type)
				`DATA_ACK_PKT :begin
					if(cp_in_sta_info_val && cp_in_sta_info)
						loc_reg_imba_pending <= 1;  
				end
				`DATA_BA_PKT  :begin
					if(cp_in_sta_info_val && cp_in_sta_info)
						loc_reg_imba_pending <= 1; 
				end
				`RESERVED     :begin
					if(loc_reg_imba_pending && (cp_in_sta_info_val && cp_in_sta_info))
						loc_reg_imba_pending <= 0; 				
				end
			endcase
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET) begin
			loc_out_done <= 0;
		end
		else if(!cp_en)begin
			loc_out_done <= 0;
		end
		else if(loc_f_type == `DATA_BA_PKT)begin
			loc_out_done <= 0;
		end
		else if((loc_cp_rd||loc_cp_tx_rd) && cp_out_ctrl_pkt_gen_en)begin
			if(cp_out_packet_length != 0)begin
				if(loc_out_count == cp_out_packet_length - 'h4)
					loc_out_done <= 1;
				else 
					loc_out_done <= 0;
			end
		end
		else begin
			loc_out_done <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET) begin
			loc_out_count <= 0;
		end
		else if(!cp_en)begin
			loc_out_count <= 0;
		end
		else if(cp_out_tx_data_val)begin
			loc_out_count <= loc_out_count +1;
		end
		else if(loc_out_count == cp_out_packet_length - 'h4)begin
			loc_out_count <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_bmp_offset <= 0;
		end
		else if(!cp_en)begin
			loc_bmp_offset <= 0;
		end 
		else if(cp_in_sta_info_val) begin
			if(cp_in_sta_info)
				loc_bmp_offset <= cp_in_sta_offset;
			else 
				loc_bmp_offset <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_sta_get_info1_dly <= 0;
		end
		else if(!cp_en)begin
			loc_sta_get_info1_dly <= 0;
		end
		else begin
			loc_sta_get_info1_dly <= loc_sta_get_info1;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_bmp_upd_done <= 0;
		end
		else if(!cp_en)begin
			loc_bmp_upd_done <= 0;
		end
		else if(loc_bmp_upd)begin
			if(addr_count == 'd128)
				loc_bmp_upd_done <= 1 ;
		end
		else if(loc_out_imdone||loc_out_done)begin
			loc_bmp_upd_done <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_ba3_ses_en <= 0;
		end
		else if(!cp_en)begin
			loc_ba3_ses_en <= 0;
		end
		else if(loc_f_type == `BA_PKT)begin
			loc_ba3_ses_en <= 1 ;
		end
		else begin
			loc_ba3_ses_en <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_ba3_ses_wen <= 0;
		end
		else if(!cp_en)begin
			loc_ba3_ses_wen <= 0;
		end
		else if((loc_f_type == `BA_PKT) && loc_bmp_upd)begin
			if(addr_count<=128)
				loc_ba3_ses_wen <= (1<<(addr_count%4))|((1<<(addr_count%4)+1));
			else 
				loc_ba3_ses_wen <= 0;
		end
		else begin
			loc_ba3_ses_wen <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_ba3_ses_addr <= 0;
		end
		else if(!cp_en)begin
			loc_ba3_ses_addr <= 0;
		end
		else if((loc_f_type == `BA_PKT)&&(loc_bmp_upd))begin
			loc_ba3_ses_addr <= (addr_count>>2) + cp_in_sta_offset + `BA_FRAME_BITMAP_OFFSET ;
		end
		else begin
			loc_ba3_ses_addr <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_ba3_ses_data <= 0;
		end
		else if(!cp_en)begin
			loc_ba3_ses_data <= 0;
		end
		else if((loc_f_type == `BA_PKT)&&loc_bmp_upd)begin
			if(addr_count<128)begin
				if((addr_count>>1) %2 == 0)
					loc_ba3_ses_data <= loc_comp_bitmap[addr_count>>1];
				else 
					loc_ba3_ses_data <= ((loc_comp_bitmap[addr_count>>1])<<'d16);
			end
			else
				loc_ba3_ses_data <= (mpdu_scf>>4);
		end
		else begin
			loc_ba3_ses_data <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			addr_count <= 0;
		end
		else if(!cp_en)begin
			addr_count <= 0;
		end
		else if((loc_f_type == `BA_PKT)&&loc_bmp_upd)begin
			if(loc_bmp_upd_done)
				addr_count <= 0;
			else 
				addr_count <= addr_count + 2;
		end
		else begin
			addr_count <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_out_tx_seqno <= 0;
		end
		else if(!cp_en)begin
			cp_out_tx_seqno <= 0;
		end
		else if((loc_f_type == `BA_PKT)&&loc_bmp_upd)begin
			if(loc_comp_bitmap[addr_count>>1])
				cp_out_tx_seqno <= ((mpdu_scf>>4)+(addr_count>>1));
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_out_tx_ac <= 0;
		end
		else if(!cp_en)begin
			cp_out_tx_ac <= 0;
		end
		else if((loc_f_type == `BA_PKT)&&loc_bmp_upd)begin
			cp_out_tx_ac <= ac_from_tid(mpdu_tid);
		end
		else begin
			cp_out_tx_ac <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_out_tx_upd_mpdu_status <= 0;
		end
		else if(!cp_en)begin
			cp_out_tx_upd_mpdu_status <= 0;
		end
		else if((loc_f_type == `BA_PKT)&&loc_bmp_upd)begin
			if(loc_comp_bitmap[addr_count>>1])
				cp_out_tx_upd_mpdu_status <= 1;
			else 
				cp_out_tx_upd_mpdu_status <= 0;
		end
		else begin
			cp_out_tx_upd_mpdu_status <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_comp_bitmap <= 0;
		end
		else if(!cp_en)begin
			loc_comp_bitmap <= 0;
		end
		else if(mpdu_fc0 == (`IEEE80211_FC0_SUBTYPE_BA | `IEEE80211_FC0_TYPE_CTL))begin
			if(mpdu_bar_type == `UU_WLAN_BAR_TYPE_COMP_BA)begin
				case(count2)
					6'h14: loc_comp_bitmap[7 :0 ]<= cp_in_rx_data;
					6'h15: loc_comp_bitmap[15:8 ]<= cp_in_rx_data;
					6'h16: loc_comp_bitmap[23:16]<= cp_in_rx_data;
					6'h17: loc_comp_bitmap[31:24]<= cp_in_rx_data;
					6'h18: loc_comp_bitmap[39:32]<= cp_in_rx_data;
					6'h19: loc_comp_bitmap[47:40]<= cp_in_rx_data;
					6'h1A: loc_comp_bitmap[55:48]<= cp_in_rx_data;
					6'h1B: loc_comp_bitmap[63:56]<= cp_in_rx_data;
				endcase	
			end
		end
		else begin
			loc_comp_bitmap <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			cp_out_tx_get_ampdu_status <= 0;
		end
		else if(!cp_en)begin
			cp_out_tx_get_ampdu_status <= 0;
		end
		else if(loc_f_type == `BA_PKT)
			if(cp_in_waiting_for_ack && loc_bmp_upd_done && (mpdu_bar_type == `UU_WLAN_BAR_TYPE_COMP_BA) && !cp_in_tx_res_val && !loc_out_imdone)begin
			cp_out_tx_get_ampdu_status <= 1;
		end
		else begin
			cp_out_tx_get_ampdu_status <= 0;
		end
	end

		

	/*---------------------------------------------------------------------------------------------------------*/
endmodule
//EOF
	/*
	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_basic_bitmap[0] <= 0;
		end
		else if(!cp_en)begin
			loc_basic_bitmap[0] <= 0;
		end
		else if(mpdu_fc0 == (`IEEE80211_FC0_SUBTYPE_BA | `IEEE80211_FC0_TYPE_CTL))begin
			if(mpdu_bar_type != `UU_WLAN_BAR_TYPE_COMP_BA)begin
				case(count2)
					6'h14: loc_basic_bitmap[0]   <= cp_in_rx_data;
					6'h15: loc_basic_bitmap[0]   <= (cp_in_rx_data<<8)|loc_basic_bitmap[0];
				endcase
			end
		end
		else begin
			loc_basic_bitmap[0] <= 0;
		end
	end
	*/
