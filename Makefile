CFLAGS  := -Wall -O3 -mtune=native
CC      := gcc
INC     := -IcNBT

all: bin bin/mcchunktools

bin:
	mkdir bin

main.o: main.c
	$(CC) $(CFLAGS) $(INC) -c -o main.o main.c

bin/mcchunktools: libnbt main.o
	$(CC) $(CFLAGS) $(INC) -o bin/mcchunktools main.o cNBT/libnbt.a -lz

libnbt:
	$(MAKE) -C cNBT libnbt.a CC="$(CC)"

clean:
	find -name \*.o -delete
	rm -f cNBT/libnbt.a
	rm -rf bin