// dlm_data_parser.c
//  TODO DOCS


// includes
#include "dlm_data_parser.h"
#include <string.h>
#include <stdint.h>


// main
//  open the file, then begin the parsing routine
int main(int argc, char* argv[])
{
    char gdat_file_name[100];
    char csv_file_name[100];
    FILE* gdat_file;
    FILE* csv_file;

    // check to make sure an argument was actually inputted
    if (argc <= 2)
    {
        printf("Incorrect arguments inputted\n");
        return INCORRECT_ARG_INPUTTED;
    }

    // set the two file names
    strcpy(gdat_file_name, argv[1]);
    strcpy(csv_file_name, argv[2]);

    // open the input and ouput files
    gdat_file = fopen(gdat_file_name, "r");
    csv_file = fopen(csv_file_name, "w");

    if (gdat_file == NULL)
    {
        printf("Failed to open file: %s\n", gdat_file_name);
        return FAILED_TO_OPEN;
    }

    if (csv_file == NULL)
    {
        printf("Failed to open file: %s\n", csv_file_name);
        return FAILED_TO_OPEN;
    }

    // do the conversion
    if (convert_gdat_to_csv(gdat_file, csv_file) != PARSER_SUCCESS)
    {
        printf("Failed conversion\n");
        return CONVERSION_FAILED;
    }

    // close the file
    fclose(gdat_file);
    fclose(csv_file);
}


// convert_gdat_to_csv
//  Takes a file pointer to a gdat file type then convert it to a CSV
//  data points are 16bits of the param ID, 32bits
int convert_gdat_to_csv(FILE* gdat, FILE* csv)
{
    unsigned char datapoint[TOTAL_SIZE] = {0};
    unsigned char metadata[METADATA_MAX_SIZE] = {0};
    uint16_t param = 0;
    uint32_t timestamp = 0;
    DPF_CONVERTER data;
    data.u64 = 0;

    // copy the file header/metadata from the gdat to the csv
    if (fgets(metadata, METADATA_MAX_SIZE, gdat) == 0)
    {
    	return BAD_METADATA;
    }

    fprintf(csv, "%s", metadata);
    fprintf(csv, "Parameter ID, Timestamp, Data\n");

    // big loop for reading the file
    while (1)
    {
        // attempt to read. If there is no more to be read, it will return 0 and leave the loop
        if (fread(datapoint, TOTAL_SIZE, sizeof(char), gdat) == 0)
        {
            return PARSER_SUCCESS;
        }

        // reset the integers
        param = 0;
        timestamp = 0;
        data.u64 = 0;

        // get the param_id
        param |= ((uint16_t)(datapoint[1]));
        param |= ((uint16_t)(datapoint[0]) << 8);

        // get the timestamp
        timestamp |= ((uint32_t)(datapoint[5]));
        timestamp |= ((uint32_t)(datapoint[4]) << (8*1));
        timestamp |= ((uint32_t)(datapoint[3]) << (8*2));
        timestamp |= ((uint32_t)(datapoint[2]) << (8*3));

        // get the datapoint
        data.u64 |= ((uint64_t)(datapoint[13]));
        data.u64 |= ((uint64_t)(datapoint[12]) << (8*1));
        data.u64 |= ((uint64_t)(datapoint[11]) << (8*2));
        data.u64 |= ((uint64_t)(datapoint[10]) << (8*3));
        data.u64 |= ((uint64_t)(datapoint[9]) << (8*4));
        data.u64 |= ((uint64_t)(datapoint[8]) << (8*5));
        data.u64 |= ((uint64_t)(datapoint[7]) << (8*6));
        data.u64 |= ((uint64_t)(datapoint[6]) << (8*7));

        // write all that data into the CSV
        fprintf(csv, "%u, %u, %f\n", param, timestamp, data.d);

        // print out what we added to the file
        //printf("%u, %u, %f\n", param, timestamp, data.d);
    }

    return PARSER_SUCCESS;
}

// End of dlm_data_parser.c
