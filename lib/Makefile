CFLAGS  := -Wall -O3 -mtune=native
CC      := gcc
INC     := -I../cNBT

all: libmcchunk.a

bin:
	mkdir bin

regionfile.o: regionfile.c mcchunk.h
	$(CC) $(CFLAGS) $(INC) -c -o regionfile.o regionfile.c

blockdb.o: blockdb.c mcchunk.h
	$(CC) $(CFLAGS) $(INC) -c -o blockdb.o blockdb.c

libmcchunk.a: regionfile.o blockdb.o
	ar -rcs libmcchunk.a regionfile.o blockdb.o

clean:
	rm -rfv *.a *.o