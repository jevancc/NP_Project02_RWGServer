CC = clang++
CPPFLAGS = -Wall -O2 -pedantic -std=c++11

NP_SHELL_CMD_SRC = $(wildcard src/*.cpp)
NP_SHELL_CMD = $(patsubst src/%.cpp,../_workSpaceTemplate/bin/%,$(NP_SHELL_CMD_SRC))

FILE_NAME = $(wildcard file/*)
FILE = $(patsubst file/%,../_workSpaceTemplate/%,$(FILE_NAME))

BUILD_IN_CMD_NAME = ls cat echo sleep
BUILD_IN_CMD = $(patsubst %,../_workSpaceTemplate/bin/%,$(BUILD_IN_CMD_NAME))

.PHONY: work

all: work

%: src/%.cpp
	$(CC) $< $(CPPFLAGS) -o $@

work: ../_workSpaceTemplate/ \
	../_workSpaceTemplate/bin/ \
	$(FILE) $(NP_SHELL_CMD) $(BUILD_IN_CMD)

../_workSpaceTemplate/bin/%: src/%.cpp | ../_workSpaceTemplate/bin/
	$(CC) $< $(CPPFLAGS) -o $@

../_workSpaceTemplate/bin/%: /bin/%
	cp -f $< $@

../_workSpaceTemplate/bin/:
	mkdir -p ../_workSpaceTemplate/bin/

../_workSpaceTemplate/%: file/%
	cp $< $@
