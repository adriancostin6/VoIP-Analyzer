#include "sip.h"

#include <fstream>

#include <algorithm>

//returns pair to be added to unordered map
std::vector<std::string> split(const std::string& s, char delimiter)
{

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);

    //get map key
    std::getline(tokenStream, token, delimiter);
    tokens.push_back(token);

    //get map value
    std::getline(tokenStream,token);
    tokens.push_back(token);

    return tokens;

}
Sip::Sip(const uint8_t* data, uint32_t size) : buffer_(data, data + size)
{
    //return if buffer is empty
    if(buffer_[0] == 32)
        return;
    std::istringstream iss(std::string(buffer_.begin(), buffer_.end()));

    std::string line;
    char del = ' ';
    std::getline(iss,line);

    //split line by spaces to see if packet is request or response
    auto res = split(line,del);

    if(res.size() == 2)
    {
        //request or response line
        if(res[0].find("SIP")!= std::string::npos)
        {
            type = Sip::RESPONSE;
            res[1].pop_back();
            header_.insert(std::make_pair(res[0],res[1]));
            h_order_.push_back(res[0]);
        }
        else
            //prevent adding empty keys if split returns empty strings
            if(res[0] != "")
            {
                type = Sip::REQUEST;

                //delete carriage return from end of line
                res[1].pop_back();
                header_.insert(std::make_pair(res[0],res[1]));
                h_order_.push_back(res[0]);
            }

        //header
        del = ':';
        while(std::getline(iss,line))
        {

            if(line == "\r")
            {
                del = '=';
                continue;
            }

            res = split(line,del);
            if(res.size()==2)
            {
                res[1].pop_back();
                header_.insert(std::make_pair(res[0],res[1]));
                h_order_.push_back(res[0]);
            }
        }
    }
}

void Sip::print() const
{
    //declare a counter for printing duplicates
    //in reverse order
    unsigned c_print = 1;

    for(auto const& key : h_order_)
    {

        int count = header_.count(key);

        //print values of non duplicate keys
        if(count == 1)
        {
            //return iterator to key value pair
            auto pair = header_.find(key);
            std::vector<std::string> duplicates;
            if(pair->first.length() == 1 )
                std::cout << pair->first << "=" << pair->second << "\n";
            else
                std::cout << pair->first << ": " << pair->second << "\n";

            continue;
        }

        //print values of duplicate keys
        if(count >1)
        {
            //duplicate values are stored in buckets, with the
            //most recently pushed values at the beginning
            //to print values in order of insertion we must
            //iterate over the bucket backwards

            //get lower and upper bound for duplicate bucket
            auto range = header_.equal_range(key);

            //goto end of range
            auto last = std::next(range.first, count-c_print);

            //print in reverse order
            std::cout << last->first << "=" << last->second << "\n";

            c_print++;

            //prevent seg fault
            if(c_print > count)
                c_print = 1;
        }
    }
}

void Sip::print(std::string path, unsigned p_num) const
{

    if(header_.size() == 0)
        return;

    path += std::to_string(p_num);
    std::ofstream of(path);

    //declare a counter for printing duplicates
    //in reverse order
    unsigned c_print = 1;

    for(auto const& key : h_order_)
    {

        int count = header_.count(key);

        //print values of non duplicate keys
        if(count == 1)
        {
            //return iterator to key value pair
            auto pair = header_.find(key);
            if(pair->first.length() == 1 )
                of<< pair->first << "=" << pair->second << "\n";
            else
                of<< pair->first << ": " << pair->second << "\n";

            continue;
        }

        //print values of duplicate keys
        if(count >1)
        {
            //duplicate values are stored in buckets, with the
            //most recently pushed values at the beginning
            //to print values in order of insertion we must
            //iterate over the bucket backwards

            //get lower and upper bound for duplicate bucket
            auto range = header_.equal_range(key);

            //goto end of range
            auto last = std::next(range.first, count-c_print);

            //print in reverse order
            of<< last->first << "=" << last->second << "\n";

            c_print++;

            //prevent seg fault
            if(c_print > count)
                c_print = 1;
        }
    }
}

std::vector<std::string> Sip::getHeader() const
{
    return h_order_;
}
