#include "rtp.h"

#include <winsock.h>

Rtp::Rtp(){}

Rtp::Rtp(const uint8_t* data, uint32_t size)
{
    //store rtp header
    rtp_h_p_ = (rtp_header*) data;
    
    //convert sequence number and timestamp from network byte to host byte 
//    rtp_h_p->seq = ntohs(rtp_h_->seq);
//    rtp_h_p->timestamp = ntohl(rtp_h_->timestamp);

    //go forward in memory by sizeof header
    data += sizeof(rtp_header);

    uint16_t extension_length = 0;
    
    //if we have an extension header
    if(rtp_h_p_->x)
    {
        //go forward two bytes from end of header to reach extension length
        data += 2;

        //cast the data to a unsigned short pointer and dereference it
        //ntohs() takes a unsigned short as a parameter and converts it
        //from network byte order(big endian) to host byte order(little endian)
        extension_length = ntohs(*(uint16_t*)data);
        
        //advance by 2 bytes(extension_length) to reach the extension header
        //and another 4 bytes(32 bits) for each increase in extension length
        //example: if the length of the extension is 1 we would advance 4 bytes
        //reach the end of the full header
        data += sizeof(extension_length) + extension_length*4;

        //update extension length 
        extension_length += extension_length * 4 + 2;
    }

    data_ = std::string((const char*)data, size - sizeof(rtp_header) - extension_length);
    rtp_h_ = *rtp_h_p_;
    rtp_h_.seq = ntohs(rtp_h_p_->seq);
    rtp_h_.timestamp = ntohl(rtp_h_p_->timestamp);
}

Rtp::~Rtp()
{
}

uint8_t Rtp::get_payload_type()
{
    return rtp_h_.pt;
}
uint16_t Rtp::get_seq_no()
{
    return rtp_h_.seq;
}
std::string Rtp::get_data()
{
    return data_;
}
uint32_t Rtp::get_timestamp()
{
    return rtp_h_.timestamp;
}
