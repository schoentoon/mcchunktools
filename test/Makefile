override CFLAGS  += -Wall -O3 -mtune=native -g
CC               := gcc
INC              := -I../cNBT -I../lib
VALGRIND         := /usr/bin/valgrind
VALGRIND_OPTS    := --leak-check=full

ifeq ($(CC), gcc)
	CXX := g++
else ifeq ($(CC), clang)
	CXX := clang++
endif

.PHONY: bin/test1 bin/test_bigwrite bin/testlevel

all: bin/test1 bin/test_bigwrite bin/testlevel

bin:
	mkdir bin

bin/test1: bin test1.c
	$(CC) $(CFLAGS) $(INC) -o bin/test1 test1.c ../lib/libmcchunk.a ../cNBT/libnbt.a -lz
	$(VALGRIND) $(VALGRIND_OPTS) ./bin/test1
	rm testdata/r.10.10.mca
	$(CXX) $(CFLAGS) $(INC) -o bin/test1 test1.cpp ../lib/libmcchunk.a ../cNBT/libnbt.a -lz
	$(VALGRIND) $(VALGRIND_OPTS) ./bin/test1
	rm testdata/r.10.10.mca

bin/test_bigwrite: bin test_bigwrite.c
	$(CC) $(CFLAGS) $(INC) -o bin/test_bigwrite test_bigwrite.c ../lib/libmcchunk.a ../cNBT/libnbt.a -lz
	$(VALGRIND) $(VALGRIND_OPTS) ./bin/test_bigwrite

bin/testlevel: bin test_level.c
	$(CC) $(CFLAGS) $(INC) -o bin/testlevel test_level.c ../lib/libmcchunk.a ../cNBT/libnbt.a -lz
	$(VALGRIND) $(VALGRIND_OPTS) ./bin/testlevel

clean:
	rm -rf bin/