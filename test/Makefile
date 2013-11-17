CFLAGS  := -Wall -O3 -mtune=native -g
CC      := gcc
INC     := -I../cNBT -I../lib

.PHONY: bin/test1

all: bin/test1

bin:
	mkdir bin

bin/test1: bin test1.c
	$(CC) $(CFLAGS) $(INC) -o bin/test1 test1.c ../lib/libmcchunk.a ../cNBT/libnbt.a -lz
	./bin/test1

clean:
	rm -rf bin/