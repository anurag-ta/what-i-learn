These are the files required for testing Linux Kernel along with LMAC running on FPGA


Steps Required for loading through XMD using Xilinx SDK

xmd
connect arm hw
fpga -f system.bit
source ps7_init.tcl
ps7_init
init_user
source stub.tcl
target 64
dow u-boot.elf
con
< pause uboot in terminal > in terminal
stop in xmd
dow -data zImage 0x3000000
dow -data ramdisk8M.image.gz 0x2000000
dow -data xilinx_15_6pm.dtb 0x2A00000
go 0x3000000

