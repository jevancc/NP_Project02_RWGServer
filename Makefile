CXX = g++
CFLAGS = -std=c++11 -Wall -O2
INCLUDES = \
	-Isrc/include \
	-Ilib/easyloggingpp/src \
	-Ilib/optional-lite/include \
	-Ilib

OUT = ./build
EXE = $(OUT)/npshell
SRCS = ./src
OBJS = $(OUT)/objs

ALL := \
	$(OBJS)/main.o \
	$(OBJS)/shell.o \
	$(OBJS)/builtin.o \
	$(OBJS)/pipe.o \
	$(OBJS)/command.o \
	$(OBJS)/task.o \
	$(OBJS)/environment.o

all: build

$(OBJS)/%.o: $(SRCS)/%.cc
	@mkdir -p $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

build: $(ALL)
	@mkdir -p $(OUT)
	$(CXX) $(CFLAGS) $(INCLUDES) $(ALL) -o $(EXE)
	@chmod 775 $(EXE)
