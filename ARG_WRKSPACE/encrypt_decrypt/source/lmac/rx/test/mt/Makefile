
TARGET := lmac_rx_tester

CC=gcc
CFLAGS= -g -Wall -Wextra -std=gnu99 -D_GNU_SOURCE -DUU_WLAN_TSF -DUSER_SPACE 
CFLAGS+=--coverage -ftest-coverage -include ../../../../fwk/inc/uu_wlan_fwk_lock.h -include ../../../../fwk/inc/uu_wlan_fwk_log.h

SRC :=  ./uu_wlan_test_main.c \
        ./uu_wlan_rx_test.c \
        ../../../rx/src/uu_wlan_rx_handler.c \
        ../../../utils/src/uu_wlan_rx_q.c \
        ../../../rx/src/uu_wlan_rx_status.c \
        ../../../utils/src/uu_wlan_buf_desc.c \
        ../../../utils/src/uu_wlan_buf.c \
        ../../../utils/src/crc32.c \
        ../../../utils/src/uu_wlan_utils.c \
	    ../../../ps/src/uu_wlan_ps_uapsd.c \
        ../../../../lmac/src/uu_wlan_reg.c 

INC :=  -I../../inc \
    -I../../../inc \
    -I../../../../inc \
    -I../../../utils/inc \
    -I../../../rx/inc \
    -I../../../sta/inc \
    -I../../../../fwk/inc \
    -I../../../cp/inc \
    -I../../../../inc/sim \
    -I../../../ps/inc \
    -I../../../cap/inc \

OBJS := $(SRC:%.c=%.o)

CUR_DIR:= ./uu_wlan_test_main.o \
        ./uu_wlan_rx_test.o \
        ./uu_wlan_rx_handler.o \
        ./uu_wlan_rx_q.o \
        ./uu_wlan_rx_status.o \
        ./uu_wlan_buf_desc.o \
        ./uu_wlan_buf.o \
        ./crc32.o \
        ./uu_wlan_utils.o \
        ./uu_wlan_reg.o \
        ./uu_wlan_ps_uapsd.o \

$(TARGET): TEST_OBJS
	$(CC) $(CFLAGS) $(INC) $(CUR_DIR)  -o $(TARGET)


TEST_OBJS: $(SRC:%.c)
	$(CC) $(CFLAGS) $(INC) -c $(SRC:%.c=%.c)

clean:
	rm -f *.o $(TARGET) *.gcda *.gcno *.gcov


