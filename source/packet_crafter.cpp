#include "packet_crafter.h"

#include <fstream>
#include <sstream>
#include <cctype>
#include <algorithm>


using namespace Tins;

void PacketCrafter::craft_sip_packet(const std::string& data, bool is_filename)
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
                sip_packet.check_packet(data);
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

        //check packet
        sip_packet.check_packet("keyboard");

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

void PacketCrafter::get_user_input()
{

}

PacketCrafter::PacketCrafter(){};
