#ifndef PACKET_CRAFTER_H
#define PACKET_CRAFTER_H
#include <string>
#include <vector>

#include <tins/tins.h>

#include "sip.h"

//PacketCrafter class
//Used for creating packets from input files or command line input
//Parameters:
//  - packets_ : vector<Sip> (stores created packets)
//  - to_check : bool (used for turning packet checker on or off)
//  - has_sdp : bool (used for showing the pressence of a message body)
//Operations:
//  - PacketCrafter() : constructor
//  - craft_sip_packet() : creates a Sip object from text file or command line input
//  - send_packets() : sends packets down the network
//  - get_user_input : command line interface for getting user input data
class PacketCrafter
{
    public:
        PacketCrafter();

        //craft the packet 
        void craft_sip_packet(const std::string& sip_data, uint8_t& p_num, bool is_filename);

        //send packets down the stream
        void send_packets();

        bool get_user_input(uint8_t& p_num);
    private:
        std::vector<Sip> packets_;
        bool to_check, has_sdp;
};
#endif
