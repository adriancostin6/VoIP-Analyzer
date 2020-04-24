#include "sip.h"

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
           header_.insert(std::make_pair(res[0],res[1]));
           h_order_.push_back(res[0]);
       } 
       else
       {
           type = Sip::REQUEST;
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
                header_.insert(std::make_pair(res[0],res[1]));
                h_order_.push_back(res[0]);
           }
        
       }
       




    }
}

void Sip::print() const
{
    for(auto const& key : header_)
    {
        if(key.lenght() == 1){
            
        }
    }
}
