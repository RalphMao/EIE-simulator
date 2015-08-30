
CXX=clang++

DEBUG=1

ifeq ($(DEBUG), 1)
	TEST_FLAGS=-Wall -g -std=c++11 -O0 -Wuninitialized -Wfatal-errors -D DEBUG=1
else
	TEST_FLAGS=-std=c++ -O2 
endif
LINK_FLAGS=-rdynamic 

O_FILES=spmat.o ptrvec.o arithm.o nzfetch.o actrw.o system.o
C_FILES=spmat.cpp ptrvec.cpp arithm.cpp actrw.cpp nzfetch.cpp system.cpp
H_FILES=*.h
EXE=simulation

TEST_C_FILES=test_spmat.cpp
TEST_O_FILES=test_spmat.o
TEST_EXE=test

all: $(O_FILES) 
	$(CXX) -o $(EXE) $(O_FILES) $(LINK_FLAGS)
    
test_nzf: $(H_FILES) spmat.cpp ptrvec.cpp nzfetch.cpp test_nzfetch.cpp
	$(CXX) -c spmat.cpp ptrvec.cpp nzfetch.cpp test_nzfetch.cpp $(TEST_FLAGS)
	$(CXX) -o $(TEST_EXE) spmat.o test_nzfetch.o nzfetch.o ptrvec.o $(LINK_FLAGS)

test_ptrvec: $(H_FILES) spmat.cpp ptrvec.cpp test_ptrvec.cpp
	$(CXX) -c test_ptrvec.cpp spmat.cpp ptrvec.cpp $(TEST_FLAGS)
	$(CXX) -o $(TEST_EXE) spmat.o test_ptrvec.o ptrvec.o $(LINK_FLAGS)

test_spmat: $(H_FILES) spmat.cpp test_spmat.cpp
	$(CXX) -c test_spmat.cpp spmat.cpp $(TEST_FLAGS)
	$(CXX) -o $(TEST_EXE) spmat.o test_spmat.o $(LINK_FLAGS)

test_arithm: $(H_FILES) arithm.cpp test_arithm.cpp
	$(CXX) -c arithm.cpp test_arithm.cpp $(TEST_FLAGS)
	$(CXX) -o $(TEST_EXE) arithm.o test_arithm.o $(LINK_FLAGS)

$(O_FILES): $(C_FILES) $(H_FILES)
	$(CXX) -c $(C_FILES) $(TEST_FLAGS)

clean:
	rm *.o test simulation
