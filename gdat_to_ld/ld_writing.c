// ld_writing.h
//  Functions to write data to an ld file

#include "ld_writing.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


const U8 edl_ver_string[] = EDL_VERSION_STRING;
const U8 magic_string[] = MAGIC_STRING;


// add_channel_to_list
//  Add a new channel to the list of channels. Send in all of the information for this
//  node
S8 add_channel_to_list(CHANNEL_DESC_LL_NODE_t* chan_head, U32 num_data_points, U32* buffer, U16 log_fq_Hz,
                       S16 offset, S16 scaler, S16 divisor, S16 b10_shift,
                       const char* name, const char* name_short, const char* unit)
{
    CHANNEL_DESC_LL_NODE_t* new_chan;

    // make a new LL node and put it in the list, updating the prev and next
    // pointers accordingly
    new_chan = (CHANNEL_DESC_LL_NODE_t*)malloc(sizeof(CHANNEL_DESC_LL_NODE_t));
    if (!new_chan)
    {
        printf("failed a malloc\n");
        return -1;
    }
    new_chan->next = chan_head->next;
    new_chan->prev = chan_head;
    chan_head->next = new_chan;
    if (new_chan->next) new_chan->next->prev = new_chan;

    // assign the things that can be just copied
    new_chan->channel_desc.num_data_points = num_data_points;
    new_chan->data_buffer = buffer;
    new_chan->channel_desc.logging_freq_hz = log_fq_Hz;
    new_chan->channel_desc.data_offset = offset;
    new_chan->channel_desc.data_scaler = scaler;
    new_chan->channel_desc.data_divisor = divisor;
    new_chan->channel_desc.data_base10_shift = b10_shift;

    // always do 32bit for now
    new_chan->channel_desc.data_size = s32_data;

    // for the strings, assign up to the null char or the maximum length of
    // the string, whichever comed first (strncpy)
    strncpy(new_chan->channel_desc.channel_name_str, name, sizeof(new_chan->channel_desc.channel_name_str));
    strncpy(new_chan->channel_desc.chan_name_short_str, name_short, sizeof(new_chan->channel_desc.chan_name_short_str));
    strncpy(new_chan->channel_desc.channel_unit_str, unit, sizeof(new_chan->channel_desc.channel_unit_str));

    // the unknown U16 cannot be 0
    new_chan->channel_desc.unknown = 0xAA55;
    for (int c = 0; c < sizeof(new_chan->channel_desc.zeroes); c++)
    {
        new_chan->channel_desc.zeroes[c] = 0;
    }

    return 0;
}


S8 init_sof_block(START_OF_FILE_t* sof, U16 year, U8 month, U8 day, U8 hour, U8 minute, U8 second,
                  const char* session, const char* short_comment, const char* team_name)
{
    char date_str[sizeof(sof->date) + 1];
    char time_str[sizeof(sof->time) + 1];

    // fill in the things that dont change
    sof->version_string_fptr = EDL_VER_STR_FLOC;
    sof->dash_version_string_fptr = 0x0;
    memcpy(sof->edl_version_string, edl_ver_string, sizeof(sof->edl_version_string));
    memcpy(sof->magic_str, magic_string, sizeof(sof->magic_str));

    // fill in the time and data with sprintfs. Make sure to keep the lengths correct
    sprintf(time_str, "%02d:%02d:%02d", hour, minute, second);
    sprintf(date_str, "%02d/%02d/%04d", day, month, year);
    strncpy(sof->time, time_str, sizeof(sof->time));
    strncpy(sof->date, date_str, sizeof(sof->date));

    // fill in the strings
    strncpy(sof->session_str, session, sizeof(sof->session_str));
    strncpy(sof->short_comment_str, short_comment, sizeof(sof->short_comment_str));
    strncpy(sof->team_name_str, team_name, sizeof(sof->team_name_str));

    return 0;
}


