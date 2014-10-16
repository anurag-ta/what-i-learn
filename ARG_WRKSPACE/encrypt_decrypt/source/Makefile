#UnComment the following line to build lmac with test driver
#BUILD_TYPE:= "lmac_test"

#Select the build platform
PLATFORM        = sim 
#PLATFORM       = zynq 

BUILDPLATFORM = $(firstword $(PLATFORM))

# PC Version
ifeq ($(BUILDPLATFORM),sim)
linux_dir:=/lib/modules/$(shell uname -r)/build
endif

# ZYNQ Version
ifeq ($(BUILDPLATFORM),zynq)
CROSS_COMPILER := arm-xilinx-linux-gnueabi-
CROSS_COMPILE := arm-xilinx-linux-gnueabi-
ARCH := arm

# Change this to correct path in the build PC
linux_dir = /home/uurmi/zedboard/ubuntu

endif

UU_DIR:= $(PWD)
CFLAGS_MODULE +=  -I$(UU_DIR)/umac/$(BUILDPLATFORM)
#CFLAGS_MODULE += -DLOG_DEBUG

ifeq ($(UU_DIR), )
   UU_DIR := /home/uurmi/uu_wlan_cvs_cap
endif

ifneq ($(BUILD_TYPE), )
   BUILD_TYPE:= "lmac_test"

# Build kernel module, to test LMAC independently (without UMAC).
# UMAC & PHY will be simulated.
obj-m += lmac_test.o umac_simulation.o 
lmac_test-y +=    \
	lmac/utils/src/crc32.o \
	lmac/utils/src/uu_wlan_buf.o \
	lmac/utils/src/uu_wlan_buf_desc.o \
	lmac/utils/src/uu_wlan_utils.o \
	lmac/utils/src/uu_wlan_rx_q.o \
	lmac/utils/src/uu_wlan_cap_q.o \
	lmac/utils/src/uu_wlan_ac_q.o \
	lmac/rx/src/uu_wlan_rx_handler.o \
	lmac/rx/src/uu_wlan_rx_status.o \
    lmac/cp/src/uu_wlan_cp_if.o \
	lmac/cp/src/uu_wlan_cp_ba_sb_rx.o \
	lmac/cp/src/uu_wlan_cp_ba_sb_tx.o \
	lmac/cp/src/uu_wlan_cp_ba_session.o \
	lmac/cp/src/uu_wlan_cp_rx_proc.o \
	lmac/cp/src/uu_wlan_cp_ctrl_pkt_gen.o \
    lmac/ps/src/uu_wlan_ps_uapsd.o \
	lmac/tx/src/uu_wlan_tx_handler.o \
	lmac/tx/src/uu_wlan_tx_aggregation.o \
    lmac/tx/src/uu_wlan_tx_status.o \
	lmac/tx/src/uu_wlan_beacon.o \
	lmac/src/uu_wlan_lmac_init.o \
	lmac/src/uu_wlan_reg.o \
	lmac/dur/src/uu_wlan_rate.o \
	lmac/dur/src/uu_wlan_lmac_tpc.o \
	lmac/dur/src/uu_wlan_duration.o \
	lmac/cap/src/uu_wlan_cap_init.o \
	lmac/cap/src/uu_wlan_cap_sm.o \
	lmac/cap/src/uu_wlan_cap_cm.o \
	lmac/cap/src/uu_wlan_cap_context.o \
	lmac/cap/src/uu_wlan_cap_handler.o \
	lmac/cap/src/uu_wlan_cap_slot_timer.o \
	lmac/cap/src/uu_wlan_cap_txop.o \
	lmac/cap/src/uu_wlan_cap_txop_sm.o \
	lmac/sta/src/uu_wlan_lmac_sta_info.o \
#    lmac/test/phy_stub_module/fwk_netlink_api/kernel_nwlink.o \
	
umac_simulation-y += \
    lmac/test/phy_stub_module/fwk_netlink_api/kernel_nwlink.o \
    lmac/test/test_driver/umac/uu_wlan_test_init.o \
	lmac/test/test_driver/umac/uu_wlan_mac_script.o 

CFLAGS_MODULE += -DUU_UMAC_STUB

else
ifeq ($(BUILDPLATFORM),sim)
obj-m += lmac_wlan.o uu_wlan.o
else
obj-m += uu_wlan.o
endif

# Build kernel module, to test MAC along with MAC framework.
# PHY will be simulated.
ifeq ($(BUILDPLATFORM),sim)
lmac_wlan-y +=	\
	lmac/utils/src/crc32.o \
	lmac/utils/src/uu_wlan_buf.o \
	lmac/utils/src/uu_wlan_buf_desc.o \
	lmac/utils/src/uu_wlan_utils.o \
	lmac/utils/src/uu_wlan_rx_q.o \
	lmac/utils/src/uu_wlan_cap_q.o \
	lmac/utils/src/uu_wlan_ac_q.o \
	lmac/rx/src/uu_wlan_rx_handler.o \
	lmac/rx/src/uu_wlan_rx_status.o \
	lmac/cp/src/uu_wlan_cp_if.o \
	lmac/cp/src/uu_wlan_cp_ba_sb_rx.o \
	lmac/cp/src/uu_wlan_cp_ba_sb_tx.o \
	lmac/cp/src/uu_wlan_cp_ba_session.o \
	lmac/cp/src/uu_wlan_cp_rx_proc.o \
	lmac/cp/src/uu_wlan_cp_ctrl_pkt_gen.o \
	lmac/ps/src/uu_wlan_ps_uapsd.o \
	lmac/tx/src/uu_wlan_tx_handler.o \
	lmac/tx/src/uu_wlan_tx_aggregation.o \
	lmac/tx/src/uu_wlan_tx_status.o \
	lmac/tx/src/uu_wlan_beacon.o \
	lmac/src/uu_wlan_lmac_init.o \
	lmac/src/uu_wlan_reg.o \
	lmac/dur/src/uu_wlan_rate.o \
	lmac/dur/src/uu_wlan_lmac_tpc.o \
	lmac/dur/src/uu_wlan_duration.o \
	lmac/cap/src/uu_wlan_cap_init.o \
	lmac/cap/src/uu_wlan_cap_sm.o \
	lmac/cap/src/uu_wlan_cap_cm.o \
	lmac/cap/src/uu_wlan_cap_context.o \
	lmac/cap/src/uu_wlan_cap_handler.o \
	lmac/cap/src/uu_wlan_cap_slot_timer.o \
	lmac/cap/src/uu_wlan_cap_txop.o \
	lmac/cap/src/uu_wlan_cap_txop_sm.o \
	lmac/sta/src/uu_wlan_lmac_sta_info.o 
