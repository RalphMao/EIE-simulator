
CXX=g++
FLAGS=-Wall -O2 -g
O_FILES=spmat.o test_spmat.o
C_FILES=spmat.cpp test_spmat.cpp
H_FILES=module.h configs.h
EXE=
TEST=test

# all: $(O_FILES)
    
test: $(O_FILES) $(H_FILES)
	$(CXX) -o $(TEST) $(O_FILES)

$(O_FILES): $(C_FILES) $(H_FILES)
	$(CXX) -c $(FLAGS) $(C_FILES) 

clean:
	rm *.o test
