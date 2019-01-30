CXX = g++
CFLAGS = -std=c++11 -Wall -O2
INCLUDES = \
	-Isrc/include \
	-Ilib/fmt/include \
	-Ilib/spdlog/include \
	-Ilib/optional-lite/include

OUT = ./build
EXE = $(OUT)/np_single_proc
SRC = ./src
LIB = ./lib
OBJ = $(OUT)/objs
ALL := \
	$(wildcard $(LIB)/fmt/src/*.cc) \
	$(patsubst $(SRC)/%.cc,$(OBJ)/%.o,$(wildcard $(SRC)/*.cc))

.PHONY: build

all: build
	@cp $(EXE) ./np_single_proc
	@chmod 775 ./np_single_proc

$(OBJ)/%.o: $(SRC)/%.cc  $(shell find $(SRC)/include -name "*.h")
	@mkdir -p $(OBJ)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

build: $(ALL)
	@mkdir -p $(OUT)
	$(CXX) $(CFLAGS) $(INCLUDES) $(ALL) -o $(EXE)
	@chmod 775 $(EXE)
