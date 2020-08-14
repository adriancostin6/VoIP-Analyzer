#include "sip.h"

#include <fstream>

#include <algorithm>
#include <regex>

//split function
//
//parameters:
//  - s: const string& (line to be split)
//  - delimiter : char (delimiter used in splitting)
//returns pair to be added to unordered map
//used in constructor
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

//split_filed function
//
//parameters:
//  - s: const string& (line to be split)
//  - delimiter : char (delimiter used in splitting)
//returns: vector of substrings as a result of the split operation
//used outside the constructor to split header fields
std::vector<std::string> split_field(const std::string& s, char delimiter)
{
    //make vector of tokens to store results
    std::vector<std::string> tokens;

    //initialize token and token stream from 
    std::string token;
    std::istringstream tokenStream(s);

    ///use getline to split the line by the delimiter and push back the tokens
    while(std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);

    return tokens;
}

//constructor
//
//parameters:
//  data : const string& (payload of the packet)
//
//usage: constructs a Sip object from payload data created by user and passed
//as a string&
Sip::Sip(const std::string& data)
{
    //construct input string stream from data
    std::istringstream iss(data);

    //init temporary string for line processing and delimiter
    std::string line;
    char del = ' ';
    //get first line from payload
    std::getline(iss,line);

    //when reading line from file, if it does not end in a carriage return
    //add so it does not pop off random characters at the end of the data
    //value (all sip lines end in a carriage return line feed)
    if(line.back() != '\r')
        line += '\r';

    //split line by spaces to see if packet is request or response
    auto res = split(line,del);

    if(res.size() == 2)
    {
        //request or response line
        if(res[0].find("SIP")!= std::string::npos)
        {
            type = Sip::RESPONSE;
            //delete carriage return from end of line
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

            //if the line does not end in CRLF add it to avoid errors
            if(line.back() != '\r')
                line += '\r';

            //if line is the CRLF encountered after the end of the SIP packet
            //change delimiter and prepare to parse SDP data
            if(line == "\r")
            {
                del = '=';
                continue;
            }

            res = split(line,del);
            //add to header only if the split returned two elements
            //which is the basic Sip syntax header: "value"
            if(res.size()==2)
            {
                res[1].pop_back();
                header_.insert(std::make_pair(res[0],res[1]));
                h_order_.push_back(res[0]);
            }
        }
    }
}

//constructor
//
//parameters:
//  - data : uint8_t* (raw pointer to a block of data)
//  - size : uint32_t (size of the data being pointed to)
//
//usage: copy data from the block being pointed to into the buffer_ member
Sip::Sip(const uint8_t* data, uint32_t size) : buffer_(data, data + size)
{
    //return if buffer is empty
    if(buffer_[0] == 32)
        return;

    //make a input string stream frum the buffer_
    std::istringstream iss(std::string(buffer_.begin(), buffer_.end()));

    //init line and delimiter
    std::string line;
    char del = ' ';

    //get first line from input stream
    std::getline(iss,line);

    //split line by spaces to see if packet is request or response
    auto res = split(line,del);

    if(res.size() == 2)
    {
        //request or response line
        if(res[0].find("SIP")!= std::string::npos)
        {
            type = Sip::RESPONSE;
            //delete carriage return from end of line
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

            //if end of Sip packet, prepare to parse SDP
            if(line == "\r")
            {
                del = '=';
                continue;
            }

            res = split(line,del);
            if(res.size()==2)
            {
                //remove carriage return from the end of the line 
                res[1].pop_back();
                header_.insert(std::make_pair(res[0],res[1]));
                h_order_.push_back(res[0]);
            }
        }
    }
}

//print method 
//
//usage : print value of sip header to the command line
void Sip::print() const
{
    auto header = header_;

    for(auto const& key : h_order_)
    {
        //get duplicate count
        int count = header_.count(key);

        //print value of non duplicates
        if(count == 1)
        {
            //return iterator to key value pair
            auto pair = header_.find(key);
            //if we have an SDP header
            if(pair->first.length() == 1 )
                std::cout<< pair->first << "=" << pair->second << "\n";
            //if any of the characters in the key is lowercase and size
            //is greater than 1 we have a regular header
            else if(
                    std::any_of(
                        pair->first.begin(),
                        pair->first.end(),
                        []( char c)
                        {
                        return islower(c);
                        }
                        )
                   )
                std::cout<< pair->first << ":" << pair->second << "\n";
            else
                //if none of the above we have a request/response line 
                std::cout<< pair->first << " " << pair->second << "\n";

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
            auto range = header.equal_range(key);

            if(range.first->first.length() == 1 )
                std::cout << range.first->first << "=" << range.first->second << "\r\n";
            else
                std::cout << range.first->first << ":" << range.first->second << "\r\n";

            header.erase(range.first);
        }
    }
}

