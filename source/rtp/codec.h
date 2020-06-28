#ifndef G711_CODEC_H
#define G711_CODEC_H

#include <string>
#include <cstdint>

struct G711Codec
{
    G711Codec();
    std::string decode(uint8_t payload_type, const std::string& to_decode);
    uint32_t get_sample_rate();
};

#endif
