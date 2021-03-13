#include "packet_crafter.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iostream>

#include <tins/tins.h>

#include "console_ui.h"

using namespace Tins;

//craft_sip_packet method
//creates a sip packet from user defined data, either from an input text file
//or from command line input
//parameters:
//  - data : const string& (filename or actual data string)
//  - p_num : uint& (keeps track of number of packets)
//  - is_filename : bool (true if data is a filename)
//usage: after constructing and validating the packet(if necessary), the method
//pushes the result to a Sip vector for storage
void PacketCrafter::craft_sip_packet(const std::string& data, uint8_t& p_num, bool is_filename)
{

    //if our string data is a filename, read the packet from the file
    if(is_filename)
    {
        std::ifstream ifs(data);
        if(ifs)
        {
            //create a string from file data stream
            std::string sip_data(std::istreambuf_iterator<char>{ifs}, {});

            //create Sip object with the data
            Sip sip_packet(sip_data);
            try{
                to_check = true;
                sip_packet.check_packet(data, to_check);
            }
            catch(const std::string& sip_ex){
                std::cout << "Error crafting SIP packet: ";
                std::cout << sip_ex << "\n";

                //if exception was thrown, exit function
                return;
            }
            //if no exceptions were thrown add packet to sip vector
            packets_.push_back(sip_packet);
        }
        else 
            std::cout << "Could not find file: " << data; 
    }
    else
    {
        //build packet
        Sip sip_packet(data);
#ifdef _WIN32
        sip_packet.print("../../temp/keyboard/keyboard_created_packet_",p_num);
#else
        sip_packet.print("../temp/keyboard/keyboard_created_packet_", p_num);
#endif
        p_num++;
        try{
            //check packet
#ifdef _WIN32
            std::string path = 
                "../../temp/keyboard/keyboard_created_packet_" + 
                std::to_string(p_num-1);
#else
            std::string path =
                "../temp/keyboard/keyboard_created_packet_" +
                std::to_string(p_num - 1);
#endif

            sip_packet.check_packet(path, to_check);
        }
        catch(const std::string& sip_ex){
            std::cout << "Error crafting SIP packet: ";
            std::cout << sip_ex << "\n";

            //if exception was thrown, exit function
            return;
        }
        //add it to vector
        packets_.push_back(sip_packet);
    }
}


//send packets method
//
//sends all stored packets over the default interface
void PacketCrafter::send_packets()
{
    //get default interface information
    NetworkInterface iface = NetworkInterface::default_interface();
    NetworkInterface::Info info = iface.addresses();

    char delim = ':';
    //run loop on all packets
    for(Sip& packet : packets_)
    {
        //reset payload after sending each packet
        std::string payload = "";
        //get the header and the header key order
        auto map = packet.get_header();
        auto order = packet.get_header_order();

        //used to iterate over duplicates using std::next
        int c_iter = 1;

        for(int i=0;i<order.size();++i)
        {
            //get key value pair for key in header order vector
            auto& key = order[i];
            auto pair = map.find(key);

            //if key not found exit
            if(pair == map.end())
                return;

            //return number of times key is found
            uint8_t count = map.count(key);


            if(count == 1)
            {        

                //if any_of the characters in the key is lowercase and the value
                //does not start with a space, we have a request field so we must
                //add it to the payload with a a space in between the method name
                //and method value
                if(
                        pair->second.front() != ' ' && 
                        !std::any_of(
                            pair->first.begin(),
                            pair->first.end(),
                            [](char c)
                            {
                                return islower(c);
                            }
                            )
                        )
                {
                    //add to payload with proper formatting
                    payload += pair->first;
                    payload += ' ';
                    payload +=pair->second;
                    payload += "\r\n";

                    //go to next header
                    continue;
                }

                //if we encounter SDP data
                if(pair->first.size() == 1)
                    delim = '=';
                payload += pair->first;
                payload += delim;
                payload += pair->second;
                payload += "\r\n";

                //check if we have sdp data
                //add CRLF accordingly
                if(i != order.size()-1 && order[i+1] == "v")
                    payload += "\r\n";
                else if(i == order.size()-1 && delim == ':')
                    payload += "\r\n";
            } 
            else
            {
                //get range of keys if duplicates are present
                auto range  = map.equal_range(key);

                //goto last duplicate
                //we need to add the elements to the payload 
                //in reverse order because that's how 
                //duplicates are stored in the unordered_multiap
                //implementation
                auto last = std::next(range.first, count-c_iter);

                //if we encounter SDP data
                if(last->first.size() == 1)
                    delim = '=';
                payload += last->first;
                payload += delim;
                payload += last->second;
                payload += "\r\n";

                //check if we have sdp data
                if(i != order.size()-1 && order[i+1] == "v")
                    payload += "\r\n";
                else if(i == order.size()-1 && delim == ':')
                    payload += "\r\n";

                //increment counter used for iterating the list in reverse
                c_iter++;
            }
        }

        //create ethernet packet 
        EthernetII eth = EthernetII("08:00:27:5C:AC:74", info.hw_addr) / 
            IP("192.168.1.8", info.ip_addr) /
            UDP(5060, 65534) /
            RawPDU(payload);

        //create a packet sender object
        PacketSender sender;

        //send packet 
        sender.send(eth,iface);
    }    
}

