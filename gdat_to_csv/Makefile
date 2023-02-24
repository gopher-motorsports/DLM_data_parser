CC=gcc
CFLAGS=-lm
DBFLAGS=-g

all: dlm_data_parser.c GopherCAN_names.o gdat_decoding.o dlm_data_parser.h ../../gophercan-lib/GopherCAN_names.h ../gdat_decoding.h
	$(CC) $(DBFLAGS) -o csv_converter GopherCAN_names.o gdat_decoding.o dlm_data_parser.c $(CFLAGS) 

GopherCAN_names.o: ../../gophercan-lib/GopherCAN_names.c
	$(CC) $(DBFLAGS) -c ../../gophercan-lib/GopherCAN_names.c $(CFLAGS)

gdat_decoding.o: ../gdat_decoding.c
	$(CC) $(DBFLAGS) -c ../gdat_decoding.c $(CFLAGS)

.PHONY: clean

clean:
	rm *.o csv_converter
