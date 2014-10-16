obj-m += k_netlink.o
k_netlink-objs := kern_test_nl.o kernel_nwlink.o
CFLAGS_MODULES = -Wall
EXTRA_CFLAGS += -g -Wall -I$(PWD)/.
def:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -g -Wall -o u_netlink user_nl_main.c uuNetlinkIf.c
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm u_netlink
