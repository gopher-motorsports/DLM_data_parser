// dlm_data_parser.h
//  Header file for dlm_data_parser.c


#ifndef DLM_DATA_PARSER_H
#define DLM_DATA_PARSER_H

// includes
#include <stdio.h>
#include <stdint.h>


// general defines
#define MAX_FILENAME_SIZE 255
#define FILE_NAME_MAX_LEN 100

// return defines
#define PARSER_SUCCESS 0
#define INCORRECT_ARG_INPUTTED -1
#define FAILED_TO_OPEN -2
#define CONVERSION_FAILED -3
#define BAD_METADATA -4

// function prototypes
int main(int argc, char* argv[]);
int convert_gdat_to_csv(FILE* gdat, FILE* csv);

#endif


// End of dlm_data_parser.h
