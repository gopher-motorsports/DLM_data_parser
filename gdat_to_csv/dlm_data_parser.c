// dlm_data_parser.c
//  Program to parse .gdat files into human readable CSVs. These files
//  might be very large, so you have been warned


// includes
#include "dlm_data_parser.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../gdat_decoding.h"


// main
//  open the file, then begin the parsing routine
int main(int argc, char* argv[])
{

    char csv_file_name[100];
    char input_file[100];
    FILE* gdat_file;
    FILE* csv_file;

    // check to make sure an argument was actually inputted
    if (argc <= 1)
    {
        printf("Incorrect arguments inputted\n");
        return INCORRECT_ARG_INPUTTED;
    }

    //coppy the input from command line into the string named 
    // input_file_name
    strcpy(input_file, argv[1]);

    // coppy the file_name into string that is specific to txt file
    strcpy(csv_file_name,input_file);

    //append .csv extention to the txt file name
    strcat(csv_file_name,".csv");

    // open the input and ouput files
    gdat_file = fopen(input_file, "r");
    //using the new appended file name to open csv file
    csv_file = fopen(csv_file_name, "w");

    if (gdat_file == NULL)
    {
        printf("Failed to open file: %s\n", input_file);
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
    DATAPOINT_t datapoint;
    METADATA_t metadata;
    uint32_t bad_packets = 0;
    uint32_t total_packets = 0;

    // copy the file header/metadata from the gdat to the csv
    if (get_file_metadata(&metadata, gdat) != DECODE_SUCCESS)
    {
    	return BAD_METADATA;
    }

    fprintf(csv, "%s", metadata.meta_str);
    fprintf(csv, "Parameter ID, Timestamp, Data\n");

    // reading file loop
    while (1)
    {
        switch (convert_data_point(&datapoint, gdat))
        {
        case DECODE_SUCCESS:
            // print out the info from the packet
            fprintf(csv, "%u, %u, %f\n",
                    datapoint.gcan_id, datapoint.timestamp,
                    datapoint.data);
            total_packets++;
            break;
        
        case END_OF_FILE:
            // we are done. Give out the info and return
            printf("Conversion complete\n");
            printf("Total packets: %u\n", total_packets);
            printf("Bad packets:   %u\n", bad_packets);
            return PARSER_SUCCESS;

        default:
            // there was some issue with the packet. Note it
            // and move on
            total_packets++;
            bad_packets++;
            break;
        }
    }

    return PARSER_SUCCESS;
}



// End of dlm_data_parser.c