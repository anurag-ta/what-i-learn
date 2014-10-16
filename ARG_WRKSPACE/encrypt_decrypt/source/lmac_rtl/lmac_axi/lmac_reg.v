//----------------------------------------------------------------------------
// lmac_reg.v - module
//----------------------------------------------------------------------------
// Filename:          lmac_reg.v
// Version:           1.00.a

module lmac_reg
(
 		AXI_AWADDR,
		AXI_AWVALID,
		AXI_WDATA,
		AXI_WVALID,
		AXI_RREADY,
		AXI_BREADY,
		AXI_ARADDR,
		AXI_ARVALID,
		AXI_WREADY,
		AXI_BRESP,
		AXI_BVALID,
		AXI_ARREADY,
		AXI_RDATA,
		AXI_RRESP,
		AXI_RVALID,
		AXI_AWREADY,

  Bus2IP_Clk,                     // Bus to IP clock
  Bus2IP_Resetn,                  // Bus to IP reset
  Bus2IP_Addr,                    // Bus to IP address bus
  Bus2IP_CS,                      // Bus to IP chip select for user logic memory selection
  Bus2IP_RNW,                     // Bus to IP read/not write
  Bus2IP_Data,                    // Bus to IP data bus
  Bus2IP_BE,                      // Bus to IP byte enables
  Bus2IP_RdCE,                    // Bus to IP read chip enable
  Bus2IP_WrCE,                    // Bus to IP write chip enable
  Bus2IP_Burst,                   // Bus to IP burst-mode qualifier 
  Bus2IP_BurstLength,             // Bus to IP burst length
  Bus2IP_RdReq,                   // Bus to IP read request
  Bus2IP_WrReq,                   // Bus to IP write request
IP2Bus_AddrAck,              // IP to Bus address acknowledgement
  IP2Bus_Data,                    // IP to Bus data bus
  IP2Bus_RdAck,                   // IP to Bus read transfer acknowledgement
  IP2Bus_WrAck,                   // IP to Bus write transfer acknowledgement
  IP2Bus_Error,                   // IP to Bus error response
  Type_of_xfer                    // Transfer Type
); 

parameter C_SLV_AWIDTH                   = 32;
parameter C_SLV_DWIDTH                   = 32;
parameter C_NUM_MEM                      = 5;
 		input [31:0]AXI_AWADDR;
		input AXI_AWVALID;
		input [31:0]AXI_WDATA;
		input AXI_WVALID;
		input AXI_RREADY;
		input AXI_BREADY;
		input [31:0] AXI_ARADDR;
		input AXI_ARVALID;
		output AXI_WREADY;
		output [1:0] AXI_BRESP;
		output AXI_BVALID;
		output AXI_ARREADY;
		output reg [31:0]AXI_RDATA;
		output [1:0] AXI_RRESP;
		output reg AXI_RVALID;
		output AXI_AWREADY;

input                                     Bus2IP_Clk;
input                                     Bus2IP_Resetn;
input      [C_SLV_AWIDTH-1 : 0]           Bus2IP_Addr;
input      [C_NUM_MEM-1 : 0]              Bus2IP_CS;
input                                     Bus2IP_RNW;
input      [C_SLV_DWIDTH-1 : 0]           Bus2IP_Data;
input      [C_SLV_DWIDTH/8-1 : 0]         Bus2IP_BE;
input      [C_NUM_MEM-1 : 0]              Bus2IP_RdCE;
input      [C_NUM_MEM-1 : 0]              Bus2IP_WrCE;
input                                     Bus2IP_Burst;
input      [7 : 0]                        Bus2IP_BurstLength;
input                                     Bus2IP_RdReq;
input                                     Bus2IP_WrReq;
output                                    IP2Bus_AddrAck;
output     [C_SLV_DWIDTH-1 : 0]           IP2Bus_Data;
output                                    IP2Bus_RdAck;
output                                    IP2Bus_WrAck;
output                                    IP2Bus_Error;
output                                    Type_of_xfer;

//----------------------------------------------------------------------------
// Implementation
//----------------------------------------------------------------------------

parameter NUM_BYTE_LANES = (C_SLV_DWIDTH+7)/8;