//needs to be non reference string so it does not get modified over and over
void Sip::print(std::string path, unsigned p_num) const
{
    //return if header is empty
    if(header_.size() == 0)
        return;

    auto header = header_;

    //add number to file path and open file
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
            //if length of key is 1 print SDP
            if(pair->first.length() == 1 )
                of<< pair->first << "=" << pair->second << "\r\n";
            //if any of the characters in the key is lowercase
            //and the length of the key is greater than 1 we have a 
            //regular header
            else if(
                    std::any_of(
                        pair->first.begin(),
                        pair->first.end(),
                        []( char c)
                        {
                        return islower(c);
                        }
                        )
                   )
                of<< pair->first << ":" << pair->second << "\r\n";
            else
                //if none of the above we have a request/response line
                of<< pair->first << " " << pair->second << "\r\n";

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
            auto range = header.equal_range(key);

            if(range.first->first.length() == 1 )
                of<< range.first->first << "=" << range.first->second << "\r\n";
            else
                of<< range.first->first << ":" << range.first->second << "\r\n";

            header.erase(range.first);
        }
    }
}

//check_header method
//
//parameters: 
//  - filename : const string& (name of file where error happens, if it throws)
//  - check : bool (turn checking headers on or off)
//usage: check stored packets for errors and throw if they are not valid 
//as specified in RFC3261
void Sip::check_packet(const std::string& filename,bool check)
{
    //if checker is turned off exit
    if(!check)
        return;

    //init file path
    std::string path = " [File location:" + filename + "]";

    // check if request or response
    if(type != RESPONSE && type != NONE )
    {
        //iterator for searching map elements
        std::unordered_multimap<
            std::string,
            std::string>::iterator it;

        //check for mandatory request headers
        
        if((it = header_.find("INVITE")) != header_.end())
        {
            check_headers(it->first, it->second, path);
        } 
        else
        if((it = header_.find("ACK")) != header_.end())
        {
            check_headers(it->first,it->second,path);
        }
        else
        if((it = header_.find("BYE")) != header_.end())
        {
            check_headers(it->first,it->second,path);
        }
        else
        if((it = header_.find("CANCEL")) != header_.end())
        {
            check_headers(it->first,it->second,path);
        }
        else
        if((it = header_.find("REGISTER")) != header_.end())
        {
            check_headers(it->first,it->second,path);
        }
        else
        if((it = header_.find("OPTIONS")) != header_.end())
        {
            check_headers(it->first,it->second,path);
        }
        else
        throw "Invalid Request - Method not found" + path;
    }
}

