CC = gcc
AR = ar

SRC=./src
INCLUDE=./include
TEST=./tests

INC		=-I./include

CFLAGS = $(INC) -std=c90

SOURCES		= $(SRC)/ornvcr_api_internal.c $(SRC)/ornvcr_api.c
OBJS    	= $(SOURCES:.c=.o)

TARGET		=liborcr.so
STATIC_TARGET	=liborcr.a

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

all: $(OBJS) $(TARGET) $(TARGET_STATIC)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) -shared

$(TARGET_STATIC): $(OBJS) $(TARGET)
	$(AR) rcs $(TARGET_STATIC) $(OBJS)

clean: 
	rm -f ${SRC}/*.o liborcr.a liborcr.so*