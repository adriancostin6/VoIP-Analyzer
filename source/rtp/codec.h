#ifndef G711_CODEC_H
#define G711_CODEC_H

#include <string>
#include <cstdint>

//G711Codec class
//
//used as a wrapper for calling g711.h functions inside the decode method
//operations: 
//  - G711Codec() : constructor
//  - decode() : used for decoding G711 ulaw and alaw data to linear pcm
//  - get_sample_rate() : getter for codec sample rate
struct G711Codec
{
    G711Codec();
    std::string decode(uint8_t payload_type, const std::string& to_decode);
    uint32_t get_sample_rate();
};

#endif
