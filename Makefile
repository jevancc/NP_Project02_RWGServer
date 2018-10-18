CXX = g++
CFLAGS = -std=c++11 -Wall
INCLUDE = src/include

OUT = ./out
SRCS = ./src
OBJS = $(OUT)/objs

ALL := \
	$(OBJS)/builtin.o

all: main

$(OBJS)/%.o: $(SRCS)/%.cc
	@mkdir -p $(OBJS)
	$(CXX) $(CFLAGS) -I$(INCLUDE) -c $< -o $@

main: $(ALL)
	@mkdir -p $(OUT)
	$(CXX) $(CFLAGS) -I$(INCLUDE) src/main.cc $(ALL) -o $(OUT)/shell
	@chmod 775 $(OUT)/shell
