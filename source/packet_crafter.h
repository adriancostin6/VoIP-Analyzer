#ifndef PACKET_CRAFTER_H
#define PACKET_CRAFTER_H
#include <string>
#include <vector>

#include <tins/tins.h>

#include "sip.h"

class PacketCrafter
{
    public:
        PacketCrafter();

        //craft the packet 
        void craft_sip_packet(const std::string& sip_data, bool is_filename);

        //send packets down the stream
        void send_packets();

        bool get_user_input();
    private:
        std::vector<Sip> packets_;
        bool to_check, has_sdp;
};
#endif
