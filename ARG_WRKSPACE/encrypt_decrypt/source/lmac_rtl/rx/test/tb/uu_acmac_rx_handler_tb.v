`timescale 1ns/1ns
`include "/home/uurmi/lmac_rtl/inc/defines.h"

module uu_acmac_rx_handler_tb();

parameter WIDTH_WORD = 32;
parameter WIDTH_HALF_WORD = 16;
parameter WIDTH_BYTE = 8;

reg                       clk;
reg                       rst_n;
reg                       rx_enable;
reg                       rx_in_ev_rxstart;
reg                       rx_in_ev_rxdata;
reg                       rx_in_ev_rxend;
reg                       rx_in_frame_valid;
reg [1:0]                 rx_in_rxend_status;
reg [WIDTH_BYTE-1:0]      rx_in_frame;
reg [WIDTH_BYTE+1:0]      rx_in_lmac_filter_flag;
reg [WIDTH_WORD-1:0]      rx_in_cp_return_value;
reg                       rx_in_cp_return_valid;


reg [31:0] value = 0;
reg [15:0] packet_length = 0;
reg [7:0] byte_1 = 0;
reg not_end_of_file = 0;
reg [15:0] sym_no = 0;

reg [2:0] ch_bandwidth = 0;
reg [1:0] format = 0;
reg [2:0] modulation = 0;
reg is_long_preamble = 0;
reg [1:0] reserved1 = 0;
reg is_L_sigvalid  = 0;
reg [3:0] L_datarate = 0;
reg [15:0] L_length = 0;
reg is_dyn_bw = 0;
reg [2:0] indicated_chan_bw = 0;
reg [7:0] rssi = 0;
reg [7:0] rcpi = 0;
reg is_smoothing = 0;
reg is_sounding = 0;
reg is_aggregated = 0;
reg is_short_GI = 0;
reg [1:0] stbc = 0;
reg [1:0] num_ext_ss = 0;
reg [6:0] mcs = 0;
reg  is_fec_ldpc_coding = 0;
reg [31:0] rx_start_of_frame_offset = 0;
reg [6:0] rec_mcs = 0;
reg reserved2 = 0;
reg [19:0] psdu_length = 0;
reg [2:0] num_sts = 0;
reg is_beamformed = 0;
reg [8:0] partial_aid = 0;
reg [5:0] group_id = 0;
reg is_tx_op_ps_allowed = 0;
reg [7:0] snr = 0;
reg [7:0] padding_byte = 0;
reg check_output = 0;
reg  [31:0] output_value_end;

wire                       rx_out_frame_decrypt_enable;
wire                       rx_out_frame_post_to_umac;
wire                       rx_out_rxend_ind;
wire                       rx_out_phyrx_error_ind;
wire                       rx_out_frame_valid;
wire [WIDTH_BYTE-1:0]      rx_out_frame;
wire [WIDTH_WORD-1:0]      rx_out_cap_return_value;
wire                       rx_out_rx_ampdu_frames_cnt;
wire                       rx_out_rx_ampdu_subframes_cnt;
wire                       rx_out_rx_phyerr_pkts_cnt;
wire                       rx_out_rx_frames_cnt;
wire                       rx_out_rx_multicast_cnt;
wire                       rx_out_rx_broadcast_cnt;
wire                       rx_out_rx_frame_forus_cnt;
wire                       rx_out_dot11_fcs_error_cnt;
wire                       rx_out_cp_process_rx_frame;
wire [11:0]                rx_out_mem_addr;
wire                       rx_out_mem_en;  


///////////////////////////////////////////////////

initial 
 begin
clk                      = 0;
rst_n                       = 0;
rx_enable                   = 0;
rx_in_ev_rxstart            = 0;
rx_in_ev_rxdata             = 0;
rx_in_ev_rxend              = 0;
rx_in_frame_valid           = 0;
rx_in_rxend_status          = 0;
rx_in_frame                 = 0;
rx_in_lmac_filter_flag      = 0;
rx_in_cp_return_value       = 0;
rx_in_cp_return_valid       = 0;
                            
 #10;
 rst_n=0;
 #50;
 rst_n=1;
end

//--------------DUT INSTANCE------------------//

uu_acmac_rx_handler U_WLAN_RX_HANDLER
                (
                 .rx_clk                       (clk),
                 .rst_n                        (rst_n),
                 .rx_enable                    (rx_enable),
                 .rx_in_ev_rxstart             (rx_in_ev_rxstart),
                 .rx_in_ev_rxdata              (rx_in_ev_rxdata),
                 .rx_in_ev_rxend               (rx_in_ev_rxend),
                 .rx_in_frame_valid            (rx_in_frame_valid),
                 .rx_in_rxend_status           (rx_in_rxend_status),
                 .rx_in_frame                  (rx_in_frame),
                 .rx_in_lmac_filter_flag       (rx_in_lmac_filter_flag),
                 .rx_in_cp_return_value        (rx_in_cp_return_value),
                 .rx_in_cp_return_valid        (rx_in_cp_return_valid),
                 .rx_out_cp_process_rx_frame   (rx_out_cp_process_rx_frame),
                 .rx_out_frame_decrypt_enable  (rx_out_frame_decrypt_enable),
                 .rx_out_frame_post_to_umac    (rx_out_frame_post_to_umac),
                 .rx_out_rxend_ind             (rx_out_rxend_ind),
                 .rx_out_phyrx_error_ind       (rx_out_phyrx_error_ind),
                 .rx_out_frame_valid           (rx_out_frame_valid),
                 .rx_out_frame                 (rx_out_frame),
                 .rx_out_mem_en                (rx_out_mem_en),
                 .rx_out_mem_addr              (rx_out_mem_addr),
                 .rx_out_cap_return_value      (rx_out_cap_return_value),
                 .rx_out_rx_ampdu_frames_cnt   (rx_out_rx_ampdu_frames_cnt),
                 .rx_out_rx_ampdu_subframes_cnt(rx_out_rx_ampdu_subframes_cnt),
                 .rx_out_rx_phyerr_pkts_cnt    (rx_out_rx_phyerr_pkts_cnt),
                 .rx_out_rx_frames_cnt         (rx_out_rx_frames_cnt),
                 .rx_out_rx_multicast_cnt      (rx_out_rx_multicast_cnt),
                 .rx_out_rx_broadcast_cnt      (rx_out_rx_broadcast_cnt),
                 .rx_out_rx_frame_forus_cnt    (rx_out_rx_frame_forus_cnt),
                 .rx_out_dot11_fcs_error_cnt   (rx_out_dot11_fcs_error_cnt)

                );

always #2.5 clk=~clk;

//initial begin
//$shm_open(`DUMPFILE);
//$shm_probe("ASM");
//end

uu_acmac_rx_handler_test U_TEST();

endmodule
