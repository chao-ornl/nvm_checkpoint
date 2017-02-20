CC = gcc
AR = ar

SRC=./src
INCLUDE=./include
TEST_SRC=./tests

INC		=-I./include

LIB     =-L./lib -lut

CFLAGS = $(INC) -std=c90

SOURCES		= $(SRC)/ornvcr_api_internal.c $(SRC)/ornvcr_api.c
OBJS    	= $(SOURCES:.c=.o)

TEST_SOURCES		= $(TEST_SRC)/ornvcr_init_test.c
#TEST_OBJS    	= $(TEST_SOURCES:.c=.o)

TARGET		=liborcr.so
TARGET_STATIC	=liborcr.a
TEST        =test_api

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

all: $(OBJS) $(TARGET) $(TARGET_STATIC) $(TEST)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LIB) -o $(TARGET) $(OBJS) -shared

$(TARGET_STATIC): $(OBJS) $(TARGET)
	$(AR) rcs $(TARGET_STATIC) $(OBJS)

$(TEST): $(TARGET) $(TEST_OBJS)
	$(CC) $(CFLAGS) $(LIB) -L. -lorcr -o $(TEST) $(TEST_SOURCES)
clean: 
	rm -f ${SRC}/*.o liborcr.a liborcr.so*