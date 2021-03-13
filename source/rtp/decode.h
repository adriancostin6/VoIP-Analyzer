#ifndef DECODE_H
#define DECODE_H

#include <list>

#include "../capture.h"
#include "rtp.h"

//structure of fmt chunk
struct wav_fmt_chunk
{
    char sub_chunk_id[4];
    uint32_t sub_chunk_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
};

//structure of data chunk 
struct wav_data_chunk
{
    char sub_chunk_id[4];
    uint32_t sub_chunk_size;
} ;

//structure of wav header
struct wav_header
{
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    wav_fmt_chunk fmt_chunk;
    wav_data_chunk data_chunk;
};

//decode function
//
//parameters: 
//  in_filename: const string& (name of input file)
//  out_filename: const string& (name of output file)
//  src_ip - string& (source ip)
//  dst_ip - string& (destination ip)
//  src_port - string& (source port)
//  dst_port - string& (destination port)
//usage: decodes RTP packets from input file and outputs a wav file
void decode(
        const std::string& in_filename,
        const std::string& out_filename,
        std::string& src_ip,
        std::string& dst_ip,
        std::string& src_port,
        std::string& dst_port
        );

#endif