wire rx_buff_read_enable;
reg rx_buff_read_ack_dly1;
reg rx_buff_read_ack_dly2; 
wire rx_buff_read_ack; 
reg [2:0]    w_nstate,w_pstate;
reg [474:0]   busy_reg;
//-----------------------------------lmac Signals--------------------------------------------//
wire [1:0]lmac_in_dma_ac;
wire [1:0] lmac_in_ac_select;
reg [31:0] lmac2IP_Data;
reg [31:0] UU_SLOT_TIMER_VALUE_R;
reg [31:0] UU_WLAN_SIFS_TIMER_VALUE_R;
reg [31:0]UU_WLAN_DIFS_VALUE_R;
reg [31:0] UU_PHY_RX_START_DELAY_R;
reg [31:0] UU_WLAN_ACK_TIMER_VALUE_R;
reg [31:0] UU_WLAN_CTS_TIMER_VALUE_R;
reg [31:0] UU_WLAN_DOT11_SHORT_RETRY_COUNT_R;
reg [31:0] UU_WLAN_DOT11_LONG_RETRY_COUNT_R;
reg [31:0] UU_WLAN_RTS_THRESHOLD_R;
reg [31:0] UU_REG_LMAC_BEACON_INRVL;
reg [31:0] UU_WLAN_CWMIN_VALUE_R;
reg [31:0] UU_WLAN_CWMAX_VALUE_R;
reg [31:0] UU_WLAN_CW_MIN_VALUE_R_BK;
reg [31:0] UU_WLAN_CW_MIN_VALUE_R_BE;
reg [31:0] UU_WLAN_CW_MIN_VALUE_R_VI;
reg [31:0] UU_WLAN_CW_MIN_VALUE_R_VO;
reg [31:0] UU_WLAN_CW_MAX_VALUE_R_BK;
reg [31:0] UU_WLAN_CW_MAX_VALUE_R_BE;
reg [31:0] UU_WLAN_CW_MAX_VALUE_R_VI;
reg [31:0] UU_WLAN_CW_MAX_VALUE_R_VO;
reg [31:0] UU_WLAN_TXOP_LIMIT_R_BK;
reg [31:0] UU_WLAN_TXOP_LIMIT_R_BE;
reg [31:0] UU_WLAN_TXOP_LIMIT_R_VI;
reg [31:0] UU_WLAN_TXOP_LIMIT_R_VO;
reg [31:0] UU_WLAN_AIFS_VALUE_R_BK;
reg [31:0] UU_WLAN_AIFS_VALUE_R_BE;
reg [31:0] UU_WLAN_AIFS_VALUE_R_VI;
reg [31:0] UU_WLAN_AIFS_VALUE_R_VO;
reg [31:0] UU_WLAN_SIGNAL_EXTENSION_R;
reg [63:0] UU_WLAN_IEEE80211_QOS_MODE_R;
reg [31:0] UU_WLAN_IEEE80211_OP_MODE_R;
reg [63:0] UU_WLAN_IEEE80211_STA_MAC_ADDR_R;
reg [31:0] UU_WLAN_SELF_CTS_R;
reg [31:0] UU_REG_DOT11_ACK_FAILURE_COUNT;
reg [31:0] UU_REG_DOT11_RTS_FAILURE_COUNT;
reg [31:0] UU_REG_DOT11_RTS_SUCCESS_COUNT;
wire [31:0] UU_REG_DOT11_FCS_ERROR_COUNT;
wire [31:0] UU_WLAN_RX_FRAMES;
wire [31:0] UU_WLAN_RX_MULTICAST_CNT;
wire [31:0] UU_WLAN_RX_BROADCAST_CNT;
wire [31:0] UU_WLAN_RX_FRAME_FOR_US;
wire [31:0] UU_WLAN_RX_AMPDU_FRAMES;
wire [31:0] UU_WLAN_RX_AMPDU_SUBFRAMES;
wire [31:0] UU_WLAN_RX_ERR_FRAMES;
reg [31:0] Rx_Status_Register;
reg [31:0]uu_wlan_tx_status_flags_g_BK;
reg [31:0]uu_wlan_tx_status_flags_g_BE;
reg [31:0]uu_wlan_tx_status_flags_g_VI;
reg [31:0]uu_wlan_tx_status_flags_g_VO;
reg [31:0]Configuration_Register_51;
reg [31:0]Configuration_Register_52;
reg [31:0]Configuration_Register_53;
reg [31:0]Configuration_Register_54;
reg [31:0]Configuration_Register_55;
reg [31:0]Configuration_Register_56;
reg [31:0]Configuration_Register_57;
reg [31:0]Configuration_Register_58;
reg [31:0]Configuration_Register_59;
reg [31:0]Configuration_Register_60;
reg [31:0]Configuration_Register_61;
reg [31:0]Configuration_Register_62;
reg [31:0]Configuration_Register_63;
reg [31:0]Configuration_Register_64;
reg [31:0]Configuration_Register_65;
reg [31:0]Configuration_Register_66;
reg [31:0]Configuration_Register_67;
reg [31:0]Configuration_Register_68;
reg [31:0]Configuration_Register_69;
reg [31:0]Configuration_Register_70;
reg [31:0]Configuration_Register_71;
reg [31:0]Configuration_Register_72;
reg [31:0]Configuration_Register_73;
reg [31:0]Configuration_Register_74;
reg [31:0]Configuration_Register_75;
reg [31:0]Configuration_Register_76;
reg [31:0]Configuration_Register_77;
reg [31:0]Configuration_Register_78;
reg [31:0]Configuration_Register_79;
reg [31:0]Configuration_Register_80;
reg [31:0]Configuration_Register_81;
reg [31:0]Configuration_Register_82;
reg [31:0]Configuration_Register_83;
reg [31:0]Configuration_Register_84;
reg [31:0]Configuration_Register_85;
reg [31:0]Configuration_Register_86;
reg [31:0]Configuration_Register_87;
reg [31:0]Configuration_Register_88;
reg [31:0]Configuration_Register_89;
reg [31:0]Configuration_Register_90;
reg [31:0]Configuration_Register_91;
reg [31:0]Configuration_Register_92;
reg [31:0]Configuration_Register_93;
reg [31:0]Configuration_Register_94;
reg [31:0]Configuration_Register_95;
reg [31:0]Configuration_Register_96;
reg [31:0]Configuration_Register_97;
reg [31:0]Configuration_Register_98;
reg [31:0]Configuration_Register_99;
reg [31:0]Configuration_Register_100;
reg [31:0]Configuration_Register_101;
reg [31:0]Configuration_Register_102;
reg [31:0]Configuration_Register_103;
reg [31:0]Configuration_Register_104;
reg [31:0]Configuration_Register_105;
reg [31:0]Configuration_Register_106;
reg [31:0]Configuration_Register_107;
reg [31:0]Configuration_Register_108;
reg [31:0]Configuration_Register_109;
reg [31:0]Configuration_Register_110;
reg [31:0]Configuration_Register_111;
reg [31:0]Configuration_Register_112;
reg [31:0]Configuration_Register_116;
reg [31:0]sta_is_used;
reg [63:0]ba_is_used;
reg [63:0]sta_mac_addr_r0;
reg [63:0]sta_mac_addr_r1;
reg [63:0]sta_mac_addr_r2;
reg [63:0]sta_mac_addr_r3;
reg [63:0]sta_mac_addr_r4;
reg [63:0]sta_mac_addr_r5;
reg [63:0]sta_mac_addr_r6;
reg [63:0]sta_mac_addr_r7;
reg [63:0]sta_mac_addr_r8;
reg [63:0]sta_mac_addr_r9;
reg [63:0]sta_mac_addr_r10;
reg [63:0]sta_mac_addr_r11;
reg [63:0]sta_mac_addr_r12;
reg [63:0]sta_mac_addr_r13;
reg [63:0]sta_mac_addr_r14;
reg [63:0]sta_mac_addr_r15;
reg [63:0]sta_mac_addr_r16;
reg [63:0]sta_mac_addr_r17;
reg [63:0]sta_mac_addr_r18;
reg [63:0]sta_mac_addr_r19;
reg [31:0] status_flag_ac0;
reg [31:0] status_flag_ac1;
reg [31:0] status_flag_ac2;
reg [31:0] status_flag_ac3;
reg [31:0] AC0_TX_STATUS_0;
reg [31:0] AC0_TX_STATUS_1;
reg [31:0] AC0_TX_STATUS_2;
reg [31:0] AC0_TX_STATUS_3;
reg [31:0] AC0_TX_STATUS_4;
reg [31:0] AC0_TX_STATUS_5;
reg [31:0] AC0_TX_STATUS_6;
reg [31:0] AC0_TX_STATUS_7;
reg [31:0] AC0_TX_STATUS_8;
reg [31:0] AC0_TX_STATUS_9;
reg [31:0] AC0_TX_STATUS_10;
reg [31:0] AC0_TX_STATUS_11;
reg [31:0] AC0_TX_STATUS_12;
reg [31:0] AC0_TX_STATUS_13;
reg [31:0] AC0_TX_STATUS_14;
reg [31:0] AC0_TX_STATUS_15;
reg [31:0] AC0_TX_STATUS_16;
reg [31:0] AC0_TX_STATUS_17;
reg [31:0] AC0_TX_STATUS_18;
reg [31:0] AC0_TX_STATUS_19;
reg [31:0] AC0_TX_STATUS_20;
reg [31:0] AC0_TX_STATUS_21;
reg [31:0] AC0_TX_STATUS_22;
reg [31:0] AC0_TX_STATUS_23;
reg [31:0] AC0_TX_STATUS_24;
reg [31:0] AC0_TX_STATUS_25;
reg [31:0] AC0_TX_STATUS_26;
reg [31:0] AC0_TX_STATUS_27;
reg [31:0] AC0_TX_STATUS_28;
reg [31:0] AC0_TX_STATUS_29;
reg [31:0] AC0_TX_STATUS_30;
reg [31:0] AC0_TX_STATUS_31;
reg [31:0] AC0_TX_STATUS_32;
reg [31:0] AC0_TX_STATUS_33;
reg [31:0] AC0_TX_STATUS_34;
reg [31:0] AC0_TX_STATUS_35;
reg [31:0] AC0_TX_STATUS_36;
reg [31:0] AC0_TX_STATUS_37;
reg [31:0] AC0_TX_STATUS_38;
reg [31:0] AC0_TX_STATUS_39;
reg [31:0] AC0_TX_STATUS_40;
reg [31:0] AC0_TX_STATUS_41;
reg [31:0] AC0_TX_STATUS_42;
reg [31:0] AC0_TX_STATUS_43;
reg [31:0] AC0_TX_STATUS_44;
reg [31:0] AC0_TX_STATUS_45;
reg [31:0] AC0_TX_STATUS_46;
reg [31:0] AC0_TX_STATUS_47;
reg [31:0] AC0_TX_STATUS_48;
reg [31:0] AC0_TX_STATUS_49;
reg [31:0] AC0_TX_STATUS_50;
reg [31:0] AC0_TX_STATUS_51;
reg [31:0] AC0_TX_STATUS_52;
reg [31:0] AC0_TX_STATUS_53;
reg [31:0] AC0_TX_STATUS_54;
reg [31:0] AC0_TX_STATUS_55;
reg [31:0] AC0_TX_STATUS_56;
reg [31:0] AC0_TX_STATUS_57;
reg [31:0] AC0_TX_STATUS_58;
reg [31:0] AC0_TX_STATUS_59;
reg [31:0] AC0_TX_STATUS_60;
reg [31:0] AC0_TX_STATUS_61;
reg [31:0] AC0_TX_STATUS_62;
reg [31:0] AC0_TX_STATUS_63;