//get user input method
//
//gets input from the command line and creates a Sip packet
//parameters: 
//  - p_num : uint& (indicates the package number)
//returns: bool (true on successful creation)
bool PacketCrafter::get_user_input(uint8_t& p_num)
{
    std::string user_input;
    std::string payload = "";

    bool is_invite = false;

    std::cout<< ConsoleUi::intro_text;

    //ask user if they want help
    std::getline(std::cin,user_input);

    if(user_input == "help")
    {
        std::cout<<ConsoleUi::help;
        std::getline(std::cin,user_input);

        if(user_input == "yes")
        {
            //read user input
            std::cout << ConsoleUi::header_check;

            //turn header checking on or off
            std::getline(std::cin, user_input);

            if(user_input == "yes")
                to_check = true;

            std::cout << ConsoleUi::packet_type;

            std::getline(std::cin, user_input);

            if(user_input == "response" && to_check)
            {
                std::cout<<"This application cannot check SIP responses, try again.\n";
                return false;
            }

            if(user_input == "response")
            {
                //create a response line
                std::cout << ConsoleUi::response_type;
            
                std::getline(std::cin ,user_input);
                payload += user_input;
                payload+="\r\n";
            }
            else 
            {
                //create a request line 
                std::cout << ConsoleUi::request_type;

                std::getline(std::cin, user_input);

                if(user_input != "INVITE")
                    if(user_input != "ACK")
                        if(user_input != "REGISTER")
                            if(user_input != "OPTIONS")
                                if(user_input != "BYE")
                                    if(user_input != "CANCEL" && to_check)
                                    {
                                        std::cout << "Header checking only works for: INVITE, ACK, REGISTER, OPTIONS, BYE, CANCEL. Try again\n";
                                        return false;
                                    }


                payload += user_input;
                payload += " ";

                if(user_input == "INVITE")
                    is_invite = true;

                std::cout << ConsoleUi::request_uri;
                std::getline(std::cin, user_input);
                payload += user_input; 
                payload += " ";

                std::cout << ConsoleUi::sip_version;
                std::getline(std::cin, user_input);
                payload += user_input;
                payload += "\r\n";
            }

            std::cout << ConsoleUi::via_field;
            std::getline(std::cin, user_input);
            payload += user_input;
            payload += " ";

            std::cout << ConsoleUi::via_addr;
            std::getline(std::cin, user_input);
            payload += user_input;
            payload += ";";

            std::cout << ConsoleUi::via_params;
            std::getline(std::cin, user_input);
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::max_forwards;
            std::getline(std::cin, user_input);
            payload += "Max-Forwards: ";
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::to_field_value;
            std::getline(std::cin, user_input);
            payload += "To: ";
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::from_field_value;
            std::getline(std::cin, user_input);
            payload += "From: ";
            payload += user_input;
            payload += "\r\n";

            if(is_invite)
            {
                std::cout << ConsoleUi::contact_field_value;
                std::getline(std::cin, user_input);

                payload += "Contact: ";
                payload += user_input;
                payload += "\r\n";
            }

            std::cout << ConsoleUi::call_id_value;
            std::getline(std::cin, user_input);
            payload += "Call-ID: ";
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::cseq_val;
            std::getline(std::cin, user_input);
            payload += "CSeq: ";
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::other_header_info;
            std::getline(std::cin, user_input);

            if(user_input == "yes")
            {   
                while(true)
                {
                    std::cout<<"\nInsert additional header. (expected: header: value)\n";
                    std::getline(std::cin,user_input);
                    payload += user_input;
                    payload += "\r\n";

                    std::cout<<"Insert another additional header? (yes/no)\n";
                    std::getline(std::cin,user_input);
                    if(user_input != "yes")
                        break;
                }
            }

            std::cout << ConsoleUi::has_sdp_body;
            std::getline(std::cin,user_input);

            if(user_input == "yes")
            {
                std::cout << ConsoleUi::content_type;
                std::getline(std::cin, user_input);
                payload += "Content-Type: ";
                payload += user_input;
                payload += "\r\n";

                std::cout << ConsoleUi::content_length;
                std::getline(std::cin, user_input);
                payload += "Content-Length: ";
                payload += user_input;
                payload += "\r\n";

                //end of header fields start of message body
                payload += "\r\n";


                std::cout << ConsoleUi::sdp_v;
                std::getline(std::cin, user_input);
                payload += user_input;
                payload += "\r\n";

                std::cout << ConsoleUi::sdp_o;
                std::getline(std::cin, user_input);
                payload += user_input;
                payload += "\r\n";

                std::cout << ConsoleUi::sdp_v;
                std::getline(std::cin, user_input);
                payload += user_input;
                payload += "\r\n";

                std::cout << ConsoleUi::sdp_m;
                std::getline(std::cin, user_input);
                payload += user_input;
                payload += "\r\n";

                std::cout << ConsoleUi::sdp_a_check;
                std::getline(std::cin, user_input);

                if(user_input == "yes")
                {   
                    while(true)
                    {
                        std::cout<<ConsoleUi::sdp_a;
                        std::getline(std::cin,user_input);
                        payload += user_input;
                        payload += "\r\n";

                        std::cout<<"Insert another additional header? (yes/no)\n";
                        std::getline(std::cin,user_input);
                        if(user_input != "yes")
                            break;
                    }
                }
            }
            craft_sip_packet(payload, p_num ,false);
            return true;
        }
        std::cout<<"Terminating package creator...\n";
        return false;
    }
    else
    {
        std::cout << ConsoleUi::header_check;

        //turn header checking on or off
        std::getline(std::cin, user_input);

        if(user_input == "yes")
            to_check = true;

        std::cout << ConsoleUi::packet_type;

        std::getline(std::cin, user_input);

        if(user_input == "response" && to_check)
        {
            std::cout<<"This application cannot check SIP responses, try again.\n";
            return false;
        }
        if(user_input == "response")
        {
            //create a response line
            std::cout << ConsoleUi::response_type;

            std::getline(std::cin ,user_input);
            payload += user_input;
            payload+="\r\n";
        }
        else
        {
            //create request line
            std::cout << ConsoleUi::request_type;

            std::getline(std::cin, user_input);

            if(user_input != "INVITE")
                if(user_input != "ACK")
                    if(user_input != "REGISTER")
                        if(user_input != "OPTIONS")
                            if(user_input != "BYE")
                                if(user_input != "CANCEL" && to_check)
                                {
                                    std::cout << "Header checking only works for: INVITE, ACK, REGISTER, OPTIONS, BYE, CANCEL. Try again\n";
                                    return false;
                                }

            payload += user_input;
            payload += " ";

            if(user_input == "INVITE")
                is_invite = true;

            std::cout << ConsoleUi::request_uri;
            std::getline(std::cin, user_input);
            payload += user_input; 
            payload += " ";

            std::cout << ConsoleUi::sip_version;
            std::getline(std::cin, user_input);
            payload += user_input;
            payload += "\r\n";
        }
        std::cout << ConsoleUi::via_field;
        std::getline(std::cin, user_input);
        payload += "Via: ";
        payload += user_input;
        payload += " ";

        std::cout << ConsoleUi::via_addr;
        std::getline(std::cin, user_input);
        payload += user_input;
        payload += ";";

        std::cout << ConsoleUi::via_params;
        std::getline(std::cin, user_input);
        payload += user_input;
        payload += "\r\n";

        std::cout << ConsoleUi::max_forwards;
        std::getline(std::cin, user_input);
        payload += "Max-Forwards: ";
        payload += user_input;
        payload += "\r\n";

        std::cout << ConsoleUi::to_field_value;
        std::getline(std::cin, user_input);
        payload += "To: ";
        payload += user_input;
        payload += "\r\n";

        std::cout << ConsoleUi::from_field_value;
        std::getline(std::cin, user_input);
        payload += "From: ";
        payload += user_input;
        payload += "\r\n";

        if(is_invite)
        {
            std::cout << ConsoleUi::contact_field_value;
            std::getline(std::cin, user_input);

            payload += "Contact: ";
            payload += user_input;
            payload += "\r\n";
        }

        std::cout << ConsoleUi::call_id_value;
        std::getline(std::cin, user_input);
        payload += "Call-ID: ";
        payload += user_input;
        payload += "\r\n";

        std::cout << ConsoleUi::cseq_val;
        std::getline(std::cin, user_input);
        payload += "CSeq: ";
        payload += user_input;
        payload += "\r\n";

        std::cout << ConsoleUi::other_header_info;
        std::getline(std::cin, user_input);

        if(user_input == "yes")
        {   
            while(true)
            {
                std::cout<<"\nInsert additional header. (expected: header: value)\n";
                std::getline(std::cin,user_input);
                payload += user_input;
                payload += "\r\n";

                std::cout<<"Insert another additional header? (yes/no)\n";
                std::getline(std::cin,user_input);
                if(user_input != "yes")
                    break;
            }
        }

        std::cout << ConsoleUi::has_sdp_body;
        std::getline(std::cin,user_input);

        if(user_input == "yes")
        {
            std::cout << ConsoleUi::content_type;
            std::getline(std::cin, user_input);
            payload += "Content-Type: ";
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::content_length;
            std::getline(std::cin, user_input);
            payload += "Content-Length: ";
            payload += user_input;
            payload += "\r\n";

            //end of header fields start of message body
            payload += "\r\n";


            std::cout << ConsoleUi::sdp_v;
            std::getline(std::cin, user_input);
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::sdp_o;
            std::getline(std::cin, user_input);
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::sdp_v;
            std::getline(std::cin, user_input);
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::sdp_m;
            std::getline(std::cin, user_input);
            payload += user_input;
            payload += "\r\n";

            std::cout << ConsoleUi::sdp_a_check;
            std::getline(std::cin, user_input);

            if(user_input == "yes")
            {   
                while(true)
                {
                    std::cout<<ConsoleUi::sdp_a;
                    std::getline(std::cin,user_input);
                    payload += user_input;
                    payload += "\r\n";

                    std::cout<<"Insert another additional header? (yes/no)\n";
                    std::getline(std::cin,user_input);
                    if(user_input != "yes")
                        break;
                }
            }
        }
        craft_sip_packet(payload, p_num, false);
        return true;
    }
}


//constructor with member initializer list
PacketCrafter::PacketCrafter() : to_check(false), has_sdp(false) {};
