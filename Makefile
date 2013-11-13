CFLAGS  := -Wall -O3 -mtune=native
CC      := gcc
INC     := -IcNBT -Ilib

all: bin lib/libmcchunk.a bin/mcchunktools

bin:
	mkdir bin

main.o: main.c
	$(CC) $(CFLAGS) $(INC) -c -o main.o main.c

bin/mcchunktools: libnbt lib/libmcchunk.a main.o
	$(CC) $(CFLAGS) $(INC) -o bin/mcchunktools main.o lib/libmcchunk.a cNBT/libnbt.a -lz 

libnbt:
	$(MAKE) -C cNBT libnbt.a CC="$(CC)"

lib/libmcchunk.a:
	$(MAKE) -C lib CC="$(CC)"

clean:
	find -name \*.o -delete
	find -name \*.a -delete
	rm -rf bin