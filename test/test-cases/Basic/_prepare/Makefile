CC = g++
CPPFLAGS = -Wall -O2 -pedantic -std=c++11

NP_SHELL_CMD_SRC = $(wildcard src/*.cpp)
NP_SHELL_CMD = $(patsubst src/%.cpp,../_template/bin/%,$(NP_SHELL_CMD_SRC))

FILE_NAME = $(wildcard file/*)
FILE = $(patsubst file/%,../_template/%,$(FILE_NAME))

BUILD_IN_CMD_NAME = ls cat
BUILD_IN_CMD = $(patsubst %,../_template/bin/%,$(BUILD_IN_CMD_NAME))

BUILD_IN_CMD_NAME_2 = echo
BUILD_IN_CMD_2 = $(patsubst %,../_template/bin2/%,$(BUILD_IN_CMD_NAME_2))

.PHONY: work

all: work

%: src/%.cpp
	$(CC) $< $(CPPFLAGS) -o $@

work: ../_template/ \
	../_template/bin/ \
	../_template/bin2/ \
	$(FILE) $(NP_SHELL_CMD) $(BUILD_IN_CMD) $(BUILD_IN_CMD_2)

../_template/bin/%: src/%.cpp | ../_template/bin/
	$(CC) $< $(CPPFLAGS) -o $@

../_template/bin/%: /bin/%
	cp -f $< $@

../_template/bin/:
	mkdir -p ../_template/bin


../_template/bin2/%: /bin/%
	cp -f $< $@

../_template/bin2/:
	mkdir -p ../_template/bin2

../_template/%: file/%
	cp $< $@
