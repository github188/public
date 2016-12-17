# target source
SRC  := $(wildcard *.c) 
OBJ  := $(SRC:%.c=%.o)

TARGET := $(OBJ:%.o=%)
.PHONY : clean all
all: $(TARGET)
CC = /opt/CodeSourcery/Sourcery_G++_Lite/bin/arm-none-linux-gnueabi-gcc
RTSP_LIBS := -L ./../rtsp/mediaServer/
INCLUDE = ./../rtsp/mediaServer/rtsp_client_public.h
$(TARGET):%:%.o 
	$(CC) $(INCLUDE) -o $@ $^ 
