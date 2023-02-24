// gdat_decoding.h
//  File that houses all the functionality into breaking a .gdat file
//  into a struct of parameter ID, timestamp, and data in a double
//  format. Prints errors to the command line

#ifndef GDAT_DECODING_H
#define GDAT_DECODING_H

#include <stdint.h>
#include <stdio.h>

// gdat specific defines
#define PARAM_ID_SIZE 2
#define TIMESTAMP_SIZE 4
#define DATA_SIZE 8 // this is a max size, it may be less than this
#define CHECKSUM_SIZE 1
#define TOTAL_SIZE (PARAM_ID_SIZE+TIMESTAMP_SIZE+DATA_SIZE+CHECKSUM_SIZE)
#define MAX_RAW_SIZE (((TOTAL_SIZE+1)*2)) // +1 for the start byte and *2 if all of the bytes need to be escaped

// string char defines
#define PACK_START 0x7E
#define ESCAPE_CHAR 0x7D
#define ESCAPE_XOR 0x20

// other defines
#define MAX_METADATA_SIZE 255

// converting to double union
// Union for converting a double to a U64
typedef union
{
	double d;
	uint64_t u64;
} DPF_CONVERTER;

typedef union
{
	float f;
	uint32_t u32;
} FLT_CONVERTER;

typedef enum
{
    DECODE_SUCCESS = 0,
    END_OF_FILE = 1,
    BAD_PACKET_CHECKSUM = 2,
    INVALID_PACKET_SIZE = 3,
    INVALID_PARAM_ID = 4,
    INVALID_DATA_SIZE_TYPE = 5
} DECODER_ERRORS_t;


typedef struct
{
    uint32_t timestamp;
    uint16_t gcan_id;
    double data;
} DATAPOINT_t;

typedef struct
{
    char meta_str[MAX_METADATA_SIZE];
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} METADATA_t;

DECODER_ERRORS_t get_file_metadata(METADATA_t* metadata, FILE* gdat);
DECODER_ERRORS_t convert_data_point(DATAPOINT_t* datapoint, FILE* gdat);

#endif // GDAT_DECODING_H

// End of gdat_decoding.h