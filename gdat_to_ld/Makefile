CC=gcc
CFLAGS=
DBFLAGS=-g

all: gdat_to_ld.c ld_writing.o gdat_to_ld.h ld_writing.h types.h
	$(CC) $(DBFLAGS) -o ld_converter ld_writing.o gdat_to_ld.c $(CFLAGS) 

fileIO.o: ld_writing.c
	$(CC) $(DBFLAGS) -c ld_writing.c $(CFLAGS)

.PHONY: clean

clean:
	rm *.o ld_converter

	