/*
 * @Author: yu.tongqing 
 * @Date: 2019-01-13 14:03:02 
 * @Last Modified by: yu.tongqing
 * @Last Modified time: 2019-01-13 17:10:38
 */

#ifndef _CODEC_H_
#define _CODEC_H_

#include <memory>
#include <functional>
#include <unordered_map>
#include <opus/opus.h>

#define CODEC_BIND(classname, type) codec_creator cc_##classname##_##type ((type), new classname());
#define DEFAULT_CODEC_PT 9999
#define CODEC_PT_NOISE 13
#define CODEC_PT_PCMU 0
#define CODEC_PT_PCMA 8

class codec;
extern std::unordered_map <unsigned short, std::shared_ptr<codec>> g_creator_map;

class codec_creator
{
public:
    codec_creator(unsigned short pt, codec* c)
    {
        g_creator_map[pt] = std::shared_ptr<codec>(c);
    }
};

class codec
{
public:
    virtual ~codec(){}
    virtual std::string decode(const std::string& last_packet, const std::string& packet) = 0;
    virtual uint32_t get_sample_rate() = 0;

    static std::shared_ptr<codec> get_codec_by_payload_type(unsigned short pt)
    {
        auto iter = g_creator_map.find(pt);
        if(iter == g_creator_map.end())
        {
            if(pt == CODEC_PT_NOISE)
            {
                return NULL;
            }
            auto c = g_creator_map[DEFAULT_CODEC_PT];
            c->set_payload_type(pt);
            return c;
        }else
        {
            auto c = iter->second;
            c->set_payload_type(pt);
            return c;
        }
    }

    void set_payload_type(unsigned short pt)
    {
        this->pt = pt;
    }
    unsigned short get_payload_type()
    {
        return this->pt;
    }
private:
    unsigned short pt;
};

class opus_codec : public codec
{
public:
    opus_codec();
    virtual ~opus_codec();
    std::string decode(const std::string& last_packet, const std::string& packet);
    virtual uint32_t get_sample_rate();
private:
    OpusDecoder* m_opus_decoder;
};

class g_711_codec : public codec
{
public:
    g_711_codec();
    virtual ~g_711_codec();
    std::string decode(const std::string& last_packet, const std::string& packet);
    virtual uint32_t get_sample_rate();
};

#endif