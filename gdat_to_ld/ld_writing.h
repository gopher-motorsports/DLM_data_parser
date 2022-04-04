// ld_writing.h
//  functions to output data stored in buffers to ld file type


#ifndef LD_WRITING_H
#define LD_WRITING_H


#include "types.h"


// important defines for the ld file
#define EDL_VER_STR_FLOC 0x40
#define EDL_VERSION_STRING {0x00, 0x00, 0x40, 0x42, 0x0F, 0x00, 0x7B, 0x52, 0x00, 0x00, 0x41, 0x44, 0x4C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x02, 0x80, 0x00, 0x02, 0x00, 0x00, 0x00, 0x14, 0x00, 0x05, 0x00}
#define MAGIC_STR_FLOC 0x5DE
#define MAGIC_STRING {0x01, 0x92, 0xB0, 0x02, 0x00, 0x00}

#define STR_LEN_LONG 0x400
#define STR_LEN_MED 0x40
#define STR_LEN_SHORT 0x20
#define STR_LEN_TINY 0x08

// always start the file metadata at the same point. It is inefficient, but still better than Motec
#define FILE_METADATA_FLOC 0x1000 // ends at 0x14C4
#define CHANNEL_DESC_START_FLOC 0x3000

typedef enum
{
    // these are reverse-endianed from how it appears in the file
    s16_data = 0x00030002,
    s32_data = 0x00040005,
} DATAPOINT_SIZE_t;

// structs for the different blocks of ld data

// Start of file block: This should be 0x6B4 bytes long, but also includes a
// in-file pointer to itself that is hard coded in and a lot of padding zeros to
// fill in the magic string at the specific location
#define START_OF_FILE_SIZE 0x06B4
typedef struct
{
    U32 version_string_fptr;        // this will always be 0x40 as the version string is hard coded
    U32 dash_version_string_fptr;   // points to the name of the EDL config name and version. Not needed and the data opens fine with this fixed to 0x0000
    U32 channel_ll_first_fptr;      // file pointer to the first node of the DLL that stores all of the channels and their details
    U32 data_start_fptr;            // pointer to the first byte of data. Not sure if this is really needed because each channel node also has a data start pointer, but we will fill it anyway
    U8  zeroes_1[0x14];             // probably pointers to more log file metadata, but we will just zero it for now
    U32 file_metadata_block_fptr;   // pointer to block that has all of the metadata. Note this pointer points to the start of the event name, which should be the first thing in that block
    U8  zeroes_2[0x18];             // probably more pointers to detail strings
    U8  edl_version_string[0x1e];   // should be at file location 0x40, this needs to be 0x1e bytes long and hold a very specific string.
    U8  date[0x0a];                 // Must be in format DD/MM/YYYY to show up in the file
    U8  zeroes_3[0x16];             // probably more pointers to detail strings
    U8  time[0x08];                 // Must be in format HH:MM:SS to show up in the file. Hour ranges from 0-23
    U8  zeroes_4[0x558];            // this is pretty much dead space. Venue shows up again at 0x15E but I dont think that is used by this version of i2

    U8  magic_str[0x06];            // 6bytes that seem to need to be correct in order for the file to open. Should be at location 0x5DE
    U8  session_str[0x40];          // stores the session string. The extra can just be zeros. I dont know if this is what is used or the session later on in the next block
    U8  short_comment_str[0x40];    // stores the short comment string. The extra can just be zeros
    U8  team_name_str[0x20];        // stores the team name string. The extra can just be zeros
    // not sure how long team_name_str can be
} START_OF_FILE_t;


// Metadata block: This stores the event name, session, long comment, and location. Long
// comment is really long so thats why it is another block I think. Also the location pointer
// is at the end of long comment for some reason, but it can just point to a chunk of data
// right after the pointer. Not sure why they didnt just put the location after the long comment
#define FILE_METADATA_SIZE 0x4C4
typedef struct
{
    U8  event_name_str[0x40];       // event name string
    U8  session_str[0x40];          // session name string
    U8  long_comment_str[0x400];    // long comment string
    U32 location_fptr;              // pointer to the location string. We will just put it right after this pointer, but we still have to link it? maybe put this block at a fixed location
    U8  location_str[0x40];         // location string. This might not be 0x40 long but I am too lazy to test
} FILE_METADATA_t;



