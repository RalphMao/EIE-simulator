
CXX=g++
FLAGS=-Wall -O2 -g 
TEST_FLAGS=-Wall -g 
LINK_FLAGS=-rdynamic

O_FILES=spmat.o ptrvec.o system.o
C_FILES=spmat.cpp ptrvec.cpp system.cpp
H_FILES=module.h configs.h system.h
EXE=simulation

TEST_C_FILES=test_spmat.cpp
TEST_O_FILES=test_spmat.o
TEST_EXE=test

# all: $(O_FILES)
    
test_ptrvec: $(H_FILES) spmat.cpp ptrvec.cpp test_ptrvec.cpp
	$(CXX) -c test_ptrvec.cpp spmat.cpp ptrvec.cpp $(TEST_FLAGS)
	$(CXX) -o $(TEST_EXE) spmat.o test_ptrvec.o ptrvec.o $(LINK_FLAGS)

test_spmat: $(H_FILES) spmat.cpp test_spmat.cpp
	$(CXX) -c test_spmat.cpp spmat.cpp $(TEST_FLAGS)
	$(CXX) -o $(TEST_EXE) spmat.o test_spmat.o $(LINK_FLAGS)

$(O_FILES): $(C_FILES) $(H_FILES)
	$(CXX) -c $(FLAGS) $(C_FILES) 

clean:
	rm *.o test
