#ifndef SIP_H
#define SIP_H

#include<iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>
#include <utility>

//Sip class
//
//defines the structure and operations made on Sip packets
//parameters:
//  - buffer_ : vector<uint8_t> (stores raw captured data)
//  - h_order_ : vector<string> (stores keys in order)
//  - header_ : unordered_multimap<string,string> (stores header_name header_value pair)
//  - PacketType : enum (defines types of sip packets)
//operations:
//  - Sip() : constructor for raw and user entered data
//  - get_header_order() : getter for header key order
//  - get_header() : getter for header
//  - print() : used for outputting contents of sip packet to file or command line
//  - check_packet() : user for checking if packet is valid
//  - check_headers() : user for checking if header is valid
//  - check_uri() : used for checking if sip uri is valid 
//  - check_sdp() : used for checking if sdp information is valid
class Sip
{
    public:
        //Constructor for real time captured data
        Sip(const uint8_t* data, uint32_t size);

        //Constructor for text file data 
        Sip(const std::string& data);

        enum PacketType {NONE=0, REQUEST, RESPONSE}type; 
        std::vector<std::string> get_header_order() const;
        std::unordered_multimap<std::string, std::string> get_header() const;

        void print() const;
        void print(std::string path, unsigned p_num) const;

        //check if header has mandatory fields
        void check_packet(const std::string& filename,bool check);
    private:
        std::vector<uint8_t> buffer_;

        //key order for multimap 
        std::vector<std::string> h_order_;
        
        std::unordered_multimap<std::string,std::string> header_;

        //check mandatory headers 
        void check_headers(const std::string& method_name,
                const std::string& request_line, const std::string& path);

        void check_sdp(const std::string& path);

        //void check_request_uri(std::vector<std::string>& request_uri_fields, std::string& path);
        //pass param for header being checked to from and via 
        void check_uri(std::vector<std::string>& request_uri_fields,
                const std::string& path, const std::string& method,
                const std::string& header_field);
};
#endif
