CXX = g++
CFLAGS = -std=c++11 -Wall -O2
INCLUDES = \
	-Isrc/include \
	-Ilib/optional-lite/include

OUT = ./build
EXE = $(OUT)/npshell
SRCS = ./src
OBJS = $(OUT)/objs

ALL := $(patsubst $(SRCS)/%.cc,$(OBJS)/%.o,$(wildcard $(SRCS)/*.cc))

.PHONY: build

all: build
	@cp $(EXE) ./npshell
	@chmod 775 ./npshell

$(OBJS)/%.o: $(SRCS)/%.cc  $(shell find $(SRCS)/include -name "*.h")
	@mkdir -p $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

build: $(ALL)
	@mkdir -p $(OUT)
	$(CXX) $(CFLAGS) $(INCLUDES) $(ALL) -o $(EXE)
	@chmod 775 $(EXE)
