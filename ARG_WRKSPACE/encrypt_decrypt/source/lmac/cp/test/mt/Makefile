
TARGET := lmac_cp_tester

CC=gcc
CFLAGS= -g -Wall -Wextra -std=gnu99 -DUSER_SPACE -DUU_WLAN_RC_RTL_TABLE
CFLAGS+= -ftest-coverage --coverage -include ../../../../fwk/inc/uu_wlan_fwk_lock.h -include ../../../../fwk/inc/uu_wlan_fwk_log.h

SRC :=  ./uu_wlan_test_main.c \
        ./uu_wlan_cp_test.c \
        ../../src/uu_wlan_cp_ba_sb_rx.c \
        ../../src/uu_wlan_cp_ba_session.c \
        ../../src/uu_wlan_cp_rx_proc.c \
        ../../src/uu_wlan_cp_ctrl_pkt_gen.c \
        ../../src/uu_wlan_cp_ba_sb_tx.c \
        ../../src/uu_wlan_cp_if.c \
        ../../../utils/src/crc32.c \
        ../../../sta/src/uu_wlan_lmac_sta_info.c \
        ../../../src/uu_wlan_reg.c \
        ../../../utils/src/uu_wlan_ac_q.c \
        ../../../utils/src/uu_wlan_buf_desc.c \
        ../../../../lmac/utils/src/uu_wlan_utils.c \
        ../../../dur/src/uu_wlan_duration.c \
        ../../../dur/src/uu_wlan_rate.c \
        ../../../src/uu_wlan_lmac_init.c \


INC :=  -I../../inc \
        -I../../../inc \
        -I../../../../fwk/inc \
        -I../../../../inc \
        -I../../../sta/inc/ \
        -I../../../tx/inc \
        -I../../../utils/inc \
        -I../../../utils/inc/mq/ \
        -I../../../../lmac/utils/inc  \
	    -I../../../../inc/sim \

OBJS := $(SRC:%.c=%.o)


CUR_DIR:= uu_wlan_test_main.o \
        uu_wlan_cp_test.o \
        uu_wlan_cp_ba_sb_rx.o \
        uu_wlan_cp_ba_session.o \
        uu_wlan_cp_rx_proc.o \
        uu_wlan_cp_ba_sb_tx.o \
        uu_wlan_cp_ctrl_pkt_gen.o \
        uu_wlan_cp_if.o \
        crc32.o \
        uu_wlan_lmac_sta_info.o \
        uu_wlan_reg.o \
        uu_wlan_ac_q.o \
        uu_wlan_buf_desc.o \
        uu_wlan_utils.o \
        uu_wlan_duration.o \
        uu_wlan_rate.o \
        uu_wlan_lmac_init.o


$(TARGET): TEST_OBJS
	$(CC) $(CFLAGS) $(INC) $(CUR_DIR)  -o $(TARGET)


TEST_OBJS: $(SRC:%.c)
	$(CC) $(CFLAGS) $(INC) -c $(SRC:%.c=%.c)

clean:
	rm -f *.o $(TARGET)
	rm -f *.gcno *.gcda *.gcov


