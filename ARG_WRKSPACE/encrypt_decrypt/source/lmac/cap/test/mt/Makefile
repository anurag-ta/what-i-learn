
TARGET := lmac_cap_tester

CC=gcc
CFLAGS= -g -Wuninitialized -Wextra -DCAP_MODULE_TESTING -DUSER_SPACE -DUU_DEBUG #-W  -std=gnu99 -D_GNU_SOURCE
CFLAGS+=--coverage -ftest-coverage

SRC :=  ./uu_wlan_test_main.c \
        ./uu_wlan_cap_test.c \
        ../../src/uu_wlan_cap_init.c \
        ../../src/uu_wlan_cap_sm.c \
        ../../src/uu_wlan_cap_txop.c \
        ../../src/uu_wlan_cap_txop_sm.c \
        ../../src/uu_wlan_cap_cm.c \
        ../../src/uu_wlan_cap_context.c \
        ../../src/uu_wlan_cap_slot_timer.c \
        ../../../src/uu_wlan_reg.c \
        ../../../utils/src/crc32.c \
        ../../../utils/src/uu_wlan_cap_q.c \
        ../../../utils/src/uu_wlan_buf.c \
        ../../../utils/src/uu_wlan_buf_desc.c \
        ../../../../lmac/utils/src/uu_wlan_tx_q.c \
        ../../../utils/src/uu_wlan_utils.c \
        ../../../utils/src/uu_wlan_rx_q.c \
        ../../../utils/src/uu_wlan_ac_q.c \


INC :=  -I../../inc \
        -I../../../inc \
        -I../../../../inc \
        -I../../../utils/inc \
        -I../../../tx/inc \
        -I../../../rx/inc \
        -I../../../cp/inc \
        -I../../../sta/inc \
        -I../../../../fwk/inc 

OBJS := $(SRC:%.c=%.o)


CUR_DIR:= ./uu_wlan_test_main.o \
        ./uu_wlan_cap_test.o \
        ./uu_wlan_cap_sm.o \
        ./uu_wlan_cap_txop.o \
        ./uu_wlan_cap_txop_sm.o \
        ./uu_wlan_cap_cm.o \
        ./uu_wlan_cap_context.o \
        ./uu_wlan_cap_init.o \
        ./uu_wlan_tx_q.o \
        ./uu_wlan_utils.o \
        ./crc32.o \
        ./uu_wlan_cap_q.o \
        ./uu_wlan_reg.o \
        ./uu_wlan_buf.o \
        ./uu_wlan_buf_desc.o \
        ./uu_wlan_cap_slot_timer.o \
        ./uu_wlan_rx_q.o \
        ./uu_wlan_ac_q.o \

$(TARGET): TEST_OBJS
	$(CC) $(CFLAGS) $(INC) $(CUR_DIR)  -o $(TARGET)


TEST_OBJS: $(SRC:%.c)
	$(CC) $(CFLAGS) $(INC) -c $(SRC:%.c=%.c)

clean:
	rm -f *.o $(TARGET) *.gcda *.gcno *.gcov