S8 init_metadata_block(FILE_METADATA_t* metadat, const char* event_name, const char* session,
                       const char* long_comment, const char* location)
{
    strncpy(metadat->event_name_str, event_name, sizeof(metadat->event_name_str));
    strncpy(metadat->session_str, session, sizeof(metadat->session_str));
    strncpy(metadat->long_comment_str, long_comment, sizeof(metadat->long_comment_str));
    strncpy(metadat->location_str, location, sizeof(metadat->location_str));

    return 0;
}


// link_id_file
//  Decide the locations of all of the different parts of the file and fill in the correct
//  fptrs. There will be lots of #defines for starts of blocks for things that can be anywhere 
S8 link_id_file(START_OF_FILE_t* sof, FILE_METADATA_t* metadat, CHANNEL_DESC_LL_NODE_t* chan_head)
{
    U32 num_channels = 0;
    U32 curr_file_loc = 0;
    CHANNEL_DESC_LL_NODE_t* chan_ptr = chan_head->next;

    // put the arbitrary things where they are #defined to be
    sof->version_string_fptr = EDL_VER_STR_FLOC;
    sof->dash_version_string_fptr = 0x00; // this does not seem to be needed
    sof->channel_ll_first_fptr = CHANNEL_DESC_START_FLOC; // there needs to be at least one channel
    sof->file_metadata_block_fptr = FILE_METADATA_FLOC;

    // the first data point will be directly after the channels
    while (chan_ptr != NULL)
    {
        num_channels++;
        chan_ptr = chan_ptr->next;
    }
    sof->data_start_fptr = CHANNEL_DESC_START_FLOC + (num_channels * CHANNEL_DESC_SIZE);

    // set the location pointer for the location string. It will be direcly after the file metadata
    // struct
    metadat->location_fptr = FILE_METADATA_FLOC + FILE_METADATA_SIZE - sizeof(metadat->location_str);

    // for each of the channels, fill in the pointers to previous and next
    chan_ptr = chan_head->next;
    curr_file_loc = CHANNEL_DESC_START_FLOC;
    while (chan_ptr != NULL)
    {
        // set this nodes location and 
        chan_ptr->location_fptr = curr_file_loc;
        curr_file_loc += CHANNEL_DESC_SIZE;

        if (chan_ptr->prev == chan_head)
        {
            chan_ptr->channel_desc.prev_fptr = 0;
        }
        else
        {
            // if the node before is not the head, set the location of this node to it and
            // set the location of the previous node to this onme
            chan_ptr->channel_desc.prev_fptr = chan_ptr->prev->location_fptr;
            chan_ptr->prev->channel_desc.next_fptr = chan_ptr->location_fptr;
        }

        // set the file location of the next node to 0, it will get fixed if the next node
        // is not null next run through the loop
        chan_ptr->channel_desc.next_fptr = 0;

        chan_ptr = chan_ptr->next;
    }

    // at the end of the channels, start putting the data buffers and add the location to the
    // channel. The curr_file_loc is now at the start of the data
    chan_ptr = chan_head->next;
    while (chan_ptr != NULL)
    {
        chan_ptr->channel_desc.data_buf_fptr = curr_file_loc;

        // move the curr_file_loc to be right after this block of data
        if (chan_ptr->channel_desc.data_size == s16_data)
        {
            // 16bit data points
            curr_file_loc += (chan_ptr->channel_desc.num_data_points * sizeof(U16));
        }
        else
        {
            // 32bit data points
            curr_file_loc += (chan_ptr->channel_desc.num_data_points * sizeof(U32));
        }

        chan_ptr = chan_ptr->next;
    }
}


