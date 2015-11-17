SRC := src
INCLUDE := include
BUILD := build
EXE := simulation
TARGET := $(BUILD)/$(EXE)


CXX = g++

DEBUG := 1
PROFILE := 1

ifeq ($(DEBUG), 1)
    CFLAGS := -Wall -g -std=c++11 -O2 -Wuninitialized -Wfatal-errors -D DEBUG=1 -D PROFILE=$(PROFILE)
else
    CFLAGS := -std=c++11 -O3 -D PROFILE=$(PROFILE)
endif

LINK_FLAGS=-rdynamic


OBJ_FILES := $(patsubst %.cpp, $(BUILD)/%.o, $(wildcard $(SRC)/*.cpp))
EXE_FILES := $(TARGET)
DEP_FILES := $(OBJ_FILES:.o=.d) $(patsubst %, %.d, $(EXE_FILES))

all:
	make clean
	@make -j 8 link
    
$(BUILD)/$(SRC)/%.o: $(SRC)/%.cpp	
	@mkdir -pv $(dir $@)
#	@echo "[cxx] $<"
	$(V) $(CXX) -o $@ -c $<  $(CFLAGS)

link: $(OBJ_FILES)
#	@echo "[link] $(TARGET)"
	$(V) $(CXX) -o $(TARGET) $(OBJ_FILES) $(CFLAGS)  $(LFLAGS) $(LINK_FLAGS)
	@ln -s $(TARGET) .
	@echo "[made soft link at current dir/$(EXE)]"
    
    
clean:
	@printf "[clean] "
	rm -rf $(BUILD)
	@printf "[clean] "
	rm -rf $(EXE)
    
.PHONY: all link extern clean
