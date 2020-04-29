#ifndef SIP_H
#define SIP_H

#include<iostream>

#include <unordered_map>
//#include <iterator>
#include <unordered_set>
#include <vector>
#include <string>
#include <sstream>
#include <utility>

class Sip
{
    public:
        //Constructor for real time captured data
        Sip(const uint8_t* data, uint32_t size);

        //Constructor for text file data 
//        Sip(const std::string& data);

        //Constructor for keyboard entered data
 //       Sip(const std::vector<std::string>& data);

        enum PacketType {REQUEST, RESPONSE}type; 
        void print() const;

    private:
        std::vector<uint8_t> buffer_;

        //key order for multimap 
        std::vector<std::string> h_order_;
        
        std::unordered_multimap<std::string,std::string> header_;
};
#endif
