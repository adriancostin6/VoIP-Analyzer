#include "sip.h"

#include <fstream>

#include <algorithm>
#include <regex>

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

std::vector<std::string> split_field(const std::string& s, char delimiter)
{

    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);

    while(std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);


    return tokens;

}

Sip::Sip(const std::string& data)
{
    std::istringstream iss(data);

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

//needs to be non reference string so it does not get modified over and over
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

void Sip::check_header(const std::string& filename)
{
    std::string path = " [File location:" + filename + "]";

    // check if request or response
    if(type != RESPONSE && type != NONE )
    {
        
        //check for mandatory request headers
        
        auto it = header_.find("INVITE");
        char delim = ' ';
        if(it != header_.end())
        {
            //check request line  
            auto res = split_field(it->second,delim); 
            //$3 = std::vector of length 2, capacity 2 = {"sip:1121@192.168.1.8", "SIP/2.0"}
            //res[0] is the request uri res[1] is the sip version 

            //RFC3261 Request URI structure:
            //  sip:user:password@host:port
            //Notes:
            //  -passing the password is NOT recommended
            //  -host can be IP:PORT or domain name
            //      - as seen from packet capture, the port value is not always present 
            //  -a valid uri should start with sip: or sips: or tel:
            //  -the user can also be a phone number in some cases

            //split_field request uri into uri + params 
            //where res[0] is the uri and the rest of the results are
            //the uri parameters 

            //RFC3261 - SIP version must be 2.0
            if(res[1]!="SIP/2.0")
                throw "Invalid Request Line - Bad SIP version" + path;

            delim = ';';
            auto request_uri_params = split_field(res[0],delim);

            //get uri fields 
            delim = ':';
            auto request_uri_fields = split_field(request_uri_params[0],delim);

            //Cases:
            // 1. "sip", "user@host"
            // 2. "sip", "user", "password@host"
            // 3. "sip", "user@host", "port"
            // 4. "sip", "user", "password@host", "port"
            if(request_uri_fields.size() == 2) 
            {
                //case 1
                if(request_uri_fields[0] != "sip")
                    if(request_uri_fields[0] != "sips")
                        if(request_uri_fields[0] != "tel")
                            throw "Invalid Request Line - Bad Request-URI (expected: sip or sips or tel)" + path;
                            
                delim = '@';
                auto user_host = split_field(request_uri_fields[1],delim);

                //valid username regex
                std::regex rgx(R"(^[a-zA-Z]+([_ -]?[a-zA-Z0-9])*$)");

                //if not valid username 
                if(!std::regex_match(user_host[0], rgx))
                {
                    //valid phone number regex
                    rgx = R"((([+][(]?[0-9]{1,3}[)]?)|([(]?[0-9]{4}[)]?))\s*[)]?[-\s\.]?[(]?[0-9]{1,3}[)]?([-\s\.]?[0-9]{3})([-\s\.]?[0-9]{3,4}))";

                    //if not valid  phone number
                    if(!std::regex_match(user_host[0],rgx))
                    {
                        //valid extension number regex
                        rgx = R"(^[0-9]+$)";
                        if(!std::regex_match(user_host[0], rgx))
                            throw "Invalid Request Line - Bad Request-URI\nUser (expected: username or phone number or sip extension)" + path;
                    }
                }

                //valid hostname or ip address regex
                rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                
                //if not valid host
                if(!std::regex_match(user_host[1], rgx))
                {
                    rgx = R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)";
                    if(!std::regex_match(user_host[1], rgx))
                        throw "Invalid Request Line - Bad Request-URI\nHost (expected: ip address or domain name)" + path;
                }
            }else
            if(request_uri_fields.size() == 3) 
            {
                if(request_uri_fields[0] != "sip")
                    if(request_uri_fields[0] != "sips")
                        if(request_uri_fields[0] != "tel")
                            throw "Invalid Request Line - Bad Request-URI (expected: sip or sips or tel)" + path;

                delim = '@';
                auto user_host = split_field(request_uri_fields[1],delim);

                //case 2: "sip" "user" "password@host"
                if(user_host.size() == 1)
                {
                    // valid user can be:
                    //  1. a name (ex: Bob)
                    //  2. extension id (ex: 1121)
                    //  3. any phone number format (ex: +40-745-699-085)

                    // valid username regex
                    std::regex rgx(R"(^[a-zA-Z]+([_ -]?[a-zA-Z0-9])*$)");

                    //if not valid username 
                    if(!std::regex_match(user_host[0], rgx))
                    {
                        //valid phone number regex
                        rgx = R"((([+][(]?[0-9]{1,3}[)]?)|([(]?[0-9]{4}[)]?))\s*[)]?[-\s\.]?[(]?[0-9]{1,3}[)]?([-\s\.]?[0-9]{3})([-\s\.]?[0-9]{3,4}))";

                        //if not valid  phone number
                        if(!std::regex_match(user_host[0],rgx))
                        {
                            //valid extension number regex
                            rgx = R"(^[0-9]+$)";
                            if(!std::regex_match(user_host[0], rgx))
                                throw "Invalid Request Line - Bad Request-URI\nUser (expected: username or phone number or sip extension)" + path;
                        }
                    }

                    auto password_host_field = split(request_uri_fields[2], delim); 
                    if(password_host_field.size() != 2)
                        throw "Invalid Request Line - Bad Request-URI" + path;
                    
                    //valid ip address or hostname regex
                    rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                    if(!std::regex_match(password_host_field[1], rgx))
                    {
                        rgx = R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)";
                        if(!std::regex_match(password_host_field[1], rgx))
                        throw "Invalid Request Line - Bad Request-URI\nHost (expected: ip address or domain name)" + path;
                    }

                }

                //case 3: "sip" "user@host" "port"
                if(user_host.size() == 2)
                {
                    // valid username regex
                    std::regex rgx(R"(^[a-zA-Z]+([_ -]?[a-zA-Z0-9])*$)");
                    
                    //if not valid username 
                    if(!std::regex_match(user_host[0], rgx))
                    {
                        //valid phone number regex
                        rgx = R"((([+][(]?[0-9]{1,3}[)]?)|([(]?[0-9]{4}[)]?))\s*[)]?[-\s\.]?[(]?[0-9]{1,3}[)]?([-\s\.]?[0-9]{3})([-\s\.]?[0-9]{3,4}))";

                        //if not valid  phone number
                        if(!std::regex_match(user_host[0],rgx))
                        {
                            //valid extension number regex
                            rgx = R"(^[0-9]+$)";
                            if(!std::regex_match(user_host[0], rgx))
                                throw "Invalid Request Line - Bad Request-URI\nUser (expected: username or phone number or sip extension)" + path;
                        }
                    }

                    //only check for valid IP address
                    //because we also have a port number 
                    rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                    if(!std::regex_match(user_host[1], rgx))
                        throw "Invalid Request Line - Bad Request-URI\nHost (expected: ip address or domain name)" + path;

                    //check port
                    if(request_uri_fields[2].front() == '-')
                        throw "Invalid Request Line - Bad Request-URI\nPort(invalid port number: must be positive)" + path;
                    try{
                    uint32_t port_number = std::stoi(request_uri_fields[2]);
                    if(port_number > 65535)
                        throw "Invalid Request Line - Bad Request-URI\nPort(invalid port number: must be less than 65535)" + path;
                    }catch(std::invalid_argument& e){
                        throw "Invalid Request Line - Bad Request-URI\nPort(invalid port number)" + path;
                    }
                }
            }
            else if(request_uri_fields.size() == 4) 
            {
                // case 4: "sip", "user", "password@host", "port"
                if(request_uri_fields[0] != "sip")
                    if(request_uri_fields[0] != "sips")
                        if(request_uri_fields[0] != "tel")
                            throw "Invalid Request Line - Bad Request-URI (expected: sip or sips or tel)" + path;
                
                delim = '@';
                auto user_host = split_field(request_uri_fields[1],delim);

                // valid username regex
                std::regex rgx(R"(^[a-zA-Z]+([_ -]?[a-zA-Z0-9])*$)");

                //if not valid username 
                if(!std::regex_match(user_host[0], rgx))
                {
                    //valid phone number regex
                    rgx = R"((([+][(]?[0-9]{1,3}[)]?)|([(]?[0-9]{4}[)]?))\s*[)]?[-\s\.]?[(]?[0-9]{1,3}[)]?([-\s\.]?[0-9]{3})([-\s\.]?[0-9]{3,4}))";

                    //if not valid  phone number
                    if(!std::regex_match(user_host[0],rgx))
                    {
                        //valid extension number regex
                        rgx = R"(^[0-9]+$)";
                        if(!std::regex_match(user_host[0], rgx))
                            throw "Invalid Request Line - Bad Request-URI\nUser (expected: username or phone number or sip extension)" + path;
                    }
                }

                auto password_host_field = split(request_uri_fields[2], delim); 
                if(password_host_field.size() != 2)
                    throw "Invalid Request Line - Bad Request-URI" + path;


                //only check for valid IP address
                //because we also have a port number 
                rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                if(!std::regex_match(password_host_field[1], rgx))
                    throw "Invalid Request Line - Bad Request-URI\nHost (expected: ip address or domain name)" + path;
                
                //check port
                if(request_uri_fields[3].front() == '-')
                    throw "Invalid Request Line - Bad Request-URI\nPort(invalid port number: must be positive)" + path;
                try{
                    uint32_t port_number = std::stoi(request_uri_fields[3]);
                    if(port_number > 65535)
                        throw "Invalid Request Line - Bad Request-URI\nPort(invalid port number: must be less than 65535)" + path;
                }catch(std::invalid_argument& e){
                    throw "Invalid Request Line - Bad Request-URI\nPort(invalid port number)" + path;
                }


            }else 
                throw "Invalid Request Line - Bad Request-URI" + path;
                



            //check invite mandatory headers
            it = header_.find("CSeq");
            if(it == header_.end())
                throw "Mandatory INVITE Request Header not found - CSeq" + path;
            else
            {   
                //check for leading whitespace and delete it for better parsing 
                if(it->second.front() == ' ')
                    it->second.erase(it->second.begin(), it->second.begin()+1);

                //CSeq found, check if value is correct
                delim = ' ';
                res = split(it->second, delim);

                //sequence number can't be negative 
                if(res[0].front() == '-')
                    throw "CSeq Header Field - Invalid sequence number (must be positive)" + path;
                
                try{
                    uint32_t sequence_number = std::stoi(res[0]);
                }catch(std::invalid_argument& ex){
                    throw "CSeq Header Field - Invalid sequence number" + path;
                }
            }
            if(header_.find("Call-ID") == header_.end())
                throw "Mandatory INVITE Request Header not found - Call-ID" + path;
            else
            {
            }

            //later
            if(header_.find("Contact") == header_.end())
                throw "Mandatory INVITE Request Header not found - Contact" + path;
            else
            {
                //Contact found, check if value is correct
            }

            if(header_.find("Max-Forwards") == header_.end())
                throw "Mandatory INVITE Request Header not found - Max-Forwards" + path;
            else
            {
                //Max-Forwards found, check if value is correct
            }
            if(header_.find("From") == header_.end())
                throw "Mandatory INVITE Request Header not found - From" + path;
            else
            {
                //From found, check if value is correct
            }
            if(header_.find("To") == header_.end())
                throw "Mandatory INVITE Request Header not found - To" + path;
            else
            {
                //To found, check if value is correct
            }
            if(header_.find("Via") == header_.end())
                throw "Mandatory INVITE Request Header not found - Via" + path;
            else
            {
                //Via found, check if value is correct
            }
            return;
        } 

        it = header_.find("ACK");
        if(it != header_.end())
        {
            //check ack mandatory headers
            if(header_.find("CSeq") == header_.end())
                throw "Mandatory ACK Request Header not found - CSeq" + path;
            else
            {
                //Cseq found, check if value is correct
            }
            if(header_.find("Call-ID") == header_.end())
                throw "Mandatory ACK Request Header not found - Call-ID" + path;
            else
            {
                //Call-ID found, check if value is correct
            }
            if(header_.find("Max-Forwards") == header_.end())
                throw "Mandatory ACK Request Header not found - Max-Forwards" + path;
            else
            {
                //Max-Forwards found, check if value is correct
            }
            if(header_.find("From") == header_.end())
                throw "Mandatory ACK Request Header not found - From" + path;
            else
            {
                //From found, check if value is correct
            }
            if(header_.find("To") == header_.end())
                throw "Mandatory ACK Request Header not found - To" + path;
            else
            {
                //To found, check if value is correct
            }
            if(header_.find("Via") == header_.end())
                throw "Mandatory ACK Request Header not found - Via" + path;
            else
            {
                //Via found, check if value is correct
            }
        }

        it = header_.find("BYE");
        if(it != header_.end())
        {
            //check bye  mandatory headers
            if(header_.find("CSeq") == header_.end())
                throw "Mandatory BYE Request Header not found - CSeq" + path;
            else
            {
                //Cseq found, check if value is correct
            }
            if(header_.find("Call-ID") == header_.end())
                throw "Mandatory BYE Request Header not found - Call-ID" + path;
            else
            {
                //Call-ID found, check if value is correct
            }
            if(header_.find("Max-Forwards") == header_.end())
                throw "Mandatory BYE Request Header not found - Max-Forwards" + path;
            else
            {
                //Max-Forwards found, check if value is correct
            }
            if(header_.find("From") == header_.end())
                throw "Mandatory BYE Request Header not found - From" + path;
            else
            {
                //From found, check if value is correct
            }
            if(header_.find("To") == header_.end())
                throw "Mandatory BYE Request Header not found - To" + path;
            else
            {
                //To found, check if value is correct
            }
            if(header_.find("Via") == header_.end())
                throw "Mandatory BYE Request Header not found - Via" + path;
            else
            {
                //Via found, check if value is correct
            }
        }
        
        it = header_.find("CANCEL");
        if(it != header_.end())
        {
            //check cancel  mandatory headers
            if(header_.find("CSeq") == header_.end())
                throw "Mandatory CANCEL Request Header not found - CSeq" + path;
            else
            {
                //Cseq found, check if value is correct
            }
            if(header_.find("Call-ID") == header_.end())
                throw "Mandatory CANCEL Request Header not found - Call-ID" + path;
            else
            {
                //Call-ID found, check if value is correct
            }
            if(header_.find("Max-Forwards") == header_.end())
                throw "Mandatory CANCEL Request Header not found - Max-Forwards" + path;
            else
            {
                //Max-Forwards found, check if value is correct
            }
            if(header_.find("From") == header_.end())
                throw "Mandatory CANCEL Request Header not found - From" + path;
            else
            {
                //From found, check if value is correct
            }
            if(header_.find("To") == header_.end())
                throw "Mandatory CANCEL Request Header not found - To" + path;
            else
            {
                //To found, check if value is correct
            }
            if(header_.find("Via") == header_.end())
                throw "Mandatory CANCEL Request Header not found - Via" + path;
            else
            {
                //Via found, check if value is correct
            }
        }
        
        it = header_.find("REGISTER");
        if(it != header_.end())
        {
            //check register  mandatory headers
            if(header_.find("CSeq") == header_.end())
                throw "Mandatory REGISTER Request Header not found - CSeq" + path;
            else
            {
                //Cseq found, check if value is correct
            }
            if(header_.find("Call-ID") == header_.end())
                throw "Mandatory REGISTER Request Header not found - Call-ID" + path;
            else
            {
                //Call-ID found, check if value is correct
            }
            if(header_.find("Max-Forwards") == header_.end())
                throw "Mandatory REGISTER Request Header not found - Max-Forwards" + path;
            else
            {
                //Max-Forwards found, check if value is correct
            }
            if(header_.find("From") == header_.end())
                throw "Mandatory REGISTER Request Header not found - From" + path;
            else
            {
                //From found, check if value is correct
            }
            if(header_.find("To") == header_.end())
                throw "Mandatory REGISTER Request Header not found - To" + path;
            else
            {
                //To found, check if value is correct
            }
            if(header_.find("Via") == header_.end())
                throw "Mandatory REGISTER Request Header not found - Via" + path;
            else
            {
                //Via found, check if value is correct
            }
        }
        
        it = header_.find("OPTIONS");
        if(it != header_.end())
        {
            //check options mandatory headers
            if(header_.find("CSeq") == header_.end())
                throw "Mandatory OPTIONS Request Header not found - CSeq" + path;
            else
            {
                //Cseq found, check if value is correct
            }
            if(header_.find("Call-ID") == header_.end())
                throw "Mandatory OPTIONS Request Header not found - Call-ID" + path;
            else
            {
                //Call-ID found, check if value is correct
            }
            if(header_.find("Max-Forwards") == header_.end())
                throw "Mandatory OPTIONS Request Header not found - Max-Forwards" + path;
            else
            {
                //Max-Forwards found, check if value is correct
            }
            if(header_.find("From") == header_.end())
                throw "Mandatory OPTIONS Request Header not found - From" + path;
            else
            {
                //From found, check if value is correct
            }
            if(header_.find("To") == header_.end())
                throw "Mandatory OPTIONS Request Header not found - To" + path;
            else
            {
                //To found, check if value is correct
            }
            if(header_.find("Via") == header_.end())
                throw "Mandatory OPTIONS Request Header not found - Via" + path;
            else
            {
                //Via found, check if value is correct
            }
        }

        throw "Invalid Request - Method not found" + path;
    }

















    // if packet is response dunno yet 
    
    // do searches on unordered map for all mandatory header fields
    // if(map has headers)
    //      return true 
    //  return false
}