//check_headers method
//
//parameters:
//  -method_name : const string& (name of method)
//  -request_line : const string& (name of the request line)
//  -path : const string& (path of file where error is thrown)
void Sip::check_headers(const std::string& method_name,
        const std::string& request_line, const std::string& path)
{
    char delim = ' ';

    //check request line  
    auto res = split_field(request_line,delim); 

    ///////////////////////////////////////////////////////////////////
    //VALIDATE REQUEST LINE
    ///////////////////////////////////////////////////////////////////

    //split_field request uri into uri + params 
    //where res[0] is the uri and the rest of the results are params
    //RFC3261 - SIP version must be 2.0
    if(res[1]!="SIP/2.0")
        throw "Invalid Request Line - Bad SIP version" + path;

    //split uri field into uri + parameters
    delim = ';';
    auto request_uri_params = split_field(res[0],delim);

    //get uri components 
    delim = ':';
    auto request_uri_fields = split_field(request_uri_params[0],delim);

    //check the sip uri
    check_uri(request_uri_fields, path, method_name, "Request-Line");

    ///////////////////////////////////////////////////////////////////
    //END REQUEST LINE VALIDATION
    ///////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////
    //VALIDATE MANDATORY HEADERS
    ///////////////////////////////////////////////////////////////////
    auto it = header_.find("CSeq");
    if(it == header_.end())
        throw "Mandatory INVITE Request Header not found - CSeq" + path;
    else
    {   
        //CSeq found, check if value is correct

        //copy the value so we do not alter it 
        std::string temp = it->second;

        //check for leading whitespace and delete it for better parsing 
        //RFC3261 - field-name: field-value
        if(temp.front() == ' ')
            temp.erase(temp.begin(), temp.begin()+1);

        //split header by spaces
        delim = ' ';
        res = split(temp, delim);

        //sequence number can't be negative 
        if(res[0].front() == '-')
            throw "Header Field - CSeq\nInvalid sequence number (must be positive)" + path;

        try{
            uint32_t sequence_number = std::stoi(res[0]);
        }catch(std::invalid_argument& ex){
            throw "Header Field - CSeq\nInvalid sequence number" + path;
        }

        //method has to match the request line
        if(res[1] != method_name)
            throw "Header Field - CSeq\nMethod does not match request line" + path;
    }

    //call id can be anything, but must be unique
    //we don't need to validate it, only see if it exists
    if(header_.find("Call-ID") == header_.end())
        throw "Mandatory INVITE Request Header not found - Call-ID" + path;

    //if it is an invite request check for presence of contact method
    it = header_.find("INVITE");
    if(it != header_.end())
    {
        it = header_.find("Contact"); 
        if(it == header_.end())
            throw "Mandatory INVITE Request Header not found - Contact" + path;
        else
        {
            delim = ' ';
            std::string temp = it->second;

            //check for leading whitespace and delete it for better parsing 
            //RFC3261 - field-name: field-value
            if(temp.front() == ' ')
                temp.erase(temp.begin(), temp.begin()+1);

            //split contact by whitespaces
            auto res = split_field(temp, delim); 

            if(res.size() > 3 )
                throw "Header Field - Contact\nToo large (expected: single line header of format Contact: \"Display name\" <uri;params>;params)" + path;

            //process contact header value
            if(res.size() == 3 )
            {
                // uri ; params -> syntax for delimiting results 
                // uri;params -> syntax when talking about uri or header parameters
                //case 1: "dis ; play" ; <uri;params>;params
                //case 2: "dis ; play" ; <uri>;params
                //case 3: "dis ; play" ; <uri;params>
                //
                
                //validate display name
                if(res[0].front() != '"') 
                    throw "Header Field - Contact\nInvalid display name (expected: \"Display Name\")" + path;
                if(res[1].back() != '"')
                    throw "Header Field - Contact\nInvalid display name (expected: \"Display Name\")" + path;

                //split second result into uri + params + header params 
                delim = ';';
                auto request_uri_params = split_field(res[2], delim);

                //case 1: <uri ; params> ; params
                //case 2: <uri ; params>
                //case 3: <uri> ; params
                if(request_uri_params[0].front() == '<')
                {
                    //case 1 and 2 
                    
                    //erase front and back <>
                    if(request_uri_params[0].back() != '>')
                    {
                        request_uri_params[0].erase(
                                request_uri_params[0].begin(),
                                request_uri_params[0].begin()+1
                                );

                        //split uri into components
                        delim = ':';
                        auto request_uri_fields = split_field(request_uri_params[0], delim);

                        check_uri(request_uri_fields, path, method_name, "Contact");
                    }
                    //case 3 : <uri> ; params
                    else if(request_uri_params[0].back() == '>')
                    {
                        //erase front and back <> 
                        request_uri_params[0].erase(
                                request_uri_params[0].begin(),
                                request_uri_params[0].begin()+1
                                );
                        request_uri_params[0].pop_back();

                        //split uri into components
                        delim = ':';
                        auto request_uri_fields = split_field(request_uri_params[0], delim);

                        check_uri(request_uri_fields, path, method_name, "Contact");
                    }
                    else
                        throw "Header Field - Contact\nInvalid URI (expected: should be enclosed in <>)" + path;
                }
                else
                    throw "Header Field - Contact\nInvalid URI (expected: should be enclosed in <>)" + path;
            }
            else if (res.size() == 2 )
            {
                //case 1: "display" <uri;params>;params
                //case 2: "display" <uri>;params
                //case 3: "display" <uri;params>
                if(res[0].front() != '"') 
                    throw "Header Field - Contact\nInvalid display name (expected: \"Display Name\")" + path;


                if(res[0].back() != '"')
                    throw "Header Field - Contact\nInvalid display name (expected: \"Display Name\")" + path;

                //split second result into uri + params + header params 
                delim = ';';
                auto request_uri_params = split_field(res[1], delim);

                //case 1: <uri ; params> ; params
                //case 2: <uri ; params>
                //case 3: <uri> ; params
                if(request_uri_params[0].front() == '<')
                {
                    //case 1 and 2 
                    if(request_uri_params[0].back() != '>')
                    {
                        request_uri_params[0].erase(
                                request_uri_params[0].begin(),
                                request_uri_params[0].begin()+1
                                );

                        delim = ':';
                        auto request_uri_fields = split_field(request_uri_params[0], delim);

                        check_uri(request_uri_fields, path, method_name, "Contact");
                    }
                    //<uri> ; params
                    else if(request_uri_params[0].back() == '>')
                    {
                        request_uri_params[0].erase(
                                request_uri_params[0].begin(),
                                request_uri_params[0].begin()+1
                                );
                        request_uri_params[0].pop_back();

                        delim = ':';
                        auto request_uri_fields = split_field(request_uri_params[0], delim);

                        check_uri(request_uri_fields, path, method_name, "Contact");
                    }
                    else
                        throw "Header Field - Contact\nInvalid URI (expected: should be enclosed in <>)" + path;
                }
                else
                    throw "Header Field - Contact\nInvalid URI (expected: should be enclosed in <>)" + path;

            }
            else if (res.size() == 1)
            {
                //split second result into uri + params + header params 
                delim = ';';
                auto request_uri_params = split_field(res[0], delim);

                //case 1: <uri ; params> ; params
                //case 2: <uri ; params>
                if(request_uri_params[0].front() == '<')
                {
                    if(request_uri_params[0].back() != '>')
                    {
                        request_uri_params[0].erase(
                                request_uri_params[0].begin(),
                                request_uri_params[0].begin()+1
                                );

                        delim = ':';
                        auto request_uri_fields = split_field(request_uri_params[0], delim);

                        check_uri(request_uri_fields, path, method_name, "Contact");
                    }
                    //<uri> ; params
                    else if(request_uri_params[0].back() == '>')
                    {
                        request_uri_params[0].erase(
                                request_uri_params[0].begin(),
                                request_uri_params[0].begin()+1
                                );
                        request_uri_params[0].pop_back();

                        delim = ':';
                        auto request_uri_fields = split_field(request_uri_params[0], delim);

                        check_uri(request_uri_fields, path, method_name, "Contact");
                    }
                    else
                        throw "Header Field - Contact\nInvalid URI (expected: should be enclosed in <>)" + path;
                }
                else
                    throw "Header Field - Contact\nInvalid URI (expected: should be enclosed in <>)" + path;
            }
            else 
                throw "Header Field - Contact\nInvalid URI (expected: \"Display Name\" <uri;params>;params)" + path;
        }
    }

    it = header_.find("Max-Forwards"); 
    if(it == header_.end())
        throw "Mandatory INVITE Request Header not found - Max-Forwards" + path;
    else
    {
        //Max-Forwards found, check if value is correct

        std::string temp = it->second;

        //check for leading whitespace and delete it for better parsing
        //RFC3261 - field-name: field-value
        if(temp.front() == ' ')
            temp.erase(temp.begin(), temp.begin()+1);

        //A UAC MUST insert a Max-Forwards header field into each 
        //request it originates with a value that SHOULD be 70.
        //Lower values should be used with caution and only in networks
        //where topologies are known by the UA.

        //we check the base use case for validation, when it is 70
        if(temp.front() == '-')
            throw "Header Field - Max-Forwards\nInvalid value (must be positive)" + path;
        try{
            uint8_t max_forwards = std::stoi(temp);
            if(max_forwards != 70)
                throw "Header Field - Max-Forwards\nInvalid value (should be 70)" + path;
        }catch(std::invalid_argument& ex){
            throw "Header Field - Max-Forwards\nInvalid value (must be a number)" + path;
        }
    }

    it = header_.find("From");
    if(it == header_.end())
        throw "Mandatory INVITE Request Header not found - From" + path;
    else
    {
        delim = ' ';
        std::string temp = it->second;

        //check for leading whitespace and delete it for better parsing 
        //RFC3261 - field-name: field-value
        if(temp.front() == ' ')
            temp.erase(temp.begin(), temp.begin()+1);

        auto res = split_field(temp, delim); 

        if(res.size() > 3 )
            throw "Header Field - From\nToo large (expected: single line header of format From: \"Display name\" <uri;params>;params)" + path;

        if(res.size() == 3 )
        {
            // uri ; params -> syntax for delimiting results 
            // uri;params -> syntax when talking about uri or header parameters
            //case 1: "dis ; play" ; <uri;params>;params
            //case 2: "dis ; play" ; <uri>;params
            //case 3: "dis ; play" ; <uri;params>
            //
            //validate display name
            if(res[0].front() != '"') 
                throw "Header Field - From\nInvalid display name (expected: \"Display Name\")" + path;


            if(res[1].back() != '"')
                throw "Header Field - From\nInvalid display name (expected: \"Display Name\")" + path;

            //split second result into uri + params + header params 
            delim = ';';
            auto request_uri_params = split_field(res[2], delim);

            //case 1: <uri ; params> ; params
            //case 2: <uri ; params>
            //case 3: <uri> ; params
            if(request_uri_params[0].front() == '<')
            {
                //case 1 and 2 
                if(request_uri_params[0].back() != '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "From");
                }
                //case 3 : <uri> ; params
                else if(request_uri_params[0].back() == '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );
                    request_uri_params[0].pop_back();

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "From");
                }
                else
                    throw "Header Field - From\nInvalid URI (expected: should be enclosed in <>)" + path;
            }
            else
                throw "Header Field - From\nInvalid URI (expected: should be enclosed in <>)" + path;
        }
        else if (res.size() == 2 )
        {
            //case 1: "display" <uri;params>;params
            //case 2: "display" <uri>;params
            //case 3: "display" <uri;params>
            if(res[0].front() != '"') 
                throw "Header Field - From\nInvalid display name (expected: \"Display Name\")" + path;


            if(res[0].back() != '"')
                throw "Header Field - From\nInvalid display name (expected: \"Display Name\")" + path;

            //split second result into uri + params + header params 
            delim = ';';
            auto request_uri_params = split_field(res[1], delim);

            //case 1: <uri ; params> ; params
            //case 2: <uri ; params>
            //case 3: <uri> ; params
            if(request_uri_params[0].front() == '<')
            {
                //case 1 and 2 
                if(request_uri_params[0].back() != '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "From");
                }
                //<uri> ; params
                else if(request_uri_params[0].back() == '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );
                    request_uri_params[0].pop_back();

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "From");
                }
                else
                    throw "Header Field - From\nInvalid URI (expected: should be enclosed in <>)" + path;
            }
            else
                throw "Header Field - From\nInvalid URI (expected: should be enclosed in <>)" + path;

        }
        else if (res.size() == 1)
        {
            //split second result into uri + params + header params 
            delim = ';';
            auto request_uri_params = split_field(res[0], delim);

            //case 1: <uri ; params> ; params
            //case 2: <uri ; params>
            if(request_uri_params[0].front() == '<')
            {
                if(request_uri_params[0].back() != '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "From");
                }
                //<uri> ; params
                else if(request_uri_params[0].back() == '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );
                    request_uri_params[0].pop_back();

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "From");
                }
                else
                    throw "Header Field - From\nInvalid URI (expected: should be enclosed in <>)" + path;
            }
            else
                throw "Header Field - From\nInvalid URI (expected: should be enclosed in <>)" + path;
        }
        else 
            throw "Header Field - From\nInvalid URI (expected: \"Display Name\" <uri;params>;params)" + path;

    }

    it = header_.find("To");
    if(it == header_.end())
        throw "Mandatory INVITE Request Header not found - To" + path;
    else
    {
        delim = ' ';
        std::string temp = it->second;

        //check for leading whitespace and delete it for better parsing 
        //RFC3261 - field-name: field-value
        if(temp.front() == ' ')
            temp.erase(temp.begin(), temp.begin()+1);

        auto res = split_field(temp, delim); 

        if(res.size() > 3 )
            throw "Header Field - To\nToo large (expected: single line header of format To: \"Display name\" <uri;params>;params)" + path;

        if(res.size() == 3 )
        {
            // uri ; params -> syntax for delimiting results 
            // uri;params -> syntax when talking about uri or header parameters
            //case 1: "dis ; play" ; <uri;params>;params
            //case 2: "dis ; play" ; <uri>;params
            //case 3: "dis ; play" ; <uri;params>
            //
            //validate display name
            if(res[0].front() != '"') 
                throw "Header Field - To\nInvalid display name (expected: \"Display Name\")" + path;


            if(res[1].back() != '"')
                throw "Header Field - To\nInvalid display name (expected: \"Display Name\")" + path;

            //split second result into uri + params + header params 
            delim = ';';
            auto request_uri_params = split_field(res[2], delim);

            //case 1: <uri ; params> ; params
            //case 2: <uri ; params>
            //case 3: <uri> ; params
            if(request_uri_params[0].front() == '<')
            {
                //case 1 and 2 
                if(request_uri_params[0].back() != '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "To");
                }
                //case 3 : <uri> ; params
                else if(request_uri_params[0].back() == '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );
                    request_uri_params[0].pop_back();

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "To");
                }
                else
                    throw "Header Field - To\nInvalid URI (expected: should be enclosed in <>)" + path;
            }
            else
                throw "Header Field - To\nInvalid URI (expected: should be enclosed in <>)" + path;
        }
        else if (res.size() == 2 )
        {
            //case 1: "display" <uri;params>;params
            //case 2: "display" <uri>;params
            //case 3: "display" <uri;params>
            if(res[0].front() != '"') 
                throw "Header Field - To\nInvalid display name (expected: \"Display Name\")" + path;


            if(res[0].back() != '"')
                throw "Header Field - To\nInvalid display name (expected: \"Display Name\")" + path;

            //split second result into uri + params + header params 
            delim = ';';
            auto request_uri_params = split_field(res[1], delim);

            //case 1: <uri ; params> ; params
            //case 2: <uri ; params>
            //case 3: <uri> ; params
            if(request_uri_params[0].front() == '<')
            {
                //case 1 and 2 
                if(request_uri_params[0].back() != '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "To");
                }
                //<uri> ; params
                else if(request_uri_params[0].back() == '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );
                    request_uri_params[0].pop_back();

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "To");
                }
                else
                    throw "Header Field - To\nInvalid URI (expected: should be enclosed in <>)" + path;
            }
            else
                throw "Header Field - To\nInvalid URI (expected: should be enclosed in <>)" + path;

        }
        else if (res.size() == 1)
        {
            //split second result into uri + params + header params 
            delim = ';';
            auto request_uri_params = split_field(res[0], delim);

            //case 1: <uri ; params> ; params
            //case 2: <uri ; params>
            if(request_uri_params[0].front() == '<')
            {
                if(request_uri_params[0].back() != '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "To");
                }
                //<uri> ; params
                else if(request_uri_params[0].back() == '>')
                {
                    request_uri_params[0].erase(
                            request_uri_params[0].begin(),
                            request_uri_params[0].begin()+1
                            );
                    request_uri_params[0].pop_back();

                    delim = ':';
                    auto request_uri_fields = split_field(request_uri_params[0], delim);

                    check_uri(request_uri_fields, path, method_name, "To");
                }
                else
                    throw "Header Field - To\nInvalid URI (expected: should be enclosed in <>)" + path;
            }
            else
                throw "Header Field - To\nInvalid URI (expected: should be enclosed in <>)" + path;
        }
        else 
            throw "Header Field - To\nInvalid URI (expected: \"Display Name\" <uri;params>;params)" + path;

    }

    it = header_.find("Via");
    if(it  == header_.end())
        throw "Mandatory INVITE Request Header not found - Via" + path;
    else
    {
        //Via found, check if value is correct

        delim = ' ';
        std::string temp = it->second;

        //check for leading whitespace and delete it for better parsing 
        //RFC3261 - field-name: field-value
        if(temp.front() == ' ')
            temp.erase(temp.begin(), temp.begin()+1);

        auto res = split_field(temp,delim);

        if(res.size() > 2)
            throw "Header Field - Via\nToo large (expected: single line header)" + path;
        if(res.size() < 2)
            throw "Header Field - Via\nInvalid header (expected: Via: SIP/Version/Transport protocol NetworkAddr/HostName(:Port))" + path;

        delim = '/';
        auto sip_ver = split_field(res[0], delim);

        //RFC 3261 - Via: SIP/2.0/Transport address:port;params where port is optional
        if(sip_ver.size() != 3)
            throw "Header Field - Via\nInvalid SIP version or transport protocol (expected: SIP/2.0.Transport protocol)" + path;
        else if (sip_ver.size() == 3)
        {
            if(sip_ver[0] == "SIP" && sip_ver[1] == "2.0")
            {
                if(sip_ver[2] != "UDP")
                    if(sip_ver[2] != "TCP")
                        if(sip_ver[2] != "TLS")
                            if(sip_ver[2] != "SCTP")
                                throw "Header Field - Via\nInvalid transport protocol (expected: UDP, TCP, TLS or SCTP)" + path;
            }
            else
                throw "Header Field - Via\nInvalid SIP version (expected: SIP/2.0/)" + path;

            delim = ';';
            auto via_params = split_field(res[1], delim);
            if(via_params.size() > 1)
            {
                //RFC 3261 - branch parametere MUST be present 
                //and start with the magic cookie "z9hG4bK"
                //  std::string to_search = "branch=z9hG4bK";
                if(
                        //search for cookie in parameter vector
                        //returns true if any of the vector elements
                        //match the substring 
                        //if substring is not found throw exception
                        !std::any_of(
                            via_params.begin(), via_params.end(),
                            [](const std::string& str){
                            //returns true if the subsequence is found in the 
                            //vector element
                            return str.find("branch=z9hG4bK") != std::string::npos; 
                            } 
                            ) 
                  ) 
                    throw "Header Field - Via\nBranch parameter not found\n(expecting: SIP/2.0/Transport addr:port;branch=z9hG4bKa...;params)" + path;

                //check address and port
                delim = ':';
                auto addr_port = split_field(via_params[0], delim);

                //RFC 3261 - port is optional
                if(addr_port.size() == 2)
                {
                    //check address and port 
                    //valid ip address or host regex
                    std::regex rgx;
                    rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                    if(!std::regex_match(addr_port[0], rgx))
                    {
                        rgx = R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)";
                        if(!std::regex_match(addr_port[0], rgx))
                            throw "Header Field - Via\nInvalid Host/Network Address" + path;
                    }

                    //check port
                    if(addr_port[1].front() == '-')
                        throw "Header Field - Via\nInvalid port value (expected: must be positive)" + path;
                    try{
                        uint32_t port_number = std::stoi(addr_port[1]);
                        if(port_number > 65535)
                            throw "Header Field - Via\nInvalid port value (expected: must be < 65535)" + path;
                    }catch(std::invalid_argument& e){
                        throw "Header Field - Via\nInvalid port value" + path;
                    }
                }
                else if(addr_port.size() == 1)
                {
                    //check address
                    //valid ip address or host regex
                    std::regex rgx;
                    rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                    if(!std::regex_match(addr_port[0], rgx))
                    {
                        rgx = R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)";
                        if(!std::regex_match(addr_port[0], rgx))
                            throw "Header Field - Via\nInvalid Host/Network Address" + path;
                    }

                }
                else
                    throw "Header Field - Via\nInvalid Host/Network Address or Port (expected: addr:port)" + path;
            }
            else 
                throw "Header Field - Via\nBranch parameter not found\n(expecting: SIP/2.0/Transport addr:port;branch=z9hG4bKa...;params)" + path;

        }
    }

    ///////////////////////////////////////////////////////////////////
    //END HEADER VALIDATION
    ///////////////////////////////////////////////////////////////////
    
    //validate sdp 
    check_sdp(path);

    //if no exception thrown, header is valid
    return;

}

