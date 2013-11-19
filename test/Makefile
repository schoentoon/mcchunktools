CFLAGS  := -Wall -O3 -mtune=native -g $(CFLAGS)
CC      := gcc
INC     := -I../cNBT -I../lib
VALGRIND:= /usr/bin/valgrind
VALGRIND_OPTS:= --leak-check=full

ifeq ($(CC), gcc)
	CXX := g++
else ifeq ($(CC), clang)
	CXX := clang++
endif

.PHONY: bin/test1 bin/test2

all: bin/test1 bin/test2

bin:
	mkdir bin

bin/test1: bin test1.c
	$(CC) $(CFLAGS) $(INC) -o bin/test1 test1.c ../lib/libmcchunk.a ../cNBT/libnbt.a -lz
	$(VALGRIND) $(VALGRIND_OPTS) ./bin/test1

bin/test2: bin test2.cpp
	$(CXX) $(CFLAGS) $(INC) -o bin/test2 test2.cpp ../lib/libmcchunk.a ../cNBT/libnbt.a -lz
	$(VALGRIND) $(VALGRIND_OPTS) ./bin/test2

clean:
	rm -rf bin/