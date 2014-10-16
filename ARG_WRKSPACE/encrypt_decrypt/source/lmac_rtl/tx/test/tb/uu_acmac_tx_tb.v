`include "../../inc/defines.h"
`timescale 1ns/1ns

module uu_acmac_tx_tb;

	reg           clk                      ;
	reg           rst_n                    ;
	reg           tx_en                    ;
	reg           tx_in_cap_data_req       ;
	reg           tx_in_cap_start_req      ;
        reg           tx_in_cap_send_ack       ;
	reg           tx_in_cap_more_data_req  ;
	reg           tx_in_cap_fb_update      ;
	reg           tx_in_cp_upd_mpdu_status ;
	reg           tx_in_cp_get_ampdu_status;
	reg           tx_in_clear_ampdu        ;
	reg           tx_in_clear_mpdu         ;
	reg           tx_in_cap_retry          ;
	reg   [2 :0]  tx_in_cap_ch_bw          ;
	reg   [2 :0]  tx_in_cap_ac             ;
	reg   [15:0]  tx_in_cap_ac0_tx_nav     ;
	reg   [15:0]  tx_in_cap_ac1_tx_nav     ;
	reg   [15:0]  tx_in_cap_ac2_tx_nav     ;
	reg   [15:0]  tx_in_cap_ac3_tx_nav     ;
	reg   [7 :0]  tx_in_cap_ac0_src        ;    
	reg   [7 :0]  tx_in_cap_ac1_src        ;
	reg   [7 :0]  tx_in_cap_ac2_src        ;
	reg   [7 :0]  tx_in_cap_ac3_src        ;
	reg   [7 :0]  tx_in_cap_ac0_lrc        ;
	reg   [7 :0]  tx_in_cap_ac1_lrc        ;
	reg   [7 :0]  tx_in_cap_ac2_lrc        ;
	reg   [7 :0]  tx_in_cap_ac3_lrc        ;
	reg   [15:0]  tx_in_BSSBasicRateSet    ;
	reg   [15:0]  tx_in_SIFS_timer_value   ;
	reg   [15:0]  tx_in_TXOP_limit_ac0     ;
	reg   [15:0]  tx_in_TXOP_limit_ac1     ;
	reg   [15:0]  tx_in_TXOP_limit_ac2     ;
	reg   [15:0]  tx_in_TXOP_limit_ac3     ;
	reg           tx_in_QOS_mode           ;
	reg   [15:0]  tx_in_RTS_Threshold      ;
	reg   [15:0]  tx_in_Long_Retry_Count   ;
	reg   [1 :0]  tx_in_cp_ac              ;	
	wire          tx_out_cp_start_ind      ;
	wire  [1 :0]  tx_out_cp_txvec_format   ;
	wire  [3 :0]  tx_out_cp_rtscts_rate    ;
	wire  [47:0]  tx_out_cp_mpdu_ra        ;
	reg   [7 :0]  tx_in_cp_ctrl_fr_len     ;
	reg   [15:0]  tx_in_cp_seqno           ;
	wire          tx_out_cp_res_val        ;
	wire  [31:0]  tx_out_cp_res            ;		
	reg           tx_in_cp_res_val         ;
	reg   [31:0]  tx_in_cp_res             ;
	reg           tx_in_sta_info_val       ;
	reg           tx_in_sta_info           ;
	reg   [13:0]  tx_in_sta_offset         ;
	wire          tx_out_sta_get_info      ;
	wire  [47:0]  tx_out_sta_addr          ;
	wire  [3 :0]  tx_out_sta_tid           ;
	wire          tx_out_sta_dir           ;		
	reg   [31:0]  tx_in_sta_ba_data        ;
	wire  [13:0]  tx_out_sta_ba_addr       ;
	wire  [31:0]  tx_out_sta_ba_data       ;
	wire          tx_out_sta_ba_en         ;
	wire  [3 :0]  tx_out_sta_ba_wen        ;
	wire  [1 :0]  tx_out_frame_ac          ;
	wire  [31:0]  tx_in_frame_data         ;
	wire  [31:0]  tx_out_frame_data        ;
	wire  [11:0]  tx_out_frame_addr        ;
	wire          tx_out_frame_en          ;
	wire          tx_out_frame_wen         ;
	reg   [7 :0]  tx_in_ctrl_data          ;
	wire  [7 :0]  tx_out_ctrl_data         ;
	wire  [7 :0]  tx_out_ctrl_addr         ;
	wire          tx_out_ctrl_en           ;
	wire          tx_out_ctrl_wen          ;
	wire          tx_out_vht_mcs_en        ;
	wire  [9 :0]  tx_out_vht_mcs_addr      ;
	reg   [39:0]  tx_in_vht_mcs_data       ;
	wire          tx_out_ht_mcs_en         ;
	wire  [7 :0]  tx_out_ht_mcs_addr       ;
	reg   [39:0]  tx_in_ht_mcs_data        ;
	wire          tx_out_lrate_en          ;
	wire  [5 :0]  tx_out_lrate_addr        ;
	reg   [39:0]  tx_in_lrate_data         ;
	wire   [11:0]  tx_in_buf_base_ptr      ;
	wire  [19:0]  tx_out_buf_pkt_len       ;
	wire          tx_in_crc_avl            ;
	wire  [31:0]  tx_in_crc                ;
	wire  [15:0]  tx_out_crc_len           ;
	wire          tx_out_crc_en            ;
	wire          tx_out_crc_data_val      ;
	wire  [7 :0]  tx_out_crc_data          ;
	wire          tx_out_cap_res_val       ;
	wire  [31:0]  tx_out_cap_res           ;
        reg           tx_in_phy_data_req       ;
	wire          tx_out_phy_start_req     ;
	wire          tx_out_phy_data_req      ;
	wire  [7 :0]  tx_out_phy_frame         ;	
	wire          tx_out_phy_frame_val     ;	
	wire  [11:0]  tx_out_sta_seqno_ac0     ;
	wire  [11:0]  tx_out_sta_seqno_ac1     ;
	wire  [11:0]  tx_out_sta_seqno_ac2     ;
	wire  [11:0]  tx_out_sta_seqno_ac3     ;
	wire  [7 :0]  tx_out_sta_bitmap_ac0    ;
	wire  [7 :0]  tx_out_sta_bitmap_ac1    ;
	wire  [7 :0]  tx_out_sta_bitmap_ac2    ;
	wire  [7 :0]  tx_out_sta_bitmap_ac3    ;
	wire  [19:0]  tx_out_sta_frame_len     ;

	/*--------------TX MEM BUF--------------*/
	reg   [1 :0]  tx_buf_in_dma_ac         ;
	reg           tx_buf_in_dma_wen        ;
	reg   [31:0]  tx_buf_in_dma_data       ;
	
	/*--------tx_ctrl_mem & tx_mem---------*/
	wire          mem_tx_ctrl_in_en  ; 
	wire          mem_tx_ctrl_in_wen ;
	wire  [7 :0]  mem_tx_ctrl_in_addr;
	wire  [7 :0]  mem_tx_ctrl_in_data;
	wire  [7 :0]  mem_tx_ctrl_out_data;
	wire  [39:0]  vht_mem_data_out  ;
	wire  [39:0]  ht_mem_data_out   ;
	wire  [39:0]  lrate_mem_data_out;
	/*------------ba memory ----------*/
	reg   [31:0]  ba_memory [0:67];
	reg   [31:0]  loc_sta_ba_data      ;
	reg   [47:0]  mac_addr[0:7];
	/*------------mcs memory ---------*/
 	wire [39:0] vht_memory   [0:719];
	wire [39:0] ht_memory    [0:159];
	wire [39:0] lrate_memory [0:35 ];
	/*---------tx memories testbench writes--------*/
	reg          mem_ctrl_in_en  ; 
	reg          mem_ctrl_in_wen ;
	reg  [7 :0]  mem_ctrl_in_addr;
	reg  [7 :0]  mem_ctrl_in_data;


	uu_acmac_tx_handler LMAC_TX(	
		.clk                      (clk                      ),
		.rst_n                    (rst_n                    ),
		.tx_en                    (tx_en                    ),
		.tx_in_cap_data_req       (tx_in_cap_data_req       ),
		.tx_in_cap_start_req      (tx_in_cap_start_req      ),
                .tx_in_cap_send_ack       (tx_in_cap_send_ack       ),
		.tx_in_cap_more_data_req  (tx_in_cap_more_data_req  ),
		.tx_in_cap_fb_update      (tx_in_cap_fb_update      ),
		.tx_in_cp_upd_mpdu_status (tx_in_cp_upd_mpdu_status ),
		.tx_in_cp_get_ampdu_status(tx_in_cp_get_ampdu_status),
		.tx_in_clear_ampdu        (tx_in_clear_ampdu        ),
		.tx_in_clear_mpdu         (tx_in_clear_mpdu         ),
		.tx_in_cap_retry          (tx_in_cap_retry          ),
		.tx_in_cap_ch_bw          (tx_in_cap_ch_bw          ),
		.tx_in_cap_ac             (tx_in_cap_ac             ),
		.tx_in_cap_ac0_tx_nav     (tx_in_cap_ac0_tx_nav     ),
		.tx_in_cap_ac1_tx_nav     (tx_in_cap_ac1_tx_nav     ),
		.tx_in_cap_ac2_tx_nav     (tx_in_cap_ac2_tx_nav     ),
		.tx_in_cap_ac3_tx_nav     (tx_in_cap_ac3_tx_nav     ),
		.tx_in_cap_ac0_src        (tx_in_cap_ac0_src        ),    
		.tx_in_cap_ac1_src        (tx_in_cap_ac1_src        ),    
         	.tx_in_cap_ac2_src        (tx_in_cap_ac2_src        ),
         	.tx_in_cap_ac3_src        (tx_in_cap_ac3_src        ),    
         	.tx_in_cap_ac0_lrc        (tx_in_cap_ac0_lrc        ),
         	.tx_in_cap_ac1_lrc        (tx_in_cap_ac1_lrc        ),
		.tx_in_cap_ac2_lrc        (tx_in_cap_ac2_lrc        ),
         	.tx_in_cap_ac3_lrc        (tx_in_cap_ac3_lrc        ),
		.tx_in_BSSBasicRateSet    (tx_in_BSSBasicRateSet    ),
		.tx_in_SIFS_timer_value   (tx_in_SIFS_timer_value   ),
		.tx_in_TXOP_limit_ac0     (tx_in_TXOP_limit_ac0     ),
		.tx_in_TXOP_limit_ac1     (tx_in_TXOP_limit_ac1     ),
		.tx_in_TXOP_limit_ac2     (tx_in_TXOP_limit_ac2     ),
		.tx_in_TXOP_limit_ac3     (tx_in_TXOP_limit_ac3     ),
		.tx_in_QOS_mode           (tx_in_QOS_mode           ),
		.tx_in_RTS_Threshold      (tx_in_RTS_Threshold      ),
		.tx_in_Long_Retry_Count   (tx_in_Long_Retry_Count   ),
		.tx_in_cp_ac              (tx_in_cp_ac              ),
		.tx_out_cp_start_ind      (tx_out_cp_start_ind      ),
		.tx_out_cp_txvec_format   (tx_out_cp_txvec_format   ),
		.tx_out_cp_rtscts_rate    (tx_out_cp_rtscts_rate    ),
		.tx_out_cp_mpdu_ra        (tx_out_cp_mpdu_ra        ),
		.tx_in_cp_ctrl_fr_len     (tx_in_cp_ctrl_fr_len     ),
		.tx_in_cp_seqno           (tx_in_cp_seqno           ),
		.tx_out_cp_res_val        (tx_out_cp_res_val        ),
		.tx_out_cp_res            (tx_out_cp_res            ),		
		.tx_in_cp_res_val         (tx_in_cp_res_val         ),
		.tx_in_cp_res             (tx_in_cp_res             ),
		.tx_in_sta_info_val       (tx_in_sta_info_val       ),
		.tx_in_sta_info           (tx_in_sta_info           ),
		.tx_in_sta_offset         (tx_in_sta_offset         ),
		.tx_out_sta_get_info      (tx_out_sta_get_info      ),
		.tx_out_sta_addr          (tx_out_sta_addr          ),
		.tx_out_sta_tid           (tx_out_sta_tid           ),
		.tx_out_sta_dir           (tx_out_sta_dir           ),		
		.tx_out_sta_seqno_ac0     (tx_out_sta_seqno_ac0     ), 
		.tx_out_sta_seqno_ac1     (tx_out_sta_seqno_ac1     ),
		.tx_out_sta_seqno_ac2     (tx_out_sta_seqno_ac2     ),
		.tx_out_sta_seqno_ac3     (tx_out_sta_seqno_ac3     ),
		.tx_out_sta_bitmap_ac0    (tx_out_sta_bitmap_ac0    ),
		.tx_out_sta_bitmap_ac1    (tx_out_sta_bitmap_ac1    ),
		.tx_out_sta_bitmap_ac2    (tx_out_sta_bitmap_ac2    ),
		.tx_out_sta_bitmap_ac3    (tx_out_sta_bitmap_ac3    ),
		.tx_out_sta_frame_len     (tx_out_sta_frame_len     ),
		.tx_in_sta_ba_data        (tx_in_sta_ba_data        ),
		.tx_out_sta_ba_addr       (tx_out_sta_ba_addr       ),
		.tx_out_sta_ba_data       (tx_out_sta_ba_data       ),
		.tx_out_sta_ba_en         (tx_out_sta_ba_en         ),
		.tx_out_sta_ba_wen        (tx_out_sta_ba_wen        ),
		.tx_out_frame_ac          (tx_out_frame_ac          ),
		.tx_in_frame_data         (tx_in_frame_data         ),
		.tx_out_frame_data        (tx_out_frame_data        ),
		.tx_out_frame_addr        (tx_out_frame_addr        ),
		.tx_out_frame_en          (tx_out_frame_en          ),
		.tx_out_frame_wen         (tx_out_frame_wen         ),
		.tx_in_ctrl_data          (mem_tx_ctrl_out_data     ),
		.tx_out_ctrl_data         (tx_out_ctrl_data         ),
		.tx_out_ctrl_addr         (tx_out_ctrl_addr         ),
		.tx_out_ctrl_en           (tx_out_ctrl_en           ),
		.tx_out_ctrl_wen          (tx_out_ctrl_wen          ),
		.tx_out_vht_mcs_en        (tx_out_vht_mcs_en        ),
		.tx_out_vht_mcs_addr      (tx_out_vht_mcs_addr      ),
		.tx_in_vht_mcs_data       (tx_in_vht_mcs_data       ),
		.tx_out_ht_mcs_en         (tx_out_ht_mcs_en         ),
		.tx_out_ht_mcs_addr       (tx_out_ht_mcs_addr       ),
		.tx_in_ht_mcs_data        (tx_in_ht_mcs_data        ),
		.tx_out_lrate_en          (tx_out_lrate_en          ),
		.tx_out_lrate_addr        (tx_out_lrate_addr        ),
		.tx_in_lrate_data         (tx_in_lrate_data         ),
		.tx_in_buf_base_ptr       (tx_in_buf_base_ptr       ),
		.tx_out_buf_pkt_len       (tx_out_buf_pkt_len       ),
		.tx_in_crc_avl            (tx_in_crc_avl            ),
		.tx_in_crc                (tx_in_crc                ),
		.tx_out_crc_len           (tx_out_crc_len           ),
		.tx_out_crc_en            (tx_out_crc_en            ),
		.tx_out_crc_data_val      (tx_out_crc_data_val      ),
		.tx_out_crc_data          (tx_out_crc_data          ),
		.tx_out_cap_res_val       (tx_out_cap_res_val       ),
		.tx_out_cap_res           (tx_out_cap_res           ),
                .tx_in_phy_data_req       (tx_in_phy_data_req       ),
		.tx_out_phy_start_req     (tx_out_phy_start_req     ),
		.tx_out_phy_data_req      (tx_out_phy_data_req      ),
		.tx_out_phy_frame         (tx_out_phy_frame         ),
		.tx_out_phy_frame_val     (tx_out_phy_frame_val     )
		);	
		
	/*------Memory_Tx_ctrl------*/
	uu_acmac_mem_tx_ctrl MEM_TX_CTRL(
		.clk                  (clk                 ),
		.mem_tx_ctrl_in_en    (mem_tx_ctrl_in_en   ),
		.mem_tx_ctrl_in_wen   (mem_tx_ctrl_in_wen  ),
		.mem_tx_ctrl_in_addr  (mem_tx_ctrl_in_addr ),
		.mem_tx_ctrl_in_data  (mem_tx_ctrl_in_data ),
		.mem_tx_ctrl_out_data (mem_tx_ctrl_out_data)
		);

	//----CRC---------//
	uu_acmac_crc32_8in CRC_32 (
		.clk      (clk                ),
		.rst_n    (rst_n              ),
		.crc_en   (tx_out_crc_en      ),
		.data_val (tx_out_crc_data_val),
		.data     (tx_out_crc_data    ),
		.len      (tx_out_crc_len     ),
		.crc_avl  (tx_in_crc_avl      ),
		.crc      (tx_in_crc          )
		);

        uu_acmac_tx_buf        U_WLAN_TX_BUF(
                .clk                        (clk                ),
               	.rst_n                      (rst_n              ),
                .tx_buf_in_dma_ac           (tx_buf_in_dma_ac   ),
                .tx_buf_in_dma_wen          (tx_buf_in_dma_wen  ),
                .tx_buf_in_dma_data         (tx_buf_in_dma_data ),        
       	        .tx_buf_in_clear_mpdu       (tx_in_clear_mpdu   ),
       	        .tx_buf_in_clear_ampdu      (tx_in_clear_ampdu  ),
	        .tx_buf_in_ac               (tx_out_frame_ac    ),
	        .tx_buf_out_frame_data      (tx_in_frame_data   ),
	        .tx_buf_in_frame_addr       (tx_out_frame_addr  ),
	        .tx_buf_in_frame_en         (tx_out_frame_en    ),
                .tx_buf_out_base_ptr        (tx_in_buf_base_ptr ),
                .tx_buf_in_pkt_len          (tx_out_buf_pkt_len )
                );

