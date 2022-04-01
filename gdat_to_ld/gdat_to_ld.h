// gdat_to_ld.h
//  This file includes functions and structs to handle the gdata
//  portion of the project

#ifndef GDAT_TO_LD_H
#define GDAT_TO_LD_H

#include "types.h"
#include "ld_writing.h"
#include <stdio.h>

// TODO add gopherCAN ID stuff so we can convert the numbers to the real
// gopherCAN names

#define MAX_FILENAME_SIZE 255
#define MAX_STR_SIZE 255


S8 build_ld_file_metadata(FILE* file, START_OF_FILE_t* sof, FILE_METADATA_t* metadat);
void cutoff_string(char* str, U32 length);


#endif // GDAT_TO_LD_H

