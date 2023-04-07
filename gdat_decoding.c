// gdat_decoding.h
//  File that houses all the functionality into breaking a .gdat file
//  into a struct of parameter ID, timestamp, and data in a double
//  format. Prints errors to the command line

#include "gdat_decoding.h"
#include <stdio.h>
#include "../gophercan-lib/GopherCAN_names.h"

static DECODER_ERRORS_t read_data_point(DATAPOINT_t* datapoint,
                                        uint8_t* bytes, uint32_t size);


// get_file_metadata
//  takes in a gdat file and fills out the metadata from the start of the
//  file. Will read until a '\n' is found. Note: will move the file pointer
//  to right after the metadata
// params:
//  metadata: A bunch of text that stores the text, and the date and time
//  gdat: .gdat file being read from
// returns:
//  result of the operation. Different macros for success, end of file,
//  and bad metadata
DECODER_ERRORS_t get_file_metadata(METADATA_t* metadata, FILE* gdat)
{
    uint32_t date_int = 0;
    uint32_t time_int = 0;

    // get the info for decoding
    fseek(gdat, 0, SEEK_SET);
    if (fscanf(gdat, "/dlm_data_%u_%u", &date_int, &time_int) == 0)
    {
        printf("Failed to get the metadata\n");
        return END_OF_FILE;
    }

    metadata->year = date_int / 10000;
    metadata->month = (date_int / 100) % 100;
    metadata->day = date_int % 100;
    metadata->hour = time_int / 10000;
    metadata->min = (time_int / 100) % 100;
    metadata->sec = time_int % 100;

    // get the info for file reading
    fseek(gdat, 0, SEEK_SET);
    if (fgets(metadata->meta_str, MAX_METADATA_SIZE, gdat) == 0)
    {
        printf("Failed to get the metadata\n");
        return END_OF_FILE;
    }

    return DECODE_SUCCESS;
}


// convert_data_point
//  takes in the gdat file and will return the information from the next
//  datapoint in the file
// params:
//  datapoint: The timestamp, id, and data(double) of the packet
//  gdat: .gdat file being read from
//  print_errors: whether to print errors or not
// returns:
//  result of the operation. Different macros for success, end of file,
//  and bad packets
DECODER_ERRORS_t convert_data_point(DATAPOINT_t* datapoint, FILE* gdat, bool print_errors)
{
    uint8_t checksum = 0;
    uint8_t raw_size = 0;
    uint8_t processed_size = 0;
    uint8_t raw_bytes[MAX_RAW_SIZE] = {0};
    uint8_t processed_bytes[TOTAL_SIZE-CHECKSUM_SIZE] = {0};
    uint8_t temp_byte = '\0';
    DECODER_ERRORS_t result;

    // find the next available start byte. Print out if any bytes are being
    // skipped as there may be lost data
    while (1)
    {
        if (fread(&temp_byte, sizeof(uint8_t), sizeof(uint8_t), gdat) != sizeof(uint8_t))
        {
            return END_OF_FILE;
        }

        if (temp_byte == PACK_START) break;

        printf("Skipped byte: %x\n", temp_byte);
    }

    // we know the first byte is a start of packet
    raw_bytes[raw_size] = PACK_START;
    raw_size++;

    // get all of the data in the message until the next start of the
    // next start byte, the end of the file, or too many packets. Also
    // make sure to seek the file pointer back by one to make a start
    // byte the beginning of the next conversion
    while (1)
    {
        if (fread(&temp_byte, sizeof(uint8_t), sizeof(uint8_t), gdat) != sizeof(uint8_t))
        {
            // assuming this is the end of the file, not an error. Either way
            // this packet is over
            break;
        }

        if (temp_byte == PACK_START)
        {
            // this packet is over. Move back the file pointer so PACK_START is
            // the first thing that is read next packet being decoded
            fseek(gdat, -1, SEEK_CUR);
            break;
        }

        // add this byte to the array
        raw_bytes[raw_size] = temp_byte;
        raw_size++;

        if (raw_size >= MAX_RAW_SIZE)
        {
            // we are out of space for bytes. There was probably an issue
            // but we will assume all is well for now
            break;
        }
    }

    // there is a bug where two PACK_STARTs in a row really fucks shit up. Make sure
    // the size is big enough to be somewhat reasonable
    if (raw_size < (PARAM_ID_SIZE+TIMESTAMP_SIZE))
    {
        return INVALID_PACKET_SIZE;
    }

    // remove all of the escape characters from the array, creating a new
    // array that does not have a start byte or any escape characters. Start
    // at 1 to skip the start byte
    for (uint8_t c = 1; c < raw_size; c++)
    {
        if (raw_bytes[c] == PACK_START)
        {
            // not sure how this happened
            printf("SOFTWARE ERROR: Packet start in middle of packet\n");
        }

        if (raw_bytes[c] == ESCAPE_CHAR)
        {
            c++;
            processed_bytes[processed_size] = raw_bytes[c]^ESCAPE_XOR;
            processed_size++;
        }
        else
        {
            processed_bytes[processed_size] = raw_bytes[c];
            processed_size++;
        }
    }

    // calculate the checksum of all of the bytes we got. This is done on the
    // processed bytes, and includes the start byte (which is not in the processed
    // byte array)
    checksum = PACK_START;
    for (uint8_t c = 0; c < (processed_size - 1); c++)
    {
        checksum += processed_bytes[c];
    }

    // check if the checksum is good
    if (checksum != processed_bytes[processed_size - 1])
    {
        // print out the details of the bad packet
        if (print_errors)
        {
            printf("FAILED CHECKSUM\n");
            printf("  Raw Bytes:");
            for (uint8_t c = 0; c < raw_size; c++) printf(" %x", raw_bytes[c]);
            printf("\n  Processed Bytes: ");
            for (uint8_t c = 0; c < processed_size; c++) printf(" %x", processed_bytes[c]);
            printf("\n");
        }
        return BAD_PACKET_CHECKSUM;
    }

    // read the data from the packet. We can remove the checksum byte
    // at this point
    processed_size--;
    result = read_data_point(datapoint, processed_bytes, processed_size);

    if (result != DECODE_SUCCESS)
    {
        if (print_errors)
        {
            printf("BAD PACKET PASSED CHECKSUM\n");
            printf("  Raw Bytes:");
            for (uint8_t c = 0; c < raw_size; c++) printf(" %x", raw_bytes[c]);
            printf("\n  Processed Bytes: ");
            for (uint8_t c = 0; c < processed_size; c++) printf(" %x", processed_bytes[c]);
            printf("\n");
        }
    }

    return result;
}


