// dlm_data_parser.h
//  Header file for dlm_data_parser.c


#ifndef DLM_DATA_PARSER_H
#define DLM_DATA_PARSER_H

// includes
#include <stdio.h>
#include <stdint.h>

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


// general defines
#define MAX_FILENAME_SIZE 255
#define FILE_NAME_MAX_LEN 100
#define PARAM_ID_SIZE 2
#define TIMESTAMP_SIZE 4
#define DATA_SIZE 8 // this is a max size
#define TOTAL_SIZE (PARAM_ID_SIZE+TIMESTAMP_SIZE+DATA_SIZE)
#define METADATA_MAX_SIZE 255

// return defines
#define PARSER_SUCCESS 0
#define INCORRECT_ARG_INPUTTED -1
#define FAILED_TO_OPEN -2
#define CONVERSION_FAILED -3
#define BAD_METADATA -4
#define PACKET_SIZE_ERR -5
#define PARAM_OUT_OF_RANGE -6
#define SIZE_NOT_FOUND -7

// string char defines
#define PACK_START 0x7E
#define ESCAPE_CHAR 0x7D
#define ESCAPE_XOR 0x20

// function prototypes
int main(int argc, char* argv[]);
int convert_gdat_to_csv(FILE* gdat, FILE* csv);
int32_t read_data_point(char* str, uint32_t size,
                        uint32_t* timestamp, uint16_t* param, double* data);

#endif


// End of dlm_data_parser.h
