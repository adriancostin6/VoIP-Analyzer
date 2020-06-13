/*
 * @Author: yu.tongqing
 * @Date: 2019-01-13 14:40:14
 * @Last Modified by: yu.tongqing
 * @Last Modified time: 2019-01-13 17:24:45
 */

#include "codec.h"
#include "g711/g711.h"



#include<stdio.h>
#include<string>




std::unordered_map <unsigned short, std::shared_ptr<codec>> g_creator_map;
CODEC_BIND(opus_codec, DEFAULT_CODEC_PT)
CODEC_BIND(g_711_codec, CODEC_PT_PCMU)
CODEC_BIND(g_711_codec, CODEC_PT_PCMA)


//opus codec
opus_codec::opus_codec()
{
    int error;
    m_opus_decoder = opus_decoder_create(48000, 1, &error);
    if(OPUS_OK != error)
    {
        printf("create opus decoder fail: %d\n", error);
        m_opus_decoder = NULL;
    }
}
opus_codec::~opus_codec()
{
    if(m_opus_decoder)
    {
        opus_decoder_destroy(m_opus_decoder);
    }
}

std::string opus_codec::decode(const std::string& last_packet, const std::string& packet)
{
    std::string result = "";
    if(!m_opus_decoder || packet.size() == 0)
    {
        return result;
    }
    int samples = opus_packet_get_samples_per_frame((const unsigned char*)packet.c_str(), 48000);
    char *pcm = (char*)malloc(samples*2);
    int status = opus_decode(m_opus_decoder, (const unsigned char*)packet.c_str(), packet.size(), (short*)pcm, samples, 0);
    if(status > 0)
    {
        result = std::string(pcm, (size_t)(status * 2));
    }
    free(pcm);
    return result;
}

uint32_t opus_codec::get_sample_rate()
{
    return 48000;
}

//g711 codec
g_711_codec::g_711_codec()
{

}

g_711_codec::~g_711_codec()
{

}

std::string g_711_codec::decode(const std::string& last_packet, const std::string& packet)
{
    std::string result = "";
    short out;
    unsigned short pt = codec::get_payload_type();
    for (int i = 0; i < packet.size(); ++i)
    {

        if(pt == CODEC_PT_PCMA)
        {
            out = alaw2linear((unsigned char)packet[i]);
        }else
        {
            out = ulaw2linear((unsigned char)packet[i]);
        }
        result.append((char*)&out, sizeof(out));
    }
    return result;
}

uint32_t g_711_codec::get_sample_rate()
{
    return 8000;
}
