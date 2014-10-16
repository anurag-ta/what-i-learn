CFLAGS		+= -Iinclude -I../../phy_stub_module/fwk_netlink_api/ -I../../phy_stub_module/phy/ 
CFLAGS		+= -g 

OBJS_PHY_STUB	= uu_wlan_test_sock_bridge.o ../../phy_stub_module/fwk_netlink_api/uuNetlinkIf.o

LIBS = -lpthread

TARGET =  uu_wlan_app$(EXE)

$(TARGET): $(OBJS_PHY_STUB)
	$(CC) $(CFLAGS) $(OBJS_PHY_STUB) -o $(@) $(LIBS)


clean:
	-rm -f $(TARGET) *.o 


