#include "packet_crafter.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>
#include <iostream>

#include <tins/tins.h>

#include "console_ui.h"

using namespace Tins;

void PacketCrafter::craft_sip_packet(const std::string& data, uint8_t& p_num, bool is_filename)
{

    //if our string data is a filename, read the packet from the file
    if(is_filename)
    {
        std::ifstream ifs(data);
        if(ifs)
        {
            std::string sip_data(std::istreambuf_iterator<char>{ifs}, {});

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
        sip_packet.print("../temp/keyboard/keyboard_created_packet_",p_num);
        p_num++;
        try{
            //check packet
            std::string path = 
                "../temp/keyboard/keyboard_created_packet_" + 
                std::to_string(p_num-1);

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


void PacketCrafter::send_packets()
{
    NetworkInterface iface = NetworkInterface::default_interface();
    
    NetworkInterface::Info info = iface.addresses();

    std::string payload = "";
    char delim = ':';
    //run loop on all packets
    for(Sip& packet : packets_)
    {
        auto map = packet.get_header();
        auto order = packet.get_header_order();

        //used to iterate over duplicates using std::next
        int c_iter = 1;

        for(int i=0;i<order.size();++i)
        {
            auto& key = order[i];
            auto pair = map.find(key);

            //if key not found exit
            if(pair == map.end())
                return;

            uint8_t count = map.count(key);


            if(count == 1)
            {        

                //if the value of the header field 
                //does not start with a space and
                //the value of the key is in all caps it 
                //means it is a request line
                //
                //any_of returns true if any of the characters
                //in the key are lowercase, and we negate the 
                //return value so that the if statement only 
                //executes if all the characters in the key are caps 
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
                    payload += pair->first;
                    payload += ' ';
                    payload +=pair->second;
                    payload += "\r\n";

                    //go to next header
                    continue;
                }

                if(pair->first.size() == 1)
                    delim = '=';
                payload += pair->first;
                payload += delim;
                payload += pair->second;
                payload += "\r\n";

                //check if we have sdp data
                if(i != order.size()-1 && order[i+1] == "v")
                    payload += "\r\n";
                else if(i == order.size()-1 && delim == ':')
                    payload += "\r\n";
            } 
            else
            {
                auto range  = map.equal_range(key);

                //goto last duplicate
                //we need to add the elements to the payload 
                //in reverse order because that's how 
                //duplicates are stored in the unordered_multiap
                //implementation
                auto last = std::next(range.first, count-c_iter);

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
                c_iter++;
            }
        }
       //        std::cout << payload;

        //create ethernet packet 
        EthernetII eth = EthernetII("08:00:27:5C:AC:74", info.hw_addr) / 
            IP("192.168.1.8", info.ip_addr) /
            UDP(5060, 65534) /
            RawPDU(payload);

        PacketSender sender;

        sender.send(eth,iface);
    }    
}

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


PacketCrafter::PacketCrafter() : to_check(false), has_sdp(false) {};
