// gdat_to_ld.h
//  This file includes functions and structs to handle the gdata
//  portion of the project

#ifndef GDAT_TO_LD_H
#define GDAT_TO_LD_H

#include "types.h"
#include "ld_writing.h"
#include <stdbool.h>
#include <stdio.h>

#define MAX_FILENAME_SIZE 255
#define MAX_STR_SIZE 255

#define STARTING_NUM_POINTS 256

// gdat current parameters
#define PARAM_ID_SIZE 2
#define TIMESTAMP_SIZE 4
#define DATA_SIZE 8
#define TOTAL_SIZE (PARAM_ID_SIZE+TIMESTAMP_SIZE+DATA_SIZE)

// struct to store all of the data for a gdat data channel
typedef struct
{
    U16 gcan_id;
    U32* timestamps;
    float* data_points; // WARNING: integer precision will be lost if the number gets to large. Make this a DPF to get integer precision up to 32bits
    U32 num_data_points;
    U32 array_size; // the array size will double every time the end is reached
} GDAT_CHANNEL_t;

typedef struct GDAT_CHANNEL_LL_NODE_t GDAT_CHANNEL_LL_NODE_t;
struct GDAT_CHANNEL_LL_NODE_t
{
    GDAT_CHANNEL_t channel;
    GDAT_CHANNEL_LL_NODE_t* next;
};

typedef union
{
	double d;
	U64 u64;
} DPF_CONVERTER;


S8 build_ld_file_metadata(FILE* file, START_OF_FILE_t* sof, FILE_METADATA_t* metadat);
void cutoff_string(char* str, U32 length);
S8 import_gdat(FILE* file, GDAT_CHANNEL_LL_NODE_t* head);
S8 build_ld_data_channels(GDAT_CHANNEL_LL_NODE_t* gdat_head, CHANNEL_DESC_LL_NODE_t* ld_head,
                          bool print_chan_stats);
double convert_float_to_frac(double dlb, S16* num, S16* den);


#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#endif // GDAT_TO_LD_H

