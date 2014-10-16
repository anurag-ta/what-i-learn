
TARGET := lmac_tx_tester

CC=gcc
CFLAGS= -g -Wall -Wextra -std=gnu99 -D_GNU_SOURCE -DUU_WLAN_TSF -DUSER_SPACE -DUU_DEBUG
CFLAGS+=--coverage -ftest-coverage -include ../../../../fwk/inc/uu_wlan_fwk_lock.h

SRC :=  ./uu_wlan_test_main.c \
        ./uu_wlan_tx_test.c \
        ../../../tx/src/uu_wlan_tx_handler.c \
        ../../../tx/src/uu_wlan_tx_aggregation.c \
        ../../../tx/src/uu_wlan_tx_status.c \
        ../../../utils/src/uu_wlan_ac_q.c \
        ../../../utils/src/crc32.c \
        ../../../utils/src/uu_wlan_buf_desc.c \
        ../../../src/uu_wlan_reg.c \
        ../../../utils/src/uu_wlan_cap_q.c \
        ../../../utils/src/uu_wlan_utils.c \

INC :=  -I../../inc \
    -I../../../inc \
    -I../../../../inc \
    -I../../../utils/inc \
    -I../../../../fwk/inc\
    -I../../../tx/inc \
    -I../../../cp/inc \
    -I../../../sta/inc \
    -I../../../cap/inc \
    -I../../../../inc/sim \

OBJS := $(SRC:%.c=%.o)

CUR_DIR:= ./uu_wlan_test_main.o \
        ./uu_wlan_tx_test.o \
        ./uu_wlan_tx_handler.o \
        ./uu_wlan_tx_aggregation.o \
        ./uu_wlan_buf_desc.o \
        ./uu_wlan_cap_q.o \
        ./uu_wlan_ac_q.o \
        ./uu_wlan_reg.o \
        ./crc32.o \
        ./uu_wlan_utils.o \
        ./uu_wlan_tx_status.o \

$(TARGET): TEST_OBJS
	$(CC) $(CFLAGS) $(INC) $(CUR_DIR)  -o $(TARGET)


#TEST_OBJS: $(SRC:%.c)
TEST_OBJS: $(SRC:%.c)
	$(CC) $(CFLAGS) $(INC) -c $(SRC:%.c=%.c)

clean:
	rm -f *.o $(TARGET) *.gcda *.gcno *.gcov