// Always Block to generate STA & BA Data
always @(tx_out_sta_ba_en,tx_out_sta_ba_wen,tx_out_sta_ba_data,tx_out_sta_ba_addr) begin
	if(tx_out_sta_ba_en)begin //{
		case(tx_out_sta_ba_wen)//{
			4'd3 :ba_memory[tx_out_sta_ba_addr] = {loc_sta_ba_data[31:16],tx_out_sta_ba_data};
			4'd12:ba_memory[tx_out_sta_ba_addr] = {tx_out_sta_ba_data,loc_sta_ba_data[15:0]};
			4'd15:ba_memory[tx_out_sta_ba_addr] = tx_out_sta_ba_data;
		endcase //}
		loc_sta_ba_data = ba_memory[tx_out_sta_ba_addr];
	end//}
	else
		loc_sta_ba_data = 0;
end

always @(posedge clk) begin tx_in_sta_ba_data <= loc_sta_ba_data; #1;end

always @(posedge clk) 
begin
	if(tx_out_sta_get_info)begin
		if((mac_addr[0]==tx_out_sta_addr)||
		   (mac_addr[1]==tx_out_sta_addr)||
		   (mac_addr[2]==tx_out_sta_addr)||
		   (mac_addr[3]==tx_out_sta_addr)||
		   (mac_addr[4]==tx_out_sta_addr)||
		   (mac_addr[5]==tx_out_sta_addr)||
		   (mac_addr[6]==tx_out_sta_addr)||
		   (mac_addr[7]==tx_out_sta_addr))begin
			tx_in_sta_info_val <= 1; 
			tx_in_sta_info <= 1; 
		end
		else begin
			tx_in_sta_info_val <= 1; 
			tx_in_sta_info <= 0; 
		end
	end 
	else begin
		tx_in_sta_info_val <= 0; 
		tx_in_sta_info <= 0; 
	end
