------------------------------------------------------------------------------
-- lmac_reg_03_sep.vhd - entity/architecture pair
------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

library proc_common_v3_00_a;
use proc_common_v3_00_a.proc_common_pkg.all;
use proc_common_v3_00_a.ipif_pkg.all;

library axi_slave_burst_v1_00_a;
use axi_slave_burst_v1_00_a.axi_slave_burst;

------------------------------------------------------------------------------
-- Entity section
------------------------------------------------------------------------------
-- Definition of Generics:
--   C_S_AXI_DATA_WIDTH           -- AXI4 slave: Data Width
--   C_S_AXI_ADDR_WIDTH           -- AXI4 slave: Address Width
--   C_S_AXI_ID_WIDTH             -- AXI4 slave: ID Width
--   C_RDATA_FIFO_DEPTH           -- AXI4 slave: FIFO Depth
--   C_INCLUDE_TIMEOUT_CNT        -- AXI4 slave: Data Timeout Count
--   C_TIMEOUT_CNTR_VAL           -- AXI4 slave: Timeout Counter Value
--   C_ALIGN_BE_RDADDR            -- AXI4 slave: Align Byte Enable read Data Address
--   C_S_AXI_SUPPORTS_WRITE       -- AXI4 slave: Support Write
--   C_S_AXI_SUPPORTS_READ        -- AXI4 slave: Support Read
--   C_FAMILY                     -- FPGA Family
--   C_S_AXI_MEM0_BASEADDR        -- User memory space 0 base address
--   C_S_AXI_MEM0_HIGHADDR        -- User memory space 0 high address
--   C_S_AXI_MEM1_BASEADDR        -- User memory space 1 base address
--   C_S_AXI_MEM1_HIGHADDR        -- User memory space 1 high address
--   C_S_AXI_MEM2_BASEADDR        -- User memory space 2 base address
--   C_S_AXI_MEM2_HIGHADDR        -- User memory space 2 high address
--   C_S_AXI_MEM3_BASEADDR        -- User memory space 3 base address
--   C_S_AXI_MEM3_HIGHADDR        -- User memory space 3 high address
--   C_S_AXI_MEM4_BASEADDR        -- User memory space 4 base address
--   C_S_AXI_MEM4_HIGHADDR        -- User memory space 4 high address
--
-- Definition of Ports:
--   S_AXI_ACLK                   -- AXI4 slave: Clock
--   S_AXI_ARESETN                -- AXI4 slave: Reset
--   S_AXI_AWADDR                 -- AXI4 slave: Write address
--   S_AXI_AWVALID                -- AXI4 slave: Write address valid
--   S_AXI_WDATA                  -- AXI4 slave: Write data
--   S_AXI_WSTRB                  -- AXI4 slave: Write strobe
--   S_AXI_WVALID                 -- AXI4 slave: Write data valid
--   S_AXI_BREADY                 -- AXI4 slave: read response ready
--   S_AXI_ARADDR                 -- AXI4 slave: read address
--   S_AXI_ARVALID                -- AXI4 slave: read address valid
--   S_AXI_RREADY                 -- AXI4 slave: read data ready
--   S_AXI_ARREADY                -- AXI4 slave: read address ready
--   S_AXI_RDATA                  -- AXI4 slave: read data
--   S_AXI_RRESP                  -- AXI4 slave: read data response
--   S_AXI_RVALID                 -- AXI4 slave: read data valid
--   S_AXI_WREADY                 -- AXI4 slave: write data ready
--   S_AXI_BRESP                  -- AXI4 slave: read response
--   S_AXI_BVALID                 -- AXI4 slave: read response valid
--   S_AXI_AWREADY                -- AXI4 slave: write address ready
--   S_AXI_AWID                   -- AXI4 slave: write address ID
--   S_AXI_AWLEN                  -- AXI4 slave: write address Length
--   S_AXI_AWSIZE                 -- AXI4 slave: write address size
--   S_AXI_AWBURST                -- AXI4 slave: write address burst
--   S_AXI_AWLOCK                 -- AXI4 slave: write address lock
--   S_AXI_AWCACHE                -- AXI4 slave: write address cache
--   S_AXI_AWPROT                 -- AXI4 slave: write address protection
--   S_AXI_WLAST                  -- AXI4 slave: write data last
--   S_AXI_BID                    -- AXI4 slave: read response ID
--   S_AXI_ARID                   -- AXI4 slave: read address ID
--   S_AXI_ARLEN                  -- AXI4 slave: read address Length
--   S_AXI_ARSIZE                 -- AXI4 slave: read address size
--   S_AXI_ARBURST                -- AXI4 slave: read address burst
--   S_AXI_ARLOCK                 -- AXI4 slave: read address lock
--   S_AXI_ARCACHE                -- AXI4 slave: read address cache
--   S_AXI_ARPROT                 -- AXI4 slave: read address protection
--   S_AXI_RID                    -- AXI4 slave: read data ID
--   S_AXI_RLAST                  -- AXI4 slave: read data last
------------------------------------------------------------------------------

