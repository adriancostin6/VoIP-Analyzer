#include "codec.h"

#include "g711.h"

#define CODEC_PCMA 8
#define CODEC_PCMU 0
#define COMFORT_NOISE 13

//constructor
G711Codec::G711Codec(){}

//get sample rate method
//
//returns: uint32_t containing sample rate
uint32_t G711Codec::get_sample_rate()
{
    return 8000;
}

//decode method 
//
//parameters:
//  - payload_type : uint8_t (RTP payload)
//  - to_decode : const string& (RTP data)
//returns: decoded string as a result
std::string G711Codec::decode(uint8_t payload_type, const std::string& to_decode)
{
    std::string res = "";

    short out;

    for(char c : to_decode)
    {
        //call specific decode function for payload type
        if(payload_type == CODEC_PCMA)
            out = alaw2linear((unsigned char)c);
        else
            out = ulaw2linear((unsigned char)c);

        //get memory address to out variable, cast it to a char pointer
        //and append it to the result string
        res.append((char*)&out, sizeof(out));
    }
    return res;
}