// read_data_point
//  Takes in an array of bytes (escape chars and start byte removed) and
//  converts into a datapoint
static DECODER_ERRORS_t read_data_point(DATAPOINT_t* datapoint,
                                        uint8_t* bytes, uint32_t size)
{
    DPF_CONVERTER read_data;
    uint32_t target_size;
    uint32_t byte_pos;
    
    // reset structs used in this conversion
    datapoint->gcan_id = 0;
    datapoint->timestamp = 0;
    datapoint->data = 0;
    read_data.u64 = 0;

    // make sure the size is reasonable
    if (size <= PARAM_ID_SIZE + TIMESTAMP_SIZE) return INVALID_PACKET_SIZE;

    // read the param and timestamp as normal
    datapoint->timestamp |= ((uint32_t)(bytes[3])) & 0xff;
    datapoint->timestamp |= ((uint32_t)(bytes[2]) << (8*1)) & 0xff00;
    datapoint->timestamp |= ((uint32_t)(bytes[1]) << (8*2)) & 0xff0000;
    datapoint->timestamp |= ((uint32_t)(bytes[0]) << (8*3)) & 0xff000000;
    datapoint->gcan_id |= ((uint16_t)(bytes[5])) & 0xff;
    datapoint->gcan_id |= ((uint16_t)(bytes[4]) << 8) & 0xff00;

    // read the data into the uint64_t based on the size
    byte_pos = (size - 1);
    for (uint32_t c = 0; c < size - (PARAM_ID_SIZE + TIMESTAMP_SIZE); c++)
    {
        read_data.u64 |= (((uint64_t)(bytes[byte_pos]) & 0xff) << (8*c));
        byte_pos--;
    }

    // convert from the correct data type to a double
    if (datapoint->gcan_id >= NUM_OF_PARAMETERS) return INVALID_PARAM_ID;
    switch (param_types[datapoint->gcan_id])
    {
    case UINT_8:
        target_size = sizeof(uint8_t);
        datapoint->data = (uint8_t)read_data.u64;
        break;
    case UINT_16:
        target_size = sizeof(uint16_t);
        datapoint->data = (uint16_t)read_data.u64;
        break;
    case UINT_32:
        target_size = sizeof(uint32_t);
        datapoint->data = (uint32_t)read_data.u64;
        break;
    case UINT_64:
        target_size = sizeof(uint64_t);
        datapoint->data = (uint64_t)read_data.u64;
        break;
    case SINT_8:
        target_size = sizeof(int8_t);
        datapoint->data = (int8_t)read_data.u64;
        break;
    case SINT_16:
        target_size = sizeof(int16_t);
        datapoint->data = (int16_t)read_data.u64;
        break;
    case SINT_32:
        target_size = sizeof(int32_t);
        datapoint->data = (int32_t)read_data.u64;
        break;
    case SINT_64:
        target_size = sizeof(int64_t);
        datapoint->data = (int64_t)read_data.u64;
        break;
    case FLOAT:
        target_size = sizeof(float);
        FLT_CONVERTER flt_con;
        flt_con.u32 = read_data.u64;
        datapoint->data = flt_con.f;
        break;
    default:
        return INVALID_DATA_SIZE_TYPE;
        break;
    }

    if (size - (PARAM_ID_SIZE + TIMESTAMP_SIZE) != target_size)
    {
        return INVALID_PACKET_SIZE;
    }

    return DECODE_SUCCESS;
}

// End of gdat_decoding.c