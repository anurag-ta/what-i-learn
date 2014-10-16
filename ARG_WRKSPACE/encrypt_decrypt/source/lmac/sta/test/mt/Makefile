
TARGET := lmac_sta_tester

CC=gcc
CFLAGS= -g -Wuninitialized -Wextra -W  -std=gnu99 -D_GNU_SOURCE -DSTA_MODULE_TESTING -DUSER_SPACE -DUU_DEBUG
CFLAGS+=--coverage -ftest-coverage

SRC :=  ./uu_wlan_test_main.c \
        ./uu_wlan_sta_test.c \
        ../../src/uu_wlan_lmac_sta_info.c \
        ../../../src/uu_wlan_lmac_init.c \
        ../../../src/uu_wlan_reg.c \

INC :=  -I../../inc \
        -I../../../inc \
        -I../../../../inc \
        -I../../../../fwk/inc \
        -I../../../utils/inc \
        -I../../../tx/inc \
        -I../../../cp/inc \

OBJS := $(SRC:%.c=%.o)

CUR_DIR:= ./uu_wlan_test_main.o \
          ./uu_wlan_sta_test.o \
          ./uu_wlan_lmac_sta_info.o \
          ./uu_wlan_lmac_init.o \
          ./uu_wlan_reg.o \

$(TARGET): TEST_OBJS
	$(CC) $(CFLAGS) $(INC) $(CUR_DIR)  -o $(TARGET)


TEST_OBJS: $(SRC:%.c)
	$(CC) $(CFLAGS) $(INC) -c $(SRC:%.c=%.c)

clean:
	rm -f *.o $(TARGET) *.gcda *.gcno *.gcov


