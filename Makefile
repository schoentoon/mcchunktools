CFLAGS  := -Wall -O3 -mtune=native
CC      := gcc
INC     := -IcNBT -Ilib

all: bin/printchunk

bin:
	mkdir bin

bin/printchunk: bin libnbt lib/libmcchunk.a print_chunk.c
	$(CC) $(CFLAGS) $(INC) -o bin/print_chunk print_chunk.c lib/libmcchunk.a cNBT/libnbt.a -lz 

libnbt:
	$(MAKE) -C cNBT libnbt.a CC="$(CC)"

lib/libmcchunk.a:
	$(MAKE) -C lib CC="$(CC)"

clean:
	find -name \*.o -delete
	find -name \*.a -delete
	rm -rf bin