// write_id_file
//  take the parts of the files and write them. Be sure to look at the fptrs in the data
//  to put everything where it belongs in the file. All values must be little endian
S8 write_id_file(START_OF_FILE_t* sof, FILE_METADATA_t* metadat, CHANNEL_DESC_LL_NODE_t* chan_head,
                 const char* filename)
{
    FILE* file;
    CHANNEL_DESC_LL_NODE_t* chan_ptr;
    U8 sof_str[START_OF_FILE_SIZE] = {0};
    U8 metadat_str[FILE_METADATA_SIZE] = {0};
    U8 chan_desc_str[CHANNEL_DESC_SIZE] = {0};

    // make the new file
    file = fopen(filename, "w");
    if (!file)
    {
        printf("failed to open file\n");
        return -1;
    }

    // if needed, convert all of the data to little endian
    // Linux with AMD: Already little endian
    // WSL and intel: Already little endian

    // fill in the byte strings for the sof and metadata
    sof_to_bytes(sof, sof_str);
    metadata_to_bytes(metadat, metadat_str);

    // write the SOF block, this will be at the start of the file
    fseek(file, 0, SEEK_SET);
    fwrite(sof_str, sizeof(sof_str), 1, file);

    // move to the location of the file metadata and write it
    fseek(file, FILE_METADATA_FLOC, SEEK_SET);
    fwrite(metadat_str, sizeof(metadat_str), 1, file);

    // for each channel, write it. Start in the correct location
    fseek(file, CHANNEL_DESC_START_FLOC, SEEK_SET);
    chan_ptr = chan_head->next;

    while(chan_ptr != NULL)
    {
        // convert to a byte string and write
        channel_desc_to_bytes(&chan_ptr->channel_desc, chan_desc_str);

        fwrite(chan_desc_str, sizeof(chan_desc_str), 1, file);
        chan_ptr = chan_ptr->next;
    }

    // for each channel, write the data buffer. This is just continuous memory
    chan_ptr = chan_head->next;
    while(chan_ptr != NULL)
    {
        if (chan_ptr->channel_desc.data_size == s16_data)
        {
            // 16bit data points
            fwrite(chan_ptr->data_buffer, chan_ptr->channel_desc.num_data_points * sizeof(U16),
                   1, file);
        }
        else
        {
            // 32bit data points
            fwrite(chan_ptr->data_buffer, chan_ptr->channel_desc.num_data_points * sizeof(U32),
                   1, file);
        }
        chan_ptr = chan_ptr->next;
    }

    fclose(file);
}


// sof_to_bytes
//  convert the SOF struct into a raw byte string for writing
void sof_to_bytes(START_OF_FILE_t* sof, U8* bytes)
{
    memcpy(bytes, &sof->version_string_fptr, sizeof(sof->version_string_fptr));
    bytes += sizeof(sof->version_string_fptr);

    memcpy(bytes, &sof->dash_version_string_fptr, sizeof(sof->dash_version_string_fptr));
    bytes += sizeof(sof->dash_version_string_fptr);

    memcpy(bytes, &sof->channel_ll_first_fptr, sizeof(sof->channel_ll_first_fptr));
    bytes += sizeof(sof->channel_ll_first_fptr);

    memcpy(bytes, &sof->data_start_fptr, sizeof(sof->data_start_fptr));
    bytes += sizeof(sof->data_start_fptr);

    memcpy(bytes, &sof->zeroes_1, sizeof(sof->zeroes_1));
    bytes += sizeof(sof->zeroes_1);

    memcpy(bytes, &sof->file_metadata_block_fptr, sizeof(sof->file_metadata_block_fptr));
    bytes += sizeof(sof->file_metadata_block_fptr);

    memcpy(bytes, &sof->zeroes_2, sizeof(sof->zeroes_2));
    bytes += sizeof(sof->zeroes_2);

    memcpy(bytes, &sof->edl_version_string, sizeof(sof->edl_version_string));
    bytes += sizeof(sof->edl_version_string);

    memcpy(bytes, &sof->date, sizeof(sof->date));
    bytes += sizeof(sof->date);

    memcpy(bytes, &sof->zeroes_3, sizeof(sof->zeroes_3));
    bytes += sizeof(sof->zeroes_3);

    memcpy(bytes, &sof->time, sizeof(sof->time));
    bytes += sizeof(sof->time);

    memcpy(bytes, &sof->zeroes_4, sizeof(sof->zeroes_4));
    bytes += sizeof(sof->zeroes_4);

    memcpy(bytes, &sof->magic_str, sizeof(sof->magic_str));
    bytes += sizeof(sof->magic_str);

    memcpy(bytes, &sof->session_str, sizeof(sof->session_str));
    bytes += sizeof(sof->session_str);

    memcpy(bytes, &sof->short_comment_str, sizeof(sof->short_comment_str));
    bytes += sizeof(sof->short_comment_str);

    memcpy(bytes, &sof->team_name_str, sizeof(sof->team_name_str));
    bytes += sizeof(sof->team_name_str);
}


