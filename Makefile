CXX = g++
CFLAGS = -std=c++11 -Wall
INCLUDES = \
	-Isrc/include \
	-Ilib/easyloggingpp/src \
	-Ilib/optional-lite/include \
	-Ilib

OUT = ./out
EXE = $(OUT)/npshell
SRCS = ./src
OBJS = $(OUT)/objs

ALL := \
	$(OBJS)/shell.o \
	$(OBJS)/builtin.o \
	$(OBJS)/pipe.o \
	$(OBJS)/command.o \
	$(OBJS)/task.o \
	$(OBJS)/environment.o
	# $(OBJS)/builtin.o

all: main

$(OBJS)/%.o: $(SRCS)/%.cc
	@mkdir -p $(OBJS)
	$(CXX) $(CFLAGS) $(INCLUDES) -c $< -o $@

main: $(ALL)
	@mkdir -p $(OUT)
	$(CXX) $(CFLAGS) $(INCLUDES) src/main.cc $(ALL) -o $(EXE)
	@chmod 775 $(EXE)