entity lmac_reg_top is
  generic
  (
    C_S_AXI_DATA_WIDTH             : integer              := 32;
    C_S_AXI_ADDR_WIDTH             : integer              := 32;
    C_S_AXI_ID_WIDTH               : integer              := 4;
    C_RDATA_FIFO_DEPTH             : integer              := 0;
    C_INCLUDE_TIMEOUT_CNT          : integer              := 1;
    C_TIMEOUT_CNTR_VAL             : integer              := 8;
    C_ALIGN_BE_RDADDR              : integer              := 0;
    C_S_AXI_SUPPORTS_WRITE         : integer              := 1;
    C_S_AXI_SUPPORTS_READ          : integer              := 1;
    C_FAMILY                       : string               := "virtex6";
    C_S_AXI_MEM0_BASEADDR          : std_logic_vector     := X"7AC00000";
    C_S_AXI_MEM0_HIGHADDR          : std_logic_vector     := X"7AC03FFF";
    C_S_AXI_MEM1_BASEADDR          : std_logic_vector     := X"7AC10000";
    C_S_AXI_MEM1_HIGHADDR          : std_logic_vector     := X"7AC13FFF";
    C_S_AXI_MEM2_BASEADDR          : std_logic_vector     := X"7AC20000";
    C_S_AXI_MEM2_HIGHADDR          : std_logic_vector     := X"7AC23FFF";
    C_S_AXI_MEM3_BASEADDR          : std_logic_vector     := X"7AC30000";
    C_S_AXI_MEM3_HIGHADDR          : std_logic_vector     := X"7AC33FFF";
    C_S_AXI_MEM4_BASEADDR          : std_logic_vector     := X"7AC40000";
    C_S_AXI_MEM4_HIGHADDR          : std_logic_vector     := X"7AC4FFFF"
  );
  port
  (
    S_AXI_ACLK                     : in  std_logic;
    S_AXI_ARESETN                  : in  std_logic;
    S_AXI_AWADDR                   : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
    S_AXI_AWVALID                  : in  std_logic;
    S_AXI_WDATA                    : in  std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    S_AXI_WSTRB                    : in  std_logic_vector((C_S_AXI_DATA_WIDTH/8)-1 downto 0);
    S_AXI_WVALID                   : in  std_logic;
    S_AXI_BREADY                   : in  std_logic;
    S_AXI_ARADDR                   : in  std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
    S_AXI_ARVALID                  : in  std_logic;
    S_AXI_RREADY                   : in  std_logic;
    S_AXI_ARREADY                  : out std_logic;
    S_AXI_RDATA                    : out std_logic_vector(C_S_AXI_DATA_WIDTH-1 downto 0);
    S_AXI_RRESP                    : out std_logic_vector(1 downto 0);
    S_AXI_RVALID                   : out std_logic;
    S_AXI_WREADY                   : out std_logic;
    S_AXI_BRESP                    : out std_logic_vector(1 downto 0);
    S_AXI_BVALID                   : out std_logic;
    S_AXI_AWREADY                  : out std_logic;
    S_AXI_AWID                     : in  std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0);
    S_AXI_AWLEN                    : in  std_logic_vector(7 downto 0);
    S_AXI_AWSIZE                   : in  std_logic_vector(2 downto 0);
    S_AXI_AWBURST                  : in  std_logic_vector(1 downto 0);
    S_AXI_AWLOCK                   : in  std_logic;
    S_AXI_AWCACHE                  : in  std_logic_vector(3 downto 0);
    S_AXI_AWPROT                   : in  std_logic_vector(2 downto 0);
    S_AXI_WLAST                    : in  std_logic;
    S_AXI_BID                      : out std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0);
    S_AXI_ARID                     : in  std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0);
    S_AXI_ARLEN                    : in  std_logic_vector(7 downto 0);
    S_AXI_ARSIZE                   : in  std_logic_vector(2 downto 0);
    S_AXI_ARBURST                  : in  std_logic_vector(1 downto 0);
    S_AXI_ARLOCK                   : in  std_logic;
    S_AXI_ARCACHE                  : in  std_logic_vector(3 downto 0);
    S_AXI_ARPROT                   : in  std_logic_vector(2 downto 0);
    S_AXI_RID                      : out std_logic_vector(C_S_AXI_ID_WIDTH-1 downto 0);
    S_AXI_RLAST                    : out std_logic
  );

  attribute MAX_FANOUT : string;
  attribute SIGIS : string;
  attribute MAX_FANOUT of S_AXI_ACLK       : signal is "10000";
  attribute MAX_FANOUT of S_AXI_ARESETN       : signal is "10000";
  attribute SIGIS of S_AXI_ACLK       : signal is "Clk";
  attribute SIGIS of S_AXI_ARESETN       : signal is "Rst";
