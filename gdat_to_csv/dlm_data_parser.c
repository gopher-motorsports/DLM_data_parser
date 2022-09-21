// dlm_data_parser.c
//  Program to parse .gdat files into human readable CSVs. These files
//  might be very large, so you have been warned


// includes
#include "dlm_data_parser.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../../gophercan-lib/GopherCAN_names.h"


// main
//  open the file, then begin the parsing routine
int main(int argc, char* argv[])
{
    char gdat_file_name[100];
    char csv_file_name[100];
    FILE* gdat_file;
    FILE* csv_file;

    // check to make sure an argument was actually inputted
    if (argc <= 1)
    {
        printf("Incorrect arguments inputted\n");
        return INCORRECT_ARG_INPUTTED;
    }

    // set the two file names
    strcpy(input_file_name, argv[1]);
    

    // open the input and ouput files
    gdat_file = fopen(input_file.gdat, "r");
    csv_file = fopen(input_file.csv, "w");

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
    char temp_char;
    bool esc_next = false;
    unsigned char byte_seq[(TOTAL_SIZE+1)*2] = {0};
    unsigned char metadata[METADATA_MAX_SIZE] = {0};
    uint32_t pack_size = 0;
    uint16_t param = 0;
    uint32_t timestamp = 0;
    double data;
    uint32_t bad_packets = 0;
    uint32_t total_packets = 0;

    // copy the file header/metadata from the gdat to the csv
    if (fgets(metadata, METADATA_MAX_SIZE, gdat) == 0)
    {
    	return BAD_METADATA;
    }

    fprintf(csv, "%s", metadata);
    fprintf(csv, "Parameter ID, Timestamp, Data\n");

    // find the first start byte
    while (1)
    {
        if (fread(&temp_char, 1, sizeof(char), gdat) == 0)
        {
            return CONVERSION_FAILED;
        }

        if (temp_char == PACK_START) break;
    }

    // big loop for reading the file
    while (1)
    {
        // attempt to read. If there is no more to be read, it will leave the loop
        if (fread(&temp_char, 1, sizeof(char), gdat) == 0) break;

        switch (temp_char)
        {
        case PACK_START:
            total_packets++;
            // convert this string into variables and add to the CSV
            if (read_data_point(byte_seq, pack_size, &timestamp, &param, &data)) 
            {
                // failed to read the last packet
                bad_packets++;
                fprintf(csv, "BAD PACKET: ");
                for (int k = 0; k < pack_size; k++)
                {
                    fprintf(csv, "%.2X ", (uint8_t)byte_seq[k]);
                }
                fprintf(csv, "\n");
                pack_size = 0;
                break;
            }
            fprintf(csv, "%u, %u, %f\n", param, timestamp, data);
            pack_size = 0;
            break;

        case ESCAPE_CHAR:
            // make sure to escape the next byte. Dont increase the size
            esc_next = true;
            break;
        
        default:
            // append this byte to the current string
            if (esc_next)
            {
                temp_char ^= ESCAPE_XOR;
                esc_next = false;
            }
            byte_seq[pack_size] = temp_char;
            pack_size++;
            break;
        }
    }

    printf("Conversion complete\n");
    printf("Total packets: %u\n", total_packets);
    printf("Bad packets:   %u\n", bad_packets);
    return PARSER_SUCCESS;
}


// takes a string and size, fills in the pointers passed in
int32_t read_data_point(char* str, uint32_t size,
                        uint32_t* timestamp, uint16_t* param, double* data)
{
    DPF_CONVERTER read_data;
    uint32_t target_size;
    // reset the integers
    *param = 0;
    *timestamp = 0;
    *data = 0;
    read_data.u64 = 0;

    // make sure the size is reasonable
    if (size <= PARAM_ID_SIZE + TIMESTAMP_SIZE) return PACKET_SIZE_ERR;

    // read the param and timestamp as normal
    *timestamp |= ((uint32_t)(str[3])) & 0xff;
    *timestamp |= ((uint32_t)(str[2]) << (8*1)) & 0xff00;
    *timestamp |= ((uint32_t)(str[1]) << (8*2)) & 0xff0000;
    *timestamp |= ((uint32_t)(str[0]) << (8*3)) & 0xff000000;
    *param |= ((uint16_t)(str[5])) & 0xff;
    *param |= ((uint16_t)(str[4]) << 8) & 0xff00;

    // read the data into the uint64_t based on the size
    uint32_t byte_pos = size - 1;
    for (uint32_t c = 0; c < size - (PARAM_ID_SIZE + TIMESTAMP_SIZE); c++)
    {
        read_data.u64 |= (((uint64_t)(str[byte_pos]) & 0xff) << (8*c));
        byte_pos--;
    }

    // convert from the correct data type to a double
    if (*param >= NUM_OF_PARAMETERS) return PARAM_OUT_OF_RANGE;
    switch (param_types[*param])
    {
    case UINT_8:
        target_size = sizeof(uint8_t);
        *data = (uint8_t)read_data.u64;
        break;
    case UINT_16:
        target_size = sizeof(uint16_t);
        *data = (uint16_t)read_data.u64;
        break;
    case UINT_32:
        target_size = sizeof(uint32_t);
        *data = (uint32_t)read_data.u64;
        break;
    case UINT_64:
        target_size = sizeof(uint64_t);
        *data = (uint64_t)read_data.u64;
        break;
    case SINT_8:
        target_size = sizeof(int8_t);
        *data = (int8_t)read_data.u64;
        break;
    case SINT_16:
        target_size = sizeof(int16_t);
        *data = (int16_t)read_data.u64;
        break;
    case SINT_32:
        target_size = sizeof(int32_t);
        *data = (int32_t)read_data.u64;
        break;
    case SINT_64:
        target_size = sizeof(int64_t);
        *data = (int64_t)read_data.u64;
        break;
    case FLOAT:
        target_size = sizeof(float);
        FLT_CONVERTER flt_con;
        flt_con.u32 = read_data.u64;
        *data = flt_con.f;
        break;
    default:
        return SIZE_NOT_FOUND;
        break;
    }

    if (size - (PARAM_ID_SIZE + TIMESTAMP_SIZE) != target_size)
    {
        return PACKET_SIZE_ERR;
    }

    return PARSER_SUCCESS;
}


// End of dlm_data_parser.c