reg [31:0] AC1_TX_STATUS_0;
reg [31:0] AC1_TX_STATUS_1;
reg [31:0] AC1_TX_STATUS_2;
reg [31:0] AC1_TX_STATUS_3;
reg [31:0] AC1_TX_STATUS_4;
reg [31:0] AC1_TX_STATUS_5;
reg [31:0] AC1_TX_STATUS_6;
reg [31:0] AC1_TX_STATUS_7;
reg [31:0] AC1_TX_STATUS_8;
reg [31:0] AC1_TX_STATUS_9;
reg [31:0] AC1_TX_STATUS_10;
reg [31:0] AC1_TX_STATUS_11;
reg [31:0] AC1_TX_STATUS_12;
reg [31:0] AC1_TX_STATUS_13;
reg [31:0] AC1_TX_STATUS_14;
reg [31:0] AC1_TX_STATUS_15;
reg [31:0] AC1_TX_STATUS_16;
reg [31:0] AC1_TX_STATUS_17;
reg [31:0] AC1_TX_STATUS_18;
reg [31:0] AC1_TX_STATUS_19;
reg [31:0] AC1_TX_STATUS_20;
reg [31:0] AC1_TX_STATUS_21;
reg [31:0] AC1_TX_STATUS_22;
reg [31:0] AC1_TX_STATUS_23;
reg [31:0] AC1_TX_STATUS_24;
reg [31:0] AC1_TX_STATUS_25;
reg [31:0] AC1_TX_STATUS_26;
reg [31:0] AC1_TX_STATUS_27;
reg [31:0] AC1_TX_STATUS_28;
reg [31:0] AC1_TX_STATUS_29;
reg [31:0] AC1_TX_STATUS_30;
reg [31:0] AC1_TX_STATUS_31;
reg [31:0] AC1_TX_STATUS_32;
reg [31:0] AC1_TX_STATUS_33;
reg [31:0] AC1_TX_STATUS_34;
reg [31:0] AC1_TX_STATUS_35;
reg [31:0] AC1_TX_STATUS_36;
reg [31:0] AC1_TX_STATUS_37;
reg [31:0] AC1_TX_STATUS_38;
reg [31:0] AC1_TX_STATUS_39;
reg [31:0] AC1_TX_STATUS_40;
reg [31:0] AC1_TX_STATUS_41;
reg [31:0] AC1_TX_STATUS_42;
reg [31:0] AC1_TX_STATUS_43;
reg [31:0] AC1_TX_STATUS_44;
reg [31:0] AC1_TX_STATUS_45;
reg [31:0] AC1_TX_STATUS_46;
reg [31:0] AC1_TX_STATUS_47;
reg [31:0] AC1_TX_STATUS_48;
reg [31:0] AC1_TX_STATUS_49;
reg [31:0] AC1_TX_STATUS_50;
reg [31:0] AC1_TX_STATUS_51;
reg [31:0] AC1_TX_STATUS_52;
reg [31:0] AC1_TX_STATUS_53;
reg [31:0] AC1_TX_STATUS_54;
reg [31:0] AC1_TX_STATUS_55;
reg [31:0] AC1_TX_STATUS_56;
reg [31:0] AC1_TX_STATUS_57;
reg [31:0] AC1_TX_STATUS_58;
reg [31:0] AC1_TX_STATUS_59;
reg [31:0] AC1_TX_STATUS_60;
reg [31:0] AC1_TX_STATUS_61;
reg [31:0] AC1_TX_STATUS_62;
reg [31:0] AC1_TX_STATUS_63;

