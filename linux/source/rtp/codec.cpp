#include "codec.h"

#include "g711.h"

#define CODEC_PCMA 8
#define CODEC_PCMU 0
#define COMFORT_NOISE 13


G711Codec::G711Codec(){}

uint32_t G711Codec::get_sample_rate()
{
    return 8000;
}

std::string G711Codec::decode(uint8_t payload_type, const std::string& to_decode)
{
    std::string res = "";

    short out;

    for(char c : to_decode)
    {
        if(payload_type == CODEC_PCMA)
            out = alaw2linear((unsigned char)c);
        else
            out = ulaw2linear((unsigned char)c);

    res.append((char*)&out, sizeof(out));
    }

    return res;
}
