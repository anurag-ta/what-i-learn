
TARGET := lmac_duration_tester

CC=gcc
CFLAGS= -g -Wuninitialized -Wextra -Wall -W  -DQ_MODULE_TESTING -DUSER_SPACE -DUU_WLAN_RC_RTL_TABLE
CFLAGS+= -ftest-coverage --coverage

SRC :=  ./uu_wlan_test_main.c \
        ./uu_wlan_duration_test.c \
        ../../src/uu_wlan_duration.c \
        ../../src/uu_wlan_rate.c \
        ../../../src/uu_wlan_reg.c \


INC :=  -I../../inc \
        -I../../../inc/ \
        -I../../../../inc/ \
        -I../../../../fwk/inc/ \

OBJS := $(SRC:%.c=%.o)


CUR_DIR:= uu_wlan_test_main.o \
        uu_wlan_duration_test.o \
        uu_wlan_duration.o \
        uu_wlan_rate.o \
        uu_wlan_reg.o \

$(TARGET): TEST_OBJS
	$(CC) $(CFLAGS) $(INC) $(CUR_DIR)  -o $(TARGET)


TEST_OBJS: $(SRC:%.c)
	$(CC) $(CFLAGS) $(INC) -c $(SRC:%.c=%.c)

clean:
	rm -f *.o $(TARGET)
	rm -f *.gcno *.gcda *.gcov


