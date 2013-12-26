CFLAGS  := -Wall -O3 -mtune=native -g
CC      := gcc
OBJCOPY := objcopy
INC     := -I../cNBT

all: libmcchunk.a

bin:
	mkdir bin

regionfile.o: regionfile.c mcchunk.h
	$(CC) $(CFLAGS) $(INC) -c -o regionfile.o regionfile.c

blockdb.o: blockdb.c mcchunk.h
	$(CC) $(CFLAGS) $(INC) -c -o blockdb.o blockdb.c

chunk.o: chunk.c mcchunk.h
	$(CC) $(CFLAGS) $(INC) -c -o chunk.o chunk.c

empty_chunk.o: blob/empty.chunk.gz
	$(OBJCOPY) --input binary --output elf64-x86-64 --binary-architecture i386 blob/empty.chunk.gz empty_chunk.o

libmcchunk.a: regionfile.o blockdb.o chunk.o empty_chunk.o
	ar -rcs libmcchunk.a regionfile.o blockdb.o chunk.o empty_chunk.o

clean:
	rm -rfv *.a *.o