reg [31:0] AC2_TX_STATUS_0;
reg [31:0] AC2_TX_STATUS_1;
reg [31:0] AC2_TX_STATUS_2;
reg [31:0] AC2_TX_STATUS_3;
reg [31:0] AC2_TX_STATUS_4;
reg [31:0] AC2_TX_STATUS_5;
reg [31:0] AC2_TX_STATUS_6;
reg [31:0] AC2_TX_STATUS_7;
reg [31:0] AC2_TX_STATUS_8;
reg [31:0] AC2_TX_STATUS_9;
reg [31:0] AC2_TX_STATUS_10;
reg [31:0] AC2_TX_STATUS_11;
reg [31:0] AC2_TX_STATUS_12;
reg [31:0] AC2_TX_STATUS_13;
reg [31:0] AC2_TX_STATUS_14;
reg [31:0] AC2_TX_STATUS_15;
reg [31:0] AC2_TX_STATUS_16;
reg [31:0] AC2_TX_STATUS_17;
reg [31:0] AC2_TX_STATUS_18;
reg [31:0] AC2_TX_STATUS_19;
reg [31:0] AC2_TX_STATUS_20;
reg [31:0] AC2_TX_STATUS_21;
reg [31:0] AC2_TX_STATUS_22;
reg [31:0] AC2_TX_STATUS_23;
reg [31:0] AC2_TX_STATUS_24;
reg [31:0] AC2_TX_STATUS_25;
reg [31:0] AC2_TX_STATUS_26;
reg [31:0] AC2_TX_STATUS_27;
reg [31:0] AC2_TX_STATUS_28;
reg [31:0] AC2_TX_STATUS_29;
reg [31:0] AC2_TX_STATUS_30;
reg [31:0] AC2_TX_STATUS_31;
reg [31:0] AC2_TX_STATUS_32;
reg [31:0] AC2_TX_STATUS_33;
reg [31:0] AC2_TX_STATUS_34;
reg [31:0] AC2_TX_STATUS_35;
reg [31:0] AC2_TX_STATUS_36;
reg [31:0] AC2_TX_STATUS_37;
reg [31:0] AC2_TX_STATUS_38;
reg [31:0] AC2_TX_STATUS_39;
reg [31:0] AC2_TX_STATUS_40;
reg [31:0] AC2_TX_STATUS_41;
reg [31:0] AC2_TX_STATUS_42;
reg [31:0] AC2_TX_STATUS_43;
reg [31:0] AC2_TX_STATUS_44;
reg [31:0] AC2_TX_STATUS_45;
reg [31:0] AC2_TX_STATUS_46;
reg [31:0] AC2_TX_STATUS_47;
reg [31:0] AC2_TX_STATUS_48;
reg [31:0] AC2_TX_STATUS_49;
reg [31:0] AC2_TX_STATUS_50;
reg [31:0] AC2_TX_STATUS_51;
reg [31:0] AC2_TX_STATUS_52;
reg [31:0] AC2_TX_STATUS_53;
reg [31:0] AC2_TX_STATUS_54;
reg [31:0] AC2_TX_STATUS_55;
reg [31:0] AC2_TX_STATUS_56;
reg [31:0] AC2_TX_STATUS_57;
reg [31:0] AC2_TX_STATUS_58;
reg [31:0] AC2_TX_STATUS_59;
reg [31:0] AC2_TX_STATUS_60;
reg [31:0] AC2_TX_STATUS_61;
reg [31:0] AC2_TX_STATUS_62;
reg [31:0] AC2_TX_STATUS_63;