//check_uri method
//
//usage: validate sip uri using RFC3261 ruleset
//parameters:
//  - request_uri_fields : vector<string>& (has all the uri components)
//  - path : const string& (path to file where error is, if we throw it)
//  - method : const string& (name of method)
//  - header_field : const string& (name of header field)
void Sip::check_uri(std::vector<std::string>& request_uri_fields,
        const std::string& path,const std::string& method,
        const std::string& header_field)
{
///////////////////////////////////////////////////////////////////////////////
//IF REGISTER - we only have a host, no username in the request line 
///////////////////////////////////////////////////////////////////////////////
    if(header_.find("REGISTER") != header_.end() && header_field == "Request-Line")
    {
        // 2 cases
        // sip host port
        // sip host
        if(request_uri_fields[0] != "sip")
            if(request_uri_fields[0] != "sips")
                if(request_uri_fields[0] != "tel")
                    throw 
                        "Invalid " +  method + " - " + header_field + "\nBad URI (expected: sip or sips or tel)" + path;

        std::regex rgx;
        if(request_uri_fields.size() == 2)
        {
            //check for valid host

            //valid hostname or ip address regex
            rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";

            //if not valid host
            if(!std::regex_match(request_uri_fields[1], rgx))
            {
                rgx = R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)";
                if(!std::regex_match(request_uri_fields[1], rgx))
                    throw "Invalid " + method + " - " + header_field + "\nBad URI - Host (expected: ip address or domain name)" + path;

            }
        }
        else
            if(request_uri_fields.size() == 3)
            {
                //valid hostname or ip address regex
                rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                //check for valid host
                if(!std::regex_match(request_uri_fields[1], rgx))
                {
                    rgx = R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)";
                    if(!std::regex_match(request_uri_fields[1], rgx))
                        throw "Invalid " + method + " - " + header_field + "\nBad URI - Host (expected: ip address or domain name)" + path;

                }

                //check for valid port
                if(request_uri_fields[2].front() == '-')
                    throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number: must be positive)" + path;
                try{
                    uint32_t port_number = std::stoi(request_uri_fields[2]);
                    if(port_number > 65535)
                        throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number: must be less than 65535)" + path;
                }catch(std::invalid_argument& e){
                    throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number)" + path;
                }

            }
            else
                throw "Invalid " + method + " - " + header_field + "\nBad URI" + path;

        return;



    }

