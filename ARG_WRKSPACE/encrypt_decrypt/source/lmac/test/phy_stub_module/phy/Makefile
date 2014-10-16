#UnComment the following line to build lmac with test driver
#BUILD_TYPE:= "lmac_test"

UU_DIR:= $(PWD)


linux_dir:= /lib/modules/$(shell uname -r)/build

obj-m += phy_stub_nl.o 
phy_stub_nl-objs := uu_phy_stub_nl.o ../fwk_netlink_api/kernel_nwlink.o \
		 uu_wlan_phy_script.o \

CFLAGS_MODULES = -Wall

ifneq ($(BUILD_TYPE), )
else
CFLAGS_MODULE +=  -DUU_WLAN_END_TO_END_TESTING 
endif

EXTRA_CFLAGS += -g -Wall -I$(UU_DIR)/. -I$(UU_DIR)/../fwk_netlink_api/ -I$(UU_DIR)/../../../inc \
		-I$(UU_DIR)/../../../../fwk/inc -I$(UU_DIR)/../../../../inc -I$(UU_DIR)/../../../../inc/sim \
        

all:
	echo "Current working dir $(UU_DIR)"
	make -C $(linux_dir)  M=$(UU_DIR) modules
clean:
	make -C $(linux_dir) M=$(UU_DIR) clean

