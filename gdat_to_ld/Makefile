CC=gcc
CFLAGS=-lm
DBFLAGS=-g

ifeq ($(OS),Windows_NT)
	RM = del
else
	RM = rm
endif

all: gdat_to_ld.c ld_writing.o GopherCAN_names.o gdat_decoding.o gdat_to_ld.h ld_writing.h types.h ../../gophercan-lib/GopherCAN_names.h ../gdat_decoding.h
	$(CC) $(DBFLAGS) -o ld_converter ld_writing.o GopherCAN_names.o gdat_decoding.o gdat_to_ld.c $(CFLAGS) 

ld_writing.o: ld_writing.c
	$(CC) $(DBFLAGS) -c ld_writing.c $(CFLAGS)

GopherCAN_names.o: ../../gophercan-lib/GopherCAN_names.c
	$(CC) $(DBFLAGS) -c ../../gophercan-lib/GopherCAN_names.c $(CFLAGS)

gdat_decoding.o: ../gdat_decoding.c
	$(CC) $(DBFLAGS) -c ../gdat_decoding.c $(CFLAGS)

.PHONY: clean

clean:
	$(RM) *.o ld_converter