reg [31:0] AC3_TX_STATUS_0;
reg [31:0] AC3_TX_STATUS_1;
reg [31:0] AC3_TX_STATUS_2;
reg [31:0] AC3_TX_STATUS_3;
reg [31:0] AC3_TX_STATUS_4;
reg [31:0] AC3_TX_STATUS_5;
reg [31:0] AC3_TX_STATUS_6;
reg [31:0] AC3_TX_STATUS_7;
reg [31:0] AC3_TX_STATUS_8;
reg [31:0] AC3_TX_STATUS_9;
reg [31:0] AC3_TX_STATUS_10;
reg [31:0] AC3_TX_STATUS_11;
reg [31:0] AC3_TX_STATUS_12;
reg [31:0] AC3_TX_STATUS_13;
reg [31:0] AC3_TX_STATUS_14;
reg [31:0] AC3_TX_STATUS_15;
reg [31:0] AC3_TX_STATUS_16;
reg [31:0] AC3_TX_STATUS_17;
reg [31:0] AC3_TX_STATUS_18;
reg [31:0] AC3_TX_STATUS_19;
reg [31:0] AC3_TX_STATUS_20;
reg [31:0] AC3_TX_STATUS_21;
reg [31:0] AC3_TX_STATUS_22;
reg [31:0] AC3_TX_STATUS_23;
reg [31:0] AC3_TX_STATUS_24;
reg [31:0] AC3_TX_STATUS_25;
reg [31:0] AC3_TX_STATUS_26;
reg [31:0] AC3_TX_STATUS_27;
reg [31:0] AC3_TX_STATUS_28;
reg [31:0] AC3_TX_STATUS_29;
reg [31:0] AC3_TX_STATUS_30;
reg [31:0] AC3_TX_STATUS_31;
reg [31:0] AC3_TX_STATUS_32;
reg [31:0] AC3_TX_STATUS_33;
reg [31:0] AC3_TX_STATUS_34;
reg [31:0] AC3_TX_STATUS_35;
reg [31:0] AC3_TX_STATUS_36;
reg [31:0] AC3_TX_STATUS_37;
reg [31:0] AC3_TX_STATUS_38;
reg [31:0] AC3_TX_STATUS_39;
reg [31:0] AC3_TX_STATUS_40;
reg [31:0] AC3_TX_STATUS_41;
reg [31:0] AC3_TX_STATUS_42;
reg [31:0] AC3_TX_STATUS_43;
reg [31:0] AC3_TX_STATUS_44;
reg [31:0] AC3_TX_STATUS_45;
reg [31:0] AC3_TX_STATUS_46;
reg [31:0] AC3_TX_STATUS_47;
reg [31:0] AC3_TX_STATUS_48;
reg [31:0] AC3_TX_STATUS_49;
reg [31:0] AC3_TX_STATUS_50;
reg [31:0] AC3_TX_STATUS_51;
reg [31:0] AC3_TX_STATUS_52;
reg [31:0] AC3_TX_STATUS_53;
reg [31:0] AC3_TX_STATUS_54;
reg [31:0] AC3_TX_STATUS_55;
reg [31:0] AC3_TX_STATUS_56;
reg [31:0] AC3_TX_STATUS_57;
reg [31:0] AC3_TX_STATUS_58;
reg [31:0] AC3_TX_STATUS_59;
reg [31:0] AC3_TX_STATUS_60;
reg [31:0] AC3_TX_STATUS_61;
reg [31:0] AC3_TX_STATUS_62;
reg [31:0] AC3_TX_STATUS_63;

