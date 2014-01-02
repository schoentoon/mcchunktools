INC             := -I../cNBT
override CFLAGS += -Wall -O3 -mtune=native -g $(INC)
CC              := gcc

OBJCOPY         := objcopy
TARGET          := $(shell ld --print-output-format)

OBJS            := regionfile.o blockdb.o chunk.o level.o
DEPS            := $(OBJS) empty_chunk.o

all: libmcchunk.a

bin:
	mkdir bin

empty_chunk.o: blob/empty.chunk.gz
	$(OBJCOPY) --input binary --output $(TARGET) --binary-architecture i386 blob/empty.chunk.gz empty_chunk.o

%.o: %.c %.h
	$(CC) $(CFLAGS) -c -o $@ $<

libmcchunk.a: $(DEPS)
	ar -rcs libmcchunk.a $(DEPS)

clean:
	rm -rfv *.a *.o