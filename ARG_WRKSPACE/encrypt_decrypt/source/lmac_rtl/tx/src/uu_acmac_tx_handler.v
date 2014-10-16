`include "../../inc/defines.h"

module uu_acmac_tx_handler(
	input                clk                      ,//System Clock
	input                rst_n                    ,//System Reset
	input                tx_en                    ,//Tx Enable
	//input requests from CAP
	input                tx_in_cap_data_req       ,//cap_out_ev_txop_txdata_req
	input                tx_in_cap_start_req      ,//cap_out_ev_txop_txstart_req
        input                tx_in_cap_send_ack       ,//cap_out_send_ack_frame
	input                tx_in_cap_more_data_req  ,//cap_out_ev_txsendmore_data_req
	input                tx_in_cap_fb_update      ,//cap_out_fb_update
	input                tx_in_cp_upd_mpdu_status ,//cp_out_tx_upd_mpdu_status
	input                tx_in_cp_get_ampdu_status,//cp_out_tx_get_ampdu_status
	input                tx_in_clear_ampdu        ,//clear ampdu indication
	input                tx_in_clear_mpdu         ,//clear mpdu indication
        input                tx_in_cap_ctrl_start_req ,//cap rx_ctrl start req	
	input                tx_in_cap_ctrl_data_req  ,//cap rx_ctrl data req
	//inputs from CAP 
	input                tx_in_cap_retry          ,//cap_out_frame_info_retry
	input        [2 :0]  tx_in_cap_ch_bw          ,//cap_out_channel_bw
	input        [2 :0]  tx_in_cap_ac             ,//cap_out_txop_owner_ac
	input        [15:0]  tx_in_cap_ac0_tx_nav     ,//cap_out_ac0_txnav_value
	input        [15:0]  tx_in_cap_ac1_tx_nav     ,//cap_out_ac1_txnav_value
	input        [15:0]  tx_in_cap_ac2_tx_nav     ,//cap_out_ac2_txnav_value
	input        [15:0]  tx_in_cap_ac3_tx_nav     ,//cap_out_ac3_txnav_value
	input        [7 :0]  tx_in_cap_ac0_src        ,//cap_out_tx_ac0_src
	input        [7 :0]  tx_in_cap_ac1_src        ,//cap_out_tx_ac0_src
	input        [7 :0]  tx_in_cap_ac2_src        ,//cap_out_tx_ac0_src
	input        [7 :0]  tx_in_cap_ac3_src        ,//cap_out_tx_ac0_src
	input        [7 :0]  tx_in_cap_ac0_lrc        ,//cap_out_tx_ac0_src
	input        [7 :0]  tx_in_cap_ac1_lrc        ,//cap_out_tx_ac0_src
	input        [7 :0]  tx_in_cap_ac2_lrc        ,//cap_out_tx_ac0_src
	input        [7 :0]  tx_in_cap_ac3_lrc        ,//cap_out_tx_ac0_src
	//input register values 
	input        [15:0]  tx_in_BSSBasicRateSet    ,//Register input
	input        [15:0]  tx_in_SIFS_timer_value   ,//Register input 	
	input        [15:0]  tx_in_TXOP_limit_ac0     ,//Register input
	input        [15:0]  tx_in_TXOP_limit_ac1     ,//Register input
	input        [15:0]  tx_in_TXOP_limit_ac2     ,//Register input
	input        [15:0]  tx_in_TXOP_limit_ac3     ,//Register input
	input                tx_in_QOS_mode           ,//Register input
	input        [15:0]  tx_in_RTS_Threshold      ,//Register input
	input        [15:0]  tx_in_Long_Retry_Count   ,//Register input 
	//interface with CP
	input        [1 :0]  tx_in_cp_ac              ,//cap_out_txop_owner_ac
	output               tx_out_cp_start_ind      ,//cp_in_tx_start_ind    
	output  reg  [1 :0]  tx_out_cp_txvec_format   ,//cp_in_tx_txvec_format 
	output  reg  [3 :0]  tx_out_cp_rtscts_rate    ,//cp_in_tx_rtscts_rate  
	output  reg  [47:0]  tx_out_cp_mpdu_ra        ,//cp_in_tx_mpdu_ra  
	input        [7 :0]  tx_in_cp_ctrl_fr_len     ,//control frame length generated 

	input        [15:0]  tx_in_cp_seqno           ,//seqno of the frame from cp
	output  reg          tx_out_cp_res_val        ,//response valid from cp
	output  reg  [31:0]  tx_out_cp_res            ,//response from cp
	input                tx_in_cp_res_val         ,//cp_out_tx_res_val
	input        [31:0]  tx_in_cp_res             ,//cp_out_tx_res
	//Station & BA Management interface 
	input                tx_in_sta_info_val       ,//response valid from sta regarding ba
	input                tx_in_sta_info           ,//response from sta regarding ba
	input        [13:0]  tx_in_sta_offset         ,//ba offset given by sta
	output               tx_out_sta_get_info      ,//request for ba avalability to sta
	output       [47:0]  tx_out_sta_addr          ,//MAC address to sta
	output       [3 :0]  tx_out_sta_tid           ,//TID to sta
	output               tx_out_sta_dir           ,//direction to sta
	output       [11:0]  tx_out_sta_seqno_ac0     ,//ac0 Sequence number to sta
	output       [11:0]  tx_out_sta_seqno_ac1     ,//ac1 Sequence number to sta
	output       [11:0]  tx_out_sta_seqno_ac2     ,//ac2 Sequence number to sta
	output       [11:0]  tx_out_sta_seqno_ac3     ,//ac3 Sequence number to sta
	output       [7 :0]  tx_out_sta_bitmap_ac0    ,//ac0 bitmap to sta
	output       [7 :0]  tx_out_sta_bitmap_ac1    ,//ac1 bitmap to sta
	output       [7 :0]  tx_out_sta_bitmap_ac2    ,//ac2 bitmap to sta
	output       [7 :0]  tx_out_sta_bitmap_ac3    ,//ac3 bitmap to sta         
	output       [19:0]  tx_out_sta_frame_len     ,//recent frame length to sta 
	//sta_ba_info memory interface (depth : 138 used * 40)(width : 16 bits)
	input        [31:0]  tx_in_sta_ba_data        ,
	output       [14:0]  tx_out_sta_ba_addr       ,
	output       [31:0]  tx_out_sta_ba_data       ,
	output               tx_out_sta_ba_en         ,
	output       [3 :0]  tx_out_sta_ba_wen        ,
	//tx_frame memory (depth : 6044 used)(width : 32 bits)
	output  reg  [1 :0]  tx_out_frame_ac          ,//select among available 4 ACs
	input        [31:0]  tx_in_frame_data         ,
	output  reg  [31:0]  tx_out_frame_data        ,
	output  reg  [13:0]  tx_out_frame_addr        ,
	output  reg          tx_out_frame_en          ,
	output  reg          tx_out_frame_wen         ,
	//tx_ctl_frame_info_g Memory interface (depth : 152 used)(width : 8 bits)
	input        [7 :0]  tx_in_ctrl_data          , 
	output  reg  [7 :0]  tx_out_ctrl_data         ,
	output  reg  [7 :0]  tx_out_ctrl_addr         ,
	output  reg          tx_out_ctrl_en           ,
	output  reg          tx_out_ctrl_wen          ,
	//vht_mcs_table_g memory interface (depth : 720 used)(width : 40 bits)
	output               tx_out_vht_mcs_en        ,
	output       [9 :0]  tx_out_vht_mcs_addr      ,
	input        [39:0]  tx_in_vht_mcs_data       ,
	//ht_mcs_table_g memory interface (depth : 160 used)(width : 40 bits)
	output               tx_out_ht_mcs_en         ,
	output       [7 :0]  tx_out_ht_mcs_addr       ,
	input        [39:0]  tx_in_ht_mcs_data        ,
	//lrate_table_g memory interface (depth : 24 used)(width : 40 bits)
	output               tx_out_lrate_en          ,
	output       [5 :0]  tx_out_lrate_addr        ,
	input        [39:0]  tx_in_lrate_data         ,
	//read and base pointers inputs from circular buffer 
	input        [13:0]  tx_in_buf_base_ptr_ac0       ,//BASE pointer from buffer//vibha
	input        [13:0]  tx_in_buf_base_ptr_ac1       ,//BASE pointer from buffer//vibha
	input        [13:0]  tx_in_buf_base_ptr_ac2       ,//BASE pointer from buffer//vibha
	input        [13:0]  tx_in_buf_base_ptr_ac3       ,//BASE pointer from buffer//vibha
	input        tx_in_buf_rd_ac0,//vibha
	input        tx_in_buf_rd_ac1,//vibha
	input        tx_in_buf_rd_ac2,//vibha
	input        tx_in_buf_rd_ac3,//vibha
	output       [19:0]  tx_out_buf_pkt_len       ,//packet length transmitted to BUFFER
	// CRC32 interface
	input                tx_in_crc_avl            ,//CRC avlability  
	input        [31:0]  tx_in_crc                ,//CRC 
	output  reg  [15:0]  tx_out_crc_len           ,//length of data
	output  reg          tx_out_crc_en            ,//CRC en
	output  reg          tx_out_crc_data_val      ,//data valid for CRC calculation
	output  reg  [7 :0]  tx_out_crc_data          ,//data for CRC calculation
	//output status to CAP
	output  reg  [1 :0]  tx_out_cap_ack_policy    ,//added for multicast
	output       [7 :0]  tx_out_cap_mpdu_fc0      ,//added for multicast
      output       [7 :0]  tx_out_cap_mpdu_fc1      ,//added for multicast
      
	output  reg          tx_out_cap_res_val       ,//response valid to CAP
	output  reg          tx_out_cap_data_end_confirm,//tx data end confirm to CAP 
	output  reg  [31:0]  tx_out_cap_res           ,//response to CAP
	//output to PHY
	input     		         tx_in_phy_data_req	      ,//Phy data request to TX
	output               tx_out_phy_start_req     ,//Start request to PHY
	output               tx_out_phy_data_req      ,//Data request to PHY
	output  reg          tx_out_phy_frame_val     ,//frame valid to PHY
	output  reg  [7 :0]  tx_out_phy_frame          //frame to PHY (byte)
	);
	
	/* Registers internal to the module 
	............................................................*/
	reg           loc_data_req          ;
	reg           loc_start_req         ;
        reg           loc_ack_req           ;
	reg           loc_more_data_req     ;
	reg   [19:0]  loc_fI_frame_len      ;
	reg   [7 :0]  loc_aggr_count        ;
	reg   [7 :0]  loc_fr_aggr_count     ;
	reg   [3 :0]  loc_fr_rtscts_rate    ;
	reg   [1 :0]  loc_txvec_format      ;
	reg   [1 :0]  loc_txvec_stbc        ;
	reg   [6 :0]  loc_txvec_mcs         ;
	reg   [2 :0]  loc_txvec_ch_bndwdth  ;
	reg   [11:0]  loc_txvec_num_ext_ss  ;
	reg   [3 :0]  loc_txvec_L_datarate  ;
	reg   [11:0]  loc_txvec_L_length    ;
	reg   [15:0]  loc_txvec_ht_length   ;
	reg   [2 :0]  loc_txvec_modulation  ;
	reg           loc_txvec_is_long_pre ;
	reg           loc_txvec_is_short_GI ;
	reg           loc_txvec_is_aggr     ;
	reg   [7 :0]  loc_mpdu_fc0          ;
	reg   [7 :0]  loc_mpdu_fc1          ;
	reg   [47:0]  loc_mpdu_ra           ;
	reg   [15:0]  loc_mpdu_scf          ;
	reg   [3 :0]  loc_mpdu_tid          ;
	reg   [1 :0]  loc_mpdu_qos_ack      ;
	wire          loc_dur_upd_val       ;
	wire  [15:0]  loc_dur_upd           ;
	reg           loc_tx_dur_en         ;
	reg   [1 :0]  loc_tx_dur_type       ;
	reg   [7 :0]  loc_tx_exp_resp_len   ;
	reg   [19:0]  count                 ;
	reg   [15:0]  loc_tx_nav            ;
	reg   [2 :0]  loc_ch_bw             ;
	reg   [2 :0]  loc_aggr_final_mpdu   ;
	// bitmap update	
	reg           loc_bmp_en            ;
	wire          loc_bmp_res_val       ;
	wire  [31:0]  loc_bmp_res           ;
	reg   [7 :0]  start_count           ;
	reg           frame_info_avl        ;
	reg           frame_info_avl_dly    ;
	reg   [15:0]  loc_txop_limit        ;
	reg           loc_sub_mpdu_sent_dly ;// added on 29/11
	wire  [19:0]  loc_sub_frame_len     ;
	wire  [19:0]  loc_sub_frame_len_pad ;
	wire  [19:0]  loc_sub_frame_len_pad_zd;
	reg           loc_fb_update         ;
	reg   [7 :0]  aggr_count            ;
	reg   [19:0]  loc_upd_len           ;
	reg           loc_upd_len_avl       ;
	reg   [15:0]  loc_reg_dur           ;
	reg   [19:0]  loc_txvec_upa_length  ;
	reg           loc_crc_added         ;
	reg           loc_bitmap_updated    ;
	reg           loc_txvec_done        ;
	reg           loc_data_done         ;
	reg           loc_phy_start_req     ;
	reg           loc_phy_data_req      ;
	reg           loc_phy_start_req_dly ;
	reg           loc_phy_data_req_dly  ;
	reg           loc_dur_avl           ;
	wire  [15:0]  loc_resp_len          ;
	reg           loc_cp_start_ind_dly  ;
	reg           loc_cp_start_ind      ;
	wire  [15:0]  loc_mpdu_offset[0:7]  ;
	wire  [7 :0]  loc_lrc               ;
	wire  [7 :0]  loc_src               ;
	wire          loc_clear             ;
	//fallback registers
	reg   [1 :0]  loc_fr_stbc_fb        ;
	reg   [2 :0]  loc_fr_n_tx_fb        ;
	reg   [6 :0]  loc_fr_mcs_fb         ;
	reg   [3 :0]  loc_fr_fallback_rate  ;
	reg   [7 :0]  loc_fr_num_sts_fb     ;
	reg   [2 :0]  loc_txvec_n_tx        ;
	reg   [2 :0]  loc_txvec_up_num_sts  ;
	wire  [1 :0]  loc_aggr_len_add [0:7];
	wire          loc_aggr_pkt_len_add  ;
	reg           loc_ctrl_frame_generated;
	//delimiter register
	wire  [31:0]  delimiter                       ;
	//aggregation lengths
	reg   [15:0]  loc_aggr_pkt_len      [0:7]     ;
	reg   [1 :0]  loc_aggr_pkt_pad_len  [0:7]     ;
	//registers for Counters
	reg           loc_reg_fI_retry                ;
	//registers for offsets
	reg   [11:0]  loc_reg_read_offset             ;
	reg   [19:0]  loc_reg_pkt_len                 ;
	//holding zerodelimiter
	wire  [31:0]  loc_reg_zero_delmt              ;
	// registers related to transmission 
	reg           loc_reg_is_ctrl_frame           ;//1-tx_ctrl_txvec|0-tx_txvec
	// registers specific to the retransmission 
	reg   [7 :0]  loc_reg_aggr_bitmap        [0:3];
	reg   [3 :0]  loc_reg_aggr_pending_mpdu  [0:3];
	reg   [11:0]  loc_reg_aggr_ssn           [0:3];
	reg           loc_reg_pre_fr_is_ctrl          ;
        reg           loc_phy_req;
	// Registers for control frame
	reg           loc_ctrl_start_req;
	reg           loc_ctrl_data_req; 
// Wires added for i/f with cp
	wire  [1 :0]  loc_resp_format                 ;
	wire  [15:0]  loc_resp_ht_length              ;
	wire  [6 :0]  loc_resp_mcs                    ;           
	wire  [3 :0]  loc_resp_L_datarate             ;      
	wire  [11:0]  loc_resp_L_length               ;       
	wire  [11:0]  loc_resp_upa_length             ;      
	wire  [1 :0]  loc_resp_stbc                   ; 
	  reg  loc_sub_mpdu_sent1;
  wire loc_sub_mpdu_sent = loc_sub_mpdu_sent1 & ~loc_sub_mpdu_sent_dly;

	/*------------------------------------------------------------------------------------------*/

	/* Module Instantiations
	............................................................................................*/
	uu_acmac_tx_duration TX_DUR (
		.clk                        (clk                     ),
		.rst_n                      (rst_n                   ),
		.tx_dur_en                  (loc_tx_dur_en           ),
		//INFO
		.tx_dur_type                (loc_tx_dur_type         ),
		.tx_dur_in_cts_self         (1'b0                    ),
		.tx_dur_in_rts_cts_rate     (loc_fr_rtscts_rate      ),
		.tx_dur_in_exp_resp_len     (loc_tx_exp_resp_len     ),
		.tx_dur_in_next_frame_len   (20'h0                   ),
		//TXVEC
                .tx_dur_in_txvec_format     (loc_txvec_format        ),
                .tx_dur_in_txvec_stbc       (loc_txvec_stbc          ),
                .tx_dur_in_txvec_mcs        (loc_txvec_mcs           ),
                .tx_dur_in_txvec_ch_bndwdth (loc_txvec_ch_bndwdth    ),
                .tx_dur_in_txvec_num_ext_ss (loc_txvec_num_ext_ss    ),
                .tx_dur_in_txvec_L_length   (loc_txvec_L_length      ),
                .tx_dur_in_txvec_ht_length  (loc_txvec_ht_length     ),
                .tx_dur_in_txvec_L_datarate (loc_txvec_L_datarate    ),
                .tx_dur_in_txvec_modulation (loc_txvec_modulation    ),
                .tx_dur_in_txvec_is_long_pre(loc_txvec_is_long_pre   ),
                .tx_dur_in_txvec_is_short_GI(loc_txvec_is_short_GI   ),
		//RATE
		.tx_dur_out_vht_mcs_en      (tx_out_vht_mcs_en       ),
		.tx_dur_out_vht_mcs_addr    (tx_out_vht_mcs_addr     ),
		.tx_dur_in_vht_mcs_data     (tx_in_vht_mcs_data      ),
		.tx_dur_out_ht_mcs_en       (tx_out_ht_mcs_en        ),
		.tx_dur_out_ht_mcs_addr     (tx_out_ht_mcs_addr      ),
		.tx_dur_in_ht_mcs_data      (tx_in_ht_mcs_data       ),
		.tx_dur_out_lrate_en        (tx_out_lrate_en         ),
		.tx_dur_out_lrate_addr      (tx_out_lrate_addr       ),
		.tx_dur_in_lrate_data       (tx_in_lrate_data        ),
		//REGISTERS 
		.tx_dur_in_BSSBasicRateSet  (tx_in_BSSBasicRateSet   ),  
		.tx_dur_in_SIFS_timer_value (tx_in_SIFS_timer_value  ),  
		//DURATION
		.tx_dur_out_dur_upd_val     (loc_dur_upd_val         ),
		.tx_dur_out_dur_upd         (loc_dur_upd             ),
// Added
		.tx_dur_out_resp_format     (loc_resp_format         ),
		.tx_dur_out_resp_ht_length  (loc_resp_ht_length      ),
		.tx_dur_out_resp_mcs        (loc_resp_mcs            ),
		.tx_dur_out_resp_L_datarate (loc_resp_L_datarate     ), 
		.tx_dur_out_resp_L_length   (loc_resp_L_length       ), 
		.tx_dur_out_resp_upa_length (loc_resp_upa_length     ), 
		.tx_dur_out_resp_stbc       (loc_resp_stbc           )
              );
	uu_acmac_tx_update_bitmap BMP_UPD(
		.clk                    (clk                 ), 
		.rst_n                  (rst_n               ), 
		.tx_bmp_en              (loc_bmp_en          ), 
		.tx_bmp_out_sta_ba_data (tx_out_sta_ba_data  ), 
		.tx_bmp_out_sta_ba_wen  (tx_out_sta_ba_wen   ), 
		.tx_bmp_out_sta_ba_en   (tx_out_sta_ba_en    ), 
		.tx_bmp_in_sta_ba_data  (tx_in_sta_ba_data   ), 
		.tx_bmp_out_sta_ba_addr (tx_out_sta_ba_addr  ), 
		.tx_bmp_in_sta_info_val (tx_in_sta_info_val  ),
		.tx_bmp_in_sta_info     (tx_in_sta_info      ),
		.tx_bmp_in_sta_offset   (tx_in_sta_offset    ),
		.tx_bmp_out_sta_get_info(tx_out_sta_get_info ),
		.tx_bmp_in_mpdu_scf     (loc_mpdu_scf        ), 
		.tx_bmp_out_rx_res_en   (loc_bmp_res_val     ), 
		.tx_bmp_out_rx_res      (loc_bmp_res         )
	);
	/*------------------------------------------------------------------------------------------*/
	/* Continuous assignments 
	............................................................................................*/
	assign loc_resp_len = get_exp_res_fr_len(loc_txvec_is_aggr,loc_mpdu_fc0,loc_mpdu_qos_ack);
	assign delimiter = delmt(loc_txvec_format,loc_aggr_pkt_len[loc_aggr_count]);
	assign loc_req = loc_data_req|loc_start_req|loc_more_data_req|loc_ctrl_start_req|loc_ctrl_data_req;
	assign tx_out_buf_pkt_len = loc_reg_pkt_len;
	assign loc_reg_zero_delmt = 32'h4E00_0000;
	assign tx_out_phy_start_req = loc_phy_start_req & ~loc_phy_start_req_dly;
	assign tx_out_phy_data_req = loc_phy_data_req & ~loc_phy_data_req_dly;
	assign loc_mpdu0_c = (loc_aggr_pkt_len[0]%4)? 'b1 :'b0;
	assign loc_mpdu1_c = (loc_aggr_pkt_len[1]%4)? 'b1 :'b0;
	assign loc_mpdu2_c = (loc_aggr_pkt_len[2]%4)? 'b1 :'b0;
	assign loc_mpdu3_c = (loc_aggr_pkt_len[3]%4)? 'b1 :'b0;
	assign loc_mpdu4_c = (loc_aggr_pkt_len[4]%4)? 'b1 :'b0;
	assign loc_mpdu5_c = (loc_aggr_pkt_len[5]%4)? 'b1 :'b0;
	assign loc_mpdu6_c = (loc_aggr_pkt_len[6]%4)? 'b1 :'b0;
	assign loc_mpdu_offset [0] = `MPDU_OFFSET;
	assign loc_mpdu_offset [1] = loc_mpdu_offset [0]+ (loc_aggr_pkt_len[0]>>2) + loc_mpdu0_c;
	assign loc_mpdu_offset [2] = loc_mpdu_offset [1]+ (loc_aggr_pkt_len[1]>>2) + loc_mpdu1_c;
	assign loc_mpdu_offset [3] = loc_mpdu_offset [2]+ (loc_aggr_pkt_len[2]>>2) + loc_mpdu2_c;
	assign loc_mpdu_offset [4] = loc_mpdu_offset [3]+ (loc_aggr_pkt_len[3]>>2) + loc_mpdu3_c;
	assign loc_mpdu_offset [5] = loc_mpdu_offset [4]+ (loc_aggr_pkt_len[4]>>2) + loc_mpdu4_c;
	assign loc_mpdu_offset [6] = loc_mpdu_offset [5]+ (loc_aggr_pkt_len[5]>>2) + loc_mpdu5_c;
	assign loc_mpdu_offset [7] = loc_mpdu_offset [6]+ (loc_aggr_pkt_len[6]>>2) + loc_mpdu6_c;
	assign loc_lrc = (tx_in_cap_ac==0) ? tx_in_cap_ac0_lrc :
	                ((tx_in_cap_ac==1) ? tx_in_cap_ac1_lrc :
	                ((tx_in_cap_ac==2) ? tx_in_cap_ac2_lrc : tx_in_cap_ac3_lrc));
	
	assign loc_src = (tx_in_cap_ac==0) ? tx_in_cap_ac0_src :
	                ((tx_in_cap_ac==1) ? tx_in_cap_ac1_src :
	                ((tx_in_cap_ac==2) ? tx_in_cap_ac2_src : tx_in_cap_ac3_src));

	assign loc_clear = tx_in_clear_ampdu | tx_in_clear_mpdu;
	assign tx_out_sta_addr = loc_mpdu_ra;
	assign tx_out_sta_tid = loc_mpdu_tid;
	assign tx_out_sta_dir = 1'b1;
	assign loc_sub_frame_len = `UU_WLAN_MPDU_DELIMITER_LEN + loc_aggr_pkt_len[loc_aggr_count];
	assign loc_sub_frame_len_pad = `UU_WLAN_MPDU_DELIMITER_LEN + loc_aggr_pkt_len[loc_aggr_count] + loc_aggr_pkt_pad_len[loc_aggr_count];
	assign loc_sub_frame_len_pad_zd = `UU_WLAN_MPDU_DELIMITER_LEN + loc_aggr_pkt_len[loc_aggr_count] + loc_aggr_pkt_pad_len[loc_aggr_count] + 4'h8;
	assign tx_out_cp_start_ind = loc_cp_start_ind & ~ loc_cp_start_ind_dly;
	assign loc_aggr_len_add[0] = (loc_aggr_pkt_len[0]%4 != 0) ? (4-(loc_aggr_pkt_len[0]%4)) :0;
	assign loc_aggr_len_add[1] = (loc_aggr_pkt_len[1]%4 != 0) ? (4-(loc_aggr_pkt_len[1]%4)) :0;
	assign loc_aggr_len_add[2] = (loc_aggr_pkt_len[2]%4 != 0) ? (4-(loc_aggr_pkt_len[2]%4)) :0;
	assign loc_aggr_len_add[3] = (loc_aggr_pkt_len[3]%4 != 0) ? (4-(loc_aggr_pkt_len[3]%4)) :0;
	assign loc_aggr_len_add[4] = (loc_aggr_pkt_len[4]%4 != 0) ? (4-(loc_aggr_pkt_len[4]%4)) :0;
	assign loc_aggr_len_add[5] = (loc_aggr_pkt_len[5]%4 != 0) ? (4-(loc_aggr_pkt_len[5]%4)) :0;
	assign loc_aggr_len_add[6] = (loc_aggr_pkt_len[6]%4 != 0) ? (4-(loc_aggr_pkt_len[6]%4)) :0;
	assign loc_aggr_len_add[7] = (loc_aggr_pkt_len[7]%4 != 0) ? (4-(loc_aggr_pkt_len[7]%4)) :0;
	assign loc_aggr_pkt_len_add = (loc_aggr_pkt_len[loc_aggr_count]%4!=0) ? 1 : 0;
	assign tx_out_sta_seqno_ac0  = loc_reg_aggr_ssn[0];
	assign tx_out_sta_seqno_ac1  = loc_reg_aggr_ssn[1];
	assign tx_out_sta_seqno_ac2  = loc_reg_aggr_ssn[2];
	assign tx_out_sta_seqno_ac3  = loc_reg_aggr_ssn[3];
	assign tx_out_sta_bitmap_ac0 = loc_reg_aggr_bitmap[0];
	assign tx_out_sta_bitmap_ac1 = loc_reg_aggr_bitmap[1];
	assign tx_out_sta_bitmap_ac2 = loc_reg_aggr_bitmap[2];
	assign tx_out_sta_bitmap_ac3 = loc_reg_aggr_bitmap[3];
	assign tx_out_sta_frame_len  = loc_upd_len;
	assign tx_out_cap_mpdu_fc0 = loc_mpdu_fc0;
      assign tx_out_cap_mpdu_fc1 = loc_mpdu_fc1;
	/*------------------------------------------------------------------------------------------*/
		
	/* Function for finding out frame_type received from Rx_handler
	............................................................................................*/
	function [15:0] get_exp_res_fr_len;
		input        txvec_is_aggr;
		input [7 :0] mpdu_fc0     ;
		input [1 :0] mpdu_qos_ack ;
		reg   [15:0] resp_length  ;
	begin   
		if (mpdu_fc0 == (`IEEE80211_FC0_TYPE_DATA | `IEEE80211_FC0_SUBTYPE_QOS))begin
			if (!mpdu_qos_ack)begin
				if (txvec_is_aggr)
					resp_length = `UU_BA_COMPRESSED_FRAME_LEN;
				else
					resp_length = `UU_ACK_FRAME_LEN;
			end
			else
				resp_length = 0;
		end
		else
			resp_length = `UU_ACK_FRAME_LEN;
		get_exp_res_fr_len = resp_length;
	end
	endfunction//get_exp_res_fr_len

	function [31:0] delmt;
		input [1 :0] format;
		input [15:0] length;
		reg   [31:0] del   ;
		reg   [31:0] delm  ;
		reg   [7 :0] crc   ;
	begin
		del = 32'h4E00_0000;
		if(format == `UU_WLAN_FRAME_FORMAT_VHT)
			delm = insert_bits(del,5'd15,5'd2,length);	
		else
			delm = insert_bits(del,5'd15,5'd4,length);
		crc = crc8_16({delm[7:0],delm[15:8]},0);
		delmt = insert_bits(delm,5'd23,5'd16,crc);
	end
	endfunction //delmt

	function [31:0] insert_bits;
		input [31:0] del;
		input [4 :0] msb;
		input [4 :0] lsb;
		input [15:0] val;
	begin
		insert_bits = (del & ~(((1'b1 << ((msb-lsb)+ 1)) - 1) << lsb))|((val << lsb)&(((1'b1 << ((msb-lsb)+ 1)) - 1) << lsb));
	end
	endfunction //insert_bits

	function [7:0] crc8_16;
		input [15:0] data;
		input [7:0] crc_in;
		reg [15:0] d;
		reg [7:0] c;
		reg [7:0] c_out;
		integer i;
	begin
		for (i=0;i<=15;i=i+1)
			d[i] = data[15-i];
		c = crc_in;
		
		c_out[0] = d[1] ^ d[3] ^ d[7] ^ d[8] ^ d[9] ^ d[15] ^ c[0] ^ c[4] ^ c[6];
		c_out[1] = d[0] ^ d[1] ^ d[2] ^ d[3] ^ d[6] ^ d[9] ^ d[14] ^ d[15] ^ c[1] ^ c[4] ^ c[5] ^ c[6] ^ c[7]; 
		c_out[2] = d[0] ^ d[2] ^ d[3] ^ d[5] ^ d[7] ^ d[9] ^ d[13] ^ d[14] ^ d[15] ^ c[0] ^ c[2] ^ c[4] ^ c[5] ^ c[7];
		c_out[3] = d[1] ^ d[2] ^ d[4] ^ d[6] ^ d[8] ^ d[12] ^ d[13] ^ d[14] ^ c[1] ^ c[3] ^ c[5] ^ c[6]; 
		c_out[4] = d[0] ^ d[1] ^ d[3] ^ d[5] ^ d[7] ^ d[11] ^ d[12] ^ d[13] ^ c[0] ^ c[2] ^ c[4] ^ c[6] ^ c[7];
		c_out[5] = d[0] ^ d[2] ^ d[4] ^ d[6] ^ d[10] ^ d[11] ^ d[12] ^ c[1] ^ c[3] ^ c[5] ^ c[7];
		c_out[6] = d[1] ^ d[3] ^ d[5] ^ d[9] ^ d[10] ^ d[11] ^ c[2] ^ c[4] ^ c[6];
		c_out[7] = d[0] ^ d[2] ^ d[4] ^ d[8] ^ d[9] ^ d[10] ^ c[3] ^ c[5] ^ c[7];
		
		crc8_16 = c_out;
	end
	endfunction //crc8_16
	/*------------------------------------------------------------------------------------------------*/

	/* Concurrent statements
	..................................................................................................*/
	
	//Latching inputs
	//================================================================================INPUT_LATCH_START
	//ac out for memory selection
	always @(posedge clk or `EDGE_OF_RESET) begin
	  if(`POLARITY_OF_RESET)
			loc_phy_req <= 0;
		else if(!tx_en)
		  loc_phy_req <= 0;
		else
		  loc_phy_req <= tx_in_phy_data_req;
  end
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			tx_out_frame_ac <= 0;
		else if(!tx_en)
			tx_out_frame_ac <= 0;
		else
			tx_out_frame_ac <= tx_in_cap_ac;
	end
	
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_txop_limit <= 0;
		else if(!tx_en)
			loc_txop_limit <= 0;
		else begin
			case(tx_in_cap_ac)
				0: loc_txop_limit <= tx_in_TXOP_limit_ac0;
				1: loc_txop_limit <= tx_in_TXOP_limit_ac1;
				2: loc_txop_limit <= tx_in_TXOP_limit_ac2;
				3: loc_txop_limit <= tx_in_TXOP_limit_ac3;
			endcase
		end
	end
	
	//registering requests from CAP & CP 
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_data_req <= 0;
		else if(!tx_en)
			loc_data_req <= 0;
		else if(loc_req)begin
			if(loc_data_done)
				loc_data_req <= 0; 
		end
		else if(tx_in_cap_data_req) 
			loc_data_req <= 1;
	end

	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_start_req <= 0;
		else if(!tx_en)
			loc_start_req <= 0;
		else if(loc_req)begin
			if(loc_txvec_done)
				loc_start_req <= 0;
		end
		else if(tx_in_cap_start_req) 
			loc_start_req <= 1;
	end
// for control response frame
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_ctrl_start_req <= 0;
		else if(!tx_en)
			loc_ctrl_start_req <= 0;
		else if(loc_req)begin
			if(loc_txvec_done)
				loc_ctrl_start_req <= 0;
		end
		else if(tx_in_cap_ctrl_start_req) 
			loc_ctrl_start_req <= 1;
	end

	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_ctrl_data_req <= 0;
		else if(!tx_en)
			loc_ctrl_data_req <= 0;
		else if(loc_req)begin
			if(loc_data_done)
				loc_ctrl_data_req <= 0;
		end
		else if(tx_in_cap_ctrl_data_req) 
			loc_ctrl_data_req <= 1;
	end
// Always Block to generate the Ack frame for Response
    always @(posedge clk or `EDGE_OF_RESET)
    begin
        if( `POLARITY_OF_RESET )
            loc_ack_req <= 1'b0;
        else if( !tx_en ) 
            loc_ack_req <= 1'b0;
        else if( loc_req && loc_data_done )
            loc_ack_req <= 1'b0;
        else if( tx_in_cap_send_ack ) 
            loc_ack_req <= 1'b1;
    end   

	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_more_data_req <= 0;
		else if(!tx_en)
			loc_more_data_req <= 0;
		else if(loc_req)begin
			if(loc_txvec_done)
				loc_more_data_req <= 0;
		end
		else if(tx_in_cap_more_data_req && loc_reg_pre_fr_is_ctrl)
			loc_more_data_req <= 1;	
	end
	
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_fb_update <= 0;
		else if(!tx_en)
			loc_fb_update <= 0;
		else if(tx_in_cap_fb_update)
			loc_fb_update <= 1;
		else if(loc_clear)
			loc_fb_update <= 0;
	end

	//registering channel bandwidth and nav
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
			loc_tx_nav <= 0;
			loc_ch_bw  <= 0;
		end
		else if(!tx_en)begin
			loc_tx_nav <= 0;
			loc_ch_bw  <= 0;
		end
		else if(loc_start_req||loc_more_data_req)begin
			if(loc_txvec_done)begin
				loc_tx_nav <= 0;
				loc_ch_bw  <= 0;
                         end
	        end 
		else if(tx_in_cap_start_req||tx_in_cap_more_data_req) begin
			case(tx_in_cap_ac)
				3'h0: loc_tx_nav <= tx_in_cap_ac0_tx_nav;
				3'h1: loc_tx_nav <= tx_in_cap_ac1_tx_nav;
				3'h2: loc_tx_nav <= tx_in_cap_ac2_tx_nav;
				3'h3: loc_tx_nav <= tx_in_cap_ac3_tx_nav;
				default:loc_tx_nav <= tx_in_cap_ac0_tx_nav;
			endcase
			loc_ch_bw <= tx_in_cap_ch_bw;
		end
	end
	//================================================================================INPUT_LATCH_END

	//Global Registers
	//================================================================================GLBL_REG_START

	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
		    loc_reg_aggr_bitmap[0] <= 0;
		    loc_reg_aggr_bitmap[1] <= 0;
		    loc_reg_aggr_bitmap[2] <= 0;
		    loc_reg_aggr_bitmap[3] <= 0;
		end
		else if(!tx_en)begin
		    loc_reg_aggr_bitmap[0] <= 0;
		    loc_reg_aggr_bitmap[1] <= 0;
		    loc_reg_aggr_bitmap[2] <= 0;
		    loc_reg_aggr_bitmap[3] <= 0;
		end
		else if(tx_in_cp_upd_mpdu_status)begin
			case(tx_in_cp_seqno)
				loc_reg_aggr_ssn[tx_in_cp_ac]+0:
					loc_reg_aggr_bitmap[tx_in_cp_ac] <= loc_reg_aggr_bitmap[tx_in_cp_ac] & 8'hFE;
				loc_reg_aggr_ssn[tx_in_cp_ac]+1:
					loc_reg_aggr_bitmap[tx_in_cp_ac] <= loc_reg_aggr_bitmap[tx_in_cp_ac] & 8'hFD;
				loc_reg_aggr_ssn[tx_in_cp_ac]+2:
					loc_reg_aggr_bitmap[tx_in_cp_ac] <= loc_reg_aggr_bitmap[tx_in_cp_ac] & 8'hFB;
				loc_reg_aggr_ssn[tx_in_cp_ac]+3:
					loc_reg_aggr_bitmap[tx_in_cp_ac] <= loc_reg_aggr_bitmap[tx_in_cp_ac] & 8'hF7;
				loc_reg_aggr_ssn[tx_in_cp_ac]+4:
					loc_reg_aggr_bitmap[tx_in_cp_ac] <= loc_reg_aggr_bitmap[tx_in_cp_ac] & 8'hEF;
				loc_reg_aggr_ssn[tx_in_cp_ac]+5:
					loc_reg_aggr_bitmap[tx_in_cp_ac] <= loc_reg_aggr_bitmap[tx_in_cp_ac] & 8'hDF;
				loc_reg_aggr_ssn[tx_in_cp_ac]+6:
					loc_reg_aggr_bitmap[tx_in_cp_ac] <= loc_reg_aggr_bitmap[tx_in_cp_ac] & 8'hBF;
				loc_reg_aggr_ssn[tx_in_cp_ac]+7:
					loc_reg_aggr_bitmap[tx_in_cp_ac] <= loc_reg_aggr_bitmap[tx_in_cp_ac] & 8'h7F;
			endcase
		end
		else if(loc_data_req)begin
			if(!loc_reg_is_ctrl_frame&&(loc_fr_aggr_count!=0)&&!loc_reg_fI_retry&&loc_sub_mpdu_sent)
				loc_reg_aggr_bitmap[tx_in_cap_ac] <= loc_reg_aggr_bitmap[tx_in_cap_ac] | (1<<loc_aggr_count);
		end
	end

	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
		    loc_reg_aggr_pending_mpdu[0] <= 0;
		    loc_reg_aggr_pending_mpdu[1] <= 0;
		    loc_reg_aggr_pending_mpdu[2] <= 0;
		    loc_reg_aggr_pending_mpdu[3] <= 0;
		end
		else if(!tx_en)begin
		    loc_reg_aggr_pending_mpdu[0] <= 0;
		    loc_reg_aggr_pending_mpdu[1] <= 0;
		    loc_reg_aggr_pending_mpdu[2] <= 0;
		    loc_reg_aggr_pending_mpdu[3] <= 0;
		end
		else if(tx_in_cp_upd_mpdu_status)begin	
			case(tx_in_cp_seqno)
				loc_reg_aggr_ssn[tx_in_cp_ac]+0:
					loc_reg_aggr_pending_mpdu[tx_in_cp_ac] <= loc_reg_aggr_pending_mpdu[tx_in_cp_ac]-1;
				loc_reg_aggr_ssn[tx_in_cp_ac]+1:
					loc_reg_aggr_pending_mpdu[tx_in_cp_ac] <= loc_reg_aggr_pending_mpdu[tx_in_cp_ac]-1;
				loc_reg_aggr_ssn[tx_in_cp_ac]+2:
					loc_reg_aggr_pending_mpdu[tx_in_cp_ac] <= loc_reg_aggr_pending_mpdu[tx_in_cp_ac]-1;
				loc_reg_aggr_ssn[tx_in_cp_ac]+3:
					loc_reg_aggr_pending_mpdu[tx_in_cp_ac] <= loc_reg_aggr_pending_mpdu[tx_in_cp_ac]-1;
				loc_reg_aggr_ssn[tx_in_cp_ac]+4:
					loc_reg_aggr_pending_mpdu[tx_in_cp_ac] <= loc_reg_aggr_pending_mpdu[tx_in_cp_ac]-1;
				loc_reg_aggr_ssn[tx_in_cp_ac]+5:
					loc_reg_aggr_pending_mpdu[tx_in_cp_ac] <= loc_reg_aggr_pending_mpdu[tx_in_cp_ac]-1;
				loc_reg_aggr_ssn[tx_in_cp_ac]+6:
					loc_reg_aggr_pending_mpdu[tx_in_cp_ac] <= loc_reg_aggr_pending_mpdu[tx_in_cp_ac]-1;
				loc_reg_aggr_ssn[tx_in_cp_ac]+7:
					loc_reg_aggr_pending_mpdu[tx_in_cp_ac] <= loc_reg_aggr_pending_mpdu[tx_in_cp_ac]-1;
			endcase
		end
		else if(loc_data_req)begin
			if(!loc_reg_is_ctrl_frame&&(loc_fr_aggr_count!=0)&&!loc_reg_fI_retry&&loc_data_done)
		    		loc_reg_aggr_pending_mpdu[tx_in_cap_ac] <= aggr_count;
		end
	end

	//aggr ssn register
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
		    loc_reg_aggr_ssn[0] <= 0;
		    loc_reg_aggr_ssn[1] <= 0;
		    loc_reg_aggr_ssn[2] <= 0;
		    loc_reg_aggr_ssn[3] <= 0;
		end
		else if(!tx_en)begin
		    loc_reg_aggr_ssn[0] <= 0;
		    loc_reg_aggr_ssn[1] <= 0;
		    loc_reg_aggr_ssn[2] <= 0;
		    loc_reg_aggr_ssn[3] <= 0;
		end
		else if(loc_data_req)begin
			if((!loc_reg_is_ctrl_frame) &&(loc_fr_aggr_count!=0)&&(loc_aggr_count==0)&&!loc_reg_fI_retry&&loc_sub_mpdu_sent)
		    		loc_reg_aggr_ssn[tx_in_cap_ac] <= loc_mpdu_scf>>4;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_reg_read_offset <= 0;
		else if(!tx_en)
			loc_reg_read_offset <= 0;
		else if(loc_data_req)begin
			if(loc_aggr_count == loc_aggr_final_mpdu)begin
				if(loc_sub_mpdu_sent)
					loc_reg_read_offset <= 0;
			end
			else begin
				if(loc_sub_mpdu_sent)
					loc_reg_read_offset <= (loc_aggr_pkt_len[loc_aggr_count]>>2) + loc_aggr_pkt_len_add+loc_reg_read_offset;
			end
		end
	end


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_reg_pkt_len <= 0;
		else if(!tx_en)
			loc_reg_pkt_len <= 0;
		else if(loc_upd_len_avl)begin
			if(aggr_count==0)
				loc_reg_pkt_len <= loc_upd_len;
			else 
				loc_reg_pkt_len <= loc_aggr_pkt_len[0] + loc_aggr_len_add[0] + 
				                   loc_aggr_pkt_len[1] + loc_aggr_len_add[1] + 
				                   loc_aggr_pkt_len[2] + loc_aggr_len_add[2] + 
							    loc_aggr_pkt_len[3] + loc_aggr_len_add[3] + 
				                   loc_aggr_pkt_len[4] + loc_aggr_len_add[4] + 
							    loc_aggr_pkt_len[5] + loc_aggr_len_add[5] + 
				                   loc_aggr_pkt_len[6] + loc_aggr_len_add[6] + 
							    loc_aggr_pkt_len[7] + loc_aggr_len_add[7];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_reg_fI_retry <= 0;
		else if(!tx_en)
			loc_reg_fI_retry <= 0;
		else if(tx_in_cap_retry)begin
			loc_reg_fI_retry <= 1;
		end
		else if(loc_data_done)begin
			loc_reg_fI_retry <= 0;
		end
	end

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_reg_pre_fr_is_ctrl <= 0;
		else if(!tx_en)
			loc_reg_pre_fr_is_ctrl <= 0;
		else if(loc_reg_is_ctrl_frame)
			loc_reg_pre_fr_is_ctrl <= 1;
		else if(loc_txvec_done&&loc_more_data_req)
			loc_reg_pre_fr_is_ctrl <= 0;
	end

	//================================================================================GLBL_REG_END
	
	//A Control FRAME Memory
	//================================================================================CTRL_MEM_START
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_ctrl_en <= 0;
		else if(!tx_en)
			tx_out_ctrl_en <= 0;
		else if((loc_data_req && loc_reg_is_ctrl_frame)||loc_ctrl_data_req)begin
			if(loc_data_done)	
                                tx_out_ctrl_en <= 0;
			else 
				tx_out_ctrl_en <= 1;
		end
		else if((loc_start_req && loc_reg_is_ctrl_frame)||loc_ctrl_start_req)begin
			if(loc_data_done)
				tx_out_ctrl_en <= 0;
			else 
				tx_out_ctrl_en <= 1;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_ctrl_wen <= 0;
		else if(!tx_en)
			tx_out_ctrl_wen <= 0;
		else 
			tx_out_ctrl_wen <= 0;
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_ctrl_data <= 0;
		else if(!tx_en)
			tx_out_ctrl_data <= 0;
		else
			tx_out_ctrl_data <= 0;
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_ctrl_addr <= 0;
		else if(!tx_en)
			tx_out_ctrl_addr <= 0;
		else if((loc_data_req && loc_reg_is_ctrl_frame)||loc_ctrl_data_req)begin
			if(count < loc_fI_frame_len)begin
				tx_out_ctrl_addr <= count + `CTRL_MPDU_OFFSET;
			end
                end
		else if((loc_start_req && loc_reg_is_ctrl_frame)||loc_ctrl_start_req)begin
			tx_out_ctrl_addr <= count;
		end
	end
	
	//================================================================================CTRL_MEM_END

	//A Tx Frame Memory
	//================================================================================TX_MEM_START
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_frame_en <= 0;
		else if(!tx_en)
			tx_out_frame_en <= 0;
		else if(loc_start_req||loc_more_data_req)
			tx_out_frame_en <= 1;
		else if(loc_data_req && !loc_reg_is_ctrl_frame)
			tx_out_frame_en <= 1;	
		else 
			tx_out_frame_en <= 0;	
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_frame_wen <= 0;
		else if(!tx_en)
			tx_out_frame_wen <= 0;
		else 
			tx_out_frame_wen <= 0;	
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_frame_data <= 0;
		else if(!tx_en)
			tx_out_frame_data <= 0;
		else 
			tx_out_frame_data <= 0;	
	end
// pasing address to memory 
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_frame_addr <= 0;
		else if(!tx_en)
			tx_out_frame_addr <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(!frame_info_avl)begin
					     if(tx_in_buf_rd_ac0)
								tx_out_frame_addr <= start_count + tx_in_buf_base_ptr_ac0;
						  else if (tx_in_buf_rd_ac1)
								tx_out_frame_addr <= start_count + tx_in_buf_base_ptr_ac1;
						  else if (tx_in_buf_rd_ac2)
								tx_out_frame_addr <= start_count + tx_in_buf_base_ptr_ac2;
						  else if (tx_in_buf_rd_ac3)
								tx_out_frame_addr <= start_count + tx_in_buf_base_ptr_ac3;
			end
			else begin
					     if(tx_in_buf_rd_ac0)
								tx_out_frame_addr <= (count >> 2)+ tx_in_buf_base_ptr_ac0;
						  else if(tx_in_buf_rd_ac1)	
								tx_out_frame_addr <= (count >> 2)+ tx_in_buf_base_ptr_ac1;
						  else if(tx_in_buf_rd_ac2)	
								tx_out_frame_addr <= (count >> 2)+ tx_in_buf_base_ptr_ac2;
						  else if(tx_in_buf_rd_ac3)	
								tx_out_frame_addr <= (count >> 2)+ tx_in_buf_base_ptr_ac3;
								
			end
		end
		else if(loc_data_req && !loc_reg_is_ctrl_frame)begin
			if(!frame_info_avl)begin
					     if(tx_in_buf_rd_ac0)
								tx_out_frame_addr <= start_count + tx_in_buf_base_ptr_ac0;
						  else if (tx_in_buf_rd_ac1)
								tx_out_frame_addr <= start_count + tx_in_buf_base_ptr_ac1;
						  else if (tx_in_buf_rd_ac2)
								tx_out_frame_addr <= start_count + tx_in_buf_base_ptr_ac2;
						  else if (tx_in_buf_rd_ac3)
								tx_out_frame_addr <= start_count + tx_in_buf_base_ptr_ac3;
			end
			else begin
				if(aggr_count == 0)begin
					if(count < loc_fI_frame_len)begin
					     if(tx_in_buf_rd_ac0)
						      tx_out_frame_addr   <= (count>>2) + `MPDU_OFFSET + tx_in_buf_base_ptr_ac0;
						  else if(tx_in_buf_rd_ac1)
						      tx_out_frame_addr   <= (count>>2) + `MPDU_OFFSET + tx_in_buf_base_ptr_ac1;
						  else if(tx_in_buf_rd_ac2)
						      tx_out_frame_addr   <= (count>>2) + `MPDU_OFFSET + tx_in_buf_base_ptr_ac2;
						  else if(tx_in_buf_rd_ac3)
						      tx_out_frame_addr   <= (count>>2) + `MPDU_OFFSET + tx_in_buf_base_ptr_ac3;
						  
					end
				end
				else if(loc_aggr_count < loc_fr_aggr_count)begin	
					if(loc_aggr_pkt_len[loc_aggr_count]!=0)begin
						if((count < loc_sub_frame_len-7)&&(((count-1)%4)==2))begin
							if(!loc_reg_fI_retry)begin
										if(tx_in_buf_rd_ac0)							
											tx_out_frame_addr <= (count>>2) + `MPDU_OFFSET + loc_reg_read_offset + tx_in_buf_base_ptr_ac0;
							         else if (tx_in_buf_rd_ac1)
											tx_out_frame_addr <= (count>>2) + `MPDU_OFFSET + loc_reg_read_offset + tx_in_buf_base_ptr_ac1;
							         else if (tx_in_buf_rd_ac2)
											tx_out_frame_addr <= (count>>2) + `MPDU_OFFSET + loc_reg_read_offset + tx_in_buf_base_ptr_ac2;
							         else if (tx_in_buf_rd_ac3)
											tx_out_frame_addr <= (count>>2) + `MPDU_OFFSET + loc_reg_read_offset + tx_in_buf_base_ptr_ac3;
							end
							else begin
										if(tx_in_buf_rd_ac0)								
											tx_out_frame_addr <= (count>>2) + loc_mpdu_offset[loc_aggr_count] +  tx_in_buf_base_ptr_ac0;
							         else if(tx_in_buf_rd_ac1)
											tx_out_frame_addr <= (count>>2) + loc_mpdu_offset[loc_aggr_count] +  tx_in_buf_base_ptr_ac1;
							         else if(tx_in_buf_rd_ac2)
											tx_out_frame_addr <= (count>>2) + loc_mpdu_offset[loc_aggr_count] +  tx_in_buf_base_ptr_ac2;
							         else if(tx_in_buf_rd_ac3)
											tx_out_frame_addr <= (count>>2) + loc_mpdu_offset[loc_aggr_count] +  tx_in_buf_base_ptr_ac3;
														
							end
						end
					end
				end
			end
		end
		else begin
			tx_out_frame_addr <= 0;
		end
	end
	//================================================================================TX_MEM_END

	
	//getting frame_info depending on the AC
	//================================================================================FRAME_INFO_START
	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			aggr_count <= 0;
		else if(!tx_en)
			aggr_count <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(loc_reg_aggr_pending_mpdu[tx_in_cap_ac])
				aggr_count <= loc_reg_aggr_pending_mpdu[tx_in_cap_ac];
			else if(start_count == 14+loc_reg_read_offset) 
				aggr_count <= tx_in_frame_data[31:24];	
		end
		else if(loc_data_req)begin
			if(!loc_reg_is_ctrl_frame)begin
				if(loc_reg_aggr_pending_mpdu[tx_in_cap_ac])
					aggr_count <= loc_reg_aggr_pending_mpdu[tx_in_cap_ac];
				else if(start_count == 14+loc_reg_read_offset)
					aggr_count <= tx_in_frame_data[31:24];	
			end
			else 
				aggr_count <= tx_in_ctrl_data;	
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_format <= 0;
		else if(!tx_en)
			loc_txvec_format <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 2+loc_reg_read_offset)
				loc_txvec_format <= tx_in_frame_data[1:0];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_is_aggr <= 0;
		else if(!tx_en)
			loc_txvec_is_aggr <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 4+loc_reg_read_offset)
				loc_txvec_is_aggr <= tx_in_frame_data[27];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_stbc <= 0;
		else if(!tx_en)
			loc_txvec_stbc <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 4+loc_reg_read_offset)
				loc_txvec_stbc <= tx_in_frame_data[29:28];
			else if(frame_info_avl)begin
				if(loc_fb_update)begin 
					if((loc_lrc==(tx_in_Long_Retry_Count>>1)) && (loc_txvec_format != `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_stbc <= loc_fr_stbc_fb;
					else if((loc_src == `UU_WLAN_SHORT_FB_RETRY_COUNT)&&(loc_txvec_format != `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_stbc <= loc_fr_stbc_fb;
				end
			end
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_mcs <= 0;
		else if(!tx_en)
			loc_txvec_mcs <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 4+loc_reg_read_offset)
				loc_txvec_mcs <= tx_in_frame_data[6:0];
			else if(frame_info_avl)begin
				if(loc_fb_update)begin 
					if((loc_lrc==(tx_in_Long_Retry_Count>>1)) && (loc_txvec_format != `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_mcs <= loc_fr_mcs_fb;
					else if((loc_src == `UU_WLAN_SHORT_FB_RETRY_COUNT)&&(loc_txvec_format != `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_mcs <= loc_fr_mcs_fb;
				end
			end
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_ch_bndwdth <= 0;
		else if(!tx_en)
			loc_txvec_ch_bndwdth <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 2+loc_reg_read_offset)
				loc_txvec_ch_bndwdth <= tx_in_frame_data[4:2];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_num_ext_ss <= 0;
		else if(!tx_en)
			loc_txvec_num_ext_ss <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 3+loc_reg_read_offset)
				loc_txvec_num_ext_ss <= tx_in_frame_data[25:24];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_ht_length <= 0;
		else if(!tx_en)
			loc_txvec_ht_length <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(loc_upd_len_avl && ((loc_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_GF)||
			                        (loc_txvec_format == `UU_WLAN_FRAME_FORMAT_HT_MF)))
				loc_txvec_ht_length <= loc_upd_len; 
		end
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_upa_length <= 0;
		else if(!tx_en)
			loc_txvec_upa_length <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(loc_upd_len_avl&&(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_VHT))
				loc_txvec_upa_length <= loc_upd_len; 
		end
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_L_length <= 0;
		else if(!tx_en)
			loc_txvec_L_length <= 0;
		else if(loc_start_req)begin
			if((tx_in_RTS_Threshold < loc_upd_len)&&loc_upd_len_avl)begin
				if(tx_in_cp_res_val && (tx_in_cp_res == `UU_SUCCESS))
					loc_txvec_L_length <= tx_in_cp_ctrl_fr_len;
			end
			else if(loc_upd_len_avl)begin
				loc_txvec_L_length <= loc_upd_len; 			  
			end
		end
		else if(loc_more_data_req)begin
			if(loc_upd_len_avl&&(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT))
				loc_txvec_L_length <= loc_upd_len; 
		end
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_L_datarate <= 0;
		else if(!tx_en)
			loc_txvec_L_datarate <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count == 2+loc_reg_read_offset)
				loc_txvec_L_datarate <= tx_in_frame_data[19:16];
			else if(frame_info_avl)begin
				if(loc_fb_update)begin 
					if((loc_lrc==(tx_in_Long_Retry_Count>>1)) && (loc_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_L_datarate <= loc_fr_fallback_rate;
					else if((loc_src == `UU_WLAN_SHORT_FB_RETRY_COUNT)&&(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_L_datarate <= loc_fr_fallback_rate;
				end
			end
		end
	end


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_modulation <= 0;
		else if(!tx_en)
			loc_txvec_modulation <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 3+loc_reg_read_offset)
				loc_txvec_modulation <= tx_in_frame_data[31:29];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_is_long_pre <= 0;
		else if(!tx_en)
			loc_txvec_is_long_pre <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 4+loc_reg_read_offset)
				loc_txvec_is_long_pre <= tx_in_frame_data[7];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_is_short_GI <= 0;
		else if(!tx_en)
			loc_txvec_is_short_GI <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count== 4+loc_reg_read_offset)
				loc_txvec_is_short_GI <= tx_in_frame_data[31];
		end
	end


	//specific to fallback
    /* Register to store number of transmit chains */
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_n_tx <= 0;
		else if(!tx_en)
			loc_txvec_n_tx <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count == 2+loc_reg_read_offset)
				loc_txvec_n_tx <= tx_in_frame_data[7:5];
			else if(frame_info_avl)begin
				if(loc_fb_update)begin 
					if((loc_lrc==(tx_in_Long_Retry_Count>>1)) && (loc_txvec_format != `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_n_tx <= loc_fr_n_tx_fb;
					else if((loc_src == `UU_WLAN_SHORT_FB_RETRY_COUNT)&&(loc_txvec_format != `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_n_tx <= loc_fr_n_tx_fb;
				end
			end
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_up_num_sts <= 0;
		else if(!tx_en)
			loc_txvec_up_num_sts <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count == 6+loc_reg_read_offset)
				loc_txvec_up_num_sts <= tx_in_frame_data[4:2];
			else if(frame_info_avl)begin
				if(loc_fb_update)begin 
					if((loc_lrc==(tx_in_Long_Retry_Count>>1)) && (loc_txvec_format != `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_up_num_sts <= loc_fr_num_sts_fb;
					else if((loc_src == `UU_WLAN_SHORT_FB_RETRY_COUNT)&&(loc_txvec_format != `UU_WLAN_FRAME_FORMAT_NON_HT))
						loc_txvec_up_num_sts <= loc_fr_num_sts_fb;
				end
			end
		end
	end	

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_fr_aggr_count <= 0;
		else if(!tx_en)
			loc_fr_aggr_count <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count== 14+loc_reg_read_offset)
				loc_fr_aggr_count <= tx_in_frame_data[31:24];
		end
		else if(loc_data_req && !loc_reg_is_ctrl_frame)begin
			if(start_count== 14+loc_reg_read_offset)
				loc_fr_aggr_count <= tx_in_frame_data[31:24];	
		end
	end
	
		
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_fr_rtscts_rate <= 0;
		else if(!tx_en)
			loc_fr_rtscts_rate <= 0;
		else if(loc_start_req)begin
if(start_count == 14+loc_reg_read_offset)
			loc_fr_rtscts_rate <= tx_in_frame_data[19:16];
		end
	end
	
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)
			loc_fI_frame_len <= 0;
		else if(!tx_en)
			loc_fI_frame_len <= 0;
                else if(tx_in_cap_ctrl_data_req)begin
			loc_fI_frame_len <= tx_in_cp_ctrl_fr_len;
		end
		else if(loc_start_req)begin
			if(start_count == 10+loc_reg_read_offset)
				loc_fI_frame_len <= tx_in_frame_data[19:0];
		end
		else if(loc_more_data_req)begin
			if(start_count == 10+loc_reg_read_offset)
				loc_fI_frame_len <= tx_in_frame_data[19:0];
		end
		else if(loc_data_req && (loc_fr_aggr_count==0))begin
			if(loc_reg_is_ctrl_frame)begin
				loc_fI_frame_len <= loc_txvec_L_length;
			end
			else begin 
				if(start_count == 10+loc_reg_read_offset)
					loc_fI_frame_len <= tx_in_frame_data[19:0];
			end
		end
	end	

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_fr_stbc_fb <= 0;
		else if(!tx_en)
			loc_fr_stbc_fb <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count == 13+loc_reg_read_offset)
				loc_fr_stbc_fb <= tx_in_frame_data[25:24];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_fr_n_tx_fb <= 0;
		else if(!tx_en)
			loc_fr_n_tx_fb <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count == 13+loc_reg_read_offset)
				loc_fr_n_tx_fb <= tx_in_frame_data[28:26];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_fr_fallback_rate <= 0;
		else if(!tx_en)
			loc_fr_fallback_rate <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count == 14+loc_reg_read_offset)
				loc_fr_fallback_rate <= tx_in_frame_data[11:8];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_fr_mcs_fb <= 0;
		else if(!tx_en)
			loc_fr_mcs_fb <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count == 14+loc_reg_read_offset)
				loc_fr_mcs_fb <= tx_in_frame_data[6:0];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_fr_num_sts_fb <= 0;
		else if(!tx_en)
			loc_fr_num_sts_fb <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count == 15+loc_reg_read_offset)
				loc_fr_num_sts_fb <= tx_in_frame_data[7:0];
		end
	end



	//lengths of mpdus to be aggregated loc_aggr_pkt_len
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
			loc_aggr_pkt_len[0] <= 0;
			loc_aggr_pkt_len[1] <= 0;
			loc_aggr_pkt_len[2] <= 0;
			loc_aggr_pkt_len[3] <= 0;
			loc_aggr_pkt_len[4] <= 0;
			loc_aggr_pkt_len[5] <= 0;
			loc_aggr_pkt_len[6] <= 0;
			loc_aggr_pkt_len[7] <= 0;
		end
		else if(!tx_en)begin
			loc_aggr_pkt_len[0] <= 0;
			loc_aggr_pkt_len[1] <= 0;
			loc_aggr_pkt_len[2] <= 0;
			loc_aggr_pkt_len[3] <= 0;
			loc_aggr_pkt_len[4] <= 0;
			loc_aggr_pkt_len[5] <= 0;
			loc_aggr_pkt_len[6] <= 0;
			loc_aggr_pkt_len[7] <= 0;
		end
		else if(loc_start_req||loc_more_data_req)begin
			if((loc_reg_aggr_pending_mpdu[tx_in_cap_ac]==0)&&(aggr_count != 0))begin
				case(start_count)
					16: loc_aggr_pkt_len[0] <= tx_in_frame_data[13:0];
					17: loc_aggr_pkt_len[1] <= tx_in_frame_data[13:0];
					18: loc_aggr_pkt_len[2] <= tx_in_frame_data[13:0];
					19: loc_aggr_pkt_len[3] <= tx_in_frame_data[13:0];
					20: loc_aggr_pkt_len[4] <= tx_in_frame_data[13:0];
					21: loc_aggr_pkt_len[5] <= tx_in_frame_data[13:0];
					21: loc_aggr_pkt_len[6] <= tx_in_frame_data[13:0];
					23: loc_aggr_pkt_len[7] <= tx_in_frame_data[13:0];
				endcase
			end
		end
	end

	//PAD lengths of mpdus to be aggregated loc_aggr_pkt_len
	always @(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
			loc_aggr_pkt_pad_len[0] <= 0;
			loc_aggr_pkt_pad_len[1] <= 0;
			loc_aggr_pkt_pad_len[2] <= 0;
			loc_aggr_pkt_pad_len[3] <= 0;
			loc_aggr_pkt_pad_len[4] <= 0;
			loc_aggr_pkt_pad_len[5] <= 0;
			loc_aggr_pkt_pad_len[6] <= 0;
			loc_aggr_pkt_pad_len[7] <= 0;
		end
		else if(!tx_en)begin
			loc_aggr_pkt_pad_len[0] <= 0;
			loc_aggr_pkt_pad_len[1] <= 0;
			loc_aggr_pkt_pad_len[2] <= 0;
			loc_aggr_pkt_pad_len[3] <= 0;
			loc_aggr_pkt_pad_len[4] <= 0;
			loc_aggr_pkt_pad_len[5] <= 0;
			loc_aggr_pkt_pad_len[6] <= 0;
			loc_aggr_pkt_pad_len[7] <= 0;
		end
		else if(loc_start_req||loc_more_data_req)begin
			if((loc_reg_aggr_pending_mpdu[tx_in_cap_ac]==0)&&(aggr_count != 0))begin
				case(start_count)
					16: loc_aggr_pkt_pad_len[0] <= tx_in_frame_data[15:14];
					17: loc_aggr_pkt_pad_len[1] <= tx_in_frame_data[15:14];
					18: loc_aggr_pkt_pad_len[2] <= tx_in_frame_data[15:14];
					19: loc_aggr_pkt_pad_len[3] <= tx_in_frame_data[15:14];
					20: loc_aggr_pkt_pad_len[4] <= tx_in_frame_data[15:14];
					21: loc_aggr_pkt_pad_len[5] <= tx_in_frame_data[15:14];
					21: loc_aggr_pkt_pad_len[6] <= tx_in_frame_data[15:14];
					23: loc_aggr_pkt_pad_len[7] <= tx_in_frame_data[15:14];
				endcase
			end
		end
		else if(loc_data_req && !loc_reg_is_ctrl_frame && (loc_fr_aggr_count!=0))begin
			case(start_count)
				16: loc_aggr_pkt_pad_len[0] <= tx_in_frame_data[15:14];
				17: loc_aggr_pkt_pad_len[1] <= tx_in_frame_data[15:14];
				18: loc_aggr_pkt_pad_len[2] <= tx_in_frame_data[15:14];
				19: loc_aggr_pkt_pad_len[3] <= tx_in_frame_data[15:14];
				20: loc_aggr_pkt_pad_len[4] <= tx_in_frame_data[15:14];
				21: loc_aggr_pkt_pad_len[5] <= tx_in_frame_data[15:14];
				21: loc_aggr_pkt_pad_len[6] <= tx_in_frame_data[15:14];
				23: loc_aggr_pkt_pad_len[7] <= tx_in_frame_data[15:14];
			endcase
		end
	end
	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_mpdu_fc0 <= 0;
		else if(!tx_en)
			loc_mpdu_fc0 <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count==24+loc_reg_read_offset)
				loc_mpdu_fc0 <= tx_in_frame_data[7:0];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_mpdu_fc1 <= 0;
		else if(!tx_en)
			loc_mpdu_fc1 <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count==24+loc_reg_read_offset)
				loc_mpdu_fc1 <= tx_in_frame_data[15:8];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_mpdu_ra <= 0;
		else if(!tx_en)
			loc_mpdu_ra <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count==25+loc_reg_read_offset)
				loc_mpdu_ra <= {16'h0,tx_in_frame_data};
			else if(start_count==26+loc_reg_read_offset)
				loc_mpdu_ra <= {tx_in_frame_data[15:0],32'h0}|loc_mpdu_ra;
		end
		else if(loc_data_req)begin
			if((start_count==25+loc_reg_read_offset)||(start_count==3+loc_reg_read_offset))
				loc_mpdu_ra <= {16'h0,tx_in_frame_data};
			else if((start_count==26+loc_reg_read_offset)||(start_count==4+loc_reg_read_offset))
				loc_mpdu_ra <= {tx_in_frame_data[15:0],32'h0}|loc_mpdu_ra;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_mpdu_scf <= 0;
		else if(!tx_en)
			loc_mpdu_scf <= 0;
		else if(loc_start_req||loc_more_data_req||loc_data_req)begin
			if(start_count==29+loc_reg_read_offset)
				loc_mpdu_scf <= tx_in_frame_data[31:16];
		end
	end

always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			tx_out_cap_ack_policy  <= 0;
			
            end
		else if(!tx_en)begin
			tx_out_cap_ack_policy  <= 0;
                  
            end
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count==30+loc_reg_read_offset)begin
				tx_out_cap_ack_policy  <= tx_in_frame_data[6:5];
                        
                  end 
		end
	end
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_mpdu_tid <= 0;
		else if(!tx_en)
			loc_mpdu_tid <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count==30+loc_reg_read_offset)
				loc_mpdu_tid <= tx_in_frame_data[3:0];
		end
		else if(loc_data_req)begin
			if((start_count==30+loc_reg_read_offset)||(start_count==8+loc_reg_read_offset))
				loc_mpdu_tid <= tx_in_frame_data[3:0];
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_mpdu_qos_ack <= 0;
		else if(!tx_en) 
			loc_mpdu_qos_ack <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count==30+loc_reg_read_offset)
				loc_mpdu_qos_ack <= tx_in_frame_data[6:5];
		end
	end	
	//================================================================================FRAME_INFO_END
	
	
	//CP module communication
	//================================================================================CP_START
//32th clock	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_cp_start_ind <= 0;
		else if(!tx_en)
			loc_cp_start_ind <= 0;	
		else if(loc_start_req)begin
			if((tx_in_RTS_Threshold < loc_upd_len)&&loc_upd_len_avl)
				loc_cp_start_ind <= 1;	
		end
		else 
			loc_cp_start_ind <= 0;	
	end
	//*************************************************************
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_cp_start_ind_dly <= 0;
		else if(!tx_en)
			loc_cp_start_ind_dly <= 0;	
		else if(loc_start_req)begin
			loc_cp_start_ind_dly <= loc_cp_start_ind;	
		end
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_cp_txvec_format <= 0;
		else if(!tx_en)
			tx_out_cp_txvec_format <= 0;	
		else if(loc_start_req||loc_more_data_req)
			tx_out_cp_txvec_format <= loc_txvec_format;
		else 
			tx_out_cp_txvec_format <= 0;	
	end
	//*************************************************************

	always@(posedge  clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_cp_rtscts_rate <= 0;
		else if(!tx_en)
			tx_out_cp_rtscts_rate <= 0;	
		else if(loc_start_req||loc_more_data_req)
			tx_out_cp_rtscts_rate <= loc_fr_rtscts_rate;
		else 
			tx_out_cp_rtscts_rate <= 0;	
	end
	//*************************************************************

	always@(posedge  clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_cp_mpdu_ra <= 0;
		else if(!tx_en)
			tx_out_cp_mpdu_ra <= 0;	
		else if(loc_start_req||loc_more_data_req)
			tx_out_cp_mpdu_ra <= loc_mpdu_ra;
		else 
			tx_out_cp_mpdu_ra <= 0;	
	end

	//================================================================================CP_END
	
	//PHY Indications
	//================================================================================PHY_IND_START
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_phy_start_req <= 0;
		else if(!tx_en)
			loc_phy_start_req <= 0;	
		else if(loc_start_req || loc_more_data_req)begin
			if(loc_upd_len_avl)
				loc_phy_start_req <= 1;		
		end
    else if(loc_ctrl_start_req)
			loc_phy_start_req <= 1;
		else
			loc_phy_start_req <= 0;
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_phy_start_req_dly <= 0;
		else if(!tx_en)
			loc_phy_start_req_dly <= 0;	
		else 
			loc_phy_start_req_dly <= loc_phy_start_req;	
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_phy_data_req <= 0;
		else if(!tx_en)
			loc_phy_data_req <= 0;	
		else if(loc_data_req || loc_ctrl_data_req)begin
			if(loc_reg_is_ctrl_frame)
				loc_phy_data_req <= 1;
			else if((aggr_count == 0)&&(loc_bitmap_updated))
				loc_phy_data_req <= 1;
			else if((loc_aggr_count < loc_fr_aggr_count)&&(loc_dur_avl))
				loc_phy_data_req <= 1;
			else 
				loc_phy_data_req <= 0;
		end
		else 
			loc_phy_data_req <= 0;	
	end 

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_phy_data_req_dly <= 0;
		else if(!tx_en)
			loc_phy_data_req_dly <= 0;	
		else 
			loc_phy_data_req_dly <= loc_phy_data_req;	
	end
	//================================================================================PHY_IND_END
	
	//Duration Calculation
	//================================================================================DUR_START
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_tx_dur_en <= 0;
		else if(!tx_en)
			loc_tx_dur_en <= 0;	
		else if ((loc_start_req  && loc_upd_len_avl) || loc_ctrl_start_req) begin
			if(loc_mpdu_ra[0] && (!(loc_mpdu_fc1 & `IEEE80211_FC1_DIR_TODS)))begin
				if(!loc_dur_avl)
					loc_tx_dur_en <= 1;
				else 
					loc_tx_dur_en <= 0;

			end
			else if(tx_in_RTS_Threshold > loc_upd_len)begin
				if(!loc_txvec_is_aggr && !loc_dur_avl)
					loc_tx_dur_en <= 1;
				else 
					loc_tx_dur_en <= 0;
			end
			else if(loc_ctrl_frame_generated)begin
				if(!loc_dur_avl)
					loc_tx_dur_en <= 1;
				else 
					loc_tx_dur_en <= 0;
			end
			else 	
				loc_tx_dur_en <= 0;
		end
		else if(loc_more_data_req && frame_info_avl_dly)begin
			if(!((((loc_txop_limit == 0)||(!tx_in_QOS_mode))&&(!(loc_mpdu_fc1 & `IEEE80211_FC1_MORE_FRAG)))&&
			      !loc_reg_pre_fr_is_ctrl))begin
				if (loc_mpdu_ra[0] && (!(loc_mpdu_fc1 & `IEEE80211_FC1_DIR_TODS)))begin 
					if(!loc_dur_avl)
						loc_tx_dur_en <= 1;
					else 
						loc_tx_dur_en <= 0;
				end
				else begin 
					if(!loc_dur_avl)
						loc_tx_dur_en <= 1;
					else 
						loc_tx_dur_en <= 0;
				end
			end
			else
				loc_tx_dur_en <= 0;
		end				
		else if(loc_data_req && frame_info_avl_dly && !loc_reg_is_ctrl_frame && aggr_count)begin
			if(!loc_dur_avl)
				loc_tx_dur_en <= 1;
			else 
				loc_tx_dur_en <= 0;
		end
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_tx_dur_type <= 0;
		else if(!tx_en)
			loc_tx_dur_type <= 0;	
		else if(loc_start_req)begin
			if(loc_mpdu_ra[0] && (!(loc_mpdu_fc1 & `IEEE80211_FC1_DIR_TODS)))
				loc_tx_dur_type <= 0;/*uu_calc_bcast_duration*/
			else if(tx_in_RTS_Threshold > loc_upd_len)
				loc_tx_dur_type <= 1;/*uu_calc_singlep_frame_duration*/	
			else 
				loc_tx_dur_type <= 2;/*uu_calc_rtscts_duration*/
		end
		else if(loc_more_data_req)begin
			if(loc_mpdu_ra[0] && (!(loc_mpdu_fc1 & `IEEE80211_FC1_DIR_TODS)))
				loc_tx_dur_type <= 0;/*uu_calc_bcast_duration*/
			else 
				loc_tx_dur_type <= 1;/*uu_calc_singlep_frame_duration*/
		end
		else if(loc_data_req && aggr_count>1)
			loc_tx_dur_type  <= 2'h3;/*uu_calc_impl_BA_duration*/
		else 
			loc_tx_dur_type <= 1;	
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_reg_dur <= 0;
		else if(!tx_en)
			loc_reg_dur <= 0;	
		else if(loc_start_req)begin
			if(loc_dur_upd_val)
				loc_reg_dur <= loc_dur_upd;
		end
		else if(loc_more_data_req)begin
			if(loc_dur_upd_val)
				loc_reg_dur <= loc_dur_upd;	
		end
		else if(loc_data_req)begin
			if(aggr_count &&loc_dur_upd_val)
				loc_reg_dur <= loc_dur_upd;
		end
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_dur_avl <= 0;
		else if(!tx_en)
			loc_dur_avl <= 0;	
		else if(loc_start_req&&frame_info_avl)begin
			if(loc_mpdu_ra[0] && (!(loc_mpdu_fc1 & `IEEE80211_FC1_DIR_TODS)))begin
				if(loc_dur_upd_val)
					loc_dur_avl <= 1;
			end
			else if(tx_in_RTS_Threshold > loc_upd_len)begin
				if(!loc_txvec_is_aggr)begin
					if(loc_dur_upd_val)
						loc_dur_avl <= 1;
				end
				else 
					loc_dur_avl <= 1;
			end
			else if(loc_ctrl_frame_generated && loc_dur_upd_val)begin	
				loc_dur_avl <= 1;
			end
		end
		else if(loc_more_data_req)begin
			if(loc_dur_upd_val)
				loc_dur_avl <= 1;	
		end
		else if(loc_data_req && !loc_reg_is_ctrl_frame && frame_info_avl && aggr_count)begin
			if(loc_dur_upd_val)
				loc_dur_avl <= 1;
		end
		else
			loc_dur_avl <= 0;
	end
	//*************************************************************added some changes
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_upd_len_avl <= 0;
		else if(!tx_en)
			loc_upd_len_avl <= 0;
		else if((loc_start_req||loc_more_data_req) && frame_info_avl)begin
			if(aggr_count)begin
				if(loc_aggr_count == loc_fr_aggr_count)begin
					if(loc_txvec_done)
						loc_upd_len_avl <= 0;
					else 
						loc_upd_len_avl <= 1;
				end
			end
			else if(loc_txvec_done)begin
				loc_upd_len_avl <= 0;
			end
			else begin
				loc_upd_len_avl <= 1;
			end
		end
		else if(loc_data_req)begin
			loc_upd_len_avl <= 0;
		end
	end
	//*************************************************************


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_upd_len <= 0;
		else if(!tx_en)
			loc_upd_len <= 0;	
		else if((loc_start_req||loc_more_data_req) && frame_info_avl)begin
			if(aggr_count)begin
				if(loc_reg_fI_retry)begin
					if((loc_aggr_count < loc_fr_aggr_count)&&((loc_reg_aggr_bitmap[tx_in_cap_ac]&(1<<loc_aggr_count))!=0)) begin
						if(loc_aggr_count == loc_aggr_final_mpdu)begin
							if(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_VHT)
								loc_upd_len <= loc_upd_len + loc_sub_frame_len_pad;
							else 
								loc_upd_len <= loc_upd_len + loc_sub_frame_len;
						end
						else begin
							loc_upd_len <= loc_upd_len + loc_sub_frame_len_pad_zd;
						end
					end				
				end
				else begin
					if(loc_aggr_count < loc_fr_aggr_count)begin
						if(loc_aggr_count == loc_aggr_final_mpdu)begin
							if(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_VHT)
								loc_upd_len <= loc_upd_len + loc_sub_frame_len_pad;
							else
								loc_upd_len <= loc_upd_len + loc_sub_frame_len;
						end
						else begin
								loc_upd_len <= loc_upd_len + loc_sub_frame_len_pad_zd;
						end
					end
				end
			end
			else 
				loc_upd_len <= loc_fI_frame_len;	
		end
		else
			loc_upd_len <= 0;	
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_tx_exp_resp_len <= 0;
		else if(!tx_en)
			loc_tx_exp_resp_len <= 0;	
		else if(loc_start_req||loc_more_data_req)
			loc_tx_exp_resp_len <= loc_resp_len;
		else if(loc_data_req)
			loc_tx_exp_resp_len <= `UU_BA_COMPRESSED_FRAME_LEN;
		else 
			loc_tx_exp_resp_len <= 0;
	end
	//*************************************************************


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_reg_is_ctrl_frame <= 0;
		else if(!tx_en)
			loc_reg_is_ctrl_frame <= 0;
		else if(loc_start_req && loc_upd_len_avl)begin
			if(tx_in_RTS_Threshold < loc_upd_len)
				loc_reg_is_ctrl_frame <= 1;
                        else if( loc_ack_req )
				loc_reg_is_ctrl_frame <= 1;
			else 
				loc_reg_is_ctrl_frame <= 0;
		end
                else if(tx_in_cap_ctrl_start_req)
			loc_reg_is_ctrl_frame <= 1;
		
		else if(loc_data_req||loc_ctrl_data_req)begin
			if(loc_data_done)
				loc_reg_is_ctrl_frame <= 0;
		end
	end
	//*************************************************************
	//================================================================================DUR_END

	always@(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_ctrl_frame_generated <= 0;
		end
		else if(!tx_en)begin
			loc_ctrl_frame_generated <= 0;
		end
		else if(tx_in_cp_res_val && (tx_in_cp_res ==`UU_SUCCESS))begin
			loc_ctrl_frame_generated <= 1;
		end
		else if(loc_txvec_done)
			loc_ctrl_frame_generated <= 0;
	end
			
	//PHY_MAC interface
	//================================================================================PHY_START
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			tx_out_phy_frame_val <= 0;
			tx_out_phy_frame     <= 0;
		end
		else if(!tx_en)begin
			tx_out_phy_frame_val <= 0;
			tx_out_phy_frame     <= 0;
		end
		else if((loc_start_req||loc_more_data_req) && loc_upd_len_avl && !loc_reg_is_ctrl_frame)begin
			//Send Txvec
			if((count < 34)&&(count>1))begin
				tx_out_phy_frame_val <= 1;
				case(count)
					8'd2 : tx_out_phy_frame <= {loc_txvec_n_tx,loc_ch_bw,tx_in_frame_data[1:0]};
					8'd3 : tx_out_phy_frame <= tx_in_frame_data[15:8];
					8'd4 : tx_out_phy_frame <= {loc_txvec_L_length[3:0],tx_in_frame_data[19:16]};
					8'd5 : tx_out_phy_frame <= loc_txvec_L_length[11:4];
					8'd6 : tx_out_phy_frame <= {tx_in_frame_data[7:5],loc_txvec_up_num_sts,tx_in_frame_data[1:0]};
					8'd7 : tx_out_phy_frame <= tx_in_frame_data[15:8 ];
					8'd8 : tx_out_phy_frame <= tx_in_frame_data[23:16];
					8'd9 : tx_out_phy_frame <= tx_in_frame_data[31:24];
					8'd10: tx_out_phy_frame <= tx_in_frame_data[7 :0 ];
					8'd11: tx_out_phy_frame <= loc_txvec_ht_length[7 :0];
					8'd12: tx_out_phy_frame <= loc_txvec_ht_length[15:8];
					8'd13: tx_out_phy_frame <= tx_in_frame_data[31:24];
					8'd14: tx_out_phy_frame <= tx_in_frame_data[7 :0 ];
					8'd15: tx_out_phy_frame <= tx_in_frame_data[15:8 ];
					8'd16: tx_out_phy_frame <= tx_in_frame_data[23:16];
					8'd17: tx_out_phy_frame <= tx_in_frame_data[31:24];
					8'd18: tx_out_phy_frame <= tx_in_frame_data[7 :0 ];
					8'd19: tx_out_phy_frame <= loc_txvec_upa_length[7 :0];
					8'd20: tx_out_phy_frame <= loc_txvec_upa_length[15:8];
					8'd21: tx_out_phy_frame <= {tx_in_frame_data[31:28],loc_txvec_upa_length[3:0]};
					8'd22: tx_out_phy_frame <= tx_in_frame_data[7 :0 ];
					8'd23: tx_out_phy_frame <= tx_in_frame_data[15:8 ];
					8'd24: tx_out_phy_frame <= tx_in_frame_data[23:16];
					8'd25: tx_out_phy_frame <= tx_in_frame_data[31:24];
					8'd26: tx_out_phy_frame <= tx_in_frame_data[7 :0 ];
					8'd27: tx_out_phy_frame <= tx_in_frame_data[15:8 ];
					8'd28: tx_out_phy_frame <= tx_in_frame_data[23:16];
					8'd29: tx_out_phy_frame <= tx_in_frame_data[31:24];
					8'd30: tx_out_phy_frame <= tx_in_frame_data[7 :0 ];
					8'd31: tx_out_phy_frame <= tx_in_frame_data[15:8 ];
					8'd32: tx_out_phy_frame <= tx_in_frame_data[23:16];
					8'd33: tx_out_phy_frame <= tx_in_frame_data[31:24];    
				endcase
			end
			else begin
				tx_out_phy_frame_val <= 0;
			end
		end
                else if(loc_ctrl_start_req || (loc_start_req && loc_reg_is_ctrl_frame))begin
			if((count < 34)&&(count>1))begin
				tx_out_phy_frame_val <= 1;
				tx_out_phy_frame <= tx_in_ctrl_data;    
			end
			else begin
				tx_out_phy_frame_val <= 0;
			end
		end
		else if(loc_data_req || loc_ctrl_data_req)begin //Send MPDU/AMPDU
			if(loc_reg_is_ctrl_frame)begin //CTRL_frame_MPDU
				if((count>0)&&(count < loc_fI_frame_len-3))begin
                    if(tx_in_phy_data_req)
					tx_out_phy_frame_val <= 1'b1;
                    else
					tx_out_phy_frame_val <= 1'b0;
                    
					if(count<5)begin
						case(count)
							8'd1 : tx_out_phy_frame <= tx_in_ctrl_data;
							8'd2 : if(loc_reg_fI_retry) 
							           tx_out_phy_frame <= tx_in_ctrl_data | `IEEE80211_FC1_RETRY;
							       else 
							           tx_out_phy_frame <= tx_in_ctrl_data;
							8'd3 : if(loc_data_req)
							          tx_out_phy_frame <= loc_reg_dur[7 :0 ];
							       else
							          tx_out_phy_frame <= tx_in_ctrl_data;
							8'd4 : if(loc_data_req)
							          tx_out_phy_frame <= loc_reg_dur[15:8 ];
							       else
							          tx_out_phy_frame <= tx_in_ctrl_data;
						endcase
					end
					else begin
              tx_out_phy_frame <= tx_in_ctrl_data;
					end
				end
				else if(tx_in_crc_avl) begin
                    if(loc_phy_req)
					tx_out_phy_frame_val <= 1'b1;
                    else
					tx_out_phy_frame_val <= 1'b0;
					case(count)
						loc_fI_frame_len+1: tx_out_phy_frame <= tx_in_crc[7 :0 ];
						loc_fI_frame_len: tx_out_phy_frame <= tx_in_crc[15:8 ];
						loc_fI_frame_len-1  : tx_out_phy_frame <= tx_in_crc[23:16];
						loc_fI_frame_len-2: tx_out_phy_frame <= tx_in_crc[31:24];					
					endcase
				end
				else begin
					tx_out_phy_frame_val <= 1'b0;
					tx_out_phy_frame     <= 1'b0;
				end
			end
			else if(aggr_count == 0)begin //NON_Aggr_MPDU
		    if((count>0)&&(count < loc_fI_frame_len-3)&&loc_bitmap_updated)begin
				  if(tx_in_phy_data_req)
				    tx_out_phy_frame_val <= 1'b1;
               else
					tx_out_phy_frame_val <= 1'b0;
					if(count<5)begin
						case(count)
							8'd1 : tx_out_phy_frame <= tx_in_frame_data[7 :0];
							8'd2 : if(loc_reg_fI_retry)
									 tx_out_phy_frame <= tx_in_frame_data[15:8] | `IEEE80211_FC1_RETRY;
							       else 
									 tx_out_phy_frame <= tx_in_frame_data[15:8];
							8'd3 : tx_out_phy_frame <= loc_reg_dur[7 :0];
							8'd4 : tx_out_phy_frame <= loc_reg_dur[15:8];
						endcase
					end
					else begin
						case(count%4)
							8'd1 : tx_out_phy_frame <= tx_in_frame_data[7 :0 ];
							8'd2 : tx_out_phy_frame <= tx_in_frame_data[15:8 ];
							8'd3 : tx_out_phy_frame <= tx_in_frame_data[23:16];
							8'd0 : tx_out_phy_frame <= tx_in_frame_data[31:24];
						endcase
					end
				end
				else if(tx_in_crc_avl) begin
                    if(tx_in_phy_data_req)
					tx_out_phy_frame_val <= 1'b1;
                 else
					tx_out_phy_frame_val <= 1'b0;
					case(count)
						loc_fI_frame_len-2: tx_out_phy_frame <= tx_in_crc[7 :0 ];
						loc_fI_frame_len-1: tx_out_phy_frame <= tx_in_crc[15:8 ];
						loc_fI_frame_len  : tx_out_phy_frame <= tx_in_crc[23:16];
						loc_fI_frame_len+1: tx_out_phy_frame <= tx_in_crc[31:24];
					endcase
				end
				else begin
					tx_out_phy_frame_val <= 1'b0;
					tx_out_phy_frame     <= 1'b0;
				end
			end
			else if((loc_aggr_count < loc_fr_aggr_count)&&loc_dur_avl)begin//Aggr_MPDU | ReAggr_MPDU
				if((loc_reg_fI_retry && ((loc_reg_aggr_bitmap[tx_in_cap_ac]&(1<<loc_aggr_count))!=0))||!loc_reg_fI_retry)begin
					if(loc_bitmap_updated&&(count < loc_sub_frame_len_pad_zd))begin
						if(count < 8)begin
              if(tx_in_phy_data_req)
                tx_out_phy_frame_val <= 1'b1;
              else
			          tx_out_phy_frame_val <= 1'b0;
							
							case(count)
								8'd0 : tx_out_phy_frame <= delimiter[7 :0 ];
								8'd1 : tx_out_phy_frame <= delimiter[15:8 ];
								8'd2 : tx_out_phy_frame <= delimiter[23:16];
								8'd3 : tx_out_phy_frame <= delimiter[31:24];
								8'd4 : tx_out_phy_frame <= tx_in_frame_data[7 :0];
								8'd5 : if(loc_reg_fI_retry)
										  tx_out_phy_frame <= tx_in_frame_data[15:8] | `IEEE80211_FC1_RETRY;
									  else 
										  tx_out_phy_frame <= tx_in_frame_data[15:8];
								8'd6 : tx_out_phy_frame <= loc_reg_dur[7 :0];
								8'd7 : tx_out_phy_frame <= loc_reg_dur[15:8];
							endcase
						end
						else if(count < loc_sub_frame_len-4)begin
						  if(tx_in_phy_data_req)
                tx_out_phy_frame_val <= 1'b1;
              else
			          tx_out_phy_frame_val <= 1'b0;
							case(count%4)
								8'd0 : tx_out_phy_frame <= tx_in_frame_data[7 :0 ];
								8'd1 : tx_out_phy_frame <= tx_in_frame_data[15:8 ];
								8'd2 : tx_out_phy_frame <= tx_in_frame_data[23:16];
								8'd3 : tx_out_phy_frame <= tx_in_frame_data[31:24];
							endcase
						end
						else if(tx_in_crc_avl) begin
                    if(tx_in_phy_data_req)
					tx_out_phy_frame_val <= 1'b1;
                    else
					tx_out_phy_frame_val <= 1'b0;
							case(count)
								loc_sub_frame_len-4 : tx_out_phy_frame <= tx_in_crc[7 :0 ];
								loc_sub_frame_len-3 : tx_out_phy_frame <= tx_in_crc[15:8 ];
								loc_sub_frame_len-2 : tx_out_phy_frame <= tx_in_crc[23:16];
								loc_sub_frame_len-1 : tx_out_phy_frame <= tx_in_crc[31:24];
							endcase
						end
						else if(loc_crc_added)begin
							if(loc_aggr_count == loc_aggr_final_mpdu)begin
								if(loc_txvec_format == `UU_WLAN_FRAME_FORMAT_VHT)begin
									if(count <= loc_sub_frame_len_pad)begin
                    if(tx_in_phy_data_req)
                      tx_out_phy_frame_val <= 1'b1;
                    else
                    		tx_out_phy_frame_val <= 1'b0;
										tx_out_phy_frame  <= `ZERO_BYTE;
									end
									else begin
										tx_out_phy_frame_val <= 1'b0;
										tx_out_phy_frame  <= 0;
									end
								end
								else begin
									tx_out_phy_frame_val <= 1'b0;
									tx_out_phy_frame  <= 0;
								end
							end
							else if(count < loc_sub_frame_len_pad)begin
                    if(tx_in_phy_data_req)
					           tx_out_phy_frame_val <= 1'b1;
                    else
                    					tx_out_phy_frame_val <= 1'b0;
								tx_out_phy_frame  <= `ZERO_BYTE;
							end
							else begin
							   if(tx_in_phy_data_req)
					         tx_out_phy_frame_val <= 1'b1;
                 else
              					tx_out_phy_frame_val <= 1'b0;
								case(count)
									loc_sub_frame_len_pad+0: tx_out_phy_frame <= loc_reg_zero_delmt[7 :0 ];
									loc_sub_frame_len_pad+1: tx_out_phy_frame <= loc_reg_zero_delmt[15:8 ];
									loc_sub_frame_len_pad+2: tx_out_phy_frame <= loc_reg_zero_delmt[23:16];
									loc_sub_frame_len_pad+3: tx_out_phy_frame <= loc_reg_zero_delmt[31:24];
									loc_sub_frame_len_pad+4: tx_out_phy_frame <= loc_reg_zero_delmt[7 :0 ];
									loc_sub_frame_len_pad+5: tx_out_phy_frame <= loc_reg_zero_delmt[15:8 ];
									loc_sub_frame_len_pad+6: tx_out_phy_frame <= loc_reg_zero_delmt[23:16];
									loc_sub_frame_len_pad+7: tx_out_phy_frame <= loc_reg_zero_delmt[31:24];
								endcase
							end
						end
						else begin
							tx_out_phy_frame_val <= 0;
							tx_out_phy_frame     <= 0;
						end
					end
					else begin
						tx_out_phy_frame_val <= 0;
						tx_out_phy_frame     <= 0;
					end
				end
			end 
			else begin
				tx_out_phy_frame_val <= 0;
				tx_out_phy_frame     <= 0;
			end
		end
	end

	//*************************************************************
	


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_crc_added <= 0;
		else if(!tx_en)
			loc_crc_added <= 0;
		else if(loc_data_req)begin
			if(count == loc_sub_frame_len-1)
				loc_crc_added <= 1;
			else if(count == loc_sub_frame_len_pad_zd)
				loc_crc_added <= 0;
		end
		else 
			loc_crc_added <= 0;
	end
	//*************************************************************

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			loc_aggr_count <= 0;
		end
		else if(!tx_en)begin
			loc_aggr_count <= 0;
		end
		else if((loc_start_req||loc_more_data_req) && frame_info_avl)begin
			if(loc_aggr_count < loc_fr_aggr_count)
				loc_aggr_count  <= loc_aggr_count+1;
		end
		else if(loc_data_req && !loc_reg_is_ctrl_frame && (loc_fr_aggr_count != 0) )begin
			if(frame_info_avl)begin
				if(loc_aggr_count < loc_fr_aggr_count)begin
					if((loc_sub_mpdu_sent && !loc_reg_fI_retry)||((loc_sub_mpdu_sent||((loc_reg_aggr_bitmap[tx_in_cap_ac]&(1<<loc_aggr_count))==0))&loc_reg_fI_retry))
						loc_aggr_count  <= loc_aggr_count+1;
				end
			end //Aggr MPDU
		end
		else 
			loc_aggr_count <= 0;
	end
	//*************************************************************
	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_aggr_final_mpdu <= 0;
		else if(!tx_en)
			loc_aggr_final_mpdu <= 0;
		else if(loc_start_req||loc_data_req||loc_more_data_req) 
			if(loc_reg_fI_retry)begin
				if((loc_reg_aggr_bitmap[tx_in_cap_ac]&8'h80)==8'h80)
					loc_aggr_final_mpdu <= 7;
				else if((loc_reg_aggr_bitmap[tx_in_cap_ac]&8'h40)==8'h40)
					loc_aggr_final_mpdu <= 6;
				else if((loc_reg_aggr_bitmap[tx_in_cap_ac]&8'h20)==8'h20)
					loc_aggr_final_mpdu <= 5;
				else if((loc_reg_aggr_bitmap[tx_in_cap_ac]&8'h10)==8'h10)
					loc_aggr_final_mpdu <= 4;
				else if((loc_reg_aggr_bitmap[tx_in_cap_ac]&8'h08)==8'h08)
					loc_aggr_final_mpdu <= 3;
				else if((loc_reg_aggr_bitmap[tx_in_cap_ac]&8'h04)==8'h04)
					loc_aggr_final_mpdu <= 2;
				else if((loc_reg_aggr_bitmap[tx_in_cap_ac]&8'h02)==8'h02)
					loc_aggr_final_mpdu <= 1;
				else if((loc_reg_aggr_bitmap[tx_in_cap_ac]&8'h01)==8'h01)
					loc_aggr_final_mpdu <= 0;
			end
			else begin
				loc_aggr_final_mpdu <= loc_fr_aggr_count-1;
			end
		else 
			loc_aggr_final_mpdu <= 0;
	end

	//================================================================================PHY_END

	//================================================================================CRC_START
	//CRC en interface
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_crc_en <= 0;
		else if(!tx_en)
			tx_out_crc_en <= 0;
		else if(loc_data_req || loc_ctrl_data_req)begin
			if((aggr_count==0)||(loc_reg_is_ctrl_frame))begin
				if(count == loc_fI_frame_len+1)
					tx_out_crc_en <= 0;
				else 
					tx_out_crc_en <= 1;
			end
			else begin
				if(count >= loc_sub_frame_len-1)
					tx_out_crc_en <= 0;
				else if(count != 0) 
					tx_out_crc_en <= 1;
			end
		end
	end

	//CRC length interface
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			tx_out_crc_len <= 0;
		else if(!tx_en)
			tx_out_crc_len <= 0;
		else if(loc_data_req || loc_ctrl_data_req)begin
			if(loc_reg_is_ctrl_frame || (aggr_count == 0))
				tx_out_crc_len <= loc_fI_frame_len-4;
			else 
				tx_out_crc_len <= loc_aggr_pkt_len[loc_aggr_count]-4;
		end
	end

	//CRC data_val & data interface
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			tx_out_crc_data_val  <= 0;
			tx_out_crc_data      <= 0;
		end
		else if(!tx_en)begin
			tx_out_crc_data_val  <= 0;
			tx_out_crc_data      <= 0;
		end
		else if(loc_data_req || loc_ctrl_data_req)begin	
			if(loc_reg_is_ctrl_frame)begin 
				if((count>0)&&(count < loc_fI_frame_len-3))begin
				  if(tx_in_phy_data_req)
					 tx_out_crc_data_val  <= 1;
					else 
					 tx_out_crc_data_val  <= 0; 
					if(count<5)begin
						case(count)
							8'd1 : tx_out_crc_data  <= tx_in_ctrl_data; 
							8'd2 : if(loc_reg_fI_retry) 
							          tx_out_crc_data  <= tx_in_ctrl_data | `IEEE80211_FC1_RETRY; 
							       else 
									tx_out_crc_data  <= tx_in_ctrl_data; 
							8'd3 : if(loc_data_req)
							          tx_out_crc_data <= loc_reg_dur[7:0 ];
							       else
							          tx_out_crc_data <= tx_in_ctrl_data; 
							8'd4 : if(loc_data_req)
							          tx_out_crc_data <= loc_reg_dur[15:8 ];
							       else
							          tx_out_crc_data <= tx_in_ctrl_data; 
						endcase
					end
					else 
						tx_out_crc_data  <= tx_in_ctrl_data; 
				end
				else begin
					tx_out_crc_data_val  <= 0;
					tx_out_crc_data      <= 0;
				end
			end
			else if(aggr_count == 0)begin 
				if((count>0)&&(count < loc_fI_frame_len-3)&&loc_bitmap_updated)begin
					if(tx_in_phy_data_req)
						tx_out_crc_data_val <= 1'b1;
					else
						tx_out_crc_data_val <= 1'b0;
					if(count<5)begin
						case(count)
							8'd1 : tx_out_crc_data  <= tx_in_frame_data[7 :0]; 
							8'd2 : if(loc_reg_fI_retry)
							            tx_out_crc_data  <= tx_in_frame_data[15:8] | `IEEE80211_FC1_RETRY; 
							       else 
							            tx_out_crc_data  <= tx_in_frame_data[15:8]; 
							8'd3 : tx_out_crc_data  <= loc_reg_dur[7 :0]; 
							8'd4 : tx_out_crc_data  <= loc_reg_dur[15:8]; 
						endcase
					end
					else begin
						case(count%4)
							8'd1 : tx_out_crc_data  <= tx_in_frame_data[7 :0 ]; 
							8'd2 : tx_out_crc_data  <= tx_in_frame_data[15:8 ]; 
							8'd3 : tx_out_crc_data  <= tx_in_frame_data[23:16]; 
							8'd0 : tx_out_crc_data  <= tx_in_frame_data[31:24]; 
						endcase
					end
				end
				else begin
					tx_out_crc_data_val <= 0;
					tx_out_crc_data     <= 0;
				end
			end
			else if((loc_aggr_count < loc_fr_aggr_count)&&loc_dur_avl)begin
				if(loc_reg_fI_retry&&((loc_reg_aggr_bitmap[tx_in_cap_ac]&(1<<loc_aggr_count))!=0))begin
					if(loc_bitmap_updated&&(count < loc_sub_frame_len-4)&&(count>3))begin
						if(loc_phy_req)
							tx_out_crc_data_val <= 1'b1;
						else
							tx_out_crc_data_val <= 1'b0;
						if(count<8)begin
							case(count)
								8'd4 : tx_out_crc_data  <= tx_in_frame_data[7 :0]; 
								8'd5 : tx_out_crc_data  <= tx_in_frame_data[15:8] | `IEEE80211_FC1_RETRY; 
								8'd6 : tx_out_crc_data  <= loc_reg_dur[7 :0]; 
								8'd7 : tx_out_crc_data  <= loc_reg_dur[15:8]; 
							endcase
						end
						else begin
							case(count%4)
								8'd0 : tx_out_crc_data  <= tx_in_frame_data[7 :0 ]; 
								8'd1 : tx_out_crc_data  <= tx_in_frame_data[15:8 ]; 
								8'd2 : tx_out_crc_data  <= tx_in_frame_data[23:16]; 
								8'd3 : tx_out_crc_data  <= tx_in_frame_data[31:24]; 
							endcase
						end
					end
					else begin 
						tx_out_crc_data_val <= 0;
						tx_out_crc_data     <= 0;
					end
				end
				else if(!loc_reg_fI_retry)begin
					if(loc_bitmap_updated&&(count < loc_sub_frame_len-4)&&(count>3))begin
						if(loc_phy_req)
							tx_out_crc_data_val <= 1'b1;
						else
							tx_out_crc_data_val <= 1'b0;
						if(count<8)begin
							case(count)
								8'd4 : tx_out_crc_data  <= tx_in_frame_data[7 :0]; 
								8'd5 : tx_out_crc_data  <= tx_in_frame_data[15:8]; 
								8'd6 : tx_out_crc_data  <= loc_reg_dur[7 :0]; 
								8'd7 : tx_out_crc_data  <= loc_reg_dur[15:8]; 
							endcase
						end
						else begin
							case(count%4)
								8'd0 : tx_out_crc_data  <= tx_in_frame_data[7 :0 ]; 
								8'd1 : tx_out_crc_data  <= tx_in_frame_data[15:8 ]; 
								8'd2 : tx_out_crc_data  <= tx_in_frame_data[23:16]; 
								8'd3 : tx_out_crc_data  <= tx_in_frame_data[31:24]; 
							endcase
						end
					end
					else begin
						tx_out_crc_data_val <= 0;
						tx_out_crc_data     <= 0;
					end	
				end				
			end
			else begin
				tx_out_crc_data_val <= 0;
				tx_out_crc_data     <= 0;
			end
		end
	end
	//================================================================================CRC_END

	//Responses to CP, CAP and PHY
	//================================================================================RESP_START
	//CAP response
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)begin
			tx_out_cap_res_val   <= 0;
			tx_out_cap_res       <= `UU_SUCCESS;
		end
		else if(!tx_en)begin
			tx_out_cap_res_val   <= 0;
			tx_out_cap_res       <= `UU_SUCCESS;
		end
		else if(loc_start_req)begin
			if(loc_txvec_done)begin
				tx_out_cap_res_val <= 1;
				tx_out_cap_res     <= `UU_SUCCESS;
			end
			else if((tx_in_cp_res == `UU_FAILURE) && tx_in_cp_res_val)begin
				tx_out_cap_res_val   <= 1;
				tx_out_cap_res       <= `UU_FAILURE;
			end
		end
                else if(loc_ctrl_start_req)begin
			if(loc_txvec_done)begin
				tx_out_cap_res_val <= 1;
				tx_out_cap_res     <= `UU_SUCCESS;
			end			
		end
		else if(loc_more_data_req && frame_info_avl)begin
			if(loc_txvec_done)begin
				tx_out_cap_res_val <= 1;
				tx_out_cap_res     <= `UU_SUCCESS;
			end
			else if ((((loc_txop_limit == 0)||!tx_in_QOS_mode)&&(!(loc_mpdu_fc1 & `IEEE80211_FC1_MORE_FRAG)))&&
			     !loc_reg_pre_fr_is_ctrl) begin
				tx_out_cap_res_val <= 1'b1;
				tx_out_cap_res     <= `UU_FAILURE;
			end
		end
		else begin
			tx_out_cap_res_val <= 0;
			tx_out_cap_res     <= `UU_SUCCESS;
		end

	end
// added 09/10/13
	always@(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
			tx_out_cap_data_end_confirm <= 0;
		end
		else if(!tx_en)begin
			tx_out_cap_data_end_confirm <= 0;
		end
		else if(loc_data_done)begin
			if(loc_fr_aggr_count == 0)begin
				tx_out_cap_data_end_confirm <= 1;
			end
			else if(loc_aggr_count == loc_fr_aggr_count)begin
				tx_out_cap_data_end_confirm <= 1;
			end
			else begin
				tx_out_cap_data_end_confirm <= 0;
			end
		end
		else begin
			tx_out_cap_data_end_confirm <= 0;
		end
	end
	
	always@(posedge clk or `EDGE_OF_RESET) begin
		if(`POLARITY_OF_RESET)begin
			tx_out_cp_res_val <= 0;
			tx_out_cp_res     <= `UU_FAILURE;
		end
		else if(!tx_en)begin
			tx_out_cp_res_val <= 0;
			tx_out_cp_res     <= `UU_FAILURE;
		end
		else if(tx_in_cp_get_ampdu_status)begin
			if(loc_reg_aggr_bitmap[tx_in_cp_ac]==0)begin
				tx_out_cp_res_val <= 1;
				tx_out_cp_res     <= `UU_SUCCESS;
			end
			else begin
				tx_out_cp_res_val <= 1;
				tx_out_cp_res     <= `UU_FAILURE;
			end
		end
		else begin
			tx_out_cp_res_val <= 0;
			tx_out_cp_res     <= `UU_FAILURE;
		end
	end
	//================================================================================RESP_END

	//Internal Signals 
	//================================================================================SIG_START

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_txvec_done <= 0;
		else if(!tx_en)
			loc_txvec_done <= 0;
		else if(loc_start_req||loc_more_data_req||loc_ctrl_start_req)begin
			if(count == 34)
				loc_txvec_done <= 1;
			else 
				loc_txvec_done <= 0;
		end
	end	

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_data_done <= 0;
		else if(!tx_en)
			loc_data_done <= 0;
		else if(loc_data_req || loc_ctrl_data_req)begin
			if(loc_reg_is_ctrl_frame || (aggr_count == 0))begin
				if(count > loc_fI_frame_len+1)
					loc_data_done <= 1;
				else 
					loc_data_done <= 0;
			end
			else if((loc_sub_mpdu_sent)&&(loc_aggr_count == loc_aggr_final_mpdu))begin
				loc_data_done <= 1;
			end
			else 
				loc_data_done <= 0;
		end
		else 	
			loc_data_done <= 0;
	end


	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			frame_info_avl <= 0;
		else if(!tx_en)
			frame_info_avl <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(start_count == 30+loc_reg_read_offset)
				frame_info_avl <= 1;
			else if(loc_txvec_done)
				frame_info_avl <= 0;
		end
		else if(loc_data_req)begin
			if((start_count == 30+loc_reg_read_offset)||(start_count == 10 + loc_reg_read_offset))
				frame_info_avl <= 1;
			else if(loc_data_done||loc_sub_mpdu_sent)
				frame_info_avl <= 0;
		end
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			frame_info_avl_dly <= 0;
		else if(!tx_en)
			frame_info_avl_dly <= 0;
		else 
			frame_info_avl_dly <= frame_info_avl;
	end

//start_count always for getting valuable info from buffer
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			start_count <= 0;
		else if(!tx_en)
			start_count <= 0;
		else if(loc_start_req||loc_more_data_req)begin
			if(frame_info_avl)
				start_count <= 0;
			else 
				start_count <= start_count + 1;
		end
		else if(loc_data_req && !loc_reg_is_ctrl_frame)begin
			if(frame_info_avl)
				start_count <= 0;
			else begin
				if(loc_aggr_count==0)begin
					if(start_count==0)
						start_count <= start_count + 1 + `MPDU_OFFSET;
					else
						start_count <= start_count + 1;
				end
				else if(start_count == 0)begin
					start_count <= loc_reg_read_offset + 27;
				end
				else 
					start_count <= start_count + 1;
			end
		end
	end
	
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			count  <= 0;
		else if(!tx_en)
			count  <= 0;
		else if(loc_start_req)begin
			if(loc_txvec_done)
				count  <= 0;
			else if(loc_upd_len_avl)begin
				if(tx_in_RTS_Threshold < loc_upd_len)begin
					if(loc_ctrl_frame_generated)
						count  <= count + 1;
				end
				else begin
					count  <= count + 1;
				end
			end
		end
    else if(loc_ctrl_start_req)begin
    if (loc_txvec_done)
                    count <= 0;
    else
			 count <= count +1;
			
		end
		else if(loc_more_data_req)begin
			if(loc_txvec_done)
				count  <= 0;
			else if(loc_upd_len_avl)begin
				count  <= count + 1;
			end
		end
		else if(loc_data_req||loc_ctrl_data_req)begin
			if(loc_data_done)
				count <= 0;
			else if(loc_reg_is_ctrl_frame)begin 
				if(count < loc_fI_frame_len-2)begin
	        if(tx_in_phy_data_req)
					  count <= count + 1;
					else if(count<1)
					  count <= count + 1;					  
				end
				else if(tx_in_crc_avl)begin
				  if(tx_in_phy_data_req)
					  count <= count + 1;
        end
			end
			else if(aggr_count == 0)begin
				if(loc_bitmap_updated)begin
					if(count < loc_fI_frame_len-2)begin
            if(tx_in_phy_data_req)
						  count <= count + 1;
					  else if(count<1)
					    count <= count + 1;											  
					end
					else if(tx_in_crc_avl)begin
            if(tx_in_phy_data_req)
						  count <= count + 1;
					end
				end
				else 
					count <= 0;
			end
			else if((loc_aggr_count < loc_fr_aggr_count)&&loc_dur_avl&&loc_bitmap_updated)begin
				if(loc_reg_fI_retry)begin
					if((count < loc_aggr_pkt_len[loc_aggr_count])&&((loc_reg_aggr_bitmap[tx_in_cap_ac]&(1<<loc_aggr_count))!=0))begin
									if(tx_in_phy_data_req/* && (count>1)*/)
											count <= count + 1;
									//else if(count<=1)
									//		count <= count + 1;						  					  
					end
					else if(count == loc_sub_frame_len_pad_zd)
						count <= 0;
					else if(tx_in_crc_avl||loc_crc_added)begin
            if(tx_in_phy_data_req)
					    count <= count + 1;
							end
					end
				else begin
					if(count < loc_sub_frame_len-4)begin
            if(tx_in_phy_data_req /*&& (count>1)*/)
						  count <= count + 1;
					  //else if(count<=1)
					  //  count <= count + 1;											  
				  end
					else if(count == loc_sub_frame_len_pad_zd)
						count <= 0;
					else if(tx_in_crc_avl ||loc_crc_added)begin
            if(tx_in_phy_data_req)
		  				  count <= count + 1;
		  			end
				end
			end

		end
	end
	

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_bmp_en <= 0;
		else if(!tx_en)
			loc_bmp_en <= 0;
		else if(loc_bitmap_updated || loc_data_done)
			loc_bmp_en <= 0;
		else if(loc_data_req && !loc_reg_is_ctrl_frame && frame_info_avl)
			loc_bmp_en <= 1;
		else
			loc_bmp_en <= 0;
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_bitmap_updated <= 0;
		else if(!tx_en)
			loc_bitmap_updated <= 0;
		else if(loc_bmp_res_val)
			loc_bitmap_updated <= 1;
		else if(loc_data_done)
			loc_bitmap_updated <= 0;
		else if(loc_sub_mpdu_sent)
			loc_bitmap_updated <= 0;
	end

	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_sub_mpdu_sent1 <= 0;
		else if(!tx_en)
			loc_sub_mpdu_sent1 <= 0;
		else if(loc_data_req && aggr_count && loc_dur_avl)begin
			if(count == loc_sub_frame_len_pad_zd-1)
				loc_sub_mpdu_sent1 <= 1;
			else 
				loc_sub_mpdu_sent1 <= 0;
		end
	end
	always @(posedge clk or `EDGE_OF_RESET)begin
		if(`POLARITY_OF_RESET)
			loc_sub_mpdu_sent_dly <= 0;
		else if(!tx_en)
			loc_sub_mpdu_sent_dly <= 0;
		else  
			loc_sub_mpdu_sent_dly <= loc_sub_mpdu_sent1;
	end



	//================================================================================SIG_END


	/*---------------------------------------------------------------------------------------------------------*/
endmodule
/*-------------------------------
  .............EoF..............
  -------------------------------*/