// metadata_to_bytes
//  convert the metadata struct into a raw byte string for writing
void metadata_to_bytes(FILE_METADATA_t* metadat, U8* bytes)
{
    memcpy(bytes, &metadat->event_name_str, sizeof(metadat->event_name_str));
    bytes += sizeof(metadat->event_name_str);

    memcpy(bytes, &metadat->session_str, sizeof(metadat->session_str));
    bytes += sizeof(metadat->session_str);

    memcpy(bytes, &metadat->long_comment_str, sizeof(metadat->long_comment_str));
    bytes += sizeof(metadat->long_comment_str);

    memcpy(bytes, &metadat->location_fptr, sizeof(metadat->location_fptr));
    bytes += sizeof(metadat->location_fptr);

    memcpy(bytes, &metadat->location_str, sizeof(metadat->location_str));
    bytes += sizeof(metadat->location_str);
}


// channel_desc_to_bytes
//  convert the channel description into a raw byte string for writing
void channel_desc_to_bytes(CHANNEL_DESC_t* chan_desc, U8* bytes)
{
    memcpy(bytes, &chan_desc->prev_fptr, sizeof(chan_desc->prev_fptr));
    bytes += sizeof(chan_desc->prev_fptr);

    memcpy(bytes, &chan_desc->next_fptr, sizeof(chan_desc->next_fptr));
    bytes += sizeof(chan_desc->next_fptr);

    memcpy(bytes, &chan_desc->data_buf_fptr, sizeof(chan_desc->data_buf_fptr));
    bytes += sizeof(chan_desc->data_buf_fptr);

    memcpy(bytes, &chan_desc->num_data_points, sizeof(chan_desc->num_data_points));
    bytes += sizeof(chan_desc->num_data_points);

    memcpy(bytes, &chan_desc->unknown, sizeof(chan_desc->unknown));
    bytes += sizeof(chan_desc->unknown);

    memcpy(bytes, &chan_desc->data_size, sizeof(chan_desc->data_size));
    bytes += sizeof(chan_desc->data_size);

    memcpy(bytes, &chan_desc->logging_freq_hz, sizeof(chan_desc->logging_freq_hz));
    bytes += sizeof(chan_desc->logging_freq_hz);


    memcpy(bytes, &chan_desc->data_offset, sizeof(chan_desc->data_offset));
    bytes += sizeof(chan_desc->data_offset);

    memcpy(bytes, &chan_desc->data_scaler, sizeof(chan_desc->data_scaler));
    bytes += sizeof(chan_desc->data_scaler);

    memcpy(bytes, &chan_desc->data_divisor, sizeof(chan_desc->data_divisor));
    bytes += sizeof(chan_desc->data_divisor);

    memcpy(bytes, &chan_desc->data_base10_shift, sizeof(chan_desc->data_base10_shift));
    bytes += sizeof(chan_desc->data_base10_shift);


    memcpy(bytes, &chan_desc->channel_name_str, sizeof(chan_desc->channel_name_str));
    bytes += sizeof(chan_desc->channel_name_str);

    memcpy(bytes, &chan_desc->chan_name_short_str, sizeof(chan_desc->chan_name_short_str));
    bytes += sizeof(chan_desc->chan_name_short_str);

    memcpy(bytes, &chan_desc->channel_unit_str, sizeof(chan_desc->channel_unit_str));
    bytes += sizeof(chan_desc->channel_unit_str);


    memcpy(bytes, &chan_desc->zeroes, sizeof(chan_desc->zeroes));
    bytes += sizeof(chan_desc->zeroes);
}

