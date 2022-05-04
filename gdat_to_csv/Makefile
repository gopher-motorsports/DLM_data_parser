CC=gcc
CFLAGS=-lm
DBFLAGS=-g

all: dlm_data_parser.c GopherCAN_names.o dlm_data_parser.h ../../gophercan-lib/GopherCAN_names.h
	$(CC) $(DBFLAGS) -o csv_converter GopherCAN_names.o dlm_data_parser.c $(CFLAGS) 

GopherCAN_names.o: ../../gophercan-lib/GopherCAN_names.c
	$(CC) $(DBFLAGS) -c ../../gophercan-lib/GopherCAN_names.c $(CFLAGS)

.PHONY: clean

clean:
	rm *.o csv_converter