///////////////////////////////////////////////////////////////////////////////
//END IF REGISTER
///////////////////////////////////////////////////////////////////////////////

    //RFC3261 Request URI structure:
    //  sip:user:password@host:port
    //Notes:
    //  -passing the password is NOT recommended
    //  -host can be IP:PORT or domain name
    //      - as seen from packet capture, the port value is not always present 
    //  -a valid uri should start with sip: or sips: or tel:
    //  -the user can also be a phone number in some case

    //Cases:
    // 1. "sip", "user@host"
    // 2. "sip", "user", "password@host"
    // 3. "sip", "user@host", "port"
    // 4. "sip", "user", "password@host", "port"
    char delim;
    if(request_uri_fields.size() == 2) 
    {
        //case 1
        if(request_uri_fields[0] != "sip")
            if(request_uri_fields[0] != "sips")
                if(request_uri_fields[0] != "tel")
                    throw "Invalid " + method + " - " + header_field + "\nBad URI (expected: sip or sips or tel)" + path;

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
                    throw "Invalid " + method + " - " + header_field + "\nBad URI - User (expected: username or phone number or sip extension)" + path;
            }
        }

        //valid hostname or ip address regex
        rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";

        //if not valid host
        if(!std::regex_match(user_host[1], rgx))
        {
            rgx = R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)";
            if(!std::regex_match(user_host[1], rgx))
                throw "Invalid " + method + " - " + header_field + "\nBad URI - Host (expected: ip address or domain name)" + path;
        }
    }else
        if(request_uri_fields.size() == 3) 
        {
            if(request_uri_fields[0] != "sip")
                if(request_uri_fields[0] != "sips")
                    if(request_uri_fields[0] != "tel")
                        throw "Invalid " + method + " - " + header_field + "\nBad URI (expected: sip or sips or tel)" + path;

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
                            throw "Invalid " + method + " - " + header_field + "\nBad URI - User (expected: username or phone number or sip extension)" + path;
                    }
                }

                auto password_host_field = split(request_uri_fields[2], delim); 
                if(password_host_field.size() != 2)
                    throw "Invalid " + method + " - " + header_field + "\nBad URI" + path;

                //valid ip address or hostname regex
                rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                if(!std::regex_match(password_host_field[1], rgx))
                {
                    rgx = R"(^(([a-zA-Z0-9]|[a-zA-Z0-9][a-zA-Z0-9\-]*[a-zA-Z0-9])\.)*([A-Za-z0-9]|[A-Za-z0-9][A-Za-z0-9\-]*[A-Za-z0-9])$)";
                    if(!std::regex_match(password_host_field[1], rgx))
                        throw "Invalid " + method + " - " + header_field + "\nBad URI - Host (expected: ip address or domain name)" + path;
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
                            throw "Invalid " + method + " - " + header_field + "\nBad URI - User (expected: username or phone number or sip extension)" + path;
                    }
                }

                //only check for valid IP address
                //because we also have a port number 
                rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
                if(!std::regex_match(user_host[1], rgx))
                    throw "Invalid " + method + " - " + header_field + "\nBad URI - Host (expected: ip address or domain name)" + path;

                //check port
                if(request_uri_fields[2].front() == '-')
                    throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number: must be positive)" + path;
                try{
                    uint32_t port_number = std::stoi(request_uri_fields[2]);
                    if(port_number > 65535)
                        throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number: must be less than 65535)" + path;
                }catch(std::invalid_argument& e){
                    throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number)" + path;
                }
            }
        }
        else if(request_uri_fields.size() == 4) 
        {
            // case 4: "sip", "user", "password@host", "port"
            if(request_uri_fields[0] != "sip")
                if(request_uri_fields[0] != "sips")
                    if(request_uri_fields[0] != "tel")
                        throw "Invalid " + method + " - " + header_field + "\nBad URI (expected: sip or sips or tel)" + path;

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
                        throw "Invalid " + method + " - " + header_field + "\nBad URI - User (expected: username or phone number or sip extension)" + path;
                }
            }

            auto password_host_field = split(request_uri_fields[2], delim); 
            if(password_host_field.size() != 2)
                throw "Invalid " + method + " - " + header_field + "\nBad URI" + path;


            //only check for valid IP address
            //because we also have a port number 
            rgx = R"(^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$)";
            if(!std::regex_match(password_host_field[1], rgx))
                throw "Invalid " + method + " - " + header_field + "\nBad URI - Host (expected: ip address or domain name)" + path;

            //check port
            if(request_uri_fields[3].front() == '-')
                throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number: must be positive)" + path;
            try{
                uint32_t port_number = std::stoi(request_uri_fields[3]);
                if(port_number > 65535)
                    throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number: must be less than 65535)" + path;
            }catch(std::invalid_argument& e){
                throw "Invalid " + method + " - " + header_field + "\nBad URI - Port(invalid port number)" + path;
            }


        }else 
            throw "Invalid " + method + " - " + header_field + "\nBad URI" + path;
}