reg [31:0]UU_LMAC_IND_GEN_BEACON;
reg [31:0] UU_LMAC_IND_RX_FRAME;
reg [31:0] UU_LMAC_IND_TX_STATUS;

wire lmac_in_dma_wen;
integer byte_index;
reg [4095:0]wready;
reg [31:0] config_reg [4095:0];
wire [4095:0]write_select;
wire  [4095:0]read_select;
wire [4096:0] reg_select;
wire in_r_data;
reg [2:0]    r_nstate,r_pstate;
reg [31:0] waddr;
reg [31:0] lmac_raddr;
reg [31:0] axi_raddr;
wire         wr_err;
wire         rd_err;
wire         in_w_idle;
wire         in_w_addr;
wire         in_w_resp;
wire         in_r_idle;
wire         in_r_addr;

wire [7:0] Rx_frame_in;

wire [31:0] RX_STATUS_REG1;
wire [31:0] RX_STATUS_REG2;
wire [31:0] RX_STATUS_REG3;
wire [31:0] RX_STATUS_REG4;
wire [31:0] RX_STATUS_REG5;
wire [31:0] RX_STATUS_REG6;
wire [31:0] RX_STATUS_REG7;
wire [31:0] RX_STATUS_REG8;
wire [31:0] RX_STATUS_REG9;
wire [31:0] RX_STATUS_REG10;
wire [31:0] RX_STATUS_REG11;
wire [31:0] RX_STATUS_REG12;
wire [31:0] RX_STATUS_REG13;
wire [31:0] RX_STATUS_REG14;
wire [31:0] RX_STATUS_REG15;
wire [31:0] RX_STATUS_REG16;

