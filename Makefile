override CFLAGS  += -Wall -O3 -mtune=native -g
CC               := gcc
INC              := -IcNBT -Ilib

all: bin/printchunk bin/analyzechunk bin/sector_counter bin/cleanup_regionfile

bin:
	mkdir bin

bin/printchunk: bin libnbt lib/libmcchunk.a print_chunk.c
	$(CC) $(CFLAGS) $(INC) -o bin/print_chunk print_chunk.c lib/libmcchunk.a cNBT/libnbt.a -lz

bin/analyzechunk: bin libnbt lib/libmcchunk.a analyze_chunk.c
	$(CC) $(CFLAGS) $(INC) -o bin/analyzechunk analyze_chunk.c lib/libmcchunk.a cNBT/libnbt.a -lz

bin/sector_counter: bin libnbt lib/libmcchunk.a sector_counter.c
	$(CC) $(CFLAGS) $(INC) -o bin/sector_counter sector_counter.c lib/libmcchunk.a cNBT/libnbt.a -lz

bin/cleanup_regionfile: bin libnbt lib/libmcchunk.a cleanup_regionfile.c
	$(CC) $(CFLAGS) $(INC) -o bin/cleanup_regionfile cleanup_regionfile.c lib/libmcchunk.a cNBT/libnbt.a -lz

libnbt:
	$(MAKE) -C cNBT libnbt.a CC="$(CC)" CFLAGS="$(CFLAGS)"

lib/libmcchunk.a:
	$(MAKE) -C lib CC="$(CC)"

clean:
	find -name \*.o -delete
	find -name \*.a -delete
	rm -rf bin

.PHONY: doc test

test: libnbt lib/libmcchunk.a
	$(MAKE) -C test CC="$(CC)"

doc:
	doxygen