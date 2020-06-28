#ifndef RTP_H
#define RTP_H

#include <string>

class Rtp
{
    public:
        Rtp(const uint8_t* data, uint32_t size);
        Rtp();
        ~Rtp();

        uint8_t get_payload_type();
        uint16_t get_seq_no();
        uint32_t get_timestamp();
        std::string get_data();
    private:
        struct rtp_header
        {
            uint8_t cc : 4;
            uint8_t x  : 1;
            uint8_t p  : 1;
            uint8_t v  : 2;
            uint8_t pt : 7;
            uint8_t m  : 1;
            uint16_t seq;
            uint32_t timestamp;
            uint32_t ssrc;
            uint32_t csrc[];
        };
        
        //this just points to the payload data, and as a result, when we
        //delete the data and read the next packet we will have a dangling pointer
        rtp_header* rtp_h_p_;

         
        std::string  data_;

        //this will be the actual hard copy of the header
        rtp_header rtp_h_;

};

#endif