//check_sdp method
//
//usage: validate SDP message body
//parameters:
//  path : const string& (path to file where error is, if we throw it)
void Sip::check_sdp(const std::string& path)
{

    ///////////////////////////////////////////////////////////////////////
    //Validate SDP 
    ////////////////////////////////////////////////////////////////////////


    //3 cases
    //Case 1: SDP info found but invalid Content-Length or Content-Type
    //Case 2: Content-Length found, but invalid SDP header fields and Content-Type
    //Case 3: Content-Type found but invalid SDP header fields and Content-Length

    //Case 2
    auto it = header_.find("Content-Length");
    if(it != header_.end())
    {
        //found it 

        //if no content type
        if(it->second != " 0")
        {
            std::string temp = it->second;

            if(temp.front() == ' ')
                temp.erase(temp.begin(),temp.begin()+1);

            if(temp.front() == '-')
                throw "Header Field - Content-Length\nInvalid length (expected: must be positive)" + path;

            try{
                uint32_t content_length = std::stoi(temp); 
            }catch(std::invalid_argument& ex){
                throw "Header Field - Content-Length\nInvalid length (expected: value should be a number)" + path;
            }


            if((it = header_.find("Content-Type")) == header_.end())
                throw "Header Field - Content Type missing (expected: Content-Type: application/sdp..etc)" + path;
            else
            {
                std::unordered_multimap<std::string,std::string>::iterator iter;
                //if you find sdp data, content type should be application/sdp 
                if(
                        (iter = header_.find("v")) != header_.end()&&
                        (iter = header_.find("o")) != header_.end()&&
                        (iter = header_.find("s")) != header_.end() &&
                        (iter = header_.find("t")) != header_.end() &&
                        (iter = header_.find("m")) != header_.end()
                  )
                {
                    if(it->second != " application/sdp")
                        throw "Header Field - Content-Type (expected: application/sdp)" + path;
                }
                else 
                    throw "SDP data incomplete or not found" + path;
            }
        }
        else
        {
            //check if there is any sdp data or any content-type data and throw
            std::unordered_multimap<std::string,std::string>::iterator iter;
            //if you find sdp data, content type should be application/sdp 
            if(
                    (iter = header_.find("v")) != header_.end()&&
                    (iter = header_.find("o")) != header_.end()&&
                    (iter = header_.find("s")) != header_.end() &&
                    (iter = header_.find("t")) != header_.end() &&
                    (iter = header_.find("m")) != header_.end()
              )
                throw "SDP Error - SDP data found, but no content length specified\n(expected: Content-Length: >0)" + path;
            
            if((iter = header_.find("Content-Type")) != header_.end())
                throw "SDP Error - SDP data found, but no content type specified\n(expected: Content-Type: ...)" + path;
        }

    }
    else
        //Case 3
        if((it = header_.find("Content-Type")) != header_.end())
        {
            auto iter = header_.find("Content-Length");

            if(iter != header_.end())
            {
                if(iter->second == " 0")
                    throw "Header Field - Content-Length (expected: value > 0)" + path;
                else 
                {
                    std::string temp = iter->second;

                    if(temp.front() == ' ')
                        temp.erase(temp.begin(),temp.begin()+1);

                    if(temp.front() == '-')
                        throw "Header Field - Content-Length\nInvalid length (expected: must be positive)" + path;

                    try{
                        uint32_t content_length = std::stoi(temp); 
                    }catch(std::invalid_argument& ex){
                        throw "Header Field - Content-Length\nInvalid length (expected: value should be a number)" + path;
                    }
                }
            }
            else
                throw "Header Field - Content-Length missing" + path;

            if(
                    (iter = header_.find("v")) != header_.end()&&
                    (iter = header_.find("o")) != header_.end()&&
                    (iter = header_.find("s")) != header_.end() &&
                    (iter = header_.find("t")) != header_.end() &&
                    (iter = header_.find("m")) != header_.end()
              )
            {
                if(it->second != " application/sdp")
                    throw "Header Field - Content-Type (expected: application/sdp)" + path;
            }
            else
                throw "SDP data incomplete or not found" + path;

        }
        else
            //case 1:
            if(
                    (it = header_.find("v")) != header_.end()&&
                    (it = header_.find("o")) != header_.end()&&
                    (it = header_.find("s")) != header_.end() &&
                    (it = header_.find("t")) != header_.end() &&
                    (it = header_.find("m")) != header_.end()
              )
            {
                auto iter = header_.find("Content-Type");

                if(iter != header_.end())
                {
                    if(iter->second != " application/sdp")
                        throw "Header Field - Content-Type (expected: application/sdp)" + path;

                    iter = header_.find("Content-Length");

                    if(iter != header_.end())
                    {
                        if(iter->second == " 0")
                            throw "Header Field - Content-Length (expected: value > 0)" + path;
                        else 
                        {
                            std::string temp = iter->second;

                            if(temp.front() == ' ')
                                temp.erase(temp.begin(),temp.begin()+1);

                            if(temp.front() == '-')
                                throw "Header Field - Content-Length\nInvalid length (expected: must be positive)" + path;

                            try{
                                uint32_t content_length = std::stoi(temp); 
                            }catch(std::invalid_argument& ex){
                                throw "Header Field - Content-Length\nInvalid length (expected: value should be a number)" + path;
                            }
                        }
                    }
                    else
                        throw "Header Field - Content-Length missing" + path;
                }
                else
                    throw "Header Field - Content-Type missing" + path;

            }
}

//getters for header and header order
std::vector<std::string> Sip::get_header_order() const
{
    return h_order_;
}
std::unordered_multimap<std::string, std::string> Sip::get_header() const
{
    return header_;
}