end entity lmac_reg_top;

------------------------------------------------------------------------------
-- Architecture section
------------------------------------------------------------------------------

architecture IMP of lmac_reg_top is

  constant USER_SLV_DWIDTH                : integer              := C_S_AXI_DATA_WIDTH;

  constant IPIF_SLV_DWIDTH                : integer              := C_S_AXI_DATA_WIDTH;

  constant ZERO_ADDR_PAD                  : std_logic_vector(0 to 31) := (others => '0');

  constant IPIF_ARD_ADDR_RANGE_ARRAY      : SLV64_ARRAY_TYPE     := 
    (
      ZERO_ADDR_PAD & C_S_AXI_MEM0_BASEADDR,-- user logic memory space 0 base address
      ZERO_ADDR_PAD & C_S_AXI_MEM0_HIGHADDR,-- user logic memory space 0 high address
      ZERO_ADDR_PAD & C_S_AXI_MEM1_BASEADDR,-- user logic memory space 1 base address
      ZERO_ADDR_PAD & C_S_AXI_MEM1_HIGHADDR,-- user logic memory space 1 high address
      ZERO_ADDR_PAD & C_S_AXI_MEM2_BASEADDR,-- user logic memory space 2 base address
      ZERO_ADDR_PAD & C_S_AXI_MEM2_HIGHADDR,-- user logic memory space 2 high address
      ZERO_ADDR_PAD & C_S_AXI_MEM3_BASEADDR,-- user logic memory space 3 base address
      ZERO_ADDR_PAD & C_S_AXI_MEM3_HIGHADDR,-- user logic memory space 3 high address
      ZERO_ADDR_PAD & C_S_AXI_MEM4_BASEADDR,-- user logic memory space 4 base address
      ZERO_ADDR_PAD & C_S_AXI_MEM4_HIGHADDR -- user logic memory space 4 high address
    );

  constant USER_NUM_MEM                   : integer              := 5;

  constant IPIF_ARD_NUM_CE_ARRAY          : INTEGER_ARRAY_TYPE   := 
    (
      0  => 1,                            -- number of ce for user logic memory space 0 (always 1 chip enable)
      1  => 1,                            -- number of ce for user logic memory space 1 (always 1 chip enable)
      2  => 1,                            -- number of ce for user logic memory space 2 (always 1 chip enable)
      3  => 1,                            -- number of ce for user logic memory space 3 (always 1 chip enable)
      4  => 1                             -- number of ce for user logic memory space 4 (always 1 chip enable)
    );

  ------------------------------------------
  -- Width of the slave address bus (32 only)
  ------------------------------------------
  constant USER_SLV_AWIDTH                : integer              := C_S_AXI_ADDR_WIDTH;

  ------------------------------------------
  -- Index for CS/CE
  ------------------------------------------
  constant USER_MEM0_CS_INDEX             : integer              := 0;

  constant USER_CS_INDEX                  : integer              := USER_MEM0_CS_INDEX;

  ------------------------------------------
  -- IP Interconnect (IPIC) signal declarations
  ------------------------------------------
  signal ipif_Bus2IP_Clk                : std_logic;
  signal ipif_Bus2IP_Resetn             : std_logic;
  signal ipif_Bus2IP_Addr               : std_logic_vector(C_S_AXI_ADDR_WIDTH-1 downto 0);
  signal ipif_Bus2IP_RNW                : std_logic;
  signal ipif_Bus2IP_BE                 : std_logic_vector(IPIF_SLV_DWIDTH/8-1 downto 0);
  signal ipif_Bus2IP_CS                 : std_logic_vector((IPIF_ARD_ADDR_RANGE_ARRAY'LENGTH)/2-1 downto 0);
  signal ipif_Bus2IP_RdCE               : std_logic_vector(calc_num_ce(IPIF_ARD_NUM_CE_ARRAY)-1 downto 0);
  signal ipif_Bus2IP_WrCE               : std_logic_vector(calc_num_ce(IPIF_ARD_NUM_CE_ARRAY)-1 downto 0);
  signal ipif_Bus2IP_Data               : std_logic_vector(IPIF_SLV_DWIDTH-1 downto 0);
  signal ipif_Bus2IP_Burst              : std_logic;
  signal ipif_Bus2IP_BurstLength        : std_logic_vector(7 downto 0);
  signal ipif_Bus2IP_WrReq              : std_logic;
  signal ipif_Bus2IP_RdReq              : std_logic;
  signal ipif_IP2Bus_AddrAck            : std_logic;
  signal ipif_IP2Bus_RdAck              : std_logic;
  signal ipif_IP2Bus_WrAck              : std_logic;
  signal ipif_IP2Bus_Error              : std_logic;
  signal ipif_IP2Bus_Data               : std_logic_vector(IPIF_SLV_DWIDTH-1 downto 0);
  signal ipif_Type_of_xfer              : std_logic;
  signal user_Bus2IP_BurstLength        : std_logic_vector(7 downto 0)   := (others => '0');
  signal user_IP2Bus_AddrAck            : std_logic;
  signal user_IP2Bus_Data               : std_logic_vector(USER_SLV_DWIDTH-1 downto 0);
  signal user_IP2Bus_RdAck              : std_logic;
  signal user_IP2Bus_WrAck              : std_logic;
  signal user_IP2Bus_Error              : std_logic;
  signal user_Type_of_xfer              : std_logic;

  ------------------------------------------
  -- Component declaration for verilog user logic
  ------------------------------------------
  component lmac_reg is
    generic
    (
      C_SLV_AWIDTH                   : integer              := 32;
      C_SLV_DWIDTH                   : integer              := 32;
      C_NUM_MEM                      : integer              := 5
    );
    port
    (
      AXI_AWADDR: in std_logic_vector (31 downto 0);
		AXI_AWVALID: in std_logic;
		AXI_WDATA: in std_logic_vector(31 downto 0);
		AXI_WVALID: in std_logic;
		AXI_RREADY: in std_logic;
		AXI_BREADY :in std_logic;
		AXI_ARADDR: in std_logic_vector(31 downto 0);
		AXI_ARVALID: in std_logic;
		AXI_WREADY: out std_logic;
		AXI_BRESP: out std_logic_vector(1 downto 0);
		AXI_BVALID: out std_logic;
		AXI_ARREADY: out std_logic;
		AXI_RDATA: out std_logic_vector(31 downto 0);
		AXI_RRESP: out std_logic_vector(1 downto 0);
		AXI_RVALID: out std_logic;
		AXI_AWREADY: out std_logic;

      Bus2IP_Clk                     : in  std_logic;
      Bus2IP_Resetn                  : in  std_logic;
      Bus2IP_Addr                    : in  std_logic_vector(C_SLV_AWIDTH-1 downto 0);
      Bus2IP_CS                      : in  std_logic_vector(C_NUM_MEM-1 downto 0);
      Bus2IP_RNW                     : in  std_logic;
      Bus2IP_Data                    : in  std_logic_vector(C_SLV_DWIDTH-1 downto 0);
      Bus2IP_BE                      : in  std_logic_vector(C_SLV_DWIDTH/8-1 downto 0);
      Bus2IP_RdCE                    : in  std_logic_vector(C_NUM_MEM-1 downto 0);
      Bus2IP_WrCE                    : in  std_logic_vector(C_NUM_MEM-1 downto 0);
      Bus2IP_Burst                   : in  std_logic;
      Bus2IP_BurstLength             : in  std_logic_vector(7 downto 0);
      Bus2IP_RdReq                   : in  std_logic;
      Bus2IP_WrReq                   : in  std_logic;
      IP2Bus_AddrAck                 : out std_logic;
      IP2Bus_Data                    : out std_logic_vector(C_SLV_DWIDTH-1 downto 0);
      IP2Bus_RdAck                   : out std_logic;
      IP2Bus_WrAck                   : out std_logic;
      IP2Bus_Error                   : out std_logic;
      Type_of_xfer                   : out std_logic
    );
  end component lmac_reg;

begin

  ------------------------------------------
  -- instantiate axi_slave_burst
  ------------------------------------------
  AXI_SLAVE_BURST_I : entity axi_slave_burst_v1_00_a.axi_slave_burst
    generic map
    (
      C_S_AXI_DATA_WIDTH             => IPIF_SLV_DWIDTH,
      C_S_AXI_ADDR_WIDTH             => C_S_AXI_ADDR_WIDTH,
      C_S_AXI_ID_WIDTH               => C_S_AXI_ID_WIDTH,
      C_RDATA_FIFO_DEPTH             => C_RDATA_FIFO_DEPTH,
      C_INCLUDE_TIMEOUT_CNT          => C_INCLUDE_TIMEOUT_CNT,
      C_TIMEOUT_CNTR_VAL             => C_TIMEOUT_CNTR_VAL,
      C_ALIGN_BE_RDADDR              => C_ALIGN_BE_RDADDR,
      C_S_AXI_SUPPORTS_WRITE         => C_S_AXI_SUPPORTS_WRITE,
      C_S_AXI_SUPPORTS_READ          => C_S_AXI_SUPPORTS_READ,
      C_ARD_ADDR_RANGE_ARRAY         => IPIF_ARD_ADDR_RANGE_ARRAY,
      C_ARD_NUM_CE_ARRAY             => IPIF_ARD_NUM_CE_ARRAY,
      C_FAMILY                       => C_FAMILY
    )
    port map
    (
      S_AXI_ACLK                     => S_AXI_ACLK,
      S_AXI_ARESETN                  => S_AXI_ARESETN,
      S_AXI_AWADDR                   => S_AXI_AWADDR,
      S_AXI_AWVALID                  => S_AXI_AWVALID,
      S_AXI_WDATA                    => S_AXI_WDATA,
      S_AXI_WSTRB                    => S_AXI_WSTRB,
      S_AXI_WVALID                   => S_AXI_WVALID,
      S_AXI_BREADY                   => S_AXI_BREADY,
      S_AXI_ARADDR                   => S_AXI_ARADDR,
      S_AXI_ARVALID                  => S_AXI_ARVALID,
      S_AXI_RREADY                   => S_AXI_RREADY,
      S_AXI_AWID                     => S_AXI_AWID,
      S_AXI_AWLEN                    => S_AXI_AWLEN,
      S_AXI_AWSIZE                   => S_AXI_AWSIZE,
      S_AXI_AWBURST                  => S_AXI_AWBURST,
      S_AXI_AWLOCK                   => S_AXI_AWLOCK,
      S_AXI_AWCACHE                  => S_AXI_AWCACHE,
      S_AXI_AWPROT                   => S_AXI_AWPROT,
      S_AXI_WLAST                    => S_AXI_WLAST,
      S_AXI_BID                      => S_AXI_BID,
      S_AXI_ARID                     => S_AXI_ARID,
      S_AXI_ARLEN                    => S_AXI_ARLEN,
      S_AXI_ARSIZE                   => S_AXI_ARSIZE,
      S_AXI_ARBURST                  => S_AXI_ARBURST,
      S_AXI_ARLOCK                   => S_AXI_ARLOCK,
      S_AXI_ARCACHE                  => S_AXI_ARCACHE,
      S_AXI_ARPROT                   => S_AXI_ARPROT,
      S_AXI_RID                      => S_AXI_RID,
      S_AXI_RLAST                    => S_AXI_RLAST,
      Bus2IP_Clk                     => ipif_Bus2IP_Clk,
      Bus2IP_Resetn                  => ipif_Bus2IP_Resetn,
      Bus2IP_Addr                    => ipif_Bus2IP_Addr,
      Bus2IP_RNW                     => ipif_Bus2IP_RNW,
      Bus2IP_BE                      => ipif_Bus2IP_BE,
      Bus2IP_CS                      => ipif_Bus2IP_CS,
      Bus2IP_RdCE                    => ipif_Bus2IP_RdCE,
      Bus2IP_WrCE                    => ipif_Bus2IP_WrCE,
      Bus2IP_Data                    => ipif_Bus2IP_Data,
      Bus2IP_Burst                   => ipif_Bus2IP_Burst,
      Bus2IP_BurstLength             => ipif_Bus2IP_BurstLength,
      Bus2IP_WrReq                   => ipif_Bus2IP_WrReq,
      Bus2IP_RdReq                   => ipif_Bus2IP_RdReq,
      IP2Bus_AddrAck                 => ipif_IP2Bus_AddrAck,
      IP2Bus_RdAck                   => ipif_IP2Bus_RdAck,
      IP2Bus_WrAck                   => ipif_IP2Bus_WrAck,
      IP2Bus_Error                   => ipif_IP2Bus_Error,
      IP2Bus_Data                    => ipif_IP2Bus_Data,
      Type_of_xfer                   => ipif_Type_of_xfer
    );

  ------------------------------------------
  -- instantiate User Logic
  ------------------------------------------
  USER_LOGIC_I : component lmac_reg
    generic map
    (
      C_SLV_AWIDTH                   => USER_SLV_AWIDTH,
      C_SLV_DWIDTH                   => USER_SLV_DWIDTH,
      C_NUM_MEM                      => USER_NUM_MEM
    )
    port map
    (
		AXI_AWADDR=>S_AXI_AWADDR,
		AXI_AWVALID=>S_AXI_AWVALID,
		AXI_WDATA=>S_AXI_WDATA,
		AXI_WVALID=>S_AXI_WVALID,
		AXI_RREADY=>S_AXI_RREADY,
		AXI_BREADY=>S_AXI_BREADY,
		AXI_ARADDR=>S_AXI_ARADDR,
		AXI_ARVALID=>S_AXI_ARVALID,
		AXI_WREADY=>S_AXI_WREADY,
		AXI_BRESP=>S_AXI_BRESP,
		AXI_BVALID=>S_AXI_BVALID,
		AXI_ARREADY=>S_AXI_ARREADY,
		AXI_RDATA=>S_AXI_RDATA,
		AXI_RRESP=>S_AXI_RRESP,
		AXI_RVALID=>S_AXI_RVALID,
		AXI_AWREADY=>S_AXI_AWREADY,

      Bus2IP_Clk                     => ipif_Bus2IP_Clk,
      Bus2IP_Resetn                  => ipif_Bus2IP_Resetn,
      Bus2IP_Addr                    => ipif_Bus2IP_Addr,
      Bus2IP_CS                      => ipif_Bus2IP_CS(USER_NUM_MEM-1 downto 0),
      Bus2IP_RNW                     => ipif_Bus2IP_RNW,
      Bus2IP_Data                    => ipif_Bus2IP_Data,
      Bus2IP_BE                      => ipif_Bus2IP_BE,
      Bus2IP_RdCE                    => ipif_Bus2IP_RdCE,
      Bus2IP_WrCE                    => ipif_Bus2IP_WrCE,
      Bus2IP_Burst                   => ipif_Bus2IP_Burst,
      Bus2IP_BurstLength             => user_Bus2IP_BurstLength,
      Bus2IP_RdReq                   => ipif_Bus2IP_RdReq,
      Bus2IP_WrReq                   => ipif_Bus2IP_WrReq,
      IP2Bus_AddrAck                 => user_IP2Bus_AddrAck,
      IP2Bus_Data                    => user_IP2Bus_Data,
      IP2Bus_RdAck                   => user_IP2Bus_RdAck,
      IP2Bus_WrAck                   => user_IP2Bus_WrAck,
      IP2Bus_Error                   => user_IP2Bus_Error,
      Type_of_xfer                   => user_Type_of_xfer
    );

  ------------------------------------------
  -- connect internal signals
  ------------------------------------------
  IP2BUS_DATA_MUX_PROC : process( ipif_Bus2IP_CS, user_IP2Bus_Data ) is
  begin

    case ipif_Bus2IP_CS is
      when "10000" => ipif_IP2Bus_Data <= user_IP2Bus_Data;
      when "01000" => ipif_IP2Bus_Data <= user_IP2Bus_Data;
      when "00100" => ipif_IP2Bus_Data <= user_IP2Bus_Data;
      when "00010" => ipif_IP2Bus_Data <= user_IP2Bus_Data;
      when "00001" => ipif_IP2Bus_Data <= user_IP2Bus_Data;
      when others => ipif_IP2Bus_Data <= (others => '0');
    end case;

  end process IP2BUS_DATA_MUX_PROC;

  ipif_IP2Bus_AddrAck <= user_IP2Bus_AddrAck;
  ipif_IP2Bus_WrAck <= user_IP2Bus_WrAck;
  ipif_IP2Bus_RdAck <= user_IP2Bus_RdAck;
  ipif_IP2Bus_Error <= user_IP2Bus_Error;

  user_Bus2IP_BurstLength(7 downto 0)<= ipif_Bus2IP_BurstLength(7 downto 0);

end IMP;
