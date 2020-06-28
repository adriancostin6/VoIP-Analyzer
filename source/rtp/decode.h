#ifndef DECODE_H
#define DECODE_H

#include <list>

#include "../capture.h"
#include "../rtp_decoder/codec.h"
#include "rtp.h"

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
struct wav_data_chunk
{
    char sub_chunk_id[4];
    uint32_t sub_chunk_size;
} ;

struct wav_header
{
    char chunk_id[4];
    uint32_t chunk_size;
    char format[4];
    wav_fmt_chunk fmt_chunk;
    wav_data_chunk data_chunk;
};

void decode(
        const std::string& in_filename,
        const std::string& out_filename,
        std::string& src_ip,
        std::string& dst_ip,
        std::string& src_port,
        std::string& dst_port
        );

#endif