end

always @(posedge clk)
begin
	if(tx_out_cp_start_ind)begin
		tx_in_cp_res_val <= 1;
	end
	else begin
		tx_in_cp_res_val <= 0;
	end
end
 
	assign mem_tx_ctrl_in_en   = tx_out_ctrl_en  | mem_ctrl_in_en  ; 
	assign mem_tx_ctrl_in_wen  = tx_out_ctrl_wen | mem_ctrl_in_wen ;
	assign mem_tx_ctrl_in_addr = tx_out_ctrl_addr| mem_ctrl_in_addr;   
	assign mem_tx_ctrl_in_data = tx_out_ctrl_data| mem_ctrl_in_data;  

	assign vht_memory[0  ] = 40'd42288908760;
	assign vht_memory[1  ] = 40'd20361326440;
	assign vht_memory[2  ] = 40'd9397535280;
	assign vht_memory[3  ] = 40'd4698767640;
	assign vht_memory[4  ] = 40'd39268272420;
	assign vht_memory[5  ] = 40'd18325193796;
	assign vht_memory[6  ] = 40'd8457781752;
	assign vht_memory[7  ] = 40'd4228890876;
	assign vht_memory[8  ] = `UU_HW_RATE_6M;
	assign vht_memory[9  ] = 40'd21144454380;
	assign vht_memory[10 ] = 40'd10180663220;
	assign vht_memory[11 ] = 40'd4698767640;
	assign vht_memory[12 ] = 40'd2349383820;
	assign vht_memory[13 ] = 40'd19289677680;
	assign vht_memory[14 ] = 40'd9162596898;
	assign vht_memory[15 ] = 40'd4228890876;
	assign vht_memory[16 ] = 40'd2114445438;
	assign vht_memory[17 ] = `UU_HW_RATE_12M;
	assign vht_memory[18 ] = 40'd14096302920;
	assign vht_memory[19 ] = 40'd6787108813;
	assign vht_memory[20 ] = 40'd3132511760;
	assign vht_memory[21 ] = 40'd1566255880;
	assign vht_memory[22 ] = 40'd12785018927;
	assign vht_memory[23 ] = 40'd6108397932;
	assign vht_memory[24 ] = 40'd2819260584;
	assign vht_memory[25 ] = 40'd1409630292;
	assign vht_memory[26 ] = `UU_HW_RATE_18M;
	assign vht_memory[27 ] = 40'd10572227190;
	assign vht_memory[28 ] = 40'd5090331610;
	assign vht_memory[29 ] = 40'd2349383820;
	assign vht_memory[30 ] = 40'd1174691910;
	assign vht_memory[31 ] = 40'd9560970676;
	assign vht_memory[32 ] = 40'd4581298449;
	assign vht_memory[33 ] = 40'd2114445438;
	assign vht_memory[34 ] = 40'd1057222719;
	assign vht_memory[35 ] = `UU_HW_RATE_24M;
	assign vht_memory[36 ] = 40'd7048151460;
	assign vht_memory[37 ] = 40'd3393554406;
	assign vht_memory[38 ] = 40'd1566255880;
	assign vht_memory[39 ] = 40'd783127940;
	assign vht_memory[40 ] = 40'd6355558542;
	assign vht_memory[41 ] = 40'd3054198966;
	assign vht_memory[42 ] = 40'd1409630292;
	assign vht_memory[43 ] = 40'd704815146;
	assign vht_memory[44 ] = `UU_HW_RATE_36M;
	assign vht_memory[45 ] = 40'd5286113595;
	assign vht_memory[46 ] = 40'd2545165805;
	assign vht_memory[47 ] = 40'd1174691910;
	assign vht_memory[48 ] = 40'd587345955;
	assign vht_memory[49 ] = 40'd4759790596;
	assign vht_memory[50 ] = 40'd2290649224;
	assign vht_memory[51 ] = 40'd1057222719;
	assign vht_memory[52 ] = 40'd528611359;
	assign vht_memory[53 ] = `UU_HW_RATE_48M;
	assign vht_memory[54 ] = 40'd4698767640;
	assign vht_memory[55 ] = 40'd2262369604;
	assign vht_memory[56 ] = 40'd1044170586;
	assign vht_memory[57 ] = 40'd522085293;
	assign vht_memory[58 ] = 40'd4228890876;
	assign vht_memory[59 ] = 40'd2036132644;
	assign vht_memory[60 ] = 40'd939753528;
	assign vht_memory[61 ] = 40'd469876764;
	assign vht_memory[62 ] = `UU_HW_RATE_54M;
	assign vht_memory[63 ] = 40'd4228890876;
	assign vht_memory[64 ] = 40'd2036132644;
	assign vht_memory[65 ] = 40'd939753528;
	assign vht_memory[66 ] = 40'd469876764;
	assign vht_memory[67 ] = 40'd3817748707;
	assign vht_memory[68 ] = 40'd1832519379;
	assign vht_memory[69 ] = 40'd845778175;
	assign vht_memory[70 ] = 40'd422889087;
	assign vht_memory[71 ] = `UU_HW_RATE_54M;
	assign vht_memory[72 ] = 40'd3524075730;
	assign vht_memory[73 ] = 40'd1696777203;
	assign vht_memory[74 ] = 40'd783127940;
	assign vht_memory[75 ] = 40'd391563970;
	assign vht_memory[76 ] = 40'd3177779271;
	assign vht_memory[77 ] = 40'd1527099483;
	assign vht_memory[78 ] = 40'd704815146;
	assign vht_memory[79 ] = 40'd352407573;
	assign vht_memory[80 ] = `UU_HW_RATE_54M;
	assign vht_memory[81 ] = 40'd4294967295;
	assign vht_memory[82 ] = 40'd1527099483;
	assign vht_memory[83 ] = 40'd704815146;
	assign vht_memory[84 ] = 40'd352407573;
	assign vht_memory[85 ] = 40'd4294967295;
	assign vht_memory[86 ] = 40'd1374389534;
	assign vht_memory[87 ] = 40'd634455642;
	assign vht_memory[88 ] = 40'd317227821;
	assign vht_memory[89 ] = `UU_HW_RATE_54M;
	assign vht_memory[90 ] = 40'd21144454380;
	assign vht_memory[91 ] = 40'd10180663220;
	assign vht_memory[92 ] = 40'd4698767640;
	assign vht_memory[93 ] = 40'd2349383820;
	assign vht_memory[94 ] = 40'd19289677680;
	assign vht_memory[95 ] = 40'd9162596898;
	assign vht_memory[96 ] = 40'd4228890876;
	assign vht_memory[97 ] = 40'd2114445438;
	assign vht_memory[98 ] = `UU_HW_RATE_6M;
	assign vht_memory[99 ] = 40'd10572227190;
	assign vht_memory[100] = 40'd5090331610;
	assign vht_memory[101] = 40'd2349383820;
	assign vht_memory[102] = 40'd1174691910;
	assign vht_memory[103] = 40'd9560970676;
	assign vht_memory[104] = 40'd4581298449;
	assign vht_memory[105] = 40'd2114445438;
	assign vht_memory[106] = 40'd1057222719;
	assign vht_memory[107] = `UU_HW_RATE_12M;
	assign vht_memory[108] = 40'd7048151460;
	assign vht_memory[109] = 40'd3393554406;
	assign vht_memory[110] = 40'd1566255880;
	assign vht_memory[111] = 40'd783127940;
	assign vht_memory[112] = 40'd6355558542;
	assign vht_memory[113] = 40'd3054198966;
	assign vht_memory[114] = 40'd1409630292;
	assign vht_memory[115] = 40'd704815146;
	assign vht_memory[116] = `UU_HW_RATE_18M;
	assign vht_memory[117] = 40'd5286113595;
	assign vht_memory[118] = 40'd2545165805;
	assign vht_memory[119] = 40'd1174691910;
	assign vht_memory[120] = 40'd587345955;
	assign vht_memory[121] = 40'd4759790596;
	assign vht_memory[122] = 40'd2290649224;
	assign vht_memory[123] = 40'd1057222719;
	assign vht_memory[124] = 40'd528611359;
	assign vht_memory[125] = `UU_HW_RATE_24M;
	assign vht_memory[126] = 40'd3524075730;
	assign vht_memory[127] = 40'd1696777203;
	assign vht_memory[128] = 40'd783127940;
	assign vht_memory[129] = 40'd391563970;
	assign vht_memory[130] = 40'd3177779271;
	assign vht_memory[131] = 40'd1527099483;
	assign vht_memory[132] = 40'd704815146;
	assign vht_memory[133] = 40'd352407573;
	assign vht_memory[134] = `UU_HW_RATE_36M;
	assign vht_memory[135] = 40'd2643056797;
	assign vht_memory[136] = 40'd1272582902;
	assign vht_memory[137] = 40'd587345955;
	assign vht_memory[138] = 40'd293672977;
	assign vht_memory[139] = 40'd2379895298;
	assign vht_memory[140] = 40'd1145324612;
	assign vht_memory[141] = 40'd528611359;
	assign vht_memory[142] = 40'd264305679;
	assign vht_memory[143] = `UU_HW_RATE_48M;
	assign vht_memory[144] = 40'd2349383820;
	assign vht_memory[145] = 40'd1131184802;
	assign vht_memory[146] = 40'd522085293;
	assign vht_memory[147] = 40'd261042646;
	assign vht_memory[148] = 40'd2114445438;
	assign vht_memory[149] = 40'd1018066322;
	assign vht_memory[150] = 40'd469876764;
	assign vht_memory[151] = 40'd234938382;
	assign vht_memory[152] = `UU_HW_RATE_54M;
	assign vht_memory[153] = 40'd2114445438;
	assign vht_memory[154] = 40'd1018066322;
	assign vht_memory[155] = 40'd469876764;
	assign vht_memory[156] = 40'd234938382;
	assign vht_memory[157] = 40'd1905566079;
	assign vht_memory[158] = 40'd916259689;
	assign vht_memory[159] = 40'd422889087;
	assign vht_memory[160] = 40'd211444543;
	assign vht_memory[161] = `UU_HW_RATE_54M;
	assign vht_memory[162] = 40'd1762037865;
	assign vht_memory[163] = 40'd848388601;
	assign vht_memory[164] = 40'd391563970;
	assign vht_memory[165] = 40'd195781985;
	assign vht_memory[166] = 40'd1586596865;
	assign vht_memory[167] = 40'd763549741;
	assign vht_memory[168] = 40'd352407573;
	assign vht_memory[169] = 40'd176203786;
	assign vht_memory[170] = `UU_HW_RATE_54M;
	assign vht_memory[171] = 40'd4294967295;
	assign vht_memory[172] = 40'd763549741;
	assign vht_memory[173] = 40'd352407573;
	assign vht_memory[174] = 40'd176203786;
	assign vht_memory[175] = 40'd4294967295;
	assign vht_memory[176] = 40'd687194767;
	assign vht_memory[177] = 40'd317227821;
	assign vht_memory[178] = 40'd158591032;
	assign vht_memory[179] = `UU_HW_RATE_54M;
	assign vht_memory[180] = 40'd14096302920;
	assign vht_memory[181] = 40'd6787108813;
	assign vht_memory[182] = 40'd3132511760;
	assign vht_memory[183] = 40'd1566255880;
	assign vht_memory[184] = 40'd12785018927;
	assign vht_memory[185] = 40'd6108397932;
	assign vht_memory[186] = 40'd2819260584;
	assign vht_memory[187] = 40'd1409630292;
	assign vht_memory[188] = `UU_HW_RATE_6M;
	assign vht_memory[189] = 40'd7048151460;
	assign vht_memory[190] = 40'd3393554406;
	assign vht_memory[191] = 40'd1566255880;
	assign vht_memory[192] = 40'd783127940;
	assign vht_memory[193] = 40'd6355558542;
	assign vht_memory[194] = 40'd3054198966;
	assign vht_memory[195] = 40'd1409630292;
	assign vht_memory[196] = 40'd704815146;
	assign vht_memory[197] = `UU_HW_RATE_12M;
	assign vht_memory[198] = 40'd4698767640;
	assign vht_memory[199] = 40'd2262369604;
	assign vht_memory[200] = 40'd1044170586;
	assign vht_memory[201] = 40'd522085293;
	assign vht_memory[202] = 40'd4228890876;
	assign vht_memory[203] = 40'd2036132644;
	assign vht_memory[204] = 40'd939753528;
	assign vht_memory[205] = 40'd469876764;
	assign vht_memory[206] = `UU_HW_RATE_18M;
	assign vht_memory[207] = 40'd3524075730;
	assign vht_memory[208] = 40'd1696777203;
	assign vht_memory[209] = 40'd783127940;
	assign vht_memory[210] = 40'd391563970;
	assign vht_memory[211] = 40'd3177779271;
	assign vht_memory[212] = 40'd1527099483;
	assign vht_memory[213] = 40'd704815146;
	assign vht_memory[214] = 40'd352407573;
	assign vht_memory[215] = `UU_HW_RATE_24M;
	assign vht_memory[216] = 40'd2349383820;
	assign vht_memory[217] = 40'd1131184802;
	assign vht_memory[218] = 40'd522085293;
	assign vht_memory[219] = 40'd261042646;
	assign vht_memory[220] = 40'd2114445438;
	assign vht_memory[221] = 40'd1018066322;
	assign vht_memory[222] = 40'd469876764;
	assign vht_memory[223] = 40'd234938382;
	assign vht_memory[224] = `UU_HW_RATE_36M;
	assign vht_memory[225] = 40'd1762037865;
	assign vht_memory[226] = 40'd848388601;
	assign vht_memory[227] = 40'd391563970;
	assign vht_memory[228] = 40'd195781985;
	assign vht_memory[229] = 40'd1586596865;
	assign vht_memory[230] = 40'd763549741;
	assign vht_memory[231] = 40'd352407573;
	assign vht_memory[232] = 40'd176203786;
	assign vht_memory[233] = `UU_HW_RATE_48M;
	assign vht_memory[234] = 40'd1566255880;
	assign vht_memory[235] = 40'd754123201;
	assign vht_memory[236] = 40'd4294967295;
	assign vht_memory[237] = 40'd174028431;
	assign vht_memory[238] = 40'd1409630292;
	assign vht_memory[239] = 40'd678710881;
	assign vht_memory[240] = 40'd4294967295;
	assign vht_memory[241] = 40'd156625588;
	assign vht_memory[242] = `UU_HW_RATE_54M;
	assign vht_memory[243] = 40'd1409630292;
	assign vht_memory[244] = 40'd678710881;
	assign vht_memory[245] = 40'd313251176;
	assign vht_memory[246] = 40'd156625588;
	assign vht_memory[247] = 40'd1269643911;
	assign vht_memory[248] = 40'd610839793;
	assign vht_memory[249] = 40'd281926058;
	assign vht_memory[250] = 40'd140963029;
	assign vht_memory[251] = `UU_HW_RATE_54M;
	assign vht_memory[252] = 40'd1174691910;
	assign vht_memory[253] = 40'd565592401;
	assign vht_memory[254] = 40'd261042646;
	assign vht_memory[255] = 40'd130521323;
	assign vht_memory[256] = 40'd1057222719;
	assign vht_memory[257] = 40'd509033161;
	assign vht_memory[258] = 40'd234938382;
	assign vht_memory[259] = 40'd117469191;
	assign vht_memory[260] = `UU_HW_RATE_54M;
	assign vht_memory[261] = 40'd1057222719;
	assign vht_memory[262] = 40'd509033161;
	assign vht_memory[263] = 40'd234938382;
	assign vht_memory[264] = 40'd4294967295;
	assign vht_memory[265] = 40'd951958119;
	assign vht_memory[266] = 40'd458129844;
	assign vht_memory[267] = 40'd211444543;
	assign vht_memory[268] = 40'd4294967295;
	assign vht_memory[269] = `UU_HW_RATE_54M;
	assign vht_memory[270] = 40'd10572227190;
	assign vht_memory[271] = 40'd5090331610;
	assign vht_memory[272] = 40'd2349383820;
	assign vht_memory[273] = 40'd1174691910;
	assign vht_memory[274] = 40'd9560970676;
	assign vht_memory[275] = 40'd4581298449;
	assign vht_memory[276] = 40'd2114445438;
	assign vht_memory[277] = 40'd10572227190;
	assign vht_memory[278] = `UU_HW_RATE_6M;
	assign vht_memory[279] = 40'd5286113595;
	assign vht_memory[280] = 40'd2545165805;
	assign vht_memory[281] = 40'd1174691910;
	assign vht_memory[282] = 40'd587345955;
	assign vht_memory[283] = 40'd4759790596;
	assign vht_memory[284] = 40'd2290649224;
	assign vht_memory[285] = 40'd1057222719;
	assign vht_memory[286] = 40'd528611359;
	assign vht_memory[287] = `UU_HW_RATE_12M;
	assign vht_memory[288] = 40'd3524075730;
	assign vht_memory[289] = 40'd1696777203;
	assign vht_memory[290] = 40'd783127940;
	assign vht_memory[291] = 40'd391563970;
	assign vht_memory[292] = 40'd3177779271;
	assign vht_memory[293] = 40'd1527099483;
	assign vht_memory[294] = 40'd704815146;
	assign vht_memory[295] = 40'd352407573;
	assign vht_memory[296] = `UU_HW_RATE_18M;
	assign vht_memory[297] = 40'd2643056797;
	assign vht_memory[298] = 40'd1272582902;
	assign vht_memory[299] = 40'd587345955;
	assign vht_memory[300] = 40'd293672977;
	assign vht_memory[301] = 40'd2379895298;
	assign vht_memory[302] = 40'd1145324612;
	assign vht_memory[303] = 40'd528611359;
	assign vht_memory[304] = 40'd264305679;
	assign vht_memory[305] = `UU_HW_RATE_24M;
	assign vht_memory[306] = 40'd1762037865;
	assign vht_memory[307] = 40'd848388601;
	assign vht_memory[308] = 40'd391563970;
	assign vht_memory[309] = 40'd195781985;
	assign vht_memory[310] = 40'd1586596865;
	assign vht_memory[311] = 40'd763549741;
	assign vht_memory[312] = 40'd352407573;
	assign vht_memory[313] = 40'd176203786;
	assign vht_memory[314] = `UU_HW_RATE_36M;
	assign vht_memory[315] = 40'd1321528398;
	assign vht_memory[316] = 40'd636291451;
	assign vht_memory[317] = 40'd293672977;
	assign vht_memory[318] = 40'd146836488;
	assign vht_memory[319] = 40'd1189947649;
	assign vht_memory[320] = 40'd572662306;
	assign vht_memory[321] = 40'd264305679;
	assign vht_memory[322] = 40'd132152839;
	assign vht_memory[323] = `UU_HW_RATE_48M;
	assign vht_memory[324] = 40'd1174691910;
	assign vht_memory[325] = 40'd565592401;
	assign vht_memory[326] = 40'd261042646;
	assign vht_memory[327] = 40'd130521323;
	assign vht_memory[328] = 40'd1057222719;
	assign vht_memory[329] = 40'd509033161;
	assign vht_memory[330] = 40'd234938382;
	assign vht_memory[331] = 40'd117469191;
	assign vht_memory[332] = `UU_HW_RATE_54M;
	assign vht_memory[333] = 40'd1057222719;
	assign vht_memory[334] = 40'd509033161;
	assign vht_memory[335] = 40'd234938382;
	assign vht_memory[336] = 40'd117469191;
	assign vht_memory[337] = 40'd951958119;
	assign vht_memory[338] = 40'd458129844;
	assign vht_memory[339] = 40'd211444543;
	assign vht_memory[340] = 40'd105722271;
	assign vht_memory[341] = `UU_HW_RATE_54M;
	assign vht_memory[342] = 40'd881018932;
	assign vht_memory[343] = 40'd424194300;
	assign vht_memory[344] = 40'd195781985;
	assign vht_memory[345] = 40'd97890992;
	assign vht_memory[346] = 40'd793298432;
	assign vht_memory[347] = 40'd381774870;
	assign vht_memory[348] = 40'd176203786;
	assign vht_memory[349] = 40'd88101893;
	assign vht_memory[350] = `UU_HW_RATE_54M;
	assign vht_memory[351] = 40'd4294967295;
	assign vht_memory[352] = 40'd381774870;
	assign vht_memory[353] = 40'd176203786;
	assign vht_memory[354] = 40'd88101893;
	assign vht_memory[355] = 40'd4294967295;
	assign vht_memory[356] = 40'd343597383;
	assign vht_memory[357] = 40'd158591032;
	assign vht_memory[358] = 40'd79295516;
	assign vht_memory[359] = `UU_HW_RATE_54M;
	assign vht_memory[360] = 40'd8457781752;
	assign vht_memory[361] = 40'd4072265288;
	assign vht_memory[362] = 40'd1879507056;
	assign vht_memory[363] = 40'd939753528;
	assign vht_memory[364] = 40'd7635497415;
	assign vht_memory[365] = 40'd3665038759;
	assign vht_memory[366] = 40'd1691556350;
	assign vht_memory[367] = 40'd845778175;
	assign vht_memory[368] = `UU_HW_RATE_6M;
	assign vht_memory[369] = 40'd4228890876;
	assign vht_memory[370] = 40'd2036132644;
	assign vht_memory[371] = 40'd939753528;
	assign vht_memory[372] = 40'd469876764;
	assign vht_memory[373] = 40'd3817748707;
	assign vht_memory[374] = 40'd1832519379;
	assign vht_memory[375] = 40'd845778175;
	assign vht_memory[376] = 40'd422889087;
	assign vht_memory[377] = `UU_HW_RATE_12M;
	assign vht_memory[378] = 40'd2819260584;
	assign vht_memory[379] = 40'd1357421762;
	assign vht_memory[380] = 40'd626502352;
	assign vht_memory[381] = 40'd313251176;
	assign vht_memory[382] = 40'd2539287823;
	assign vht_memory[383] = 40'd1221679586;
	assign vht_memory[384] = 40'd563852116;
	assign vht_memory[385] = 40'd281926058;
	assign vht_memory[386] = `UU_HW_RATE_18M;
	assign vht_memory[387] = 40'd2114445438;
	assign vht_memory[388] = 40'd1018066322;
	assign vht_memory[389] = 40'd469876764;
	assign vht_memory[390] = 40'd234938382;
	assign vht_memory[391] = 40'd1905566079;
	assign vht_memory[392] = 40'd916259689;
	assign vht_memory[393] = 40'd422889087;
	assign vht_memory[394] = 40'd211444543;
	assign vht_memory[395] = `UU_HW_RATE_24M;
	assign vht_memory[396] = 40'd1409630292;
	assign vht_memory[397] = 40'd678710881;
	assign vht_memory[398] = 40'd313251176;
	assign vht_memory[399] = 40'd156625588;
	assign vht_memory[400] = 40'd1269643911;
	assign vht_memory[401] = 40'd610839793;
	assign vht_memory[402] = 40'd281926058;
	assign vht_memory[403] = 40'd140963029;
	assign vht_memory[404] = `UU_HW_RATE_36M;
	assign vht_memory[405] = 40'd1057222719;
	assign vht_memory[406] = 40'd509033161;
	assign vht_memory[407] = 40'd234938382;
	assign vht_memory[408] = 40'd117469191;
	assign vht_memory[409] = 40'd951958119;
	assign vht_memory[410] = 40'd458129844;
	assign vht_memory[411] = 40'd211444543;
	assign vht_memory[412] = 40'd105722271;
	assign vht_memory[413] = `UU_HW_RATE_48M;
	assign vht_memory[414] = 40'd939753528;
	assign vht_memory[415] = 40'd452473920;
	assign vht_memory[416] = 40'd208834117;
	assign vht_memory[417] = 40'd104417058;
	assign vht_memory[418] = 40'd845778175;
	assign vht_memory[419] = 40'd407226528;
	assign vht_memory[420] = 40'd187950705;
	assign vht_memory[421] = 40'd93975352;
	assign vht_memory[422] = `UU_HW_RATE_54M;
	assign vht_memory[423] = 40'd845778175;
	assign vht_memory[424] = 40'd407226528;
	assign vht_memory[425] = 40'd187950705;
	assign vht_memory[426] = 40'd93975352;
	assign vht_memory[427] = 40'd761434645;
	assign vht_memory[428] = 40'd366503875;
	assign vht_memory[429] = 40'd169155635;
	assign vht_memory[430] = 40'd84577817;
	assign vht_memory[431] = `UU_HW_RATE_54M;
	assign vht_memory[432] = 40'd704815146;
	assign vht_memory[433] = 40'd339355440;
	assign vht_memory[434] = 40'd156625588;
	assign vht_memory[435] = 40'd78312794;
	assign vht_memory[436] = 40'd634455642;
	assign vht_memory[437] = 40'd305419896;
	assign vht_memory[438] = 40'd140963029;
	assign vht_memory[439] = 40'd70481514;
	assign vht_memory[440] = `UU_HW_RATE_54M;
	assign vht_memory[441] = 40'd4294967295;
	assign vht_memory[442] = 40'd305419896;
	assign vht_memory[443] = 40'd140963029;
	assign vht_memory[444] = 40'd70481514;
	assign vht_memory[445] = 40'd4294967295;
	assign vht_memory[446] = 40'd274877906;
	assign vht_memory[447] = 40'd126876486;
	assign vht_memory[448] = 40'd63434583;
	assign vht_memory[449] = `UU_HW_RATE_54M;
	assign vht_memory[450] = 40'd7048151460;
	assign vht_memory[451] = 40'd3393554406;
	assign vht_memory[452] = 40'd1566255880;
	assign vht_memory[453] = 40'd783127940;
	assign vht_memory[454] = 40'd6355558542;
	assign vht_memory[455] = 40'd3054198966;
	assign vht_memory[456] = 40'd1409630292;
	assign vht_memory[457] = 40'd704815146;
	assign vht_memory[458] = `UU_HW_RATE_6M;
	assign vht_memory[459] = 40'd3524075730;
	assign vht_memory[460] = 40'd1696777203;
	assign vht_memory[461] = 40'd783127940;
	assign vht_memory[462] = 40'd391563970;
	assign vht_memory[463] = 40'd3177779271;
	assign vht_memory[464] = 40'd1527099483;
	assign vht_memory[465] = 40'd704815146;
	assign vht_memory[466] = 40'd352407573;
	assign vht_memory[467] = `UU_HW_RATE_12M;
	assign vht_memory[468] = 40'd2349383820;
	assign vht_memory[469] = 40'd1131184802;
	assign vht_memory[470] = 40'd522085293;
	assign vht_memory[471] = 40'd261042646;
	assign vht_memory[472] = 40'd2114445438;
	assign vht_memory[473] = 40'd1018066322;
	assign vht_memory[474] = 40'd469876764;
	assign vht_memory[475] = 40'd234938382;
	assign vht_memory[476] = `UU_HW_RATE_18M;
	assign vht_memory[477] = 40'd1762037865;
	assign vht_memory[478] = 40'd848388601;
	assign vht_memory[479] = 40'd391563970;
	assign vht_memory[480] = 40'd195781985;
	assign vht_memory[481] = 40'd1586596865;
	assign vht_memory[482] = 40'd763549741;
	assign vht_memory[483] = 40'd352407573;
	assign vht_memory[484] = 40'd176203786;
	assign vht_memory[485] = `UU_HW_RATE_24M;
	assign vht_memory[486] = 40'd1174691910;
	assign vht_memory[487] = 40'd565592401;
	assign vht_memory[488] = 40'd261042646;
	assign vht_memory[489] = 40'd130521323;
	assign vht_memory[490] = 40'd1057222719;
	assign vht_memory[491] = 40'd509033161;
	assign vht_memory[492] = 40'd234938382;
	assign vht_memory[493] = 40'd117469191;
	assign vht_memory[494] = `UU_HW_RATE_36M;
	assign vht_memory[495] = 40'd881018932;
	assign vht_memory[496] = 40'd424194300;
	assign vht_memory[497] = 40'd195781985;
	assign vht_memory[498] = 40'd97890992;
	assign vht_memory[499] = 40'd793298432;
	assign vht_memory[500] = 40'd381774870;
	assign vht_memory[501] = 40'd176203786;
	assign vht_memory[502] = 40'd88101893;
	assign vht_memory[503] = `UU_HW_RATE_48M;
	assign vht_memory[504] = 40'd783127940;
	assign vht_memory[505] = 40'd377061600;
	assign vht_memory[506] = 40'd174028431;
	assign vht_memory[507] = 40'd87014215;
	assign vht_memory[508] = 40'd704815146;
	assign vht_memory[509] = 40'd339355440;
	assign vht_memory[510] = 40'd156625588;
	assign vht_memory[511] = 40'd78312794;
	assign vht_memory[512] = `UU_HW_RATE_54M;
	assign vht_memory[513] = 40'd704815146;
	assign vht_memory[514] = 40'd339355440;
	assign vht_memory[515] = 40'd156625588;
	assign vht_memory[516] = 40'd78312794;
	assign vht_memory[517] = 40'd634455642;
	assign vht_memory[518] = 40'd305419896;
	assign vht_memory[519] = 40'd140963029;
	assign vht_memory[520] = 40'd70481514;
	assign vht_memory[521] = `UU_HW_RATE_54M;
	assign vht_memory[522] = 40'd587345955;
	assign vht_memory[523] = 40'd282796200;
	assign vht_memory[524] = 40'd130521323;
	assign vht_memory[525] = 40'd65260661;
	assign vht_memory[526] = 40'd528611359;
	assign vht_memory[527] = 40'd254516580;
	assign vht_memory[528] = 40'd117469191;
	assign vht_memory[529] = 40'd58734595;
	assign vht_memory[530] = `UU_HW_RATE_54M;
	assign vht_memory[531] = 40'd528611359;
	assign vht_memory[532] = 40'd254516580;
	assign vht_memory[533] = 40'd4294967295;
	assign vht_memory[534] = 40'd58734595;
	assign vht_memory[535] = 40'd475773097;
	assign vht_memory[536] = 40'd229064922;
	assign vht_memory[537] = 40'd4294967295;
	assign vht_memory[538] = 40'd52861135;
	assign vht_memory[539] = `UU_HW_RATE_54M;
	assign vht_memory[540] = 40'd6041272680;
	assign vht_memory[541] = 40'd2908760920;
	assign vht_memory[542] = 40'd1342505040;
	assign vht_memory[543] = 40'd671252520;
	assign vht_memory[544] = 40'd5443126870;
	assign vht_memory[545] = 40'd2617884828;
	assign vht_memory[546] = 40'd1208254536;
	assign vht_memory[547] = 40'd604127268;
	assign vht_memory[548] = `UU_HW_RATE_6M;
	assign vht_memory[549] = 40'd3020636340;
	assign vht_memory[550] = 40'd1454380460;
	assign vht_memory[551] = 40'd671252520;
	assign vht_memory[552] = 40'd335626260;
	assign vht_memory[553] = 40'd2721563435;
	assign vht_memory[554] = 40'd1308942414;
	assign vht_memory[555] = 40'd604127268;
	assign vht_memory[556] = 40'd302063634;
	assign vht_memory[557] = `UU_HW_RATE_12M;
	assign vht_memory[558] = 40'd2013757560;
	assign vht_memory[559] = 40'd969586973;
	assign vht_memory[560] = 40'd447501680;
	assign vht_memory[561] = 40'd223750840;
	assign vht_memory[562] = 40'd1814375623;
	assign vht_memory[563] = 40'd872628276;
	assign vht_memory[564] = 40'd402751512;
	assign vht_memory[565] = 40'd201375756;
	assign vht_memory[566] = `UU_HW_RATE_18M;
	assign vht_memory[567] = 40'd1510318170;
	assign vht_memory[568] = 40'd727190230;
	assign vht_memory[569] = 40'd335626260;
	assign vht_memory[570] = 40'd167813130;
	assign vht_memory[571] = 40'd1360781717;
	assign vht_memory[572] = 40'd654471207;
	assign vht_memory[573] = 40'd302063634;
	assign vht_memory[574] = 40'd151031817;
	assign vht_memory[575] = `UU_HW_RATE_24M;
	assign vht_memory[576] = 40'd1006878780;
	assign vht_memory[577] = 40'd484793486;
	assign vht_memory[578] = 40'd223750840;
	assign vht_memory[579] = 40'd111875420;
	assign vht_memory[580] = 40'd906439923;
	assign vht_memory[581] = 40'd436314138;
	assign vht_memory[582] = 40'd201375756;
	assign vht_memory[583] = 40'd100687878;
	assign vht_memory[584] = `UU_HW_RATE_36M;
	assign vht_memory[585] = 40'd755159085;
	assign vht_memory[586] = 40'd363595115;
	assign vht_memory[587] = 40'd167813130;
	assign vht_memory[588] = 40'd83906565;
	assign vht_memory[589] = 40'd679970085;
	assign vht_memory[590] = 40'd327235603;
	assign vht_memory[591] = 40'd151031817;
	assign vht_memory[592] = 40'd75515908;
	assign vht_memory[593] = `UU_HW_RATE_48M;
	assign vht_memory[594] = 40'd671252520;
	assign vht_memory[595] = 40'd323195657;
	assign vht_memory[596] = 40'd4294967295;
	assign vht_memory[597] = 40'd74583613;
	assign vht_memory[598] = 40'd604127268;
	assign vht_memory[599] = 40'd290876092;
	assign vht_memory[600] = 40'd4294967295;
	assign vht_memory[601] = 40'd67125252;
	assign vht_memory[602] = `UU_HW_RATE_54M;
	assign vht_memory[603] = 40'd604127268;
	assign vht_memory[604] = 40'd290876092;
	assign vht_memory[605] = 40'd134250504;
	assign vht_memory[606] = 40'd67125252;
	assign vht_memory[607] = 40'd543774296;
	assign vht_memory[608] = 40'd261788482;
	assign vht_memory[609] = 40'd120825453;
	assign vht_memory[610] = 40'd60412726;
	assign vht_memory[611] = `UU_HW_RATE_54M;
	assign vht_memory[612] = 40'd503439390;
	assign vht_memory[613] = 40'd242396743;
	assign vht_memory[614] = 40'd111875420;
	assign vht_memory[615] = 40'd55937710;
	assign vht_memory[616] = 40'd453219961;
	assign vht_memory[617] = 40'd218157069;
	assign vht_memory[618] = 40'd100687878;
	assign vht_memory[619] = 40'd50343939;
	assign vht_memory[620] = `UU_HW_RATE_54M;
	assign vht_memory[621] = 40'd4294967295;
	assign vht_memory[622] = 40'd218157069;
	assign vht_memory[623] = 40'd100687878;
	assign vht_memory[624] = 40'd50343939;
	assign vht_memory[625] = 40'd4294967295;
	assign vht_memory[626] = 40'd196341362;
	assign vht_memory[627] = 40'd90621579;
	assign vht_memory[628] = 40'd45310789;
	assign vht_memory[629] = `UU_HW_RATE_54M;
	assign vht_memory[630] = 40'd5286113595;
	assign vht_memory[631] = 40'd2545165805;
	assign vht_memory[632] = 40'd1174691910;
	assign vht_memory[633] = 40'd587345955;
	assign vht_memory[634] = 40'd4759790596;
	assign vht_memory[635] = 40'd2290649224;
	assign vht_memory[636] = 40'd1057222719;
	assign vht_memory[637] = 40'd528611359;
	assign vht_memory[638] = `UU_HW_RATE_6M;
	assign vht_memory[639] = 40'd2643056797;
	assign vht_memory[640] = 40'd1272582902;
	assign vht_memory[641] = 40'd587345955;
	assign vht_memory[642] = 40'd293672977;
	assign vht_memory[643] = 40'd2379895298;
	assign vht_memory[644] = 40'd1145324612;
	assign vht_memory[645] = 40'd528611359;
	assign vht_memory[646] = 40'd264305679;
	assign vht_memory[647] = `UU_HW_RATE_12M;
	assign vht_memory[648] = 40'd1762037865;
	assign vht_memory[649] = 40'd848388601;
	assign vht_memory[650] = 40'd391563970;
	assign vht_memory[651] = 40'd195781985;
	assign vht_memory[652] = 40'd1586596865;
	assign vht_memory[653] = 40'd763549741;
	assign vht_memory[654] = 40'd352407573;
	assign vht_memory[655] = 40'd176203786;
	assign vht_memory[656] = `UU_HW_RATE_18M;
	assign vht_memory[657] = 40'd1321528398;
	assign vht_memory[658] = 40'd636291451;
	assign vht_memory[659] = 40'd293672977;
	assign vht_memory[660] = 40'd146836488;
	assign vht_memory[661] = 40'd1189947649;
	assign vht_memory[662] = 40'd572662306;
	assign vht_memory[663] = 40'd264305679;
	assign vht_memory[664] = 40'd132152839;
	assign vht_memory[665] = `UU_HW_RATE_24M;
	assign vht_memory[666] = 40'd881018932;
	assign vht_memory[667] = 40'd424194300;
	assign vht_memory[668] = 40'd195781985;
	assign vht_memory[669] = 40'd97890992;
	assign vht_memory[670] = 40'd793298432;
	assign vht_memory[671] = 40'd381774870;
	assign vht_memory[672] = 40'd176203786;
	assign vht_memory[673] = 40'd88101893;
	assign vht_memory[674] = `UU_HW_RATE_36M;
	assign vht_memory[675] = 40'd660764199;
	assign vht_memory[676] = 40'd318145725;
	assign vht_memory[677] = 40'd146836488;
	assign vht_memory[678] = 40'd73418244;
	assign vht_memory[679] = 40'd594973824;
	assign vht_memory[680] = 40'd286331153;
	assign vht_memory[681] = 40'd132152839;
	assign vht_memory[682] = 40'd66076419;
	assign vht_memory[683] = `UU_HW_RATE_48M;
	assign vht_memory[684] = 40'd587345955;
	assign vht_memory[685] = 40'd282796200;
	assign vht_memory[686] = 40'd130521323;
	assign vht_memory[687] = 40'd65260661;
	assign vht_memory[688] = 40'd528611359;
	assign vht_memory[689] = 40'd254516580;
	assign vht_memory[690] = 40'd117469191;
	assign vht_memory[691] = 40'd58734595;
	assign vht_memory[692] = `UU_HW_RATE_54M;
	assign vht_memory[693] = 40'd528611359;
	assign vht_memory[694] = 40'd254516580;
	assign vht_memory[695] = 40'd117469191;
	assign vht_memory[696] = 40'd58734595;
	assign vht_memory[697] = 40'd475773097;
	assign vht_memory[698] = 40'd229064922;
	assign vht_memory[699] = 40'd105722271;
	assign vht_memory[700] = 40'd52861135;
	assign vht_memory[701] = `UU_HW_RATE_54M;
	assign vht_memory[702] = 40'd440509466;
	assign vht_memory[703] = 40'd212097150;
	assign vht_memory[704] = 40'd97890992;
	assign vht_memory[705] = 40'd48945496;
	assign vht_memory[706] = 40'd396506176;
	assign vht_memory[707] = 40'd190887435;
	assign vht_memory[708] = 40'd88101893;
	assign vht_memory[709] = 40'd44050946;
	assign vht_memory[710] = `UU_HW_RATE_54M;
	assign vht_memory[711] = 40'd4294967295;
	assign vht_memory[712] = 40'd190887435;
	assign vht_memory[713] = 40'd88101893;
	assign vht_memory[714] = 40'd44050946;
	assign vht_memory[715] = 40'd4294967295;
	assign vht_memory[716] = 40'd171798691;
	assign vht_memory[717] = 40'd79295516;
	assign vht_memory[718] = 40'd39646328;
	assign vht_memory[719] = `UU_HW_RATE_54M;


	assign ht_memory[0  ] = 40'd42288908760;
	assign ht_memory[1  ] = 40'd20361326440;
	assign ht_memory[2  ] = 40'd39268272420;
	assign ht_memory[3  ] = 40'd18325193796;
	assign ht_memory[4  ] = `UU_HW_RATE_6M;
	assign ht_memory[5  ] = 40'd21144454380;
	assign ht_memory[6  ] = 40'd10180663220;
	assign ht_memory[7  ] = 40'd19289677680;
	assign ht_memory[8  ] = 40'd9162596898;
	assign ht_memory[9  ] = `UU_HW_RATE_12M;
	assign ht_memory[10 ] = 40'd14096302920;
	assign ht_memory[11 ] = 40'd6787108813;
	assign ht_memory[12 ] = 40'd12785018927;
	assign ht_memory[13 ] = 40'd6108397932;
	assign ht_memory[14 ] = `UU_HW_RATE_18M;
	assign ht_memory[15 ] = 40'd10572227190;
	assign ht_memory[16 ] = 40'd5090331610;
	assign ht_memory[17 ] = 40'd9560970676;
	assign ht_memory[18 ] = 40'd4581298449;
	assign ht_memory[19 ] = `UU_HW_RATE_24M;
	assign ht_memory[20 ] = 40'd7048151460;
	assign ht_memory[21 ] = 40'd3020636340;
	assign ht_memory[22 ] = 40'd6355558542;
	assign ht_memory[23 ] = 40'd3054198966;
	assign ht_memory[24 ] = `UU_HW_RATE_36M;
	assign ht_memory[25 ] = 40'd5286113595;
	assign ht_memory[26 ] = 40'd2545165805;
	assign ht_memory[27 ] = 40'd4759790596;
	assign ht_memory[28 ] = 40'd2290649224;
	assign ht_memory[29 ] = `UU_HW_RATE_48M;
	assign ht_memory[30 ] = 40'd4698767640;
	assign ht_memory[31 ] = 40'd2262369604;
	assign ht_memory[32 ] = 40'd4228890876;
	assign ht_memory[33 ] = 40'd2036132644;
	assign ht_memory[34 ] = `UU_HW_RATE_54M;
	assign ht_memory[35 ] = 40'd4228890876;
	assign ht_memory[36 ] = 40'd2036132644;
	assign ht_memory[37 ] = 40'd3817748707;
	assign ht_memory[38 ] = 40'd1832519379;
	assign ht_memory[39 ] = `UU_HW_RATE_54M;
	assign ht_memory[40 ] = 40'd21144454380;
	assign ht_memory[41 ] = 40'd10180663220;
	assign ht_memory[42 ] = 40'd19289677680;
	assign ht_memory[43 ] = 40'd9162596898;
	assign ht_memory[44 ] = `UU_HW_RATE_6M;
	assign ht_memory[45 ] = 40'd10572227190;
	assign ht_memory[46 ] = 40'd5090331610;
	assign ht_memory[47 ] = 40'd9560970676;
	assign ht_memory[48 ] = 40'd4581298449;
	assign ht_memory[49 ] = `UU_HW_RATE_12M;
	assign ht_memory[50 ] = 40'd7048151460;
	assign ht_memory[51 ] = 40'd3393554406;
	assign ht_memory[52 ] = 40'd6355558542;
	assign ht_memory[53 ] = 40'd3054198966;
	assign ht_memory[54 ] = `UU_HW_RATE_18M;
	assign ht_memory[55 ] = 40'd5286113595;
	assign ht_memory[56 ] = 40'd2545165805;
	assign ht_memory[57 ] = 40'd4759790596;
	assign ht_memory[58 ] = 40'd2290649224;
	assign ht_memory[59 ] = `UU_HW_RATE_24M;
	assign ht_memory[60 ] = 40'd3524075730;
	assign ht_memory[61 ] = 40'd1696777203;
	assign ht_memory[62 ] = 40'd3177779271;
	assign ht_memory[63 ] = 40'd1527099483;
	assign ht_memory[64 ] = `UU_HW_RATE_36M;
	assign ht_memory[65 ] = 40'd2643056797;
	assign ht_memory[66 ] = 40'd1272582902;
	assign ht_memory[67 ] = 40'd2379895298;
	assign ht_memory[68 ] = 40'd1145324612;
	assign ht_memory[69 ] = `UU_HW_RATE_48M;
	assign ht_memory[70 ] = 40'd2349383820;
	assign ht_memory[71 ] = 40'd1131184802;
	assign ht_memory[72 ] = 40'd2110387001;
	assign ht_memory[73 ] = 40'd1018066322;
	assign ht_memory[74 ] = `UU_HW_RATE_54M;
	assign ht_memory[75 ] = 40'd2114445438;
	assign ht_memory[76 ] = 40'd1018066322;
	assign ht_memory[77 ] = 40'd1905566079;
	assign ht_memory[78 ] = 40'd916259689;
	assign ht_memory[79 ] = `UU_HW_RATE_54M;
	assign ht_memory[80 ] = 40'd14096302920;
	assign ht_memory[81 ] = 40'd6787108813;
	assign ht_memory[82 ] = 40'd12785018927;
	assign ht_memory[83 ] = 40'd6108397932;
	assign ht_memory[84 ] = `UU_HW_RATE_6M;
	assign ht_memory[85 ] = 40'd7048151460;
	assign ht_memory[86 ] = 40'd3393554406;
	assign ht_memory[87 ] = 40'd6355558542;
	assign ht_memory[88 ] = 40'd3054198966;
	assign ht_memory[89 ] = `UU_HW_RATE_12M;
	assign ht_memory[90 ] = 40'd4698767640;
	assign ht_memory[91 ] = 40'd2262369604;
	assign ht_memory[92 ] = 40'd4228890876;
	assign ht_memory[93 ] = 40'd2036132644;
	assign ht_memory[94 ] = `UU_HW_RATE_18M;
	assign ht_memory[95 ] = 40'd3524075730;
	assign ht_memory[96 ] = 40'd1696777203;
	assign ht_memory[97 ] = 40'd3177779271;
	assign ht_memory[98 ] = 40'd1527099483;
	assign ht_memory[99 ] = `UU_HW_RATE_24M;
	assign ht_memory[100] = 40'd2349383820;
	assign ht_memory[101] = 40'd1131184802;
	assign ht_memory[102] = 40'd2114445438;
	assign ht_memory[103] = 40'd1018066322;
	assign ht_memory[104] = `UU_HW_RATE_36M;
	assign ht_memory[105] = 40'd1762037865;
	assign ht_memory[106] = 40'd848388601;
	assign ht_memory[107] = 40'd1586596865;
	assign ht_memory[108] = 40'd763549741;
	assign ht_memory[109] = `UU_HW_RATE_48M;
	assign ht_memory[110] = 40'd1566255880;
	assign ht_memory[111] = 40'd754123201;
	assign ht_memory[112] = 40'd1409630292;
	assign ht_memory[113] = 40'd678710881;
	assign ht_memory[114] = `UU_HW_RATE_54M;
	assign ht_memory[115] = 40'd1409630292;
	assign ht_memory[116] = 40'd678710881;
	assign ht_memory[117] = 40'd1269643911;
	assign ht_memory[118] = 40'd610839793;
	assign ht_memory[119] = `UU_HW_RATE_54M;
	assign ht_memory[120] = 40'd10572227190;
	assign ht_memory[121] = 40'd5090331610;
	assign ht_memory[122] = 40'd9560970676;
	assign ht_memory[123] = 40'd4581298449;
	assign ht_memory[124] = `UU_HW_RATE_6M;
	assign ht_memory[125] = 40'd5286113595;
	assign ht_memory[126] = 40'd2545165805;
	assign ht_memory[127] = 40'd4759790596;
	assign ht_memory[128] = 40'd2290649224;
	assign ht_memory[129] = `UU_HW_RATE_12M;
	assign ht_memory[130] = 40'd3524075730;
	assign ht_memory[131] = 40'd1696777203;
	assign ht_memory[132] = 40'd3177779271;
	assign ht_memory[133] = 40'd1527099483;
	assign ht_memory[134] = `UU_HW_RATE_18M;
	assign ht_memory[135] = 40'd2643056797;
	assign ht_memory[136] = 40'd1272582902;
	assign ht_memory[137] = 40'd2379895298;
	assign ht_memory[138] = 40'd1145324612;
	assign ht_memory[139] = `UU_HW_RATE_24M;
	assign ht_memory[140] = 40'd1762037865;
	assign ht_memory[141] = 40'd848388601;
	assign ht_memory[142] = 40'd1586596865;
	assign ht_memory[143] = 40'd763549741;
	assign ht_memory[144] = `UU_HW_RATE_36M;
	assign ht_memory[145] = 40'd1321528398;
	assign ht_memory[146] = 40'd636291451;
	assign ht_memory[147] = 40'd1189947649;
	assign ht_memory[148] = 40'd572662306;
	assign ht_memory[149] = `UU_HW_RATE_48M;
	assign ht_memory[150] = 40'd1174691910;
	assign ht_memory[151] = 40'd565592401;
	assign ht_memory[152] = 40'd1057222719;
	assign ht_memory[153] = 40'd509033161;
	assign ht_memory[154] = `UU_HW_RATE_54M;
	assign ht_memory[155] = 40'd1057222719;
	assign ht_memory[156] = 40'd509033161;
	assign ht_memory[157] = 40'd951958119;
	assign ht_memory[158] = 40'd458129844;
	assign ht_memory[159] = `UU_HW_RATE_54M;

	assign lrate_memory[0 ] = 40'd274877906943 ;
	assign lrate_memory[1 ] = `UU_HW_RATE_1M ;
	assign lrate_memory[2 ] = 40'd137438953471 ;
	assign lrate_memory[3 ] = `UU_HW_RATE_2M ;
	assign lrate_memory[4 ] = 40'd49977801262  ;
	assign lrate_memory[5 ] = `UU_HW_RATE_5M5;
	assign lrate_memory[6 ] = 40'd24988900631  ;
	assign lrate_memory[7 ] = `UU_HW_RATE_11M;
	assign lrate_memory[8 ] = 40'd45812984490  ;
	assign lrate_memory[9 ] = `UU_HW_RATE_6M ;
	assign lrate_memory[10] = 40'd30541989660  ;
	assign lrate_memory[11] = `UU_HW_RATE_9M ;
	assign lrate_memory[12] = 40'd22906492245  ;
	assign lrate_memory[13] = `UU_HW_RATE_12M;
	assign lrate_memory[14] = 40'd15270994830  ;
	assign lrate_memory[15] = `UU_HW_RATE_18M;
	assign lrate_memory[16] = 40'd11453246122  ;
	assign lrate_memory[17] = `UU_HW_RATE_24M;
	assign lrate_memory[18] = 40'd7635497415   ;
	assign lrate_memory[19] = `UU_HW_RATE_36M;
	assign lrate_memory[20] = 40'd5726623061   ;
	assign lrate_memory[21] = `UU_HW_RATE_48M;
	assign lrate_memory[22] = 40'd5090331610   ;
	assign lrate_memory[23] = `UU_HW_RATE_54M;

	assign vht_mem_data_out = tx_out_vht_mcs_en ? vht_memory[tx_out_vht_mcs_addr] : 0; 
	assign ht_mem_data_out = tx_out_ht_mcs_en ? ht_memory[tx_out_ht_mcs_addr] : 0; 
	assign lrate_mem_data_out = tx_out_lrate_en ? lrate_memory[tx_out_lrate_addr] : 0; 