assign rx_buff_read_ack = (rx_buff_read_ack_dly1 && (!rx_buff_read_ack_dly2));

assign rx_buff_read_enable=(Bus2IP_Addr[15:0]>=16'h7FFF && Bus2IP_Addr[15:0]<=16'hFFFF)?Bus2IP_RdCE[4]:0;

assign IP2Bus_RdAck = rx_buff_read_ack;


always @( posedge Bus2IP_Clk)
  begin
    if(Bus2IP_Resetn == 0) 
    begin
      rx_buff_read_ack_dly1 <= 0;
      rx_buff_read_ack_dly2 <= 0;
    end
    else
    begin
      rx_buff_read_ack_dly1 <= rx_buff_read_enable;
      rx_buff_read_ack_dly2 <= rx_buff_read_ack_dly1;
    end
  end
  
  assign IP2Bus_RdAck=rx_buff_read_ack;
  
lmac_top lmac_top_module (
    .clk(Bus2IP_Clk), 
    .rst_n(Bus2IP_Resetn), 
    .lmac_en(lmac_en), 
    .cap_en(cap_en), 
    .cp_en(cp_en), 
    .rx_en(rx_en), 
    .tx_en(tx_en), 
    .sta_en(sta_en), 
    .lmac_in_umac_bea(Bus2IP_BE), 
    .lmac_in_umac_data(Bus2IP_Data), 
    .lmac_in_umac_addr(Bus2IP_Addr), 
    .lmac_in_umac_wrn_rd(Bus2IP_RNW),
    .lmac_in_umac_val(Bus2IP_WrReq), 
    .lmac_in_frame_valid(lmac_in_frame_valid), 
    .lmac_in_frame(lmac_in_frame), 
    .lmac_in_BSSBasicRateSet(lmac_in_BSSBasicRateSet), 
    .lmac_in_SIFS_timer_value(UU_WLAN_SIFS_TIMER_VALUE_R), 
    .lmac_in_rxend_status(lmac_in_rxend_status), 
    .lmac_in_filter_flag(lmac_in_filter_flag), 
    .lmac_in_ac0_txframeinfo_valid(lmac_in_ac0_txframeinfo_valid), 
    .lmac_in_ac1_txframeinfo_valid(lmac_in_ac1_txframei