// Channel descriptors. This is a Doubly-linked-list of each channel, its name, pointer to
// its buffer, and other important things. This is pretty solved as far as I can tell, but
// there are a few parts that dont seem to effect anything
#define CHANNEL_DESC_SIZE 0x7C
typedef struct
{
    U32 prev_fptr;                  // pointer to the node before. The first node will fill this pointer with 0x0000
    U32 next_fptr;                  // pointer to the next node in the DLL. The last node will fill this pointer with 0x0000
    U32 data_buf_fptr;              // pointer to the first element of the data buffer
    U32 num_data_points;            // number of data points logged in this channel
    U16 unknown;                    // this value seems to be random for each channel
    U32 data_size;                  // not super sure how this works, but I do know that 0x05000400 means read as a s32 and 0x02000300 means read as s16
    U16 logging_freq_hz;            // frequency of logging. I guess that means 1hz is the minimum

    // TODO order of operation on this
    S16 data_offset;                // this s16 is added to the data for each point
    S16 data_scaler;                // this s16 is multiplied to each data point
    S16 data_divisor;               // each data point is divided by this value
    S16 data_base10_shift;          // each data point is multiplied by 10^-[value]. This is signed so it can move the decimal either way

    // names
    U8 channel_name_str[0x20];      // name string
    U8 chan_name_short_str[0x08];   // short channel name for some displays
    U8 channel_unit_str[0x08];      // name of the unit for this channel. Some units are recognized by i2 and then it can do unit conversions for us
    // TODO get a complete list of compatable unit names, probably from the EDL

    U8 zeroes[0x2C];                // This only works when some of it is zero and I dont know what it does, so just fill it all with zero
} CHANNEL_DESC_t;


// there will be a DLL of each channel in programm memory before it is put into the .Id file
typedef struct CHANNEL_DESC_LL_NODE_t CHANNEL_DESC_LL_NODE_t;
struct CHANNEL_DESC_LL_NODE_t
{
    CHANNEL_DESC_t channel_desc;
    U32* data_buffer;               // buffer to where the data is stored. Planning to only support s32 encoding for now
    U32 location_fptr;              // used during linking
    CHANNEL_DESC_LL_NODE_t* prev;
    CHANNEL_DESC_LL_NODE_t* next;
};




// function prototypes

S8 add_channel_to_list(CHANNEL_DESC_LL_NODE_t* chan_head, U32 num_data_points, U32* buffer, U16 log_fq_Hz,
                       S16 offset, S16 scaler, S16 divisor, S16 b10_shift,
                       const char* name, const char* name_short, const char* unit);
S8 init_sof_block(START_OF_FILE_t* sof, U16 year, U8 month, U8 day, U8 hour, U8 minute, U8 second,
                  const char* session, const char* short_comment, const char* team_name);
S8 init_metadata_block(FILE_METADATA_t* metadat, const char* event_name, const char* session,
                       const char* long_comment, const char* location);
S8 link_id_file(START_OF_FILE_t* sof, FILE_METADATA_t* metadat, CHANNEL_DESC_LL_NODE_t* chan_head);
S8 write_id_file(START_OF_FILE_t* sof, FILE_METADATA_t* metadat, CHANNEL_DESC_LL_NODE_t* chan_head,
                 const char* filename);
void sof_to_bytes(START_OF_FILE_t* sof, U8* bytes);
void metadata_to_bytes(FILE_METADATA_t* metadat, U8* bytes);
void channel_desc_to_bytes(CHANNEL_DESC_t* chan_desc, U8* bytes);


#endif // LD_WRITING_H