endif
uu_wlan-y +=	\
	umac/src/uu_wlan_eeprom.o \
	umac/src/uu_wlan_umac_init.o \
	umac/src/uu_mac_fwk_cb.o \
	umac/src/uu_mac_tx.o \
	umac/src/uu_mac_rate.o \
	umac/src/uu_mac_drv.o \
	umac/src/uu_wlan_recv.o \
	umac/src/uu_wlan_agg.o \
	umac/src/uu_wlan_queue.o \
	umac/src/uu_wlan_reg_init.o \
	umac/src/uu_mac_drv_txstatus.o \
	umac/src/uu_mac_scheduling.o \
    umac/src/uu_mac_txstatus.o \
    umac/src/uu_wlan_umac_dfs.o \
    umac/src/uu_wlan_umac_tpc.o
uu_wlan-y +=    \
    umac/src/$(BUILDPLATFORM)/uu_wlan_init_platform.o \
    umac/src/$(BUILDPLATFORM)/uu_wlan_platform.o
endif


CFLAGS_MODULE += -Wall

# Partially adding options corresponding to -Wextra
CFLAGS_MODULE += -Wclobbered -Wignored-qualifiers -Wmissing-parameter-type -Wold-style-declaration -Wtype-limits -Wuninitialized -Winit-self -Wunreachable-code -Wfloat-equal -Wtype-limits # -Wsign-compare

CFLAGS_MODULE += -O1
#CFLAGS_MODULE += -g


CFLAGS_MODULE +=  -I$(UU_DIR)/inc \
	-I$(UU_DIR)/fwk/inc \
	-I$(UU_DIR)/umac/inc \
	-I$(UU_DIR)/lmac/inc \
	-I$(UU_DIR)/lmac/utils/inc \
	-I$(UU_DIR)/lmac/tx/inc \
	-I$(UU_DIR)/lmac/cp/inc \
	-I$(UU_DIR)/lmac/rx/inc \
	-I$(UU_DIR)/lmac/cap/inc \
	-I$(UU_DIR)/lmac/dur/inc \
	-I$(UU_DIR)/lmac/sta/inc \
	-I$(UU_DIR)/lmac/test/phy_stub_module/phy \
	-I$(UU_DIR)/lmac/test/phy_stub_module/fwk_netlink_api


CFLAGS_MODULE +=  -I$(UU_DIR)/inc/$(BUILDPLATFORM)

CFLAGS_MODULE += -DPHY_STUB

CFLAGS_MODULE += -DUU_KTIMER
CFLAGS_MODULE += -DUU_WLAN_RC_RTL_TABLE

#TODO: Cleanup this & corresponding C code.
#CFLAGS_MODULE += -DCONFIG_THREAD  --No longer required, as not using the thread to simulate RTL trigger.
#CFLAGS_MODULE += -DLMAC_UMAC_TX_THREAD  --No longer required, as we removed the intermediate Tx Q & directly writing to AC Q.

# TODO: Get rid of this macro. Currently used in UMAC only.
#CFLAGS_MODULE += -DUU_WLAN_UMAC_RCAC

# for TPC
#CFLAGS_MODULE += -DUU_WLAN_TPC
# for DFS
#CFLAGS_MODULE += -DUU_WLAN_DFS
# for debugging TPC and DFS
#CFLAGS_MODULE += -DUU_WLAN_DFSTPC_DEBUG
#for constant number for TPC Request
#CFLAGS_MODULE += -DUU_TPC_HRTIMER_CONST


#TODO: Cleanup these macros
# for sending VHT frames
#CFLAGS_MODULE += -DUU_WLAN_VHT_FRAME
#for sending data at 20 MHz
#CFLAGS_MODULE += -DUU_WLAN_DATA_20
#for sending data at 40 MHz
#CFLAGS_MODULE += -DUU_WLAN_DATA_40
#for sending data at 80 MHz
#CFLAGS_MODULE += -DUU_WLAN_DATA_80
#for sending data at 160 MHz
#CFLAGS_MODULE += -DUU_WLAN_DATA_160

# This is for TSF synchronization. And is mandatory to enable for IBSS.
CFLAGS_MODULE += -DUU_WLAN_TSF

# This is for keeping Beacon frames in Beacon QID.
# This is mandatory to enable for IBSS.
CFLAGS_MODULE += -DUU_WLAN_BQID

# This is for enabling some functions in IBSS. First enable UU_WLAN_TSF, and UU_WLAN_BQID.
CFLAGS_MODULE += -DUU_WLAN_IBSS



all:
	echo Current working dir $(UU_DIR)
	echo Current working LINUX SRC dir $(linux_dir)
	make -C $(linux_dir)  M=$(UU_DIR) modules
clean:
	make -C $(linux_dir) M=$(UU_DIR) clean

