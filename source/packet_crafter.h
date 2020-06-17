#ifndef PACKET_CRAFTER_H
#define PACKET_CRAFTER_H
#include <string>
#include <vector>

#include <tins/tins.h>

#include "sip.h"

class PacketCrafter
{
    public:
        PacketCrafter():

        //craft the packet 
        void craft_sip_packet(std::string& sip_data);

        //send packets down the stream
        void send_packets();
    private:
        std::vector<Sip> packets_;
}
#endif
