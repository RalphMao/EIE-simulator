
SRC_FILES := %.c %.cpp
OBJ_FILES := $(patsubst %.c, %, $(wildcard *.c)) $(patsubst %.cpp, %, $(wildcard *.cpp)) 

CFLAGS := -I/usr/local/cuda/include -std=c99 -L/usr/local/cuda/lib64 -lcudart -lcusparse -lcublas
CPPFLAGS := -O2 -larmadillo -Wfatal-errors

all:$(OBJ_FILES)

clean:
	rm $(OBJ_FILES)
%:%.c
	gcc -o $@ $< $(CFLAGS)

%:%.cpp
	g++ -o $@ $< $(CPPFLAGS)