/////
//uu_acmac_tx_category1_test U_TEST1();
//uu_acmac_tx_category2_test U_TEST2();
uu_acmac_tx_category3_test U_TEST3();
//uu_acmac_tx_category4_test U_TEST4();
//uu_acmac_tx_category5_test U_TEST5();
////

	always #2.5 clk=~clk;

	initial 
	begin
		clk                      = 0;
		rst_n                    = 0;
		tx_en                    = 0;
		tx_in_cap_data_req       = 0;
		tx_in_cap_start_req      = 0;
                tx_in_cap_send_ack       = 0;
		tx_in_cap_more_data_req  = 0;
		tx_in_cap_fb_update      = 0;
		tx_in_cp_upd_mpdu_status = 0;
		tx_in_cp_get_ampdu_status= 0;
		tx_in_cap_ch_bw          = 0;
                tx_in_clear_mpdu         = 0;
                tx_in_clear_ampdu        = 0;
		tx_in_cap_ac             = 0;
	        tx_in_cap_retry          = 0;
		tx_in_cap_ac0_tx_nav     = 0;
		tx_in_cap_ac1_tx_nav     = 0;
		tx_in_cap_ac2_tx_nav     = 0;
		tx_in_cap_ac3_tx_nav     = 0;
		tx_in_BSSBasicRateSet    = 0;
	        tx_in_cap_ac0_src        = 0;    
	        tx_in_cap_ac1_src        = 0;    
	        tx_in_cap_ac2_src        = 0;    
	        tx_in_cap_ac3_src        = 0;    
	        tx_in_cap_ac0_lrc        = 0;    
	        tx_in_cap_ac1_lrc        = 0;    
	        tx_in_cap_ac2_lrc        = 0;    
	        tx_in_cap_ac3_lrc        = 0;    
		tx_in_SIFS_timer_value   = 0;
		tx_in_TXOP_limit_ac0     = 1;
		tx_in_TXOP_limit_ac1     = 1;
		tx_in_TXOP_limit_ac2     = 1;
		tx_in_TXOP_limit_ac3     = 1;
		tx_in_cp_ctrl_fr_len     = 0;
		tx_in_QOS_mode           = 0;
		tx_in_RTS_Threshold      = 0;
                tx_in_Long_Retry_Count   = 0;
		tx_in_cp_res             = 0;
		tx_in_ctrl_data          = 0;
		tx_in_vht_mcs_data       = 0;
		tx_in_ht_mcs_data        = 0;
		tx_in_lrate_data         = 0;
		tx_in_cp_seqno           = 0;
		tx_in_sta_offset         = 0;
		tx_in_cp_ac              = 0;
                tx_in_phy_data_req       = 0;
		mac_addr[0]              = 0;
		mac_addr[1]              = 0;
		mac_addr[2]              = 0;
		mac_addr[3]              = 0;
		mac_addr[4]              = 0;
		mac_addr[5]              = 0;
		mac_addr[6]              = 0;
		mac_addr[7]              = 0;
                tx_buf_in_dma_ac         = 0;
	        tx_buf_in_dma_wen        = 0;
	        tx_buf_in_dma_data       = 0;
	        mem_ctrl_in_en           = 0; 
	        mem_ctrl_in_wen          = 0;
	        mem_ctrl_in_addr         = 0;
	        mem_ctrl_in_data         = 0;
		/*for(int i=1; i<=137; i++) begin 
			if(i>=10)
				ba_memory[i] = i;
			else
				ba_memory[i] = 0;
		end*/
		#10;
		rst_n=0;
		#50;
		rst_n=1;
	end

//	initial begin
//		$shm_open(`DUMPFILE);
//		$shm_probe("ASM");
//	end
	
endmodule


