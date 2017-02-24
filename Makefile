CC = gcc
AR = ar

SRC=./src
INCLUDE=./include
TEST_SRC=./tests

INC		=-I./include

HASH_LIB=./lib/libut.a

CFLAGS = $(INC) -lpthread

SOURCES		= $(SRC)/ornvcr_api_internal.c $(SRC)/ornvcr_api.c
OBJS    	= $(SOURCES:.c=.o)

TEST_SOURCES1		= $(TEST_SRC)/ornvcr_init_test.c 
TEST_SOURCES2		= $(TEST_SRC)/ornvcr_register_test.c
#TEST_OBJS    	= $(TEST_SOURCES:.c=.o)

TARGET		=liborcr.so
TARGET_STATIC	=liborcr.a
TEST1        =test_init
TEST2        =test_register

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

all: $(OBJS)  $(TARGET_STATIC) $(TEST1) $(TEST2)

#$(TARGET): $(OBJS)
#	$(CC) $(CFLAGS) $(LIB) -o $(TARGET) $(OBJS) -shared

$(TARGET_STATIC): $(OBJS)
	$(AR) rcs $(TARGET_STATIC) $(OBJS)

$(TEST1): $(TARGET_STATIC)
	$(CC) $(CFLAGS) -o $(TEST1) $(TEST_SOURCES1) $(TARGET_STATIC) $(HASH_LIB)

$(TEST2): $(TARGET_STATIC)
	$(CC) $(CFLAGS) -o $(TEST2) $(TEST_SOURCES2) $(TARGET_STATIC) $(HASH_LIB)

clean: 
	rm -f ${SRC}/*.o liborcr.a liborcr.so*
