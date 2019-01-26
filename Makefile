CXX = g++
CFLAGS = -std=c++11 -Wall -O2
INCLUDES = \
	-Isrc/include \
	-Ilib/easyloggingpp/src \
	-Ilib/optional-lite/include

OUT = ./build
EXE = $(OUT)/npshell_server
SRCS = ./src
LIBS = ./lib
OBJS = $(OUT)/objs
ALL := \
	$(OBJS)/lib/easyloggingpp.o \
	$(patsubst $(SRCS)/%.cc,$(OBJS)/%.o,$(wildcard $(SRCS)/*.cc))

.PHONY: build

all: build
	@cp $(EXE) ./npshell_server
	@chmod 775 ./npshell_server

$(OBJS)/%.o: $(SRCS)/%.cc  $(shell find $(SRCS)/include -name "*.h")
	@mkdir -p $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJS)/lib/easyloggingpp.o: $(LIBS)/easyloggingpp/src/easylogging++.cc
	@mkdir -p $(OBJS)/lib
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

build: $(ALL)
	@mkdir -p $(OUT)
	$(CXX) $(CFLAGS) $(INCLUDES) $(ALL) -o $(EXE)
	@chmod 775 $(EXE)
