CC = g++
CPPFLAGS = -Wall -O2 -pedantic -std=c++11

NP_SHELL_CMD_SRC = $(wildcard src/*.cpp)
NP_SHELL_CMD = $(patsubst src/%.cpp,../_template/bin/%,$(NP_SHELL_CMD_SRC))

FILE_NAME = $(wildcard file/*)
FILE = $(patsubst file/%,../_template/%,$(FILE_NAME))

BUILD_IN_CMD_NAME = ls cat echo sleep
BUILD_IN_CMD = $(patsubst %,../_template/bin/%,$(BUILD_IN_CMD_NAME))

BUILD_IN_CMD_NAME_2 = touch
BUILD_IN_CMD_2 = $(patsubst %,../_template/bin2/%,$(BUILD_IN_CMD_NAME_2))

.PHONY: dirs

all: dirs $(FILE) $(NP_SHELL_CMD) $(BUILD_IN_CMD) $(BUILD_IN_CMD_2)

dirs:
	@mkdir -p ../_template/bin
	@mkdir -p ../_template/bin2

%: src/%.cpp
	$(CC) $< $(CPPFLAGS) -o $@

../_template/bin/%: src/%.cpp dirs
	$(CC) $< $(CPPFLAGS) -o $@

../_template/bin/%: /bin/% dirs
	@cp -f $< $@

../_template/bin2/%: /bin/% dirs
	@cp -f $< $@

../_template/%: file/% dirs
	@cp $< $@
