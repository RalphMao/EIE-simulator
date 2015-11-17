
PLATFORM := TK1
GPU_ID := 0
TEST_POWER := 1

SRC_FILES := %.c %.cpp
OBJ_FILES := $(patsubst %.c, %, $(wildcard *.c)) $(patsubst %.cpp, %, $(wildcard *.cpp)) 

ifeq ($(PLATFORM), PC) 
    INCLUDE_PATH := /usr/local/cuda/include
    LIB_PATH := /usr/local/cuda/lib64
else
    INCLUDE_PATH := /usr/local/cuda-6.5/targets/armv7-linux-gnueabihf/include
    LIB_PATH := /usr/local/cuda-6.5/targets/armv7-linux-gnueabihf/lib
endif

ifeq ($(TEST_POWER), 1)
    TIMES := 100000
else
    TIMES := 1
endif

CFLAGS := -I$(INCLUDE_PATH) -std=c99 -L$(LIB_PATH) -lcudart -lcusparse -lcublas -D GPU_ID=$(GPU_ID) -D TIMES=$(TIMES)
CPPFLAGS := -O2 -larmadillo -Wfatal-errors

all:$(OBJ_FILES)

clean:
	rm $(OBJ_FILES)
%:%.c
	gcc -o $@ $< $(CFLAGS)

%:%.cpp
	g++ -o $@ $< $(CPPFLAGS)

